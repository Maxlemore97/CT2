/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zurich University of             -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                 -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Description:  Implementation of module counter
 * --               The module provides a counter that can be used as
 * --               pseudo random number generator for a dice.
 * --
 * -- $Id: counter.c 2977 2016-02-15 16:05:50Z ruan $
 * --------------------------------------------------------------- */

/* user includes */
#include "counter.h"
#include "dice.h"

#include <stdio.h>
#include <stdlib.h>

/* variables visible within the whole module*/
static uint8_t dice_counter = 1;

/* function definitions */

uint8_t counter_read(void);
void counter_increment(void);

/// STUDENTS: To be programmed

uint8_t counter_read(void) {
	return rand()%6+1;
}

void counter_increment(void){
	++dice_counter;
}




/// END: To be programmed
