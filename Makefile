MCU_TARGET     = ATtiny24
FLASHER        = avrdude -C avrdude.conf -c pi_1 -p $(MCU_TARGET) -P gpio
F_CPU          = 8000000
#BAUDRATE       = -b 9600
BAUDRATE       = -B 200
CFLAGS	       = -std=c99

all: fuses program

program:
	avr-gcc -g ${CFLAGS} -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a -c -o main.o main.c
	avr-gcc -g ${CFLAGS} -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a -c -o spi.o spi.c
	avr-gcc -g ${CFLAGS} -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a -o main.elf main.o spi.o
	avr-size main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	$(FLASHER) ${BAUDRATE} -U flash:w:main.hex

fuses:
	$(FLASHER) ${BAUDRATE} -U lfuse:w:0x62:m -U hfuse:w:0xdf:m

shell:
	$(FLASHER) -t

clean:
	rm main.elf main.o spi.o
