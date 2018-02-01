/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_TF_H
#define GLXX_TF_H

#include "gl_public_api.h"
#include "glxx_enum_types.h"
#include "../common/khrn_types.h"
#include <stdbool.h>
#include "glxx_buffer.h"
#include "../gl20/gl20_program.h"

#define tf_enumify(x) E_TF_##x=x

enum glxx_tf_prim_mode
{
   tf_enumify(GL_POINTS),
   tf_enumify(GL_LINES),
   tf_enumify(GL_TRIANGLES),
};

enum glxx_tf_status
{
   GLXX_TF_CREATED = (1 << 0), // set on BindTransformFeedback
   GLXX_TF_ACTIVE = (1 << 1), // set on BeginTransformFeedback, cleared on EndTransformFeedback
   GLXX_TF_PAUSED = (1 << 2), // set on PauseTransformFeedback, cleared on (Resume|End)TransformFeedback
};

typedef struct
{
   khrn_resource *res;
   v3d_addr_t addr;
   size_t offset;
   size_t size;
#if !V3D_VER_AT_LEAST(4,1,34,0)
   size_t stream_position;
#endif
}glxx_tf_active_buffer;

typedef struct glxx_transform_feedback
{
   int32_t                       name;
   enum glxx_tf_status           status;
   enum glxx_tf_prim_mode        prim_mode;
   // bounded buffer to general binding point
   GLXX_BUFFER_BINDING_T         bound_buffer;
   // bounded buffers to indexed binding points
   GLXX_INDEXED_BINDING_POINT_T  binding_points[V3D_MAX_TF_BUFFERS];
   // Program used by the tf
   struct GL20_PROGRAM_T_        *program;
   char                          *debug_label;

   khrn_resource          *res;

   //useful info when tf is active (between BeginTransformFeedback and EndTransformFeedback)
   unsigned num_active_buffers;
   glxx_tf_active_buffer active_buffers[V3D_MAX_TF_BUFFERS];

} GLXX_TRANSFORM_FEEDBACK_T;

extern GLXX_TRANSFORM_FEEDBACK_T* glxx_tf_create(unsigned name);

extern bool glxx_tf_add_resource_writes(const GLXX_TRANSFORM_FEEDBACK_T *tf,
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      GLXX_HW_RENDER_STATE_T *rs,
      bool *requires_flush);

extern bool glxx_tf_is_active(const GLXX_TRANSFORM_FEEDBACK_T *tf);
extern bool glxx_tf_is_paused(const GLXX_TRANSFORM_FEEDBACK_T *tf);
extern bool glxx_tf_in_use(const GLXX_TRANSFORM_FEEDBACK_T *tf);

/* a tf is "created" when glBindTransformFeedback gets called */
extern bool glxx_tf_was_created(const GLXX_TRANSFORM_FEEDBACK_T *tf);

extern bool glxx_tf_draw_mode_allowed(const GLXX_TRANSFORM_FEEDBACK_T *tf, GLXX_PRIMITIVE_T draw_mode);

extern void glxx_tf_delete_buffer(GLXX_TRANSFORM_FEEDBACK_T *tf, GLXX_BUFFER_T *buffer_obj,
      GLuint buffer);

extern bool glxx_tf_record_enable(GLXX_HW_RENDER_STATE_T *rs,
      GLXX_TRANSFORM_FEEDBACK_T *tf,
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      bool point_size_used);

extern bool glxx_tf_post_draw(GLXX_HW_RENDER_STATE_T *rs, const GLXX_TRANSFORM_FEEDBACK_T *tf);

extern void glxx_tf_incr_start_count(GLXX_HW_RENDER_STATE_T *rs);

#if V3D_VER_AT_LEAST(4,1,34,0)
extern bool glxx_tf_record_disable(GLXX_HW_RENDER_STATE_T *rs);
extern bool glxx_store_tf_buffers_state(GLXX_HW_RENDER_STATE_T *rs);
#endif

#if !V3D_VER_AT_LEAST(4,1,34,0)
/* returns false if capturing the desired tf overflows the limits of the buffers
 * bound to the indexed binding point sof the tf */
extern bool glxx_tf_capture_to_buffers_no_overflow(const GLXX_TRANSFORM_FEEDBACK_T *tf,
   const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
   size_t draw_count, size_t instance_count);

/* returns the number of primitives that would be captured to the tf */
extern unsigned glxx_tf_update_stream_pos(GLXX_TRANSFORM_FEEDBACK_T *tf,
      const GLXX_PROGRAM_TFF_POST_LINK_T *ptf,
      unsigned draw_count, unsigned instance_count);
#endif

#endif
