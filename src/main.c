#include <stdlib.h>
#include <string.h>

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/otg_hs.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/usb/class/audio.h>
#include <unicore-mx/usb/class/midi.h>

#include "usb-descriptors.h"
#include "display.h"
#include "spi.h"

#define SPI1_PIN_AF                     5 /* Alt mode for SPI */

#define SPI_DISP_RESET              GPIO3 /* A3: reset line */
#define SPI_DISP_DC                 GPIO4 /* A4: Display Data/#C */
#define SPI1_CLOCK_PIN              GPIO5 /* A5 */
#define SPI1_MISO_PIN               GPIO6 /* A6 - disconnected */
#define SPI1_MOSI_PIN               GPIO7 /* A7 */

/* - */

void cdcacm_control_request(
    usbd_device *usbd_dev,
    uint8_t ep,
    const struct usb_setup_data *setup_data
);

void cdcacm_submit_transmit(
    usbd_device *usbd_dev,
    void *data,
    size_t len
);

void cdcacm_submit_receive(usbd_device *usbd_dev);

void usb_midi_submit_receive(usbd_device *usbd_dev);

/* - */

static void cdcacm_set_config(usbd_device *usbd_dev,
                const struct usb_config_descriptor *cfg)
{
    (void)cfg;

    {
        usbd_ep_prepare(usbd_dev, 0x01, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
        usbd_ep_prepare(usbd_dev, 0x82, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
        usbd_ep_prepare(usbd_dev, 0x83, USBD_EP_INTERRUPT, 16, USBD_INTERVAL_NA, USBD_EP_NONE);

        cdcacm_submit_receive(usbd_dev);
    }

    {
        usbd_ep_prepare(usbd_dev, 0x04, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
        usbd_ep_prepare(usbd_dev, 0x84, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);

        usb_midi_submit_receive(usbd_dev);
    }

}

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
        usbd_dev = usbd_init(USBD_STM32_OTG_FS, NULL, &info);

        usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);
        usbd_register_setup_callback(usbd_dev, cdcacm_control_request);
    }

    int delay_counter = 0;

    display_init();

    while (1) {
        usbd_poll(usbd_dev, 0);
        delay_counter++;

        if (delay_counter > 1000000) {
            const char alive[] = "ALIVE 2 ]\r\n";
            cdcacm_submit_transmit(usbd_dev, (void *)alive, sizeof(alive));
            display_send_string("* ALIVE ");
            delay_counter = 0;
        }
    }
}

