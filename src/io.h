#pragma once

#include "stdint.h"

namespace Kernel {
	namespace IO {
		inline uint16_t ins(uint16_t _port) {
			uint16_t rv;
			asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
			return rv;
		}

		inline void outs(uint16_t _port, uint16_t _data) {
			asm volatile ("outw %1, %0" : : "dN" (_port), "a" (_data));
		}

		inline unsigned int inl(uint16_t _port) {
			unsigned int rv;
			asm volatile ("inl %%dx, %%eax" : "=a" (rv) : "dN" (_port));
			return rv;
		}

		inline void outl(uint16_t _port, unsigned int _data) {
			asm volatile ("outl %%eax, %%dx" : : "dN" (_port), "a" (_data));
		}

		inline unsigned char inb(uint16_t _port) {
			unsigned char rv;
			asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
			return rv;
		}

		inline void outb(uint16_t _port, unsigned char _data) {
			asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
		}

		inline void outsm(uint16_t port, unsigned char * data, unsigned long size) {
			asm volatile ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
		}

		inline void insm(uint16_t port, unsigned char * data, unsigned long size) {
			asm volatile ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
		}
	}
}
