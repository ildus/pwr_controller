#include "timer.h"

#include "avr/io.h"
#include "avr/interrupt.h"

volatile uint8_t timer0_overflow;
volatile unsigned long millis_cnt;

ISR(TIM0_OVF_vect)
{
	millis_cnt += (int) ((1000 / (F_CPU/1024)) * 256);
}

void
setup_timer0(void)
{
	TCCR0B |= _BV(CS02) | _BV(CS00); // prescaler = 1024

	// initialize counters
    TCNT0 = 0;
	timer0_overflow = 0;
	millis_cnt = 0;

	// enable overflow interrupt
	TIMSK0 |= _BV(TOIE0);

	// enable global interrupts
	sei();
}

unsigned long
millis(void)
{
	return millis_cnt;
}
