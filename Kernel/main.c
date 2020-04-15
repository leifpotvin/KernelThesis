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
		DDRB = 0x80;
		for (int i = 0; i < 20; ++i)
		{
			PORTB |= 0x80;
			delay(200);
			PORTB &= ~(0x80);
			delay(50);
		}
		delay(10000);
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
	
	new(0, f0, true);
    while (1) 
    {
    }
}

