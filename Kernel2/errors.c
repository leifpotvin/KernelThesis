/*
 * errors.c
 *
 * Created: 4/19/2020 11:14:29 PM
 *  Author: Nathan Potvin
 */ 

#include <util/delay.h>

#include "kernel.h"

/*
 *	Error handler for stack overflow.
 *
 *	Current version indicates the error externally and never returns.
 */
void stack_overflow()
{
	DDRB |= 0x80;
	while (1)
	{
		PORTB ^= 0x80;
		_delay_ms(250);
	}
}

/*
 *	Error handler for initialized thread. Indicates that the scheduler attempted 
 *	to schedule a thread that has not been initialized.
 *	Current version indicates the error externally and never returns.
 */
void uninitialized_thread_error()
{
	DDRB |= 0x80;
	while (1)
	{
		PORTB ^= 0x80;
		_delay_ms(1000);
	}
}