/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-06-08T00:10:42+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: cpu.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:50:41+03:00
 */



#include <cpu.h>
#include <string.h>

#define HZ		100
#define LATCH 		((1193180 + HZ / 2) / HZ)
#define CALIBRATE_LATCH (5 * LATCH)
#define CALIBRATE_TIME	(5 * 1000020 / HZ)

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(*(array)))
#define _countof(array) ARRAY_SIZE(array)

//Clear task switch flag.
static inline void clts()
{
	__asm__ __volatile__ ("clts");
}

//Set task switch flag.
static inline void slts()
{
	unsigned int __cr0;
	__asm__ __volatile__ (
		"movl %%cr0, %0\n"
		"or $8, %0\n"
		"movl %0, %%cr0" : "=r"(__cr0));
}

#define EFLAGS_BASE		    0x2

#define EFLAGS_CARRY	    0x1
#define EFLAGS_PARITY	    0x4
#define EFLAGS_AUX_CARRY    0x10
#define EFLAGS_ZERO		    0x40
#define EFLAGS_SIGN		    0x80
#define EFLAGS_TRAP		    0x100
#define EFLAGS_IF		    0x200		//Interrupt enable
#define EFLAGS_DIRECTION    0x400
#define EFLAGS_OVERFLOW	    0x800

#define EFLAGS_IOPL0		0x0		//I/O privilege level 0. Only ring 0 can perform I/O operations
#define EFLAGS_IOPL1		0x1000		//I/O privilege level 1
#define EFLAGS_IOPL2		0x2000		//I/O privilege level 2
#define EFLAGS_IOPL3		0x3000		//I/O privilege level 3. Everyone can perform I/O operations

#define EFLAGS_NESTED_TASK	0x4000
#define EFLAGS_RESUME_FLAG	0x10000
#define EFLAGS_V86		    0x20000		//Virtual-8086 mode
#define EFLAGS_AC		    0x40000
#define EFLAGS_VINT		    0x80000
#define EFLAGS_VINT_PENDING	0x100000	//Alignement check
#define EFLAGS_ID		    0x200000	//CPUID detection

//Read the timestamp counter from the model-specific register
//and put it into eax:edx registers. This is a 64-bit value.
#define rdtscll(val) \
	__asm__ __volatile__( "rdtsc" : "=A"(val) )

//Memory barrier: force strict CPU ordering (required for all
//out-of-order Intel CPUs to preserve the order of execution).
#define barrier() \
	__asm__ __volatile__ ("lock; addl $0,0(%%esp)": : :"memory")

//Write back and invalidate cache. (Only for M386)
#define wbinvd() \
	__asm__ __volatile__ ("wbinvd" : : : "memory");

//No operation.
#define nop() \
	__asm__ __volatile__ ("nop");

//A macro that returns current instruction pointer.
#define get_eip() ({ void *eip; __asm__("movl $1f,%0\n1:" : "=g" (eip)); eip; })

// --- INTERRUPT CONTROL ---------------------------------------------- //

//Enable all interrupts.
#define enable()	__asm__ __volatile__ ("sti" : : : "memory");

//Disable all interrupts.
#define disable()	__asm__ __volatile__ ("cli" : : : "memory");

//Save EFLAGS register.
#define save_flags( flags ) \
	__asm__ __volatile__ ("pushfl ; popl %0" : "=g"(flags) : )

//Restore EFLAGS register.
#define restore_flags( flags ) \
	__asm__ __volatile__ ("pushl %0 ; popfl" : : "g"(flags) : "memory", "cc")

//Interrupt control: save flags and disable irqs.
#define local_irq_save(flags) \
	__asm__ __volatile__ ("pushfl ; popl %0 ; cli" : "=g"(flags) : : "memory")

//Interrupt control: save flags and enable irqs.
#define local_irq_set(flags) \
	__asm__ __volatile__ ("pushfl ; popl %0 ; sti" : "=g"(flags) : : "memory")

//Interrupt control: restore flags.
#define local_irq_restore(flags)	restore_flags(flags)

//Halt the CPU until an interrupt occurs.
#define idle() \
	__asm__ __volatile__ ("hlt" : : : "memory");

//Secure idle.
#define safe_idle() \
	do { enable(); idle();  } while(0)


struct regs_t {
	unsigned long eax;
	unsigned long ebx;
	unsigned long ecx;
	unsigned long edx;
};

struct cpu_name_t
{
	int family;
	char *model_names[16];
};

static struct cpu_name_t intel_cpu_names[] =
{
	{
		.family = 4, .model_names =
		{
			[ 0 ] = "486 DX-25/33",
			[ 1 ] = "486 DX-50",
			[ 2 ] = "486 SX",
			[ 3 ] = "486 DX/2",
			[ 4 ] = "486 SL",
			[ 5 ] = "486 SX/2",
			[ 7 ] = "486 DX/2-WB",
			[ 8 ] = "486 DX/4",
			[ 9 ] = "486 DX/4-WB",
		}
	}, {
		.family = 5, .model_names =
		{
			[ 0 ] = "Pentium 60/66 A-step",
			[ 1 ] = "Pentium 60/66",
			[ 2 ] = "Pentium 75-200",
			[ 3 ] = "Overdrive PODP5V83",
			[ 4 ] = "Pentium MMX",
			[ 7 ] = "Mobile Pentium 75-200",
			[ 8 ] = "Mobile Pentium MMX",
		}
	}, {
		.family = 6, .model_names =
		{
			[ 0  ] = "Pentium Pro A-step",
			[ 1  ] = "Pentium Pro",
			[ 3  ] = "Pentium II (Klamath)",
			[ 4  ] = "Pentium II (Deschutes)",
			[ 5  ] = "Pentium II (Deschutes)",
			[ 6  ] = "Mobile Pentium II",
			[ 7  ] = "Pentium III (Katmai)",
			[ 8  ] = "Pentium III (Coppermine)",
			[ 9  ] = "Mobile Pentium III",
			[ 10 ] = "Pentium III (Cascades)",
			[ 11 ] = "Pentium III (Tualatin)",
			[ 15 ] = "Core2Duo",
		}
	}, {
		.family = 15, .model_names =
		{
			[ 0 ] = "Pentium IV (Unknown)",
			[ 1 ] = "Pentium IV (Willamette)",
			[ 2 ] = "Pentium IV (Northwood)",
			[ 4 ] = "Pentium IV (Foster)",
			[ 5 ] = "Pentium IV (Foster)",
		}
	},
};

static struct cpu_name_t amd_cpu_names[] =
{
	{
		.family = 4, .model_names =
		{
			[ 3  ] = "486 DX/2",
			[ 7  ] = "486 DX/2-WB",
			[ 8  ] = "486 DX/4",
			[ 9  ] = "486 DX/4-WB",
			[ 14 ] = "Am5x86-WT",
			[ 15 ] = "Am5x86-WB",
		}
	}, {
		.family = 5, .model_names =
		{
			[ 0 ] = "K5/SSA5",
			[ 1 ] = "K5",
			[ 2 ] = "K5",
			[ 3 ] = "K5",
			[ 6 ] = "K6",
			[ 7 ] = "K6",
			[ 8 ] = "K6-2",
			[ 9 ] = "K6-3",
			[ 13 ] = "K6-2+"
		}
	}, {
		.family = 6, .model_names =
		{
			[ 0 ] = "Athlon",
			[ 1 ] = "Athlon",
			[ 2 ] = "Athlon",
			[ 3 ] = "Duron",
			[ 4 ] = "Athlon (Thunderbird)",
			[ 6 ] = "Athlon (Palamino)",
			[ 7 ] = "Duron (Morgan)",
			[ 8 ] = "Athlon (Thoroughbred)",
			[ 10 ] = "Athlon (Barton)"
		}
	},
};

struct cache_info_t {
	unsigned char index;
	char *string;
};

//Cache TLB informations
static struct cache_info_t cache_strings[] =
{
	{ 0x01, "Instruction TLB: 4KB Pages 4-way set associative, 32 entries" },
	{ 0x02, "Instruction TLB: 4MB Pages, 4-way set associative, 2 entries" },
	{ 0x03, "Data TLB: 4KB Pages, 4-way set associative, 64 entries" },
	{ 0x04, "Data TLB: 4MB Pages, 4-way set associative, 8 entries" },
	{ 0x06, "1st-level instruction cache: 8KB, 4-way set associative, 32 byte line size" },
	{ 0x08, "1st-level instruction cache: 16KB, 4-way set associative, 32 byte line size" },
	{ 0x0A, "1st-level data cache: 8KB, 2-way set associative, 32 byte line size" },
	{ 0x0C, "1st-level data cache: 16KB, 4-way set associative, 32 byte line size" },
	{ 0x22, "3rd-level cache: 512KB, 4-way set associative, dual-sectored line, 64 byte sector size" },
	{ 0x23, "3rd-level cache: 1MB, 8-way set associative, dual-sectored line, 64 byte sector size" },
	{ 0x40, "No 2nd-level cache or, if processor contains a valid 2nd-level cache, no 3rd-level cache" },
	{ 0x41, "2nd-level cache: 128KB, 4-way set associative, 32 byte line size" },
	{ 0x42, "2nd-level cache: 256KB, 4-way set associative, 32 byte line size" },
	{ 0x43, "2nd-level cache: 512KB, 4-way set associative, 32 byte line size" },
	{ 0x44, "2nd-level cache: 1MB, 4-way set associative, 32 byte line size" },
	{ 0x45, "2nd-level cache: 2MB, 4-way set associative, 32 byte line size" },
	{ 0x50, "Instruction TLB: 4KB and 2MB or 4MB pages, 64 entries" },
	{ 0x51, "Instruction TLB: 4KB and 2MB or 4MB pages, 128 entries" },
	{ 0x52, "Instruction TLB: 4KB and 2MB or 4MB pages, 256 entries" },
	{ 0x5B, "Data TLB: 4KB and 4MB pages, 64 entries" },
	{ 0x5C, "Data TLB: 4KB and 4MB pages, 128 entries" },
	{ 0x5D, "Data TLB: 4KB and 4MB pages, 256 entries" },
	{ 0x66, "1st-level data cache: 8KB, 4-way set associative, 64 byte line size" },
	{ 0x67, "1st-level data cache: 16KB, 4-way set associative, 64 byte line size" },
	{ 0x68, "1st-level data cache: 32KB, 4-way set associative, 64 byte line size" },
	{ 0x70, "Trace cache: 12K-op, 8-way set associative" },
	{ 0x71, "Trace cache: 16K-op, 8-way set associative" },
	{ 0x72, "Trace cache: 32K-op, 8-way set associative" },
	{ 0x79, "2nd-level cache: 128KB, 8-way set associative, dual-sectored line, 64 byte sector size" },
	{ 0x7A, "2nd-level cache: 256KB, 8-way set associative, dual-sectored line, 64 byte sector size" },
	{ 0x7B, "2nd-level cache: 512KB, 8-way set associative, dual-sectored line, 64 byte sector size" },
	{ 0x7C, "2nd-level cache: 1MB, 8-way set associative, dual-sectored line, 64 byte sector size" },
	{ 0x82, "2nd-level cache: 256KB, 8-way set associative, 32 byte line size" },
	{ 0x83, "2nd-level cache: 512KB, 8-way set associative, 32 byte line size" },
	{ 0x84, "2nd-level cache: 1MB, 8-way set associative, 32 byte line size" },
	{ 0x85, "2nd-level cache: 2MB, 8-way set associative, 32 byte line size" },
};

//Check if the 'cpuid' instruction is available.
//This is true if the cpuid detection flag in EFLAGS register
//is changeable.
static inline int cpuid_is_available() {
	int flag1, flag2;

	__asm__ __volatile__ (
		"pushfl\n"
		"pushfl\n"
		"popl %0\n"
		"movl %0, %1\n"
		"xorl %2, %0\n"
		"pushl %0\n"
		"popfl\n"
		"pushfl\n"
		"popl %0\n"
		"popfl\n"
		: "=r"(flag1), "=r"(flag2) : "i"(EFLAGS_ID)
	);
	return( (flag1^flag2) & EFLAGS_ID );
}

//Get the CPUID information.
//Not all the Intel CPUs support the CPUID instruction!!!
//Only some Intel486 family and subsequent Intel processors
//provide this method for determinig the architecture flags.
//Execution of CPUID on a processor that does not support this
//instruction will result in an invalid opcode exception.
//To determine if it is possible to use this instruction we can
//use bit 21 of the EFLAGS register. If software can change the
//value of this flag, the CPUID instruction is executable.
static inline void cpuid(unsigned int op, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
	asm volatile( "cpuid" : "=a" ( *eax ), "=b" ( *ebx ), "=c" ( *ecx ),
		"=d" ( *edx ) : "a" ( op ) : "cc" );
}

//Check if the processor is a 486 or higher.
//This is true if the alignement check bit in EFLAGS register
//is changeable.
static inline int cpu_is_486() {
	int flag1, flag2;

	asm volatile (
		"pushfl\n"
		"pushfl\n"
		"popl %0\n"
		"movl %0, %1\n"
		"xorl %2, %0\n"
		"pushl %0\n"
		"popfl\n"
		"pushfl\n"
		"popl %0\n"
		"popfl\n"
		: "=r"(flag1), "=r"(flag2) : "i"(EFLAGS_AC)
	);
	return( (flag1^flag2) & EFLAGS_AC );
}

//Read the timestamp counter
static inline void rdtsc(unsigned long *low, unsigned long *high)
{
	asm volatile( "rdtsc" : "=a" ( *low ), "=d" ( *high ) );
}

//Calibrate the TSC
static unsigned long calibrate_tsc( void )
{
	unsigned long startlow, starthigh, endlow, endhigh, count;

	outportb( 0x61, ( inportb( 0x61 ) & ~0x02 ) | 0x01 );
	outportb( 0x43, 0xb0 );
	outportb( 0x42, CALIBRATE_LATCH & 0xff );
	outportb( 0x42, CALIBRATE_LATCH >> 8 );
	rdtsc( &startlow, &starthigh );
	count = 0;
	do {
		count++;
	} while ( ( inportb( 0x61 ) & 0x20 ) == 0 );
	rdtsc( &endlow, &endhigh );
 	if ( count <= 1 ) {
		return 0;
	}
	asm ( "subl %2, %0\n"
		"sbbl %3,%1" : "=a" ( endlow ), "=d" ( endhigh ) :
		"g" ( startlow ), "g" ( starthigh ), "0" ( endlow ), "1" ( endhigh ) );
	if ( endhigh != 0 ) {
		return 0;
	}
	if ( endlow <= CALIBRATE_TIME ) {
		return 0;
	}

	asm ( "divl %2" : "=a" ( endlow ), "=d" ( endhigh ) :
		"r" ( endlow ),"0" ( 0 ), "1" ( CALIBRATE_TIME ) );

	return endlow;
}

//Get the CPU speed
static unsigned long speed( void )
{
	unsigned long tsc_quotient = calibrate_tsc();
	unsigned long cpu_hz = 0;

	if(tsc_quotient != 0)
	{
		unsigned long eax = 0, edx = 1000000;
		asm ( "divl %2" : "=a" ( cpu_hz ), "=d" ( edx ) :
			"r" ( tsc_quotient ), "0" ( eax ), "1" ( edx ) );
	}
	return cpu_hz / 1000000;
}

//Get the cpu brand string if it is available.
int cpu_get_brandstring( struct cpuinfo_t *cpu )
{
	unsigned int *s = (unsigned int *)cpu->name;
	char *p, *q;
	unsigned int dummy, eax;

	// Check if brand string is available.
	cpuid( CPU_BRAND_STRING_IS_AVAILABLE,
		&eax, &dummy, &dummy, &dummy);
	if( eax<0x80000004 )
	{
		// Brand string not supported! Zero the cpu name.
		memset( cpu->name, 0, sizeof(cpu->name) );
		return( -1 );
	}

	// Get the cpu brand string.
	cpuid( CPU_GET_BRAND_STRING1, &s[0], &s[1], &s[2], &s[3] );
	cpuid( CPU_GET_BRAND_STRING2, &s[4], &s[5], &s[6], &s[7] );
	cpuid( CPU_GET_BRAND_STRING3, &s[8], &s[9], &s[10], &s[11] );
	cpu->name[48] = '\0';

	// Intel chips right justify the string.
	// Left justify this string.
	p = q = &cpu->name[0];
	while ( *p == ' ' )
		p++;
	if ( p != q )
	{
		while ( *p )
			*q++ = *p++;
		// Zero pad the rest.
		while ( q <= &cpu->name[48] )
			*q++ = '\0';
	}

	// Well done.
	return( 0 );
}

//! Get the cpu model name.
int cpu_get_modelname( struct cpuinfo_t *cpu )
{
	struct cpu_name_t *names;
	int count, i;

	// First of all try to get the brand string.
	if( cpu_get_brandstring(cpu)==0 )
	{
		// Well done, brand string supported.
		return( 0 );
	}

	// Find the vendor of the CPU.
	if( strcmp(cpu->vendor.string, "GenuineIntel")==0 )
	{
		// Intel CPU name.
		// Try to identify the cpu name with the brand ID.
		// This is useful to identify Celeron and Xeon
		// processor.
		switch( cpu->brand.id )
		{
			case 1:
			case 10:
				strncpy( cpu->name,
					"Intel(R) Celeron(R)",
					sizeof(cpu->name) );
				return( 0 );
			break;

			case 3:
				if( cpu->signature.num==0x6b1 )
					strncpy( cpu->name,
						"Intel(R) Celeron(R)",
						sizeof(cpu->name) );
				else
					strncpy( cpu->name,
						"Intel(R) Pentium(R) Xeon",
						sizeof(cpu->name) );
				return( 0 );
			break;

			case 6:
				strncpy( cpu->name,
					"Mobile Intel(R) Pentium(R) III",
					sizeof(cpu->name) );
				return( 0 );
			break;

			case 7:
			case 15:
				strncpy( cpu->name,
					"Mobile Intel(R) Celeron(R)",
					sizeof(cpu->name) );
				return( 0 );
			break;

			case 11:
				if( cpu->signature.num<0xf13 )
					strncpy( cpu->name,
						"Intel(R) Xeon MP",
						sizeof(cpu->name) );
				else
					strncpy( cpu->name,
						"Intel(R) Xeon",
						sizeof(cpu->name) );
				return( 0 );
			break;

			case 12:
				strncpy( cpu->name,
					"Intel(R) Xeon MP",
					sizeof(cpu->name) );
				return( 0 );
			break;

			case 13:
				if( cpu->signature.num<0xf13 )
					strncpy( cpu->name,
						"Intel(R) Xeon",
						sizeof(cpu->name) );
				else
					strncpy( cpu->name,
						"Mobile Intel(R) Pentium 4",
						sizeof(cpu->name) );
			break;
		}
		// Find the cpu name in the intel list.
		names = intel_cpu_names;
		count = _countof( intel_cpu_names );
	}
	else if( strcmp(cpu->vendor.string, "AuthenticAMD")==0 )
	{
		// AMD CPU name.
		// Find the cpu name in the amd list.
		names = amd_cpu_names;
		count = _countof( amd_cpu_names );
	}
	else
	{
		// Copy the unknown string to the cpu name and vendor string.
		strncpy( cpu->vendor.string, "Unknown", sizeof(cpu->vendor.string) );
		switch( cpu->signature.flags.family )
		{
			case 3:
			strncpy( cpu->name, "386", sizeof(cpu->name) );
			break;

			case 4:
			strncpy( cpu->name, "486", sizeof(cpu->name) );
			break;

			default:
			strncpy( cpu->name, "Unknown", sizeof(cpu->name) );
			break;
		}
		return( -1 );
	}

	// Get the right CPU model name.
	for( i=0; i<count; i++ )
	{
		if( cpu->signature.flags.family==names[i].family )
		{
			int id = cpu->signature.flags.model;
			// Copy the cpu name.
			strncpy( cpu->name,
				names[i].model_names[ id ],
				sizeof(cpu->name) );
		}
	}

	return( 0 );
}

//! Get the CPU informations and fill the cpuinfo_t * structure.
int cpu_get_info( struct cpuinfo_t *cpu )
{
	unsigned int dummy;

	// Reset the structure.
	memset( cpu, 0, sizeof(struct cpuinfo_t) );

	// Check if 'cpuid' instruction is available.
	if( !cpuid_is_available() )
	{
		if( !cpu_is_486() )
		{
			// 80386 detected.
			cpu->signature.flags.family = 3;
		}
		else
		{
			// 80486 detected.
			cpu->signature.flags.family = 4;
		}
	}
	else
	{
		// Execute the 'cpuid' instruction command #0.
		cpuid( GET_CPU_VENDOR,
			&dummy, &cpu->vendor.num[0],
			&cpu->vendor.num[2], &cpu->vendor.num[1] );

		// Execute the 'cpuid' instruction command #1.
		cpuid( GET_CPU_INFO,
			&cpu->signature.num, &cpu->brand.num,
			&cpu->feature.num[0], &cpu->feature.num[1] );

		// Evaluate the frequency if it is possible.
		cpu->frequency = speed();
	}

	// Get the model name.
	cpu_get_modelname( cpu );

	// Well done.
	return( 0 );
}

//! Dump CPU internal cache informations.
void cpu_dump_cache_info()
{
	unsigned int cache_info[4];
	char *desc;
	unsigned int i, j, n;
	unsigned int dummy;

	// Get the number of times to iterate (lower 8-bits).
	cpuid( GET_CPU_CACHE, &n, &dummy, &dummy, &dummy );
	n &= 0xff;
	for( i=0; i<n; i++ )
	{
		cpuid( GET_CPU_CACHE,
			&cache_info[0], &cache_info[1],
			&cache_info[2], &cache_info[3] );
	}

	// If bit 31 is set this is an unknown format.
	for( i=0; i<3; i++ )
		if ( (cache_info[i] & 0x80000000) )
			cache_info[i] = 0;

	// Look up in the cache strings table.
	desc = (char *)cache_info;
	// Byte 0 is the level count, not a descriptor.
	for( i=1; i<4*sizeof(unsigned int); i++ )
	{
		if ( desc[i] )
		{
			for( j = 0; j < _countof(cache_strings); j++ )
			{
				if ( desc[i]==cache_strings[j].index )
				{
					printf( "%s\n", cache_strings[j].string );
					break;
				}
			}
		}
	}
}

//Write AMD K6 MSR
void AMD_K6_Write_msr(unsigned long msr, unsigned long v1, unsigned long v2, struct regs_t *regs) {
    asm __volatile__ (
    	"pushfl\n"
        "cli\n"
        "wbinvd\n"
	"wrmsr\n"
	"popfl\n"
	: "=a" (regs->eax), "=b" (regs->ebx), "=c" (regs->ecx),
	"=d" (regs->edx)
  	: "a" (v1), "d" (v2), "c" (msr) );
}

//Read AMD K6 MSR
void AMD_K6_Read_msr(unsigned long msr, struct regs_t *regs) {
    asm __volatile__ (
	"pushfl\n"
        "cli\n"
        "wbinvd\n"
        "xorl %%eax, %%eax\n"
        "xorl %%edx, %%edx\n"
	"rdmsr\n"
	"popfl\n"
	: "=a" (regs->eax), "=b" (regs->ebx), "=c" (regs->ecx),
	"=d" (regs->edx) : "c" (msr) );
}

void AMD_K6_Writeback() {
	int mem;
	int c;
	struct regs_t regs;

	mem = ( 0xffffffff / 1024 ) / 4;
	if( !strncmp( cpu.vendor.string, "AuthenticAMD", strlen(cpu.vendor.string) ) )
	{
		if ( cpu.signature.flags.family != 5 )
			return;

    		c = cpu.signature.flags.model;

    		//model 8 stepping 0-7 use old style, 8-F use new style
		if( cpu.signature.flags.model == 8 ) {
			if( cpu.signature.flags.stepping < 8 )
				c = 7;
			else
				c = 9;

			switch (c) {
				//old style Write back
				case 6:
				case 7:
					AMD_K6_Read_msr( 0xC0000082, &regs );

					if ( ( ( regs.eax >> 1 ) & 0x7F ) == 0 ) {
						printf( "AMD K6: WriteBack currently disabled\n" );
						printf( "AMD K6: Enabling WriteBack to %dMB\n", mem * 4 );
						AMD_K6_Write_msr( 0xC0000082, ( ( mem << 1 ) & 0x7F ), 0, &regs );
					}
					else
						printf( "AMD K6: WriteBack currently enabled (%ldMB)\n", ( ( regs.eax >> 1 ) & 0x7F ) * 4 );
				break;

				//new style Write back
				case 9:
					AMD_K6_Read_msr( 0xC0000082, &regs );
					if ( ( ( regs.eax >> 22 ) & 0x3FF ) == 0 )
					{
						printf( "AMD K6 : WriteBack Disabled\n" );
					}
					else
						printf( "AMD K6 : WriteBack Enabled (%ldMB)\n", ( ( regs.eax >> 22 ) & 0x3FF ) * 4 );
			 		printf( "AMD K6 : Enabled WriteBack (%dMB)\n", mem * 4 );
					AMD_K6_Write_msr( 0xC0000082, ( ( mem << 22 ) & 0x3FF ), 0, &regs );
				break;

				default: //dont set it on Unknowns + k5's
					break;
			}
		}
	}
}

extern void enable_sse();
extern unsigned char is_sse_enabled = 0;

// Initialize the main CPU.
void init_cpu(struct cpuinfo_t *CPU)
{
		cpu_get_info(CPU);

		printf_clr(0x0d,"\nCPU: Detected '%s' %u.%u MHz processor.\n\n"
		 "CPU: %s (%d %d %d)\n",
		CPU->vendor.string,
		CPU->frequency/1000, CPU->frequency%1000,
		CPU->name,
		CPU->signature.flags.family,
		CPU->signature.flags.model,
		CPU->signature.flags.stepping );

		printf_clr(0x0d,"CPU features support:\n\n");
		printf("{  ");

		if (CPU->feature.flags.edx.pse )    printf(" PSE,");
	    if (CPU->feature.flags.edx.pae)     printf(" PAE,");
	    if (CPU->feature.flags.edx.apic)    printf(" APIC,");
	    if (CPU->feature.flags.edx.mtrr)    printf(" MTRR,");
		if (CPU->feature.flags.edx.fxsr) {
			    printf(" OXFSXR");
				unsigned int __cr4;
				__asm__ __volatile__ (
					"movl %%cr4, %0\n"
					"or $512, %0\n"
					"movl %0, %%cr4" : "=r"(__cr4));
		}
		else printf("\b");

        printf("  }\n\n");
		//printf("OXFSXR support!\n");

		// Set bit 9 of CR4 (the OXFSXR). Setting this bit assumes that the
		// operating system provides facility for saving and restoring the
		// SSE state, using the "fxsave" and "fxrstor" instructions.
		// This is also needed to use SSE or SSE2 processor extensions.



		printf_clr(0x0d,"CPU Complex Instructions support:\n\n");
		printf("{  ");

	    if (CPU->feature.flags.edx.tsc)      printf(" TSC,");
	    if (CPU->feature.flags.edx.msr)      printf(" MSR,");
	    if (CPU->feature.flags.edx.sse){     printf(" SSE,"); enable_sse(); is_sse_enabled = 1;}
	    if (CPU->feature.flags.edx.sse2)     printf(" SSE2,");
	    if (CPU->feature.flags.ecx.sse3)     printf(" SSE3,");
		if (CPU->feature.flags.ecx.ssse3)    printf(" SSSE3,");
	    if (CPU->feature.flags.ecx.sse41)	printf(" SSE4.1,");
	    if (CPU->feature.flags.ecx.sse42)	printf(" SSE4.2,");
	    if (CPU->feature.flags.ecx.avx)      printf(" AVX,");
	    if (CPU->feature.flags.ecx.f16c)     printf(" F16C,");
	    if (CPU->feature.flags.ecx.rdrand)   printf(" RDRAND"); else printf("\b");
	    printf("  }\n\n");

	cpu_dump_cache_info();

	//AMD_K6_Writeback();
}
