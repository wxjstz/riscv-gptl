#include <gpt.h>

int is_gpt_mbr(struct protective_mbr *mbr)
{
    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            if (mbr->partition[i].type != 0xee)
                return 0;
        } else {
            if (mbr->partition[i].type != 0)
                return 0;
        }
    }
    return 1;
}

int gpt_partitions_num(struct gpt_header *header)
{
    return header->partition_entries_num;
}

size_t gpt_partition_logic_blocks_num(struct gpt_partition_entry * entry)
{
    return entry->lba_last - entry->lba_first + 1;
}

size_t gpt_partition_offset(struct gpt_partition_entry * entry, size_t lba)
{
    if (entry->lba_first + lba > entry->lba_last)
        return entry->lba_last;
    return entry->lba_first + lba;
}
