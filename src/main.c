#include <stdlib.h>
#include <string.h>

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/otg_hs.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/usb/class/audio.h>
#include <unicore-mx/usb/class/midi.h>

#include "display.h"
#include "spi.h"

#define SPI1_PIN_AF                     5 /* Alt mode for SPI */

#define SPI_DISP_RESET              GPIO3 /* A3: reset line */
#define SPI_DISP_DC                 GPIO4 /* A4: Display Data/#C */
#define SPI1_CLOCK_PIN              GPIO5 /* A5 */
#define SPI1_MISO_PIN               GPIO6 /* A6 - disconnected */
#define SPI1_MOSI_PIN               GPIO7 /* A7 */

/* - */

const struct usbd_info usb_midi_device_info;

void usb_midi_set_config(
    usbd_device *usbd_dev,
    const struct usb_config_descriptor *cfg
);

// - //

void display_transport_reset(void)
{
    volatile int j;

    gpio_set(GPIOA, SPI_DISP_RESET);

    for(j = 0; j < 48000; j++)
        ;

    gpio_clear(GPIOA, SPI_DISP_RESET);

    for(j = 0; j < 480000; j++)
        ;

    gpio_set(GPIOA, SPI_DISP_RESET);
}

void display_transport_set_control(void)
{
    gpio_clear(GPIOA, SPI_DISP_DC);
}

void display_transport_set_data(void)
{
    gpio_set(GPIOA, SPI_DISP_DC);
}

void display_transport_write(char byte)
{
    spi_write(byte);
    for (volatile int d = 0; d < 1000; d++)
        ;
}


// - //


void usb_midi_received_callback(const uint8_t * buf, size_t len) {
    //                                1         2
    //                      012345678901234567890123456
    static char string[] = " * MIDI in 00000000 ";

    int t  = len;

    int t1 = (t      ) & 0xff;
    int t2 = (t >>  8) & 0xff;
    int t3 = (t >> 16) & 0xff;
    int t4 = (t >> 24) & 0xff;

    #define nibble_char(x) ((x < 10) ? '0' + x : 'a' + x)

    string[18] = nibble_char((t1 >> 0) & 0x0f);
    string[17] = nibble_char((t1 >> 4) & 0x0f);

    string[16] = nibble_char((t2 >> 0) & 0x0f);
    string[15] = nibble_char((t2 >> 4) & 0x0f);

    string[14] = nibble_char((t3 >> 0) & 0x0f);
    string[13] = nibble_char((t3 >> 4) & 0x0f);

    string[12] = nibble_char((t4 >> 0) & 0x0f);
    string[11] = nibble_char((t4 >> 4) & 0x0f);

    display_send_string(string);
}

// - //


int main(void)
{
    usbd_device *usbd_dev;

    {
        rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_48MHZ]);

        rcc_periph_clock_enable(RCC_SPI1);
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOB);
    }

    {
        gpio_mode_setup(
            GPIOA,
            GPIO_MODE_AF,
            GPIO_PUPD_PULLUP,
            GPIO9 | GPIO11 | GPIO12
        );

        gpio_set_af(
            GPIOA,
            GPIO_AF10,
            GPIO9 | GPIO11 | GPIO12
        );
    }

    {
        gpio_mode_setup(
            GPIOA,
            GPIO_MODE_OUTPUT,
            GPIO_PUPD_PULLUP,
            SPI_DISP_RESET | SPI_DISP_DC
        );

        gpio_mode_setup(
            GPIOA,
            GPIO_MODE_AF,
            GPIO_PUPD_PULLUP,
            SPI1_MOSI_PIN | SPI1_MISO_PIN | SPI1_CLOCK_PIN
        );

        gpio_set_af(
            GPIOA,
            SPI1_PIN_AF,
            SPI1_MOSI_PIN | SPI1_MISO_PIN | SPI1_CLOCK_PIN
        );
    }

    {
        spi_init(0, 0);
        display_transport_reset();
        display_init();
    }

    {
        usbd_dev = usbd_init(USBD_STM32_OTG_FS, NULL, &usb_midi_device_info);

        usbd_register_set_config_callback(usbd_dev, usb_midi_set_config);
    }

    int delay_counter = 0;

    display_init();

    while (1) {
        usbd_poll(usbd_dev, 0);
        delay_counter++;

        if (delay_counter > 1000000) {
            const char alive[] = "ALIVE 2 ]\r\n";
            display_send_string("* ALIVE ");
            delay_counter = 0;
        }
    }
}

