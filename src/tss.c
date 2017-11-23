/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-07-19T12:10:04+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: tss.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T23:02:53+03:00
 */



#include <tss.h>

struct tss TSS;

void tss_set_kernel_stack(unsigned short ss,unsigned int esp){
    TSS.ss0 = ss;
    TSS.esp0 = esp;
}
void install_tss(unsigned short gdt_sel, unsigned short kernel_ss, unsigned int kernel_esp) {
	unsigned int base = (unsigned int) &TSS;
    unsigned int limit = base+sizeof(struct tss);


    memset((void*) &TSS, 0, sizeof(struct tss));
	tss_set_kernel_stack(kernel_ss, kernel_esp);

	TSS.cs=0x0b;
	TSS.ss = TSS.ds = TSS.es = TSS.fs = TSS.gs = 0x13;

    gdt_set_entry(gdt_sel, base, limit, 0xE9, 0x00);
	flush_tss();
    //gdt_flush(); uncomment it if you find it necessary
}

struct tss* create_proc_tss(page_directory_t* proc_dir,unsigned short kernel_ss, unsigned int kernel_esp){
    vm_switch_to_page_directory(proc_dir);
    struct tss *proc_tss = kmalloc(sizeof(struct tss));
    if(!proc_tss) return 0;

    memset(proc_tss, 0, sizeof(struct tss));
    proc_tss->ss0 = kernel_ss;
    proc_tss->esp0 = kernel_esp;

    unsigned int base = (unsigned int) proc_tss;
    unsigned int limit = base+sizeof(struct tss);

    gdt_set_entry(5, base, limit, 0xE9, 0x00);
	__asm__ __volatile__("ltr %%ax" : : "a" (0x2B)); // Selector is 0x28 with RPL 3 so 0x2B
    vm_switch_to_page_directory(kernel_page_directory);
    return proc_tss;
}
void tss_change_page_directory(page_directory_t* dir){
    if(dir)
        TSS.cr3 = (unsigned int)dir;
}
void switch_to_um(){
    unsigned int esp = 0;
    __asm__ __volatile__("mov %%esp, %0": "=r"(esp)); //we should set the stack of kernel before it gets expanded
    tss_set_kernel_stack(0x10,esp);
    usermode_switch();
}
void enter_v86(){
	v86_mode_switch();
}
