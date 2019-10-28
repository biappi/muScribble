#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>

#define SPI1                        (0x40013000)
#define SPI1_CR1                    (*(volatile uint32_t *)(SPI1))
#define SPI1_CR2                    (*(volatile uint32_t *)(SPI1 + 0x04))
#define SPI1_SR                     (*(volatile uint32_t *)(SPI1 + 0x08))
#define SPI1_DR                     (*(volatile uint32_t *)(SPI1 + 0x0c))

#define SPI_CR1_CLOCK_PHASE         (1 <<  0)
#define SPI_CR1_CLOCK_POLARITY      (1 <<  1)
#define SPI_CR1_MASTER              (1 <<  2)
#define SPI_CR1_BAUDRATE            (7 <<  3)
#define SPI_CR1_SPI_EN              (1 <<  6)
#define SPI_CR1_LSBFIRST            (1 <<  7)
#define SPI_CR1_SSI                 (1 <<  8)
#define SPI_CR1_SSM                 (1 <<  9)
#define SPI_CR1_16BIT_FORMAT        (1 << 11)
#define SPI_CR1_TX_CRC_NEXT         (1 << 12)
#define SPI_CR1_HW_CRC_EN           (1 << 13)
#define SPI_CR1_BIDIOE              (1 << 14)

#define SPI_CR2_SSOE                (1 <<  2)

#define SPI_SR_RX_NOTEMPTY          (1 <<  0)
#define SPI_SR_TX_EMPTY             (1 <<  1)
#define SPI_SR_BUSY                 (1 <<  7)

void spi_init(int polarity, int phase)
{
    SPI1_CR1 = SPI_CR1_MASTER | (5 << 3) | (polarity << 1) | (phase << 0);
    SPI1_CR2 |= SPI_CR2_SSOE;
    SPI1_CR1 |= SPI_CR1_SPI_EN;
}

uint8_t spi_read(void)
{
    volatile uint32_t reg;

    do {
        reg = SPI1_SR;
    } while(!(reg & SPI_SR_RX_NOTEMPTY));

    return (uint8_t)SPI1_DR;
}

void spi_write(const char byte)
{
    volatile uint32_t reg;

    do {
        reg = SPI1_SR;
    } while ((reg & SPI_SR_TX_EMPTY) == 0);

    SPI1_DR = byte;

    do {
        reg = SPI1_SR;
    } while ((reg & SPI_SR_TX_EMPTY) == 0);
}

