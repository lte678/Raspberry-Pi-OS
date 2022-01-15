#include <kernel/register.h>
#include <kernel/print.h>
#include <kernel/types.h>

#define SDEMMC_BASE 0x3F300000

#define SDEMMC_ARG2 (SDEMMC_BASE + 0x0)
#define SDEMMC_BLKSIZECNT (SDEMMC_BASE + 0x4)
#define SDEMMC_ARG1 (SDEMMC_BASE + 0x8)
#define SDEMMC_CMDTM (SDEMMC_BASE + 0xC)
#define SDEMMC_RESP0 (SDEMMC_BASE + 0x10)
#define SDEMMC_RESP1 (SDEMMC_BASE + 0x14)
#define SDEMMC_RESP2 (SDEMMC_BASE + 0x18)
#define SDEMMC_RESP3 (SDEMMC_BASE + 0x1C)
#define SDEMMC_DATA (SDEMMC_BASE + 0x20)
#define SDEMMC_STATUS (SDEMMC_BASE + 0x24)
#define SDEMMC_CONTROL0 (SDEMMC_BASE + 0x28)
#define SDEMMC_CONTROL1 (SDEMMC_BASE + 0x2C)
#define SDEMMC_INTERRUPT (SDEMMC_BASE + 0x30)
#define SDEMMC_IRPT_MASK (SDEMMC_BASE + 0x34)
#define SDEMMC_IRPT_EN (SDEMMC_BASE + 0x38)
#define SDEMMC_CONTROL2 (SDEMMC_BASE + 0x3C)
#define SDEMMC_FORCE_IRPT (SDEMMC_BASE + 0x50)
#define SDEMMC_BOOT_TIMEOUT (SDEMMC_BASE + 0x70)
#define SDEMMC_DBG_SEL (SDEMMC_BASE + 0x74)
#define SDEMMC_EXRDFIFO_CFG (SDEMMC_BASE + 0x80)
#define SDEMMC_EXRDFIFO_EN (SDEMMC_BASE + 0x84)
#define SDEMMC_TUNE_STEP (SDEMMC_BASE + 0x88)
#define SDEMMC_TUNE_STEPS_STD (SDEMMC_BASE + 0x8c)
#define SDEMMC_TUNE_STEPS_DDR (SDEMMC_BASE + 0x90)
#define SDEMMC_SPI_INT_SPT (SDEMMC_BASE + 0xf0)
#define SDEMMC_SLOTISR_VER (SDEMMC_BASE + 0xfc)

#define SDMMC_SLEEP (1000000)

struct sd_cmd {
    unsigned char cmd_idx;
    unsigned char type;
    unsigned char is_data;
    unsigned char index_check_en;
    unsigned char crc_check_en;
    unsigned char response_type;
    unsigned char xfer_multi_block;
    unsigned char xfer_data_dir;
    unsigned char xfer_auto_cmd_en;
    unsigned char xfer_blkcnt_en;
};


static unsigned int sd_exec_cmd(struct sd_cmd cmd, unsigned int arg) {
    uint32_t cmd_word;
    cmd_word = 0;
    cmd_word |= (cmd.cmd_idx & 0b111111) << 24;
    cmd_word |= (cmd.type & 0b11) << 22;
    cmd_word |= (cmd.is_data & 0b1) << 21;
    cmd_word |= (cmd.index_check_en & 0b1) << 20;
    cmd_word |= (cmd.crc_check_en & 0b1) << 19;
    cmd_word |= (cmd.response_type & 0b11) << 16;
    cmd_word |= (cmd.xfer_multi_block & 0b1) << 5;
    cmd_word |= (cmd.xfer_data_dir & 0b1) << 4;
    cmd_word |= (cmd.xfer_auto_cmd_en & 0b11) << 2;
    cmd_word |= (cmd.xfer_blkcnt_en & 0b1) << 1;

    put32(SDEMMC_ARG1, arg);
    put32(SDEMMC_CMDTM, cmd_word);
    
    // Response stuff...
    unsigned int timeout = SDMMC_SLEEP;
    while(get32(SDEMMC_INTERRUPT) | 0x01) {
        timeout -= 1;
        if(!timeout) {
            return -1;
        }
    }
    // get32(SDEMMC_RESP0);

    return 0;
}

int sd_initialize() {
    struct sd_cmd cmd0;

    cmd0 = (struct sd_cmd){0};
    cmd0.cmd_idx = 0x0;
    cmd0.index_check_en = 1;
    cmd0.crc_check_en = 1;
    cmd0.response_type = 0x10;
    if(sd_exec_cmd(cmd0, 0) != 0) {
        uart_print("SD card initialization failed!\n");
        return -1;
    }

    uart_print("Initalized SD card!\n");
    return 0;
}

//static unsigned char sd_read_byte() {
//  
//}