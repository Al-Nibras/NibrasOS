//  NibrasOS
//  cpuid.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-21T19:52:52+03:00.
//	Last modification time on 2017-07-29T23:54:29+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

static inline void cpuid(int code, unsigned int *a, unsigned int *d) {
	asm volatile("cpuid":"=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}

static inline int cpuid_string(int code, unsigned int where[4]) {
	asm volatile("cpuid":"=a"(*where),"=b"(*(where+1)),
							 "=c"(*(where+2)),"=d"(*(where+3)):"a"(code));
	return (int)where[0];
}
