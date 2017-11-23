#ifndef __PROCESS_H
#define __PROCESS_H

#ifndef __PAGING_H
    #include <paging.h>
#endif
#ifndef __FAT32_H
    #include <fat32.h>
#endif

#define PROCESS_STATE_READY   0
#define PROCESS_STATE_RUNNING 1


typedef struct proc{
    int pid;
    short prior;
    char state;
    char name[8];
    unsigned int entry_point;
    unsigned int binary_size;

    page_directory_t *pd;
    unsigned long long context_count;
    pte_t* *pages;
    unsigned int pages_count;
    struct proc* next;

    unsigned int user_esp;
    unsigned int user_ebp;
    unsigned int kern_esp;
    unsigned int eip;
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int eflags;
    //struct thrd* threadList;
}proc_t;

void sched();
int sys_fork(unsigned int eip);
int sys_exec(unsigned int eip);
unsigned int fork();
int tasking_init();
extern volatile proc_t *current_process; //defined int process.c
extern unsigned volatile long long proc_id;
extern unsigned int read_eip();         //defined in start.asm
extern void terminate_proc(struct regs*r);
void* create_kernel_stack();

extern char tasking_enabled; //defined int process.c

#endif
