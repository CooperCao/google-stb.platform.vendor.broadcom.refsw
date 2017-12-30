/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <map>
#include <algorithm>
#include <memory>
#include <assert.h>

using namespace std;

#include <ft2build.h>
#include FT_FREETYPE_H

#include "texture_packer.h"

#include "build_atlas.h"

class Character
{
public:
   Character(char character, vector<unsigned char>::size_type n = 0, unsigned width = 0, unsigned height = 0, unsigned pitch = 0,
      int advance_x = 0, int advance_y = 0, int bitmap_left = 0, int bitmap_top = 0) :
      m_character(character), m_bitmap(n > 0 ? n : 1),
      m_advance_x(advance_x), m_advance_y(advance_y),
      m_bitmap_left(bitmap_left), m_bitmap_top(bitmap_top),
      m_width(width), m_height(height), m_pitch(pitch)
   {}
   ~Character() {}

   unsigned BitmapSize() const { return m_pitch * m_height; }
   unsigned Width() const { return m_width; }
   unsigned Height() const { return m_height; }
   unsigned Pitch() const { return m_pitch; }

public:
   char m_character;
   vector<uint8_t> m_bitmap;
   int m_advance_x;
   int m_advance_y;
   int m_bitmap_left;
   int m_bitmap_top;

private:
   unsigned int m_width;
   unsigned int m_height;
   unsigned int m_pitch;
};

#define CHAR_COUNT 128

// special.  Will continue until the pen position goes out of bounds, formatting in complete words across the
// display.

vector<float> Atlas::Generate(const char *str, int x, int y, int rx, int ry, unsigned *position)
{
   int pen_x = x;
   int pen_y = ry - y;

   size_t len = strlen(str);
   vector<float> v(2 * len);
   bool scan_ahead = true;

   for (int i = 0; i < (int)len; i++)
   {
      const char * word = &str[i];
      int word_pen_x = pen_x;

      if (scan_ahead)
      {
         while (*word != ' ' && *word != '\n' && *word != '\0')
         {
            word_pen_x += (m_char_advance[(*word * 4) + 0] / 64);
            if ((word_pen_x + m_char_advance[(str[i] * 4) + 2]) > (rx - m_char_size[(int)*word] / 2))
            {
               // word wrap to new line
               pen_x = x;
               pen_y -= m_ascender;
               break;
            }
            word++;
         }
         scan_ahead = false;
      }

      // start a new line
      if (str[i] == '\n')
      {
         pen_x = x;
         pen_y -= (m_ascender * 2);
      }

      if (str[i] == ' ' || str[i] == '\n')
      {
         scan_ahead = true;
      }

      // if we have gone off the screen, record where we got to and quit
      if (pen_y < 32)
      {
         if (position != NULL)
            *position += i;
         break;
      }

      v[(i * 2) + 0] = (pen_x + m_char_advance[(str[i] * 4) + 2]) / (float)rx;
      v[(i * 2) + 1] = (pen_y + m_char_advance[(str[i] * 4) + 3]) / (float)ry;
      pen_x += m_char_advance[(str[i] * 4) + 0] / 64;
   }

   return v;
}

bool Atlas::Build(const char *font_name,
                  const unsigned size)
{
   FT_Library library;
   FT_Face    face;
   vector<Character> set;

   FT_Init_FreeType(&library);
   FT_New_Face(library, font_name, 0, &face);

   // 72 is the h/v dpi
   FT_Set_Char_Size(face, size * 64, 0, 0, 0);

   m_ascender = face->size->metrics.ascender / 64;

   // first CHAR_COUNT chars should be enough to print anything a latin set
   for (int i = 0; i < CHAR_COUNT; i++)
   {
      if (i != '\n')
      {
         FT_Load_Char(face, i, FT_LOAD_RENDER);

         FT_GlyphSlot slot = face->glyph;

         Character p((char)i, (slot->bitmap.width * slot->bitmap.rows),
            slot->bitmap.width, slot->bitmap.rows, slot->bitmap.width,
            slot->advance.x, slot->advance.y, slot->bitmap_left, slot->bitmap_top);

         std::copy(slot->bitmap.buffer, slot->bitmap.buffer + p.BitmapSize(), p.m_bitmap.begin());

         set.emplace_back(p);
      }
      else
         // CR -> just move on
         set.emplace_back(Character((char)i));
   }

   TexturePacker tp;

   // populate the structure with all the textures
   // range based for not supported in GCC4.5
   for (vector<Character>::const_iterator it = set.begin(); it != set.end(); ++it)
   {
      // round to even number as we divide by two in places
      unsigned wh = (std::max(it->Width(), it->Height()) + 1) & ~0x1;
      // these are all square, so use width for the height
      // use 2 texel border round each character
      // TODO: can this be 1?
      tp.AddRectangle(wh + 4, wh + 4);
   }

   // pack them
   unsigned width, height;
   tp.AssignCoords(&width, &height);

   // create the texture
   m_texture.resize(width * height);
   m_width = width;
   m_height = height;

   m_char_map.resize(4 * set.size());
   m_char_size.resize(set.size());
   m_char_advance.resize(4 * set.size());

   // fill in the final result
   int i = 0;
   auto iter = tp.Get().begin();
   // range based for not supported in GCC4.5
   for (vector<Character>::const_iterator it = set.begin(); it != set.end(); ++it)
   {
      auto r = *iter;
      printf("character %d, { %d, %d, %d, %d }\n", it->m_character, r->X(), r->Y(), r->Width(), r->Height());

      // copy the subtexture to its packed destination
      const uint8_t *src = &it->m_bitmap[0];
      uint8_t *dst = &m_texture[0] + ((r->Y() + 2) * m_width) + (r->X() + 2);
      unsigned src_stride = it->Pitch();
      unsigned dst_stride = m_width;

      for (unsigned y = 0; y < it->Height(); y++)
      {
         memcpy(&dst[1], src, src_stride);

         dst += dst_stride;
         src += src_stride;
      }

      // populate the array containing the texture coordinates
      m_char_map[(i * 4) + 0] = (float)(r->X() + 1) / (float)m_width;
      m_char_map[(i * 4) + 1] = (float)(r->Y() + 1) / (float)m_height;
      m_char_map[(i * 4) + 2] = (float)(r->Width() - 2) / (float)m_width;
      m_char_map[(i * 4) + 3] = (float)(r->Height() - 2) / (float)m_height;

      // width and height are the same, this is the size in pixels from the center of the point, to the border
      m_char_size[i] = (float)(r->Width() - 2);

      int offset = (int)m_char_size[i] / 2;

      // advance
      m_char_advance[(i * 4) + 0] = it->m_advance_x;
      m_char_advance[(i * 4) + 1] = it->m_advance_y;
      m_char_advance[(i * 4) + 2] = it->m_bitmap_left + offset;
      m_char_advance[(i * 4) + 3] = it->m_bitmap_top - offset;

      // move to next character
      std::advance(iter, 1);
      i++;
   }

   return true;
}
