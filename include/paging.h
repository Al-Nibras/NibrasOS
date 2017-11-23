#ifndef __PAGING_H
#define __PAGING_H

extern unsigned int placement_address;
extern unsigned int sys_stack; //defined in kmain.c
#include <memory.h>
#include <isrs.h>

typedef struct pte{
	unsigned ps    		: 1;
	unsigned rw         : 1;
	unsigned us      	: 1;
	unsigned rsvd1     	: 2;
	unsigned a   		: 1;
	unsigned d   		: 1;
	unsigned rsvd2      : 2;
	unsigned avail      : 3;
	unsigned frame      : 20;
} pte_t;

typedef struct page_table{
	pte_t pages[1024];
} page_table_t;

typedef struct pde{
	unsigned p    		: 1;
	unsigned rw         : 1;
	unsigned us      	: 1;
	unsigned pwt     	: 1;
	unsigned pcd   		: 1;
	unsigned a   		: 1;
	unsigned d     		: 1;
	unsigned ps    		: 1;
	unsigned g    		: 1;
	unsigned avail		: 3;
	unsigned frame      : 20;
} pde_t;

typedef struct page_directory{
	pde_t tables[1024];
	unsigned int tables_phys_addrs[1024];
	unsigned int phys_addr;
} page_directory_t;

extern page_directory_t* kernel_page_directory; //defined in paging.c
extern page_directory_t *current_page_directory;

void vm_allocate_region(page_directory_t* dir,char iden_map, unsigned int start,unsigned int end, char rw,char us);
void vm_free_region(page_directory_t* dir, unsigned int start,unsigned int end);
int vm_alloc_page(pte_t *page);
void vm_free_page(page_directory_t* dir,unsigned int virtual_address);
void vm_init();
void page_fault_handler(struct regs *r);
void vm_init_table(page_table_t* table,unsigned int frame, unsigned int virt,char ps,char rw,char us);
void vm_page_dir_add_table(page_directory_t *pde,page_table_t* table, unsigned int virt,char ps,char rw,char us);
unsigned int get_blocks_counter();
page_directory_t* vm_get_current_page_directory();
extern void flush_TLB(unsigned int address); //defined in start.asm
void enable_paging();
void disable_paging();
page_directory_t* vm_create_page_directory();
page_directory_t* vm_clone_page_directory(page_directory_t* pd);
void vm_map_physical_address (page_directory_t* dir, unsigned int virt, unsigned int phys, char ps,char rw,char us);
pte_t* vm_map_page_in_dir(page_directory_t* dir,unsigned int virtual_address, unsigned int physical_address, char ps,char rw,char us);
page_table_t *vm_clone_page_table(page_table_t *src, unsigned int *phys_addr);
void vm_pde_set_frame(pde_t* pde, unsigned int frame);
#endif /* __PAGING_H */
