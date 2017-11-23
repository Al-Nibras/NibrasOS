//  NibrasOS
//  irqs.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-18T23:18:37+03:00.
//	Last modification time on 2017-07-29T23:53:56+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <irqs.h>
/* each irqN() is defined in start.asm:IRQ cinfiguration */

void* IRQs[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void irq_remap(){ 
	/*
		When computer boots, the default PIC mapping to IDT will as fellow:
		IRQ 0..7 - INT 0x8..0xF
	    IRQ 8..15 - INT 0x70..0x77
		since we already set INT 0-32 to Exceptions
		hence, there are conflicts with IRQS 0..7 and Exceptions 0..7
		we need to remap the PICs so that IRQs 0..15 correspond to ISRs 32..47 int IDT entries

	    Master PIC - command(port): 0x20, data: 0x21
		Slave PIC - command(port): 0xA0, data: 0xA1

		How to Program PIC and remap IRQ is explained here:
		http://wiki.osdev.org/PIC
	*/
		asm volatile("cli"); /* clear IF before configuring IRQ (hardware interrupts, PIC) */

		outportb(0x20, 0x11); 	/*restart PIC1 */
		outportb(0xA0, 0x11); 	/* restart PIC2 */

		outportb(0x21, 0x20); 	/* PIC1 IRQ now starts at 32 */
		outportb(0xA1, 0x28); 	/* PIC2 IRQ now starts at 40 */

		/* setup cascading */
		outportb(0x21, 0x04);
		outportb(0xA1, 0x02);

		/* setup complettion */
		outportb(0x21, 0x01);
		outportb(0xA1, 0x01);
		outportb(0x21, 0x0);
		outportb(0xA1, 0x0);

		asm volatile("sti"); /* we must reenable IF to get IRQ working (hardware interrupts, PIC) */
}
void irq_setup(){

	irq_remap();

	/* IDT entries from 0 - 31 are preserved for Exceptions */
	idt_set_entry(32,(unsigned) irq0,0x08,0x8e);
	idt_set_entry(33,(unsigned) irq1,0x08,0x8e);
	idt_set_entry(34,(unsigned) irq2,0x08,0x8e);
	idt_set_entry(35,(unsigned) irq3,0x08,0x8e);
	idt_set_entry(36,(unsigned) irq4,0x08,0x8e);
	idt_set_entry(37,(unsigned) irq5,0x08,0x8e);
	idt_set_entry(38,(unsigned) irq6,0x08,0x8e);
	idt_set_entry(39,(unsigned) irq7,0x08,0x8e);
	idt_set_entry(40,(unsigned) irq8,0x08,0x8e);
	idt_set_entry(41,(unsigned) irq9,0x08,0x8e);
	idt_set_entry(42,(unsigned) irq10,0x08,0x8e);
	idt_set_entry(43,(unsigned) irq11,0x08,0x8e);
	idt_set_entry(44,(unsigned) irq12,0x08,0x8e);
	idt_set_entry(45,(unsigned) irq13,0x08,0x8e);
	idt_set_entry(46,(unsigned) irq14,0x08,0x8e);
	idt_set_entry(47,(unsigned) irq15,0x08,0x8e);
}

/* installs a custom IRQ handler for the given IRQ */
void irq_install_handler(unsigned irq, void (*handler)(struct regs *r)){
	if(handler && irq >= 0 && irq < 16)
		IRQs[irq] = (unsigned)handler;
}
/* uninstalls the IRQ handler of the given IRQ */
void irq_uninstall_handler(unsigned irq){
	IRQs[irq] = 0; //NULL
}

void irq_handler(struct regs *r){
	/* Pointer to a irq_handler fucntion */
	void (*handler)(struct regs *r) = IRQs[r->intrr_no - 32];

	if (r->intrr_no >= 40)
		 outportb(0xA0, 0x20); /* Send reset signal to slave. */

	/* Send reset signal to master. (As well as slave, if necessary): send EOI to the master interrupt controller */
	outportb(0x20, 0x20);

	if(handler)
			handler(r);
}

void disable_irq(char num){
	if(num >=0 && num <16){
		/*
			;assembly code
			in ax, 0x21
			or ax, (1 << num)
			out 0x21, ax
		*/
	}
}
