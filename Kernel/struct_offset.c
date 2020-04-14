/*
 * struct_offset.c
 *
 * Created: 4/9/2020 7:04:39 PM
 *  Author: Nathan Potvin
 */ 

#ifdef __STRUCT_OFFSETS__

#include "kernel.h"

#define _ASMDEFINE(sym, val) asm volatile ("\n- " #sym " %0 \n" : : "i" (val))

#define ASMDEFINE_OFFSETOF(s, m) _ASMDEFINE(offsetof_##s##_##m, offsetof(s, m));

#define ASMDEFINE_SIZEOF(s) _ASMDEFINE(sizeof_##s, sizeof(s));

void kernel_data_struct_defineOffsets()
{
	ASMDEFINE_OFFSETOF(kernel_data_struct, stacks);
	ASMDEFINE_OFFSETOF(kernel_data_struct, thread_ctrl_tbl);
	ASMDEFINE_OFFSETOF(kernel_data_struct, disable_status);
	ASMDEFINE_OFFSETOF(kernel_data_struct, delay_status);
	ASMDEFINE_OFFSETOF(kernel_data_struct, delay_ctrs);
	ASMDEFINE_OFFSETOF(kernel_data_struct, cur_thread_id);
	ASMDEFINE_OFFSETOF(kernel_data_struct, cur_thread_mask);
	ASMDEFINE_OFFSETOF(kernel_data_struct, system_time_millis);
	
	ASMDEFINE_OFFSETOF(thread_ctrl_struct, stack_ptr);
	ASMDEFINE_OFFSETOF(thread_ctrl_struct, stack_base);
	ASMDEFINE_OFFSETOF(thread_ctrl_struct, canary_ptr);
	ASMDEFINE_OFFSETOF(thread_ctrl_struct, entry_pnt);
	
	ASMDEFINE_SIZEOF(kernel_data_struct);
	ASMDEFINE_SIZEOF(stack_struct);
	ASMDEFINE_SIZEOF(thread_ctrl_struct);
}

#endif	/* __STRUCT_OFFSETS__ */