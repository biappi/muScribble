#include <stdint.h>

#define WFI() __asm__ volatile ("wfi")

void platform_init(void);
void platform_poll(void);
uint32_t platform_jiffies(void);
