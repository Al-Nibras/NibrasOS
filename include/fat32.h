#ifndef __FAT32_H
#define __FAT32_H

#ifndef __ATA_H
    #include <ata.h>
#endif
#ifndef __GENERIC_LIST_H
    #include <generic_tree.h>
#endif

#define little_endian 0

#if little_endian

#define MBR_MAGIC_CODE 0x55AA
#else
#define MBR_MAGIC_CODE 0xAA55
#endif

//extern void* *binaries;
//extern char* *proc_names;
struct mbr_partition_entry{
    char boot_flag;
    char chs_begin[3];              //to be read using 3-bytes integer type
    char type_code;                 // 0x0B or 0x0C for FAT32 type
    char chs_end[3];                //to be read using 3-bytes integer type
    unsigned int lba_begin;         //first LBA this partition begins at
    unsigned int number_of_sectors;
} __attribute__ ((packed));

struct mbr{
    unsigned char exec_code[446];
    struct mbr_partition_entry partition1;
    struct mbr_partition_entry partition2;
    struct mbr_partition_entry partition3;
    struct mbr_partition_entry partition4;
    unsigned short mbr_magic_code;            //for sanity check
} __attribute__ ((packed));

struct bpb{ //super-block (Volume ID)
    unsigned char  jump[3]; //jmp + nop
    unsigned char  oem_name[8];
    unsigned short bytes_per_sec;       //always 512-bytes
    unsigned char  secs_per_clus;       //1,2,4,8,16,32,64, or 128
    unsigned short rsvd_sec_count;      //usually 0x20
    unsigned char  fats_count;          //always 2
    unsigned short root_entry_count;    //usually 0x2, not available for FAT32
    unsigned short secs_count;
    unsigned char  media_type;          //0xF8 for Hard-Disks
    unsigned short secs_per_fat;        //depends on disk size, not available for FAT32
    unsigned short secs_per_track;
    unsigned short heads_count;
    unsigned int   hidden_sec_count;
    unsigned int   num_of_sec_in_partition;

    // Extended block for FAT32
    unsigned int   secs_per_fat32;      //FAT32's table size
    unsigned short flags;
    unsigned short fat32_version;       // (HighByte = Major Version, Low Byte = Minor Version)
    unsigned int   root_directory_first_clus;
    unsigned short filesystem_info_sec_num;
    unsigned short backupboot_sec_num;
    unsigned char  rsvd[12];
    unsigned char  logical_drive_num;
    unsigned char  unused;              //could be the high_byte of logical_drive_num
    unsigned char  extended_sign;       //0x29
    unsigned int   volume_id;           //Serial Number of Partition
    unsigned char  volume_lbl[11];      //Volume Name of Partition
    unsigned char  filesystem[8];       //FAT Name (FAT32)
    unsigned char  exec_code[420];
    unsigned short mbr_sign;           //always = MBR_MAGIC_CODE
} __attribute__ ((packed)); //512-bytes structure

/* NOTE:
    (unsigned)fat_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors;
    (unsigned)cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
    (unsigned char)sectors_per_cluster = BPB_SecPerClus;
    (unsigned)root_dir_first_cluster = BPB_RootClus;

    //Normally clusters are at least 4k (8 sectors), and sizes of 8k, 16k and 32k are also widely used.
    (unsigned)lba_addr = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster; //because cluster counting starts at 2
*/

struct fat_entry{
    unsigned char name[8];
    unsigned char ext[3];
    unsigned char attr;
    unsigned char rsvd;
    unsigned char create_time_ms;
    unsigned short create_time;
    unsigned short create_date;
    unsigned short access_date;

    unsigned short clus_num_high;

    unsigned short modification_time;
    unsigned short modification_date;

    unsigned short clus_num_low;
    unsigned int size;
};
typedef struct fat_entry fat32_file;


struct partition_info{
    unsigned int fat_lba;
    unsigned int fat_size; //in bytes
    unsigned int data_lba;
    unsigned int root_lba;
    unsigned int root_clus_num;
    unsigned char type_code;
};


extern void* *binaries;
extern char* *proc_names;
extern unsigned bin_cnt;

void fat32_init(unsigned short base_reg, unsigned char slave);
void fat32_read_sector(unsigned int sec, unsigned char* buf, unsigned int count);
size_t fat32_list_directory(unsigned char partition_num, unsigned int dir_lba);
int fat32_find_file(unsigned char partition_num, unsigned int dir_lba,const char* file_name,struct fat_entry* file_buf);
int file_name_cmp(const char *a,const char *b);
#endif
