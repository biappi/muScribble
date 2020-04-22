#include "display.h"
#include "platform-private.h"

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

void display_send_2x_width_character(char c) {
    const uint8_t *glyph = font[c];
    uint8_t doubled[8 * 2];

    for (int i = 0; i < 8; i++) {
        uint8_t line = glyph[i];

        uint8_t top_line = line;

        doubled[(i * 2)    ] = top_line;
        doubled[(i * 2) + 1] = top_line;
    }
    
    display_send_data(doubled, 16);
}

void display_send_2x_character_top(char c) {
    const uint8_t *glyph = font[c];
    uint8_t doubled[8 * 2];

    for (int i = 0; i < 8; i++) {
        uint8_t line = glyph[i];

        uint8_t top_line = 
            ((line & 0x01)     ) |
            ((line & 0x01) << 1) |
            ((line & 0x02) << 1) |
            ((line & 0x02) << 2) |
            ((line & 0x04) << 2) |
            ((line & 0x04) << 3) |
            ((line & 0x08) << 3) |
            ((line & 0x08) << 4) 
        ;

        doubled[(i * 2)    ] = top_line;
        doubled[(i * 2) + 1] = top_line;
    }
    
    display_send_data(doubled, 16);
}

void display_send_2x_character_bottom(char c) {
    const uint8_t *glyph = font[c];
    uint8_t doubled[8 * 2];

    for (int i = 0; i < 8; i++) {
        uint8_t line = glyph[i];

        uint8_t top_line = 
            ((line & 0x10) >> 4) | 
            ((line & 0x10) >> 3) |
            ((line & 0x20) >> 3) |
            ((line & 0x20) >> 2) |
            ((line & 0x40) >> 2) |
            ((line & 0x40) >> 1) |
            ((line & 0x80) >> 1) |
            ((line & 0x80)     ) 
        ;

        doubled[(i * 2)    ] = top_line;
        doubled[(i * 2) + 1] = top_line;
    }
    
    display_send_data(doubled, 16);
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

void display_select_all() {
    gpio_clear(GPIOA, DISP_CS_PORTA);
    gpio_clear(GPIOB, DISP_CS_PORTB);
}

void display_select_none() {
    gpio_set(GPIOA, DISP_CS_PORTA);
    gpio_set(GPIOB, DISP_CS_PORTB);
}

void display_select(display_selection_t s) {
    switch(s) {
        case display_selection_1:
            display_select_none();
            gpio_clear(GPIOA, DISP_CS1);
            break;

        case display_selection_2:
            display_select_none();
            gpio_clear(GPIOB, DISP_CS2);
            break;

        case display_selection_3:
            display_select_none();
            gpio_clear(GPIOB, DISP_CS3);
            break;

        case display_selection_4:
            display_select_none();
            gpio_clear(GPIOB, DISP_CS4);
            break;

        case display_selection_5:
            display_select_none();
            gpio_clear(GPIOB, DISP_CS5);
            break;

        case display_selection_6:
            display_select_none();
            gpio_clear(GPIOB, DISP_CS6);
            break;

        case display_selection_7:
            display_select_none();
            gpio_clear(GPIOB, DISP_CS7);
            break;

        case display_selection_8:
            display_select_none();
            gpio_clear(GPIOB, DISP_CS8);
            break;

        case display_selection_all:
            display_select_all();
            break;

        case display_selection_none:
            display_select_none();
            break;
    }
}

