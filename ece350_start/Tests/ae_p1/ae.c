/*
 * ae.c
 *
 *  Created on: Oct 26, 2023
 *      Author: mstachow
 */
#include "ae.h"
#include "ae_p1.h"
#include "k_task.h"
void ae_init()
{
	osKernelInit();
#if TEST == 1
	TCB task1;
	task1.ptask=&single_task;
	task1.stack_size = 0x400;
	osCreateTask(&task1);
#elif TEST == 2
	TCB task1;
	task1.ptask=&first_task;
	task1.stack_size = 0x400;
	osCreateTask(&task1);
	TCB task2;
	task2.ptask = &second_task;
	task2.stack_size = 0x400;
	osCreateTask(&task2);
	TCB task3;
	task3.ptask = &third_task;
	task3.stack_size = 0x400;
	osCreateTask(&task3);
#endif
	osKernelStart();

}

