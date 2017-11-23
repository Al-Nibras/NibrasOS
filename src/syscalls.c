/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-08-019T01:17:41+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: syscalls.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T23:02:31+03:00
 */



#include <syscalls.h>

void* syscalls[MAX_SYSCALL] = {
    &putc,
    &puts,
    &puthex,
    &putlhex,
    &putdec,
    &printf,
    &sys_exit,
    &printf_clr,
    &lock,
    &unlock,
    &fork,
    &sys_idle
 };

void syscall_dispatcher(struct regs *r){
    // Firstly, check if the requested syscall number is valid.
   // The syscall number is found in EAX.
   //puts("syscall_dispatcher...\n");
   if (r->eax >= MAX_SYSCALL || r->eax < 0) return;
        //asm volatile("iret");

    if(r->eax == 6) /* sys_exit */{
        terminate_proc(r);
        return;
    }
    else if(r->eax == 10) /* sys_fork */{
        fork(r);
        return;
    }
    else if(r->eax == 5){
        if(r->ecx == 0xC)
            set_text_color(0x0B,0x00);
        vsprintf(0, putc, r->ecx, r->ebx);
        return;
    }
   // Get the required syscall location.
   void *syscall = syscalls[r->eax];

   // We don't know how many parameters the function wants, so we just
   // push them all onto the stack in the correct order. The function will
   // use all the parameters it wants, and we can pop them all back off afterwards.
   int ret;
   asm volatile (
  			"push %1\n"
  			"push %2\n"
  			"push %3\n"
  			"push %4\n"
  			"push %5\n"
  			"call *%6\n"
  			"pop %%ebx\n"
  			"pop %%ebx\n"
  			"pop %%ebx\n"
  			"pop %%ebx\n"
  			"pop %%ebx\n"
  			: "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (syscall)
        );
    reset_text_color();
    r->eax = ret;
}
void syscall_setup(){
    isr_install_handler(0x32,&syscall_dispatcher);
}

void sys_write(const char* str, ...){
    va_list ap;
    va_start(ap, str);
    printf(ap);
    //sys_write(ap);
    va_end(ap);
}
void sys_idle(){
    while(1) __asm__ __volatile__ ("pause");
}
void sys_exit(){
    //terminate_proc();
}
