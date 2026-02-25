/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zurich University of             -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                 -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Description:  Implementation of module lcd
 * --               Performs all the interactions with the lcd
 * --
 * -- $Id: lcd.c 5144 2020-09-01 06:17:21Z ruan $
 * ------------------------------------------------------------------
 */

/* standard includes */
#include <stdio.h>
#include <reg_ctboard.h>
#include <hal_ct_seg7.h>

/* user includes */
#include "display.h"
#include "reg_ctboard.h"

/* macros */
#define LCD_ADDR_LINE1      0u
#define LCD_ADDR_LINE2      20u

#define NR_OF_CHAR_PER_LINE 20u

#define LCD_CLEAR           "                    "

/* function declarations */
/// STUDENTS: To be programmed


void display_write_value(uint8_t slot_nr, uint8_t value);
void display_write(uint8_t total_value);
void display_clear(void);
void display_write_throw(uint8_t dice_number);


/// END: To be programmed

/* function definitions */

/*
 * see header file
 */
void display_write_throw(uint8_t dice_number)
{
   hal_ct_seg7_bin_write(dice_number); 
}

/// STUDENTS: To be programmed

void display_write_value(uint8_t slot_nr, uint8_t value){
	uint8_t start;
	if (slot_nr == 1){
		start = 0;
	}
	if (slot_nr == 2){
		start = 3;
	}
	if (slot_nr == 3){
		start = 6;
	}
	if (slot_nr == 4){
		start = 9;
	}
	if (slot_nr == 5){
		start = 12;
	}
	if (slot_nr == 6){
		start = 15;
	}
	
	char first = ' ';
	char second = '0';
	if(value >= 10)
		first = value/10 + '0';

	second += value%10;
	
	CT_LCD->ASCII[start] = first;
	CT_LCD->ASCII[start+1] = second;
	
}

void display_clear(void){
	for(uint8_t i = 0; i <= NR_OF_CHAR_PER_LINE; i++){
		CT_LCD->ASCII[i+LCD_ADDR_LINE1] = ' ';
	}
	
	for(uint8_t i = 0; i <= NR_OF_CHAR_PER_LINE; i++){
		CT_LCD->ASCII[i+LCD_ADDR_LINE2] = ' ';
	}
	
	// Green background
	CT_LCD->BG.GREEN = 65535u;
}

void display_write_total(uint8_t total_value){
	char buffer[NR_OF_CHAR_PER_LINE] = {' '};
	
	snprintf(buffer, sizeof(buffer), "total throws %u", total_value); 
	
	for(uint8_t i = 0; i <= NR_OF_CHAR_PER_LINE; i++){
		CT_LCD->ASCII[LCD_ADDR_LINE2+i] = buffer[i];
	}
}

/// END: To be programmed
