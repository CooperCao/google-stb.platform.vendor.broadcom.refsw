/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_BUFFER_H
#define GLXX_BUFFER_H

#include "gl_public_api.h"

#include "../common/khrn_int_common.h"
#include "../common/khrn_resource.h"

/* Generic buffer bindings stored in the server state. Others are stored
 * within objects and are noted below.
 */
enum {
   GLXX_BUFTGT_ARRAY,                      // Vertex attributes
   GLXX_BUFTGT_COPY_READ,                  // Buffer copy source
   GLXX_BUFTGT_COPY_WRITE,                 // Buffer copy destination
   GLXX_BUFTGT_PIXEL_PACK,
   GLXX_BUFTGT_PIXEL_UNPACK,
   GLXX_BUFTGT_DRAW_INDIRECT,
   GLXX_BUFTGT_DISPATCH_INDIRECT,
   GLXX_BUFTGT_UNIFORM_BUFFER,             // Indexed uniform block storage
   GLXX_BUFTGT_ATOMIC_COUNTER_BUFFER,
   GLXX_BUFTGT_SHADER_STORAGE_BUFFER,
   GLXX_BUFTGT_TEXTURE_BUFFER,
   // Element array buffer binding is in GLXX_VAO_T
   // Transform feedback buffer is in GLXX_TRANSFORM_FEEDBACK_T
   GLXX_BUFTGT_CTX_COUNT
} glxx_buffer_target_t;

typedef struct GLXX_BUFFER_ {
   /*
    * name of the buffer
    * This is here solely to let us answer glGet<blah>() queries. Note that due
    * to the shared context semantics it's not necessarily a no-op to get the name
    * of the bound buffer and then call glBindBuffer() with that name
    * Invariant:
    * name != 0
    */
   uint32_t    name;
   GLenum      usage;
   void *      mapped_pointer;
   GLsizeiptr  mapped_size;
   GLintptr    mapped_offset;
   GLbitfield  mapped_access_flags;
   size_t      size;
   char        *debug_label;
   khrn_resource *resource;

   bool enabled; // true if the name associated with this object has been bound
} GLXX_BUFFER_T;

/*
 * Currently bound buffer
 * We store both the object and its integer name because it may have been
 * deleted FROM ANOTHER EGL CONTEXT SHARING WITH THIS ONE and thus
 * disassociated from its name.  The name can be obtained from the object.
 * Invariant:
 * obj is either NULL or a valid GLXX_BUFFER_T pointer.
 */
typedef struct GLXX_BUFFER_BINDING_T_ {
   GLXX_BUFFER_T    *obj;
   uint32_t          buffer;
} GLXX_BUFFER_BINDING_T;


/*
 * if size == SIZE_MAX, use full size, even if changed after binding buffer (see
 * BindBufferBase)
 */
typedef struct GLXX_INDEXED_BINDING_POINT_T_ {
   GLXX_BUFFER_BINDING_T   buffer;
   size_t offset;
   size_t size;
} GLXX_INDEXED_BINDING_POINT_T;

/* Return the bound size of this point. The size is not relative to the offset.
 * If the bound point size is SIZE_MAX, return bounded buffer size */
extern size_t glxx_indexed_binding_point_get_size(const GLXX_INDEXED_BINDING_POINT_T *binding);

extern void glxx_buffer_init(GLXX_BUFFER_T *buffer, uint32_t name);
extern void glxx_buffer_enable(GLXX_BUFFER_T *buffer);
extern void glxx_buffer_term(void *v);

extern bool glxx_buffer_data(GLXX_BUFFER_T *buffer, size_t size,
      const void *data, GLenum usage);
extern bool glxx_buffer_subdata(GLXX_BUFFER_T *buffer, size_t offset,
      size_t size, const void *data);

static inline size_t glxx_buffer_get_size(const GLXX_BUFFER_T *buffer)
{
   return buffer->size;
}

static inline bool glxx_buffer_write_now_would_flush(const GLXX_BUFFER_T *buffer)
{
   return buffer->resource && khrn_resource_has_reader_or_writer(buffer->resource);
}

extern bool glxx_buffer_copy_subdata(GLXX_BUFFER_T *src, GLXX_BUFFER_T *dst,
   size_t src_offset, size_t dst_offset, size_t size);

/* maps in user space "size" bytes of buffer, starting from offset */
void* glxx_buffer_map_range(GLXX_BUFFER_T *buffer, size_t offset, size_t size, GLbitfield access);
void glxx_buffer_unmap_range(GLXX_BUFFER_T *buffer, size_t offset, size_t size, GLbitfield access);

/* maps buffer in user space, starting from offset till the end of the buffer */
static inline void* glxx_buffer_map(GLXX_BUFFER_T *buffer, size_t offset, GLbitfield access)
{
   assert(offset <= buffer->size);
   return glxx_buffer_map_range(buffer, offset, buffer->size - offset, access);
}

static inline void glxx_buffer_unmap(GLXX_BUFFER_T *buffer, size_t offset, GLbitfield access)
{
   assert(offset <= buffer->size);
   glxx_buffer_unmap_range(buffer, offset, buffer->size - offset, access);
}


#endif
