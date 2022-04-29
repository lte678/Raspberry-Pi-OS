#pragma once

#define PERIPHERAL_INTERRUPT_CLOCK1  1
#define PERIPHERAL_INTERRUPT_CLOCK3  3
#define PERIPHERAL_INTERRUPT_USB     9
#define PERIPHERAL_INTERRUPT_AUX_INT 29
#define PERIPHERAL_INTERRUPT_I2C_SPI_SLV 43
#define PERIPHERAL_INTERRUPT_PWA0    45
#define PERIPHERAL_INTERRUPT_PWA1    46
#define PERIPHERAL_INTERRUPT_SMI     48
#define PERIPHERAL_INTERRUPT_GPIO0   49
#define PERIPHERAL_INTERRUPT_GPIO1   50
#define PERIPHERAL_INTERRUPT_GPIO2   51
#define PERIPHERAL_INTERRUPT_GPIO3   52
#define PERIPHERAL_INTERRUPT_I2C     53
#define PERIPHERAL_INTERRUPT_SPI     54
#define PERIPHERAL_INTERRUPT_PCM     55
#define PERIPHERAL_INTERRUPT_UART    57


// From vectors.s
extern void irq_init();

extern void enable_peripheral_interrupt(int interrupt_number);
extern void disable_peripheral_interrupt(int interrupt_number);