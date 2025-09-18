/*
 * scheduler.c
 *
 *  Created on: Feb 1, 2024
 *      Author: ajgho
 */
///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "scheduler.h"
///////////////////////////////////////////////////////////////////////////////
// Global symbols
///////////////////////////////////////////////////////////////////////////////
// These are just two variables used for setting PSP
// This is because __set_psp is not linking for arm compiler reasons so for now
// I'm using a jank method to grab and set the appropriate PSP when starting
// The very first task
static unsigned int PSP;
static unsigned int* PSP_PTR = &PSP;

// This is our main scheduler defined in memory

// This is our global pointer pointing to the next task.
// I think we could just do some memory math into scheduler but save it for
// if we need to optimize
void *next_stack;

// This points to the stack_high of the previous running task when yielding
// We need this to be able to save and load our current context properly
void *top_of_stack;

// This will be a global containing the time from osSleep set in SVC 6
int sleep_time;

// These are globals set by SysTick
pq_elem* expired = 0;
pq_elem* done_sleep = 0;
pq_elem* done_periodic = 0;

// This will be a global containing the target TID and deadline for setDeadling (svc 8)
unsigned int target_TID = 0;
unsigned int target_deadline = 0;
///////////////////////////////////////////////////////////////////////////////
// Function Definitions
///////////////////////////////////////////////////////////////////////////////

__attribute__((naked)) void run_scheduler(scheduler* s, scheduler_op op){
	pq_elem* elem;

	// We reset any expired tasks and re-insert them into the ready queue
	__asm volatile("push {r7, lr}\n");

	while(expired && expired->data){
		elem = expired->next;
		s->bb[expired->data].state = TASK_READY;
		s->bb[expired->data].deadline = s->bb[expired->data].original_deadline;
		s->bb[expired->data].timeslice_length = s->bb[expired->data].original_deadline;
		insert_in_pq(&(s->scheduling_q), expired);
		expired = elem;
	}

	// We simply re-insert woken up tasks into the ready queue
	// Nvm reset the deadline too? Could just use 1 queue for pyield and sleep?
	while(done_sleep && done_sleep->data){
		elem = done_sleep->next;
		s->bb[done_sleep->data].state = TASK_READY;
		s->bb[done_sleep->data].deadline = s->bb[done_sleep->data].original_deadline;
		s->bb[done_sleep->data].timeslice_length = s->bb[done_sleep->data].original_deadline;
		insert_in_pq(&(s->scheduling_q), done_sleep);
		done_sleep = elem;
	}

	// We reset any expired tasks and re-insert them into the ready queue
	while(done_periodic && done_periodic->data){
		elem = done_periodic->next;
		s->bb[done_periodic->data].state = TASK_READY;
		s->bb[done_periodic->data].deadline = s->bb[done_periodic->data].original_deadline;
		s->bb[done_periodic->data].timeslice_length = s->bb[done_periodic->data].original_deadline;
		insert_in_pq(&(s->scheduling_q), done_periodic);
		done_periodic = elem;
	}
	__asm volatile("pop {r7, lr}\n");


	// Queue should now be sorted and first is earliest deadline
	switch(op){
	case SCHEDULER_START:
		// A fair assumption to make is that by the time this is called,
		// the first entry in bb should be set. (A task has been created
		// before calling osStart)
		s->running = s->scheduling_q.head->data;
		s->bb[s->running].state = TASK_RUNNING;
		s->task = s->bb[s->running].ptask;
		next_stack = s->bb[s->running].stack_high;

		// Set PSP for popping registers
		// And pop r4-r11 and start thread
		PSP = s->bb[s->running].stack_high;
		__asm volatile(
			"LDR r1, =PSP_PTR\n"
			"LDR r1, [r1]\n"
			"LDR r1, [r1]\n"
			"LDMIA r1!, {r4-r11}\n"
			"MSR PSP, r1\n"
		);
		__asm volatile(
			"MOV lr, #0xFFFFFFFD\n"
		);
		break;

	case SCHEDULER_YIELD:
		if(s->running){
			__asm volatile("push {r7, lr}\n");
			elem = remove_from_pq(&(s->scheduling_q), s->running);
			s->bb[s->running].deadline = s->bb[s->running].original_deadline;
			s->bb[s->running].timeslice_length = s->bb[s->running].original_deadline;
			insert_in_pq(&(s->scheduling_q), elem);
			__asm volatile("pop {r7, lr}\n");
		}

		if(s->running != s->scheduling_q.head->data){
			// Unset running status
			s->bb[s->running].state = TASK_READY;
			// Get top of stack to update in pendsv
			top_of_stack = &s->bb[s->running].stack_high;

			// Set new next task
			s->running = s->scheduling_q.head->data;
			s->bb[s->running].state = TASK_RUNNING;
			s->task = s->bb[s->running].ptask;
			next_stack = s->bb[s->running].stack_high;

			// Trigger pendsv
			SCB->ICSR |= 1<<28;
			__asm volatile("isb\n");
		}
		break;
	case SCHEDULER_KILL:
		// "Kill" Task
		s->bb[s->running].state = TASK_KILLED;
		// Get top of stack to update in pendsv
		top_of_stack = &s->bb[s->running].stack_high;

		__asm volatile("push {r7, lr}\n");
		remove_from_pq(&(s->scheduling_q), s->running);

		SET_FLG(F_CALLED_FROM_KERNEL);
		if(RTX_ERR == k_mem_dealloc(s->bb[s->running].stack_low - s->bb[s->running].stack_size)){
			// Force a crash for debugging
			*((int*)0) += 1;
		}
		USET_FLG(F_CALLED_FROM_KERNEL);

		__asm volatile("pop {r7, lr}\n");

		// This assumes the null function will always
		// a) Be in our queue and
		// b) Never be dequeued
		s->running = s->scheduling_q.head->data;
		s->bb[s->running].state = TASK_RUNNING;
		s->task = s->bb[s->running].ptask;
		next_stack = s->bb[s->running].stack_high;

		// Trigger pendsv
		SCB->ICSR |= 1<<28;
		__asm volatile("isb\n");
		__asm volatile("MOV lr, #0xFFFFFFFD\n");
		// Do we need some sort of clean up here?
		break;

	case SCHEDULER_PREEMPT:
		// This could also run itself...
		// Difference from Yield is that Yield resets the current caller
		if(s->running != s->scheduling_q.head->data){
			// Unset running status
			s->bb[s->running].state = TASK_READY;
			// Get top of stack to update in pendsv
			top_of_stack = &s->bb[s->running].stack_high;

			// Set new next task
			s->running = s->scheduling_q.head->data;
			s->bb[s->running].state = TASK_RUNNING;
			s->task = s->bb[s->running].ptask;
			next_stack = s->bb[s->running].stack_high;

			// Trigger pendsv
			SCB->ICSR |= 1<<28;
			__asm volatile("isb\n");
		}
		break;

	case SCHEDULER_SLEEP:
		// Just Preempt but reinsert in different q, too lazy to code smart
		// also doesn't reset timeslice time
		s->bb[s->running].state = TASK_SLEEPING;

		// Set the sleep time to the deadline
		// We do this to keep track of sleeping times in SysTick
		s->bb[s->running].deadline = sleep_time;
		top_of_stack = &s->bb[s->running].stack_high;

		__asm volatile("push {r7, lr}\n");
		elem = remove_from_pq(&(s->scheduling_q), s->running);

		// Set new next task
		s->running = s->scheduling_q.head->data;
		s->bb[s->running].state = TASK_RUNNING;
		s->task = s->bb[s->running].ptask;
		next_stack = s->bb[s->running].stack_high;

		// Insert elem back into queue
		insert_in_pq(&(s->sleeping_q), elem);
		__asm volatile("pop {r7, lr}\n");

		sleep_time = 0;
		SCB->ICSR |= 1<<28;
		__asm volatile("isb\n");
		break;

	case SCHEDULER_PERIODICYIELD:
		s->bb[s->running].state = TASK_SLEEPING;

		// Set the sleep time to the deadline
		// We do this to keep track of sleeping times in SysTick

		// This line below should be redundant, they should be the same value
		s->bb[s->running].deadline = s->bb[s->running].timeslice_length;
		top_of_stack = &s->bb[s->running].stack_high;

		__asm volatile("push {r7, lr}\n");
		elem = remove_from_pq(&(s->scheduling_q), s->running);

		// Set new next task
		s->running = s->scheduling_q.head->data;
		s->bb[s->running].state = TASK_RUNNING;
		s->task = s->bb[s->running].ptask;
		next_stack = s->bb[s->running].stack_high;

		// Insert elem back into queue
		insert_in_pq(&(s->periodic_q), elem);
		__asm volatile("pop {r7, lr}\n");

		SCB->ICSR |= 1<<28;
		__asm volatile("isb\n");
		break;

	case SCHEDULER_SETDEADLINE:
		globalVariables.svc_return[s->running] = RTX_ERR;
		if(target_deadline > 0){
			__asm volatile("push {r7, lr}\n");
			elem = remove_from_pq(&(s->scheduling_q), target_TID);
			if(!elem){
				elem = remove_from_pq(&(s->periodic_q), target_TID);
			}
			__asm volatile("pop {r7, lr}\n");
		}
		if(elem){
			*(elem->deadline) = target_deadline;
			s->bb[target_TID].original_deadline = target_deadline;
			s->bb[target_TID].deadline = target_deadline;
			s->bb[target_TID].timeslice_length = target_deadline;
			globalVariables.svc_return[s->running] = RTX_OK;
			__asm volatile("push {r7, lr}\n");
			insert_in_pq(&(s->scheduling_q), elem);
			__asm volatile("pop {r7, lr}\n");

			if(s->running != s->scheduling_q.head->data){
				// Unset running status
				s->bb[s->running].state = TASK_READY;
				// Get top of stack to update in pendsv
				top_of_stack = &s->bb[s->running].stack_high;

				// Set new next task
				s->running = s->scheduling_q.head->data;
				s->bb[s->running].state = TASK_RUNNING;
				s->task = s->bb[s->running].ptask;
				next_stack = s->bb[s->running].stack_high;

				// Trigger pendsv
				SCB->ICSR |= 1<<28;
				__asm volatile("isb\n");
			}
		}
		break;

	default:
		// NOT IMPLEMENTED
		// Do we have some sort of error system?
		break;
	}

	// Reset tick count
	globalVariables.curr_tick_count = 0;

	// Reset elem
	elem = 0;

	// Go back to svc func
	__asm volatile("BX lr\n");
}
///////////////////////////////////////////////////////////////////////////////
