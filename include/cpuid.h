#ifndef __CPUID_H
#define __CPUID_H

/* Vendor-strings. */
/* TODO: You should use the <cpuid.h> header that comes with GCC instead. */

enum cpuid_requests {
	CPUID_GETVENDORSTRING,
	CPUID_GETFEATURES,
	CPUID_GETTLB,
	CPUID_GETSERIAL,

	CPUID_INTELEXTENDED=0x80000000,
	CPUID_INTELFEATURES,
	CPUID_INTELBRANDSTRING,
	CPUID_INTELBRANDSTRINGMORE,
	CPUID_INTELBRANDSTRINGEND,
};

typede struct cpuinfo{
	char vendor[sizeof(unsigned int)*3]; /* EBX,ECX,EDX regs*/
	//TODO:
	/*	write up all you need to know about cpu
		write up helper functions that takes a pointer to struct of cpuinfo_t and instantiate it with all information and
		return a negative value to indicate an error occurred
	*/
} cpuinfo_t;


 /** issues a single request to CPUID. Fits 'intel features', for instance
 *  note that even if only "eax" and "edx" are of interest, other registers
 *  will be modified by the operation, so we need to tell the compiler about it.
 */
static inline void cpuid(int code, uint32_t *a, uint32_t *d);

/* issues a complete request, storing general registers output as a string */
static inline int cpuid_string(int code, uint32_t where[4]);

#endif /* __CPUID_H */
