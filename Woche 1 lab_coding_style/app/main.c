/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zurich University of             -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                 -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Application for an electronic dice
 * --
 * -- $Id: main.c 5144 2020-09-01 06:17:21Z ruan $
 * --------------------------------------------------------------- */

/* standard includes */
#include <stdint.h>

/* user includes */
#include "statistics.h"
#include "counter.h"
#include "display.h"
#include "dice.h"
#include "button_state.h"

/* function definitions */

/*
 * Pushing button T0 displays a pseudo random dice value.
 * Throws are recorded and statistics are continuously displayed.
 */

int main(void)
{
    uint8_t dice_number;
    uint8_t i;
    uint8_t number_of_throws;
    uint8_t button_state;
    
    display_clear();

    while (1) {
        button_state = button_state_eval();

        if (button_state & 0x01) {
            dice_number = counter_read();
            display_write_throw(dice_number);
            statistics_add_throw(dice_number);
        }

        counter_increment();

        // display statistics: per number and total 
        for (i = 1; i <= NR_OF_DICE_VALUES; i++) {
            number_of_throws = statistics_read(i);
            if (number_of_throws != ERROR_VALUE) {
                display_write_value(i, number_of_throws);
            }
        }
        display_write_total(statistics_read(0));
    }
}
