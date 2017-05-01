/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#include "bsg_font_texture_packer.h"
#include "bsg_exception.h"

#include <memory>
#include <algorithm>
#include <stdint.h>

namespace bsg
{

class FontTextureNode
{
public:
   FontTextureNode(uint32_t x, uint32_t y, uint32_t w, uint32_t h) :
      m_rect(x, y, w, h, 0)
   {
      m_left = m_right = NULL;
   }

   ~FontTextureNode()
   {
      delete m_left;
      delete m_right;
   }

   bool AssignRectangle(FontTextureRectangle *newRect, uint32_t tex)
   {
      if (m_left != NULL && m_right != NULL)
      {
         return m_left->AssignRectangle(newRect, tex)  ||
                m_right->AssignRectangle(newRect, tex);
      }

      if (newRect->GetWidth() <= m_rect.GetWidth() && newRect->GetHeight() <= m_rect.GetHeight())
      {
         newRect->SetX(m_rect.GetX());
         newRect->SetY(m_rect.GetY());
         newRect->SetTexture(tex);

         m_left  = new FontTextureNode(m_rect.GetX(),       m_rect.GetY()      + newRect->GetHeight(),
                                       newRect->GetWidth(), m_rect.GetHeight() - newRect->GetHeight());

         m_right = new FontTextureNode(m_rect.GetX()     + newRect->GetWidth(), m_rect.GetY(),
                                       m_rect.GetWidth() - newRect->GetWidth(), m_rect.GetHeight());

         return true;
      }

      return false;
   }

private:
   FontTextureNode      *m_left;
   FontTextureNode      *m_right;
   FontTextureRectangle m_rect;
};


FontTexturePacker::~FontTexturePacker()
{
}

void FontTexturePacker::AddRectangle(unsigned int width, unsigned int height)
{
   m_rects.push_back(FontTextureRectangle(0, 0, width, height, 0));
}


bool OriginalAreaComp(FontTextureRectangle *const &elem0, FontTextureRectangle *const &elem1)
{
   return (elem1->GetWidth() * elem1->GetHeight()) < (elem0->GetWidth() * elem0->GetHeight());
}

bool WidthComp(FontTextureRectangle *const &elem0, FontTextureRectangle *const &elem1)
{
   return elem1->GetWidth() < elem0->GetWidth();
}

bool HeightComp(FontTextureRectangle *const &elem0, FontTextureRectangle *const &elem1)
{
   return elem1->GetHeight() < elem0->GetHeight();
}

uint32_t FontTexturePacker::AssignCoords(std::vector<uint32_t> &width, std::vector<uint32_t> &height, compareRectFunc /*compRectFunc*/)
{
   if (width.size() != 0 || height.size() != 0)
      BSG_THROW("Internal error");

   std::vector<FontTextureRectangle *>    sorted_rects(m_rects.size());

   uint32_t tex = 0;

   for (uint32_t i = 0; i < m_rects.size(); ++i)
      sorted_rects[i] = &m_rects[i];

   //std::sort(sorted_rects.begin(), sorted_rects.end(), compRectFunc);

   // keeps going until a power2 size is available - starts at 128
   bool     retry = true;
   uint32_t start = 0;

   width.push_back(128);
   height.push_back(128);

   while (retry)
   {
      std::auto_ptr<FontTextureNode> top(new FontTextureNode(0, 0, width[tex], height[tex]));

      bool  ok   = true;

      for (uint32_t i = start; ok && i < sorted_rects.size(); i++)
      {
         ok = top->AssignRectangle(sorted_rects[i], tex);

         retry = !ok;

         if (retry)
         {
            if (width[tex] == 2048 && height[tex] == 2048)
            {
               tex += 1;
               start = i;
               width.push_back(128);
               height.push_back(128);
            }
            else
            {
               // increase the initial size and try again
               if (width[tex] > height[tex])
                  height[tex] *= 2;
               else
                  width[tex]  *= 2;
            }
         }
      }
   }

   return tex + 1;
}

}

#endif /* BSG_STAND_ALONE */
