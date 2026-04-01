/* ----------------------------------------------------------------------------
 * --  _____       ______  _____                                              -
 * -- |_   _|     |  ____|/ ____|                                             -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems              -
 * --   | | | '_ \|  __|  \___ \   Zuercher Hochschule Winterthur             -
 * --  _| |_| | | | |____ ____) |  (University of Applied Sciences)           -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland               -
 * ----------------------------------------------------------------------------
 * --
 * -- Description : Main program.
 * --
 * --               Task 1: - Setup ADC3 channel 4 (6 bit)
 * --                       - Read analog value and output to 7 Segment
 * --               Task 2: - Implement convert_to_fixpoint function:
 * --                         Converts binary data into fixpoint value
 * --               Task 3: - Implement possibility to choose resolution
 * --                         for analog conversion
 * --               Task 4: - Implement a moving average filter
 * --
 * -- $Id: main.c 5605 2023-01-05 15:52:42Z frtt $
 * ------------------------------------------------------------------------- */

/* standard includes */
#include <stdint.h>

/* user includes */
#include "reg_ctboard.h"
#include "adc.h"
#include "hal_ct_lcd.h"


/* -- Macros for accessing CT Board LCD
 * ------------------------------------------------------------------------- */

#define LCD            (*(uint16_t *)0x60000310)

#define ASCII_CHAR_V   (0x56)
#define ASCII_CHAR_DOT (0x2e)
#define ASCII_ZERO     (0x30)

#define LCD_POSITION_0 (uint16_t)(0x0000)
#define LCD_POSITION_1 (uint16_t)(0x0100)
#define LCD_POSITION_2 (uint16_t)(0x0200)
#define LCD_POSITION_3 (uint16_t)(0x0300)
#define LCD_POSITION_4 (uint16_t)(0x0400)
#define LCD_POSITION_5 (uint16_t)(0x0500)


/* -- Macros used by student code
 * ------------------------------------------------------------------------- */
/// STUDENTS: To be programmed
#define HEXSW_RES_MASK    (0x03) // Mask to read the lowest 2 bits of the HEX switch
#define DIPSW_FILTER_MASK (0x01) // Mask to read DIP Switch 0
/// END: To be programmed


/* -- Local function declaration
 * ------------------------------------------------------------------------- */
static uint16_t normalize_value(uint16_t value, adc_resolution_t resolution);
static void display_on_lcd(uint16_t bcd_value);
static void convert_hex_to_ascii(uint16_t hex_value, char* characters);


/* -- M A I N
 * ------------------------------------------------------------------------- */

int main(void)
{
    /// STUDENTS: To be programmed
    adc_init();

    while (1) {
        adc_resolution_t current_res;
        uint16_t led_mask = 0;

        /* * 1. Read the HEX switch to determine the desired resolution.
         * We only care about the lowest 2 bits (values 0, 1, 2, or 3).
         */
         uint8_t hex_sw = CT_HEXSW & HEXSW_RES_MASK;        
        
         /* * 2. Set the resolution enum and the LED pattern based on the switch.
         * The LEDs are mapped to bits. 0x003F = 0000 0000 0011 1111 (6 LEDs on).
         */
        switch (hex_sw) {
            case 0: // xx00b -> 6-bit
                current_res = ADC_RES_6BIT;
                led_mask = 0x003F; /* 6 LEDs */
                break;
            case 1: // xx01b -> 8-bit
                current_res = ADC_RES_8BIT;
                led_mask = 0x00FF; /* 8 LEDs */
                break;
            case 2: // xx10b -> 10-bit
                current_res = ADC_RES_10BIT;
                led_mask = 0x03FF; /* 10 LEDs */
                break;
            case 3: // xx11b -> 12-bit
                current_res = ADC_RES_12BIT;
                led_mask = 0x0FFF; /* 12 LEDs */
                break;
            default:
                current_res = ADC_RES_6BIT;
                led_mask = 0x003F;
                break;
        }

        /* * 3. Update the LEDs on the board.
         */
        CT_LED->HWORD.LED15_0 = led_mask;

       /* * 4. Get the raw ADC value using our selected resolution.
         */
        uint16_t raw_val = adc_get_value(current_res);

        /* * 5. Feed the filter.
         * We ALWAYS send the new value to the filter so its history 
         * stays updated, even if we don't display the filtered result yet.
         */
        uint16_t filtered_val = adc_filter_value(raw_val);
        uint16_t display_val;

        /* * 6. Check DIP Switch 0 to toggle the filter output.
         * If the switch is ON, we use the smoothed value.
         * If OFF, we use the jumpy raw value.
         */
        if (CT_DIPSW->BYTE.S7_0 & DIPSW_FILTER_MASK) {
            display_val = filtered_val;
        } else {
            display_val = raw_val;
        }

        /* * 7. Display the CHOSEN value on the 7-segment display!
         * Now the 7-segment display will also stop fluctuating when the filter is ON.
         */
        CT_SEG7->BIN.HWORD = display_val;

        /* * 8. Normalize to voltage range (0 - 3300) and display on LCD.
         * We pass the chosen display_val to the math function.
         */
        uint16_t norm_val = normalize_value(display_val, current_res);
        display_on_lcd(norm_val);
    }
    /// END: To be programmed
}


/* -- Local function definitions
 * ------------------------------------------------------------------------- */

/*
 * Normalize value to an integer scale 0 - 3300 (0.0V - 3.3 V)
 *       ADC_RES_6BIT:     value 0 - 63          return 0 - 3300
 *       ADC_RES_8BIT:     value 0 - 255         return 0 - 3300
 *       ADC_RES_10BIT:    value 0 - 1023        return 0 - 3300
 *       ADC_RES_12BIT:    value 0 - 4095        return 0 - 3300
 */
static uint16_t normalize_value(uint16_t value, adc_resolution_t resolution)
{
    /* Use of 32bit -> 4096(12bit) * 3300 = 13'516'800 */
    uint32_t normalized;

    /// STUDENTS: To be programmed
    uint16_t max_val = 63; // Default
    
    /* * STEP 1: Determine the maximum value based on the chosen resolution.
     * 6-bit  = 2^6 - 1  = 63
     * 8-bit  = 2^8 - 1  = 255
     * 10-bit = 2^10 - 1 = 1023
     * 12-bit = 2^12 - 1 = 4095
     */
    switch (resolution) {
        case ADC_RES_6BIT:  
            max_val = 63;   
            break;
        case ADC_RES_8BIT:  
            max_val = 255;  
            break;
        case ADC_RES_10BIT: 
            max_val = 1023; 
            break;
        case ADC_RES_12BIT: 
            max_val = 4095; 
            break;
    }
    
    /* * STEP 2: Scale using integer arithmetic.
     * We map the raw value (0 to max_val) to the voltage range (0 to 3300).
     */
    normalized = ((uint32_t)value * 3300) / max_val;
    /// END: To be programmed

    /* Return 16bit value -> max after normalization = 3300 */
    return (uint16_t)normalized;
}

/*
 * Display value on LCD display.
 * The data is interpreted and displayed as a fixed point number with
 * three places after the decimal point.
 */
static void display_on_lcd(uint16_t data_3dec_places)
{
    enum
    {   value_size =5,
        voltage_size = 7,
        dot_position = 1
    };
    char character_values[value_size];
    char character_voltage[voltage_size];
    char temp_character_voltage[voltage_size];
    uint8_t character_size = 0;
    uint8_t start_point;
    uint8_t i;
    
    // get the values
    convert_hex_to_ascii(data_3dec_places, character_values);    
    
    // find first space or nul character
    for(i = 0; i < value_size; i++){
        if(character_values[i] < ASCII_ZERO){
            character_size = i;
            break;
        }
    }
    
    start_point = value_size - character_size - 1;
    
    // shift chars to right and fill with zeros
    for(i = 0; i < voltage_size; i++){
        if(i < start_point){
            temp_character_voltage[i] = ASCII_ZERO;
        } else{
            temp_character_voltage[i] = character_values[i - start_point];
        }
    }
    
    // add dot, voltage and nul char
    for(i = 0; i < value_size; i++){
        if(i < dot_position){
            character_voltage[i] = temp_character_voltage[i];
        } else if(i == dot_position){
            character_voltage[i] = ASCII_CHAR_DOT;
        } else if(i > dot_position){
            character_voltage[i] = temp_character_voltage[i-1];
        }
    }
    character_voltage[voltage_size - 2] = ASCII_CHAR_V;
    character_voltage[voltage_size - 1] = 0;
    
    // write characters to display
    hal_ct_lcd_write(0, character_voltage);
}

static void convert_hex_to_ascii(uint16_t hex_value, char* characters){
    uint8_t i = 0;
    uint8_t char_size;
    enum {array_size = 5};
    char temp_characters[array_size];
    
    // cut and convert digits to ascii
    while(hex_value > 0){
        temp_characters[i] = (hex_value % 10) + ASCII_ZERO;
        hex_value /= 10;
        i++;
    }
    
    // invert order
    char_size = i;
    for (i = 0; i < char_size; i++){
        characters[i] = temp_characters[char_size - i - 1];
    }
    
    // fill rest with spaces
    for (i = char_size; i < array_size; i++){
        characters[i] = 32;
    }
    
    // end last part with zero character
    characters[array_size - 1] = 0;
}
