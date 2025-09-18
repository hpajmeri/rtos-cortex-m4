

#ifndef INC_LINKED_LIST_H_
#define INC_LINKED_LIST_H_

#include "common.h"
#include <stddef.h> // For size_t...

typedef struct mem_metadata{
	unsigned int start; //starting address
	unsigned int end; //ending address
	size_t size; //size of block
	unsigned int tid;
	struct mem_metadata* next;
	struct mem_metadata* prev;
}mem_node;

typedef struct linked_list{
	mem_node* head;
	mem_node* tail;
}LL;

void in_order_insert(mem_node *node); //ONLY TO BE USED BY FREE LIST
void end_insert(mem_node *node); //ONLY TO BE USED BY ALLOCATED LIST
mem_node* remove_node(unsigned int starting_address, LL *list); //Both lists can use

#endif
