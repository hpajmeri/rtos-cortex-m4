/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <queue.h>
///////////////////////////////////////////////////////////////////////////////
// Macros
///////////////////////////////////////////////////////////////////////////////
#define DEFAULT_TIMESLICE 5
#define DEFAULT_DEADLINE 5

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 //PendSV is bits 23-16
///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////
typedef struct task_control_block{
	void (*ptask)(void* args); 	//entry address. I believe args should be at ptr in r0?
	unsigned int stack_high; 	//start starting address (high address)
	unsigned int tid; 			//task ID
	char state; 				//task's state
	unsigned short stack_size; 	//stack size. Must be a multiple of 8
	unsigned short max_stack_size;
	unsigned int stack_low;
//	circ_q_elem q_elem;
	pq_elem q_elem;

	unsigned int timeslice_length;
	unsigned int deadline;
	unsigned int original_deadline;

}TCB;

typedef struct t_task_stack_frame{
	uint32_t R4;
	uint32_t R5;
	uint32_t R6;
	uint32_t R7;
	uint32_t R8;
	uint32_t R9;
	uint32_t R10;
	uint32_t R11;
	uint32_t R0; //
	uint32_t R1;
	uint32_t R2;
	uint32_t R3;
	uint32_t R12;
	uint32_t LR; // return Register
	uint32_t PC; // ptask
	uint32_t xPSR;
} task_stack_frame;
///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////
void osKernelInit(void);
int osCreateTask(TCB* task);
int osCreateDeadlineTask(int deadline, int s_size, TCB* task);
int osKernelStart();
__attribute__((naked)) void osYield();
__attribute__((naked)) void osTaskExit();
int osTaskInfo(unsigned int tid, TCB* tcb);
void osSleep(int timeInMs);
void osPeriodYield();
int osSetDeadline(int deadline, unsigned int tid);

volatile uint32_t* write_task_to_stack(volatile uint32_t* stack, task_stack_frame* sf);
///////////////////////////////////////////////////////////////////////////////

#endif /* INC_K_TASK_H_ */
