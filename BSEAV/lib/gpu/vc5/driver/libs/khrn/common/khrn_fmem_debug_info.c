/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifdef KHRN_GEOMD

#include "khrn_fmem.h"
#include "khrn_options.h"
#include "vcos.h"
#include "libs/util/log/log.h"

#define DEBUG_INFO_VECTOR_INITIAL_CAPACITY 500
#define MAX_DEBUG_INFO_VECTORS 64

static struct fmem_debug_info_vector const *s_debug_infos[MAX_DEBUG_INFO_VECTORS];
static unsigned s_num_debug_infos;
static VCOS_MUTEX_T s_debug_info_lock;

bool fmem_debug_info_init_process(void)
{
   return vcos_mutex_create(&s_debug_info_lock, "GL_DEBUG_INFO") == VCOS_SUCCESS;
}

void fmem_debug_info_term_process(void)
{
   vcos_mutex_delete(&s_debug_info_lock);
}

void fmem_debug_info_init(struct fmem_debug_info_vector *info)
{
   if (!khrn_options.geomd)
      return;

   info->capacity = DEBUG_INFO_VECTOR_INITIAL_CAPACITY;
   info->size = 0;
   info->sorted = false;
   info->v = malloc(info->capacity * sizeof(struct fmem_debug_info));
   info->renderstate_id = ~0u;
   if (info->v == NULL)
      info->capacity = 0;
}

void fmem_debug_info_deinit(struct fmem_debug_info_vector *info)
{
   if (!khrn_options.geomd)
      return;

   vcos_mutex_lock(&s_debug_info_lock);

   for (unsigned i = 0; i < s_num_debug_infos; ++i)
   {
      if (s_debug_infos[i] == info)
      {
         assert(s_num_debug_infos > 0);
         s_debug_infos[i] = s_debug_infos[--s_num_debug_infos];
      }
   }

   vcos_mutex_unlock(&s_debug_info_lock);

   free(info->v);
   info->capacity = 0;
   info->size = 0;
   info->sorted = false;

   assert(s_num_debug_infos < MAX_DEBUG_INFO_VECTORS);
}

void fmem_debug_info_insert(khrn_fmem *fmem, v3d_addr_t addr,
                            unsigned draw_id, unsigned renderstate_id)
{
   if (!khrn_options.geomd)
      { return; }

   struct fmem_debug_info_vector *info = &fmem->persist->debug_info;
   if (info->capacity == info->size)
   {
      unsigned new_capacity = info->capacity * 2;
      struct fmem_debug_info *v_new = malloc(new_capacity * sizeof(struct fmem_debug_info));
      if (v_new == NULL)
         return;

      memcpy(v_new, info->v, info->capacity * sizeof(struct fmem_debug_info));
      free(info->v);
      info->v = v_new;
      info->capacity = new_capacity;
   }
   info->v[info->size].addr = addr;
   info->v[info->size].draw_id = draw_id;
   info->renderstate_id = renderstate_id;
   assert(info->sorted == false);
   ++info->size;
}

static int compare_debug_info(const void *p1, const void *p2)
{
   const struct fmem_debug_info *a = p1;
   const struct fmem_debug_info *b = p2;

   if (a->addr < b->addr)
      return -1;

   if (a->addr == b->addr)
      return 0;

   return 1;
}

void fmem_debug_info_prepare_for_queries(struct fmem_debug_info_vector *info)
{
   if (!khrn_options.geomd)
      return;

   vcos_mutex_lock(&s_debug_info_lock);

   assert(info->sorted == false);
   qsort(info->v, info->size, sizeof(struct fmem_debug_info), compare_debug_info);
   info->sorted = true;

   assert(s_num_debug_infos < MAX_DEBUG_INFO_VECTORS);
   s_debug_infos[s_num_debug_infos] = info;
   s_num_debug_infos++;

   vcos_mutex_unlock(&s_debug_info_lock);
}

bool fmem_debug_info_query(v3d_addr_t addr, unsigned *rs_id, unsigned *draw_id)
{
   if (!khrn_options.geomd)
      return false;

   vcos_mutex_lock(&s_debug_info_lock);

   bool res = false;
   for (unsigned i = 0; i < s_num_debug_infos; ++i)
   {
      struct fmem_debug_info_vector const *info = s_debug_infos[i];
      int first = 0;
      int last = info->size - 1;
      int middle = (first + last) / 2;

      assert(info->sorted == true);

      while (first <= last)
      {
         if (info->v[middle].addr == addr)
         {
            *rs_id   = info->renderstate_id;
            *draw_id = info->v[middle].draw_id;
            res = true;
            goto unlock_out;
         }
         else if (info->v[middle].addr < addr)
         {
            first = middle + 1;
         }
         else
         {
            last = middle - 1;
         }

         middle = (first + last) / 2;
      }
   }

unlock_out:
   vcos_mutex_unlock(&s_debug_info_lock);
   return res;
}

#endif