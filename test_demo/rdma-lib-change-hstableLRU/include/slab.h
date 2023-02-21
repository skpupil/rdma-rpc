//#include "chunk.h"
#pragma once

struct node
{
        uint64_t start;
        uint64_t end;
        struct node *pre;
        struct node *next;
};

class Chunk {
public:
        uint64_t st_addr;
        uint64_t size;
        bool is_out = true;
};

class Slab {
        uint64_t remote_addr;
        uint64_t total;
        node  *head;
        node  *tail;
	
public:
	
	Slab();	
       	Slab(const Slab&)=delete;
 	Slab& operator=(const Slab&)=delete;
 	static Slab& get_instance();
	

        int init(uint64_t r_addr, uint64_t tot );
	int add_node(node* former, node* latter);
	int remove_node(node* cur);
	uint64_t get_mem(uint64_t siz);

	int free_mem(Chunk ch);

};
