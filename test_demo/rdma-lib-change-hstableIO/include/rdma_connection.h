/*
 * An example RDMA client side code. 
 * Author: Animesh Trivedi 
 *         atrivedi@apache.org
 */

#ifndef RDMA_CONN
#define RDMA_CONN 1

#include "rdma_common.h"
#include "slab.h"

class client_connection {
public:
	/* These are basic RDMA resources */
	/* These are RDMA connection related resources */
	 struct rdma_event_channel *cm_event_channel = NULL;
	 struct rdma_cm_id *cm_client_id = NULL;
	 struct ibv_pd *pd = NULL;
	 struct ibv_comp_channel *io_completion_channel = NULL;
	 struct ibv_cq *client_cq = NULL;
	 struct ibv_qp_init_attr qp_init_attr;
	 struct ibv_qp *client_qp;
	/* These are memory buffers related resources */
	 struct ibv_mr *client_metadata_mr = NULL, 
			     *client_src_mr = NULL, 
			     *client_dst_mr = NULL, 
			     *server_metadata_mr = NULL;
	 struct rdma_buffer_attr client_metadata_attr, server_metadata_attr;
	 struct ibv_send_wr client_send_wr, *bad_client_send_wr = NULL;
	 struct ibv_recv_wr server_recv_wr, *bad_server_recv_wr = NULL;
	 struct ibv_sge client_send_sge, server_recv_sge;
	/* Source and Destination buffers, where RDMA operations source and sink */
	 char *src = NULL, *dst = NULL; 


	client_connection();



	/* This is our testing function */
	 int check_src_dst();
	
	 /* This function prepares client side connection resources for an RDMA connection */
	 int client_prepare_connection(struct sockaddr_in *s_addr);

	/* Pre-posts a receive buffer before calling rdma_connect () */
	 int client_pre_post_recv_buffer();

	/* Connects to the RDMA server */
	 int client_connect_to_server(); 

	/* Exchange buffer metadata with the server. The client sends its, and then receives
	 * from the server. The client-side metadata on the server is _not_ used because
	 * this program is client driven. But it shown here how to do it for the illustration
	 * purposes
	 */
	 int client_xchange_metadata_with_server();

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	 int client_remote_memory_ops(); 

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	 int client_remote_memory_string_swap_out_ops();

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 */ 
	 int client_remote_memory_string_put_ops(char* srcc, uint32_t start_index, size_t len); 

	 int client_remote_memory_string_swap_in_ops(); 

	 int client_remote_memory_string_get_ops(char* dst, uint32_t start_index, size_t len); 

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	 int client_remote_memory_array_ops(); 

	 int client_remote_memory_array_swap_out_ops(uint64_t* srcc, uint64_t r_addr, uint64_t length);
	int rdma_write(const void* srcc, uint64_t r_addr, uint64_t length);

	 int client_remote_memory_array_swap_in_ops(uint64_t* dstt, uint64_t r_addr, uint64_t length );

	 int rdma_read(const void* dstt, uint64_t r_addr, uint64_t length );

	struct ibv_mr* prepare_mr(struct ibv_pd* pd, char* buf, uint32_t len) ;

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	 int client_remote_memory_read(void* ctx,  char* dst, uint32_t len, 
			uint32_t lkey, char* remote_addr, uint32_t rkey); 

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 2) RDMA write from src -> remote buffer 
	 */ 
	 int client_remote_memory_write(void* ctx, char* src, uint32_t len, 
			uint32_t lkey, char* remote_addr, uint32_t rkey); 

	/* This function disconnects the RDMA connection from the server and cleans up 
	 * all the resources.
	 */
	 int client_disconnect_and_clean();


};

/*

void usage() {
	printf("Usage:\n");
	printf("rdma_client: [-a <server_addr>] [-p <server_port>] -s string (required)\n");
	printf("(default IP is 127.0.0.1 and port is %d)\n", DEFAULT_RDMA_PORT);
	exit(1);
}

int main(int argc, char **argv) {
	struct sockaddr_in server_sockaddr;
	int ret, option;
	bzero(&server_sockaddr, sizeof server_sockaddr);
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	// buffers are NULL 
	src = dst = NULL; 
	// Parse Command Line Arguments 
	while ((option = getopt(argc, argv, "s:a:p:")) != -1) {
		switch (option) {
			case 's':
				printf("Passed string is : %s , with count %u \n", 
						optarg, 
						(unsigned int) strlen(optarg));
				src = (char*)calloc(strlen(optarg) , 1);
				if (!src) {
					rdma_error("Failed to allocate memory : -ENOMEM\n");
					return -ENOMEM;
				}
				// Copy the passes arguments //
				strncpy(src, optarg, strlen(optarg));
				dst = (char*)calloc(strlen(optarg), 1);
				if (!dst) {
					rdma_error("Failed to allocate destination memory, -ENOMEM\n");
					free(src);
					return -ENOMEM;
				}
				break;
			case 'a':
				// remember, this overwrites the port info 
				ret = get_addr(optarg, (struct sockaddr*) &server_sockaddr);
				if (ret) {
					rdma_error("Invalid IP \n");
					return ret;
				}
				break;
			case 'p':
				// passed port to listen on 
				server_sockaddr.sin_port = htons(strtol(optarg, NULL, 0)); 
				break;
			default:
				usage();
				break;
		}
	}
	if (!server_sockaddr.sin_port) {
		// no port provided, use the default port 
		server_sockaddr.sin_port = htons(DEFAULT_RDMA_PORT);
	}
	if (src == NULL) {
		printf("Please provide a string to copy \n");
		usage();
	}
	ret = client_prepare_connection(&server_sockaddr);
	if (ret) { 
		rdma_error("Failed to setup client connection , ret = %d \n", ret);
		return ret;
	}
	ret = client_pre_post_recv_buffer(); 
	if (ret) { 
		rdma_error("Failed to setup client connection , ret = %d \n", ret);
		return ret;
	}
	ret = client_connect_to_server();
	if (ret) { 
		rdma_error("Failed to setup client connection , ret = %d \n", ret);
		return ret;
	}
	ret = client_xchange_metadata_with_server();
	if (ret) {
		rdma_error("Failed to setup client connection , ret = %d \n", ret);
		return ret;
	}

	//ret = client_remote_memory_ops();
	//if (ret) {
	//	rdma_error("Failed to finish remote memory ops, ret = %d \n", ret);
	//	return ret;
	//}

	ret = client_remote_memory_string_swap_out_ops();
	if (ret) {
		rdma_error("Failed to finish remote memory string out ops, ret = %d \n", ret);
		return ret;
	}
	//client_remote_memory_array_ops
	ret = client_remote_memory_string_swap_in_ops();
	if (ret) {
		rdma_error("Failed to finish remote memory string in ops, ret = %d \n", ret);
		return ret;
	}
	printf("start put and get \n");
	//client_remote_memory_string_put_ops
	char* srcc = new char[5];
	//char* srcc = "klxkl";
	srcc[0] = 'k';
	srcc[1] = 'l';
	srcc[2] = 'k';
	srcc[3] = 'x';
	srcc[4] = 'k';
	ret = client_remote_memory_string_put_ops(srcc, 1, 5);
	if (ret) {
		rdma_error("Failed to finish remote memory string put ops, ret = %d \n", ret);
		return ret;
	}

	char* destt = new char[5];
	ret = client_remote_memory_string_get_ops(destt, 1, 5);
	if (ret) {
		rdma_error("Failed to finish remote memory string get ops, ret = %d \n", ret);
		return ret;
	}

	if (check_src_dst()) {
		rdma_error("src and dst buffers do not match \n");
	} else {
		printf("...\nSUCCESS, source and destination buffers match \n");
	}
	ret = client_disconnect_and_clean();
	if (ret) {
		rdma_error("Failed to cleanly disconnect and clean up resources \n");
	}
	return ret;
}

*/

#endif
