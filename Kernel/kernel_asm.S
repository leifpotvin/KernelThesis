
/*
 * kernel_asm.S
 *
 * Created: 4/9/2020 11:58:22 AM
 *  Author: Nathan Potvin
 */ 

#include "kernel.h"

		.section .text

		/* Function Defs */
		.global yield
		.global schedule

		.global stack_overflow

		/* Variable Defs */
		.global kernel_data
		.global kernel_data.cur_thread_id


#define X_HI			r27
#define X_LO			r26

#define DISABLE_STAT	r18
#define DELAY_STAT		r19
#define READY_STAT		r18

#define TID				r20
#define TMSK			r21

#define OFFSET_HI		r23
#define OFFSET_LO		r22
#define OFFSET			r22

#define CANARY_REG		r24

yield:
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

		//if (kernel_data.thread_ctrl_tbl[cur_thread_id].canary_ptr != CANARY) rcall stack_overflow();
		; load cur_thread_id
		ldi X_HI, hi8(kernel_data.cur_thread_id)
		ldi X_LO, lo8(kernel_data.cur_thread_id)
		ld TID, X

		ldi X_HI, hi8(kernel_data.thread_ctrl_tbl)
		ldi X_LO, lo8(kernel_data.thread_ctrl_tbl)

		; compute canary_ptr offset
		ldi OFFSET, 8			; load sizeof(thread_ctrl_struct)
		mul OFFSET, TID
		movw OFFSET, r0

		add X_LO, OFFSET_LO		; increment kernel_data.thread_ctrl_tbl by offset to kernel_data.thread_ctrl_tbl[TID]
		adc X_HI, OFFSET_HI

		adiw X_LO, 4			; increment kernel_data.thread_ctrl_tbl[TID] to kernel_data.thread_ctrl_tbl[TID].canary_ptr

		ld CANARY_REG, X
		subi CANARY_REG, CANARY
		breq schedule
		rcall stack_overflow

schedule:
	
		cli

		; load and compute ready stat
		ldi X_HI, hi8(kernel_data.disable_status)
		ldi X_LO, lo8(kernel_data.disable_status)
		ld DISABLE_STAT, X+
		ld DELAY_STAT, X
		or DISABLE_STAT, DELAY_STAT
		com READY_STAT

		tst READY_STAT
		breq sleep_mode

		; load cur_thread_id
		ldi X_HI, hi8(kernel_data.cur_thread_id)
		ldi X_LO, lo8(kernel_data.cur_thread_id)
		ld TID, X

		; load cur_thread_mask
		ldi X_HI, hi8(kernel_data.cur_thread_mask)
		ldi X_LO, lo8(kernel_data.cur_thread_mask)
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