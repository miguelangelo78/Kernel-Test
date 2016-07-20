/*
 * pci.h
 *
 *  Created on: 20/07/2016
 *      Author: Miguel
 */

#ifndef SRC_MODULES_PCI_PCI_H_
#define SRC_MODULES_PCI_PCI_H_

#define PCI_VENDOR_ID            0x00 // 2
#define PCI_DEVICE_ID            0x02 // 2
#define PCI_COMMAND              0x04 // 2
#define PCI_STATUS               0x06 // 2
#define PCI_REVISION_ID          0x08 // 1

#define PCI_PROG_IF              0x09 // 1
#define PCI_SUBCLASS             0x0a // 1
#define PCI_CLASS                0x0b // 1
#define PCI_CACHE_LINE_SIZE      0x0c // 1
#define PCI_LATENCY_TIMER        0x0d // 1
#define PCI_HEADER_TYPE          0x0e // 1
#define PCI_BIST                 0x0f // 1
#define PCI_BAR0                 0x10 // 4
#define PCI_BAR1                 0x14 // 4
#define PCI_BAR2                 0x18 // 4
#define PCI_BAR3                 0x1C // 4
#define PCI_BAR4                 0x20 // 4
#define PCI_BAR5                 0x24 // 4

#define PCI_INTERRUPT_LINE       0x3C // 1

#define PCI_SECONDARY_BUS        0x09 // 1

#define PCI_HEADER_TYPE_DEVICE  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_SATA   0x0106

#define PCI_ADDRESS_PORT 0xCF8
#define PCI_VALUE_PORT   0xCFC

#define PCI_NONE 0xFFFF

typedef void (*pci_hitfunc_t)(uint32_t device, uint16_t vendor_id, uint16_t device_id, void * extra);

static inline int pci_extract_bus(uint32_t device) {
	return (uint8_t)(device >> 16);
}

static inline int pci_extract_slot(uint32_t device) {
	return (uint8_t)(device >> 8);
}

static inline int pci_extract_func(uint32_t device) {
	return (uint8_t)device;
}

static inline uint32_t pci_get_addr(uint32_t device, int field) {
	return 0x80000000 | (pci_extract_bus(device) << 16) | (pci_extract_slot(device) << 11) | (pci_extract_func(device) << 8) | ((field) & 0xFC);
}

static inline uint32_t pci_box_device(int bus, int slot, int func) {
	return (uint32_t)((bus << 16) | (slot << 8) | func);
}

/******************************/
/**** PCI IOCTL Functions: ****/
/******************************/

inline uint32_t pci_read_field(uint32_t device, int field, int size) {
	uint32_t ret;
	MOD_IOCTLD("pci_driver", ret, 0, (uintptr_t)device, (uintptr_t)field, (uintptr_t)size);
	return ret;
}

inline void pci_write_field(uint32_t device, int field, int size, uint32_t value) {
	MOD_IOCTL("pci_driver", 1, (uintptr_t)device, (uintptr_t)field, (uintptr_t)size, (uintptr_t)value);
}

inline uint16_t pci_find_type(uint32_t dev) {
	uint16_t ret;
	MOD_IOCTLDT("pci_driver", uint16_t, ret, 2, (uintptr_t)dev);
	return ret;
}

inline const char * pci_vendor_lookup(unsigned short vendor_id) {
	char* ret;
	MOD_IOCTLDT("pci_driver", char*, ret, 3, (uintptr_t)vendor_id);
	return (const char*)ret;
}

inline const char * pci_device_lookup(unsigned short vendor_id, unsigned short device_id) {
	char* ret;
	MOD_IOCTLDT("pci_driver", char*, ret, 4, (uintptr_t)vendor_id, (uintptr_t)device_id);
	return (const char*)ret;
}

inline void pci_scan_hit(pci_hitfunc_t f, uint32_t dev, void * extra) {
	MOD_IOCTL("pci_driver", 5, (uintptr_t)f, (uintptr_t)dev, (uintptr_t)extra);
}

inline void pci_scan_func(pci_hitfunc_t f, int type, int bus, int slot, int func, void * extra) {
	MOD_IOCTL("pci_driver", 6, (uintptr_t)f, (uintptr_t)type, (uintptr_t)bus, (uintptr_t)slot, (uintptr_t)func, (uintptr_t)extra);
}

inline void pci_scan_slot(pci_hitfunc_t f, int type, int bus, int slot, void * extra) {
	MOD_IOCTL("pci_driver", 7, (uintptr_t)f, (uintptr_t)type, (uintptr_t)bus, (uintptr_t)slot, (uintptr_t)extra);
}

inline void pci_scan_bus(pci_hitfunc_t f, int type, int bus, void * extra) {
	MOD_IOCTL("pci_driver", 8, (uintptr_t)f, (uintptr_t)type, (uintptr_t)bus, (uintptr_t)extra);
}

inline void pci_scan(pci_hitfunc_t f, int type, void * extra) {
	MOD_IOCTL("pci_driver", 8, (uintptr_t)f, (uintptr_t)type, (uintptr_t)extra);
}

#endif /* SRC_MODULES_PCI_PCI_H_ */
