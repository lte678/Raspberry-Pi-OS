#ifndef STRING_H
#define STRING_H

#define KEYCODE_ESCAPE 27
#define KEYCODE_CTRL_C  3

int itos(int num, char *buffer, unsigned int n);
int utos(unsigned int num, char *buffer, unsigned int n);
int ltos(long num, char *buffer, unsigned int n);
int ultos(unsigned long num, char *buffer, unsigned int n);
int strncmp(char *s1, char *s2, unsigned int n);
int strcmp(char *s1, char *s2);
int strlen(char *s);
void strncpy(char *dest, char *src, unsigned int n);
int atoi(const char* string, int* result);
int is_character(char c);
int is_alphanumeric(char c);
int is_csi_sequence_terminator(char c);

#endif // STRING_H