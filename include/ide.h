#ifndef __IDE_H
#define __IDE_H

#ifndef __IO_H
#include <io.h>
#endif
#ifndef __TYPES_H
#include <types.h>
#endif

#define    ATA_STATUS_BSY       0x80
#define    ATA_STATUS_DRDY      0x40
#define    ATA_STATUS_DF        0x20
#define    ATA_STATUS_DSC       0x10
#define    ATA_STATUS_DRQ       0x08
#define    ATA_STATUS_CORR      0x04
#define    ATA_STATUS_IDX       0x02
#define    ATA_STATUS_ERR       0x01

#define    ATA_ERROR_BBK        0x80
#define    ATA_ERROR_UNC        0x40
#define    ATA_ERROR_MC         0x20
#define    ATA_ERROR_IDNF       0x10
#define    ATA_ERROR_MCR        0x08
#define    ATA_ERROR_ABRT       0x04
#define    ATA_ERROR_TK0NF      0x02
#define    ATA_ERROR_AMNF       0x01


// ATA-Commands
#define      ATA_CMD_READ_PIO          0x20
#define      ATA_CMD_READ_PIO_EXT      0x24
#define      ATA_CMD_READ_DMA          0xC8
#define      ATA_CMD_READ_DMA_EXT      0x25
#define      ATA_CMD_WRITE_PIO         0x30
#define      ATA_CMD_WRITE_PIO_EXT     0x34
#define      ATA_CMD_WRITE_DMA         0xCA
#define      ATA_CMD_WRITE_DMA_EXT     0x35
#define      ATA_CMD_CACHE_FLUSH       0xE7
#define      ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define      ATA_CMD_PACKET            0xA0
#define      ATA_CMD_IDENTIFY_PACKET   0xA1
#define      ATA_CMD_IDENTIFY          0xEC

#define      ATAPI_CMD_READ            0xA8
#define      ATAPI_CMD_EJECT           0x1B




//ATA_IDENTIFY_BUFFER returned value specification
#define    ATA_IDENTITY_DEVICETYPE   0
#define    ATA_IDENTITY_CYLINDERS    2
#define    ATA_IDENTITY_HEADS        6
#define    ATA_IDENTITY_SECTORS      12
#define    ATA_IDENTITY_SERIAL       20
#define    ATA_IDENTITY_MODEL        54
#define    ATA_IDENTITY_CAPABILITIES 98
#define    ATA_IDENTITY_FIELDVALID   106
#define    ATA_IDENTITY_MAX_LBA      120
#define    ATA_IDENTITY_COMMANDSETS  164
#define    ATA_IDENTITY_MAX_LBA_EXT  200

#define      ATA_MASTER     0x00
#define      ATA_SLAVE      0x01
#define      IDE_ATA         0x00
#define      IDE_ATAPI       0x01


// ATA-ATAPI Task-File:
#define      ATA_REG_DATA           0x00
#define      ATA_REG_ERROR          0x01
#define      ATA_REG_FEATURES       0x01
#define      ATA_REG_SECCOUNT0      0x02
#define      ATA_REG_LBA0           0x03
#define      ATA_REG_LBA1           0x04
#define      ATA_REG_LBA2           0x05
#define      ATA_REG_HDDEVSEL       0x06
#define      ATA_REG_COMMAND        0x07
#define      ATA_REG_STATUS         0x07
#define      ATA_REG_SECCOUNT1      0x08
#define      ATA_REG_LBA3           0x09
#define      ATA_REG_LBA4           0x0A
#define      ATA_REG_LBA5           0x0B
#define      ATA_REG_CONTROL        0x0C
#define      ATA_REG_ALTSTATUS      0x0C
#define      ATA_REG_DEVADDRESS     0x0D

// Channels
#define      ATA_PRIMARY            0x00
#define      ATA_SECONDARY          0x01

// Directions
#define      ATA_READ               0x00
#define      ATA_WRITE              0x01


struct ide_device {
   unsigned char  reserved;    // 0 (Empty) or 1 (This Drive really exists).
   unsigned char  channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
   unsigned char  drive;       // 0 (Master Drive) or 1 (Slave Drive).
   unsigned short type;        // 0: ATA, 1:ATAPI.
   unsigned short sign;       // Drive Signature
   unsigned short capabilities;// Features.
   unsigned int   commandsets; // Command Sets Supported.
   unsigned int   size;       // Size in Sectors.
   unsigned char  model[41];   // Model in string.
};


/* NOTE:
BAR0 is the Base of I/O Ports used by Primary Channel.
BAR1 is the Base of I/O Ports which control Primary Channel.
BAR2 is the Base of I/O Ports used by Secondary Channel.
BAR3 is the Base of I/O Ports which control Secondary Channel.
BAR4 is the Base of 8 I/O Ports controls Primary Channel's Bus Master IDE [BMIDE].
BAR4 + 8 is the Base of 8 I/O Ports controls Secondary Channel's Bus Master IDE [BMIDE].
*/

struct channel {
   unsigned short base;  // I/O Base.
   unsigned short ctrl;  // Control Base
   unsigned short bmide; // Bus Master IDE
   unsigned char  nIEN;  // nIEN (No Interrupt);
};

void ide_write(unsigned char channel, unsigned char reg, unsigned char data);
unsigned char ide_read(unsigned char channel, unsigned char reg);

void ide_read_identity_buffer(unsigned char channel, unsigned char reg, unsigned int buffer, unsigned int quads);

/* NOTE: When we send a command, we should wait for 400 ns, then we should read Status Port */
unsigned char ide_polling(unsigned char channel, unsigned int advanced_check);
unsigned char ide_print_error(unsigned int drive, unsigned char err);
void ide_disable_irqs();
void ide_reenable_irqs();
void ide_initialize(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3,unsigned int BAR4);
unsigned char ide_ata_access(unsigned char isreading,unsigned char drive,unsigned int lba,unsigned char numsects,unsigned short selector, unsigned int edi);
void ide_wait_irq();
void ide_irq_handler(struct regs *r);
unsigned char ide_atapi_read(unsigned char drive, unsigned int lba, unsigned char numsects,unsigned short selector, unsigned int edi);

//Standard Function For Reading from ATA/ATAPI Drive:
void ide_read_sectors(unsigned char drive, unsigned char numsects, unsigned int lba, unsigned short es, unsigned int edi, char *errors);
//Standard Function to write to ATA Drive
void ide_write_sectors(unsigned char drive, unsigned char numsects, unsigned int lba, unsigned short es, unsigned int edi,char *errors);
void ide_atapi_eject(unsigned char drive,char *errors);

#endif /* __IDE_H */
