/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <vector>

class Atlas
{
public:
   Atlas() {}
   ~Atlas() {}

   bool Build(const char *font_name, const unsigned size);

   std::vector<float> Generate(const char *str, int x, int y, int rx, int ry, unsigned *position);

   unsigned Width() const { return m_width; }
   unsigned Height() const { return m_height; }
   const std::vector<unsigned char>& Texture() const { return m_texture; }
   const std::vector<float>& CharMap() const { return m_char_map; }
   const std::vector<float>& CharSize() const { return m_char_size; }
   unsigned Ascender() const { return m_ascender; }

private:
   std::vector<uint8_t> m_texture;
   unsigned m_width;
   unsigned m_height;
   unsigned m_ascender;
   std::vector<float> m_char_map;
   std::vector<float> m_char_size;
   std::vector<int> m_char_advance;
};