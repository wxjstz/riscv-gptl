#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <stdint.h>

#define bswap_func_helper(name,type)				\
	static inline type name(type in) {				\
		type r;										\
		uint8_t *p1 = (uint8_t *)&in;					\
		uint8_t *p2 = (uint8_t *)&r + sizeof(type);	\
		for ( int i = 0; i < sizeof(type); i++)		\
			*--p2 = *p1++;							\
		return r;									\
	}
bswap_func_helper(bswap16, uint16_t)
    bswap_func_helper(bswap32, uint32_t)
    bswap_func_helper(bswap64, uint64_t)
#undef bswap_func_helper
#define LITTLE_ENDIAN
#if defined(LITTLE_ENDIAN)
#define cpu_to_le16(in)	(in)
#define cpu_to_le32(in)	(in)
#define cpu_to_le64(in)	(in)
#define cpu_to_be16(in)	bswap16(in)
#define cpu_to_be32(in)	bswap32(in)
#define cpu_to_be64(in)	bswap64(in)
#define le16_to_cpu(in)	(in)
#define le32_to_cpu(in)	(in)
#define le64_to_cpu(in)	(in)
#define be16_to_cpu(in)	bswap16(in)
#define be32_to_cpu(in)	bswap32(in)
#define be64_to_cpu(in)	bswap64(in)
#elif defined(BIG_ENDIAN)
#define cpu_to_le16(in)	bswap16(in)
#define cpu_to_le32(in)	bswap32(in)
#define cpu_to_le64(in)	bswap64(in)
#define cpu_to_be16(in)	(in)
#define cpu_to_be32(in)	(in)
#define cpu_to_be64(in)	(in)
#define le16_to_cpu(in)	bswap16(in)
#define le32_to_cpu(in)	bswap32(in)
#define le64_to_cpu(in)	bswap64(in)
#define be16_to_cpu(in)	(in)
#define be32_to_cpu(in)	(in)
#define be64_to_cpu(in)	(in)
#else
#error "endian.h requires the definition of LITTLE_ENDIAN or BIG_ENDIAN"
#endif				/* BIG_ENDIAN */
#define htons	cpu_to_be16
#define htonl	cpu_to_be32
#define htonll	cpu_to_be64
#define ntohs	be16_to_cpu
#define ntohl	be32_to_cpu
#define ntohll	be64_to_cpu
#endif				/* __ENDIAN_H__ */
