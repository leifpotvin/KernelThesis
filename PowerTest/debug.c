/*
 * debug.c
 *
 * Created: 2/21/2020 11:22:35 AM
 *  Author: potvinnl
 */ 

#include <avr/io.h>

#include "PSerial.h"

void byte_2_str(uint8_t b, char *str)
{
	str[1] = b % 16;
	if (str[1] < 10)
	{
		str[1] += '0';
	}
	else
	{
		str[1] += 'A' - 10;
	}
	str[0] = b / 16;
	if (str[0] < 10)
	{
		str[0] += '0';
	}
	else
	{
		str[0] += 'A' - 10;
	}
}

void print_c(char c)
{
	PSerial_writew(0, c);
}

void print_s(char *str)
{
	while (*str != 0x00)
	{
		print_c(*str);
		++str;
	}
}

void print_b(uint8_t b)
{
	char str[3];
	str[2] = 0x00;
	byte_2_str(b, str);
	print_s(str);
}

void print_4b(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
	char str[3];
	str[2] = 0x00;
	byte_2_str(b3, str);
	print_s(str);
	byte_2_str(b2, str);
	print_s(str);
	byte_2_str(b1, str);
	print_s(str);
	byte_2_str(b0, str);
	print_s(str);
}

void print_addr(void *ptr)
{
	uint16_t p = (uint16_t) ptr;
	char str[5];
	str[4] = 0x00;
	byte_2_str((uint8_t) p % 0xff, &(str[2]));
	p = p / 0xff;
	byte_2_str((uint8_t) p % 0xff, &(str[0]));
	print_s(str);
}

void print_u16(uint16_t num)
{
	char str[6];
	str[5] = 0x0;
	for (int i = 4; i >= 0; --i)
	{
		str[i] = ((num % 10) + '0');
		num /= 10;
	}
	print_s(str);
}

void mem_dump()
{
	PSerial_open(0, 9600, SERIAL_8E2);
	PSerial_readw(0);
	
	#	define REG_FILE (void*)(32)
	#	define IO (void*)(64+32)
	#	define EIO (void*)(416+64+32)
	#	define SRAM (void*)(8192+416+64+32)
	char *mem_part_name[4] = {"REG FILE", "I/O MEM", "EXT. I/O MEM", "SRAM"};
	void* mem_parts[4] = {REG_FILE, IO, EIO, SRAM};
	void *i = (void*) 0x00;
	for (int k = 0; k < 4; ++k)
	{
		print_s("**************\n");
		print_s(mem_part_name[k]);
		print_c('\n');
		print_s("**************\n");
		while (i < mem_parts[k])
		{
			print_addr(i);
			print_s(":    ");
			for (uint8_t j = 0; j < 8; ++j)
			{
				print_b(*(uint8_t *)(i+j));
				print_c(' ');
			}
			print_s(" :  ");
			for (uint8_t j = 0; j < 8; ++j)
			{
				if (*(uint8_t *)(i+j) >= ' ' && *(uint8_t *)(i+j) <= '~')
				{
					print_c(*(uint8_t *)(i+j));
					print_c(' ');
				}
				else
				{
					switch (*(uint8_t *)(i+j))
					{
						case '\n':
						print_s("\\n");
						break;
						case '\r':
						print_s("\\r");
						break;
						case '\0':
						print_s("\\0");
						break;
						default:
						print_s(" .");
					}
				}
			}
			
			print_c('\n');
			i += 8;
		}
	}
}