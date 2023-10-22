# AVR toolchain with the ATmega328

Use the AVR toolchain and Pocket AVR Programmer to compile and upload a basic C program for an ATmega328.

## Prerequisites

### Hardware Tools

- [Pocket AVR Programmer](https://www.sparkfun.com/products/9825)

### Software Tools and Executables

- `avr-gcc`
- `avr-objcopy`
- `avrdude`
- USBtiny LibUSB-Win32 device driver (Windows only)

#### Ubuntu and other Debian-based Linux distros

The AVR toolchain can be installed on Ubuntu and other Debian-based Linux distributions with the following commands.

1. Update package sources

   `apt update`

2. Install AVR packages

   `apt install -y gcc-avr binutils-avr avrdude`

Ubuntu and other Debian-based Linux distributions do not require installation of a separate driver to work with the Pocker AVR Programmer.

#### Windows

To install the AVR toolchain on Windows, see [WinAVR](https://winavr.sourceforge.net/).

To install the USBtiny LibUSB-Win32 device driver to work with the Pocket AVR Programmer, see Sparkfun's [Pocket AVR Programmer Hookup Guide](https://learn.sparkfun.com/tutorials/pocket-avr-programmer-hookup-guide).

#### macOS

To install the AVR toolchain on macOS, see [homebrew-avr](https://github.com/osx-cross/homebrew-avr).

macOS does not require installation of a separate driver to work with the Pocker AVR Programmer.

## Compile and Upload

A simple sample program `blink.c` is included in the repo to test the compilation and upload process. It can be used with the example commands in each step.

1. Compile program with `avr-gcc`

   `avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p <C file> -o <out file>`

   Replace `<C file>` with the name of your C program and `<out file>` with the name of the output file. For instance, `avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p blink.c -o blink.out`

   `-Wall` turns on all warnings, `-Os` optimizes for size, `-DF_CPU=16000000UL` sets the `F_CPU` macro for calculating delays based on a 16MHz clock, and `-mmcu=atmega328p` tells the compiler to compile for the ATmega328p variant of the ATmega328. This last argument can be changed for other variants of the ATmega328.

2. Extract the eeprom segment of the `.out` file to a `.hex` file in Intel hex format

   `avr-objcopy -O ihex -R .eeprom <out file> <hex file>`

   e.g. `avr-objcopy -O ihex -R .eeprom blink.out blink.hex`

3. Upload the program to the ATmega328 via the Pocket AVR Programmer

   `avrdude -c usbtiny -p atmega328p -U flash:w:<hex file>`

   e.g. `avrdude -c usbtiny -p atmega328p -U flash:w:blink.hex`

### Makefile

On Ubuntu and other Debian-based distros, the included Makefile can also be used for compilation and uploading.

1. Compile program

   `make <C file name>.out`

   e.g. If the program is named `blink.c`, the command to compile would be `make blink.out`

2. Extract `.out` to `.hex`

   `make <out file name>.hex`

   e.g. `make blink.hex`

3. Upload to ATmega328

   `make <hex file name>.upload`

   e.g. `make blink.upload`

   Note: This command will run the previous two commands, thus running the first and second commands are not strictly necessary.
