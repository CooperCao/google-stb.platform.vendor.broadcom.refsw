/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_color.h"
#include "interface/khronos/common/khrn_options.h"
#include "interface/khronos/include/GLES2/gl2.h"
#include "interface/khronos/include/GLES2/gl2ext.h"
#include "interface/khronos/common/khrn_api_interposer.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/common/2708/khrn_tfconvert_4.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"
#include "middleware/khronos/glxx/2708/glxx_tu_4.h"
#include "middleware/khronos/glxx/2708/glxx_shader_4.h"
#include "middleware/khronos/glxx/2708/glxx_attr_sort_4.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/gl20/2708/gl20_support_4.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"
#include "middleware/khronos/gl11/2708/gl11_support_4.h"
#include "middleware/khronos/gl11/2708/gl11_shader_4.h"
#include "middleware/khronos/ext/egl_khr_image.h"

#include "interface/khronos/common/khrn_client_platform.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/glxx/glxx_tweaker.h"

#include "vcfw/rtos/rtos.h"

#include <limits.h>
#if WIN32
#define snprintf sprintf_s
#endif

// David Emett: I flush in VG when the batch count goes over 0x1ef0
// David Emett: the hardware flushes on 0x1f01 i think
#define HW2116_BATCH_COUNT_LIMIT 0x1ef0

// Global variables -------------------------------------------

static uint32_t *dummy_texture_data = NULL;
static MEM_HANDLE_T dummy_texture_handle = MEM_HANDLE_INVALID;
static MEM_LOCK_T dummy_texture_lbh = { 0 };

/*************************************************************
 Static function forwards
 *************************************************************/
static bool install_uniforms(
   uint32_t *startaddr_location,
   GLXX_SERVER_STATE_T *state,
   GL20_PROGRAM_T *program,
   uint32_t count,
   MEM_HANDLE_T hmap,
   GL20_HW_INDEXED_UNIFORM_T *iu,
   uint32_t * num_vpm_rows,
   GLXX_ATTRIB_T *attrib,
   bool egl_output,
   unsigned int fb_height,
   bool secure);

static bool get_shaders(
    GL20_PROGRAM_T *program,
    GLXX_HW_SHADER_RECORD_T *shader_out,
    MEM_HANDLE_T *cunifmap_out,
    MEM_HANDLE_T *vunifmap_out,
    MEM_HANDLE_T *funifmap_out,
    uint32_t *color_varyings_out,
    GLXX_SERVER_STATE_T *state,
    GLXX_ATTRIB_T *attrib,
    uint32_t *mergeable_attribs,
    uint32_t *cattribs_order,
    uint32_t *vattribs_order);

bool do_vcd_setup(
   GLXX_HW_SHADER_RECORD_T *shader_record,
   GLXX_ATTRIB_T *attrib,
   MEM_HANDLE_T *attrib_handles,
   uint32_t cattribs_live,
   uint32_t vattribs_live,
   uint32_t * mergeable_attribs,
   uint32_t * cattribs_order,
   uint32_t * vattribs_order,
   uint32_t * num_vpm_rows_c,
   uint32_t * num_vpm_rows_v,
   uint32_t *attr_count
   );

static bool glxx_install_tex_param(GLXX_SERVER_STATE_T *state, uint32_t *location,
   uint32_t u0, uint32_t u1, bool secure);

static uint32_t convert_primitive_type(GLenum mode);
static uint32_t convert_index_type(GLenum type);

static bool backend_uniform_address( uint32_t u1,
                  GLXX_SERVER_STATE_T *state,
                  GL20_HW_INDEXED_UNIFORM_T *iu,
                  uint32_t *location);

#ifdef TIMELINE_EVENT_LOGGING

static void EventLog(uint32_t t, uint32_t c, uint32_t d, char *desc)
{
   EventData ev;
   ev.eventType = t;
   ev.eventCode = c;
   ev.eventRow  = khrn_event_cpu_row();
   ev.eventData = d;
   ev.desc = desc;
   ev.eventSecs = 0;
   ev.eventNanosecs = 0;
   khrn_remote_event_log(&ev);
}

#else
#define EventLog(t, c, d, desc)
#endif

/*************************************************************
 Global Functions
 *************************************************************/

void glxx_hw_finish_context(GLXX_SERVER_STATE_T *state, bool wait)
{
   khrn_render_state_flush_all(KHRN_RENDER_STATE_TYPE_GLXX);
   if (wait) {
      khrn_hw_wait();
      if (state != NULL)
         state->frame_number++;  /* Finish is a nominal frame delimiter, so bump the counter */
   }
}

void glxx_hw_invalidate_frame(GLXX_SERVER_STATE_T *state, bool color, bool depth, bool stencil, bool multisample, bool preserveBuf, bool main_buffer)
{
   GLXX_HW_FRAMEBUFFER_T fb;
   GLXX_HW_RENDER_STATE_T *rs;

   rs = glxx_install_framebuffer(state, &fb, main_buffer);
   if (!rs)
      return;

   glxx_hw_invalidate_internal(rs, color, depth, stencil, multisample, preserveBuf);
}

/*!
 * \brief Terminate
 *
 */

void glxx_hw_term(void)
{
   gl11_hw_shader_cache_reset();

   dummy_texture_data = NULL;

   if (dummy_texture_handle != MEM_HANDLE_INVALID)
   {
      mem_unlock(dummy_texture_handle);
      mem_release(dummy_texture_handle);
      dummy_texture_handle = MEM_HANDLE_INVALID;
   }
}

static MEM_HANDLE_T get_texture_11(GLXX_SERVER_STATE_T *state,
   unsigned texunit)
{
   vcos_assert(IS_GL_11(state));
   vcos_assert(texunit < GL11_CONFIG_MAX_TEXTURE_UNITS);

   switch (state->texunits[texunit].target_enabled) {
   case GL_TEXTURE_2D:
      return state->bound_texture[texunit].mh_twod;
   case GL_TEXTURE_EXTERNAL_OES:
      return state->bound_texture[texunit].mh_external;
   case GL_TEXTURE_CUBE_MAP:
   case GL_NONE:
      break;
   default:
      UNREACHABLE();
      break;
   }
   return MEM_HANDLE_INVALID;
}

static MEM_HANDLE_T get_texture_20(GLXX_SERVER_STATE_T *state,
   unsigned index, bool *in_vshader)
{
   const GLXX_DRAW_BATCH_T *batch = &state->batch;
   const GL20_SAMPLER_INFO_T *sinfo = batch->sampler_info;
   const GL20_UNIFORM_INFO_T *uinfo = batch->uniform_info;
   const GL20_UNIFORM_INFO_T *ui = uinfo + sinfo[index].uniform;
   unsigned texunit = batch->uniform_data[ui->offset + sinfo[index].index];

   if (texunit >= GLXX_CONFIG_MAX_TEXTURE_UNITS)
      return MEM_HANDLE_INVALID;

   *in_vshader = sinfo[index].in_vshader;

   switch (ui->type) {
   case GL_SAMPLER_2D:
      return state->bound_texture[texunit].mh_twod;
   case GL_SAMPLER_EXTERNAL_OES:
      return state->bound_texture[texunit].mh_external;
   case GL_SAMPLER_CUBE:
      return state->bound_texture[texunit].mh_cube;
   default:
      UNREACHABLE();
      break;
   }
   return MEM_HANDLE_INVALID;
}

static void set_current_render_state(GLXX_SERVER_STATE_T *state, uint32_t rs_name) {
   state->changed_cfg = true;
   state->changed_linewidth = true;
   state->changed_polygon_offset = true;
   state->changed_viewport = true;
   state->old_flat_shading_flags = ~0;

   state->current_render_state = rs_name;
}

GLXX_HW_RENDER_STATE_T *glxx_install_framebuffer(GLXX_SERVER_STATE_T *state, GLXX_HW_FRAMEBUFFER_T *fb, bool main_buffer)
{
   MEM_HANDLE_T hcolor, hdepth, hstencil;
   KHRN_IMAGE_T *color;
   uint32_t i;
   bool multisample;
   GLXX_HW_RENDER_STATE_T *rs;
   BEGL_WindowHandle win = NULL;

   if (state->mh_bound_framebuffer != MEM_INVALID_HANDLE && !main_buffer)
   {
      GLXX_FRAMEBUFFER_T *framebuffer = (GLXX_FRAMEBUFFER_T *)mem_lock(state->mh_bound_framebuffer, NULL);

      hcolor = glxx_attachment_info_get_images(&framebuffer->attachments.color, &fb->mh_ms_color);
      hdepth = glxx_attachment_info_get_images(&framebuffer->attachments.depth, NULL);
      hstencil = glxx_attachment_info_get_images(&framebuffer->attachments.stencil, NULL);

      fb->mh_depth = hdepth != MEM_INVALID_HANDLE ? hdepth : hstencil;  /* TODO naughty */
      fb->have_depth = framebuffer->attachments.depth.type != GL_NONE;
      fb->have_stencil = framebuffer->attachments.stencil.type != GL_NONE;

      multisample = (framebuffer->attachments.color.samples != 0);
   }
   else
   {
      hcolor = state->mh_draw;
      hdepth = state->mh_depth;
      multisample = state->mh_color_multi != MEM_INVALID_HANDLE;    /* TODO naughty */

      if (multisample)
      {
         fb->mh_ms_color = state->mh_color_multi;  /* TODO naughty */
         fb->mh_depth = state->mh_ds_multi;        /* TODO naughty */
      }
      else
      {
         fb->mh_ms_color = MEM_INVALID_HANDLE;  /* TODO naughty */
         fb->mh_depth = state->mh_depth;        /* TODO naughty */
      }

      fb->have_depth = state->mh_depth != MEM_INVALID_HANDLE && state->config_depth_bits > 0;
      fb->have_stencil = state->mh_depth && state->config_stencil_bits > 0;
   }

   if(hcolor==MEM_INVALID_HANDLE)
      return NULL;

   color = (KHRN_IMAGE_T *)mem_lock(hcolor, NULL);

   fb->mh_preserve_image = state->mh_preserve;

   fb->mh_color_image = hcolor; /* TODO naughty */

   /* wins has to be something sensible for platforms which support rendering into client provided surfaces
      EG. ANDROID/DFB
      win is only valid for BEGL_BufferUsage_eSwapChain0, BEGL_BufferUsage_eSwapChain1 & BEGL_BufferUsage_eSwapChain2
      so this shouldn't reprogram for pixmaps and FBO's */

   if (color->window_state)
      win = color->window_state->window;

   if (win != NULL)
   {
      /* abstract platform always has an opaque_buffer_handle */
      vcos_assert(color->opaque_buffer_handle != NULL);

      if (1) /*state->swapchainc > 0)*/
      {
         BEGL_BufferSettings bufferSettings;

         /* have we already locked the image?, If yes then the size cant have changed, just carry on */
         if (!color->alreadyLocked)
         {
            bool locked;
            KHRN_IMAGE_FORMAT_T format = ABGR_8888;
            uint32_t padded_width, padded_height, align;

            EventData *lastEv = khrn_remote_get_last_event();
            uint32_t funcData = 0;
            if (lastEv->eventType != eEVENT_END && lastEv->eventType != eEVENT_INVALID_TYPE)
            {
               // An event is already in progress. Terminate it and start again after the delay.
               funcData = lastEv->eventData;
               EventLog(eEVENT_END, eEVENT_API, funcData, 0);
            }

            EventLog(eEVENT_WAITING, eEVENT_API, funcData, "Waiting for render buffer");

            locked = platform_lock_buffer(color->opaque_buffer_handle, color->window_state, &color->fence);
            vcos_assert(locked);

            EventLog(eEVENT_START, eEVENT_API, funcData, 0);
            if (funcData == 0)
            {
               // We started this event, so end it now
               EventLog(eEVENT_END, eEVENT_API, funcData, 0);
            }

            /* Set our alreadyLocked flag so that subsequent installFramebuffer calls will not try to lock the
               buffer again. The flag is cleared during swapBuffers, when all draw calls are finished. After that
               anyone trying to lock the buffer again will block until it's been displayed and recycled.
            */
            if (locked)
            {
               vcos_atomic_increment(&color->alreadyLocked);

               /* after the lock the bufferSettings are updated with the correct address */
               platform_get_buffersettings(color->opaque_buffer_handle, &bufferSettings);

               padded_width = bufferSettings.width;
               padded_height = bufferSettings.height;
               align = 0;

               if ((bufferSettings.format == BEGL_BufferFormat_eA8B8G8R8) ||
                   (bufferSettings.format == BEGL_BufferFormat_eA8B8G8R8_TFormat))
                  format = ABGR_8888;
               else if ((bufferSettings.format == BEGL_BufferFormat_eX8B8G8R8) ||
                        (bufferSettings.format == BEGL_BufferFormat_eX8B8G8R8_TFormat))
                  format = XBGR_8888;
               else if (bufferSettings.format == BEGL_BufferFormat_eR8G8B8X8)
                  format = RGBX_8888;
               else if (bufferSettings.format == BEGL_BufferFormat_eR8G8B8A8)
                  format = RGBA_8888;
               else if ((bufferSettings.format == BEGL_BufferFormat_eR5G6B5) ||
                        (bufferSettings.format == BEGL_BufferFormat_eR5G6B5_TFormat))
                  format = RGB_565;

               KHRN_IMAGE_CREATE_FLAG_T image_create_flags =
                  ((color->flags & IMAGE_FLAG_TEXTURE) ? IMAGE_CREATE_FLAG_TEXTURE : 0) |
                  ((color->flags & IMAGE_FLAG_RSO_TEXTURE) ? IMAGE_CREATE_FLAG_RSO_TEXTURE : 0) |
                  ((color->flags & IMAGE_FLAG_RENDER_TARGET) ? IMAGE_CREATE_FLAG_RENDER_TARGET : 0) |
                  ((color->flags & IMAGE_FLAG_DISPLAY) ? IMAGE_CREATE_FLAG_DISPLAY : 0);

               khrn_image_platform_fudge(&format, &padded_width, &padded_height, &align, image_create_flags);

               color->width = bufferSettings.width;
               color->height = bufferSettings.height;
               color->format = format;
               color->stride = khrn_image_get_stride(format, padded_width);
               color->secure = bufferSettings.secure;

               /* the underlying OS may have changed the color format to one not supported by v3d.  All bets are off */
               vcos_assert(platform_supported_format(color->format) != false);

               /* Re-wrap the back buffer handle - this is where we render NEXT */
               MEM_ASSIGN(color->mh_storage, MEM_HANDLE_INVALID);

               color->mh_storage = mem_wrap(bufferSettings.secure ? NULL : bufferSettings.cachedAddr,
                                            bufferSettings.physOffset,
                                            bufferSettings.pitchBytes * bufferSettings.height,
                                            bufferSettings.alignment, MEM_FLAG_DIRECT | (bufferSettings.secure ? MEM_FLAG_SECURE : 0),
                                            "wrapped pixmap");

               /* resize the depth and stencil if its changed */
               if (state->mh_depth) {
                  int scale = (multisample) ? 2 : 1;
                  KHRN_IMAGE_T *depth = (KHRN_IMAGE_T *)mem_lock(state->mh_depth, NULL);
                  if ((depth->width != (bufferSettings.width*scale)) ||
                      (depth->height != (bufferSettings.height*scale)))
                  {
                     khrn_interlock_write_immediate(&depth->interlock);
                     khrn_image_resize(depth, bufferSettings.width*scale, bufferSettings.height*scale);
                  }
                  mem_unlock(state->mh_depth);
               }

               if (multisample) {
                  KHRN_IMAGE_T *color_multi = (KHRN_IMAGE_T *)mem_lock(state->mh_color_multi, NULL);
                  if (color_multi->width != (bufferSettings.width*2) || color_multi->height != (bufferSettings.height*2))
                  {
                     khrn_interlock_write_immediate(&color_multi->interlock);
                     khrn_image_resize(color_multi, bufferSettings.width*2, bufferSettings.height*2);
                  }
                  mem_unlock(state->mh_color_multi);

                  if (state->mh_ds_multi != MEM_INVALID_HANDLE)
                  {
                     KHRN_IMAGE_T *ds_multi = (KHRN_IMAGE_T *)mem_lock(state->mh_ds_multi, NULL);
                     if (ds_multi->width != (bufferSettings.width*2) || ds_multi->height != (bufferSettings.height*2))
                     {
                        khrn_interlock_write_immediate(&ds_multi->interlock);
                        khrn_image_resize(ds_multi, bufferSettings.width*2, bufferSettings.height*2);
                     }
                     mem_unlock(state->mh_ds_multi);
                  }
               }
            }
         }
      }
   }

   fb->width = color->width;
   fb->height = color->height;
   fb->col_format = color->format;
   fb->flags = color->flags;
   fb->dither = state->caps.dither;
#ifdef GLXX_FORCE_MULTISAMPLE
   fb->ms = true;
#else
   fb->ms = multisample;
   fb->stereo_mode = (state->hints.stereo_rendering != 0);
#endif
   fb->pad_width = color->stride * 8 / khrn_image_get_bpp(color->format);
   fb->pad_height = fb->height;

   i = khrn_interlock_render_state_i(khrn_interlock_get_writer(&color->interlock));
   /* TODO: usurp KHRN_RENDER_STATE_TYPE_COPY_BUFFER? */
   if (i != (uint32_t)~0 && khrn_render_state_get_type(i) == KHRN_RENDER_STATE_TYPE_GLXX)
   {
      rs = (GLXX_HW_RENDER_STATE_T *)khrn_render_state_get_data(i);
      /* If render state has changed then reissue all the start of frame instructions */
      if (rs->name != state->current_render_state) {
         set_current_render_state(state, rs->name);
      }
   }
   else
   {
      i = khrn_render_state_start(KHRN_RENDER_STATE_TYPE_GLXX);
      rs = (GLXX_HW_RENDER_STATE_T *)khrn_render_state_get_data(i);
      rs->name = i;

      /* TODO: is this the right place to reset these?          */
      /*       changed_settings now in set_current_render_state */
      set_current_render_state(state, rs->name);

      if (!glxx_hw_start_frame_internal(rs, fb))
      {
         glxx_hw_discard_frame(rs);
         mem_unlock(hcolor);
         return NULL;    /* TODO: distinguish between out-of-memory and null clip rectangle */
      }
   }

   mem_unlock(hcolor);
   return rs;
}

/*!
 * \brief Converts the texture wrap setting from the GLenum
 *        representation to the internal one used in the simulator.
 *
 * \param wrap is the GL texture wrap setting.
 */
int glxx_convert_wrap(GLenum wrap)
{
   switch (wrap) {
   case GL_REPEAT:
      return 0;
   case GL_CLAMP_TO_EDGE:
      return 1;
   case GL_MIRRORED_REPEAT:
      return 2;
   default:
      UNREACHABLE();
      return 0;
   }
}


/*!
 * \brief Converts the filter from the GLenum
 *        representation to the internal one used in the simulator.
 *
 * \param filter is the GL mipmap filter.
 */
int glxx_convert_filter(GLenum filter)
{
   switch (filter) {
   case GL_NEAREST:
      return 1;
   case GL_LINEAR:
      return 0;
   case GL_NEAREST_MIPMAP_NEAREST:
      return 2;
   case GL_NEAREST_MIPMAP_LINEAR:
      return 3;
   case GL_LINEAR_MIPMAP_NEAREST:
      return 4;
   case GL_LINEAR_MIPMAP_LINEAR:
      return 5;
   default:
     UNREACHABLE();
      return 0;
   }
}

/*!
 * \brief \a Undocumented
 *
 * TODO: I don't know what this function is doing.
 *
 * \param mode OpenGL mode flag.
 */
uint32_t glxx_enable_back(GLenum mode)
{
   vcos_assert(mode == GL_FRONT || mode == GL_BACK || mode == GL_FRONT_AND_BACK);

   return mode == GL_FRONT;
}

 /*!
 * \brief \a Undocumented
 *
 * TODO: I don't know what this function is doing.
 *
 * \param mode OpenGL mode flag.
 */
uint32_t glxx_enable_front(GLenum mode)
{
   vcos_assert(mode == GL_FRONT || mode == GL_BACK || mode == GL_FRONT_AND_BACK);

   return mode == GL_BACK;
}


/*!
 * \brief \a Undocumented
 *
 * TODO: I don't know what this function is doing.
 *
 * \param mode OpenGL mode flag.
 */
uint32_t glxx_front_facing_is_clockwise(GLenum mode)
{
   vcos_assert(mode == GL_CW || mode == GL_CCW);

   return mode == GL_CW;
}

uint32_t glxx_hw_primitive_mode_to_type(GLenum primitive_mode)
{
   switch (primitive_mode)
   {
   case GL_POINTS:
      return 0;
   case GL_LINES:
   case GL_LINE_LOOP:
   case GL_LINE_STRIP:
      return 1;
   case GL_TRIANGLES:
   case GL_TRIANGLE_STRIP:
   case GL_TRIANGLE_FAN:
      return 2;
   default:
      UNREACHABLE();
      return 0;
   }
}

bool do_vcd_setup(
   GLXX_HW_SHADER_RECORD_T *shader_record,
   GLXX_ATTRIB_T *attrib,
   MEM_HANDLE_T *attrib_handles,
   uint32_t cattribs_live,
   uint32_t vattribs_live,
   uint32_t *mergeable_attribs,
   uint32_t *cattribs_order,
   uint32_t *vattribs_order,
   uint32_t *num_vpm_rows_c,
   uint32_t *num_vpm_rows_v,
   uint32_t *attr_count
   )
{
   bool anything = false;
   uint32_t i, j;
   uint32_t n = 0, nv = 0, nc = 0;
   uint32_t last_vattrib = -1;
   uint32_t count_vpm_setup_c = 0;
   uint32_t count_vpm_setup_v = 0;
   uint32_t vpm_offset_v, vpm_offset_c;
   uint32_t total_vpm_offset_v, total_vpm_offset_c;
   uint32_t vattrsel = 0;
   uint32_t cattrsel = 0;

   UNUSED_NDEBUG(cattribs_live);
   UNUSED_NDEBUG(vattribs_live);
   UNUSED(mergeable_attribs);

   vcos_assert(!(cattribs_live & ~vattribs_live));

   vpm_offset_v = 0;             /* vpm_offset counts from the start of one read setup to make sure */
   vpm_offset_c = 0;             /* we never exceed 15 rows at a time.                              */
   total_vpm_offset_v = 0;       /* total_vpm_offset counts from the start of the VPM to ensure     */
   total_vpm_offset_c = 0;       /* attribute data never overlaps once loaded.                      */

   /*for(i = 0;i<4;i++) {
      num_vpm_rows_c[i] = 0;
      num_vpm_rows_v[i] = 0;
   } */
   memset(num_vpm_rows_c, 0, 4 * sizeof(uint32_t));
   memset(num_vpm_rows_v, 0, 4 * sizeof(uint32_t));

   for (j = 0; j < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; j++) {
      uint32_t jstart = j;
      i = vattribs_order[j];

      if (i!=(uint32_t)~0) {
         uint32_t length;
         uint32_t new_num_rows;

         last_vattrib = j;

         vcos_assert(vattribs_live & 15<<(4*i) && attrib[i].enabled);

         length = attrib[i].size * khrn_get_type_size(attrib[i].type);

#ifdef GLXX_WANT_ATTRIBUTE_MERGING
         while(j<GLXX_CONFIG_MAX_VERTEX_ATTRIBS-1 && vattribs_order[j+1]!=(uint32_t)~0 && mergeable_attribs[vattribs_order[j]] == vattribs_order[j+1])
         {
            uint32_t new_length = length + attrib[vattribs_order[j+1]].size * khrn_get_type_size(attrib[vattribs_order[j+1]].type);
            if( ((((new_length + 3) & ~3)+3)/4) > 15) //too big to fit in current vpm_setup
               break;
            //do merge
            j++;
            length = new_length;
            last_vattrib = j;
         }
#endif
         vcos_assert( ((((length + 3) & ~3)+3)/4) <= 15 );//small enough to fit in a vpm_setup

         glxx_big_mem_insert(&shader_record->attr[n].base, attrib_handles[i], (uint32_t)attrib[i].pointer);

         shader_record->attr[n].base = i; /* SW-5891 temporarily store i here so can duplicate shader record if needed */
         shader_record->attr[n].sizem1 = length - 1;
         shader_record->attr[n].stride = attrib[i].stride ? attrib[i].stride : attrib[i].size * khrn_get_type_size(attrib[i].type);

         shader_record->attr[n].voffset = total_vpm_offset_v;
         total_vpm_offset_v += (length + 3) & ~3;
         vpm_offset_v += (length + 3) & ~3;
         vattrsel |= 1<<n;

         new_num_rows = (vpm_offset_v+3)/4;

         //can only load 15 rows at a time
         if(new_num_rows > 15)
         {
            count_vpm_setup_v ++;

            vpm_offset_v = (length + 3) & ~3;
            num_vpm_rows_v[count_vpm_setup_v] = (vpm_offset_v+3)/4;
         }
         else
            num_vpm_rows_v[count_vpm_setup_v] = new_num_rows;

         if (cattribs_order[jstart]!=(uint32_t)~0) /*want all the vattribs from this iteration in cattribs */
         {
            shader_record->attr[n].coffset = total_vpm_offset_c;
            vpm_offset_c += (length + 3) & ~3;
            total_vpm_offset_c += (length + 3) & ~3;

            new_num_rows = (vpm_offset_c+3)/4;

            //can only load 15 rows at a time
            if(new_num_rows > 15)
            {
               count_vpm_setup_c ++;

               vpm_offset_c = (length + 3) & ~3;
               num_vpm_rows_c[count_vpm_setup_c] = (vpm_offset_c+3)/4;
            }
            else
               num_vpm_rows_c[count_vpm_setup_c] = new_num_rows;

            nc++;
            cattrsel |= 1<<n;
         }

         nv++;
         n++;

         anything = true;
      }
   }

   //now handle any cattribs that didn't match the merged vattribs
   for (j = last_vattrib+1; j < GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2; j++) {

      i = cattribs_order[j];

      if (i!=(uint32_t)~0)
      {
         uint32_t length;
         uint32_t new_num_rows;

         vcos_assert(n<GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

         length = attrib[i].size * khrn_get_type_size(attrib[i].type);

#ifdef GLXX_WANT_ATTRIBUTE_MERGING
         while(j<GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2-1 && cattribs_order[j+1]!=(uint32_t)~0  && mergeable_attribs[cattribs_order[j]] == cattribs_order[j+1])
         {
            //do merge
            j++;
            length += attrib[cattribs_order[j]].size * khrn_get_type_size(attrib[cattribs_order[j]].type);
         }
#endif

         glxx_big_mem_insert(&shader_record->attr[n].base, attrib_handles[i], (uint32_t)attrib[i].pointer);

         shader_record->attr[n].base = i; /* SW-5891 temporarily store i here so can duplicate shader record if needed */
         shader_record->attr[n].sizem1 = length - 1;
         shader_record->attr[n].stride = attrib[i].stride ? attrib[i].stride : attrib[i].size * khrn_get_type_size(attrib[i].type);

         shader_record->attr[n].voffset = 0xff;

         shader_record->attr[n].coffset = total_vpm_offset_c;
         vpm_offset_c += (length + 3) & ~3;
         total_vpm_offset_c += (length + 3) & ~3;
         cattrsel |= 1<<n;

         new_num_rows = (vpm_offset_c+3)/4;

         //can only load 15 rows at a time
         if(new_num_rows > 15)
         {
            count_vpm_setup_c ++;

            vpm_offset_c = (length + 3) & ~3;
            num_vpm_rows_c[count_vpm_setup_c] = (vpm_offset_c+3)/4;
         }
         else
            num_vpm_rows_c[count_vpm_setup_c] = new_num_rows;

         nc++;
         n++;

         anything = true;
      }
   }

   if (nc == 0 || nv == 0) {
      void *block;
      MEM_LOCK_T lbh;

      if (n == 8)
         return false;     /* No space. Surely there's a better way */

      block = glxx_big_mem_alloc_junk(4, 4, &lbh);
      shader_record->attr[n].base = khrn_hw_addr(block, &lbh);
      if (!shader_record->attr[n].base) return false;
      shader_record->attr[n].sizem1 = 0;
      shader_record->attr[n].stride = 0;

      if (nc == 0) {
         vcos_assert(vpm_offset_c == 0);
         shader_record->attr[n].coffset = 0;
         cattrsel |= 1<<n;
         num_vpm_rows_c[0] = 1;
      } else shader_record->attr[n].coffset = 0xff;

      if (nv == 0) {
         vcos_assert(vpm_offset_v == 0);
         shader_record->attr[n].voffset = 0;
         vattrsel |= 1<<n;
         num_vpm_rows_v[0] = 1;
      } else shader_record->attr[n].voffset = 0xff;

      n++;
   }

   shader_record->vattrsel = vattrsel;
   shader_record->vattrsize = total_vpm_offset_v;
   shader_record->cattrsel = cattrsel;
   shader_record->cattrsize = total_vpm_offset_c;

   *attr_count = n;

   return true;
}

/* TODO: get rid of fb and attrib parameters */
static void calculate_and_hide(GLXX_SERVER_STATE_T *state, GLXX_HW_FRAMEBUFFER_T *fb, GLXX_ATTRIB_T *attrib)
{
   float *modelview;
   int i;

#if GL_EXT_texture_format_BGRA8888
   /* TODO: we enumerate over textures elsewhere. Feels slightly wasteful. */
   for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      MEM_HANDLE_T thandle = MEM_INVALID_HANDLE;

      state->shader.common.texture_rb_swap[i] = false;
      if (IS_GL_11(state))
      {
          switch (state->texunits[i].target_enabled) {
          case GL_TEXTURE_2D:
              thandle = state->bound_texture[i].mh_twod;
              break;
          case GL_TEXTURE_EXTERNAL_OES:
              thandle = state->bound_texture[i].mh_external;
              break;
          }
      }
      else
      {
         if (i < state->batch.num_samplers)
         {
            GL20_UNIFORM_INFO_T *ui;
            int index;

            vcos_assert(state->batch.sampler_info != NULL);

            ui = &state->batch.uniform_info[state->batch.sampler_info[i].uniform];
            index = state->batch.uniform_data[ui->offset + state->batch.sampler_info[i].index];

            /* Index may not be valid, in which case a dummy texture will be set up later */
            if (index >= 0 && index < GLXX_CONFIG_MAX_TEXTURE_UNITS) {
                switch (ui->type) {
                case GL_SAMPLER_2D:
                    thandle = state->bound_texture[index].mh_twod;
                    break;
                case GL_SAMPLER_EXTERNAL_OES:
                    thandle = state->bound_texture[index].mh_external;
                    break;
                case GL_SAMPLER_CUBE:
                    thandle = state->bound_texture[index].mh_cube;
                    break;
                default:
                    UNREACHABLE();
                    break;
                }
            }
         }
      }
      if (thandle != MEM_INVALID_HANDLE)
      {
         GLXX_TEXTURE_T *texture;

         texture = (GLXX_TEXTURE_T *)mem_lock(thandle, NULL);
         if (glxx_texture_check_complete(texture) == COMPLETE)
            state->shader.common.texture_rb_swap[i] = tu_image_format_rb_swap(glxx_texture_get_tformat(texture));

         mem_unlock(thandle);
      }
   }
#endif

   state->shader.common.fb_rb_swap = tu_image_format_rb_swap(khrn_image_to_tf_format(fb->col_format));  /* TODO: slight hack - using tu function to determine fb thing */

   /* TODO: this copying shouldn't be necessary */
   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if (attrib[i].enabled)
      {
         state->shader.common.attribs[i].type = attrib[i].type;
         state->shader.common.attribs[i].size = attrib[i].size;
         state->shader.common.attribs[i].norm = attrib[i].normalized;
      }
      else
      {
         state->shader.common.attribs[i].type = 0;
         state->shader.common.attribs[i].size = 0;
         state->shader.common.attribs[i].norm = 0;
      }
   }

   state->shader.common.primitive_type = glxx_hw_primitive_mode_to_type(state->batch.primitive_mode);
   state->shader.common.render_alpha = khrn_image_get_alpha_size(fb->col_format)>0;
   state->shader.common.rgb565 = khrn_image_get_green_size(fb->col_format) == 6;

   /* structure compare */
   /* for the programming to succeed, if the fwd/rev masks are any of the magic values, they must be the same
      or if they are different they must not be magic */
   state->shader.common.stencil_config = fb->have_stencil && state->caps.stencil_test;

   state->shader.common.use_depth = fb->have_depth;
   state->shader.common.blend.ms = fb->ms;

   if (khrn_workarounds.FB_BOTTOM_UP)
   {
      /* work off if the output image is an EGL image.  This can be a wrapped image, or an eglSurface or a Pixmap */
      /* used to only trigger of RSO, but make it work from any egl image irrespective of format */
      state->shader.common.egl_output = (fb->flags & IMAGE_FLAG_DISPLAY) ? true : false;
   }
   else if (khrn_workarounds.FB_TOP_DOWN)
   {
      /* on a B1 we can't rasterize TFormat upside down in HW, so we need to import the original flip from B0 */
      state->shader.common.egl_output = ((fb->flags & IMAGE_FLAG_DISPLAY) &&
         (khrn_image_is_tformat(fb->col_format) || khrn_image_is_lineartile(fb->col_format))) ? true : false;
   }
   else
      state->shader.common.egl_output = false;

   if (state->changed_backend)
   {
      if (state->caps.blend)
      {
         state->shader.common.blend.equation           = state->blend_equation.rgb;
         state->shader.common.blend.equation_alpha     = state->blend_equation.alpha;
         state->shader.common.blend.src_function       = state->blend_func.src_rgb;
         state->shader.common.blend.src_function_alpha = state->blend_func.src_alpha;
         state->shader.common.blend.dst_function       = state->blend_func.dst_rgb;
         state->shader.common.blend.dst_function_alpha = state->blend_func.dst_alpha;
      }
      else
      {
         state->shader.common.blend.equation = GL_FUNC_ADD;
         state->shader.common.blend.equation_alpha = GL_FUNC_ADD;
         state->shader.common.blend.src_function = GL_ONE;
         state->shader.common.blend.src_function_alpha = GL_ONE;
         state->shader.common.blend.dst_function = GL_ZERO;
         state->shader.common.blend.dst_function_alpha = GL_ZERO;
      }
      if (state->caps.color_logic_op)
         state->shader.common.blend.logic_op = state->logic_op;
      else
         state->shader.common.blend.logic_op = GL_COPY;

      /* TODO: sample_alpha_to_one? */
      state->shader.common.blend.sample_alpha_to_coverage = state->caps.sample_alpha_to_coverage;
      state->shader.common.blend.sample_coverage    = state->caps.sample_coverage;
      state->shader.common.blend.sample_coverage_v  = state->sample_coverage;

      state->changed_backend = false;
   }

   if (IS_GL_11(state))
   {
      /* TODO: This was already done in DrawElements_impl. Better just to copy? */
      gl11_hw_setup_attribs_live(state, &state->shader.cattribs_live, &state->shader.vattribs_live);

      /* Has to be copied every time because no changed flags are set when mode changes */
      state->shader.point_smooth = state->caps_fragment.point_smooth && state->batch.primitive_mode == 0;
      state->shader.line_smooth = state->caps_fragment.line_smooth &&
         (state->batch.primitive_mode >= 1 && state->batch.primitive_mode <= 3);

      if (state->changed_misc)
      {
         if (state->caps_fragment.alpha_test)
            state->shader.alpha_func = state->alpha_func.func;
         else
            state->shader.alpha_func = GL_ALWAYS;

         if (state->caps_fragment.fog)
            state->shader.fog_mode = state->fog.mode;
         else
            state->shader.fog_mode = 0;

         /*
            Choose which user clip plane test to use so that complementary planes don't produce gaps or overlap
         */
         if (!state->caps.clip_plane[0])
            state->shader.user_clip_plane = 0;
         else if (state->planes[0][0] > 0 ||
            (state->planes[0][0] == 0 && (state->planes[0][1] > 0 ||
            (state->planes[0][1] == 0 && (state->planes[0][2] > 0 ||
            (state->planes[0][2] == 0 && (state->planes[0][3] >= 0)))))))
            state->shader.user_clip_plane = 1;
         else
            state->shader.user_clip_plane = -1;

         state->changed_misc = false;
      }

      if (state->changed_light)
      {
         for (i = 0; i < GL11_CONFIG_MAX_LIGHTS; i++)
         {
            if (state->shader.lighting && state->lights[i].enabled)
            {
               state->shader.lights[i].enabled = true;
               state->shader.lights[i].position_w_is_0 = state->lights[i].position[3] == 0.0f;
               state->shader.lights[i].spot_cutoff_is_180 = state->lights[i].spot.cutoff == 180.0f;
            }
            else
            {
               state->shader.lights[i].enabled = false;
               state->shader.lights[i].position_w_is_0 = false;
               state->shader.lights[i].spot_cutoff_is_180 = false;
            }
         }
         if (state->shader.lighting)
            state->shader.two_side = state->lightmodel.two_side;
         else state->shader.two_side = false;

         state->changed_light = false;
      }

      //TODO: better handling of matrix stacks
      //optimisation: moved here from install_uniforms to avoid repeated calculation
      modelview = state->modelview.body[state->modelview.pos];

      if (state->caps_fragment.fog || state->shader.lighting || state->batch.primitive_mode == 0)
         khrn_memcpy(state->current_modelview, modelview, 16*4);

      for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
         GL11_TEXUNIT_T *texunit = &state->texunits[i];
         if (state->shader.texunits[i].props.active) {
            khrn_memcpy(texunit->current_matrix,texunit->stack.body[texunit->stack.pos],16*4);
         }
      }
      gl11_matrix_mult(state->projection_modelview, state->projection.body[state->projection.pos], modelview);
      gl11_matrix_invert_3x3(state->modelview_inv, modelview);

      if (state->caps.clip_plane[0])
      {
         GLfloat inv[16];
         GLfloat blah[16];

         gl11_matrix_invert_4x4(inv, state->projection.body[state->projection.pos]);
         gl11_matrix_mult(blah, inv, state->projection.body[state->projection.pos]);
         gl11_matrix_mult_row(state->projected_clip_plane, state->planes[0], inv);
         state->projected_clip_plane[0] /= state->viewport.internal[0];    /* xscale */
         state->projected_clip_plane[1] /= state->viewport.internal[1];    /* yscale */
         state->projected_clip_plane[2] /= (16777215.0f * state->viewport.internal[4]);    /* zscale */
      }
   }
   else
   {
      vcos_assert(!state->changed_misc);
      vcos_assert(!state->changed_light);
      /*
         Can set changed_texunit accidentally because of has_color/has_alpha stuff
         (more specifically, we don't check for IS_ES_11 in glTexImage2D)
      */
   }
}

bool glxx_hw_get_attr_live(GLXX_SERVER_STATE_T *state, GLXX_ATTRIB_T *attrib)
{
   /* TODO: This function ignores it's parameter 'mode' */
   if (IS_GL_11(state))
   {
      if (state->changed_texunit)
      {
         if (!gl11_hw_get_texunit_properties(state, &attrib[GL11_IX_TEXTURE_COORD], 0  /*TODO: points*/))
            return false;        // out of memory during blob construction
         state->changed_texunit = false;
      }
      gl11_hw_setup_attribs_live(state,&state->batch.cattribs_live,&state->batch.vattribs_live);
   }
   else
   {
      if (state->mh_program != MEM_INVALID_HANDLE)
      {
         GL20_PROGRAM_T *program = (GL20_PROGRAM_T *)mem_lock(state->mh_program, NULL);
         state->batch.cattribs_live = program->result.cattribs_live;
         state->batch.vattribs_live = program->result.vattribs_live;
         mem_unlock(state->mh_program);
      }
   }
   return true;
}

/* insert clip instruction. is_empty is set to true if the region is empty - i.e. nothing to draw.
   returns false if failed a memory alloc
*/
static bool do_changed_cfg(GLXX_SERVER_STATE_T *state, GLXX_HW_FRAMEBUFFER_T *fb, uint32_t color_varyings, bool * is_empty)
{
   uint32_t flat_shading_flags;
   uint8_t * instr;
   *is_empty = false;

   if (state->changed_viewport)
   {
      int x, y, xmax, ymax;
      x = 0;
      y = 0;
      xmax = fb->width;
      ymax = fb->height;

      x = _max(x, state->viewport.x);
      y = _max(y, state->viewport.y);
      xmax = _min(xmax, state->viewport.x + state->viewport.width);
      ymax = _min(ymax, state->viewport.y + state->viewport.height);

      if (state->caps.scissor_test)
      {
         x = _max(x, state->scissor.x);
         y = _max(y, state->scissor.y);
         xmax = _min(xmax, state->scissor.x + state->scissor.width);
         ymax = _min(ymax, state->scissor.y + state->scissor.height);
      }

      if (x >= xmax || y >= ymax)
      {
         *is_empty = true;
         goto done;   /* empty region - nothing to draw */
      }

      instr = glxx_big_mem_alloc_cle(32);
      if (!instr) goto fail;

      add_byte(&instr, KHRN_HW_INSTR_STATE_CLIP);     //(9)
      add_short(&instr, x);

      if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) &&
         state->shader.common.egl_output)
         add_short(&instr, fb->height - ymax);
      else
         add_short(&instr, y);

      add_short(&instr, xmax - x);
      add_short(&instr, ymax - y);

      add_byte(&instr, KHRN_HW_INSTR_STATE_CLIPPER_XY);   //(9)
      add_float(&instr, 8.0f * (float)state->viewport.width);
      if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) &&
         state->shader.common.egl_output)
         add_float(&instr, -8.0f * (float)state->viewport.height);
      else
         add_float(&instr, 8.0f * (float)state->viewport.height);

      add_byte(&instr, KHRN_HW_INSTR_STATE_VIEWPORT_OFFSET);  //(5)
      add_short(&instr, 8 * (2*state->viewport.x + state->viewport.width));
      if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) &&
         state->shader.common.egl_output)
         add_short(&instr, 8 * (2*fb->height - 2*state->viewport.y - state->viewport.height));
      else
         add_short(&instr, 8 * (2*state->viewport.y + state->viewport.height));

      add_byte(&instr, KHRN_HW_INSTR_STATE_CLIPPER_Z);    //(9)
      add_float(&instr, 0.5f * (state->viewport.far - state->viewport.near));
      add_float(&instr, 0.5f * (state->viewport.far + state->viewport.near));

      state->changed_viewport = false;
   }

   if (glxx_tweaker_update(&state->tweak_state) || state->changed_cfg)
   {
      uint32_t enfwd, enrev, cwise, endo, rasosm, zfunc, enzu, enez;

      enfwd = !state->caps.cull_face || glxx_enable_front(state->cull_mode);
      enrev = !state->caps.cull_face || glxx_enable_back(state->cull_mode);
      cwise = glxx_front_facing_is_clockwise(state->front_face);
      if (khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN)
      {
         if (state->shader.common.egl_output)
            cwise = (!cwise) & 0x1;
      }
      endo = state->caps.polygon_offset_fill;
      rasosm = state->caps.multisample && fb->ms;
      zfunc = (fb->have_depth && state->caps.depth_test) ? glxx_hw_convert_test_function(state->depth_func) : 7;

      if (glxx_tweaker_update(&state->tweak_state) && zfunc == glxx_hw_convert_test_function(GL_EQUAL))
         zfunc = glxx_hw_convert_test_function(GL_LEQUAL);

      enzu = fb->have_depth && state->depth_mask && state->caps.depth_test;

      /* Only safe to do early z if zfunc is less or lequal AND
         There are no side effects due to stencil funcs i.e. zfails */
      enez = (zfunc == 1 || zfunc == 3) &&
                !(state->caps.stencil_test && state->stencil_op.front.zfail != GL_KEEP) &&
                !(state->caps.stencil_test && state->stencil_op.back.zfail  != GL_KEEP);

      if (khrn_workarounds.HW2905)
      {
         GLXX_HW_RENDER_STATE_T *rs = (GLXX_HW_RENDER_STATE_T *)khrn_render_state_get_data(state->current_render_state);
         if (fb->ms && (rs->depth_load || rs->stencil_load))
            enez = 0;
      }

      instr = glxx_big_mem_alloc_cle(4);
      if (!instr) goto fail;

      add_byte(&instr, KHRN_HW_INSTR_STATE_CFG);     //(4)
      add_byte(&instr, enfwd | enrev << 1 | cwise << 2 | endo << 3 | rasosm << 6);
      add_byte(&instr, zfunc << 4 | enzu << 7);
      add_byte(&instr, enez | 1<<1);

      state->changed_cfg = false;
   }

   if (state->changed_polygon_offset)
   {
      instr = glxx_big_mem_alloc_cle(5);
      if (!instr) goto fail;

      add_byte(&instr, KHRN_HW_INSTR_STATE_DEPTH_OFFSET);    //(5)
      add_short(&instr, float_to_bits(state->polygon_offset.factor) >> 16);
      add_short(&instr, float_to_bits(state->polygon_offset.units) >> 16);
      state->changed_polygon_offset = false;
   }

   if (state->changed_linewidth)
   {
      instr = glxx_big_mem_alloc_cle(5);
      if (!instr) goto fail;

      add_byte(&instr,KHRN_HW_INSTR_STATE_LINE_WIDTH);   //(5)
      add_float(&instr, state->line_width);
      state->changed_linewidth = false;
   }

   flat_shading_flags = (IS_GL_11(state) && (state->shade_model == GL_FLAT)) ? color_varyings : 0;
   if (state->old_flat_shading_flags == (uint32_t)~0 || state->old_flat_shading_flags != flat_shading_flags)
   {
      instr = glxx_big_mem_alloc_cle(5);
      if (!instr) goto fail;

      add_byte(&instr, KHRN_HW_INSTR_STATE_FLATSHADE);       //(5)
      add_word(&instr, flat_shading_flags);
      state->old_flat_shading_flags = flat_shading_flags;
   }

done:
   return true;
fail:
   return false;
}

static void reset_egl_image(MEM_HANDLE_T thandle)
{
   if (thandle != MEM_HANDLE_INVALID)
   {
      GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(thandle, NULL);
      if (texture && texture->external_image != MEM_HANDLE_INVALID)
      {
         EGL_IMAGE_T *egl_image = (EGL_IMAGE_T *)mem_lock(texture->external_image, NULL);
         if (egl_image)
         {
            MEM_ASSIGN(egl_image->mh_tf_image, MEM_HANDLE_INVALID);
            mem_unlock(texture->external_image);
         }
         mem_unlock(thandle);
      }
   }
}

/* This removes any scratch images at the end of the draw call. */
static void reset_egl_images_in_textures(GLXX_SERVER_STATE_T *state)
{
   if (IS_GL_11(state))
   {
      for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
         reset_egl_image(get_texture_11(state, i));
   }
   else
   {
      for (int i = 0; i < state->batch.num_samplers; i++)
      {
         vcos_assert(state->batch.sampler_info != NULL);

         if (i < GLXX_CONFIG_MAX_TEXTURE_UNITS)
         {
            bool in_vshader = false;
            reset_egl_image(get_texture_20(state, i, &in_vshader));
         }
      }
   }
}

bool glxx_is_stencil_updated(GLXX_SERVER_STATE_T *state) {
   bool update_front = state->stencil_mask.front != 0 && ( state->stencil_op.front.fail  != GL_KEEP ||
                                                           state->stencil_op.front.zfail != GL_KEEP ||
                                                           state->stencil_op.front.zpass != GL_KEEP   );
   bool update_back = state->stencil_mask.back != 0 && ( state->stencil_op.back.fail   != GL_KEEP ||
                                                         state->stencil_op.back.zfail  != GL_KEEP ||
                                                         state->stencil_op.back.zpass  != GL_KEEP   );
   return update_front || update_back;
}

///////////
extern uint32_t xxx_shader;

/*!
 * \brief Processes a batch of primitives and bins them.
 *
 * This function is called for each batch of primitives in the scene. It first performs
 * per-batch tasks like setting up the shaders and uniforms and then runs the binning
 * pipeline, binning the primitives into the according tile lists.
 *
 * \param mode    is the GLenum mode value provided by OpenGL
 * \param count   is the number of
 * \param type    is a GL type indicator.
 * \param indices_offset is ??? (TODO: Document)
 * \param state   is the OpenGL server state.
 * \param attrib  is ??? (TODO: Document)
 * \param indices_handle  is ??? (TODO: Document)
 * \param attrib_handles  is ??? (TODO: Document)
 * \param max_index is ??? (TODO: Document)
 */
bool glxx_hw_draw_triangles(
   GLsizei count,
   GLenum type,
   uint32_t indices_offset,
   GLXX_SERVER_STATE_T *state,
   GLXX_ATTRIB_T *attrib,
   MEM_HANDLE_T indices_handle,
   MEM_HANDLE_T *attrib_handles,
   uint32_t max_index,
   MEM_HANDLE_OFFSET_T *interlocks,
   uint32_t interlock_count,
   bool secure)
{
   uint32_t i, j;
   GLXX_HW_FRAMEBUFFER_T fb;
   MEM_HANDLE_T cunif_map, vunif_map, funif_map;
   uint32_t cattribs_live;
   uint32_t vattribs_live;

   uint32_t cunif_count;
   uint32_t vunif_count;
   uint32_t funif_count;
   uint32_t mergeable_attribs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   uint32_t vattribs_order[GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2];
   uint32_t cattribs_order[GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2];
   uint32_t num_vpm_rows_c[4];
   uint32_t num_vpm_rows_v[4];
   GLXX_HW_RENDER_STATE_T *rs;
   GLXX_HW_SHADER_RECORD_T *shader_record;
   MEM_LOCK_T shader_record_lbh;
   uint32_t attr_count;
   uint8_t *instr;

   //GL 1.1 specific
   uint32_t color_varyings;

   //gl 2.0 specific
   GL20_PROGRAM_T *program = NULL;
   GL20_HW_INDEXED_UNIFORM_T iu;
   bool locked_program_items = false;

   if(IS_GL_11(state)) {
      if (count == 0)
         return true;
   } else {
      program = (GL20_PROGRAM_T *)mem_lock(state->mh_program, NULL);
      if(!program->linked)
         goto done;
   }

   rs = glxx_install_framebuffer(state, &fb, false);
   if (!rs)
      goto done;

   if (khrn_workarounds.HW2116)
   {
      if (rs->batch_count >= HW2116_BATCH_COUNT_LIMIT)
      {
         glxx_hw_render_state_flush(rs);
         rs = glxx_install_framebuffer(state, &fb, false);
         if (!rs)
            goto done;
         vcos_assert(rs->batch_count == 0);
      }
      rs->batch_count++;
   }

   if (khrn_workarounds.LNLOOP)
   {
      /* line loop doesnt complete with two verticies */
      if ((count == 2) && (state->batch.primitive_mode == GL_LINE_LOOP))
         state->batch.primitive_mode = GL_LINES;
   }

   if (!glxx_lock_fixer_stuff(rs))
      goto fail2;

   if (!IS_GL_11(state)) {
      state->batch.sampler_info = (GL20_SAMPLER_INFO_T *)mem_lock(program->mh_sampler_info, NULL);
      state->batch.uniform_info = (GL20_UNIFORM_INFO_T *)mem_lock(program->mh_uniform_info, NULL);
      state->batch.uniform_data = (uint32_t *)mem_lock(program->mh_uniform_data, NULL);
      state->batch.num_samplers = mem_get_size(program->mh_sampler_info) / sizeof(GL20_SAMPLER_INFO_T);
      locked_program_items = true;
   }

   calculate_and_hide(state, &fb, attrib);
   if (glxx_is_stencil_updated(state)) rs->stencil_used = true;

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) mergeable_attribs[i] = (uint32_t)~0;

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      uint32_t      *attrib_handles_i    = &attrib_handles[i];
      uint32_t      *mergeable_attribs_i = &mergeable_attribs[i];
      GLXX_ATTRIB_T *attrib_i            = &attrib[i];

      if (*attrib_handles_i != MEM_INVALID_HANDLE)
      {
         //look for potentially mergable attributes (packed in same buffer)
         for (j = 0; j < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; j++)
         {
            if (*mergeable_attribs_i == (uint32_t)~0 //not already found a potential merge
               && *attrib_handles_i == attrib_handles[j])
            {
               int size = attrib_i->size*khrn_get_type_size(attrib_i->type);
               GLXX_ATTRIB_T *attrib_j            = &attrib[j];

               if ((uint32_t)attrib_i->pointer + size == (uint32_t)attrib_j->pointer
                  && attrib_i->stride == attrib_j->stride
                  && (size & 3) == 0)
               {
                  //can merge such that j follows i
                  *mergeable_attribs_i = j;
               }
            }
         }
      }
   }

   if (DRAW_TEX_LOGGING)
   {
      vcos_log(LOG_INFO, "state_name: %d rs: %d draw_triangles: count %d type %d",
         state->name, rs->name, count, type);
      vcos_log(LOG_INFO, "--------------------");
   }

   /* TODO: only allocate space for as many vertex attributes as we need */
   /* TODO: extended vertex stride? */
   shader_record = (GLXX_HW_SHADER_RECORD_T *)glxx_big_mem_alloc_junk(100, 16, &shader_record_lbh);
   if (!shader_record) goto fail;

   /* create or retrieve shaders from cache and setup attribs_live */

   cattribs_live = state->batch.cattribs_live;
   vattribs_live = state->batch.vattribs_live;
   if (!vcos_verify(get_shaders(
         program,
         shader_record,
         &cunif_map, &vunif_map, &funif_map,
         &color_varyings,
         state,
         attrib,
         mergeable_attribs,
         cattribs_order,
         vattribs_order)))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto fail;
   }

   /* Create VCD configuration */

   do_vcd_setup(shader_record, attrib, attrib_handles, cattribs_live, vattribs_live, mergeable_attribs, cattribs_order, vattribs_order,
      num_vpm_rows_c, num_vpm_rows_v, &attr_count);
   vcos_assert(attr_count >= 1 && attr_count <= 8);

   if (!IS_GL_11(state))
      gl20_hw_iu_init(&iu);

   /* Install uniforms */
   cunif_count = mem_get_size(cunif_map)/8;
   vunif_count = mem_get_size(vunif_map)/8;
   funif_count = mem_get_size(funif_map)/8;

   if (!install_uniforms(
      &shader_record->cunif,
      state,
      program,
      cunif_count,
      cunif_map,
      &iu,
      num_vpm_rows_c,
      attrib,/*TODO for GL 2.0 NULL is passed instead of attrib - does this matter? */
      state->shader.common.egl_output,
      fb.height,
      secure))
   {
      goto fail;
   }

   if (!install_uniforms(
      &shader_record->vunif,
      state,
      program,
      vunif_count,
      vunif_map,
      &iu,
      num_vpm_rows_v,
      attrib,
      state->shader.common.egl_output,
      fb.height,
      secure))
   {
      goto fail;
   }

   if (!install_uniforms(
      &shader_record->funif,
      state,
      program,
      funif_count,
      funif_map,
      &iu,
      0,
      attrib,
      state->shader.common.egl_output,
      fb.height,
      secure))
   {
      goto fail;
   }

   /* after all uniforms are installed, loop over samplers and remove any egl image scratch
      which where created during install_uniforms */
   reset_egl_images_in_textures(state);

   if (!IS_GL_11(state)) {
      mem_unlock(program->mh_sampler_info);
      mem_unlock(program->mh_uniform_info);
      mem_unlock(program->mh_uniform_data);
      locked_program_items = false;
   }

   if (!IS_GL_11(state)) {
      gl20_hw_iu_close(&iu);
   }

   /* emit any necessary config change instructions */
   bool is_empty;
   bool ok = do_changed_cfg(state, &fb, color_varyings, &is_empty);
   if (!ok) goto fail;
   if (is_empty)
   {
      glxx_unlock_fixer_stuff();
      goto done;   /* empty region - nothing to draw */
   }

   if (type == 0)
   {
      uint32_t offset = 0;
      while(count>0)
      {
         uint32_t step;
         uint32_t batch_count;
         uint32_t batch_indices_offset = indices_offset;

         /* SW-5891 hardware can only do 65536 vertices at a time */
         if(indices_offset>0 && indices_offset+count>USHRT_MAX)
         {
            /* current shader record is no use, (indices_offset+batch_count)*stride > 16bit, so batch_count = 0 */
            /* set up correct attrib pointer in next shade record */
            batch_indices_offset = batch_count = step = 0;
         }
         else if(count>(GLsizei)USHRT_MAX)
         {
            /* count * stride > 16 bit so do no more than ushrt_max in this batch */
            /* for lines, triangle strips etc. will need 1 or more of the previous vertices so step < ushrt_max */
            /* TODO for line loops and triangle fans would have to copy vertex data so can insert first vertex at beginning of new chunk */
            switch(state->batch.primitive_mode)
            {
               case GL_POINTS:
                  step = batch_count = USHRT_MAX;
                  break;
               case GL_LINES:
                  step = batch_count = USHRT_MAX - (USHRT_MAX % 2);
                  break;
               case GL_LINE_LOOP:
                  goto fail;/* not implemented as require expensive alloc and copy */
               case GL_LINE_STRIP:
                  batch_count = USHRT_MAX;
                  step = USHRT_MAX - 1;
                  break;
               case GL_TRIANGLES:
                  step = batch_count = USHRT_MAX - (USHRT_MAX % 3);
                  break;
               case GL_TRIANGLE_STRIP:
                  batch_count = USHRT_MAX;
                  step = USHRT_MAX - 2;
                  break;
               case GL_TRIANGLE_FAN:
                  goto fail;/* not implemented as require expensive alloc and copy */
               default:
                  UNREACHABLE();
                  return 0;
            }
         }
         else
         {
            step = batch_count = count;
         }

         instr = glxx_big_mem_alloc_cle(20);
         if (!instr) goto fail;

         add_byte(&instr, KHRN_HW_INSTR_GL_SHADER);     //(5)
         add_pointer(&instr, (uint8_t *)shader_record + (attr_count & 7), &shader_record_lbh);

         // Emit a GLDRAWARRAYS instruction
         add_byte(&instr, KHRN_HW_INSTR_GLDRAWARRAYS);
         add_byte(&instr, convert_primitive_type(state->batch.primitive_mode));  //Primitive mode
         add_word(&instr, batch_count);       //Length (number of vertices)
         add_word(&instr, batch_indices_offset);            //Index of first vertex

         add_byte(&instr, KHRN_HW_INSTR_NOP);         //Pad to the same length as KHRN_HW_INSTR_GLDRAWELEMENTS to make it easier for ourselves
         add_byte(&instr, KHRN_HW_INSTR_NOP);
         add_byte(&instr, KHRN_HW_INSTR_NOP);
         add_byte(&instr, KHRN_HW_INSTR_NOP);

         add_byte(&instr, KHRN_HW_INSTR_NOP);        //(1) TODO: is this necessary?

         vcos_assert(step <= (uint32_t)count);

         count = count - step;

         if(count > 0)
         {
            GLXX_HW_SHADER_RECORD_T *new_shader_record;
            MEM_LOCK_T new_shader_record_lbh;
            /* some vertices were not drawn */
            /* create a new shader record with offset pointers for another GLDRAWARRAYS instruction */
            new_shader_record = (GLXX_HW_SHADER_RECORD_T *)glxx_big_mem_alloc_junk(100, 16, &new_shader_record_lbh);
            if (!new_shader_record) goto fail;
            khrn_memcpy(new_shader_record,shader_record,sizeof(GLXX_HW_SHADER_RECORD_T));
            shader_record = new_shader_record;
            /* get_shaders() put copies of the handles we need in shader_record->fshader etc. */
            glxx_big_mem_insert(&shader_record->fshader,shader_record->fshader,0);
            glxx_big_mem_insert(&shader_record->vshader,shader_record->vshader,0);
            glxx_big_mem_insert(&shader_record->cshader,shader_record->cshader,0);

            offset += step + indices_offset;
            indices_offset = 0;
            /* advance attribute pointers */
            for(j=0; j < attr_count; j++)
            {
               i = shader_record->attr[j].base;
               vcos_assert(i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);/* stored i here in do_vcd_setup */
               glxx_big_mem_insert(&shader_record->attr[j].base, attrib_handles[i], (uint32_t)attrib[i].pointer + offset*shader_record->attr[j].stride);
               vcos_assert(i == shader_record->attr[j].base);/* check big_mem_insert didn't change this */
            }
         }
         else
         {
            count = 0;
         }
      }
   }
   else
   {
      instr = glxx_big_mem_alloc_cle(20);
      if (!instr) goto fail;

      add_byte(&instr, KHRN_HW_INSTR_GL_SHADER);     //(5)
      add_pointer(&instr, (uint8_t *)shader_record + (attr_count & 7), &shader_record_lbh);

      // Emit a GLDRAWELEMENTS instruction
      add_byte(&instr, KHRN_HW_INSTR_GLDRAWELEMENTS);   //(14)
      add_byte(&instr,
         convert_primitive_type(state->batch.primitive_mode) |   //Primitive mode
         convert_index_type(type) << 4);  //Index type

      add_word(&instr, count);        //Length (number of indices)
      if (!glxx_big_mem_add_fix(&instr, indices_handle, indices_offset))
         goto fail;

      add_word(&instr, _min(max_index, 0xffff));    //Maximum index (primitives using a greater index will cause error)

      add_byte(&instr, KHRN_HW_INSTR_NOP);        //(1) TODO: is this necessary?
   }

   /* Mark the state as drawn so that clearing works properly */
   rs->drawn = true;
   rs->xxx_empty = false;

   //now add references to vertex attribute cache interlocks to fixer
   for (i = 0; i < interlock_count; i++)
   {
      KHRN_INTERLOCK_T *lock = (KHRN_INTERLOCK_T *)((char *)mem_lock(interlocks[i].mh_handle, NULL) + interlocks[i].offset);
      khrn_interlock_read(lock, khrn_interlock_user(rs->name));
      mem_unlock(interlocks[i].mh_handle);
      glxx_hw_insert_interlock(interlocks[i].mh_handle, interlocks[i].offset);
      //fixer should then mark all the interlocks as in use and when the next flush occurs transfer them to the hardware
      //client is then able to wait until hardware has finished before deleting a cache entry
   }

   glxx_unlock_fixer_stuff();

done:

   if(!IS_GL_11(state)) {
      mem_unlock(state->mh_program);
   }

   return true;

fail:
   glxx_unlock_fixer_stuff();

fail2:
   if(!IS_GL_11(state)) {
      mem_unlock(state->mh_program);
      if(locked_program_items)
      {
         mem_unlock(program->mh_sampler_info);
         mem_unlock(program->mh_uniform_info);
         mem_unlock(program->mh_uniform_data);
         locked_program_items = false;
      }
   }

   glxx_hw_discard_frame(rs);

   return false;
}

/*************************************************************
 Static Functions
 *************************************************************/

static void save_shader_tool_file(GL20_PROGRAM_T *program, GLXX_SERVER_STATE_T *state)
{
   // Create a .sts shader file
   FILE                    *sts = NULL;
   char                    buf[4096];
   char                    stsName[1024];
   GL20_SHADER_T           *shader;
   int32_t                 count, i;
   MEM_HANDLE_T            *handles;
   GLXX_LINK_RESULT_KEY_T  *key = &state->shader.common;

   // Work out the running program name
#ifdef WIN32
   {
      GetModuleFileNameA(0, buf, 4096);

      if (strlen(buf) > 0)

      {
         char *lastSlash = strrchr(buf, '\\');
         if (lastSlash != 0 && lastSlash != buf + strlen(buf) - 1)
            strcpy(stsName, lastSlash + 1);
         else
            strcpy(stsName, buf);
      }
   }
#else

   {
      FILE *fp;

#ifdef ANDROID
      snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
#else
      snprintf(buf, sizeof(buf), "/proc/%d/comm", getpid());
#endif

      fp = fopen(buf, "r");
      if (fp != NULL)
      {
         char *s;
         buf[0] = '\0';
         s = fgets(buf, 4096, fp);
         fclose(fp);
         if (s == NULL)
         {
             fprintf(stderr, "Warning : unable to get the command line\n");
             return;
         }

         if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = '\0';

         strcpy(stsName, buf);
      }
   }
#endif /* !WIN32 */


   snprintf(buf, sizeof(buf), "%s/%s-%d-%d.sts", khrn_options.glsl_sts_save_dir, stsName, program->name, program->debug_save_count++);

   printf("Creating %s\n", buf);

   sts = fopen(buf, "w");
   if (sts == NULL)
   {
      fprintf(stderr, "Warning : could not open %s for writing\n", buf);
      return;
   }

   fprintf(sts, "<vertexShader>");
   shader = (GL20_SHADER_T *)mem_lock(program->mh_vertex, NULL);
   count = mem_get_size(shader->mh_sources_compile) / sizeof(MEM_HANDLE_T);

   handles = (MEM_HANDLE_T *)mem_lock(shader->mh_sources_compile, NULL);
   for (i = 0; i < count; i++)
   {
      fprintf(sts, "%s\n", (char *)mem_lock(handles[i], NULL));
      mem_unlock(handles[i]);
   }
   mem_unlock(shader->mh_sources_compile);
   mem_unlock(program->mh_vertex);
   fprintf(sts, "</vertexShader>\n");

   fprintf(sts, "<fragmentShader>");
   shader = (GL20_SHADER_T *)mem_lock(program->mh_fragment, NULL);
   count = mem_get_size(shader->mh_sources_compile) / sizeof(MEM_HANDLE_T);

   handles = (MEM_HANDLE_T *)mem_lock(shader->mh_sources_compile, NULL);
   for (i = 0; i < count; i++)
   {
      fprintf(sts, "%s\n", (char *)mem_lock(handles[i], NULL));
      mem_unlock(handles[i]);
   }
   mem_unlock(shader->mh_sources_compile);
   mem_unlock(program->mh_fragment);
   fprintf(sts, "</fragmentShader>\n");

   fprintf(sts, "hasDepth=%d\n", key->use_depth ? 1 : 0);
   fprintf(sts, "hasStencil=%d\n", (key->stencil_config & 0x1) != 0 ? 1 : 0);
   fprintf(sts, "hasMSAA=%d\n", state->mh_color_multi == MEM_INVALID_HANDLE ? 0 : 1);
   fprintf(sts, "is565=%d\n", key->rgb565 ? 1 : 0);
   fprintf(sts, "renderAlpha=%d\n", key->render_alpha ? 1 : 0);

   fprintf(sts, "colorMask=0x%08X\n", key->blend.color_mask);
   fprintf(sts, "blendEnabled=%d\n", state->caps.blend ? 1 : 0);

   fprintf(sts, "blendColor=0x");
   for (i = 0; i < 4; i++)
      fprintf(sts, "%02X", (uint32_t)(state->blend_color[i] * 255.0f));
   fprintf(sts, "\n");

   fprintf(sts, "blendDstRGB=%d\n", key->blend.dst_function);
   fprintf(sts, "blendSrcRGB=%d\n", key->blend.src_function);
   fprintf(sts, "blendSrcAlpha=%d\n", key->blend.src_function_alpha);
   fprintf(sts, "blendDstAlpha=%d\n", key->blend.dst_function_alpha);
   fprintf(sts, "blendEquationRGB=%d\n", key->blend.equation);
   fprintf(sts, "blendEquationAlpha=%d\n", key->blend.equation_alpha);
   fprintf(sts, "sampleAlphaToCoverageEnabled=%d\n", state->caps.sample_alpha_to_coverage ? 1 : 0);
   fprintf(sts, "sampleCoverageEnabled=%d\n", state->caps.sample_coverage ? 1 : 0);
   fprintf(sts, "sampleCoverageInvert=%d\n", state->sample_coverage.invert ? 1 : 0);
   fprintf(sts, "sampleCoverageValue=%f\n", state->sample_coverage.value);

   fclose(sts);
}

static bool get_shaders(
    GL20_PROGRAM_T *program,
    GLXX_HW_SHADER_RECORD_T *shader_out,
    MEM_HANDLE_T *cunifmap_out,
    MEM_HANDLE_T *vunifmap_out,
    MEM_HANDLE_T *funifmap_out,
    uint32_t *color_varyings_out,
    GLXX_SERVER_STATE_T *state,
    GLXX_ATTRIB_T *attrib,
    uint32_t *mergeable_attribs,
    uint32_t *cattribs_order_out,
    uint32_t *vattribs_order_out)
{
   bool ok = false;
   bool saveSts = false;
   bool wasInCache = false;

   if (IS_GL_11(state))
   {
      ok = gl11_hw_get_shaders(
         shader_out,
         cunifmap_out,
         vunifmap_out,
         funifmap_out,
         color_varyings_out,
         state,
         attrib,
         mergeable_attribs,
         cattribs_order_out,
         vattribs_order_out);

   }
   else
   {
      xxx_shader = program->name;

      ok = gl20_link_result_get_shaders(
         &program->result,
         shader_out,
         cunifmap_out,
         vunifmap_out,
         funifmap_out,
         state,
         attrib,
         mergeable_attribs,
         cattribs_order_out,
         vattribs_order_out,
         &wasInCache);
   }

   if (!ok && khrn_options.gl_error_assist)
   {
      if (IS_GL_11(state))
         printf("FAILED TO SCHEDULE ES1 SHADER\n");
      else
      {
         printf("FAILED TO SCHEDULE ES2 SHADER PROGRAM %d\n", program->name);
         saveSts = true;
      }
   }

   if (!wasInCache && (saveSts || khrn_options.glsl_sts_saving_on) && program != NULL)
      save_shader_tool_file(program, state);

   return ok;
}

static bool install_uniform(uint32_t *ptr, uint32_t u0, uint32_t u1,
   GLXX_SERVER_STATE_T *state,
   GL20_HW_INDEXED_UNIFORM_T *iu,
   uint32_t * num_vpm_rows,
   GLXX_ATTRIB_T *attrib,
   bool egl_output,
   unsigned int fb_height,
   bool secure)
{
   switch (u0)
   {
   case BACKEND_UNIFORM:
   {
      if(u1 & (GLXX_ATTRIB_MASK - 0x80000000)) //u1 is an offset into the server state struct
      {
         float *x  = GLXX_READ_ATTRIB_OFFSET(attrib, u1);
         *ptr = *(uint32_t *)x;
      }
      else// gl 1.1 and 2.0 specials
      {
         uint32_t u = u1 & ~3;
         uint32_t j = u1 % 4;
         //uint32_t k;
         switch (u)
         {
         //common 1.1 and 2.0 cases
         case BACKEND_UNIFORM_VPM_READ_SETUP:
            {
               uint8_t data_start = 0;
               uint32_t i;
               vcos_assert(j <= 3 && num_vpm_rows[j] <= 15);
               for (i = 0; i < j; i++)
                  data_start += num_vpm_rows[i];
               data_start &= 0x3F;
               *ptr = 0x00001a00 | ((num_vpm_rows[j] & 15) << 20) | data_start;
            }
            break;
         case BACKEND_UNIFORM_VPM_WRITE_SETUP:
            *ptr = 0x00001a00;
            break;
         case BACKEND_UNIFORM_STENCIL_FRONT:
            *ptr =
               (state->stencil_func.front.mask & 0xff) |
               _max(_min(state->stencil_func.front.ref, 0xff), 0) << 8 |
               glxx_hw_convert_test_function(state->stencil_func.front.func) << 16 |
               glxx_hw_convert_operation(state->stencil_op.front.fail) << 19 |
               glxx_hw_convert_operation(state->stencil_op.front.zpass) << 22 |
               glxx_hw_convert_operation(state->stencil_op.front.zfail) << 25 |
               1 << 30;
            break;
         case BACKEND_UNIFORM_STENCIL_BACK:
            *ptr =
               (state->stencil_func.back.mask & 0xff) |
               _max(_min(state->stencil_func.back.ref, 0xff), 0) << 8 |
               glxx_hw_convert_test_function(state->stencil_func.back.func) << 16 |
               glxx_hw_convert_operation(state->stencil_op.back.fail) << 19 |
               glxx_hw_convert_operation(state->stencil_op.back.zpass) << 22 |
               glxx_hw_convert_operation(state->stencil_op.back.zfail) << 25 |
               2 << 30;
            break;
         case BACKEND_UNIFORM_STENCIL_MASK:
            *ptr =
               (state->stencil_mask.front & 0xff) |
               (state->stencil_mask.back & 0xff) << 8 |
               0 << 30;
            break;
         //2.0 specific cases
         case BACKEND_UNIFORM_BLEND_COLOR:
            vcos_assert(!IS_GL_11(state));
            *ptr = color_floats_to_rgba(state->blend_color[0], state->blend_color[1], state->blend_color[2], state->blend_color[3]);
            break;
         case BACKEND_UNIFORM_FBHEIGHT:
         {
            if (khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN)
               *ptr = float_to_bits((float)fb_height);
            break;
         }
         case BACKEND_UNIFORM_EGL_OUTPUT_FORMAT:
            if (khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN)
               *ptr = egl_output;
            break;
         case BACKEND_UNIFORM_DEPTHRANGE_NEAR:
            vcos_assert(!IS_GL_11(state));
            *ptr = *(uint32_t *)&state->viewport.internal[6];
            break;
         case BACKEND_UNIFORM_DEPTHRANGE_FAR:
            vcos_assert(!IS_GL_11(state));
            *ptr = *(uint32_t *)&state->viewport.internal[7];
            break;
         case BACKEND_UNIFORM_DEPTHRANGE_DIFF:
            vcos_assert(!IS_GL_11(state));
            *ptr = *(uint32_t *)&state->viewport.internal[8];
            break;
         default:
            UNREACHABLE();
            goto fail;
         }
      }
      break;
   }
   case BACKEND_UNIFORM_TEX_CUBE_STRIDE:
      if(IS_GL_11(state)) {
         *ptr = 0;
      } else {
         if(!glxx_install_tex_param(state, ptr, u0, u1, secure))
            goto fail;
      }
      break;
   case BACKEND_UNIFORM_TEX_NOT_USED:
      *ptr = 0;
      break;
   case BACKEND_UNIFORM_TEX_PARAM0:
   case BACKEND_UNIFORM_TEX_PARAM1:
      if(!glxx_install_tex_param(state, ptr, u0, u1, secure))
         goto fail;
      break;
   case BACKEND_UNIFORM_ADDRESS:
   {
//      count_platform_specific ++;
      if(!backend_uniform_address(u1, state, iu, ptr))
         goto fail;
      break;
   }
   default:
      UNREACHABLE();
      goto fail;
   }

   return true;
fail:
   return false;
}

static bool install_uniforms(
   uint32_t *startaddr_location,
   GLXX_SERVER_STATE_T *state,
   GL20_PROGRAM_T *program,
   uint32_t count,
   MEM_HANDLE_T hmap,
   GL20_HW_INDEXED_UNIFORM_T *iu,
   uint32_t * num_vpm_rows,
   GLXX_ATTRIB_T *attrib,
   bool egl_output,
   unsigned int fb_height,
   bool secure)
{
   //debugging
   //uint32_t count_platform_specific = 0;
   uint32_t i;
   uint32_t *ptr;
   MEM_LOCK_T ptr_lbh;
   uint32_t *map;
   //gl 2.0 specific
   uint32_t max = 0;

   ptr = glxx_big_mem_alloc_junk(4 * count, 4, &ptr_lbh);

   if (!ptr) return false;
   *startaddr_location = khrn_hw_addr(ptr, &ptr_lbh); //PTR

   map = (uint32_t *)mem_lock(hmap, NULL);

   if(!IS_GL_11(state)) {
      max = mem_get_size(program->mh_uniform_data);
   }

   for (i = count; i != 0; --i, ptr++)
   {
      uint32_t u0 = *map++;
      uint32_t u1 = *map++;

      bool     backendUniform = u0 == BACKEND_UNIFORM;

      if (backendUniform && u1 < 0x80000000)
      {
         //gl 2.0 uniforms
         *ptr = state->batch.uniform_data[u1];
      }
      else if (backendUniform && (u1 & (GLXX_STATE_MASK - 0x80000000)))
      {
          //u1 is an offset into the server state struct
         *ptr = *(uint32_t *)GLXX_READ_STATE_OFFSET(state, u1);
      }
      else if (u0 == BACKEND_UNIFORM_LITERAL)
      {
         *ptr = u1;
      }
      else
      {
         if (!install_uniform(ptr, u0, u1, state, iu, num_vpm_rows, attrib, egl_output, fb_height, secure))
            goto fail;
      }
   }

   mem_unlock(hmap);

   return true;
fail:
   mem_unlock(hmap);
   return false;
}

static int texture_buffer(GLenum target)
{
    switch (target) {
    case GL_TEXTURE_2D:
        return TEXTURE_BUFFER_TWOD;
    case GL_TEXTURE_CUBE_MAP:
        return TEXTURE_BUFFER_POSITIVE_X;
    case GL_TEXTURE_EXTERNAL_OES:
        return TEXTURE_BUFFER_EXTERNAL;
    default:
        UNREACHABLE();
    }
    return TEXTURE_BUFFER_TWOD;
}

static bool use_dummy(uint32_t *location, uint32_t u0)
{
   int i;

   if (dummy_texture_handle == MEM_HANDLE_INVALID)
   {
      dummy_texture_handle = mem_alloc_ex(64, 4096,
                                          (MEM_FLAG_T)(MEM_FLAG_DIRECT | MEM_FLAG_NO_INIT),
                                          "Dummy texture data",
                                          MEM_COMPACT_DISCARD);
      if (dummy_texture_handle == MEM_HANDLE_INVALID)
         return false;

      dummy_texture_data = mem_lock(dummy_texture_handle, &dummy_texture_lbh);
      if (dummy_texture_data == NULL)
         return false;
      for (i = 0; i < 16; i++)
         dummy_texture_data[i] = 0xff000000;
   }

   switch (u0) {
      case BACKEND_UNIFORM_TEX_PARAM0:
         *location = khrn_hw_addr(dummy_texture_data, &dummy_texture_lbh);
         break;
      case BACKEND_UNIFORM_TEX_PARAM1:
         *location = 4 << 8 | 4 << 20;
         break;
      default:
         *location = 0;
   }
   return true;
}

static KHRN_IMAGE_FORMAT_T image_for_texturing_format(KHRN_IMAGE_FORMAT_T format)
{
   /* YUV contains no alpha */
   return (format == YV12_RSO) ? XBGR_8888_TF : khrn_image_to_tf_format(format);
}

static KHRN_IMAGE_FORMAT_T image_for_shadow_format(KHRN_IMAGE_FORMAT_T format)
{
   /* YUV contains no alpha */
   return (format == YV12_RSO) ? XBGR_8888_RSO : format;
}

static MEM_HANDLE_T image_for_texturing(GLXX_SERVER_STATE_T *state,
   MEM_HANDLE_T heglimage, bool secure)
{
   EGL_IMAGE_T *egl_image = (EGL_IMAGE_T*)mem_lock(heglimage, NULL);

   if (egl_image)
   {
      /* if the image is an underlying platform client buffer, then it may have resized since it
         originally was mapped */
      if ((egl_image->platform_client_buffer) &&
          (egl_image->buffer) &&
          (egl_image->mh_image != MEM_HANDLE_INVALID) &&
          (egl_image->mh_tf_image != MEM_HANDLE_INVALID))
      {
         MEM_HANDLE_T new_image = khrn_platform_image_wrap(egl_image->buffer);

         KHRN_IMAGE_T *src = (KHRN_IMAGE_T *)mem_lock(egl_image->mh_image, NULL);
         KHRN_IMAGE_T *dst = (KHRN_IMAGE_T *)mem_lock(egl_image->mh_tf_image, NULL);

         uintptr_t src_offset = mem_lock_offset(src->mh_storage);
         uintptr_t dst_offset = mem_lock_offset(dst->mh_storage);

         if (src_offset != dst_offset)
         {
            MEM_ASSIGN(egl_image->mh_image, new_image);
            MEM_ASSIGN(egl_image->mh_tf_image, MEM_HANDLE_INVALID);
         }
         MEM_ASSIGN(new_image, MEM_HANDLE_INVALID);

         mem_unlock(src->mh_storage);
         mem_unlock(dst->mh_storage);

         mem_unlock(egl_image->mh_image);
         mem_unlock(egl_image->mh_tf_image);
      }

      /* already have a conversion this draw call, just return that */
      if (egl_image->mh_tf_image != MEM_HANDLE_INVALID)
      {
         MEM_HANDLE_T res = egl_image->mh_tf_image;
         mem_unlock(heglimage);
         return res;
      }

      KHRN_IMAGE_T *src = (KHRN_IMAGE_T *)mem_lock(egl_image->mh_image, NULL);

      /* egl image is already bound as TF, just return this */
      if (khrn_image_is_tformat(src->format))
      {
         mem_unlock(heglimage);
         return egl_image->mh_image;
      }

      /* if the context is secure, promote to secure target */
      bool image_secure = src->secure || secure;

      /* Create the tformat variant */
      egl_image->mh_tf_image = khrn_image_create(image_for_texturing_format(src->format),
         src->width, src->height, IMAGE_CREATE_FLAG_TEXTURE, image_secure);

      /* copy the image if required */
      if (khrn_options.shadow_egl_images || src->format == YV12_RSO)
      {
         /* Create a RSO scratch to travel down the pipeline */
         MEM_HANDLE_T hscratch = khrn_image_create(
            image_for_shadow_format(src->format),
            src->width, src->height,
            IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_DISPLAY,
            image_secure);

         if (hscratch != MEM_HANDLE_INVALID)
         {
            KHRN_IMAGE_T *scratch = mem_lock(hscratch, NULL);
            if (!image_secure)
            {
               void *dp = mem_lock(scratch->mh_storage, NULL);
               khrn_hw_flush_dcache_range(dp, mem_get_size(scratch->mh_storage));
               mem_unlock(scratch->mh_storage);
            }

            mem_copy2d(src->format, scratch->mh_storage, src->mh_storage,
               src->width, src->height, src->stride, scratch->stride, image_secure);
            mem_unlock(hscratch);

            khrn_rso_to_tf_convert(state, hscratch, egl_image->mh_tf_image);

            MEM_ASSIGN(hscratch, MEM_INVALID_HANDLE);
         }
      }
      else
         khrn_rso_to_tf_convert(state, egl_image->mh_image, egl_image->mh_tf_image);

      mem_unlock(heglimage);

      return egl_image->mh_tf_image;
   }

   return MEM_HANDLE_INVALID;
}

vcos_static_assert(GL11_CONFIG_MAX_TEXTURE_UNITS <= GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS);
static bool glxx_install_tex_param(GLXX_SERVER_STATE_T *state, uint32_t *location,
   uint32_t u0, uint32_t u1, bool secure)
{
   bool ok = false;
   bool in_vshader = false;

   *location = 0;

   MEM_HANDLE_T thandle;
   if (IS_GL_11(state))
      thandle = get_texture_11(state, u1);
   else
      thandle = get_texture_20(state, u1, &in_vshader);

   GLXX_TEXTURE_T *texture = NULL;
   if (thandle != MEM_HANDLE_INVALID)
   {
      texture = (GLXX_TEXTURE_T *)mem_lock(thandle, NULL);

      switch (glxx_texture_check_complete(texture))
      {
      case INCOMPLETE:
         texture = NULL;
         break;
      case OUT_OF_MEMORY:
         goto end;
      case COMPLETE:
         /* NOTHING */
         break;
      }
   }

   if (texture == NULL)
   {
      vcos_assert(!IS_GL_11(state));
      ok = use_dummy(location, u0);
      goto end;
   }

   bool is_cube = (texture->target == GL_TEXTURE_CUBE_MAP);
   MEM_HANDLE_T handle = MEM_INVALID_HANDLE;
   switch (u0)
   {
      case BACKEND_UNIFORM_TEX_PARAM0:
      {
         uint32_t mipmap_count = 0;
         uint32_t type = 0;
         uint32_t offset = 0;

         if (texture->external_image != MEM_INVALID_HANDLE)
         {
            MEM_HANDLE_T himage = image_for_texturing(state, texture->external_image, secure);
            KHRN_IMAGE_T *image = (KHRN_IMAGE_T*)mem_lock(himage, NULL);
            type = tu_image_format_to_type(khrn_image_to_tf_format(image->format));
            handle = image->mh_storage;
            offset = image->offset;
            mem_unlock(himage);
         }
         else
         {
            type = tu_image_format_to_type(glxx_texture_get_tformat(texture));
            int buf_index = texture_buffer(texture->target);

            if (texture->binding_type != TEXTURE_STATE_COMPLETE_UNBOUND || (!is_cube && texture->framebuffer_sharing))
            {
               KHRN_IMAGE_T *image;

               image = mem_lock(texture->mh_mipmaps[buf_index][0], NULL);
               handle = image->mh_storage;
               offset = image->offset;
               mem_unlock(texture->mh_mipmaps[buf_index][0]);
            }
            else
            {
               handle = glxx_texture_get_storage_handle(texture);
               offset = glxx_texture_get_mipmap_offset(texture, buf_index, 0, false);
               /* printf("offset = %p, texture (%d, %d), buf_index %d\n", offset, texture->width, texture->height, buf_index); */
            }
         }

         vcos_assert(!is_cube || !IS_GL_11(state));

         mipmap_count = glxx_texture_get_mipmap_count(texture);

         vcos_assert(handle != MEM_INVALID_HANDLE);
         vcos_assert(mipmap_count >= 1 && mipmap_count <= LOG2_MAX_TEXTURE_SIZE + 1);

         offset += (mipmap_count - 1) | (type & 15) << 4 | is_cube << 9;
         if(!glxx_big_mem_insert(location, handle, offset))
            goto end;

         if (!glxx_hw_texture_fix(thandle, in_vshader))
            goto end;

         break;
      }
      case BACKEND_UNIFORM_TEX_PARAM1:
      {
         uint32_t type;
         if (texture->external_image != MEM_INVALID_HANDLE)
            type = tu_image_format_to_type(image_for_texturing_format(glxx_texture_get_tformat(texture)));
         else
            type = tu_image_format_to_type(glxx_texture_get_tformat(texture));
         *location =
            ((type >> 4) & 1) << 31 |
            glxx_convert_wrap(texture->wrap.s) << 0 |
            glxx_convert_wrap(texture->wrap.t) << 2 |
            glxx_convert_filter(texture->min) << 4 |
            glxx_convert_filter(texture->mag) << 7 |
            (texture->width & 0x7ff) << 8 |
            1 << 19 |     /* ETCFLIPY */
            (texture->height & 0x7ff) << 20;
         break;
      }
      case BACKEND_UNIFORM_TEX_CUBE_STRIDE:
      {
         vcos_assert(!IS_GL_11(state));

         uint32_t cube_stride = 0;
         if (is_cube)
         {
            uint32_t stride = glxx_texture_get_cube_stride(texture);

            vcos_assert(!(stride & 0xfff));
               cube_stride = stride | (1 << 30);
         }
         /* disable automatic level of detail in vertex shader */
         if (in_vshader)
            cube_stride |= 1 | (1 << 30);

         *location = cube_stride;
         break;
      }
      default:
         UNREACHABLE();
         return false;
   }
   ok = true;
end:
   if (thandle != MEM_HANDLE_INVALID)
       mem_unlock(thandle);

   if (!ok)
      MEM_ASSIGN(handle, MEM_HANDLE_INVALID);

   return ok;
}

static bool backend_uniform_address( uint32_t u1,
                  GLXX_SERVER_STATE_T *state,
                  GL20_HW_INDEXED_UNIFORM_T *iu,
                  uint32_t *location)
{
   uint32_t index = u1 & 0xffff;
   uint32_t size = u1 >> 16;
   uint32_t i;
   uint32_t *texa = NULL;
   MEM_LOCK_T texa_lbh = { 0 };
   UNUSED_NDEBUG(state);

   vcos_assert(!IS_GL_11(state));
   for (i = 0; i < iu->count; i++)
   {
      if (iu->index[i] == index && iu->size[i] == size)
      {
         texa = iu->addr[i];
         texa_lbh = iu->lbh[i];
         break;
      }
   }

   if (i == iu->count)
   {
      texa = glxx_big_mem_alloc_junk(4 * size, 4, &texa_lbh);
      if(!texa)
      {
         *location = 0;
         return false;
      }

      khrn_memcpy(texa, state->batch.uniform_data + index, 4 * size);

      if (iu->count < IU_MAX)
      {
         iu->index[iu->count] = index;
         iu->size[iu->count] = size;
         iu->addr[iu->count] = texa;
         iu->lbh[iu->count] = texa_lbh;
         iu->count++;
      }
   }

   *location = khrn_hw_addr(texa, &texa_lbh); //PTR
   return true;
}

/*!
 * \brief Converts the primitive type from its GLenum representation
 *        to the representation used in the simulator (unsigned int).
 *
 * Also asserts that the mode is within the valid/implemented range.
 *
 * \param mode is the GL primitive type.
 */
static uint32_t convert_primitive_type(GLenum mode)
{
   vcos_assert(mode <= GL_TRIANGLE_FAN);

   return (uint32_t)mode;
}

/*!
 * \brief Converts the index type from the GLenum
 *        representation to the internal one used in the simulator.
 *
 * \param type is the GL index type specifier.
 */
static uint32_t convert_index_type(GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_BYTE:
      return 0; // VCM_SRCTYPE_8BIT;
   case GL_UNSIGNED_SHORT:
      return 1; // VCM_SRCTYPE_16BIT;
   default:
      UNREACHABLE(); // unsupported index type
      return 0;
   }
}

static void draw_tex_log(GLXX_SERVER_STATE_T *state, GLXX_HW_FRAMEBUFFER_T *fb, GLXX_HW_RENDER_STATE_T *rs,
                         GLfloat Xs, GLfloat Ys, GLfloat Zw, GLfloat Ws, GLfloat Hs)
{
   GL11_CACHE_KEY_T * entry = &state->shader;
   int i;

   UNUSED(fb);
   UNUSED(rs);
   UNUSED(Xs);
   UNUSED(Ys);
   UNUSED(Zw);
   UNUSED(Ws);
   UNUSED(Hs);

   vcos_log(LOG_INFO, "state_name: %d rs: %d drawTex: Xs %f Ys %f Zw %f Ws %f Hs %f\n",
      state->name, rs->name, Xs, Ys, Zw, Ws, Hs);

   for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (entry->texunits[i].props.active)
      {
         GLXX_TEXTURE_T * texture;
         vcos_log(LOG_INFO, "texunit:%d mode:0x%x coord_replace:0x%x",i,entry->texunits[i].mode,entry->texunits[i].coord_replace);
         vcos_log(LOG_INFO, "   active:%d complex:%d has_color:%d has_alpha:%d",entry->texunits[i].props.active,entry->texunits[i].props.complex,
            entry->texunits[i].props.has_color, entry->texunits[i].props.has_alpha);
         vcos_log(LOG_INFO, "   rgb: combine:0x%x scale:%f source:0x%x 0x%x 0x%x operand:0x%x 0x%x 0x%x",entry->texunits[i].rgb.combine,entry->texunits[i].rgb.scale,
            entry->texunits[i].rgb.source[0],entry->texunits[i].rgb.source[1],entry->texunits[i].rgb.source[2],
            entry->texunits[i].rgb.operand[0],entry->texunits[i].rgb.operand[1],entry->texunits[i].rgb.operand[2]);
         vcos_log(LOG_INFO, "   alpha: combine:0x%x scale:%f source:0x%x 0x%x 0x%x operand:0x%x 0x%x 0x%x",entry->texunits[i].alpha.combine,entry->texunits[i].alpha.scale,
            entry->texunits[i].alpha.source[0],entry->texunits[i].alpha.source[1],entry->texunits[i].alpha.source[2],
            entry->texunits[i].alpha.operand[0],entry->texunits[i].alpha.operand[1],entry->texunits[i].alpha.operand[2]);
         texture = mem_lock(state->bound_texture[i].mh_twod, NULL);
         vcos_log(LOG_INFO, "   handle: %d crop_rect: Ucr %d Vcr %d Wcr %d Hcr %d W %d H %d",
            state->bound_texture[i].mh_twod,
            texture->crop_rect.Ucr,texture->crop_rect.Vcr,texture->crop_rect.Wcr,texture->crop_rect.Hcr,texture->width,texture->height);
         vcos_log(LOG_INFO, "   wrap: s 0x%x t 0x%x mag: 0x%x min: 0x%x",texture->wrap.s,texture->wrap.t,texture->mag,texture->min);
         mem_unlock(state->bound_texture[i].mh_twod);
      }
   }

   vcos_log(LOG_INFO, "viewport: x %d y %d w %d h %d",state->viewport.x,state->viewport.y,state->viewport.width,state->viewport.height);
   vcos_log(LOG_INFO, "scissor_test: %d",state->caps.scissor_test);
   if (state->caps.scissor_test)
      vcos_log(LOG_INFO, "  scissor: x %d y %d w %d h %d",state->scissor.x,state->scissor.y,state->scissor.width,state->scissor.height);
   vcos_log(LOG_INFO, "blend: %d",state->caps.blend);
   if (state->caps.blend)
   {
      vcos_log(LOG_INFO, "  color_mask 0x%x eqn 0x%x eqn_alpha 0x%x",
         entry->common.blend.color_mask,entry->common.blend.equation,entry->common.blend.equation_alpha);
      vcos_log(LOG_INFO, "  src_func 0x%x src_func_a 0x%x dst_func 0x%x dst_func_a 0x%x",
         entry->common.blend.src_function,entry->common.blend.src_function_alpha,entry->common.blend.dst_function,entry->common.blend.dst_function_alpha);
      vcos_log(LOG_INFO, "  blend_color: (%f %f %f %f)",
         state->blend_color[0],state->blend_color[1],state->blend_color[2],state->blend_color[3]);
   }
   vcos_log(LOG_INFO, "stencil_test: %d depth_test: %d",state->caps.stencil_test,state->caps.depth_test);
   vcos_log(LOG_INFO, "logic_op: %d func: 0x%x",state->caps.color_logic_op,entry->common.blend.logic_op);
   vcos_log(LOG_INFO, "dither: %d multisample: %d",state->caps.dither,state->caps.multisample);
   vcos_log(LOG_INFO, "sample_a2c: %d sample_c: %d sample_a21: %d",
      state->caps.sample_alpha_to_coverage,state->caps.sample_coverage,state->caps.sample_alpha_to_one);
   vcos_log(LOG_INFO, "polygon_offset: %d alpha_test: %d fog: %d",
      state->caps.polygon_offset_fill,state->caps_fragment.alpha_test,state->caps_fragment.fog);
   vcos_log(LOG_INFO, "color: (%f %f %f %f)",
         state->copy_of_color[0],state->copy_of_color[1],state->copy_of_color[2],state->copy_of_color[3]);

   vcos_log(LOG_INFO, "cattribs_live:0x%x vattribs_live:0x%x",entry->cattribs_live,entry->vattribs_live);

   /* TODO frame buffer format */
   vcos_log(LOG_INFO, "framebuffer: format 0x%x has_depth %d has_stencil %d ms %d w %d h %d",
      fb->col_format, fb->have_depth, fb->have_stencil, fb->ms, fb->width, fb->height);

   vcos_log(LOG_INFO, "--------------------");
}

bool glxx_hw_draw_tex(GLXX_SERVER_STATE_T *state, float Xs, float Ys, float Zw, float Ws, float Hs, bool secure)
{
   uint32_t i;
   GLXX_HW_FRAMEBUFFER_T fb;
   MEM_HANDLE_T cunif_map, vunif_map, funif_map;
   uint32_t cattribs_live;
   uint32_t vattribs_live;

   uint32_t cunif_count;
   uint32_t vunif_count;
   uint32_t funif_count;
   uint32_t mergeable_attribs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   uint32_t vattribs_order[GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2];
   uint32_t cattribs_order[GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2];
   uint32_t num_vpm_rows_c[4];
   uint32_t num_vpm_rows_v[4];
   GLXX_HW_RENDER_STATE_T *rs;
   GLXX_HW_SHADER_RECORD_T *shader_record;
   MEM_LOCK_T shader_record_lbh;
   uint32_t attr_count = 1;
   uint32_t stride = 3*4;/* sizeof(float)*(3 vertices) */
   uint8_t *instr;
   uint32_t *attrib_data;
   MEM_LOCK_T attrib_data_lbh;
   float *p;
   float x,y,dw,dh;
   float s[GL11_CONFIG_MAX_TEXTURE_UNITS],t[GL11_CONFIG_MAX_TEXTURE_UNITS],sw[GL11_CONFIG_MAX_TEXTURE_UNITS],sh[GL11_CONFIG_MAX_TEXTURE_UNITS];
   int merge_index;

   uint32_t color_varyings;

   GLXX_ATTRIB_T attrib[8];

   //unused gl 2.0 specific
   GL20_PROGRAM_T *program = NULL;
   GL20_HW_INDEXED_UNIFORM_T iu;
   //

   vcos_assert(IS_GL_11(state));

   rs = glxx_install_framebuffer(state, &fb, false);
   if (!rs)
      goto done;

   if(!glxx_lock_fixer_stuff(rs))
      goto fail2;

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      attrib[i].enabled = 0;
      mergeable_attribs[i] = (uint32_t)~0;
   }

   for(i = 0;i<4;i++)
   {
      num_vpm_rows_c[i] = 0;
      num_vpm_rows_v[i] = 0;
   }

   glxx_hw_get_attr_live(state, attrib);

   attrib[GL11_IX_VERTEX].enabled = 1;
   attrib[GL11_IX_VERTEX].size = 3;
   attrib[GL11_IX_VERTEX].type = GL_FLOAT;
   attrib[GL11_IX_VERTEX].pointer = 0;
   attrib[GL11_IX_COLOR].value[0] = state->copy_of_color[0];
   attrib[GL11_IX_COLOR].value[1] = state->copy_of_color[1];
   attrib[GL11_IX_COLOR].value[2] = state->copy_of_color[2];
   attrib[GL11_IX_COLOR].value[3] = state->copy_of_color[3];
   merge_index = GL11_IX_VERTEX;
   for(i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      if(state->shader.texunits[i].props.active) {
         GLXX_TEXTURE_T * texture;
         attrib[GL11_IX_TEXTURE_COORD+i].enabled = 1;
         attrib[GL11_IX_TEXTURE_COORD+i].size = 2;
         attrib[GL11_IX_TEXTURE_COORD+i].type = GL_FLOAT;
         attrib[GL11_IX_TEXTURE_COORD+i].pointer = (void *)stride;
         stride += 2*4;/* sizeof(float)*(2 texcoords) */

         texture = mem_lock(state->bound_texture[i].mh_twod, NULL);
         s[i] = ((float)texture->crop_rect.Ucr)/texture->width;
         t[i] = ((float)texture->crop_rect.Vcr)/texture->height;
         sw[i] = ((float)texture->crop_rect.Wcr)/texture->width;
         sh[i] = ((float)texture->crop_rect.Hcr)/texture->height;
         mem_unlock(state->bound_texture[i].mh_twod);

         mergeable_attribs[merge_index] = GL11_IX_TEXTURE_COORD+i;
         merge_index = GL11_IX_TEXTURE_COORD+i;
      }
   }

   calculate_and_hide(state, &fb, attrib);

   state->shader.common.primitive_type = glxx_hw_primitive_mode_to_type(GL_TRIANGLE_FAN);
   state->shader.drawtex = true;

   if (DRAW_TEX_LOGGING)
      draw_tex_log(state, &fb, rs, Xs, Ys, Zw, Ws, Hs);

   attrib_data = glxx_big_mem_alloc_junk(stride * 4, 4, &attrib_data_lbh);
   if(!attrib_data)
      goto fail;

   /* calculate vertices and tex coords for quad */
   p = (float *)attrib_data;

   x = 2.0f*Xs/state->viewport.width - 1.0f;
   y = 2.0f*Ys/state->viewport.height - 1.0f;
   dw = 2.0f*Ws/state->viewport.width;
   dh = 2.0f*Hs/state->viewport.height;

   *p++ = x;
   *p++ = y;
   *p++ = Zw;
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      if (state->shader.texunits[i].props.active) {
         *p++ = s[i];
         *p++ = t[i];
      }
   }

   *p++ = x + dw;
   *p++ = y;
   *p++ = Zw;
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      if (state->shader.texunits[i].props.active) {
         *p++ = s[i] + sw[i];
         *p++ = t[i];
      }
   }

   *p++ = x + dw;
   *p++ = y + dh;
   *p++ = Zw;
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      if (state->shader.texunits[i].props.active) {
         *p++ = s[i] + sw[i];
         *p++ = t[i] + sh[i];
      }
   }

   *p++ = x;
   *p++ = y + dh;
   *p++ = Zw;
   for (i=0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      if (state->shader.texunits[i].props.active) {
         *p++ = s[i];
         *p++ = t[i] + sh[i];
      }
   }

   vcos_assert((uint8_t *)p == (uint8_t *)attrib_data + stride*4);

   /* TODO: only allocate space for as many vertex attributes as we need */
   /* TODO: extended vertex stride? */
   shader_record = (GLXX_HW_SHADER_RECORD_T *)glxx_big_mem_alloc_junk(100, 16, &shader_record_lbh);
   if (!shader_record) goto fail;

   /* create or retrieve shaders from cache and setup attribs_live */

   cattribs_live = state->batch.cattribs_live;
   vattribs_live = state->batch.vattribs_live;
   if (!vcos_verify(get_shaders(
         program,
         shader_record,
         &cunif_map, &vunif_map, &funif_map,
         &color_varyings,
         state,
         attrib,
         mergeable_attribs,
         cattribs_order,
         vattribs_order)))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto fail;
   }

   /* Create VCD configuration */

   shader_record->attr[0].base = khrn_hw_addr(attrib_data, &attrib_data_lbh);
   shader_record->attr[0].sizem1 = stride-1;
   shader_record->attr[0].stride = stride;
   shader_record->attr[0].coffset = 0;
   shader_record->cattrsel = 1;
   num_vpm_rows_c[0] = (((stride + 3) & ~3)+3)/4;
   shader_record->attr[0].voffset = 0;
   shader_record->vattrsel = 1;
   num_vpm_rows_v[0] = (((stride + 3) & ~3)+3)/4;

   shader_record->vattrsize = (stride + 3) & ~3;
   shader_record->cattrsize = (stride + 3) & ~3;

   /* Install uniforms */
   cunif_count = mem_get_size(cunif_map)/8;
   vunif_count = mem_get_size(vunif_map)/8;
   funif_count = mem_get_size(funif_map)/8;

   if (!install_uniforms(
      &shader_record->cunif,
      state,
      program,
      cunif_count,
      cunif_map,
      &iu,
      num_vpm_rows_c,
      attrib,/*TODO for GL 2.0 NULL is passed instead of attrib - does this matter? */
      state->shader.common.egl_output,
      fb.height,
      secure))
   {
      goto fail;
   }

   if (!install_uniforms(
      &shader_record->vunif,
      state,
      program,
      vunif_count,
      vunif_map,
      &iu,
      num_vpm_rows_v,
      attrib,
      state->shader.common.egl_output,
      fb.height,
      secure))
   {
      goto fail;
   }

   if (!install_uniforms(
      &shader_record->funif,
      state,
      program,
      funif_count,
      funif_map,
      &iu,
      0,
      attrib,
      state->shader.common.egl_output,
      fb.height,
      secure))
   {
      goto fail;
   }

   /* after all uniforms are installed, loop over samplers and remove any egl image scratch
      which where created during install_uniforms */
   reset_egl_images_in_textures(state);

   /* emit any necessary config change instructions */
   bool is_empty;
   bool ok = do_changed_cfg(state, &fb, color_varyings, &is_empty);
   if(!ok) goto fail;
   if(is_empty)
   {
      glxx_unlock_fixer_stuff();
      goto done;   /* empty region - nothing to draw */
   }

   instr = glxx_big_mem_alloc_cle(20);
   if (!instr) goto fail;

   add_byte(&instr, KHRN_HW_INSTR_GL_SHADER);     //(5)
   add_pointer(&instr, (uint8_t *)shader_record + (attr_count & 7), &shader_record_lbh);

   // Emit a GLDRAWARRAYS instruction
   add_byte(&instr, KHRN_HW_INSTR_GLDRAWARRAYS);
   add_byte(&instr, convert_primitive_type(GL_TRIANGLE_FAN));  //Primitive mode
   add_word(&instr, 4);       //Length (number of vertices)
   add_word(&instr, 0);            //Index of first vertex

   add_byte(&instr, KHRN_HW_INSTR_NOP);         //Pad to the same length as KHRN_HW_INSTR_GLDRAWELEMENTS to make it easier for ourselves
   add_byte(&instr, KHRN_HW_INSTR_NOP);
   add_byte(&instr, KHRN_HW_INSTR_NOP);
   add_byte(&instr, KHRN_HW_INSTR_NOP);

   add_byte(&instr, KHRN_HW_INSTR_NOP);        //(1) TODO: is this necessary?

   add_byte(&instr, KHRN_HW_INSTR_NOP);        //(1) TODO: is this necessary?

   /* Mark the state as drawn so that clearing works properly */
   rs->drawn = true;
   rs->xxx_empty = false;

   glxx_unlock_fixer_stuff();

done:
   state->shader.drawtex = false;
   return true;

fail:
   glxx_unlock_fixer_stuff();

fail2:
   state->shader.drawtex = false;
   glxx_hw_discard_frame(rs);
   return false;
}

bool glxx_schedule_during_link(GLXX_SERVER_STATE_T *state, void *prog)
{
   uint32_t i;
   MEM_HANDLE_T cunif_map, vunif_map, funif_map;
   uint32_t mergeable_attribs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   uint32_t vattribs_order[GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2];
   uint32_t cattribs_order[GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2];
   GLXX_ATTRIB_T attrib[8];
   uint32_t color_varyings;
   GL20_PROGRAM_T *program = (GL20_PROGRAM_T*)prog;
   GLXX_HW_FRAMEBUFFER_T fb;
   GLXX_HW_RENDER_STATE_T *rs;
   bool unlockFixer = false;
   bool lockedProgramItems = false;

   assert(IS_GL_20(state));
   if (!program->linked)
      return true;

   /* if the buffer is complete, we can use the installed version to link against, which has a good chance
      of getting the correct version in the cache, otherwise use the main framebuffer to stop the link from
      failing. */
   bool is_complete = glxx_check_framebuffer_status(state, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

   rs = glxx_install_framebuffer(state, &fb, !is_complete);
   if (!rs)
      goto out_of_mem;

   if(!glxx_lock_fixer_stuff(rs))
      goto out_of_mem;

   unlockFixer = true;

   memset(attrib, 0, sizeof(attrib));

   // Set attributes to be exactly what is expected by the shader
   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      // Select each nibble in turn
      uint8_t attribMask = (program->attribs_live >> (4 * i)) & 0xF;

      // Count number of bits in mask
      uint32_t bits;
      for (bits = 0; attribMask > 0; attribMask = attribMask >> 1)
         bits += (attribMask & 1);

      attrib[i].enabled = bits > 0;
      attrib[i].size = bits;
      attrib[i].type = GL_FLOAT; // We have to guess - and this is by far the most common
      attrib[i].pointer = 0;
   }

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      mergeable_attribs[i] = (uint32_t)~0;

   state->batch.sampler_info = (GL20_SAMPLER_INFO_T *)mem_lock(program->mh_sampler_info, NULL);
   state->batch.uniform_info = (GL20_UNIFORM_INFO_T *)mem_lock(program->mh_uniform_info, NULL);
   state->batch.uniform_data = (uint32_t *)mem_lock(program->mh_uniform_data, NULL);
   state->batch.num_samplers = mem_get_size(program->mh_sampler_info) / sizeof(GL20_SAMPLER_INFO_T);
   lockedProgramItems = true;

   calculate_and_hide(state, &fb, attrib);
   state->shader.common.primitive_type = glxx_hw_primitive_mode_to_type(GL_TRIANGLES);

   if (!get_shaders(program, NULL, &cunif_map, &vunif_map, &funif_map, &color_varyings,
                    state, attrib, mergeable_attribs, cattribs_order, vattribs_order))
      goto bad_sched;

   glxx_unlock_fixer_stuff();
   return true;

bad_sched:
   glsl_link_error(ERROR_LINKER, 10, 0, NULL);  // Insufficient resources to schedule
   goto done;

out_of_mem:
   glsl_link_error(ERROR_CUSTOM, 6, 0, NULL);   // Out of memory
   goto done;

done:
   program->linked = GL_FALSE;
   if (lockedProgramItems)
   {
      mem_unlock(program->mh_sampler_info);
      mem_unlock(program->mh_uniform_info);
      mem_unlock(program->mh_uniform_data);
   }

   if (unlockFixer)
      glxx_unlock_fixer_stuff();

   {
      MEM_HANDLE_T handle = mem_strdup_ex(error_buffer, MEM_COMPACT_DISCARD);
      if (handle != MEM_INVALID_HANDLE)
      {
         MEM_ASSIGN(program->mh_info, handle);
         mem_release(handle);
      }
   }

   return false;
}
