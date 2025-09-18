#include "common.h"
#include "k_task.h"
#include "main.h"
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!


int i = 0;
int j = 0;
int k = 0;
int m = 0;
int i_cnt;
int i_test = 0;
int i_test2 = 0;


void TaskA(void *) {
	while(1){
		__disable_irq();
		printf("%d, %d\r\n", i_test, i_test2);
		__enable_irq();
		osPeriodYield();
	}
}

void TaskB(void *) {
	while(1){
		i_test = i_test + 1;
		osPeriodYield();
	}
}

void TaskC(void *) {
	while(1){
		i_test2 = i_test2 + 1;
//		osTaskExit(); // Comment this line out
		osPeriodYield();
	}
}

void t1(){
	while(1){
		if(i_test > 1){
			i_test = 0;
			osPeriodYield();
		}
		i = i + 1;
	}
}

void t2(){
	while(1){
		j = j + 1;
		osSleep(2);
	}
}

void t3(){
	while(1){
		if(i_test > 1){
			i_test = 0;
			osPeriodYield();
		}
		k += 1;
	}
}

void t4(){
	while(1){
		if(i_test > 1){
			i_test = 0;
			if(!m)
				osSetDeadline(4, 1);
			osYield();
			m += 1;
			if(m == 897)
				m = 2;
		}

	}
}


void Task1(void *) {
	while(1){
		printf("1\r\n");
		for (i_cnt = 0; i_cnt < 5000; i_cnt++);
		osYield();
	}
}


void Task2(void *) {
	while(1){
		printf("2\r\n");
		for (i_cnt = 0; i_cnt < 5000; i_cnt++);
		osYield();
	}
}


void Task3(void *) {
	while(1){
		printf("3\r\n");
		for (i_cnt = 0; i_cnt < 5000; i_cnt++);
		osYield();
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
  osCreateDeadlineTask(4, STACK_SIZE, &st_mytask);

  st_mytask.ptask = &TaskB;
  osCreateDeadlineTask(4, STACK_SIZE, &st_mytask);

  st_mytask.ptask = &TaskC;
  osCreateDeadlineTask(12, STACK_SIZE, &st_mytask);



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

  while (1){
//	  if(i_test == 1000) printf("100\r\n");

	  printf("%d\r\n", i_test);
  }
}
