#include <guid.h>
#include <gpt.h>
#include <gptl.h>
#include <string.h>
#include <platform.h>
#include <malloc.h>

/* 4750544c-0000-0000-0000-524953432d56 */
static const guid_t gptl_guid = {.b = {
                                       0x4c, 0x54, 0x50, 0x47,  /* 'GPTL' -> 47 50 54 4c */
                                       0x00, 0x00,
                                       0x00, 0x00,
                                       0x00, 0x00,
                                       0x52, 0x49, 0x53, 0x43, 0x2d, 0x56       /* 'RISC-V' -> 52 49 53 43 2d 56 */
                                       }
};

static struct gpt_partition_entry gptl_partition_entry;
static struct gptl_header gptl_header;
static const char *bootargs;

void *gptl_read(size_t offset, size_t size, void *buff)
{
    offset +=
        gpt_partition_offset(&gptl_partition_entry,
                             0) * platform_logic_block_size();
    return platform_read(offset, size, buff);
}

struct gptl_header *search_gptl(void)
{
    size_t bs;
    struct protective_mbr mbr;
    struct gpt_header gpt_header;
    bs = platform_logic_block_size();
    platform_read(0, sizeof(mbr), &mbr);
    if (!is_gpt_mbr(&mbr))
        return NULL;
    platform_read(1 * bs, sizeof(gpt_header), &gpt_header);
    for (int i = 0; i < gpt_partitions_num(&gpt_header); i++) {
        size_t size = sizeof(gptl_partition_entry);
        size_t offset = gpt_header.lba_parttion_entries * bs + i * size;
        platform_read(offset, size, &gptl_partition_entry);
        if (!compare_guid
            (&(gptl_partition_entry.partition_type_uuid), &gptl_guid)) {
            uint32_t magic, version, header_size;
            gptl_read(0, sizeof(gptl_header), &gptl_header);
            magic = gptl_header_get_field(&gptl_header, magic);
            version = gptl_header_get_field(&gptl_header, version);
            header_size = gptl_header_get_field(&gptl_header, header_size);
            if ((magic != GPTL_MAGIC)
                || (header_size < sizeof(gptl_header))) {
                printf("Image corruption in GPTL partition\n");
                abort();
            }
            if (version > GPTL_VERSION_MAX) {
                printf("Unsupported image version\n");
                abort();
            }
            bootargs = NULL;
            if (header_size > sizeof(gptl_header)) {
                size_t offset = sizeof(gptl_header);
                size_t size = header_size - sizeof(gptl_header);
                bootargs = gptl_read(offset, size, malloc(size));
            }
            return &gptl_header;
        }
        memset(&gptl_partition_entry, 0, sizeof(gptl_partition_entry));
    }
    return NULL;
}

const char *gptl_bootargs(void)
{
    return bootargs;
}

size_t gptl_load_opensbi(void *buff)
{
    if (gptl_header.opensbi_size) {
        size_t size = gptl_header_get_field(&gptl_header, opensbi_size);
        size_t offset =
            gptl_header_get_field(&gptl_header, opensbi_offset);
        gptl_read(offset, size, buff);
        return size;
    }
    return 0;
}

size_t gptl_load_kernel(void *buff)
{
    if (gptl_header.kernel_size) {
        size_t size = gptl_header_get_field(&gptl_header, kernel_size);
        size_t offset = gptl_header_get_field(&gptl_header, kernel_offset);
        gptl_read(offset, size, buff);
        return size;
    }
    return 0;
}

size_t gptl_load_initrd(void *buff)
{
    if (gptl_header.initrd_size) {
        size_t size = gptl_header_get_field(&gptl_header, initrd_size);
        size_t offset = gptl_header_get_field(&gptl_header, initrd_offset);
        gptl_read(offset, size, buff);
        return size;
    }
    return 0;
}

size_t gptl_load_fdt(void *buff)
{
    if (gptl_header.fdt_size) {
        size_t size = gptl_header_get_field(&gptl_header, fdt_size);
        size_t offset = gptl_header_get_field(&gptl_header, fdt_offset);
        gptl_read(offset, size, buff);
        return size;
    }
    return 0;
}
