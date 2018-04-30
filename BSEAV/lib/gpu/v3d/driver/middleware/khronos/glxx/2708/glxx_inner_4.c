/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_color.h"
#include "interface/khronos/common/khrn_options.h"

#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"
#include "middleware/khronos/glxx/2708/glxx_tu_4.h"
#include <GLES2/gl2.h>
#include <EGL/eglext.h>

//XXXXXXXXXXXXXXXX
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_tformat.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/2708/khrn_pool_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/common/2708/khrn_fmem_4.h"
#include "middleware/khronos/common/khrn_mem.h"

#if defined(ANDROID)
#include <cutils/log.h>
#endif

/*************************************************************
 Defines
 *************************************************************/
/*#define ALLOW_FORCE_LOCKUP*/

/*************************************************************
 Static data
 *************************************************************/

static GLXX_HW_RENDER_STATE_T *render_state = NULL;

#define stencil_clear_mode(value) \
   ((value)<<8 | /* Stencil value */\
   7<<16 |  /* func : not equal */\
   2<<22 |  /* zpop : pass op = replace */\
   3<<28 |  /* wmc : write mask = 0xFF */\
   3<<30    /* wsel : set both front & back states */)

#define stencil_keep_mode 0

#define stencil_mask_mode(mask) \
   (((mask) & 0xFF) |        /* front mask */\
   (((mask) & 0xFF) << 8)    /* back mask */)

/* Clear depth & stencil (depth & stencil optional depending on uniforms and z-func)
 * Uniform 0 = stencil mode (depends on whether stencil is being cleared)
 * Uniform 1 = stencil write mask
 * z-func set depending on whether depth is being cleared */
static uint32_t clear_shader_ds[] =
{
   /* generated 2010-05-11 11:08:25 */
   0x15827d80, 0x10020ae7, /* mov  stencil, unif */
   0x15827d80, 0x10020ae7, /* mov  stencil, unif */
   0x159cffc0, 0x40020b27, /* mov  tlbz, z       ; sbwait */
   0x009e7000, 0x500009e7, /* nop                ; sbdone */
   0x009e7000, 0x300009e7, /* nop                ; thrend */
   0x009e7000, 0x100009e7, /* nop */
   0x009e7000, 0x100009e7, /* nop */
};

/* Clear depth, stencil & colour (depth & stencil optional depending on uniforms and z-func)
 * Uniform 0 = stencil mode (depends on whether stencil is being cleared)
 * Uniform 1 = stencil write mask
 * z-func set depending on whether depth is being cleared */
static uint32_t clear_shader_dsc[] =
{
   /* generated 2010-05-11 11:08:25 */
   0x15827d80, 0x10020ae7, /* mov  stencil, unif */
   0x15827d80, 0x10020ae7, /* mov  stencil, unif */
   0x159cffc0, 0x40020b27, /* mov  tlbz, z       ; sbwait */
   0x15827d80, 0x10020ba7, /* mov  tlbc, unif */
   0x009e7000, 0x500009e7, /* nop                ; sbdone */
   0x009e7000, 0x300009e7, /* nop                ; thrend */
   0x009e7000, 0x100009e7, /* nop */
   0x009e7000, 0x100009e7, /* nop */
};

/* Clear depth, stencil & colour (masked) (depth & stencil optional depending on uniforms and z-func)
 * Uniform 0 = colour mask
 * Uniform 1 = stencil mode (depends on whether stencil is being cleared)
 * Uniform 2 = stencil write mask
 * z-func set depending on whether depth is being cleared */
static uint32_t clear_shader_pdsc[] =
{
   0x15827d80, 0x10020867, /* mov  r1, unif*/
   0x15827d80, 0x10020ae7, /* mov  stencil, unif*/
   0x15827d80, 0x10020ae7, /* mov  stencil, unif*/
   0x179e7240, 0x40020827, /* not  r0, r1             ; sbwait*/
   0x14827380, 0x80020867, /* and  r1, r1, unif  ; loadc*/
   0x149e7100, 0x10020827, /* and  r0, r0, r4*/
   0x159cffc0, 0x10020b27, /* mov  tlbz, z*/
   0x159e7040, 0x10020ba7, /* or   tlbc, r0, r1*/
   0x009e7000, 0x500009e7, /* nop                ; sbdone*/
   0x009e7000, 0x300009e7, /* nop                ; thrend*/
   0x009e7000, 0x100009e7, /* nop*/
   0x009e7000, 0x100009e7, /* nop*/
};

/* Clear depth, stencil & colour (masked & multisampled) (depth & stencil optional depending on uniforms and z-func)
 * Uniform 0 = colour mask
 * Uniform 1 = stencil mode (depends on whether stencil is being cleared)
 * Uniform 2 = stencil write mask
 * z-func set depending on whether depth is being cleared */
static uint32_t clear_shader_mpdsc[] =
{
   0x15827d80, 0x10020867, /* mov  r1, unif*/
   0x15827d80, 0x10020ae7, /* mov  stencil, unif*/
   0x15827d80, 0x10020ae7, /* mov  stencil, unif*/
   0x179e7240, 0x40020827, /* not  r0, r1        ; sbwait*/
   0x14827380, 0x80020867, /* and  r1, r1, unif  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159cffc0, 0x10020b27, /* mov  tlbz, z*/
   0x159e7440, 0x80020b67, /* or   tlbm, r2, r1  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159e7440, 0x80020b67, /* or   tlbm, r2, r1  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159e7440, 0x80020b67, /* or   tlbm, r2, r1  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159e7440, 0x10020b67, /* or   tlbm, r2, r1*/
   0x009e7000, 0x500009e7, /* nop                ; sbdone*/
   0x009e7000, 0x300009e7, /* nop                ; thrend*/
   0x009e7000, 0x100009e7, /* nop*/
   0x009e7000, 0x100009e7, /* nop*/
};

/* Clear colour
 * Uniform 0 = color */
static uint32_t clear_shader_c[] =
{
   /* generated 2010-05-11 11:08:25 */
   0x009e7000, 0x100009e7, /* nop */
   0x009e7000, 0x100009e7, /* nop */
   0x15827d80, 0x40020ba7, /* mov  tlbc, unif    ; sbwait */
   0x009e7000, 0x500009e7, /* nop                ; sbdone */
   0x009e7000, 0x300009e7, /* nop                ; thrend */
   0x009e7000, 0x100009e7, /* nop */
   0x009e7000, 0x100009e7, /* nop */
};

/* Clear colour (masked)
 * Uniform 0 = mask
 * Uniform 1 = color */
static uint32_t clear_shader_pc[] =
{
   0x009e7000, 0x100009e7, /* nop*/
   0x15827d80, 0x10020867, /* mov  r1, unif*/
   0x179e7240, 0x40020827, /* not  r0, r1        ; sbwait*/
   0x14827380, 0x80020867, /* and  r1, r1, unif  ; loadc*/
   0x149e7100, 0x10020827, /* and  r0, r0, r4*/
   0x159e7040, 0x10020ba7, /* or   tlbc, r0, r1*/
   0x009e7000, 0x500009e7, /* nop                ; sbdone*/
   0x009e7000, 0x300009e7, /* nop                ; thrend*/
   0x009e7000, 0x100009e7, /* nop*/
   0x009e7000, 0x100009e7, /* nop*/
};

/* Clear colour (masked & multisampled)
 * Uniform 0 = mask
 * Uniform 1 = color */
static uint32_t clear_shader_mpc[] =
{
   0x009e7000, 0x100009e7, /* nop*/
   0x15827d80, 0x10020867, /* mov  r1, unif*/
   0x179e7240, 0x40020827, /* not  r0, r1        ; sbwait*/
   0x14827380, 0x80020867, /* and  r1, r1, unif  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159e7440, 0x80020b67, /* or   tlbm, r2, r1  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159e7440, 0x80020b67, /* or   tlbm, r2, r1  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159e7440, 0x80020b67, /* or   tlbm, r2, r1  ; loadc*/
   0x149e7100, 0x100208a7, /* and  r2, r0, r4*/
   0x159e7440, 0x10020b67, /* or   tlbm, r2, r1*/
   0x009e7000, 0x500009e7, /* nop                ; sbdone*/
   0x009e7000, 0x300009e7, /* nop                ; thrend*/
   0x009e7000, 0x100009e7, /* nop*/
   0x009e7000, 0x100009e7, /* nop*/
};

#define CLR_COLOR   1
#define CLR_DEPTH   2
#define CLR_STENCIL 4
#define CLR_MASKED  8
#define CLR_MULTISAMPLE  16

static uint32_t *clear_shaders[] =
{
   /* this bank contain shaders for egl contexts without depth buffers */
   0,                /* Non-masked, non-multisample */
   clear_shader_c,
   clear_shader_ds,
   clear_shader_dsc,
   clear_shader_ds,
   clear_shader_dsc,
   clear_shader_ds,
   clear_shader_dsc,
   0,                /* Masked, non-multisample */
   clear_shader_pc,
   clear_shader_ds,
   clear_shader_pdsc,
   clear_shader_ds,
   clear_shader_pdsc,
   clear_shader_ds,
   clear_shader_pdsc,
   0,                /* Non-masked, multisample */
   clear_shader_c,
   clear_shader_ds,
   clear_shader_dsc,
   clear_shader_ds,
   clear_shader_dsc,
   clear_shader_ds,
   clear_shader_dsc,
   0,                /* Masked, multisample */
   clear_shader_mpc,
   clear_shader_ds,
   clear_shader_mpdsc,
   clear_shader_ds,
   clear_shader_mpdsc,
   clear_shader_ds,
   clear_shader_mpdsc,
};

static uint32_t clear_shader_sizes[] =
{
   0,
   sizeof(clear_shader_c),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_dsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_dsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_dsc),
   0,
   sizeof(clear_shader_pc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_pdsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_pdsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_pdsc),
   0,
   sizeof(clear_shader_c),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_dsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_dsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_dsc),
   0,
   sizeof(clear_shader_mpc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_mpdsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_mpdsc),
   sizeof(clear_shader_ds),
   sizeof(clear_shader_mpdsc),
};

/*************************************************************
 Static function forwards
 *************************************************************/
static bool draw_rect(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs, GLXX_HW_FRAMEBUFFER_T *fb, bool color, bool depth, bool stencil, int x, int y, int xmax, int ymax);
static bool populate_master_cl(GLXX_HW_FRAMEBUFFER_T *fb);
static bool create_bin_cl(void);
static bool create_master_cl(void);

/*************************************************************
 Global Functions
 *************************************************************/

/*!
 * \brief Clears state and frame information to start a new frame.
 *
 * This function is used to initialise the simulator hardware for a new
 * frame. It resets the state of various registers, all control lists, the texture
 * handle list and the frame data pointer. It also emits a clear instruction into
 * the new, blank master control list so that the frame buffer is cleared.
 *
 * \param color   is the clear colour to be used.
 * \param depth   is the clear Z depth.
 * \param stencil is the clear stencil to be used.
 * \param state   is the OpenGL server state.
 */
bool glxx_hw_clear(bool color, bool depth, bool stencil, GLXX_SERVER_STATE_T *state)
{
   GLXX_HW_FRAMEBUFFER_T fb;
   GLXX_HW_RENDER_STATE_T *rs;
   bool result;
   bool useDrawRect = false;
   bool clearingAllBuffers = true;
   bool depth_storage = false;
   int x, y, xmax, ymax;

   //moved install frame buffer until after the check

   depth &= state->depth_mask;

   /* Need to check for nothing to clear again here, since the depthMask may have changed the
    * set of buffers to be cleared */
   if (!color && !depth && !stencil)
      return true;

   rs = glxx_install_framebuffer(state, &fb, false);
   if (!rs)
      return true;    /* TODO: or false? */

   if (DRAW_TEX_LOGGING)
   {
      vcos_log(LOG_INFO, "rs: %d clear: color %d depth %d stencil %d",
         rs->name, color, depth, stencil);
      vcos_log(LOG_INFO, "--------------------");
   }

   if (fb.depth != NULL)
   {
      KHRN_IMAGE_T *depth = fb.depth;
      depth_storage = (depth->mh_storage != MEM_HANDLE_INVALID);
   }

   x = 0;
   y = 0;
   xmax = fb.width;
   ymax = fb.height;

   if (state->caps.scissor_test)
   {
      x = _max(x, state->scissor.x);
      y = _max(y, state->scissor.y);
      xmax = _min(xmax, state->scissor.x + state->scissor.width);
      ymax = _min(ymax, state->scissor.y + state->scissor.height);
      if (x >= xmax || y >= ymax) return true;  /* Successfully cleared nothing */
   }

   /* Depth/Stencil may contain valid info if there is storage or if some was drawn there */
   if (!color || state->shader.common.blend.color_mask != 0xFFFFFFFF)
      clearingAllBuffers = false;
   if (fb.have_depth && !depth && (depth_storage || rs->drawn) )
      clearingAllBuffers = false;
   if ((fb.have_stencil && rs->stencil_used) && (!stencil || (state->stencil_mask.front & 0xFF) != 0xFF) && (depth_storage || rs->drawn) )
      clearingAllBuffers = false;
   if (x != 0 || y != 0 || xmax != (int)fb.width || ymax != (int)fb.height)
      clearingAllBuffers = false;

   rs->stencil_used |= stencil;     /* Make sure a subsequent no-stencil-clear won't overwrite the clear value */

   if (rs->drawn) {
      /* If we're clearing everything clean out the fmem and control list then do a h/w clear */
      if (clearingAllBuffers) {
         rs->hw_frame_count = 0;
         khrn_fmem_discard(rs->fmem);           /* Discard our data and reallocate. */
         rs->fmem = khrn_fmem_init(khrn_interlock_user(rs->name));   /* TODO: Possibly a better way      */
         if (!rs->fmem) goto fail;

         glxx_lock_fixer_stuff(rs);
         result = create_bin_cl();
         glxx_unlock_fixer_stuff();
         if (!result) goto fail;

         state->changed_cfg = true;             /* We discarded these so flag to */
         state->changed_linewidth = true;       /* recreate the records.         */
         state->changed_polygon_offset = true;
         state->changed_viewport = true;
         state->old_flat_shading_flags = ~0;
         rs->drawn = false;

         useDrawRect = false;
      } else                  /* If we're not clearing everything then we've no choice... */
         useDrawRect = true;
   }
   else
   {
      /* Start of a frame, so choose between h/w clear or draw_rect */
      useDrawRect = !clearingAllBuffers;
   }

   if (useDrawRect)
   {
      khrn_driver_incr_counters(KHRN_PERF_SOFT_CLEARS);

      result = draw_rect(state, rs, &fb, color, depth, stencil, x, y, xmax, ymax);
   }
   else
   {
      khrn_driver_incr_counters(KHRN_PERF_HARD_CLEARS);

      if (color)
      {
         KHRN_IMAGE_FORMAT_T col_format;
         rs->color_buffer_valid = true;
         rs->color_load = false;
         col_format = khrn_image_to_tf_format(fb.col_format);
         if (tu_image_format_rb_swap(col_format))
         {
            rs->color_value = color_floats_to_rgba(
               state->clear_color[2],
               state->clear_color[1],
               state->clear_color[0],
               state->clear_color[3]);
         }
         else
         {
            rs->color_value = color_floats_to_rgba(
               state->clear_color[0],
               state->clear_color[1],
               state->clear_color[2],
               state->clear_color[3]);
         }

         if (fb.ms)
            rs->ms_color_buffer_valid = true;
      }
      if (fb.have_depth && depth)
      {
         rs->ds_buffer_valid = true;
         rs->depth_load = false;
         rs->depth_value = state->clear_depth;
      }
      if (fb.have_stencil && stencil)
      {
         rs->ds_buffer_valid = true;
         rs->stencil_load = false;
         rs->stencil_value = (rs->stencil_value & ~state->stencil_mask.front) | (state->clear_stencil & state->stencil_mask.front);
      }
      rs->hw_cleared = true;
      result = true;
   }

   if (!result) goto fail;

   return true;

fail:
   glxx_hw_discard_frame(rs);
   return false;
}

void glxx_hw_invalidate_internal(GLXX_HW_RENDER_STATE_T *rs, bool color, bool depth, bool stencil, bool multisample)
{
   if (color)
   {
      rs->color_buffer_valid = false;
      rs->color_load = false;
   }

   if ((depth || !rs->installed_fb.have_depth) && (stencil || !rs->installed_fb.have_stencil))
      rs->ds_buffer_valid = false;

   if (multisample || !rs->installed_fb.ms)
      rs->ms_color_buffer_valid = false;
}

bool glxx_hw_start_frame_internal(GLXX_HW_RENDER_STATE_T *rs, GLXX_HW_FRAMEBUFFER_T *fb)
{
   uint32_t tilesize = fb->ms ? (KHRN_HW_TILE_HEIGHT>>1) : KHRN_HW_TILE_HEIGHT;

   assert(render_state == NULL);

   rs->xxx_empty = false;
   rs->installed_fb.color = NULL;
   rs->installed_fb.depth = NULL;
   rs->installed_fb.ms_color = NULL;
   rs->hw_frame_count = 0;

   KHRN_IMAGE_T *color = fb->color;
   bool color_valid_to_load = !khrn_interlock_is_invalid(&color->interlock);
   khrn_interlock_write(&color->interlock, khrn_interlock_user(rs->name));

   if(!glxx_lock_fixer_stuff(rs))
      goto quit2;

   render_state->num_tiles_x = (fb->pad_width + tilesize - 1) / tilesize;
   render_state->num_tiles_y = (fb->pad_height + tilesize - 1) / tilesize;

   if (render_state->num_tiles_x == 0 || render_state->num_tiles_y == 0)
      goto quit;

   render_state->installed_fb.ms = fb->ms;

   render_state->fmem = khrn_fmem_init(khrn_interlock_user(render_state->name));
   if (!render_state->fmem)
      goto quit;
   if (!create_bin_cl())
      goto quit;

   KHRN_MEM_ASSIGN(render_state->installed_fb.color, fb->color);
   KHRN_MEM_ASSIGN(render_state->installed_fb.depth, fb->depth);
   KHRN_MEM_ASSIGN(render_state->installed_fb.ms_color, fb->ms_color);

   render_state->installed_fb.width = fb->width;
   render_state->installed_fb.height = fb->height;
   render_state->installed_fb.pad_width = fb->pad_width;
   render_state->installed_fb.pad_height = fb->pad_height;
   render_state->installed_fb.col_format = fb->col_format;
   render_state->installed_fb.flags = fb->flags;
   render_state->installed_fb.have_depth = fb->have_depth;
   render_state->installed_fb.have_stencil = fb->have_stencil;

   bool depth_storage = false;
   bool depth_valid_to_load = false;
   if (fb->depth != NULL)
   {
      KHRN_IMAGE_T *depth = fb->depth;
      depth_valid_to_load = !khrn_interlock_is_invalid(&depth->interlock);
      khrn_interlock_write(&depth->interlock, khrn_interlock_user(rs->name));
      depth_storage = (depth->mh_storage != MEM_HANDLE_INVALID);
   }

   if (fb->ms_color != NULL)
   {
      KHRN_IMAGE_T *ms_color_image = fb->ms_color;
      khrn_interlock_write(&ms_color_image->interlock, khrn_interlock_user(rs->name));
   }

   render_state->color_buffer_valid = true;
   // render_state->ds_buffer_valid = depth_storage; // This is not sensible, since the storage won't be created if the buffer is invalid.
                                                   // There would be a circular dependency between ds_buffer_valid and mh_storage
   render_state->ds_buffer_valid = true;
   render_state->ms_color_buffer_valid = true;
   render_state->color_load = color_valid_to_load;
   render_state->depth_load = depth_valid_to_load && fb->have_depth && depth_storage;
   render_state->stencil_load = depth_valid_to_load && fb->have_stencil && depth_storage;
   render_state->drawn = false;
   render_state->hw_cleared = false;
   render_state->stencil_used = render_state->stencil_load;
   render_state->vshader_has_texture = false;
#ifndef NDEBUG
   /* useful visual indicator that the color buffer has become undefined */
   render_state->color_value = 0xff0000ff;
#else
   /* This color is pushed out to the tiles when they are undefined.
      This can happen if the user calls eglSwapBuffers() followed by eglSwapBuffers() */
   render_state->color_value = 0;
#endif
   render_state->depth_value = 1.0f;
   render_state->stencil_value = 0;
   render_state->dither = true;

   if (khrn_workarounds.HW2116)
      render_state->batch_count = 0;

   glxx_unlock_fixer_stuff();

   return true;
quit:
   glxx_unlock_fixer_stuff();
quit2:
   color = fb->color;
   khrn_interlock_release(&color->interlock, khrn_interlock_user(rs->name));
   return false;
}

static bool create_master_cl(void)
{
   uint8_t *instr;
   uint32_t offset;
   uint8_t pixel_format;

   instr = glxx_big_mem_alloc_cle(39);

   if(!instr)
      goto fail;

   KHRN_IMAGE_T *color = render_state->installed_fb.color;
   KHRN_IMAGE_FORMAT_T col_format = render_state->installed_fb.col_format;

   // Clear colour and depth
   add_byte(&instr, KHRN_HW_INSTR_STATE_CLEARCOL);   //(14)

   add_word(&instr, render_state->color_value);
   add_word(&instr, render_state->color_value);
   add_word(&instr, (uint32_t)(render_state->depth_value * 16777215.0f));
   add_byte(&instr, render_state->stencil_value);

   add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_RENDERING_MODE);  //(11)

   assert(col_format == ABGR_8888_RSO ||
          col_format == XBGR_8888_RSO ||
          col_format == ARGB_8888_RSO ||
          col_format == XRGB_8888_RSO ||
          col_format == RGBA_8888_RSO ||
          col_format == RGBX_8888_RSO ||
          col_format == BGRA_8888_RSO ||
          col_format == BGRX_8888_RSO ||
          col_format == RGB_565_RSO   ||
          col_format == ABGR_8888_TF  ||
          col_format == XBGR_8888_TF  ||
          col_format == ARGB_8888_TF  ||
          col_format == XRGB_8888_TF  ||
          col_format == RGBA_8888_TF  ||
          col_format == BGRA_8888_TF  ||
          col_format == RGBX_8888_TF  ||
          col_format == RGB_565_TF    ||
          col_format == ABGR_8888_LT  ||
          col_format == XBGR_8888_LT  ||
          col_format == ARGB_8888_LT  ||
          col_format == XRGB_8888_LT  ||
          col_format == RGBA_8888_LT  ||
          col_format == RGBX_8888_LT  ||
          col_format == RGB_565_LT
          );

   if (khrn_workarounds.FB_TOP_DOWN)
   {
      if ((color->flags & IMAGE_FLAG_DISPLAY) && (khrn_image_is_rso(color->format)))
         /* rendering upside down causes the HW to decrement its pointers rather than increment.  Cause this to point to the end of the buffer */
         offset = color->offset + ((color->height - 1) * color->stride);
      else
         offset = color->offset;
   }
   else
      offset = color->offset;

   if (!glxx_big_mem_add_fix(&instr, color->mh_storage, offset))
      goto fail;

   add_short(&instr, render_state->installed_fb.pad_width);   //Width (pixels)

   /* Calculate pixel format (0 = 565 dithered, 1 = 8888, 2 = 565 not dithered) */
   pixel_format = 1<<2;
   if (col_format == RGB_565_RSO || col_format == RGB_565_TF || col_format == RGB_565_LT)
   {
      if (render_state->dither && !khrn_options.force_dither_off)
         pixel_format = 0<<2;
      else
         pixel_format = 2<<2;
   }

   add_short(&instr, render_state->installed_fb.pad_height);  //Height (pixels)
   add_byte(&instr,
      render_state->installed_fb.ms<<0 |                                                                                  /* Multisample mode */
      0<<1 |                                                                                       /* Not 64-bit */
      pixel_format |
      render_state->installed_fb.ms<<4 |                                                                                  /* 1x or 4x decimation */
      (khrn_image_is_tformat(render_state->installed_fb.col_format) ? (1<<6) :
       khrn_image_is_lineartile(render_state->installed_fb.col_format) ? (2<<6) :
       (((khrn_workarounds.FB_TOP_DOWN) && (color->flags & IMAGE_FLAG_DISPLAY) &&
          khrn_image_is_rso(render_state->installed_fb.col_format)) ? (3<<6) : (0<<6)))
      );                                                   /* Memory format */
   add_byte(&instr, 0);     // unused

   /*
      Clear tile buffer
      On 2708A0 this requires a dummy store. On 2708B0 we disable the store.
   */
   add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_COORDS);     //(3)
   add_byte(&instr, 0);
   add_byte(&instr, 0);
   add_byte(&instr, KHRN_HW_INSTR_STORE_GENERAL);         //(7)
   add_short(&instr, 0);                          /* store = none */
   add_word(&instr, 0);                           /* no address needed */

   if (khrn_workarounds.GFXH30)
   {
      /* 2760sim will complain if it gets a primitive with rasosm=1 but tlbms=0.
      * this could happen if we didn't have a STATE_CFG before the first
      * primitive. we don't put STATE_CFGs before the GFXH-30 workaround
      * primitives, so put one here to cover the case where the first primitive is
      * a GFXH-30 workaround primitive */
      add_byte(&instr, KHRN_HW_INSTR_STATE_CFG);             //(4)
      add_byte(&instr, 0); /* rasosm=0 is fine whatever tlbms is */
      add_byte(&instr, 0);
      add_byte(&instr, 0);
   }
   else
   {
      add_byte(&instr, KHRN_HW_INSTR_NOP);
      add_byte(&instr, KHRN_HW_INSTR_NOP);
      add_byte(&instr, KHRN_HW_INSTR_NOP);
      add_byte(&instr, KHRN_HW_INSTR_NOP);
   }

   // Populate the master control list with branch instructions to tile control lists
   if(!populate_master_cl(&render_state->installed_fb))
      goto fail;

   return true;
fail:
   return false;
}

static bool create_bin_cl(void)
{
   uint8_t *instr;
   uint32_t state_size;

   khrn_fmem_start_bin(render_state->fmem);
   instr = glxx_big_mem_alloc_cle(19);
   if(!instr)
      goto fail;
   add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_BINNING_MODE);  //(16)

   state_size = render_state->num_tiles_x * render_state->num_tiles_y * KHRN_HW_TILE_STATE_SIZE;

   if (!glxx_big_mem_add_special(&instr, KHRN_FMEM_SPECIAL_BIN_MEM, 0)) goto fail;
   if (!glxx_big_mem_add_special(&instr, KHRN_FMEM_SPECIAL_BIN_MEM_SIZE, -(int)state_size)) goto fail;
   if (!glxx_big_mem_add_special(&instr, KHRN_FMEM_SPECIAL_BIN_MEM_END, -(int)state_size)) goto fail;

   add_byte(&instr, render_state->num_tiles_x);
   add_byte(&instr, render_state->num_tiles_y);
   add_byte(&instr,
         (render_state->installed_fb.ms ? 1 : 0) |   //Multisample mode
         0<<1   |   //64-bit
         1<<2   |   //Auto-initialise tile state data array
         0<<3   |   //Tile allocation initial block size (32 bytes)
         2<<5);      //Tile allocation block size (32 bytes)
   add_byte(&instr, KHRN_HW_INSTR_START_TILE_BINNING);       //(1)

#ifdef ALLOW_FORCE_LOCKUP
   {
      char buf[64];
      sprintf(buf, "/%d.bin", vcos_process_id_current());
      FILE *fp = fopen(buf, "r");
      if (fp != NULL)
      {
         fclose(fp);
         printf("ISSUING WAIT SEM\n");
         add_byte(&instr, KHRN_HW_INSTR_WAIT_SEMAPHORE);
         add_byte(&instr, KHRN_HW_INSTR_WAIT_SEMAPHORE);
      }
      else
      {
         /* Ensure primitive format is reset. TODO: is this necessary? */
         add_byte(&instr, KHRN_HW_INSTR_PRIMITIVE_LIST_FORMAT); //(2)
         add_byte(&instr, 0x12);   /* 16 bit triangle */
      }
   }
#else
   /* Ensure primitive format is reset. TODO: is this necessary? */
   add_byte(&instr, KHRN_HW_INSTR_PRIMITIVE_LIST_FORMAT); //(2)
   add_byte(&instr, 0x12);   /* 16 bit triangle */
#endif

   return true;
fail:
   glxx_unlock_fixer_stuff();
   return false;
}

bool glxx_hw_render_state_would_flush(GLXX_HW_RENDER_STATE_T *rs)
{
   assert(!rs->xxx_empty);
   return (rs->drawn || rs->hw_cleared);
}

bool glxx_hw_render_state_flush(GLXX_HW_RENDER_STATE_T *rs)
{
   GLXX_HW_RENDER_STATE_T *stashed_rs = NULL;

   assert(!rs->xxx_empty);

   if (rs->drawn || rs->hw_cleared)
   {
      if (DRAW_TEX_LOGGING)
      {
         vcos_log(LOG_INFO, "render_state_flush: rs: %d", rs->name);
         vcos_log(LOG_INFO, "--------------------");
      }

      /* Interlock transfer delayed until we know that we can't fail */

      /*
      This function can be called in the middle of doing other fixer stuff, so need to
      push and pop static render state
      */
      if (render_state)
      {
         stashed_rs = render_state;
         glxx_unlock_fixer_stuff();
      }

      if (!vcos_verify(glxx_lock_fixer_stuff(rs)))//lock fixer stuff can only fail in start_frame
         goto quit2;

      // Terminate tile control lists if necessary
      uint8_t *instr = glxx_big_mem_alloc_cle(2);
      if (!instr)
         goto quit;

      add_byte(&instr, KHRN_HW_INSTR_NOP);   //(1)
      add_byte(&instr, KHRN_HW_INSTR_FLUSH);

      if (!khrn_fmem_start_render(render_state->fmem))
         goto quit;

      if (!create_master_cl())
         goto quit;

      glxx_unlock_fixer_stuff();

      /* Now transfer everything */
      KHRN_IMAGE_T *color = rs->installed_fb.color;

      bool secure = color->secure;
      uint64_t v3dfence = color->v3dfence;
      color->v3dfence = 0;

      if (v3dfence)
         khrn_issue_fence_wait_job(v3dfence);

      khrn_interlock_transfer(&color->interlock, khrn_interlock_user(rs->name), KHRN_INTERLOCK_FIFO_HW_RENDER);

      if (rs->installed_fb.depth != NULL)
      {
         KHRN_IMAGE_T *depth = rs->installed_fb.depth;
         khrn_interlock_transfer(&depth->interlock, khrn_interlock_user(rs->name), KHRN_INTERLOCK_FIFO_HW_RENDER);
      }

      if (rs->installed_fb.ms_color != NULL)
      {
         KHRN_IMAGE_T *ms_color = rs->installed_fb.ms_color;
         khrn_interlock_transfer(&ms_color->interlock, khrn_interlock_user(rs->name), KHRN_INTERLOCK_FIFO_HW_RENDER);
      }

      /* Submit a job */
      khrn_issue_bin_render_job(rs, secure);

      KHRN_MEM_ASSIGN(rs->installed_fb.color, NULL);
      KHRN_MEM_ASSIGN(rs->installed_fb.depth, NULL);
      KHRN_MEM_ASSIGN(rs->installed_fb.ms_color, NULL);

      khrn_render_state_finish(rs->name);
      if (stashed_rs) glxx_lock_fixer_stuff(stashed_rs);
   }
   else
      glxx_hw_discard_frame(rs);

   return true;

quit:
   glxx_unlock_fixer_stuff();
quit2:
   glxx_hw_discard_frame(rs);
   if (stashed_rs) glxx_lock_fixer_stuff(stashed_rs);
   return false;
}


/*!
 * \brief Sets up the master control list by inserting the branches to all the per-tile lists.
 * Also inserts a tile coordinate instruction before each one and a store subsample afterwards.
 */
static bool populate_master_cl(GLXX_HW_FRAMEBUFFER_T *fb)
{
   KHRN_IMAGE_T *depth = NULL;
   if (fb->depth != NULL)
      depth = fb->depth;

   KHRN_IMAGE_T *ms_color = NULL;
   if (fb->ms_color != NULL)
      ms_color = fb->ms_color;

   KHRN_IMAGE_T *color_image = fb->color;

   bool load_standard = render_state->color_load && !fb->ms;
   bool load_full_color = fb->ms && render_state->color_load;
   bool load_full_depth = render_state->depth_load || render_state->stencil_load;
   bool store_full_color = fb->ms && render_state->ms_color_buffer_valid;
   bool store_full_depth = (fb->have_depth || fb->have_stencil) && render_state->ds_buffer_valid;

   bool load_full = load_full_color || load_full_depth;
   bool store_full = store_full_color || store_full_depth;

   MEM_HANDLE_T ds_handle = MEM_HANDLE_INVALID;
   MEM_HANDLE_T ms_color_handle = MEM_HANDLE_INVALID;
   uint32_t ds_stride = 0, ds_offset = 0;
   uint32_t ms_color_stride = 0, ms_color_offset = 0;
   if (load_full || store_full)
   {
      if (load_full_depth || store_full_depth)
      {
         assert(depth != NULL);
         assert(depth->format == DEPTH_32_TLBD);
         ds_stride = khrn_image_get_bpp(depth->format) * (KHRN_HW_TILE_HEIGHT * KHRN_HW_TILE_WIDTH / 8);
         ds_offset = depth->offset;
         /* Allocate some memory if necessary. TODO: this code should probably be in khrn_image */
         if (!khrn_image_alloc_storage(depth, "KHRN_IMAGE_T.storage OpenGL ES depth buffer"))
            return false;

         ds_handle = depth->mh_storage;
      }

      if (load_full_color || store_full_color)
      {
         assert(ms_color != NULL);
         assert(ms_color->format == COL_32_TLBD);
         ms_color_stride = khrn_image_get_bpp(ms_color->format) * (KHRN_HW_TILE_HEIGHT * KHRN_HW_TILE_WIDTH / 8);
         ms_color_offset = ms_color->offset;
         if (!khrn_image_alloc_storage(ms_color, "KHRN_IMAGE_T.storage OpenGL ES ms color buffer"))
            return false;

         ms_color_handle = ms_color->mh_storage;
      }
   }
   assert(color_image->mh_storage != MEM_HANDLE_INVALID);

   khrn_driver_incr_counters(KHRN_PERF_TB_GRP_COLOR_STORES);

   /* Decide how much room we need for an instruction */
   uint32_t alloc_tile = 11;

   if (load_standard)
   {
      alloc_tile += 7;
      khrn_driver_incr_counters(KHRN_PERF_TB_GRP_COLOR_LOADS);
   }
   if (load_full)
   {
      if (load_full_color)
      {
         alloc_tile += 5;
         khrn_driver_incr_counters(KHRN_PERF_TB_GRP_MS_COLOR_LOADS);
      }

      if (load_full_depth)
      {
         alloc_tile += 5;
         khrn_driver_incr_counters(KHRN_PERF_TB_GRP_DS_LOADS);
      }

      if (load_full_color && load_full_depth)
         // Tile coordinates and store command between the two loads
         alloc_tile += 10;
   }
   if (load_standard && load_full)
   {
      alloc_tile += 10;
   }
   if (store_full)
   {
      if (store_full_color)
      {
         alloc_tile += 8;
         khrn_driver_incr_counters(KHRN_PERF_TB_GRP_MS_COLOR_STORES);
      }
      if (store_full_depth)
      {
         alloc_tile += 8;
         khrn_driver_incr_counters(KHRN_PERF_TB_GRP_DS_STORES);
      }
   }

   bool alloc_by_row = false;
   uint32_t alloc_row = alloc_tile * render_state->num_tiles_x;
   uint32_t *gfxh30_shader_rec = NULL;
   MEM_LOCK_T gfxh30_shader_rec_lbh;
   uint32_t *gfxh30_vertices;
   if (!khrn_workarounds.GFXH30)
   {
      alloc_by_row = (alloc_row < 2048);
   }
   else
   {
      MEM_LOCK_T lbh;
      gfxh30_vertices = glxx_big_mem_alloc_junk(36, 4, &lbh);
      if (!gfxh30_vertices) goto fail;
      gfxh30_vertices[0] = 0xff00ff00;
      gfxh30_vertices[1] = 0;
      gfxh30_vertices[2] = 0;
      gfxh30_vertices[3] = 0xff00ff00;
      gfxh30_vertices[4] = 0;
      gfxh30_vertices[5] = 0;
      gfxh30_vertices[6] = 0xff00ff01;
      gfxh30_vertices[7] = 0;
      gfxh30_vertices[8] = 0;

      gfxh30_shader_rec = glxx_big_mem_alloc_junk(16, 16, &gfxh30_shader_rec_lbh);
      if (!gfxh30_shader_rec) goto fail;
      gfxh30_shader_rec[0] = 12 << 8;
      gfxh30_shader_rec[1] = 0;
      gfxh30_shader_rec[2] = 0;
      gfxh30_shader_rec[3] = khrn_hw_addr(gfxh30_vertices, &lbh);
   }

   /*
      Insert a series of:
      - tile coords
      - branch instructions
      - store subsample instructions
      into the master control list.
   */
   uint8_t *instr;
   for (unsigned int y = 0; y < render_state->num_tiles_y; y++) {
      if (alloc_by_row) {
         uint32_t alloc = alloc_row;
         instr = glxx_big_mem_alloc_cle(alloc);
         if(!instr)
            goto fail;
      }
      for (unsigned int x = 0; x < render_state->num_tiles_x; x++) {
         uint32_t gfxh30_counter = 0;
         if (!alloc_by_row) {
            uint32_t alloc = alloc_tile;
            if (khrn_workarounds.GFXH30)
            {
               gfxh30_counter++;
               if (gfxh30_counter == 5)
               {
                  gfxh30_counter = 0;
                  alloc += 15;
               }
            }
            instr = glxx_big_mem_alloc_cle(alloc);
            if(!instr)
               goto fail;
         }

         if (load_standard)
         {
            uint16_t flags;
            uint32_t offset;
            KHRN_IMAGE_T *image;
            KHRN_IMAGE_FORMAT_T col_format;

            if (load_standard)
               image = color_image;

            khrn_driver_incr_counters(KHRN_PERF_TB_COLOR_LOADS);

            add_byte(&instr, KHRN_HW_INSTR_LOAD_GENERAL);          /*(7) */

            flags = 1;      /* load = colour */
            if (khrn_image_is_tformat(fb->col_format))
               flags |= (1 << 4);
            else if (khrn_image_is_lineartile(fb->col_format))
               flags |= (2 << 4);
            else if ((khrn_workarounds.FB_TOP_DOWN) && (fb->flags & IMAGE_FLAG_DISPLAY) && khrn_image_is_rso(fb->col_format))
               flags |= (3 << 4);

            col_format = khrn_image_to_rso_format(fb->col_format);

            switch (col_format)
            {
            case ABGR_8888_RSO:
            case XBGR_8888_RSO:
            case RGBA_8888_RSO:
            case RGBX_8888_RSO:
            case BGRA_8888_RSO:
            case BGRX_8888_RSO:
            case ARGB_8888_RSO:
            case XRGB_8888_RSO: flags |= 0<<8; break;
            case RGB_565_RSO: flags |= 2<<8; break;
            default: UNREACHABLE();
            }
            add_short(&instr, flags);

            if (khrn_workarounds.FB_TOP_DOWN)
            {
               if ((fb->flags & IMAGE_FLAG_DISPLAY) && (khrn_image_is_rso(image->format)))
                  /* rendering upside down causes the HW to decrement its pointers rather than increment.  Cause this to point to the end of the buffer */
                  offset = image->offset + ((image->height - 1) * image->stride);
               else
                  offset = image->offset;
            }
            else
            {
               offset = image->offset;
            }

            if (! glxx_big_mem_add_fix(&instr, image->mh_storage, offset) ) return false;
         }

         if (load_standard && load_full)
         {
            add_byte(&instr, KHRN_HW_INSTR_STORE_GENERAL);          //(7)
            add_short(&instr, 0 | 1 << 13 | 1 << 14);      /* store = none. disable colour & depth clear */
            add_word(&instr, 0);                           /* no address needed */
            add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_COORDS);      //(3)
            add_byte(&instr, x);
            add_byte(&instr, y);
         }

         if (load_full)
         {
            uint32_t flags;

            // Loading depth from multisample buffer
            if (load_full_depth)
            {
               khrn_driver_incr_counters(KHRN_PERF_TB_DS_LOADS);
               assert(ds_offset + ds_stride * (y * render_state->num_tiles_x + x) < mem_get_size(ds_handle));

               add_byte(&instr, KHRN_HW_INSTR_LOAD_FULL);          //(5)

               flags = 0;
               flags |= 1<<0; /* disable colour read */

               if (! glxx_big_mem_add_fix(&instr,
                     ds_handle,
                           ds_offset + ds_stride * (y * render_state->num_tiles_x + x) + flags) )
               {
                  return false;
               }
            }

            // Doing two loads so we need to action the first load with
            // a tile coords and a store commands
            if (load_full_depth && load_full_color)
            {
               add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_COORDS);   //(3)
               add_byte(&instr, x);
               add_byte(&instr, y);

               add_byte(&instr, KHRN_HW_INSTR_STORE_GENERAL);          //(7)
                           add_short(&instr, 0 | 1 << 13 | 1 << 14);      /* store = none. disable colour & depth clear */
                           add_word(&instr, 0);                           /* no address needed */
            }

            // Loading the colour form the multisample buffer
            if (load_full_color)
            {
               khrn_driver_incr_counters(KHRN_PERF_TB_MS_COLOR_LOADS);
               assert(ms_color_offset + ms_color_stride * (y * render_state->num_tiles_x + x) < mem_get_size(ms_color_handle));

               add_byte(&instr, KHRN_HW_INSTR_LOAD_FULL);          //(5)

               flags = 0;
               flags |= 1<<1;            /* disable depth read */

               if (! glxx_big_mem_add_fix(&instr,
                     ms_color_handle,
                     ms_color_offset + ms_color_stride * (y * render_state->num_tiles_x + x) + flags) )
               {
                  return false;
               }
            }
         }

#ifdef ALLOW_FORCE_LOCKUP
         {
            char buf[64];
            sprintf(buf, "/%d.rdr", vcos_process_id_current());
            FILE *fp = fopen(buf, "r");
            if (fp != NULL)
            {
               fclose(fp);
               printf("ISSUING WAIT SEM\n");
               add_byte(&instr, KHRN_HW_INSTR_WAIT_SEMAPHORE);
               add_byte(&instr, KHRN_HW_INSTR_WAIT_SEMAPHORE);
               add_byte(&instr, KHRN_HW_INSTR_WAIT_SEMAPHORE);
            }
            else
            {
               add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_COORDS);   //(3)
               add_byte(&instr, x);
               add_byte(&instr, y);
            }
         }
#else
         add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_COORDS);   //(3)
         add_byte(&instr, x);
         add_byte(&instr, y);
#endif

         /* Ensure primitive format is reset. TODO: is this necessary? */
         add_byte(&instr, KHRN_HW_INSTR_PRIMITIVE_LIST_FORMAT); //(2)
         add_byte(&instr, 0x12);   /* 16 bit triangle */

         if (khrn_workarounds.GFXH30)
         {
            if (gfxh30_counter == 0)
            {
               add_byte(&instr, KHRN_HW_INSTR_NV_SHADER);    //(5; dummy)
               add_pointer(&instr, gfxh30_shader_rec, &gfxh30_shader_rec_lbh);
               add_byte(&instr, KHRN_HW_INSTR_GLDRAWARRAYS); //(10; dummy)
               add_byte(&instr, 1<<4 | 4);     /* triangles */
               add_word(&instr, 3);
               add_word(&instr, 0);
            }
         }

         add_byte(&instr, KHRN_HW_INSTR_BRANCH_SUB);          //(5)

         if (! glxx_big_mem_add_special(&instr,
                     KHRN_FMEM_SPECIAL_BIN_MEM,
                     (x + y * render_state->num_tiles_x) * KHRN_HW_CL_BLOCK_SIZE_MIN) )
         {
            return false;
         }

         if (store_full)
         {
            uint32_t flags;

            if (store_full_depth)
            {
               khrn_driver_incr_counters(KHRN_PERF_TB_DS_STORES);

               assert(ds_offset + ds_stride * (y * render_state->num_tiles_x + x) < mem_get_size(ds_handle));

               flags = 1<<2;                               /* disable clear. Not last tile in frame. */
               flags |= 1<<0; /* disable colour write */

               add_byte(&instr, KHRN_HW_INSTR_STORE_FULL);          //(5)

               if (! glxx_big_mem_add_fix(&instr,
                     ds_handle,
                     ds_offset + ds_stride * (y * render_state->num_tiles_x + x) + flags) )
               {
                  return false;
               }

               add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_COORDS);   //(3)
               add_byte(&instr, x);
               add_byte(&instr, y);

            }

            if (store_full_color)
            {
               khrn_driver_incr_counters(KHRN_PERF_TB_MS_COLOR_STORES);

               assert(ms_color_offset + ms_color_stride * (y * render_state->num_tiles_x + x) < mem_get_size(ms_color_handle));

               flags = 1<<2;                               /* disable clear. Not last tile in frame. */
               flags |= 1<<1;            /* disable depth write */

               add_byte(&instr, KHRN_HW_INSTR_STORE_FULL);          //(5)

               if (! glxx_big_mem_add_fix(&instr,
                     ms_color_handle,
                     ms_color_offset + ms_color_stride * (y * render_state->num_tiles_x + x) + flags) )
               {
                  return false;
               }

               add_byte(&instr, KHRN_HW_INSTR_STATE_TILE_COORDS);   //(3)
               add_byte(&instr, x);
               add_byte(&instr, y);

            }
         }

         if (x == render_state->num_tiles_x - 1 && y == render_state->num_tiles_y - 1) {
            add_byte(&instr, KHRN_HW_INSTR_STORE_SUBSAMPLE_EOF);     // (1). Last tile needs special store instruction
            render_state->hw_frame_count++;
         }
         else
         {
            add_byte(&instr, KHRN_HW_INSTR_STORE_SUBSAMPLE);
         }

         khrn_driver_incr_counters(KHRN_PERF_TB_COLOR_STORES);
      }
   }

   return true;

fail:

   return false;
}

static bool draw_rect(
   GLXX_SERVER_STATE_T *state,
   GLXX_HW_RENDER_STATE_T *rs,
   GLXX_HW_FRAMEBUFFER_T *fb,
   bool color, bool depth, bool stencil,
   int x, int y, int xmax, int ymax)
{
   if(!vcos_verify(glxx_lock_fixer_stuff(rs)))//lock fixer stuff can only fail in start_frame
      return false;

   render_state->drawn = true;
   render_state->color_buffer_valid = true;
   if (fb->have_depth || fb->have_stencil)
      render_state->ds_buffer_valid = true;
   if (fb->ms)
      render_state->ms_color_buffer_valid = true;

   bool egl_output;
   if (khrn_workarounds.FB_BOTTOM_UP)
   {
      /* work off if the output image is an EGL image.  This can be a wrapped image, or an eglSurface or a Pixmap */
      /* used to only trigger of RSO, but make it work from any egl image irrespective of format */
      egl_output = (fb->flags & IMAGE_FLAG_DISPLAY) ? true : false;
   }
   else if (khrn_workarounds.FB_TOP_DOWN)
   {
      /* on a B1 we can't rasterize TFormat upside down in HW, so we need to import the original flip from B0 */
      egl_output = ((fb->flags & IMAGE_FLAG_DISPLAY) &&
                  (khrn_image_is_tformat(fb->col_format) || khrn_image_is_lineartile(fb->col_format))) ? true : false;
   }
   else
      egl_output = false;

   // Set up control list
   uint8_t *instr = glxx_big_mem_alloc_cle(34);
   if(!instr)
      goto fail;

   /*
    * Emit scissor/clipper/viewport instructions
    */
   add_byte(&instr, KHRN_HW_INSTR_STATE_CLIP);     //(9)
   add_short(&instr, x);
   if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) && egl_output)
      add_short(&instr, fb->height - ymax);
   else
      add_short(&instr, y);
   add_short(&instr, xmax - x);
   add_short(&instr, ymax - y);
   state->changed_viewport = true;  /* Clear and render might end up with different clip rectangles - clear doesn't clip to viewport */

   /*
    * Emit a Configuration record
    */
   add_byte(&instr, KHRN_HW_INSTR_STATE_CFG);     /*(4) */
   if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) && egl_output)
      add_byte(&instr, 1 | 1<<1 | 1<<2);   /* enfwd, enrev, cwise */
   else
      add_byte(&instr, 1 | 1<<1);        /*enfwd, enrev */

   if (depth)
      add_byte(&instr, 7 << 4 | 1 << 7); /* zfunc=always, enzu */
   else if (stencil)
      add_byte(&instr, 7 << 4);          /* zfunc=always, !enzu */
   else
      add_byte(&instr, 0);               /* zfunc=never, !enzu */

   add_byte(&instr, 1<<1);            /* not enez, enezu */

   state->changed_cfg = true;       /* Clear and render probably use different configs */

   add_byte(&instr, KHRN_HW_INSTR_STATE_VIEWPORT_OFFSET);  //(5)
   add_short(&instr, 0);
   add_short(&instr, 0);

   //TODO: other miscellaneous pieces of state

   /* Select the correct clear shader */
   uint32_t selector = 0;
   if (color)
      selector |= CLR_COLOR;
   if (depth)
      selector |= CLR_DEPTH;
   if (stencil)
      selector |= CLR_STENCIL;

   if (color && state->shader.common.blend.color_mask != 0xFFFFFFFF)
      selector |= CLR_MASKED;

   if (fb->ms)
      selector |= CLR_MULTISAMPLE;

   uint32_t *shader_code = clear_shaders[selector];
   assert(shader_code);

   /* How many uniforms will we have? */

   uint32_t uniform_count = 0;
   if (stencil || depth)
      uniform_count += 2;
   if (color)
      uniform_count++;
   if (color && state->shader.common.blend.color_mask != 0xFFFFFFFF)
      uniform_count++;

   MEM_LOCK_T fshader_lbh;
   uint32_t *locked_addr = glxx_big_mem_alloc_junk(ALIGN_UP(clear_shader_sizes[selector], 16) + (ALIGN_UP(uniform_count, 4) * 4) + 48 + 16,
      16, &fshader_lbh);
   if (!locked_addr)
      goto fail;

   uint32_t *fshader = locked_addr;

   khrn_memcpy(locked_addr, shader_code, clear_shader_sizes[selector]);

   locked_addr += ALIGN_UP(clear_shader_sizes[selector], 16) / 4;
   uint32_t *funif = locked_addr;

   uint32_t uniform_indx = 0;

   /* First uniform is always color mask if needed */
   if (color && state->shader.common.blend.color_mask != 0xFFFFFFFF)
   {
      KHRN_IMAGE_FORMAT_T col_format;
      col_format = khrn_image_to_tf_format(fb->col_format);
      if (tu_image_format_rb_swap(col_format))
      {
         uint32_t color_mask = state->shader.common.blend.color_mask;
         color_mask = (color_mask & 0xff00ff00) | ((color_mask & 0xff0000) >> 16) | ((color_mask & 0xff) << 16);
         ((uint32_t *)locked_addr)[uniform_indx++] = color_mask;
      }
      else
         ((uint32_t *)locked_addr)[uniform_indx++] = state->shader.common.blend.color_mask;
   }

   /* Set the stencil mode uniform (if needed) */
   if (stencil || depth)
   {
      if (stencil)
      {
         ((uint32_t *)locked_addr)[uniform_indx++] = stencil_clear_mode(state->clear_stencil);
         ((uint32_t *)locked_addr)[uniform_indx++] = stencil_mask_mode(state->stencil_mask.front);
      }
      else
      {
         ((uint32_t *)locked_addr)[uniform_indx++] = stencil_keep_mode;
         ((uint32_t *)locked_addr)[uniform_indx++] = stencil_keep_mode;
      }
   }

   /* Set the colour uniform (if needed) */
   if (color)
   {
      KHRN_IMAGE_FORMAT_T col_format;
      col_format = khrn_image_to_tf_format(fb->col_format);
      if (tu_image_format_rb_swap(col_format))
      {
         ((uint32_t *)locked_addr)[uniform_indx++] = color_floats_to_rgba(
            state->clear_color[2],
            state->clear_color[1],
            state->clear_color[0],
            state->clear_color[3]);
      }
      else
      {
         ((uint32_t *)locked_addr)[uniform_indx++] = color_floats_to_rgba(
            state->clear_color[0],
            state->clear_color[1],
            state->clear_color[2],
            state->clear_color[3]);
      }
   }

   locked_addr += ALIGN_UP(uniform_count, 4);
   uint32_t *vdata = locked_addr;

   uint32_t z = float_to_bits(state->clear_depth);
   if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) && egl_output)
      vdata[0] = (fb->height - y) << 20 | x << 4;
   else
      vdata[0] = y << 20 | x << 4;
   vdata[1] = z;
   vdata[2] = 0x3f800000;
   if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) && egl_output)
      vdata[3] = (fb->height - y) << 20 | xmax << 4;
   else
      vdata[3] = y << 20 | xmax << 4;
   vdata[4] = z;
   vdata[5] = 0x3f800000;
   if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) && egl_output)
      vdata[6] = (fb->height - ymax) << 20 | xmax << 4;
   else
      vdata[6] = ymax << 20 | xmax << 4;
   vdata[7] = z;
   vdata[8] = 0x3f800000;
   if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) && egl_output)
      vdata[9] = (fb->height - ymax) << 20 | x << 4;
   else
      vdata[9] = ymax << 20 | x << 4;
   vdata[10] = z;
   vdata[11] = 0x3f800000;

   //TODO: stencil, masked color

   locked_addr += 12;
   uint32_t *rec = locked_addr;

   rec[0] = 1 | 12 << 8;  //flags: single-threaded. No point size, clipping, clip header. vstride=12
   rec[1] = khrn_hw_addr(fshader, &fshader_lbh); //PTR
   rec[2] = khrn_hw_addr(funif, &fshader_lbh);   //PTR
   rec[3] = khrn_hw_addr(vdata, &fshader_lbh);   //PTR

   add_byte(&instr, KHRN_HW_INSTR_NV_SHADER);     //(5)
   add_pointer(&instr, rec, &fshader_lbh);

   /*
    * Now insert a GLDrawElements or GLDrawArrays record
    */

   // Emit a GLDRAWARRAYS instruction
   add_byte(&instr, KHRN_HW_INSTR_GLDRAWARRAYS);
   add_byte(&instr, 6);                         //Primitive mode (triangle_fan)
   add_word(&instr, 4);                         //Length (number of vertices)
   add_word(&instr, 0);                         //Index of first vertex

   add_byte(&instr, KHRN_HW_INSTR_NOP);        //(1) TODO: is this necessary?

   glxx_unlock_fixer_stuff();
   return true;

fail:
   glxx_unlock_fixer_stuff();
   return false;
}

uint32_t glxx_hw_convert_operation(GLenum operation)
{
   switch (operation) {
   case GL_ZERO:
      return 0;
   case GL_KEEP:
      return 1;
   case GL_REPLACE:
      return 2;
   case GL_INCR:
      return 3;
   case GL_DECR:
      return 4;
   case GL_INVERT:
      return 5;
   case GL_INCR_WRAP:
      return 6;
   case GL_DECR_WRAP:
      return 7;
   default:
      UNREACHABLE();
      return 0;
   }
}

uint32_t glxx_hw_convert_test_function(GLenum function)
{
   assert(function >= GL_NEVER && function < GL_NEVER + 8);
   return function - GL_NEVER;
}


bool glxx_lock_fixer_stuff(GLXX_HW_RENDER_STATE_T *rs)
{
   assert(render_state == NULL);
   render_state = rs;
   return true;
}

void glxx_unlock_fixer_stuff(void)
{
   assert(render_state != NULL);
   render_state = NULL;
}


bool glxx_hw_texture_fix(GLXX_TEXTURE_T *texture, bool in_vshader)
{
   bool result = true;

   assert(render_state != NULL);

   /* This state is used to make sure that a previous render has completed
    * It is conservative because we don't check if a flush is needed
    * The effect is that a wait render is inserted into the job which introduces
    * a delay before the next render starts.
    * To improve this we should check if a flush is going to be precipitated
    */
   if (in_vshader)
      render_state->vshader_has_texture = true;

   khrn_interlock_read((KHRN_INTERLOCK_T *)((char *)texture + glxx_texture_get_interlock_offset(texture)), khrn_interlock_user(render_state->name));
   result &= glxx_hw_insert_interlock(texture, glxx_texture_get_interlock_offset(texture));

   if (texture->explicit_mipmaps) {
      int min_buffer = 0, max_buffer = 0, i, j;

      switch (texture->target) {
      case GL_TEXTURE_2D:
         min_buffer = TEXTURE_BUFFER_TWOD;
         max_buffer = TEXTURE_BUFFER_TWOD;
         break;
      case GL_TEXTURE_CUBE_MAP:
         min_buffer = TEXTURE_BUFFER_POSITIVE_X;
         max_buffer = TEXTURE_BUFFER_NEGATIVE_Z;
         break;
      case GL_TEXTURE_EXTERNAL_OES:
         min_buffer = TEXTURE_BUFFER_EXTERNAL;
         max_buffer = TEXTURE_BUFFER_EXTERNAL;
         break;
      default:
         UNREACHABLE();
      }

      for (i = min_buffer; i <= max_buffer; i++) {
         for (j = 0; j <= LOG2_MAX_TEXTURE_SIZE; j++) {
            if (texture->mipmaps[i][j] != NULL) {
               khrn_interlock_read(&texture->mipmaps[i][j]->interlock, khrn_interlock_user(render_state->name));
               result &= glxx_hw_insert_interlock(texture->mipmaps[i][j], offsetof(KHRN_IMAGE_T, interlock));
            }
         }
      }
   }
   return result;
}

void glxx_hw_discard_frame(GLXX_HW_RENDER_STATE_T *rs)
{
   //do the tidying up that glxx_hw_flush and hw_callback do
   //but without flushing to the hardware

   if (rs->installed_fb.color != NULL)
   {
      KHRN_IMAGE_T *color = rs->installed_fb.color;
      //release the interlock rather than transferring it
      khrn_interlock_release(&color->interlock, khrn_interlock_user(rs->name));
   }

   if (rs->installed_fb.depth != NULL)
   {
      KHRN_IMAGE_T *depth = rs->installed_fb.depth;
      //release the interlock rather than transferring it
      khrn_interlock_release(&depth->interlock, khrn_interlock_user(rs->name));
   }

   if (rs->installed_fb.ms_color != NULL)
   {
      KHRN_IMAGE_T *ms_color = rs->installed_fb.ms_color;
      //release the interlock rather than transferring it
      khrn_interlock_release(&ms_color->interlock, khrn_interlock_user(rs->name));
   }

   /*junk_mem = rs->current_junk_mem;
   while (junk_mem != MEM_HANDLE_INVALID)
   {
      MEM_HANDLE_T next_junk_mem;
      next_junk_mem = *(MEM_HANDLE_T *)mem_lock(junk_mem);
      mem_unlock(junk_mem);
      free_junk_mem(junk_mem);
      junk_mem = next_junk_mem;
   }

   rs->current_junk_mem = MEM_HANDLE_INVALID;
   rs->current_junk_offset = 0;*/

   rs->hw_frame_count = 0;

   if (rs->fmem) khrn_fmem_discard(rs->fmem);

   KHRN_MEM_ASSIGN(rs->installed_fb.color, NULL);
   KHRN_MEM_ASSIGN(rs->installed_fb.depth, NULL);
   KHRN_MEM_ASSIGN(rs->installed_fb.ms_color, NULL);

   khrn_render_state_finish(rs->name);
}

uint32_t glxx_render_state_name(void)
{
   assert(render_state != NULL);
   return render_state->name;
}

uint8_t *glxx_big_mem_alloc_cle(int size)
{
   assert(render_state != NULL);
   return khrn_fmem_cle(render_state->fmem, size);
}

uint32_t *glxx_big_mem_alloc_junk(int size, int align, MEM_LOCK_T *lbh)
{
   assert(render_state != NULL);
   return khrn_fmem_junk(render_state->fmem, size, align, lbh);
}

bool glxx_big_mem_insert(uint32_t *location, MEM_HANDLE_T handle, uint32_t offset)
{
   assert(render_state != NULL);
   return khrn_fmem_fix(render_state->fmem, location, handle, offset);
}

bool glxx_big_mem_add_fix(uint8_t **p, MEM_HANDLE_T handle, uint32_t offset)
{
   assert(render_state != NULL);
   return khrn_fmem_add_fix(render_state->fmem, p, handle, offset);
}

bool glxx_big_mem_add_special(uint8_t **p, uint32_t special_i, uint32_t offset)
{
   assert(render_state != NULL);
   return khrn_fmem_add_special(render_state->fmem, p, special_i, offset);
}

bool glxx_hw_insert_interlock(void *p, uint32_t offset)
{
   assert(render_state != NULL);
   return khrn_fmem_interlock(render_state->fmem, p, offset);
}
