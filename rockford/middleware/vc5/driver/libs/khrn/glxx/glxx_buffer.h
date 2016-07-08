/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 vertex buffer structure declaration.
=============================================================================*/

#ifndef GLXX_BUFFER_H
#define GLXX_BUFFER_H

#include "gl_public_api.h"

#include "../common/khrn_int_common.h"
#include "../common/khrn_res_interlock.h"

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
   KHRN_RES_INTERLOCK_T *resource;

   // Record the counter of the last TF-enabled draw call that wrote to this buffer.
   // If we request reading from this buffer, we put in a TF wait instruction to
   // wait for up to that many TF draws to finish.
   unsigned    last_tf_write_count;

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
 * if size == -1, use full size, even if changed after binding buffer (see
 * BindBufferBase)
 */
typedef struct GLXX_INDEXED_BINDING_POINT_T_ {
   GLXX_BUFFER_BINDING_T   buffer;
   GLintptr                offset;
   GLsizeiptr              size;
} GLXX_INDEXED_BINDING_POINT_T;

extern void glxx_buffer_init(GLXX_BUFFER_T *buffer, uint32_t name);
extern void glxx_buffer_enable(GLXX_BUFFER_T *buffer);
extern void glxx_buffer_term(void *v, size_t size);

extern bool glxx_buffer_data(GLXX_BUFFER_T *buffer, size_t size,
      const void *data, GLenum usage);
extern bool glxx_buffer_subdata(GLXX_BUFFER_T *buffer, size_t offset,
      size_t size, const void *data);

/* you need to call khrn_res_interlock_aquire on this returned interlock if you
 * want to keep it for longer periods */
static inline KHRN_RES_INTERLOCK_T* glxx_buffer_get_res_interlock(const GLXX_BUFFER_T *buffer)
{
   return buffer->resource;
}

static inline size_t glxx_buffer_get_size(const GLXX_BUFFER_T *buffer)
{
   return buffer->size;
}

/*
 * The TF-aware version should be used when reading from a buffer that may have been
 * written to by TF in the current render state. It will check what the last rs wait counter
 * to write to this buffer was, and insert TF wait instructions if the current wait counter
 * is behind the wait counter of the last TF write.
 */
KHRN_RES_INTERLOCK_T * glxx_buffer_get_tf_aware_res_interlock(GLXX_HW_RENDER_STATE_T *rs,
                                                              GLXX_BUFFER_T *buffer);

static inline bool glxx_buffer_write_now_would_flush(const GLXX_BUFFER_T *buffer)
{
   return buffer->resource && khrn_interlock_write_now_would_flush(&buffer->resource->interlock);
}

extern bool glxx_buffer_copy_subdata(GLXX_BUFFER_T *src, GLXX_BUFFER_T *dst,
   size_t src_offset, size_t dst_offset, size_t size);

/* call this only on an element_array buffer */
extern int glxx_buffer_find_max(GLXX_BUFFER_T *buffer, unsigned count,
      unsigned per_index_count, size_t indices_offset, bool primitive_restart);

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
