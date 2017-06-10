MCU_TARGET     = ATtiny24
FLASHER        = avrdude -c usbasp -p $(MCU_TARGET)
F_CPU          = 8000000
BAUDRATE       = 9600

all: fuses program

program:
	avr-gcc -g -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a -c -o main.o main.c
	avr-gcc -g -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a -c -o spi.o spi.c
	avr-gcc -g -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a -o main.elf main.o spi.o
	avr-size main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	$(FLASHER) -b ${BAUDRATE} -U flash:w:main.hex

fuses:
	# -B 200
	$(FLASHER) -b ${BAUDRATE} -U lfuse:w:0x62:m -U hfuse:w:0xdf:m

shell:
	$(FLASHER) -t

clean:
	rm main.elf main.o spi.o
