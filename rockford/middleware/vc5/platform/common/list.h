/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>
#include <stdbool.h>

/*
 * A simple doubly-linked list implementation.
 *
 * It contains enough of the implementation to add and remove list items
 * from the front and back of the list. It can be easily extended to do more,
 * if necessary.
 *
 * Usage example:
 *
 * struct my_stuff
 * {
 *     int some_of_my_data;
 *     ...
 *     struct list link;
 *     ...
 *     int more_of_my_stuff;
 * };
 *
 * struct list list, *list_ptr;
 * struct my_stuff my_stuff, *my_stuff_ptr
 *
 * list_init(&list);
 * list_push_front(&list, &my_stuff);
 * list_ptr = list_pop_front(&list);
 * my_stuff_ptr = list_entry(list_ptr, struct my_stuff, link);
 */

/*
 * Get pointer to a container from pointer to a member
 *
 * ptr: pointer to a member variable within a larger container
 * type: type of the container
 * member: name of a member element within the container pointed to by ptr
 */
#ifndef container_of
#define container_of(ptr, type, member) \
   (type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member))
#endif

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

struct list
{
   struct list *prev;
   struct list *next;
};

static inline void list_init(struct list *head)
{
   head->prev = head;
   head->next = head;
}

static inline bool list_empty(const struct list *head)
{
   return head->next == head;
}

static inline void list_add_between(struct list *item, struct list *prev,
      struct list *next)
{
   item->prev = prev;
   item->next = next;

   prev->next = item;
   next->prev = item;
}

static inline void list_push_front(struct list *head, struct list *item)
{
   list_add_between(item, head, head->next);
}

static inline void list_push_back(struct list *head, struct list *item)
{
   list_add_between(item, head->prev, head);
}

static inline void list_del_between(struct list *prev, struct list *next)
{
   prev->next = next;
   next->prev = prev;
}

static inline void list_del(struct list *item)
{
   list_del_between(item->prev, item->next);
   item->prev = NULL;
   item->next = NULL;
}

static inline struct list *list_pop_front(struct list *head)
{
   if (list_empty(head))
      return NULL;

   struct list *front = head->next;
   list_del(front);
   return front;
}

static inline struct list *list_pop_back(struct list *head)
{
   if (list_empty(head))
      return NULL;

   struct list *back = head->prev;
   list_del(back);
   return back;
}

#endif /* __LIST_H__ */
