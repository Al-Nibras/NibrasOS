/* Master Header of NibrasOS */

#ifndef  __SYSTEM_H
#define  __SYSTEM_H

#include <io.h>
#include <kheap.h> //includes->paging, paging includes->memory
#include <types.h>
#include <screen.h>
#include <syscalls.h>
#include <process.h>
#include <fat32.h>
#include <va_list.h>
#include <cpu.h>

// extern unsigned char *memcpy(unsigned char *dest, const unsigned char *src, unsigned int count);
// extern unsigned char *memset(unsigned char *dest, unsigned char val, unsigned int count);
// extern unsigned short *memsetw(unsigned short *dest, unsigned short val, unsigned int count);
//
// extern int strlen(const char *str);
// extern unsigned char inportb (unsigned short _port);
// extern void outportb (unsigned short _port, unsigned char _data);


/* screen.c */



//
// extern void println(const char * str);
// extern void puts(const char *str);
// extern void putc(char c);
// extern void puthex(int i);
// extern void putlhex(long long i);
// extern void putdec(int num);
// extern void reset_text_color();
// extern void set_text_color(unsigned char foreground, unsigned char background);
// extern void init_video();
// extern void kpanic(const char* str);
// extern void printf(const char *format,...);
// /* math.c */
// extern int pow(int num, int p);
// extern int pon(int num);
// extern void swapi(int *x,int *y);
// extern void swapsh(short *x,short *y);
// extern void swapc(char *x,char *y);
//
//
// /* gdt.c */
// extern void gdt_setup();
// extern void gdt_set_entry(int index, unsigned long base, unsigned long limit, unsigned char acs, unsigned char gran);
//
//
// /* idt.c */
// extern void idt_set_entry(unsigned char num, unsigned long base, unsigned short sel, unsigned char access);
// extern void idt_setup();
//

/* isrs.c */



// extern void isr_setup();
// extern void fault_handler(struct regs *r);
// extern void isr_install_handler(unsigned int isr, void (*handler)(struct regs *r));
// /* irqs.c */
// extern void irq_handler(struct regs *r);
// extern void irq_setup();
// extern void irq_install_handler(unsigned irq, void (*handler)(struct regs *r));
// extern void irq_uninstall_handler(unsigned irq);
//
//
// /* timer.c */
// //extern void timer_setup();
// extern void init_timer(unsigned int hz);
// extern void delay(unsigned int hz);
//
//
// /* kb.c */
// extern void keyboard_setup();
// extern unsigned int kb_type();
// extern void wait_for_input_buffer();
// extern void wait_if_PS2_busy();
//
// /* paging.c */
// extern void paging_setup();

#endif /* __SYSTEM_H */
