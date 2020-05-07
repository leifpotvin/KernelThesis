/*
 * kernel.c
 *
 * Created: 4/17/2020 3:10:47 PM
 *  Author: Nathan Potvin
 */ 

#include <avr/io.h>
#include <util/atomic.h>

#include "kernel.h"


/****************************************************************************
*	External function declarations
****************************************************************************/

extern void __attribute__ ((naked)) schedule();
extern void init_system_timer();
extern void init_serial();

/****************************************************************************
*	Local function declarations
****************************************************************************/

void push_pthread(uint8_t tid, PTHREAD func);
void copy_stack(uint8_t *, uint8_t *, volatile uint8_t **);

// macro to initialize a thread's control structure and stack canary
#define THREAD_INIT(tid, stack, stack_size)	\
		kernel_data.thread_ctrl_tbl[tid].stack_ptr = stack + stack_size - 1; \
		kernel_data.thread_ctrl_tbl[tid].stack_base = stack + stack_size - 1;\
		kernel_data.thread_ctrl_tbl[tid].canary_ptr = stack;				 \
		*(kernel_data.thread_ctrl_tbl[tid].canary_ptr) = CANARY;			 \
		kernel_data.thread_ctrl_tbl[tid].entry_pnt							 \
		= (PTHREAD) uninitialized_thread_error;

/****************************************************************************
*	Kernel function definitions
****************************************************************************/

/*
 *	Initialized the kernel and copies the stack to thread0's stack and returns 
 *	executing in thread0
 */
void init()
{
	// initialize kernel data and enable interrupts on exit
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		// initialize the thread control structure and stack canary for 
		// each thread
		THREAD_INIT(THREAD0, kernel_data.stacks.stack0, T0_STACKSZ);
		THREAD_INIT(THREAD1, kernel_data.stacks.stack1, T1_STACKSZ);
		THREAD_INIT(THREAD2, kernel_data.stacks.stack2, T2_STACKSZ);
		THREAD_INIT(THREAD3, kernel_data.stacks.stack3, T3_STACKSZ);
		THREAD_INIT(THREAD4, kernel_data.stacks.stack4, T4_STACKSZ);
		THREAD_INIT(THREAD5, kernel_data.stacks.stack5, T5_STACKSZ);
		THREAD_INIT(THREAD6, kernel_data.stacks.stack6, T6_STACKSZ);
		THREAD_INIT(THREAD7, kernel_data.stacks.stack7, T7_STACKSZ);
		
		// copy the stack to the thread 0 stack and set the stack pointer 
		// register to thread0's stack pointer
		copy_stack(GCC_STACK_BASE, *(uint8_t **)STACK_POINTER
				, &(kernel_data.thread_ctrl_tbl[THREAD0].stack_ptr));
		*STACK_POINTER = kernel_data.thread_ctrl_tbl[THREAD0].stack_ptr;
		
		// initialize the disable status to disable all but thread  0
		kernel_data.schedule_ctrl.disable_status = 
			THREAD1_MSK | THREAD2_MSK | THREAD3_MSK | THREAD4_MSK 
		  | THREAD5_MSK | THREAD6_MSK | THREAD7_MSK;
		// initialize the delay_status so no threads are delayed
		kernel_data.schedule_ctrl.delay_status = 0x00;
		// initialize the current thread and current thread mask to thread0
		kernel_data.schedule_ctrl.cur_thread_id = THREAD0;
		kernel_data.schedule_ctrl.cur_thread_msk = THREAD0_MSK;
		
		// initialize other functionality
		
		init_system_timer();
		#	ifdef SERIAL
		init_serial();
		#	endif /* SERIAL */
	}
}

/*
 *	Initializes a thread's stack to begin execution at a given entry point.
 *	If the given tid is the same as the calling thread, the scheduler will 
 *	be invoked and new will not return
 *
 *	tid:			thread id of the thread being initialized
 *	entry_point:	function pointer that will be the thread's entry point. 
 *					Must be a no arg function. Any functions above 0xffff 
 *					in program memory must make use of a trampoline table
 *	enabled:		boolean value. True if the thread should be enabled 
 *					when this function exits
 */
void new(uint8_t tid, PTHREAD entry_point, bool enabled)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		kernel_data.thread_ctrl_tbl[tid].stack_ptr =
			 kernel_data.thread_ctrl_tbl[tid].stack_base;
		kernel_data.thread_ctrl_tbl[tid].entry_pnt = entry_point;
		push_pthread(tid, entry_point);
		kernel_data.thread_ctrl_tbl[tid].stack_ptr -= THREAD_STACK_CONTEXT_SZ;
		// for preemptive builds initialize the status register 
		// part of the context to have the interrupt enabled
		#		ifdef PREEMPTIVE
		*(kernel_data.thread_ctrl_tbl[tid].stack_ptr + 1) = 0x80;
		#		endif /* PREEMPTIVE */
		
		if (enabled)
		{
			kernel_data.schedule_ctrl.disable_status &= ~(1<<tid);
		}
		else
		{
			kernel_data.schedule_ctrl.disable_status |= 1<<tid;
		}
		
		if (kernel_data.schedule_ctrl.cur_thread_id == tid)
		{
			schedule();
		}
	}
}

/*
 *	enabled the specified thread allowing it to be scheduled
 *
 *	tid:	thread id of the thread to be enabled
 */
void enable(uint8_t tid)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		kernel_data.schedule_ctrl.disable_status &= ~(1<<tid);
	}
}

/****************************************************************************
*	Local function definitions
****************************************************************************/

/*
 *	Pushes a given function pointer to the stack of a given thread.
 *	The function pointer is pushed big endian to allow it to be returned 
 *	to with the ret instruction
 *
 *	tid:	the thread id of the stack that the function pointer will be 
 *	pushed too
 *	func:	the function pointer to be pushed
 */
void push_pthread(uint8_t tid, PTHREAD func)
{
	// push low order byte
	*(kernel_data.thread_ctrl_tbl[tid].stack_ptr--) = 
		(uint8_t) ((((uint16_t) func) & 0x00ff) >> 0);
	// push middle order byte
	*(kernel_data.thread_ctrl_tbl[tid].stack_ptr--) = 
		(uint8_t) ((((uint16_t) func) & 0xff00) >> 8);
	// push high order byte
	// because the ATmega2560 has a 17 bit address space for program memory 
	// and gcc uses 16 bit address space, the high order byte is always zero 
	// for a function pointer and trampoline tables are used for functions 
	// in high parts of program memory
	*(kernel_data.thread_ctrl_tbl[tid].stack_ptr--) = (uint8_t) (0x00);
}

/*
 *	Copies the stack defined by s1_base and s1_ptr to the stack defined by 
 *	s2_ptr
 *
 *	s1_base:	pointer to the base of the stack to be copied
 *	s1_ptr:		the stack pointer of the stack to be copied
 *	s2_ptr:		a pointer to the stack pointer of the stack to be copied to
 */
void copy_stack(uint8_t *s1_base, uint8_t *s1_ptr, volatile uint8_t **s2_ptr)
{
	while (s1_base > s1_ptr)
	{
		**s2_ptr = *s1_base;
		--s1_base;
		--(*s2_ptr);
	}
}