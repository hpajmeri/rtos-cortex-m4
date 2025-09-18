/*
 * k_task.c
 *
 *  Created on: Jan 24, 2024
 *      Author: ajgho
 */
///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "common.h"
#include "k_task.h"
#include "common.h"
#include "main.h"

pq_elem null_elem_sleep;
pq_elem null_elem_periodic;

///////////////////////////////////////////////////////////////////////////////
// Function Definitions
///////////////////////////////////////////////////////////////////////////////
void null_task(void){
	// DO NOT USE PRINTF IN NULL
	// THIS WILL MESS UP STACK OF FIRST TASK
	// UNLESS WE GIVE IT MORE SPACE
	while(1){
		osYield();
	}
}

volatile uint32_t* write_task_to_stack(volatile uint32_t* stack, task_stack_frame* sf){
	*(--stack) = sf->xPSR;
	*(--stack) = sf->PC;
	*(--stack) = sf->LR;
	*(--stack) = sf->R12;
	*(--stack) = sf->R3;
	*(--stack) = sf->R2;
	*(--stack) = sf->R1;
	*(--stack) = sf->R0;
	*(--stack) = sf->R11;
	*(--stack) = sf->R10;
	*(--stack) = sf->R9;
	*(--stack) = sf->R8;
	*(--stack) = sf->R7;
	*(--stack) = sf->R6;
	*(--stack) = sf->R5;
	*(--stack) = sf->R4;

	return stack;
}

int osCreateTask(TCB* task){
	return osCreateDeadlineTask(DEFAULT_DEADLINE, task->stack_size, task);
}

int osCreateDeadlineTask(int deadline, int s_size, TCB* task){
	/*
	 *  Create a new task -> instantiate a TCB and thread stack
		Assigns the TCB to the the first free TCB with state terminated
		Check TCB field for validity (maybe stuff just not 0?)
		Assigns task id to task
		Returns success or failure
		Puts tasks into ready state and pushes TID to scheduling queue
	 *
	 */
	if(CHK_FLG_NSET(F_KERNEL_INIT_CALLED)){
		return RTX_ERR;
	}

	if(s_size<0x200)
	{
		return RTX_ERR;
	}

	void* addr = k_mem_alloc(s_size);
	if(!addr){
		return RTX_ERR;
	}
	addr += (s_size);// - 1);

	for(uint8_t i=0; i<MAX_TASKS; ++i)
	{
		TCB *currTCB = &MAIN_BB[i];
		if(((int)currTCB->state==TASK_KILLED) && (task->stack_size <= currTCB->max_stack_size)) {
			task_stack_frame givenTaskFrame;
			givenTaskFrame.R4 = 255;
			givenTaskFrame.R5 = 255;
			givenTaskFrame.R6 = 255;
			givenTaskFrame.R7 = 255;
			givenTaskFrame.R8 = 255;
			givenTaskFrame.R9 = 255;
			givenTaskFrame.R10 = 255;
			givenTaskFrame.R11 = 255;
			givenTaskFrame.R0 = 255;
			givenTaskFrame.R1 = 255;
			givenTaskFrame.R2 = 255;
			givenTaskFrame.R3 = 255;
			givenTaskFrame.R12 = 255;
			givenTaskFrame.LR = null_task;
			givenTaskFrame.PC = *(task)->ptask;

			// Check for and skip push{r7, lr}
			// Thread functions will not return, no need to keep lr...
			if(*((unsigned char*)givenTaskFrame.PC - 1) == 0x80){
				if(*((unsigned char*)givenTaskFrame.PC) == 0xb5){

					// Except if we have sub sp, #8 right after...
					if(*((unsigned char*)givenTaskFrame.PC + 1) == 0x82)
						if(*((unsigned char*)givenTaskFrame.PC + 2) == 0xb0){
							givenTaskFrame.PC -= 2;
						}
					givenTaskFrame.PC += 2;
				}
			}
			givenTaskFrame.xPSR = 1<<24;

			// We only want to update global_stack_ptr if we have not written
			// This BB index yet
			currTCB->stack_low = (unsigned int) addr;
			unsigned int top_stack = write_task_to_stack(addr, &givenTaskFrame);

			// "Top of stack" only is context right now
			currTCB->stack_high = top_stack;
			currTCB->stack_size = s_size;
			currTCB->ptask = task->ptask;
			currTCB->state = TASK_READY;
			currTCB->deadline = deadline;
			currTCB->original_deadline = deadline;
			currTCB->timeslice_length = deadline; // Think it's also deadline...

			currTCB->q_elem.data = i;
			currTCB->q_elem.next = 0;
			currTCB->q_elem.prev = 0;

			// Don't think we need the this line below
			// Just for sanity I guess...
			currTCB->q_elem.deadline = &(currTCB->deadline);

			// Might want to disable/enable interrupts during this call...
			insert_in_pq(&MAIN_SCHEDULER.scheduling_q, &(currTCB->q_elem));

			// Want to yield
			// This is because if new task lowest deadline, yield will set that and preempt this caller
			// Otherwise yield should return back into this caller
			if(CHK_FLG_SET(F_KERNEL_START_CALLED)){
				__asm volatile("svc #2");
			}
			return RTX_OK;
		}
	}

	return RTX_ERR;
}

void osKernelInit(void)
{
	if(CHK_FLG_SET(F_KERNEL_INIT_CALLED)){
		// Nothing to do here...
		return;
	}

	SET_FLG(F_KERNEL_INIT_CALLED);

	SHPR3 |= 0xFFU << 24; //Set the priority of SysTick to be the weakest
	SHPR3 |= 0xFEU << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //set the priority of SVC higher than PendSV

	for(uint32_t i=0; i<MAX_TASKS; ++i)
	{
		MAIN_BB[i].stack_high = NULL;
		MAIN_BB[i].stack_low = NULL;
		MAIN_BB[i].tid = i;
		//		MAIN_BB[i].original_tid = NULL;
		MAIN_BB[i].state = TASK_KILLED;
		MAIN_BB[i].stack_size = NULL;
		MAIN_BB[i].max_stack_size = MAX_SHORT;

		MAIN_BB[i].original_deadline = DEFAULT_DEADLINE;
		MAIN_BB[i].deadline = DEFAULT_DEADLINE;
		MAIN_BB[i].timeslice_length = DEFAULT_TIMESLICE;

		MAIN_BB[i].q_elem.next = NULL;
		MAIN_BB[i].q_elem.prev = NULL;
		MAIN_BB[i].q_elem.data = i;
		MAIN_BB[i].q_elem.deadline = &MAIN_BB[i].deadline;
	}

	MAIN_SCHEDULER.bb = MAIN_BB;

	globalVariables.global_stack_ptr  = (*(uint32_t**) 0x0) - 0x200;//sizeof(MAIN_BB) - sizeof(MAIN_SCHEDULER) - sizeof(globalVariableStruct);
	MAIN_BB[0].stack_low = globalVariables.global_stack_ptr;

	task_stack_frame null_sf;
	null_sf.R4 = 255;
	null_sf.R5 = 255;
	null_sf.R6 = 255;
	null_sf.R7 = 255;
	null_sf.R8 = 255;
	null_sf.R9 = 255;
	null_sf.R10 = 255;
	null_sf.R11 = 255;
	null_sf.R0 = 255;
	null_sf.R1 = 255;
	null_sf.R2 = 255;
	null_sf.R3 = 255;
	null_sf.R12 = 255;
	null_sf.LR = null_task;
	null_sf.PC = null_task;
	null_sf.xPSR = 1<<24;

	if(*((unsigned char*)null_sf.PC - 1) == 0x80){
		if(*((unsigned char*)null_sf.PC) == 0xb5){
			null_sf.PC += 2;
		}
	}

	// Give some extra room for null stack
	unsigned int new_ptr = globalVariables.global_stack_ptr;
	new_ptr -= 0x100;
	//globalVariables.global_stack_ptr -= 0x100;
	new_ptr = write_task_to_stack(new_ptr, &null_sf);
	globalVariables.null_task_stack = new_ptr;

	MAIN_BB[0].max_stack_size = STACK_SIZE;
	MAIN_BB[0].state = TASK_READY;
	MAIN_BB[0].ptask = null_task;
	MAIN_BB[0].stack_high = globalVariables.null_task_stack;
	MAIN_BB[0].stack_size = STACK_SIZE;
	MAIN_BB[0].q_elem.data = 0;
	MAIN_BB[0].q_elem.next = 0;
	MAIN_BB[0].q_elem.prev = 0;
	MAIN_BB[0].q_elem.deadline = &(MAIN_BB[0].deadline);

	// Remaining of the 0x200, since 0x100 was first put as empty space
	new_ptr -= (0x100 - sizeof(task_stack_frame));

	globalVariables.global_stack_ptr = new_ptr;
//	enqueue(&MAIN_SCHEDULER.scheduling_q, &MAIN_BB[0].q_elem);

	init_pq(&(MAIN_SCHEDULER.scheduling_q), &(MAIN_BB[0].q_elem));

	null_elem_sleep.data = 0;
	null_elem_sleep.deadline = 0;
	null_elem_sleep.next = 0;
	null_elem_sleep.prev = 0;

	null_elem_periodic = null_elem_sleep;

	init_pq(&(MAIN_SCHEDULER.sleeping_q), &null_elem_sleep);
	init_pq(&(MAIN_SCHEDULER.periodic_q), &null_elem_periodic);
}

int osKernelStart(){
	if (globalVariables.kernelFuncStatus == (unsigned int)F_DEFAULT){
		return RTX_ERR;
	}
	else if(CHK_FLG_SET(F_KERNEL_INIT_CALLED) && CHK_FLG_NSET(F_KERNEL_START_CALLED)){
		SET_FLG(F_KERNEL_START_CALLED);
		globalVariables.tick_enabled = true;
		__asm volatile("svc #3\n");
	}
	else{
		return RTX_ERR;
	}
}

__attribute__((naked)) void osYield(){
	if(CHK_FLG_NSET(F_KERNEL_START_CALLED)){
		__asm("BX lr");
	}
	globalVariables.tick_enabled = false;
	__asm volatile("svc #2\n");
	globalVariables.tick_enabled = true;
	__asm volatile("BX lr\n");
}

__attribute__((naked)) void osTaskExit(){
	if(CHK_FLG_NSET(F_KERNEL_START_CALLED)){
		__asm("BX lr");
	}
	globalVariables.tick_enabled = false;
	__asm volatile("svc #4\n");
	__asm volatile("BX lr\n");
}

int osTaskInfo(unsigned int tid, TCB* tcb){
	if(CHK_FLG_NSET(F_KERNEL_INIT_CALLED)){
		return RTX_ERR;
	}
	if(!tcb){return RTX_ERR;} //no pointer received

	int match_id = -1;

	for (int i = 0; i<MAX_TASKS; i++){
		if ((MAIN_BB[i].tid == tid) && ((int)MAIN_BB[i].state != TASK_KILLED)){ //
			match_id = i;
			break;
		}
	}

	if (match_id==-1){return RTX_ERR;} //no task in BB with matching tid

	tcb->ptask = MAIN_BB[match_id].ptask;
	tcb->stack_high = MAIN_BB[match_id].stack_high;
	tcb->stack_low = MAIN_BB[match_id].stack_low;
	tcb->tid = MAIN_BB[match_id].tid;
	tcb->state = MAIN_BB[match_id].state;
	tcb->stack_size = MAIN_BB[match_id].stack_size;
	tcb->max_stack_size = MAIN_BB[match_id].max_stack_size;

	tcb->deadline = MAIN_BB[match_id].deadline;
	tcb->original_deadline = MAIN_BB[match_id].original_deadline;
	tcb->timeslice_length = MAIN_BB[match_id].timeslice_length;

	tcb->q_elem.data = MAIN_BB[match_id].q_elem.data;
	tcb->q_elem.next = MAIN_BB[match_id].q_elem.next;
	tcb->q_elem.prev = MAIN_BB[match_id].q_elem.prev;
	tcb->q_elem.deadline = MAIN_BB[match_id].q_elem.deadline;


	return RTX_OK;
}

__attribute__((naked)) void osSleep(int timeInMs){
	globalVariables.tick_enabled = false;
	__asm volatile("svc #6\n");
	__asm volatile("BX lr\n");
}

__attribute__((naked)) void osPeriodYield(){
	globalVariables.tick_enabled = false;
	__asm volatile("svc #7\n");
	__asm volatile("BX lr\n");
}

int osSetDeadline(int deadline, unsigned int tid){
	globalVariables.tick_enabled = false;
	__asm volatile("svc #8");
	globalVariables.tick_enabled = true;
	return globalVariables.svc_return[MAIN_SCHEDULER.running];

//	__asm volatile("cpsid if\n");
//	pq_elem* target = remove_from_pq(&(MAIN_SCHEDULER.scheduling_q), tid);
//	if(!target){
//		target = remove_from_pq(&(MAIN_SCHEDULER.periodic_q), tid);
//	}
//	// Might want to also check sleeping q
//	// Waiting for TA answer...
//	// https://piazza.com/class/lqo42wf3gam164/post/358
//
//	if(target){
//		*(target->deadline) = deadline;
//		MAIN_BB[tid].original_deadline = deadline;
//
//		insert_in_pq(&(MAIN_SCHEDULER.scheduling_q), target);
//
//		// Should trigger pendsv...
//		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_YIELD);
//		ret = RTX_OK;
//
//		// Sanity set
//		SCB->ICSR |= 1<<28;
//	}
//	// Hoping this enables interrupts and IMMEDIATELY triggers PendSV with isb
//	// Not sure if this will A, break things or B, be "blocking"
//	__asm volatile(
//		"cpsie if\n"
//		"isb\n"
//	);
//	return ret;
}

///////////////////////////////////////////////////////////////////////////////
