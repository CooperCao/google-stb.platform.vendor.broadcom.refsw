/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <list>
#include <memory>

class TextureRectangle
{
public:
   TextureRectangle(unsigned x = 0, unsigned y = 0, unsigned width = 0, unsigned height = 0) :
      m_x(x), m_y(y), m_width(width), m_height(height) {}
   ~TextureRectangle() {}

   unsigned X() const { return m_x; }
   unsigned Y() const { return m_y; }
   unsigned Width() const { return m_width; }
   unsigned Height() const { return m_height; }

   void SetX(unsigned x) { m_x = x; }
   void SetY(unsigned y) { m_y = y; }

private:
   unsigned m_x, m_y;
   unsigned m_width, m_height;
};

class TexturePacker
{
public:
   ~TexturePacker() {};

   void AddRectangle(unsigned width, unsigned height);
   bool AssignCoords(unsigned *width, unsigned *height);

   const std::list<std::shared_ptr<TextureRectangle>> &Get() const
   {
      return m_rects;
   }

protected:
   std::list<std::shared_ptr<TextureRectangle>> m_rects;
};