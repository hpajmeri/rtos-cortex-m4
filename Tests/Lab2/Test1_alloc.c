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

/*
 * This test tests mass creating a bunch of integer arrays of size 20 each
 * It then reallocs every other array with a smaller size, the expectation is that
 * At low differences to the original size, there won't be enough space to split a node so there will be no extfrag
 * But at high differences we will see that grow until the split nodes get taken up by the smaller reallocs
 * We then free in a non-linear order with the expectation that everything should be coalesced at the end
 * We know if it's coalesced if we can actively reserve the max amount of memory
 *
 *
 * */

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
		printf("B: %d\r\n", t2mem[y]);
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

  int **arrays = k_mem_alloc(20*sizeof(int*));
  for(int i = 0; i < 20; i++){
	  arrays[i] = k_mem_alloc(20*sizeof(int));
	  for(int j = 0; j < 10; j++){
		  arrays[i][j] = j;
	  }
  }

  for(int i = 0; i < 20; i++){
	  if(i%2){
		  // Skip every other
		  continue;
	  }
	  if(RTX_ERR == k_mem_dealloc(arrays[i])){
		  printf("ERROR DEALLOCING %d\r\n", i);
	  }

	  arrays[i] = k_mem_alloc((20 - i)*sizeof(int));
	  printf("ARR %d size %d\r\n", i, (20-i) * 4);
	  printf("ARR %d size 80\r\n", i + 1);

	  // Should expect no increase (absorbed) -> increase -> decrease (they are allocing into the space)
	  // as i increases
	  for(int k = 0; k < 20; k++){
		  int n = k_mem_count_extfrag(k * sizeof(int));
		  if(n)
			  printf("EXT %d bytes | %d\r\n", k * 4, n);
	  }

  }

  // Freeing arrays[0] should effectively free arrays since they have the same pointer, do it last
  int order[] = {11,4,8,12,16,2,6,10,14,18,19,15,13,17,9,1,3,7,5,0};

  // Try dealloc in "weird" order
  for(int i = 0; i < 20; i++){
	  if(i == 19){
		  int m = 2;
	  }
	  if(RTX_ERR == k_mem_dealloc(arrays[order[i]])){
		  printf("ERROR DEALLOCING %d\r\n", order[i]);
	  }
  }

  int *test = k_mem_alloc(FREE_LIST.head->size - sizeof(mem_node));
  if(!test){
	  printf("ERROR ALLOCATING REMAINDER\r\n");
	  return -1;
  }
  if(RTX_ERR == k_mem_dealloc(test)){
	  printf("ERROR DEALLOCING REMAINDER\r\n");
	  return -2;
  }

  printf("DONE!\r\n");

  return 0;
}

