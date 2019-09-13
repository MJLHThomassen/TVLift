#ifndef SLIST_H
#define SLIST_H

/**
 * @brief Singly Linked List 
 */
typedef struct slist* slist;

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

#endif // SLIST_H