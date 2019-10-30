/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 * Copyright (C) 2015 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/audio.h>
#include <unicore-mx/usb/class/midi.h>

void usb_midi_received_callback(const uint8_t * buf, size_t len);

/*
 * All references in this file come from Universal Serial Bus Device Class
 * Definition for MIDI Devices, release 1.0.
 */

/*
 * Table B-1: MIDI Adapter Device Descriptor
 */
static const struct usb_device_descriptor dev_desc = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,    /* was 0x0110 in Table B-1 example descriptor */
	.bDeviceClass = 0,   /* device defined at interface level */
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x6666,  /* Prototype product vendor ID */
	.idProduct = 0x5119, /* dd if=/dev/random bs=2 count=1 | hexdump */
	.bcdDevice = 0x0100,
	.iManufacturer = 1,  /* index to string desc */
	.iProduct = 2,       /* index to string desc */
	.iSerialNumber = 0,
	.bNumConfigurations = 1
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
	.bLength = USB_DT_STRING_SIZE(10),
	.bDescriptorType = USB_DT_STRING,
	/* unicore-mx */
	.wData = {
		0x0075, 0x006e, 0x0069, 0x0063, 0x006f, 0x0072, 0x0065, 0x002d,
		0x006d, 0x0078
	}
};

static const struct usb_string_descriptor string_1 = {
	.bLength = USB_DT_STRING_SIZE(9),
	.bDescriptorType = USB_DT_STRING,
	/* MIDI Demo */
	.wData = {
		0x004d, 0x0049, 0x0044, 0x0049, 0x0020, 0x0044, 0x0065, 0x006d,
		0x006f
	}
};

/*
 * MIDI = मिडी
 * Demo, DEMO = नमूना
 */

static const struct usb_string_descriptor string_2 = {
	.bLength = USB_DT_STRING_SIZE(10),
	.bDescriptorType = USB_DT_STRING,
	/* यूनिकोर-मस */
	.wData = {
		0x092f, 0x0942, 0x0928, 0x093f, 0x0915, 0x094b, 0x0930, 0x002d,
		0x092e, 0x0938
	}
};

static const struct usb_string_descriptor string_3 = {
	.bLength = USB_DT_STRING_SIZE(10),
	.bDescriptorType = USB_DT_STRING,
	/* मिडी नमूना */
	.wData = {
		0x092e, 0x093f, 0x0921, 0x0940, 0x0020, 0x0928, 0x092e, 0x0942,
		0x0928, 0x093e
	}
};

static const struct usb_string_descriptor **string_data[2] = {
	(const struct usb_string_descriptor *[]){&string_0, &string_1},
	(const struct usb_string_descriptor *[]){&string_2, &string_3}
};

static const struct usbd_info_string string = {
	.lang_list = &string_lang_list,
	.count = 2,
	.data = string_data
};

static const struct __attribute__((packed)) {
	/*
	 * Table B-2: MIDI Adapter Configuration Descriptor
	 */
	struct usb_config_descriptor config;

	/*
	 * Table B-3: MIDI Adapter Standard AC Interface Descriptor
	 */
	struct usb_interface_descriptor audio_control_iface;

	/*
	 * Table B-4: MIDI Adapter Class-specific AC Interface Descriptor
	 */
	struct {
		struct usb_audio_header_descriptor_head header_head;
		struct usb_audio_header_descriptor_body header_body;
	} __attribute__((packed)) audio_control_functional_descriptors;

	/*
	 * Table B-5: MIDI Adapter Standard MS Interface Descriptor
	 */
	struct usb_interface_descriptor midi_streaming_iface;

	/*
	 * Class-specific MIDI streaming interface descriptor
	 */
	struct {
		struct usb_midi_header_descriptor header;
		struct usb_midi_in_jack_descriptor in_embedded;
		struct usb_midi_in_jack_descriptor in_external;
		struct usb_midi_out_jack_descriptor out_embedded;
		struct usb_midi_out_jack_descriptor out_external;
	} __attribute__((packed)) midi_streaming_functional_descriptors;

	struct {
		/*
		 * Standard endpoint descriptor
		 */
		struct usb_endpoint_descriptor bulk_endp;

		/*
		 * Midi specific endpoint descriptor.
		 */
		struct usb_midi_endpoint_descriptor midi_bulk_endp;
	} __attribute__((packed)) out, in;
} config_desc = {
	.config = {
		.bLength = USB_DT_CONFIGURATION_SIZE,
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(config_desc),
		.bNumInterfaces = 2, /* control and data */
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0x80, /* bus powered */
		.bMaxPower = 0x32
	},

	.audio_control_iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
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
			.baInterfaceNr = 0x01,
		},
	},

	.midi_streaming_iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 1,
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
			.bEndpointAddress = 0x01,
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
			.bEndpointAddress = 0x81,
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

const struct usbd_info usb_midi_device_info= {
	.device = {
		.desc = &dev_desc,
		.string = &string
	},

	.config = {{
		.desc = (const struct usb_config_descriptor *) &config_desc,
		.string = &string
	}}
};

/* SysEx identity message, preformatted with correct USB framing information */
const uint8_t sysex_identity[] = {
	0x04,	/* USB Framing (3 byte SysEx) */
	0xf0,	/* SysEx start */
	0x7e,	/* non-realtime */
	0x00,	/* Channel 0 */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x7d,	/* Educational/prototype manufacturer ID */
	0x66,	/* Family code (byte 1) */
	0x66,	/* Family code (byte 2) */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x51,	/* Model number (byte 1) */
	0x19,	/* Model number (byte 2) */
	0x00,	/* Version number (byte 1) */
	0x04,	/* USB Framing (3 byte SysEx) */
	0x00,	/* Version number (byte 2) */
	0x01,	/* Version number (byte 3) */
	0x00,	/* Version number (byte 4) */
	0x05,	/* USB Framing (1 byte SysEx) */
	0xf7,	/* SysEx end */
	0x00,	/* Padding */
	0x00,	/* Padding */
};

static bool error_recoverable(usbd_transfer_status status)
{
	switch (status) {
	case USBD_ERR_TIMEOUT:
	case USBD_ERR_IO:
	case USBD_ERR_BABBLE:
	case USBD_ERR_DTOG:
	case USBD_ERR_SHORT_PACKET:
	case USBD_ERR_OVERFLOW:
	return true;

	case USBD_ERR_RES_UNAVAIL:
	case USBD_SUCCESS:
	case USBD_ERR_SIZE:
	case USBD_ERR_CONN:
	case USBD_ERR_INVALID:
	case USBD_ERR_CONFIG_CHANGE:
	case USBD_ERR_CANCEL:
	default:
	return false;
	}
}

static void resubmit_for_recoverable_error(usbd_device *usbd_dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		if (error_recoverable(status)) {
			usbd_transfer_submit(usbd_dev, transfer);
		}
	}
}

static void send_sysex_identify(usbd_device *usbd_dev)
{
	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = 0x81,
		.ep_size = 64,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = (void *) sysex_identity,
		.length = sizeof(sysex_identity),
		.flags = USBD_FLAG_NO_SUCCESS_CALLBACK,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = resubmit_for_recoverable_error
	};

	usbd_transfer_submit(usbd_dev, &transfer);
}

static void usbmidi_data_rx_cb(usbd_device *usbd_dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		return;
	}

	if (transfer->transferred) {

        usb_midi_received_callback(
            transfer->buffer,
            transfer->transferred
        );

        /* This implementation treats any message from the host as a SysEx
         * identity request. This works well enough providing the host
         * packs the identify request in a single 8 byte USB message.
         */

        // send_sysex_identify(usbd_dev);
	}

	/* Accept more data from host */
	usbd_transfer_submit(usbd_dev, transfer);

	// usbmidi_target_data_rx_cb();
}

void usb_midi_set_config(
    usbd_device *usbd_dev,
    const struct usb_config_descriptor *cfg
) {
	(void)cfg;

	usbd_ep_prepare(usbd_dev, 0x01, USBD_EP_BULK, 64, USBD_INTERVAL_NA,
					USBD_EP_NONE);
	usbd_ep_prepare(usbd_dev, 0x81, USBD_EP_BULK, 64, USBD_INTERVAL_NA,
					USBD_EP_NONE);

	static uint8_t buf[64];

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = 0x01,
		.ep_size = 64,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = buf,
		.length = sizeof(buf),
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = usbmidi_data_rx_cb,
	};

	usbd_transfer_submit(usbd_dev, &transfer);
}

static volatile usbd_urb_id button_event_urb_id = USBD_INVALID_URB_ID;

static void button_send_event_callback(usbd_device *usbd_dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		if (error_recoverable(status)) {
			button_event_urb_id = usbd_transfer_submit(usbd_dev, transfer);
			return;
		}
	}

	button_event_urb_id = USBD_INVALID_URB_ID;
}

static void button_send_event(usbd_device *usbd_dev, int pressed)
{
	static char buf[4] = { 0x08, /* USB framing: virtual cable 0, note on */
			0x80, /* MIDI command: note on, channel 1 */
			60,   /* Note 60 (middle C) */
			64,   /* "Normal" velocity */
	};

	buf[0] |= pressed;
	buf[1] |= pressed << 4;

	if (button_event_urb_id != USBD_INVALID_URB_ID) {
		/* already in progress */
		return;
	}

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = 0x81,
		.ep_size = 64,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = buf,
		.length = sizeof(buf),
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = button_send_event_callback
	};

	button_event_urb_id = usbd_transfer_submit(usbd_dev, &transfer);
}

