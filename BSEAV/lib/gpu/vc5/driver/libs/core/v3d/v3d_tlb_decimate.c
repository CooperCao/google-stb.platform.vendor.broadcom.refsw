/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_tlb_decimate.h"

void v3d_tlb_sample_to_decimated_coords(
   uint32_t *x_dec, uint32_t *y_dec,
   uint32_t x_px, uint32_t y_px, uint32_t si,
   uint32_t num_samp, v3d_decimate_t decimate)
{
   switch (decimate)
   {
   case V3D_DECIMATE_SAMPLE0:
      /* When storing, we should only get si == 0 -- the other samples aren't
       * stored
       *
       * When loading, we might get si != 0, but we map all samples to the same
       * location in the decimated image anyway (to get "replicating"
       * behaviour) */
      *x_dec = x_px;
      *y_dec = y_px;
      break;
   case V3D_DECIMATE_4X:
      /* When storing, should only get called with si == 0, after 4x decimation
       * has reduced number of samples to 1 per pixel
       *
       * When loading, we might get si != 0, but as with sample0, we map all
       * samples to the same location to get replicating behaviour */
      assert(num_samp == 4);
      *x_dec = x_px;
      *y_dec = y_px;
      break;
   case V3D_DECIMATE_16X:
      not_impl();
      break;
   case V3D_DECIMATE_ALL_SAMPLES:
      assert(num_samp == 4);
      *x_dec = (x_px * 2) + (si & 1);
      *y_dec = (y_px * 2) + (si >> 1);
      break;
   default:
      unreachable();
   }
}

void v3d_tlb_pixel_from_decimated_coords(
   uint32_t *x_px, uint32_t *y_px,
   uint32_t x_dec, uint32_t y_dec,
   uint32_t num_samp, v3d_decimate_t decimate)
{
   switch (decimate)
   {
   case V3D_DECIMATE_SAMPLE0:
      *x_px = x_dec;
      *y_px = y_dec;
      break;
   case V3D_DECIMATE_4X:
      assert(num_samp == 4);
      *x_px = x_dec;
      *y_px = y_dec;
      break;
   case V3D_DECIMATE_16X:
      not_impl();
      break;
   case V3D_DECIMATE_ALL_SAMPLES:
      assert(num_samp == 4);
      *x_px = x_dec / 2;
      *y_px = y_dec / 2;
      break;
   default:
      unreachable();
   }
}
