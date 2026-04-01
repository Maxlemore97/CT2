/* ----------------------------------------------------------------------------
 * --  _____       ______  _____                                              -
 * -- |_   _|     |  ____|/ ____|                                             -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems              -
 * --   | | | '_ \|  __|  \___ \   Zurich University of                       -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                           -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland               -
 * ----------------------------------------------------------------------------
 * --
 * -- Description:  Implementation of module adc.
 * --
 * --
 * -- $Id: adc.c 5610 2023-02-03 09:22:02Z frtt $
 * ------------------------------------------------------------------------- */

/* standard includes */
#include <stdint.h>
#include <reg_stm32f4xx.h>

/* user includes */
#include "adc.h"


/* -- Macros
 * ------------------------------------------------------------------------- */

#define PERIPH_GPIOF_ENABLE (0x00000020)
#define PERIPH_ADC3_ENABLE  (0x00000400)

/* Configuring pin for ADC: PF.6 */
#define GPIOF_MODER_ANALOG (0x3 << 12)


/* -- Macros used by student code
 * ------------------------------------------------------------------------- */

/// STUDENTS: To be programmed

#define ADC_CR1_RES_MASK    (0x3 << 24)

#define ADC_CR2_ADON        (0x1 << 0)
#define ADC_CR2_SWSTART     (0x1 << 30)

#define ADC_SR_EOC          (0x1 << 1)

#define ADC_CHANNEL_4       (0x4)

#define FILTER_SIZE         (16)

/// END: To be programmed


/* Public function definitions
 * ------------------------------------------------------------------------- */

/*
 *  See header file
 */
void adc_init(void)
{
    /* Enable peripheral clocks */
    RCC->AHB1ENR |= PERIPH_GPIOF_ENABLE;
    RCC->APB2ENR |= PERIPH_ADC3_ENABLE;

    /* Configure PF.6 as input */
    GPIOF->MODER |= GPIOF_MODER_ANALOG;

    /* ADC common init */
    ADCCOM->CCR = 0;        // TSVREF = '0'    -> Temp sensor disabled
                            // VBATE = '0'     -> VBAT disabled
                            // ADCPRE = '00'   -> APB2 / 2 -> 21 MHz
                            // DMA = '00'      -> DMA disabled
                            // DELAY = '0000'  -> Delay 5 cycles
                            // MULTI = '00000' -> ADC independent mode

    /* Configure ADC3 */

    /// STUDENTS: To be programmed

    /* Set sample time for channel 4: 3 cycles (SMP4 = 000 in SMPR2) */
    ADC3->SMPR2 &= ~(0x7 << 12);

    /* Set channel 4 as first conversion in regular sequence */
    ADC3->SQR3 = ADC_CHANNEL_4;

    /* Set sequence length to 1 conversion (L = 0000 in SQR1) */
    ADC3->SQR1 = 0;

    /// END: To be programmed
}


/*
 *  See header file
 */
uint16_t adc_get_value(adc_resolution_t resolution)
{
    uint16_t adc_value;

    /// STUDENTS: To be programmed

    /* Set resolution in CR1 */
    ADC3->CR1 &= ~ADC_CR1_RES_MASK;
    ADC3->CR1 |= (uint32_t)resolution;

    /* Enable ADC (ADON) */
    ADC3->CR2 |= ADC_CR2_ADON;

    /* Start conversion (SWSTART) */
    ADC3->CR2 |= ADC_CR2_SWSTART;

    /* Wait for end of conversion (EOC) */
    while (!(ADC3->SR & ADC_SR_EOC)) {}

    /* Read converted value */
    adc_value = (uint16_t)ADC3->DR;

    /// END: To be programmed

    return adc_value;
}


/*
 *  See header file
 */
uint16_t adc_filter_value(uint16_t adc_value)
{
    uint16_t filtered_value = 0;

    /// STUDENTS: To be programmed

    static uint16_t buffer[FILTER_SIZE] = { 0 };
    uint32_t sum = 0;
    uint8_t i;

    /* Shift values: oldest drops off at index 0 */
    for (i = 0; i < FILTER_SIZE - 1; i++) {
        buffer[i] = buffer[i + 1];
    }

    /* Insert new value at the end */
    buffer[FILTER_SIZE - 1] = adc_value;

    /* Calculate average */
    for (i = 0; i < FILTER_SIZE; i++) {
        sum += buffer[i];
    }
    filtered_value = (uint16_t)(sum / FILTER_SIZE);

    /// END: To be programmed

    return filtered_value;
}
