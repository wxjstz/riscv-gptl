#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

static inline uint8_t read8(void *addr)
{
	return *(volatile uint8_t *)addr;
}

static inline uint16_t read16(void *addr)
{
	return *(volatile uint16_t *)addr;
}

static inline uint32_t read32(void *addr)
{
	return *(volatile uint32_t *)addr;
}

static inline uint64_t read64(void *addr)
{
	return *(volatile uint64_t *)addr;
}

static inline void write8(void *addr, uint8_t v)
{
	*(volatile uint8_t *)addr = v;
}

static inline void write16(void *addr, uint16_t v)
{
	*(volatile uint16_t *)addr = v;
}

static inline void write32(void *addr, uint32_t v)
{
	*(volatile uint32_t *)addr = v;
}

static inline void write64(void *addr, uint64_t v)
{
	*(volatile uint64_t *)addr = v;
}

#endif
