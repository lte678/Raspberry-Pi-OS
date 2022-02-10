#include <kernel/string.h>
#include "uart.h"


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

void print_hex(unsigned char* bytes, unsigned short n) {
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

void print_hex_uint32(unsigned int num) {
    // This is the big endian ordered version of num
    unsigned char repr[] = {num >> 24,
        (num & 0xFF0000) >> 16,
        (num & 0xFF00) >> 8,
        num & 0xFF};

    print_hex(repr, 4);
}