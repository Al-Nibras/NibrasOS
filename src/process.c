/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-07-13T08:01:58+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: process.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:56:27+03:00
 */



#include<process.h>

static unsigned int volatile KERNEL_STACK_ALLOC_TOP = 0xFFF01000;
static unsigned int volatile KERNEL_STACK_ALLOC_BASE = 0xFFFF1000;
static unsigned int volatile CACHE_PAGE = 0xFFFFEFFF;
static unsigned int volatile CACHE_PAGE_VIRT = 0;

char tasking_enabled = 0;

#define MAX_PROCESS_COUNT 512


volatile proc_t *current_process;
volatile size_t procs_count = 0;
volatile proc_t *idle_task;

volatile proc_t*  *ready_queue;
volatile size_t   queue_last, queue_first;

static unsigned int kernel_stack;
static unsigned int kernel_cache_page;

unsigned volatile long long proc_id = 0;

static unsigned int idle = 0;

void clear_queue() {
    memset(ready_queue,0,procs_count+1);
    queue_first = 0;
    queue_last  = 0;
}
void queue_insert(proc_t *task) {
    // if(procs_count)
    //     ready_queue = krealloc(ready_queue,sizeof(proc_t*)*(procs_count+1));
    if(ready_queue)
        ready_queue[queue_last++] = task;
    else {
        ready_queue = kmalloc(sizeof(proc_t*)*MAX_PROCESS_COUNT);
        if(!ready_queue) return;
        ready_queue[queue_last++] = task;
    }
    procs_count++;
}
proc_t* queue_remove(){
    ready_queue[queue_first] = 0;
    --procs_count;
    return ready_queue[++queue_first];
}
proc_t* queue_peek(){
    return ready_queue[queue_first];
}
void dispatch(){
    queue_insert(current_process);
    queue_remove();
}
int tasking_init(){
    idle = read_eip();
    kernel_cache_page = kmalloc(4096);
    asm ("cli");
    ready_queue = kmalloc(sizeof(proc_t*)*MAX_PROCESS_COUNT); //gets expanded every time a process is added
    if(!ready_queue) return -1;
    clear_queue();
    tss_change_page_directory(kernel_page_directory);


    //kernel_cache_page = mem_alloc_page(0);
    //vm_map_page(CACHE_PAGE,kernel_cache_page,1, 1, 1); //identity map it into kerne's pd


    //loading all /bin/* processes
    for(size_t i = 0; i<bin_cnt; ++i){
        proc_t *task = kmalloc(sizeof(proc_t));
        if(!task) return -1;

        kernel_stack = mem_alloc_page(0);
        vm_map_page(KERNEL_STACK_ALLOC_TOP-0x1000,kernel_stack,1, 1, 0); //identity map it into kerne's pd

        memset(task,0,sizeof(proc_t));
        task->pid = proc_id++;

        task->pd = vm_create_page_directory();

        unsigned int eip;
        if(elf32_load_proc(task,proc_names[i],&eip)<0) return -1;
        task->eip = eip;

        memcpy(task->name,proc_names[i],8);

        unsigned int stack_block = mem_alloc_page(0);
        vm_map_page_in_dir(task->pd, 0xC0000000-0x3000,stack_block,1, 1, 1);
        task->pages[task->pages_count++] = 0xC0000000-0x3000;
        stack_block = mem_alloc_page(0);
        vm_map_page_in_dir(task->pd, 0xC0000000-0x2000,stack_block,1, 1, 1); //mapping two pages for User Stack
        task->pages[task->pages_count++] = 0xC0000000-0x2000;

        // unsigned int kernel_stack = mem_alloc_page(0);
        // vm_map_page_in_dir(task->pd, 0xC0000000-0x1000,kernel_stack,1, 1, 0);
        // vm_map_page(0xC0000000,kernel_stack,1, 1, 0); //identity map it to kerne's pd
        // task->pages[task->pages_count++] = kernel_stack;
        vm_map_page_in_dir(task->pd, KERNEL_STACK_ALLOC_TOP-0x1000,kernel_stack,1, 1, 0);
        task->kern_esp = KERNEL_STACK_ALLOC_TOP;

        // vm_map_page_in_dir(task->pd, CACHE_PAGE,kernel_cache_page,1, 1, 1); //save cloning headache in fork()


        task->pid = proc_id++;
        task->user_esp = task->user_ebp = 0xC0000000-0x2000;
        task->eflags = 0x202; //standard EFLAGS
        if(!strcmp(proc_names[i],"idle") || !strcmp(proc_names[i],"IDLE"))
            idle_task = task; //just do nothing
        else
            queue_insert(task); //in order to avoid adding idle_proc to ready_queue

        KERNEL_STACK_ALLOC_TOP+=0X1000;
    }
    //vm_switch_to_page_directory(kernel_page_directory); //refresh it

    printf_clr(0x0E,"\n\nMultitasking initialized successfully, %d processes found\n\nStart executing READY_QUEUE\n\n",procs_count);
    tasking_enabled = 1;
    asm ("sti");
    return 0;
}

extern void copy_page(void* dest,void*src);
unsigned int fork(struct regs *r){
    asm ("cli");
    proc_t* parent = current_process;
    if(!parent) return 0;

    parent->eax      = r->eax;
    parent->ebx      = r->ebx;
    parent->ecx      = r->ecx;
    parent->edx      = r->edx;
    parent->esi      = r->esi;
    parent->edi      = r->edi;
    parent->kern_esp = r->esp;
    parent->user_esp = r->user_esp;
    parent->user_ebp = r->ebp;
    parent->eflags   = r->eflags; //standard EFLAGS
    parent->eip      = r->eip;


    proc_t* child = kmalloc(sizeof(proc_t));
    memset(child,0,sizeof(proc_t));
    child->pid = proc_id++;
    child->next = parent->next;
    parent->next = child;

    child->pd = vm_clone_page_directory(parent->pd);
    if(!child->pd) {
        printf_clr(0x04, "ERROR! fork()-> cannot clone parent's virtual address space, fork() aborted\n");
        return 0;
    }

    child->eax      = r->eax;
    child->ebx      = r->ebx;
    child->ecx      = r->ecx;
    child->edx      = r->edx;
    child->esi      = r->esi;
    child->edi      = r->edi;
    child->kern_esp = parent->kern_esp;
    child->user_esp = r->user_esp;
    child->user_ebp = r->ebp;
    child->eflags   = r->eflags; //standard EFLAGS
    child->eip      = r->eip;

    for(int i = 0; i<parent->pages_count; ++i){
        if(parent->pages[i]>= 0xFFF01000)   //it's kernel_stack
            continue;
            void* block = mem_alloc_page(0);
            vm_map_physical_address(child->pd, parent->pages[i],block,1,1,1);
            memcpy(kernel_cache_page,parent->pages[i],4096);
            __asm__ __volatile__("mov %0, %%cr3" :: "r"(child->pd));
            memcpy(parent->pages[i],kernel_cache_page,4096);
            __asm__ __volatile__("mov %0, %%cr3" :: "r"(current_page_directory));
			// unsigned int phys;
			// page_table_t* table = vm_clone_page_table(parent->pd->tables_phys_addrs[i], &phys);
			// child->pd->tables_phys_addrs[i] = phys;
            // vm_pde_set_frame(&child->pd->tables[i], table);
    }
    kernel_stack = mem_alloc_page(0);
    vm_map_physical_address(child->pd, KERNEL_STACK_ALLOC_TOP-0x1000,kernel_stack,1, 1, 0);
    child->kern_esp = KERNEL_STACK_ALLOC_TOP;
    KERNEL_STACK_ALLOC_TOP+=0x1000;
    // child->pages = kmalloc(sizeof(pte_t*)*parent->pages_count);
    // for(int c = 0; c<parent->pages_count; ++c)
    //     child->pages[c] = parent->pages[c];
    // child->pages_count = parent->pages_count;

    // unsigned int stack_block = mem_alloc_page(0);
    //
    // vm_map_physical_address(child->pd,0xC0000000-0x3000,stack_block,1,1,1);
    // child->pages[child->pages_count++] = stack_block;
    // stack_block = mem_alloc_page(0);
    // vm_map_physical_address(child->pd,0xC0000000-0x2000,stack_block,1,1,1);
    // child->pages[child->pages_count++] = stack_block;


    if (current_process == parent) {
        queue_insert(child);

        asm ("sti");
        printf_clr(0x0c,"\nProcess %d forked a child with PID: %d \n",parent->pid,child->pid);
        KERNEL_STACK_ALLOC_TOP+=0x1000;

        return child->pid;
   }
   else {
        asm ("sti");
        return 0;  // it's the child - by convention return 0
    }
}
int sys_fork(unsigned int eip){
    asm ("cli");
    proc_t *parent = current_process;
    proc_t *child = kmalloc(sizeof(proc_t));
    if(!child || !parent) return -1;

    child->pd = vm_get_current_page_directory();
    if(!child->pd) return -1;

    memset(child,0,sizeof(proc_t));
    unsigned int stack_block = mem_alloc_page(0);
    vm_map_page_in_dir(child->pd, 0xC0000000-0x2000,stack_block,1, 1, 1);
    child->pages[child->pages_count++] = stack_block;
    stack_block = mem_alloc_page(0);
    vm_map_page_in_dir(child->pd, 0xC0000000-0x3000,stack_block,1, 1, 1); //mapping two pages for User Stack
    child->pages[child->pages_count++] = stack_block;

    unsigned int kernel_stack = mem_alloc_page(0);
    vm_map_page_in_dir(child->pd, 0xC0000000-0x1000,kernel_stack,1, 1, 0);
    child->pages[child->pages_count++] = kernel_stack;

    child->kern_esp = kernel_stack;
    child->pid = proc_id++;
    child->user_esp = child->user_ebp = 0xC0000000-0x1000;
    child->eip = eip;
    parent->next = child;
    asm ("sti");
    return 0;
}
int sys_exec(unsigned int eip){
    asm ("cli");
    proc_t *task = kmalloc(sizeof(proc_t));
    if(!task) return -1;

    page_directory_t* pd = vm_create_page_directory();
    if(!pd) return -1;

    task->pd = pd;

    memset(task,0,sizeof(proc_t));
    unsigned int stack_block = mem_alloc_page(0);
    vm_map_page_in_dir(task->pd, 0xBFFFF000,stack_block,1, 1, 1);
    task->pages[task->pages_count++] = stack_block;
    stack_block = mem_alloc_page(0);
    vm_map_page_in_dir(task->pd, 0xBFFFF000-0x1000,stack_block,1, 1, 1); //mapping two pages for User Stack
    task->pages[task->pages_count++] = stack_block;

    unsigned int kernel_stack = mem_alloc_page(0);
    vm_map_page_in_dir(task->pd, 0xC0000000,kernel_stack,1, 1, 0);

    task->pages[task->pages_count++] = kernel_stack;


    task->kern_esp = kernel_stack;
    task->pid = proc_id++;
    task->user_esp = task->user_ebp = 0xBFFFF000;
    task->eip = eip;
    task->eflags = 0x202; //standard EFLAGS
    if(current_process){
        proc_t* temp = current_process;
        while(temp->next)
            temp = temp->next;
        temp->next = task;
    }
    else
        current_process = task;
    asm ("sti");
}

char wraning_flag = 0;
void sched(struct regs *r){
    if(!current_process && !ready_queue) return; //indeicates that `tasking_init` hasn't been invoked yet, hence there's no need to `sched`
    //vm_switch_to_page_directory(kernel_page_directory);
    if(!current_process && ready_queue){
        current_process = queue_peek();
        if(!current_process){
            //current_process = idle_task;
            if(!wraning_flag) {printf_clr(0x0c,"\nNo running processes -> kernel idle state ...\n\n"); wraning_flag=1;}
            // __asm__ __volatile__("jmp %0" :: "r"(idle));

            r->eip = idle;
            r->user_esp = r->user_esp;
            r->ebp = r->ebp;
            r->cs = 0x08;
            r->ss = r->ds = r->fs = r->gs = r->es = 0x10;

            r->eflags |= 0x200; //IF enable
            __asm__ __volatile__("mov %0, %%cr3" :: "r"(kernel_page_directory));
            return;
        }
        else
            printf_clr(0x0c,"\nContext switch -> PID %d running...\n\n",current_process->pid);
    }
    else {
        current_process->eip = r->eip;
        current_process->user_esp = r->user_esp;
        current_process->user_ebp = r->ebp;
        current_process->kern_esp = r->esp;
        current_process->eax = r->eax;
        current_process->ebx = r->ebx;
        current_process->ecx = r->ecx;
        current_process->edx = r->edx;
        current_process->esi = r->esi;
        current_process->edi = r->edi;
        current_process->eflags = r->eflags;

        queue_insert(current_process);

        proc_t* next = queue_remove();
        if(next) current_process = next;

        if(strcmp(current_process->name,"IDLE") )
            printf_clr(0x0c,"\nContext switch -> PID %d running...\n\n",current_process->pid);

        // if(!current_process->next) {
            // idle_proc();
            // //there's no process left, create idle process
            // proc_t* idle = kmalloc(sizeof(proc_t));
            // memset(idle,0,sizeof(proc_t));
            // idle->pid = 0;
            // idle->next = current_process;
            // idle->pd = vm_get_current_page_directory();
            // unsigned int stack_block = mem_alloc_page(0);
            // vm_map_page_in_dir(idle->pd, 0xBFFFF000,stack_block,1, 1, 1);
            // stack_block = mem_alloc_page(0);
            // vm_map_page_in_dir(idle->pd, 0xBFFFF000-0x1000,stack_block,1, 1, 1); //mapping two pages for User Stack
            //
            // unsigned int kernel_stack = mem_alloc_page(0);
            // vm_map_page_in_dir(idle->pd, 0xC0000000,kernel_stack,1, 1, 0);
            // tss_set_kernel_stack(0x10,kernel_stack);
            // vm_switch_to_page_directory(current_process->pd);
            // idle->esp = idle->ebp = 0xBFFFF000;
            // idle->eip = &idle_proc;
            // current_process = idle;
        // }
        // current_process = current_process->next;
    }

    r->eip = current_process->eip;
    r->user_esp = current_process->user_esp;
    r->ebp = current_process->user_ebp;
    r->cs = 0x1B;
    r->ss = r->ds = r->fs = r->gs = r->es = 0x23;
    r->esp = current_process->kern_esp;
    r->eax = current_process->eax;
    r->ebx = current_process->ebx;
    r->ecx = current_process->ecx;
    r->edx = current_process->edx;
    r->esi = current_process->esi;
    r->edi = current_process->edi;
    r->eflags = current_process->eflags | 0x200; //IF enable

    //create_proc_tss(current_process->pd,0x10,current_process->kern_esp);
    install_tss(/* gdt_sel */0x05, /* kernel_ss */0x10, /* kern_esp */ current_process->kern_esp);
    tss_change_page_directory(current_process->pd->phys_addr);
    //__asm__ __volatile__("mov %0, %%cr3" :: "r"(current_process->pd->phys_addr));
    current_page_directory = current_process->pd;
}
void* create_kernel_stack(){
    unsigned int phys_addr;
    unsigned int virt_addr;
    void*              ret;

    phys_addr =  mem_alloc_page(0);
    if (!phys_addr) return 0;

    virt_addr = KERNEL_STACK_ALLOC_TOP + PAGE_SIZE;

    vm_map_physical_address(kernel_page_directory,virt_addr,phys_addr,1,1,0);
    ret = (void*) virt_addr;

    KERNEL_STACK_ALLOC_TOP+=PAGE_SIZE;
    return ret;
}

void terminate_proc(struct regs *r){
       //unmap all allocated regions for this process
       for(size_t i = 0; i<current_process->pages_count; ++i){
            vm_free_page(current_process->pd,current_process->pages[i]);
       }
       //unmap current_process's page_directory from kernel_page_directory
       vm_free_page(kernel_page_directory,current_process->pd);
       //all process mess has been cleaned up now
       queue_remove(current_process);
       printf_clr(0x0D,"\nProcess (%s) terminated successfully\n\n",current_process->name);
       memset(current_process,0,sizeof(proc_t));
       kfree(current_process);
       current_process = 0;
       sched(r);
      // asm ("jmp sched"); //to fire sched again
}
