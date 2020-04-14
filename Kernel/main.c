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
		delay(3);
	}
}

void f1()
{
	while(1)
	{
		delay(5);
	}
}

void f2()
{
	while(1)
	{
		delay(7);
	}
}

int main(void)
{
    init();
	
	new(1, f1, true);
	new(2, f2, true);
	new(0, f0, true);
	
	yield();
    while (1) 
    {
    }
}

