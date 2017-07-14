#include "avr/io.h"
#include "avr/interrupt.h"
#include <util/delay.h>

#include "spi.h"
#include "timer.h"

/*
 * internal 1.1V voltage reference, by datasheet
 * external capacitor should be connected to Aref
 * calibrated for actual chip, i got 1.197V
 */
const double INTERNAL_VREF = 1.197;

#define ADC0 0b0000
#define ADC1 0b0001
#define ADC2 0b0010
#define ADC3 0b0011
#define ADC4 0b0100
#define ADC5 0b0101
#define ADC6 0b0110
#define ADC7 0b0111

#define DDR_LED DDRB
#define PORT_LED PORTB
#define LED_RED PB1
#define LED_GREEN PB0

void setup_led(void)
{
	DDR_LED |= _BV(LED_RED) | _BV(LED_GREEN);
	PORT_LED &= ~(_BV(LED_RED) | _BV(LED_GREEN));
}

void
adc_init(void)
{
	// power ADC
	ADCSRA &= ~_BV(ADEN);
	PRR &= ~_BV(PRADC);

	// enable adc, prescaler = 128
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

	// internal Vref
	ADMUX = _BV(REFS1);
}

uint16_t
adc_read(uint8_t pin)
{
	// select the corresponding channel 0~7
	// ANDing with ’7′ will always keep the value
	// of ‘ch’ between 0 and 7
	pin &= 0b00000111;			  // AND operation with 7
	ADMUX = (ADMUX & 0xF8) | pin; // clears the bottom 3 bits before ORing

	// start single convertion
	ADCSRA |= _BV(ADSC);

	while (ADCSRA & _BV(ADSC))
	{
		/*
		 * wait for conversion to complete,
		 * ADSC becomes ’0′ again
		 */
	}
	return (ADC);
}

uint16_t
read_voltage(uint8_t pin)
{
	int		sum = 0;

	for (int i = 0; i < 9; i++)
		sum += adc_read(pin);

	return (uint16_t) (sum / 10.0);
	//v = (sum / 10) * INTERNAL_VREF / 1024;
	//return (uint8_t) (v * 100);
}

int
main(void)
{
	unsigned long old_millis = 0;
	setup_led();
	PORT_LED |= _BV(LED_GREEN);

	// spi_init(false);
	// adc_init();
	while (1) {
		unsigned long current_time = millis();
		if (current_time - old_millis > 3000)
		{
			PORT_LED ^=  _BV(LED_RED);
			old_millis = current_time;
		}
		// uint16_t volt = read_voltage(ADC1);
		// spi_transfer_byte_as_slave('v');
		// spi_transfer_byte_as_slave((uint8_t)(volt & 0x00FF));
		// spi_transfer_byte_as_slave((uint8_t)(volt >> 8));
		_delay_ms(100);
	}
	//spi_end();
}
