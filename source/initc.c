extern unsigned char __bss_start;
extern unsigned char __bss_end;

void zero_bss(void) {
    unsigned int *p = (unsigned int*)&__bss_start;
    while(p < (unsigned int*)&__bss_end) {
        *p = 0;
        p++;
    }
}