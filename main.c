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

#define DDR_PWR DDRA
#define PORT_PWR PORTA
#define PIN_PWR PA3

void setup_led(void)
{
	DDR_LED |= _BV(LED_RED) | _BV(LED_GREEN);
	PORT_LED &= ~(_BV(LED_RED) | _BV(LED_GREEN));
}

void enable_power(bool enable)
{
	DDR_PWR |= _BV(PIN_PWR);
	if (enable)
		PORT_PWR |= _BV(PIN_PWR);
	else
		PORT_PWR &= ~_BV(PIN_PWR);
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

void setup_btn(void)
{
	DDRA &= ~_BV(PA1);
	PORTA &= ~_BV(PA1);
}

typedef struct ButtonState {
	bool clicked;
	unsigned long last_click;
	char old_level;
} ButtonState;

void check_button_state(ButtonState *state)
{
	char level = PINA & _BV(PA1);
	if (level && state->old_level != level)
	{
		if (millis() - state->last_click > 100) {
			state->clicked = true;
		}
		state->last_click = millis();
	}
	state->old_level = level;
}

int
main(void)
{
	unsigned long	click_time = 0;
	ButtonState		btn_state;
	bool			pwr_on = false;

	sei();
	setup_led();
	setup_timer0();
	setup_btn();
	enable_power(pwr_on);
	//PORT_LED |= _BV(LED_GREEN);
	//PORT_LED |= _BV(LED_RED);

	//spi_init(false);
	//adc_init();
	while (1) {
		check_button_state(&btn_state);
		if (!btn_state.old_level)
			click_time = 0;

		if (!pwr_on && btn_state.clicked)
		{
			btn_state.clicked = false;
			pwr_on = true;
			enable_power(pwr_on);
			PORT_LED |= _BV(LED_GREEN);
		}
		else if (pwr_on && btn_state.clicked)
		{
			if (click_time && millis() - click_time > 3000)
			{
				btn_state.clicked = false;
				pwr_on = false;
				enable_power(pwr_on);
				PORT_LED &= ~_BV(LED_GREEN);
				_delay_ms(5000);
			}
			else if (click_time == 0)
				click_time = millis();
		}

		//uint16_t volt = 100; // read_voltage(ADC2);
		//spi_transfer_byte_as_slave('v');
		//spi_transfer_byte_as_slave((uint8_t)(volt & 0x00FF));
		//spi_transfer_byte_as_slave((uint8_t)(volt >> 8));
		_delay_ms(10);
	}
	spi_end();
}
