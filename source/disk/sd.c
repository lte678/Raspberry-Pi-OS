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
#define SD_INTERRUPT_DATA_READY (1<<5)

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

struct sd_cid {
    uint8_t manufacturer;
    uint16_t application;
    char product_name[5];
    uint8_t product_rev;
    uint32_t product_serial;
    uint8_t _rsrvd0;
    uint8_t _rsrvd1;
    uint8_t _rsrvd2;    
};

struct sd_status {
    unsigned char csd_overwrite_err;
    unsigned char wp_erase_skip;
    unsigned char card_ecc_disabled;
    unsigned char erase_reset;
    unsigned char current_state;
    unsigned char ready_for_data;
    unsigned char app_cmd;
    unsigned char ake_seq_error; // Unused
};

// Shamelessly borrowed from
// https://github.com/bztsrc/raspi3-tutorial/blob/master/0D_readfile/delays.c
void wait_usec(unsigned int n)
{
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // calculate expire value for counter
    t+=((f/1000)*n)/1000;
    do{asm volatile ("mrs %0, cntpct_el0" : "=r"(r));}while(r<t);
}

static void sd_print_err() {
    unsigned int err = get32(SDEMMC_INTERRUPT);
    uart_print("  INTERRUPT: 0x");
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

static void sd_print_status(struct sd_status *s) {
    uart_print("SD Card Status:\n");
    uart_print("  CSD_OVERWRITE ");
    print_int(s->csd_overwrite_err);
    uart_print("\n");
    uart_print("  WP_ERASE_SKIP ");
    print_int(s->wp_erase_skip);
    uart_print("\n");
    uart_print("  CARD_ECC_RST  ");
    print_int(s->card_ecc_disabled);
    uart_print("\n");
    uart_print("  ERASE_RESET   ");
    print_int(s->erase_reset);
    uart_print("\n");
    uart_print("  CURRENT_STATE ");
    switch(s->current_state) {
    case 0:
        uart_print("idle");
        break;
    case 1:
        uart_print("ready");
        break;
    case 2:
        uart_print("ident");
        break;
    case 3:
        uart_print("stby");
        break;
    case 4:
        uart_print("tran");
        break;
    case 5:
        uart_print("data");
        break;
    case 6:
        uart_print("rcv");
        break;
    case 7:
        uart_print("prg");
        break;
    case 8:
        uart_print("dis");
        break;
    default:
        uart_print("unknown");
    }
    //print_int(s->current_state);
    uart_print("\n");
    uart_print("  RDY_FOR_DATA  ");
    print_int(s->ready_for_data);
    uart_print("\n");
    uart_print("  APP_CMD       ");
    print_int(s->app_cmd);
    uart_print("\n");
}

static struct sd_status sd_unpack_status(unsigned int reg) {
    struct sd_status s = {0};
    s.ake_seq_error = (reg & 0x00000008) >> 3;
    s.app_cmd = (reg & 0x00000020) >> 5;
    s.ready_for_data = (reg & 0x00000100) >> 8;
    s.current_state = (reg & 0x0001E00) >> 9;
    s.erase_reset = (reg & 0x00002000) >> 13;
    s.card_ecc_disabled = (reg & 0x00004000) >> 14;
    s.wp_erase_skip = (reg & 0x00008000) >> 15;
    s.csd_overwrite_err = (reg & 0x00010000) >> 16;
    return s;
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
    uint32_t interrupt = get32(SDEMMC_INTERRUPT);
    if(interrupt & SD_INTERRUPT_CTO_ERR) {
        #ifdef DEBUG_SD
        uart_print("SD Timeout during CMD");
        print_int(cmd.cmd_idx);
        uart_print("\n");
        #endif /* DEBUG_SD */
        return -2;
    } else if(interrupt & SD_INTERRUPT_ERR) {
        #ifdef DEBUG_SD
        uart_print("SD error during CMD");
        print_int(cmd.cmd_idx);
        uart_print(" (");
        print_hex_uint32(cmd_word);
        uart_print("): ");
        sd_print_err();
        #endif /* DEBUG_SD */
        return -1;
    }
    *ret = get32(SDEMMC_RESP0);
    if(cmd.response_type == 1) {
        // 136 bit response
        ret[1] = get32(SDEMMC_RESP1);
        ret[2] = get32(SDEMMC_RESP2);
        ret[3] = get32(SDEMMC_RESP3);
    }
    #ifdef DEBUG_SD
    uart_print("Response for CMD");
    print_int(cmd.cmd_idx);
    uart_print(" (");
    print_hex_uint32(cmd_word);
    uart_print("): ");
    print_hex_uint32(*ret);
    if(cmd.response_type == 1) {
        uart_print(" ");
        print_hex_uint32(ret[1]);
        uart_print(" ");
        print_hex_uint32(ret[2]);
        uart_print(" ");
        print_hex_uint32(ret[3]);
    }
    uart_print("\n");
    #endif /* DEBUG_SD */
    return 0;
}

static int sd_exec_cmd0() {
    // Send card into idle mode
    struct sd_cmd cmd;
    unsigned int resp;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 0;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x00;
    if(sd_exec_cmd(cmd, 0, &resp)) {
        uart_print("SD card error during CMD0\n");
        return -1;
    }
    return 0;
}

static int sd_exec_cmd2(unsigned int *resp) {
    // Get card information / CID
    // Requires 16 byte buffer (or 4 uint32_t)
    struct sd_cmd cmd;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 2;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x01; // 136 bits
    if(sd_exec_cmd(cmd, 0, resp)) {
        uart_print("SD card error during CMD2\n");
        return -1;
    }
    return 0;
}

static int sd_exec_cmd3(unsigned int *resp, unsigned int *status) {
    // Get card RCA
    struct sd_cmd cmd;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 3;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x02;
    if(sd_exec_cmd(cmd, 0, resp)) {
        uart_print("SD card error during CMD3\n");
        return -1;
    }
    // Contains status bits 23, 22, 19, 12:0 in lower 2 octets
    *status = 0;
    *status |= *resp & 0x1FFF;
    *status |= (*resp & 0x2000) << 6;
    *status |= (*resp & 0x4000) << 8;
    *status |= (*resp & 0x8000) << 8;
    //struct sd_status stat = sd_unpack_status(status);
    //sd_print_status(&stat);
    return 0;
}

static int sd_exec_cmd7(unsigned int rca, unsigned int *status) {
    // Select card using RCA
    struct sd_cmd cmd;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 7;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x02;
    if(sd_exec_cmd(cmd, rca << 16, status)) {
        uart_print("SD card error during CMD7\n");
        *status = 0;
        return -1;
    }
    return 0;
}

static int sd_exec_cmd8() {
    // Check if SD card is V1 or V2
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

static int sd_exec_cmd13(unsigned int rca, struct sd_status *status) {
    // Select card using RCA
    struct sd_cmd cmd;
    unsigned int resp;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 13;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x02;
    if(sd_exec_cmd(cmd, rca << 16, &resp)) {
        uart_print("SD card error during CMD13\n");
        *status = (struct sd_status){0};
        return -1;
    }
    *status = sd_unpack_status(resp);
    return 0;
}

static int sd_exec_cmd55() {
    // Announce an application specific command
    struct sd_cmd cmd;
    unsigned int resp;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 55;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x02;
    if(sd_exec_cmd(cmd, 0, &resp)) {
        #ifdef DEBUG_SD
        uart_print("SD card error during CMD55\n");
        #endif /* DEBUG_SD */
        return -1;
    }
    // We dont need to do anything with the response / no response
    return 0;
}

static int sd_exec_acmd41(unsigned int *resp) {
    // Initialize the card for data transfer
    // Requires a call to CMD55 beforehand.
    if(sd_exec_cmd55()) {
        return -1;
    }

    struct sd_cmd cmd;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 41;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x02;
    
    if(sd_exec_cmd(cmd, 0x51ff8000, resp)) {
        #ifdef DEBUG_SD
        uart_print("SD card error during CMD41\n");
        #endif /* DEBUG_SD */
        return -1;
    }
    return 0;
}

static int sd_exec_cmd17() {
    // Read one block of data from the SD card
    put32(SDEMMC_BLKSIZECNT, (1 << 16) | 512);

    struct sd_cmd cmd;
    unsigned int resp;
    int ret;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 17;
    cmd.is_data = 1;
    cmd.xfer_data_dir = 1;
    cmd.index_check_en = 0;
    cmd.crc_check_en = 0;
    cmd.response_type = 0x02;
    
    if((ret = sd_exec_cmd(cmd, 0x00, &resp))) {
        #ifdef DEBUG_SD
        uart_print("SD card error during CMD17\n");
        #endif /* DEBUG_SD */
        return ret;
    }
    return 0;
}

static int sd_get_rca(unsigned int *rca, unsigned int *status) {
    if(sd_exec_cmd3(rca, status)) {
        return -1;
    }
    *rca = (*rca & 0xFFFF0000) >> 16;
    return 0;
}

static struct sd_cid sd_get_cid() {
    struct sd_cid cid;
    cid = (struct sd_cid){0};

    if(sd_exec_cmd2((unsigned int*)&cid)) {
        uart_print("SD error returning CID\n");
    }
    return cid;
}

static void print_cid(struct sd_cid *cid) {
    uart_print("Manufacturer: ");
    print_uint(cid->manufacturer);
    uart_print("\n");
    uart_print("Application:  ");
    print_uint(cid->application);
    uart_print("\n");
    // TODO: memcpy
    uart_print("Product Name: ");
    for(int i = 0; i < 5; i++) {
        uart_send(cid->product_name[i]);
    }
    uart_print("\n");
    uart_print("Revision:     ");
    print_uint(cid->product_rev);
    uart_print("\n");
    uart_print("Serial Number:");
    print_uint(cid->product_serial);
    uart_print("\n");
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

    // Enable all the interrupt bits
    put32(SDEMMC_IRPT_MASK, ~0);
    put32(SDEMMC_IRPT_EN, ~0);

    // CMD0: Send into idle mode
    // -> idle
    if(sd_exec_cmd0()) {
        return -1;
    }
    // CMD8: Check if SD V1 or V2
    if(sd_exec_cmd8()) {
        return -1;
    }

    // Try to initialize card for 1 second
    // idle -> ready
    int attempts = 100;
    while(attempts) {
        // ACMD41: Initialize card
        unsigned int resp;
        if(sd_exec_acmd41(&resp)) {
            return -1;
        }
        if(resp & 0x80000000) {
            break;
        }

        attempts--;
        wait_usec(100000);
    }

    // CMD2: Get card CID
    // ready -> ident
    unsigned int resp[4];
    if(sd_exec_cmd2(resp)) {
        return -1;
    }

    // CMD3: Get card RCA
    // ident -> stby
    unsigned int status_reg;
    struct sd_status status;
    unsigned int rca;
    if(sd_get_rca(&rca, &status_reg)) {
        return -1;
    }

    // CMD13: Get status
    //sd_exec_cmd13(rca, &status);
    //sd_print_status(&status);

    //CMD7: select card
    // stby -> tran
    if(sd_exec_cmd7(rca, &status_reg)) {
        return -1;
    }
    //sd_exec_cmd13(rca, &status);
    //sd_print_status(&status);

    return 0;
}

int sd_initialize() {
    if(sd_reset() != 0) {
        uart_print("SD card initialization failed!\n");
        return -1;
    }
    uart_print("Initalized SD card!\n");

    // struct sd_cid cid = sd_get_cid();
    // print_cid(&cid);

    unsigned char buff[512];
    unsigned int *buff2 = (unsigned int*)buff;
    if(sd_exec_cmd17()) {
        uart_print("Read failed!\n");
    }
    int i = 0;
    while(get32(SDEMMC_INTERRUPT) & SD_INTERRUPT_DATA_READY && i < 128) {
        buff2[i] = get32(SDEMMC_DATA);
        i++;
    }
    uart_print("SD: Read ");
    print_int(i*4);
    uart_print(" bytes\n");

    print_hex(buff, 512);

    return 0;
}

//static unsigned char sd_read_byte() {
//  
//}