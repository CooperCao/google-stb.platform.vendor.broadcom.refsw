/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_TLB_DECIMATE_H
#define V3D_TLB_DECIMATE_H

#include "v3d_gen.h"

/* Where does sample si of pixel (x_px, y_px) map to in the decimated image? */
extern void v3d_tlb_sample_to_decimated_coords(
   uint32_t *x_dec, uint32_t *y_dec,
   uint32_t x_px, uint32_t y_px, uint32_t si,
   uint32_t num_samp, v3d_decimate_t decimate);

/* Where does pixel (x_px, y_px) map to in the decimated image? If the pixel
 * covers multiple elements in the decimated image, the element with the
 * smallest (x, y) coord is returned.
 *
 * Note that this function can be used to find out the size of the decimated
 * image given the size in pixels:
 *
 * v3d_tlb_pixel_to_decimated_coords(
 *    &width_dec, &height_dec,
 *    width_px, height_px,
 *    num_samp, decimate)
 */
static inline void v3d_tlb_pixel_to_decimated_coords(
   uint32_t *x_dec, uint32_t *y_dec,
   uint32_t x_px, uint32_t y_px,
   uint32_t num_samp, v3d_decimate_t decimate)
{
   /* We assume the 0th sample always has the smallest (x, y) coord... */
   v3d_tlb_sample_to_decimated_coords(x_dec, y_dec, x_px, y_px, /*si=*/0, num_samp, decimate);
}

extern void v3d_tlb_pixel_from_decimated_coords(
   uint32_t *x_px, uint32_t *y_px,
   uint32_t x_dec, uint32_t y_dec,
   uint32_t num_samp, v3d_decimate_t decimate);

#endif
