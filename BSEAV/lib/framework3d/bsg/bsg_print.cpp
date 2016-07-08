/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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

#include <algorithm>

#include "bsg_print.h"
#include "bsg_application.h"
#include "bsg_font_texture_packer.h"
#include "bsg_exception.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#undef max

using namespace bsg;
using namespace std;

// Used to default position args to (0,0) when its a reference
Vec2 Print::s_defaultPosition;

static const char *s_defaultEffectText =
   "OPTIONS                                                     \n"
   "{                                                           \n"
   "   SortOrder = BACK_TO_FRONT;                               \n"
   "}                                                           \n"
   "                                                            \n"
   "PASS 0                                                      \n"
   "{                                                           \n"
   "   SEMANTICS                                                \n"
   "   {                                                        \n"
   "      a_position    = VATTR_POSITION;                       \n"
   "      a_tc          = VATTR_TEXCOORD1;                      \n"
   "      u_textColor   = VECTOR4_USER;                         \n"
   "      u_mvp         = MATRIX4_MODEL_VIEW_PROJECTION;        \n"
   "   }                                                        \n"
   "                                                            \n"
   "   STATE                                                    \n"
   "   {                                                        \n"
   "      EnableBlend = true;                                   \n"
   "      BlendFunc = SRC_ALPHA, ONE_MINUS_SRC_ALPHA;           \n"
   "      EnableDepthTest = true;                               \n"
   "   }                                                        \n"
   "                                                            \n"
   "   SAMPLER_2D u_textSampler                                 \n"
   "   {                                                        \n"
   "      Unit = 0;                                             \n"
   "      Wrap = CLAMP, CLAMP;                                  \n"
   "      Filter = LINEAR_MIPMAP_LINEAR, LINEAR;                \n"
   "   }                                                        \n"
   "                                                            \n"
   "   VERTEX_SHADER                                            \n"
   "   {                                                        \n"
   "      attribute  vec4  a_position;                          \n"
   "      attribute  vec2  a_tc;                                \n"
   "      varying    vec2  v_tc;                                \n"
   "      uniform    mat4  u_mvp;                               \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         gl_Position = u_mvp * a_position;                  \n"
   "         v_tc        = a_tc;                                \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "                                                            \n"
   "   FRAGMENT_SHADER                                          \n"
   "   {                                                        \n"
   "      precision mediump float;                              \n"
   "                                                            \n"
   "      uniform sampler2D   u_textSampler;                    \n"
   "      uniform vec4        u_textColor;                      \n"
   "      varying vec2        v_tc;                             \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         float alpha = texture2D(u_textSampler, v_tc).r;    \n"
   "         if (alpha < 0.01)                                  \n"
   "            discard;                                        \n"
   "         alpha *= u_textColor.a;                            \n"
   "         gl_FragColor = vec4(u_textColor.rgb, alpha);       \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "}                                                           \n";

static EffectHandle  s_defaultEffect;

class GlyphMapCallback : public DrawCallback
{
public:
   GlyphMapCallback(TextureHandle tex, MaterialHandle &mat) :
      m_material(mat),
      m_texture(tex)
   {
   }

   virtual bool OnDraw()
   {
      m_material->SetTexture("u_textSampler", m_texture);
      return true;
   }

private:
   MaterialHandle    m_material;
   TextureHandle     m_texture;
};

const PrintOptions PrintOptions::m_default;

Vec2 PrintOptions::AnchorOffset(const Vec2 &mn, const Vec2 &mx) const
{
   Vec2  res;

   switch (m_anchor)
   {
   case eTOP_LEFT:
      res = Vec2(mn.X(), mx.Y());
      break;

   case eBOTTOM_LEFT:
      res = Vec2(mn.X(), mn.Y());
      break;

   case eTOP_RIGHT:
      res = Vec2(mx.X(), mx.Y());
      break;

   case eBOTTOM_RIGHT:
      res = Vec2(mx.X(), mn.Y());
      break;

   case eCENTER:
      res = (mn + mx) / 2.0f;
      break;

   case eTOP_CENTER:
      res = Vec2((mn.X() + mx.X()) / 2.0f, mx.Y());
      break;

   case eBOTTOM_CENTER:
      res = Vec2((mn.X() + mx.X()) / 2.0f, mn.Y());
      break;

   case eCENTER_LEFT:
      res = Vec2(mn.X(), (mn.Y() + mx.Y()) / 2.0f);
      break;

   case eCENTER_RIGHT:
      res = Vec2(mx.X(), (mn.Y() + mx.Y()) / 2.0f);
      break;
   }

   return res;
}

struct Character
{
   Character(uint32_t ix);
   Character(FT_GlyphSlot slot, uint32_t ix);

   std::vector<uint8_t> m_bitmap;
   uint32_t             m_width;
   uint32_t             m_height;
   uint32_t             m_pitch;
   int32_t              m_advanceX;
   int32_t              m_advanceY;
   int32_t              m_bitmapLeft;
   int32_t              m_bitmapTop;
   uint32_t             m_character;
};

Character::Character(uint32_t ix)
{
   m_width = 0;
   m_height = 0;
   m_pitch = 0;

   m_bitmap.resize(1);

   m_advanceX = 0;
   m_advanceY = 0;
   m_bitmapLeft = 0;
   m_bitmapTop = 0;
   m_character = ix;
}

Character::Character(FT_GlyphSlot slot, uint32_t ix)
{
   m_width      = slot->bitmap.width;
   m_height     = slot->bitmap.rows;
   m_pitch      = slot->bitmap.width;

   m_bitmap.resize(m_pitch * m_height + 1);

   m_advanceX   = slot->advance.x;
   m_advanceY   = slot->advance.y;
   m_bitmapLeft = slot->bitmap_left;
   m_bitmapTop  = slot->bitmap_top;
   m_character  = ix;

   memcpy(&m_bitmap[0], slot->bitmap.buffer, m_pitch * m_height);
}

static uint32_t FloatToFixed(float f)
{
   return (uint32_t)(f * 64.0f + 0.5f);
}

static float FixedToFloat(uint32_t i)
{
   return (float)i / 64.0f;
}

Vec2 PrintFont::GetKerning(uint32_t prev, uint32_t next)
{
   FT_Vector theKerning;
   theKerning.x = 0;
   theKerning.y = 0;

   if (prev != 0 && next != 0)
      FT_Get_Kerning((FT_Face)m_face, prev, next, FT_KERNING_DEFAULT, &theKerning);

   return Vec2(FixedToFloat(theKerning.x), FixedToFloat(theKerning.y));
}

void PrintFont::Load(const string &name, float pointSize)
{
   vector<Character>   charSet;

   // By default we will enable the ascii rangle of chars
   UseCharRange(0x20, 0x7f);

   if (m_face)
      BSG_THROW("Font cannot be reloaded");

   if (FT_Init_FreeType((FT_Library *)&m_library))
      BSG_THROW("Failed to initialize freetype");

   std::string resourceName = Application::Instance()->FindResource(name);

   if (FT_New_Face((FT_Library)m_library, resourceName.c_str(), 0, (FT_Face *)&m_face))
      BSG_THROW("Failed to open font " << name);

   FT_Face  face = (FT_Face)m_face;

   FT_Set_Char_Size(face, FloatToFixed(pointSize), 0, 0, 0);

   m_lineHeight = FixedToFloat(face->size->metrics.height);

   // Read all the used glyphs
   for (uint32_t r = 0; r < m_range.size(); ++r)
   {
      const std::pair<uint32_t, uint32_t> &range = m_range[r];

      for (uint32_t c = range.first; c <= range.second; ++c)
      {
         if (FT_Load_Char(face, c, FT_LOAD_RENDER) || face == 0)
            BSG_THROW("Failed to load freetype character");

         charSet.push_back(Character(face->glyph, c));
      }
   }

   FontTexturePacker tp;

   // populate the structure with all the textures
   for (vector<Character>::iterator it = charSet.begin(); it != charSet.end(); ++it)
   {
      // round to even number as we divide by two in places
      uint32_t wh = (std::max((*it).m_width, (*it).m_height) + 1) & ~0x1;
      // these are all square, so use width for the height
      // use 2 texel border round each character
      // TODO: can this be 1?
      tp.AddRectangle(wh + 4, wh + 4);
   }

   // pack them
   std::vector<uint32_t>   width;
   std::vector<uint32_t>   height;

   uint32_t numTextures = tp.AssignCoords(width, height);

   vector<vector<uint8_t> >   data(numTextures);

   // create the textures
   for (uint32_t t = 0; t < numTextures; ++t)
   {
      data[t].resize(width[t] * height[t]);
      memset(&data[t][0], 0, width[t] * height[t]);
   }

   // fill in the final result
   for (uint32_t i = 0; i < charSet.size(); ++i)
   {
      const FontTextureRectangle &r = tp.GetRectangle(i);
      Character                  &p = charSet[i];

      // copy the sub-texture to its packed destination
      uint8_t  *src;
      uint8_t  *dst;
      uint32_t src_stride;
      uint32_t dst_stride;

      uint32_t tex = r.GetTexture();

      src = &p.m_bitmap[0];
      dst = &data[tex][0] + ((r.GetY() + 2) * width[tex]) + (r.GetX() + 2);
      src_stride = p.m_pitch;
      dst_stride = width[tex];

      for (uint32_t y = 0; y < p.m_height; y++)
      {
         memcpy(&dst[1], src, p.m_width);

         dst += dst_stride;
         src += src_stride;
      }

      // Create an entry for this character
      Vec2  u, v;

      u[0] =        (r.GetX() + 1)      / (float)width[tex];
      u[1] = u[0] + (r.GetWidth() - 2)  / (float)width[tex];

      v[1] = (float)(r.GetY() + 1)      / (float)height[tex];
      v[0] = v[1] + (r.GetHeight() - 2) / (float)height[tex];

      // width and height are the same, this is the size in pixels from the center of the point, to the border
      float size     = (float)(r.GetWidth() - 2);
      int32_t offset = (int32_t)size / 2;

      // advance
      float advanceX = FixedToFloat(p.m_advanceX);
      Vec2  middle((float)(p.m_bitmapLeft + offset), (float)(p.m_bitmapTop - offset));

      m_charInfo[p.m_character] = PrintCharInfo(advanceX, middle, u, v, size, tex);
   }

   m_texture.resize(numTextures);

   for (uint32_t t = 0; t < numTextures; ++t)
   {
      m_texture[t] = TextureHandle(New);
      m_texture[t]->SetAutoMipmap(true);
      m_texture[t]->TexImage2D(GL_LUMINANCE, width[t], height[t], GL_LUMINANCE, GL_UNSIGNED_BYTE, &data[t][0]);
   }
}

PrintFont::PrintFont() :
   m_face(0),
   m_library(0)
{
}

PrintFont::~PrintFont()
{
   if (m_face != 0)
   {
      FT_Done_Face((FT_Face)m_face);
      m_face = 0;
   }

   if (m_library != 0)
   {
      FT_Done_FreeType((FT_Library)m_library);
      m_library = 0;
   }
}

//////////////////////////////
// PrintCallback
//
// Standard LR top to bottom behaviour for text -- can be overidden
//
void PrintCallback::OnBegin(PrintState &)
{
}

void PrintCallback::OnNewline(PrintState &state)
{
   state.SetPenX(0.0f);
   state.SetPenY(state.GetPenY() - state.GetLineHeight());

   // Keeping track of previous character can be useful for kerning (not currently implemented)
   state.SetPreviousChar(0);
}

bool PrintCallback::OnCharacter(PrintState &state)
{
   PrintCharInfo  info = state.GetCharInfo();

   float cx  = state.GetPenX() + info.GetMiddleX();
   float cy  = state.GetPenY() + info.GetMiddleY();
   float sz2 = 0.5f * info.GetSize();

   float x0 = cx - sz2;
   float y0 = cy - sz2;
   float x1 = cx + sz2;
   float y1 = cy + sz2;

   const Vec2  &u = info.GetU();
   const Vec2  &v = info.GetV();

   state.SetQuad(Vec4(x0, y0, u[0], v[0]), Vec4(x1, y1, u[1], v[1]));

   state.SetPenX(state.GetPenX() + info.GetAdvanceX());

   return true;
}

void PrintCallback::OnEnd(PrintState &)
{
}

//! PrintWrapper is a simple formatting callback which
//! does ragged right formatting of text
const std::vector<uint32_t> &PrintRaggedRight::Format(const std::vector<uint32_t> &str)
{
   std::vector<uint32_t>::const_iterator   readIter = str.begin();

   m_result.clear();

   float    position  = 0.0f;
   bool     startPara = true;
   bool     lineBreak = false;

   while (readIter != str.end())
   {
      uint32_t inch  = *readIter;
      float    width = GetAdvanceX(inch);

      // Have we hit the side?
      if (position + width > m_limit)
      {
         uint32_t rewind = 0;

         // Give up -- there's not enough room for a single character.
         if (position == 0.0f)
            return m_result;

         std::vector<uint32_t>::const_iterator   rewindIter = m_result.end() - 1;

         // Backtrack to last space or newline
         while (rewindIter != m_result.begin() && *rewindIter != '\n' && *rewindIter != ' ')
         {
            rewind++;
            --rewindIter;
         }

         if (!(rewindIter == m_result.begin() || *rewindIter == '\n'))
         {
            // Split on previous space
            m_result.resize(m_result.size() - rewind);
            readIter = readIter - rewind;
         }
         else
         {
            // Can't split on a space so just split the word
            // Could hypenate, but that involves a bit more logic
         }

         m_result.push_back('\n');
         lineBreak = true;
         position = 0.0f;
      }
      else
      {
         // Character fits
         if ((inch == ' ' || inch == '\t') && lineBreak && !startPara)
         {
            // Don't put a space at the start of a line (unless we are starting a paragraph)
            // in the input
            ++readIter;
         }
         else
         {
            position += width;
            m_result.push_back(inch);
            ++readIter;
            lineBreak = false;
         }

         if (inch == '\n')
            startPara = true;

         if (inch != ' ' && inch != '\t')
            startPara = false;

      }
   }

   return m_result;
}

//////////////////////////////
// Print
//
Print::Print() :
   m_encoding(eASCII),
   m_material(New),
   m_bespoke(false),
   m_callback(new PrintCallback()),
   m_formatter(0)
{}

Print::~Print()
{
   delete m_callback;
   delete m_formatter;
}

//! Return the minimum and maximum points in screen units for 2D and world units for 3D
void Print::GetBounds(Vec2 &mn, Vec2 &mx) const
{
   mn = m_min;
   mx = m_max;
}

float Print::GetWidth() const
{
   return m_max.X() - m_min.X();
}

float Print::GetHeight() const
{
   return m_max.Y() - m_min.Y();
}

void Print::Init(const PrintFontHandle &font, const EffectHandle &effect, const PrintOptions &options, bool bespoke)
{
   m_font     = font;
   m_effect   = effect;
   m_options  = options;
   m_bespoke  = bespoke;

   m_material->SetEffect(m_effect);

   if (!m_bespoke)
      m_material->SetUniform("u_textColor", Vec4(1.0f));
}

void Print::SetFont(const PrintFontHandle &font, const EffectHandle &effect, const PrintOptions &options)
{
   Init(font, effect, options);
}

void Print::SetFont(const PrintFontHandle &font, const PrintOptions &options)
{
   Init(font, GetDefaultEffect(), options);
}

void Print::SetCallback(PrintCallback *newCallback)
{
   delete m_callback;

   m_callback = newCallback;
}

void Print::SetFormatter(PrintFormatter *newFormatter)
{
   delete m_formatter;

   m_formatter = newFormatter;
}

EffectHandle Print::GetDefaultEffect() const
{
   if (s_defaultEffect.IsNull())
   {
      s_defaultEffect = EffectHandle(New);
      s_defaultEffect->Read(s_defaultEffectText);
   }

   return s_defaultEffect;
}

static uint32_t Offset(uint32_t n)
{
   return sizeof(GLfloat) * n;
}

void Print::SetText(const string &text, float height, const Vec2 &position)
{
   SetText(text, eASCII, height, position);
}

void Print::SetText(const PrintString &text, eStringEncoding encoding, float height, const Vec2 &position)
{
   m_text     = text;
   m_encoding = encoding;

   Reset(height, position);
}

void Print::Reset(float height, const Vec2 &position)
{
   if (m_font.IsNull())
      BSG_THROW("Font missing from Print object");

   m_min = Vec2( std::numeric_limits<float>::max());
   m_max = Vec2(-std::numeric_limits<float>::max());

   PrintState        state(height, m_font);                // State that is passed to callbacks to allow apps to modify behaviour

   vector<uint32_t>  text;

   // Decode string
   typedef EncodingConstIterator Iter;
   Iter    begin = Iter::Begin(m_text, m_encoding);
   Iter    end   = Iter::End(m_text, m_encoding);

   for (Iter textIter = begin; textIter != end; ++textIter)
   {
      uint32_t ch = *textIter;
      text.push_back(ch);
   }

   if (m_formatter != 0)
   {
      m_formatter->SetFont(height, m_font);
      text = m_formatter->Format(text);
   }

   // Generate geometry
   uint32_t          numTextures(m_font->GetNumTextures());

   vector<Vec2>      vb(text.size() * 12);                  // Holds all the character data
   vector<uint32_t>  vbStart(numTextures);                  // vbStart and vbEnd hold start and end
   vector<uint32_t>  vbEnd(numTextures);                    // points in vb for bits of text that use different textures

   uint32_t          vbi  = 0;
   float             lastLinePos = 0.0f;

   for (uint32_t tex = 0; tex < numTextures; ++tex)
   {
      vbStart[tex] = vbi;
      vbEnd[tex]   = vbi;

      state.SetPenX(0.0f);
      state.SetPenY(0.0f);
      lastLinePos = 0.0f;

      m_callback->OnBegin(state);

      for (uint32_t n = 0; n < text.size(); ++n)
      {
         uint32_t   ch  = text[n];

         if (ch == '\n')
         {
            // start a new line
            m_callback->OnNewline(state);
            lastLinePos = state.GetPenY();
         }
         else if (m_font->IsValidGlyph(ch))
         {
            state.SetCurrentChar(ch);

            if (m_callback->OnCharacter(state))
            {
               if (m_font->GetTextureForChar(ch) == tex)
               {
                  const Vec4  &p0 = state.GetBL();
                  const Vec4  &p1 = state.GetTR();

                  m_min.X() = std::min(m_min.X(), p0.X()); m_max.X() = std::max(m_max.X(), p0.X());
                  m_min.X() = std::min(m_min.X(), p1.X()); m_max.X() = std::max(m_max.X(), p1.X());

                  m_min.Y() = std::min(m_min.Y(), p0.Y()); m_max.Y() = std::max(m_max.Y(), p0.Y());
                  m_min.Y() = std::min(m_min.Y(), p1.Y()); m_max.Y() = std::max(m_max.Y(), p1.Y());

                  uint32_t ix = vbi * 12;

                  vb[ix + 0 ] = Vec2(p0.X(), p0.Y()); vb[ix + 1 ] = Vec2(p0.Z(), p0.W());
                  vb[ix + 2 ] = Vec2(p0.X(), p1.Y()); vb[ix + 3 ] = Vec2(p0.Z(), p1.W());
                  vb[ix + 4 ] = Vec2(p1.X(), p0.Y()); vb[ix + 5 ] = Vec2(p1.Z(), p0.W());
                  vb[ix + 6 ] = Vec2(p1.X(), p0.Y()); vb[ix + 7 ] = Vec2(p1.Z(), p0.W());
                  vb[ix + 8 ] = Vec2(p0.X(), p1.Y()); vb[ix + 9 ] = Vec2(p0.Z(), p1.W());
                  vb[ix + 10] = Vec2(p1.X(), p1.Y()); vb[ix + 11] = Vec2(p1.Z(), p1.W());

                  ++vbi;

                  vbEnd[tex] = vbi;
               }
            }
         }
      }

      m_callback->OnEnd(state);
   }

   Vec2  anchor;

   if (m_options.GetUseBoundingBox())
      anchor = m_options.AnchorOffset(m_min, m_max);
   else
      anchor = m_options.AnchorOffset(Vec2(0.0f, lastLinePos), Vec2(m_max.X(), 0.0f));

   // Put origin at anchor point and position
   for (uint32_t i = 0; i < vb.size(); i += 2)
      vb[i] = vb[i] - anchor + position;

   m_max = m_max - anchor + position;
   m_min = m_min - anchor + position;

   // Get rid of any existing surface and geometry data
   m_surfaces.clear();
   Clear();

   for (uint32_t tex = 0; tex < numTextures; ++tex)
   {
      if (vbStart[tex] != vbEnd[tex])
      {
         uint32_t vbCount = vbEnd[tex] - vbStart[tex];

         // Create surfaces for text
         SurfaceHandle surface = SurfaceHandle(New);

         m_surfaces.push_back(surface);

         surface->SetDrawData(GL_TRIANGLES, 6 * vbCount, 2 * sizeof(Vec2), &vb[vbStart[tex] * 2 * 6]);
         surface->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(2, GL_FLOAT, 2 * sizeof(Vec2), Offset(0)));
         surface->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, 2 * sizeof(Vec2), Offset(2)));

         surface->SetBound(Bound(Length(m_max - m_min) / 2.0f, ((m_min + m_max) / 2.0f).Lift(0.0f)));

         surface->SetCull(false);

         AppendSurface(surface, m_material, new GlyphMapCallback(m_font->GetTexture(tex), m_material));
      }
   }
}

void Print::DeleteDefaultEffect()
{
   s_defaultEffect.Clear();
}

#endif
