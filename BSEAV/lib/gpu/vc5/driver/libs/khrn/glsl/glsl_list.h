/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_LIST_H
#define GLSL_LIST_H

#define DECLARE_LIST(type) \
\
struct glsl_link_ ## type { \
   type *next; \
   type *prev; \
}; \
\
struct glsl_list_ ## type { \
   type *head; \
   type *tail; \
   int count; \
}

#define DECLARE_LIST_FUNCTIONS(type, link_field) \
\
static inline void glsl_list_##type##_init(struct glsl_list_##type *list) { \
   list->head = NULL; \
   list->tail = NULL; \
   list->count = 0; \
} \
\
static inline void glsl_list_##type##_append(struct glsl_list_##type *list, type *node) { \
   node->link_field.prev = list->tail; \
   node->link_field.next = NULL; \
\
   if (!list->head) list->head = node; \
   if (list->tail) list->tail->link_field.next = node; \
   list->tail = node; \
\
   list->count++; \
} \
\
static inline void glsl_list_##type##_remove(struct glsl_list_##type *list, type *node) { \
   if (node->link_field.next) node->link_field.next->link_field.prev = node->link_field.prev; \
   if (node->link_field.prev) node->link_field.prev->link_field.next = node->link_field.next; \
\
   list->count--; \
   if (list->head == node) list->head = node->link_field.next; \
   if (list->tail == node) list->tail = node->link_field.prev; \
\
   assert(list->count != 0 || (list->head == NULL && list->tail == NULL)); \
}

#define LIST_FOR_EACH(node, list_ptr, link_field) \
   for((node) = (list_ptr)->head; (node) != NULL; (node) = (node)->link_field.next)

/* Node lists. Declare a Node type and form a list of them */

#define DECLARE_NODE_LIST(type,alloc)                \
\
typedef struct glsl_list_node##type type##ChainNode; \
\
DECLARE_LIST(type ## ChainNode); \
\
struct glsl_list_node##type { \
   struct glsl_link_##type##ChainNode l; \
   type *ptr; \
}; \
\
DECLARE_LIST_FUNCTIONS(type ## ChainNode, l) \
\
typedef struct glsl_list_##type##ChainNode type##Chain; \
\
static inline void glsl_node_list_##type##_append(type ## Chain *chain, type *ptr) { \
   type ## ChainNode *node = (type ## ChainNode *)alloc(sizeof( type ## ChainNode )); \
   node->ptr = ptr; \
   glsl_list_##type##ChainNode_append(chain, node); \
} \
\
static inline bool glsl_node_list_##type##_contains(type ## Chain *chain, type *ptr) { \
   type ## ChainNode *node; \
\
   LIST_FOR_EACH(node, chain, l) \
      if (node->ptr == ptr) return true; \
\
   return false; \
}

#define NODE_LIST_FOR_EACH(node, list_ptr) LIST_FOR_EACH(node, list_ptr, l)

#endif // GLSL_LIST_H
