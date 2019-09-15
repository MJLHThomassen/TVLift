#include "map.h"

#include "slist.h"

typedef struct map_entry
{
    void* key;
    void* value;
} map_entry;

struct map
{
    slist entries;
};

void map_new(map* m)
{
    map newMap = (map)calloc(1, sizeof(map));
    *m = newMap;
}

void map_add(map m, const void* key, const void* value)
{
    map_entry* newEntry = (map_entry*)malloc(sizeof(map_entry));

    newEntry->key = key;
    newEntry->value = value;

    slist_add(m->entries, newEntry);
}

size_t map_count(const map m)
{
    return slist_count(m->entries);
}
