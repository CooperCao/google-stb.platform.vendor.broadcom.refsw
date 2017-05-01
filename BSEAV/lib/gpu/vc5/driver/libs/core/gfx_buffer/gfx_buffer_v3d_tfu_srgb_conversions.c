/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_buffer_v3d_tfu_srgb_conversions.h"

#include "vcos.h"

uint32_t gfx_buffer_srgb8_to_tfu13(uint32_t srgb)
{
   uint32_t i13 = 0;

   switch (srgb)
   {
#include "gfx_buffer_v3d_tfu_lut_srgbtotfu13.inc"
   default: unreachable();
   }

   return i13;
}

uint32_t gfx_buffer_tfu13_to_srgb8(uint32_t tfu13)
{
   uint32_t srgb = 0;

   if (tfu13 < 0x40)
   {
      switch (tfu13 & 0x3f)
      {
#include "gfx_buffer_v3d_tfu_lut_tfu13tosrgb0.inc"
      default: unreachable();
      }
   }
   else if (tfu13 < 0x100)
   {
      switch ((tfu13 >> 1) & 0x7f)
      {
#include "gfx_buffer_v3d_tfu_lut_tfu13tosrgb1.inc"
      default: unreachable();
      }
   }
   else if (tfu13 < 0x400)
   {
      switch ((tfu13 >> 2) & 0xff)
      {
#include "gfx_buffer_v3d_tfu_lut_tfu13tosrgb2.inc"
      default: unreachable();
      }
   }
   else if (tfu13 < 0x800)
   {
      switch ((tfu13 >> 3) & 0xff)
      {
#include "gfx_buffer_v3d_tfu_lut_tfu13tosrgb3.inc"
      default: unreachable();
      }
   }
   else
   {
      switch ((tfu13 >> 4) & 0x1ff)
      {
#include "gfx_buffer_v3d_tfu_lut_tfu13tosrgb4.inc"
      default: unreachable();
      }
   }

   return srgb;
}
