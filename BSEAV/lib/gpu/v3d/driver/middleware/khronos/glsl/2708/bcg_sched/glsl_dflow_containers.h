/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>
#include "middleware/khronos/glsl/glsl_fastmem.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_forward.h"
#include "middleware/khronos/glsl/glsl_common.h"

///////////////////////////////////////////////////////////////////////////////
// Memory Management
///////////////////////////////////////////////////////////////////////////////

static inline void *bcg_glsl_malloc(uint32_t sz)
{
   return glsl_fastmem_malloc(sz, true);
}

static inline void bcg_glsl_free(void *ptr)
{
   UNUSED(ptr);
}

///////////////////////////////////////////////////////////////////////////////
// VectorBase
///////////////////////////////////////////////////////////////////////////////

typedef struct VectorBase_s
{
   void *data;
   uint32_t capacity;
   uint32_t size;
} VectorBase;

extern void VectorBase_Constr(VectorBase *vector, uint32_t capacity);
extern void VectorBase_Destr(VectorBase *vector);

extern bool VectorBase_Extend(VectorBase  *vector, uint32_t size);
extern void VectorBase_Clear(VectorBase  *vector);

///////////////////////////////////////////////////////////////////////////////
// NodeVector -- an expanding array of DFlowNode pointers
///////////////////////////////////////////////////////////////////////////////

typedef struct NodeVector_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} NodeVector;

typedef DFlowNode       **NodeVector_iterator;
typedef const DFlowNode **NodeVector_const_iterator;

// Functions
void NodeVector_Constr(NodeVector *self, uint32_t capacity);
void NodeVector_push_back(NodeVector *self, DFlowNode *node);

// Inline functions
static inline NodeVector *NodeVector_new(uint32_t capacity)
{
   NodeVector  *vec = (NodeVector *)bcg_glsl_malloc(sizeof(NodeVector));

   if (vec != NULL)
      NodeVector_Constr(vec, capacity);

   return vec;
}

static inline void NodeVector_Destr(NodeVector *self)
{
   VectorBase_Destr(&self->m_vector);
}

static inline void NodeVector_delete(NodeVector *self)
{
   if (self != NULL)
   {
      NodeVector_Destr(self);
      bcg_glsl_free(self);
   }
}

static inline void NodeVector_clear(NodeVector *self)
{
   self->m_end = 0;
   VectorBase_Clear(&self->m_vector);
}

static inline DFlowNode **NodeVector_lindex(NodeVector *self, uint32_t i)
{
   NodeVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (NodeVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static inline const DFlowNode **NodeVector_const_lindex(const NodeVector *self, uint32_t i)
{
   NodeVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (NodeVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static inline DFlowNode *NodeVector_index(NodeVector *self, uint32_t i)
{
   NodeVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (NodeVector_iterator)self->m_vector.data;

   return resultList[i];
}

static inline const DFlowNode *NodeVector_const_index(const NodeVector *self, uint32_t i)
{
   NodeVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (NodeVector_const_iterator)self->m_vector.data;

   return resultList[i];
}

static inline uint32_t NodeVector_size(const NodeVector *self)
{
   return self->m_end;
}

// Iterator
static inline NodeVector_iterator NodeVector_begin(NodeVector *self)
{
   NodeVector_iterator   data  = (NodeVector_iterator)self->m_vector.data;

   return &data[0];
}

static inline NodeVector_const_iterator NodeVector_const_begin(const NodeVector *self)
{
   NodeVector_const_iterator   data  = (NodeVector_const_iterator)self->m_vector.data;

   return &data[0];
}

static inline NodeVector_iterator NodeVector_end(NodeVector *self)
{
   NodeVector_iterator   data  = (NodeVector_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static inline NodeVector_const_iterator NodeVector_const_end(const NodeVector *self)
{
   NodeVector_const_iterator   data  = (NodeVector_const_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static inline void NodeVector_next(NodeVector_iterator *iter)
{
   (*iter)++;
}

static inline void NodeVector_const_next(NodeVector_const_iterator *iter)
{
   (*iter)++;
}

// Iterator methods
static inline DFlowNode *NodeVector_star(NodeVector_iterator self)
{
   return *self;
}

static inline const DFlowNode *NodeVector_const_star(NodeVector_const_iterator self)
{
   return *self;
}

///////////////////////////////////////////////////////////////////////////////
// DataflowVector -- an expanding array of Dataflow pointers
///////////////////////////////////////////////////////////////////////////////

typedef struct DataflowVector_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} DataflowVector;

typedef Dataflow       **DataflowVector_iterator;
typedef const Dataflow **DataflowVector_const_iterator;

// Functions
void DataflowVector_Constr(DataflowVector *self, uint32_t capacity);
void DataflowVector_push_back(DataflowVector *self, Dataflow *node);

// Inline functions
static inline void DataflowVector_Destr(DataflowVector *self)
{
   VectorBase_Destr(&self->m_vector);
}

static inline void DataflowVector_clear(DataflowVector *self)
{
   self->m_end = 0;
   VectorBase_Clear(&self->m_vector);
}

static inline Dataflow **DataflowVector_lindex(DataflowVector *self, uint32_t i)
{
   DataflowVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (DataflowVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static inline const Dataflow **DataflowVector_const_lindex(const DataflowVector *self, uint32_t i)
{
   DataflowVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (DataflowVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static inline Dataflow *DataflowVector_index(DataflowVector *self, uint32_t i)
{
   DataflowVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (DataflowVector_iterator)self->m_vector.data;

   return resultList[i];
}

static inline const Dataflow *DataflowVector_const_index(const DataflowVector *self, uint32_t i)
{
   DataflowVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (DataflowVector_const_iterator)self->m_vector.data;

   return resultList[i];
}

static inline uint32_t DataflowVector_size(const DataflowVector *self)
{
   return self->m_end;
}

// Iterator
static inline DataflowVector_iterator DataflowVector_begin(DataflowVector *self)
{
   DataflowVector_iterator   data  = (DataflowVector_iterator)self->m_vector.data;

   return &data[0];
}

static inline DataflowVector_const_iterator DataflowVector_const_begin(const DataflowVector *self)
{
   DataflowVector_const_iterator   data  = (DataflowVector_const_iterator)self->m_vector.data;

   return &data[0];
}

static inline DataflowVector_iterator DataflowVector_end(DataflowVector *self)
{
   DataflowVector_iterator   data  = (DataflowVector_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static inline DataflowVector_const_iterator DataflowVector_const_end(const DataflowVector *self)
{
   DataflowVector_const_iterator   data  = (DataflowVector_const_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static inline void DataflowVector_next(DataflowVector_iterator *iter)
{
   (*iter)++;
}

static inline void DataflowVector_const_next(DataflowVector_const_iterator *iter)
{
   (*iter)++;
}

// Iterator methods
static inline Dataflow *DataflowVector_star(DataflowVector_iterator self)
{
   return *self;
}

static inline const Dataflow *DataflowVector_const_star(DataflowVector_const_iterator self)
{
   return *self;
}

///////////////////////////////////////////////////////////////////////////////
// NodeList -- a linked list of vector pointers
///////////////////////////////////////////////////////////////////////////////

typedef struct NodeLink_s
{
   DFlowNode         *m_node;
   struct NodeLink_s *m_next;
} NodeLink;

typedef struct NodeList_s
{
   NodeLink    *m_head;
   NodeLink    *m_tail;
   uint32_t     m_size;
} NodeList;

typedef NodeLink        *NodeList_iterator;
typedef const NodeLink  *NodeList_const_iterator;
typedef bool (*NodeList_Pred)(const DFlowNode *l, const DFlowNode *r);

// Functions
void     NodeList_Destr(NodeList *self);
uint32_t NodeList_remove(NodeList *self, DFlowNode *node);
void     NodeList_sort(NodeList *self, NodeList_Pred cmp);
void     NodeList_insert(NodeList *self, DFlowNode *node, NodeList_Pred cmp);

// Inline functions
static inline void NodeList_Constr(NodeList *self)
{
   self->m_head = NULL;
   self->m_tail = NULL;
   self->m_size = 0;
}

static inline uint32_t NodeList_size(const NodeList *self)
{
   return self->m_size;
}

static inline DFlowNode *NodeList_star(NodeList_iterator self)
{
   return self->m_node;
}

static inline DFlowNode *NodeList_const_star(NodeList_const_iterator self)
{
   return self->m_node;
}

static inline void NodeList_push_back(NodeList *self, DFlowNode *node)
{
   NodeLink *link = (NodeLink *)bcg_glsl_malloc(sizeof(NodeLink));

   link->m_next = NULL;
   link->m_node = node;

   if (self->m_head == NULL)
      self->m_head = link;

   if (self->m_tail != NULL)
      self->m_tail->m_next = link;

   self->m_tail = link;
   self->m_size++;
}

static inline NodeList_iterator NodeList_begin(NodeList *self)
{
   return self->m_head;
}

static inline NodeList_const_iterator NodeList_const_begin(const NodeList *self)
{
   return self->m_head;
}

static inline NodeList_iterator NodeList_end(NodeList *self)
{
   UNUSED(self);
   return NULL;
}

static inline NodeList_const_iterator NodeList_const_end(const NodeList *self)
{
   UNUSED(self);
   return NULL;
}

static inline void NodeList_next(NodeList_iterator *iter)
{
   *iter = (*iter)->m_next;
}

static inline void NodeList_const_next(NodeList_const_iterator *iter)
{
   *iter = (*iter)->m_next;
}

static inline DFlowNode *NodeList_front(const NodeList *self)
{
   if (self->m_size == 0)
      return NULL;

   return self->m_head->m_node;
}

static inline void NodeList_pop(NodeList *self)
{
   NodeLink *head;

   if (self->m_size == 0)
      return;

   head = self->m_head;

   if (self->m_size == 1)
      self->m_tail = NULL;

   self->m_head = head->m_next;
   self->m_size--;

   bcg_glsl_free(head);
}

///////////////////////////////////////////////////////////////////////////////
// NodeSet -- a set of Node pointers ordered on unique id
///////////////////////////////////////////////////////////////////////////////

typedef struct NodeTree_s
{
   DFlowNode         *m_node;
   bool               m_erased;
   struct NodeTree_s *m_less;
   struct NodeTree_s *m_more;
} NodeTree;

typedef struct NodeSet_s
{
   uint32_t  m_size;
   NodeTree *m_tree;
} NodeSet;

void NodeSet_insert(NodeSet *self, DFlowNode *node);
void NodeSet_erase(NodeSet *self, DFlowNode *node);
void NodeSet_clear(NodeSet *self);
void NodeSet_Flatten(NodeSet *self, NodeVector *result);

static inline void NodeSet_Constr(NodeSet *self)
{
   self->m_size = 0;
   self->m_tree = NULL;
}

static inline void NodeSet_Destr(NodeSet *self)
{
   NodeSet_clear(self);
}

static inline uint32_t NodeSet_size(NodeSet *self)
{
   return self->m_size;
}

///////////////////////////////////////////////////////////////////////////////
// NodeVectorMap -- a map from int32_t to pointers to vectors of node pointers
///////////////////////////////////////////////////////////////////////////////

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_nodevectormap.h"

typedef NodeVectorMap_T NodeVectorMap;

static inline void NodeVectorMap_Constr(NodeVectorMap *self, uint32_t capacity)
{
   NodeVectorMap_init(self, capacity);
}

static inline void NodeVectorMap_Destr(NodeVectorMap *self)
{
   NodeVectorMap_term(self);
}
