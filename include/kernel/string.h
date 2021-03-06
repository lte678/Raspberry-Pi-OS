#ifndef STRING_H
#define STRING_H

extern int itos(int num, char *buffer, unsigned int n);
extern int utos(unsigned int num, char *buffer, unsigned int n);
extern int ltos(long num, char *buffer, unsigned int n);
extern int ultos(unsigned long num, char *buffer, unsigned int n);
extern int strncmp(char *s1, char *s2, unsigned int n);
extern int strcmp(char *s1, char *s2);
extern int strlen(char *s);
extern void strncpy(char *dest, char *src, unsigned int n);
extern int atoi(const char* string, int* result);

#endif // STRING_H