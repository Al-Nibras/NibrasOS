#ifndef __KHEAP_H
#define __KHEAP_H

#include <paging.h>

#define KMALLOC_STATE_FREE 0x00DEAD00
#define KMALLOC_STATE_USED 0x00FEED00
#define KHEAP_END 0xF0000000

struct kmalloc_chunk {
	int state;
	unsigned int length;
	struct kmalloc_chunk *next;
	struct kmalloc_chunk *prev;
};

#define KUNIT sizeof(struct kmalloc_chunk)



void * kmalloc(unsigned int length);
void* kcalloc(size_t size, char clear);
void  kfree(void *ptr);

void kheap_init(void *start_address, unsigned int length);
void kmalloc_debug();

#endif
