/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#define KHRN_MAP_C
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"

// For memcpy
#include "middleware/khronos/common/khrn_mem.h"

///////////////////////////////////////////////////////////////////////////////
// VectorBase
///////////////////////////////////////////////////////////////////////////////

void VectorBase_Constr(VectorBase *vector, uint32_t capacity)
{
   vector->data     = (capacity == 0) ? NULL : bcg_glsl_malloc(capacity);
   vector->capacity = vector->data ? capacity : 0;
   vector->size     = 0;
}

void VectorBase_Destr(VectorBase *vector)
{
   if (vector->data)
      bcg_glsl_free(vector->data);
}

bool VectorBase_Extend(VectorBase *vector, uint32_t size)
{
   uint32_t req_capacity = vector->size + size;
   if (req_capacity > vector->capacity)
   {
      uint32_t new_capacity = _max(vector->capacity + (vector->capacity >> 1), req_capacity);

      void *new_data = bcg_glsl_malloc(new_capacity);

      if (vector->data)
      {
         khrn_memcpy(new_data, vector->data, vector->size);
         bcg_glsl_free(vector->data);
      }

      vector->data = new_data;
      vector->capacity = new_capacity;
   }

   vector->size += size;
   return true;
}

void VectorBase_Clear(VectorBase *vector)
{
   // Resets the size so that memory can be reused.
   vector->size     = 0;
}

///////////////////////////////////////////////////////////////////////////////
// NodeVector -- an expanding array of DFlowNode pointers
///////////////////////////////////////////////////////////////////////////////

void NodeVector_Constr(NodeVector *self, uint32_t capacity)
{
   self->m_end  = 0;
   VectorBase_Constr(&self->m_vector, sizeof(DFlowNode *) * capacity);
}


void NodeVector_push_back(NodeVector *self, DFlowNode *node)
{
   if (VectorBase_Extend(&self->m_vector, sizeof(DFlowNode *)))
   {
      DFlowNode   **resultList = (DFlowNode **)self->m_vector.data;
      uint32_t    i = self->m_end;

      self->m_end++;
      resultList[i] = node;
   }
}

///////////////////////////////////////////////////////////////////////////////
// DataflowVector -- an expanding array of Dataflow pointers
///////////////////////////////////////////////////////////////////////////////

void DataflowVector_Constr(DataflowVector *self, uint32_t capacity)
{
   self->m_end  = 0;
   VectorBase_Constr(&self->m_vector, sizeof(DFlowNode *) * capacity);
}


void DataflowVector_push_back(DataflowVector *self, Dataflow *node)
{
   if (VectorBase_Extend(&self->m_vector, sizeof(Dataflow *)))
   {
      Dataflow **resultList = (Dataflow **)self->m_vector.data;
      uint32_t   i = self->m_end;

      self->m_end++;
      resultList[i] = node;
   }
}

///////////////////////////////////////////////////////////////////////////////
// NodeList -- a linked list of vector pointers
///////////////////////////////////////////////////////////////////////////////
void NodeList_Destr(NodeList *self)
{
   NodeList_iterator iter;
   NodeList_iterator next = NULL;

   for (iter = self->m_head; iter != NULL; iter = next)
   {
      next = iter->m_next;
      bcg_glsl_free(iter);
   }

   // Reset for safety
   NodeList_Constr(self);
}

uint32_t NodeList_remove(NodeList *self, DFlowNode *node)
{
   uint32_t          numRemoved = 0;
   NodeList_iterator iter       = self->m_head;
   NodeList_iterator prev       = NULL;
   NodeList_iterator next       = NULL;

   for (iter = NodeList_begin(self); iter != NULL; iter = next)
   {
      next = iter->m_next;

      if (iter->m_node == node)
      {
         if (prev != NULL)
            prev->m_next = next;

         if (self->m_head == iter)
            self->m_head = next;

         self->m_size--;

         bcg_glsl_free(iter);
         numRemoved++;
      }
      else
      {
         prev = iter;
      }

      iter = next;
   }

   self->m_tail = prev;

   return numRemoved;
}

static void Insert(NodeLink *link, NodeList *list, NodeList_Pred cmp)
{
   NodeLink **prevPtr = &list->m_head;
   NodeLink *next     = list->m_head;

   if (next == NULL)
   {
      // Insert into empty list
      list->m_head = link;
      list->m_tail = link;
      list->m_size = 1;
      link->m_next = NULL;
   }
   else
   {
      // Find insertion point
      while (next != NULL && !cmp(link->m_node, next->m_node))
      {
         prevPtr = &next->m_next;
         next    = next->m_next;
      }

      // Reached end of list, so update tail
      if (next == NULL)
         list->m_tail = link;

      // Link in the link
      *prevPtr = link;
      link->m_next = next;
      list->m_size++;
   }
}

// In place sort via insert sort
void NodeList_sort(NodeList *self, NodeList_Pred cmp)
{
   NodeList result;
   NodeList_Constr(&result);

   while (self->m_size != 0)
   {
      NodeLink *item = self->m_head;

      self->m_head = self->m_head->m_next;
      self->m_size--;

      Insert(item, &result, cmp);
   }

   *self = result;
}

// Insert a node into a sorted list
void NodeList_insert(NodeList *self, DFlowNode *node, NodeList_Pred cmp)
{
   NodeLink *link = (NodeLink *)bcg_glsl_malloc(sizeof(NodeLink));

   link->m_next = NULL;
   link->m_node = node;

   Insert(link, self, cmp);
}

///////////////////////////////////////////////////////////////////////////////
// NodeSet -- a set (tree) of Node pointers ordered on unique id
///////////////////////////////////////////////////////////////////////////////

static void NodeTree_Constr(NodeTree *self, DFlowNode *node, NodeTree *less, NodeTree *more)
{
   self->m_erased = false;
   self->m_node   = node;
   self->m_less   = less;
   self->m_more   = more;
}

static NodeTree *NodeTree_new(DFlowNode *node, NodeTree *less, NodeTree *more)
{
   NodeTree *self = (NodeTree *)bcg_glsl_malloc(sizeof(NodeTree));

   if (self != NULL)
      NodeTree_Constr(self, node, less, more);

   return self;
}

static void NodeTree_delete(NodeTree *self)
{
   if (self != NULL)
      bcg_glsl_free(self);
}

static bool NodeTree_insert(NodeTree *self, NodeTree **selfPtr, DFlowNode *node)
{
   bool  inserted = false;

   if (self == NULL)
   {
      *selfPtr = NodeTree_new(node, NULL, NULL);
      inserted = true;
   }
   else
   {
      uint32_t    selfId = DFlowNode_UniqueId(self->m_node);
      uint32_t    id     = DFlowNode_UniqueId(node);

      if (id == selfId)
      {
         self->m_erased = false;      // It might have been erased, so unerase
         inserted       = false;
      }
      else if (id > selfId)
      {
         inserted = NodeTree_insert(self->m_more, &self->m_more, node);
      }
      else
      {
         inserted = NodeTree_insert(self->m_less, &self->m_less, node);
      }
   }

   return inserted;
}

void NodeSet_insert(NodeSet *self, DFlowNode *node)
{
   bool inserted = NodeTree_insert(self->m_tree, &self->m_tree, node);

   if (inserted)
      self->m_size++;
}

static bool NodeTree_erase(NodeTree *self, uint32_t id)
{
   bool  erased = false;

   if (self != NULL)
   {
      uint32_t selfId = DFlowNode_UniqueId(self->m_node);

      if (id == selfId)
      {
         if (!self->m_erased)
         {
            self->m_erased = true;
            erased = true;
         }
      }
      else if (id > selfId)
      {
         erased = NodeTree_erase(self->m_more, id);
      }
      else
      {
         erased = NodeTree_erase(self->m_less, id);
      }
   }

   return erased;
}

void NodeSet_erase(NodeSet *self, DFlowNode *node)
{
   bool erased = NodeTree_erase(self->m_tree, DFlowNode_UniqueId(node));

   if (erased)
      self->m_size--;
}

static void NodeTree_clear(NodeTree *self)
{
   if (self == NULL)
      return;

   NodeTree_clear(self->m_less);
   NodeTree_clear(self->m_more);
   NodeTree_delete(self);
}

void NodeSet_clear(NodeSet *self)
{
   NodeTree_clear(self->m_tree);
   self->m_size = 0;
   self->m_tree = NULL;
}

static void NodeTree_Flatten(NodeTree *self, NodeVector *result)
{
   if (self == NULL)
      return;

   NodeTree_Flatten(self->m_less, result);

   if (!self->m_erased)
      NodeVector_push_back(result, self->m_node);

   NodeTree_Flatten(self->m_more, result);
}

void NodeSet_Flatten(NodeSet *self, NodeVector *result)
{
   NodeTree_Flatten(self->m_tree, result);
}

///////////////////////////////////////////////////////////////////////////////
// NodeVectorMap -- a map from int32_t to pointers to vectors of node pointers
///////////////////////////////////////////////////////////////////////////////

void *NodeVectorMap_Alloc(uint32_t size, const char *ident)
{
   return bcg_glsl_malloc(size);
}

void  NodeVectorMap_Free(void *map)
{
   bcg_glsl_free(map);
}

// Rest of implementation already included
