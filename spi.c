#include "spi.h"

#include <avr/io.h>
#include <util/atomic.h>

#define PORT_SPI PORTA
#define DDR_SPI DDRA

#define DD_USCK		DDA4
#define DD_DO		DDA5
#define DD_DI		DDA6

/* Initialize pins for spi communication */
void
spi_init(void)
{
	DDR_SPI |= _BV(DD_USCK);	/* USCK as output */
	DDR_SPI |= _BV(DD_DO);		/* DO as output */
	DDR_SPI &= ~_BV(DD_DI);		/* DI as input */

	// write 0 to USCK and DO outputs
	//PORT_SPI &= ~((1 << DD_DO) | (1 << DD_USCK));
}

/* Shift byte through target device and get one byte */
uint8_t
spi_transfer_byte(uint8_t data)
{
	USIDR = data;

	// clear counter and counter overflow interrupt flag
	USISR = _BV(USIOIF);

	/*
	 * three wire mode
	 * software clock strobe by USITC
	 * make one strobe
	 */
	USICR = (1 << USIWM0) |  (1 << USICLK) | (1 << USICS1) | _BV(USITC);
	while (!(USISR & _BV(USIOIF)))
		USICR |= _BV(USITC);

	return USIDR;
}

void
spi_end(void)
{
	USICR &= ~(_BV(USIWM1) | _BV(USIWM0));
}
