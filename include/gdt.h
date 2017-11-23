#ifndef __GDT_H
#define __GDT_H

#ifndef __TYPES_H
#include <types.h>

#define GDT_ENTRIES_NUM		6

/* GDT best explained in: https://0xax.gitbooks.io/linux-insides/content/Booting/linux-bootstrap-2.html */
struct gdt_entry{
	unsigned short limit_low; 	//bits: 0-15
	unsigned short base_low; 	//bits: 16-31
	unsigned char base_middle;  //bits: 32-39
	unsigned char access;	    //bits: 40-47; contains DPL: (bits 45-46)
	unsigned char granularity;	//bits: 48-55
	unsigned char base_high;	//bits: 56-63
}__attribute__((packed));

struct gdt_ptr{
	unsigned short length;
	unsigned int base;
}__attribute__((packed));

extern struct gdt_entry gdt_entries[GDT_ENTRIES_NUM]; // defined in gdt.c
extern struct gdt_ptr gdt_pointer; // defined in gdt.c

extern void gdt_flush(); 	   /* defined in start.asm */
extern void usermode_switch(); /* defined in start.asm */
extern void v86_mode_switch(); /* defined in start.asm */
void gdt_set_entry(int index, unsigned long base, unsigned long limit, unsigned char acs, unsigned char gran);
void general_protection_exception_handler(struct regs *r);
void gdt_setup();
//void general_protection_exception_handler(struct regs *r);
void switch_to_um();
void enter_v86();

#endif /* __TYPES_H */
#endif /* __GDT_H */
