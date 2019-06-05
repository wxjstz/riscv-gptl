
#ifndef __GPTL_IMAGE_H__
#define __GPTL_IMAGE_H__

#include <endian.h>

struct gptl_header {
	uint32_t magic;		/* "GPTL" -> 0x4750544c */

	uint32_t version;
	uint32_t header_size;
	uint32_t total_size;

	uint32_t opensbi_offset;
	uint32_t opensbi_size;

	uint32_t kernel_offset;
	uint32_t kernel_size;

	uint32_t initrd_offset;
	uint32_t initrd_size;

	uint32_t fdt_offset;
	uint32_t fdt_size;

	char commandline[0];
};

void *gptl_read(size_t offset, size_t size, void *buff);
struct gptl_header *search_gptl(void);
const char *gptl_bootargs(void);
size_t gptl_load_opensbi(void *buff);
size_t gptl_load_kernel(void *buff);
size_t gptl_load_initrd(void *buff);
size_t gptl_load_fdt(void *buff);

#define GPTL_MAGIC	0x4750544c
#define GPTL_VERSION	0
#define GPTL_VERSION_MAX	0

#define gptl_header_get_field(hdr,field)		ntohl((hdr)->field)
#define gptl_header_set_field(hdr,field,ver)	((hdr)->field = htonl(ver))

#endif				/* __GPTL_IMAGE_H__ */
