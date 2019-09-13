#include "slist.h"

#include <stddef.h>

typedef struct slist_node
{
    struct slist_node* next;
    void* item;
} slist_node;

struct slist
{
    slist_node* first;
    slist_node* last;
    size_t count;
};

void slist_new(slist* list)
{
    slist newList = (slist)calloc(1, sizeof(slist));
    *list = newList;
}

void slist_add(slist list, void* item)
{
    slist_node* newNode = (slist_node*)calloc(1, sizeof(slist_node));
    newNode->item = item;

    list->last->next = newNode;
    list->last = newNode;
}

size_t slist_count(const slist list)
{
    return list->count;
}
