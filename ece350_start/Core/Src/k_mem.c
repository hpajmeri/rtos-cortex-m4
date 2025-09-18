
#include "common.h"
#include "linked_list.h"
#include "k_mem.h"

static mem_node FREE_LIST_BASE;

int k_mem_init(){

	if(CHK_FLG_SET(F_KERNEL_INIT_CALLED) && CHK_FLG_NSET(F_MEMORY_INIT_CALLED)){
		FREE_LIST_BASE.next = 0;
		FREE_LIST_BASE.prev = 0;
		FREE_LIST_BASE.start = &_img_end;
		FREE_LIST_BASE.end = ((void*)&_estack - (void*)&_Min_Stack_Size);
		FREE_LIST_BASE.size = FREE_LIST_BASE.end - FREE_LIST_BASE.start;

		FREE_LIST.head = &FREE_LIST_BASE;
		FREE_LIST.tail = &FREE_LIST_BASE;

		ALLOCATED_LIST.head = 0;
		ALLOCATED_LIST.tail = 0;

		SET_FLG(F_MEMORY_INIT_CALLED);

		// Need to also set up FREE_LIST
		// THIS WILL BE DONE WITH THE LINKER SYMBOL
		return RTX_OK;
	}
	return RTX_ERR;
}

void* k_mem_alloc(size_t size){
	//Word Align everything

	// For now, I am assuming that mem_node will always be word_aligned (which so far it is)
	// We will only be unaligned when creating a new block since if every other block is aligned
	// When coalescing, we will be in multiples of 4

	if(!size){
		return 0;
	}

	size_t total_size = size + sizeof(mem_node);

	// We need to be able to fit both mem_node AND the requested size
	// This is also assuming mem_node is word-aligned which it is right now
	if(total_size % WORD_ALLIGNMENT){
		size += (4 - (total_size % WORD_ALLIGNMENT));
		total_size += (4 - (total_size % WORD_ALLIGNMENT));
	}

	// check if we have space in the free list
	mem_node *traverse_node = FREE_LIST.head;
	int perfect_fit = 0;

	while (traverse_node != 0){
		if (traverse_node->size >= total_size){
			perfect_fit = (traverse_node->size == total_size);
			break;
		}
		traverse_node = traverse_node->next;
	}

	if(!traverse_node){
		return 0; // Return null
	}


	// We take from the front of the node (first fit?)
	mem_node *allocated_node = traverse_node->start; // We fit the node data at the start of found space

	// Start of user data
	allocated_node->start = traverse_node->start + sizeof(mem_node);

	// Don't want to update BASE start to be some weird value in heap, treat it separately
	if(traverse_node == &FREE_LIST_BASE){
		traverse_node->size -= total_size;
		traverse_node->start += total_size;
	}
	else{
		// Remove the node, not necessarily needed but helps in doing less pointer math for non-absorb case
		remove_node(traverse_node, &FREE_LIST);

		// If a perfect fit, no need to do anything, just reuse node
		if(!perfect_fit){
			// Not a perfect fit, need to either split or absorb remainder
			size_t size_diff = traverse_node->size - total_size;
			if(size_diff > (sizeof(mem_node) + 4)){
				// Create new node and insert remainder into free list if not BASE
				traverse_node = (mem_node*)((uint32_t)allocated_node + (uint32_t)total_size);
				traverse_node->start = traverse_node;
				traverse_node->end = traverse_node->start + size_diff;
				traverse_node->size = size_diff;
				traverse_node->next = 0;
				traverse_node->prev = 0;
				in_order_insert(traverse_node);
			}
			else{
				// Not enough space to make a new node, absorb remainder
				total_size = traverse_node->size;
				size += size_diff;
			}
		}
	}

	// Set the end, size, tid of the allocated node
	allocated_node->end = allocated_node->start + size - 1;
	allocated_node->size = size;
	allocated_node->tid = MAIN_SCHEDULER.running;

	end_insert(allocated_node);

	// Return start of alloc_node
	return allocated_node->start;
}


int k_mem_dealloc(void* ptr){
	if(!ptr){
		// They want us to return ok on a null for some reason?
		// https://piazza.com/class/lqo42wf3gam164/post/229
		return RTX_OK;
	}
	// Can only dealloc if the process that made the mem is the one requesting
	// To free
	mem_node* node = ptr - sizeof(mem_node);

	// We don't care which TID called dealloc as long as kernel called it
	if((CHK_FLG_NSET(F_CALLED_FROM_KERNEL)) && (MAIN_SCHEDULER.running != node->tid)){
		return RTX_ERR;
	}
	// They might be able to free a random memory location
	// Look through the allocated list too...

	//ADDED ON MARCH 6
	// Reclaim back mem_node memory
	node->start -= sizeof(mem_node);
	node->size += sizeof(mem_node);
	mem_node* traverse_node = remove_node(node, &ALLOCATED_LIST);
	if(!traverse_node){
		// They tried to free an invalid memory location
		// matched in the earlier check
		return RTX_ERR;
	}

	// Only total_size in alloc needs to be strictly alligned, the mem_node size does not need to be... ( ITHINK)
	in_order_insert(traverse_node);
	return RTX_OK;
}


int k_mem_count_extfrag(size_t size){
	int count = 0;

	mem_node* traverse_node = FREE_LIST.head;
	while(traverse_node){
		if(traverse_node->size < size){
			count++;
		}
		traverse_node = traverse_node->next;
	}

	return count;
}
