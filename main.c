#include "avr/io.h"
#include "spi.h"
#include <util/delay.h>

const double INTERNAL_VREF = 1.1; /* internal 1.1V voltage reference, by datasheet
									external capacitor should be connected to Aref*/

#define ADC0 0b0000
#define ADC1 0b0001
#define ADC2 0b0010
#define ADC3 0b0011
#define ADC4 0b0100
#define ADC5 0b0101
#define ADC6 0b0110
#define ADC7 0b0111

void adc_init(void)
{
	// enable adc, prescaler = 128
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

	// internal Vref with external capacitor
	// enable adc pin
	ADMUX = _BV(REFS1) | _BV(REFS0);
}

uint16_t adc_read(uint8_t pin)
{
	// select the corresponding channel 0~7
	// ANDing with ’7′ will always keep the value
	// of ‘ch’ between 0 and 7
	pin &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8) | pin; // clears the bottom 3 bits before ORing

	// start single convertion
	// write ’1′ to ADSC
	ADCSRA |= _BV(ADSC);

	// wait for conversion to complete
	// ADSC becomes ’0′ again
	// till then, run loop continuously
	while (ADCSRA & _BV(ADSC));
	return (ADC);
}

uint8_t read_voltage(uint8_t pin)
{
	int sum = 0;
	for (int i = 0; i < 9; i++)
		sum += adc_read(pin);

	return (uint8_t) (sum  * INTERNAL_VREF / 1024);
}

int main(void)
{
	spi_init(SPI_MODE0);
	adc_init();
	while (1)
	{
		uint8_t volt = read_voltage(ADC1);
		spi_transfer_byte('v');
		spi_transfer_byte(volt);
		_delay_ms(20);
	}
	spi_end();
}
