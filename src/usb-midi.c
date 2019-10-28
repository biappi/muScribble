#include <unicore-mx/stm32/otg_hs.h>
#include <unicore-mx/usb/class/audio.h>
#include <unicore-mx/usb/class/midi.h>

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
		.ep_addr = 0x84,
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

void usbmidi_data_rx_cb(usbd_device *usbd_dev,
		const usbd_transfer *transfer, usbd_transfer_status status,
		usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBD_SUCCESS) {
		return;
	}

	/* This implementation treats any message from the host as a SysEx
	 * identity request. This works well enough providing the host
	 * packs the identify request in a single 8 byte USB message.
	 */
	if (transfer->transferred) {
// 		send_sysex_identify(usbd_dev);

        //                              1         2
        //                    012345678901234567890123456
        static char string[] = "MIDI  ] midi len 00000000\r\b";

        int t  = transfer->transferred;

        int t1 = (t      ) & 0xff;
        int t2 = (t >>  8) & 0xff;
        int t3 = (t >> 16) & 0xff;
        int t4 = (t >> 24) & 0xff;

        #define nibble_char(x) ((x < 10) ? '0' + x : 'a' + x)

        string[24] = nibble_char((t1 >> 0) & 0x0f);
        string[23] = nibble_char((t1 >> 4) & 0x0f);

        string[22] = nibble_char((t2 >> 0) & 0x0f);
        string[21] = nibble_char((t2 >> 4) & 0x0f);

        string[20] = nibble_char((t3 >> 0) & 0x0f);
        string[19] = nibble_char((t3 >> 4) & 0x0f);

        string[18] = nibble_char((t4 >> 0) & 0x0f);
        string[17] = nibble_char((t4 >> 4) & 0x0f);
	}

	/* Accept more data from host */
	usbd_transfer_submit(usbd_dev, transfer);

	// usbmidi_target_data_rx_cb();
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

static void send_test_note_on(usbd_device *usbd_dev, int pressed)
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
		.ep_addr = 0x84,
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

void usb_midi_submit_receive(usbd_device *usbd_dev)
{
        static uint8_t buf[64];

        const usbd_transfer transfer = {
            .ep_type = USBD_EP_BULK,
            .ep_addr = 0x04,
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

