#ifndef STRING_H
#define STRING_H

int itos(int num, char *buffer, unsigned int n);
int utos(unsigned int num, char *buffer, unsigned int n);
int ltos(long num, char *buffer, unsigned int n);
int ultos(unsigned long num, char *buffer, unsigned int n);
int strncmp(char *s1, char *s2, unsigned int n);
int strcmp(char *s1, char *s2);
int strlen(char *s);
void strncpy(char *dest, char *src, unsigned int n);
int atoi(const char* string, int* result);

#endif // STRING_H