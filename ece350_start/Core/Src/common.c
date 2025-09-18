/*
 * common.c
 *
 *  Created on: Feb 5, 2024
 *      Author: hpajmeri
 */

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "main.h"
#include "scheduler.h"
#include "common.h"
#include "k_task.h"
#include "linked_list.h"
///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////
TCB MAIN_BB [MAX_TASKS];
globalVariableStruct globalVariables = {.kernelFuncStatus = F_DEFAULT, .global_stack_ptr = 0};
scheduler MAIN_SCHEDULER;
LL FREE_LIST;
LL ALLOCATED_LIST;
int checkGlobalFlagsSet(uint32_t bitmask){
	if((bitmask & globalVariables.kernelFuncStatus) == bitmask){
		return 1;
	}
	return 0;
}

int checkGlobalFlagsNotSet(uint32_t bitmask){
	if((bitmask & globalVariables.kernelFuncStatus) == 0){
		return 1;
	}
	return 0;
}

void setGlobalFlags(uint32_t bitmask){
	globalVariables.kernelFuncStatus |= bitmask;
}

void unsetGlobalFlags(uint32_t bitmask){
	bitmask = bitmask ^ 0xFFFFFFFF;
	globalVariables.kernelFuncStatus &= bitmask;
}
