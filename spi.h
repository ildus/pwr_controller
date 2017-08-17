#ifndef _SPI_H_
#define _SPI_H_

#include <avr/io.h>

#define PORT_SPI PORTA
#define DDR_SPI DDRA

#define DD_USCK		PA4
#define DD_DO		PA5
#define DD_DI		PA6

#define PORT_SS		PORTB
#define DDR_SS		DDRB
#define DD_SS		PB2
#define PIN_SS		PINB

#endif
