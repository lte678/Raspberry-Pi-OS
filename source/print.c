#include <kernel/string.h>
#include <kernel/types.h>

#include "uart.h"

#define get_byte(field, offset) (field & (0xfful << (offset * 8ul))) >> (offset * 8ul)


static char hex_char_upper(unsigned char c) {
    switch(c) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8: 
    case 9:
        return '0' + c;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return 'A' + (c - 10);
    default:
        return ' ';
    }
}

/* static char hex_char_lower(unsigned char c) {
    switch(c) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8: 
    case 9:
        return '0' + c;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return 'a' + (c - 10);
    default:
        return ' ';
    }
} */

void print_int(int number) {
    // Theoretically, we should never surpass 12 characters (including null byte)
    char buff[12];
    if(itos(number, buff, sizeof(buff)) > 0) {
        uart_print(buff);
        return;
    }
    uart_print("ERR");
}


void print_uint(unsigned int number) {
    // Theoretically, we should never surpass 12 characters (including null byte)
    char buff[12];
    if(utos(number, buff, sizeof(buff)) > 0) {
        uart_print(buff);
        return;
    }
    uart_print("ERR");
}

void print_long(long number) {
    // Theoretically, we should never surpass 21 characters (including null byte)
    char buff[21];
    if(ltos(number, buff, sizeof(buff)) > 0) {
        uart_print(buff);
        return;
    }
    uart_print("ERR");
}

void print_ulong(unsigned long number) {
    // Theoretically, we should never surpass 21 characters (including null byte)
    char buff[21];
    if(ultos(number, buff, sizeof(buff)) > 0) {
        uart_print(buff);
        return;
    }
    uart_print("ERR");
}

void print_hex(const unsigned char* bytes, unsigned short n) {
    char buff[3];
    int i;

    buff[2] = 0;
    for(i = 0; i < n; i++) {
        // Note: Upper means upper bits, and upper case in this situation
        buff[0] = hex_char_upper((bytes[i] & 0xF0) >> 4);
        buff[1] = hex_char_upper(bytes[i] & 0x0F);
        uart_print(buff);
    }
}

void print_hex_be(const unsigned char* bytes, unsigned short n) {
    char buff[3];
    int i;

    buff[2] = 0;
    for(i = n - 1; i >= 0; i--) {
        // Note: Upper means upper bits, and upper case in this situation
        buff[0] = hex_char_upper((bytes[i] & 0xF0) >> 4);
        buff[1] = hex_char_upper(bytes[i] & 0x0F);
        uart_print(buff);
    }
}


void print_hex_uint32(unsigned int num) {
    // This is the big endian ordered version of num
    unsigned char repr[] = {
        get_byte(num, 3),
        get_byte(num, 2),
        get_byte(num, 1),
        get_byte(num, 0)
    };

    print_hex(repr, 4);
}

void print_hex_uint64(uint64_t num) {
    // This is the big endian ordered version of num
    unsigned char repr[] = {
        get_byte(num, 7),
        get_byte(num, 6),
        get_byte(num, 5),
        get_byte(num, 4),
        get_byte(num, 3),
        get_byte(num, 2),
        get_byte(num, 1),
        get_byte(num, 0)
    };

    print_hex(repr, 8);
}

void print_address(void* addr) {
    uart_print("0x");
    uint64_t addr_int = (uint64_t)addr;
    print_hex_be((unsigned char*)&addr_int, 8);
}

// Terminal control commands

void term_set_cursor(int x, int y) {
    uart_print("\x1B[");
    print_int(x);
    uart_print(";");
    print_int(y);
    uart_print("H");
}

void term_set_cursor_column(int x) {
    uart_print("\x1B[");
    print_int(x);
    uart_print("G");
}

void term_set_cursor_row(int x) {
    // TODO
}

void term_set_bold() {
    uart_print("\x1B[1m");
}

void term_set_color(int code) {
    uart_print("\x1B[38;5;");
    print_int(code);
    uart_print("m");
}

void term_reset_font() {
    uart_print("\x1B[0m");
}