#ifndef __EXT2_FS_H
#define __EXT2_FS_H

#include <ext2.h>
#ifndef __FS_H
#include <fs.h>
#endif
#ifndef __IDE_H
#include <ide.h>
#endif

#define EXT2_DEBUG_BLOCK_DESCRIPTORS 0

#define BGDS ext2_disk_bg_descriptors
#define SB ext2_disk_superblock
#define BGD ext2_disk_root_block
#define RN ext2_root_fsnode
#define DC ext2_disk_cache

#define BLOCKBIT(n)  (bg_buffer[((n) >> 3)] & (1 << (((n) % 8))))
#define BLOCKBYTE(n) (bg_buffer[((n) >> 3)])
#define SETBIT(n)    (1 << (((n) % 8)))

#define SECTORSIZE		512
#define DISK_PORT		0x1F0

typedef struct {
	unsigned int block_no;
	unsigned int last_use;
	unsigned char  dirty;
	unsigned char *block;
} ext2_disk_cache_entry_t;

/** Prototypes */
unsigned int ext2_disk_node_from_file(ext2_inodetable_t *inode, ext2_dir_t *direntry, fs_node_t *fnode);
ext2_inodetable_t *ext2_disk_inode(unsigned int inode);
ext2_inodetable_t *ext2_disk_alloc_inode(ext2_inodetable_t *parent, unsigned int no, char *name, unsigned short mode, unsigned int *inode_no);
fs_node_t *finddir_ext2_disk(fs_node_t *node, char *name);
void insertdir_ext2_disk(ext2_inodetable_t *p_node, unsigned int no, unsigned int inode, char *name, unsigned char type);
void ext2_disk_write_inode(ext2_inodetable_t *inode, unsigned int index);
unsigned int write_ext2_disk(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer);
unsigned int read_ext2_disk(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer);

void ext2_flush_dirty(unsigned int ent_no);
void ext2_disk_read_block(unsigned int block_no, unsigned char *buf);

void ext2_disk_write_block(unsigned int block_no, unsigned char *buf);

void ext2_set_real_block(ext2_inodetable_t *inode, unsigned int block, unsigned int real);
/**
 * Return the actual block number represented by the 'block'th block
 * in the 'inode'.
 */
unsigned int ext2_get_real_block(ext2_inodetable_t *inode, unsigned int block);

/**
 * Allocate memory for a block in an inode whose inode number is 'no'.
 */
void ext2_disk_inode_alloc_block(ext2_inodetable_t *inode, unsigned int inode_no, unsigned int block);

/**
 * Read the 'block'th block within an inode 'inode', and put it into
 * the buffer 'buf'. In other words, this function reads the actual file
 * content. 'no' is the inode number of 'inode'.
 * @return the actual block number read from.
 */
unsigned int ext2_disk_inode_read_block(ext2_inodetable_t *inode, unsigned int no, unsigned int block, unsigned char *buf);

/**
 * Write to the 'block'th block within an inode 'inode' from the buffer 'buf'.
 * In other words, this function writes to the actual file content.
 * @return the actual block number read from.
 */
unsigned int ext2_disk_inode_write_block(ext2_inodetable_t *inode, unsigned int inode_no, unsigned int block, unsigned char *buf);
/**
 * Create a new, regular, and empty file under directory 'parent'.
 */
void ext2_create(fs_node_t *parent, char *name, unsigned short permission);

/**
 * Make a new directory. 'name' consists the name for the new directory
 * to be created using 'permission' under 'parent' directory.
 * Message will be displayed in the terminal for success or failure.
 */
void ext2_mkdir(fs_node_t *parent, char *name, unsigned short permission);
unsigned char mode_to_filetype(unsigned short mode);

/**
 * Allocate a new inode with parent as the parent directory node and name as the filename
 * within that parent directory. Returns a pointer to a memory-copy of the node which
 * the client can (and should) free.
 * 'ftype' is file type, used when adding the entry to the parent dir. 1 for regular file,
 * 2 for directory, etc... 'no' is the inode number of 'parent'.
 * Upon return, the inode number of the newly allocated inode will be stored in 'inode_no'.
 *
 * This function assumes that parent directory 'parent' does not contain any entry with
 * same name as 'name'. Caller shuold ensure this.
 * Note that inode just created using this function has size of 0, which means no data
 * blocks have been allocated to the inode.
 */
ext2_inodetable_t *ext2_disk_alloc_inode(ext2_inodetable_t *parent,unsigned int no,char *name,unsigned short mode,unsigned int *inode_no);

/**
 * Return the 'index'th directory entry in the directory represented by 'inode'.
 * Caller should free the memory.
 */
ext2_dir_t *ext2_disk_direntry(ext2_inodetable_t *inode, unsigned int no, unsigned int index);

/**
 * Return the inode object on disk representing 'inode'th inode.
 * Caller should free the memory.
 */
ext2_inodetable_t *ext2_disk_inode(unsigned int inode);

/**
 * Write the 'inode' into the inode table at position 'index'.
 */
void ext2_disk_write_inode(ext2_inodetable_t *inode, unsigned int index);
unsigned int write_ext2_disk(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer);

unsigned int read_ext2_disk(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer);

void open_ext2_disk(fs_node_t *node, unsigned char read, unsigned char write);
void close_ext2_disk(fs_node_t *node);

/**
 * Return the 'index'th entry in the directory 'node'.
 * Caller should free the memory.
 */
struct dirent *readdir_ext2_disk(fs_node_t *node, unsigned int index);

/**
 * Insert an entry named 'name' with type 'type' into a directory 'p_node'
 * at the end.
 * This function assumes that parent directory 'p_node' does not contain
 * any entry with same name as 'name'. Caller should ensure this.
 */
void insertdir_ext2_disk(ext2_inodetable_t *p_node, unsigned int no, unsigned int inode, char *name, unsigned char type);

/**
 * Find the actual inode in the ramdisk image for the requested file.
 */
fs_node_t *finddir_ext2_disk(fs_node_t *node, char *name);

/**
 * Initialize in-memory struct 'fnode' using on-disk structs 'inode' and 'direntry'.
 */
unsigned int ext2_disk_node_from_file(ext2_inodetable_t *inode, ext2_dir_t *direntry,fs_node_t *fnode);

/**
 * Intiailize in-memory struct 'fnode' that represents "/" using 'inode'.
 */
unsigned int ext2_disk_node_root(ext2_inodetable_t *inode, fs_node_t *fnode);
void ext2_disk_read_superblock();
void ext2_disk_sync();
void ext2_disk_mount(unsigned int offset_sector, unsigned int max_sector);
void ext2_disk_forget_superblock();
fs_node_t* ext2_get_root_fsnode();
#endif /* __EXT2_FS_H */
