#ifndef SLIST_H
#define SLIST_H

#include "common.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * @brief Singly Linked List 
 */
typedef struct slist_s* slist;

typedef struct slist_iter_s* slist_iter;

typedef struct slist_conf_s
{
    void* (*mem_alloc)(size_t size);
    void* (*mem_calloc)(size_t blocks, size_t size);
    void (*mem_free)(void* block);
    void (*item_copy)(const void* item, void** copy);
    void (*item_delete)(void* item);
} slist_conf;

/**
 * @brief Initializes a configuration object with default values.
 * 
 * @param[out] conf The configuration object to initialize.
 */
void slist_conf_init(slist_conf* conf);

/**
 * @brief Create a new Singly Linked List.
 * 
 * @param[out] list Pointer to the created slist.
 */
util_err_t slist_new(slist* list);

/**
 * @brief Create a new Singly Linked List using the specified configuration.
 * 
 * @param[in] conf The configuration to use.
 * @param[out] list Pointer to the created slist.
 */
util_err_t slist_new_conf(const slist_conf* const conf, slist* list);

/**
 * @brief Deletes the Singly Linked List and frees all of its resources.
 * Resources used by added items are not freed.
 * 
 * @param[in] list The slist to delete.
 */
void slist_delete(slist list);

/**
 * @brief Creates a shallow copy of the list, only copies the slist but not the items
 * (the items in the list still point to the same items as the original list).
 * 
 * @param[in] list The slist to be copied.
 * @param[out] copy A pointer to where to store the copied slist.
 */
util_err_t slist_copy_shallow(const slist list, slist* copy);

/**
 * Creates a deel copy of the list, copies both slist and the items
 * using a user defined copy function.
 * 
 * @param[in] list The slist to be copied.
 * @param[in] copyFn The usder-defined function to create a deep copy of an item.
 * @param[out] copy A pointer to where to store the copied slist .
 */
util_err_t slist_copy_deep(const slist list, void (copyFn)(const void* item, void** copy), slist* copy);

/**
 * @brief Add an item to the end of the list.
 * 
 * @param[in] list The slist to add the item to.
 * @param[in] item Item to add.
 */
util_err_t slist_add(slist list, void* item);

/**
 * @brief Removes the specified item from the list.
 * 
 * @param list[in] The slist to remove the item from.
 * @param item [in] The item to remove.
 * @return util_err_t UTIL_OK if the item was succesfully removed, or
 * UTIL_ERR_NOT_FOUND if the item was not found.
 */
util_err_t slist_remove(slist list, void* item);

/**
 * @brief Remove the item at the specified index from the list.
 * 
 * @param[in] list The slist to remove the item from.
 * @param[in] index The index of the item to remove.
 */
util_err_t slist_remove_at(slist list, size_t index);

/**
 * @brief Removes all items from the list and frees resources used for list storage.
 * Resources used by items are not freed.
 * 
 * @param[in] list The slist to clear.
 */
void slist_clear(slist list);

/**
 * @brief Gets the number of items in the list.
 * 
 * @param[in] list The slist to get the count of.
 * 
 * @return size_t The number of items in the list.
 */
size_t slist_count(const slist list);

/*
 * Itterators
 */

util_err_t  slist_iter_new(const slist list, slist_iter* iter);
util_err_t  slist_iter_delete(slist_iter iter);
util_err_t  slist_iter_next(slist_iter iter, void** item);
size_t      slist_iter_index(const slist_iter iter);

#define TYPED_SLIST(T)                                         \
    typedef struct slist_##T* slist_##T;                       \
    inline void               slist_##T##_new(slist_##T* list) \
    {                                                          \
        slist_new((slist*)list);                               \
    }                                                          \
                                                               \
    inline void slist_##T##_add(slist_##T list, T* item)       \
    {                                                          \
        slist_add((slist)list, (void*)item);                   \
    }                                                          \
                                                               \
    inline size_t slist_##T##_count(const slist_##T list)      \
    {                                                          \
        return slist_count((slist)list);                       \
    }

#endif // SLIST_H