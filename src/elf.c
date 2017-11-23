/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-04-08T20:38:02+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: elf.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:51:19+03:00
 */



#include <elf.h>

extern unsigned int read_eip(); //defined int start.asm

int valid_elf(elf_header_t * elf_head) {
    if(elf_head->e_ident[EI_MAG0] != ELFMAG0)
        return 0;
    if(elf_head->e_ident[EI_MAG1] != ELFMAG1)
        return 9;
    if(elf_head->e_ident[EI_MAG2] != ELFMAG2)
        return 0;
    if(elf_head->e_ident[EI_MAG3] != ELFMAG3)
        return 0;
    if(elf_head->e_ident[EI_CLASS] != ELFCLASS32)
        return 0;
    if(elf_head->e_ident[EI_DATA] != ELFDATA2LSB)
        return 0;
    if(elf_head->e_ident[EI_VERSION] != EV_CURRENT)
        return 0;
    if(elf_head->e_machine != EM_386)
        return 0;
    if(/* elf_head->e_type != ET_REL && */ elf_head->e_type != ET_EXEC)
        return 0;
    return 1;
}
extern char* *proc_names;
extern void* *binaries;
int elf32_load_proc(proc_t* proc, const char* process_name, unsigned int *entry_point){
    str_to_cap(process_name);
    int i;
    for(i = 0; i<512;++i)
      if(!strcmp(proc_names[i],process_name))
        break;
    if(i>=512){
        printf_clr(0x04,"ERROR: exec()-> Process %s not found!\n",process_name);
         return -1;
     }

    if(!proc) return -2;
    void* proc_exec = binaries[i];
    elf_header_t * head = proc_exec;
    if(!valid_elf(head)){
        printf_clr(0x04,"ERROR: exec()-> Process %s: Invalid executable!\n",process_name);
         return -3;
     }

    elf_program_header_t * prgm_head = (void*)head + head->e_phoff;

    uint32_t seg_begin = 0, seg_end = 0;
    vm_switch_to_page_directory(proc->pd);
    for(uint32_t i = 0; i <= head->e_phnum; i++) {
        if(prgm_head->p_type == PT_LOAD) {
            seg_begin = prgm_head->p_vaddr;
            seg_end= seg_begin + prgm_head->p_memsz;

            // allocate this region
            unsigned int frame_cntr = 0;
            size_t seg_size = seg_end-seg_begin;
            do {
                void* block = mem_alloc_page(0);
                pte_t *page = vm_map_page_in_dir(proc->pd,seg_begin+frame_cntr,block,1,1,1);
                if(seg_size>PAGE_SIZE) seg_size -= PAGE_SIZE;
                else seg_size = 0;
                frame_cntr += PAGE_SIZE;
                if(!proc->pages_count){
                    proc->pages = kmalloc(sizeof(pte_t));
                    proc->pages[0] = seg_begin+frame_cntr;
                    proc->pages_count = 1;
                }
                else {
                    pte_t* *temp = kmalloc(sizeof(pte_t)*(proc->pages_count+1));
                    memset(&temp[0],0,proc->pages_count+1);
                    for(size_t s = 0; s<proc->pages_count; ++s)
                        temp[s] = proc->pages[s];
                    memset(&proc->pages[0],0,proc->pages_count);
                    kfree(proc->pages);
                    proc->pages = temp;
                    proc->pages[proc->pages_count++] = seg_begin+frame_cntr;
            }
            }while(seg_size>PAGE_SIZE);
            if(seg_size%PAGE_SIZE > 0 && seg_size%PAGE_SIZE == seg_size){
                void* block = mem_alloc_page(0);
                vm_map_page_in_dir(proc->pd,seg_begin+frame_cntr,block,1,1,1);
                if(!proc->pages_count){
                    proc->pages = kmalloc(sizeof(pte_t));
                    proc->pages[0] = seg_begin+frame_cntr;
                    proc->pages_count = 1;
                }
                else {
                    pte_t* *temp = kmalloc(sizeof(pte_t)*(proc->pages_count+1));
                    memset(&temp[0],0,proc->pages_count+1);
                    for(size_t s = 0; s<proc->pages_count; ++s)
                        temp[s] = proc->pages[s];
                    memset(&proc->pages[0],0,proc->pages_count);
                    kfree(proc->pages);
                    proc->pages = temp;
                    proc->pages[proc->pages_count++] = seg_begin+frame_cntr;
                }
            }
            // Load segment data
            vm_switch_to_page_directory(proc->pd); //refresh
            memcpy((void*)seg_begin, proc_exec + prgm_head->p_offset, prgm_head->p_filesz);
            // Fill zeros in the region [filesz, memsz]
            memset((void*)(seg_begin + prgm_head->p_filesz), 0, prgm_head->p_memsz - prgm_head->p_filesz);
            // If this is the code segment
            if(prgm_head->p_flags == PF_X + PF_R + PF_W || prgm_head->p_flags == PF_X + PF_R) {
                if(entry_point) *entry_point = head->e_entry;// + seg_begin;
                else proc->eip = head->e_entry;
            }
        }
        prgm_head++;
    }
    vm_switch_to_page_directory(kernel_page_directory);
    return 0;
}


// void terminate(){
//     /* get physical address of stack */
//     void* stackFrame = vm_get_physical_address(current_process->pd,(uint32_t)current_process->threads[0].stack);
//     vm_unmap_physical_address(current_process->pd,stackFrame);
//
//     //TODO:
//     //unmap all allocated regions for this process
//     for(int i = 0; i<current_process->pages_count; ++i)
//         vm_free_page(current_process->pd,current_process->pages[i]);
//
//     um_exit(kern_eip);
// }
