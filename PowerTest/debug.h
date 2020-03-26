/*
 * debug.h
 *
 * Created: 2/21/2020 11:23:08 AM
 *  Author: potvinnl
 */ 

#include <avr/io.h>
#include <util/delay.h>

void byte_2_str(uint8_t b, char *str);

void print_c(char c);

void print_s(char *str);

void print_b(uint8_t b);

void print_4b(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);

void print_addr(void *ptr);

void print_u16(uint16_t num);

void mem_dump();