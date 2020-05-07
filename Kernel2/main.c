/*
 * Kernel2.c
 *
 * Created: 4/17/2020 3:06:45 PM
 * Author : Nathan Potvin
 */ 

#include <avr/io.h>

#include <util/delay.h>
#include <stdbool.h>

#include "kernel.h"


void t0()
{
	while(1)
	{
		
	}
}

void t1()
{
	while(1)
	{
		delay(5);
	}
}

void t2()
{
	while(1)
	{

	}
}

int main(void)
{
	init();
// 	new(7, t0, true);
// 	new(6, t0, true);
// 	new(5, t0, true);
// 	new(4, t0, true);
// 	new(3, t0, true);
// 	new(2, t0, true);
// 	new(1, t0, true);
	new(0, t0, true);
}

