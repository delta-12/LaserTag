/* Simple program for an ATmega328 to blink 3 LEDs when a button is pressed */

/* Branched and Edited by Tyler Hansen */
/* Branched and Edited by Kushal Parekh */
/* Branched and Edited by Benjamin Esquieres */

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include <util/delay.h>

#define DELAY_MS 25U
#define LED_COUNT 3U

int main()
{
    uint8_t i = 0U;

    /* PORTB pin 1 is an input from a button */
    DDRB &= ~(_BV(PORTB1));

    /* PORTC pins 0, 1, 2 are outputs to LEDs */
    DDRC |= _BV(PORTC0) | _BV(PORTC1) | _BV(PORTC2);

    while (1)
    {
        if (!(PINB & _BV(PINB1)))
        {
            /* Toggle between LEDs when the button is pressed */
            PORTC = _BV(i);
            i++;
            i %= LED_COUNT;

            _delay_ms(DELAY_MS);
        }
        else
        {
            /* Turn off all LEDs when button is released */
            PORTC &= ~_BV(PORTC0) & ~_BV(PORTC1) & ~_BV(PORTC2);
        }
    }

    return 0;
}
