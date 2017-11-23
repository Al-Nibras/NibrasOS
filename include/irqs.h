#ifndef __IRQS_H
#define __IRQS_H

#include <types.h>
/* each irqN() is defined in start.asm:IRQ cinfiguration */
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void irq_remap();
void irq_setup();

/* installs a custom IRQ handler for the given IRQ */
void irq_install_handler(unsigned irq, void (*handler)(struct regs *r));

/* uninstalls the IRQ handler of the given IRQ */
void irq_uninstall_handler(unsigned irq);
void irq_handler(struct regs *r);
void disable_irq(char num);

#endif /* __IRQS_H */
