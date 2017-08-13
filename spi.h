#ifndef _SPI_H_
#define _SPI_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#define PORT_SPI PORTA
#define DDR_SPI DDRA

#define DD_USCK		PA4
#define DD_DO		PA5
#define DD_DI		PA6

#define PORT_SS		PORTB
#define DDR_SS		DDRB
#define DD_SS		PB2
#define PIN_SS		PINB

#define BUF_SIZE	4
#define spi_enabled (USICR & _BV(USIOIE))

extern uint8_t			spi_buf[BUF_SIZE];
extern volatile int		ibuf;

static inline void
spi_end(void)
{
	if (!spi_enabled)
		return;

	USICR &= ~(_BV(USIWM0) | _BV(USICS1) | _BV(USIOIE));
}

static inline void
spi_transfer_data(uint8_t high, uint8_t low)
{
	spi_buf[0] = 'l';
	spi_buf[1] = low;
	spi_buf[2] = high;
	spi_buf[3] = high ^ low;
}

/* Initialize pins for spi communication */
static inline void
spi_init()
{
	/* SS as input */
	DDR_SS &= ~_BV(DD_SS);
	PORT_SS |= _BV(DD_SS);

	/* any logical change causes interrupt */
	MCUCR |= _BV(ISC00);

	DDR_SPI |= _BV(DD_DO);				/* DO as output */
	DDR_SPI &= ~(_BV(DD_DI) | _BV(DD_USCK));	/* DI and SCK as inputs */
	PORT_SPI |= _BV(DD_DI) | _BV(DD_USCK);		/* enable pullups */

	/* enable interrupt */
	GIMSK |= _BV(INT0);
}

static inline void
spi_enable()
{
	/*
	 * three wire mode
	 * external clock, both edges
	 * enable overflow interrupt
	 */

	if (spi_enabled)
		return;

	ibuf = 0;
	USIDR = spi_buf[ibuf];
	USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USIOIE);
	USISR = _BV(USIOIF);
}

#endif
