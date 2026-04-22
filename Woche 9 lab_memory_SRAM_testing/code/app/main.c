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
// Basisadresse des SRAMs auf dem STM32F4 (FMC Bank 2 / NE2)
#define SRAM_BASE_ADDR 0x64000000

/// END: To be programmed

int main(void)
{
    hal_fmc_sram_init_t init;
    hal_fmc_sram_timing_t timing;
    
    /* add your required automatic (local) variables here */ 
    /// STUDENTS: To be programmed
    // Zeiger auf den Start des SRAMs. 'volatile' zwingt den Compiler, jeden Zugriff physisch auf dem Bus auszuführen und nichts wegzuoptimieren.
    volatile uint8_t *sram = (volatile uint8_t *)SRAM_BASE_ADDR;
    
    // Variablen zum Speichern aufgetretener Fehler
    uint8_t data_error = 0;
    uint16_t address_error = 0;
    
    // Hilfsvariablen für Muster, Adressen und Schleifen
    uint8_t pattern;
    uint16_t test_address;
    uint16_t address;
    int i, t_idx, a_idx;
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
    data_error = 0;
    
    // Schleife über alle 8 Datenleitungen (D0 bis D7)
    for (i = 0; i < NR_OF_DATA_LINES; i++) {
        // Erzeuge ein Bitmuster mit genau einer '1', die nach links wandert
        // (z.B. 0x01, 0x02, 0x04, 0x08...)
        pattern = 1 << i;
        
        // Schreibe das Muster an eine beliebige aber feste Adresse im SRAM
        sram[WALKING_ONES_ADDRESS] = pattern;
        
        // Lese den Wert direkt wieder aus. Wenn er nicht dem Muster entspricht,
        // ist die entsprechende Datenleitung auf dem Weg zum SRAM defekt.
        if (sram[WALKING_ONES_ADDRESS] != pattern) {
            // Speichere den Fehler, indem das fehlgeschlagene Bit per ODER-Verknüpfung in der Fehlermaske gesetzt wird. So gehen vorherige Fehler nicht verloren.
            data_error |= pattern;
        }
    }
    
    // Ausgabe der gesammelten Datenbus-Fehler auf den LEDs 0 bis 7.
    // Leuchtet eine LED, ist die zugehörige Datenleitung fehlerhaft.
    CT_LED->BYTE.LED7_0 = data_error;
    /// END: To be programmed
    
    /* Address Bus Test 
     * (1)  Write default values
     *
     * Write the memory at all the power of 2 addresses (including 0x0000)
     * to the default value of CHECKER_BOARD
     *
     * (2)  Perform tests
     *
     * Select one power of 2 addresses after the other as test_address 
     * (starting from the highest all the way down to 0x0000) --> 
     * - Write the memory at test_address to INVERSE_CHECKER_BOARD
     * - For all the power of 2 addresses including 0x0000
     * o Read the memory content
     * o Verify that the read value is either
     * (a) equal to CHECKER_BOARD in case a different address 
     * than test_addressed has been read or
     * (b) equal to INVERSE_CHECKER_BOARD in case the address
     * at test_address has been read
     * o Errors found shall be indicated on LED31--16
     */
    
    /// STUDENTS: To be programmed
    // Initialisierung: Alle relevanten Adressen (nur 1 Bit gesetzt + 0x0000) mit einem definierten Grundmuster (0xAA) beschreiben.
    for (a_idx = 0; a_idx <= NR_OF_ADDRESS_LINES; a_idx++) {
        // Wenn der Index NR_OF_ADDRESS_LINES erreicht, prüfen wir Adresse 0x0000. Ansonsten schieben wir eine '1' an die entsprechende Adress-Bit-Position (1, 2, 4, 8...).
        address = (a_idx == NR_OF_ADDRESS_LINES) ? 0x0000 : (1 << a_idx);
        sram[address] = CHECKER_BOARD;
    }
    /// END: To be programmed

    /* (2)  Perform tests */
    /// STUDENTS: To be programmed
    address_error = 0;
    
    // Äußere Schleife: Definiert die aktuell zu testende Adresse (test_address). Wir arbeiten uns von der höchsten Adresse runter bis zur 0x0000.
    for (t_idx = NR_OF_ADDRESS_LINES; t_idx >= 0; t_idx--) {
        test_address = (t_idx == NR_OF_ADDRESS_LINES) ? 0x0000 : (1 << t_idx);
        
        // 1. Markiere die aktuelle Test-Adresse mit dem inversen Muster (0x55)
        sram[test_address] = INVERSE_CHECKER_BOARD;

        // Innere Schleife: Geht nun erneut durch ALLE relevanten Adressen, um zu schauen, ob das Schreiben auf 'test_address' versehentlich eine andere Adresse beeinflusst hat (z.B. durch einen Kurzschluss zweier Adressleitungen).
        for (a_idx = NR_OF_ADDRESS_LINES; a_idx >= 0; a_idx--) {
            address = (a_idx == NR_OF_ADDRESS_LINES) ? 0x0000 : (1 << a_idx);
            
            if (address == test_address) {
                // Wenn wir die Adresse lesen, auf die wir gerade absichtlich 0x55 geschrieben haben, MUSS dort 0x55 stehen. Steht etwas anderes dort (z.B. weil der Schreibbefehl wegen einer defekten Adressleitung woanders landete), loggen wir einen Fehler für test_address.
                if (sram[address] != INVERSE_CHECKER_BOARD) {
                    address_error |= test_address; 
                }
            } else {
                // Wenn wir JEDE ANDERE Adresse lesen, MUSS dort immer noch das alte Grundmuster (0xAA) stehen. Hat es sich verändert, gab es eine Überlappung und wir loggen einen Fehler für diese fälschlicherweise überschriebene Adresse.
                if (sram[address] != CHECKER_BOARD) {
                    address_error |= address; 
                }
            }
        }
        
        // 3. Wichtig: Nach dem Überprüfen muss die Test-Adresse wieder auf das Grundmuster (0xAA) 
        // zurückgesetzt werden, damit der Durchlauf für die nächste test_address eine saubere Basis hat.
        sram[test_address] = CHECKER_BOARD;
    }
    
    // Ausgabe der gesammelten Adressbus-Fehler auf den LEDs 16 bis 31.
    // Leuchtet eine LED, deutet dies auf einen Fehler bei dieser speziellen Adressleitung hin.
    CT_LED->HWORD.LED31_16 = address_error;
    /// END: To be programmed
    
    /* Device Test 
     * (1) Fill the whole memory with known increment pattern.
     * Address     Data
     * 0x000       0x01
     * 0x001       0x02
     * .....       ....
     * 0x0FE       0xFF
     * 0x0FF       0x00
     * 0x100       0x01
     * .....       ....
     *
     * (2) First test: Read back each location and check pattern.
     * In case of error, write address with wrong data to 7-segment and
     * wait for press on button T0.
     * Bitwise invert  the pattern in each location for the second test
     *
     * (3) Second test: Read back each location and check for new pattern.
     * In case of error, write address with wrong data to 7-segment and
     * wait for press on button T0.
     */
    /// STUDENTS: To be programmed
    
    // (1) Fill the whole memory (0x000 to 0x7FF for a 2K SRAM)
    for (address = 0; address <= 0x7FF; address++) {
        // Berechne das Pattern: (Adresse + 1). Der Cast auf uint8_t 
        // sorgt für den korrekten Überlauf (z.B. 0x100 wird zu 0x00).
        sram[address] = (uint8_t)(address + 1);
    }

    // (2) First test: Pattern lesen, prüfen und danach invertiert zurückschreiben
    for (address = 0; address <= 0x7FF; address++) {
        uint8_t expected_pattern = (uint8_t)(address + 1);
        
        // Lese den Wert und vergleiche
        if (sram[address] != expected_pattern) {
            // Fehlerfall: Adresse auf der 7-Segment-Anzeige ausgeben
            CT_SEG7->BIN.HWORD = address;
            // Warten, bis Taste T0 gedrückt wird, um fortzufahren
            while (!hal_ct_button_is_pressed(HAL_CT_BUTTON_T0)) {
            }
        }
        
        // Bitweise invertiertes Muster in die gleiche Speicherstelle schreiben (~ Operator)
        sram[address] = ~expected_pattern;
    }

    // (3) Second test: Invertiertes Pattern lesen und prüfen
    for (address = 0; address <= 0x7FF; address++) {
        // Das nun erwartete Pattern ist das invertierte Muster von vorhin
        uint8_t expected_pattern = (uint8_t)~(address + 1);
        
        if (sram[address] != expected_pattern) {
            // Fehlerfall: Adresse auf der 7-Segment-Anzeige ausgeben
            CT_SEG7->BIN.HWORD = address;
            // Warten, bis Taste T0 gedrückt wird, um fortzufahren
            while (!hal_ct_button_is_pressed(HAL_CT_BUTTON_T0)) {
            }
        }
    }
    /// END: To be programmed
    
    // Write 'End'
    CT_SEG7->RAW.BYTE.DS0 = 0xA1;
    CT_SEG7->RAW.BYTE.DS1 = 0xAB;
    CT_SEG7->RAW.BYTE.DS2 = 0x86;
    CT_SEG7->RAW.BYTE.DS3 = 0xFF;
    
    while(1){
    }

}