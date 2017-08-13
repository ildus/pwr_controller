#include "timer.h"
#include "avr/interrupt.h"

volatile unsigned int millis_cnt;

ISR(TIM0_OVF_vect)
{
	millis_cnt += (int) ((1000.0 / (F_CPU/1024)) * 256);
}
