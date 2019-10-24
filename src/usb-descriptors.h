
static const struct usb_device_descriptor dev_desc = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
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

    // - //

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

    // - //

	struct usb_interface_descriptor audio_control_iface;

	struct {
		struct usb_audio_header_descriptor_head header_head;
		struct usb_audio_header_descriptor_body header_body;
	} __attribute__((packed)) audio_control_functional_descriptors;

	struct usb_interface_descriptor midi_streaming_iface;

	struct {
		struct usb_midi_header_descriptor header;
		struct usb_midi_in_jack_descriptor in_embedded;
		struct usb_midi_in_jack_descriptor in_external;
		struct usb_midi_out_jack_descriptor out_embedded;
		struct usb_midi_out_jack_descriptor out_external;
	} __attribute__((packed)) midi_streaming_functional_descriptors;

	struct {
		struct usb_endpoint_descriptor bulk_endp;
		struct usb_midi_endpoint_descriptor midi_bulk_endp;
	} __attribute__((packed)) out, in;


} config_desc = {
    .config = {
        .bLength = USB_DT_CONFIGURATION_SIZE,
        .bDescriptorType = USB_DT_CONFIGURATION,
        .wTotalLength = sizeof(config_desc),
        .bNumInterfaces = 4,
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

    // - //

	.audio_control_iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 2,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = USB_CLASS_AUDIO,
		.bInterfaceSubClass = USB_AUDIO_SUBCLASS_CONTROL,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	},

	.audio_control_functional_descriptors = {
		.header_head = {
			.bLength = sizeof(struct usb_audio_header_descriptor_head) +
					   1 * sizeof(struct usb_audio_header_descriptor_body),
			.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_AUDIO_TYPE_HEADER,
			.bcdADC = 0x0100,
			.wTotalLength =
					sizeof(struct usb_audio_header_descriptor_head) +
					1 * sizeof(struct usb_audio_header_descriptor_body),
			.binCollection = 1,
		},
		.header_body = {
			.baInterfaceNr = 0x02,
		},
	},

	.midi_streaming_iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 3,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_CLASS_AUDIO,
		.bInterfaceSubClass = USB_AUDIO_SUBCLASS_MIDISTREAMING,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	},

	.midi_streaming_functional_descriptors = {
		/* Table B-6: Midi Adapter Class-specific MS Interface Descriptor */
		.header = {
			.bLength = sizeof(struct usb_midi_header_descriptor),
			.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_MIDI_SUBTYPE_MS_HEADER,
			.bcdMSC = 0x0100,
			.wTotalLength =
				sizeof(struct usb_midi_header_descriptor) +
				sizeof(struct usb_midi_in_jack_descriptor) * 2 +
				sizeof(struct usb_midi_out_jack_descriptor) * 2,
		},

		/* Table B-7: MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
		.in_embedded = {
			.bLength = sizeof(struct usb_midi_in_jack_descriptor),
			.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_IN_JACK,
			.bJackType = USB_MIDI_JACK_TYPE_EMBEDDED,
			.bJackID = 0x01,
			.iJack = 0x00,
		},

		/* Table B-8: MIDI Adapter MIDI IN Jack Descriptor (External) */
		.in_external = {
			.bLength = sizeof(struct usb_midi_in_jack_descriptor),
			.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_IN_JACK,
			.bJackType = USB_MIDI_JACK_TYPE_EXTERNAL,
			.bJackID = 0x02,
			.iJack = 0x00,
		},

		/* Table B-9: MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
		.out_embedded = {
			.head = {
				.bLength = sizeof(struct usb_midi_out_jack_descriptor),
				.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
				.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_OUT_JACK,
				.bJackType = USB_MIDI_JACK_TYPE_EMBEDDED,
				.bJackID = 0x03,
				.bNrInputPins = 1,
			},
			.source[0] = {
				.baSourceID = 0x02,
				.baSourcePin = 0x01,
			},
			.tail = {
				.iJack = 0x00,
			}
		},

		/* Table B-10: MIDI Adapter MIDI OUT Jack Descriptor (External) */
		.out_external = {
			.head = {
				.bLength = sizeof(struct usb_midi_out_jack_descriptor),
				.bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
				.bDescriptorSubtype = USB_MIDI_SUBTYPE_MIDI_OUT_JACK,
				.bJackType = USB_MIDI_JACK_TYPE_EXTERNAL,
				.bJackID = 0x04,
				.bNrInputPins = 1,
			},
			.source[0] = {
				.baSourceID = 0x01,
				.baSourcePin = 0x01,
			},
			.tail = {
				.iJack = 0x00,
			}
		}
	},

	.out = {
		.bulk_endp = {
			/* Table B-11: MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 0x04,
			.bmAttributes = USB_ENDPOINT_ATTR_BULK,
			.wMaxPacketSize = 0x40,
			.bInterval = 0x00,
		},

		.midi_bulk_endp = {
			/* Table B-12: MIDI Adapter Class-specific Bulk OUT Endpoint
			 * Descriptor
			 */
			.head = {
				.bLength = sizeof(struct usb_midi_endpoint_descriptor),
				.bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT,
				.bDescriptorSubType = USB_MIDI_SUBTYPE_MS_GENERAL,
				.bNumEmbMIDIJack = 1,
			},
			.jack[0] = {
				.baAssocJackID = 0x01,
			},
		}
	},

	.in = {
		.bulk_endp = {
			/* Table B-11: MIDI Adapter Standard Bulk IN Endpoint Descriptor */
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 0x84,
			.bmAttributes = USB_ENDPOINT_ATTR_BULK,
			.wMaxPacketSize = 0x40,
			.bInterval = 0x00,
		},

		.midi_bulk_endp = {
			/* Table B-14: MIDI Adapter Class-specific Bulk IN Endpoint
			 * Descriptor
			 */
			.head = {
				.bLength = sizeof(struct usb_midi_endpoint_descriptor),
				.bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT,
				.bDescriptorSubType = USB_MIDI_SUBTYPE_MS_GENERAL,
				.bNumEmbMIDIJack = 1,
			},
			.jack[0] = {
				.baAssocJackID = 0x03,
			},
		}
	}

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
