int itos(int num, char *buffer, unsigned int n) {
    int i;
    int j;
    int digit;
    unsigned char sign;
    char tmp_buffer[12];

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
        digit = (num % 10);
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

int utos(unsigned int num, char *buffer, unsigned int n) {
    int i;
    int j;
    int digit;
    char tmp_buffer[12];

    if(n > sizeof(tmp_buffer)) {
        n = sizeof(tmp_buffer);
    }

    for(i = 0; i < (n - 1); i++) {
        digit = (num % 10);
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