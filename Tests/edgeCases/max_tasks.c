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


void genericThreadFunc(void){
	int x = 0;
	while(x<2){
		//DEBUG_PRINTF("GY\r\n");
		printf("GY\r\n");
		osYield();
		x++;
	}
//	DEBUG_PRINTF("GEX\r\n");
	printf("GEX\r\n");
	osTaskExit();
}

void toBeReplacedThreadFunc(void){
	while(1){
//		DEBUG_PRINTF("RPY\r\n");
		printf("RPY\r\n");
		osYield();
		break;
	}
//	DEBUG_PRINTF("RPEX\r\n");
	printf("RPEX\r\n");
	osTaskExit();
}

void replacingThreadFunc(void){
	int x = 0;
	while (x<2){
		if (x==1){
//			DEBUG_PRINTF("C\r\n");
			printf("C\r\n");
			TCB task_generic = {genericThreadFunc, 1, 1, 0, 1500};
			osCreateTask(&task_generic);
		}
//		DEBUG_PRINTF("RY\r\n");
		printf("RY\r\n");
		osYield();
		x++;
	}
//	DEBUG_PRINTF("RX\r\n");
	printf("RX\r\n");
	osTaskExit();
}

int main(void)
{

  /* MCU Configuration: Don't change this or the whole chip won't work!*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* MCU Configuration is now complete. Start writing your code below this line */
  osKernelInit();


  for (int i = 1; i<MAX_TASKS; i++){
	  if (i==4){
		TCB task_to_be_replaced = {toBeReplacedThreadFunc, 1, i, 0, 1000};
		osCreateTask(&task_to_be_replaced);
	  }else if (i==5){
		TCB task_replacing = {replacingThreadFunc, 1, i, 0, 1000};
		osCreateTask(&task_replacing);
	  }else{
		TCB task_generic = {genericThreadFunc, 1, i, 0, 600};
		osCreateTask(&task_generic);
	  }

  }

  osKernelStart();


}

