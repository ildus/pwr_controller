#include "spi.h"

#include <avr/io.h>
#include <util/atomic.h>

#define PORT_SPI PORTA
#define DDR_SPI DDRA

#define DD_USCK		DDA4
#define DD_DO		DDA5
#define DD_DI		DDA6

uint8_t data_to_send = 10,
	cmd;

ISR(USI_OVF_vect) {
	cmd = USIDR;
	USIDR = 0xFF;
	USISR |= _BV(USIOIF);
}

/* Initialize pins for spi communication */
void
spi_init(bool is_master)
{
	DDR_SPI |= _BV(DD_DO);		/* DO as output */

	if (!is_master)
	{
		/*
		 * three wire mode
		 * external clock
		 * enable interrupt
		 */
		USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USIOIE);
		USISR = _BV(USIOIF);
		USIDR = 0xFF;

		// set as inputs 
		DDR_SPI &= ~(_BV(DD_USCK) | _BV(DD_DI));
 		// set pullup
		PORT_SPI |= _BV(DD_USCK) | _BV(DD_DI);
	}
	else
	{
		DDR_SPI &= ~_BV(DD_DI);		/* DI as input */
		DDR_SPI |= _BV(DD_USCK);	/* USCK as output */
	}
		

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
