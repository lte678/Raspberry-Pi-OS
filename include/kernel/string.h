#ifndef STRING_H
#define STRING_H

extern int itos(int num, char *buffer, unsigned int n);
extern int utos(unsigned int num, char *buffer, unsigned int n);
extern int ltos(long num, char *buffer, unsigned int n);
extern int ultos(unsigned long num, char *buffer, unsigned int n);
extern int strcmp(char *s1, char *s2);

#endif // STRING_H