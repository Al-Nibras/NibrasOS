#ifndef __PCI_H
#define __PCI_H

#ifndef __IO_H
#include <io.h>
#endif

#define PCI_CONFIG_ADDR     0xCF8
#define PCI_CONFIG_DATA     0xCFC

typedef struct pci_device {
    unsigned int vendor;
    unsigned int device;
    unsigned int func;
} pci_device_t;

const char* PCI_CLASS_IDS[18] =
{
    "no class specification",
    "Mass Storage Controller",
    "Network Controller",
    "Display Controller",
    "Multimedia Device",
    "Memory Controller",
    "Bridge Device",
    "Simple Communication Controller",
    "Base System Peripheral",
    "Input Device",
    "Docking Station",
    "Processor",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent I/O Controller",
    "Satellite Communication Controller",
    "Encryption/Decryption Controller",
    "Data Acquisition and Signal Processing Controller"
};

/**
* Read word at offset from pci device at bus `bus`, device `slot`, and function `func` (for multifunc device)
*/
unsigned short pci_config_read_word(unsigned short bus, unsigned short slot, unsigned short func, unsigned char offset);

/**
* Get vendor ID of PCI device at bus `bus`, device `slot`
*/
unsigned short pci_get_vendor_id(unsigned short bus, unsigned short slot, unsigned short func);

/**
* Get device ID of PCI device at bus `bus`, device `slot`
*/
unsigned short pci_get_device_id(unsigned short bus, unsigned short slot, unsigned short func);
/**
* Get class ID of pci device device on bus `bus`
*/
unsigned short pci_get_device_class_id(unsigned short bus, unsigned short slot, unsigned short func);

/**
* Get subclassclass ID of pci device device on bus `bus`
*/
unsigned short pci_get_device_subclass_id(unsigned short bus, unsigned short slot, unsigned short func);

/**
* Initialize PCI device array and such
*/
void pci_init();

/**
* Loop through PCI devices and print out information for the first 16 devices
*/
void pci_probe();

#endif
