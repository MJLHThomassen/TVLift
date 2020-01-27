#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Map
 */
typedef struct map_s* map;

typedef struct map_iter_s* map_iter;

/**
 * @brief Create a new Map
 * 
 * @param[out] m Pointer where to store the new map
 */
void map_new(map* m);

/**
 * @brief Copies the map and all its items
 * 
 * @param[in] m The map to be copied
 * @param[out] copy A pointer to where to store the copied map
 */
void map_copy(const map m, map* copy);

/**
 * @brief Add a value with the specified key to the map
 * 
 * @param[in] m The map
 * @param[in] key Key of the value to add
 * @param[in] value Value to add
 */
void map_add(map m, const void * key, const void * value);

/**
 * @brief Gets the value with the specified key from the map
 * 
 * @param[in] m The map
 * @param[in] key The key of the value to get
 * @param[out] value The
 */
void map_get(map m, const void* key, const void ** value);

/**
 * @brief Removes the value with the specified key from the map
 * 
 * @param m The map
 * @param key The key of the value to remove
 */
void map_remove(map m, const void* key);

/**
 * @brief Gets the number of entries in the map
 * 
 * @param[in] m The map 
 * 
 * @return size_t The number of entries in the map
 */
size_t map_count(const map m);

void   map_iter_new(const map map, map_iter* iter);
void   map_iter_delete(map_iter iter);
void   map_iter_next(map_iter iter, void** key, void** value);

#define TYPED_MAP(T)                                 \
    typedef struct map_##T* map_##T;                 \
    inline void map_##T##_new(map_##T* map)          \
    {                                                \
        map_new((map*)map);                          \
    }                                                \
                                                     \
    inline void map_##T##_add(map_##T map, T* item)  \
    {                                                \
        map_add((map)map, (void*)item);              \
    }                                                \
                                                     \
    inline size_t map_##T##_count(const map_##T map) \
    {                                                \
        return map_count((map)map);                  \
    }

#endif // MAP_H