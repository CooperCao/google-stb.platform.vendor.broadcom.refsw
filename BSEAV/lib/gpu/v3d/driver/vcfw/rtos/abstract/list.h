/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef _LIST_H
#define _LIST_H

#define LIST_INIT(head) ((head)->l_first=NULL)

#define LIST_HEAD(name, type) struct name { struct type *l_first; }
#define LIST_ENTRY(type)  struct { struct type *l_next, *l_prev; const void *l_head; }
#define LIST_INITIALIZER(head) {NULL}
#define LIST_EMPTY(head) ((head)->l_first == NULL)
#define LIST_FIRST(head) ((head)->l_first)
#define LIST_NEXT(elm, field) ((elm)->field.l_next)
#define LIST_PREV(elm, field) ((elm)->field.l_prev)

#define LIST_INSERT_HEAD(head, new_elm, field) do { \
    (new_elm)->field.l_head = (const void *)head; \
    if ( ((new_elm)->field.l_next = (head)->l_first) != NULL ) (head)->l_first->field.l_prev = (new_elm); \
    (head)->l_first = (new_elm); (new_elm)->field.l_prev = NULL; \
        }  while(0)

#define LIST_INSERT_AFTER(head, elm, new_elm, field) do { \
    assert((elm)->field.l_head == (const void *)head); \
    (new_elm)->field.l_head = (const void *)head; \
    (new_elm)->field.l_prev = (elm); \
    if (((new_elm)->field.l_next = elm->field.l_next)!=NULL)  elm->field.l_next->field.l_prev = new_elm; \
    (elm)->field.l_next = (new_elm); \
        } while(0)

#define LIST_INSERT_BEFORE(head, elm, new_elm, field) do { \
    assert((elm)->field.l_head == (const void *)head); \
    (new_elm)->field.l_head = (const void *)head; \
    (new_elm)->field.l_next = (elm); \
    if (((new_elm)->field.l_prev = (elm)->field.l_prev)!=NULL) elm->field.l_prev->field.l_next = new_elm; else (head)->l_first = (new_elm); \
    (elm)->field.l_prev = (new_elm); \
        } while(0)

#define LIST_REMOVE(head, elm, field) do { \
    assert((elm)->field.l_head == (const void *)head); \
    (elm)->field.l_head = NULL; \
    if ((elm)->field.l_next) (elm)->field.l_next->field.l_prev = (elm)->field.l_prev;  \
    if ((elm)->field.l_prev) (elm)->field.l_prev->field.l_next = (elm)->field.l_next; else (head)->l_first = (elm)->field.l_next; \
        } while(0)

#define LIST_REMOVE_HEAD(head, field) do { \
    assert((head)->l_first); \
    assert((head)->l_first->field.l_head == (const void *)head); \
    (head)->l_first->field.l_head = NULL; \
    (head)->l_first = (head)->l_first->field.l_next; \
    if ((head)->l_first) { (head)->l_first->field.l_prev = NULL;} \
        } while(0)

#endif /* _LIST_H */
