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

uint8_t nibble_char(uint8_t x) {
    return (x < 10) ? ('0' + x) : ('a' + (x - 10));
}

void usb_midi_received_callback(const uint8_t * buf, size_t len) {
    //                                 1     1
    //                       01234567890123456
    static uint8_t string1[] = "* MIDI          ";
    static uint8_t string2[] = "00 00 00 00 00  ";

    uint8_t byte;
    
    byte = buf[0];
    string2[0]  = nibble_char((byte & 0xf0) >> 4);
    string2[1]  = nibble_char((byte & 0x0f)     );

    byte = buf[1];
    string2[3]  = nibble_char((byte & 0xf0) >> 4);
    string2[4]  = nibble_char((byte & 0x0f)     );

    byte = buf[2];
    string2[6] = nibble_char((byte & 0xf0) >> 4);
    string2[7] = nibble_char((byte & 0x0f)     );

    byte = buf[3];
    string2[9]  = nibble_char((byte & 0xf0) >> 4);
    string2[10] = nibble_char((byte & 0x0f)     );

    byte = buf[4];
    string2[12] = nibble_char((byte & 0xf0) >> 4);
    string2[13] = nibble_char((byte & 0x0f)     );

    display_send_string(string1);
    display_send_string(string2);
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

    display_send_string("* START         ");

    while (1) {
        usbd_poll(usbd_dev, 0);
        delay_counter++;

        if (delay_counter > 1000000) {
            static int i = 1;
            if (i == 7) {
                i = 0;
                display_send_empty_screen(); 
            }

            i++;

            display_send_string("* ALIVE         ");
            delay_counter = 0;
        }
    }
}

