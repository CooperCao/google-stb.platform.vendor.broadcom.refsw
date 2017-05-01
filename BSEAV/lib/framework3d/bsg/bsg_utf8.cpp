/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_utf8.h"

namespace bsg
{

static uint32_t Classify(unsigned char ch)
{
   uint32_t bytes;

   if ((ch & 0x80) == 0)
      bytes = 1;
   else if ((ch & 0xe0) == 0xc0)
      bytes = 2;
   else if ((ch & 0xf0) == 0xe0)
      bytes = 3;
   else if ((ch & 0xf8) == 0xf0)
      bytes = 4;
   else
      bytes  = 0;

   return bytes;
}

static uint32_t Decode(const std::vector<unsigned char> &str, uint32_t index)
{
   unsigned char  ch    = str[index];
   uint32_t       bytes = Classify(ch);
   uint32_t       code  = 0;
   uint32_t       size  = str.size();

   switch (bytes)
   {
   case 1:  code  = ch & 0x7f;  break;
   case 2:  code  = ch & 0x1f;  break;
   case 3:  code  = ch & 0x0f;  break;
   case 4:  code  = ch & 0x07;  break;
   default: code  = 0;          break;
   }

   for (uint32_t b = 1; b < bytes; ++b)
   {
      if (index + b < size)
         code = (code << 6) | (str[index + b] & 0x3f);
   }

   return code;
}

// Skip to next valid start character or end of string
void UTF_8_ConstIterator::SkipToNext()
{
   uint32_t size = m_str.size();

   while (m_index < size && ((m_str[m_index] & 0xc0) == 0x80))
      ++m_index;
}

// Dereference
//
// Returns the current character, if valid or NULL otherwise
//
uint32_t UTF_8_ConstIterator::operator*() const
{
   return Decode(m_str, m_index);
}

// Pre increment
//
// Moves to the next character
//
EncConstIterator &UTF_8_ConstIterator::operator++()
{
   uint32_t step = Classify(m_str[m_index]);

   // Check for a duff start
   if (step == 0)
      SkipToNext();
   else
      m_index += step;

   return *this;
}

// Dereference
//
// Returns the current character, if valid or NULL otherwise
//
uint32_t ASCII_ConstIterator::operator*() const
{
   return m_str[m_index];
}

// Pre increment
//
// Moves to the next character
//
EncConstIterator &ASCII_ConstIterator::operator++()
{
   ++m_index;
   return *this;
}

}
