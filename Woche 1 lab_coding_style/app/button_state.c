/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zurich University of             -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                 -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Description:  Implementation of module button_state
 * --               The module handles the button state.
 * --
 * -- $Id: button_state.c 2026-01-13 frtt $
 * --------------------------------------------------------------- */

/* standard includes */
#include <reg_ctboard.h>

/* user includes */
#include "button_state.h"


static uint8_t last_button_state = 0x0;

/* function definitions */

uint8_t button_state_eval(void) {
    uint8_t button_pressed;
    uint8_t button_state;

    button_state = CT_BUTTON;
    button_pressed = ~last_button_state & button_state;
    last_button_state = button_state;
    
    return button_pressed;
}
