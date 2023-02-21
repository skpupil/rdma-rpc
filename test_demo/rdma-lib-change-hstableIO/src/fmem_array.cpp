//#include "rdma_common.h"
#include "rdma_connection.h"
#include "fmem_array.h"
//#include "slab.h"

fmem_array::fmem_array(client_connection* connection, uint64_t siz){
	conn = connection;
	
	Slab& slab  = Slab::get_instance();
	uint64_t addr = slab.get_mem(siz * sizeof(uint64_t));
	st_addr = addr;
        size = siz * sizeof(uint64_t);	
	debug("konglx: fmem_array addr : %ld size: %ld\n", addr, size);

}
uint64_t fmem_array::at(uint64_t index){
	uint64_t* cur = (uint64_t*)malloc(sizeof(uint64_t));

	conn->client_remote_memory_array_swap_in_ops(cur,st_addr + index*(sizeof(uint64_t)), sizeof(uint64_t));

	return *cur;	
}
int fmem_array::modify(uint64_t index, uint64_t val){
	uint64_t* cur = &val;

	conn->client_remote_memory_array_swap_out_ops(cur,st_addr + index * sizeof(uint64_t), sizeof(uint64_t));
	
	return 0;
}
uint64_t* fmem_array::swapin(){
	return local_addr;
}
int fmem_array::swapout(uint64_t* arr){
	this->local_addr = arr;
	conn->client_remote_memory_array_swap_out_ops(arr,st_addr, size);
	
	uint64_t* dstt = (uint64_t*)calloc(size , sizeof(uint64_t));
	conn->client_remote_memory_array_swap_in_ops(dstt, st_addr, size );
	debug("konglx: fmem_array swap in dstt first element: %ld\n", dstt[0]);
	return 0;

}
int fmem_array::length(){
	return this->size/sizeof(uint64_t);
}
int fmem_array::print_arr(){
	for(int i = 0;i<this->length();i++){
		printf("%ld, ", *(local_addr + i * sizeof(uint64_t)));
	}
	printf("\n");
	return 0;
}
