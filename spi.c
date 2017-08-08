#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"

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

volatile uint8_t	cmd;
volatile int		ibuf = 0;
uint8_t				buf[BUF_SIZE];

ISR(EXT_INT0_vect)
{
	if (PIN_SS & DD_SS)
		spi_end();
	else
	{
		/*
		 * three wire mode
		 * external clock, both edges
		 * enable overflow interrupt
		 */

		ibuf = 0;
		USIDR = buf[ibuf];
		USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USIOIE);
		USISR = _BV(USIOIF);
	}
}

ISR(USI_OVF_vect)
{
	ibuf++;
	if (ibuf == BUF_SIZE)
		ibuf = 0;

	cmd = USIDR;
	USIDR = buf[ibuf];
	USISR |= _BV(USIOIF);
}

/* Initialize pins for spi communication */
void
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

	/* enable INT0 */
	GIMSK |= _BV(INT0);
}

uint8_t
spi_transfer_data(uint8_t *data)
{
	for (int i = 0; i < BUF_SIZE; i++)
		buf[i] = data[i];

	return cmd;
}

void
spi_end(void)
{
	//disable SPI
	USICR &= ~(_BV(USIWM0) | _BV(USICS1) | _BV(USIOIE));
}
