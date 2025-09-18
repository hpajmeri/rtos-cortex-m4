/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "scheduler.h"
#include "k_task.h"
#include "linked_list.h"
#include <stdint.h>
#include <stddef.h> // For size_t...
///////////////////////////////////////////////////////////////////////////////
// Macros
///////////////////////////////////////////////////////////////////////////////
#define TID_NULL 0 //predefined Task ID for the NULL task
#define MAX_TASKS 16 //maximum number of tasks in the system
#define STACK_SIZE 0x200 //min. size of each task’s stack
#define TASK_DORMANT 0 //state of terminated task
#define TASK_READY 1 //state of task that can be scheduled but is not running
#define TASK_RUNNING 2 //state of running task
#define TASK_SLEEPING 3 //state of running task
#define TASK_KILLED 255 //TMP!
#define RTX_ERR 0
#define RTX_OK 1
#define MAX_SHORT 0xFFFF
#define WORD_ALLIGNMENT 4 //Alligned to word

// #define DEBUG_ECE350 1		// COMMENT OUT THIS LINE FOR YOUR FINAL SUBMISSION
// debug printf that is only enabled when DEBUG is defined
#ifdef DEBUG_ECE350
    #define DEBUG_PRINTF(fmt, ...) printf("DEBUG_PRINTF<<" fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINTF(fmt, ...)
#endif

#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif

#define CHK_FLG_SET(bitmask) ((bitmask & globalVariables.kernelFuncStatus) == bitmask)
#define CHK_FLG_NSET(bitmask) ((bitmask & globalVariables.kernelFuncStatus) == 0)
#define SET_FLG(bitmask) globalVariables.kernelFuncStatus |= bitmask;
#define USET_FLG(bitmask) globalVariables.kernelFuncStatus &= (bitmask ^ 0xFFFFFFFF);

///////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////
#define F_DEFAULT ((uint32_t)0U)
#define F_KERNEL_INIT_CALLED ((uint32_t)1U)
#define F_KERNEL_START_CALLED ((uint32_t)2U)
#define F_MEMORY_INIT_CALLED ((uint32_t)4U)
#define F_CALLED_FROM_KERNEL ((uint32_t)8U)

typedef struct globalVariableStruct_t
{
	//kernelFuncsCalledStatus kernelFuncStatus;
	uint32_t kernelFuncStatus;
	volatile uint32_t * global_stack_ptr;

	// We need this for when we have no tasks
	void* null_task_stack;
	unsigned int curr_tick_count;
	int svc_return[MAX_TASKS];
	int tick_enabled;
} globalVariableStruct;

///////////////////////////////////////////////////////////////////////////////
// External Symbols
///////////////////////////////////////////////////////////////////////////////
extern struct t_scheduler;
extern struct t_scheduler MAIN_SCHEDULER;

extern struct task_control_block;
extern struct task_control_block MAIN_BB[MAX_TASKS];

extern struct globalVariableStruct_t;
extern struct globalVariableStruct_t globalVariables;

extern struct linked_list;
extern struct linked_list FREE_LIST;
extern struct linked_list ALLOCATED_LIST;
///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////
int checkGlobalFlagsSet(uint32_t bitmask);
int checkGlobalFlagsNotSet(uint32_t bitmask);
void setGlobalFlags(uint32_t bitmask);
void unsetGlobalFlags(uint32_t bitmask);

#endif /* INC_COMMON_H_ */
