#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"

volatile int		ibuf = 0;
uint8_t				spi_buf[BUF_SIZE];

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
		USIDR = spi_buf[ibuf];
		USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USIOIE);
		USISR = _BV(USIOIF);
	}
}

ISR(USI_OVF_vect)
{
	ibuf++;
	if (ibuf == BUF_SIZE)
		ibuf = 0;

	USIDR = spi_buf[ibuf];
	USISR |= _BV(USIOIF);
}

