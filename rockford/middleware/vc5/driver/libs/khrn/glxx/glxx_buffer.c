/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 1.1 vertex buffer structure.
=============================================================================*/

#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"

#include "libs/platform/gmem.h"
#include "../common/khrn_render_state.h"
#include "../common/khrn_process.h"
#include "glxx_buffer.h"
#include "glxx_hw_render_state.h"
#include "libs/core/v3d/v3d_align.h"

static const khrn_res_interlock_gmem_args c_gmem_args =
{
   .align = V3D_ATTR_REC_ALIGN,
   .usage = GMEM_USAGE_ALL,
   .desc = "GLXX_BUFFER_T.resource"
};

static inline unsigned glxx_translate_access_flags(GLbitfield access)
{
   static_assrt(KHRN_MAP_READ_BIT                == GL_MAP_READ_BIT);
   static_assrt(KHRN_MAP_WRITE_BIT               == GL_MAP_WRITE_BIT);
   static_assrt(KHRN_MAP_INVALIDATE_RANGE_BIT    == GL_MAP_INVALIDATE_RANGE_BIT);
   static_assrt(KHRN_MAP_INVALIDATE_BUFFER_BIT   == GL_MAP_INVALIDATE_BUFFER_BIT);
   static_assrt(KHRN_MAP_FLUSH_EXPLICIT_BIT      == GL_MAP_FLUSH_EXPLICIT_BIT);
   static_assrt(KHRN_MAP_UNSYNCHRONIZED_BIT      == GL_MAP_UNSYNCHRONIZED_BIT);
   return access;
}

void glxx_buffer_init(GLXX_BUFFER_T *buffer, uint32_t name)
{
   assert(buffer);

   buffer->name = name;
   buffer->usage = GL_STATIC_DRAW;
   buffer->mapped_pointer = NULL;
   buffer->mapped_size = 0;
   buffer->mapped_offset = 0;
   buffer->mapped_access_flags = 0;
   buffer->resource = NULL;
   buffer->size = 0;
   buffer->debug_label = NULL;

   buffer->last_tf_write_count = 0;

   buffer->enabled = false;
}

void glxx_buffer_enable(GLXX_BUFFER_T *buffer)
{
   assert(!buffer->enabled);
   buffer->enabled = true;
}

// Only called from memory manager internals during destruction of a GLXX_BUFFER_T
void glxx_buffer_term(void *v, size_t size)
{
   GLXX_BUFFER_T *buffer = (GLXX_BUFFER_T *)v;
   UNUSED(size);

   free(buffer->debug_label);
   buffer->debug_label = NULL;

   if (buffer->mapped_pointer != NULL)
      glxx_buffer_unmap_range(buffer, buffer->mapped_offset,
         buffer->mapped_size, buffer->mapped_access_flags);

   khrn_res_interlock_refdec(buffer->resource);
}

void* glxx_buffer_map_range(GLXX_BUFFER_T* buffer, size_t offset, size_t size, GLbitfield access)
{
   return khrn_res_interlock_map_range(&buffer->resource, offset, size, glxx_translate_access_flags(access), &c_gmem_args);
}

void glxx_buffer_unmap_range(GLXX_BUFFER_T *buffer, size_t offset, size_t size, GLbitfield access)
{
   khrn_res_interlock_unmap_range(buffer->resource, offset, size, glxx_translate_access_flags(access));
}

/*
 * Set a buffer object's size, contents and usage.
 * As the resource associated with the buffer is always the size of the buffer, we reallocate
 * even when shrinking. Thus making a buffer smaller can lead to GL_OUT_OF_MEMORY.
 * Returns true if successful or false if out of memory
 */
bool glxx_buffer_data(GLXX_BUFFER_T *buffer, size_t size, const void *data, GLenum usage)
{
   assert(buffer->enabled);
   // resize?
   if (size != buffer->size)
   {
      // release existing resource (if any)
      khrn_res_interlock_refdec(buffer->resource);
      buffer->resource = NULL;
      buffer->size = 0;

      // all done if size 0
      if (size > 0)
      {
         // attempt to allocate new resource, or fail
         buffer->resource = khrn_res_interlock_create(size, c_gmem_args.align, c_gmem_args.usage, c_gmem_args.desc);
         if (!buffer->resource)
            return false;
         buffer->size = size;
      }
   }

   // update usage
   buffer->usage = usage;

   if (data == NULL || size == 0)
      return true;

   return glxx_buffer_subdata(buffer, 0, size, data);
}

bool glxx_buffer_subdata(GLXX_BUFFER_T *buffer, size_t offset, size_t size, const void *data)
{
   assert(buffer->enabled);
   assert(buffer->resource != NULL && data != NULL);
   assert(size > 0 && (offset + size) <= buffer->size);

   void* ptr = khrn_res_interlock_map_range(&buffer->resource, offset, size, KHRN_MAP_WRITE_BIT | KHRN_MAP_INVALIDATE_RANGE_BIT, &c_gmem_args);
   if (!ptr)
      return false;
   memcpy(ptr, data, size);
   khrn_res_interlock_unmap_range(buffer->resource, offset, size, KHRN_MAP_WRITE_BIT);
   return true;
}

bool glxx_buffer_copy_subdata(
   GLXX_BUFFER_T *read_buffer,
   GLXX_BUFFER_T *write_buffer,
   size_t read_offset,
   size_t write_offset,
   size_t size
   )
{
   assert(read_buffer->enabled);

   void* read_ptr = khrn_res_interlock_map_range(&read_buffer->resource, read_offset, size, KHRN_MAP_READ_BIT, NULL);
   if (!read_ptr)
      return false;
   bool ok = glxx_buffer_subdata(write_buffer, write_offset, size, read_ptr);
   khrn_res_interlock_unmap_range(read_buffer->resource, read_offset, size, KHRN_MAP_READ_BIT);
   return ok;
}

int glxx_buffer_find_max(
   GLXX_BUFFER_T *buffer,
   unsigned count,
   unsigned per_index_size,
   size_t offset,
   bool primitive_restart
   )
{
   assert(buffer->enabled);
   // expect higher level API to perform basic alignment and range checks
   assert((offset % per_index_size) == 0);
   assert((offset + per_index_size*count) <= buffer->size);

   size_t size = per_index_size * count;
   void* ptr = khrn_res_interlock_map_range(&buffer->resource, offset, size, KHRN_MAP_READ_BIT, &c_gmem_args);
   if (!ptr)
      return -1;
   int ret = find_max(count, per_index_size, ptr, primitive_restart);
   khrn_res_interlock_unmap_range(buffer->resource, offset, size, KHRN_MAP_READ_BIT);
   return ret;
}

// todo, this control list code shouldn't live here
KHRN_RES_INTERLOCK_T* glxx_buffer_get_tf_aware_res_interlock(GLXX_HW_RENDER_STATE_T *rs,
                                                             GLXX_BUFFER_T *buffer)
{
   assert(buffer->enabled);
   KHRN_RES_INTERLOCK_T *res_i = buffer->resource;

   bool writing_to_buffer = res_i->interlock.is_writer
         && khrn_interlock_get_actions(&res_i->interlock, (KHRN_RENDER_STATE_T*)rs);

   if(!writing_to_buffer)
   {
      // This rs is not writing to the buffer.
      // If another rs is then we will make this rs a dependency of that one.
      return res_i;
   }

   unsigned tf_required_wait_count = buffer->last_tf_write_count;
   if (tf_required_wait_count <= rs->tf_waited_count)
   {
      // No more waiting is required.
      return res_i;
   }

   {
      uint8_t *instr = khrn_fmem_cle(&rs->fmem,
         V3D_CL_FLUSH_TRANSFORM_FEEDBACK_DATA_SIZE +
         V3D_CL_WAIT_TRANSFORM_FEEDBACK_SIZE +
         V3D_CL_FLUSH_VCD_CACHE_SIZE);
      if (!instr)
         return NULL;

      /* Need to flush or the TF wait could get stuck waiting forever */
      v3d_cl_flush_transform_feedback_data(&instr);

      v3d_cl_wait_transform_feedback(&instr, tf_required_wait_count);
      rs->tf_waited_count = tf_required_wait_count;

      v3d_cl_flush_vcd_cache(&instr);

      /* Starting from 3.2, attribute data goes through L2T cache */
      instr = khrn_fmem_cle(&rs->fmem, V3D_CL_L2T_CACHE_FLUSH_CONTROL_SIZE);
      if (!instr)
         return NULL;

      v3d_cl_l2t_cache_flush_control(&instr, 0, ~0, V3D_L2T_FLUSH_MODE_FLUSH);
   }

   return res_i;
}
