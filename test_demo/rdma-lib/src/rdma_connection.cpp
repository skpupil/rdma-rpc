/*
 * An example RDMA client side code. 
 * Author: Animesh Trivedi 
 *         atrivedi@apache.org
 */

#include "rdma_common.h"
//#include "slab.h"
#include "rdma_connection.h"
//class client_connection {

//	/* These are basic RDMA resources */
//	/* These are RDMA connection related resources */
//	struct rdma_event_channel *cm_event_channel = NULL;
//	struct rdma_cm_id *cm_client_id = NULL;
//	struct ibv_pd *pd = NULL;
//	struct ibv_comp_channel *io_completion_channel = NULL;
//	struct ibv_cq *client_cq = NULL;
//	struct ibv_qp_init_attr qp_init_attr;
//	struct ibv_qp *client_qp;
//	/* These are memory buffers related resources */
//	struct ibv_mr *client_metadata_mr = NULL, 
//		      *client_src_mr = NULL, 
//		      *client_dst_mr = NULL, 
//		      *server_metadata_mr = NULL;
//	struct rdma_buffer_attr client_metadata_attr, server_metadata_attr;
//	struct ibv_send_wr client_send_wr, *bad_client_send_wr = NULL;
//	struct ibv_recv_wr server_recv_wr, *bad_server_recv_wr = NULL;
//	struct ibv_sge client_send_sge, server_recv_sge;
//	/* Source and Destination buffers, where RDMA operations source and sink */
//	char *src = NULL, *dst = NULL; 
//	int ret  = 0;

	client_connection::client_connection(){

		struct sockaddr_in server_sockaddr;
		bzero(&server_sockaddr, sizeof server_sockaddr);
		server_sockaddr.sin_family = AF_INET;
		server_sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		/* buffers are NULL */
		src = dst = NULL;

		//src = (char*)"abcduefghijk";
		char* optarg = (char*)"abcdefghijkiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
		src = (char*)calloc(strlen(optarg) , 1);
		if (!src) {
			rdma_error("Failed to allocate memory : -ENOMEM\n");
				//	return -ENOMEM;
		}
		/* Copy the passes arguments */
		strncpy(src, optarg, strlen(optarg));
		dst = (char*)calloc(strlen(src), 1);

		char* sserver_addr = (char*)"11.1.181.194";
		int ret = get_addr(sserver_addr, (struct sockaddr*) &server_sockaddr);
		if (ret) {
			rdma_error("Invalid IP \n");
		}


		if (!server_sockaddr.sin_port) {
			// no port provided, use the default port 
			server_sockaddr.sin_port = htons(DEFAULT_RDMA_PORT);
		}
		if (src == NULL) {
			printf("Please provide a string to copy \n");
			//usage();
		}
		ret = client_prepare_connection(&server_sockaddr);
		if (ret) {
			rdma_error("Failed to setup client connection , ret = %d \n", ret);
		}
		ret = client_pre_post_recv_buffer();
		if (ret) {
			rdma_error("Failed to setup client connection , ret = %d \n", ret);
		}
		ret = client_connect_to_server();
		if (ret) {
			rdma_error("Failed to setup client connection , ret = %d \n", ret);
		}
		ret = client_xchange_metadata_with_server();
		if (ret) {
			rdma_error("Failed to setup client connection , ret = %d \n", ret);
		}
	}



	/* This is our testing function */
	int client_connection::check_src_dst() 
	{
		return 0;
		//return memcmp((void*) src, (void*) dst, strlen(src));
	}

	/* This function prepares client side connection resources for an RDMA connection */
	int client_connection::client_prepare_connection(struct sockaddr_in *s_addr)
	{
		struct rdma_cm_event *cm_event = NULL;
		int ret = -1;
		/*  Open a channel used to report asynchronous communication event */
		cm_event_channel = rdma_create_event_channel();
		if (!cm_event_channel) {
			rdma_error("Creating cm event channel failed, errno: %d \n", -errno);
			return -errno;
		}
		debug("RDMA CM event channel is created at : %p \n", cm_event_channel);
		/* rdma_cm_id is the connection identifier (like socket) which is used 
		 * to define an RDMA connection. 
		 */
		ret = rdma_create_id(cm_event_channel, &cm_client_id, 
				NULL,
				RDMA_PS_TCP);
		if (ret) {
			rdma_error("Creating cm id failed with errno: %d \n", -errno); 
			return -errno;
		}
		/* Resolve destination and optional source addresses from IP addresses  to
		 * an RDMA address.  If successful, the specified rdma_cm_id will be bound
		 * to a local device. */
		ret = rdma_resolve_addr(cm_client_id, NULL, (struct sockaddr*) s_addr, 2000);
		if (ret) {
			rdma_error("Failed to resolve address, errno: %d \n", -errno);
			return -errno;
		}
		debug("waiting for cm event: RDMA_CM_EVENT_ADDR_RESOLVED\n");
		ret  = process_rdma_cm_event(cm_event_channel, 
				RDMA_CM_EVENT_ADDR_RESOLVED,
				&cm_event);
		if (ret) {
			rdma_error("Failed to receive a valid event, ret = %d \n", ret);
			return ret;
		}
		/* we ack the event */
		ret = rdma_ack_cm_event(cm_event);
		if (ret) {
			rdma_error("Failed to acknowledge the CM event, errno: %d\n", -errno);
			return -errno;
		}
		debug("RDMA address is resolved \n");

		/* Resolves an RDMA route to the destination address in order to 
		 * establish a connection */
		ret = rdma_resolve_route(cm_client_id, 2000);
		if (ret) {
			rdma_error("Failed to resolve route, erno: %d \n", -errno);
			return -errno;
		}
		debug("waiting for cm event: RDMA_CM_EVENT_ROUTE_RESOLVED\n");
		ret = process_rdma_cm_event(cm_event_channel, 
				RDMA_CM_EVENT_ROUTE_RESOLVED,
				&cm_event);
		if (ret) {
			rdma_error("Failed to receive a valid event, ret = %d \n", ret);
			return ret;
		}
		/* we ack the event */
		ret = rdma_ack_cm_event(cm_event);
		if (ret) {
			rdma_error("Failed to acknowledge the CM event, errno: %d \n", -errno);
			return -errno;
		}
		printf("Trying to connect to server at : %s port: %d \n", 
				inet_ntoa(s_addr->sin_addr),
				ntohs(s_addr->sin_port));
		/* Protection Domain (PD) is similar to a "process abstraction" 
		 * in the operating system. All resources are tied to a particular PD. 
		 * And accessing recourses across PD will result in a protection fault.
		 */
		pd = ibv_alloc_pd(cm_client_id->verbs);
		if (!pd) {
			rdma_error("Failed to alloc pd, errno: %d \n", -errno);
			return -errno;
		}
		debug("pd allocated at %p \n", pd);
		/* Now we need a completion channel, were the I/O completion 
		 * notifications are sent. Remember, this is different from connection 
		 * management (CM) event notifications. 
		 * A completion channel is also tied to an RDMA device, hence we will 
		 * use cm_client_id->verbs. 
		 */
		io_completion_channel = ibv_create_comp_channel(cm_client_id->verbs);
		if (!io_completion_channel) {
			rdma_error("Failed to create IO completion event channel, errno: %d\n",
					-errno);
			return -errno;
		}
		debug("completion event channel created at : %p \n", io_completion_channel);
		/* Now we create a completion queue (CQ) where actual I/O 
		 * completion metadata is placed. The metadata is packed into a structure 
		 * called struct ibv_wc (wc = work completion). ibv_wc has detailed 
		 * information about the work completion. An I/O request in RDMA world 
		 * is called "work" ;) 
		 */
		client_cq = ibv_create_cq(cm_client_id->verbs /* which device*/, 
				CQ_CAPACITY /* maximum capacity*/, 
				NULL /* user context, not used here */,
				io_completion_channel /* which IO completion channel */, 
				0 /* signaling vector, not used here*/);
		if (!client_cq) {
			rdma_error("Failed to create CQ, errno: %d \n", -errno);
			return -errno;
		}
		debug("CQ created at %p with %d elements \n", client_cq, client_cq->cqe);
		ret = ibv_req_notify_cq(client_cq, 0);
		if (ret) {
			rdma_error("Failed to request notifications, errno: %d\n", -errno);
			return -errno;
		}
		/* Now the last step, set up the queue pair (send, recv) queues and their capacity.
		 * The capacity here is define ally but this can be probed from the 
		 * device. We just use a small number as defined in rdma_common.h */
		bzero(&qp_init_attr, sizeof qp_init_attr);
		qp_init_attr.cap.max_recv_sge = MAX_SGE; /* Maximum SGE per receive posting */
		qp_init_attr.cap.max_recv_wr = MAX_WR; /* Maximum receive posting capacity */
		qp_init_attr.cap.max_send_sge = MAX_SGE; /* Maximum SGE per send posting */
		qp_init_attr.cap.max_send_wr = MAX_WR; /* Maximum send posting capacity */
		qp_init_attr.qp_type = IBV_QPT_RC; /* QP type, RC = Reliable connection */
		/* We use same completion queue, but one can use different queues */
		qp_init_attr.recv_cq = client_cq; /* Where should I notify for receive completion operations */
		qp_init_attr.send_cq = client_cq; /* Where should I notify for send completion operations */
		/*Lets create a QP */
		ret = rdma_create_qp(cm_client_id /* which connection id */,
				pd /* which protection domain*/,
				&qp_init_attr /* Initial attributes */);
		if (ret) {
			rdma_error("Failed to create QP, errno: %d \n", -errno);
			return -errno;
		}
		client_qp = cm_client_id->qp;
		debug("QP created at %p \n", client_qp);
		return 0;
	}

	/* Pre-posts a receive buffer before calling rdma_connect () */
	int client_connection::client_pre_post_recv_buffer()
	{
		int ret = -1;
		server_metadata_mr = rdma_buffer_register(pd,
				&server_metadata_attr,
				sizeof(server_metadata_attr),
				(IBV_ACCESS_LOCAL_WRITE));
		if(!server_metadata_mr){
			rdma_error("Failed to setup the server metadata mr , -ENOMEM\n");
			return -ENOMEM;
		}
		server_recv_sge.addr = (uint64_t) server_metadata_mr->addr;
		server_recv_sge.length = (uint32_t) server_metadata_mr->length;
		server_recv_sge.lkey = (uint32_t) server_metadata_mr->lkey;
		/* now we link it to the request */
		bzero(&server_recv_wr, sizeof(server_recv_wr));
		server_recv_wr.sg_list = &server_recv_sge;
		server_recv_wr.num_sge = 1;
		ret = ibv_post_recv(client_qp /* which QP */,
				&server_recv_wr /* receive work request*/,
				&bad_server_recv_wr /* error WRs */);
		if (ret) {
			rdma_error("Failed to pre-post the receive buffer, errno: %d \n", ret);
			return ret;
		}
		debug("Receive buffer pre-posting is successful \n");
		return 0;
	}

	/* Connects to the RDMA server */
	int client_connection::client_connect_to_server() 
	{
		struct rdma_conn_param conn_param;
		struct rdma_cm_event *cm_event = NULL;
		int ret = -1;
		bzero(&conn_param, sizeof(conn_param));
		conn_param.initiator_depth = 3;
		conn_param.responder_resources = 3;
		conn_param.retry_count = 3; // if fail, then how many times to retry
		ret = rdma_connect(cm_client_id, &conn_param);
		if (ret) {
			rdma_error("Failed to connect to remote host , errno: %d\n", -errno);
			return -errno;
		}
		debug("waiting for cm event: RDMA_CM_EVENT_ESTABLISHED\n");
		ret = process_rdma_cm_event(cm_event_channel, 
				RDMA_CM_EVENT_ESTABLISHED,
				&cm_event);
		if (ret) {
			rdma_error("Failed to get cm event, ret = %d \n", ret);
			return ret;
		}
		ret = rdma_ack_cm_event(cm_event);
		if (ret) {
			rdma_error("Failed to acknowledge cm event, errno: %d\n", 
					-errno);
			return -errno;
		}
		printf("The client is connected successfully \n");
		return 0;
	}

	/* Exchange buffer metadata with the server. The client sends its, and then receives
	 * from the server. The client-side metadata on the server is _not_ used because
	 * this program is client driven. But it shown here how to do it for the illustration
	 * purposes
	 */
	int client_connection::client_xchange_metadata_with_server()
	{
		struct ibv_wc wc[2];
		int ret = -1;
		debug("konglx: client_xchange_metadata_with_server src len: %ld %s\n", strlen(src), src);
		
		debug("pd  at %p \n", pd);
		
		client_src_mr = rdma_buffer_register(pd,
				src,
				strlen(src),
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE|
					IBV_ACCESS_REMOTE_READ|
					IBV_ACCESS_REMOTE_WRITE));
		if(!client_src_mr){
			rdma_error("Failed to register the first buffer, ret = %d \n", ret);
			return ret;
		}
		debug("konglx: src address is: %lld\n", (long long int)src);
		/* we prepare metadata for the first buffer */
		client_metadata_attr.address = (uint64_t) client_src_mr->addr; 
		//printf("konglingxin: client_metadata_attr.addr: %lld",(uint64_t)(client_metadata_attr.address));
		debug("konglingxin: client_metadata_attr.addr: %lld\n",(long long int)(client_metadata_attr.address));
		client_metadata_attr.length = 20 * 1024 * 1024;//client_src_mr->length; 
		debug("we tell server to alloc %d bytes", client_metadata_attr.length);	
		client_metadata_attr.stag.local_stag = client_src_mr->lkey;
		/* now we register the metadata memory */
		client_metadata_mr = rdma_buffer_register(pd,
				&client_metadata_attr,
				sizeof(client_metadata_attr),
				IBV_ACCESS_LOCAL_WRITE);
		if(!client_metadata_mr) {
			rdma_error("Failed to register the client metadata buffer, ret = %d \n", ret);
			return ret;
		}
		/* now we fill up SGE */
		client_send_sge.addr = (uint64_t) client_metadata_mr->addr;
		client_send_sge.length = (uint32_t) client_metadata_mr->length;
		client_send_sge.lkey = client_metadata_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_SEND;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to send client metadata, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 2 work completion. One for our 
		 * send and one for recv that we will get from the server for 
		 * its buffer information */
		ret = process_work_completion_events(io_completion_channel, 
				wc, 2);
		if(ret != 2) {
			rdma_error("We failed to get 2 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("slab begin to init: addr: %ld", server_metadata_attr.address);
		Slab& slab = Slab::get_instance();
		slab.init(server_metadata_attr.address, 100);
		debug("Server sent us its buffer location and credentials, showing \n");
		show_rdma_buffer_attr(&server_metadata_attr);
		return 0;
	}

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	int client_connection::client_remote_memory_ops() 
	{
		struct ibv_wc wc;
		int ret = -1;
		client_dst_mr = rdma_buffer_register(pd,
				dst,
				strlen(src),
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE | 
					IBV_ACCESS_REMOTE_WRITE | 
					IBV_ACCESS_REMOTE_READ));
		if (!client_dst_mr) {
			rdma_error("We failed to create the destination buffer, -ENOMEM\n");
			return -ENOMEM;
		}
		/* Step 1: is to copy the local buffer into the remote buffer. We will 
		 * reuse the previous variables. */
		/* now we fill up SGE */
		client_send_sge.addr = (uint64_t) client_src_mr->addr;
		client_send_sge.length = (uint32_t) client_src_mr->length;
		client_send_sge.lkey = client_src_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address;
		
		debug("\n\n\n client_remote_memory_ops(): addr %ld\n\n", server_metadata_attr.address);
		
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write client src buffer, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("Client side WRITE is complete \n");
		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) client_dst_mr->addr;
		client_send_sge.length = 1;;//(uint32_t) client_dst_mr->length - 6 ;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = client_dst_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  server_metadata_attr.address + 1;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: dst: %s\n",dst);
		debug("Client side READ is complete \n");

		debug("\n\nREAD roll 2; \n\n");

		/* Step 1: is to copy the local buffer into the remote buffer. We will 
		 * reuse the previous variables. */
		/* now we fill up SGE */
		src[1] = 'k';
		src[2] = 'l';
		client_send_sge.addr = (uint64_t) client_src_mr->addr;
		client_send_sge.length = (uint32_t) client_src_mr->length;
		client_send_sge.lkey = client_src_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address;
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write client src buffer, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("roll2 Client side WRITE is complete \n");


		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) client_dst_mr->addr;
		client_send_sge.length = 5;//(uint32_t) client_dst_mr->length - 6 ;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = client_dst_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		//char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  server_metadata_attr.address + 1;
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: dst: %s\n",dst);
		debug("Client side READ is complete \n");


		return 0;
	}

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	int client_connection::client_remote_memory_string_swap_out_ops() 
	{
		struct ibv_wc wc;
		int ret = -1;
		/* Step 1: is to copy the local buffer into the remote buffer. We will 
		 * reuse the previous variables. */
		/* now we fill up SGE */
		client_send_sge.addr = (uint64_t) client_src_mr->addr;
		client_send_sge.length = (uint32_t) client_src_mr->length;
		client_send_sge.lkey = client_src_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write client src buffer, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("Client side WRITE is complete \n");
		return 0;
	}	
	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 */ 
	int client_connection::client_remote_memory_string_put_ops(char* srcc, uint32_t start_index, size_t len) 
	{
		struct ibv_wc wc;
		int ret = -1;

		ibv_mr*	client_src_mr2 = rdma_buffer_register(pd,
				srcc,
				len,
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE|
					IBV_ACCESS_REMOTE_READ|
					IBV_ACCESS_REMOTE_WRITE));
		if(!client_src_mr2){
			rdma_error("Failed to register the first buffer, ret = %d \n", ret);
			return ret;
		}

		/* Step 1: is to copy the local buffer into the remote buffer. We will 
		 * reuse the previous variables. */
		/* now we fill up SGE */
		client_send_sge.addr = (uint64_t) client_src_mr2->addr;
		client_send_sge.length = (uint32_t) client_src_mr2->length;
		client_send_sge.lkey = client_src_mr2->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address + start_index;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write client src buffer, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("Client side WRITE is complete put ops\n");
		rdma_buffer_deregister(client_src_mr2);	
		return 0;
	}	

	int client_connection::client_remote_memory_string_swap_in_ops() 
	{
		struct ibv_wc wc;
		int ret = -1;


		client_dst_mr = rdma_buffer_register(pd,
				dst,
				strlen(src),
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE | 
					IBV_ACCESS_REMOTE_WRITE | 
					IBV_ACCESS_REMOTE_READ));
		if (!client_dst_mr) {
			rdma_error("We failed to create the destination buffer, -ENOMEM\n");
			return -ENOMEM;
		}

		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) client_dst_mr->addr;
		client_send_sge.length = (uint32_t) client_dst_mr->length;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = client_dst_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  server_metadata_attr.address ;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: dst: %s\n",dst);
		debug("Client side READ is complete swap in \n");

		return 0;
	}

	int client_connection::client_remote_memory_string_get_ops(char* dst, uint32_t start_index, size_t len) 
	{
		struct ibv_wc wc;
		int ret = -1;


		ibv_mr* client_dst_mr2 = rdma_buffer_register(pd,
				dst,
				len,
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE | 
					IBV_ACCESS_REMOTE_WRITE | 
					IBV_ACCESS_REMOTE_READ));
		if (!client_dst_mr2) {
			rdma_error("We failed to create the destination buffer, -ENOMEM\n");
			return -ENOMEM;
		}

		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) client_dst_mr2->addr;
		client_send_sge.length = (uint32_t) client_dst_mr2->length;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = client_dst_mr2->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  server_metadata_attr.address + start_index ;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n", 
					-errno);
			return -errno;
		}
		//printf("konglx: dst: %s\n",dst);
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: dst: %s\n",dst);
		debug("Client side READ is complete get ops \n");
		rdma_buffer_deregister(client_dst_mr2);	

		return 0;
	}

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 1) RDMA write from src -> remote buffer 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	int client_connection::client_remote_memory_array_ops() 
	{
		struct ibv_wc wc;
		int ret = -1;
		client_dst_mr = rdma_buffer_register(pd,
				dst,
				strlen(src),
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE | 
					IBV_ACCESS_REMOTE_WRITE | 
					IBV_ACCESS_REMOTE_READ));
		if (!client_dst_mr) {
			rdma_error("We failed to create the destination buffer, -ENOMEM\n");
			return -ENOMEM;
		}
		/* Step 1: is to copy the local buffer into the remote buffer. We will 
		 * reuse the previous variables. */
		/* now we fill up SGE */
		client_send_sge.addr = (uint64_t) client_src_mr->addr;
		client_send_sge.length = (uint32_t) client_src_mr->length;
		client_send_sge.lkey = client_src_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address;
		debug("client_remote_memory_array_ops:  before wrote: write to addr: %ld \n\n", server_metadata_attr.address);
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write client src buffer, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("Client side WRITE is complete \n");
		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) client_dst_mr->addr;
		client_send_sge.length = 1;;//(uint32_t) client_dst_mr->length - 6 ;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = client_dst_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  server_metadata_attr.address + 1;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: dst: %s\n",dst);
		debug("Client side READ is complete \n");

		debug("\n\nREAD roll 2; \n\n");

		/* Step 1: is to copy the local buffer into the remote buffer. We will 
		 * reuse the previous variables. */
		/* now we fill up SGE */
		src[1] = 'k';
		src[2] = 'l';
		client_send_sge.addr = (uint64_t) client_src_mr->addr;
		client_send_sge.length = (uint32_t) client_src_mr->length;
		client_send_sge.lkey = client_src_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address;
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write client src buffer, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("roll2 Client side WRITE is complete \n");


		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) client_dst_mr->addr;
		client_send_sge.length = 5;//(uint32_t) client_dst_mr->length - 6 ;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = client_dst_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		//char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  server_metadata_attr.address + 1;
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: dst: %s\n",dst);
		debug("Client side READ is complete \n");


		return 0;
	}

	int client_connection::client_remote_memory_array_swap_out_ops(uint64_t* srcc, uint64_t r_addr, uint64_t length)
	{
		struct ibv_wc wc;
		int ret = -1;

		ibv_mr* local_array_src_mr = rdma_buffer_register(pd,
				srcc,
				length,
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE|
					IBV_ACCESS_REMOTE_READ|
					IBV_ACCESS_REMOTE_WRITE));
		if(!local_array_src_mr){
			rdma_error("Failed to register the local array src mr buffer, ret = %d \n", ret);
			return ret;
		}

		/* Step 1: is to copy the local buffer into the remote buffer. We will
		 * reuse the previous variables. */
		/* now we fill up SGE */
		client_send_sge.addr = (uint64_t)local_array_src_mr->addr;//(uint64_t) client_src_mr->addr;
		client_send_sge.length = (uint32_t)local_array_src_mr->length;//(uint32_t) client_src_mr->length;
		client_send_sge.lkey = local_array_src_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = r_addr;//server_metadata_attr.address;
		debug("before wrote: write to addr: %ld ", r_addr);
		
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write local array src buffer, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		debug("Client side WRITE array is complete \n");
		return 0;
	}
	int client_connection::rdma_write(const void* srcc, uint64_t r_addr, uint64_t length)
	{
		struct ibv_wc wc;
		int ret = -1;

		ibv_mr* local_array_src_mr = rdma_buffer_register(pd,
				const_cast<void*>(srcc),
				length,
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE|
					IBV_ACCESS_REMOTE_READ|
					IBV_ACCESS_REMOTE_WRITE));
		if(!local_array_src_mr){
			rdma_error("Failed to register the local array src mr buffer in rdma_write, ret = %d \n", ret);
			return ret;
		}

		/* Step 1: is to copy the local buffer into the remote buffer. We will
		 * reuse the previous variables. */
		/* now we fill up SGE */
		client_send_sge.addr = (uint64_t)local_array_src_mr->addr;//(uint64_t) client_src_mr->addr;
		client_send_sge.length = (uint32_t)local_array_src_mr->length;//(uint32_t) client_src_mr->length;
		client_send_sge.lkey = local_array_src_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_WRITE;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		client_send_wr.wr.rdma.remote_addr = r_addr;//server_metadata_attr.address;
		debug("before wrote: write to addr: %ld ", r_addr);
		
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to write local array src buffer, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		rdma_buffer_deregister(local_array_src_mr);
		debug("Client side WRITE rdma_write is complete \n");
		return 0;
	}


	int client_connection::client_remote_memory_array_swap_in_ops(uint64_t* dstt, uint64_t r_addr, uint64_t length )
	{
		struct ibv_wc wc;
		int ret = -1;


		ibv_mr* local_array_dst_mr = rdma_buffer_register(pd,
				dstt,
				length,//strlen(src),
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE |
					IBV_ACCESS_REMOTE_WRITE |
					IBV_ACCESS_REMOTE_READ));
		if (!local_array_dst_mr) {
			rdma_error("We failed to create the local array destination buffer, -ENOMEM\n");
			return -ENOMEM;
		}

		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) local_array_dst_mr->addr;
		client_send_sge.length = (uint32_t) local_array_dst_mr->length;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = local_array_dst_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  r_addr;//server_metadata_attr.address ;
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		//printf("konglx: dst: %s\n",dst);
		debug("Client side array READ is complete swap in \n");

		return 0;
	}
	int client_connection::rdma_read(const void* dstt, uint64_t r_addr, uint64_t length )
	{
		struct ibv_wc wc;
		int ret = -1;

		//void* dst = const_cast<void*>(dstt);

		ibv_mr* local_array_dst_mr = rdma_buffer_register(pd,
				const_cast<void*>(dstt),
				length,//strlen(src),
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE |
					IBV_ACCESS_REMOTE_WRITE |
					IBV_ACCESS_REMOTE_READ));
		if (!local_array_dst_mr) {
			rdma_error("We failed to create the local array destination buffer in rdma_read, -ENOMEM\n");
			return -ENOMEM;
		}

		/* Now we prepare a READ using same variables but for destination */
		client_send_sge.addr = (uint64_t) local_array_dst_mr->addr;
		client_send_sge.length = (uint32_t) local_array_dst_mr->length;
		//printf("client_send_age.length: %d\n", (uint32_t)client_send_sge.length);
		debug("konglx: client_send_sge.length: %ld\n", (long int)client_send_sge.length);
		client_send_sge.lkey = local_array_dst_mr->lkey;
		/* now we link to the send work request */
		bzero(&client_send_wr, sizeof(client_send_wr));
		client_send_wr.sg_list = &client_send_sge;
		client_send_wr.num_sge = 1;
		client_send_wr.opcode = IBV_WR_RDMA_READ;
		client_send_wr.send_flags = IBV_SEND_SIGNALED;
		/* we have to tell server side info for RDMA */
		client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
		char* addr = (char*)server_metadata_attr.address;
		client_send_wr.wr.rdma.remote_addr =  r_addr;//server_metadata_attr.address ;
		/* Now we post it */
		ret = ibv_post_send(client_qp,
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n",
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel,
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		//printf("konglx: dst: %s\n",dst);
		debug("Client side rdma_read is complete \n");
		rdma_buffer_deregister(local_array_dst_mr);
		return 0;
	}

	struct ibv_mr* client_connection::prepare_mr(struct ibv_pd* pd, char* buf, uint32_t len) {
		auto client_mr = rdma_buffer_register(pd,
				buf,
				len,
				(enum ibv_access_flags)(IBV_ACCESS_LOCAL_WRITE | 
					IBV_ACCESS_REMOTE_WRITE | 
					IBV_ACCESS_REMOTE_READ));
		if (!client_mr) {
			rdma_error("We failed to create the destination buffer, -ENOMEM\n");
			//return -ENOMEM;
			return nullptr;
		}
		return client_mr;
	}

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 2) RDMA read from remote bufer -> dst
	 */ 
	int client_connection::client_remote_memory_read(void* ctx,  char* dst, uint32_t len, 
			uint32_t lkey, char* remote_addr, uint32_t rkey) 
	{
		struct ibv_wc wc;
		int ret = -1;
		/*
		   client_dst_mr = rdma_buffer_register(pd,
		   dst,
		   strlen(src),
		   (IBV_ACCESS_LOCAL_WRITE | 
		   IBV_ACCESS_REMOTE_WRITE | 
		   IBV_ACCESS_REMOTE_READ));
		   if (!client_dst_mr) {
		   rdma_error("We failed to create the destination buffer, -ENOMEM\n");
		   return -ENOMEM;
		   }
		   */
		/* Now we prepare a READ using same variables but for destination */
		//client_send_sge.addr = (uint64_t) client_dst_mr->addr;
		//client_send_sge.length = len//1;;//(uint32_t) client_dst_mr->length - 6 ;
		ibv_sge client_send_sge {
			(uint64_t)dst,
				len,
				lkey,
		};
		debug("client_send_age.length: %d\n", len);//(uint32_t)client_send_sge.length);

		//client_send_sge.lkey = client_dst_mr->lkey;
		ibv_send_wr client_send_wr
		{
			(uintptr_t)ctx,
				nullptr,
				&client_send_sge,
				1,
				IBV_WR_RDMA_READ,
				IBV_SEND_SIGNALED,
				{},
				{
					{
						(uint64_t)remote_addr,
						rkey,
					},
				},
				{},
				{},
		};
		ibv_send_wr* bad_client_send_wr = nullptr;

		/* now we link to the send work request */
		//bzero(&client_send_wr, sizeof(client_send_wr));
		/*
		   client_send_wr.wr_id = (uintptr_t)ctx;
		   client_send_wr.sg_list = &client_send_sge;
		   client_send_wr.num_sge = 1;
		   client_send_wr.opcode = IBV_WR_RDMA_READ;
		   client_send_wr.send_flags = IBV_SEND_SIGNALED;
		   */
		/* we have to tell server side info for RDMA */
		//client_send_wr.wr.rdma.rkey = rkey;//server_metadata_attr.stag.remote_stag;
		//char* addr = (char*)server_metadata_attr.address;
		//client_send_wr.wr.rdma.remote_addr =  remote_addr//server_metadata_attr.address + 5;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to read client dst buffer from the master, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: dst: %s\n",dst);
		debug("Client side READ is complete \n");
		return 0;
	}

	/* This function does :
	 * 1) Prepare memory buffers for RDMA operations 
	 * 2) RDMA write from src -> remote buffer 
	 */ 
	int client_connection::client_remote_memory_write(void* ctx, char* src, uint32_t len, 
			uint32_t lkey, char* remote_addr, uint32_t rkey) 
	{
		struct ibv_wc wc;
		int ret = -1;
		/*
		   client_dst_mr = rdma_buffer_register(pd,
		   dst,
		   strlen(src),
		   (IBV_ACCESS_LOCAL_WRITE | 
		   IBV_ACCESS_REMOTE_WRITE | 
		   IBV_ACCESS_REMOTE_READ));
		   if (!client_dst_mr) {
		   rdma_error("We failed to create the destination buffer, -ENOMEM\n");
		   return -ENOMEM;
		   }
		   */
		/* Now we prepare a READ using same variables but for destination */
		//client_send_sge.addr = (uint64_t) client_dst_mr->addr;
		//client_send_sge.length = len//1;;//(uint32_t) client_dst_mr->length - 6 ;
		ibv_sge client_send_sge {
			(uint64_t)src,
				len,
				lkey,
		};
		debug("client_send_age.length: %d\n", len);//(uint32_t)client_send_sge.length);

		//client_send_sge.lkey = client_dst_mr->lkey;
		ibv_send_wr client_send_wr
		{
			(uintptr_t)ctx,
				nullptr,
				&client_send_sge,
				1,
				IBV_WR_RDMA_WRITE,
				IBV_SEND_SIGNALED,
				{},
				{
					{
						(uint64_t)remote_addr,
						rkey,
					},
				},
				{},
				{},
		}; 
		ibv_send_wr* bad_client_send_wr = nullptr;

		/* now we link to the send work request */
		//bzero(&client_send_wr, sizeof(client_send_wr));
		/*
		   client_send_wr.wr_id = (uintptr_t)ctx;
		   client_send_wr.sg_list = &client_send_sge;
		   client_send_wr.num_sge = 1;
		   client_send_wr.opcode = IBV_WR_RDMA_READ;
		   client_send_wr.send_flags = IBV_SEND_SIGNALED;
		   */
		/* we have to tell server side info for RDMA */
		//client_send_wr.wr.rdma.rkey = rkey;//server_metadata_attr.stag.remote_stag;
		//char* addr = (char*)server_metadata_attr.address;
		//client_send_wr.wr.rdma.remote_addr =  remote_addr//server_metadata_attr.address + 5;
		/* Now we post it */
		ret = ibv_post_send(client_qp, 
				&client_send_wr,
				&bad_client_send_wr);
		if (ret) {
			rdma_error("Failed to WRITE client dst buffer from the master, errno: %d \n", 
					-errno);
			return -errno;
		}
		/* at this point we are expecting 1 work completion for the write */
		ret = process_work_completion_events(io_completion_channel, 
				&wc, 1);
		if(ret != 1) {
			rdma_error("We failed to get 1 work completions , ret = %d \n",
					ret);
			return ret;
		}
		printf("konglx: src: %s\n",src);
		debug("Client side WRITE is complete \n");
		return 0;
	}

	/* This function disconnects the RDMA connection from the server and cleans up 
	 * all the resources.
	 */
	int client_connection::client_disconnect_and_clean()
	{
		struct rdma_cm_event *cm_event = NULL;
		int ret = -1;
		/* active disconnect from the client side */
		ret = rdma_disconnect(cm_client_id);
		if (ret) {
			rdma_error("Failed to disconnect, errno: %d \n", -errno);
			//continuing anyways
		}
		ret = process_rdma_cm_event(cm_event_channel, 
				RDMA_CM_EVENT_DISCONNECTED,
				&cm_event);
		if (ret) {
			rdma_error("Failed to get RDMA_CM_EVENT_DISCONNECTED event, ret = %d\n",
					ret);
			//continuing anyways 
		}
		ret = rdma_ack_cm_event(cm_event);
		if (ret) {
			rdma_error("Failed to acknowledge cm event, errno: %d\n", 
					-errno);
			//continuing anyways
		}
		/* Destroy QP */
		rdma_destroy_qp(cm_client_id);
		/* Destroy client cm id */
		ret = rdma_destroy_id(cm_client_id);
		if (ret) {
			rdma_error("Failed to destroy client id cleanly, %d \n", -errno);
			// we continue anyways;
		}
		/* Destroy CQ */
		ret = ibv_destroy_cq(client_cq);
		if (ret) {
			rdma_error("Failed to destroy completion queue cleanly, %d \n", -errno);
			// we continue anyways;
		}
		/* Destroy completion channel */
		ret = ibv_destroy_comp_channel(io_completion_channel);
		if (ret) {
			rdma_error("Failed to destroy completion channel cleanly, %d \n", -errno);
			// we continue anyways;
		}
		/* Destroy memory buffers */
		rdma_buffer_deregister(server_metadata_mr);
		rdma_buffer_deregister(client_metadata_mr);	
		rdma_buffer_deregister(client_src_mr);	
		rdma_buffer_deregister(client_dst_mr);	
		/* We free the buffers */
		free(src);
		free(dst);
		/* Destroy protection domain */
		ret = ibv_dealloc_pd(pd);
		if (ret) {
			rdma_error("Failed to destroy client protection domain cleanly, %d \n", -errno);
			// we continue anyways;
		}
		rdma_destroy_event_channel(cm_event_channel);
		printf("Client resource clean up is complete \n");
		return 0;
	}





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
