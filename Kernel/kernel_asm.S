
/*
 * kernel_asm.S
 *
 * Created: 4/9/2020 11:58:22 AM
 *  Author: Nathan Potvin
 */ 

#ifdef PREEMPTIVE
#include "kernel.h"
#include "struct_offset.h"

#define SP_LO_IO_ADDR	0x3d
#define SP_HI_IO_ADDR	0x3e

		.section .text

		/* Function Defs */
		.global yield
		.global schedule

		//.global sleep_mode
		.global stack_overflow

		/* Variable Defs */
		.global kernel_data


#define X_LO			r26
#define X_HI			r27
#define X_LO_BKUP		r20
#define X_HI_BKUP		r21

#define TID				r18
#define TMSK			r19
#define CTRL_SZ			r19
#define OFFSET_LO		r20
#define OFFSET_HI		r21
#define SP_LO			r20
#define SP_HI			r21
#define CANARY_REG		r19

#define DISABLE_STAT	r22
#define DELAY_STAT		r23
#define READY_STAT		r23

yield:
		; push callee save registers to stack
		push r2
		push r3
		push r4
		push r5
		push r6
		push r7
		push r8
		push r9
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15
		push r16
		push r17
	
		push r28
		push r29
		
		; save stack pointer
		ldi X_HI, hi8(kernel_data + offsetof_kernel_data_struct_cur_thread_id)
		ldi X_LO, lo8(kernel_data + offsetof_kernel_data_struct_cur_thread_id)
		ld TID, X
		
		ldi CTRL_SZ, sizeof_thread_ctrl_struct		; compute offset of current trhead's control table
		mul CTRL_SZ, TID
		movw X_LO, r0
		
		ldi OFFSET_HI, hi8(kernel_data + offsetof_kernel_data_struct_thread_ctrl_tbl + offsetof_thread_ctrl_struct_stack_ptr)
		ldi OFFSET_LO, lo8(kernel_data + offsetof_kernel_data_struct_thread_ctrl_tbl + offsetof_thread_ctrl_struct_stack_ptr)
		
		add X_LO, OFFSET_LO
		adc X_HI, OFFSET_HI
		
		in SP_HI, SP_HI_IO_ADDR
		in SP_LO, SP_LO_IO_ADDR
		
		st X+, SP_LO
		st X, SP_HI
		
		; verify canary
		ldi OFFSET_HI, hi8(offsetof_thread_ctrl_struct_canary_ptr - offsetof_thread_ctrl_struct_stack_ptr - 1)
		ldi OFFSET_LO, lo8(offsetof_thread_ctrl_struct_canary_ptr - offsetof_thread_ctrl_struct_stack_ptr - 1)
		
		add X_LO, OFFSET_LO
		adc X_HI, OFFSET_HI
		
		ld X_LO_BKUP, X+ // need to derefence
		ld X_HI_BKUP, X
		movw X_LO, X_LO_BKUP
		ld CANARY_REG, X
		subi CANARY_REG, CANARY
		breq schedule
		rcall stack_overflow

schedule:
	
		cli

		; load and compute ready stat
		ldi X_HI, hi8(kernel_data + offsetof_kernel_data_struct_disable_status)
		ldi X_LO, lo8(kernel_data + offsetof_kernel_data_struct_disable_status)
		ld DISABLE_STAT, X+
		ld DELAY_STAT, X
		or DELAY_STAT, DISABLE_STAT
		com READY_STAT

		tst READY_STAT
		breq sleep_mode

		; load cur_thread_id
//		ldi X_HI, hi8(kernel_data + offsetof_kernel_data_struct_cur_thread_id)
//		ldi X_LO, lo8(kernel_data + offsetof_kernel_data_struct_cur_thread_id)
//		ld TID, X

		; load cur_thread_mask
		ldi X_HI, hi8(kernel_data + offsetof_kernel_data_struct_cur_thread_mask)
		ldi X_LO, lo8(kernel_data + offsetof_kernel_data_struct_cur_thread_mask)
		ld TMSK, X

top_of_schedule_loop_schedule:
		lsl TMSK				; TMSK = TMSK << 1
		adc TMSK, r1			; TMSK = TMSK + 0 + carry flag

		inc TID					; TID = TID + 1
		andi TID, 0b111			; TID = TID % 8

		mov r0, READY_STAT
		and r0, TMSK
		breq top_of_schedule_loop_schedule

		sei


restore:
		; save current thread and thread mask
		ldi X_HI, hi8(kernel_data + offsetof_kernel_data_struct_cur_thread_id)
		ldi X_LO, lo8(kernel_data + offsetof_kernel_data_struct_cur_thread_id)
		st X, TID
		ldi X_HI, hi8(kernel_data + offsetof_kernel_data_struct_cur_thread_mask)
		ldi X_LO, lo8(kernel_data + offsetof_kernel_data_struct_cur_thread_mask)
		st X, TMSK

		; restore stack pointer
		ldi CTRL_SZ, sizeof_thread_ctrl_struct		; compute offset of current trhead's control table
		mul CTRL_SZ, TID
		movw X_LO, r0
		
		ldi OFFSET_HI, hi8(kernel_data + offsetof_kernel_data_struct_thread_ctrl_tbl + offsetof_thread_ctrl_struct_stack_ptr)
		ldi OFFSET_LO, lo8(kernel_data + offsetof_kernel_data_struct_thread_ctrl_tbl + offsetof_thread_ctrl_struct_stack_ptr)
		
		add X_LO, OFFSET_LO
		adc X_HI, OFFSET_HI
		
		ld SP_LO, X+
		ld SP_HI, X

		out SP_HI_IO_ADDR, SP_HI
		out SP_LO_IO_ADDR, SP_LO

		; restore callee save registers
		pop r29
		pop r28

		pop r17
		pop r16
		pop r15
		pop r14
		pop r13
		pop r12
		pop r11
		pop r10
		pop r9
		pop r8
		pop r7
		pop r6
		pop r5
		pop r4
		pop r3
		pop r2

		ret

sleep_mode:
		sei
		rjmp schedule
#endif