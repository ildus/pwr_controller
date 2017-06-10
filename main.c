#include "spi.h"
#include <util/delay.h>

int main(void)
{
	uint8_t	level = 0;

	spi_init(SPI_MODE0);
	while (1)
	{
		spi_transfer_byte(level++);
		_delay_ms(20);
	}
	spi_end();
}
