#ifndef __CPU_H
#define __CPU_H

extern unsigned char is_sse_enabled; //defined in cpu.c
//! CPUID operations.
enum
{
	//! Get the cpu vendor string.
	GET_CPU_VENDOR	= 0,
	//! Get cpu informations.
	GET_CPU_INFO	= 1,
	//! Get cpu cache informations.
	GET_CPU_CACHE	= 2,
	//! Check if the brand string is available.
	CPU_BRAND_STRING_IS_AVAILABLE	= 0x80000000,
	//! Get 1st part of the brand string.
	CPU_GET_BRAND_STRING1		= 0x80000002,
	//! Get 1st part of the brand string.
	CPU_GET_BRAND_STRING2		= 0x80000003,
	//! Get 1st part of the brand string.
	CPU_GET_BRAND_STRING3		= 0x80000004,
};

//! The CPU flags mask in register EDX after a CPUID instruction.
struct cpu_flags_edx_t
{
	unsigned long fpu	: 1;	//!< Floating-point unit on-chip.
	unsigned long vme	: 1;	//!< Virtual mode extension.
	unsigned long de 	: 1;	//!< Debugging extension.
	unsigned long pse	: 1;	//!< Page size extension.
	unsigned long tsc	: 1;	//!< Time stamp counter.
	unsigned long msr	: 1;	//!< Model specific register.
	unsigned long pae	: 1;	//!< Physical address extension.
	unsigned long mce	: 1;	//!< Machine check exception.
	unsigned long cx8	: 1;	//!< cmpxchg8 instruction supported.
	unsigned long apic	: 1;	//!< On-chip APIC hardware support.
	unsigned long _res0	: 1;	//!< Reserved.
	unsigned long sep	: 1;	//!< Fast system call.
	unsigned long mtrr	: 1;	//!< Memory type range register.
	unsigned long pge	: 1;	//!< Page Global enable.
	unsigned long mca	: 1;	//!< Machine check architecture.
	unsigned long cmov	: 1;	//!< Conditional move instruction.
	unsigned long pat	: 1;	//!< Page attribute table.
	unsigned long pse_36: 1;	//!< 36-bit page size extension.
	unsigned long psn	: 1;	//!< Processor serial number present.
	unsigned long clfsh	: 1;	//!< clflush instruction supported.
	unsigned long _res1	: 1;	//!< Reserved.
	unsigned long ds	: 1;	//!< Debug store.
	unsigned long acpi	: 1;	//!< Thermal monitor and controlled clock support.
	unsigned long mmx	: 1;	//!< MMX technology support.
	unsigned long fxsr	: 1;	//!< Fast floating point save and restore.
	unsigned long sse	: 1;	//!< Streaming SIMD extension support.
	unsigned long sse2	: 1;	//!< Streaming SIMD extension 2 support.
	unsigned long ss	: 1;	//!< Self-snoop.
	unsigned long htt	: 1;	//!< Hyper-threading technology.
	unsigned long tm	: 1;	//!< Thermal monitor supported.
	unsigned long _res2	: 1;	//!< Reserved.
	unsigned long sbf	: 1;	//!< Signal break on ferr.
} __attribute__ ((packed));

//! The CPU flags mask in register ECX after a CPUID instruction.
struct cpu_flags_ecx_t
{
	unsigned long sse3		: 1; //!< Streaming SIMD Extensions 3.
	unsigned long pclmulqdq	: 1; //!< PCLMULQDQ Instruction
	unsigned long dtes64	: 1; //!< 64-Bit Debug Store Area
	unsigned long monitor	: 1; //!< MONITOR/MWAIT
	unsigned long ds_cpl	: 1; //!< CPL Qualified Debug Store
	unsigned long vmx		: 1; //!< Virtual Machine Extensions
	unsigned long smx		: 1; //!< Safer Mode Extensions
	unsigned long est		: 1; //!< Enhanced SpeedStep Technolog
	unsigned long tm2		: 1; //!< Thermal monitor 2.
	unsigned long ssse3 	: 1; //!< Supplemental Streaming SIMD Extensions 3
	unsigned long cntxt_id 	: 1; //!< L1 Context ID
	unsigned long fma 		: 1; //!< Fused Multiply Add
	unsigned long cx16 		: 1; //!< CMPXCHG16B Instruction
	unsigned long xtpr 		: 1; //!< xTPR Update Control
	unsigned long pdcm 		: 1; //!< Perf/Debug Capability MSR
	unsigned long pcid 		: 1; //!< Process-context Identifier
	unsigned long dca  		: 1; //!< Direct Cache Access
	unsigned long sse41  	: 1; //!< Streaming SIMD Extensions 4.1
	unsigned long sse42  	: 1; //!< Streaming SIMD Extensions 4.2
	unsigned long x2apic  	: 1; //!< Extended xAPIC Support
	unsigned long movbe  	: 1; //!< MOVBE Instruction
	unsigned long popcnt  	: 1; //!< POPCNT Instruction
	unsigned long tsc  		: 1; //!< Local APIC supports TSC Deadline
	unsigned long aesni  	: 1; //!< AESNI Instruction
	unsigned long xsave  	: 1; //!< XSAVE/XSTOR States
	unsigned long osxsave  	: 1; //!< OS Enabled Extended State Management
	unsigned long avx  		: 1; //!< AVX Instructions
	unsigned long f16c  	: 1; //!< 16-bit Floating Point Instructions
	unsigned long rdrand  	: 1; //!< RDRAND Instruction
} __attribute__ ((packed));

//! The CPU signature mask.
struct cpu_signature_t
{
	unsigned long stepping	: 4;	//! Stepping ID.
	unsigned long model 	: 4;	//! Model.
	unsigned long family 	: 4;	//! Family.
	unsigned long type 	: 2;	//! Processor type.
	unsigned long _res0 	: 2;	//! Reserved.
	unsigned long ext_model	: 4;	//! Extended Model.
	unsigned long ext_family: 8;	//! Extended Family.
	unsigned long _res1	: 4;	//! Reserved.
} __attribute__ ((packed));

//! All the CPU informations.
struct cpuinfo_t
{
	//! The name of the CPU.
	char name[64];
	//! The frequency in kHz.
	unsigned int frequency;
	//! How many loops the CPU does in a clock tick.
	unsigned long loops_per_tick;

	//Si dovrebbe aggiungere la variabile per la cache della cpu

	//! Vendor string.
	union
	{
		unsigned int num[4];
		char string[16];
	} vendor;
	//! CPU signature.
	union
	{
		unsigned int num;
		struct cpu_signature_t flags;
	} signature;
	//! Feature flags.
	union
	{
		unsigned int num[2];
		struct
		{
			struct cpu_flags_ecx_t ecx;
			struct cpu_flags_edx_t edx;
		} flags;
	} feature;
	//! Brand ID.
	union
	{
		unsigned int num;
		unsigned long id : 8;
	} brand;
};

//Global structure for CPU informations.
struct cpuinfo_t cpu;


static inline void prefetch(const void *p)
{
	if((cpu.signature.flags.family == 6 && cpu.signature.flags.model>=7) || cpu.signature.flags.family>=15)	//Pentium III or Pentium IV
		__asm__ __volatile__ ("prefetchnta (%0)" : : "r"(p));
	else if(cpu.signature.flags.family >= 6) //Se AMD Athlon
		__asm__ __volatile__ ("prefetch (%0)" : : "r"(p));
}

void init_cpu(struct cpuinfo_t* CPU);

#endif
