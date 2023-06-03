#include <kernel/string.h>

#include <kernel/alloc.h>


int ltos(long num, char *buffer, unsigned int n) {
    int i;
    int j;
    int digit;
    unsigned char sign;
    char tmp_buffer[21];

    if(n > sizeof(tmp_buffer)) {
        n = sizeof(tmp_buffer);
    }
    
    if(num < 0) {
        sign = 0;
        num = -num;
    } else {
        sign = 1;
    }
    for(i = 0; i < (n - 2); i++) {
        digit = (num % 10u);
        num = (num - digit) / 10;

        tmp_buffer[i] = '0' + digit;

        if(!num) {
            // Add minus if required
            if(!sign) {
                i++;
                tmp_buffer[i] = '-';
            }
            // We are done, now reverse the result
            for(j = 0; j <= i; j++) {
                buffer[j] = tmp_buffer[i - j];
            }

            // Terminate with null char
            buffer[i + 1] = '\0';
            return i + 1;  // Bytes written
        }
    }

    // Terminate with null char
    buffer[n - 1] = '\0';

    return -1;
}


int ultos(unsigned long num, char *buffer, unsigned int n) {
    int i;
    int j;
    int digit;
    char tmp_buffer[21];

    if(n > sizeof(tmp_buffer)) {
        n = sizeof(tmp_buffer);
    }

    for(i = 0; i < (n - 1); i++) {
        digit = (num % 10u);
        num = (num - digit) / 10;

        tmp_buffer[i] = '0' + digit;

        if(!num) {
            // We are done, now reverse the result
            for(j = 0; j <= i; j++) {
                buffer[j] = tmp_buffer[i - j];
            }

            // Terminate with null char
            buffer[i + 1] = '\0';
            return i + 1;  // Bytes written
        }
    }

    // Terminate with null char
    buffer[n - 1] = '\0';

    return -1;
}


int itos(int num, char *buffer, unsigned int n) {
    return ltos((long)num, buffer, n);
}


int utos(unsigned int num, char *buffer, unsigned int n) {
    return ultos((unsigned long)num, buffer, n);
}

/*
 * Returns 0 if strings match
 */
int strncmp(char *s1, char *s2, unsigned int n) {
    unsigned int i = 0;
    while((*s1 || *s2) && (i < n || n == 0)) {
        if(*s1 != *s2) {
            return -1;
        }
        s1++;
        s2++;
        i++;
    }
    return 0;
}

int strcmp(char *s1, char *s2) {
    return strncmp(s1, s2, 0);
}

int strlen(char *s) {
    int i = 0;
    while(s[i]) {
        i++;
    }
    return i;
}

void strncpy(char *dest, char *src, unsigned int n) {
    unsigned int i = 0;
    while(i < n) {
        *dest = *src;
        if(!(*src)) {
            return;
        }
        dest++;
        src++;
        i++;
    }
}

int atoi(const char* string, int* result) {
    // First find end of string, so we can work backwards
    int length = 0;
    while(string[length]) {
        length++;
    }
    
    int res = 0;
    int i = 0;
    int base = 1;
    int sign_reached = 0;
    while(i < length && !sign_reached) {
        char c = string[(length - 1) - i];
        switch(c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            res += (c - '0') * base;
            base *= 10;
            break;
        case '.':
            break;
        case '-':
            sign_reached = 1;
            res = -res;
            break;
        case '+':
            sign_reached = 1;
            break;
        default:
            // Invalid character
            return 1;
        }
        i++;
    }
    // Check rest of buffer after sign, and make sure it is only whitespace
    while(i < length) {
        if(string[(length - 1) - i] != ' ') {
            return 1;
        }
    }
    *result = res;
    return 0;
}

/**
 * @brief Returns 1 if the char describes a letter, number or symbol.
 * 
 * @param c 
 * @return int 
 */
int is_character(char c) {
    if(c >= 32 && c < 127) {
        return 1;
    } 
    return 0;
}

/**
 * @brief Returns 1 if the char is a letter or number.
 * 
 * @param c 
 * @return int 
 */
int is_alphanumeric(char c) {
    if(c >= '0' && c <= '9') {
        return 1;
    } 
    if(c >= 'a' && c <= 'z') {
        return 1;
    }
    if(c >= 'A' && c <= 'Z') {
        return 1;
    }
    return 0;
}


char* escape_string(char* s, uint32_t len) {
    // First pass to determine output buffer length
    uint32_t buffer_len = 1;
    for(uint32_t i = 0; i < len; i++) {
        if(!is_character(s[i])) {
            buffer_len += 4;
        } else {
            buffer_len += 1;
        }
    }
    char* out = kmalloc(buffer_len, 0);
    if(!out) {
        return 0;
    }

    // Second pass to escape characters
    char* j = out;
    for(uint32_t i = 0; i < len; i++) {
        if(!is_character(s[i])) {
            j[0] = '\\';
            j[1] = 'x';
            j[2] = hex_char_upper((s[i] & 0xF0) >> 4);
            j[3] = hex_char_upper(s[i] & 0x0F);
            j += 4;
        } else {
            *j = s[i];
            j += 1;
        }
    }
    *j = 0;
    return out;
}


/**
 * @brief Determines if the character terminates a
 * https://en.wikipedia.org/wiki/ANSI_escape_code#CSIsection
 * 
 * @param c 
 * @return int 
 */
int is_csi_sequence_terminator(char c) {
    char* matches = "@[\\]^_`{|}~";
    while(*matches) {
        if(c == *matches) {
            return 1;
        }
        matches++;
    }
    if(c >= 'a' && c <= 'z') {
        return 1;
    }
    if(c >= 'A' && c <= 'Z') {
        return 1;
    }
    return 0;
}

char hex_char_upper(unsigned char c) {
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