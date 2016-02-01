/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.  
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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
      m_left = m_right = nullptr;
   }

   ~FontTextureNode()
   {
      delete m_left;
      delete m_right;
   }

   bool AssignRectangle(FontTextureRectangle *newRect, uint32_t tex)
   {
      if (m_left != nullptr && m_right != nullptr)
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
      std::unique_ptr<FontTextureNode> top(new FontTextureNode(0, 0, width[tex], height[tex]));

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
