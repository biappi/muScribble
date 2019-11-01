#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "display.h"

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

    int delay_counter = 0;

    platform_init();

    display_send_string("* START         ");

    while (1) {
        platform_poll();
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

