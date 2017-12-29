/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <atomic>

#include <stdio.h>

#include "default_nexus.h"
#include "../helpers/extent.h"

namespace nxpl
{

class NativeWindowInfo
{
public:
   NativeWindowInfo() : m_info() {};
   NativeWindowInfo(const NXPL_NativeWindowInfoEXT *info) :
      m_info(*info)
   {};
   ~NativeWindowInfo() {};

   bool Init()
   {
      bool res = true;
      if (m_windowCount.fetch_add(1) != 0)
      {
         m_windowCount--;
         res = false;
      }
      return res;
   }

   void Term()
   {
      m_windowCount--;
   }

   helper::Extent2D GetExtent2D() const
   {
      return helper::Extent2D(m_info.width, m_info.height);
   }

   void UpdateNativeWindowInfo(const NXPL_NativeWindowInfoEXT *info)
   {
      m_info = *info;
   }

   void UpdateNativeWindowInfo(const NXPL_NativeWindowInfo *info)
   {
      m_info.width = info->width;
      m_info.height = info->height;
      m_info.x = info->x;
      m_info.y = info->y;
      m_info.stretch = info->stretch;
      m_info.clientID = info->clientID;
      m_info.zOrder = info->zOrder;
   }

   void UpdateNativeWindowInfo(NXPL_DisplayType type)
   {
      m_info.type = type;
   }

   NXPL_DisplayType GetType() const
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

   bool GetStretch() const
   {
      return m_info.stretch;
   }

   uint32_t GetClientID() const
   {
      return 0;
   }

   bool operator==(const NativeWindowInfo& rhs) const
   {
      return ((m_info.magic == rhs.m_info.magic) &&
              (m_info.width == rhs.m_info.width) &&
              (m_info.height == rhs.m_info.height) &&
              (m_info.x == rhs.m_info.x) &&
              (m_info.y == rhs.m_info.y) &&
              (m_info.stretch == rhs.m_info.stretch) &&
              (m_info.type == rhs.m_info.type));
   }

   bool operator!=(const NativeWindowInfo& rhs) const
   {
      return !(*this == rhs);
   }

private:
   static std::atomic<int> m_windowCount;

   NXPL_NativeWindowInfoEXT m_info;
};

}
