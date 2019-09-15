#ifndef SLIST_H
#define SLIST_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Singly Linked List 
 */
typedef struct slist* slist;

typedef struct slist_iter* slist_iter;

/**
 * @brief Create a new Singly Linked List
 * 
 * @param[out] list Pointer to the created slist
 */
void slist_new(slist* list);

/**
 * @brief Add an item to the end of the list
 * 
 * @param[in] list Pointer to the list
 * @param[in] item Item to add
 */
void slist_add(slist list, void* item);

/**
 * @brief Gets the number of items in the list
 * 
 * @param[in] list Pointer to the list 
 * 
 * @return size_t The number of items in the list
 */
size_t slist_count(const slist list);

void slist_iter_new(const slist list, slist_iter* iter);
void slist_iter_next(slist_iter iter, void** item);


#define TYPED_SLIST(T)                                                                       \
    typedef struct slist_##T* slist_##T;                                                     \
    inline void slist_##T##_new(slist_##T* list)                                             \
    {                                                                                        \
        slist_new((slist*)list);                                                             \
    }                                                                                        \
                                                                                             \
    inline void slist_##T##_add(slist_##T list, T* item)                                     \
    {                                                                                        \
        slist_add((slist)list, (void*)item);                                                 \
    }                                                                                        \
                                                                                             \
    inline size_t slist_##T##_count(const slist_##T list)                                    \
    {                                                                                        \
        return slist_count((slist)list);                                                     \
    }

#endif // SLIST_H