#include "platform-private.h"
#include "spi.h"

void display_transport_reset(void)
{
    volatile int j;

    gpio_set(SPI_DISP_RESET_PORT, SPI_DISP_RESET_PIN);

    for(j = 0; j < 48000; j++)
        ;

    gpio_clear(SPI_DISP_RESET_PORT, SPI_DISP_RESET_PIN);

    for(j = 0; j < 480000; j++)
        ;

    gpio_set(SPI_DISP_RESET_PORT, SPI_DISP_RESET_PIN);
}

void display_transport_set_control(void)
{
    gpio_clear(SPI_DISP_DC_PORT, SPI_DISP_DC_PIN);
}

void display_transport_set_data(void)
{
    gpio_set(SPI_DISP_DC_PORT, SPI_DISP_DC_PIN);
}

void display_transport_write(char byte)
{
    spi_write(byte);
    for (volatile int d = 0; d < 100; d++) ;
}

