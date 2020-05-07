/*
 * kernel.h
 *
 * Created: 4/17/2020 3:09:30 PM
 *  Author: Nathan Potvin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "kernel_config.h"

#ifndef KERNEL_H_
#define KERNEL_H_

/****************************************************************************
*	Kernel build parameters
****************************************************************************/

/****************************************************************************
*	Define thread and stack constants
****************************************************************************/
#define MAX_THREADS 8

#define THREAD0 0
#define THREAD1 1
#define THREAD2 2
#define THREAD3 3
#define THREAD4 4
#define THREAD5 5
#define THREAD6 6
#define THREAD7 7

#define THREAD0_MSK 0b00000001
#define THREAD1_MSK 0b00000010
#define THREAD2_MSK 0b00000100
#define THREAD3_MSK 0b00001000
#define THREAD4_MSK 0b00010000
#define THREAD5_MSK 0b00100000
#define THREAD6_MSK 0b01000000
#define THREAD7_MSK 0b10000000


#define CANARY 0xaa

#define STACK_POINTER ((volatile uint8_t **)(0x5d))
#define GCC_STACK_BASE (uint8_t *) RAMEND

#ifndef PREEMPTIVE
#define THREAD_STACK_CONTEXT_SZ 18
#else
#define THREAD_STACK_CONTEXT_SZ 33
#endif	/* PREEMPTIVE */

/****************************************************************************
*	Define system timer parameters
****************************************************************************/

#define USEC_PER_MILLIS 1000

#ifdef PREEMPTIVE

#	if TIME_SLICE < (1<<8)*64
#		define PRESCALLER_SELECT 0b100
#		define TIMER2_PRESCALLER 64
#		define USEC_PRE_TIME_SLIZE (TIME_SLICE / TIMER2_PRESCALLER)
#	else
#		define PRESCALLER_SELECT 0b101
#		define TIMER2_PRESCALLER 128
#		define USEC_PRE_TIME_SLIZE (TIME_SLICE / TIMER2_PRESCALLER)
#	endif


#	define STR2(x) #x
#	define STR(x) STR2(x)
#	pragma message "Using timer 2 prescaler: " STR(TIMER2_PRESCALLER)


#else /* PREEMPTIVE */

#	define PRESCALLER_SELECT 0b101
#	define TIMER2_PRESCALLER 128

#endif /* PREEMPTIVE */

#ifndef __ASSEMBLER__
/****************************************************************************
*	Kernel data structures
****************************************************************************/

typedef void (*PTHREAD)();

typedef struct  
{
	uint8_t stack0[T0_STACKSZ];
	uint8_t stack1[T1_STACKSZ];
	uint8_t stack2[T2_STACKSZ];
	uint8_t stack3[T3_STACKSZ];
	uint8_t stack4[T4_STACKSZ];
	uint8_t stack5[T5_STACKSZ];
	uint8_t stack6[T6_STACKSZ];
	uint8_t stack7[T7_STACKSZ];
} stack_struct;

typedef struct  
{
	volatile uint8_t *stack_ptr;
	uint8_t *stack_base;
	uint8_t *canary_ptr;
	PTHREAD entry_pnt;
} thread_ctrl_struct;

typedef struct  
{
	uint8_t disable_status;
	uint8_t delay_status;
	uint16_t delay_ctrs[MAX_THREADS];
	uint8_t cur_thread_id;
	uint8_t cur_thread_msk;
} schedule_ctrl_struct;

typedef struct  
{
	stack_struct stacks;
	thread_ctrl_struct thread_ctrl_tbl[MAX_THREADS];
	schedule_ctrl_struct schedule_ctrl;
	volatile uint32_t system_time;
} kernel_data_struct;

kernel_data_struct kernel_data;

/****************************************************************************
*	Kernel function prototypes
****************************************************************************/

void init();
void new(uint8_t, PTHREAD, bool);
void delay(uint16_t);
void disable(uint8_t);
void enable(uint8_t);

/****************************************************************************
*	Cooperative kernel function prototypes
****************************************************************************/

#ifndef PREEMPTIVE
void yield();
#endif /* PREEMPTIVE */

/****************************************************************************
*	Preemptive kernel function prototypes
****************************************************************************/

#ifdef PREEMPTIVE
void lock();
void unlock();
#endif /* PREEMPTIVE */

/****************************************************************************
*	Error function prototypes
****************************************************************************/

void stack_overflow();
void uninitialized_thread_error();

#endif /* __ASSEMBLER__ */

#endif /* KERNEL_H_ */