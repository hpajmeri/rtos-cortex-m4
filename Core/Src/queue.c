/*
 * queue.h
 *
 *  Created on: Feb 1, 2024
 *      Author: ajgho
 */
///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "queue.h"
///////////////////////////////////////////////////////////////////////////////
// Function Definitions
///////////////////////////////////////////////////////////////////////////////
void enqueue(circ_q* q, circ_q_elem* elem){
	if(!(q->head && q->tail)){
		elem->next = 0;
		elem->prev = 0;
		q->head = elem;
		q->tail = elem;
	}
	else{
		q->tail->next = elem;
		elem->prev = q->tail;
		elem->next = q->head;
		q->tail = elem;
	}
	q->size++;
	return;
}

unsigned int dequeue(circ_q* q){
//	unsigned int ret_val = 0;
	if(!q->head){
		// ERROR?
		//ret_val = 0xFFFFFFFF;
		return 0xFFFFFFFF;
	}
	if(q->size == 1){
		// Everything will be gone
//		ret_val = q->head->data;
		q->head = 0;
		q->tail = 0;
		q->size--;
		return q->head->data;
	}
	else{
//		ret_val = q->head->data;
		q->tail->next = q->head->next;
		q->head->next->prev = q->tail;
		q->head = q->head->next;
		q->size--;
		return q->head->data;
	}
//	q->size--;

//	return ret_val;
}

void advance_queue(circ_q* q){
	if((q->head && q->tail) && (q->head != q->tail)){
		q->head = q->head->next;
		q->tail = q->tail->next;
	}
	return;
}
///////////////////////////////////////////////////////////////////////////////
