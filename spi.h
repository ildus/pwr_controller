#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04

void spi_init(int spi_mode);
uint8_t spi_transfer_byte(uint8_t data);
void spi_end(void);

#endif /* _SPI_H_ */
