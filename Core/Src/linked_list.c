

#include "linked_list.h"
#include "common.h"

void in_order_insert(mem_node *node){ //ONLY USED BY FREELIST
	mem_node* traverseNode = FREE_LIST.head;

	//Loop through free list until we find spot in address space that this node belongs to
	while(traverseNode){
		if(traverseNode->start >= node->end){
			break;
		}
		traverseNode = traverseNode->next;
	}

	if(!traverseNode){
		// No spot was found...
		// Insert at tail
		if(FREE_LIST.tail)
			FREE_LIST.tail->next = node;
		node->prev = FREE_LIST.tail;
		node->next = NULL;
		FREE_LIST.tail = node;

		// List was empty
		if(!FREE_LIST.head){
			FREE_LIST.head = node;
		}
	}
	else{
		node->next = traverseNode;
		if(traverseNode->prev){
			node->prev = traverseNode->prev;
			traverseNode->prev->next = node;
		}
		else{
			// At list head
			FREE_LIST.head = node;
			node->prev = 0;
		}

		traverseNode->prev = node;
	}

	// Coalesce favours nodes in freelist

	if(node->next && ((node->end + 1) == node->next->start)){ // Right coalesce
		// Grab temp pointer
		traverseNode = node->next;

		// Remove and coalesce
		node->next->start = node->start;
		node->next->size += node->size;

		node->next->prev = node->prev;
		if(node->prev){
			node->prev->next = node->next;
		}

		// Need to set this or else we lose node
		remove_node(node, &FREE_LIST);
		node = traverseNode;
	}
	if(node->prev && (node->start == (node->prev->end + 1))){
		// Grab temp pointer
		traverseNode = node->prev;

		// Coalesce and remove
		node->start = node->prev->start;
		node->size += node->prev->size;
		node->prev = node->prev->prev;
		if(node->prev){
			node->prev->next = node;
		}

		remove_node(traverseNode, &FREE_LIST);
	}
}

void end_insert(mem_node *node){ //ONLY USED BY ALLOCATED LIST
	if(ALLOCATED_LIST.tail)
		ALLOCATED_LIST.tail->next = node;

	node->prev = ALLOCATED_LIST.tail;
	node->next = NULL;
	ALLOCATED_LIST.tail = node;

	if(!ALLOCATED_LIST.head)
		ALLOCATED_LIST.head = node;
}

mem_node* remove_node(unsigned int starting_address, LL *list){
	mem_node* traverseNode = list->head;
	while (traverseNode){
		if (traverseNode == starting_address){

			if(traverseNode->prev){
				traverseNode->prev->next = traverseNode->next;

				if(traverseNode->next)
					traverseNode->next->prev = traverseNode->prev;
				else
					// We are at tail, update list tail
					list->tail = traverseNode->prev;
			}
			else{
				// We are at head, update list head
				list->head = traverseNode->next;
				if(list->head)
					list->head->prev = 0;
				else
					// No more elements left
					list->tail = 0;
			}

			// Fully DC traverseNode
			traverseNode->next = 0;
			traverseNode->prev = 0;
			return traverseNode;
		}
		traverseNode = traverseNode->next;
	}
	return NULL;
}
