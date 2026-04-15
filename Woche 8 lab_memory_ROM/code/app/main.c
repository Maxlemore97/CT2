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
 * -- Aufgabe 5.2: Memory Test
 * --   Liest fortlaufend den EEPROM-Bereich 0x400..0x4FF aus.
 * --   Bei Abweichung vom erwarteten Wert (i == Byte) haelt das
 * --   Programm an und zeigt den Adressindex (erwarteter Wert)
 * --   auf LED23..16 und den gelesenen fehlerhaften Wert auf
 * --   LED07..00 an. Mit Taste T0 wird die Pruefung fortgesetzt.
 * --
 * -- $Id: main.c 5605 2023-01-05 15:52:42Z frtt $
 * ------------------------------------------------------------------
 */

/* standard includes */
#include <stdint.h>

/* user includes */
#include "reg_ctboard.h"
#include "hal_ct_buttons.h"


/// STUDENTS: To be programmed

/* EEPROM-Testbereich: NE2 -> Basis 0x64000000, Testbereich ab 0x400 */
#define EEPROM_TEST_BASE  ((volatile uint8_t *)0x64000400)
#define EEPROM_TEST_SIZE  256

/// END: To be programmed

int main(void)
{
    /// STUDENTS: To be programmed

    while (1) {
        for (uint32_t i = 0; i < EEPROM_TEST_SIZE; i++) {
            uint8_t expected = (uint8_t)i;
            uint8_t actual   = EEPROM_TEST_BASE[i];

            if (actual != expected) {
                CT_LED->BYTE.LED23_16 = expected;
                CT_LED->BYTE.LED07_00 = actual;
                while (!hal_ct_button_is_pressed(HAL_CT_BUTTON_T0));
            }
        }
    }

    /// END: To be programmed
}
