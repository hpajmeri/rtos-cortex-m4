/*
 * queue2.h
 *
 *  Created on: Mar 28, 2024
 *      Author: ajgho
 */

#ifndef INC_QUEUE2_H_
#define INC_QUEUE2_H_

///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////
struct t_pq_elem;
typedef struct t_pq_elem pq_elem;

typedef struct t_pq_elem{
	pq_elem* prev;
	pq_elem* next;
	unsigned int data; // Could also be pointer to data
	unsigned int *deadline; // The priority
}pq_elem;

typedef struct t_pq{
	pq_elem* head;
	pq_elem* tail;	// Should always be the null task
	unsigned int size;
}pq;

///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////
void init_pq(pq* q, pq_elem* null_task_elem); 		// Inits tail sentinel (null task)
void insert_in_pq(pq* q, pq_elem* elem); 		// Sorted insertion
pq_elem* dec_pq_deadlines(pq* q, unsigned int dec);	// Decrements deadline of ALL pq elems, returns linked list of "expired" elements
pq_elem* remove_from_pq(pq* q, unsigned int tid);	// Removes specific element in list
pq_elem* pq_dequeue(pq* q);						// Removes first element from list

///////////////////////////////////////////////////////////////////////////////


#endif /* INC_QUEUE2_H_ */
