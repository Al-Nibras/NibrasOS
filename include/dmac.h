//ISA DMAC Generic Registers
#ifndef __DMAC_H
#define __DMAC_H

#define DMAC_SLAVE_STATUS_REG           0x08
#define DMAC_SLAVE_CMD_REG              0x08
#define DMAC_SLAVE_REQ_REG              0x09
#define DMAC_SLAVE_SINGLE_MASK_REG      0x0a
#define DMAC_SLAVE_MODE_REG             0x0b
#define DMAC_SLAVE_CLR_BYTE_PTR_FF      0x0c
#define DMAC_SLAVE_INTM_REG             0x0d
#define DMAC_SLAVE_MASTER_CLR           0x0d
#define DMAC_SLAVE_CLR_MASK_REG         0x0e
#define DMAC_SLAVE_WRITE_MASK_REG       0x0f

#define DMAC_MASTER_STATUS_REG          0xd0
#define DMAC_MASTER_CMD_REG             0xd0
#define DMAC_MASTER_REQ_REG             0xd2
#define DMAC_MASTER_SINGLE_MASK_REG     0xd4
#define DMAC_MASTER_MODE_REG            0xd6
#define DMAC_MASTER_CLR_BYTE_PTR_FF     0xd8
#define DMAC_MASTER_INTM_REG            0xda
#define DMAC_MASTER_MASTER_CLR          0xda
#define DMAC_MASTER_CLR_MASK_REG        0xdc
#define DMAC_MASTER_WRITE_MASK_REG      0xde


//ISA DMAC Channel Registers
#define DMAC_SLAVE_CH0_ADDR             0x00
#define DMAC_SLAVE_CH0_CNTR             0x01
#define DMAC_SLAVE_CH1_ADDR             0x02
#define DMAC_SLAVE_CH1_CNTR             0x03
#define DMAC_SLAVE_CH2_ADDR             0x04
#define DMAC_SLAVE_CH2_CNTR             0x05
#define DMAC_SLAVE_CH3_ADDR             0x06
#define DMAC_SLAVE_CH3_CNTR             0x07

#define DMAC_MASTER_CH4_ADDR            0xc0
#define DMAC_MASTER_CH4_CNTR            0xc2
#define DMAC_MASTER_CH5_ADDR            0xc4
#define DMAC_MASTER_CH5_CNTR            0xc6
#define DMAC_MASTER_CH6_ADDR            0xc8
#define DMAC_MASTER_CH6_CNTR            0xca
#define DMAC_MASTER_CH7_ADDR            0xcc
#define DMAC_MASTER_CH7_CNTR            0xce


//specific to AT DMAC
#define DMAC_EXTENDED_PAGE_EXTRA0         0x80
#define	DMAC_EXTENDED_PAGE_CH2_ADDR_BYTE  0x81
#define	DMAC_EXTENDED_PAGE_CH3_ADDR_BYTE  0x82
#define	DMAC_EXTENDED_PAGE_CH1_ADDR_BYTE  0x83
#define	DMAC_EXTENDED_PAGE_EXTRA1         0x84
#define	DMAC_EXTENDED_PAGE_EXTRA2         0x85
#define	DMAC_EXTENDED_PAGE_EXTRA3         0x86
#define	DMAC_EXTENDED_PAGE_CH6_ADDR_BYTE  0x87
#define	DMAC_EXTENDED_PAGE_CH7_ADDR_BYTE  0x88
#define	DMAC_EXTENDED_PAGE_CH5_ADDR_BYTE  0x89
#define	DMAC_EXTENDED_PAGE_EXTRA4         0x8c
#define	DMAC_EXTENDED_PAGE_EXTRA5         0x8d
#define	DMAC_EXTENDED_PAGE_EXTRA6         0x8e
#define	DMAC_EXTENDED_PAGE_DRAM_REFRESH   0x8f

extern void outportb(unsigned char port, unsigned char byte); //defined in kmain.c
extern char inportb(unsigned char port); //defined in kmain.c

/* NOTE: DMAC deals only with physical addresses, if paging is enabled, we need to `identity-map` the address */
void dmac_channel_set_address(unsigned char channel, unsigned short base);

/* NOTE: since ISA x86 has two DMACs, and each one has 16-bits registers, `count` may not exceed 64K at a time.
         if we want to read/write more than 64KB (up to 16MB at a time), we can use `Extended Page Address registers` */
void dmac_channel_set_count(unsigned char channel, unsigned short count);
void dmac_extended_page_addr_set(unsigned char reg, unsigned char extra_page_seg);
void dmac_mask_channel(char set, unsigned char channel);

//DMAC MODES
#define DMAC_MODE_SELF_TEST            0x00
#define DMAC_MODE_TRANSFER_ON_DEMAND   0x00
#define DMAC_MODE_MASK_SEL             0x03
#define DMAC_MODE_READ_TRANSFER        0x04
#define DMAC_MODE_WRITE_TRANSFER       0x08
#define DMAC_MODE_MASK_TRA             0x0c
#define DMAC_MODE_MASK_AUTO            0x10
#define DMAC_MODE_MASK_IDEC            0x20
#define DMAC_MODE_TRANSFER_SINGLE      0x40
#define DMAC_MODE_TRANSFER_BLOCK       0x80
#define DMAC_MODE_MASK                 0xc0


void dmac_set_mode(unsigned char channel, unsigned char mode);
void dmac_set_read_mode(unsigned char channel);
void dmac_set_write_mode(unsigned char channel);


void dmac_reset(char dmac);
void dmac_reset_flipflop(char dmac);
void dmac_clear_mask_register(char dmac);


#endif /* __DMAC_H */
