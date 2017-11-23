//  NibrasOS
//  kmain.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-19T03:06:48+03:00.
//	Last modification time on 2017-07-29T23:57:50+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//


#include <multiboot.h>
#include <system.h>

#define interrupt(n) __asm__ __volatile__ ("int $" #n : : : "cc", "memory")
static struct phys_mem_holes null_hole = {0,0};

static struct cpuinfo_t CPU;

unsigned char *memcpy(unsigned char *dest, const unsigned char *src, unsigned int count){

	if(!dest || !src || !count ) return 0;
	// while (--count != 0)
	// 	dest[count] = src[count];
	//
	// dest[count] = src[count]; //count now is 0, copy first element
	//fast_memcpy(dest,src,count);
	// if(CPU.feature.flags.edx.sse && count >= 256) return fast_memcpy(dest,src,count);
	// else {
		// count /= 4;
		// asm volatile ("cld; rep movsl" : "+c" (count), "+S" (src), "+D" (dest) :: "memory"); //faster implementation
		// return dest;
	// }

	int remainder = count % sizeof(unsigned int);
	while(remainder>0){
		*dest++ = *src++;
		count--;
		--remainder;
	}
	count /= sizeof(unsigned int);
	asm volatile ("cld; rep movsl" : "+c" (count), "+S" (src), "+D" (dest) :: "memory"); //faster implementation

	return dest;

}
unsigned char *memset(unsigned char *dest, unsigned char val, unsigned int count){
	// while (count-- != 0)
	// 	*dest++ = val;
	int remainder = count % sizeof(unsigned int);
	while(remainder>0){
		*dest++ = val;
		count--;
		--remainder;
	}
	count /= sizeof(unsigned int);
	asm volatile ("cld; rep stosl" : "+c" (count), "+a" (val) , "+D" (dest) :: "memory"); //faster implementation
	return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, unsigned int count){
	while (count-- != 0)
		*dest++ = val;
	return dest;
}
void *memmove(void * restrict dest, const void * restrict src, size_t count) {
	size_t i;
	unsigned char *a = dest;
	const unsigned char *b = src;
	if (src < dest) {
		for ( i = count; i > 0; --i) {
			a[i-1] = b[i-1];
		}
	} else {
		for ( i = 0; i < count; ++i) {
			a[i] = b[i];
		}
	}
	return dest;
}
uint32_t __attribute__ ((pure)) krand() {
	static uint32_t x = 123456789;
	static uint32_t y = 362436069;
	static uint32_t z = 521288629;
	static uint32_t w = 88675123;

	uint32_t t;

	t = x ^ (x << 11);
	x = y; y = z; z = w;
	return w = w ^ (w >> 19) ^ t ^ (t >> 8);
}

extern unsigned test_cpu_mode();//defined in start.asm
extern unsigned int end;//defined in link.ld
extern unsigned int code;//defined in link.ld

#define MMAP_NEXT(m) (memory_map_t*)((unsigned int)m + m->size + sizeof(unsigned int))

void read_mmap(multiboot_info_t* mbt, struct phys_mem_holes* holes, struct phys_mem_avail* avails){
    memory_map_t* mmap = (memory_map_t*) mbt->mmap_addr;
		if(!mmap) return;
		printf_clr(LIGHT_RED,"   base\t\t\t  length\t\t   size\t\t\ttype\n");
    while((unsigned int)mmap < mbt->mmap_addr + mbt->mmap_length) {
        // process the current memory map entry
        mmap = MMAP_NEXT(mmap);

				delay(1);
				unsigned long long address = mmap->base_addr_high;
				address <<= sizeof(unsigned int);
				address |= mmap->base_addr_low;

				unsigned long long length = mmap->length_high;
				length <<= sizeof(unsigned int);
				length |= mmap->length_low;

				puthex(address);puts("\t\t");
				puthex(length);puts("\t\t");
				puthex(mmap->size);puts("\t      ");
				char* type;
				switch (mmap->type) {
					case MULTIBOOT_MMAP_AVAIL:
						type = "usable";
						struct phys_mem_avail temp_hole = {address,length};
						*avails++ = temp_hole;
						break;
					case MULTIBOOT_MMAP_RSVD:
						type = "reserved";
						struct phys_mem_holes temp_avail = {address,length};
						*holes++ = temp_avail;
						break;
					case MULTIBOOT_MMAP_ACPI_REC: type = "ACPI_REC";  break;
					case MULTIBOOT_MMAP_ACPI_NVS: type = "ACPI_NVS";  break;
					default:				type = "undefined";		  break;
				}
				println(type);
    }
		*holes = null_hole;
		println(0);
}
unsigned int get_total_size(struct phys_mem_avail* avails){
	/* search multiboot structure for largest value */
	unsigned int size = 0;
	if(!avails) return;

	while (avails->base != null_hole.base && avails->len != null_hole.len) {
		if(avails->base>= 0x100000 && avails->base<avails->len)
			size += (avails->len - avails->base);
		else if(avails->len<=0x100000)
			size += (avails->len);
		++avails;
	}
	return size;
}

void print_holes(struct phys_mem_holes* holes){
		if(!holes) return;
		else if(holes->base == null_hole.base && holes->len == null_hole.len) printf("No Physical Memory Holes found!\n");

		while (holes->base != null_hole.base && holes->len != null_hole.len) {
			printf_clr(DARK_GREY,"{ base: 0x%x, len: 0x%x }\n",holes->base,holes->len);
			++holes;
		}
		println(0);
}

struct phys_mem_holes holes[100];
struct phys_mem_avail avails[100];
extern unsigned int end; //defined in link.ld
unsigned int sys_stack = 0;
void kernel_main(unsigned int magic_no, multiboot_info_t *mboot){
	unsigned int esp = 0;
	__asm__ __volatile__("mov %%esp, %0": "=r"(esp)); //we should set the stack of kernel before it gets expanded
	sys_stack = esp;

	gdt_setup();
	idt_setup();
	isr_setup();
	irq_setup();
	init_timer(100); /* 1193180 / 1 = 1193180 Hz, default System Clock interrupt value */
	init_video(2);


	//install_tss(5,0x10,sys_stack);


	read_mmap(mboot,holes,avails);
	unsigned int mem_sz = get_total_size(avails);

	memory_init(mem_sz, &end, avails);
	vm_init();

	vram_up = kmalloc(0x1000*~0xfffff000); vram_up_start = vram_up; vram_down = kmalloc(4096*4096); vram_down_start = vram_down;
	keyboard_setup();

	printf_clr(LIGHT_CYAN,"Configuring NibrasOS ....\n");

	if( !test_cpu_mode() )
		printf_clr(RED,"Real Mode\n\n");
	else
		printf_clr(RED,"Protected Mode\n\n");

	printf("Total Memory: 0x%x Bytes ",mem_sz);

	printf("Travseraling Physical Memory Holes...\n");
	print_holes(holes);


	printf_clr(LIGHT_GREEN,"\n\nHAL is fully checked and initialized, NibrasOS is ready now :)\n\n");

    ide_init(~0xFFFFFE0F);
	printf("\n");

	vfs_init();
	fat32_init(0x1F0,0x00);
	fat32_read_partition_info(1);
	fat32_list_root_directory(1);
	syscall_setup();

	init_cpu(&CPU);
	syscall_setup();
	tasking_init();

	__asm__ __volatile__ ("\t\t\njmp halt");
}
