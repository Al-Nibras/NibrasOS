#ifndef __IDT_H
#define __IDT_H

struct idt_entry{
	unsigned short base_low;
	unsigned short selector; 	/* Kernel Code Segment */

	unsigned char zero;     /* This will ALWAYS be set to 0! , is set 1 if it's a GDT instead ot LDT*/
	unsigned char flags;    /* access flags */

	unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
	unsigned short length;
	unsigned int base;
} __attribute__((packed));

extern void idt_load(); /* defined in start.asm */

void idt_set_entry(unsigned char num, unsigned long base, unsigned short sel, unsigned char access);
void idt_setup();


#endif /* __IDT_H */
