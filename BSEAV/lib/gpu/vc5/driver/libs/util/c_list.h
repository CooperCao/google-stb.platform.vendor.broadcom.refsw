/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stddef.h>
#include <stdlib.h>

#ifndef C_LIST
#error "C_LIST must be defined before including c_list.h"
#endif
#ifndef C_LIST_VALUE_TYPE
#error "C_LIST_VALUE_TYPE must be defined before including c_list.h"
#endif

#ifndef C_LIST_MALLOC
#define C_LIST_MALLOC malloc
#endif
#ifndef C_LIST_FREE
#define C_LIST_FREE free
#endif

#ifndef C_LIST_ID
#define C_LIST_ID2(x,y) x##y
#define C_LIST_ID1(x,y) C_LIST_ID2(x,y)
#define C_LIST_ID(x) C_LIST_ID1(C_LIST,x)
#endif

#define C_LIST_NODE C_LIST_ID(_node)

typedef struct C_LIST_NODE
{
   struct C_LIST_NODE* prev;
   struct C_LIST_NODE* next;
   C_LIST_VALUE_TYPE val;
} C_LIST_NODE;

typedef struct C_LIST
{
   C_LIST_NODE* head;
   C_LIST_NODE* tail;
} C_LIST;

static inline void C_LIST_ID(_init)(C_LIST* list)
{
   list->head = NULL;
   list->tail = NULL;
}

static inline void C_LIST_ID(_term)(C_LIST* list)
{
   for (C_LIST_NODE* node = list->head; node; )
   {
      C_LIST_NODE* next = node->next;
      C_LIST_FREE(node);
      node = next;
   }
}

static inline void C_LIST_ID(_push_back)(C_LIST* list, C_LIST_VALUE_TYPE val)
{
   assert(!list->head == !list->tail);

   C_LIST_NODE* n = C_LIST_MALLOC(sizeof(C_LIST_NODE));
   n->val = val;
   n->next = NULL;
   n->prev = list->tail;

   if (list->tail)
      list->tail->next = n;
   list->tail = n;
   if (!list->head)
      list->head = n;
}

static inline void C_LIST_ID(_erase)(C_LIST* list, C_LIST_NODE* node)
{
   assert(!list->head == !list->tail);
   assert(!node->prev || node->prev->next == node);
   assert(!node->next || node->next->prev == node);

   if (node->prev)
      node->prev->next = node->next;
   if (node->next)
      node->next->prev = node->prev;
   if (list->head == node)
      list->head = node->next;
   if (list->tail == node)
      list->tail = node->prev;
   C_LIST_FREE(node);
}

//  Auto undef all macros
#undef C_LIST
#undef C_LIST_VALUE_TYPE
#undef C_LIST_NODE
#undef C_LIST_MALLOC
#undef C_LIST_FREE
