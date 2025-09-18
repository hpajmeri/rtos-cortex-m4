#include "common.h"
#include "k_task.h"
#include "main.h"
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!

int flag = 1;

void TaskA(void *) {
	while(MAIN_BB[1].deadline != 1){
		__disable_irq();
		printf("A:, %d\r\n", MAIN_BB[1].deadline);
		__enable_irq();
//		osPeriodYield();
		if (MAIN_BB[1].deadline==10 && flag){
			flag = 0;
			osSetDeadline(15, 2);
			osYield();
		}
	}
}

void TaskB(void *) {
	while(MAIN_BB[2].deadline != 1){
		__disable_irq();
		printf("B:, %d\r\n", MAIN_BB[2].deadline);
		__enable_irq();
//		osPeriodYield();
	}
}



int main(void) {
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

  k_mem_init();

//  sanity_test(255, 10);

  TCB st_mytask;
  st_mytask.stack_size = 0x400;

  st_mytask.ptask = &TaskA;
  osCreateDeadlineTask(20, STACK_SIZE, &st_mytask);

//  osCreateDeadlineTask(13, STACK_SIZE, &st_mytask);

  st_mytask.ptask = &TaskB;
  osCreateDeadlineTask(100, STACK_SIZE, &st_mytask);

//  st_mytask.ptask = &TaskC;
//  osCreateDeadlineTask(20, STACK_SIZE, &st_mytask);



//  st_mytask.ptask = &Task1;
//  osCreateTask(&st_mytask);
//
//  st_mytask.ptask = &Task2;
//  osCreateTask(&st_mytask);
//
//  st_mytask.ptask = &Task3;
//  osCreateTask(&st_mytask);

//  st_mytask.ptask = &t3;
//  osCreateDeadlineTask(10, 0x800, &st_mytask);
////  osCreateTask(&st_mytask);
//
//  st_mytask.ptask = &t1;
//  osCreateDeadlineTask(3, 0x400, &st_mytask);
////  osCreateTask(&st_mytask);
//
//  st_mytask.ptask = &t2;
//  osCreateTask(&st_mytask);
//
//  st_mytask.ptask = &t4;
//  osCreateTask(&st_mytask);


  osKernelStart();

//  while (1){
////	  if(i_test == 1000) printf("100\r\n");
//
//	  printf("%d\r\n", i_test);
//  }
}
