#include "rdma_connection.h"
#include "fmem_array.h"

int main(){
	client_connection* conn = new client_connection();
	
	int ret = conn->client_remote_memory_ops();
        if (ret) {
              rdma_error("Failed to finish remote memory ops, ret = %d \n", ret);
        }

	fmem_array farr = fmem_array(conn, 10);

uint64_t* arr = (uint64_t*)malloc(10 * sizeof(uint64_t));
        arr[0] = 255;
        arr[1] = 512;

	ret = farr.swapout(arr);//conn.client_remote_memory_ops();
        if (ret) {
              rdma_error("Failed to finish remote memory fmem array ops, ret = %d \n", ret);
        }

	debug("test_general at index 0 : %ld\n", farr.at(0));
	
	uint64_t changee = 9999;	
	farr.modify(0, changee);

	debug("test_general at index 0 : %ld\n", farr.at(0));

/*	
        uint64_t at(uint64_t index);
        int modify(uint64_t index, uint64_t val);
        uint64_t* swapin();
        int swapout(uint64_t* arr);
        int length();
        int print_arr();
*/


	conn->client_disconnect_and_clean();
	return 0;

}
