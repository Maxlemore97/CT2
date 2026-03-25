/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zuercher Hochschule Winterthur   -
 * --  _| |_| | | | |____ ____) |  (University of Applied Sciences) -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Module      : SPI Library
 * -- Description :
 * --
 * -- $Id: hal_spi.c 4707 2019-02-26 09:32:59Z ruan $
 * ------------------------------------------------------------------
 */
#include <reg_stm32f4xx.h>
#include "hal_spi.h"

#define BIT_TXE  (uint32_t)0x00000002
#define BIT_RXNE (uint32_t)0x00000001
#define BIT_BSY  (uint32_t)0x00000080

static void set_ss_pin_low(void);
static void set_ss_pin_high(void);
static void wait_10_us(void);

/*
 * according to description in header file
 */
void hal_spi_init(void)
{
    RCC->APB2ENR |= 0x00001000;     /**< enable SPI clock */
    RCC->AHB1ENR |= 0x00000001;     /**< start clock on GPIO A */
    GPIOA->OSPEEDR &= 0xFFFF00FF;   /**< clear P4 to P7 */
    GPIOA->OSPEEDR |= 0x0000FF00;   /**< set P4 to P7 to 100 MHz */
    GPIOA->MODER &= 0xFFFF00FF;     /**< clear mode on P5 to P7 */
    /* P5 to P7, P4 output mode */
    GPIOA->MODER |= 0x0000A900;     /**< Set alternate function mode on */
    /* P5 to P7, P4 output mode */
    GPIOA->AFRL &= 0x0000FFFF;      /**< clear alternate function */
    GPIOA->AFRL |= 0x55550000;      /**< Set SPI1 alternate function */

    SPI1->CR2 = 0x0000;             /**< set spi to default state */
    SPI1->CR1 = 0x0000;             /**< set spi to default state */

    /// STUDENTS: To be programmed
    SPI1->CR1 |= (0x7 << 3);   // BR[2:0] = 111 --> fPCLK/256 = 164 kHz
    SPI1->CR1 |= (0x1 << 2);   // MSTR = 1      --> Master mode
    SPI1->CR1 |= (0x1 << 9);   // SSM = 1       --> Software slave management
    SPI1->CR1 |= (0x1 << 8);   // SSI = 1       --> Internal slave select high
    SPI1->CR1 |= (0x1 << 6);   // SPE = 1       --> SPI enable (set last)
    /// END: To be programmed
    
    set_ss_pin_high();
}

/*
 * according to description in header file
 */
uint8_t hal_spi_read_write(uint8_t send_byte)
{
    /// STUDENTS: To be programmed

    uint8_t rec_byte;

    set_ss_pin_low();

    while (!(SPI1->SR & BIT_TXE)) {}    // wait until Tx buffer empty
    SPI1->DR = send_byte;                // write byte to data register

    while (!(SPI1->SR & BIT_RXNE)) {}   // wait until Rx buffer not empty
    rec_byte = SPI1->DR;                 // read received byte

    while (SPI1->SR & BIT_BSY) {}        // wait until SPI not busy

    wait_10_us();
    set_ss_pin_high();

    return rec_byte;

    /// END: To be programmed
}

/**
 * \brief  Set Slave-Select Pin (P5.5 --> PA4) low
 *
 * No parameters
 *
 * No returns
 */
static void set_ss_pin_low(void)
{
    GPIOA->BSRR = 0x00100000;          // Set P5.5 --> PA4 low
}

/**
 * \brief  Set Slave-Select Pin (P5.5 --> PA4) high
 *
 * No parameters
 *
 * No returns
 */
static void set_ss_pin_high(void)
{
    GPIOA->BSRR = 0x00000010;          // Set P5.5 --> PA4 high
}

/**
 * \brief  Wait for approximately 10us
 *
 * No parameters
 *
 * No returns
 */
static void wait_10_us(void)
{
    uint8_t counter = 0;
    while (counter < 160) {
        counter++;
    }
}
