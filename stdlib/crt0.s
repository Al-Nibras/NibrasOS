;*****************************************************************
;*			@Author: Abdulaziz Alfaifi <al-nibras>               *
;*			@Date:   2017-08-11T17:05:27+03:00					 *
;*			@Email:  al-nibras@outlook.sa						 *
;*			@Project: NibrasOS									 *
;*			@Filename: crt0.s								     *
;*			@Last modified by:   al-nibras						 *
;*			@Last modified time: 2017-11-23T22:57:48+03:00		 *
;*****************************************************************

SECTION .text

extern main
extern _fini
extern _init
global _start

_start:
    mov ebp,0x00
    push ebp         ;now EIP=0 in the stackframe
    push ebp         ;now EBP=0 in the stackframe
    mov ebp, esp
    push esi
    push edi

    call _init      ;if needed, to initialize stdio, env, global constructors, threads and such


    pop edi
    pop esi

    call main

    mov edi,eax
    call _exit


;global _init
;_init:
;    ;nothing to initialize for user-space right now
;    push ebp
;    mov ebp, esp
;    ;GCC will fill this space in between
;    pop ebp
;    ret


global _exit
_exit:
    call _fini
    mov eax,6
    int 0x32
    jmp $

;global _fini
;_fini:
;    ;nothing to finalize for user-space right now
;    push ebp
;    mov ebp, esp
;    pop ebp
;    ret
