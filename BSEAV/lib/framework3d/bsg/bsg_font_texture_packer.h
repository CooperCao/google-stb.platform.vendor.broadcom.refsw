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

#ifndef __BSG_FONT_TEXTURE_PACKER_H__
#define __BSG_FONT_TEXTURE_PACKER_H__

#include "bsg_common.h"

#include <vector>
#include <stdint.h>

namespace bsg
{

// @cond
// TODO consider creating a bsg rectangle type for this sort of thing
class FontTextureRectangle 
{
public:
   FontTextureRectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t texture) :
      m_x(x),
      m_y(y),
      m_width(width),
      m_height(height),
      m_texture(texture)
   {}

   uint32_t GetX()       const { return m_x;       }
   uint32_t GetY()       const { return m_y;       }
   uint32_t GetWidth()   const { return m_width;   }
   uint32_t GetHeight()  const { return m_height;  }
   uint32_t GetTexture() const { return m_texture; }

   void SetX(uint32_t v)       { m_x       = v;    }
   void SetY(uint32_t v)       { m_y       = v;    }
   void SetWidth(uint32_t v)   { m_width   = v;    }
   void SetHeight(uint32_t v)  { m_height  = v;    }
   void SetTexture(uint32_t v) { m_texture = v;    }

private:
   uint32_t m_x;
   uint32_t m_y;
   uint32_t m_width;
   uint32_t m_height;
   uint32_t m_texture;
};

typedef bool (*compareRectFunc)(FontTextureRectangle *const &elem0, FontTextureRectangle *const &elem1);

bool OriginalAreaComp(FontTextureRectangle *const &elem0, FontTextureRectangle *const &elem1);
bool WidthComp(FontTextureRectangle *const &elem0, FontTextureRectangle *const &elem1);
bool HeightComp(FontTextureRectangle *const &elem0, FontTextureRectangle *const &elem1);

class FontTexturePacker {
public:
   ~FontTexturePacker();

   void     AddRectangle(uint32_t width, uint32_t height);
   uint32_t AssignCoords(std::vector<uint32_t> &width, std::vector<uint32_t> &height, compareRectFunc compRectFunc = OriginalAreaComp);

   const FontTextureRectangle &GetRectangle(uint32_t index) const { return m_rects[index]; }

protected:
   std::vector<FontTextureRectangle> m_rects;
};
// @endcond
}

#endif /* __BSG_FONT_TEXTURE_PACKER_H__ */

#endif /* BSG_STAND_ALONE */
