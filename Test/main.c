/*
 * Test.c
 *
 * Created: 4/4/2020 5:16:01 PM
 * Author : liefp
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

volatile uint8_t sleep;

ISR(TIMER2_COMPA_vect)
{
	sleep ^= 0x01;
	OCR2A = sleep ? 0xff : 0x0f;
}

void s()
{
	//SMCR |= 0b010<<SM0;
	//SMCR |= 0b1<<SE;
	SMCR = 0b00001111;
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

void set_tim_2()
{
	TCCR2A = 0b1 << WGM21;
	TCCR2B = (0b1 << FOC2A) | (0b111 << CS20);
	
	OCR2A = 0xff;
	
	TIMSK2 = 0b1 << OCIE2A;
	
	sei();
}

int main(void)
{
	uint8_t mode = 2;
	
	switch (mode)
	{
		case 0:
			while (1);
			break;
		case 1:
			SMCR = 0b00001111;
			__asm__("sleep");
			break;
		case 2:
			set_tim_2();
			SMCR = 0b00001111;
			while(1)
			{
				if (sleep) __asm__("sleep");
			}
			break;
		case 3:
			DDRB = 0x80;
			for (uint8_t *ptr = (uint8_t *) 0x200; ptr < (uint8_t *) 0x2200; ++ptr)
			{
				*ptr = 0xff;
			}
			while(1)
			{
				_delay_ms(250);
				PORTB ^= 0x80;
			}
	}
}

