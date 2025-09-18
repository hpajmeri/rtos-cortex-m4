#include "main.h"
#include <stdio.h>
#include "common.h"
#include "k_task.h"
#include "k_mem.h"
#include "stdint.h"

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
  k_mem_init();


  //allocate 1 byte (should get bumped up to 4 bytes)
  uint8_t* p_temp1 = (uint8_t*)k_mem_alloc(1);
  *p_temp1 = 5;

  //allocate 4 bytes
  uint32_t* p_temp2 = (uint32_t*)k_mem_alloc(4);
  *p_temp2 = 6;

  //check that the allocated spaces has kept the values stored into them
  printf("*%p = %d, *%p = %d \r\n", p_temp1, *p_temp1, p_temp2, *p_temp2 );
  //addresses should differ by sizeof(metadata) + 4
  printf("addrs differ by %d bytes, which includes %d bytes of metadata \r\n", (uint32_t)p_temp2 - (uint32_t)p_temp1, sizeof(mem_node));


  //deallocate a valid pointer, should return RTX_OK
  if (k_mem_dealloc(p_temp1) == RTX_OK){
	  printf("*%p dealloc OK\r\n", p_temp1);
  }else{
	  printf("*%p dealloc Error\r\n", p_temp1);
  }

  //deallocate an random pointer, should return RTX_ERR
  if (k_mem_dealloc(p_temp1+35) == RTX_OK){
	  printf("*%p dealloc OK\r\n", p_temp1+35);
  }else{
	  printf("*%p dealloc Error\r\n", p_temp1+35);
  }

  //allocate again, should get the first spot (p_temp1) according to "first fit" algorithm
  uint32_t* p_temp3 = (uint32_t*)k_mem_alloc(4);
  *p_temp3 = 7;
  printf("*%p = %d\r\n", p_temp3, *p_temp3);


  printf("back to main\r\n");
  while (1);
 }
