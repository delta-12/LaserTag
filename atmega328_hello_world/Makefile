# compiler options
CC=avr-gcc
CFLAGS= -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p

# compile and upload all
all: blink.upload

# compile .c file to .out files
%.out: %.c
	$(CC) $(CFLAGS) $< -o $@

# extract eeprom segment of .out file to .hex file in intel hex format
%.hex: %.out
	avr-objcopy -O ihex -R .eeprom $< $@

# upload to board
%.upload: %.hex
	avrdude -c usbtiny -p atmega328p -U flash:w:$<

# remove output files
clean:
	rm -f *.hex *.out
