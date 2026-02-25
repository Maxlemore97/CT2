
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
 * -- $Id: button_state.h 2026-01-13 frtt $
 * --------------------------------------------------------------- */

/* re-definition guard */
#ifndef _BUTTON_STATE_H
#define _BUTTON_STATE_H

/* standard includes */
#include <stdint.h>

/* function declarations */

/*
 * Retunrs the button status.
 */

uint8_t button_state_eval(void);

#endif
