void wait_cycles(unsigned int n)
{
    if(n) while(n--) { asm volatile("nop"); }
}

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