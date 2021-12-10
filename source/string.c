int itos(int num, char *buffer, unsigned int n) {
    int i;
    int digit;

    for(i = 0; i < (n - 1); i++) {
        digit = (num % 10);
        num = (num - digit) / 10;

        buffer[i] = '0' + digit;

        if(!num) {
            // Terminate with null char
            buffer[i + 1] = '\0';
            return i + 1;  // Bytes written
        }
    }

    // Terminate with null char
    buffer[n - 1] = '\0';

    return -1;
}