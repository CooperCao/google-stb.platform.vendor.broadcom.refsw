/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#ifndef __BSG_FONT_H__
#define __BSG_FONT_H__

#include <string>
#include <vector>
#include <stdint.h>

#include "bsg_common.h"
#include "bsg_gl_texture.h"
#include "bsg_vector.h"
#include "bsg_material.h"

namespace bsg
{

class Font;

// @cond
struct FontTraits
{
   typedef Font         Base;
   typedef Font         Derived;
   typedef FontTraits   BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<FontTraits>    FontHandle;
//! @}

typedef std::vector<FontHandle>  Fonts;

namespace prv
{

class CharAdvance
{
public:
   float    m_advanceX;
   uint32_t m_unused;  // m_advanceY
   int32_t m_middleX;
   int32_t m_middleY;
};

}

class FontMemInfo;

//! The Font class is used to manage a single font instantiation, at a given point size.
class Font : public RefCount
{
   friend class Handle<FontTraits>;

public:
   virtual ~Font();

   //! Set the font size in pixels
   void SetFontInPixels(const std::string &fontName, float pointSizePixels, bool usePoints = true);
   void SetFontInPixels(const FontMemInfo &fontInfo, float pointSizePixels, bool usePoints = true);

   //! Set the font size as a percentage of the screenHeight.
   //! Useful when the display size is liable to change.
   void SetFontInPercent(const std::string &fontName, float pointSizePercent, uint32_t screenHeight, bool usePoints = true);
   void SetFontInPercent(const FontMemInfo &fontInfo, float pointSizePercent, uint32_t screenHeight, bool usePoints = true);

   //! Render some text
   void DrawTextString(const std::string &str, float xpos, float ypos, const Vec4 &color);

   //! Returns the font name
   const std::string &FontName() const { return m_fontName; }

   //! Returns the font size in pixels
   float PointSizePixels() const { return m_pointSize; }

   //! Return a texture handle for the packed font map which can be used for rendering
   TextureHandle GetTexture();

   //! Returns a string containing the Effect text used for rendering text.
   //! Avoids always needing a .bfx file in every app that uses fonts.
   const char *EffectString() const;

   //! Return the distance between lines using this font.
   float LineHeight() const { return m_lineHeight; }

protected:
   Font();

private:
   //! Returns a specially formatted array of 2D points representing the string to be rendered.
   //! This is only useful for the BSG font rendering algorithm.
   void MakeStringPointArray(std::string &str, float x, float y, int32_t irx, int32_t iry, std::vector<Vec2> *arr);
   void MakeStringQuadArray (std::string &str, float x, float y, int32_t irx, int32_t iry, std::vector<Vec2> *arr);
   void SetFont(const std::string &fontName, const unsigned char *fontInMemory, uint32_t size, float pointSize, bool usePoints);

   void Reset();

private:
   std::string       m_fontName;
   bool              m_usePoints;
   float             m_pointSize;

   uint8_t           *m_data;
   uint32_t          m_texWidth;
   uint32_t          m_texHeight;
   float             m_lineHeight;
   Vec4              *m_charMap;
   float             *m_charSize;
   prv::CharAdvance  *m_charAdvance;
   void              *m_library;
   void              *m_face;

   // GL texture for the font (lazily filled)
   TextureHandle     m_texture;
   EffectHandle      m_effect;
   MaterialHandle    m_material;
};


}

#endif /* __BSG_FONT_H__ */

#endif /* BSG_STAND_ALONE */
