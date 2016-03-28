#pragma once

#include <stdint.h>
#include <attr.h>

typedef struct node {
	struct node * next;
	struct node * prev;
	void * value;
	void * owner;
} __attribute__((packed)) node_t;

typedef struct {
	node_t * head;
	node_t * tail;
	size_t length;
} __attribute__((packed)) list_t;

void list_destroy(list_t * list);
void list_free(list_t * list);
void list_append(list_t * list, node_t * item);
node_t * list_insert(list_t * list, void * item);
list_t * list_create(void);
node_t * list_find(list_t * list, void * value);
int list_index_of(list_t * list, void * value);
void list_remove(list_t * list, size_t index);
void list_delete(list_t * list, node_t * node);
node_t * list_pop(list_t * list);
node_t * list_dequeue(list_t * list);
list_t * list_copy(list_t * original);
void list_merge(list_t * target, list_t * source);

void list_append_after(list_t * list, node_t * before, node_t * node);
node_t * list_insert_after(list_t * list, node_t * before, void * item);

void list_append_before(list_t * list, node_t * after, node_t * node);
node_t * list_insert_before(list_t * list, node_t * after, void * item);

int list_size(list_t * list);
node_t * list_get(list_t* list, int index);
node_t * list_get_last(list_t * list);
node_t * list_get_first(list_t * list);
int list_index_of_node(list_t * list, node_t * node);

node_t * list_get_next_nth(node_t* node, int increments);
void list_clear(list_t * list);

#define foreach(i, list) for (node_t * i = (list)->head; i != NULL; i = i->next)
#define foreachr(i, list) for (node_t * i = (list)->tail; i != NULL; i = i->prev)
#define forl(j,cond,inc,list) j; for(node_t * node = list_get(list,i); node != NULL && cond; node=list_get_next_nth(node,inc), i++)
