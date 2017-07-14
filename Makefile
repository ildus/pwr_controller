MCU_TARGET     = ATtiny24
FLASHER        = avrdude -C avrdude.conf -c pi_1 -p $(MCU_TARGET) -P gpio
F_CPU          = 8000000
#BAUDRATE       = -b 9600
BAUDRATE       = -B 200
CFLAGS	       = -std=c11 -DF_CPU=${F_CPU} -Wall -Os -mmcu=attiny24a

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
	$(FLASHER) ${BAUDRATE} -U lfuse:w:0x62:m -U hfuse:w:0xdf:m

shell:
	$(FLASHER) -t

clean:
	rm *.elf *.o *.hex
