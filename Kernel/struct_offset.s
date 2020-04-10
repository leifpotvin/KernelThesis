	.file	"struct_offset.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.comm	kernel_data,344,1
	.text
.global	kernel_data_struct_defineOffsets
	.type	kernel_data_struct_defineOffsets, @function
kernel_data_struct_defineOffsets:
	push r28
	push r29
	in r28,__SP_L__
	in r29,__SP_H__
/* prologue: function */
/* frame size = 0 */
/* stack size = 2 */
.L__stack_usage = 2
/* #APP */
 ;  16 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_stacks 0 

 ;  0 "" 2
 ;  17 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_thread_ctrl_tbl 256 

 ;  0 "" 2
 ;  18 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_disable_status 320 

 ;  0 "" 2
 ;  19 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_delay_status 321 

 ;  0 "" 2
 ;  20 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_delay_ctrs 322 

 ;  0 "" 2
 ;  21 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_cur_thread_id 338 

 ;  0 "" 2
 ;  22 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_cur_thread_mask 339 

 ;  0 "" 2
 ;  23 "struct_offset.c" 1
	
-> offsetof_kernel_data_struct_system_time_millis 340 

 ;  0 "" 2
/* #NOAPP */
	nop
/* epilogue start */
	pop r29
	pop r28
	ret
	.size	kernel_data_struct_defineOffsets, .-kernel_data_struct_defineOffsets
	.ident	"GCC: (GNU) 5.4.0"
.global __do_clear_bss
