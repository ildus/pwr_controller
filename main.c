#include "avr/io.h"
#include "avr/interrupt.h"
#include <util/delay.h>
#include <stdbool.h>

#include "spi.h"
#include "timer.h"

/*
 * internal 1.1V voltage reference, by datasheet
 * external capacitor should be connected to Aref
 * calibrated for actual chip, i got 1.208V
 */
const double INTERNAL_VREF = 1.208,
			 DELIM_COEF = 0.18; // 1.5K and 330 resistors

#define ADC0 0b0000

#define DDR_LED DDRB
#define PORT_LED PORTB
#define LED_RED PB1
#define LED_GREEN PB0

#define DDR_PWR DDRA
#define PORT_PWR PORTA
#define PIN_PWR PA3
#define PIN_OUT PA2

#define LOW_LEVEL		3.5
#define LOWEST_LEVEL	3.2

bool			pwr_on = false,
				enable_red = false;

static void enable_power(bool enable)
{
	pwr_on = enable;
	DDR_PWR |= _BV(PIN_PWR);

	if (pwr_on)
	{
		PORT_PWR |= _BV(PIN_PWR);
		PORT_LED |= _BV(LED_GREEN);
		PORT_LED &= ~_BV(LED_RED);
		enable_red = true;
	}
	else
	{
		PORT_PWR &= ~_BV(PIN_PWR);
		PORT_LED &= ~_BV(LED_GREEN);
	}
}

static inline void
check_red(float voltage)
{
	if (enable_red && voltage <= LOWEST_LEVEL)
		PORT_LED ^= _BV(LED_RED);
	else if (enable_red && voltage <= LOW_LEVEL)
		PORT_LED |= _BV(LED_RED);
	else
		PORT_LED &= ~_BV(LED_RED);
}

/*
 * >= 4.5	- 0001
 * >= 4		- 0010
 * >= 3.5	- 0011
 * >= 3.1	- 0100
 * >= 3.0	- 0101
 * >= 2.9	- 0110
 * else		- 0111
 */
static inline void
set_pins_out(float voltage)
{
	unsigned char res;

	res = 0b111;

	if (voltage >= 4.5)
		res = 0b001;
	else if (voltage >= 4)
		res = 0b010;
	else if (voltage >= 3.5)
		res = 0b011;
	else if (voltage >= 3.1)
		res = 0b100;
	else if (voltage >= 3.0)
		res = 0b101;
	else if (voltage >= 2.9)
		res = 0b110;

	if (res & 0b100)
		PORT_SPI |= _BV(DD_USCK);
	else
		PORT_SPI &= ~_BV(DD_USCK);

	if (res & 0b010)
		PORT_SPI |= _BV(DD_DO);
	else
		PORT_SPI &= ~_BV(DD_DO);

	if (res & 0b001)
		PORT_SPI |= _BV(DD_DI);
	else
		PORT_SPI &= ~_BV(DD_DI);

	if (PIN_SS & _BV(DD_SS))
		PORT_PWR |= _BV(PIN_OUT);
	else
		PORT_PWR &= ~_BV(PIN_OUT);
}

static inline void
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

static inline uint16_t
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

static inline uint16_t
read_voltage(uint8_t pin)
{
	uint16_t	sum = 0;

	for (int i = 0; i < 9; i++)
		sum += adc_read(pin);

	return sum;
	//v = (sum / 10) * INTERNAL_VREF / 1024;
	//return (uint8_t) (v * 100);
}

typedef struct ButtonState {
	bool clicked;
	unsigned long last_click;
	char old_level;
} ButtonState;

int
main(void)
{
	unsigned int	click_time = 0,
					adc_time = 0,
					opi_time = 0;
	ButtonState		btn_state = {false,0,0};

	// setup btn
	DDRA &= ~_BV(PA1);
	PORTA &= ~_BV(PA1);

	//setup LED
	DDR_LED |= _BV(LED_RED) | _BV(LED_GREEN);
	PORT_LED &= ~(_BV(LED_RED) | _BV(LED_GREEN));

	//setup voltage lines, SCK, DO, DI
	DDR_SPI |= _BV(DD_USCK);
	DDR_SPI |= _BV(DD_DO);
	DDR_SPI |= _BV(DD_DI);
	PORT_SPI &= ~_BV(DD_USCK);
	PORT_SPI &= ~_BV(DD_DO);
	PORT_SPI &= ~_BV(DD_DI);

	// set SS as input
	DDR_SS &= ~_BV(DD_SS);
	PORT_SS |= _BV(DD_SS);

	// set check pin as output
	DDR_PWR |= _BV(PIN_OUT);

	sei();
	setup_timer0();

	adc_init();
	enable_power(pwr_on);

	while (1)
	{
		uint16_t	volt = read_voltage(ADC0);
		float		voltage = volt * INTERNAL_VREF / 1024 / DELIM_COEF / 10;

		//check button state
		char level = PINA & _BV(PA1);
		if (level && btn_state.old_level != level)
		{
			if (millis() - btn_state.last_click > 50) {
				btn_state.clicked = true;
			}
			btn_state.last_click = millis();
		}
		btn_state.old_level = level;

		if (!btn_state.old_level)
			click_time = 0;

		if (!pwr_on && btn_state.clicked)
		{
			btn_state.clicked = false;
			if (voltage > LOWEST_LEVEL)
				enable_power(true);
			else
				enable_red = !enable_red;
		}
		else if (pwr_on && btn_state.clicked)
		{
			if (click_time && millis() - click_time > 3000)
			{
				btn_state.clicked = false;
				enable_power(false);
				enable_red = false;
			}
			else if (click_time == 0)
				click_time = millis();
		}

		if (millis() - adc_time > 2000)
		{
			if (voltage <= LOWEST_LEVEL && pwr_on)
			{
				enable_power(false);
				enable_red = true;
			}

			if (pwr_on && (PIN_SS & _BV(DD_SS)))
			{
				if (opi_time == 0)
					opi_time = millis();
				else if (opi_time && ((millis() - opi_time) > 30000))
				{
					enable_power(false);
					enable_red = true;
				}
			}
			else
				opi_time = 0;

			check_red(voltage);
			set_pins_out(voltage);
			adc_time = millis();
		}
		_delay_ms(10);
	}
}
