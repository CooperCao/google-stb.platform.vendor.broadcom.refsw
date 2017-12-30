/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"

#include "libs/platform/gmem.h"
#include "../common/khrn_render_state.h"
#include "../common/khrn_process.h"
#include "glxx_buffer.h"
#include "glxx_hw_render_state.h"
#include "libs/core/v3d/v3d_align.h"


static inline unsigned glxx_translate_access_flags(GLbitfield access)
{
   static_assrt(KHRN_ACCESS_READ                == GL_MAP_READ_BIT);
   static_assrt(KHRN_ACCESS_WRITE               == GL_MAP_WRITE_BIT);
   static_assrt(KHRN_ACCESS_INVALIDATE_RANGE    == GL_MAP_INVALIDATE_RANGE_BIT);
   static_assrt(KHRN_ACCESS_INVALIDATE_BUFFER   == GL_MAP_INVALIDATE_BUFFER_BIT);
   static_assrt(KHRN_ACCESS_FLUSH_EXPLICIT      == GL_MAP_FLUSH_EXPLICIT_BIT);
   static_assrt(KHRN_ACCESS_UNSYNCHRONIZED      == GL_MAP_UNSYNCHRONIZED_BIT);
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
   unused(size);

   free(buffer->debug_label);
   buffer->debug_label = NULL;

   if (buffer->mapped_pointer != NULL)
      glxx_buffer_unmap_range(buffer, buffer->mapped_offset,
         buffer->mapped_size, buffer->mapped_access_flags);

   khrn_resource_refdec(buffer->resource);
}

void* glxx_buffer_map_range(GLXX_BUFFER_T* buffer, size_t offset, size_t size, GLbitfield access)
{
   return khrn_resource_begin_access(&buffer->resource, offset, size, glxx_translate_access_flags(access) | KHRN_ACCESS_ORPHAN, KHRN_RESOURCE_PARTS_ALL);
}

void glxx_buffer_unmap_range(GLXX_BUFFER_T *buffer, size_t offset, size_t size, GLbitfield access)
{
   khrn_resource_end_access(buffer->resource, offset, size, glxx_translate_access_flags(access));
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

   // Reallocate if size doesn't match.
   bool reallocate = size != buffer->size;

   // Must orphan the existing storage if it's in use.
   if (!reallocate)
   {
      reallocate = (buffer->resource != NULL)
                && khrn_resource_write_now_would_stall(buffer->resource);
   }

   if (reallocate)
   {
      // release existing resource (if any)
      khrn_resource_refdec(buffer->resource);
      buffer->resource = NULL;
      buffer->size = 0;

      // all done if size 0
      if (size > 0)
      {
         // attempt to allocate new resource, or fail
         buffer->resource = khrn_resource_create(
            size,
            V3D_TMU_ML_ALIGN,
            GMEM_USAGE_V3D_RW,
            "GLXX_BUFFER_T.resource");
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

   khrn_access_flags_t map_flags = KHRN_ACCESS_WRITE | KHRN_ACCESS_INVALIDATE_RANGE | KHRN_ACCESS_ORPHAN;
   void* ptr = khrn_resource_begin_access(&buffer->resource, offset, size, map_flags, KHRN_RESOURCE_PARTS_ALL);
   if (!ptr)
      return false;
   memcpy(ptr, data, size);
   khrn_resource_end_access(buffer->resource, offset, size, map_flags);
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

   void* read_ptr = khrn_resource_begin_access(&read_buffer->resource, read_offset, size, KHRN_ACCESS_READ, KHRN_RESOURCE_PARTS_ALL);
   if (!read_ptr)
      return false;
   bool ok = glxx_buffer_subdata(write_buffer, write_offset, size, read_ptr);
   khrn_resource_end_access(read_buffer->resource, read_offset, size, KHRN_ACCESS_READ);
   return ok;
}

size_t glxx_indexed_binding_point_get_size(const GLXX_INDEXED_BINDING_POINT_T *binding)
{
   GLXX_BUFFER_T *buffer = binding->buffer.obj;
   assert(buffer != NULL);
   size_t binding_size;

   if (binding->size == SIZE_MAX)
   {
      // use full buffer size
      assert(binding->offset == 0);
      binding_size = glxx_buffer_get_size(buffer);
   }
   else
   {
      binding_size = gfx_zmin(binding->offset + binding->size, glxx_buffer_get_size(buffer));
   }
   return binding_size;
}
