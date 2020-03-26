/*
 * PSerial.h
 *
 * Created: 2/9/2020 2:11:41 PM
 * Author: Nathan Potvin
 * Author: Matthew Glancy
 */

#include <stdint.h>

#ifndef PSERIAL_H_
#define PSERIAL_H_

#define DATABITS 1
#define STOPBITS 3
#define PARITYBITS 4

#define SERIAL_5N1  (0x00 | (0 << DATABITS))
#define SERIAL_6N1  (0x00 | (1 << DATABITS))
#define SERIAL_7N1  (0x00 | (2 << DATABITS))
#define SERIAL_8N1  (0x00 | (3 << DATABITS))     // (the default)
#define SERIAL_5N2  (0x08 | (0 << DATABITS))
#define SERIAL_6N2  (0x08 | (1 << DATABITS))
#define SERIAL_7N2  (0x08 | (2 << DATABITS))
#define SERIAL_8N2  (0x08 | (3 << DATABITS))
#define SERIAL_5E1  (0x20 | (0 << DATABITS))
#define SERIAL_6E1  (0x20 | (1 << DATABITS))
#define SERIAL_7E1  (0x20 | (2 << DATABITS))
#define SERIAL_8E1  (0x20 | (3 << DATABITS))
#define SERIAL_5E2  (0x28 | (0 << DATABITS))
#define SERIAL_6E2  (0x28 | (1 << DATABITS))
#define SERIAL_7E2  (0x28 | (2 << DATABITS))
#define SERIAL_8E2  (0x28 | (3 << DATABITS))
#define SERIAL_5O1  (0x30 | (0 << DATABITS))
#define SERIAL_6O1  (0x30 | (1 << DATABITS))
#define SERIAL_7O1  (0x30 | (2 << DATABITS))
#define SERIAL_8O1  (0x30 | (3 << DATABITS))
#define SERIAL_5O2  (0x38 | (0 << DATABITS))
#define SERIAL_6O2  (0x38 | (1 << DATABITS))
#define SERIAL_7O2  (0x38 | (2 << DATABITS))
#define SERIAL_8O2  (0x38 | (3 << DATABITS))

#define UART_0 (UART_PORT *) 0x0C0;
#define UART_1 (UART_PORT *) 0x0C8;
#define UART_2 (UART_PORT *) 0x0D0;
#define UART_3 (UART_PORT *) 0x130;

typedef struct 
{
	uint8_t UCSRnA;
	uint8_t UCSRnB;
	uint8_t UCSRnC;
	uint8_t reserved;
	uint16_t UBRRn;
	uint8_t UDRn;
} volatile UART_PORT;

void PSerial_open(uint8_t port, long speed, int framing);
int PSerial_read(uint8_t port);
char PSerial_readw(uint8_t port);
int PSerial_write(uint8_t port, uint8_t data);
void PSerial_writew(uint8_t port, uint8_t data);

#endif /* PSERIAL_H_ */