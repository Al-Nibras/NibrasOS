/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-06-27T20:10:47+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: ata.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:50:11+03:00
 */

#include <ata.h>

ide_channel_regs_t ide_channels[2];
ide_device_t ide_devices[4];
unsigned char ide_buf[2048] = {0};
unsigned char ide_irq_invoked = 0;
unsigned char atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
mbr_t mbr;

void ide_detect() {
	//TODO
}

static void ata_io_wait(unsigned short bus) {
	inportb(bus + ATA_REG_ALTSTATUS);
	inportb(bus + ATA_REG_ALTSTATUS);
	inportb(bus + ATA_REG_ALTSTATUS);
	inportb(bus + ATA_REG_ALTSTATUS);
}

static int ata_wait(unsigned short bus, int advanced) {
	unsigned char status = 0;

	ata_io_wait(bus);

	while ((status = inportb(bus + ATA_REG_STATUS)) & ATA_SR_BSY);

	if (advanced) {
		status = inportb(bus + ATA_REG_STATUS);
		if (status   & ATA_SR_ERR)  return 1;
		if (status   & ATA_SR_DF)   return 1;
		if (!(status & ATA_SR_DRQ)) return 1;
	}

	return 0;
}

static void ata_select(unsigned short bus) {
	outportb(bus + ATA_REG_HDDEVSEL, 0xA0);
}

void ata_wait_ready(unsigned short bus) {
	while (inportb(bus + ATA_REG_STATUS) & ATA_SR_BSY);
}

void ide_init(unsigned short bus) {
    irq_install_handler(14,&ide_irq_handler);
    irq_install_handler(15,&ide_irq_handler);

	printf_clr(0x04, "initializing IDE device on bus 0x%x\n", bus);

	outportb(bus + 1, 1);
	outportb(bus + 0x306, 0);

	ata_select(bus);
	ata_io_wait(bus);

	outportb(bus + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	ata_io_wait(bus);

	int status = inportb(bus + ATA_REG_COMMAND);
    printf("ATA_STATUS_REG = "); printbin(status); printf("\n");

	ata_wait_ready(bus);

	ata_identify_t device;
	unsigned short * buf = (unsigned short *)&device;

	for (int i = 0; i < 256; ++i)
		buf[i] = inports(bus);


	unsigned char * ptr = (unsigned char *)&device.model;
	for (int i = 0; i < 39; i+=2) {
		unsigned char tmp = ptr[i+1];
		ptr[i+1] = ptr[i];
		ptr[i] = tmp;
	}

    printf("ata device: %s\n", device.model);

    printf("LBA48 sectors_count = %d\tsize = %d GB\n", device.sectors_48, device.sectors_48/1024/1024/2);
    printf("LBA28 sectors_count = %d\tsize = %d GB\n", device.sectors_28,device.sectors_28/1024/1024/2);

	outportb(bus + ATA_REG_CONTROL, 0x02);
}

void ide_read_sector(unsigned short bus, unsigned char slave, unsigned int lba, unsigned char * buf) {
	outportb(bus + ATA_REG_CONTROL, 0x02);

	ata_wait_ready(bus);

	outportb(bus + ATA_REG_HDDEVSEL,  0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
	outportb(bus + ATA_REG_FEATURES, 0x00);
	outportb(bus + ATA_REG_SECCOUNT0, 1);
	outportb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
	outportb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
	outportb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
	outportb(bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	if (ata_wait(bus, 1)) {
		printf_clr(0x04,"Error during ATA read\n");
	}

	int size = 256;
	inportsm(bus,buf,size);
	ata_wait(bus, 0);
}
void ide_read_sector_n(unsigned short bus, unsigned char slave, unsigned int lba, unsigned char * buf,unsigned int count) {
	outportb(bus + ATA_REG_CONTROL, 0x02);

	ata_wait_ready(bus);

	outportb(bus + ATA_REG_HDDEVSEL,  0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
	outportb(bus + ATA_REG_FEATURES, 0x00);
	outportb(bus + ATA_REG_SECCOUNT0, 1);
	outportb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
	outportb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
	outportb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
	outportb(bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	if (ata_wait(bus, 1)) {
		printf_clr(0x04,"Error during ATA read\n");
	}
	// if(count>512)
	// 	count -= (count%512);
	// if(count%2 != 0)
	// 	--count;

	if(count%2 != 0){
		unsigned short temp_data;
		inportsm(bus,buf,(count-1)/2);//devided by two becuase it's a `word by word` reading
		ata_wait(bus, 0);
		inportsm(bus,&temp_data,1);//devided by two becuase it's a `word by word` reading
		temp_data &= 0x0000ffff;
		*(unsigned char*)&buf[count-1] = (unsigned char)temp_data;
	}
	else
		inportsm(bus,buf,count/2);//devided by two becuase it's a `word by word` reading
	ata_wait(bus, 0);
}

void ide_write_sector(unsigned short bus, unsigned char slave, unsigned int lba, unsigned char * buf) {
	outportb(bus + ATA_REG_CONTROL, 0x02);

	ata_wait_ready(bus);

	outportb(bus + ATA_REG_HDDEVSEL,  0xe0 | slave << 4 |
								 (lba & 0x0f000000) >> 24);
	ata_wait(bus, 0);
	outportb(bus + ATA_REG_FEATURES, 0x00);
	outportb(bus + ATA_REG_SECCOUNT0, 0x01);
	outportb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
	outportb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
	outportb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
	outportb(bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
	ata_wait(bus, 0);
	int size = 256;
	outportsm(bus,buf,size);
	outportb(bus + 0x07, ATA_CMD_CACHE_FLUSH);
	ata_wait(bus, 0);
}
void ide_write_sector_n(unsigned short bus, unsigned char slave, unsigned int lba, unsigned char * buf,unsigned int count) {
	outportb(bus + ATA_REG_CONTROL, 0x02);

	ata_wait_ready(bus);

	outportb(bus + ATA_REG_HDDEVSEL,  0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
	ata_wait(bus, 0);
	outportb(bus + ATA_REG_FEATURES, 0x00);
	outportb(bus + ATA_REG_SECCOUNT0, 0x01);
	outportb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
	outportb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
	outportb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
	outportb(bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
	ata_wait(bus, 0);

	if(count>512)
		count -= (count%512);
	outportsm(bus,buf,count/2);
	outportb(bus + 0x07, ATA_CMD_CACHE_FLUSH);
	ata_wait(bus, 0);
}

static int ide_cmp(unsigned int * ptr1, unsigned int * ptr2, unsigned int size) {
	if(size % 4)
        size += (size%4);
	unsigned int i = 0;
	while (i < size) {
		if (*ptr1 != *ptr2) return 1;
		ptr1++;
		ptr2++;
		i += 4;
	}
	return 0;
}

void ide_write_sector_retry(unsigned short bus, unsigned char slave, unsigned int lba, unsigned char * buf) {
	unsigned char * read_buf = kmalloc(512);
	IRQ_OFF;
	do {
		ide_write_sector(bus,slave,lba,buf);
		ide_read_sector(bus,slave,lba,read_buf);
	} while (ide_cmp((unsigned int*)buf,(unsigned int*)read_buf,512));
	IRQ_RES;
}
