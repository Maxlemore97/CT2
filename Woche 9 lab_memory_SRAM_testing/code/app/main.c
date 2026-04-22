/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zurich University of             -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                 -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Application for testing external memory
 * --
 * -- $Id: main.c 5605 2023-01-05 15:52:42Z frtt $
 * ------------------------------------------------------------------
 */

#include <stdint.h>
#include "hal_rcc.h"
#include "hal_fmc.h"
#include "hal_ct_lcd.h"
#include "reg_ctboard.h"
#include "hal_ct_buttons.h"
#include "hal_ct_seg7.h"

#define NR_OF_DATA_LINES           8
#define NR_OF_ADDRESS_LINES       11
#define CHECKER_BOARD           0xAA
#define INVERSE_CHECKER_BOARD   0x55
#define WALKING_ONES_ADDRESS    0x7FF

/* Set-up the macros (#defines) for your test */
/// STUDENTS: To be programmed

#define SRAM_BASE_ADDR  0x64000000UL

/// END: To be programmed

int main(void)
{
    hal_fmc_sram_init_t init;
    hal_fmc_sram_timing_t timing;
    
    /* add your required automatic (local) variables here */
    /// STUDENTS: To be programmed

    volatile uint8_t *walking_ones_addr =
        (volatile uint8_t *)(SRAM_BASE_ADDR + WALKING_ONES_ADDRESS);
    uint8_t pattern;
    uint8_t failed_patterns;
    uint8_t i;

    /* --- Variablen für den Address Bus Test --- */
    volatile uint8_t *sram = (volatile uint8_t *)SRAM_BASE_ADDR;

    /* Die 12 Testadressen aus dem PDF (Zweierpotenzen + 0x000) */
    uint16_t addresses[12] = {
        0x400, 0x200, 0x100, 0x080,
        0x040, 0x020, 0x010, 0x008,
        0x004, 0x002, 0x001, 0x000
    };

    /* Welche LED gehört zu welcher Testadresse (Abb. 9 im PDF).
       Wert = Bitmaske für CT_LED->HWORD.LED31_16.
       z.B. 0x0400 -> Bit 10 gesetzt -> LED26 leuchtet.
       Reihenfolge passt 1:1 zum Array "addresses" oben. */
    uint16_t led_bit[12] = {
        0x0400,  /* Adresse 0x400 -> LED26 (A10) */
        0x0200,  /* Adresse 0x200 -> LED25 (A9)  */
        0x0100,  /* Adresse 0x100 -> LED24 (A8)  */
        0x0080,  /* Adresse 0x080 -> LED23 (A7)  */
        0x0040,  /* Adresse 0x040 -> LED22 (A6)  */
        0x0020,  /* Adresse 0x020 -> LED21 (A5)  */
        0x0010,  /* Adresse 0x010 -> LED20 (A4)  */
        0x0008,  /* Adresse 0x008 -> LED19 (A3)  */
        0x0004,  /* Adresse 0x004 -> LED18 (A2)  */
        0x0002,  /* Adresse 0x002 -> LED17 (A1)  */
        0x0001,  /* Adresse 0x001 -> LED16 (A0)  */
        0x0800   /* Adresse 0x000 -> LED27       */
    };

    uint16_t address_error_leds = 0x0000;
    uint16_t test_address;
    uint16_t check_address;
    uint8_t  expected;
    uint8_t  actual_value;
    int k, j;

    /// END: To be programmed

    init.address_mux = DISABLE;                             // setup peripheral
    init.type = HAL_FMC_TYPE_SRAM;
    init.width = HAL_FMC_WIDTH_8B;
    init.write_enable = ENABLE;

    timing.address_setup = 0xFF;                            // all in HCLK
                                                            // cycles
    timing.address_hold = 0xFF;
    timing.data_setup = 0xFF;

    hal_fmc_init_sram(HAL_FMC_SRAM_BANK2, init, timing);    // init external bus
                                                            // bank 2 (NE2)
                                                            // asynch
    
    /* Data Bus Test - Walking ONES test */
    /// STUDENTS: To be programmed

    failed_patterns = 0x00;

    for (i = 0; i < NR_OF_DATA_LINES; i++) {
        pattern = (uint8_t)(0x01u << i);
        *walking_ones_addr = pattern;
        if (*walking_ones_addr != pattern) {
            /* Bit des fehlgeschlagenen Patterns auf LED7..0 anzeigen */
            failed_patterns |= pattern;
        }
    }

    CT_LED->BYTE.LED7_0 = failed_patterns;

    /// END: To be programmed
    
    /* Address Bus Test 
     * (1)  Write default values
     *
     *      Write the memory at all the power of 2 addresses (including 0x0000)
     *      to the default value of CHECKER_BOARD
     *
     * (2)  Perform tests
     *
     *      Select one power of 2 addresses after the other as test_address 
     *      (starting from the highest all the way down to 0x0000) --> 
     *          - Write the memory at test_address to INVERSE_CHECKER_BOARD
     *          - For all the power of 2 addresses including 0x0000
     *              o Read the memory content
     *              o Verify that the read value is either
     *                  (a) equal to CHECKER_BOARD in case a different address 
     *                      than test_addressed has been read or
     *                  (b) equal to INVERSE_CHECKER_BOARD in case the address
     *                      at test_address has been read
     *              o Errors found shall be indicated on LED31--16
     */
    
    /// STUDENTS: To be programmed

    /*
     * Ablauf:
     *   1) Alle 12 Testadressen mit 0xAA (CHECKER_BOARD) vorschreiben.
     *   2) Für jede der 12 Testadressen:
     *        a) 0x55 (INVERSE_CHECKER_BOARD) an die Testadresse schreiben.
     *        b) Alle 12 Adressen durchlesen und kontrollieren:
     *           - an der Testadresse MUSS 0x55 stehen,
     *           - an allen anderen Adressen MUSS 0xAA stehen.
     *           Wenn irgendwo was falsches steht -> Fehler-LED für
     *           diese Testadresse merken.
     *        c) Testadresse wieder auf 0xAA zurückschreiben.
     *   3) Am Ende die gemerkten Fehler-LEDs auf LED31..16 ausgeben.
     */

    /* Schritt 1: alle 12 Adressen auf 0xAA initialisieren */
    for (i = 0; i < 12; i++) {
        sram[addresses[i]] = CHECKER_BOARD;
    }

    /* Schritt 2: Testschleife über alle 12 Adressen */
    for (k = 0; k < 12; k++) {
        test_address = addresses[k];

        /* a) Inverses Pattern an die Testadresse schreiben */
        sram[test_address] = INVERSE_CHECKER_BOARD;

        /* b) Alle 12 Adressen prüfen */
        for (j = 0; j < 12; j++) {
            check_address = addresses[j];

            /* Welcher Wert wird erwartet? */
            if (check_address == test_address) {
                expected = INVERSE_CHECKER_BOARD;   /* 0x55 */
            } else {
                expected = CHECKER_BOARD;           /* 0xAA */
            }

            /* Wert aus dem Speicher lesen */
            actual_value = sram[check_address];

            /* Vergleich */
            if (actual_value != expected) {
                /* Fehler: LED für diese Testadresse merken */
                address_error_leds = address_error_leds | led_bit[k];
            }
        }

        /* c) Testadresse wieder auf 0xAA zurücksetzen */
        sram[test_address] = CHECKER_BOARD;
    }

    /* Schritt 3: Fehler-LEDs anzeigen */
    CT_LED->HWORD.LED31_16 = address_error_leds;

     /// END: To be programmed
    
    /* Device Test 
     * (1) Fill the whole memory with known increment pattern.
     *          Address     Data
     *          0x000       0x01
     *          0x001       0x02
     *          .....       ....
     *          0x0FE       0xFF
     *          0x0FF       0x00
     *          0x100       0x01
     *          .....       ....
     *
     * (2) First test: Read back each location and check pattern.
     *     In case of error, write address with wrong data to 7-segment and
     *     wait for press on button T0.
     *     Bitwise invert  the pattern in each location for the second test
     *
     * (3) Second test: Read back each location and check for new pattern.
     *     In case of error, write address with wrong data to 7-segment and
     *     wait for press on button T0.
     */
    /// STUDENTS: To be programmed




    /// END: To be programmed
    
    // Write 'End'
    CT_SEG7->RAW.BYTE.DS0 = 0xA1;
    CT_SEG7->RAW.BYTE.DS1 = 0xAB;
    CT_SEG7->RAW.BYTE.DS2 = 0x86;
    CT_SEG7->RAW.BYTE.DS3 = 0xFF;
    
    while(1){
    }

}
