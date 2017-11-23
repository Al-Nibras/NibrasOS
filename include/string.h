#ifndef __STRING_H
#define __STRING_H

#ifndef __TYPES_H
    #include <types.h>
#endif
#ifndef __LIST_H
    #include <list.h>
#endif

#define MAX_ULONG 0xffffffffffffffff

char small_to_cap(char c);
char cap_to_small(char c);
void itoa(char *buf, int base, int d);
int strcpy(char* dest,char* src);
int strcmp(const char* a, const char* b);
int strncmp(const char* s1, const char* s2, size_t n);
char * strdup(const char *str);
char * strtok_r(char * str, const char * delim, char ** saveptr);
unsigned int lfind(const char * str, const char accept);
unsigned int rfind(const char * str, const char accept);
char * strstr(const char * haystack, const char * needle);
unsigned char startswith(const char * str, const char * accept);
unsigned int strspn(const char * str, const char * accept);
char * strpbrk(const char * str, const char * accept);
void str_to_small(char* str);
#endif /* __STRING_H */
