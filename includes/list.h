#ifndef __LIST_H__
#define __LIST_H__

#define offsetof(type,member)			((uintptr_t)&(((type*)0)->member))
#define container_of(ptr,type,member)	((type*)((uintptr_t)ptr - offsetof(type,member)))

struct list_node {
	struct list_node *prev;
	struct list_node *next;
};

void list_init(struct list_node *list);
void list_remove(struct list_node *node);
void list_insert_after(struct list_node *node, struct list_node *after);
void list_insert_befor(struct list_node *node, struct list_node *befor);

#define list_for_each(node, head, type, member)					\
	for (type *node = container_of(head.next, type, member);	\
			&(node->member) != &head;							\
			node = container_of(node->member.next, type, member))

#endif				/* __LIST_H__ */
