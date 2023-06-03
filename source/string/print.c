#include <kernel/string.h>
#include <kernel/types.h>
#include <kernel/valist.h>
#include <kernel/chardev.h>

#include "uart.h"

#define get_byte(field, offset) (field & (0xfful << (offset * 8ul))) >> (offset * 8ul)


void print_int(int number) {
    // Theoretically, we should never surpass 12 characters (including null byte)
    char buff[12];
    if(itos(number, buff, sizeof(buff)) > 0) {
        write_string_char(&global_uart, buff);
        return;
    }
    write_string_char(&global_uart, "ERR");
}


void print_uint(unsigned int number) {
    // Theoretically, we should never surpass 12 characters (including null byte)
    char buff[12];
    if(utos(number, buff, sizeof(buff)) > 0) {
        write_string_char(&global_uart, buff);
        return;
    }
    write_string_char(&global_uart, "ERR");
}

void print_long(long number) {
    // Theoretically, we should never surpass 21 characters (including null byte)
    char buff[21];
    if(ltos(number, buff, sizeof(buff)) > 0) {
        write_string_char(&global_uart, buff);
        return;
    }
    write_string_char(&global_uart, "ERR");
}

void print_ulong(unsigned long number) {
    // Theoretically, we should never surpass 21 characters (including null byte)
    char buff[21];
    if(ultos(number, buff, sizeof(buff)) > 0) {
        write_string_char(&global_uart, buff);
        return;
    }
    write_string_char(&global_uart, "ERR");
}

void print_hex(const unsigned char* bytes, unsigned short n) {
    char buff[3];
    int i;

    buff[2] = 0;
    for(i = 0; i < n; i++) {
        // Note: Upper means upper bits, and upper case in this situation
        buff[0] = hex_char_upper((bytes[i] & 0xF0) >> 4);
        buff[1] = hex_char_upper(bytes[i] & 0x0F);
        write_string_char(&global_uart, buff);
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
        write_string_char(&global_uart, buff);
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
    write_string_char(&global_uart, "0x");
    uint64_t addr_int = (uint64_t)addr;
    print_hex_be((unsigned char*)&addr_int, 8);
}


void _print(char *fstring, int numargs, ...) {
    va_list args;
    va_start(args, numargs);

    // States
    // 0 : Normal text
    // 1 : Start of format string
    // 
    int state = 0;
    int escaped = 0;
    int args_printed = 0;
    //char *format_start = 0;
    char *format_end = 0;

    while(*fstring) {
        if(escaped) {
            write_char(&global_uart, fstring, 1);
            escaped = 0;
        } else if(*fstring == '\\') {
            escaped = 1;
        } else if(state == 0) {
            // Outside format tag
            if(*fstring == '{') {
                // Start formatting code
                //format_start = fstring + 1;
                state = 1;
            } else {
                write_char(&global_uart, fstring, 1);
            }
        } else {
            // Inside format tag
            if(*fstring == '}') {
                format_end = fstring;
                if(args_printed < numargs) {
                    if(*(format_end - 1) == 'l') {
                        // The last character specifies the variable length. Eg.: ul, xl...
                        switch(*(format_end - 2)) {
                            case 'u':
                                unsigned long ul_arg = va_arg(args, unsigned long);
                                print_ulong(ul_arg);
                                break;
                            case 'x':
                                // Long hex
                                unsigned long xl_arg = va_arg(args, unsigned long);
                                print_hex_uint64(xl_arg);
                                break;
                            default:
                                // If we only recognize the l, then print a signed long
                                long l_arg = va_arg(args, long);
                                print_long(l_arg);
                        }
                    } else {
                        // The last character is our format
                        switch(*(format_end - 1)) {
                            case 'c':
                                int c_arg = va_arg(args, int);
                                write_char(&global_uart, &c_arg, 1);
                                break;
                            case 'i':
                            case 'd':
                                int i_arg = va_arg(args, int);
                                print_int(i_arg);
                                break;
                            case 'u':
                                unsigned int u_arg = va_arg(args, unsigned int);
                                print_uint(u_arg);
                                break;
                            case 's':
                                char* s_arg = va_arg(args, char*);
                                write_string_char(&global_uart, s_arg);
                                break;
                            case 'x':
                                int x_arg = va_arg(args, unsigned int);
                                print_hex_uint32(x_arg);
                                break;
                            case 'p':
                                void* p_arg = va_arg(args, void*);
                                print_address(p_arg);
                                break;
                            default:
                                // Discard argument, even if invalid format
                                va_arg(args, int);
                            }
                    }
                    args_printed++;
                }
                state = 0;
            }
        }
        fstring++;
    }
    va_end(args);
}