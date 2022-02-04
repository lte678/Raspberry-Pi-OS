#define GPIO_MODE_INPUT 0b000
#define GPIO_MODE_OUTPUT 0b001
#define GPIO_MODE_ALT0 0b100
#define GPIO_MODE_ALT1 0b101
#define GPIO_MODE_ALT2 0b110
#define GPIO_MODE_ALT3 0b111
#define GPIO_MODE_ALT4 0b011
#define GPIO_MODE_ALT5 0b010

#define GPIO_PULLUP_DISABLED 0b00
#define GPIO_PULLUP_DOWN 0b01
#define GPIO_PULLUP_UP 0b10

int gpio_set_mode(unsigned char gpio, unsigned char mode);
int gpio_set_pullup(unsigned char gpio, unsigned char mode);
int gpio_set_high_detect(unsigned char gpio, unsigned char enable);