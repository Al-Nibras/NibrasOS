//  NibrasOS
//  memory.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-07-15T13:44:49+03:00.
//	Last modification time on 2017-07-29T23:49:33+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <memory.h>

struct phys_mem_avail null_hole = {0,0};

static unsigned int memory_start_address = 0;
static unsigned int memory_size = 0;
static unsigned int *frames_bitmap = 0;
static unsigned int bitmap_cells = 0;
static unsigned int frames_count = 0;
static int allocated_frames_count = 0;

static void mem_set_bit(int bit){
  frames_bitmap[bit / 32] |= (1 << (bit % 32));
}
static void mem_unset_bit(int bit){
  frames_bitmap[bit / 32] &= ~ (1 << (bit % 32));
}
static int mem_test_bit(int bit){
  return frames_bitmap[bit / 32] & (1 << (bit % 32));
}

int mem_find_first_free_frame(){
  if(!frames_bitmap){
    printf_clr(0x04,"frames_bitmap is NULL");
    return -1;
  }
  else if(allocated_frames_count >= frames_count){
    printf_clr(0x04,"OUT OF MEMORY!");
    return -1;
  }

  for (unsigned int i=0; i<bitmap_cells; i++)
		if (frames_bitmap[i] != 0xffffffff){
			for (int j=0; j<BITMAP_SIZE_PER_PAGE; j++)
				if (! (frames_bitmap[i] & (1<<j)) )
					return i*BITMAP_SIZE_PER_PAGE+j;
    }
	return -1;
}
static int mem_find_first_free_frames(unsigned int size){
  if(!size) return -1;
  if(size == 1) return mem_find_first_free_frame();


  for (unsigned int i=0; i<bitmap_cells; i++)
		if (frames_bitmap[i] != 0xffffffff)
		for (int j=0; j<BITMAP_SIZE_PER_PAGE; j++) {	//! test each bit in the dword
            int bit = (1<<j);
            if ( !(frames_bitmap[i] & bit) ) {
					int startingBit = i*32;
					startingBit+=j;		//get the free bit in the dword at index i

					unsigned int free=0; //loop through each bit to see if its enough space
					for (unsigned int count=0; count<=size;count++) {
						if (!mem_test_bit(startingBit+count) )
							free++;	// this bit is clear (free frame)

						if (free==size)
							return i*BITMAP_SIZE_PER_PAGE+j; //free count==size needed; return index
					}
				}
			}

	return -1;
}



void *mem_alloc_page(char clear_page){
    if(!frames_bitmap){
      printf_clr(0x04,"frames_bitmap is NULL");
      return 0;
    }
    else if(allocated_frames_count >= frames_count){
      printf_clr(0x04,"OUT OF MEMORY!");
      return 0;
    }

    for(int i =0 ;i<bitmap_cells;++i){
      if(frames_bitmap[i] == 0xffffffff) continue; //early break
      for(int j = 0; j<BITMAP_SIZE_PER_PAGE; ++j){
        if( !(frames_bitmap[i]&(1<<j)) ) //free page
        {
          mem_set_bit(i*BITMAP_SIZE_PER_PAGE+j);
          unsigned int page_frame = i*BITMAP_SIZE_PER_PAGE+j;
          void* page_addr = page_frame*PAGE_SIZE;
          if(clear_page)
            memset(page_addr,0,PAGE_SIZE);
          ++allocated_frames_count;
          return page_addr;
        }
      }
    }
    printf_clr(0x04,"OUT OF MEMORY!");
    return 0;
}

void* mem_alloc_pages(unsigned int size) {
  if(!size) return 0;
	if ((frames_count - allocated_frames_count) <= size)
		return 0;//no enough space
	int frame = mem_find_first_free_frames(size);

	if (frame < 0)
		return 0;	//no enough space

	for (unsigned int i=0; i<size; i++)
		mem_set_bit(frame+i);

	unsigned int phys_addr = frame * PAGE_SIZE;
	allocated_frames_count+=size;

  return (void*)phys_addr;
}

void mem_free_pages(void* page, unsigned int size) {
	unsigned int phys_addr = (unsigned int)page;
	int frame = phys_addr / PAGE_SIZE;
	for (unsigned int i=0; i<size; i++)
		mem_set_bit(frame+i);

	allocated_frames_count-=size;
}

static void unset_avail_mem(struct phys_mem_avail* avails){
		if(!avails) return;
		while (avails->base != null_hole.base && avails->len != null_hole.len) {
            if(avails->base >= 0x100000)
                mem_init_region(avails->base,avails->len);
			++avails;
		}
}
void memory_init(unsigned int mem_sz, unsigned int mem_start_addr, struct phys_mem_avail *avail_mem){
    if(!mem_sz) return;
    memory_start_address = mem_start_addr;
    frames_bitmap = (unsigned int*)mem_start_addr;
    memory_size = mem_sz;
    frames_count = mem_sz/PAGE_SIZE;
    bitmap_cells = (frames_count/BITMAP_SIZE_PER_PAGE)+1; //added one forsake of floor()!

    printf_clr(0x0d,"memory_size: 0x%x\nframes_count: 0x%x\nbitmap_cells: 0x%x\n",memory_size,frames_count,bitmap_cells);
    memsetw(frames_bitmap,0xffffffff,bitmap_cells);

    allocated_frames_count = frames_count;

    unset_avail_mem(avail_mem);
    allocated_frames_count += 256; //first 1MB frames count (0x100000 / 0x1000 = 256)

    unsigned int kernel_reserved_region = memory_start_address;
    int extra = kernel_reserved_region%0x1000;
    if(extra) kernel_reserved_region += (0x1000-extra);
    int kernel_frames = (kernel_reserved_region/0x1000)/BITMAP_SIZE_PER_PAGE;
    for (int i = 0; i <= kernel_frames; i++) {
        frames_bitmap[i] = 0xffffffff; //set first 1MB+3KB of memory
        ++allocated_frames_count;
    }
}
// void* mem_alloc_after(int count){
//     int bits[count];
//     for(int i =0; i<count; ++i){
//         bits[i] = mem_find_first_free_frame();
//         mem_alloc_page(0);
//     }
//     void* address = mem_alloc_page(0);
//     for(int i =0; i<count; ++i)
//         mem_unset_bit(bits[i]);
//
//     return address;
// }
void mem_init_region (unsigned int base, unsigned int size) {
  //makes sure RAM reserved regions are safe
  //by unsetting only available regions frames
	int align = (base / PAGE_ALIGN);
	int blocks = size / PAGE_SIZE;
	while(blocks>0) {
		mem_unset_bit(align++);
		--allocated_frames_count;
        --blocks;
	}
	//mem_set_bit(0);	//first blocks is always set [BIOS and Kernel regions]
}
void mem_deinit_region(unsigned int base, unsigned int size) {
  int align = base / PAGE_ALIGN;
	int blocks = size / PAGE_SIZE;
  while(blocks>0){
		mem_set_bit(align++);
		++allocated_frames_count;
    --blocks;
	}
}
void mem_free_page(void * page_addr) {
  if(!page_addr) return;
  unsigned int phys_address = (unsigned int)page_addr;
	int frame = phys_address / PAGE_SIZE;
	mem_unset_bit(frame);
  --allocated_frames_count;
}
extern unsigned int end; //defined in link.ld
static void* end_seg = &end;
void* kern_data_seg_alloc(unsigned int size){
    void* temp = end_seg;
    end_seg += size;
    return temp;
}
void* mem_alloc_at(unsigned int base, unsigned int size){
    if(!base || !size) return;
    void* temp = base;
    int frame = (base&0xfffff000)/0x1000;
    int req_frames_count = 1;
    while( size%0x1000 > 0 && size%0x1000 != size){
        req_frames_count++;
        size -=0x1000;
    }
    for(int i = 0; i<=frame+req_frames_count; ++i)
        if(!mem_test_bit(i))
            mem_set_bit(i);
    return temp;
}
