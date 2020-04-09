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

void f0()
{
	while(1)
	{
		yield();
	}
}

void f1()
{
	while(1)
	{
		yield();
	}
}

void f2()
{
	while(1)
	{
		yield();
	}
}

int main(void)
{
    init();
	
	new(1, f1, true);
	new(2, f2, true);
	new(0, f0, true);
    while (1) 
    {
		x_malloc(10);
    }
}

