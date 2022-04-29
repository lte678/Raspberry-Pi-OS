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


int strcmp(char *s1, char *s2) {
    while(*s1 && *s2) {
        if(*s1 != *s2) {
            return -1;
        }
        s1++;
        s2++;
    }
    if(*s1 || *s2) {
        return -1;
    }
    return 0;
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