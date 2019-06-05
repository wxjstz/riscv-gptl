#define LITTLE_ENDIAN

#include <list.h>

void list_init(struct list_node *list)
{
    list->next = list;
    list->prev = list;
}

void list_remove(struct list_node *node)
{
    struct list_node *prev = node->prev;
    struct list_node *next = node->next;
    prev->next = next;
    next->prev = prev;
}

void list_insert_after(struct list_node *node, struct list_node *after)
{
    after->prev = node;
    after->next = node->next;
    after->prev->next = after;
    if (after->next)
        after->next->prev = after;
}

void list_insert_befor(struct list_node *node, struct list_node *befor)
{
    befor->next = node;
    befor->prev = node->prev;
    befor->next->prev = befor;
    if (befor->prev)
        befor->prev->next = befor;
}
