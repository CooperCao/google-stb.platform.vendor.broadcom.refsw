/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bitmap.h"

#include <stdio.h>

namespace dbpl
{

Bitmap::Bitmap(void *context, BEGL_PixmapInfoEXT const *bufferRequirements) :
   WrappedBitmap(context, NULL)
{
   DFBSurfaceDescription desc = {};
   desc.flags = static_cast<DFBSurfaceDescriptionFlags>(DSDESC_CAPS |
                                                        DSDESC_WIDTH |
                                                        DSDESC_HEIGHT |
                                                        DSDESC_PIXELFORMAT);
   desc.caps = DSCAPS_GL;
   desc.width = bufferRequirements->width;
   desc.height = bufferRequirements->height;
   BeglToDfbFormat(bufferRequirements->format, &desc.pixelformat);

   DFBCHECK( m_display->dfb->CreateSurface( m_display->dfb, &desc, &m_surface ));

   // lazy surface creation, make sure m_settings are updated
   Lock();
   Unlock();
}

Bitmap::~Bitmap()
{
   DFBCHECK( m_surface->Release( m_surface ));
}

}
