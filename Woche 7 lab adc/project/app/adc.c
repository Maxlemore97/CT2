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
/* ADC Control Register 1 (CR1) */
#define ADC_CR1_RES_MASK     (0x03000000) // Mask to clear Resolution bits (bits 25:24)

/* ADC Control Register 2 (CR2) */
#define ADC_CR2_ADON         (0x00000001) // Bit 0: Turn ADC ON
#define ADC_CR2_SWSTART      (0x40000000) // Bit 30: Start regular conversion

/* ADC Status Register (SR) */
#define ADC_SR_EOC           (0x00000002) // Bit 1: End Of Conversion flag

/* ADC Regular Sequence Register 3 (SQR3) */
#define ADC_SQR3_SQ1_MASK    (0x0000001F) // Mask to clear the 1st conversion slot (bits 4:0)
#define ADC_SQR3_SQ1_CH4     (0x00000004) // Value to select Channel 4 for the 1st conversion

/* ADC Sample Time Register 2 (SMPR2) */
#define ADC_SMPR2_SMP4_MASK  (0x00007000) // Mask to clear sample time for Channel 4 (bits 14:12)

/* ADC Regular Sequence Register 1 (SQR1) */
#define ADC_SQR1_L_MASK      (0x00F00000) // Mask to clear the sequence length (bits 23:20)
/// END: To be programmed
 /* ------------------------------------------------------------------------- */

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
    /* * STEP 1: Turn on ADC3.
     * set the ADON bit in Control Register 2 (CR2) to wake it up.
     */
    ADC3->CR2 |= ADC_CR2_ADON;
    
    /* * STEP 2: Set the sampling time for Channel 4.
     * The ADC needs to "charge up" its internal capacitor 
     * to match the incoming voltage. The task asks for 3 cycles.
     * We use SMPR2 because it controls channels 0-9. Bits 14:12 control Channel 4.
     * 000 = 3 cycles. We just clear the bits to guarantee they are 000.
     */
    ADC3->SMPR2 &= ~ADC_SMPR2_SMP4_MASK;
    
    /* * STEP 3: Set the sequence length.
     * ADC can convert a whole list of channels automatically (a sequence). 
     * We only want to read ONE channel right now. 
     * SQR1 bits 23:20 control length. 0000 = 1 conversion.
     */
    ADC3->SQR1 &= ~ADC_SQR1_L_MASK;
    /// END: To be programmed
}


/*
 * See header file
 */
uint16_t adc_get_value(adc_resolution_t resolution)
{
    uint16_t adc_value;

    /// STUDENTS: To be programmed
    /* * STEP 1: Set the resolution.
     * The task asks for 6-bit. We clear the old resolution bits first, 
     * then apply the new one passed into the function.
     */
    ADC3->CR1 &= ~ADC_CR1_RES_MASK;
    ADC3->CR1 |= resolution;

    /* * STEP 2: Tell the ADC WHICH channel to convert.
     * We only have a sequence length of 1, we still have to tell 
     * the ADC what goes into "Slot 1" (SQ1) of that sequence.
     * We clear SQ1 bits, then write '4' to select Channel 4.
     */
    ADC3->SQR3 &= ~ADC_SQR3_SQ1_MASK;
    ADC3->SQR3 |= ADC_SQR3_SQ1_CH4;

    /* * STEP 3: Start the conversion!
     * Setting the SWSTART bit in CR2 pulls the trigger.
     */
    ADC3->CR2 |= ADC_CR2_SWSTART;

    /* * STEP 4: Wait for it to finish.
     * Conversion takes a few clock cycles. We are stuck in this while-loop 
     * until the hardware sets the End Of Conversion (EOC) bit in the Status Register (SR).
     */
    while ((ADC3->SR & ADC_SR_EOC) == 0) {
        // Just wait...
    }

    /* * STEP 5: Read the data.
     * The result is stored in the Data Register (DR). 
     * Reading this register automatically clears the EOC flag for us!
     */
    adc_value = (uint16_t)(ADC3->DR & 0xFFFF);
    /// END: To be programmed

    return adc_value;
}


/*
 * See header file
 */
uint16_t adc_filter_value(uint16_t adc_value)
{
    uint16_t filtered_value = 0;

    /// STUDENTS: To be programmed
    /* * STEP 1: Create a history array that remembers values between calls.
     * The 'static' keyword is crucial here!
     */
    static uint16_t history[16] = {0};
    uint32_t sum = 0;
    
    /* * STEP 2: Shift all old values one position to the right.
     * We start at the end (index 15) and copy the value from the left.
     * At the same time, we add the shifted values to our sum.
     */
    for (int i = 15; i > 0; i--) {
        history[i] = history[i - 1];
        sum += history[i];
    }
    
    /* * STEP 3: Insert the brand new value at the very front (index 0).
     * And don't forget to add it to the sum as well.
     */
    history[0] = adc_value;
    sum += history[0];

    /* * STEP 4: Calculate the average.
     * Since we have 16 values, we divide the sum by 16.
     */
    filtered_value = (uint16_t)(sum / 16);
    /// END: To be programmed

    return filtered_value;
}
