#ifndef __TYPES_H
#define __TYPES_H

#define UINT32_MAX 4294967295
#define INT32_MAX  2147483647
#define UINT16_MAX 65535
#define INT16_MAX  32767
#define UINT8_MAX  255
#define INT8_MAX   127


typedef unsigned long u64int,uint64_t;
typedef long s64int,int64_t;

typedef unsigned int u32int,uint32_t;
typedef int s32int,int32_t;
typedef unsigned int size_t;

typedef unsigned short u16int,uint16_t;
typedef short s16int,int16_t;

typedef unsigned char u8int,uint8_t;
typedef char s8int,int8_t;

#define NULL 0


/* this type can be used to produce dynamic signed and sized types */
struct flex_type{
  union{
    u64int u64;
    u32int u32;
    u16int u16;
    u8int u8;
  }u; //unsigned
  union{
    s64int s64;
    s32int s32;
    s16int s16;
    s8int s8;
  }s;//signed
};

/* This defines what the stack looks like after an ISR was running */
struct regs
{	/* The order is important and it's set according to Stack in start.asm */
	unsigned int gs, fs, es, ds;      					          /* already pushed by start.asm: common_ISR */
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;          /* pushed by 'pusha' instruction in common_ISR */
	unsigned int intrr_no, err_code;    				          /* pushed by each isr function in start.asm: isrN */
	unsigned int eip, cs, eflags, user_esp, ss;     		      /* pushed by the CPU automatically */
} __attribute__((packed));
typedef struct regs register_t,stackframe_t;

#endif /* __TYPES_H */
