//  NibrasOS
//  stdlib.h
//
//  Created by Abdulaziz Alfaifi <al-nibras> on 2017-09-08T14:55:52+03:00.
//	Last modification time on 2017-09-08T14:56:28+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//


#ifndef STDLIB_H
#define STDLIB_H

typedef __builtin_va_list va_list;
#define va_start(ap,last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap,type) __builtin_va_arg(ap,type)
#define va_copy(dest, src) __builtin_va_copy(dest,src)

#define UINT32_MAX 4294967295
#define INT32_MAX  2147483647
#define UINT16_MAX 65535
#define INT16_MAX  32767
#define UINT8_MAX  255
#define INT8_MAX   127
#define MAX_ULONG  0xffffffffffffffff


typedef unsigned long u64int,uint64_t;
typedef long s64int,int64_t;

typedef unsigned int u32int,uint32_t;
typedef int s32int,int32_t;
typedef unsigned int size_t;

typedef unsigned short u16int,uint16_t;
typedef short s16int,int16_t;

typedef unsigned char u8int,uint8_t;
typedef char s8int,int8_t;

#define NULL 0


struct regs
{	/* The order is important and it's set according to Stack in start.asm */
	unsigned int gs, fs, es, ds;      					          /* already pushed by start.asm: common_ISR */
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;          /* pushed by 'pusha' instruction in common_ISR */
	unsigned int intrr_no, err_code;    				          /* pushed by each isr function in start.asm: isrN */
	unsigned int eip, cs, eflags, user_esp, ss;     		      /* pushed by the CPU automatically */
} __attribute__((packed));
typedef struct regs register_t,stackframe_t;

typedef unsigned int pid_t;

//system
pid_t fork();
void putc(char c);
void puts(const char *str);
void printf(const char* format, ...);
void idle();
void exit(char status);
void lock(unsigned int *semaphore);
void unlock(unsigned int *semaphore);
unsigned char *memcpy(unsigned char *dest, const unsigned char *src, unsigned int count);
unsigned char *memset(unsigned char *dest, unsigned char val, unsigned int count);
void *memmove(void * restrict dest, const void * restrict src, size_t count);
uint32_t __attribute__ ((pure)) rand();


//String.c
char small_to_cap(char c);
char cap_to_small(char c);
void itoa(char *buf, int base, int d);
int strcpy(char* dest,char* src);
int strcmp(const char* a, const char* b);
int strncmp(const char* s1, const char* s2, size_t n);
char * strdup(char* dest, const char * src);
char * strtok_r(char * str, const char * delim, char ** saveptr);
unsigned int lfind(const char * str, const char accept);
unsigned int rfind(const char * str, const char accept);
char * strstr(const char * haystack, const char * needle);
unsigned char startswith(const char * str, const char * accept);
unsigned int strspn(const char * str, const char * accept);
char * strpbrk(const char * str, const char * accept);
void str_to_small(char* str);
void str_to_cap(char* str);
int isspace(char c);
int atoi(char * string);

//Math.c
long long pow(int num, int p);
int pon(int num);
void swapi(int *x,int *y);
void swapsh(short *x,short *y);
void swapc(char *x,char *y);

#endif /* STDLIB_H */
