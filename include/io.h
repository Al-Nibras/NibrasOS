#ifndef __IO_H
#define __IO_H

//TODO:
//Define all known I/O ports as macros


#define insl(port, buffer, count) __asm__ __volatile__("cld; rep; insl" :: "D" (buffer), "d" (port), "c" (count))
#define IRQ_OFF { asm volatile ("cli"); }
#define IRQ_RES { asm volatile ("sti"); }
#define PAUSE   { asm volatile ("hlt"); }
#define IRQS_ON_AND_PAUSE { asm volatile ("sti\nhlt\ncli"); }

unsigned char inportb(unsigned short _port);
unsigned short inportw(unsigned short _port);
unsigned int inportl(unsigned short _port);
void outportb (unsigned short _port, unsigned char val);
void outportw(unsigned short port,unsigned short val);
void outportl(unsigned short port,unsigned int val);
unsigned short inports(unsigned short _port);
void outports(unsigned short _port, unsigned short _data);
void outportsm(unsigned short port, unsigned char * data, unsigned long size);
void inportsm(unsigned short port, unsigned char * data, unsigned long size);

#endif
