/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_tf.h"
#include "glxx_hw_render_state.h"

static bool tf_init(GLXX_TRANSFORM_FEEDBACK_T* tf, unsigned name)
{
   memset(tf, 0, sizeof(GLXX_TRANSFORM_FEEDBACK_T));
   tf->name = name;
   tf->prim_mode = GL_POINTS;
   return true;
}

static void tf_term(void *v)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = (GLXX_TRANSFORM_FEEDBACK_T *)v;

   if (glxx_tf_is_active(tf))
   {
      khrn_resource_refdec(tf->res);
      for (unsigned i = 0; i < tf->num_active_buffers; i++)
         khrn_resource_refdec(tf->active_buffers[i].res);
      KHRN_MEM_ASSIGN(tf->program, NULL);
   }

   KHRN_MEM_ASSIGN(tf->bound_buffer.obj, NULL);
   for (int i = 0; i < V3D_MAX_TF_BUFFERS; ++i)
      KHRN_MEM_ASSIGN(tf->binding_points[i].buffer.obj, NULL);


   free(tf->debug_label);
   tf->debug_label = NULL;
}

GLXX_TRANSFORM_FEEDBACK_T* glxx_tf_create(unsigned name)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = KHRN_MEM_ALLOC_STRUCT(GLXX_TRANSFORM_FEEDBACK_T);

   if (!tf)
      return NULL;

   if (!tf_init(tf, name))
   {
      KHRN_MEM_ASSIGN(tf, NULL);
      return NULL;
   }

   khrn_mem_set_term(tf, tf_term);
   return tf;
}

bool glxx_tf_is_active(const GLXX_TRANSFORM_FEEDBACK_T *tf)
{
   return (tf->status & GLXX_TF_ACTIVE) == GLXX_TF_ACTIVE;
}

bool glxx_tf_is_paused(const GLXX_TRANSFORM_FEEDBACK_T *tf)
{
   return (tf->status & GLXX_TF_PAUSED) == GLXX_TF_PAUSED;
}

bool glxx_tf_was_created(const GLXX_TRANSFORM_FEEDBACK_T *tf)
{
   return (tf->status & GLXX_TF_CREATED) == GLXX_TF_CREATED;
}

bool glxx_tf_in_use(const GLXX_TRANSFORM_FEEDBACK_T *tf)
{
   return (glxx_tf_is_active(tf) & !glxx_tf_is_paused(tf));
}

bool glxx_tf_draw_mode_allowed(const GLXX_TRANSFORM_FEEDBACK_T *tf,
      GLXX_PRIMITIVE_T draw_mode)
{
   assert(glxx_tf_in_use(tf));

#if V3D_VER_AT_LEAST(4,1,34,0)
   switch(tf->prim_mode)
   {
      case GL_POINTS:
         return draw_mode == GL_POINTS;
      case GL_LINES:
         return (draw_mode == GL_LINES || draw_mode == GL_LINE_STRIP ||
               draw_mode == GL_LINE_LOOP);
      case GL_TRIANGLES:
         return (draw_mode == GL_TRIANGLES || draw_mode == GL_TRIANGLE_STRIP ||
               draw_mode == GL_TRIANGLE_FAN);
      default:
         unreachable();
         return false;
   }
#else
   return (unsigned)tf->prim_mode == (unsigned)draw_mode;
#endif
}

// Return false if operation is invalid and rs requires flush,
// or we ran out of memory.
bool glxx_tf_add_resource_writes(const GLXX_TRANSFORM_FEEDBACK_T *tf,
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      GLXX_HW_RENDER_STATE_T *rs,
      bool *requires_flush)
{
   assert(glxx_tf_in_use(tf) && (ptf->varying_count > 0));

   assert(ptf->addr_count == tf->num_active_buffers);
   for (unsigned i = 0; i < tf->num_active_buffers; ++i)
   {
      const glxx_tf_active_buffer *active = &tf->active_buffers[i];

      if (!khrn_fmem_record_resource_self_read_conflicting_write(
            &rs->fmem, active->res, KHRN_STAGE_BIN,
            KHRN_RESOURCE_PARTS_ALL, requires_flush))
         return false;
   }

   return true;
}

void glxx_tf_delete_buffer(GLXX_TRANSFORM_FEEDBACK_T *tf, GLXX_BUFFER_T *buffer_obj,
      GLuint buffer)
{
   if (tf->bound_buffer.buffer == buffer)
   {
      KHRN_MEM_ASSIGN(tf->bound_buffer.obj, NULL);
      tf->bound_buffer.buffer = 0;
   }

   for (int i = 0; i < V3D_MAX_TF_BUFFERS; ++i)
   {
      if (tf->binding_points[i].buffer.buffer == buffer)
      {
         KHRN_MEM_ASSIGN(tf->binding_points[i].buffer.obj, NULL);
         tf->binding_points[i].buffer.buffer = 0;
      }
   }
}

static void write_specs(uint8_t **instr,
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      bool point_size_used)
{
   for (unsigned i = 0; i < ptf->spec_count; ++i)
   {
      V3D_TF_SPEC_T  spec = ptf->spec[i];
      uint8_t        packed_spec[2];

      // At link time, all tf specs use 7 as first user varying.
      // Here at draw call time, we decrement 1 from all user varyings if point size is not used.
      // Additionally, 6 (before decrementing) is not allowed.
      if (!point_size_used && spec.first >= 6)
      {
         assert(spec.first != 6);
         spec.first -= 1;
      }
      v3d_pack_tf_spec(packed_spec, &spec);
      v3d_cl_add_8(instr, packed_spec[0]);
      v3d_cl_add_8(instr, packed_spec[1]);
   }
}

#if !V3D_VER_AT_LEAST(4,1,34,0)
/* saturate on overflow */
static size_t multiply_overflow(size_t a, size_t b)
{
   if (a == 0)
      return 0;

   size_t res = a * b;
   if ( res / a != b)
      return SIZE_MAX;
   return res;
}

static size_t add_overflow(size_t a, size_t b)
{
   size_t res = a + b;
   if ( res < a)
      return SIZE_MAX;
   return res;
}

/* return false when we overflow size_t */
static bool verts_and_prim_count(enum glxx_tf_prim_mode tf_prim_mode,
      size_t draw_count, size_t instance_count,
      size_t *vertices, size_t *prim_count){
   size_t verts_per_prim;
   switch (tf_prim_mode)
   {
      case GL_POINTS:
         verts_per_prim = 1;
         break;
      case GL_LINES:
         verts_per_prim = 2;
         break;
      case GL_TRIANGLES:
         verts_per_prim = 3;
         break;
      default:
         unreachable();
   }

   size_t verts_per_instance = draw_count - (draw_count % verts_per_prim);
   *vertices = multiply_overflow(verts_per_instance, instance_count);
   if (*vertices == SIZE_MAX)
         return false;
   *prim_count = *vertices / verts_per_prim;
   return true;
}

bool glxx_tf_capture_to_buffers_no_overflow(const GLXX_TRANSFORM_FEEDBACK_T *tf,
   const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
   size_t draw_count, size_t instance_count)
{
   assert(glxx_tf_in_use(tf) && (ptf->varying_count > 0));

   // Count how many bytes would be written to each buffer by this draw call
   size_t  vertices, prim_count;
   if (!verts_and_prim_count(tf->prim_mode, draw_count, instance_count, &vertices, &prim_count))
      return false;

   size_t bytes_to_write[V3D_MAX_TF_BUFFERS] = { 0, };

   for (unsigned i = 0; i < ptf->spec_count; ++i)
   {
      const V3D_TF_SPEC_T *spec = &ptf->spec[i];
      //bytes_to_write[spec->buffer] += vertices * spec->count * sizeof(uint32_t);
      size_t res;
      res = multiply_overflow(vertices, spec->count);
      res = multiply_overflow(res, sizeof(uint32_t));
      assert(spec->buffer < tf->num_active_buffers);
      bytes_to_write[spec->buffer] = add_overflow(bytes_to_write[spec->buffer], res);
   }

   // check if we exceed buffer limits
   for (unsigned i = 0; i < tf->num_active_buffers; ++i)
   {
      const glxx_tf_active_buffer *active = &tf->active_buffers[i];

      //(offset + stream_position + bytes_to_write[i] <= binding_size);
      size_t res = add_overflow(active->offset, active->stream_position);
      res = add_overflow(res, bytes_to_write[i]);
      if (res > active->size)
         return false;
   }

   return true;
}

unsigned glxx_tf_update_stream_pos(GLXX_TRANSFORM_FEEDBACK_T *tf,
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      unsigned draw_count, unsigned instance_count)
{
   assert(glxx_tf_in_use(tf) && (ptf->varying_count > 0));

   size_t  vertices, prim_count;
   verif(verts_and_prim_count(tf->prim_mode, draw_count, instance_count, &vertices, &prim_count));

   if (prim_count == 0)
      return 0;

   size_t bytes_to_write[V3D_MAX_TF_BUFFERS] = { 0, };

   for (unsigned i = 0; i < ptf->spec_count; ++i)
   {
      const V3D_TF_SPEC_T *spec = &ptf->spec[i];
      bytes_to_write[spec->buffer] += vertices * spec->count * sizeof(uint32_t);
   }

   for (unsigned i = 0; i < tf->num_active_buffers; ++i)
   {
      glxx_tf_active_buffer *active = &tf->active_buffers[i];
      active->stream_position += bytes_to_write[i];
   }

   return prim_count;
}

static bool emit_specs_and_buff_addrs(const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      bool point_size_used,
      unsigned num_active_buffers, const glxx_tf_active_buffer *active_buffers,
      khrn_fmem *fmem)
{
   assert(ptf->varying_count > 0);

   unsigned size = V3D_CL_TRANSFORM_FEEDBACK_ENABLE_SIZE +
              V3D_TF_SPEC_PACKED_SIZE * ptf->spec_count +
              sizeof(v3d_addr_t) * num_active_buffers;

   uint8_t *instr = khrn_fmem_cle(fmem, size);
   if (!instr)
      return false;

   v3d_cl_transform_feedback_enable(&instr, 0, num_active_buffers, ptf->spec_count);

   write_specs(&instr, ptf, point_size_used);

   //transform feedback buffer addresses
   for (unsigned i = 0; i < num_active_buffers; ++i)
   {
      const glxx_tf_active_buffer *active = &active_buffers[i];
      v3d_cl_add_addr(&instr, active->addr + active->offset + active->stream_position);
   }
   return true;
}
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
static bool emit_specs(const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
         bool point_size_used, khrn_fmem *fmem)
{
   assert((ptf->varying_count > 0));

   unsigned size = V3D_CL_TRANSFORM_FEEDBACK_SPECS_SIZE +
              V3D_TF_SPEC_PACKED_SIZE * ptf->spec_count;

   uint8_t *instr = khrn_fmem_cle(fmem, size);
   if (!instr)
      return false;

   v3d_cl_transform_feedback_specs(&instr, ptf->spec_count, true);
   write_specs(&instr, ptf, point_size_used);
   return true;
}

static bool emit_load_buffers_state(gmem_handle_t handle, khrn_fmem *fmem)
{
   assert(handle != GMEM_HANDLE_INVALID);
   v3d_addr_t addr = khrn_fmem_sync_and_get_addr(fmem, handle, V3D_BARRIER_PTB_PCF_READ, 0);

   uint8_t *instr = khrn_fmem_cle(fmem, V3D_CL_PRIM_COUNTS_FEEDBACK_SIZE);
   if (!instr)
      return false;
   v3d_cl_prim_counts_feedback(&instr, V3D_PCF_OPERATION_LD_BUF_STATE_WAIT0, false, addr);
   return true;
}

static bool emit_buff_addrs(unsigned num_active_buffers,
      const glxx_tf_active_buffer *active_buffers,
      khrn_fmem *fmem)
{
   unsigned size = V3D_CL_TRANSFORM_FEEDBACK_BUFFER_SIZE * num_active_buffers;
   uint8_t *instr = khrn_fmem_cle(fmem, size);
   if (!instr)
      return false;

   for (unsigned i = 0; i < num_active_buffers; ++i)
   {
      const glxx_tf_active_buffer *active = &active_buffers[i];
      if (active->offset < active->size)
         v3d_cl_transform_feedback_buffer(&instr, i, (active->size - active->offset) / 4,
               active->addr + active->offset);
      else
         v3d_cl_transform_feedback_buffer(&instr, i, 0, 0);
   }
   return true;
}
#endif

static void fmem_sync_active_buffers(unsigned num_active_buffers,
      const glxx_tf_active_buffer *active_buffers,
      khrn_fmem *fmem)
{
   for (unsigned i = 0; i < num_active_buffers; ++i)
   {
      const glxx_tf_active_buffer *active = &active_buffers[i];
      khrn_fmem_sync(fmem, active->res->num_handles, active->res->handles, V3D_BARRIER_PTB_TF_WRITE, 0);
      assert(active->addr == khrn_resource_get_addr(active->res));
   }
}

bool glxx_tf_record_enable(GLXX_HW_RENDER_STATE_T *rs,
      GLXX_TRANSFORM_FEEDBACK_T *tf,
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      bool point_size_used)
{
   assert(glxx_tf_in_use(tf) && (ptf->varying_count > 0));
   khrn_fmem *fmem = &rs->fmem;

   if (rs->tf.res == tf->res)
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      /* if not enabled, just enable the previous specs, otherwise, we are done; */
      if (!rs->tf.enabled)
      {
         uint8_t *instr = khrn_fmem_cle(&rs->fmem, V3D_CL_TRANSFORM_FEEDBACK_SPECS_SIZE);
         if (!instr)
            return false;
         v3d_cl_transform_feedback_specs(&instr, 0, true);
      }
#else
      /* tf is enabled only when we use prim_mode+"_TF",
       * so we never call glxx_tf_record_disable */
      assert(rs->tf.enabled);
#endif
   }
   else
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      if (!glxx_store_tf_buffers_state(rs))
         return false;
#endif
      // record that we are writing to this resource
      if (!khrn_fmem_record_resource_write(fmem, tf->res, KHRN_STAGE_BIN,
            KHRN_RESOURCE_PARTS_ALL, NULL))
         return false;

      assert(ptf->addr_count == tf->num_active_buffers);
      fmem_sync_active_buffers(tf->num_active_buffers, tf->active_buffers, fmem);

      bool res;
#if V3D_VER_AT_LEAST(4,1,34,0)
      if (khrn_resource_has_storage(tf->res))
         res = emit_load_buffers_state(tf->res->handles[0], fmem);
      else
         res = emit_buff_addrs(tf->num_active_buffers, tf->active_buffers, fmem);
      res &= emit_specs(ptf, point_size_used, fmem);
#else
      /* we can't do store/load on previous hw */
      assert(!khrn_resource_has_storage(tf->res));

      res = emit_specs_and_buff_addrs(ptf, point_size_used,
            tf->num_active_buffers, tf->active_buffers, fmem);
#endif
      if (!res)
         return false;

      KHRN_MEM_ASSIGN(rs->tf.last_used, tf);
      /* no need to take a refcount of tf->res since we already got one in
       * fmem with khrn_fmem_record_resource */
      rs->tf.res = tf->res;
   }

   rs->tf.enabled = true;
   rs->tf.used = true;

   return true;
}

#if V3D_VER_AT_LEAST(4,1,34,0)
bool glxx_tf_record_disable(GLXX_HW_RENDER_STATE_T *rs)
{
   if (rs->tf.enabled)
   {
      uint8_t *instr = khrn_fmem_cle(&rs->fmem, V3D_CL_TRANSFORM_FEEDBACK_SPECS_SIZE);
      if (!instr) return false;

      v3d_cl_transform_feedback_specs(&instr, 0, false);
      rs->tf.enabled = false;
   }
   return true;
}

static bool alloc_storage(khrn_resource *res)
{
   /* alloc 8 32 bit words */
   return khrn_resource_alloc(
      res, 8 * sizeof(uint32_t), V3D_PRIM_COUNTS_ALIGN,
      GMEM_USAGE_V3D_RW | GMEM_USAGE_HINT_DYNAMIC,
      "tf_storage");
}

bool glxx_store_tf_buffers_state(GLXX_HW_RENDER_STATE_T *rs)
{
   GLXX_TRANSFORM_FEEDBACK_T *tf = rs->tf.last_used;

   if (!tf || !glxx_tf_is_active(tf))
      return true;

   khrn_resource *res = tf->res;

   if (rs->tf.res != res)
      /* we started a new BeginTF on the same tf, no point to store */
      return true;

   if(!khrn_resource_has_storage(res))
   {
      if (!alloc_storage(res))
         return false;
   }

   v3d_addr_t addr = khrn_fmem_sync_and_get_addr(&rs->fmem, res->handles[0], V3D_BARRIER_PTB_PCF_WRITE, 0);

   uint8_t *instr = khrn_fmem_cle(&rs->fmem, V3D_CL_PRIM_COUNTS_FEEDBACK_SIZE);
   if (!instr)
      return false;

   v3d_cl_prim_counts_feedback(&instr, V3D_PCF_OPERATION_ST_BUF_STATE_RAW, false, addr);
   glxx_tf_incr_start_count(rs);

   return true;
}
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
static bool emit_explicit_tf_flush_count(GLXX_HW_RENDER_STATE_T *rs)
{
   uint8_t *instr = khrn_fmem_cle(&rs->fmem, V3D_CL_TF_DRAW_FLUSH_AND_COUNT_SIZE);
   if (!instr)
      return false;

   v3d_cl_tf_draw_flush_and_count(&instr);
   return true;
}
#endif

void glxx_tf_incr_start_count(GLXX_HW_RENDER_STATE_T *rs)
{
   rs->tf.started_count += 1;
}

bool glxx_tf_post_draw(GLXX_HW_RENDER_STATE_T *rs, const GLXX_TRANSFORM_FEEDBACK_T *tf)
{
   assert(rs->tf.last_used == tf && glxx_tf_is_active(rs->tf.last_used));

#if V3D_VER_AT_LEAST(4,1,34,0)
   if (!emit_explicit_tf_flush_count(rs))
      return false;
#endif

   glxx_tf_incr_start_count(rs);

   /* Record the counter of the last TF-enabled draw call that wrote to this
    * buffer; used for wait tf*/
   for (unsigned i = 0; i < tf->num_active_buffers; ++i)
   {
      khrn_resource *res = tf->active_buffers[i].res;
      assert(khrn_resource_get_write_stages(res, (khrn_render_state *)rs) & KHRN_STAGE_BIN);
      res->last_tf_write_count = rs->tf.started_count;
   }
   return true;
}
