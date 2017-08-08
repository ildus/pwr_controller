#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>
#include <stdbool.h>

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04

void spi_init();
uint8_t spi_transfer_data(uint8_t *data);
void spi_end(void);

#endif /* _SPI_H_ */
