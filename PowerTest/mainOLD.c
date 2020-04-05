/*
 * PowerTest.c
 *
 * Created: 3/25/2020 12:53:31 PM
 * Author : liefp
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#include "PSerial.h"
#include "debug.h"

#define MAX_ARG_LEN 12

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

uint16_t module_vec;
void (*sleep_mode)();

volatile bool sleeping;

void read_commands();
void disable_modules();

ISR(INT0_vect)
{
	sleeping = false;
}

ISR(INT1_vect)
{
	module_vec &= ~(USART0);
	disable_modules();
	PSerial_open(0, 9600, SERIAL_8E2);
}

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

void disable_modules()
{
	PRR0 = (uint8_t) (module_vec & 0x00ff);
	PRR1 = (uint8_t) ((module_vec >> 8) & 0x00ff);
}

void (*sleep_modes[7])() = {no_sleep, idle, adcnrm, power_down, power_save, standby, ext_standby};

void read_arg(char *arg)
{
	char c = PSerial_readw(0);
	int i;
	for (i = 0; c != ';' && i < MAX_ARG_LEN; ++i)
	{
		c = PSerial_readw(0);
		if (c != ';') arg[i] = c;
	}
	arg[i-1] = '\0';
}

bool str_comp(char *s1, char *s2, uint8_t n)
{
	int i;
	for (i = 0; i < n && s1[i] != '\0' && s2[i] != '\0'; ++i)
	{
		if (s1[i] != s2[i]) return false;
	}
	return s1[i] == s2[i];
}

void get_disable()
{
	char arg[MAX_ARG_LEN];
	
	read_arg(arg);
	
	if (str_comp(arg, "TIM0", MAX_ARG_LEN))
	{
		print_s("TIM0 disabled.\n\r");
		(module_vec) |= TIM0;
	}
	else if (str_comp(arg, "TIM1", MAX_ARG_LEN))
	{
		print_s("TIM1 disabled.\n\r");
		(module_vec) |= TIM1;
	}
	else if (str_comp(arg, "TIM2", MAX_ARG_LEN))
	{
		print_s("TIM2 disabled.\n\r");
		(module_vec) |= TIM2;
	}
	else if (str_comp(arg, "TIM3", MAX_ARG_LEN))
	{
		print_s("TIM3 disabled.\n\r");
		(module_vec) |= TIM3;
	}
	else if (str_comp(arg, "TIM4", MAX_ARG_LEN))
	{
		print_s("TIM4 disabled.\n\r");
		(module_vec) |= TIM4;
	}
	else if (str_comp(arg, "TIM5", MAX_ARG_LEN))
	{
		print_s("TIM5 disabled.\n\r");
		(module_vec) |= TIM5;
	}
	else if (str_comp(arg, "USART0", MAX_ARG_LEN))
	{
		print_s("USART0 disabled.\n\r");
		(module_vec) |= USART0;
	}
	else if (str_comp(arg, "USART1", MAX_ARG_LEN))
	{
		print_s("USART1 disabled.\n\r");
		(module_vec) |= USART1;
	}
	else if (str_comp(arg, "USART2", MAX_ARG_LEN))
	{
		print_s("USART2 disabled.\n\r");
		(module_vec) |= USART2;
	}
	else if (str_comp(arg, "USART3", MAX_ARG_LEN))
	{
		print_s("USART3 disabled.\n\r");
		(module_vec) |= USART3;
	}
	else if (str_comp(arg, "ADC_", MAX_ARG_LEN))
	{
		print_s("ADC_ disabled.\n\r");
		(module_vec) |= ADC_;
	}
	else if (str_comp(arg, "SPI", MAX_ARG_LEN))
	{
		print_s("SPI disabled.\n\r");
		(module_vec) |= SPI;
	}
	else if (str_comp(arg, "TWI", MAX_ARG_LEN))
	{
		print_s("TWI disabled.\n\r");
		(module_vec) |= TWI;
	}
	else
	{
		print_s("\nInvalid arg. Valid args for d command include: TIM0, TIM1, TIM2, TIM3, TIM4, TIM5, USART0, USART1, USART2, USART3, ADC_, SPI, TWI\n\r");
	}
}

void get_enable()
{
	char arg[MAX_ARG_LEN];
	
	read_arg(arg);
	
	if (str_comp(arg, "TIM0", MAX_ARG_LEN))
	{
		print_s("TIM0 enabled.\n\r");
		(module_vec) &= ~TIM0;
	}
	else if (str_comp(arg, "TIM1", MAX_ARG_LEN))
	{
		print_s("TIM1 enabled.\n\r");
		(module_vec) &= ~TIM1;
	}
	else if (str_comp(arg, "TIM2", MAX_ARG_LEN))
	{
		print_s("TIM2 enabled.\n\r");
		(module_vec) &= ~TIM2;
	}
	else if (str_comp(arg, "TIM3", MAX_ARG_LEN))
	{
		print_s("TIM3 enabled.\n\r");
		(module_vec) &= ~TIM3;
	}
	else if (str_comp(arg, "TIM4", MAX_ARG_LEN))
	{
		print_s("TIM4 enabled.\n\r");
		(module_vec) &= ~TIM4;
	}
	else if (str_comp(arg, "TIM5", MAX_ARG_LEN))
	{
		print_s("TIM5 enabled.\n\r");
		(module_vec) &= ~TIM5;
	}
	else if (str_comp(arg, "USART0", MAX_ARG_LEN))
	{
		print_s("USART0 enabled.\n\r");
		(module_vec) &= ~USART0;
	}
	else if (str_comp(arg, "USART1", MAX_ARG_LEN))
	{
		print_s("USART1 enabled.\n\r");
		(module_vec) &= ~USART1;
	}
	else if (str_comp(arg, "USART2", MAX_ARG_LEN))
	{
		print_s("USART2 enabled.\n\r");
		(module_vec) &= ~USART2;
	}
	else if (str_comp(arg, "USART3", MAX_ARG_LEN))
	{
		print_s("USART3 enabled.\n\r");
		(module_vec) &= ~USART3;
	}
	else if (str_comp(arg, "ADC_", MAX_ARG_LEN))
	{
		print_s("ADC_ enabled.\n\r");
		(module_vec) &= ~ADC_;
	}
	else if (str_comp(arg, "SPI", MAX_ARG_LEN))
	{
		print_s("SPI enabled.\n\r");
		(module_vec) &= ~SPI;
	}
	else if (str_comp(arg, "TWI", MAX_ARG_LEN))
	{
		print_s("TWI enabled.\n\r");
		(module_vec) &= ~TWI;
	}
	else
	{
		print_s("\nInvalid arg. Valid args for e command include: TIM0, TIM1, TIM2, TIM3, TIM4, TIM5, USART0, USART1, USART2, USART3, ADC_, SPI, TWI\n\r");
	}
}

void get_sleep_mode()
{
	char arg[MAX_ARG_LEN];
	
	read_arg(arg);
	
	if (str_comp(arg, "no_sleep", MAX_ARG_LEN))
	{
		print_s("Set no_sleep mode\n\r");
		sleep_mode = no_sleep;
	}
	else if (str_comp(arg, "idle", MAX_ARG_LEN))
	{
		print_s("Set idle mode\n\r");
		sleep_mode = idle;
	}
	else if (str_comp(arg, "adcnrm", MAX_ARG_LEN))
	{
		print_s("Set adcnrm mode\n\r");
		sleep_mode = adcnrm;
	}
	else if (str_comp(arg, "power_down", MAX_ARG_LEN))
	{
		print_s("Set power_down mode\n\r");
		sleep_mode = power_down;
	}
	else if (str_comp(arg, "power_save", MAX_ARG_LEN))
	{
		print_s("Set power_save mode\n\r");
		sleep_mode = power_save;
	}
	else if (str_comp(arg, "standby", MAX_ARG_LEN))
	{
		print_s("Set standby mode\n\r");
		sleep_mode = standby;
	}
	else if (str_comp(arg, "ext_standby", MAX_ARG_LEN))
	{
		print_s("Set ext_standby mode\n\r");
		sleep_mode = ext_standby;
	}
	else
	{
		print_s("\n\rInvalid arg. Valid args for s command include: no_sleep, idle, adcnrm, power_down, power_save, standby, ext_standby\n\r");
	}
}

void ls_cmd()
{
	print_s("Commands: \n\r");
	print_s("e [module];     :    enables a module.\n\r");
	print_s("d [module];     :    disables a module.\n\r");
	print_s("s [sleep mode]; :    sets a sleep mode to be used.\n\r");
	print_s("b               :    enters given sleep mode with disabled modules.\n\r");
	
	print_s("Modules: TIM0, TIM1, TIM2, TIM3, TIM4, TIM5, USART0, USART1, USART2, USART3, ADC_, SPI, TWI\n\r");
	print_s("Sleep modes: no_sleep, idle, adcnrm, power_down, power_save, standby, ext_standby\n\r");
}

void ls()
{
	print_s("Modules:");
	
	print_s("\n\r    TIM0    :    ");
	module_vec & TIM0 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM1    :    ");
	module_vec & TIM1 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM2    :    ");
	module_vec & TIM2 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM3    :    ");
	module_vec & TIM3 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM4    :    ");
	module_vec & TIM4 ? print_c('d') : print_c('e');
	print_s("\n\r    TIM5    :    ");
	module_vec & TIM5 ? print_c('d') : print_c('e');
	print_s("\n\r    USART0  :    ");
	module_vec & USART0 ? print_c('d') : print_c('e');
	print_s("\n\r    USART1  :    ");
	module_vec & USART1 ? print_c('d') : print_c('e');
	print_s("\n\r    USART2  :    ");
	module_vec & USART2 ? print_c('d') : print_c('e');
	print_s("\n\r    USART3  :    ");
	module_vec & USART3 ? print_c('d') : print_c('e');
	print_s("\n\r    ADC_    :    ");
	module_vec & ADC_ ? print_c('d') : print_c('e');
	print_s("\n\r    SPI     :    ");
	module_vec & SPI ? print_c('d') : print_c('e');
	print_s("\n\r    TWI     :    ");
	module_vec & TWI ? print_c('d') : print_c('e');
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

void start_sleep()
{	
	print_s("Beginning sleep:\n\r");
	
	ls();
	
	disable_modules();
	
	sleeping = true;
	sleep_mode();
}

void proc_command()
{
	char command = PSerial_readw(0);
	switch (command)
	{
		case 'd':
			get_disable();
			break;
		case 'e':
			get_enable();
			break;
		case 's':
			get_sleep_mode(sleep_mode);
			break;
		case 'c':
			ls_cmd();
			break;
		case 'l':
			ls();
			break;
		case 'b':
			start_sleep();
			break;
		default:
			print_s("Unknown command: ");
			print_c(command);
			print_s("\n\r");
			break;
	}
}

void read_commands()
{
	while (1)
	{
		proc_command();
	}
}

int main(void)
{
	sleeping = false;
	
	DDRD &= ~(0x01);
	PORTD |= 0x01;
	
	uint8_t isc0 = 0b10;
	EICRA = (isc0 << ISC00);
	
	EIMSK = (0b1 << INT0) | (0b1 << INT0);
	
	sei();
	
    PSerial_open(0, 9600, SERIAL_8E2);
	
	module_vec = 0x0000;
	sleep_mode = no_sleep;
	
	ls_cmd();
	
    read_commands();
}

