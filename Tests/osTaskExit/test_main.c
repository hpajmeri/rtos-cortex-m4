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

/**
  * @brief  The application entry point.
  * @retval int
  */

int x = 0;

__attribute__((naked)) void threadfunc1(void){
	while(1){
		printf("A\r\n");
		x++;
		if(x > 15){
			osTaskExit();
		}
		osYield();
	}
}

__attribute__((naked)) void threadfunc2(void){
	while(1){
		printf("B\r\n");
		x++;
		if(x > 20){
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

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* MCU Configuration is now complete. Start writing your code below this line */
  osKernelInit();

  TCB task1 = {threadfunc1, 10, 10, 0, 50, 512};
  TCB task2 = {threadfunc2, 10, 10, 0, 50, 512};

  osCreateTask(&task1);
  osCreateTask(&task2);

  osKernelStart();
}

