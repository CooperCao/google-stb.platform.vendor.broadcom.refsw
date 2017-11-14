/******************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************/
#pragma once

namespace helper
{

class Extent2D
{
public:
   Extent2D(int width = 0, int height = 0) :
      m_width(width),
      m_height(height)
   {}
   ~Extent2D() {}

   int GetWidth() const
   {
      return m_width;
   };

   int GetHeight() const
   {
      return m_height;
   }

   bool operator==(const Extent2D& rhs) const
   {
      return ((m_width == rhs.m_width) && (m_height == rhs.m_height));
   }

   bool operator!=(const Extent2D& rhs) const
   {
      return !(*this == rhs);
   }

private:
   int m_width;
   int m_height;
};

}