/*
 * kernel.c
 *
 * Created: 4/7/2020 10:22:27 AM
 *  Author: Nathan Potvin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "kernel.h"

// local function prototypes
void init_system_timer();
void push_pthread(uint8_t, PTHREAD);

/****************************************************************************
*	Local function definitions
****************************************************************************/

/*
 *	Initializes the system timer.
 */
void init_system_timer()
{
	// initialize all needed registers
	uint8_t com2a = 0b00;	// output pin is disconnected
	uint8_t com2b = 0b00;	// output pin is disconnected
	uint8_t wgm2 = 0b010;	// CTC mode with TOP at OCRA
	uint8_t foc2a = 0b0;
	uint8_t foc2b = 0b0;
	uint8_t cs2 = 0b101;	// use clkt2s/128 prescaller
	uint8_t ocie2a = 0b1;	// enable compare match A interrupt
	uint8_t ocie2b = 0b0;	// disable compare match b interrupt
	uint8_t toie2 = 0b0;	// disable overflow interrupt
	
	TCCR2A = (com2a << COM2A0) | (com2b << COM2B0) | ((wgm2 & 0b11) << WGM20);
	TCCR2B = (foc2a << FOC2A) | (foc2b << FOC2B) | (((wgm2 & 0b100) >> 2) << WGM22) | (cs2 << CS20);
	OCR2A = (F_CPU / 1000) / 128 - 1;
	TIMSK2 = (ocie2a << OCIE2A) | (ocie2b << OCIE2B) | (toie2 << TOIE2);
	
	// set timer to zero
	kernel_data.system_time_millis = 0;
}

/****************************************************************************
*	Kernel function definitions
****************************************************************************/

/*
 *	Pushes a given function pointer to the stack of a given thread.
 *	The function pointer is pushed big endian to allow it to be returned to with the ret instruction
 *
 *	tid:	the thread id of the stack that the function pointer will be pushed too
 *	func:	the function pointer to be pushed
 */
void push_pthread(uint8_t tid, PTHREAD func)
{
	// push low order byte
	*(kernel_data.thread_ctrl_tbl[tid].stack_ptr--) = (uint8_t) ((((uint16_t) func) & 0x00ff) >> 0);
	// push middle order byte
	*(kernel_data.thread_ctrl_tbl[tid].stack_ptr--) = (uint8_t) ((((uint16_t) func) & 0xff00) >> 8);
	// push high order byte
	// because the ATmega2560 has a 17 bit address space for program memory and gcc uses 
	// 16 bit address space, the high order byte is always zero for a function pointer and 
	//trampoline tables are used for functions in high parts of program memory
	*(kernel_data.thread_ctrl_tbl[tid].stack_ptr--) = (uint8_t) (0x00);
}

/*
 *	Initializes kernel data structures.
 */
void init()
{	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// set the stack base pointer for each thread
		kernel_data.thread_ctrl_tbl[0].stack_base = (uint8_t *) (kernel_data.stacks.stack0 + T0_STACKSZ - 1);
		kernel_data.thread_ctrl_tbl[1].stack_base = (uint8_t *) (kernel_data.stacks.stack1 + T1_STACKSZ - 1);
		kernel_data.thread_ctrl_tbl[2].stack_base = (uint8_t *) (kernel_data.stacks.stack2 + T2_STACKSZ - 1);
		kernel_data.thread_ctrl_tbl[3].stack_base = (uint8_t *) (kernel_data.stacks.stack3 + T3_STACKSZ - 1);
		kernel_data.thread_ctrl_tbl[4].stack_base = (uint8_t *) (kernel_data.stacks.stack4 + T4_STACKSZ - 1);
		kernel_data.thread_ctrl_tbl[5].stack_base = (uint8_t *) (kernel_data.stacks.stack5 + T5_STACKSZ - 1);
		kernel_data.thread_ctrl_tbl[6].stack_base = (uint8_t *) (kernel_data.stacks.stack6 + T6_STACKSZ - 1);
		kernel_data.thread_ctrl_tbl[7].stack_base = (uint8_t *) (kernel_data.stacks.stack7 + T7_STACKSZ - 1);
		
		// set stack canary pointer for each thread
		kernel_data.thread_ctrl_tbl[0].canary_ptr = kernel_data.stacks.stack0;
		kernel_data.thread_ctrl_tbl[1].canary_ptr = kernel_data.stacks.stack1;
		kernel_data.thread_ctrl_tbl[2].canary_ptr = kernel_data.stacks.stack2;
		kernel_data.thread_ctrl_tbl[3].canary_ptr = kernel_data.stacks.stack3;
		kernel_data.thread_ctrl_tbl[4].canary_ptr = kernel_data.stacks.stack4;
		kernel_data.thread_ctrl_tbl[5].canary_ptr = kernel_data.stacks.stack5;
		kernel_data.thread_ctrl_tbl[6].canary_ptr = kernel_data.stacks.stack6;
		kernel_data.thread_ctrl_tbl[7].canary_ptr = kernel_data.stacks.stack7;
		
		// initialize remaining variables for each thread
		for (uint8_t i = 0; i < MAX_THREADS; ++i)
		{
			// initialize the stack pointer to the stack base
			kernel_data.thread_ctrl_tbl[i].stack_ptr = kernel_data.thread_ctrl_tbl[i].stack_base;
			// initialize the entry point for each thread to null
			kernel_data.thread_ctrl_tbl[i].entry_pnt = (PTHREAD) 0x0000;
			// set the canary in each stack
			*(kernel_data.thread_ctrl_tbl[i].canary_ptr) = CANARY;
			// clear each delay counter
			kernel_data.delay_ctrs[i] = 0x0000;
		}
		
		// set only thread 0 as enabled
		kernel_data.disable_status = 0xfe;
		// set all threads to not be delayed
		kernel_data.delay_status = 0x00;
		
		// set current thread to thread 0
		kernel_data.cur_thread_id = 0;
		kernel_data.cur_thread_mask = 0x01;
		
		// copy the stack to the thread 0 stack
		uint8_t *ptr = (uint8_t *) GCC_STACK_BASE;
		while (ptr > (uint8_t *) SP)
		{
			*(kernel_data.thread_ctrl_tbl[0].stack_ptr--) = *(ptr--);
		}
		SP = (uint16_t) kernel_data.thread_ctrl_tbl[0].stack_ptr;
		
		// initialize system timer
		init_system_timer();
	}
}

/*
 *	Creates a new thread at a given thread id.
 *	If the given thread id has already been created, the thread is reset.
 *	If the given thread_id is the current thread, the scheduler will be invoked.
 *	
 *	thread_id:		id of the new thread. Must be between zero and MAX_THREADS
 *	entry_point:	function pointer to the entry point of the thread
 *	enabled:		boolean value if the thread is enabled
 */
void new(uint8_t thread_id, PTHREAD entry_point, bool enabled)
{
	// reset stack pointer to its base
	kernel_data.thread_ctrl_tbl[thread_id].stack_ptr = kernel_data.thread_ctrl_tbl[thread_id].stack_base;
	// push the entry point to the stack
	push_pthread(thread_id, entry_point);
	// decrement the stack pointer for the thread's context
	kernel_data.thread_ctrl_tbl[thread_id].stack_ptr -= THREAD_STACK_CONTEXT_SZ;
	// save the entry point to the thread's control data
	kernel_data.thread_ctrl_tbl[thread_id].entry_pnt = entry_point;
	
	// set or clear thread's disable bit
	if (!enabled)
	{
		kernel_data.disable_status |= (0x1 << thread_id);
	}
	else
	{
		kernel_data.disable_status &= ~(0x1 << thread_id);
	}
	
	// if the current thread is being recreated, verify stack canary and invoke the scheduler
	if (thread_id == kernel_data.cur_thread_id)
	{
		if (*(kernel_data.thread_ctrl_tbl[thread_id].canary_ptr) != CANARY) stack_overflow();
		schedule();
	}
}

/*
 *	Delays the current thread by at least the specified number of milliseconds and yield.
 *
 *	t: minimum delay time in milliseconds
 */
void delay(uint16_t t)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// set current thread's delay bit
		kernel_data.delay_status |= kernel_data.cur_thread_mask;
		// set current thread's delay counter
		kernel_data.delay_ctrs[kernel_data.cur_thread_id] = t;
	}
	yield();
}

/*
 *	Function that will be entered when a stack overflow is detected.
 */
void stack_overflow()
{
	DDRB |= 0x80;
	while (1)
	{
		PORTB ^= 0x80;
		_delay_ms(250);
	}
}
