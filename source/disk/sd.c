#include <kernel/register.h>
#include <kernel/print.h>
#include <kernel/types.h>

#define SDEMMC_BASE 0x3F300000

/*
* SDEMMC Registers
*/
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

/*
* SDEMMC Interrupt flags
*/
#define SD_INTERRUPT_ACMD_ERR (1<<24)
#define SD_INTERRUPT_DEND_ERR (1<<22)
#define SD_INTERRUPT_DCRC_ERR (1<<21)
#define SD_INTERRUPT_DTO_ERR (1<<20)
#define SD_INTERRUPT_CBAD_ERR (1<<19)
#define SD_INTERRUPT_CEND_ERR (1<<18)
#define SD_INTERRUPT_CCRC_ERR (1<<17)
#define SD_INTERRUPT_CTO_ERR (1<<16)
#define SD_INTERRUPT_ERR (1<<15)

#define SDMMC_SLEEP (10000000)

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

static void sd_print_err() {
    unsigned int err = get32(SDEMMC_INTERRUPT);
    uart_print("SD card error, INTERRUPT: 0x");
    print_hex_uint32(err);
    uart_print("\n");
    // Bit details:
    uart_print("  ACMD_ERR   ");
    print_int((err & SD_INTERRUPT_ACMD_ERR) != 0);
    uart_print("\n");
    uart_print("  DEND_ERR   ");
    print_int((err & SD_INTERRUPT_DEND_ERR) != 0);
    uart_print("\n");
    uart_print("  DCRC_ERR   ");
    print_int((err & SD_INTERRUPT_DCRC_ERR) != 0);
    uart_print("\n");
    uart_print("  DTO_ERR    ");
    print_int((err & SD_INTERRUPT_DTO_ERR) != 0);
    uart_print("\n");
    uart_print("  CBAD_ERR   ");
    print_int((err & SD_INTERRUPT_CBAD_ERR) != 0);
    uart_print("\n");
    uart_print("  CEND_ERR   ");
    print_int((err & SD_INTERRUPT_CEND_ERR) != 0);
    uart_print("\n");
    uart_print("  CCRC_ERR   ");
    print_int((err & SD_INTERRUPT_CCRC_ERR) != 0);
    uart_print("\n");
    uart_print("  CTO_ERR    ");
    print_int((err & SD_INTERRUPT_CTO_ERR) != 0);
    uart_print("\n");
    uart_print("  ERR        ");
    print_int((err & SD_INTERRUPT_ERR) != 0);
    uart_print("\n");
}

static int sd_exec_cmd(struct sd_cmd cmd, unsigned int arg, unsigned int *ret) {
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

    put32(SDEMMC_INTERRUPT, 0xFFFFFFFF);
    put32(SDEMMC_ARG1, arg);
    put32(SDEMMC_CMDTM, cmd_word);
    
    unsigned int timeout = SDMMC_SLEEP;
    while(!(get32(SDEMMC_INTERRUPT) & 0x01)) {
        if(!timeout--) return -1;
    }
    // Check if error occured
    if(get32(SDEMMC_INTERRUPT) & SD_INTERRUPT_ERR) {
        sd_print_err();
        return -1;
    }
    *ret = get32(SDEMMC_RESP0);

    #ifdef DEBUG_SD
    uart_print("Response for CMD");
    print_hex_int(cmd.cmd_idx);
    uart_print(": ");
    print_hex_uint32(resp);
    uart_print("\n");
    #endif /* DEBUG_SD */
    return 0;
}

static int sd_exec_cmd0() {
    // Send card into idle mode
    struct sd_cmd cmd;
    unsigned int resp;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 0x00;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x00;
    if(sd_exec_cmd(cmd, 0, &resp)) {
        uart_print("SD card error during CMD0\n");
        return -1;
    }
    return 0;
}

static int sd_exec_cmd8() {
    // Send card into idle mode
    struct sd_cmd cmd;
    unsigned int resp;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 0x08;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x02;
    if(sd_exec_cmd(cmd, 0x000001AA, &resp)) {
        #ifdef DEBUG_SD
        uart_print("SD card error during CMD8\n");
        #endif /* DEBUG_SD */
        return -1;
    }
    if(resp != 0x000001AA) {
        #ifdef DEBUG_SD
        uart_print("SD card unexpected response during CMD8!\n");
        #endif /* DEBUG_SD */
        return -1;
    }
    return 0;
}

static int sd_reset() {
    // TODO: Set clock frequency

    put32(SDEMMC_CONTROL0, 0);
    put32(SDEMMC_CONTROL1, 0);
    // Set SRST_HC bit
    put32(SDEMMC_CONTROL1, 0x1<<24);
    unsigned int timeout = SDMMC_SLEEP;
    while(get32(SDEMMC_CONTROL1) & 1<<24) {
        if(!timeout--) return -1;
    }
    // Set CLK_INTLEN bit
    put32(SDEMMC_CONTROL1, get32(SDEMMC_CONTROL1) | 0x1);

    // Set clock
    // Set CLK_EN bit
    put32(SDEMMC_CONTROL1, get32(SDEMMC_CONTROL1) | 0x1<<2);

    timeout = SDMMC_SLEEP;
    while(!(get32(SDEMMC_CONTROL1) & 0x1<<1)) {
        if(!timeout--) return -1;
    }

    put32(SDEMMC_IRPT_MASK, ~0);
    put32(SDEMMC_IRPT_EN, ~0);

    // CMD0: Send into idle mode
    if(sd_exec_cmd0()) {
        return -1;
    }
    
    // CMD8: Check if SD V1 or V2
    if(sd_exec_cmd8()) {
        return -1;
    }

    return 0;
}

int sd_initialize() {
    if(sd_reset() != 0) {
        uart_print("SD card initialization failed!\n");
        return -1;
    }

    uart_print("Initalized SD card!\n");
    return 0;
}

//static unsigned char sd_read_byte() {
//  
//}