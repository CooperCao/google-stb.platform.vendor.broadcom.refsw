/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "directfb_helpers.h"

bool DfbToBeglFormat(DFBSurfacePixelFormat dfb_format, BEGL_BufferFormat *result)
{
   bool  ok = true;

   switch (dfb_format)
   {
   case DSPF_ABGR:                  *result = BEGL_BufferFormat_eA8B8G8R8;           break;
   case DSPF_RGB32:                 *result = BEGL_BufferFormat_eX8B8G8R8;           break;
   case DSPF_RGB16:                 *result = BEGL_BufferFormat_eR5G6B5;             break;
   default:                         *result = BEGL_BufferFormat_INVALID; ok = false; break;
   }

   return ok;
}

bool BeglToDfbFormat(BEGL_BufferFormat format, DFBSurfacePixelFormat *dfb_format)
{
   bool  ok = true;

   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8 : *dfb_format = DSPF_ABGR;       break;
   case BEGL_BufferFormat_eX8B8G8R8 : *dfb_format = DSPF_RGB32;      break;
   case BEGL_BufferFormat_eR5G6B5   : *dfb_format = DSPF_RGB16;      break;
   default:                            ok = false;                   break;
   }

   return ok;
}
