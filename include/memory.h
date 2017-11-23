#ifndef __MEMORY_H
#define __MEMORY_H

/* NOTE: I use the term PAGE instead of FRAME just for simpleness, PAGE=FRAME in this header */
#define BITMAP_SIZE_PER_PAGE  				     32
#define PAGE_SIZE                            0x1000
#define PAGE_ALIGN                       0xfffff000
#define PAGE_FRAME_SIZE                         0xC

struct phys_mem_avail{
	unsigned int base;
	unsigned int len;
} __attribute__((packed));
struct phys_mem_holes{
	unsigned int base;
	unsigned int len;
} __attribute__((packed));

void memory_init(unsigned int mem_sz, unsigned int mem_start_addr, struct phys_mem_avail *avail_mem);
void *mem_alloc_page(char clear_page);
void *mem_alloc_pages(unsigned int size);
void* mem_alloc_after(int count);
void mem_free_page(void * page_addr);
void mem_free_pages(void * page_addr,unsigned int size);
void* mem_alloc_at(unsigned int base, unsigned int size);
#endif /* __MEMORY_H */
