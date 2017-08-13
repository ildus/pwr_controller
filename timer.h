#ifndef timer_h
#define timer_h

#include "avr/io.h"

extern volatile unsigned int millis_cnt;

#define millis() (millis_cnt)

static inline void
setup_timer0(void)
{
	TCCR0B |= _BV(CS02) | _BV(CS00); // prescaler = 1024

	// initialize counters
	TCNT0 = 0;
	millis_cnt = 0;

	// enable overflow interrupt
	TIMSK0 |= _BV(TOIE0);
}

#endif // timer_h
