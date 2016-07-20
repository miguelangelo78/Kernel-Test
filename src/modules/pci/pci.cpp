/*
 * pci.cpp
 *
 *  Created on: 20/07/2016
 *      Author: Miguel
 */

#include <system.h>
#include <kernel_headers/kheaders.h>
#include <module.h>
#include <stdint.h>
#include "pci.h"
#include "pci_list.h"

$LLVMENABLE(0);

/**     ****     *****            **/
/** PCI Read and Write functions: **/
/**     ****     *****            **/
static uint32_t pci_read_field_(uint32_t device, int field, int size) {
	outl(PCI_ADDRESS_PORT, pci_get_addr(device, field));

	if (size == 4) {
		uint32_t t = inl(PCI_VALUE_PORT);
		return t;
	} else if (size == 2) {
		uint16_t t = inl(PCI_VALUE_PORT + (field & 2));
		return t;
	} else if (size == 1) {
		uint8_t t = inl(PCI_VALUE_PORT + (field & 3));
		return t;
	}
	return 0xFFFF;
}

static void pci_write_field_(uint32_t device, int field, int size, uint32_t value) {
	outl(PCI_ADDRESS_PORT, pci_get_addr(device, field));
	outl(PCI_VALUE_PORT, value);
}
/**********************************/

/***************************************/
/**** PCI Implementation Functions: ****/
/***************************************/

static uint16_t pci_find_type_(uint32_t dev) {
	return (pci_read_field(dev, PCI_CLASS, 1) << 8) | pci_read_field(dev, PCI_SUBCLASS, 1);
}

/* Actually execute the function callback: */
static void pci_scan_hit_(pci_hitfunc_t f, uint32_t dev, void * extra) {
	int dev_vend = (int)pci_read_field(dev, PCI_VENDOR_ID, 2);
	int dev_dvid = (int)pci_read_field(dev, PCI_DEVICE_ID, 2);

	f(dev, dev_vend, dev_dvid, extra);
}

/* Execute callback IF there's a scan hit: */
static void pci_scan_func_(pci_hitfunc_t f, int type, int bus, int slot, int func, void * extra) {
	uint32_t dev = pci_box_device(bus, slot, func);
	if (type == -1 || type == pci_find_type(dev))
		pci_scan_hit(f, dev, extra);

	if (pci_find_type(dev) == PCI_TYPE_BRIDGE)
		pci_scan_bus(f, type, pci_read_field(dev, PCI_SECONDARY_BUS, 1), extra);
}

/* Scan a specific slot: */
static void pci_scan_slot_(pci_hitfunc_t f, int type, int bus, int slot, void * extra) {
	uint32_t dev = pci_box_device(bus, slot, 0);
	if (pci_read_field(dev, PCI_VENDOR_ID, 2) == PCI_NONE)
		return;

	pci_scan_func(f, type, bus, slot, 0, extra);
	if (!pci_read_field(dev, PCI_HEADER_TYPE, 1))
		return;

	for (int func = 1; func < 8; func++) {
		uint32_t dev = pci_box_device(bus, slot, func);
		if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE)
			pci_scan_func(f, type, bus, slot, func, extra);
	}
}

/* Scan a specific bus: */
static void pci_scan_bus_(pci_hitfunc_t f, int type, int bus, void * extra) {
	for(int slot = 0; slot < 32; slot++)
		pci_scan_slot(f, type, bus, slot, extra);
}

/* Scan everything: */
static void pci_scan_(pci_hitfunc_t f, int type, void * extra) {
	pci_scan_bus(f, type, 0, extra);

	if(!pci_read_field(0, PCI_HEADER_TYPE, 1))
		return;

	for(int func = 1; func < 8; func++) {
		uint32_t dev = pci_box_device(0, 0, func);
		if(pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE)
			pci_scan_bus(f, type, func, extra);
		else
			break;
	}
}
/***************************************/


/************************************/
/**** PCI List Lookup Functions: ****/
/************************************/
static const char * pci_vendor_lookup_(unsigned short vendor_id) {
	for (unsigned int i = 0; i < PCI_VENTABLE_LEN; ++i)
		if (PciVenTable[i].VenId == vendor_id)
			return PciVenTable[i].VenFull;
	return "";
}

static const char * pci_device_lookup_(unsigned short vendor_id, unsigned short device_id) {
	for (unsigned int i = 0; i < PCI_DEVTABLE_LEN; ++i)
		if (PciDevTable[i].VenId == vendor_id && PciDevTable[i].DevId == device_id)
			return PciDevTable[i].ChipDesc;
	return "";
}
/***************************************/

/*******************************/
/**** PCI Module Functions: ****/
/*******************************/
static int pci_mod_init(void) {
	return 0;
}

static int pci_mod_finit(void) {
	return 0;
}

static uintptr_t pci_ioctl(void * ioctl_packet) {
	uintptr_t * d = (uintptr_t*)ioctl_packet;
	SWITCH_IOCTL(ioctl_packet) {
		case 0: return (uintptr_t)pci_read_field_((uint32_t)d[1],(int)d[2],(int)d[3]);
		case 1: pci_write_field_((uint32_t)d[1],(int)d[2],(int)d[3], (uint32_t)d[4]); return 0;
		case 2: return (uintptr_t)pci_find_type_((uint32_t)d[1]);
		case 3: return (uintptr_t)pci_vendor_lookup_((unsigned short)d[1]);
		case 4: return (uintptr_t)pci_device_lookup_((unsigned short)d[1],(unsigned short)d[2]);
		case 5: pci_scan_hit_((pci_hitfunc_t)d[1],(uint32_t)d[2],(void*)d[3]); return 0;
		case 6: pci_scan_func_((pci_hitfunc_t)d[1], (int)d[2], (int)d[3], (int)d[4], (int)d[5], (void*)d[6]); return 0;
		case 7: pci_scan_slot_((pci_hitfunc_t)d[1], (int)d[2], (int)d[3], (int)d[4], (void*)d[5]); return 0;
		case 8: pci_scan_bus_((pci_hitfunc_t)d[1], (int)d[2], (int)d[3], (void*)d[4]); return 0;
		case 9: pci_scan_((pci_hitfunc_t)d[1], (int)d[2], (void*)d[3]); return 0;
	}
	return IOCTL_NULL;
}
/***************************************/

MODULE_DEF(pci_driver, pci_mod_init, pci_mod_finit, MODT_IO, "Miguel S.", pci_ioctl);
