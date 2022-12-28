#include <kernel/mmap.h>
#include <kernel/print.h>
#include <kernel/chardev.h>
#include <kernel/register.h>
#include <kernel/pagetable.h>

#include "uart.h"

extern void dummy ( unsigned int );

#define UART_PHYSICAL_ADDR 0x3F200000ul
#define UART_MAPPING_SIZE  0x00020000ul
static uint64_t uart_baseaddr = UART_PHYSICAL_ADDR;

#define GPFSEL1         (uart_baseaddr + 0x00004ul)
#define GPSET0          (uart_baseaddr + 0x0001Cul)
#define GPCLR0          (uart_baseaddr + 0x00028ul)
#define GPPUD           (uart_baseaddr + 0x00094ul)
#define GPPUDCLK0       (uart_baseaddr + 0x00098ul)

#define AUX_ENABLES     (uart_baseaddr + 0x15004ul)
#define AUX_MU_IO_REG   (uart_baseaddr + 0x15040ul)
#define AUX_MU_IER_REG  (uart_baseaddr + 0x15044ul)
#define AUX_MU_IIR_REG  (uart_baseaddr + 0x15048ul)
#define AUX_MU_LCR_REG  (uart_baseaddr + 0x1504Cul)
#define AUX_MU_MCR_REG  (uart_baseaddr + 0x15050ul)
#define AUX_MU_LSR_REG  (uart_baseaddr + 0x15054ul)
#define AUX_MU_MSR_REG  (uart_baseaddr + 0x15058ul)
#define AUX_MU_SCRATCH  (uart_baseaddr + 0x1505Cul)
#define AUX_MU_CNTL_REG (uart_baseaddr + 0x15060ul)
#define AUX_MU_STAT_REG (uart_baseaddr + 0x15064ul)
#define AUX_MU_BAUD_REG (uart_baseaddr + 0x15068ul)

//GPIO14  TXD0 and TXD1
//GPIO15  RXD0 and RXD1
//alt function 5 for uart1
//alt function 0 for uart0

//((250,000,000/115200)/8)-1 = 270


//GPIO14  TXD0 and TXD1
//GPIO15  RXD0 and RXD1

static unsigned char uart_recv ( void )
{
    while(1)
    {
        if(get32(AUX_MU_LSR_REG)&0x01) break;
    }
    return get32(AUX_MU_IO_REG) & 0xFF;
}


static void uart_send (unsigned char c)
{
    while(1)
    {
        if(get32(AUX_MU_LSR_REG)&0x20) break;
    }
    put32(AUX_MU_IO_REG, (unsigned int)c);
}


static int64_t uart_chardev_write(struct char_dev *dev, char *buf, int64_t count) {
    for(int64_t i = 0; i < count; i++) {
        if(buf[i] == '\n') {
            uart_send('\r');
            uart_send('\n');
        } else {
            uart_send(buf[i]);
        }
        
    }
    return count;
}

static int64_t uart_chardev_read(struct char_dev *dev, char *buf, int64_t count) {
    for(int64_t i = 0; i < count; i++) {
        buf[i] = uart_recv();
    }
    return count;
}

struct char_dev global_uart = {
    .read = uart_chardev_read,
    .write = uart_chardev_write,
    .driver_str = "BCM_UART"
};

unsigned int uart_lcr ( void )
{
    return(get32(AUX_MU_LSR_REG));
}


unsigned int uart_check ( void )
{
    if(get32(AUX_MU_LSR_REG)&0x01) return(1);
    return(0);
}


void uart_flush ( void )
{
    while(1)
    {
        if((get32(AUX_MU_LSR_REG)&0x100)==0) break;
    }
}


/**
 * @brief Initialized the hardware and uses a pre-mem identity memory mapping.
 * 
 */
void uart_pre_init ( void )
{
    unsigned int ra;

    put32(AUX_ENABLES,1);
    put32(AUX_MU_IER_REG,0);
    put32(AUX_MU_CNTL_REG,0);
    put32(AUX_MU_LCR_REG,3);
    put32(AUX_MU_MCR_REG,0);
    put32(AUX_MU_IER_REG,0);
    put32(AUX_MU_IIR_REG,0xC6);
    put32(AUX_MU_BAUD_REG,270);
    ra=get32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    ra&=~(7<<15); //gpio15
    ra|=2<<15;    //alt5
    put32(GPFSEL1,ra);
    put32(GPPUD,0);
    for(ra=0;ra<150;ra++) dummy(ra);
    put32(GPPUDCLK0,(1<<14)|(1<<15));
    for(ra=0;ra<150;ra++) dummy(ra);
    put32(GPPUDCLK0,0);
    put32(AUX_MU_CNTL_REG,3);

    print("UART peripheral initialized @ 0x{xl}\n", uart_baseaddr);
}

/**
 * @brief Initialize the memory mappings to move out of pre-mem
 * 
 */
int uart_init() {
    // No error checking!
    uint64_t new_uart_baseaddr = (uint64_t)mmap(UART_PHYSICAL_ADDR, UART_MAPPING_SIZE);
    if(!new_uart_baseaddr) {
        print("uart: error: failed to remap IO region\n");
        return -1;
    }
    uart_baseaddr = new_uart_baseaddr;
    print("UART peripheral remapped @ 0x{xl}\n", uart_baseaddr);
    return 0;
}