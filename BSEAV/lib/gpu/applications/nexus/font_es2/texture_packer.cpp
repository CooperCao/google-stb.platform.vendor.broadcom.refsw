/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "texture_packer.h"

#include <algorithm>
#include <memory>

using namespace std;

class TextureNode
{
public:
   TextureNode(unsigned int x = 0, unsigned int y = 0, unsigned int w = 0, unsigned int h = 0)
   {
      m_rect =  shared_ptr<TextureRectangle>(new TextureRectangle(x, y, w, h));
   }
   ~TextureNode() {}

   bool assignRectangle(shared_ptr<TextureRectangle> rect);

private:
   unique_ptr<TextureNode> m_left;
   unique_ptr<TextureNode> m_right;
   shared_ptr<TextureRectangle> m_rect;
};

bool TextureNode::assignRectangle(shared_ptr<TextureRectangle> newRect)
{
   if (!m_rect)
   {
      if (m_left->assignRectangle(newRect))
         return true;
      return m_right->assignRectangle(newRect);
   }
   else
   {
      if (newRect->Width() <= m_rect->Width() && newRect->Height() <= m_rect->Height())
      {
         newRect->SetX(m_rect->X());
         newRect->SetY(m_rect->Y());

         m_left = unique_ptr<TextureNode>(new TextureNode(m_rect->X(), m_rect->Y() + newRect->Height(), newRect->Width(), m_rect->Height() - newRect->Height()));
         m_right = unique_ptr<TextureNode>(new TextureNode(m_rect->X() + newRect->Width(), m_rect->Y(), m_rect->Width() - newRect->Width(), m_rect->Height()));

         m_rect.reset();
         return true;
      }
      return false;
   }
}

void TexturePacker::AddRectangle(unsigned width, unsigned height)
{
   m_rects.emplace_back(new TextureRectangle(0, 0, width, height));
}

bool TexturePacker::AssignCoords(unsigned *width, unsigned *height)
{
   list<shared_ptr<TextureRectangle>> sortedrects = m_rects;
   sortedrects.sort(
      [](const shared_ptr<TextureRectangle> &lhs, const shared_ptr<TextureRectangle> &rhs) {
      return (rhs->Width() * rhs->Height()) < (lhs->Width() * lhs->Height());
   });
   unsigned int size_power2 = 128;

retry:
   unique_ptr<TextureNode> top(new TextureNode(0, 0, size_power2, size_power2));

   // range based for not supported in GCC4.5
   for (list<shared_ptr<TextureRectangle>>::const_iterator it = sortedrects.begin(); it != sortedrects.end(); ++it)
   {
      if (!top->assignRectangle(*it))
      {
         // increase the initial size and try again
         size_power2 <<= 1;
         goto retry;
      }
   }

   *width = size_power2;
   *height = size_power2;

   return true;
}
