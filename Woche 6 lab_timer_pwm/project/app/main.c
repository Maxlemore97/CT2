/* ----------------------------------------------------------------------------
 * --  _____       ______  _____                                              -
 * -- |_   _|     |  ____|/ ____|                                             -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems              -
 * --   | | | '_ \|  __|  \___ \   Zuercher Hochschule Winterthur             -
 * --  _| |_| | | | |____ ____) |  (University of Applied Sciences)           -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland               -
 * ----------------------------------------------------------------------------
 * --
 * -- Project     : CT2 lab - Timer PWM
 * -- Description : Main program and interrupt service routine
 * --
 * --               Task 1: - Setup timer TIM4 to 1s.
 * --                       - Setup interrupt to toggle LED
 * --               Task 2: - Setup timer TIM3 to PWM mode
 * --                       - Read DIP switches to set duty cycles of channels
 * --               Task 3: - Use interrupt of TIM4 to create a transition
 * --                         from one colour to another
 * --
 * -- $Id: main.c 5605 2023-01-05 15:52:42Z frtt $
 * ------------------------------------------------------------------------- */

/* standard includes */
#include <stdint.h>
#include <reg_stm32f4xx.h>
#include <reg_ctboard.h>

/* user includes */
#include "timer.h"


/* -- Macros used as by student code
 * ------------------------------------------------------------------------- */

/// STUDENTS: To be programmed

#define STEP_SIZE  4000u
#define MASK_4BIT  0x0F

/// END: To be programmed

/* -- Function prototypes
 * ------------------------------------------------------------------------- */
 void TIM4_IRQHandler(void);
 

/* -- global variables visible only within this module
 * ------------------------------------------------------------------------- */

/* cyclic counter value between 0 and 0xF */
static uint16_t cycle_counter_4bit = 0;

/* -- M A I N
 * ------------------------------------------------------------------------- */

int main(void)
{
    /// STUDENTS: To be programmed

    uint16_t red, green, blue;

    tim4_init();
    tim3_init();

    while (1) {
        if (CT_DIPSW->BYTE.S31_24 & 0x80) {
            // Aufgabe 3: Automatischer Farbwechsel
            red   = cycle_counter_4bit * STEP_SIZE;
            green = (0x0F - cycle_counter_4bit) * STEP_SIZE;
            blue  = (CT_DIPSW->BYTE.S23_16 & MASK_4BIT) * STEP_SIZE;
        } else {
            // Aufgabe 2: Manuelle Farbeinstellung
            red   = (CT_DIPSW->BYTE.S7_0   & MASK_4BIT) * STEP_SIZE;
            green = (CT_DIPSW->BYTE.S15_8  & MASK_4BIT) * STEP_SIZE;
            blue  = (CT_DIPSW->BYTE.S23_16 & MASK_4BIT) * STEP_SIZE;
        }

        tim3_set_compare_register(PWM_CH1, red);
        tim3_set_compare_register(PWM_CH2, green);
        tim3_set_compare_register(PWM_CH3, blue);
    }

    /// END: To be programmed
}


/* -- Interrupt service routine
 * ------------------------------------------------------------------------- */

void TIM4_IRQHandler(void)
{
    /// STUDENTS: To be programmed

    static uint8_t led_state = 0;

    tim4_reset_uif();

    if (led_state == 0) {
        CT_LED->BYTE.LED31_24 = 0xFF;
        led_state = 1;
    } else {
        CT_LED->BYTE.LED31_24 = 0x00;
        led_state = 0;
    }

    cycle_counter_4bit = (cycle_counter_4bit + 1) & 0x0F;

    /// END: To be programmed
}
