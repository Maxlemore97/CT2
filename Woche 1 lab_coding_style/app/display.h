/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zurich University of             -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                 -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Description:  Interface of module lcd
 * --               Performs all the interactions with the lcd
 * --
 * -- $Id: lcd.h 5144 2020-09-01 06:17:21Z ruan $
 * -- $Id: display.h 2026_01_13 frtt
 * ------------------------------------------------------------------
 */

/* re-definition guard */
#ifndef _DISPLAY_H
#define _DISPLAY_H

/* standard includes */
#include <stdint.h>

/* function declarations */

/*
 * \brief  Writes 'value' to the indicated 'position' on the lcd
 * 
 * \param  slot_nr: A number between 1 and 6 indicating at which one of the 
 *         six available slots on the lcd the 'value' shall be printed. The 
 *         values at the other 5 slots remain unchanged.
 * 
 * \param  value: The value to be printed
 */
void display_write_value(uint8_t slot_nr, uint8_t value);

/*
 * \brief  Writes an explanatory string followed by 'total_value' on the lcd. 
 * 
 * \param  total_value: The value to be printed
 */
void display_write_total(uint8_t total_value);


/*
 * \brief  Clears the lcd and switches it to light green. 
 */
void display_clear(void);

/*
 * \brief Writes 'throw' to the seven-segment display.
 *
 * \param trown dice number to be displayed
 */
void display_write_throw(uint8_t dice_number);

#endif
