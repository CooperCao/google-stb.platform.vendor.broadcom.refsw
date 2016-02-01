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
