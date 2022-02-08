#include <kernel/gpio.h>
#include <kernel/register.h>
#include <kernel/types.h>
#include <kernel/delay.h>
#include <kernel/print.h>

#define GPIO_GPFSEL0   (0x3F200000)
#define GPIO_GPHEN0    (0x3F200064)
#define GPIO_GPHEN1    (0x3F200068)
#define GPIO_GPPUD     (0x3F200094)
#define GPIO_GPPUDCLK0 (0x3F200098)
#define GPIO_GPPUDCLK1 (0x3F20009C)


static int gpio_valid(unsigned char gpio) {
    if(gpio > 53) return 0;
    return 1;
}

int gpio_set_mode(unsigned char gpio, unsigned char mode) {
    // Check if in range
    if(!gpio_valid(gpio)) return -1;
    if(mode & ~0b111) return -1;

    unsigned int gpfsel_n = GPIO_GPFSEL0 + 4 * (gpio / 10);
    uint32_t reg = get32(gpfsel_n);
    // Clear bits
    reg &= ~(0b111 << (gpio % 10) * 3);
    // Set bits
    reg |= mode << (gpio % 10) * 3;
    put32(gpfsel_n, reg);
    
    return 0;
}

int gpio_set_pullup(unsigned char gpio, unsigned char mode) {
    // Check if in range
    if(!gpio_valid(gpio)) return -1;
    if(mode & ~0b11) return -1;

    put32(GPIO_GPPUD, (uint32_t)mode);
    // According to BCM2837 documenation
    wait_cycles(150);
    // For GPIOs 0 - 31:  GPIO_GPPUDCLK0
    // For GPIOs 32 - 53: GPIO_GPPUDCLK1
    if(gpio < 32) {
        put32(GPIO_GPPUDCLK0, 1<<gpio);
        wait_cycles(150);
        put32(GPIO_GPPUD, 0);
        put32(GPIO_GPPUDCLK0, 0);
    } else {
        put32(GPIO_GPPUDCLK1, 1<<(gpio - 32));
        wait_cycles(150);
        put32(GPIO_GPPUD, 0);
        put32(GPIO_GPPUDCLK1, 0);
    }
    return 0;
}

int gpio_set_high_detect(unsigned char gpio, unsigned char enable) {
    // Check if in range
    if(!gpio_valid(gpio)) return -1;
    
    unsigned int addr;
    uint32_t reg;
    if(gpio < 32) {  
        addr = GPIO_GPHEN0;
    } else {
        addr = GPIO_GPHEN1;
        gpio -= 32;
    }
    // Set bit if enabled, clear otherwise
    reg = get32(addr);
    if(enable) {
        reg |= (1<<gpio);
    } else {
        reg &= ~(1<<gpio);
    }
    put32(addr, reg);
    return 0;
}