#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/otg_hs.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/usb/class/audio.h>
#include <unicore-mx/usb/class/midi.h>


#define SPI1_PIN_AF                     5 /* Alt mode for SPI */

#define SPI1_CLOCK_PIN              GPIO5 /* A5 */
#define SPI1_MISO_PIN               GPIO6 /* A6 - disconnected */
#define SPI1_MOSI_PIN               GPIO7 /* A7 */


#define SPI_DISP_RESET_PORT         GPIOB
#define SPI_DISP_RESET_PIN          GPIO0

#define SPI_DISP_DC_PORT            GPIOB
#define SPI_DISP_DC_PIN             GPIO1

// Port A
#define DISP_CS1                    GPIO15

// Port B
#define DISP_CS2                    GPIO3
#define DISP_CS3                    GPIO4
#define DISP_CS4                    GPIO5
#define DISP_CS5                    GPIO6
#define DISP_CS6                    GPIO7
#define DISP_CS7                    GPIO8
#define DISP_CS8                    GPIO9

#define DISP_CS_PORTA DISP_CS1
#define DISP_CS_PORTB ( \
    DISP_CS2 | \
    DISP_CS3 | \
    DISP_CS4 | \
    DISP_CS5 | \
    DISP_CS6 | \
    DISP_CS7 | \
    DISP_CS8   \
)

