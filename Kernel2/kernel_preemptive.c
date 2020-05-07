/*
 * kernel_preemptive.c
 *
 * Created: 4/17/2020 3:11:09 PM
 *  Author: Nathan Potvin
 */ 

#include <avr/interrupt.h>

#include "kernel.h"

#ifdef PREEMPTIVE

/****************************************************************************
*	Local function declarations
****************************************************************************/

void __attribute__ ((naked)) save_context();
void __attribute__ ((naked)) restore_context();
void __attribute__ ((naked)) schedule();

/****************************************************************************
*	ISR definitions
****************************************************************************/

/*
 *	Timer 2 compare match A ISR
 *
 *	Set to run once per millisecond. Increments system clock and decrements 
 *	delay counters for any delayed threads.
 */
ISR(TIMER2_COMPA_vect)
{
	// increment the compare match to the next millisecond
	OCR2A += (F_CPU / USEC_PER_MILLIS) / TIMER2_PRESCALLER - 1;
	
	// decrement delay counters and clear delay_status bit for any threads 
	// where counter reaches zero
	uint8_t msk = 0x01;
	for (uint8_t i = 0; i < MAX_THREADS; ++i)
	{
		if ((kernel_data.schedule_ctrl.delay_status & msk) 
		&& !(--kernel_data.schedule_ctrl.delay_ctrs[i]))
		{
			kernel_data.schedule_ctrl.delay_status &= ~msk;
		}
		msk <<= 1;
	}
	
	// increment system clock
	++kernel_data.system_time;
}

/*
 *	Timer 2 compare match B ISR.
 *
 *	Set to trigger at the end of the current threads time slice.
 */
__attribute__ ((naked)) ISR(TIMER2_COMPB_vect)
{
	// invloke the scheduler at the save context entry point
	asm volatile ("rjmp save_context");
}

/****************************************************************************
*	Kernel function definitions
****************************************************************************/

/*
 *	Initializes the system timer and the timer to invoke the 
 *	scheduler preemptively.
 */
void init_system_timer()
{
	// initialize all needed registers
	uint8_t com2a = 0b00;				// output pin is disconnected
	uint8_t com2b = 0b00;				// output pin is disconnected
	uint8_t wgm2 = 0b000;				// Normal mode
	uint8_t foc2a = 0b0;
	uint8_t foc2b = 0b0;
	uint8_t cs2 = PRESCALLER_SELECT;	// use prescaller selected dynamically 
										// at compile time
	uint8_t ocie2a = 0b1;				// enable compare match A interrupt
	uint8_t ocie2b = 0b1;				// enable compare match b interrupt
	uint8_t toie2 = 0b0;				// disable overflow interrupt
	
	TCCR2A = (com2a << COM2A0) | (com2b << COM2B0) | ((wgm2 & 0b11) << WGM20);
	TCCR2B = (foc2a << FOC2A) | (foc2b << FOC2B) 
		   | (((wgm2 & 0b100) >> 2) << WGM22) | (cs2 << CS20);
	OCR2A = (uint8_t) ((F_CPU / USEC_PER_MILLIS) / TIMER2_PRESCALLER - 1);
	OCR2B = (uint8_t) (TIME_SLICE / TIMER2_PRESCALLER - 1);
	TIMSK2 = (ocie2a << OCIE2A) | (ocie2b << OCIE2B) | (toie2 << TOIE2);
	
	// set timer to zero
	kernel_data.system_time = 0;
}

/*
 *	Delays the current thread and invokes the scheduler.
 */
void delay(uint16_t delay_millis)
{
	// atomically set the delay counter and set the delay status bit.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		kernel_data.schedule_ctrl.delay_ctrs[kernel_data.schedule_ctrl.cur_thread_id] = 
			delay_millis;
		kernel_data.schedule_ctrl.delay_status |= 
			kernel_data.schedule_ctrl.cur_thread_msk;
	}
	asm volatile ("jmp save_context");
}

/*
 *	Locks the current thread preventing the scheduler from being invoked.
 */
void lock()
{
	// clear the timer compare match B interrupt mask
	TIMSK2 &= ~(0b1 << OCIE2B);
}

/*
 *	Unlock the thread to allow the scheduler to be invoked preemptively.
 */
void unlock()
{
	// set the timer compare match B interrupt mask
	TIMSK2 |= (0b1 << OCIE2B);
}


/****************************************************************************
*	Local function definitions
****************************************************************************/

/*
 *	Saves the current thread context.
 *	Acts as an entry point for the scheduler.
 *	This function enters scheduler after completion.
 */
void __attribute__ ((naked)) save_context()
{	
	// save general purpose registers to stack
	asm volatile ("push r0\n\
				   push r1\n\
				   push r2\n\
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
				   push r18\n\
				   push r19\n\
				   push r20\n\
				   push r21\n\
				   push r22\n\
				   push r23\n\
				   push r24\n\
				   push r25\n\
				   push r26\n\
				   push r27\n\
				   push r28\n\
				   push r29\n\
				   push r30\n\
				   push r31");
	
	// save status register
	asm volatile ("in r0, 0x3f\n\
				   push r0");
	
	//	save stack pointer		   
	kernel_data.thread_ctrl_tbl[kernel_data.schedule_ctrl.cur_thread_id].stack_ptr = 
		*STACK_POINTER;
	
	// jump to the scheduler
	asm volatile ("rjmp schedule");
}

/*
 *	Restores the context of the current thread.
 *	Is invoked after the scheduler runs.
 */
void __attribute__ ((naked)) restore_context()
{
	// increment OCR2B to match on at the end of the next time slice
	OCR2B = TCNT2 + (TIME_SLICE / TIMER2_PRESCALLER - 1);
	
	// enable the TIMER2_COMPB interrupt to allow for rescheduling
	TIMSK2 |= 0b1<<OCIE2B;
	
	// restore stack pointer
	*STACK_POINTER = 
		kernel_data.thread_ctrl_tbl[kernel_data.schedule_ctrl.cur_thread_id].stack_ptr;
	
	// restore status register
	asm volatile ("pop r0\n\
				   out 0x3f, r0");
	
	// restore general purpose registers
	asm volatile ("pop r31\n\
				   pop r30\n\
				   pop r29\n\
				   pop r28\n\
				   pop r27\n\
				   pop r26\n\
				   pop r25\n\
				   pop r24\n\
				   pop r23\n\
				   pop r22\n\
				   pop r21\n\
				   pop r20\n\
				   pop r19\n\
				   pop r18\n\
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
				   pop r2\n\
				   pop r1\n\
				   pop r0");
	
	// the next address on the stack should be the address of the next instruction 
	// to be executed in the current thread
	asm volatile ("reti");
}

/*
 *	Verifies the stack canary and schedulers the next thread.
 *	If no thread is enabled, the scheduler enters a sleep mode.
 */
void __attribute__ ((naked)) schedule()
{
	// disable TIMER2_COMPB interrupt to prevent the scheduler from being invoked 
	// during sleep
	TIMSK2 &= ~(0b1<<OCIE2B);
	
	// verify canary
	if (*(kernel_data.thread_ctrl_tbl[kernel_data.schedule_ctrl.cur_thread_id].canary_ptr) != CANARY)
	{
		stack_overflow();
	}
	
	// compute ready status and sleep if no threads are ready
	uint8_t ready_status;
	do
	{
		ready_status = kernel_data.schedule_ctrl.disable_status;
		ready_status |= kernel_data.schedule_ctrl.delay_status;
		ready_status = ~ready_status;
		if (!ready_status)
		{
			// enter the extended standby sleep mode if no threads are ready
			SMCR = SLEEP_MODE_EXT_STANDBY | (0b1 << SE);
			sei();
			asm volatile ("sleep");
			cli();
		}
	} while (!ready_status);
	
	// schedule the next thread
	do
	{
		kernel_data.schedule_ctrl.cur_thread_id = 
			(kernel_data.schedule_ctrl.cur_thread_id + 1) & 0x07;
		asm volatile ("lsl %1\n\
					   clr r1\n\
					   adc %1, r1"
					   : "=r" (kernel_data.schedule_ctrl.cur_thread_msk)
					   : "r" (kernel_data.schedule_ctrl.cur_thread_msk));
	} while (!(kernel_data.schedule_ctrl.cur_thread_msk & ready_status));
	
	// jump to restore the new current thread's context
	asm volatile ("rjmp restore_context");
}

#endif /* PREEMPTIVE */