#include "main.h"
#include <stdio.h>
#include "common.h"
#include "k_task.h"
#include "k_mem.h"
#include <stdlib.h>

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

  int N = 100;
  volatile uint32_t* p_temp[N];
  uint32_t timer_start, timer_end, timer_cnt;
  uint32_t full_size = FREE_LIST.head->size;


  // Test malloc perf
  volatile uint32_t* p_temp1;
  timer_start = SysTick->VAL;
  for (int i = 0; i < N; i ++){
	  p_temp[i] = (uint32_t*)k_mem_alloc(4);
  }
  timer_end = SysTick->VAL;
  timer_cnt = timer_start - timer_end;
  printf("k_mem_alloc %lu\r\n", timer_cnt);
  for(int i = 0; i < N; i++){
  	  k_mem_dealloc(p_temp[i]);
  }

  timer_start = SysTick->VAL;
  for (int i = 0; i < N; i ++){
	  p_temp[i] = (uint32_t*)malloc(4);
  }

  timer_end = SysTick->VAL;
  timer_cnt = timer_start - timer_end;
  printf("malloc %lu\r\n", timer_cnt);
  for(int i = 0; i < N; i++){
  	  free(p_temp[i]);
  }




  // Test dealloc perf
  for (int i = 0; i < N; i ++){
	  p_temp[i] = (uint32_t*)malloc(4);
  }
  timer_start = SysTick->VAL;
  for (int i = N-1; i >= 0; i--){
	  free(p_temp[i]);
  }
  timer_end = SysTick->VAL;
  timer_cnt = timer_start - timer_end;
  printf("free %lu | %lu | %lu\r\n", timer_cnt, timer_start, timer_end);


  for (int i = 0; i < N; i++){
	  p_temp[i] = (uint32_t*)k_mem_alloc(4);
  }
  timer_start = SysTick->VAL;
  for (int i = N-1; i >= 0; i--){
	  //if (k_mem_dealloc(p_temp[i]) != RTX_OK) printf("dealloc error \r\n"); //try this line once to make sure it goes through
	  k_mem_dealloc(p_temp[i]); //estimate runtime using this line
  }
  timer_end = SysTick->VAL;
  timer_cnt = timer_start - timer_end;
  printf("k_mem_dealloc %lu | %lu | %lu\r\n", timer_cnt, timer_start, timer_end);


  // Test max memory
  uint32_t m = k_mem_alloc(FREE_LIST.head->size - sizeof(mem_node)); // Should take all of mem
  uint32_t m2 = k_mem_alloc(1); // Should fail (no mem left)
  if(!m){
	  printf("ERR1\r\n");
  }
  if(m2){
	  printf("ERR2\r\n");
  }

  if(RTX_OK != k_mem_dealloc(m)) printf("ERR3\r\n");

  m = k_mem_alloc(FREE_LIST.head->size);
  if(m){
	  printf("ERR4\r\n");
  }

  printf("Basic full mem size memory checks done\r\n");

  int x = 0;

  printf("Starting mass creation test\r\n");
  printf("Full mem size is %d, should expect %d allocations\r\n", full_size, full_size/(sizeof(mem_node)+4));

  while(m = k_mem_alloc(1)){ // Should have same result as line below, just testing rudimentary alignment
//  while(m = k_mem_alloc(4)){
	  x++;
	  m2 = m;
  }
  printf("Allocated %d \r\n", x);

  while(RTX_OK == k_mem_dealloc(m2)){
	  x--;
	  m2 -= (4 + sizeof(mem_node));
  }

  printf("Remaining unfreed %d\r\n", x);

  if(FREE_LIST.head != FREE_LIST.tail){
	  printf("ISSUE WITH FREE_LIST COALESCE!\r\n");
  }
  else{
	  printf("FREE_LIST is 1 node as expected\r\n");
  }

//  while(){
//
//  }


  printf("back to main\r\n");
  while (1);
 }

