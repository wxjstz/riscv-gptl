#include <guid.h>

char *dump_guid(int endian, guid_t * g, void *buff)
{
    char *p = buff;
    const int *e_tab;
    static const int be_tab[] =
        { 0, 1, 2, 3, -1, 4, 5, -1, 6, 7, -1, 8, 9, -1, 10, 11, 12, 13, 14,
        15
    };
    static const int mix_tab[] =
        { 3, 2, 1, 0, -1, 5, 4, -1, 7, 6, -1, 8, 9, -1, 10, 11, 12, 13, 14,
        15
    };
    static const char hex_tab[] =
        { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c',
        'd', 'e', 'f'
    };
    switch (endian) {
    case GUID_BIG_ENDIAN:
        e_tab = be_tab;
        break;
    case GUID_MIXED_ENDIAN:
        e_tab = mix_tab;
        break;
    default:
        return NULL;
    }
    for (int i = 0; i < 20; i++) {
        int pos = e_tab[i];
        if (pos == -1)
            *p++ = '-';
        else {
            *p++ = hex_tab[(g->b[pos & 0xf] >> 4) & 0xf];
            *p++ = hex_tab[(g->b[pos & 0xf] >> 0) & 0xf];
        }
    }
    *p++ = '\0';
    return buff;
}

int compare_guid(const guid_t * g1, const guid_t * g2)
{
    const char *p1 = g1->b;
    const char *p2 = g2->b;
    for (int i = 0; i < 16; i++) {
        int t = *p1++ - *p2++;
        if (t)
            return t;
    }
    return 0;
}
