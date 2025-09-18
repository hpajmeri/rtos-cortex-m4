/*
 * queue.h
 *
 *  Created on: Jan 31, 2024
 *      Author: ajgho
 */

// NOTE:
// Expected usage is that circ_q_elems will be managed by application/OS
// I.e. we will create all possible elements for the queue (16) and
// handle what happens to the element when it is pushed/popped ourselves
// Assuming we will never need to statically create circ_q_elems, we can just
// Keep them in an array circ_q_elem elems[16];


#ifndef QUEUE_H
#define QUEUE_H
///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////
struct t_circ_q_elem;
typedef struct t_circ_q_elem circ_q_elem;

typedef struct t_circ_q_elem{
	circ_q_elem* prev;
	circ_q_elem* next;
	unsigned int data; // Could also be pointer to data
}circ_q_elem;

typedef struct t_circ_q{
	circ_q_elem* head;
	circ_q_elem* tail;
	unsigned int size;
}circ_q;

///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////
void enqueue(circ_q* q, circ_q_elem* elem);
unsigned int dequeue(circ_q* q);
void advance_queue(circ_q* q);
///////////////////////////////////////////////////////////////////////////////

#endif
