//  NibrasOS
//  gdt.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-15T19:55:12+03:00.
//	Last modification time on 2017-07-29T23:54:11+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <gdt.h>

/* GDT best explained in: https://0xax.gitbooks.io/linux-insides/content/Booting/linux-bootstrap-2.html */

struct gdt_entry gdt_entries[GDT_ENTRIES_NUM];
struct gdt_ptr gdt_pointer; /* to be referenced by start.asm */

void gdt_set_entry(int index, unsigned long base, unsigned long limit, unsigned char acs, unsigned char gran){

	gdt_entries[index].base_low = (base & 0xffff);
	gdt_entries[index].base_middle = (base >> 16) & 0xff;
    gdt_entries[index].base_high = (base >> 24) & 0xff;

	gdt_entries[index].limit_low = limit & 0xffff;
	gdt_entries[index].granularity = ((limit >> 16) & 0x0F); //setting limit_high bits (4bits: 48-51)

	gdt_entries[index].granularity |= gran & 0xf0;
	gdt_entries[index].access = acs;

}

void general_protection_exception_handler(struct regs *r){
	printf_clr(0x04,"General Protection Fault!\nError Code: 0x%x\n",r->err_code);
	unsigned int eip;
	unsigned short cs;
	__asm__("movl 4(%%esp), %%eax\n\t"
			"movl %%eax, %0" : :"r"(eip) : );
	__asm__("movw 8(%%esp), %%bx\t\n"
			"movw %%bx, %0" : :"r"(cs) : );
	printf_clr(0x0c,"EIP: 0x%x\t\tCS: 0x%x\n",eip,cs);
	halt();
}

void gdt_setup(){

	gdt_pointer.length = (sizeof (struct gdt_entry) * GDT_ENTRIES_NUM) - 1;
	gdt_pointer.base = (unsigned long int)&gdt_entries;

	/* NULL descriptor: first GDT entry,  GDT[0x00]*/
	gdt_set_entry(0,0,0,0,0);

    gdt_set_entry(1, 0x0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel's Code Segment descriptor, GDT[0x08]
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel's Data Segment descriptor, GDT[0x10]
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
  	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

	/*
		Selector is 8 bytes long: 0x00, 0x08,0x10,0x18,...etc:
		GDT Granularity level: 0xCF = ring 0, kernel-mode
		GDT ACCESS PERMISSIONS: 9: READ, A: EXECUTE, 2: WRITE
		hence, 9A=RX, 92=RW permissions for our kernel
		NOTE: can’t be both writable and executable at the same time.
		our kernel can access the whole memory, that is, 0x0 - 0xFFFFFFFF
		Reference: Intel manual [33], chapter 3, for more details on GDT fields and segmentation
	*/

	gdt_flush();
}
