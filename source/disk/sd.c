#include <kernel/register.h>
#include <kernel/print.h>
#include <kernel/types.h>
#include <kernel/delay.h>
#include <kernel/string.h>
#include <kernel/block.h>
#include <kernel/mmap.h>

#define SDEMMC_PHYSICAL 0x3F300000
#define SDEMMC_MAPPING_SIZE 0x10000
static uint64_t sdemmc_base = SDEMMC_PHYSICAL;

/*
* SDEMMC Registers
*/
#define SDEMMC_ARG2 (sdemmc_base + 0x0ul)
#define SDEMMC_BLKSIZECNT (sdemmc_base + 0x4ul)
#define SDEMMC_ARG1 (sdemmc_base + 0x8ul)
#define SDEMMC_CMDTM (sdemmc_base + 0xCul)
#define SDEMMC_RESP0 (sdemmc_base + 0x10ul)
#define SDEMMC_RESP1 (sdemmc_base + 0x14ul)
#define SDEMMC_RESP2 (sdemmc_base + 0x18ul)
#define SDEMMC_RESP3 (sdemmc_base + 0x1Cul)
#define SDEMMC_DATA (sdemmc_base + 0x20ul)
#define SDEMMC_STATUS (sdemmc_base + 0x24ul)
#define SDEMMC_CONTROL0 (sdemmc_base + 0x28ul)
#define SDEMMC_CONTROL1 (sdemmc_base + 0x2Cul)
#define SDEMMC_INTERRUPT (sdemmc_base + 0x30ul)
#define SDEMMC_IRPT_MASK (sdemmc_base + 0x34ul)
#define SDEMMC_IRPT_EN (sdemmc_base + 0x38ul)
#define SDEMMC_CONTROL2 (sdemmc_base + 0x3Cul)
#define SDEMMC_FORCE_IRPT (sdemmc_base + 0x50ul)
#define SDEMMC_BOOT_TIMEOUT (sdemmc_base + 0x70ul)
#define SDEMMC_DBG_SEL (sdemmc_base + 0x74ul)
#define SDEMMC_EXRDFIFO_CFG (sdemmc_base + 0x80ul)
#define SDEMMC_EXRDFIFO_EN (sdemmc_base + 0x84ul)
#define SDEMMC_TUNE_STEP (sdemmc_base + 0x88ul)
#define SDEMMC_TUNE_STEPS_STD (sdemmc_base + 0x8cul)
#define SDEMMC_TUNE_STEPS_DDR (sdemmc_base + 0x90ul)
#define SDEMMC_SPI_INT_SPT (sdemmc_base + 0xf0ul)
#define SDEMMC_SLOTISR_VER (sdemmc_base + 0xfcul)

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

#define SDMMC_SLEEP (4000000)

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
    char application[3];
    char product_name[6];
    uint8_t product_rev;
    uint32_t product_serial;   
};

struct sd_csd {
	unsigned char structure;
	unsigned char mmca_vsn;
	unsigned short cmdclass;
    // This is a huge strucure. Just add the ones we need
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

static unsigned int rca;

/*
static void sd_print_err() {
    unsigned int err = get32(SDEMMC_INTERRUPT);
    print("  INTERRUPT: 0x{x}\n", err);
    // Bit details:
    print("  ACMD_ERR   {i}\n", (err & SD_INTERRUPT_ACMD_ERR) != 0);
    print("  DEND_ERR   {i}\n", (err & SD_INTERRUPT_DEND_ERR) != 0);
    print("  DCRC_ERR   {i}\n", (err & SD_INTERRUPT_DCRC_ERR) != 0);
    print("  DTO_ERR    {i}\n", (err & SD_INTERRUPT_DTO_ERR) != 0);
    print("  CBAD_ERR   {i}\n", (err & SD_INTERRUPT_CBAD_ERR) != 0);
    print("  CEND_ERR   {i}\n", (err & SD_INTERRUPT_CEND_ERR) != 0);
    print("  CCRC_ERR   {i}\n", (err & SD_INTERRUPT_CCRC_ERR) != 0);
    print("  CTO_ERR    {i}\n", (err & SD_INTERRUPT_CTO_ERR) != 0);
    print("  ERR        {i}\n", (err & SD_INTERRUPT_ERR) != 0);
}
*/


static void sd_print_status(struct sd_status *s) {
    print("SD Card Status:\n");
    print("  CSD_OVERWRITE {i}\n", (int)s->csd_overwrite_err);
    print("  WP_ERASE_SKIP {i}\n", (int)s->wp_erase_skip);
    print("  CARD_ECC_RST  {i}\n", (int)s->card_ecc_disabled);
    print("  ERASE_RESET   {i}\n", (int)s->erase_reset);
    print("  CURRENT_STATE ");
    switch(s->current_state) {
    case 0:
        print("idle");
        break;
    case 1:
        print("ready");
        break;
    case 2:
        print("ident");
        break;
    case 3:
        print("stby");
        break;
    case 4:
        print("tran");
        break;
    case 5:
        print("data");
        break;
    case 6:
        print("rcv");
        break;
    case 7:
        print("prg");
        break;
    case 8:
        print("dis");
        break;
    default:
        print("unknown");
    }
    print("\n");
    print("  RDY_FOR_DATA  {i}\n", (int)s->ready_for_data);
    print("  APP_CMD       {i}\n", (int)s->app_cmd);
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

static struct sd_cid sd_unpack_cid(unsigned int r[4]) {
    struct sd_cid cid;
    cid.manufacturer    = (r[0] & 0x00FF0000) >> 16;
    cid.application[0]  = (r[0] & 0x0000FF00) >> 8;
    cid.application[1]  = (r[0] & 0x000000FF);
    cid.application[2]  = '\0';
    cid.product_name[0] = (r[1] & 0xFF000000) >> 24;
    cid.product_name[1] = (r[1] & 0x00FF0000) >> 16;
    cid.product_name[2] = (r[1] & 0x0000FF00) >> 8;
    cid.product_name[3] = (r[1] & 0x000000FF);
    cid.product_name[4] = (r[2] & 0xFF000000) >> 24;
    cid.product_name[5] = '\0';
    cid.product_rev     = (r[2] & 0x00FF0000) >> 16;
    cid.product_serial  = (r[2] & 0x0000FFFF) << 16;
    cid.product_serial |= (r[3] & 0xFFFF0000) >> 16;
    return cid;
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
        print("SD Timeout during CMD{i}\n", (int)cmd.cmd_idx);
        #endif /* DEBUG_SD */
        return -2;
    } else if(interrupt & SD_INTERRUPT_ERR) {
        #ifdef DEBUG_SD
        print("SD error during CMD{i} ({u}):\n", (int)cmd.cmd_idx, cmd_word);
        sd_print_err();
        #endif /* DEBUG_SD */
        return -1;
    }

    // response_type == 0 gives no result
    if(cmd.response_type == 2) {
        *ret = get32(SDEMMC_RESP0);
    } else if(cmd.response_type == 1) {
        // 136 bit response
        // Maintain correct byte ordering
        ret[0] = get32(SDEMMC_RESP3);
        ret[1] = get32(SDEMMC_RESP2);
        ret[2] = get32(SDEMMC_RESP1);
        ret[3] = get32(SDEMMC_RESP0);
    }

    #ifdef DEBUG_SD
    print("Response for CMD{i} ({u}): {x}", (int)cmd.cmd_idx, cmd_word, *ret);
    if(cmd.response_type == 1) {
        print(" {x} {x} {x}", ret[1], ret[2], ret[3]);
    }
    print("\n");
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
        print("SD card error during CMD0\n");
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
        print("SD card error during CMD2\n");
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
        print("SD card error during CMD3\n");
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
        print("SD card error during CMD7\n");
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
        print("SD card error during CMD8\n");
        #endif /* DEBUG_SD */
        return -1;
    }
    if(resp != 0x000001AA) {
        #ifdef DEBUG_SD
        print("SD card unexpected response during CMD8!\n");
        #endif /* DEBUG_SD */
        return -1;
    }
    return 0;
}

static int sd_exec_cmd10(unsigned int rca, unsigned int *resp) {
    // Get card information / CID (with specific RCA)
    // Requires 16 byte buffer (or 4 uint32_t)
    struct sd_cmd cmd;
    cmd = (struct sd_cmd){0};
    cmd.cmd_idx = 10;
    cmd.index_check_en = 1;
    cmd.crc_check_en = 1;
    cmd.response_type = 0x01; // 136 bits
    if(sd_exec_cmd(cmd, rca << 16, resp)) {
        print("SD card error during CMD10\n");
        return -1;
    }
    // Reverse the string buffer. All other values are in correct order.

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
        print("SD card error during CMD13\n");
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
        print("SD card error during CMD55\n");
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
        print("SD card error during CMD41\n");
        #endif /* DEBUG_SD */
        return -1;
    }
    return 0;
}

static int sd_exec_cmd17(uint32_t block) {
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
    
    if((ret = sd_exec_cmd(cmd, block, &resp))) {
        #ifdef DEBUG_SD
        print("SD card error during CMD17\n");
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
    unsigned int cid_reg[4];
    cid = (struct sd_cid){0};

    // Put card into standby mode
    // TODO: Cache the current sd card state to avoid unnecessary context switches
    unsigned int status;
    if(sd_exec_cmd7(0, &status)) {
        print("SD error switching to standby\n");
    }
    // Get the CID for a card with specific RCA
    if(sd_exec_cmd10(rca, cid_reg)) {
        print("SD error returning CID\n");
    }
    cid = sd_unpack_cid(cid_reg);
    // Switch back to transfer mode
    if(sd_exec_cmd7(rca, &status)) {
        print("SD error switching to transfer mode\n");
    }
    return cid;
}

static void sd_print_cid(struct sd_cid *cid) {
    print("  Manufacturer:  {i}\n", (int)cid->manufacturer);
    print("  Application:   {s}\n", cid->application);
    print("  Product Name:  {s}\n", cid->product_name);
    print("  Revision:      {i}\n", (int)cid->product_rev);
    print("  Serial Number: {u}\n", cid->product_serial);
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
    if(sd_get_rca(&rca, &status_reg)) {
        return -1;
    }

    // CMD13: Get status
    //struct sd_status status;
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

static int sd_read_nblk(struct block_dev *dev, void *buf, unsigned int n) {
    // struct sd_cid cid = sd_get_cid();
    // print_cid(&cid);

    if(sd_exec_cmd17(dev->iblk * dev->block_size)) {
        print("SD card error during read!\n");
        return -1;
    }
    int i = 0;
    unsigned int *buf2 = (unsigned int*)buf;
    while(get32(SDEMMC_INTERRUPT) & SD_INTERRUPT_DATA_READY && i < 128) {
        buf2[i] = get32(SDEMMC_DATA);
        uint8_t data[4];
        *(uint32_t*)data = buf2[i];
        if(n > i*4) {
            ((uint8_t*)buf)[i*4] = data[0];
        } if(n > i*4 + 1) {
            ((uint8_t*)buf)[i*4 + 1] = data[1];
        } if(n > i*4 + 2) {
            ((uint8_t*)buf)[i*4 + 2] = data[2];
        } if(n > i*4 + 3) {
            ((uint8_t*)buf)[i*4 + 3] = data[3];
        }
        i++;
    }
    dev->iblk++;

    return 1;
}

static int sd_read_blk(struct block_dev *dev, void *buf) {
    return sd_read_nblk(dev, buf, dev->block_size);
}

static int sd_seek_blk(struct block_dev *dev, unsigned int iblk) {
    // Seeking for SD cards is trivial.
    dev->iblk = iblk;
    return 0;
}

int sd_initialize(struct block_dev *dev) {
    sdemmc_base = (uint64_t)mmap(SDEMMC_PHYSICAL, SDEMMC_MAPPING_SIZE);
    if(!sdemmc_base) {
        print("sd: error: failed to create io mapping!\n");
        return -1;
    }
    print("SD EMMC peripheral mapped @ 0x{xl}\n", sdemmc_base);

    if(sd_reset() != 0) {
        print("SD card initialization failed!\n");
        return -1;
    }
    print("Initalized SD card!\n");
    dev->block_size = 512;
    strncpy(dev->driver_str, "SD_DEVICE", sizeof(dev->driver_str));
    dev->read_blk = sd_read_blk;
    dev->read_nblk = sd_read_nblk;
    dev->seek_blk = sd_seek_blk;

    return 0;
}

//static unsigned char sd_read_byte() {
//  
//}

static int monoterm_sd_help() {
    print("Usage: 'sd [help|status|cid]'\n");
    return 0;
}

static int monoterm_sd_status() {
    // CMD13: Get status
    struct sd_status status;
    if(sd_exec_cmd13(rca, &status)) {
        print("SD card returned error for CMD13\n");
        return -1;
    }
    sd_print_status(&status);
    return 0;
}

static int monoterm_sd_cid() {
    struct sd_cid cid = sd_get_cid();
    sd_print_cid(&cid);
    return 0;
}

int monoterm_sd(int argc, char *argv[]) {
    if(argc == 1) {
        return monoterm_sd_help();
    } else if(argc == 2) {
        if(!strcmp(argv[1], "status")) {
            return monoterm_sd_status();
        } else if(!strcmp(argv[1], "cid")) {
            return monoterm_sd_cid();
        } else {
            // Command not recognized
            return monoterm_sd_help();
        }
    }
    return monoterm_sd_help();
}