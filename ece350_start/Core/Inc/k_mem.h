/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

#include "common.h"

int k_mem_init();
void* k_mem_alloc(size_t size);
int k_mem_dealloc(void* ptr);
int k_mem_count_extfrag(size_t size);

extern uint32_t _img_end;
extern uint32_t _estack;
extern uint32_t _Min_Stack_Size;

#endif /* INC_K_MEM_H_ */
