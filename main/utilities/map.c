#include "map.h"

#include "slist.h"

typedef struct map_entry_s
{
    void const* key;
    void const* value;
} * map_entry;

struct map_s
{
    slist entries;
};

struct map_iter_s
{
    map        m;
    slist_iter iter;
};

void map_new(map* m)
{
    map newMap = (map)calloc(1, sizeof(map));
    
    slist_new(&newMap->entries);

    *m = newMap;
}

void map_copy(map m, map* copy)
{
    map newMap = (map)calloc(1, sizeof(map));

    slist_copy_shallow(m->entries, &newMap->entries);

    *copy = newMap;
}

void map_add(map m, void const* key, void const* value)
{
    map_entry newEntry = (map_entry)malloc(sizeof(*newEntry));

    newEntry->key = key;
    newEntry->value = value;

    slist_add(m->entries, newEntry);
}

void map_get(map m, const void* key, void const** value)
{
    if (slist_count(m->entries) == 0)
    {
        *value = NULL;
        return;
    }

    slist_iter iter;
    slist_iter_new(m->entries, &iter);

    map_entry entry;
    do
    {
        // Get the next entry in the list
        slist_iter_next(iter, (void**)(&entry));

        // If this entry's key matches, return the value
        if (entry->key == key)
        {
            *value = entry->value;
            return;
        }

    } while (slist_iter_index(iter) < slist_count(m->entries));

    // Key not found
    *value = NULL;
}

void map_remove(map m, const void* key)
{
    if (slist_count(m->entries) == 0)
    {
        return;
    }

    slist_iter iter;
    slist_iter_new(m->entries, &iter);

    map_entry entry;
    do
    {
        // Get the next entry in the list
        slist_iter_next(iter, (void**)(&entry));

        // If this entry's key matches, return the value
        if (entry->key == key)
        {
            slist_remove_at(m->entries, slist_iter_index(iter));
            return;
        }

    } while (slist_iter_index(iter) < slist_count(m->entries));
}

size_t map_count(const map m)
{
    return slist_count(m->entries);
}

void map_iter_new(const map map, map_iter* iter)
{
    map_iter newIter = (map_iter)malloc(sizeof(*newIter));

    newIter->m = map;
    slist_iter_new(map->entries, &newIter->iter);

    *iter = newIter;
}

void map_iter_delete(map_iter iter)
{
    free(iter);
}

void map_iter_next(map_iter iter, void** key, void** value)
{
    map_entry entry;
    slist_iter_next(iter->iter, (void**)&entry);

    if(entry == NULL)
    {
        *key = NULL;
        *value = NULL;
    }
    else
    {
        *key = entry->key;
        *value = entry->value;
    }
}