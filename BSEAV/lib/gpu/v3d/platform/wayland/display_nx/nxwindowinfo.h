/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <string.h>

#include "default_wayland.h"
#include "../helpers/extent.h"

#include "nexus_surface_client.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

namespace wlpl
{

class NxWindowInfo
{
public:
   NxWindowInfo() :
         m_clientID(0)
   {
   }

   NxWindowInfo(const WLPL_NexusWindowInfoEXT *info) :
         m_clientID(0),
         m_info(*info)
   {
   }

   ~NxWindowInfo() = default;

   bool Init();
   void Term();

   helper::Extent2D GetExtent2D() const
   {
      return helper::Extent2D(m_info.width, m_info.height);
   }

   void UpdateNativeWindowInfo(const WLPL_NexusWindowInfoEXT *info)
   {
      m_info = *info;
   }

   void UpdateNativeWindowInfo(WLPL_DisplayType type)
   {
      m_info.type = type;
   }

   WLPL_DisplayType GetType() const
   {
      return m_info.type;
   }

   uint32_t GetWidth() const
   {
      return m_info.width;
   }

   uint32_t GetHeight() const
   {
      return m_info.height;
   }

   uint32_t GetX() const
   {
      return m_info.x;
   }

   uint32_t GetY() const
   {
      return m_info.y;
   }

   uint32_t GetZOrder() const
   {
      return m_info.zOrder;
   }

   bool GetStretch() const
   {
      return m_info.stretch;
   }

   NEXUS_BlendEquation GetColorBlend() const
   {
      return m_info.colorBlend;
   }

   NEXUS_BlendEquation GetAlphaBlend() const
   {
      return m_info.alphaBlend;
   }

   uint32_t GetClientID() const
   {
      return m_clientID;
   }

   NEXUS_SurfaceClientHandle GetSurfaceClient() const
   {
      return m_surfaceClient;
   }

   bool operator==(const NxWindowInfo& rhs) const
   {
      return ((m_info.magic == rhs.m_info.magic) &&
              (m_info.width == rhs.m_info.width) &&
              (m_info.height == rhs.m_info.height) &&
              (m_info.x == rhs.m_info.x) &&
              (m_info.y == rhs.m_info.y) &&
              (m_info.stretch == rhs.m_info.stretch) &&
              (m_info.zOrder == rhs.m_info.zOrder) &&
              (memcmp(&m_info.colorBlend, &rhs.m_info.colorBlend,
                  sizeof(NEXUS_BlendEquation) == 0)) &&
              (memcmp(&m_info.alphaBlend, &rhs.m_info.alphaBlend,
                  sizeof(NEXUS_BlendEquation) == 0)) &&
              (m_info.type == rhs.m_info.type));
   }

   bool operator!=(const NxWindowInfo& rhs) const
   {
      return !(*this == rhs);
   }

private:
   WLPL_NexusWindowInfoEXT m_info = {};

#ifdef NXCLIENT_SUPPORT
   NxClient_AllocResults m_allocResults;
#endif
   uint32_t m_clientID;
   NEXUS_SurfaceClientHandle m_surfaceClient;
};

}
