/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
