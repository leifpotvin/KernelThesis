/*
 * main2.c
 *
 * Created: 4/4/2020 6:34:41 PM
 *  Author: Nathan Potvin
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "PSerial.h"
#include "debug.h"

#define TIM5 0x2000
#define TIM4 0x1000
#define TIM3 0x0800
#define TIM2 0x0040
#define TIM1 0x0008
#define TIM0 0x0020

#define USART3 0x0400
#define USART2 0x0200
#define USART1 0x0100
#define USART0 0x0002

#define ADC_ 0x0001

#define SPI 0x0004

#define TWI 0x0080

volatile bool sleeping;

/*
 *	end the sleep on the button press
 */
ISR(INT0_vect)
{
	sleeping = false;
}

/*
 *	fucntions to execute each sleep mode
 */
void no_sleep()
{
	while(sleeping);
}

void idle()
{
	SMCR |= 0b000<<SM0;
	SMCR |= 0b1<<SE;
	__asm__("sleep");
}

void adcnrm()
{
	SMCR |= 0b001<<SM0;
	SMCR |= 0b1<<SE;
	__asm__("sleep");
}

void power_down()
{
	SMCR |= 0b010<<SM0;
	SMCR |= 0b1<<SE;
	__asm__("sleep");
}

void power_save()
{
	SMCR |= 0b011<<SM0;
	SMCR |= 0b1<<SE;
	__asm__("sleep");
}

void standby()
{
	SMCR |= 0b110<<SM0;
	SMCR |= 0b1<<SE;
	__asm__("sleep");
}

void ext_standby()
{
	SMCR |= 0b111<<SM0;
	SMCR |= 0b1<<SE;
	__asm__("sleep");
}

// array of the sleep modes
void (*sleep_modes[7])() = {no_sleep, idle, adcnrm, power_down
						  , power_save, standby, ext_standby};
uint16_t modules[14] = {0x0000, TIM0, TIM1, TIM2, TIM3, TIM4, TIM5
					  , USART0, USART1, USART2, USART3, ADC_, SPI, TWI};

// array of modules that can be disabled for power savings 
// with the PRR register
void (*sleep_mode)();
uint16_t module_disable_vect;

/*
 *	disable modules by setting PRR according to the module_disable_vect
 */
void disable_modules()
{
	PRR0 = (uint8_t) (module_disable_vect & 0x00ff);
	PRR1 = (uint8_t) ((module_disable_vect >> 8) & 0x00ff);
}

/*
 *	enable all modules in the PRR register
 */
void enable_modules()
{
	PRR0 = 0x00;
	PRR1 = 0x00;
}

/*
 *	reset the state
 */
void reset()
{
	PSerial_open(0, 9600, SERIAL_8E2);
	sleep_mode = no_sleep;
	module_disable_vect = 0x0000;
	enable_modules();
	
	DDRD &= ~(0x01);
	PORTD |= 0x01;
	
	uint8_t isc0 = 0b10;
	EICRA = (isc0 << ISC00);
	
	EIMSK = (0b1 << INT0) | (0b1 << INT0);
	
	sei();
}

/*
 *	print the current sleep configuration to the terminal
 */
void print_state()
{
	print_s("Modules:");
	
	print_s("\n\r    TIM0    :    ");
	module_disable_vect & TIM0 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM1    :    ");
	module_disable_vect & TIM1 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM2    :    ");
	module_disable_vect & TIM2 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM3    :    ");
	module_disable_vect & TIM3 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM4    :    ");
	module_disable_vect & TIM4 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM5    :    ");
	module_disable_vect & TIM5 ? print_c('d') : print_c('e');
	print_s("\n\r    USART0  :    ");
	module_disable_vect & USART0 ? print_c('d') : print_c('e');
	print_s("\n\r    USART1  :    ");
	module_disable_vect & USART1 ? print_c('d') : print_c('e');
	print_s("\n\r    USART2  :    ");
	module_disable_vect & USART2 ? print_c('d') : print_c('e');
	print_s("\n\r    USART3  :    ");
	module_disable_vect & USART3 ? print_c('d') : print_c('e');
	print_s("\n\r    ADC_    :    ");
	module_disable_vect & ADC_ ? print_c('d') : print_c('e');
	print_s("\n\r    SPI     :    ");
	module_disable_vect & SPI ? print_c('d') : print_c('e');
	print_s("\n\r    TWI     :    ");
	module_disable_vect & TWI ? print_c('d') : print_c('e');
	print_s("\n\r");
	
	print_s("Sleep mode:\n\r");
	if (sleep_mode == no_sleep)
	{
		print_s("    no_sleep\n\r");
	}
	else if (sleep_mode == idle)
	{
		print_s("    idle\n\r");
	}
	else if (sleep_mode == adcnrm)
	{
		print_s("    adcnrm\n\r");
	}
	else if (sleep_mode == power_down)
	{
		print_s("    power_down\n\r");
	}
	else if (sleep_mode == power_save)
	{
		print_s("    power_save\n\r");
	}
	else if ( sleep_mode == standby)
	{
		print_s("    standby\n\r");
	}
	else if (sleep_mode == ext_standby)
	{
		print_s("    ext_standby\n\r");
	}
	else
	{
		print_s("    Not a valid sleep_mode\n\r");
	}
	
	while (!(UCSR0A & (1<<TXC0)));
}


int main()
{
	for (int i = 0; i < 7; ++i)
	{
		for (int j = 0; j < 14; ++j)
		{
			reset();
			sleep_mode = sleep_modes[i];
			module_disable_vect = modules[j];
			print_state();
			disable_modules();
			sleeping = true;
			sleep_mode();
		}
		
		reset();
		sleep_mode = sleep_modes[i];
		module_disable_vect = modules[1] | modules[2] | modules[3] 
						    | modules[4] | modules[5] | modules[6] 
							| modules[7] | modules[8] | modules[9] 
							| modules[10] | modules[11] | modules[12] 
							| modules[13];
		print_state();
		disable_modules();
		sleeping = true;
		sleep_mode();
		
		reset();
		sleep_mode = sleep_modes[i];
		module_disable_vect = modules[1] | modules[2] | modules[3] | modules[4] 
							| modules[5] | modules[6];
		print_state();
		disable_modules();
		sleeping = true;
		sleep_mode();
		
		reset();
		sleep_mode = sleep_modes[i];
		module_disable_vect = modules[7] | modules[8] | modules[9] | modules[10];
		print_state();
		disable_modules();
		sleeping = true;
		sleep_mode();
	}
}