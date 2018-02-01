/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_query.h"
#include "glxx_hw_render_state.h"
#include "../common/khrn_mem.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/util/assert_helpers.h"

struct glxx_query;

typedef struct glxx_query_block
{
   struct glxx_query_block* prev;
   gmem_handle_t handle;
   v3d_addr_t v3d_addr;
   uint32_t* cpu_addr;
   unsigned used_queries;

   glxx_instanced_query_t query[];
} glxx_query_block;

static glxx_query_block* new_query_block(
   khrn_fmem* fmem,
   size_t query_count,
   size_t mem_size,
   size_t mem_align,
   uint32_t bin_rw_flags,
   uint32_t render_rw_flags)
{
   size_t cpu_size = sizeof(glxx_query_block) + sizeof(glxx_instanced_query_t)*query_count;
   glxx_query_block* block = (glxx_query_block *)malloc(cpu_size);
   if (!block)
      return NULL;

   // Allocate dynamic memory (this will be quickly fulfilled by talloc on the BSG platform).
   block->handle = gmem_alloc_and_map(
      mem_size,
      mem_align,
      GMEM_USAGE_V3D_RW | GMEM_USAGE_HINT_DYNAMIC,
      "Query buffer");
   if (!block->handle)
      goto error;

   // Zero fill and sync.
   block->cpu_addr = (uint32_t*)gmem_get_ptr(block->handle);
   memset(block->cpu_addr, 0, mem_size);
   gmem_flush_mapped_buffer(block->handle);

   block->prev = NULL;
   block->v3d_addr = khrn_fmem_sync_and_get_addr(fmem, block->handle, bin_rw_flags, render_rw_flags);
   block->used_queries = 0;
   return block;

error:
   gmem_free(block->handle);
   free(block);
   return NULL;
}

/* this lock protects fields instance, and done_updates in a GLXX_QUERY_T;
 * we just have a lock for all the queries */
static VCOS_MUTEX_T queries_updates_lock;

bool glxx_queries_updates_lock_init(void)
{
   if (vcos_mutex_create(&queries_updates_lock, "queries_updates_lock") != VCOS_SUCCESS)
      return false;
   return true;
}


static bool query_init(GLXX_QUERY_T* query, unsigned name)
{
   memset(query, 0, sizeof(GLXX_QUERY_T));

   query->name = name;
   query->target = GL_NONE;
   query->debug_label = NULL;
   return true;
}

static void query_term(void *v)
{
   GLXX_QUERY_T *query = (GLXX_QUERY_T *)v;

   free(query->debug_label);
   query->debug_label = NULL;
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
   switch(target)
   {
      case GL_ANY_SAMPLES_PASSED:
      case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
         return GLXX_Q_OCCLUSION;
      case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
         return GLXX_Q_PRIM_WRITTEN;
      case GL_PRIMITIVES_GENERATED:
         return GLXX_Q_PRIM_GEN;
      default:
         unreachable();
         return GLXX_Q_COUNT;
   }
}

bool glxx_query_begin_new_instance(GLXX_QUERY_T *query, enum glxx_query_target target)
{
   enum glxx_query_type type = glxx_query_target_to_type(target);
   if (query->target != GL_NONE && query->type != type)
      return false;

   assert(query->target != GL_NONE || query->instance == 0);

   query->target = target;
   query->type = type;

   vcos_mutex_lock(&queries_updates_lock);

   query->result = 0;
   query->required_updates = 0;
   query->done_updates = 0;
   query->prim_drawn_by_us = 0;
   query->instance++;

   vcos_mutex_unlock(&queries_updates_lock);

   return true;
}

static v3d_addr_t occlusion_query_counter_hw_addr(khrn_fmem *fmem, GLXX_QUERY_T *query)
{
   khrn_fmem_persist *persist = fmem->persist;
   glxx_query_block *block = persist->occlusion_query_list;

   // Could allocate a block for more than 16 counters at a time.
   if (block == NULL || block->used_queries == V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS)
   {
      block = new_query_block(
         fmem,
         V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS,
         V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE * V3D_MAX_CORES,
         V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN,
         0,
         V3D_BARRIER_TLB_OQ_READ | V3D_BARRIER_TLB_OQ_WRITE);
      if (!block)
         return 0;

      block->prev = persist->occlusion_query_list;
      persist->occlusion_query_list = block;
   }

   unsigned index = block->used_queries++;
   khrn_mem_acquire(query);
   block->query[index].query = query;
   block->query[index].instance = query->instance;
   query->required_updates++;

   return block->v3d_addr + sizeof(uint32_t)*index;
}

/* query can be NULL */
static bool instanced_query_equal_with_query(glxx_instanced_query_t *iquery,
      GLXX_QUERY_T *query)
{
   assert(iquery);
   bool res = iquery->query == query && iquery->instance == (query ? query->instance :0);
   return res;
}

/* if query is NULL, disable oclusion queries  on that rs */
bool glxx_occlusion_query_record(GLXX_HW_RENDER_STATE_T *rs, GLXX_QUERY_T *query)
{
   assert(!query || (query->type == GLXX_Q_OCCLUSION));

   glxx_instanced_query_t *iquery = &rs->last_started_query[GLXX_Q_OCCLUSION];

   if (instanced_query_equal_with_query(iquery, query))
      return true;

   v3d_addr_t hw_addr = 0;
   if (query != NULL)
   {
      hw_addr = occlusion_query_counter_hw_addr(&rs->fmem, query);
      if (!hw_addr)
         return false;
   }

   uint8_t *instr = glxx_hw_render_state_begin_cle(rs, GLXX_CL_STATE_OCCLUSION_QUERY);
   if (!instr)
      return false;

   v3d_cl_occlusion_query_counter_enable(&instr, hw_addr);

   glxx_hw_render_state_end_cle(rs, GLXX_CL_STATE_OCCLUSION_QUERY, instr);

   KHRN_MEM_ASSIGN(rs->last_started_query[GLXX_Q_OCCLUSION].query, query);
   rs->last_started_query[GLXX_Q_OCCLUSION].instance = query ? query->instance : 0;
   return true;
}

void glxx_occlusion_queries_update(glxx_query_block *query_list, bool valid_results)
{
   vcos_mutex_lock(&queries_updates_lock);

   unsigned num_cores = valid_results ? khrn_get_num_cores() : 0;

   for (glxx_query_block* b = query_list; b; b = b->prev)
   {
      if (valid_results)
         gmem_invalidate_mapped_buffer(b->handle);

      for (unsigned i = 0; i < b->used_queries; ++i)
      {
         GLXX_QUERY_T *query = b->query[i].query;
         if (b->query[i].instance == query->instance)
         {
            query->done_updates++;

            for (unsigned c = 0; c != num_cores; ++c)
               query->result += b->cpu_addr[c * V3D_OCCLUSION_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_COUNTERS + i];
         }
      }
   }

   vcos_mutex_unlock(&queries_updates_lock);
}

void glxx_queries_release(glxx_query_block *query_list)
{
   for (glxx_query_block *b = query_list; b; )
   {
      for (unsigned i = 0; i < b->used_queries; ++i)
         KHRN_MEM_ASSIGN(b->query[i].query, NULL);

      gmem_free(b->handle);

      glxx_query_block* next = b->prev;
      free(b);
      b = next;
   }
}


#if V3D_VER_AT_LEAST(4,1,34,0)

static v3d_addr_t primitive_counts_hw_addr(
   khrn_fmem *fmem,
   glxx_instanced_query_t *iquery_pg,
   glxx_instanced_query_t *iquery_pw)
{
   khrn_fmem_persist *persist = fmem->persist;
   glxx_query_block *block = persist->prim_counts_query_list;

   unsigned const num_queries = 64; // 1kb
   if (block == NULL || block->used_queries == num_queries)
   {
      block = new_query_block(
         fmem,
         num_queries,
         num_queries/2 * (sizeof(uint32_t) * 8u),  // 2 per block of 8.
         V3D_PRIM_COUNTS_ALIGN,
         V3D_BARRIER_PTB_PCF_WRITE,
         0);
      if (!block)
         return 0;

      block->prev = persist->prim_counts_query_list;
      persist->prim_counts_query_list = block;
   }

   unsigned pair_index = block->used_queries;
   block->used_queries += 2;

   glxx_instanced_query_t* iquery[2];
   iquery[0] = iquery_pg;
   iquery[1] = iquery_pw;

   for (unsigned i = 0, b = pair_index; i != 2; ++i, ++b)
   {
      struct glxx_query* query = iquery[i]->query;
      block->query[b].query = query;

      if (query)
      {
         khrn_mem_acquire(query);
         block->query[b].instance = iquery[i]->instance;

         if (query->instance == iquery[i]->instance)
            query->required_updates++;
      }
   }

   return block->v3d_addr + sizeof(uint32_t)*8u*pair_index;
}

bool glxx_write_prim_counts_feedback(GLXX_HW_RENDER_STATE_T *rs)
{
   v3d_addr_t hw_addr = 0;
   glxx_instanced_query_t *iquery_pg, *iquery_pw;

   iquery_pg = &rs->last_started_query[GLXX_Q_PRIM_GEN];
   iquery_pw = &rs->last_started_query[GLXX_Q_PRIM_WRITTEN];

   if (iquery_pg->query != NULL || iquery_pw->query != NULL)
   {
      hw_addr = primitive_counts_hw_addr(&rs->fmem, iquery_pg, iquery_pw);
      if (!hw_addr)
         return false;
   }

   uint8_t *instr = khrn_fmem_cle(&rs->fmem, V3D_CL_PRIM_COUNTS_FEEDBACK_SIZE);
   if (!instr)
      return false;

   v3d_cl_prim_counts_feedback(&instr, V3D_PCF_OPERATION_ST_PRIM_COUNTS_AND_ZERO, false, hw_addr);
   if (hw_addr != 0)
      glxx_tf_incr_start_count(rs);

   return true;
}

bool glxx_prim_counts_query_record(GLXX_HW_RENDER_STATE_T *rs,
      GLXX_QUERY_T *query_pg, GLXX_QUERY_T *query_pw)
{
   assert(!query_pg ||(query_pg->type == GLXX_Q_PRIM_GEN));
   assert(!query_pw ||(query_pw->type == GLXX_Q_PRIM_WRITTEN));

   glxx_instanced_query_t *iquery_pg, *iquery_pw;

   iquery_pg = &rs->last_started_query[GLXX_Q_PRIM_GEN];
   iquery_pw = &rs->last_started_query[GLXX_Q_PRIM_WRITTEN];

   if (instanced_query_equal_with_query(iquery_pg, query_pg) &&
       instanced_query_equal_with_query(iquery_pw, query_pw))
      return true;  //nothing new

   /* write prim counts of previous queries (can be NULL) and also zero the
    * counters after they are written */
   if (!glxx_write_prim_counts_feedback(rs))
      return false;

   KHRN_MEM_ASSIGN(iquery_pg->query, query_pg);
   iquery_pg->instance = query_pg ? query_pg->instance : 0;
   KHRN_MEM_ASSIGN(iquery_pw->query, query_pw);
   iquery_pw->instance = query_pw ? query_pw->instance : 0;
   return true;
}

void glxx_prim_counts_queries_update(glxx_query_block *query_list, bool valid_results)
{
   vcos_mutex_lock(&queries_updates_lock);

   for (glxx_query_block* b = query_list; b; b = b->prev)
   {
      if (valid_results)
         gmem_invalidate_mapped_buffer(b->handle);

      for (unsigned i = 0; i < b->used_queries; i ++)
      {
         GLXX_QUERY_T *query = b->query[i].query;
         if (query && (b->query[i].instance == query->instance))
         {
            query->done_updates++;

            if (valid_results)
            {
               unsigned pair_index = i / 2;
               unsigned sub_index = i & 1;
               query->result += b->cpu_addr[pair_index*8 + 4 + sub_index]; // the first 4 words are
                                                                           // the count of words sucesfully written to buffer 0 -3
            }
         }
      }
   }

   vcos_mutex_unlock(&queries_updates_lock);
}

void glxx_prim_drawn_by_us_record(GLXX_HW_RENDER_STATE_T *rs, unsigned no_prim)
{
   if (rs->last_started_query[GLXX_Q_PRIM_GEN].query != NULL)
   {
      vcos_mutex_lock(&queries_updates_lock);
      rs->last_started_query[GLXX_Q_PRIM_GEN].query->prim_drawn_by_us += no_prim;
      vcos_mutex_unlock(&queries_updates_lock);
   }
}

#endif

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
   else if (query->type == GLXX_Q_PRIM_GEN)
   {
      assert(query->result >= query->prim_drawn_by_us);
      res = query->result - query->prim_drawn_by_us;
   }
   else
      res = query->result;

   vcos_mutex_unlock(&queries_updates_lock);

   return res;
}
