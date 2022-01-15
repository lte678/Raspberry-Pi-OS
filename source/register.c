void put32(unsigned int addr, unsigned int data) {
    *((unsigned int*)(unsigned long int)addr) = data; 
}

unsigned int get32(unsigned int addr) {
    return *((unsigned int*)(unsigned long int)addr); 
}