#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/otg_hs.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/usb/class/audio.h>
#include <unicore-mx/usb/class/midi.h>


#define SPI1_PIN_AF                     5 /* Alt mode for SPI */

#define SPI_DISP_RESET              GPIO3 /* A3: reset line */
#define SPI_DISP_DC                 GPIO4 /* A4: Display Data/#C */
#define SPI1_CLOCK_PIN              GPIO5 /* A5 */
#define SPI1_MISO_PIN               GPIO6 /* A6 - disconnected */
#define SPI1_MOSI_PIN               GPIO7 /* A7 */


