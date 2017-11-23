/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-05-25T17:30:53+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: fat32.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:51:52+03:00
 */



#include <fat32.h>

static unsigned short bus;
static unsigned char isslave;

static struct mbr MBR;

static struct bpb BPB1;
static struct bpb BPB2;
static struct bpb BPB3;
static struct bpb BPB4;
static struct partition_info partition1_info;
static struct partition_info partition2_info;
static struct partition_info partition3_info;
static struct partition_info partition4_info;
static struct partition_info* partitions[4] = {&partition1_info,&partition2_info,&partition3_info,&partition4_info};

static gtree_t* fat32_tree;
static gtreenode_t *current_working_dir;
static gtreenode_t *prev_working_dir;

void* *binaries = 0;
char* *proc_names = 0;
//for 8.3 FAT naming pattern
void truncate_file_name(const char* file_name,const char* ext, char* buf){
    if(!file_name || !file_name || !buf) return;
    char temp_name[12] = {0};
    //removing spaces from name:
    unsigned char i = 0;
    while(file_name[i] != ' ' && i<8) ++i;

    memcpy(temp_name,file_name,i);
    temp_name[i] = '.';
    memcpy(&temp_name[i+1],ext,3);

    strcpy(buf, temp_name);
}

void fat32_init(unsigned short base_reg,unsigned char slave){
    bus = base_reg;
    isslave = slave;
    binaries = kmalloc(64*sizeof(void*));
    proc_names = kmalloc(64*8); //8: file_name cannot exceed 8 chars in fat32
    memset(binaries,0,64*sizeof(void*));
    memset(proc_names,0,64*sizeof(void*));

    memset(&MBR,0,sizeof(struct mbr));
    memset(&BPB1,0,sizeof(struct bpb));
    memset(&BPB2,0,sizeof(struct bpb));
    memset(&BPB3,0,sizeof(struct bpb));
    memset(&BPB4,0,sizeof(struct bpb));

    memset(&partition1_info,0,sizeof(struct partition_info));
    memset(&partition2_info,0,sizeof(struct partition_info));
    memset(&partition3_info,0,sizeof(struct partition_info));
    memset(&partition4_info,0,sizeof(struct partition_info));

    ide_read_sector_n(bus,slave, 0, (unsigned char*)&MBR,sizeof(struct mbr));

    printf("printing MBR...\nMBR.MBR_MAGIC_CODE = 0x%x\nMBR.partition1 = %d\nMBR.partition2 = %d\nMBR.partition3 = %d\nMBR.partition4 = %d\n",MBR.mbr_magic_code,MBR.partition1.lba_begin,MBR.partition2.lba_begin,MBR.partition3.lba_begin,MBR.partition4.lba_begin);
    if(MBR.mbr_magic_code != MBR_MAGIC_CODE){
        printf_clr(0x04,"%s\n","ERROR: fat32_init() -> Bad FAT32 MBR Sector found,failed to initialize\n\n");
        return;
    }
    if(MBR.partition1.lba_begin){
        ide_read_sector_n(bus,slave, MBR.partition1.lba_begin, (unsigned char*)&BPB1,sizeof(struct bpb));
        if(BPB1.volume_id){
            partition1_info.fat_lba = MBR.partition1.lba_begin + BPB1.rsvd_sec_count;
            partition1_info.fat_size = BPB1.secs_per_fat32;
            partition1_info.data_lba = partition1_info.fat_lba + (partition1_info.fat_size * BPB1.fats_count);
            partition1_info.root_lba = partition1_info.data_lba + (BPB1.secs_per_clus * (BPB1.root_directory_first_clus - 2));
            partition1_info.root_clus_num = BPB1.root_directory_first_clus;
            partition1_info.type_code = MBR.partition1.type_code;
        }
        else {
            printf_clr(0x04,"%s\n","ERROR: fat32_init() -> Bad FAT32 BPB Sector found for `BPB%d`,failed to initialize\n\n",1);
            return;
        }
    }
    if(MBR.partition2.lba_begin){
        ide_read_sector_n(bus,slave, MBR.partition2.lba_begin, (unsigned char*)&BPB2,sizeof(struct bpb));
        if(BPB2.volume_id){
            partition2_info.fat_lba = MBR.partition2.lba_begin + BPB2.rsvd_sec_count;
            partition2_info.fat_size = BPB2.secs_per_fat32;
            partition2_info.data_lba = partition2_info.fat_lba + (partition2_info.fat_size * BPB2.fats_count);
            partition2_info.root_lba = partition2_info.data_lba + (BPB2.secs_per_clus * (BPB2.root_directory_first_clus - 2));
            partition2_info.root_clus_num = BPB2.root_directory_first_clus;
            partition2_info.type_code = MBR.partition2.type_code;
        }
        else {
            printf_clr(0x04,"%s\n","ERROR: fat32_init() -> Bad FAT32 BPB Sector found for `BPB%d`,failed to initialize\n\n",2);
            return;
        }
    }
    if(MBR.partition3.lba_begin){
        ide_read_sector_n(bus,slave, MBR.partition3.lba_begin, (unsigned char*)&BPB3,sizeof(struct bpb));
        if(BPB3.volume_id){
            partition3_info.fat_lba = MBR.partition3.lba_begin + BPB3.rsvd_sec_count;
            partition3_info.fat_size = BPB3.secs_per_fat32;
            partition3_info.data_lba = partition3_info.fat_lba + (partition3_info.fat_size * BPB3.fats_count);
            partition3_info.root_lba = partition3_info.data_lba + (BPB3.secs_per_clus * (BPB3.root_directory_first_clus - 2));
            partition3_info.root_clus_num = BPB3.root_directory_first_clus;
            partition3_info.type_code = MBR.partition3.type_code;
        }
        else {
            printf_clr(0x04,"%s\n","ERROR: fat32_init() -> Bad FAT32 BPB Sector found for `BPB%d`,failed to initialize\n\n",3);
            return;
        }
    }
    if(MBR.partition4.lba_begin){
        ide_read_sector_n(bus,slave, MBR.partition4.lba_begin, (unsigned char*)&BPB4,sizeof(struct bpb));
        if(BPB4.volume_id){
            partition4_info.fat_lba = MBR.partition4.lba_begin + BPB4.rsvd_sec_count;
            partition4_info.fat_size = BPB4.secs_per_fat32;
            partition4_info.data_lba = partition4_info.fat_lba + (partition4_info.fat_size * BPB4.fats_count);
            partition4_info.root_lba = partition4_info.data_lba + (BPB4.secs_per_clus * (BPB4.root_directory_first_clus - 2));
            partition4_info.root_clus_num = BPB4.root_directory_first_clus;
            partition4_info.type_code = MBR.partition4.type_code;
        }
        else {
            printf_clr(0x04,"%s\n","ERROR: fat32_init() -> Bad FAT32 BPB Sector found for `BPB%d`,failed to initialize\n\n",4);
            return;
        }
    }

    fat32_tree = tree_create();
    // char *root_path = "/";
    // tree_insert(fat32_tree,0,root_path);
    current_working_dir = prev_working_dir = fat32_tree->root;
    printf("\n\n");
}
void fat32_read_partition_info(unsigned char num){
    printf("reading partition%d...\n\n",num);
    struct mbr_partition_entry *temp_mbr_entry;
    switch(num){
        case 1: temp_mbr_entry = &MBR.partition1;break;
        case 2: temp_mbr_entry = &MBR.partition2;break;
        case 3: temp_mbr_entry = &MBR.partition3;break;
        default: temp_mbr_entry = &MBR.partition4;break;
    }
    printf(".boot_flag=0x%x\n.chs_begin=0x%x%x%x\n.chs_end=0x%x%x%x\n.lba_begin=%d\n.number_of_sectors=%d\n.type_code = 0x%x\n\n",temp_mbr_entry->boot_flag,temp_mbr_entry->chs_begin[0],temp_mbr_entry->chs_begin[1],temp_mbr_entry->chs_begin[2],temp_mbr_entry->chs_end[0],temp_mbr_entry->chs_end[1],temp_mbr_entry->chs_end[2],temp_mbr_entry->lba_begin,temp_mbr_entry->number_of_sectors,temp_mbr_entry->type_code);
}
void fat32_read_bpb(unsigned char bpb_num){
    struct bpb *temp_bpb;
    switch(bpb_num){
        case 1: temp_bpb = &BPB1; break;
        case 2: temp_bpb = &BPB2; break;
        case 3: temp_bpb = &BPB3; break;
        default: temp_bpb = &BPB4; break;
    }
    if(!temp_bpb->volume_id){
        printf("ERROR: fat32_read_bpb() -> No matched partition found for `BPB%d`\n\n",bpb_num);
        return;
    }
    printf("reading BPB%d...\n\n",bpb_num);
    for(int i = 0 ; i<sizeof(struct bpb); ++i){
        printf("%x ", ((unsigned char*)temp_bpb)[i]);
    }
    printf("\b\n\n");
}
void fat32_list_root_directory(unsigned char partition_num){
    printf_clr(0x0C,"Root Directory: \n");
    fat32_list_directory(partition_num, partitions[partition_num-1]->root_lba);
}
void fat32_read_file(unsigned char partition_num, struct fat_entry* file){
    printf_clr(0x0C,"\nReading file...\n\n");

    struct bpb* temp_bpb;
    switch(partition_num){
        case 1: temp_bpb = &BPB1; break;
        case 2: temp_bpb = &BPB2; break;
        case 3: temp_bpb = &BPB3; break;
        default: temp_bpb = &BPB4; break;
    }

    unsigned int file_clus = (file->clus_num_high << 16) | file->clus_num_low;
    unsigned int file_next_clus = file_clus;
    if(!file_clus)
        file_clus=2;
    else if(file_clus==1)
        file_clus++;

    unsigned int file_sec = partitions[partition_num-1]->data_lba+temp_bpb->secs_per_clus * (file_clus-2);

    char *buf;
    if(file->size>=512){
        size_t buf_size = file->size+1;
        size_t temp_size = file->size;
        while (temp_size%512 > 0 && temp_size%512 != temp_size) {
            buf_size += 512;
            temp_size-=512;
        }
        buf = kmalloc(buf_size);
        buf[buf_size-1] = 0;
    }
    else{
        buf = kmalloc(513);
        buf[512] = 0;
    }
    unsigned int fat_buf[512];

    unsigned int sec_counter = file_sec % temp_bpb->secs_per_clus;
    for(size_t s = 0; s < file->size; s+=512, sec_counter++){
        if( sec_counter >= temp_bpb->secs_per_clus){ //exceeded cluster limit
            unsigned int clus_offset_in_fat = file_next_clus / (512 / sizeof(unsigned int));
            unsigned int sec_offset_in_clus = file_next_clus % (512 / sizeof(unsigned int));

            fat32_read_sector(partitions[partition_num-1]->fat_lba+clus_offset_in_fat,(unsigned char*)&fat_buf[0],512);
            file_next_clus = fat_buf[sec_offset_in_clus] & 0x0fffffff; //last 4-bits are cleared in FAT32 cluster addressing

            if(file_next_clus >= 0xfffffff8)
                break; //EOF
            file_sec = partitions[partition_num-1]->data_lba+temp_bpb->secs_per_clus * (file_next_clus-2);
            sec_counter = 0;
        }
        fat32_read_sector(file_sec+sec_counter,(unsigned char*)&buf[s],512);
        buf[s+512] = 0;
        printf("%s",&buf[s]);
    }
    printf("\n");
    kfree(buf);
}
void fat32_read_sector(unsigned int sec, unsigned char* buf, unsigned int count){
    // if(!buf || !count) return;
    // if(count>512){
    //     unsigned int remainder = count%512;
    //     count -= remainder;
    //     unsigned int i;
    //     for( i = 0; i<count; i+=512 )
    //         ide_read_sector(bus,slave, sec, &buf[i]);
    //     if(remainder)
    //         ide_read_sector_n(bus,slave, sec, &buf[i],remainder);
    // }
    // else
    if(count == 512)
        ide_read_sector(bus,isslave, sec, buf);
    else
        ide_read_sector_n(bus,isslave, sec, buf,count);

}
void fat32_travseraling_fat(char partition_num,char fat_num, unsigned int entry){
    unsigned int fat[128];
    unsigned int lba;
    if(fat_num >= 2){
        lba = partitions[partition_num-1]->fat_lba+partitions[partition_num-1]->fat_size;
        printf("Travseraling FAT%d at LBA: %d\n\n",2,lba);
        ide_read_sector_n(bus,isslave, lba, fat,128);
    }
    else {
        lba = partitions[partition_num-1]->fat_lba;
        printf("Travseraling FAT%d at LBA: %d\n\n",1,lba);
        ide_read_sector_n(bus,isslave, lba, fat,128);
    }
    // for(int i = 0;i<128;++i)
    //     printf("0x%x ",fat[i]);
    // printf("\b\n\n");
    unsigned int next_clus = fat[entry];
    printf("Entry = %d\tNext Cluster is: 0x%x\n\n",entry,next_clus);
    while(!next_clus){
        printf("Entry = %d\tNext Cluster is: 0x%x\n\n",next_clus,fat[next_clus]);
        next_clus = fat[next_clus];
    }

}
static char bin_flag = 0;
unsigned bin_cnt = 0;
size_t fat32_list_directory(unsigned char partition_num, unsigned int dir_lba){
    struct fat_entry entries[512];
    size_t dir_size = 0;
    unsigned int files_count = 0;
    unsigned int subdirs_count = 0;
    memset(entries,0,sizeof(struct fat_entry)*512);
    ide_read_sector(bus,isslave, dir_lba, (unsigned char*)&entries[0]);

    for(int i = 0; i<512; ++i){
        if(entries[i].name[0] == 0x00) //if first byte is NULL, we reached out End Of Directory or EOF
            break;
        else if(!strcmp(entries[i].name, ".") || !strcmp(entries[i].name, "..")) //current & parent directories
            continue;
        else if(entries[i].name[0] == 0xe5) //no longer used directory record (deleted)
            continue;
        else if(entries[i].attr & (1<<1) || entries[i].attr & (1<<2)) // hidden or system record, should not be listed
            continue;
        else if(entries[i].attr & 0x0f) //Long-Name FAT32 type (MS's Private License)
            continue;
        //else if(entries[i].attr & (1<<5)) //archive
            //continue;
        else if(entries[i].attr & (1<<4)){ // record is directory/subdirectory
            struct bpb* temp_bpb;
            switch(partition_num){
                case 1: temp_bpb = &BPB1; break;
                case 2: temp_bpb = &BPB2; break;
                case 3: temp_bpb = &BPB3; break;
                default: temp_bpb = &BPB4; break;
            }
            unsigned int dir_clus = (entries[i].clus_num_high << 16) | entries[i].clus_num_low;
            unsigned int dir_sec = partitions[partition_num-1]->data_lba+temp_bpb->secs_per_clus * (dir_clus-2);
            //gtreenode_t* temp = tree_insert(fat32_tree,current_working_dir,entries[i].name);
            //prev_working_dir = current_working_dir;
            //current_working_dir = temp;
            printf("\nlisting directory: %s ...\n",entries[i].name);
            if(!strcmp(entries[i].name,"BIN")){
                //loading binaries
                bin_flag = 1;
            }
            else
                bin_flag = 0;
            size_t size = fat32_list_directory(partition_num, dir_sec);
            dir_size+=size;
            ++subdirs_count;
            //current_working_dir = prev_working_dir;
            continue;
        }
        if(!bin_flag){
            char temp_name[12];
            truncate_file_name(entries[i].name,entries[i].ext,temp_name);
            printf("%s: %s\tSize: %d bytes\n",(entries[i].attr & (1<<4)) ? "Directory" : "File",temp_name,entries[i].size);
            if(entries[i].size){
                dir_size+=entries[i].size;
                fat32_read_file(partition_num, &entries[i]);
            }
        }
        else {
            binaries[bin_cnt] = kmalloc(entries[i].size);
            proc_names[bin_cnt] = kmalloc(sizeof(entries[i].name));
            memcpy(proc_names[bin_cnt],entries[i].name,8);
            fat32_load_file_into_buf(1,&entries[i],binaries[bin_cnt]);
            ++bin_cnt;
        }
        //tree_insert(fat32_tree,current_working_dir,temp_name);
        ++files_count;
    }
    printf("Files Count: %d\tFolders Count: %d\tTotal Size: %d bytes\n",files_count,subdirs_count,dir_size);
    return dir_size;
}

void fat32_print_logical_tree(){
    char* array[1024];
    printf("printing VFS ...\n\n");
    tree_print_recur(fat32_tree,array,1024);
}
void fat32_load_file_into_buf(unsigned char partition_num,struct fat_entry* file,void *buf){
    struct bpb* temp_bpb;
    switch(partition_num){
        case 1: temp_bpb = &BPB1; break;
        case 2: temp_bpb = &BPB2; break;
        case 3: temp_bpb = &BPB3; break;
        default: temp_bpb = &BPB4; break;
    }

    unsigned int file_clus = (file->clus_num_high << 16) | file->clus_num_low;
    unsigned int file_next_clus = file_clus;
    if(!file_clus)
        file_clus=2;
    else if(file_clus==1)
        file_clus++;

    unsigned int file_sec = partitions[partition_num-1]->data_lba+temp_bpb->secs_per_clus * (file_clus-2);
    unsigned int fat_buf[512];

    unsigned int sec_counter = file_sec % temp_bpb->secs_per_clus;
    size_t s;
    for(s = 0; s < file->size; s+=512, sec_counter++){
        if( sec_counter >= temp_bpb->secs_per_clus){ //exceeded cluster limit
            unsigned int clus_offset_in_fat = file_next_clus / (512 / sizeof(unsigned int));
            unsigned int sec_offset_in_clus = file_next_clus % (512 / sizeof(unsigned int));

            fat32_read_sector(partitions[partition_num-1]->fat_lba+clus_offset_in_fat,(unsigned char*)&fat_buf[0],512);
            file_next_clus = fat_buf[sec_offset_in_clus] & 0x0fffffff; //last 4-bits are cleared in FAT32 cluster addressing

            if(file_next_clus >= 0xfffffff8)
                break; //EOF
            file_sec = partitions[partition_num-1]->data_lba+temp_bpb->secs_per_clus * (file_next_clus-2);
            sec_counter = 0;
        }
        if(file->size-s < 512){
            char temp_buf[512];
            fat32_read_sector(file_sec+sec_counter,temp_buf,512);
            memcpy(&buf[s],temp_buf,file->size-s);
            break;
        }
        else
            fat32_read_sector(file_sec+sec_counter,(unsigned char*)&buf[s],512);
    }
}
void print_binaries(){
    for(int i = 0; i<512;++i){
        if(!binaries[i]) break;
        printf("process%d: 0x%x\nname: %s\n\n",i,binaries[i],proc_names[i]);
    }
}
void hex_dump(unsigned short proc_idx){
    for(size_t i = 0; i<916; ++i){
        unsigned char byte = ((unsigned char*)binaries[proc_idx])[i];
        printf("0x%x ",byte);
    }
    printf("\n");
}
