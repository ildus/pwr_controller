#include "spi.h"

#include <avr/io.h>
#include <util/atomic.h>

#define PORT_SPI    PORTA
#define DDR_SPI     DDRA

#define DD_SCK      DDA4
#define DD_MISO     DDA5
#define DD_MOSI     DDA6
#define DD_SS       DDA7

/* Initialize pins for spi communication */
void spi_init(int spi_mode)
{
	USICR &= ~(_BV(USISIE) | _BV(USIOIE) | _BV(USIWM1));
    USICR |= _BV(USIWM0) | _BV(USICS1) | _BV(USICLK);
    PORT_SPI |= _BV(DD_SCK);	  //set the USCK pin as output
    PORT_SPI |= _BV(DD_MISO);     //set the MISO pin as output
    PORT_SPI &= ~_BV(DD_MOSI);    //set the MOSI pin as input
    DDR_SPI &= ~((1<<DD_MOSI)|(1<<DD_MISO)|(1<<DD_SS)|(1<<DD_SCK));

	if (spi_mode == SPI_MODE1)
        USICR |= _BV(USICS0);
    else
        USICR &= ~_BV(USICS0);
}

/* Shift byte through target device and get one byte */
uint8_t spi_transfer_byte(uint8_t data)
{
	USIDR = data;
    USISR = _BV(USIOIF);                //clear counter and counter overflow interrupt flag
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //ensure a consistent clock period
        while ( !(USISR & _BV(USIOIF)) ) USICR |= _BV(USITC);
    }
    return USIDR;
}

void spi_end(void)
{
    USICR &= ~(_BV(USIWM1) | _BV(USIWM0));
}
