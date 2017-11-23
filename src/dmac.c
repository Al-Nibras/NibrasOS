//  NibrasOS
//  dmac.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-07-22T17:18:45+03:00.
//	Last modification time on 2017-07-29T23:54:22+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//


//ISA DMAC Generic Registers
#include <dmac.h>


/* NOTE: DMAC deals only with physical addresses, if paging is enabled, we need to `identity-map` the address */
void dmac_channel_set_address(unsigned char channel, unsigned short base){
  if(channel > 8) return;
  unsigned char port = 0;
  switch(channel){
    case 0: port = DMAC_SLAVE_CH0_ADDR; break;
    case 1: port = DMAC_SLAVE_CH1_ADDR; break;
    case 2: port = DMAC_SLAVE_CH2_ADDR; break;
    case 3: port = DMAC_SLAVE_CH3_ADDR; break;

    case 4: port = DMAC_MASTER_CH4_ADDR; break;
    case 5: port = DMAC_MASTER_CH5_ADDR; break;
    case 6: port = DMAC_MASTER_CH6_ADDR; break;
    case 7: port = DMAC_MASTER_CH7_ADDR; break;

    default: break; //impossible case
  }

  outportb(port,base & 0xff); //base_low
  outportb(port,(base >> 8) & 0xff); //base_high
}

/* NOTE: since ISA x86 has two DMACs, and each one has 16-bits registers, `count` may not exceed 64K at a time.
         if we want to read/write more than 64KB (up to 16MB at a time), we can use `Extended Page Address registers` */
void dmac_channel_set_count(unsigned char channel, unsigned short count){
  if(channel >= 8) return;
  unsigned char port = 0;
  switch(channel){
    case 0: port = DMAC_SLAVE_CH0_CNTR; break;
    case 1: port = DMAC_SLAVE_CH1_CNTR; break;
    case 2: port = DMAC_SLAVE_CH2_CNTR; break;
    case 3: port = DMAC_SLAVE_CH3_CNTR; break;

    case 4: port = DMAC_MASTER_CH4_CNTR; break;
    case 5: port = DMAC_MASTER_CH5_CNTR; break;
    case 6: port = DMAC_MASTER_CH6_CNTR; break;
    case 7: port = DMAC_MASTER_CH7_CNTR; break;

    default: return;
  }

  outportb(port,count & 0xff); //base_low
  outportb(port,(count >> 8) & 0xff); //base_high
}
void dmac_extended_page_addr_set(unsigned char reg, unsigned char extra_page_seg){
  unsigned char port = 0;
  switch(reg){
    case 1: port = DMAC_EXTENDED_PAGE_CH1_ADDR_BYTE; break;
    case 2: port = DMAC_EXTENDED_PAGE_CH2_ADDR_BYTE; break;
    case 3: port = DMAC_EXTENDED_PAGE_CH3_ADDR_BYTE; break;

    case 5: port = DMAC_EXTENDED_PAGE_CH5_ADDR_BYTE; break;
    case 6: port = DMAC_EXTENDED_PAGE_CH6_ADDR_BYTE; break;
    case 7: port = DMAC_EXTENDED_PAGE_CH7_ADDR_BYTE; break;

    case 4: //may NOT be writing to channel 4, its a cascade between master DMAC and slave DMAC
    default: return;
  }
  outportb(port,extra_page_seg); //adds one extra byte to port's base register to extend 64KB limit
}
void dmac_mask_channel(char set, unsigned char channel){
    if(channel >= 8) return;
    char dmac = (channel < 4) ? 0 : 1;
    if(channel >= 4) channel -= 4;

    char channel_mask = 0;
    switch(channel){
      case 0: channel_mask = 0b00; break;
      case 1: channel_mask = 0b01; break;
      case 2: channel_mask = 0b10; break;
      case 3: channel_mask = 0b11; break; /* NOTE: be careful on masking Slave DMAC's channel 4, will mask all cascaded channels! */
      default: return;
    }
    if(!dmac)
      outportb(DMAC_SLAVE_SINGLE_MASK_REG, ( (set) ? 0x4|channel_mask : channel_mask) ); /* 0x4: 0b100 | channel_mask, bit3 sets or clears channel_mask */
    else
     outportb(DMAC_MASTER_SINGLE_MASK_REG, ( (set) ? 0x4|channel_mask : channel_mask) ); /* 0x4: 0b100 | channel_mask, bit3 sets or clears channel_mask */
}
void dmac_set_mode(unsigned char channel, unsigned char mode) {
  if(channel >= 8) return;
  char dmac = (channel < 4) ? 0 : 1;
  if(channel >= 4) channel -= 4;
	dmac_mask_channel(0,channel); //mask off channel
	outportb( !(dmac) ? (DMAC_SLAVE_MODE_REG) : DMAC_MASTER_MODE_REG, channel|mode);
  dmac_mask_channel(1,channel); //mask on channel
}
void dmac_set_read_mode(unsigned char channel) {
	dmac_set_mode(channel, DMAC_MODE_READ_TRANSFER | DMAC_MODE_TRANSFER_SINGLE | DMAC_MODE_MASK_AUTO);
}
void dmac_set_write_mode(unsigned char channel) {
	dmac_set_mode(channel, DMAC_MODE_WRITE_TRANSFER | DMAC_MODE_TRANSFER_SINGLE | DMAC_MODE_MASK_AUTO);
}


void dmac_reset(char dmac){
  if(dmac < 0 || dmac > 1) return;
  if(!dmac)
    outportb(0x0d,0x00);  // the value bieng written to the register can be anything, becuase it's ignored
  else
    outportb(0xd8,0x00);
}
void dmac_reset_flipflop(char dmac){
  if(dmac < 0 || dmac > 1) return;
  if(!dmac)
    outportb(DMAC_SLAVE_CLR_BYTE_PTR_FF,0x00);  // the value bieng written to the register can be anything, becuase it's ignored
  else
    outportb(DMAC_MASTER_CLR_BYTE_PTR_FF,0x00);
}
void dmac_clear_mask_register(char dmac){
  if(dmac < 0 || dmac > 1) return;
  if(!dmac)
    outportb(DMAC_SLAVE_CLR_MASK_REG,0x00);  // the value bieng written to the register can be anything, becuase it's ignored
  else
    outportb(DMAC_MASTER_CLR_MASK_REG,0x00);
}
