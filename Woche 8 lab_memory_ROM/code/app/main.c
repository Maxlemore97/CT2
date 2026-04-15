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

/* standard includes */
#include <stdint.h>

/// STUDENTS: To be programmed
#include "hal_ct_buttons.h"
#include "hal_ct_seg7.h"
#include "reg_ctboard.h"

#define EEPROM_BASE_ADDR 0x64000000
#define TEST_AREA_OFFSET 0x0400
#define TEST_AREA_SIZE   256
/// END: To be programmed

int main(void)
{
    /// STUDENTS: To be programmed
    uint8_t *eeprom = (uint8_t *)(EEPROM_BASE_ADDR + TEST_AREA_OFFSET);
    uint16_t index;
    uint16_t error_count = 0;

    /* Programm-Setup: 7-Segment-Anzeige mit 0 initialisieren */
    hal_ct_seg7_bin_write(error_count);

    /* Fortlaufender Test in einer Endlosschleife */
    while (1) {
        for (index = 0; index < TEST_AREA_SIZE; index++) {
            uint8_t expected_value = (uint8_t)index;
            uint8_t actual_value = eeprom[index];

            /* Prüfen, ob der gelesene Wert dem Erwartungswert entspricht */
            if (actual_value != expected_value) {
                
                /* Fehler hochzählen und auf dem 7-Segment anzeigen */
                error_count++;
                hal_ct_seg7_bin_write(error_count);

                /* Fehlerhafte Werte an den LEDs anzeigen */
                /* LED23..16 zeigt den Adressindex (Soll-Wert) */
                CT_LED->BYTE.LED23_16 = expected_value;
                
                /* LED7..0 zeigt das tatsächlich gelesene Byte (Ist-Wert) */
                CT_LED->BYTE.LED7_0 = actual_value;

                /* Warten, bis der Taster T0 gedrückt und wieder losgelassen wird */
                while (!hal_ct_button_is_pressed(HAL_CT_BUTTON_T0)) {
                    /* blockieren */
                }
                
                /* Nach Bestätigung LEDs wieder löschen*/
                CT_LED->BYTE.LED23_16 = 0x00;
                CT_LED->BYTE.LED7_0 = 0x00;
            }
        }

        /* Fehlerzähler nach jedem kompletten Prüfdurchlauf (0..255) zurücksetzen */
        error_count = 0;
        hal_ct_seg7_bin_write(error_count);
    }
    /// END: To be programmed
}