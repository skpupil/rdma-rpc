#include "iostream"
#include "rdma_common.h"
#include "slab.h"
#include <cassert>


Slab::Slab(){}

int Slab::init(uint64_t r_addr, uint64_t tot){
	
	debug("Slab is init with remote addr is %ld, total %ld bytes \n", r_addr, tot);
	remote_addr = r_addr;
       	total = tot;
	head = (struct node*)malloc(sizeof(struct node));
	tail = (struct node*)malloc(sizeof(struct node));
	node* achunk = (struct node*)malloc(sizeof(struct node));

	head->pre = nullptr;
	head->next = tail;

	tail->pre = head;
	tail->next = nullptr;

	achunk->start = r_addr;
	achunk->end = r_addr + tot;
	add_node(head, achunk);
	return 0;
}

int Slab::add_node(node* former, node* latter){

	latter->pre = former;
	latter->next = former->next;

	former->next->pre = latter;
	former->next = latter;
	return 0;
}
int Slab::remove_node(node* cur){
	cur->pre->next = cur->next;
	cur->next->pre = cur->pre;

	free(cur);
	return 0;
}
uint64_t Slab::get_mem(uint64_t siz){
	//return remote_addr;
	
	
	node* p = head->next;
	uint64_t ret;
	while(p->next != nullptr){
		if(p->end - p->start == siz){
			ret = p->start;
			remove_node(p);
			return ret;
		}else if(p->end - p->start > siz){
			ret = p->start;
			p->start += siz;
			return ret;
		}else if(p->end - p->start < siz){
			p = p->next;
		}
	}
	assert(1 == 0);
	return -1;
	
}
int Slab::free_mem(Chunk ch){
	node* p = head->next;
	uint64_t ret;
	while(p->next != nullptr){
		if(p->start > ch.st_addr + ch.size){

			node* cur_chunk = (struct node*)malloc(sizeof(struct node));
			cur_chunk->start = ch.st_addr;
			cur_chunk->end = ch.st_addr + ch.size;
			add_node(p->pre, cur_chunk);
			return 0;
		}else if(p->start == ch.st_addr + ch.size){
			p->start = ch.st_addr;
			return 0;
		}else if(p->end == ch.st_addr){
			p->end = ch.st_addr + ch.size;
			return 0;
		}else if(p->end < ch.st_addr){
			p = p->next;
		}else{
			assert(1 == 0);
		}
	}
	node* cur_chunk = (struct node*)malloc(sizeof(struct node));
	cur_chunk->start = ch.st_addr;
	cur_chunk->end = ch.st_addr + ch.size;
	add_node(tail->pre, cur_chunk);
	return 0;	

}
Slab& Slab::get_instance(){
	static Slab instance;
	return instance;

}
