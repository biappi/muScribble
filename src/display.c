#include "display.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>

extern const unsigned char fb_font[256][8];

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

void display_init(void)
{
    int i;
    int k = 0;
    int row = 0;
    volatile int j;

    uint8_t dbuf2[64] = {0};

    int page = 0;
    int seg = 0;

    char test[] = "Hello, world!";

    display_send_cmd(SSD1306_DISPLAYOFF);
    display_send_cmd1(0xD6, 0x01);
    display_send_cmd(0xA1);
    display_send_cmd1(SSD1306_SETCONTRAST, 0xCF);
    display_send_cmd1(SSD1306_CHARGEPUMP,  0x14);
    display_send_cmd1(SSD1306_MEMORYMODE,  0x00);
    display_send_cmd1(SSD1306_SETCOMPINS, 0x12);
    display_send_cmd1(SSD1306_SETDISPLAYOFFSET, 0x40);

    display_send_cmd1(SSD1306_SETVCOMDETECT, 0x00);
    display_send_cmd1(SSD1306_SETMULTIPLEX, 63);
    display_send_cmd(SSD1306_COMSCANINC);
    display_send_cmd(SSD1306_DISPLAYALLON_RESUME);
    display_send_cmd(SSD1306_DISPLAYON);
    
    display_send_cmd2(SSD1306_PAGEADDR, 0, 0xFF);
    display_send_cmd2(SSD1306_COLUMNADDR, 0, WIDTH - 1);
    display_send_cmd1(SSD1306_SETSTARTLINE, 0);

    for (page = 0; page < 4; page++) {
        display_send_cmd2(SSD1306_PAGEADDR, page, 0xFF);

        for (seg= 0; seg < 32; seg++) {
            display_send_data(dbuf2, 8);
        }

        display_send_cmd1(SSD1306_SETSTARTLINE, row);
    }
    
    display_send_cmd1(SSD1306_SETSTARTLINE, 0);

    for (page = 0; page < 4; page++) {
        display_send_cmd2(SSD1306_PAGEADDR, page, 0xFF);
        for (seg= 0; seg < 32; seg++) {
            display_send_data(fb_font[test[k % sizeof(test)]], 8);
            k++;
        }

        display_send_cmd1(SSD1306_SETSTARTLINE, row);
    }
}

