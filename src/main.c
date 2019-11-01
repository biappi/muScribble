#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "display.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

uint8_t logic_control_strip[116] = { 0 };

uint8_t nibble_char(uint8_t x) {
    return (x < 10) ? ('0' + x) : ('a' + (x - 10));
}

void byte_string(char * string, uint8_t byte) {
    string[0]  = nibble_char((byte & 0xf0) >> 4);
    string[1]  = nibble_char((byte & 0x0f)     );
}

void send_small_dump(const uint8_t * buf, size_t len) {
    //                                   1     1
    //                         01234567890123456
    static uint8_t string[] = "00 00 00 00 00  ";

    for (int i = 0; i < 5; i++) {
        string[(i * 3)    ] = '-';
        string[(i * 3) + 1] = '-';
    }

    for (int i = 0; i < MIN(len, 5); i++) {
        byte_string(string + (i * 3), buf[i]);
    }


    display_send_string(string);
}
enum usb_midi_packet_types {
    usb_midi_sysex_start_or_continue = 0x04,
    usb_midi_sysex_ends_single_byte  = 0x05,
    usb_midi_sysex_ends_two_bytes    = 0x06,
    usb_midi_sysex_ends_three_bytes  = 0x07,
};


struct buffer {
    size_t  len;
    uint8_t buf[255];
};

struct sysex_parser {
    enum {
        sysex_parser_idle,
        sysex_parser_accumulating,
        sysex_parser_ended,
    } state;

    struct buffer sysex;
};

void sysex_parse(struct sysex_parser * parser, const uint8_t * packet)
{
    const uint8_t
    cable_id    = (packet[0] & 0xf0) >> 0;

    const enum usb_midi_packet_types
    packet_type = (packet[0] & 0x0f);

    struct buffer
    *sysex      = &(parser->sysex);

    switch (packet_type) {
        case usb_midi_sysex_start_or_continue:
            switch (parser->state) {
                case sysex_parser_idle:
                case sysex_parser_ended:
                    sysex->len = 0;
                    break;

                case sysex_parser_accumulating:
                    break;
            }

            sysex->buf[sysex->len++] = packet[1];
            sysex->buf[sysex->len++] = packet[2];
            sysex->buf[sysex->len++] = packet[3];
            parser->state = sysex_parser_accumulating;
            break;

        case usb_midi_sysex_ends_single_byte:
            sysex->buf[sysex->len++] = packet[1];
            parser->state = sysex_parser_ended;
            break;

        case usb_midi_sysex_ends_two_bytes:
            sysex->buf[sysex->len++] = packet[1];
            sysex->buf[sysex->len++] = packet[2];
            parser->state = sysex_parser_ended;
            break;

        case usb_midi_sysex_ends_three_bytes:
            sysex->buf[sysex->len++] = packet[1];
            sysex->buf[sysex->len++] = packet[2];
            sysex->buf[sysex->len++] = packet[3];
            parser->state = sysex_parser_ended;
            break;
    }
}

void usb_midi_received_callback(const uint8_t * buf, size_t len)
{
    static struct sysex_parser parser = { 0 };

    for (int i = 0; i < len / 4; i++) {
        sysex_parse(&parser, buf + (i * 4));

        if (parser.state == sysex_parser_ended) {
            const uint8_t * midiData = parser.sysex.buf;

            parser.state = sysex_parser_idle;

            if (
                (midiData[0] == 0xf0) &&
                (midiData[5] == 0x12)
            ) {
                const uint8_t  offset =  midiData[6];
                const uint8_t *src    = &midiData[7];
                      uint8_t *dst    = (logic_control_strip + offset);

                while (*src != 0xf7) {
                    *dst++ = *src++;
                }

                display_goto_line(0);
                display_send_string(logic_control_strip);
                display_send_string("<<<");
            }
        }

    }
}

// - //


int main(void)
{
    platform_init();

    while (1) {
        platform_poll();
    }
}

