/*
 * kernel_cooperative.c
 *
 * Created: 4/14/2020 9:20:12 AM
 *  Author: Nathan Potvin
 */ 

#include <avr/interrupt.h>
#include <util/atomic.h>

#ifndef PREEMPTIVE
#include "kernel.h"
#endif /* PREEMPTIVE */

void sleep_mode();

/****************************************************************************
*	ISR definitions
****************************************************************************/

/*
 *	ISR for timer 2. Set to fire every millisecond
 */
ISR(TIMER2_COMPA_vect)
{
	// check if each thread is delayed and decrement its counter if so
	uint8_t msk = 0x01;
	for (uint8_t i = 0; i < MAX_THREADS; ++i)
	{
		if (kernel_data.delay_status & msk)
		{
			if (!(kernel_data.delay_ctrs[i] = kernel_data.delay_ctrs[i] - 1))
			{
				// clear thread's delay bit if counter reached zero
				kernel_data.delay_status &= ~msk;
			}
		}
		msk <<= 1;
	}
	
	// increment system time
	++kernel_data.system_time_millis;
}


void yield()
{
	asm volatile ("push r2\n\
				   push r3\n\
				   push r4\n\
				   push r5\n\
				   push r6\n\
				   push r7\n\
				   push r8\n\
				   push r9\n\
				   push r10\n\
				   push r11\n\
				   push r12\n\
				   push r13\n\
				   push r14\n\
				   push r15\n\
				   push r16\n\
				   push r17\n\
				   push r28\n\
				   push r29");
	
	if (*kernel_data.thread_ctrl_tbl[kernel_data.cur_thread_id].canary_ptr != CANARY)
	{
		stack_overflow();
	}
	
	kernel_data.thread_ctrl_tbl[kernel_data.cur_thread_id].stack_ptr = (uint8_t *) SP;
	
	cli();
	asm volatile ("jmp schedule");
}

void __attribute__ ((OS_main)) schedule()
{
	uint8_t ready_status = 0x00;
	while (!(ready_status = ~(kernel_data.disable_status | kernel_data.delay_status)))
	{
		sleep_mode();
	}
	uint8_t tid = kernel_data.cur_thread_id;
	uint8_t tmsk = kernel_data.cur_thread_mask;
	
	do
	{
		tid = (tid + 1) & 0x07;
		asm volatile ("lsl %1\n"
					  "adc %1, r1"
					  : "=r" (tmsk)
					  : "r" (tmsk));
	} while (!(ready_status & tmsk));
	
	kernel_data.cur_thread_id = tid;
	kernel_data.cur_thread_mask = tmsk;
	
	SP = (uint16_t) kernel_data.thread_ctrl_tbl[tid].stack_ptr;
	
	asm volatile ("pop r29\n\
				   pop r28\n\
				   pop r17\n\
				   pop r16\n\
				   pop r15\n\
				   pop r14\n\
				   pop r13\n\
				   pop r12\n\
				   pop r11\n\
				   pop r10\n\
				   pop r9\n\
				   pop r8\n\
				   pop r7\n\
				   pop r6\n\
				   pop r5\n\
				   pop r4\n\
				   pop r3\n\
				   pop r2");
	sei();
}

void sleep_mode()
{
	sei();
	uint8_t sleep_mode_enable = 0b1;
	uint8_t sleep_mode_select = 0b111;
	SMCR |= (sleep_mode_select << SM0) | (sleep_mode_enable << SE);
	asm volatile ("sleep");
	cli();
}