/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_server.h"
#include "glxx_ds_to_color.h"

typedef struct
{
   GFX_LFMT_T ds_fmt, color_fmt;
   unsigned stencil_mask;
   unsigned depth_mask;
} DS_COLOR_MAP_T;

static const DS_COLOR_MAP_T mapping[] =
{
   {GFX_LFMT_S8D24_UINT_UNORM,  GFX_LFMT_R32_UINT, 0x000000ff , 0xffffff00},
   {GFX_LFMT_D24X8_UNORM,       GFX_LFMT_R32_UINT, 0          , 0xffffffff},
   {GFX_LFMT_D32_FLOAT,         GFX_LFMT_R32_UINT, 0          , 0xffffffff},
   {GFX_LFMT_D16_UNORM,         GFX_LFMT_R16_UINT, 0          , 0xffffffff},
   {GFX_LFMT_S8_UINT,           GFX_LFMT_R8_UINT , 0xffffffff , 0},
};

static const DS_COLOR_MAP_T* ds_lfmt_to_mapping(GFX_LFMT_T lfmt)
{
   unsigned num_mappings = sizeof(mapping)/sizeof(mapping[0]);
   for (unsigned i =0; i < num_mappings; i++)
   {
      if(mapping[i].ds_fmt == gfx_lfmt_fmt(lfmt))
         return &mapping[i];
   }

   return NULL;
}

GFX_LFMT_T glxx_ds_lfmt_to_color(GFX_LFMT_T ds_lfmt)
{
   const DS_COLOR_MAP_T* col_map = ds_lfmt_to_mapping(ds_lfmt);
   assert(col_map);

   return gfx_lfmt_set_format(ds_lfmt, col_map->color_fmt);
}

unsigned glxx_ds_color_lfmt_get_stencil_mask(GFX_LFMT_T ds_lfmt)
{
   const DS_COLOR_MAP_T* col_map = ds_lfmt_to_mapping(ds_lfmt);
   assert(col_map);

   return col_map->stencil_mask;
}

unsigned glxx_ds_color_lfmt_get_depth_mask(GFX_LFMT_T ds_lfmt)
{
   const DS_COLOR_MAP_T* col_map = ds_lfmt_to_mapping(ds_lfmt);
   assert(col_map);

   return col_map->depth_mask;
}
