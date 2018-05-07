/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

class WrappedBitmap
{
public:
   WrappedBitmap(IDirectFBSurface *surface) :
      m_surface(surface) {}
   ~WrappedBitmap() = default;

   IDirectFBSurface *GetSurface() const
   {
      return m_surface;
   }

protected:
   IDirectFBSurface           *m_surface;
};
