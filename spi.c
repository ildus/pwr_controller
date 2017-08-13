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
		spi_enable();
}

ISR(USI_OVF_vect)
{
	ibuf++;
	if (ibuf == BUF_SIZE)
		ibuf = 0;

	USIDR = spi_buf[ibuf];
	USISR |= _BV(USIOIF);
}

