#include "display.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>

extern const unsigned char font[256][8];

#define WIDTH 128
#define PIXEL_HEIGHT 64
#define HEIGHT (PIXEL_HEIGHT / 8)

#define SSD1306_MEMORYMODE                           0x20
#define SSD1306_COLUMNADDR                           0x21
#define SSD1306_PAGEADDR                             0x22
#define SSD1306_SETCONTRAST                          0x81
#define SSD1306_CHARGEPUMP                           0x8D
#define SSD1306_SEGREMAP                             0xA0
#define SSD1306_DISPLAYALLON_RESUME                  0xA4
#define SSD1306_DISPLAYALLON                         0xA5
#define SSD1306_NORMALDISPLAY                        0xA6
#define SSD1306_INVERTDISPLAY                        0xA7
#define SSD1306_SETMULTIPLEX                         0xA8
#define SSD1306_DISPLAYOFF                           0xAE
#define SSD1306_DISPLAYON                            0xAF
#define SSD1306_COMSCANINC                           0xC0
#define SSD1306_COMSCANDEC                           0xC8
#define SSD1306_SETDISPLAYOFFSET                     0xD3
#define SSD1306_SETDISPLAYCLOCKDIV                   0xD5
#define SSD1306_SETPRECHARGE                         0xD9
#define SSD1306_SETCOMPINS                           0xDA
#define SSD1306_SETVCOMDETECT                        0xDB
#define SSD1306_SETLOWCOLUMN                         0x00
#define SSD1306_SETHIGHCOLUMN                        0x10
#define SSD1306_SETSTARTLINE                         0x40
#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A
#define SSD1306_DEACTIVATE_SCROLL                    0x2E
#define SSD1306_ACTIVATE_SCROLL                      0x2F
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3

static void display_send_data(const uint8_t *buf, const size_t len)
{
    /* TODO: CS */

    display_transport_set_data();

    for (int i = 0; i < len; i++)
        display_transport_write(buf[i]);
}

static inline void display_send_cmd(uint8_t cmd)
{
    display_transport_set_control();
    display_transport_write(cmd);
}

static inline void display_send_cmd1(uint8_t cmd, uint8_t arg1)
{
    display_transport_set_control();
    display_transport_write(cmd);
    display_transport_write(arg1);
}

static inline void display_send_cmd2(uint8_t cmd, uint8_t arg1, uint8_t arg2)
{
    display_transport_set_control();
    display_transport_write(cmd);
    display_transport_write(arg1);
    display_transport_write(arg2);
}

void display_send_character(char c) {
    display_send_data(font[c], 8);
}

void display_send_string(const char *string) {
    while (*string != 0)
        display_send_character(*string++);
}

void display_send_empty_line(void) {
    uint8_t test[32] = {0x00};
    display_send_data(test, sizeof(test));
    display_send_data(test, sizeof(test));
    display_send_data(test, sizeof(test));
    display_send_data(test, sizeof(test));
}

void display_send_empty_screen(void) {
    display_goto_line_column(0, 0);
    for (int i = 0; i < 8; i++)
        display_send_empty_line();
}

void display_goto_line_column(int line, int column) {
    display_send_cmd2(SSD1306_PAGEADDR, line & 0x0f, 0xff);
    display_send_cmd2(SSD1306_COLUMNADDR, column & 0x7f, WIDTH - 1);
}

void display_init(void) {
    display_send_cmd(SSD1306_DISPLAYOFF);
    display_send_cmd1(SSD1306_CHARGEPUMP, 0x14);
    display_send_cmd1(SSD1306_MEMORYMODE, 0x00);
    display_send_empty_screen();
    display_send_cmd(SSD1306_DISPLAYON);
}

