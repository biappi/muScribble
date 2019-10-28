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

{




}

{
}

{
}

{
}

// - //


int main(void)
{
    usbd_device *usbd_dev;

    {
        rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_48MHZ]);

        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOB);
    }

    {
        gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
             GPIO9 | GPIO11 | GPIO12);
        gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);
    }

    {
        usbd_dev = usbd_init(USBD_STM32_OTG_FS, NULL, &info);

        usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);
        usbd_register_setup_callback(usbd_dev, cdcacm_control_request);
    }

    int delay_counter = 0;

    while (1) {
        usbd_poll(usbd_dev, 0);
        delay_counter++;

        if (delay_counter > 1000000) {
            const char alive[] = "ALIVE 2 ]\r\n";
            cdcacm_submit_transmit(usbd_dev, (void *)alive, sizeof(alive));
            delay_counter = 0;
        }
    }
}

