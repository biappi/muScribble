const struct usbd_info usb_midi_device_info;

void usb_midi_set_config(
    usbd_device *usbd_dev,
    const struct usb_config_descriptor *cfg
);

