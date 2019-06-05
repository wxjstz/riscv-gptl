#ifndef __GUID_H__
#define __GUID_H__

#define GUID_BIG_ENDIAN		0
#define GUID_MIXED_ENDIAN	1

#include <stdint.h>

typedef struct {
	uint8_t b[16];
} guid_t;

char *dump_guid(int endian, guid_t * g, void *buff);
int compare_guid(const guid_t * g1, const guid_t * g2);

#endif				/* __GUID_H__ */
