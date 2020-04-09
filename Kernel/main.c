/*
 * Kernel.c
 *
 * Created: 4/7/2020 10:21:29 AM
 * Author : Nathan Potvin
 */ 

#include <avr/io.h>

#include "kernel.h"

void f0();
void f1();
void f2();

int main(void)
{
    init();
	kernel_data.delay_status = 0x11;
	kernel_data.delay_ctrs[0] = 4;
	kernel_data.delay_ctrs[4] = 3;
	kernel_data.delay_ctrs[1] = 5;
    while (1) 
    {
		x_malloc(10);
    }
}

