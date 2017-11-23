#ifndef __EXT2_H
#define __EXT2_H

#ifndef __TYPES_H
#include <types.h>
#endif
#ifndef __FS_H
#include <fs.h>
#endif


#define EXT2_SUPER_MAGIC 0xEF53

/* Super block struct. */
struct ext2_superblock {
	unsigned int inodes_count;
	unsigned int blocks_count;
	unsigned int r_blocks_count;
	unsigned int free_blocks_count;
	unsigned int free_inodes_count;
	unsigned int first_data_block;
	unsigned int log_block_size;
	unsigned int log_frag_size;
	unsigned int blocks_per_group;
	unsigned int frags_per_group;
	unsigned int inodes_per_group;
	unsigned int mtime;
	unsigned int wtime;

	unsigned short mnt_count;
	unsigned short max_mnt_count;
	unsigned short magic;
	unsigned short state;
	unsigned short errors;
	unsigned short minor_rev_level;

	unsigned int lastcheck;
	unsigned int checkinterval;
	unsigned int creator_os;
	unsigned int rev_level;

	unsigned short def_resuid;
	unsigned short def_resgid;

	/* EXT2_DYNAMIC_REV */
	unsigned int first_ino;
	unsigned short inode_size;
	unsigned short block_group_nr;
	unsigned int feature_compat;
	unsigned int feature_incompat;
	unsigned int feature_ro_compat;

	unsigned char uuid[16];
	unsigned char volume_name[16];

	unsigned char last_mounted[64];

	unsigned int algo_bitmap;

	/* Performance Hints */
	unsigned char prealloc_blocks;
	unsigned char prealloc_dir_blocks;
	unsigned short _padding;

	/* Journaling Support */
	unsigned char journal_uuid[16];
	unsigned int journal_inum;
	unsigned int jounral_dev;
	unsigned int last_orphan;

	/* Directory Indexing Support */
	unsigned int hash_seed[4];
	unsigned char def_hash_version;
	unsigned short _padding_a;
	unsigned char _padding_b;

	/* Other Options */
	unsigned int default_mount_options;
	unsigned int first_meta_bg;
	unsigned char _unused[760];

} __attribute__ ((packed));

typedef struct ext2_superblock ext2_superblock_t;

/* Block group descriptor. */
struct ext2_bgdescriptor {
	unsigned int block_bitmap;
	unsigned int inode_bitmap;		// block no. of inode bitmap
	unsigned int inode_table;
	unsigned short free_blocks_count;
	unsigned short free_inodes_count;
	unsigned short used_dirs_count;
	unsigned short pad;
	unsigned char reserved[12];
} __attribute__ ((packed));

typedef struct ext2_bgdescriptor ext2_bgdescriptor_t;

/* File Types */
#define EXT2_S_IFSOCK	0xC000
#define EXT2_S_IFLNK	0xA000
#define EXT2_S_IFREG	0x8000
#define EXT2_S_IFBLK	0x6000
#define EXT2_S_IFDIR	0x4000
#define EXT2_S_IFCHR	0x2000
#define EXT2_S_IFIFO	0x1000

/* setuid, etc. */
#define EXT2_S_ISUID	0x0800
#define EXT2_S_ISGID	0x0400
#define EXT2_S_ISVTX	0x0200

/* rights */
#define EXT2_S_IRUSR	0x0100
#define EXT2_S_IWUSR	0x0080
#define EXT2_S_IXUSR	0x0040
#define EXT2_S_IRGRP	0x0020
#define EXT2_S_IWGRP	0x0010
#define EXT2_S_IXGRP	0x0008
#define EXT2_S_IROTH	0x0004
#define EXT2_S_IWOTH	0x0002
#define EXT2_S_IXOTH	0x0001

/* This is not actually the inode table.
 * It represents an inode in an inode table on disk. */
struct ext2_inodetable {
	unsigned short mode;
	unsigned short uid;
	unsigned int size;			// file length in byte.
	unsigned int atime;
	unsigned int ctime;
	unsigned int mtime;
	unsigned int dtime;
	unsigned short gid;
	unsigned short links_count;
	unsigned int blocks;
	unsigned int flags;
	unsigned int osd1;
	unsigned int block[15];
	unsigned int generation;
	unsigned int file_acl;
	unsigned int dir_acl;
	unsigned int faddr;
	unsigned char osd2[12];
} __attribute__ ((packed));

typedef struct ext2_inodetable ext2_inodetable_t;

/* Represents directory entry on disk. */
struct ext2_dir {
	unsigned int inode;
	unsigned short rec_len;
	unsigned char name_len;
	unsigned char file_type;
	char name;		/* Actually a set of characters, at most 255 bytes */
} __attribute__ ((packed));

typedef struct ext2_dir ext2_dir_t;

void ext2_ramdisk_mount(unsigned int offset);

#endif
