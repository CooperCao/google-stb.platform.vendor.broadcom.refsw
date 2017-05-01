/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_DS_TO_COLOR_H
#define GLXX_DS_TO_COLOR_H

GFX_LFMT_T glxx_ds_lfmt_to_color(GFX_LFMT_T ds_lfmt);
unsigned glxx_ds_color_lfmt_get_stencil_mask(GFX_LFMT_T lfmt);
unsigned glxx_ds_color_lfmt_get_depth_mask(GFX_LFMT_T lfmt);

#endif
