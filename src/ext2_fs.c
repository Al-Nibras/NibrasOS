/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-04-22T11:59:17+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: ext2_fs.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:51:37+03:00
 */



#include <ext2_fs.h>

unsigned int BLOCKSIZE = 1024;
unsigned int CACHEENTRIES = 10240;
unsigned int PTRS_PER_BLOCK = 256;

ext2_disk_cache_entry_t *ext2_disk_cache   = NULL;	// LSU block cache
ext2_superblock_t *ext2_disk_superblock    = NULL;
ext2_bgdescriptor_t *ext2_disk_root_block  = NULL;
fs_node_t *ext2_root_fsnode                = NULL;

unsigned int ext2_disk_inodes_per_group = 0;
unsigned int ext2_disk_bg_descriptors = 0;		// Total number of block groups

unsigned int ext2_offset = 0;


static unsigned int btos(unsigned int block) {
	return ext2_offset + block * (BLOCKSIZE / SECTORSIZE);
}

static unsigned char volatile lock;

static unsigned int _now = 1;
static unsigned int ext2_time() {
	return _now++;
}

void ext2_flush_dirty(unsigned int ent_no) {
	// write out to the disk
	for (unsigned int i = 0; i < BLOCKSIZE / SECTORSIZE; ++i) {
		ide_write_sector_retry(DISK_PORT, 0, btos(DC[ent_no].block_no) + i, (unsigned char *)((unsigned int)DC[ent_no].block + SECTORSIZE * i));
	}
	DC[ent_no].dirty = 0;
}

void ext2_disk_read_block(unsigned int block_no, unsigned char *buf) {
	if (!block_no) return;
	//spin_lock(&lock);

	if (!DC) {
		/* There is not disk cache, do a raw read */
		for (unsigned int i = 0; i < BLOCKSIZE / SECTORSIZE; ++i) {
			ide_read_sector(DISK_PORT, 0, btos(block_no) + i, (unsigned char *)((unsigned int)buf + SECTORSIZE * i));
		}
	//	spin_unlock(&lock);
		return;
	}

	int oldest = -1;
	unsigned int oldest_age = UINT32_MAX;
	for (unsigned int i = 0; i < CACHEENTRIES; ++i) {
		if (DC[i].block_no == block_no) {
			DC[i].last_use = ext2_time();
			memcpy(buf, DC[i].block, BLOCKSIZE);
			// spin_unlock(&lock);
			return;
		}
		if (DC[i].last_use < oldest_age) {
			oldest = i;
			oldest_age = DC[i].last_use;
		}
	}

	for (unsigned int i = 0; i < BLOCKSIZE / SECTORSIZE; ++i) {
		ide_read_sector(DISK_PORT, 0, btos(block_no) + i, (unsigned char *)((unsigned int)(DC[oldest].block) + SECTORSIZE * i));
	}

	if (DC[oldest].dirty) {
		ext2_flush_dirty(oldest);
	}
	memcpy(buf, DC[oldest].block, BLOCKSIZE);
	DC[oldest].block_no = block_no;
	DC[oldest].last_use = ext2_time();
	DC[oldest].dirty = 0;
	//spin_unlock(&lock);
}

void ext2_disk_write_block(unsigned int block_no, unsigned char *buf) {
	if (!block_no) {
		printf("[kernel/ext2] block_no = 0?\n");
		printf("[kernel/ext2] Investigate the call before this, you have done something terrible!\n");
		return;
	}
	// spin_lock(&lock);

	// update the cache
	int oldest = -1;
	unsigned int oldest_age = UINT32_MAX;
	for (unsigned int i = 0; i < CACHEENTRIES; ++i) {
		if (DC[i].block_no == block_no) {
			DC[i].last_use = ext2_time();
			DC[i].dirty = 1;
			memcpy(DC[i].block, buf, BLOCKSIZE);
			// spin_unlock(&lock);
			return;
		}
		if (DC[i].last_use < oldest_age) {
			oldest = i;
			oldest_age = DC[i].last_use;
		}
	}
	if (DC[oldest].dirty) {
		ext2_flush_dirty(oldest);
	}
	memcpy(DC[oldest].block, buf, BLOCKSIZE);
	DC[oldest].block_no = block_no;
	DC[oldest].last_use = ext2_time();
	DC[oldest].dirty = 1;
	// spin_unlock(&lock);
}

void ext2_set_real_block(ext2_inodetable_t *inode, unsigned int block, unsigned int real) {
	if (block < 12) {
		inode->block[block] = real;
		return;
	} else if (block < 12 + PTRS_PER_BLOCK) {
		unsigned char *tmp = kmalloc(BLOCKSIZE);
		ext2_disk_read_block(inode->block[12], tmp);
		((unsigned int *)tmp)[block - 12] = real;
		ext2_disk_write_block(inode->block[12], tmp);
		kfree(tmp);
		return;
	} else if (block < 12 + PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK) {
		unsigned int a = block - 12;
		unsigned int b = a - PTRS_PER_BLOCK;
		unsigned int c = b / PTRS_PER_BLOCK;
		unsigned int d = b - c * PTRS_PER_BLOCK;
		unsigned char *tmp = kmalloc(BLOCKSIZE);
		ext2_disk_read_block(inode->block[13], tmp);
		unsigned int nblock = ((unsigned int *)tmp)[c];
		ext2_disk_read_block(nblock, tmp);
		((unsigned int *)tmp)[d] = real;
		ext2_disk_write_block(nblock, tmp);
		kfree(tmp);
		return;
	} else if (block < 12 + PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK * PTRS_PER_BLOCK) {
		unsigned int a = block - 12;
		unsigned int b = a - PTRS_PER_BLOCK;
		unsigned int c = b - PTRS_PER_BLOCK * PTRS_PER_BLOCK;
		unsigned int d = c / (PTRS_PER_BLOCK * PTRS_PER_BLOCK);
		unsigned int e = c - d * PTRS_PER_BLOCK * PTRS_PER_BLOCK;
		unsigned int f = e / PTRS_PER_BLOCK;
		unsigned int g = e - f * PTRS_PER_BLOCK;
		unsigned char *tmp = kmalloc(BLOCKSIZE);
		ext2_disk_read_block(inode->block[14], tmp);
		unsigned int nblock = ((unsigned int *)tmp)[d];
		ext2_disk_read_block(nblock, tmp);
		nblock = ((unsigned int *)tmp)[f];
		ext2_disk_read_block(nblock, tmp);
		((unsigned int *)tmp)[g] = nblock;
		ext2_disk_write_block(nblock, tmp);
		kfree(tmp);
		return;
	}

	printf_clr(0x04,"Attempted to set a file block that was too high :(\n");
}
/**
 * Return the actual block number represented by the 'block'th block
 * in the 'inode'.
 */
unsigned int ext2_get_real_block(ext2_inodetable_t *inode, unsigned int block) {
	if (block < 12) {
		return inode->block[block];
	} else if (block < 12 + PTRS_PER_BLOCK) {
		unsigned char *tmp = kmalloc(BLOCKSIZE);
		ext2_disk_read_block(inode->block[12], tmp);
		unsigned int nblock = ((unsigned int *)tmp)[block - 12];
		kfree(tmp);
		return nblock;
	} else if (block < 12 + PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK) {
		unsigned int a = block - 12;
		unsigned int b = a - PTRS_PER_BLOCK;
		unsigned int c = b / PTRS_PER_BLOCK;
		unsigned int d = b - c * PTRS_PER_BLOCK;
		unsigned char *tmp = kmalloc(BLOCKSIZE);
		ext2_disk_read_block(inode->block[13], tmp);
		unsigned int nblock = ((unsigned int *)tmp)[c];
		ext2_disk_read_block(nblock, tmp);
		nblock = ((unsigned int *)tmp)[d];
		kfree(tmp);
		return nblock;
	} else if (block < 12 + PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK + PTRS_PER_BLOCK * PTRS_PER_BLOCK * PTRS_PER_BLOCK) {
		unsigned int a = block - 12;
		unsigned int b = a - PTRS_PER_BLOCK;
		unsigned int c = b - PTRS_PER_BLOCK * PTRS_PER_BLOCK;
		unsigned int d = c / (PTRS_PER_BLOCK * PTRS_PER_BLOCK);
		unsigned int e = c - d * PTRS_PER_BLOCK * PTRS_PER_BLOCK;
		unsigned int f = e / PTRS_PER_BLOCK;
		unsigned int g = e - f * PTRS_PER_BLOCK;
		unsigned char *tmp = kmalloc(BLOCKSIZE);
		ext2_disk_read_block(inode->block[14], tmp);
		unsigned int nblock = ((unsigned int *)tmp)[d];
		ext2_disk_read_block(nblock, tmp);
		nblock = ((unsigned int *)tmp)[f];
		ext2_disk_read_block(nblock, tmp);
		nblock = ((unsigned int *)tmp)[g];
		kfree(tmp);
		return nblock;
	}

	printf_clr(0x04,"Attempted to get a file block that was too high :(\n");
	return 0;
}

/**
 * Allocate memory for a block in an inode whose inode number is 'no'.
 */
void ext2_disk_inode_alloc_block(ext2_inodetable_t *inode, unsigned int inode_no, unsigned int block) {
	printf_clr(0x0C, "allocating block %d for inode #%d\n", block, inode_no);
	unsigned int block_no = 0, block_offset = 0, group = 0;
	char *bg_buffer = kmalloc(BLOCKSIZE);
	for (unsigned int i = 0; i < BGDS; ++i) {
		if (BGD[i].free_blocks_count > 0) {
			ext2_disk_read_block(BGD[i].block_bitmap, (unsigned char *)bg_buffer);
			while (BLOCKBIT(block_offset))
				++block_offset;
			block_no = block_offset + SB->blocks_per_group * i + 1;
			group = i;
			break;
		}
	}
	if (!block_no) {
		printf_clr(0x04, "No available blocks!\n");
		kfree(bg_buffer);
		return;
	}

	// Found a block (block_no), we need to mark it as in-use
	unsigned char b = BLOCKBYTE(block_offset);
	b |= SETBIT(block_offset);
	BLOCKBYTE(block_offset) = b;
	ext2_disk_write_block(BGD[group].block_bitmap, (unsigned char *)bg_buffer);
	kfree(bg_buffer);

	ext2_set_real_block(inode, block, block_no);

	// Now update available blocks count
	BGD[group].free_blocks_count -= 1;
	ext2_disk_write_block(2, (unsigned char *)BGD);

	inode->blocks++;
	ext2_disk_write_inode(inode, inode_no);
}

/**
 * Read the 'block'th block within an inode 'inode', and put it into
 * the buffer 'buf'. In other words, this function reads the actual file
 * content. 'no' is the inode number of 'inode'.
 * @return the actual block number read from.
 */
unsigned int ext2_disk_inode_read_block(ext2_inodetable_t *inode, unsigned int no, unsigned int block, unsigned char *buf) {
	// if the memory for 'block'th block has not been allocated to this inode, we need to
	// allocate the memory first using block bitmap.
	if (block >= inode->blocks) {
		/* Invalid block requested, return 0s */
		memset(buf, 0x00, BLOCKSIZE);
		printf_clr(0x04, "An invalid inode block [%d] was requested [have %d]\n", block, inode->blocks);
		return 0;
	}

	// The real work to read a block from an inode.
	unsigned int real_block = ext2_get_real_block(inode, block);
	ext2_disk_read_block(real_block, buf);
	return real_block;
}

/**
 * Write to the 'block'th block within an inode 'inode' from the buffer 'buf'.
 * In other words, this function writes to the actual file content.
 * @return the actual block number read from.
 */
unsigned int ext2_disk_inode_write_block(ext2_inodetable_t *inode, unsigned int inode_no, unsigned int block, unsigned char *buf) {
	/* We must allocate blocks up to this point to account for unused space in the middle. */
	while (block >= inode->blocks) {
		ext2_disk_inode_alloc_block(inode, inode_no, inode->blocks);
		if (block != inode->blocks - 1) {
			/* Clear the block */
			unsigned int real_block = ext2_get_real_block(inode, inode->blocks - 1);
			unsigned char * empty = kmalloc(BLOCKSIZE);
			memset(empty, 0x00, BLOCKSIZE);
			ext2_disk_write_block(real_block, empty);
			kfree(empty);
		}
	}

	// The real work to write to a block of an inode.
	unsigned int real_block = ext2_get_real_block(inode, block);

	printf_clr(0x07, "virtual block %d maps to real block %d\n", block, real_block);

	ext2_disk_write_block(real_block, buf);
	return real_block;
}

/**
 * Create a new, regular, and empty file under directory 'parent'.
 */
void ext2_create(fs_node_t *parent, char *name, unsigned short permission) {

	printf_clr(0x02, "Creating file %s\n", name);
	unsigned short mode = permission | EXT2_S_IFREG;
	ext2_inodetable_t *parent_inode = ext2_disk_inode(parent->inode);

	// Check to make sure no same name in the parent dir
	fs_node_t *b_exist = finddir_ext2_disk(parent, name);
	if (b_exist) {
		printf(0x0C, "File already exists!\n");
		kfree(b_exist);
		kfree(parent_inode);
		return;
	}
	kfree(b_exist);

	// Create the inode under 'parent'
	unsigned int inode_no;
	ext2_inodetable_t *inode = ext2_disk_alloc_inode(parent_inode, parent->inode, name, mode, &inode_no);
	kfree(parent_inode);

	if (inode == NULL) {
		printf_clr(0x04, "Failed to create file (inode allocation failed)\n");
		return;
	}

	kfree(inode);
}

/**
 * Make a new directory. 'name' consists the name for the new directory
 * to be created using 'permission' under 'parent' directory.
 * Message will be displayed in the terminal for success or failure.
 */
void ext2_mkdir(fs_node_t *parent, char *name, unsigned short permission) {

	unsigned short mode = permission | EXT2_S_IFDIR;
	ext2_inodetable_t *parent_inode = ext2_disk_inode(parent->inode);

	// Check to make sure no same name in the parent dir
	fs_node_t *b_exist = finddir_ext2_disk(parent, name);
	if (b_exist) {
		printf_clr(0x0C, "%s already exists\n", name);
		kfree(b_exist);
		kfree(parent_inode);
		return;
	}
	kfree(b_exist);

	// Create the inode under 'parent'
	unsigned int inode_no;
	ext2_inodetable_t *inode = ext2_disk_alloc_inode(parent_inode, parent->inode, name, mode, &inode_no);
	kfree(parent_inode);

	if (inode == NULL) {
		printf_clr(0x04, "Could not create %s\n", name);
		return;
	}

	// Init this newly created dir, put '.' and '..' into it.
	// Here we pass in 0 as the inode number for '.' and '..' because the
	// 'cd' command can handle them correctly, so it does not matter.
	insertdir_ext2_disk(inode, inode_no, 0, ".", 2);
	insertdir_ext2_disk(inode, inode_no, 0, "..", 2);

	kfree(inode);
}

unsigned char mode_to_filetype(unsigned short mode) {
	unsigned short ftype = mode & 0xF000;
	switch (ftype) {
		case EXT2_S_IFREG:
			return 1;
		case EXT2_S_IFDIR:
			return 2;
		case EXT2_S_IFCHR:
			return 3;
		case EXT2_S_IFBLK:
			return 4;
		case EXT2_S_IFIFO:
			return 5;
		case EXT2_S_IFSOCK:
			return 6;
		case EXT2_S_IFLNK:
			return 7;
	}

	// File type is unknown
	return 0;
}

/**
 * Allocate a new inode with parent as the parent directory node and name as the filename
 * within that parent directory. Returns a pointer to a memory-copy of the node which
 * the client can (and should) kfree.
 * 'ftype' is file type, used when adding the entry to the parent dir. 1 for regular file,
 * 2 for directory, etc... 'no' is the inode number of 'parent'.
 * Upon return, the inode number of the newly allocated inode will be stored in 'inode_no'.
 *
 * This function assumes that parent directory 'parent' does not contain any entry with
 * same name as 'name'. Caller shuold ensure this.
 * Note that inode just created using this function has size of 0, which means no data
 * blocks have been allocated to the inode.
 */
ext2_inodetable_t *ext2_disk_alloc_inode
(
	ext2_inodetable_t *parent,
	unsigned int no,
	char *name,
	unsigned short mode,
    unsigned int *inode_no
) {
	if ((parent->mode & EXT2_S_IFDIR) == 0 || name == NULL) {
		printf("[kernel/ext2] No name or bad parent.\n");
		return NULL;
	}

	ext2_inodetable_t *inode;

	unsigned int node_no = 0, node_offset = 0, group = 0;
	char *bg_buffer = kmalloc(BLOCKSIZE);
	/* Locate a block with an available inode. Will probably be the first block group. */
	for (unsigned int i = 0; i < BGDS; ++i) {
		if (BGD[i].free_inodes_count > 0) {
#if EXT2_DEBUG_BLOCK_DESCRIPTORS
			printf("Group %d has %d kfree inodes!\n", i, BGD[i].free_inodes_count);
#endif
			ext2_disk_read_block(BGD[i].inode_bitmap, (unsigned char *)bg_buffer);
			while (BLOCKBIT(node_offset))
				++node_offset;
			node_no = node_offset + ext2_disk_inodes_per_group * i + 1;
			group = i;
			break;
		}
	}
	if (!node_no) {
		printf("[kernel/ext2] Failure: No kfree inodes in block descriptors!\n");
		kfree(bg_buffer);
		return NULL;
	}
	/* Alright, we found an inode (node_no), we need to mark it as in-use... */
	unsigned char b = BLOCKBYTE(node_offset);
#if EXT2_DEBUG_BLOCK_DESCRIPTORS
	printf("Located an inode at #%d (%d), the byte for this block is currently set to %x\n", node_no, node_offset, (unsigned int)b);
#endif
	b |= SETBIT(node_offset);
#if EXT2_DEBUG_BLOCK_DESCRIPTORS
	printf("We would want to set it to %x\n", (unsigned int)b);
	printf("Setting it in our temporary buffer...\n");
#endif
	BLOCKBYTE(node_offset) = b;
#if EXT2_DEBUG_BLOCK_DESCRIPTORS
	printf("\nWriting back out.\n");
#endif
	ext2_disk_write_block(BGD[group].inode_bitmap, (unsigned char *)bg_buffer);
	kfree(bg_buffer);
#if EXT2_DEBUG_BLOCK_DESCRIPTORS
	printf("Okay, now we need to update the available inodes count...\n");
	printf("it is %d, it should be %d\n", BGD[group].free_inodes_count, BGD[group].free_inodes_count - 1);
	printf("\n");
	printf("%d\n", BGD[group].free_inodes_count);
#endif
	BGD[group].free_inodes_count -= 1;
#if EXT2_DEBUG_BLOCK_DESCRIPTORS
	printf("%d\n", BGD[group].free_inodes_count);
	printf("\nOkay, writing the block descriptors back to disk.\n");
#endif
	ext2_disk_write_block(2, (unsigned char *)BGD);

#if EXT2_DEBUG_BLOCK_DESCRIPTORS
	printf("Alright, we have an inode (%d), time to write it out to disk and make the file in the directory.\n", node_no);
#endif

	// Get the inode struct from the disk and init it
	inode = ext2_disk_inode(node_no);
	inode->size = 0;
	inode->blocks = 0;
	inode->mode = mode;
	ext2_disk_write_inode(inode, node_no);
	*inode_no = node_no;

	// Create an entry in the parent directory
	unsigned char ftype = mode_to_filetype(mode);
	printf("[kernel/ext2] Allocated inode, inserting directory entry [%d]...\n", node_no);
	insertdir_ext2_disk(parent, no, node_no, name, ftype);

	return inode;
}

/**
 * Return the 'index'th directory entry in the directory represented by 'inode'.
 * Caller should kfree the memory.
 */
ext2_dir_t *ext2_disk_direntry(ext2_inodetable_t *inode, unsigned int no, unsigned int index) {
	unsigned char *block = kmalloc(BLOCKSIZE);
	unsigned char block_nr = 0;
	ext2_disk_inode_read_block(inode, no, block_nr, block);
	unsigned int dir_offset = 0;
	unsigned int total_offset = 0;
	unsigned int dir_index = 0;

	while (total_offset < inode->size && dir_index <= index) {
		ext2_dir_t *d_ent = (ext2_dir_t *)((unsigned int*)block + dir_offset);

		if (dir_index == index) {
			ext2_dir_t *out = kmalloc(d_ent->rec_len);
			memcpy(out, d_ent, d_ent->rec_len);
			kfree(block);
			return out;
		}

		dir_offset += d_ent->rec_len;
		total_offset += d_ent->rec_len;
		dir_index++;

		// move on to the next block of this directory if needed.
		if (dir_offset >= BLOCKSIZE) {
			block_nr++;
			dir_offset -= BLOCKSIZE;
			ext2_disk_inode_read_block(inode, no, block_nr, block);
		}
	}

	kfree(block);
	return NULL;
}

/**
 * Return the inode object on disk representing 'inode'th inode.
 * Caller should kfree the memory.
 */
ext2_inodetable_t *ext2_disk_inode(unsigned int inode) {
	unsigned int group = inode / ext2_disk_inodes_per_group;
	if (group > BGDS) {
		return NULL;
	}
	unsigned int inode_table_block = BGD[group].inode_table;
	inode -= group * ext2_disk_inodes_per_group;	// adjust index within group
	unsigned int block_offset		= ((inode - 1) * SB->inode_size) / BLOCKSIZE;
	unsigned int offset_in_block    = (inode - 1) - block_offset * (BLOCKSIZE / SB->inode_size);

	unsigned char *buf                = kmalloc(BLOCKSIZE);
	ext2_inodetable_t *inodet   = kmalloc(SB->inode_size);

	ext2_disk_read_block(inode_table_block + block_offset, buf);
	ext2_inodetable_t *inodes = (ext2_inodetable_t *)buf;

	memcpy(inodet, (unsigned char *)((unsigned int)inodes + offset_in_block * SB->inode_size), SB->inode_size);

	kfree(buf);
	return inodet;
}

/**
 * Write the 'inode' into the inode table at position 'index'.
 */
void ext2_disk_write_inode(ext2_inodetable_t *inode, unsigned int index) {
	unsigned int group = index / ext2_disk_inodes_per_group;
	if (group > BGDS) {
		return;
	}

	unsigned int inode_table_block = BGD[group].inode_table;
	index -= group * ext2_disk_inodes_per_group;	// adjust index within group
	unsigned int block_offset = ((index - 1) * SB->inode_size) / BLOCKSIZE;
	unsigned int offset_in_block = (index - 1) - block_offset * (BLOCKSIZE / SB->inode_size);

	ext2_inodetable_t *inodet = kmalloc(BLOCKSIZE);
	/* Read the current table block */
	ext2_disk_read_block(inode_table_block + block_offset, (unsigned char *)inodet);
	memcpy((unsigned char *)((unsigned int)inodet + offset_in_block * SB->inode_size), inode, SB->inode_size);
	ext2_disk_write_block(inode_table_block + block_offset, (unsigned char *)inodet);
	kfree(inodet);
}

unsigned int write_ext2_disk(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer) {
	ext2_inodetable_t *inode = ext2_disk_inode(node->inode);
	unsigned int end = offset + size;
	unsigned int start_block 	= offset / BLOCKSIZE;
	unsigned int end_block		= end / BLOCKSIZE;
	unsigned int end_size		= end - end_block * BLOCKSIZE;
	unsigned int size_to_write  = end - offset;
	printf("[kernel/ext2] Write at node 0x%x, offset %d, size %d, buffer=0x%x\n", node, offset, size, buffer);
	if (end_size == 0) {
		end_block--;
	}

	// need to update if size has increased.
	if (inode->size < end) {
		inode->size = end;
		ext2_disk_write_inode(inode, node->inode);
	}

	if (start_block == end_block) {
		void *buf = kmalloc(BLOCKSIZE);
		ext2_disk_inode_read_block(inode, node->inode, start_block, buf);
		memcpy((unsigned char *)((unsigned int*)buf + (offset % BLOCKSIZE)), buffer, size_to_write);
		printf("[kernel/ext2] Single-block write.\n");
		ext2_disk_inode_write_block(inode, node->inode, start_block, buf);
		kfree(buf);
		kfree(inode);
		return size_to_write;
	} else {
		unsigned int block_offset;
		unsigned int blocks_read = 0;
		for (block_offset = start_block; block_offset < end_block; block_offset++, blocks_read++) {
			if (block_offset == start_block) {
				void *buf = kmalloc(BLOCKSIZE);
				ext2_disk_inode_read_block(inode, node->inode, block_offset, buf);
				memcpy((unsigned char *)((unsigned int)buf + (offset % BLOCKSIZE)), buffer, BLOCKSIZE - (offset % BLOCKSIZE));
				printf("[kernel/ext2] Writing block [loop...]...\n");
				ext2_disk_inode_write_block(inode, node->inode, start_block, buf);
				kfree(buf);
			} else {
				printf("[kernel/ext2] Writing block [buffer...?]...\n");
				ext2_disk_inode_write_block(inode, node->inode, block_offset,
						buffer + BLOCKSIZE * blocks_read - (block_offset % BLOCKSIZE));
			}
		}
		void *buf = kmalloc(BLOCKSIZE);
		ext2_disk_inode_read_block(inode, node->inode, end_block, buf);
		memcpy(buf, buffer + BLOCKSIZE * blocks_read - (block_offset % BLOCKSIZE), end_size);
		printf("[kernel/ext2] Writing block [tail]...\n");
		ext2_disk_inode_write_block(inode, node->inode, end_block, buf);
		kfree(buf);
	}
	kfree(inode);
	return size_to_write;
}

unsigned int read_ext2_disk(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer) {
	ext2_inodetable_t *inode = ext2_disk_inode(node->inode);
	unsigned int end;
	if (offset + size > inode->size) {
		end = inode->size;
	} else {
		end = offset + size;
	}
	unsigned int start_block  = offset / BLOCKSIZE;
	unsigned int end_block    = end / BLOCKSIZE;
	unsigned int end_size     = end - end_block * BLOCKSIZE;
	unsigned int size_to_read = end - offset;
	if (end_size == 0) {
		end_block--;
	}
	if (start_block == end_block) {
		void *buf = kmalloc(BLOCKSIZE);
		ext2_disk_inode_read_block(inode, node->inode, start_block, buf);
		memcpy(buffer, (unsigned char *)(((unsigned int)buf) + (offset % BLOCKSIZE)), size_to_read);
		kfree(buf);
		kfree(inode);
		return size_to_read;
	} else {
		unsigned int block_offset;
		unsigned int blocks_read = 0;
		for (block_offset = start_block; block_offset < end_block; block_offset++, blocks_read++) {
			if (block_offset == start_block) {
				void *buf = kmalloc(BLOCKSIZE);
				ext2_disk_inode_read_block(inode, node->inode, block_offset, buf);
				memcpy(buffer, (unsigned char *)(((unsigned int)buf) + (offset % BLOCKSIZE)), BLOCKSIZE - (offset % BLOCKSIZE));
				kfree(buf);
			} else {
				void *buf = kmalloc(BLOCKSIZE);
				ext2_disk_inode_read_block(inode, node->inode, block_offset, buf);
				memcpy(buffer + BLOCKSIZE * blocks_read - (offset % BLOCKSIZE), buf, BLOCKSIZE);
				kfree(buf);
			}
		}
		void *buf = kmalloc(BLOCKSIZE);
		ext2_disk_inode_read_block(inode, node->inode, end_block, buf);
		memcpy(buffer + BLOCKSIZE * blocks_read - (offset % BLOCKSIZE), buf, end_size);
		kfree(buf);
	}
	kfree(inode);
	return size_to_read;
}

void
open_ext2_disk(fs_node_t *node, unsigned char read, unsigned char write) {
	/* Nothing to do here */
}

void
close_ext2_disk(fs_node_t *node) {
	/* Nothing to do here */
}

/**
 * Return the 'index'th entry in the directory 'node'.
 * Caller should kfree the memory.
 */
struct dirent *readdir_ext2_disk(fs_node_t *node, unsigned int index) {

	ext2_inodetable_t *inode = ext2_disk_inode(node->inode);
	if( !(inode->mode & EXT2_S_IFDIR) ) {
        printf_clr(0x04,"ERROR: readdir_ext2_disk()-> node is not directory\n");
        return;
    }
	ext2_dir_t *direntry = ext2_disk_direntry(inode, node->inode, index);
	if (!direntry) {
		return NULL;
	}
	struct dirent *dirent = kmalloc(sizeof(struct dirent));
	memcpy(&dirent->name, &direntry->name, direntry->name_len);
	dirent->name[direntry->name_len] = '\0';
	dirent->ino = direntry->inode;
	kfree(direntry);
	kfree(inode);
	return dirent;
}

/**
 * Insert an entry named 'name' with type 'type' into a directory 'p_node'
 * at the end.
 * This function assumes that parent directory 'p_node' does not contain
 * any entry with same name as 'name'. Caller should ensure this.
 */
void insertdir_ext2_disk(ext2_inodetable_t *p_node, unsigned int no, unsigned int inode, char *name, unsigned char type) {
	/* XXX HACK This needs to be seriously fixed up. */
	printf_clr(0x02, "Request to insert new directory entry at 0x%x#%d->%d '%s' type %d\n", p_node, no, inode, name, type);
	if( !(p_node->mode & EXT2_S_IFDIR)){
        printf_clr(0x04,"ERROR: insertdir_ext2_disk()-> node is not directory\n");
        return;
    }
	void *block = kmalloc(BLOCKSIZE);
	unsigned int block_nr = 0;
	ext2_disk_inode_read_block(p_node, no, block_nr, block);
	unsigned int dir_offset = 0;
	unsigned int total_offset = 0;

	// first, iterate pass the last entry in the parent directory.
	while (total_offset < p_node->size) {
		ext2_dir_t *d_ent = (ext2_dir_t *)((unsigned int*)block + dir_offset);
		if (d_ent->rec_len + total_offset == p_node->size) {
			d_ent->rec_len = d_ent->name_len + sizeof(ext2_dir_t);
			while (d_ent->rec_len % 4 > 0) {
				d_ent->rec_len++;
			}
			dir_offset   += d_ent->rec_len;
			total_offset += d_ent->rec_len;
			break;
		}

		dir_offset += d_ent->rec_len;
		total_offset += d_ent->rec_len;

		// move on to the next block of this directory if needed.
		if (dir_offset >= BLOCKSIZE) {
			block_nr++;
			dir_offset -= BLOCKSIZE;
			ext2_disk_inode_read_block(p_node, no, block_nr, block);
			printf_clr(0x02, "Advancing to next block...\n");
		}
	}

	printf_clr(0x02, "total offset = %d; block = %d; offset within block = %d\n", total_offset, block_nr, dir_offset);

	// Put the new directory entry at 'dir_offset' in block 'block_nr'.
	unsigned int size = p_node->size - total_offset;
	if (dir_offset + size > BLOCKSIZE) {
		printf_clr(0x0C, "Directory entry is beyond first block of directory. This might be bad.\n");
	}
	ext2_dir_t *new_entry = kmalloc(size);

	// Initialize the new entry.
	new_entry->inode = inode;
	new_entry->rec_len = size;
	new_entry->name_len = (unsigned char)strlen(name);
	new_entry->file_type = type;
	memcpy(&new_entry->name, name, strlen(name));

	// Write back to block.
	memcpy(((unsigned char *)block) + dir_offset, new_entry, size);
	memset(((unsigned char *)block) + dir_offset + new_entry->rec_len, 0x00, 4);
	ext2_disk_inode_write_block(p_node, no, block_nr, block);

	kfree(new_entry);

	// Update parent node size
	//p_node->size += size;
	ext2_disk_write_inode(p_node, no);

	kfree(block);
}

/**
 * Find the actual inode in the ramdisk image for the requested file.
 */
fs_node_t *finddir_ext2_disk(fs_node_t *node, char *name) {

	ext2_inodetable_t *inode = ext2_disk_inode(node->inode);
	if( !(inode->mode & EXT2_S_IFDIR)){
        printf_clr(0x04,"ERROR: finddir_ext2_disk()-> node is not directory\n");
        return;
    }
	void *block = kmalloc(BLOCKSIZE);
	ext2_dir_t *direntry = NULL;
	unsigned char block_nr = 0;
	ext2_disk_inode_read_block(inode, node->inode, block_nr, block);
	unsigned int dir_offset = 0;
	unsigned int total_offset = 0;

	// Look through the requested entries until we find what we're looking for
	while (total_offset < inode->size) {
		ext2_dir_t *d_ent = (ext2_dir_t *)((unsigned int*)block + dir_offset);

		if (strlen(name) != d_ent->name_len) {
			dir_offset += d_ent->rec_len;
			total_offset += d_ent->rec_len;

			// move on to the next block of this directory if need.
			if (dir_offset >= BLOCKSIZE) {
				block_nr++;
				dir_offset -= BLOCKSIZE;
				ext2_disk_inode_read_block(inode, node->inode, block_nr, block);
			}

			continue;
		}

		char *dname = kmalloc(sizeof(char) * (d_ent->name_len + 1));
		memcpy(dname, &(d_ent->name), d_ent->name_len);
		dname[d_ent->name_len] = '\0';
		if (!strcmp(dname, name)) {
			kfree(dname);
			direntry = kmalloc(d_ent->rec_len);
			memcpy(direntry, d_ent, d_ent->rec_len);
			break;
		}
		kfree(dname);

		dir_offset += d_ent->rec_len;
		total_offset += d_ent->rec_len;

		// move on to the next block of this directory if need.
		if (dir_offset >= BLOCKSIZE) {
			block_nr++;
			dir_offset -= BLOCKSIZE;
			ext2_disk_inode_read_block(inode, node->inode, block_nr, block);
		}
	}
	kfree(inode);
	kfree(block);
	if (!direntry) {
		// We could not find the requested entry in this directory.
		return NULL;
	}
	fs_node_t *outnode = kmalloc(sizeof(fs_node_t));

	inode = ext2_disk_inode(direntry->inode);

	if (!ext2_disk_node_from_file(inode, direntry, outnode)) {
		printf_clr(0x04, "finddir_ext2_disk: Couldn't find the inode\n");
	}

	kfree(direntry);
	kfree(inode);
	return outnode;
}

/**
 * Initialize in-memory struct 'fnode' using on-disk structs 'inode' and 'direntry'.
 */
unsigned int ext2_disk_node_from_file(ext2_inodetable_t *inode, ext2_dir_t *direntry,
								  fs_node_t *fnode) {
	if (!fnode) {
		/* You didn't give me a node to write into, go **** yourself */
		return 0;
	}
	/* 0x0Brmation from the direntry */
	fnode->inode = direntry->inode;
	memcpy(&fnode->name, &direntry->name, direntry->name_len);
	fnode->name[direntry->name_len] = '\0';
	/* 0x0Brmation from the inode */
	fnode->uid = inode->uid;
	fnode->gid = inode->gid;
	fnode->length = inode->size;
	fnode->mask = inode->mode & 0xFFF;
	/* File Flags */
	fnode->flags = 0;
	if ((inode->mode & EXT2_S_IFREG) == EXT2_S_IFREG) {
		fnode->flags |= FS_FILE;
		fnode->create = NULL;
		fnode->mkdir = NULL;
	}
	if ((inode->mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
		fnode->flags |= FS_DIRECTORY;
		fnode->create = ext2_create;
		fnode->mkdir = ext2_mkdir;
	}
	if ((inode->mode & EXT2_S_IFBLK) == EXT2_S_IFBLK) {
		fnode->flags |= FS_BLOCKDEVICE;
	}
	if ((inode->mode & EXT2_S_IFCHR) == EXT2_S_IFCHR) {
		fnode->flags |= FS_CHARDEVICE;
	}
	if ((inode->mode & EXT2_S_IFIFO) == EXT2_S_IFIFO) {
		fnode->flags |= FS_PIPE;
	}
	if ((inode->mode & EXT2_S_IFLNK) == EXT2_S_IFLNK) {
		fnode->flags |= FS_SYMLINK;
	}

	fnode->atime   = inode->atime;
	fnode->mtime   = inode->mtime;
	fnode->ctime   = inode->ctime;
	printf_clr(0x0B, "file a/m/c times are %d/%d/%d\n", fnode->atime, fnode->mtime, fnode->ctime);

	fnode->read    = read_ext2_disk;
	fnode->write   = write_ext2_disk;
	fnode->open    = open_ext2_disk;
	fnode->close   = close_ext2_disk;
	fnode->readdir = readdir_ext2_disk;
	fnode->finddir = finddir_ext2_disk;
	return 1;
}

/**
 * Intiailize in-memory struct 'fnode' that represents "/" using 'inode'.
 */
unsigned int ext2_disk_node_root(ext2_inodetable_t *inode, fs_node_t *fnode) {
	if (!fnode) {
		return 0;
	}
	/* 0x0Brmation for root dir */
	fnode->inode = 2;
	fnode->name[0] = '/';
	fnode->name[1] = '\0';
	/* 0x0Brmation from the inode */
	fnode->uid = inode->uid;
	fnode->gid = inode->gid;
	fnode->length = inode->size;
	fnode->mask = inode->mode & 0xFFF;
	/* File Flags */
	fnode->flags = 0;
	if ((inode->mode & EXT2_S_IFREG) == EXT2_S_IFREG) {
		printf_clr(0x04, "ERROR: Root appears to be a regular file.\n");
		return 0;
	}
	if ((inode->mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
		fnode->flags |= FS_DIRECTORY;
		fnode->create = ext2_create;
		fnode->mkdir = ext2_mkdir;
	} else {
		printf_clr(0x04, "ERROR: Root doesn't appear to be a directory.\n");
		return 0;
	}
	if ((inode->mode & EXT2_S_IFBLK) == EXT2_S_IFBLK) {
		fnode->flags |= FS_BLOCKDEVICE;
	}
	if ((inode->mode & EXT2_S_IFCHR) == EXT2_S_IFCHR) {
		fnode->flags |= FS_CHARDEVICE;
	}
	if ((inode->mode & EXT2_S_IFIFO) == EXT2_S_IFIFO) {
		fnode->flags |= FS_PIPE;
	}
	if ((inode->mode & EXT2_S_IFLNK) == EXT2_S_IFLNK) {
		fnode->flags |= FS_SYMLINK;
	}

	fnode->atime   = inode->atime;
	fnode->mtime   = inode->mtime;
	fnode->ctime   = inode->ctime;

	fnode->read    = read_ext2_disk;
	fnode->write   = write_ext2_disk;
	fnode->open    = open_ext2_disk;
	fnode->close   = close_ext2_disk;
	fnode->readdir = readdir_ext2_disk;
	fnode->finddir = finddir_ext2_disk;
	return 1;
}

void ext2_disk_read_superblock() {
	printf_clr(0x0A, "Volume Name: '%s'\n", SB->volume_name);
	printf_clr(0x0A, "%d inodes\n", SB->inodes_count);
	printf_clr(0x0A, "%d blocks\n", SB->blocks_count);
	printf_clr(0x0A, "%d free blocks\n", SB->free_blocks_count);
	printf_clr(0x0A, "Last mount time: 0x%x\n", SB->mtime);
	printf_clr(0x0A, "Last write time: 0x%x\n", SB->wtime);
	printf_clr(0x0A, "Mounted %d times.\n", SB->mnt_count);
	printf_clr(0x0A, "Magic Number: 0x%x\n", SB->magic);
	printf_clr(0x0A, "feature_incompat = 0x%x\n", SB->feature_incompat);
}

void ext2_disk_sync() {
	//spin_lock(&lock);
	for (unsigned int i = 0; i < CACHEENTRIES; ++i) {
		if (DC[i].dirty) {
			ext2_flush_dirty(i);
		}
	}
	//spin_unlock(&lock);
}

void ext2_disk_mount(unsigned int offset_sector, unsigned int max_sector) {
	printf_clr(0x02, "Mounting EXT2 partition between sectors [%d:%d].\n", offset_sector, max_sector);

	ext2_offset = offset_sector;

	BLOCKSIZE = 1024;

	SB = kmalloc(BLOCKSIZE);

	ext2_disk_read_block(1, (unsigned char *)SB);

	if( SB->magic != EXT2_SUPER_MAGIC){
        printf_clr(0x04, "ERROR: ext2_disk_mount()->bad block (bad magic number)\n");
        return;
    }
	if (SB->inode_size == 0) {
		SB->inode_size = 128;
	}
	BLOCKSIZE = 1024 << SB->log_block_size;
	if (BLOCKSIZE > 2048) {
		CACHEENTRIES /= 4;
	}
	PTRS_PER_BLOCK = BLOCKSIZE / 4;
	printf_clr(0x02, "Log block size = %d -> %d\n", SB->log_block_size, BLOCKSIZE);
	BGDS = SB->blocks_count / SB->blocks_per_group;
	if (SB->blocks_per_group * BGDS < SB->blocks_count) {
		BGDS += 1;
	}
	ext2_disk_inodes_per_group = SB->inodes_count / BGDS;

	printf_clr(0x02, "Allocating cache...\n");
	DC = kmalloc(sizeof(ext2_disk_cache_entry_t) * CACHEENTRIES);
	for (unsigned int i = 0; i < CACHEENTRIES; ++i) {
		DC[i].block = kmalloc(BLOCKSIZE);
		if (i % 128 == 0) {
			printf_clr(0x0B, "Allocated cache block #%d\n", i+1);
		}
	}
	printf_clr(0x02, "Cache allocated successfully\n");

	// load the block group descriptors
	int bgd_block_span = sizeof(ext2_bgdescriptor_t) * BGDS / BLOCKSIZE + 1;
	BGD = kmalloc(BLOCKSIZE * bgd_block_span);
	printf_clr(0x0B, "bgd_block_span = %d\n", bgd_block_span);

	int bgd_offset = 2;

	if (BLOCKSIZE > 1024) {
		bgd_offset = 1;
	}

	for (int i = 0; i < bgd_block_span; ++i) {
		ext2_disk_read_block(bgd_offset + i, (unsigned char *)((unsigned int)BGD + BLOCKSIZE * i));
	}

#if EXT2_DEBUG_BLOCK_DESCRIPTORS
	char * bg_buffer = kmalloc(BLOCKSIZE * sizeof(char));
	for (unsigned int i = 0; i < BGDS; ++i) {
		printf_clr(0x0B, "Block Group Descriptor #%d @ %d", i, bgd_offset + i * SB->blocks_per_group);
		printf_clr(0x0B, "\tBlock Bitmap @ %d\n", BGD[i].block_bitmap); {
			printf_clr(0x0B, "Examining block bitmap at %d\n", BGD[i].block_bitmap);
			ext2_disk_read_block(BGD[i].block_bitmap, (unsigned char *)bg_buffer);
			unsigned int j = 0;
			while (BLOCKBIT(j)) {
				++j;
			}
			printf_clr(0x0B, "First kfree block in group is %d", j + BGD[i].block_bitmap - 2);
		}
		printf_clr(0x0B, "\tInode Bitmap @ %d\n", BGD[i].inode_bitmap); {
			printf_clr(0x0B, "Examining inode bitmap at %d\n", BGD[i].inode_bitmap);
			ext2_disk_read_block(BGD[i].inode_bitmap, (unsigned char *)bg_buffer);
			unsigned int j = 0;
			while (BLOCKBIT(j)) {
				++j;
			}
			printf_clr(0x0B, "First kfree inode in group is %d\n", j + ext2_disk_inodes_per_group * i + 1);
		}
		printf_clr(0x0B, "Inode Table  @ %d", BGD[i].inode_table);
		printf_clr(0x0B, "\tfree Blocks =  %d", BGD[i].free_blocks_count);
		printf_clr(0x0B, "\tfree Inodes =  %d\n", BGD[i].free_inodes_count);
	}
	kfree(bg_buffer);
#endif



	ext2_inodetable_t *root_inode = ext2_disk_inode(2);

	/* BUG: DEBUGGING: */
	kmalloc_debug();
	while(1);



	RN = (fs_node_t *)kmalloc(sizeof(fs_node_t));
	if (!ext2_disk_node_root(root_inode, RN)) {
		printf_clr(0x0C, "ERROR: ext2_disk_mount()-> root_inode could not be allocated\n");
	}
	printf_clr(0x02, "Root file system is ready.\n");
	fs_root = RN;
	printf_clr(0x02, "Mounted EXT2 disk, root VFS node is at 0x%x\n", RN);
}

void ext2_disk_forget_superblock() {
	kfree(SB);
}
fs_node_t* ext2_get_root_fsnode(){
	return ext2_root_fsnode;
}
