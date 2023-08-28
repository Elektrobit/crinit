// SPDX-License-Identifier: MIT
/**
 * @file list.h
 * Implementation of a doubly linked intrusive list.
 *
 * Intrusive lists use a list structure containing a next pointer pointing to the next list element for singly linked
 * list and a next and prev pointer pointing to the next and previous element for a doubly linked list. This list type
 * gets embedded into the entry type struct which should be listed. With the help of a container_of macro the parent
 * struct can be retrived for a given list entry. To construct a list a list head is generated, which is not embedded
 * into the struct managed in the list. This head points to the first (head->next) and last (head-prev) entry in the
 * list or to itself, if the list is empty. Thus iterating the list is as simple as starting at head->next and iterating
 * through the entries with cur->next, until cur->next points to head again.
 */
#ifndef CRINIT_LIST_H
#define CRINIT_LIST_H

#include <stdlib.h>

/**
 * Macro to find the container struct to a given struct field.
 *
 * @param ptr    Pointer to the field in the structure.
 * @param type   Type of the container wrapping the field.
 * @param member Name of the field the pointer points to.
 */
#define container_of(ptr, type, member) ((type *)((char *)(ptr)-offsetof(type, member)))

/**
 * Simple intrusive list struct.
 */
typedef struct crinitList {
    struct crinitList *prev;  ///< Pointer to last entry in list.
    struct crinitList *next;  ///< Pointer to next entry in list.
} crinitList_t;

/**
 * Type for a list entry compare function.
 *
 * The comparison function shall return a value > 0,
 * if the container type of e2 is larger than the one of e2,
 * a value < 0 if less than the one of e2 and 0 if they are equal.
 */
typedef int (*crinitListCmp_t)(crinitList_t *e1, crinitList_t *e2);

/**
 * Initializes a new list head by pointing the previous and
 * next pointer to itself.
 *
 * @param list List to be initialized.
 */
#define CRINIT_LIST_INIT(list) \
    { &(list), &(list) }

/**
 * Initializes a new list head by pointing the previous and
 * next pointer to itself.
 *
 * @param list Pointer to list to be initialized.
 */
static inline void crinitListInit(crinitList_t *list) {
    list->next = list;
    list->prev = list;
}

/**
 * Insert a new list entry.
 *
 * @param entry Entry to be inserted.
 * @param prev Entry to be inserted after.
 * @param next Entry to be inserted before.
 */
static inline void crinitListInsert(crinitList_t *entry, crinitList_t *prev, crinitList_t *next) {
    entry->next = next;
    entry->prev = prev;
    next->prev = entry;
    prev->next = entry;
}

/**
 * Insert a new entry at the beginning of the list.
 *
 * @param list Pointer to the list.
 * @param entry Entry to be appended.
 */
static inline void crinitListPrepend(crinitList_t *list, crinitList_t *entry) {
    crinitListInsert(entry, list, list->next);
}

/**
 * Appends a new entry at the end of the list.
 *
 * @param list Pointer to the list.
 * @param entry Entry to be appended.
 */
static inline void crinitListAppend(crinitList_t *list, crinitList_t *entry) {
    crinitListInsert(entry, list->prev, list->prev->next);
}

/**
 * Insert sorted based on a simple comparison callback.
 *
 * @param list List to insert the entry into.
 * @param entry Entry to be inserted.
 * @param entryCmp Compare callback.
 */
static inline void crinitListInsertSorted(crinitList_t *list, crinitList_t *entry, crinitListCmp_t entryCmp) {
    crinitList_t *cur;

    for (cur = list->next; cur != list; cur = cur->next) {
        if (entryCmp(cur, entry) >= 0) {
            crinitListInsert(entry, cur->prev, cur);
            break;
        }
    }

    if (cur == list) {
        crinitListAppend(list, entry);
    }
}

/**
 * Deletes the current entry from the list.
 *
 * Removes the entry from the list by updating the next pointer of
 * the previous and the prev pointer of the next entry, while
 * setting the next and prev pointers of the entry itself to null.
 * However this will cause a stack error if either entry, entry->next
 * or entry->prev is NULL.
 *
 * @param entry Entry to be removed from the list.
 */
static inline void crinitListDelete(crinitList_t *entry) {
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = NULL;
    entry->prev = NULL;
}

/**
 * Returns wether the list is empty or not.
 *
 * Checks if the list is empty by checking if the next pointer
 * of the list points to the list itself.
 *
 * @param list The list to be checked.
 * @return int Returns true if list is empty, false otherwise.
 */
static inline int crinitListIsEmpty(const crinitList_t *list) {
    return list->next == list;
}

/**
 * Return the struct containing this list pointer,
 *
 * @param entry   Pointer to the list entry.
 * @param type    Type of the containing struct.
 * @param member  Name of the list member within the container type.
 */
#define crinitListEntry(entry, type, member) container_of(entry, type, member)

/**
 * Return the containing entry for the first list entry,
 *
 * @param list    Pointer to the list head.
 * @param type    Type of the containing struct.
 * @param member  Name of the list member within the container type.
 */
#define crinitListFirstEntry(list, type, member) crinitListEntry((list)->next, type, member)

/**
 * Return the containing entry for the last list entry,
 *
 * @param list    Pointer to the list head.
 * @param type    Type of the containing struct.
 * @param member  Name of the list member within the container type.
 */
#define crinitListLastEntry(list, type, member) crinitListEntry((list)->prev, type, member)

/**
 * Return the previous containing entry for the current entry,
 *
 * @param entry   Current container entry.
 * @param member  Name of the list member within the container type.
 */
#define crinitListPrevEntry(entry, member) crinitListEntry((entry)->member.prev, __typeof(*(entry)), member)

/**
 * Return the next containing entry for the current entry,
 *
 * @param entry   Current container entry.
 * @param member  Name of the list member within the container type.
 */
#define crinitListNextEntry(entry, member) crinitListEntry((entry)->member.next, __typeof(*(entry)), member)

/**
 * Returns wether the current container entry contains the list head.
 *
 * @param entry   Current container entry.
 * @param list    Pointer to the list head.
 * @param member  Name of the list member within the container type.
 */
#define crinitListEntryIsHead(entry, list, member) (&(entry)->member == (list))

/**
 * Iterates over all entries in the list.
 *
 * @param entry   Current container entry.
 * @param list    Pointer to the list head.
 * @param member  Name of the list member within the container type.
 */
#define crinitListForEachEntry(entry, list, member)                        \
    for ((entry) = crinitListFirstEntry(list, __typeof(*(entry)), member); \
         !crinitListEntryIsHead(entry, list, member); (entry) = crinitListNextEntry(entry, member))

/**
 * Safely iterates over all entries in the list.
 *
 * This macro additionally keeps track of the next entry of the current
 * entry, so the loop won't break if the current entry gets removed from the list.
 *
 * @param entry   Current container entry.
 * @param temp    Temporary pointer, pointing to the next entry of the current container entry.
 * @param list    Pointer to the list head.
 * @param member  Name of the list member within the container type.
 */
#define crinitListForEachEntrySafe(entry, temp, list, member)              \
    for ((entry) = crinitListFirstEntry(list, __typeof(*(entry)), member), \
        (temp) = crinitListNextEntry(entry, member);                       \
         !crinitListEntryIsHead(entry, list, member); (entry) = (temp), (temp) = crinitListNextEntry(temp, member))

#endif /* CRINIT_LIST_H */
