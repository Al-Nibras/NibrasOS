/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-08-10T00:11:42+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: pci.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T23:06:41+03:00
 */



#include <pci.h>

pci_device_t pci_devices[16];
unsigned int pci_devices_size = 0;

unsigned short pci_config_read_word(unsigned short bus, unsigned short slot, unsigned short func, unsigned char offset) {
    unsigned int address;
    unsigned int lbus = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    unsigned short tmp = 0;

    // Create config addr
    address = (unsigned int)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    // Write out PCI config address
    outportl(PCI_CONFIG_ADDR, address);

    // Read PCI config data
    // (offset & 2) * 8 = 0 will choose first word of 32bits register
    tmp = (unsigned short)((inportl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);

    return(tmp);
}

unsigned short pci_get_vendor_id(unsigned short bus, unsigned short slot, unsigned short func) {
    return pci_config_read_word(bus, slot, func, 0);
}


unsigned short pci_get_device_id(unsigned short bus, unsigned short slot, unsigned short func) {
    return pci_config_read_word(bus, slot, func, 2);
}

unsigned short pci_get_device_class_id(unsigned short bus, unsigned short slot, unsigned short func) {
    unsigned int class_id_seg = pci_config_read_word(bus, slot, func, 0xA);
    return (class_id_seg & ~0x00FF) >> 8;
}

unsigned short pci_get_device_subclass_id(unsigned short bus, unsigned short slot, unsigned short func) {
    unsigned int subclass_id_seg = pci_config_read_word(bus, slot, func, 0xA);
    return (subclass_id_seg & ~0xFF00);
}

void pci_init() {
    printf_clr(0x04,"Scanning for PCI devices...\n");
    pci_probe();
}


void pci_probe() {
    for(unsigned short bus = 0; bus < 256; bus++) {
            for (unsigned short slot = 0; slot < 32; slot++) {
                for(unsigned short function = 0; function < 8; function++) {
                    if(pci_devices_size > 15) {
                        printf("[pci] not adding above device... limit reached\n");
                        return;
                    }
                    unsigned short vendor_id = pci_get_vendor_id(bus, slot, function);
                    if(vendor_id == 0xFFFF) continue;
                    unsigned short device_id = pci_get_device_id(bus, slot, function);
                    unsigned short class_id = pci_get_device_class_id(bus, slot, function);
                    printf("[pci] %x:%x - %s\n", vendor_id, device_id, PCI_CLASS_IDS[class_id]);

                    // Add current device to local pci_devices array
                    pci_devices[pci_devices_size].vendor = vendor_id;
                    pci_devices[pci_devices_size].device = device_id;
                    pci_devices[pci_devices_size].func = function;

                    ++pci_devices_size;
                }
            }
        }
}
