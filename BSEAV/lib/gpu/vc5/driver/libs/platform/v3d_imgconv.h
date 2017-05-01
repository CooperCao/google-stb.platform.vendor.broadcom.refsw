/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_IMGCONV_H
#define V3D_IMGCONV_H
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "gmem.h"
#include "v3d_scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A container to hold the information about a target for an image
 * conversion. This may be the source or destination image.
 *
 *  desc          - The buffer description.
 *  x, y, z       - The first pixel to copy.
 *  start_elem    - The first element in an image array to copy.
 *  array_pitch   - The pitch between to elements in bytes.
 *  plane_size    - size of each plane in the desc. This is calculated
 *                  in the init function.
 */
typedef struct v3d_imgconv_base_tgt
{
   GFX_BUFFER_DESC_T desc;
   unsigned int x;
   unsigned int y;
   unsigned int z;
   unsigned int start_elem;
   unsigned int array_pitch;

   unsigned int plane_sizes[GFX_BUFFER_MAX_PLANES];
} v3d_imgconv_base_tgt;


/*
 * The container for a gmem backed imgconv target.
 *
 *  base          - The base container.
 *  handle        - The gmem handle.
 *  offset        - Offset into the gmem block where the image begins.
 *  deps          - The buffers dependencies to wait on before
 *                  performing the conversion.
 */
typedef struct v3d_imgconv_gmem_tgt
{
   struct v3d_imgconv_base_tgt base;
   gmem_handle_t handle;
   size_t offset;
   v3d_scheduler_deps deps;
} v3d_imgconv_gmem_tgt;

void v3d_imgconv_init_gmem_tgt(struct v3d_imgconv_gmem_tgt *tgt,
      gmem_handle_t handle, size_t offset,
      const v3d_scheduler_deps *deps, const GFX_BUFFER_DESC_T *desc,
      unsigned int x, unsigned int y, unsigned int z, unsigned int start_elem,
      unsigned int array_pitch);

/*
 * The container for a gmem backed imgconv target.
 *
 *  base          - The base container.
 *  data          - The image data.
 */
typedef struct v3d_imgconv_ptr_tgt
{
   struct v3d_imgconv_base_tgt base;
   void *data;
} v3d_imgconv_ptr_tgt;

void v3d_imgconv_init_ptr_tgt(struct v3d_imgconv_ptr_tgt *tgt, void *ptr,
      const GFX_BUFFER_DESC_T *desc, unsigned int x, unsigned int y,
      unsigned int z, unsigned int start_elem, unsigned int array_pitch);

/*
 * Copies one image to another. It may also perform an image conversion.
 * The method it used to perform this depend on the source and destination
 * images, and what hardware the conversion is running on.
 *
 * Params:
 *  dst             - The image to copy to.
 *  src             - The image to copy from.
 *  job_id          - see below
 *  width           - The width of the data to copy.
 *  height          - The height of the data to copy.
 *  depth           - The depth of the data to copy (number of slices).
 *  num_elems       - How many elements in an image array to copy.
 *
 *  We return:
 *  true and  *job_id == 0, if the copy/conversions were done synchronous;
 *                          It also means that we've waited for the src and dst dependencies.
 *  true and *job_id != 0, if we've succeeded submiting all the asynchonous copy/conversions jobs.
 *                The caller needs to wait on this job_id for those converisons to complete.
 *  false and *job_id != 0, if we've succeeded submitting some asynchronous copy/conversions and then failed.
 *                The caller needs to wait on this job_id for those conversion to complete.
 *  false and  *job_id == 0,  we failed doing any copy/conversions.
 */
extern bool v3d_imgconv_convert(
      const struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      uint64_t *job_id,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems,
      bool secure_context);

/*
 * Copies one image to another. It will NOT perform an image conversion.
 * Source and destination must have the same number of planes
 * and the same bit depth. Each pixel (or block in case of compressed format)
 * will be treated as opaque data of the relevant size.
 *
 * Params:
 *  src_width           - The width of the data to copy (in src pixels).
 *  src_height          - The height of the data to copy (in src pixels).
 * The destination width and height may be different if only one of the images
 * is compressed.
 *
 * Other parameters and return value are identical to v3d_imgconv_convert().
 */
extern bool v3d_imgconv_memcpy(
      const struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      uint64_t *job_id,
      unsigned int src_width,
      unsigned int src_height,
      unsigned int depth,
      unsigned int num_elems,
      bool secure_context);

/*
 * This is similar to v3d_imgconv_convert. The difference is it takes
 * data from a pointer to user supplied data. Because of this we guarantee
 * the data pointed to by src will not be used after this function returns.
 */
bool v3d_imgconv_convert_from_ptr(
      const struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_ptr_tgt *src,
      uint64_t *job_id,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems,
      bool secure_context);

/*
 * This is similar to v3d_imgconv_convert. The difference is it writes image data
 * to a user supplied pointer and the conversion si always done immediately.
 */
bool v3d_imgconv_convert_to_ptr(
      const struct v3d_imgconv_ptr_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems,
      bool secure_context);

#ifdef __cplusplus
}
#endif

#endif
