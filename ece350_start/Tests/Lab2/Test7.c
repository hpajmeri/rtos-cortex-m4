/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "k_mem.h"
#include "k_task.h"
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!

// This tests deallocating and accessing different memories in different thread funcs
// Number 7 in https://piazza.com/class/lqo42wf3gam164/post/264

/**
  * @brief  The application entry point.
  * @retval int
  */

int x = 0;
int y = 0;
int *t1mem = 4;
int *t2mem = 4;


TCB glob;

void threadfunc1(void){
	int *k = k_mem_alloc(sizeof(int)*10);
	t1mem = k;
	printf("%u\r\n", k);
	while(1){
		t1mem[x] = x;
		printf("%A: %d\r\n", t1mem[x]);
		if(x > 0){
			if(RTX_ERR == k_mem_dealloc(t2mem)){
				printf("NWA\r\n");
			}
		}
		x++;
		if(x > 10){
			if(RTX_OK == k_mem_dealloc(t1mem)){
				printf("DA\r\n");
			}
			else{
				printf("NDA\r\n");
			}
			osTaskExit();
		}
		osYield();
	}
}

void threadfunc2(void){
	int *k = k_mem_alloc(sizeof(int)*10);
	t2mem = k;
	printf("%u\r\n", k);
	while(1){
		t2mem[y] = y;
		printf("%B: %d\r\n", t2mem[y]);
		if(RTX_ERR == k_mem_dealloc(t1mem)){
			printf("NWB\r\n");
		}
		y++;
		if(y > 20){
			if(RTX_OK == k_mem_dealloc(t2mem)){
				printf("DB\r\n");
			}
			else{
				printf("NDB\r\n");
			}

			osTaskExit();
		}
		osYield();
	}
}

int main(void)
{

  /* MCU Configuration: Don't change this or the whole chip won't work!*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

//  osYield();
//  osTaskExit();

  /* Initialize all configured peripherals */
  //TCB task1 = {threadfunc1, 10, 10, 0, 0x3600, 0x10};
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* MCU Configuration is now complete. Start writing your code below this line */
  osKernelInit();

  if(!k_mem_init()){
	  printf("FAILED TO INIT\r\n");
	  return -1;
  }
  int* array = k_mem_alloc(10*sizeof(int));
  if(!array){
	  printf("FAILED TO ALLOC\r\n");
	  return -2;
  }

  for(int i = 0; i < 10; i++){
	  array[i] = i;
  }

  for(int i = 0; i < 10; i++){
	  printf("%d\r\n", array[i]);
  }


  if(!k_mem_dealloc(array)){
	  printf("FAILED TO DEALLOC\r\n");
	  return -3;
  }

  TCB task1 = {threadfunc1, 10, 10, 0, 1024, 1024};
  TCB task2 = {threadfunc2, 10, 10, 0, 1024, 1024};

  osCreateTask(&task1);
  osCreateTask(&task2);

  osKernelStart();

  return 0;
}
