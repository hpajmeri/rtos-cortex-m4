/*
 * queue2.c
 *
 *  Created on: Mar 28, 2024
 *      Author: ajgho
 */


#include "queue2.h"


void init_pq(pq* q, pq_elem* null_task_elem){
	// Initializes null task sentinel node
	q->tail = null_task_elem;
	q->head = null_task_elem;

	q->tail->data = 0;
	q->tail->prev = 0;
	q->tail->next = 0;
}

void insert_in_pq(pq* q, pq_elem* elem){
	pq_elem* tmp;
	pq_elem* qptr = q->head;

	// Shouldn't need to null check qptr here
	while(qptr->data && (*(qptr->deadline) <= *(elem->deadline))){
		qptr = qptr->next;
	}
	tmp = qptr->prev;

	// Want to insert between tmp and qptr
	elem->next = qptr;
	qptr->prev = elem;

	elem->prev = tmp;
	if(tmp){
		tmp->next = elem;
	}
	else {
		q->head = elem;
	}
}

pq_elem* dec_pq_deadlines(pq* q, unsigned int dec){
	pq_elem* ret = 0;
	pq_elem* ret_head = 0;
	pq_elem* ptr = q->tail;
	pq_elem* prev = 0;
	pq_elem* ptr_ahead, *ptr_behind;


	// Go tail to head instead of head to tail
	// This is to help with starving kinda
	while(ptr){
		prev = ptr->prev;
		if(ptr->data && (*(ptr->deadline) <= dec)){
			// Remove pointer
			ptr_ahead = ptr->next;
			ptr_behind = ptr->prev;

			ptr_ahead->prev = ptr_behind;
			if(ptr_behind){
				ptr_behind->next = ptr_ahead;
			}
			else{
				q->head = ptr_ahead;
			}

			// Insert into return list
			if(!ret){
				ret_head = ptr;
			}
			ptr->prev = ret;
			ptr->next = 0;

			if(ret)
				ret->next = ptr;
			ret = ptr;
		}
		else{
			*(ptr->deadline) -= dec;
		}
		ptr = prev;
	}

	return ret_head;
}

pq_elem* remove_from_pq(pq* q, unsigned int tid){
	pq_elem* ptr = q->head;
	pq_elem* ret = 0;
	pq_elem* ptr_ahead, *ptr_behind;
	while(tid && ptr){
		if(ptr->data == tid){
			ptr_ahead = ptr->next;
			ptr_behind = ptr->prev;

			ptr_ahead->prev = ptr_behind;
			if(ptr_behind){
				ptr_behind->next = ptr_ahead;
			}
			else{
				q->head = ptr_ahead;
			}

			ret = ptr;
			break;
		}
		ptr = ptr->next;
	}

	return ret;
}

pq_elem* pq_dequeue(pq* q){
	pq_elem* ptr = q->head;
	if(ptr->data){
		// Remove if not null node
		ptr->next->prev = 0;
		q->head = ptr->next;
	}
	return ptr;
}
