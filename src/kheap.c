//  NibrasOS
//  kheap.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-07-12T00:41:10+03:00.
//	Last modification time on 2017-07-29T23:59:26+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <kheap.h>

static struct kmalloc_chunk *head = 0;
static struct kmalloc_chunk *tail = 0;
static unsigned int max_length = 0;
static unsigned int allocated_size = 0;
unsigned int blocks_counter = 0;

int kmallocs = 0;
static void kmerge(struct kmalloc_chunk *c)
{
	if(!c) return;
	if(c->state != KMALLOC_STATE_FREE) return;
	if(c->next && c->next->state==KMALLOC_STATE_FREE) {
		c->length += c->next->length;
		if(c->next->next)
			c->next->next->prev = c;
		c->next = c->next->next;
	}
	// kmerge(c->next);
	// kmerge(c->prev);
}

static int kexpand(unsigned int size){
		if(!size) return -1;
		struct kmalloc_chunk* expansion_address = (struct kmalloc_chunk*)((char*)head+max_length);
		if((void*)expansion_address>=KHEAP_END || (void*)expansion_address+0x400000>KHEAP_END) return -1;

		unsigned int blocks_count = (size/0x400000)+1;
		size_t temp_size = size;
        while (temp_size%0x400000 > 0 && temp_size%0x400000 != temp_size) {
			blocks_count++;
            temp_size-=0x400000;
        }

		disable_paging();
		for(unsigned int i = 0, frame = 0; i<blocks_count; ++i,frame+=0x400000,max_length+=0x400000){
			page_table_t *heap_table = mem_alloc_page(0);
			if(!heap_table) break;
			vm_init_heap_table(heap_table,(void*)expansion_address+frame, 1, 1, 0);
			vm_page_dir_add_table(vm_get_current_page_directory(),heap_table,(void*)expansion_address+frame, 1,1,0);

			// pte_t *block = mem_alloc_page(0);
			// if(!block){
			//     printf_clr(0x04,"KHEAP: kexpand()->line 101: out of memory!\n");
			//     return -1;
			// }
			// vm_map_page((unsigned int)expansion_address+frame, block);
		}
		enable_paging();
		expansion_address->state = KMALLOC_STATE_FREE;
		expansion_address->length = 0x400000*blocks_count;
		expansion_address->next = 0;
		if(tail && tail != expansion_address){
			if(!tail->next) {
				expansion_address->prev = tail;
				tail->next = expansion_address;
			}
			else {
				expansion_address->prev = tail->next;
				tail->next->next = expansion_address;
			}
		}
		else if (!tail) tail = expansion_address;

		// kmerge(expansion_address);
		kmerge(expansion_address->prev);
		return 0;
}

/*
Initialize the linked list by creating a single chunk at
a given start address and length.  The chunk is initially
free and has no next or previous chunks.
*/

void kheap_init(void *start_address, unsigned int length)
{
    if(!length) return;
	head = (struct kmalloc_chunk *)start_address;
	head->state = KMALLOC_STATE_FREE;
    head->length = length;
	head->next = 0;
	head->prev = 0;
    max_length = length;
	tail = head;
}

/*
Split a large chunk into two, such that the current chunk
has the desired length, and the next chunk has the remainder.
*/

static void ksplit( struct kmalloc_chunk *c, unsigned int length )
{
	struct kmalloc_chunk *n = (struct kmalloc_chunk *)((char *)c + length);
	n->state = KMALLOC_STATE_FREE;
	n->length = c->length - length;
	n->prev = c;
	n->next = c->next;

	if(c->next) c->next->prev = n;

	c->next = n;
	c->length = length;
}

/*
Allocate a chunk of memory of the given length.
To avoid fragmentation, round up the length to
a multiple of the chunk size.  Then, search fo
a chunk of the desired size, and split it if necessary.
*/

void *kmalloc(unsigned int length)
{
	if(!length) return;
	// round up length to a multiple of KUNIT (16 bytes-alignment)
	int extra = length%KUNIT;
	if(extra) length += (KUNIT-extra);

	length += KUNIT;

	struct kmalloc_chunk *temp = head;
	if(allocated_size+length > max_length){
		if(kexpand((allocated_size+length)-max_length) != 0){
			printf_clr(0x04,"KHEAP: kmalloc()->line 117: out of memory!\n");
			return 0;
		}
		temp = tail;
	}
	while(1) { 
		if(!temp) {
			if(kexpand(length) != 0){
				printf_clr(0x04,"KHEAP: kmalloc()->line 126: out of memory!\n");
				return 0;
			}
			temp = tail; break; //heap expanded, temp now points to the new add region
		}
		if(temp->state==KMALLOC_STATE_FREE && temp->length>=length) break;
		temp = temp->next;
	}
	// split the chunk if the remainder is greater than two units
	if(temp->length > length && ((unsigned int)temp->length-length) > 2*KUNIT){
		 ksplit(temp,length);
	}
	temp->state = KMALLOC_STATE_USED;
    if(temp->next) tail = temp->next; else tail = temp;

    allocated_size += temp->length;
	// return a pointer to the memory following the chunk header
	++blocks_counter;
	++kmallocs;
	return ++temp;
}
void* kcalloc(size_t size, char clear){
	if(size<=0) return;
	void* block = kmalloc(size);
	if(clear) memset(block,0,size);
	return block;
}
void* krealloc(void* ptr,size_t size){
	if(size<=0 || !ptr) return 0;

	struct kmalloc_chunk *c = (struct kmalloc_chunk *)ptr;
	c--;
	if(c->state != KMALLOC_STATE_FREE && c->state != KMALLOC_STATE_USED){
		printf_clr(0x04,"KHEAP: currupted block-> kfree(0x%x)\n",ptr);
		return 0;
	}
	else if(c->state == KMALLOC_STATE_FREE){
		printf_clr(0x04,"KHEAP: un-allocated block passed-> krealloc(0x%x)\n",ptr);
		return 0;
	}

	void* block = kmalloc(size);
	if(!block) return 0;
	size_t length = c->length-KUNIT;
	++c;
	memcpy(block,ptr,length);
	kfree(c);
	return block;
}

/*
Free memory by marking the chunk as de-allocated,
then attempting to merge it with the predecessor and successor.
*/
void kfree(void *ptr)
{
	if(!ptr) return;
	struct kmalloc_chunk *c = (struct kmalloc_chunk *)ptr;
	c--;

	if(c->state==KMALLOC_STATE_FREE) {
		printf_clr(0x04,"KHEAP: block is already free (0x%x)\n",ptr);
		return;
	}
	else if(c->state != KMALLOC_STATE_FREE && c->state != KMALLOC_STATE_USED){
		printf_clr(0x04,"KHEAP: currupted block-> kfree(0x%x)\n",ptr);
		return;
	}
	c->state = KMALLOC_STATE_FREE;

	unsigned int length = c->length;

	//zeroing block's data
	++c; //zeros only the data without damagaing its defined structures(state,length,pre,next)
	memset(c,0,length-KUNIT);
	--c;

    if(!c->next) tail = c;
	allocated_size -= c->length;

	kmerge(c);
	kmerge(c->prev);
	--blocks_counter;
	--kmallocs;
}

void kmalloc_debug()
{
	struct kmalloc_chunk *c = head;
	printf_clr(0x0e,"\nstate\t\tptr\t\tprev\t\tnext\t\tlength\n");
	while(c) {
		if(c->state==KMALLOC_STATE_FREE) {
			printf("free\t    ");
		} else if(c->state==KMALLOC_STATE_USED) {
			printf("used\t    ");
		} else {
			printf("kheap list corrupted at %x!\n",c);
			return;
		}
        puthex(c);printf("\t    ");puthex(c->prev);printf("\t    ");puthex(c->next);printf("\t\t%d bytes\n",c->length-KUNIT);
		c=c->next;
	}
}
unsigned int get_blocks_counter(){
	return blocks_counter;
}

/* BUG: unexposed blocks have uncorrect `prev`s or `next`s members! (broken linked-list chain)*/
void get_bug(){
	printf("kmallocs: %d\nblocks_counter: %d\n");
}
