//  NibrasOS
//  isrs.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-17T22:17:59+03:00.
//	Last modification time on 2017-07-29T23:53:52+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <isrs.h>

void* ISRs[256] = {
	isr0,isr1,isr2,isr3,isr4,isr5,isr6,isr7,isr8,isr9,isr10,isr11,isr12,isr13,isr14,isr15,isr16,
	isr17,isr18,isr19,isr20,isr21,isr22,isr23,isr24,isr25,isr26,isr27,isr28,isr29,isr30,isr31
};

void isr_install_handler(unsigned int isr, void (*handler)(struct regs *r)) {
	ISRs[isr] = handler;
	if(isr == 0x32 ){
		idt_set_entry(isr, isr50, 0x08, 0xEE);
	}
}
void isr_setup(){
	idt_set_entry(0 , (unsigned long)isr0,  0x08, 0x8E);
	idt_set_entry(1 , (unsigned long)isr1,  0x08, 0x8E);
	idt_set_entry(2 , (unsigned long)isr2,  0x08, 0x8E);
	idt_set_entry(3 , (unsigned long)isr3,  0x08, 0x8E);
	idt_set_entry(4 , (unsigned long)isr4,  0x08, 0x8E);
	idt_set_entry(5 , (unsigned long)isr5,  0x08, 0x8E);
	idt_set_entry(6 , (unsigned long)isr6,  0x08, 0x8E);
	idt_set_entry(7 , (unsigned long)isr7,  0x08, 0x8E);
	idt_set_entry(8 , (unsigned long)isr8,  0x08, 0x8E);
	idt_set_entry(9 , (unsigned long)isr9,  0x08, 0x8E);
	idt_set_entry(10, (unsigned long)isr10, 0x08, 0x8E);
	idt_set_entry(11, (unsigned long)isr11, 0x08, 0x8E);
	idt_set_entry(12, (unsigned long)isr12, 0x08, 0x8E);
	idt_set_entry(13, (unsigned long)isr13, 0x08, 0x8E);
	idt_set_entry(14, (unsigned long)isr14, 0x08, 0x8E);
	idt_set_entry(15, (unsigned long)isr15, 0x08, 0x8E);
	idt_set_entry(16, (unsigned long)isr16, 0x08, 0x8E);
	idt_set_entry(17, (unsigned long)isr17, 0x08, 0x8E);
	idt_set_entry(18, (unsigned long)isr18, 0x08, 0x8E);
	idt_set_entry(19, (unsigned long)isr19, 0x08, 0x8E);
	idt_set_entry(20, (unsigned long)isr20, 0x08, 0x8E);
	idt_set_entry(21, (unsigned long)isr21, 0x08, 0x8E);
	idt_set_entry(22, (unsigned long)isr22, 0x08, 0x8E);
	idt_set_entry(23, (unsigned long)isr23, 0x08, 0x8E);
	idt_set_entry(24, (unsigned long)isr24, 0x08, 0x8E);
	idt_set_entry(25, (unsigned long)isr25, 0x08, 0x8E);
	idt_set_entry(26, (unsigned long)isr26, 0x08, 0x8E);
	idt_set_entry(27, (unsigned long)isr27, 0x08, 0x8E);
	idt_set_entry(28, (unsigned long)isr28, 0x08, 0x8E);
	idt_set_entry(29, (unsigned long)isr29, 0x08, 0x8E);
	idt_set_entry(30, (unsigned long)isr30, 0x08, 0x8E);
	idt_set_entry(31, (unsigned long)isr31, 0x08, 0x8E);
	idt_set_entry(0x32,(unsigned long)isr50,0x08,0xEE);

	for(int i = 0; i<256; ++i)
		ISRs[i] = 0; //reset it to NULL after install
}
unsigned char *exception_messages[] =
{
	"Division By Zero",
	"Debug",
	"NMI",
	"Breakpoint",
	"Overflow",
	"Out of Bounds",
	"invalid Instruction",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check Exception",
	"Machine Check Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void fault_handler(struct regs *r){
	void (*handler)(struct regs *r) = ISRs[r->intrr_no];
	if(r->intrr_no == 0xffffff32){ //syscall_dispatcher
		handler = ISRs[0x32];
	}
 	if(handler){
		handler(r);
		return;
	}
	else
		printf("\nException: %s\nError Code: %d",exception_messages[r->intrr_no],r->err_code);
	halt();
}
