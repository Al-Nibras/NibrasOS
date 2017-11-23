#ifndef __TSS_H
#define __TSS_H

#ifndef __GDT_H
    #include <gdt.h>
#endif
#ifndef __KHEAP_H
    #include <kheap.h>
#endif

struct tss{
    /* NOTE: must be packed this way, to match its real hardware structure */
    unsigned int prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    unsigned int esp0;       // The stack pointer to load when we change to kernel mode.
    unsigned int ss0;        // The stack segment to load when we change to kernel mode.
    unsigned int esp1;       // everything below here is unusued now..
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short iomap_base;
}__attribute__((packed));


extern void flush_tss();
extern struct tss TSS; //defined in tss.c

void tss_set_kernel_stack(unsigned short ss,unsigned int esp);
void install_tss(unsigned short gdt_sel, unsigned short kernel_ss, unsigned int kernel_esp);
void switch_to_um();
void enter_v86();
struct tss* create_proc_tss(page_directory_t* proc_dir,unsigned short kernel_ss, unsigned int kernel_esp);
#endif
