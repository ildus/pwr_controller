#include "spi.h"

#include <avr/io.h>
#include <util/atomic.h>

#define PORT_SPI PORTA
#define DDR_SPI DDRA

#define DD_USCK		PA4
#define DD_DO		PA5
#define DD_DI		PA6

volatile uint8_t	data_to_send = 0,
			cmd;

ISR(USI_OVF_vect)
{
	cmd = USIDR;
	USIDR = 0xAA;
	USISR |= _BV(USIOIF);
}

/* Initialize pins for spi communication */
void
spi_init(bool is_master)
{

	if (!is_master)
	{
		sei();

		DDR_SPI |= _BV(DD_DO);				/* DO as output */
		DDR_SPI &= ~(_BV(DD_DI) | _BV(DD_USCK));	/* DI and SCK as inputs */
		PORT_SPI |= _BV(DD_DI) | _BV(DD_USCK);		/* enable pullups */

		/*
		 * three wire mode
		 * external clock, both edges
		 * enable overflow interrupt
		 */

		USIDR = 0;
		USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USIOIE);
		USISR = _BV(USIOIF);
	}
	else
	{
		DDR_SPI |= _BV(DD_DO) | _BV(DD_USCK);		/* DO and USCK as output */
		DDR_SPI &= ~_BV(DD_DI);				/* DI as input */
	}
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

uint8_t
spi_transfer_byte_as_slave(uint8_t data)
{
	data_to_send = data;
	return cmd;
}

void
spi_end(void)
{
	USICR &= ~(_BV(USIWM1) | _BV(USIWM0));
}
