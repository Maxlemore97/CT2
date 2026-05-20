/* ----------------------------------------------------------------------------
 * --  _____       ______  _____                                              -
 * -- |_   _|     |  ____|/ ____|                                             -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems              -
 * --   | | | '_ \|  __|  \___ \   Zurich University of                       -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                           -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland               -
 * ----------------------------------------------------------------------------
 * -- $Id: main.c 4800 2019-05-09 15:30:18Z ruan $
 * ------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdio.h>
#include <reg_stm32f4xx.h>
#include "hal_ct_lcd.h"
#include "hal_timer.h"
#include <reg_ctboard.h>


/* -- macros
 * ------------------------------------------------------------------------- */
#define NUMBER_OF_TIMER_2_INTERRUPTS (uint32_t)1000
#define RELOAD_VALUE_TIM2            (uint32_t)84000

#define IRQNUM_TIM2                  28
#define IRQNUM_TIM3                  29

#define STRING_LENGTH_FOR_32BIT      11     // 4G --> 10 bit plus end of string


/* -- function prototypes
 * ------------------------------------------------------------------------- */
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);


/* -- functions with module-wide scope
 * ------------------------------------------------------------------------- */
static void print_results(void);
static uint8_t convert_uint32_t_to_string(char ret_val[], uint32_t value);
static uint16_t read_hex_switch(void);


/* -- variables with module-wide scope
 * ------------------------------------------------------------------------- */
static volatile hal_bool_t measurement_done = FALSE;
static uint32_t tim3_interrupt_counter = 0;
static uint32_t tim2_interrupt_counter = 0;
static uint32_t min_latency = 100000;
static uint32_t max_latency = 0;
static uint32_t sum_latency = 0;
static uint32_t avg_latency = 0;
static uint32_t sum_tisr = 0;       // time of interrupt service routine
static uint32_t avg_tisr = 0;       // average time of interrupt service routine
static volatile uint32_t dummy_counter;

/* -- M A I N
 * ------------------------------------------------------------------------- */

int main(void)
{
    hal_timer_base_init_t timer_init;
    uint16_t reload_value_tim3;
      
    while (1){
        
        /* wait for button press to start test */
        while ( !(CT_BUTTON & 0x1) ) {
            
            /* dummy read to display the HEX switch position on SEG7 */
            read_hex_switch();
        }
        
        /* reset statistics */
        measurement_done = FALSE;
        tim3_interrupt_counter = 0;
        tim2_interrupt_counter = 0;
        min_latency = 100000;
        max_latency = 0;
        sum_latency = 0;
        avg_latency = 0;
        sum_tisr = 0;
        avg_tisr = 0;
        
        /* init display, Use RED background while test is running */
        hal_ct_lcd_clear();
        hal_ct_lcd_color(HAL_LCD_RED, 0xffff);
        hal_ct_lcd_color(HAL_LCD_BLUE, 0u);
        hal_ct_lcd_color(HAL_LCD_GREEN, 0u);

        /* init timer2 with a clock source frequency of 84MHz 
           --> generate a timer2 interrupt every 1ms */
        TIM2_ENABLE();
        TIM2_RESET();

        timer_init.mode = HAL_TIMER_MODE_UP;
        timer_init.run_mode = HAL_TIMER_RUN_CONTINOUS;
        timer_init.prescaler = 0u;
        timer_init.count = RELOAD_VALUE_TIM2;     //counter overflow every 1ms
        
        hal_timer_init_base(TIM2, timer_init);
        hal_timer_irq_set(TIM2, HAL_TIMER_IRQ_UE, ENABLE);

        /* read and display the amount of load selected for timer 3*/
        reload_value_tim3 = read_hex_switch();
        
        /* init timer3 with a clock source frequency of 84MHz */
        TIM3_ENABLE();
        TIM3_RESET();

        timer_init.mode = HAL_TIMER_MODE_UP;
        timer_init.run_mode = HAL_TIMER_RUN_CONTINOUS;
        timer_init.prescaler = 0u;
        timer_init.count = reload_value_tim3;  // from hex switch
        
        hal_timer_init_base(TIM3, timer_init);
        hal_timer_irq_set(TIM3, HAL_TIMER_IRQ_UE, ENABLE);

        /* set default interrupt priorities: 
                load on both timers set to same priority   */
        NVIC->IP[IRQNUM_TIM2] = 0x10;      //set priority level of timer2 to 1
        NVIC->IP[IRQNUM_TIM3] = 0x10;      //set priority level of timer3 to 1


        /* Set interrupt priorities based on dip switches 
         *  - S7..S4   --> priority for load on timer3
         *  - S15..S11 --> priority for timer2
         * 
         * Reading of dip switches  through  CT_DIPSW->BYTE.S7_0 and
         * CT_DIPSW->BYTE.S15_8
         * 
         * Priorities
         *  - The lower a priority level, the greater the priority
         *  - 4-bit priority level in bits [7:4] of NVIC->IP[] register,
         *    i.e. 0x00 - 0xF0 
         */

        /// STUDENTS: To be programmed
        
        /* Aufgabe 4.4: Einlesen und Setzen der Interrupt-Prioritäten
         * Die Prioritäten befinden sich in den oberen 4 Bits [7:4] jedes Bytes.
         * Wir lesen die entsprechenden DIP-Switch-Bytes (S7..S0 bzw. S15..S8)
         * und maskieren sie mit 0xF0, um die unteren 4 Bits zu ignorieren.
         */
        uint8_t priority_tim3 = CT_DIPSW->BYTE.S7_0 & 0xF0;   // S7..S4 -> Prio Timer 3
        uint8_t priority_tim2 = CT_DIPSW->BYTE.S15_8 & 0xF0;  // S15..S12 -> Prio Timer 2
        
        /* Zuweisung in den Nested Vectored Interrupt Controller (NVIC) */
        NVIC->IP[IRQNUM_TIM3] = priority_tim3;
        NVIC->IP[IRQNUM_TIM2] = priority_tim2;

        /// END: To be programmed

        /* start timer2 */
        hal_timer_start(TIM2);
        
        /* if there is load --> start timer 3 */
        if (reload_value_tim3 != 0) {
            hal_timer_start(TIM3);
        }
        
        /* wait for measurement to finish */
        while(!measurement_done){
        }
        
        /* print out measurement */
        avg_latency = sum_latency / tim2_interrupt_counter;
        avg_tisr = sum_tisr / tim2_interrupt_counter;
        print_results();

    }
}

/**
 * \brief   Timer 2 ISR: Measuring Interrupt latency and Interrupt Service Time
 */
void TIM2_IRQHandler(void)
{
    /// STUDENTS: To be programmed
    
    /* Aufgabe 4.1 a): 1. Latenzzeit sofort zu Beginn der ISR auslesen */
    uint32_t current_latency = TIM2->CNT;
    
    /* Aufgabe 4.1 a): 2. Setzen Sie die IRQ Bedingung zurück */
    hal_timer_irq_clear(TIM2, HAL_TIMER_IRQ_UE);
    
    /* Aufgabe 4.1 a): 3. Inkrementieren Sie den vorgegebenen Zähler */
    tim2_interrupt_counter++;
    
    /* Aufgabe 4.1 a): 4. Variablen für Minimum, Maximum und Summe aktualisieren */
    if (current_latency < min_latency) {
        min_latency = current_latency;
    }
    if (current_latency > max_latency) {
        max_latency = current_latency;
    }
    sum_latency += current_latency;
    
    /* Aufgabe 4.1 a): 5. Nach definierter Anzahl stoppen */
    if (tim2_interrupt_counter >= NUMBER_OF_TIMER_2_INTERRUPTS) {
        hal_timer_stop(TIM2);
        hal_timer_stop(TIM3);
        measurement_done = TRUE;
    }
    
    /* Aufgabe 4.3 b): 6. Dauer der Interrupt Service Routine (t_ISR) messen.
     * Auslesen des Zählerstands am Ende der ISR und Differenz zum ersten Auslesen.
     */
    uint32_t isr_end_time = TIM2->CNT;
    sum_tisr += (isr_end_time - current_latency);

    /// END: To be programmed
}

/**
 * \brief  Timer 3 ISR: Generating load
 */
void TIM3_IRQHandler(void)
{
    hal_timer_irq_clear(TIM3, HAL_TIMER_IRQ_UE);
    tim3_interrupt_counter++;
    
    /* add a little bit of delay in the ISR */
    for (dummy_counter = 0; dummy_counter < 3; dummy_counter++) {
    }
}


/* -- local function definitions
 * ------------------------------------------------------------------------- */

/**
 * \brief  Prints the minimal, maximal and average interrupt latency and the 
 *         number of occured timer3 interrupts to the display
 */
static void print_results(void)
{
  
    char label_min[] = "min ";
    char label_max[] = "max ";
    char label_avg[] = "avg ";
    char label_load[] = "load ";

    char ret_val[STRING_LENGTH_FOR_32BIT];
    uint8_t length = 0;
    uint8_t pos = 0;

    // set lcd backlight green
    hal_ct_lcd_color(HAL_LCD_RED, 0u);
    hal_ct_lcd_color(HAL_LCD_GREEN, 0xffff);

    // write label "min"
    hal_ct_lcd_write(pos, label_min);
    
    /* Add writing of min, max and average values to LCD, 
     * include the appropriate labels.
     * 
     * Add writing of the counted number of external interrupts to LCD,
     * include the appropriate label
     */
    
    /// STUDENTS: To be programmed
    
    /* Aufgabe 4.2: Ausgabe "min" (Label wurde bereits geschrieben) */
    pos = 4; // Hinter "min " label, welches die Länge 4 hat
    length = convert_uint32_t_to_string(ret_val, min_latency);
    hal_ct_lcd_write(pos, ret_val);
    
    /* Aufgabe 4.2: Ausgabe "max" */
    pos = 9; 
    hal_ct_lcd_write(pos, label_max);
    pos += 4; // Länge von "max "
    length = convert_uint32_t_to_string(ret_val, max_latency);
    hal_ct_lcd_write(pos, ret_val);
    
    /* Aufgabe 4.3 b): Ausgabe der ISR Dauer (am Ende der 1. Zeile gemäss Abbildung 3) */
    pos = 17;
    length = convert_uint32_t_to_string(ret_val, avg_tisr);
    hal_ct_lcd_write(pos, ret_val);

    /* Aufgabe 4.2: Ausgabe "avg" (Wir nutzen hier Zeile 2, Position 20) */
    pos = 20; 
    hal_ct_lcd_write(pos, label_avg);
    pos += 4; // Länge von "avg "
    length = convert_uint32_t_to_string(ret_val, avg_latency);
    hal_ct_lcd_write(pos, ret_val);
    
    /* Aufgabe 4.3 a): Ausgabe "load " mit Timer 3 Interrupts-Zähler auf Zeile 2 */
    pos = 28;
    hal_ct_lcd_write(pos, label_load);
    pos += 5; // Länge von "load "
    length = convert_uint32_t_to_string(ret_val, tim3_interrupt_counter);
    hal_ct_lcd_write(pos, ret_val);

    /// END: To be programmed
}


/**
 * \brief  Converts an uint32_t value into a string.
 * \param  ret_val: Pointer to the array where the result of the conversion will
                    be stored. The array must be large enough to hold the string
 * \param  value:   The value to be converted
 * \return The functions returns a negative value in the case of an error.  
 *         Otherwise, the number of characters written is returned.
 */
static uint8_t convert_uint32_t_to_string(char ret_val[], uint32_t value)
{
    return (uint8_t) snprintf(ret_val, STRING_LENGTH_FOR_32BIT, "%d", value);
}

/**
 * \brief  Read the setting on the hex switch and display the selected load
 *         on the 7-segment: none / 10 kHz / 100 kHz / 1000 kHz
 * \return The appropriate reload value for the load on timer3
 */
static uint16_t read_hex_switch(void)
{
    uint8_t hex_sw_pos = CT_HEXSW & 0x0F;
    uint16_t ret_value;
    
    switch (hex_sw_pos) {
        case 1: case 5: case 9: case 13:
            /* 10 kHz */
            CT_SEG7->RAW.WORD = (uint32_t)0xFFFFF9C0; // 10
            ret_value = (uint16_t)8400;
            break;
        case 2: case 6: case 10: case 14:
            /* 100 kHz */
            CT_SEG7->RAW.WORD = (uint32_t)0xFFF9C0C0; // 100
            ret_value = (uint16_t)840;
            break;
        case 3: case 7: case 11: case 15:
            /* 1000 kHz */
            CT_SEG7->RAW.WORD = (uint32_t)0xF9C0C0C0; // 1000
            ret_value = (uint16_t)84;
            break;
        default:
            /* no load */
            CT_SEG7->RAW.WORD = (uint32_t)0xFFFFFFC0; // 0 --> no load
            ret_value = (uint16_t)0;
    }
    return ret_value;
}

/* ============================================================================
 * OVERALL CODE EXPLANATION (INTERRUPT PERFORMANCE LAB):
 * ----------------------------------------------------------------------------
 * This program measures Interrupt Latency and the duration of an Interrupt 
 * Service Routine (t_ISR) under varying load conditions and priority settings.
 * 
 * 1. TIM2 (Measurement Timer): Runs continuously, triggering an interrupt 
 *    every 1ms (upcounter mapped to the 84MHz system clock).
 *    - Latency Measurement: Inside TIM2_IRQHandler, TIM2->CNT is read immediately.
 *      Since TIM2 resets to 0 upon generating the interrupt, the value of CNT 
 *      represents the exact number of clock ticks passed before the ISR started.
 *    - t_ISR Measurement: TIM2->CNT is read again at the very end of the ISR.
 *      The difference between this end value and the start value gives the 
 *      total CPU execution time of the ISR.
 * 
 * 2. TIM3 (Load Generator): Creates artificial background CPU load. Its reload
 *    value (frequency: off, 10kHz, 100kHz, 1MHz) is selected via the Hex Switch.
 *    It runs a dummy loop inside its ISR to purposefully keep the CPU busy.
 * 
 * 3. NVIC & Priorities (DIP Switches): The Nested Vectored Interrupt Controller
 *    (NVIC) dictates which interrupt can interrupt another (Preemption).
 *    - The 4-bit priority values for TIM2 and TIM3 are dynamically read from  
 *      the DIP switches (S15..S8 and S7..S0). A LOWER value = HIGHER priority.
 *    - If TIM2 > TIM3: TIM2 preempts TIM3. Latency and t_ISR remain perfectly stable.
 *    - If TIM2 < TIM3: TIM3 preempts TIM2. Both Latency and t_ISR increase.
 *      At 1MHz load, TIM3 fires so rapidly (every 84 ticks) that the CPU is 100% 
 *      occupied servicing TIM3 (Interrupt Storm). This causes TIM2 to "starve" 
 *      completely, meaning TIM2_IRQHandler never gets executed.
 * ========================================================================= */

