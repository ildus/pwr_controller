#include "avr/io.h"
#include "avr/interrupt.h"
#include <util/delay.h>

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

bool			pwr_on = false,
				enable_red = false;
unsigned long	pwr_time = 0;

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
		pwr_time = millis();
	}
	else
	{
		pwr_time = 0;
		PORT_PWR &= ~_BV(PIN_PWR);
		PORT_LED &= ~_BV(LED_GREEN);

		/* disable intterupt */
		GIMSK &= ~_BV(INT0);

		spi_end();
	}
}

static inline void
check_red(float voltage)
{
	if (enable_red && voltage <= 2.9)
		PORT_LED ^= _BV(LED_RED);
	else if (enable_red && voltage <= 3.1)
		PORT_LED |= _BV(LED_RED);
	else
		PORT_LED &= ~_BV(LED_RED);
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
	unsigned long	click_time = 0,
					adc_time = 0;
	ButtonState		btn_state = {false,0,0};

	// setup btn
	DDRA &= ~_BV(PA1);
	PORTA &= ~_BV(PA1);

	//setup LED
	DDR_LED |= _BV(LED_RED) | _BV(LED_GREEN);
	PORT_LED &= ~(_BV(LED_RED) | _BV(LED_GREEN));

	sei();
	setup_timer0();

	adc_init();
	enable_power(pwr_on);

	while (1)
	{
		uint16_t	volt = read_voltage(ADC0);
		float		voltage = volt * INTERNAL_VREF / 1024 / DELIM_COEF;

		//check button state
		char level = PINA & _BV(PA1);
		if (level && btn_state.old_level != level)
		{
			if (millis() - btn_state.last_click > 100) {
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
			if (voltage > 2.9)
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

		if (pwr_on && pwr_time && millis() - pwr_time > 15000)
		{
			pwr_time = 0;
			spi_init();
		}

		if (millis() - adc_time > 2000)
		{
			if (voltage <= 2.9 && pwr_on)
			{
				enable_power(false);
				enable_red = true;
			}

			check_red(voltage);
			if (pwr_on)
				spi_transfer_data((uint8_t)(volt >> 8),
						(uint8_t)(volt & 0x00FF));

			adc_time = millis();
		}
		_delay_ms(10);
	}
}
