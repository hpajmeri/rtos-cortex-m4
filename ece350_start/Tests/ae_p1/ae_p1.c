/*
 * ae_p1.c
 *
 *  Created on: Oct 26, 2023
 *      Author: mstachow
 */
#include <stdio.h>
#include "ae.h"

int x = 0;

void single_task(void* args){
	while(1){
		printf("Hello, world\r\n");
		osYield();
	}
}

void third_task(void* args){
	while(1)
	{
		x++;
		for(int i = 0; i < 10000000; i++);
		osYield();
	}
}

void first_task(void*args)
{
	while(1){
		printf("Hello, world!\r\n");

		if(x > 10)
			osTaskExit();
		osYield();
	}
}

void second_task(void* args)
{
	TCB myTCB;
	while(1){
		printf("There we go\r\n");
	    osTaskInfo(0,&myTCB);
	    printf("x is: %d\r\n",x);
		osYield();
	}
}
