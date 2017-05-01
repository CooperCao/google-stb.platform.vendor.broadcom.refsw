/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef _GLXX_PIXEL_STORE_H
#define _GLXX_PIXEL_STORE_H

typedef struct {
   int32_t  alignment;
   uint32_t row_length;
   uint32_t skip_rows;
   uint32_t skip_pixels;
} GLXX_PIXEL_PACK_STATE_T;

typedef struct {
   GLXX_PIXEL_PACK_STATE_T pack;
   GLXX_PIXEL_PACK_STATE_T unpack;
   uint32_t unpack_image_height;
   uint32_t unpack_skip_images;
} GLXX_PIXEL_STORE_STATE_T;

#endif /* _GLXX_PIXEL_STORE_H*/
