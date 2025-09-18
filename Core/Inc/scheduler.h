/*
 * scheduler.h
 *
 *  Created on: Feb 1, 2024
 *      Author: ajgho
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_
///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "queue.h"
#include "queue2.h"
#include "k_task.h"
#include "common.h"
#include "main.h"
///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////
typedef enum enum_scheduler_op{
	SCHEDULER_START = 0,
	SCHEDULER_YIELD = 1,
	SCHEDULER_KILL = 2,
	SCHEDULER_PREEMPT = 3,
	SCHEDULER_SLEEP = 4,
	SCHEDULER_PERIODICYIELD = 5,
	SCHEDULER_SETDEADLINE = 6
}scheduler_op;

typedef struct t_scheduler{
	// Not sure if task_stack needs to be of type:
	// void (*ptask)(void* args); to get the args

//	circ_q scheduling_q;	// pointer to internal queue
	pq scheduling_q;
	pq periodic_q;
	pq sleeping_q;

	TCB* bb;				// pointer to bb
	unsigned int running; 	// Used to index into bb
	void* task;				// Points to "current task context"
} scheduler;

extern struct globalVariableStruct_t;
extern struct globalVariableStruct_t globalVariables;
///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////
__attribute__((naked)) void run_scheduler(scheduler* s, scheduler_op op);
///////////////////////////////////////////////////////////////////////////////

#endif /* INC_SCHEDULER_H_ */
