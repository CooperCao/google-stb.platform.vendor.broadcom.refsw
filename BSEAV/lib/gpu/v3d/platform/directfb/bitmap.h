/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <directfb_version.h>
#include <directfb.h>

#include "nexus_base_mmap.h"
#include "EGL/egl.h"
#include "default_directfb.h"
#include "display_priv.h"
#include "../helpers/extent.h"
#include "dfbcheck.h"

static bool DfbToBeglFormat(DFBSurfacePixelFormat dfb_format, BEGL_BufferFormat *result)
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

static bool BeglToDfbFormat(BEGL_BufferFormat format, DFBSurfacePixelFormat *dfb_format)
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

namespace dbpl
{

class WrappedBitmap
{
public:
   WrappedBitmap(void *context, IDirectFBSurface *surface) :
      m_settings(),
      m_display(static_cast<DBPL_Display*>(context)),
      m_surface(surface) {}
   ~WrappedBitmap() = default;

   IDirectFBSurface *GetSurface() const
   {
      return m_surface;
   }

   const BEGL_BufferSettings & GetCreateSettings() const
   {
      return m_settings;
   };

   void Lock()
   {
      void *addr;
      int pitch;

      DFBCHECK( m_surface->Lock( m_surface, DSLF_WRITE, &addr, &pitch));
      m_settings.cachedAddr = addr;
      m_settings.pitchBytes = pitch;
      m_settings.physOffset = NEXUS_AddrToOffset(m_settings.cachedAddr);
      DFBCHECK( m_surface->GetSize ( m_surface, reinterpret_cast<int *>(&m_settings.width), reinterpret_cast<int *>(&m_settings.height)));
      DFBSurfacePixelFormat dfb_format;
      DFBCHECK( m_surface->GetPixelFormat ( m_surface, &dfb_format));
      DfbToBeglFormat(dfb_format, &m_settings.format);
   }

   void Unlock()
   {
      DFBCHECK( m_surface->Unlock( m_surface ));
   }

protected:
   BEGL_BufferSettings         m_settings;
   DBPL_Display               *m_display;
   IDirectFBSurface           *m_surface;
};

class Bitmap : public WrappedBitmap
{
public:
   Bitmap(void *context, BEGL_PixmapInfoEXT const *bufferRequirements);
   ~Bitmap();
};

}
