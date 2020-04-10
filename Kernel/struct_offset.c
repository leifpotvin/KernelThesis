/*
 * struct_offset.c
 *
 * Created: 4/9/2020 7:04:39 PM
 *  Author: Nathan Potvin
 */ 

#ifdef __STRUCT_OFFSETS__

#include "kernel.h"

#define _ASMDEFINE(sym, val) asm volatile ("\n- " #sym " %0 \n" : : "i" (val))

#define ASMDEFINE(s, m) _ASMDEFINE(offsetof_##s##_##m, offsetof(s, m));

void kernel_data_struct_defineOffsets()
{
	ASMDEFINE(kernel_data_struct, stacks);
	ASMDEFINE(kernel_data_struct, thread_ctrl_tbl);
	ASMDEFINE(kernel_data_struct, disable_status);
	ASMDEFINE(kernel_data_struct, delay_status);
	ASMDEFINE(kernel_data_struct, delay_ctrs);
	ASMDEFINE(kernel_data_struct, cur_thread_id);
	ASMDEFINE(kernel_data_struct, cur_thread_mask);
	ASMDEFINE(kernel_data_struct, system_time_millis);
}

#endif	/* __STRUCT_OFFSETS__ */