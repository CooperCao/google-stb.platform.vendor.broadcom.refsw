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

#include "bsg_font.h"
#include "bsg_fonts.h"
#include "bsg_gl_texture.h"
#include "bsg_font_texture_packer.h"
#include "bsg_exception.h"
#include "bsg_application.h"

#include <vector>
#include <fstream>

using namespace std;

#include <ft2build.h>
#include FT_FREETYPE_H

namespace bsg
{

class FontCharacter
{
public:
   uint8_t *m_bitmap;
   uint32_t m_width;
   uint32_t m_height;
   uint32_t m_pitch;
   int32_t  m_advanceX;
   int32_t  m_advanceY;
   int32_t  m_bitmapLeft;
   int32_t  m_bitmapTop;
   char     m_character;
};

const uint32_t CHAR_COUNT = 128;

static uint32_t FloatToFixed(float f)
{
   return (uint32_t)(f * 64.0f + 0.5f);
}

static float FixedToFloat(uint32_t i)
{
   return (float)i / 64.0f;
}

Font::Font() :
   m_usePoints(false),
   m_pointSize(0.0f),
   m_data(NULL),
   m_charMap(NULL),
   m_charSize(NULL),
   m_charAdvance(NULL),
   m_library(NULL),
   m_face(NULL),
   m_effect(New)
{
}

Font::~Font()
{
   Reset();
}

void Font::SetFontInPercent(const std::string &fontName, float pointSizePercent, uint32_t screenHeight, bool usePoints)
{
   float pixels = screenHeight * pointSizePercent / 100.0f;

   SetFontInPixels(fontName, pixels, usePoints);
}

void Font::SetFontInPercent(const FontMemInfo &fontInfo, float pointSizePercent, uint32_t screenHeight, bool usePoints)
{
   float pixels = screenHeight * pointSizePercent / 100.0f;

   SetFontInPixels(fontInfo, pixels, usePoints);
}

void Font::SetFontInPixels(const std::string &fontName, float pointSize, bool usePoints)
{
   SetFont(fontName, 0, 0, pointSize, usePoints);
}

void Font::SetFontInPixels(const FontMemInfo &fontInfo, float pointSize, bool usePoints)
{
   SetFont(fontInfo.GetName(), fontInfo.GetAddress(), fontInfo.GetSize(), pointSize, usePoints);
}

void Font::SetFont(const std::string &fontName, const unsigned char *fontInMemory, uint32_t size, float pointSize, bool usePoints)
{
   static   GLint numUniforms = -1;

   if (numUniforms == -1)
      glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &numUniforms);

   // The points shader uses two arrays of 127 plus one vec = 127 + 127 + 1 = 255 worst case with
   // no tight packing.
   m_usePoints = usePoints && (numUniforms >= 255);

   std::istringstream iss(EffectString(), istringstream::in);
   m_effect = EffectHandle(New);
   m_effect->Load(iss);

   Reset();

   m_texture  = TextureHandle(New);
   m_material = MaterialHandle(New);

   m_fontName  = fontName;
   m_pointSize = pointSize;

   vector<FontCharacter>   charSet;

   if (FT_Init_FreeType((FT_Library*)&m_library))
      BSG_THROW("Failed to initialize freetype");

   if (fontInMemory == 0)
   {
      std::string resourceName = Application::Instance()->FindResource(fontName);

      if (FT_New_Face((FT_Library)m_library, resourceName.c_str(), 0, (FT_Face*)&m_face))
         BSG_THROW("Failed to open font " << fontName);
   }
   else
   {
      if (FT_New_Memory_Face((FT_Library)m_library, fontInMemory, size, 0, (FT_Face*)&m_face))
         BSG_THROW("Failed to load memory font " << fontName);
   }

   FT_Face face = (FT_Face)m_face;

   // 72 is the h/v dpi
   FT_Set_Char_Size(face, FloatToFixed(pointSize), 0, 0, 0);

   m_lineHeight = FixedToFloat(face->size->metrics.height);

   // first CHAR_COUNT chars should be enough to print anything a latin set
   for (uint32_t i = 0; i < CHAR_COUNT; i++)
   {
      FT_GlyphSlot slot;
      FontCharacter p;

      if (i != '\n')
      {
         if (FT_Load_Char(face, i, FT_LOAD_RENDER))
            BSG_THROW("Failed to load freetype character");

         slot = face->glyph;

         p.m_width = slot->bitmap.width;
         p.m_height = slot->bitmap.rows;
         p.m_pitch = slot->bitmap.width;
         p.m_bitmap = new uint8_t[p.m_pitch * p.m_height];
         p.m_advanceX = slot->advance.x;
         p.m_advanceY = slot->advance.y;
         p.m_bitmapLeft = slot->bitmap_left;
         p.m_bitmapTop = slot->bitmap_top;
         p.m_character = i;

         memcpy(p.m_bitmap, slot->bitmap.buffer, p.m_pitch * p.m_height);
      }
      else
      {
         // CR -> just move on
         p.m_width = 0;
         p.m_height = 0;
         p.m_pitch = 0;
         p.m_bitmap = new uint8_t[1];
         p.m_advanceX = 0;
         p.m_advanceY = 0;
         p.m_bitmapLeft = 0;
         p.m_bitmapTop = 0;
         p.m_character = i;
      }

      // insert vector
      charSet.push_back(p);
   }

   FontTexturePacker tp;

   // populate the structure with all the textures
   for (vector<FontCharacter>::iterator it = charSet.begin();
      it != charSet.end();
      ++it)
   {
      // round to even number as we divide by two in places
      uint32_t wh = (std::max((*it).m_width, (*it).m_height) + 1) & ~0x1;
      // these are all square, so use width for the height
      // use 2 texel border round each character
      // TODO: can this be 1?
      tp.AddRectangle(wh + 4, wh + 4);
   }

   // pack them
   std::vector<uint32_t> widthV;
   std::vector<uint32_t> heightV;

   uint32_t numTextures = tp.AssignCoords(widthV, heightV);

   if (numTextures > 1)
      BSG_THROW("Font too large to fit a single texture");

   uint32_t width  = widthV[0];
   uint32_t height = heightV[0];

   // create the texture
   m_data = new uint8_t[width * height];
   memset(m_data, 0, width * height);
   m_texWidth = width;
   m_texHeight = height;

   m_charMap = new Vec4[charSet.size()];
   memset(m_charMap, 0, 4 * charSet.size());

   m_charSize = new float[charSet.size()];
   memset(m_charSize, 0, charSet.size());

   m_charAdvance = new prv::CharAdvance[charSet.size()];
   memset(m_charAdvance, 0, sizeof(prv::CharAdvance) * charSet.size());

   // fill in the final result
   int   j = 0;

   for (vector<FontCharacter>::iterator it = charSet.begin(); it != charSet.end(); ++it)
   {
      const FontTextureRectangle &r = tp.GetRectangle(j);
      FontCharacter *p = &(*it);

      // copy the sub-texture to its packed destination
      uint8_t  *src, *dst;
      uint32_t src_stride, dst_stride;

      src = p->m_bitmap;
      dst = m_data + ((r.GetY() + 2) * m_texWidth) + (r.GetX() + 2);
      src_stride = p->m_pitch;
      dst_stride = m_texWidth;

      for (uint32_t y = 0; y < p->m_height; y++)
      {
         memcpy(&dst[1], src, p->m_width);

         dst += dst_stride;
         src += src_stride;
      }

      // populate the array containing the texture coordinates
      m_charMap[j][0] = (float)(r.GetX() + 1)      / (float)m_texWidth;
      m_charMap[j][1] = (float)(r.GetY() + 1)      / (float)m_texHeight;
      m_charMap[j][2] = (float)(r.GetWidth() - 2)  / (float)m_texWidth;
      m_charMap[j][3] = (float)(r.GetHeight() - 2) / (float)m_texHeight;

      // width and height are the same, this is the size in pixels from the center of the point, to the border
      m_charSize[j] = (float)(r.GetWidth() - 2);

      int32_t offset = (int32_t)m_charSize[j] / 2;

      // advance
      m_charAdvance[j].m_advanceX = FixedToFloat(p->m_advanceX);
      m_charAdvance[j].m_unused   = p->m_advanceY;
      m_charAdvance[j].m_middleX  = p->m_bitmapLeft + offset;
      m_charAdvance[j].m_middleY  = p->m_bitmapTop  - offset;

      // move to next character
      j++;
   }

   // delete as the atlas has been built
   for (vector<FontCharacter>::iterator it = charSet.begin();
      it != charSet.end();
      ++it)
   {
      FontCharacter *p = &(*it);
      delete [] p->m_bitmap;
   }
   charSet.clear();

   m_material->SetEffect(m_effect);
   m_material->SetTexture("u_tex", GetTexture());

   if (usePoints)
   {
      m_material->SetUniformValue("u_size", m_charSize, CHAR_COUNT);
      m_material->SetUniformValue("u_tlut", m_charMap, CHAR_COUNT);
   }
}

void Font::Reset()
{
   delete [] m_data;
   delete [] m_charMap;
   delete [] m_charSize;
   delete [] m_charAdvance;

   m_data        = 0;
   m_charMap     = 0;
   m_charSize    = 0;
   m_charAdvance = 0;

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

   m_texture.Clear();
   m_material.Clear();
}

TextureHandle Font::GetTexture()
{
   if (!m_texture->HasData())
   {
      m_texture->TexImage2D(GL_LUMINANCE, m_texWidth, m_texHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_data);
      delete [] m_data;
      m_data = NULL;
   }

   return m_texture;
}

static float ViewScale(float n)
{
   return n * 2.0f - 1.0f;
}

void Font::MakeStringQuadArray(string &str, float x, float y, int32_t irx, int32_t iry, vector<Vec2> *arr)
{
   vector<Vec2>   &res   = *arr;

   float rx = (float)irx;
   float ry = (float)iry;

   float wrapx = rx;

   float qox = 0.0f;
   float qoy = 0.0f;

   y -= m_lineHeight;  // Move origin to top left of first char

   float pen_x = x;
   float pen_y = y;

   uint32_t len = str.length();

   res.clear();
   res.resize(len * 12);

   bool scan_ahead = true;

   for (uint32_t i = 0; i < len; i++)
   {
      const char *word = &str[i];
      uint32_t   ch    = *word;

      float word_pen_x = pen_x;

      if (scan_ahead)
      {
         while (ch != ' ' && ch != '\n' && ch != '\0')
         {
            word_pen_x += m_charAdvance[ch].m_advanceX;

            if (word_pen_x > wrapx)
            {
               // word wrap to new line
               pen_x  = x;
               pen_y -= m_lineHeight;
               break;
            }

            ch = *word++;
         }
         scan_ahead = false;
      }

      // start a new line
      if (str[i] == '\n')
      {
         pen_x = x;
         pen_y -= m_lineHeight;
      }

      if (str[i] == ' ' || str[i] == '\n')
      {
         scan_ahead = true;
      }

      // if we have gone off the screen, record where we got to and quit
      if (pen_y < -2 * (int32_t)m_lineHeight)
      {
         // Clip the string where we stopped drawing
         str = str.substr(0, i);
         break;
      }

      // Two triangles per character = 4 unique points, 6 submitted
      uint32_t chi = (uint32_t)str[i];

      float cx = (pen_x + m_charAdvance[chi].m_middleX) / rx;
      float cy = (pen_y + m_charAdvance[chi].m_middleY) / ry;
      float sx = (0.5f * m_charSize[chi]) / rx;
      float sy = (0.5f * m_charSize[chi]) / ry;

      float x0 = ViewScale(cx - sx) + qox;
      float y0 = ViewScale(cy - sy) + qoy;
      float x1 = ViewScale(cx + sx) + qox;
      float y1 = ViewScale(cy + sy) + qoy;

      float u0 = m_charMap[chi][0];
      float v1 = m_charMap[chi][1];
      float u1 = u0 + m_charMap[chi][2];
      float v0 = v1 + m_charMap[chi][3];

      uint32_t ix = i * 12;

      res[ix + 0 ] = Vec2(x0, y0); res[ix + 1 ] = Vec2(u0, v0);
      res[ix + 2 ] = Vec2(x0, y1); res[ix + 3 ] = Vec2(u0, v1);
      res[ix + 4 ] = Vec2(x1, y0); res[ix + 5 ] = Vec2(u1, v0);
      res[ix + 6 ] = Vec2(x1, y0); res[ix + 7 ] = Vec2(u1, v0);
      res[ix + 8 ] = Vec2(x0, y1); res[ix + 9 ] = Vec2(u0, v1);
      res[ix + 10] = Vec2(x1, y1); res[ix + 11] = Vec2(u1, v1);

      pen_x += m_charAdvance[chi].m_advanceX;
   }
}

void Font::MakeStringPointArray(string &str, float x, float y, int32_t irx, int32_t iry, vector<Vec2> *arr)
{
   const Application &app = *Application::Instance();

   float rx = (float)irx;
   float ry = (float)iry;

   y -= m_lineHeight;  // Move origin to top left of first char

   float pen_x = x;
   float pen_y = y;

   uint32_t len = str.length();

   arr->clear();
   arr->resize(len);

   bool scan_ahead = true;

   for (uint32_t i = 0; i < len; i++)
   {
      const char *word = &str[i];
      uint32_t   ch    = *word;

      float word_pen_x = pen_x;

      if (scan_ahead)
      {
         while (ch != ' ' && ch != '\n' && ch != '\0')
         {
            word_pen_x += m_charAdvance[ch].m_advanceX;

            if (word_pen_x > rx)
            {
               // word wrap to new line
               pen_x  = x;
               pen_y -= m_lineHeight;
               break;
            }

            ch = *word++;
         }
         scan_ahead = false;
      }

      // start a new line
      if (str[i] == '\n')
      {
         pen_x = x;
         pen_y -= m_lineHeight;
      }

      if (str[i] == ' ' || str[i] == '\n')
      {
         scan_ahead = true;
      }

      // if we have gone off the screen, record where we got to and quit
      if (pen_y < -2 * (int32_t)m_lineHeight)
      {
         // Clip the string where we stopped drawing
         str = str.substr(0, i);
         break;
      }

      ((*arr)[i]).X() = (pen_x + m_charAdvance[(uint32_t)str[i]].m_middleX) / rx;
      ((*arr)[i]).Y() = (pen_y + m_charAdvance[(uint32_t)str[i]].m_middleY) / ry;

      /* Adjust position of character by 1/4 of sprite width to correct center point */
      if (app.IsStereo())
         ((*arr)[i]).X() += (0.5f * m_charSize[(uint32_t)str[i]] / app.GetWindowWidth());

      pen_x += m_charAdvance[(uint32_t)str[i]].m_advanceX;
   }

   if (app.IsStereo())
   {
      m_material->SetUniformValue("u_scale", Vec2(2.0f, 1.0f));
      m_material->SetUniformValue("u_clamp", Vec2(0.5f, 1.0f));
   }
   else
   {
      m_material->SetUniformValue("u_scale", Vec2(1.0f, 1.0f));
      m_material->SetUniformValue("u_clamp", Vec2(1.0f, 1.0f));
   }
}

void Font::DrawTextString(const string &str, float xpos, float ypos, const Vec4 &color)
{
   if (str.length() == 0)
      return;

   const Application &app = *Application::Instance();

   uint32_t width    = app.GetWindowWidth();
   uint32_t height   = app.GetWindowHeight();

   app.glViewport(0, 0, width, height);

   m_material->SetUniformValue("u_color", color);

   std::string       clippedString(str);
   std::vector<Vec2> vertArray;

   if (m_usePoints)
      MakeStringPointArray(clippedString, xpos, ypos, width, height, &vertArray);
   else
      MakeStringQuadArray(clippedString, xpos, ypos, width, height, &vertArray);

   if (clippedString.length() == 0)
      return;

   m_material->MakeActive(0);

   // Default state
   ShadowState glState;

   glState.UpdateGLState(m_material->GetGLState(0));

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   GLProgram &prog = m_material->GetEffect()->GetPass(0)->Program();

   if (m_usePoints)
   {
      GLint posLoc = prog.GetAttribLocation("a_position");
      glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)&vertArray[0]);
      glEnableVertexAttribArray(posLoc);

      GLint charLoc = prog.GetAttribLocation("a_character");
      glVertexAttribPointer(charLoc, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, (const GLvoid*)clippedString.c_str());
      glEnableVertexAttribArray(charLoc);

      glDrawArrays(GL_POINTS, 0, clippedString.length());

      glDisableVertexAttribArray(posLoc);
      glDisableVertexAttribArray(charLoc);
   }
   else
   {
      GLint posLoc = prog.GetAttribLocation("a_position");
      glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const GLvoid*)&vertArray[0]);
      glEnableVertexAttribArray(posLoc);

      GLint tcLoc  = prog.GetAttribLocation("a_tc");
      glVertexAttribPointer(tcLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const GLvoid*)&vertArray[1]);
      glEnableVertexAttribArray(tcLoc);

      glDrawArrays(GL_TRIANGLES, 0, vertArray.size() / 2);

      glDisableVertexAttribArray(posLoc);
      glDisableVertexAttribArray(tcLoc);
   }

   // Rest to default state
   glState.SetToDefault();
}

static const char *s_pointEffect =
   "PASS 0                                                      \n"
   "{                                                           \n"
   "   SEMANTICS                                                \n"
   "   {                                                        \n"
   "      a_position    = VATTR_POSITION;                       \n"
   "      a_character   = VATTR_USER;                           \n"
   "      u_size        = SCALAR_USER;                          \n"
   "      u_scale       = VECTOR2_USER;                         \n"
   "      u_clamp       = VECTOR2_USER;                         \n"
   "      u_tlut        = VECTOR4_USER;                         \n"
   "      u_color       = VECTOR4_USER;                         \n"
   "   }                                                        \n"
   "                                                            \n"
   "   STATE                                                    \n"
   "   {                                                        \n"
   "      EnableBlend = true;                                   \n"
   "      BlendFunc = SRC_ALPHA, ONE_MINUS_SRC_ALPHA;           \n"
   "      EnableDepthTest = false;                              \n"
   "   }                                                        \n"
   "                                                            \n"
   "   SAMPLER_2D u_tex                                         \n"
   "   {                                                        \n"
   "      Unit = 0;                                             \n"
   "      Wrap = CLAMP, CLAMP;                                  \n"
   "      Filter = LINEAR, LINEAR;                              \n"
   "   }                                                        \n"
   "                                                            \n"
   "   VERTEX_SHADER                                            \n"
   "   {                                                        \n"
   "      attribute  vec4  a_position;                          \n"
   "      attribute  float a_character;                         \n"
   "      uniform    float u_size[127];                         \n"
   "      uniform    vec4  u_tlut[127];                         \n"
   "      uniform    vec2  u_scale;                             \n"
   "      varying    vec2  start_position;                      \n"
   "      varying    vec2  scale;                               \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         int i = int(max(0.0, min(a_character, 126.0)));    \n"
   "         gl_PointSize = u_size[i];                          \n"
   "         gl_Position = (a_position - 0.5) * 2.0;            \n"
   "                                                            \n"
   "         vec4 lut = u_tlut[i];                              \n"
   "         start_position = lut.xy;                           \n"
   "         scale = u_scale * lut.zw;                          \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "                                                            \n"
   "   FRAGMENT_SHADER                                          \n"
   "   {                                                        \n"
   "      precision mediump float;                              \n"
   "                                                            \n"
   "      uniform sampler2D   u_tex;                            \n"
   "      uniform vec4        u_color;                          \n"
   "      uniform vec2        u_clamp;                          \n"
   "      varying vec2        start_position;                   \n"
   "      varying vec2        scale;                            \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         if(gl_PointCoord.x > u_clamp.x ||                  \n"
   "            gl_PointCoord.y > u_clamp.y)                    \n"
   "            discard;                                        \n"
   "         vec2 tc = (gl_PointCoord * scale) + start_position;\n"
   "         vec4 col = texture2D(u_tex, tc);                   \n"
   "         if (col.r < 0.01)                                  \n"
   "            discard;                                        \n"
   "         col.a = col.r * u_color.a;                         \n"
   "         col.rgb = u_color.rgb;                             \n"
   "         gl_FragColor = col;                                \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "}                                                           \n";

static const char *s_quadEffect =
   "PASS 0                                                      \n"
   "{                                                           \n"
   "   SEMANTICS                                                \n"
   "   {                                                        \n"
   "      a_position    = VATTR_POSITION;                       \n"
   "      a_tc          = VATTR_TEXCOORD1;                      \n"
   "      u_color       = VECTOR4_USER;                         \n"
   "   }                                                        \n"
   "                                                            \n"
   "   STATE                                                    \n"
   "   {                                                        \n"
   "      EnableBlend = true;                                   \n"
   "      BlendFunc = SRC_ALPHA, ONE_MINUS_SRC_ALPHA;           \n"
   "      EnableDepthTest = false;                              \n"
   "   }                                                        \n"
   "                                                            \n"
   "   SAMPLER_2D u_tex                                         \n"
   "   {                                                        \n"
   "      Unit = 0;                                             \n"
   "      Wrap = CLAMP, CLAMP;                                  \n"
   "      Filter = LINEAR, LINEAR;                              \n"
   "   }                                                        \n"
   "                                                            \n"
   "   VERTEX_SHADER                                            \n"
   "   {                                                        \n"
   "      attribute  vec2  a_position;                          \n"
   "      attribute  vec2  a_tc;                                \n"
   "      varying    vec2  v_tc;                                \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         gl_Position = vec4(a_position.xy, 0.0, 1.0);       \n"
   "         v_tc        = a_tc;                                \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "                                                            \n"
   "   FRAGMENT_SHADER                                          \n"
   "   {                                                        \n"
   "      precision mediump float;                              \n"
   "                                                            \n"
   "      uniform sampler2D   u_tex;                            \n"
   "      uniform vec4        u_color;                          \n"
   "      varying vec2        v_tc;                             \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         float alpha = texture2D(u_tex, v_tc).r;            \n"
   "         if (alpha < 0.01)                                  \n"
   "            discard;                                        \n"
   "         alpha *= u_color.a;                                \n"
   "         gl_FragColor = vec4(u_color.rgb, alpha);           \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "}                                                           \n";


const char *Font::EffectString() const
{
   return m_usePoints ? s_pointEffect : s_quadEffect;
}

}

#endif /* BSG_STAND_ALONE */

