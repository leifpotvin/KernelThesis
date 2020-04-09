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

#include "Kernel.h"

void init_system_timer();
#ifdef HEAP
void init_heap();
#endif /* HEAP */

void push_context();

void stack_overflow();

ISR(TIMER2_COMPA_vect)
{
	uint8_t msk = 0x01;
	for (uint8_t i = 0; i < MAX_THREADS; ++i)
	{
		if (kernel_data.delay_status & msk)
		{
			if (!(kernel_data.delay_ctrs[i] = kernel_data.delay_ctrs[i] - 1))
			{
				kernel_data.delay_status &= ~msk;
			}
		}
		msk <<= 1;
	}
	
	++kernel_data.system_time_millis;
}

void init()
{	
	kernel_data.thread_ctrl_tbl[0].stack_base = (uint8_t *) (kernel_data.stacks.stack0 + T0_STACKSZ - 1);
	kernel_data.thread_ctrl_tbl[1].stack_base = (uint8_t *) (kernel_data.stacks.stack1 + T1_STACKSZ - 1);
	kernel_data.thread_ctrl_tbl[2].stack_base = (uint8_t *) (kernel_data.stacks.stack2 + T2_STACKSZ - 1);
	kernel_data.thread_ctrl_tbl[3].stack_base = (uint8_t *) (kernel_data.stacks.stack3 + T3_STACKSZ - 1);
	kernel_data.thread_ctrl_tbl[4].stack_base = (uint8_t *) (kernel_data.stacks.stack4 + T4_STACKSZ - 1);
	kernel_data.thread_ctrl_tbl[5].stack_base = (uint8_t *) (kernel_data.stacks.stack5 + T5_STACKSZ - 1);
	kernel_data.thread_ctrl_tbl[6].stack_base = (uint8_t *) (kernel_data.stacks.stack6 + T6_STACKSZ - 1);
	kernel_data.thread_ctrl_tbl[7].stack_base = (uint8_t *) (kernel_data.stacks.stack7 + T7_STACKSZ - 1);

	kernel_data.thread_ctrl_tbl[0].stack_ptr = kernel_data.thread_ctrl_tbl[0].stack_base;
	kernel_data.thread_ctrl_tbl[1].stack_ptr = kernel_data.thread_ctrl_tbl[1].stack_base;
	kernel_data.thread_ctrl_tbl[2].stack_ptr = kernel_data.thread_ctrl_tbl[2].stack_base;
	kernel_data.thread_ctrl_tbl[3].stack_ptr = kernel_data.thread_ctrl_tbl[3].stack_base;
	kernel_data.thread_ctrl_tbl[4].stack_ptr = kernel_data.thread_ctrl_tbl[4].stack_base;
	kernel_data.thread_ctrl_tbl[5].stack_ptr = kernel_data.thread_ctrl_tbl[5].stack_base;
	kernel_data.thread_ctrl_tbl[6].stack_ptr = kernel_data.thread_ctrl_tbl[6].stack_base;
	kernel_data.thread_ctrl_tbl[7].stack_ptr = kernel_data.thread_ctrl_tbl[7].stack_base;
	
	kernel_data.thread_ctrl_tbl[0].canary_ptr = &kernel_data.stacks.stack0[0];
	kernel_data.thread_ctrl_tbl[1].canary_ptr = &kernel_data.stacks.stack1[1];
	kernel_data.thread_ctrl_tbl[2].canary_ptr = &kernel_data.stacks.stack2[2];
	kernel_data.thread_ctrl_tbl[3].canary_ptr = &kernel_data.stacks.stack3[3];
	kernel_data.thread_ctrl_tbl[4].canary_ptr = &kernel_data.stacks.stack4[4];
	kernel_data.thread_ctrl_tbl[5].canary_ptr = &kernel_data.stacks.stack5[5];
	kernel_data.thread_ctrl_tbl[6].canary_ptr = &kernel_data.stacks.stack6[6];
	kernel_data.thread_ctrl_tbl[7].canary_ptr = &kernel_data.stacks.stack7[7];
	
	kernel_data.thread_ctrl_tbl[0].entry_pnt = (PTHREAD) 0x0000;
	kernel_data.thread_ctrl_tbl[1].entry_pnt = (PTHREAD) 0x0000;
	kernel_data.thread_ctrl_tbl[2].entry_pnt = (PTHREAD) 0x0000;
	kernel_data.thread_ctrl_tbl[3].entry_pnt = (PTHREAD) 0x0000;
	kernel_data.thread_ctrl_tbl[4].entry_pnt = (PTHREAD) 0x0000;
	kernel_data.thread_ctrl_tbl[5].entry_pnt = (PTHREAD) 0x0000;
	kernel_data.thread_ctrl_tbl[6].entry_pnt = (PTHREAD) 0x0000;
	kernel_data.thread_ctrl_tbl[7].entry_pnt = (PTHREAD) 0x0000;
	
	kernel_data.stacks.stack0[0] = CANARY;
	kernel_data.stacks.stack1[0] = CANARY;
	kernel_data.stacks.stack2[0] = CANARY;
	kernel_data.stacks.stack3[0] = CANARY;
	kernel_data.stacks.stack4[0] = CANARY;
	kernel_data.stacks.stack5[0] = CANARY;
	kernel_data.stacks.stack6[0] = CANARY;
	kernel_data.stacks.stack7[0] = CANARY;
	
	kernel_data.disable_status = 0xff;
	kernel_data.delay_status = 0x00;
	
	kernel_data.delay_ctrs[0] = 0x0000;
	kernel_data.delay_ctrs[1] = 0x0000;
	kernel_data.delay_ctrs[2] = 0x0000;
	kernel_data.delay_ctrs[3] = 0x0000;
	kernel_data.delay_ctrs[4] = 0x0000;
	kernel_data.delay_ctrs[5] = 0x0000;
	kernel_data.delay_ctrs[6] = 0x0000;
	kernel_data.delay_ctrs[7] = 0x0000;
	
	kernel_data.cur_thread_id = 0;
	kernel_data.cur_thread_mask = 0x01;
	
	init_system_timer();
	
#	ifdef HEAP
	init_heap();
#	endif /* HEAP */
}

void init_system_timer()
{
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
	
	kernel_data.system_time_millis = 0;
	
	sei();
}

#ifdef HEAP
void init_heap()
{
	
}

void *x_malloc(size_t size)
{
	return (void *) 0x0000;
}

void x_free(void *ptr)
{
	
}
#endif /* HEAP */

void new(uint8_t thread_id, PTHREAD entry_point, bool disabled)
{
	kernel_data.thread_ctrl_tbl[thread_id].stack_ptr = kernel_data.thread_ctrl_tbl[thread_id].stack_base - THREAD_STACK_CONTEXT_SZ;
	kernel_data.thread_ctrl_tbl[thread_id].entry_pnt = entry_point;
	
	if (disabled)
	{
		kernel_data.disable_status &= ~(0x1 << thread_id);
	}
	else
	{
		kernel_data.disable_status |= 0x1 << thread_id;
	}
	
	if (thread_id == kernel_data.cur_thread_id)
	{
		schedule();
	}
}

#ifndef PREEMPTIVE
void push_context()
{
	__asm__("push r2");
	__asm__("push r3");
	__asm__("push r4");
	__asm__("push r5");
	__asm__("push r6");
	__asm__("push r7");
	__asm__("push r8");
	__asm__("push r9");
	__asm__("push r10");
	__asm__("push r11");
	__asm__("push r12");
	__asm__("push r13");
	__asm__("push r14");
	__asm__("push r15");
	__asm__("push r16");
	__asm__("push r17");
	
	__asm__("push r28");
	__asm__("push r29");
}
#else
void push_context()
{
	// 	__asm__("push r0");
	// 	__asm__("push r1");
	// 	__asm__("push r2");
	// 	__asm__("push r3");
	// 	__asm__("push r4");
	// 	__asm__("push r5");
	// 	__asm__("push r6");
	// 	__asm__("push r7");
	// 	__asm__("push r8");
	// 	__asm__("push r9");
	// 	__asm__("push r10");
	// 	__asm__("push r11");
	// 	__asm__("push r12");
	// 	__asm__("push r13");
	// 	__asm__("push r14");
	// 	__asm__("push r15");
	// 	__asm__("push r16");
	// 	__asm__("push r17");
	// 	__asm__("push r18");
	// 	__asm__("push r19");
	// 	__asm__("push r20");
	// 	__asm__("push r21");
	// 	__asm__("push r22");
	// 	__asm__("push r23");
	// 	__asm__("push r24");
	// 	__asm__("push r25");
	// 	__asm__("push r26");
	// 	__asm__("push r27");
	// 	__asm__("push r28");
	// 	__asm__("push r29");
	// 	__asm__("push r30");
	// 	__asm__("push r31");
	//
	// 	__asm__("");
}
#endif /* PREEMPTIVE */

// void yield()
// {
// 	push_context();
// 	
// 	if (*((uint8_t *) kernel_data.thread_ctrl_tbl[kernel_data.cur_thread_id].canary_ptr) != CANARY)
// 	{
// 		stack_overflow();
// 	}
// 	
// 	schedule();
// }

// void schedule()
// {
// 	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
// 	{
// 		__asm__("");
// 	}
// }

// void sleep()
// {
// 	
// }

void stack_overflow()
{
	DDRB |= 0x80;
	while (1)
	{
		PORTB ^= 0x80;
		_delay_ms(250);
	}
}