#ifndef __GPT_H__
#define __GPT_H__
#include <stdint.h>
#include <guid.h>

#define CHS_GET_HEAD(chs)		((chs)[0])
#define CHS_GET_SECTOR(chs)		((chs)[1] & 0x3f)
#define CHS_GET_CYLINDER(chs)	((((chs)[1] & 0xc0) << 2) | (chs)[2])

#define CHS_SET_HEAD(chs,h)		((chs)[0] = (h) & 0xff)
#define CHS_SET_SECTOR(chs,s)	((chs)[1] &= ~0x3f, (chs)[1] |= (s) & 0x3f)
#define CHS_SET_CYLINDER(chs,c)	((chs)[1] &= ~0xc0, (chs)[1] |= ((c) >> 2) & 0xc0, (chs)[2] = (c) & 0xff)

#define MBR_PARTITON_GET_LBA_START(pe)	le32_to_cpu((pe)->lba_start)
#define MBR_PARTITON_GET_BLOCK_NUM(pe)	le32_to_cpu((pe)->block_num)

#define MBR_PARTITON_SET_LBA_START(pe,v)	((pe)->lba_start = cpu_to_le32(v))
#define MBR_PARTITON_SET_BLOCK_NUM(pe,v)	((pe)->block_num = cpu_to_le32(v))

struct protective_mbr {
	uint8_t boot_code[446];	/* bootstrap code */
	struct partition_entry {
		uint8_t status;	/* 0x80 -> bootable, 0x00 -> inactive */
		uint8_t chs_start[3];	/* start chs address of the partition */
		uint8_t type;	/* partition type */
		uint8_t chs_end[3];	/* end chs address of the partition */
		uint32_t lba_start;	/* logic block adress for the partition begins */
		uint32_t block_num;	/* number of blocks contained in the partition */
	} partition[4];
	uint8_t boot_signature[2];	/* 0x55 0xaa -> bootable */
} __attribute__ ((packed));

struct gpt_header {
	uint64_t signature;	/* 'EFI PART' */
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_checksum;
	uint32_t reserved;
	uint64_t lba_current;
	uint64_t lba_backup;
	uint64_t lba_first_usable;
	uint64_t lba_last_usable;
	guid_t disk_uuid;
	uint64_t lba_parttion_entries;
	uint32_t partition_entries_num;
	uint32_t partition_entry_size;
	uint32_t partition_entries_checksum;
} __attribute__ ((packed));

#define GPT_PARTTION_ATTRIBUTE_READ_ONLY		(1 << 60)
#define GPT_PARTTION_ATTRIBUTE_SHADOW			(1 << 61)
#define GPT_PARTTION_ATTRIBUTE_HIDDEN			(1 << 62)
#define GPT_PARTTION_ATTRIBUTE_NON_AUTOMOUNT	(1 << 63)

struct gpt_partition_entry {
	guid_t partition_type_uuid;
	guid_t unique_partition_uuid;
	uint64_t lba_first;
	uint64_t lba_last;
	uint64_t attribute;
	uint8_t partition_name[72];	/* UTF-16LE */
} __attribute__ ((packed));

int is_gpt_mbr(struct protective_mbr *mbr);
int gpt_partitions_num(struct gpt_header *header);
size_t gpt_partition_logic_blocks_num(struct gpt_partition_entry *entry);
size_t gpt_partition_offset(struct gpt_partition_entry *entry, size_t lba);

#endif				/* __GPT_H__ */
