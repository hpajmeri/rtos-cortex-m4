/*
 * svc.c
 *
 *  Created on: Feb 1, 2024
 *      Author: ajgho
 */
///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include "svc.h"
#include "k_task.h"
///////////////////////////////////////////////////////////////////////////////
// Global Symbols
///////////////////////////////////////////////////////////////////////////////
// Forward declaring our scheduler and stack
extern scheduler MAIN_SCHEDULER;
extern void* next_stack;
extern struct globalVariableStruct_t globalVariables;
///////////////////////////////////////////////////////////////////////////////
// Function Definitions
///////////////////////////////////////////////////////////////////////////////
// Prepares the stack and calls svc_hander_main
__attribute__((naked)) void SVC_Handler(void)
{
	/*
	 * Declare SVC_Handler_Main
	 * Check if we are in thread or handler mode
	 * -> If handler, grab MSP into r0, else grab PSP
	 * Then branch to SVC_Handler_Main()
	 * Leave with magic numbers
	 */
	__asm volatile(
		".global SVC_Handler_Main\n"
		"TST lr, #4\n"
		"MOV r2, r0\n"
		"MOV r12, r1\n"
		"ITE EQ\n"
		"MRSEQ r0, MSP\n"
		"MRSNE r0, PSP\n"
		"B SVC_Handler_Main\n"
		"END_SVC:\n"
		"BX lr\n"
	);
}



// Actually handles system call
__attribute__((naked)) void SVC_Handler_Main(unsigned int*svc_args){
	/*
	 * _______________________________________________
	 * Stack (svc_args) contains:
	 * r0, r1, r2, r3, r12, r14, the return address and xPSR
	 * 	(and arbitrary r3 after again)
	 *
	 * First argument (r0) is svc_args[0]
	 * _______________________________________________
  	 */

	// Look at function before stacked return address to find svc num
	unsigned int svc_number = ((char*)svc_args[6])[-2];

//	__asm volatile("pop {r0, r7}\n");

	switch(svc_number){
	case 0:
		break;
	case 1:
		// Start thread
		__asm volatile("MRS r0, PSP\n");

		// Load R4-R11, SVC return will handle r0-r3, etc...
		// Then set PSP as bottom of rest of stack we set up
		__asm volatile(
			"LDMIA r0!, {r4-r11}\n"
			"MOV lr, #0xFFFFFFFD\n"
			"MSR PSP, r0\n"
		);
		break;
	case 2:
		// osYield()
		// Yields current task execution
		__asm volatile("push {r7, lr}\n");
		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_YIELD);
		__asm volatile("pop {r7, lr}\n");

		break;
	case 3:
		// osStart()
		// Starts scheduler and runs first task
		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_START);
		break;
	case 4:
		//osTaskExit()
		// Don't push onto stack, we don't care about saving anything
		// We will not return to this task
		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_KILL);
		break;
	case 5:
		// Preemption
		// Basically just osYield except you can't get rescheduled
		__asm volatile("push {r7, lr}\n");
		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_PREEMPT);
		__asm volatile("pop {r7, lr}\n");
		break;

	case 6:
		// Sleep
		// r2 stores the sleep MS
		__asm volatile(
			".global sleep_time\n"
			"LDR r1, =sleep_time\n"
		);
		__asm volatile("STR r2, [r1]\n");

		__asm volatile("push {r7, lr}\n");
		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_SLEEP);
		__asm volatile("pop {r7, lr}\n");
		break;
	case 7:
		// Periodic Yield
		// Sleep but different q and sleep time is remaining time
		__asm volatile("push {r7, lr}\n");
		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_PERIODICYIELD);
		__asm volatile("pop {r7, lr}\n");
		break;
	case 8:
		// Change Deadlines
		// r2 stores deadline
		// r12 stores TID
		__asm volatile(
			".global target_TID\n"
			".global target_deadline\n"
			"LDR r1, =target_TID\n"
		);
		__asm volatile(
			"STR r12, [r1]\n"
			"LDR r1, =target_deadline\n"
		);
		__asm volatile("STR r2, [r1]\n");

		__asm volatile("push {r7, lr}\n");
		run_scheduler(&MAIN_SCHEDULER, SCHEDULER_SETDEADLINE);
		__asm volatile("pop {r7, lr}\n");
	default:
		break;

	}
	// Return
	__asm volatile("B END_SVC\n");

}

// Handles PendSV interrupts (Solely for context switching ATM)
__attribute__((naked)) void PendSV_Handler(void){
	/*
	 * Should also enable/disable interrupts using cspid i / cspie i
	 *
	 * Gets global symbol for task ptrs
	 * 	This is not finalized, should be changed with the scheduler
	 * Pushes software frame onto stack
	 * Grabs software frame from next task stack ptr
	 * Updates PSP
	 * Exits in thread mode using PSP
	 *
	 * */
	__asm volatile(
		".global next_stack\n"
		".global top_of_stack\n"
		"cpsid if\n"
		"MRS r0, PSP\n"
	);
	__asm volatile(
		"STMFD r0!, {r4-r11}\n"
		"LDR r1, =next_stack\n"
	);
	__asm volatile(
		"LDR r2, =top_of_stack\n"
		"LDR r2, [r2]"
	);
	__asm volatile(
		"STR r0, [r2]\n"
	);
	__asm volatile(
		"LDR r1, [r1]\n"
	);
	__asm volatile(
		"LDMIA r1!, {r4-r11}\n"
		"MSR PSP, r1\n"
	);

	__asm volatile("cpsie if\n"); // Re-enable interrupts

	globalVariables.tick_enabled = true;

	__asm volatile(
		"MOV lr, #0xFFFFFFFD\n"	// Probably not needed and may cause bugs
		"BX lr\n"
	);
}
///////////////////////////////////////////////////////////////////////////////
