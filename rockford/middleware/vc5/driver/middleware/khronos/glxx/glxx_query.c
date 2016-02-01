/*=============================================================================
Copyright (c) 20014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  async queries

FILE DESCRIPTION
asynchronous queries implementation.
=============================================================================*/
#include "middleware/khronos/glxx/glxx_query.h"
#include "middleware/khronos/glxx/glxx_hw_render_state.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "helpers/v3d/v3d_gen.h"

/* this lock protects fields instance, and done_updates in a GLXX_QUERY_T;
 * we just have a lock for all the queries */
static VCOS_MUTEX_T queries_updates_lock;

bool glxx_queries_updates_lock_init(void)
{
   if (vcos_mutex_create(&queries_updates_lock, "queries_updates_lock") != VCOS_SUCCESS)
      return false;
   return true;
}


bool query_init(GLXX_QUERY_T* query, unsigned name)
{
   memset(query, 0, sizeof(GLXX_QUERY_T));

   query->name = name;
   query->target = GL_NONE;
   query->debug_label = NULL;
   return true;
}

void query_term(void *v, size_t size)
{
   GLXX_QUERY_T *query = (GLXX_QUERY_T *)v;

   free(query->debug_label);
   query->debug_label = NULL;

   UNUSED(size);
}

GLXX_QUERY_T* glxx_query_create(unsigned name)
{
   GLXX_QUERY_T *query = KHRN_MEM_ALLOC_STRUCT(GLXX_QUERY_T);

   if (!query)
      return NULL;

   if (!query_init(query, name))
   {
      KHRN_MEM_ASSIGN(query, NULL);
      return NULL;
   }

   khrn_mem_set_term(query, query_term);
   return query;
}

enum glxx_query_type glxx_query_target_to_type(enum glxx_query_target target)
{
   enum glxx_query_type type;

   assert(target != GL_NONE);

   switch(target)
   {
      case GL_ANY_SAMPLES_PASSED:
      case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
         type = GLXX_Q_OCCLUSION;
         break;
      case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
         type = GLXX_Q_TRANSF_FEEDBACK;
         break;
      default:
         UNREACHABLE();
   }
   return type;
}

bool glxx_query_begin_new_instance(GLXX_QUERY_T *query, enum glxx_query_target target)
{
   if (query->target == GL_NONE)
   {
      /* this the first time glBeginQuery gets called on this object --> set
      * target */
      query->target = target;
      query->type = glxx_query_target_to_type(query->target);
      assert(query->instance == 0);
   }
   else if (query->target != target)
      return false;

   vcos_mutex_lock(&queries_updates_lock);

   query->result = 0;
   query->required_updates = 0;
   query->done_updates = 0;
   query->instance++;

   vcos_mutex_unlock(&queries_updates_lock);

   return true;
}

v3d_addr_t query_counter_hw_addr(KHRN_FMEM_T *fmem, GLXX_QUERY_T *query)
{
   KHRN_QUERY_BLOCK_T *block;
   unsigned index;

   khrn_fmem_new_query_entry(fmem, &block, &index);
   if (block == NULL)
      return 0;;

   uint32_t *qv = &block->query_values[index];
   *qv = 0;

   for (uint32_t core = 1; core != khrn_get_num_cores_uncapped(); ++core)
      qv[core * V3D_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS] = 0;

   khrn_mem_acquire(query);
   block->query[index].obj = query;
   block->query[index].instance = query->instance;

   query->required_updates++;

   return khrn_fmem_hw_address(fmem, qv);
}

/* if query is NULL, disable oclusion queries  on that rs */
static bool record_occlusion_query(GLXX_HW_RENDER_STATE_T *rs, GLXX_QUERY_T *query)
{
   v3d_addr_t hw_addr = 0;
   uint8_t *instr;
   uint64_t instance;

   instance = 0;
   if (query)
      instance = query->instance;

   if (rs->last_occlusion_query.query == query &&
       rs->last_occlusion_query.instance == instance)
      return true;

   if (query != NULL)
   {
      hw_addr = query_counter_hw_addr(&rs->fmem, query);
      if (!hw_addr)
         return false;
   }

   instr = glxx_hw_render_state_begin_cle(rs, GLXX_CL_STATE_OCCLUSION_QUERY);
   if (!instr)
      return false;

   v3d_cl_occlusion_query_counter_enable(&instr, hw_addr);

   glxx_hw_render_state_end_cle(rs, GLXX_CL_STATE_OCCLUSION_QUERY, instr);

   KHRN_MEM_ASSIGN(rs->last_occlusion_query.query, query);
   rs->last_occlusion_query.instance = instance;
   return true;
}

bool glxx_query_enable(GLXX_HW_RENDER_STATE_T *rs, GLXX_QUERY_T *query)
{
   bool res;
   assert(query);
   assert(query->target != GL_NONE);

   switch (query->type)
   {
      case GLXX_Q_OCCLUSION:
         res = record_occlusion_query(rs, query);
         break;
      case GLXX_Q_TRANSF_FEEDBACK:
         res= true;
         break;
      default:
         UNREACHABLE();
         res = false;
   }
   return res;
}

bool glxx_query_disable(GLXX_HW_RENDER_STATE_T *rs,
      enum glxx_query_type type)
{
   bool res;

   switch (type)
   {
      case GLXX_Q_OCCLUSION:
         res = record_occlusion_query(rs, NULL);
         break;
      case GLXX_Q_TRANSF_FEEDBACK:
         res = true;
         break;
      default:
         UNREACHABLE();
         res = false;
   }
   return res;
}

unsigned glxx_query_get_result(GLXX_QUERY_T *query)
{
   unsigned res;

   vcos_mutex_lock(&queries_updates_lock);

   assert(query->required_updates == query->done_updates);

   if (query->type == GLXX_Q_OCCLUSION)
   {
      if (query->result != 0)
         res = GL_TRUE;
      else
         res = GL_FALSE;
   }
   else
      res = query->result;

   vcos_mutex_unlock(&queries_updates_lock);

   return res;
}


void glxx_queries_update(KHRN_QUERY_BLOCK_T *query_list, bool valid_results)
{
   KHRN_QUERY_BLOCK_T *b;
   unsigned i;
   unsigned core;

   b = query_list;
   vcos_mutex_lock(&queries_updates_lock);
   while (b)
   {
      for (i = 0; i < b->count; ++i)
      {
         GLXX_QUERY_T *query = b->query[i].obj;
         if (b->query[i].instance == query->instance)
         {
            query->done_updates++;

            if (valid_results)
            {
               query->result += b->query_values[i];
               /* add the query values from different cores */
               for (core = 1; core < khrn_get_num_cores_uncapped(); core++)
                  query->result += b->query_values[core * V3D_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS + i];
            }
         }
      }
      b = b->prev;
   }
   vcos_mutex_unlock(&queries_updates_lock);
}
