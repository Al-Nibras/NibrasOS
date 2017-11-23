#ifndef __FS_H
#define __FS_H

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_UP  ".."
#define PATH_DOT "."

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

#define	_IFMT		0170000	/* type of file */
#define		_IFDIR	0040000	/* directory */
#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */

struct fs_node;

typedef unsigned int (*read_type_t) (struct fs_node *, unsigned int, unsigned int, unsigned char *);
typedef unsigned int (*write_type_t) (struct fs_node *, unsigned int, unsigned int, unsigned char *);
typedef void (*open_type_t) (struct fs_node *, unsigned char read, unsigned char write);
typedef void (*close_type_t) (struct fs_node *);
typedef struct dirent *(*readdir_type_t) (struct fs_node *, unsigned int);
typedef struct fs_node *(*finddir_type_t) (struct fs_node *, char *name);
typedef void (*create_type_t) (struct fs_node *, char *name, unsigned short permission);
typedef void (*mkdir_type_t) (struct fs_node *, char *name, unsigned short permission);

typedef struct fs_node {
	char name[256];			// The filename.
	unsigned int mask;			// The permissions mask.
	unsigned int uid;			// The owning user.
	unsigned int gid;			// The owning group.
	unsigned int flags;			// Flags (node type, etc).
	unsigned int inode;			// Inode number.
	unsigned int length;		// Size of the file, in byte.
	unsigned int impl;			// Used to keep track which fs it belongs to.
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	create_type_t create;
	mkdir_type_t mkdir;
	struct fs_node *ptr;	// Used by mountpoints and symlinks.
	unsigned int offset;
	int shared_with;
	unsigned int atime;
	unsigned int mtime;
	unsigned int ctime;
} fs_node_t;

struct dirent {
	unsigned int ino;			// Inode number.
	char name[256];			// The filename.
};

struct stat  {
	unsigned short  st_dev;
	unsigned short  st_ino;
	unsigned int  st_mode;
	unsigned short  st_nlink;
	unsigned short  st_uid;
	unsigned short  st_gid;
	unsigned short  st_rdev;
	unsigned int  st_size;
	unsigned int  st_atime;
	unsigned int  __unused1;
	unsigned int  st_mtime;
	unsigned int  __unused2;
	unsigned int  st_ctime;
	unsigned int  __unused3;
};

extern fs_node_t *fs_root;
extern fs_node_t * null_device_create();
extern fs_node_t * serial_device_create(int device);

unsigned int read_fs(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer);
unsigned int write_fs(fs_node_t *node, unsigned int offset, unsigned int size, unsigned char *buffer);
void open_fs(fs_node_t *node, unsigned char read, unsigned char write);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, unsigned int index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);
int mkdir_fs(char *name, unsigned short permission);
int create_file_fs(char *name, unsigned short permission);
fs_node_t *kopen(char *filename, unsigned int flags);
char *canonicalize_path(char *cwd, char *input);
fs_node_t *clone_fs(fs_node_t * source);

#endif
