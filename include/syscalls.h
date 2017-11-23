#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#ifndef __TYPES_H
    #include <types.h>
#endif
#ifndef __ISRS_H
    #include <isrs.h>
#endif
#ifndef __VA_LIST_H
    #include <va_list.h>
#endif
#define MAX_SYSCALL 13

//syscalls
extern void putc(char c);
extern void puts(const char *str);
extern void puthex(unsigned int i);
extern void putlhex(long long i);
extern void putdec(int num);
extern void println(const char * str);
extern void printf(const char * str,...);
extern void vsprintf(char * str, void (*putchar)(char), const char * format, va_list arg);
extern void printf_clr(char fg_color, const char *format, ...);
extern void terminate_proc();
extern void lock(unsigned int *semaphore); //define in spinlock.c
extern void unlock(unsigned int *semaphore); //define in spinlock.c
extern void vsprintf(char * str, void (*putchar)(char), const char * format, va_list arg);
extern unsigned int fork();
void syscall_dispatcher(struct regs *r);
void syscall_setup();


//syscalls
void sys_write(const char* str,...);
void sys_idle();
void sys_exit();
void sys_vsprintf(const char * format, va_list arg);

#endif
