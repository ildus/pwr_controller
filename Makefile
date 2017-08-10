MCU_TARGET     = ATtiny24

ifdef USBASP
FLASHER        = avrdude -c usbasp -p $(MCU_TARGET)
BAUDRATE       = -b 9600
else
FLASHER        = avrdude -C avrdude.conf -c pi_1 -p $(MCU_TARGET) -P gpio
BAUDRATE       = -B 200
endif

F_CPU          = 8000000
CFLAGS	       = -std=c99 -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a

CC = avr-gcc
OBJ = main.o spi.o timer.o
DEPS = spi.h timer.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: main.hex

main.elf: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	avr-size $@

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

upload: fuses
	$(FLASHER) ${BAUDRATE} -U flash:w:main.hex

fuses:
	$(FLASHER) ${BAUDRATE} -U lfuse:w:0xE2:m -U hfuse:w:0xdf:m

shell:
	$(FLASHER) -t

clean:
	rm *.elf *.o *.hex
