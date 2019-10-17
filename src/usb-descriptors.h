
static const struct usb_device_descriptor dev_desc = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = USB_CLASS_CDC,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0483,
    .idProduct = 0x5740,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const struct __attribute__((packed)) {
    struct usb_config_descriptor config;
    const struct usb_interface_descriptor comm_iface;
    const struct {
        struct usb_cdc_header_descriptor header;
        struct usb_cdc_call_management_descriptor call_mgmt;
        struct usb_cdc_acm_descriptor acm;
        struct usb_cdc_union_descriptor cdc_union;
    } cdcacm_functional_descriptors;
    struct usb_endpoint_descriptor comm_endp;

    struct usb_interface_descriptor data_iface;
    struct usb_endpoint_descriptor data_endp[2];
} config_desc = {
    .config = {
        .bLength = USB_DT_CONFIGURATION_SIZE,
        .bDescriptorType = USB_DT_CONFIGURATION,
        .wTotalLength = sizeof(config_desc),
        .bNumInterfaces = 2,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = 0x80,
        .bMaxPower = 0x32
    },

    .comm_iface = {
        .bLength = USB_DT_INTERFACE_SIZE,
        .bDescriptorType = USB_DT_INTERFACE,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = USB_CLASS_CDC,
        .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
        .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
        .iInterface = 0,
    },

    .cdcacm_functional_descriptors = {
        .header = {
            .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
            .bcdCDC = 0x0110,
        },
        .call_mgmt = {
            .bFunctionLength =
                sizeof(struct usb_cdc_call_management_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
            .bmCapabilities = 0,
            .bDataInterface = 1,
        },
        .acm = {
            .bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = USB_CDC_TYPE_ACM,
            .bmCapabilities = 0,
        },
        .cdc_union = {
            .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
            .bDescriptorType = CS_INTERFACE,
            .bDescriptorSubtype = USB_CDC_TYPE_UNION,
            .bControlInterface = 0,
            .bSubordinateInterface0 = 1,
         },
    },

    /*
     * This notification endpoint isn't implemented. According to CDC spec its
     * optional, but its absence causes a NULL pointer dereference in Linux
     * cdc_acm driver.
     */
    .comm_endp = {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x83,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = 16,
        .bInterval = 255,
    },

    .data_iface = {
        .bLength = USB_DT_INTERFACE_SIZE,
        .bDescriptorType = USB_DT_INTERFACE,
        .bInterfaceNumber = 1,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = USB_CLASS_DATA,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface = 0,
    },

    .data_endp = {{
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x01,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    }, {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x82,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    }},
};

static const struct usb_string_descriptor string_lang_list = {
    .bLength = USB_DT_STRING_SIZE(2),
    .bDescriptorType = USB_DT_STRING,
    .wData = {
        USB_LANGID_ENGLISH_UNITED_STATES,
        USB_LANGID_HINDI
    }
};

/* string descriptor string_[0..5] generated using usb-string.py */

static const struct usb_string_descriptor string_0 = {
    .bLength = USB_DT_STRING_SIZE(16),
    .bDescriptorType = USB_DT_STRING,
    /* Mad Resistor LLP */
    .wData = {
        0x004d, 0x0061, 0x0064, 0x0020, 0x0052, 0x0065, 0x0073, 0x0069,
        0x0073, 0x0074, 0x006f, 0x0072, 0x0020, 0x004c, 0x004c, 0x0050
    }
};


static const struct usb_string_descriptor string_1 = {
    .bLength = USB_DT_STRING_SIZE(12),
    .bDescriptorType = USB_DT_STRING,
    /* CDC-ACM Demo */
    .wData = {
        0x0043, 0x0044, 0x0043, 0x002d, 0x0041, 0x0043, 0x004d, 0x0020,
        0x0044, 0x0065, 0x006d, 0x006f
    }
};

static const struct usb_string_descriptor string_2 = {
    .bLength = USB_DT_STRING_SIZE(4),
    .bDescriptorType = USB_DT_STRING,
    /* DEMO */
    .wData = {
        0x0044, 0x0045, 0x004d, 0x004f
    }
};

static const struct usb_string_descriptor **string_data[2] = {
    (const struct usb_string_descriptor *[]){&string_0, &string_1, &string_2},
    (const struct usb_string_descriptor *[]){&string_0, &string_1, &string_2},
};

static const struct usbd_info_string string = {
    .lang_list = &string_lang_list,
    .count = 3,
    .data = string_data
};

const struct usbd_info info = {
    .device = {
        .desc = &dev_desc,
        .string = &string
    },

    .config = {{
        .desc = (const struct usb_config_descriptor *) &config_desc,
        .string = &string
    }}
};
