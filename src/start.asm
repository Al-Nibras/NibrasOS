
;*****************************************************************
;*			@Author: Abdulaziz Alfaifi <al-nibras>               *
;*			@Date:   2017-04-11T00:11:42+03:00					 *
;*			@Email:  al-nibras@outlook.sa						 *
;*			@Project: NibrasOS									 *
;*			@Filename: start.asm								 *
;*			@Last modified by:   al-nibras						 *
;*			@Last modified time: 2017-11-23T22:57:48+03:00		 *
;*****************************************************************



;*****************************************************************
;*                    Kernel's entry point                       *
;*****************************************************************

MBALIGN     equ  1<<0                       	 		; align loaded modules on page boundaries
MEMINFO     equ  1<<1                  					; provide memory map
VIDINFO		equ  1<<2							 		; needed to set video modes later
FLAGS       equ (1<<0) | (1<<1) | (1<<2); | (1<<16)     ; this is the Multiboot 'flag' field
MAGIC       equ  0x1BADB002            			 		; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)       					; checksum of above, to prove we are multiboot

extern code
extern bss
extern end

SECTION .multiboot
multiboot:
align 4
dd  MAGIC
dd  FLAGS
dd  CHECKSUM
dd  multiboot
dd  code
dd  bss
dd  end
dd  start
dd  1
dd  1024
dd  768
dd  8


SECTION .text
global start
start:
	;The bootloader has loaded Kernel into 32-bit protected mode on a x86 CPU now
	mov esp, _sys_stack     ;This points the stack to our new stack area
	and esp, -16			;Ensure stack is 16-bit aligned
	push ebx 				;Push mboot struct initialized by GRUB
	push eax				;Push Multiboot magic value
  cli
	extern kernel_main
	call kernel_main
  jmp $



    ;*****************************************************************
    ;*                    GDT configuration                          *
    ;*****************************************************************
	global gdt_flush
	extern gdt_pointer

	gdt_flush:
		lgdt [gdt_pointer]

		mov ax, 0x10 ; 0x10 kernel's data segment
		mov ds,ax
		mov es,ax
		mov fs,ax
		mov gs,ax
		mov ss,ax

		jmp 0x08:flush ; far jump kernel's CS:Offset

	flush:
		ret ; return to our C code



    ;*****************************************************************
    ;*                    IDT configuration                          *
    ;*****************************************************************
        global idt_load
        extern idt_pointer
            idt_load:
            lidt [idt_pointer]
            ret



    ;*****************************************************************
    ;*                     ISR configuration                	     *
    ;*****************************************************************
    global isr0
    global isr1
    global isr2
    global isr3
    global isr4
    global isr5
    global isr6
    global isr7
    global isr8
    global isr9
    global isr10
    global isr11
    global isr12
    global isr13
    global isr14
    global isr15
    global isr16
    global isr17
    global isr18
    global isr19
    global isr20
    global isr21
    global isr22
    global isr23
    global isr24
    global isr25
    global isr26
    global isr27
    global isr28
    global isr29
    global isr30
    global isr31
	global isr50
	;Devide by zero exception
	isr0:
		cli
		push byte 0 ; dummy zero
		push byte 0 ; interrupt number
		jmp common_ISR
	;Debug Exception (e.g. single-step trap)
	isr1:
		cli
		push byte 0 ; dummy zero
		push byte 1 ; interrupt number
		jmp common_ISR
    ;Non Maskable Interrupt Exception
    isr2:
		cli
		push byte 0 ; dummy zero
		push byte 2 ; interrupt number
		jmp common_ISR
	;Breakpoint Exception
	isr3:
		cli
		push byte 0 ; dummy zero
		push byte 3 ; interrupt number
		jmp common_ISR
	;Overflow Exception
	isr4:
		cli
		push byte 0 ; dummy zero
		push byte 4 ; interrupt number
		jmp common_ISR
	;Out of Bounds Exception
	isr5:
		cli
		push byte 0 ; dummy zero
		push byte 5 ; interrupt number
		jmp common_ISR
	;Illegal Instruction Exception
	isr6:
		cli
		push byte 0 ; dummy zero
		push byte 6 ; interrupt number
		jmp common_ISR
	;No Coprocessor Exception
	isr7:
		cli
		push byte 0 ; dummy zero
		push byte 7 ; interrupt number
		jmp common_ISR
	;Double Fault Exception
	isr8:
		cli
		;Hardware already pushes a value in the Stack
		push byte 8 ; interrupt number
		jmp common_ISR
	;Coprocessor Segment Overrun Exception
	isr9:
		cli
		push byte 0 ; dummy zero
		push byte 9 ; interrupt number
		jmp common_ISR
	;Bad TSS Exception
	isr10:
		cli
		;Hardware already pushes a value in the Stack
		push byte 10 ; interrupt number
		jmp common_ISR
	;Segment Not Present Exception
	isr11:
		cli
		;Hardware already pushes a value in the Stack
		push byte 11 ; interrupt number
		jmp common_ISR
	;Stack Fault Exception
	isr12:
		cli
		;Hardware already pushes a value in the Stack
		push byte 12 ; interrupt number
		jmp common_ISR
	;General Protection Fault Exception
  isr13:
		cli
		;Hardware already pushes a value in the Stack
		push byte 13 ; interrupt number
		jmp common_ISR

  ;Page Fault Exception
	isr14:
		cli
		;Hardware already pushes a value in the Stack
		push byte 14 ; interrupt number
		jmp common_ISR

	;Unknown Interrupt Exception
	isr15:
		cli
		push byte 0
		push byte 15 ; interrupt number
		jmp common_ISR
	;Coprocessor Fault Exception
	isr16:
		cli
		push byte 0
		push byte 16 ; interrupt number
		jmp common_ISR
	;Alignment Check Exception
	isr17:
		cli
		push byte 0
		push byte 17 ; interrupt number
		jmp common_ISR
	;Machine Check Exception
	isr18:
		cli
		push byte 0
		push byte 18 ; interrupt number
		jmp common_ISR


	;Reserved ISRs
	isr19:
		cli
		push byte 0
		push byte 19 ; interrupt number
		jmp common_ISR
	isr20:
		cli
		push byte 0
		push byte 20 ; interrupt number
		jmp common_ISR
	isr21:
		cli
		push byte 0
		push byte 21 ; interrupt number
		jmp common_ISR
	isr22:
		cli
		push byte 0
		push byte 22 ; interrupt number
		jmp common_ISR
	isr23:
		cli
		push byte 0
		push byte 23 ; interrupt number
		jmp common_ISR
	isr24:
		cli
		push byte 0
		push byte 24 ; interrupt number
		jmp common_ISR
	isr25:
		cli
		push byte 0
		push byte 25 ; interrupt number
		jmp common_ISR
	isr26:
		cli
		push byte 0
		push byte 26 ; interrupt number
		jmp common_ISR
	isr27:
		cli
		push byte 0
		push byte 27 ; interrupt number
		jmp common_ISR
	isr28:
		cli
		push byte 0
		push byte 28 ; interrupt number
		jmp common_ISR
	isr29:
		cli
		push byte 0
		push byte 29 ; interrupt number
		jmp common_ISR
	isr30:
		cli
		push byte 0
		push byte 30 ; interrupt number
		jmp common_ISR
	isr31:
		cli
		push byte 0
		push byte 31 ; interrupt number
		jmp common_ISR
	isr50:
		cli
		push byte 0
		push byte 0x32 ; interrupt number
		jmp common_ISR

extern fault_handler 		; Kernel C-Level Handler, written in main.c

common_ISR:
	pushad
	push ds
	push es				;Notice that I don't need to push `SS` because it's pushed by CPU automatically along with `ESP`
	push fs
	push gs

	mov ax, 0x10 		;kernel's Data Segment Discriptor
	mov ds,ax
	mov ss,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	mov eax,esp
	push eax	;saving Stack pointer and parameterize it to fault_handler(), we cannot use this: push esp, illegal

	mov eax, fault_handler
	call eax

	pop eax
	pop gs
	pop fs
	pop es
	pop ds

	popad
	add esp,8       ; cleanup stack, 8 bytes for: dummy zero and interrupt number pushed onto stack

	sti
	iretd		;pops 5 things at once: cs, eip, eflags, ss, esp



;*****************************************************************
;*                     IRQ configuration                	     *
;*****************************************************************
		extern tasking_enabled

		global irq0
		global irq1
		global irq2
		global irq3
		global irq4
		global irq5
		global irq6
		global irq7
		global irq8
		global irq9
		global irq10
		global irq11
		global irq12
		global irq13
		global irq14
		global irq15

		;IRQs from 0 through 15 are mapped to IDT entries from 32 to 47
		irq0:
			cli
			push byte 0
			push byte 32 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq1:
			cli
			push byte 0
			push byte 33 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq2:
			cli
			push byte 0
			push byte 34 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq3:
			cli
			push byte 0
			push byte 35 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq4:
			cli
			push byte 0
			push byte 36 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq5:
			cli
			push byte 0
			push byte 37 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq6:
			cli
			push byte 0
			push byte 38 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq7:
			cli
			push byte 0
			push byte 39 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq8:
			cli
			push byte 0
			push byte 40 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq9:
			cli
			push byte 0
			push byte 41 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq10:
			cli
			push byte 0
			push byte 42 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq11:
			cli
			push byte 0
			push byte 43 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq12:
			cli
			push byte 0
			push byte 44 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq13:
			cli
			push byte 0
			push byte 45 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq14:
			cli
			push byte 0
			push byte 46 ; IDT entry number(like an interrupt number)
			jmp common_IRQ
		irq15:
		cli
		push byte 0
		push byte 47 ; IDT entry number(like an interrupt number)
		jmp common_IRQ

extern irq_handler ; defined in irq.c
common_IRQ:
	cmp DWORD [tasking_enabled],1
	jz timer

	pushad
	push ds
	push es
	push fs
	push gs

	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	mov eax,esp
	push eax 		;saving Stack pointer and parameterize it to irq_handler(), we cannot use this: push esp, illegal

	mov eax, irq_handler
	call eax
	pop eax

	pop gs
	pop fs
	pop es
	pop ds
	popad

	add esp,8
	sti
	iretd

extern TSS
timer:
	pushad
	push ds
	push es
	push fs
	push gs

	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	mov eax,esp
	push eax 		;saving Stack pointer and parameterize it to irq_handler(), we cannot use this: push esp, illegal

	mov eax, irq_handler
	call eax
	pop eax

	pop gs
	pop fs
	pop es
	pop ds
	popad

	add esp,8
	push eax
	mov eax,[TSS+28]
	mov cr3,eax
	pop eax
	iretd
;***************************************************************************************************
;*								I/O Helpful functions											  *
;***************************************************************************************************

global test_PS2_controller
test_PS2_controller:
	mov al,0xaa
	out 0x64, al
	in al, 0x60

	cmp al, 0x55 			;0x55 test succeeded, 0xFC test failed
	jz .success
	jmp .fail

	.success:
		mov eax,0
		jmp .end
	.fail:
		mov eax,-1
	.end:
		ret

global set_kb_controller_repeate_delay
set_kb_controller_repeate_delay:
		mov al,0xf3
		out 0x64,al
		mov al,[esp+4]
		out 0x64, al
		in al, 0x60

		cmp al, 0x55 			;0x55 test succeeded, 0xFC test failed
		jz .success
		jmp .fail

		.success:
			mov eax,0
			jmp .end
		.fail:
			mov eax,-1
		.end:
			ret

global read_port
read_port: 					;parameter should be 32-bits
	mov edx, [esp + 4]
	in al, dx
	ret

global write_port
write_port:
	mov   edx, [esp + 4]    ;parameter should be 32-bits
	mov   al, [esp + 4 + 4] ;parameter should be 32-bits
	out   dx, al  			;return value in eax: 32-bits
	ret

;getting CPU information
global cpuid
_cpuid:
	mov eax,[esp+4]
	cpuid
	ret

global test_cpu_mode
test_cpu_mode:
	mov eax,cr0
	and eax,0x1
	ret


global flush_TLB
flush_TLB:
    cli
	invlpg	[esp+4]
	sti
    ret

global halt
halt:
  hlt
  jmp halt

global system_reset
system_reset:     ;sends cpu_reset command to Keyboard Controller
mov al,0xfe
out 0x64,al
ret

global enable_sse
enable_sse:
	;now enable SSE and the like
	mov eax, cr0
	and ax, 0xFFFB		;clear coprocessor emulation CR0.EM
	or ax, 0x2			;set coprocessor monitoring  CR0.MP
	mov cr0, eax
	mov eax, cr4
	or ax, 3 << 9		;set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
	mov cr4, eax
	ret

global fast_memcpy
fast_memcpy:
	mov edi,  [esp+4];   //dest pointer
	mov esi, [esp+8]  ;  //src pointer
   	mov ebx, [esp+12]  ;   //ebx is our counter
   	shr ebx, 7		  ;      //divide by 128 (8 * 128bit registers)


   loop_copy:
	 prefetchnta [ESI+128]; //SSE2 prefetch
	 prefetchnta [ESI+160];
	 prefetchnta [ESI+192];
	 prefetchnta [ESI+224];

	 movdqa xmm0, [ESI+0]; //move data from src to registers
	 movdqa xmm1, [ESI+16];
	 movdqa xmm2, [ESI+32];
	 movdqa xmm3, [ESI+48];
	 movdqa xmm4, [ESI+64];
	 movdqa xmm5, [ESI+80];
	 movdqa xmm6, [ESI+96];
	 movdqa xmm7, [ESI+112];

	 movntdq [EDI+0], xmm0; //move data from registers to dest
	 movntdq [EDI+16], xmm1;
	 movntdq [EDI+32], xmm2;
	 movntdq [EDI+48], xmm3;
	 movntdq [EDI+64], xmm4;
	 movntdq [EDI+80], xmm5;
	 movntdq [EDI+96], xmm6;
	 movntdq [EDI+112], xmm7;

	 add esi, 128;
	 add edi, 128;
	 dec ebx;

	 jnz loop_copy; //loop please
   loop_copy_end:
   	mov eax,ebx
	ret

;*********************************************************************
;*                        	    Multi-Tasking 		                 *
;*********************************************************************
global flush_tss
flush_tss:
	mov ax, 0x2b     ; Load the index of our TSS structure - The index is
					  ; 0x28, as it is the 5th selector and each is 8 bytes
				      ; long, but we set the bottom two bits (making 0x2B)
					  ; so that it has an RPL of 3, not zero.
	ltr ax           ; Load 0x2B into the task state register.
  	ret


global usermode_switch
usermode_switch:
	cli
	mov ax,0x23					;0x23= 0x20=gdt_entries[0x8*0x4]=>(User's DS){0010 0000} | {0000 0011}=>0010 0011 (0x23), lowest two bits are set for Usermode-RPL
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	mov eax,esp
	push 0x23					;0x23= 0x20=gdt_entries[0x8*0x4]=>(User's DS){0010 0000} | {0000 0011}=>0010 0011 (0x23), lowest two bits are set for Usermode-RPL
	push eax					;current ESP
	pushfd						;EFLAGS

	pop eax
	or eax, 0x200				; enable IF in EFLAGS
	push eax

	push 0x1b					;0x1b= 0x18=gdt_entries[0x8*0x3]=>(User's CS){0001 1000} | {0000 0011}=>0001 1011 (0x1b), lowest two bits are set for Usermode-RPL

	lea eax,[a]			;we just need a way to save EIP(next instruction to execute),the is one of many ways
	push eax

	;after we've built the stackframe for usermode, we are ready to execute iretd or SYSEXIT(in some Archs)
	iretd
	a:
		;add esp,4			;reset Stack
		ret

global v86_mode_switch
v86_mode_switch:
	cli
	mov ax,0x23					;0x23= 0x20=gdt_entries[0x8*0x4]=>(User's DS){0010 0000} | {0000 0011}=>0010 0011 (0x23), lowest two bits are set for Usermode-RPL
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	mov eax,esp
	push 0x23					;0x23= 0x20=gdt_entries[0x8*0x4]=>(User's DS){0010 0000} | {0000 0011}=>0010 0011 (0x23), lowest two bits are set for Usermode-RPL
	push eax					;current ESP
	pushfd						;EFLAGS

	pop eax
	or eax, 0x200				; enable IF in EFLAGS
	push eax

	push 0x1b					;0x1b= 0x18=gdt_entries[0x8*0x3]=>(User's CS){0001 1000} | {0000 0011}=>0001 1011 (0x1b), lowest two bits are set for Usermode-RPL

	lea eax,[_eip_]			;we just need a way to save EIP(next instruction to execute),the is one of many ways
	push eax

	;after we've built the stackframe for usermode, we are ready to execute iretd or SYSEXIT(in some Archs)
	iretd
	_eip_:
		add esp,4			;reset Stack






global copy_page_physical
copy_page_physical:
   push ebx              ; According to __cdecl, we must preserve the contents of EBX.
   pushf                 ; push EFLAGS, so we can pop it and reenable interrupts
                         ; later, if they were enabled anyway.
   cli                   ; Disable interrupts, so we aren't interrupted.
                         ; Load these in BEFORE we disable paging!
   mov ebx, [esp+12]     ; Source address
   mov ecx, [esp+16]     ; Destination address

   mov edx, cr0          ; Get the control register...
   and edx, 0x7fffffff   ; and...
   mov cr0, edx          ; Disable paging.

   mov edx, 1024         ; 1024*4bytes = 4096 bytes to copy

.loop:
   mov eax, [ebx]        ; Get the word at the source address
   mov [ecx], eax        ; Store it at the dest address
   add ebx, 4            ; Source address += sizeof(word)
   add ecx, 4            ; Dest address += sizeof(word)
   dec edx               ; One less word to do
   jnz .loop

   mov edx, cr0          ; Get the control register again
   or  edx, 0x80000000   ; and...
   mov cr0, edx          ; Enable paging.

   popf                  ; Pop EFLAGS back.
   pop ebx               ; Get the original value of EBX back.
   ret


global copy_page
copy_page:
  push ebx              ; According to __cdecl, we must preserve the contents of EBX.
  pushf                 ; push EFLAGS, so we can pop it and reenable interrupts
                        ; later, if they were enabled anyway.
  cli                   ; Disable interrupts, so we aren't interrupted.
                        ; Load these in BEFORE we disable paging!
  mov ebx, [esp+12]     ; Source address
  mov ecx, [esp+16]     ; Destination address

  mov edx, cr0          ; Get the control register...
  and edx, 0x7fffffff   ; and...
  mov cr0, edx          ; Disable paging.

  mov edx, 1024         ; 1024*4bytes = 4096 bytes to copy

.loop:
  mov eax, [ebx]        ; Get the word at the source address
  mov [ecx], eax        ; Store it at the dest address
  add ebx, 4            ; Source address += sizeof(word)
  add ecx, 4            ; Dest address += sizeof(word)
  dec edx               ; One less word to do
  jnz .loop

  mov edx, cr0          ; Get the control register again
  or  edx, 0x80000000   ; and...
  mov cr0, edx          ; Enable paging.

  popf                  ; Pop EFLAGS back.
  pop ebx               ; Get the original value of EBX back.
  ret
;*********************************************************************
;*                        	  Syscalls	  			                 *
;*********************************************************************

global um_execute
um_execute: ;um_execute(proc_esp,proc_eip)

	;proc_esp
	;proc_eip
	;0x23
	;proc_esp
	;0x200
	;0x1b
	;proc_eip

	cli
	mov ax,0x23
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	mov eax,[esp+4]
	push 0x23
	push eax
	push DWORD 0x200
	push 0x1b

	mov eax,[esp+24]
	push eax

	iretd

global um_exit
um_exit:
	cli
	mov eax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	sti
	mov eax, [esp+4]
	push eax
	ret
global read_eip
read_eip:
	mov eax,[ebp+4]
	ret


global sys_acquire
sys_acquire:
	mov eax,[esp+4] ;[esp+4]: pointer to data
	wait_mutex:
	lock bts DWORD [eax], 0
	pause
	jc wait_mutex
	ret
global sys_release
sys_release:
	mov eax, [esp+4]
    mov DWORD [eax], 0
	ret

;*********************************************************************
;*                        Kernel's Data	  			                 *
;*********************************************************************
SECTION .bss
align 4
global _sys_stack_bottom
_sys_stack_bottom:
	resb 1048576               ; Kernel's Stack reserves 1MB of memory here
global _sys_stack
_sys_stack:
