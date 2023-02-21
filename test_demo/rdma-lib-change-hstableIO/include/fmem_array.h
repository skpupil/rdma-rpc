#include "slab.h"

class fmem_array: public Chunk {
	public:	
	client_connection* conn;
	uint64_t* local_addr;

	fmem_array(client_connection* connection, uint64_t siz);
	uint64_t at(uint64_t index);
	int modify(uint64_t index, uint64_t val);
	uint64_t* swapin();
	int swapout(uint64_t* arr);
	int length();
	int print_arr();
};
