//  NibrasOS
//  paging.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-07-03T15:36:45+03:00.
//	Last modification time on 2017-07-29T23:52:50+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <paging.h>

/* NOTE: physical_address = PFN * page_size + offset */

#define PAGE_DIRECTORY_INDEX(x) 	 (((unsigned int)x >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) 		 (((unsigned int)x >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*(unsigned int*)x & ~0xfff)
#define VM_SET_BIT(PTE_or_PDE,bit) 	 (*(unsigned int*)PTE_or_PDE |= (1<<bit))
#define VM_CLEAR_BIT(PTE_or_PDE,bit) (*(unsigned int*)PTE_or_PDE &= ~(1<<bit))

page_directory_t *current_page_directory = 0;
page_directory_t *kernel_page_directory = 0;

static pte_t *vm_pt_get_pte_index(page_table_t *pt,unsigned int virtual_address){
	if(!pt) return 0;
	return &pt->pages[PAGE_TABLE_INDEX(virtual_address)];
}
static page_table_t *vm_pd_get_pt_index(page_directory_t *pd,unsigned int virtual_address){
	if(!pd) return 0;
	return &pd->tables[PAGE_DIRECTORY_INDEX(virtual_address)];
}
void vm_pde_set_frame(pde_t* pde, unsigned int frame) {
	*(unsigned int*)pde = ( (*(unsigned int*)pde) & ~0xFFFFF000) | frame;
 }
static void vm_pte_set_frame(pte_t* pte, unsigned int frame) {
	*(unsigned int*)pte = ( (*(unsigned int*)pte) & ~0xFFFFF000) | frame;
}
int vm_alloc_page(pte_t *page){
	if(!page) return -1;
	void *block = mem_alloc_page(0);
	if(!block) return -1;

	VM_SET_BIT(page, 0);
	//page->frame = (unsigned int)block/0x1000; //devide phys_addr by 4096 to get PFN
	vm_pte_set_frame(page, block);
	return 0;
}

void vm_allocate_region(page_directory_t* dir,char iden_map, unsigned int start,unsigned int end, char rw,char us){
	uint32_t start_va = start & 0xfffff000;
	uint32_t end_va = end & 0xfffff000;
	while(start_va <= end_va) {
        if(iden_map)
            //vm_map_page_in_dir(dir, start_va, start_va / PAGE_SIZE, 1,rw,us);
			vm_map_page_in_dir(dir, start_va, 0, 1,rw,us);
        else
			vm_map_page_in_dir(dir, start_va, 0, 1,rw,us);
        start_va = start_va + PAGE_SIZE;
    }
}
void vm_free_region(page_directory_t* dir, unsigned int start,unsigned int end){
	uint32_t start_va = start & 0xfffff000;
    uint32_t end_va = end & 0xfffff000;
    while(start_va <= start_va) {
        vm_free_page(dir, start_va);
        start_va = start_va + PAGE_SIZE;
    }
}
void vm_free_page(page_directory_t* dir,unsigned int virtual_address){
	if(!dir) return;
    uint32_t page_dir_idx = PAGE_DIRECTORY_INDEX(virtual_address), page_tbl_idx = PAGE_TABLE_INDEX(virtual_address);
    if(!&dir->tables[page_dir_idx])
        return;
    page_table_t * table = &dir->tables[page_dir_idx];
    if(!table->pages[page_tbl_idx].ps)
        return;
	dir->tables_phys_addrs[page_dir_idx] = 0;

    mem_free_page(table->pages[page_tbl_idx].frame*PAGE_SIZE);
    table->pages[page_tbl_idx].ps = 0;
    table->pages[page_tbl_idx].frame = 0;
}
// void vm_free_page(pte_t *page){
// 	if(!page || !page->ps) return;
// 	mem_free_page(page->frame*0x1000); //multiply PFN by 4096 to get phys_addr
// 	VM_CLEAR_BIT(page, 0);
// }
void vm_switch_to_page_directory(const page_directory_t *new_pde){
	if(!new_pde) return;
	disable_paging();
	current_page_directory = new_pde;
	__asm__ __volatile__("mov %0, %%cr3" :: "r"(new_pde));
	enable_paging();
}

void vm_map_page(void* virtual_address, void* physical_address){
   disable_paging();
   pde_t* e = &kernel_page_directory->tables[PAGE_DIRECTORY_INDEX((unsigned int)virtual_address)];
   if (!e->p) {
      //! page table not present, allocate it
      page_table_t* table = (page_table_t*) mem_alloc_page(0);
      if (!table)
         return;
      //! clear page table
      memset(table, 0, sizeof(page_table_t));
	  kernel_page_directory->tables_phys_addrs[PAGE_DIRECTORY_INDEX((unsigned int)virtual_address)] = table;
      //! create a new entry
      pde_t* entry = &kernel_page_directory->tables[PAGE_DIRECTORY_INDEX ((unsigned int)virtual_address)];

      //! map in the table (Can also just do *entry |= 3) to enable these bits
      VM_SET_BIT(entry, 0);
	  VM_SET_BIT(entry, 1);
      vm_pde_set_frame(entry, (unsigned int)table);
   }
   // get table
   page_table_t* table = (page_table_t*)PAGE_GET_PHYSICAL_ADDRESS(e);
   // get page
   pte_t* page = &table->pages[PAGE_TABLE_INDEX((unsigned int) virtual_address)];
   // map it
   if(!physical_address)
   		physical_address = mem_alloc_page(0);
   if(!physical_address)
		return;
   vm_pte_set_frame(page, (unsigned int)physical_address);
   VM_SET_BIT(page, 0);
   VM_SET_BIT(page, 1);
   vm_switch_to_page_directory(kernel_page_directory); //just to reload cr3
}

extern void kheap_init(void *start_address, int length);
void vm_init() {
	printf_clr(0x0e,"\nSetting up Virtual Memory...\n");
	page_directory_t* dir = (page_directory_t*) mem_alloc_pages(3);
	if (!dir){
		printf_clr(0x0e,"Virtual Memory setup failed\n");
		return;
	}
	memset(dir, 0, sizeof (page_directory_t));
	dir->phys_addr = dir;
	kernel_page_directory = dir;

	page_table_t* table = (page_table_t*) mem_alloc_page(0);
	if (!table){
		printf_clr(0x0e,"Virtual Memory setup failed\n");
		return;
	}
	page_table_t* table2 = (page_table_t*) mem_alloc_page(0);
	if (!table2){
		printf_clr(0x0e,"Virtual Memory setup failed\n");
		return;
	}

	//Setting up Higher-Half Kernel
	vm_init_table(table, 0x00, 0x00, 1, 1, 0);
	vm_init_table(table2,  0x100000, 0xc0000000, 1, 1, 0);
	vm_page_dir_add_table(dir,table,0x00, 1,1,0);
	vm_page_dir_add_table(dir,table2,0xc0000000,1,1,0);

	printf_clr(0x0e,"Configuring Kernel Memory Layout...\n");


	printf_clr(0x0e,"Mapping Kernel's Heap...\n");
	unsigned int frame = 0x500000;
	unsigned int virt = 0;
	//252: it's supposed to be 256 tables = 1GB, but we already mapped 4MB
	for(int i = 0; i<10; ++i,frame+=0x400000,virt+=0x400000){
		page_table_t *heap_table = mem_alloc_page(0);
		if(!heap_table) break;
		vm_init_heap_table(heap_table,0xc0400000+virt, 1, 1, 0);
		vm_page_dir_add_table(dir,heap_table,0xc0400000+virt, 1,1,0);
	}

	kheap_init(0xc0400000,virt-0x1000);
	printf("Kernel's Heap initial end address: 0x%x\n",0xc0400000+virt-0x1000);

	vm_switch_to_page_directory(dir);
	enable_paging();
	//installing fault_handler
	isr_install_handler(14,page_fault_handler);
}
void vm_init_table(page_table_t* table,unsigned int frame, unsigned int virt,char ps,char rw,char us){
	if(!table) return;
	memset(table,0,sizeof(page_table_t));

	for (unsigned int i=0; i<1024; i++, frame+=4096, virt+=4096) {
		pte_t page; *(unsigned int*)&page = 0;
		if(ps) VM_SET_BIT(&page, 0); //present
		if(rw) VM_SET_BIT(&page, 1); //rear/write
		if(us) VM_SET_BIT(&page, 2); //user-mode

		vm_pde_set_frame(&page, frame);
		table->pages[PAGE_TABLE_INDEX(virt)] = page;
	}
}
void vm_init_heap_table(page_table_t* table, unsigned int virt,char ps,char rw,char us){
	if(!table) return;
	memset(table,0,sizeof(page_table_t));

	for (unsigned int i=0; i<1024; i++, virt+=4096) {
		pte_t page; *(unsigned int*)&page = 0;
		if(ps) VM_SET_BIT(&page, 0); //present
		if(rw) VM_SET_BIT(&page, 1); //rear/write
		if(us) VM_SET_BIT(&page, 2); //user-mode
		void* frame = mem_alloc_page(0);
		if(!frame) return;
		vm_pde_set_frame(&page, frame);
		table->pages[PAGE_TABLE_INDEX(virt)] = page;
	}
}

void vm_page_dir_add_table(page_directory_t *pde,page_table_t* table, unsigned int virt,char ps,char rw,char us){
	if(!pde || !table) return;

	kernel_page_directory->tables_phys_addrs[PAGE_DIRECTORY_INDEX(virt)] = table;

	pde_t* entry = &pde->tables[PAGE_DIRECTORY_INDEX(virt)];
	if(ps) VM_SET_BIT (entry, 0); 				  //present
	if(rw) VM_SET_BIT (entry, 1); 			      //rear/write
	if(us) VM_SET_BIT (entry, 2); 				  //user-mode
	vm_pde_set_frame(entry, (unsigned int)table); //must be table's physical_address
}

int vm_create_page_table(page_directory_t* dir, unsigned int virt, char ps,char rw,char us) {
	if(!dir) return -1;
	pde_t *entry = &dir->tables[PAGE_DIRECTORY_INDEX(virt)];
	if ( !(unsigned int)entry || !entry->p ) //does not exist
	{
		void* block = mem_alloc_page(0);
		if (!block) return -1;
		dir->tables_phys_addrs[PAGE_DIRECTORY_INDEX((unsigned int)virt)] = (unsigned int)block;
		if(ps) VM_SET_BIT(entry, 0);
	 	if(rw) VM_SET_BIT(entry, 1);
	 	if(us) VM_SET_BIT(entry, 2);
		vm_pde_set_frame(entry, (unsigned int)block);
		//TODO: map physical_to_virtual for this `dir`
		vm_map_page_in_dir(dir,(unsigned int)block,(unsigned int)block,ps,rw,us);
    }
	return 0; // success
}
pte_t* vm_map_page_in_dir(page_directory_t* dir,unsigned int virtual_address, unsigned int physical_address, char ps,char rw,char us){
	if(!dir) return 0;
	disable_paging();
	pde_t* e = &dir->tables[PAGE_DIRECTORY_INDEX((unsigned int)virtual_address)];
	if (!e->p) {
	   page_table_t* table = (page_table_t*) mem_alloc_page(0);
	   if (!table)
		  return 0;
	   memset(table, 0, sizeof(page_table_t));
	   dir->tables_phys_addrs[PAGE_DIRECTORY_INDEX((unsigned int)virtual_address)] = table;
	   pde_t* entry = &dir->tables[PAGE_DIRECTORY_INDEX ((unsigned int)virtual_address)];

	   if(ps) VM_SET_BIT(entry, 0);
	   if(rw) VM_SET_BIT(entry, 1);
	   if(us) VM_SET_BIT(entry, 2);
	   vm_pde_set_frame(entry, (unsigned int)table);
	}
	page_table_t* table = (page_table_t*)PAGE_GET_PHYSICAL_ADDRESS(e);
	pte_t* page = &table->pages[PAGE_TABLE_INDEX((unsigned int) virtual_address)];

	if(ps) VM_SET_BIT(page, 0);
	if(rw) VM_SET_BIT(page, 1);
	if(us) VM_SET_BIT(page, 2);

	if(!physical_address)
		  physical_address = mem_alloc_page(0);
	if(!physical_address)
	  return 0;
	vm_pde_set_frame(page, (unsigned int)physical_address);
	enable_paging();
	return page;
}

void* vm_get_physical_address(page_directory_t* dir, unsigned int virtual_address) {
	pde_t *entry = &dir->tables[PAGE_DIRECTORY_INDEX(virtual_address)];
	if ( !*(unsigned int*)entry || !entry->p) //does not exist
		return 0;
	page_table_t* table = (page_table_t*)PAGE_GET_PHYSICAL_ADDRESS(entry);
	pte_t* page = &table->pages[PAGE_TABLE_INDEX(virtual_address)];

    return (void*) PAGE_GET_PHYSICAL_ADDRESS(page);
}

extern copy_page_physical(void* src_page, void* dest_page);
page_table_t *vm_clone_page_table(page_table_t *src, unsigned int *phys_addr)
{
	if(!src || !phys_addr) return 0;
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)mem_alloc_page(0);
	if(!table) return 0;

	vm_map_physical_address(current_page_directory, table, table, 1,1,1);

	*phys_addr = table;
    memset(table, 0, sizeof(page_table_t));

    // For every entry in the table...
    for (int i = 0; i < 1024; i++)
    {
        // If the source entry has a frame associated with it...
        if (src->pages[i].frame)
        {
			void *page = mem_alloc_page(0);
			if(!page) break;
			//memcpy(&table->pages[i],&src->pages[i],sizeof(pte_t));


            // Physically copy the data across. This function is in process.s.
            copy_page_physical(src->pages[i].frame*PAGE_SIZE, table->pages[i].frame*PAGE_SIZE);
            VM_SET_BIT(&table->pages[i],0);
            if(src->pages[i].rw) VM_SET_BIT(&table->pages[i], 1);
            if(src->pages[i].us) VM_SET_BIT(&table->pages[i], 2);
            vm_pte_set_frame(&table->pages[i], page);

            // vm_map_physical_address(current_page_directory, table->pages[i].frame*0x1000, table->pages[i].frame*0x1000, 1,1,1);
            // vm_map_physical_address(current_page_directory, src->pages[i].frame*0x1000, src->pages[i].frame*0x1000, 1,1,1);

            // unsigned int* src_page = src->pages[i].frame*0x1000;
            // unsigned int* dest_page = table->pages[i].frame*0x1000;
            // for(int p = 0; p<1024; ++p)
            //     dest_page[p] = src_page[p];
            //
            // vm_unmap_physical_address(current_page_directory,src_page);
            // vm_unmap_physical_address(current_page_directory,dest_page);
        }
    }
    return table;
}
page_directory_t* vm_clone_page_directory(page_directory_t* src){
    page_directory_t* dir = (page_directory_t*) mem_alloc_pages(3);
	if (!dir)
        return 0;
	vm_map_physical_address(kernel_page_directory,dir,dir,1,1,0);
	// vm_map_page(dir, dir);
	vm_map_physical_address(kernel_page_directory,(unsigned int)dir+0x1000,(unsigned int)dir+0x1000,1,1,0);
	// vm_map_page((unsigned int)dir+0x1000, (unsigned int)dir+0x1000);
	vm_map_physical_address(kernel_page_directory,(unsigned int)dir+0x2000,(unsigned int)dir+0x2000,1,1,0);
	// vm_map_page((unsigned int)dir+0x2000, (unsigned int)dir+0x2000);


    //parent's pd
    vm_map_physical_address(current_page_directory,dir,dir,1,1,0);
    // vm_map_page(dir, dir);
    vm_map_physical_address(current_page_directory,(unsigned int)dir+0x1000,(unsigned int)dir+0x1000,1,1,0);
    // vm_map_page((unsigned int)dir+0x1000, (unsigned int)dir+0x1000);
    vm_map_physical_address(current_page_directory,(unsigned int)dir+0x2000,(unsigned int)dir+0x2000,1,1,0);

    memset(dir, 0, sizeof(page_directory_t));

	unsigned int offset = (unsigned int)&dir->tables_phys_addrs[0] - (unsigned int)dir;
	dir->phys_addr = (unsigned int)dir;


	for(int i = 0; i<1024; ++i){
		if (!src->tables[i].p)
           continue;
		if(*(unsigned int*)&kernel_page_directory->tables[i] == *(unsigned int*)&src->tables[i]){
			//Kernel Virtual Address Space
			// It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tables_phys_addrs[i] = src->tables_phys_addrs[i];
 		}
		else {
			//A process Virtual Address Space
            vm_map_physical_address(current_page_directory, src->tables_phys_addrs[i], src->tables_phys_addrs[i], 1,1,1);
			unsigned int phys;
			page_table_t* table = vm_clone_page_table(src->tables_phys_addrs[i], &phys);
			memcpy(&dir->tables[i],&src->tables[i],sizeof(pde_t));
			vm_pde_set_frame(&dir->tables[i], phys);
			dir->tables_phys_addrs[i] = phys;
		}
	 }
    return dir;
}
page_directory_t* vm_create_page_directory(){
	return vm_clone_page_directory(kernel_page_directory);
}
void vm_map_physical_address (page_directory_t* dir, unsigned int virt, unsigned int phys, char ps,char rw,char us) {
		pde_t *entry = &dir->tables[PAGE_DIRECTORY_INDEX(virt)];
        if (!entry || !entry->p)
                vm_create_page_table(dir, virt, ps,rw,us);
		page_table_t* table = dir->tables_phys_addrs[PAGE_DIRECTORY_INDEX(virt)];
		pte_t* page = &table->pages[PAGE_TABLE_INDEX((unsigned int) virt)];
		vm_pte_set_frame(page, (unsigned int)phys);
		if(ps) VM_SET_BIT(page, 0);
		if(rw) VM_SET_BIT(page, 1);
		if(us) VM_SET_BIT(page, 2);
}
void vm_unmap_physical_address(page_directory_t* dir,unsigned int virt){
	pde_t *entry = &dir->tables[PAGE_DIRECTORY_INDEX(virt)];
	if (!entry || !entry->p)
		return;
	page_table_t* table = (page_table_t*)PAGE_GET_PHYSICAL_ADDRESS(entry);
	pte_t* page = &table->pages[PAGE_TABLE_INDEX((unsigned int) virt)];
	mem_free_page(page->frame*0x1000);
	dir->tables_phys_addrs[PAGE_DIRECTORY_INDEX((unsigned int)virt)] = 0;
	memset(page,0,sizeof(pte_t)); //just in-case plah plah plah...
	vm_pte_set_frame(page, 0);
	VM_CLEAR_BIT(page, 0);
	VM_CLEAR_BIT(page, 1);
	VM_CLEAR_BIT(page, 2);
}

extern void halt(); //defined in start.asm
void page_fault_handler(struct regs *r){
	// The faulting address is stored in CR2 register.
	unsigned int faulting_address;
	__asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));

	int ps = !(r->err_code & 0x1);  //if 1: occured because page is NOT present
	int rw = r->err_code  & 0x2; 	//and so on...
	int us = r->err_code  & 0x4;
	int rsvd = r->err_code& 0x8;
	int IF = r->err_code  & 0x10;

	printf_clr(0x4,"Page fault!\n");
	printf_clr(0x7,"Faulting address: 0x%x\n",faulting_address);
	printf_clr(0x7,"%s\t", (us) ? "user-mode" 							: "kernel-mode");
	printf_clr(0x7,"%s\t", (ps) ? "present" 							: "NOT present");
	printf_clr(0x7,"%s\t", (rw) ? "readable/writable" 					: "readable");
	printf_clr(0x7,"%s",  (rsvd)? "reserved bits were written over\t" 	: "");
	printf_clr(0x7,"%s\n", (IF) ? "occured during an instruction fetch" : "");
	printf_clr(0x4,"halting...\n");

	halt();
}
page_directory_t* vm_get_current_page_directory(){
	return current_page_directory;
}

void disable_paging(){
	unsigned int cr0;
	__asm__ __volatile__("mov %%cr0, %0": "=r"(cr0));
	cr0 &= ~0x80000000; // Disable paging!
	__asm__ __volatile__("mov %0, %%cr0":: "r"(cr0));
}
void enable_paging(){
	unsigned int cr0;
	__asm__ __volatile__("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000; // Enable paging!
	__asm__ __volatile__("mov %0, %%cr0":: "r"(cr0));
}
