/*
 * Kernel.h
 *
 * Created: 4/7/2020 10:22:16 AM
 *  Author: Nathan Potvin
 */ 

#ifndef __ASSEMBLER__
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif /* __ASSEMBLER__ */

#ifndef KERNEL_H_
#define KERNEL_H_

#define MAX_THREADS 8

#define DEFAULT_STACK_SZ 0x20
#define T0_STACKSZ DEFAULT_STACK_SZ
#define T1_STACKSZ DEFAULT_STACK_SZ
#define T2_STACKSZ DEFAULT_STACK_SZ
#define T3_STACKSZ DEFAULT_STACK_SZ
#define T4_STACKSZ DEFAULT_STACK_SZ
#define T5_STACKSZ DEFAULT_STACK_SZ
#define T6_STACKSZ DEFAULT_STACK_SZ
#define T7_STACKSZ DEFAULT_STACK_SZ

#define CANARY 0xaa

#ifndef PREEMPTIVE
#define THREAD_STACK_CONTEXT_SZ 18
#else
#define THREAD_STACK_CONTEXT_SZ 32
#endif

#ifndef __ASSEMBLER__
typedef void (*PTHREAD)();

typedef struct 
{
	void *stack_ptr;
	void *stack_base;
	volatile void *canary_ptr;
	PTHREAD entry_pnt;
} thread_ctrl_struct;

typedef struct 
{
	volatile uint8_t stack0[T0_STACKSZ];
	volatile uint8_t stack1[T1_STACKSZ];
	volatile uint8_t stack2[T2_STACKSZ];
	volatile uint8_t stack3[T3_STACKSZ];
	volatile uint8_t stack4[T4_STACKSZ];
	volatile uint8_t stack5[T5_STACKSZ];
	volatile uint8_t stack6[T6_STACKSZ];
	volatile uint8_t stack7[T7_STACKSZ];
} stack_struct;

typedef struct
{
	volatile stack_struct stacks;
	volatile thread_ctrl_struct thread_ctrl_tbl[MAX_THREADS];
	volatile uint8_t disable_status;
	volatile uint8_t delay_status;
	volatile uint16_t delay_ctrs[MAX_THREADS];
	
	volatile uint8_t cur_thread_id;
	volatile uint8_t cur_thread_mask;
	
	uint32_t system_time_millis;
} kernel_data_struct;

volatile kernel_data_struct kernel_data;

/*	Global Funcs	*/
void init();
void new(uint8_t, PTHREAD, bool);
void yield();
void schedule();

#ifdef HEAP
void *x_malloc(size_t);
void x_free(void *);
#endif /* HEAP */

#endif /* __ASSEMBLER__ */

#endif /* KERNEL_H_ */
