//  NibrasOS
//  idt.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-16T00:37:25+03:00.
//	Last modification time on 2017-07-29T23:54:00+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <idt.h>

struct idt_entry idt_entries[256];
struct idt_ptr idt_pointer;

void idt_set_entry(unsigned char num, unsigned long base, unsigned short sel, unsigned char access){
	idt_entries[num].base_low = base & 0xffff;
	idt_entries[num].base_high = (base >> 16) & 0xffff; /* just like this: (base & 0xffff0000) */
	idt_entries[num].flags = access;
	idt_entries[num].zero = 0;
	idt_entries[num].selector = sel;
}
void idt_setup(){
	idt_pointer.length = (sizeof (struct idt_entry) * 256) - 1;
	idt_pointer.base = &idt_entries;

	memset((void*)&idt_entries, 0, sizeof(struct idt_entry) * 256);

	idt_load(); /* executes LIDT instruction */
}
