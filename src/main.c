#include <stdlib.h>
#include <string.h>

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/otg_hs.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>

#include "usb-descriptors.h"

static void cdcacm_control_request(usbd_device *usbd_dev, uint8_t ep,
                const struct usb_setup_data *setup_data)
{
    (void) ep; /* assuming ep == 0 */

    const uint8_t bmReqMask = USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT;
    const uint8_t bmReqVal = USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE;

    if ((setup_data->bmRequestType & bmReqMask) != bmReqVal) {
        /* Pass on to usb stack internal */
        usbd_ep0_setup(usbd_dev, setup_data);
        return;
    }

    switch (setup_data->bRequest) {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
        /*
         * This Linux cdc_acm driver requires this to be implemented
         * even though it's optional in the CDC spec, and we don't
         * advertise it in the ACM functional descriptor.
         */
        usbd_ep0_transfer(usbd_dev, setup_data, NULL, 0, NULL);
    return;
    case USB_CDC_REQ_SET_LINE_CODING:
        if (setup_data->wLength < sizeof(struct usb_cdc_line_coding)) {
            break;
        }

        static uint8_t tmp_buf[sizeof(struct usb_cdc_line_coding)];

        /* Just read what ever host is sending and do the status stage */
        usbd_ep0_transfer(usbd_dev, setup_data, tmp_buf,
            setup_data->wLength, NULL);
    return;
    }

    usbd_ep0_stall(usbd_dev);
}

static uint8_t bulk_buf[64];

static void cdcacm_data_rx_cb(usbd_device *usbd_dev,
    const usbd_transfer *_transfer, usbd_transfer_status status,
    usbd_urb_id urb_id);

static void cdcacm_data_tx_cb(usbd_device *usbd_dev,
    const usbd_transfer *_transfer, usbd_transfer_status status,
    usbd_urb_id urb_id);

static void rx_from_host(usbd_device *usbd_dev)
{
    const usbd_transfer transfer = {
        .ep_type = USBD_EP_BULK,
        .ep_addr = 0x01,
        .ep_size = 64,
        .ep_interval = USBD_INTERVAL_NA,
        .buffer = bulk_buf,
        .length = 64,
        .flags = USBD_FLAG_SHORT_PACKET,
        .timeout = USBD_TIMEOUT_NEVER,
        .callback = cdcacm_data_rx_cb
    };

    usbd_transfer_submit(usbd_dev, &transfer);
}

static void tx_to_host(usbd_device *usbd_dev, void *data, size_t len)
{
    const usbd_transfer transfer = {
        .ep_type = USBD_EP_BULK,
        .ep_addr = 0x82,
        .ep_size = 64,
        .ep_interval = USBD_INTERVAL_NA,
        .buffer = data,
        .length = len,
        .flags = USBD_FLAG_SHORT_PACKET,
        .timeout = USBD_TIMEOUT_NEVER,
        .callback = cdcacm_data_tx_cb
    };

    usbd_transfer_submit(usbd_dev, &transfer);
}

static void cdcacm_data_tx_cb(usbd_device *usbd_dev,
    const usbd_transfer *transfer, usbd_transfer_status status,
    usbd_urb_id urb_id)
{
    (void) urb_id;
    (void) transfer;

    if (status == USBD_SUCCESS) {
        rx_from_host(usbd_dev);
    }
}

static void cdcacm_data_rx_cb(usbd_device *usbd_dev,
    const usbd_transfer *transfer, usbd_transfer_status status,
    usbd_urb_id urb_id)
{
    (void) urb_id;

    if (status == USBD_SUCCESS) {
        if (transfer->transferred) {
            tx_to_host(usbd_dev, transfer->buffer, transfer->transferred);
        } else {
            usbd_transfer_submit(usbd_dev, transfer); /* re-submit */
        }
    }
}

static void cdcacm_set_config(usbd_device *usbd_dev,
                const struct usb_config_descriptor *cfg)
{
    (void)cfg;
    usbd_ep_prepare(usbd_dev, 0x01, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
    usbd_ep_prepare(usbd_dev, 0x82, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
    usbd_ep_prepare(usbd_dev, 0x83, USBD_EP_INTERRUPT, 16, USBD_INTERVAL_NA, USBD_EP_NONE);

    rx_from_host(usbd_dev);
}


// - //

static void cdcacm_target_init() {
    rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_48MHZ]);


    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    // rcc_periph_clock_enable(RCC_OTGFS);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
         GPIO9 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);
}

int main(void)
{
    usbd_device *usbd_dev;

    cdcacm_target_init();

    usbd_dev = usbd_init(USBD_STM32_OTG_HS, NULL, &info);

    usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);
    usbd_register_setup_callback(usbd_dev, cdcacm_control_request);

    int porcoddio = 0;

    while (1) {
        usbd_poll(usbd_dev, 0);
        porcoddio++;

        if (porcoddio > 1000) {
            const char * PORCODDIO = "porcoddio\r\n";
            tx_to_host(usbd_dev, (void *)PORCODDIO, sizeof(PORCODDIO));
            porcoddio = 0;
        }
    }
}

