/*
 * Test.c
 *
 * Created: 4/4/2020 5:16:01 PM
 * Author : liefp
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t t = 0;

ISR(INT0_vect)
{
}

ISR(TIMER0_COMPA_vect)
{
	t = (t + 1) & 0x0f;
	if (!t) PORTB ^= 0x80;
}

ISR(TIMER2_COMPA_vect)
{
// 	t = (t + 1) & 0x0f;
// 	if (!t) PORTB ^= 0x80;
	
	//PORTB ^= 0x80;
}

void idle()
{
	//SMCR |= 0b010<<SM0;
	//SMCR |= 0b1<<SE;
	SMCR = 0b00000101;
	__asm__("sleep");
}

void set_bt_wake()
{
	DDRD &= ~(0x01);
	PORTD |= 0x01;
	
	uint8_t isc0 = 0b10;
	EICRA = (isc0 << ISC00);
	
	EIMSK = (0b1 << INT0) | (0b1 << INT0);
	
	sei();
}

void set_tim_wake0()
{
	TCCR0A = 0b1 << WGM01;
	TCCR0B = (0b1 << FOC0A) | (0b101 << CS00);
	OCR0A = 0xff;
	
	TIMSK0 = 0b1 << OCIE0A;
	
	sei();
}

void set_tim_wake2()
{
	TCCR2A = 0b1 << WGM21;
	TCCR2B = (0b1 << FOC2A) | (0b111 << CS20);
	OCR2A = 0xff;
	
	TIMSK2 = 0b1 << OCIE2A;
	
	sei();
}

int main(void)
{
	set_tim_wake2();	
	
	DDRB = 0x80;
    for (int i = 0; i < 4; ++i)
	{
		PORTB ^= 0x80;
		_delay_ms(1000);
	}
	
	idle();
	
    while (1) 
    {
		PORTB ^= 0x80;
		_delay_ms(250);
    }
}

