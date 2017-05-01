/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_UTF8_H__
#define __BSG_UTF8_H__

#include <stdint.h>
#include <vector>

namespace bsg
{

enum eStringEncoding
{
   eASCII,
   eUTF_8
};

// @cond
class EncConstIterator
{
public:
   EncConstIterator(const std::vector<unsigned char> &str, uint32_t index = 0) :
      m_str(str),
      m_index(index)
   {
   }

   virtual ~EncConstIterator() {}

   bool operator==(const EncConstIterator &rhs) const
   {
      return m_index == rhs.m_index &&
             &m_str  == &rhs.m_str;
   }

   virtual uint32_t operator*() const        = 0;
   virtual EncConstIterator &operator++() = 0;

   virtual EncConstIterator  *Clone() const = 0;

protected:
   const std::vector<unsigned char> &m_str;
   uint32_t                         m_index;
};

// ASCII_ConstIterator
//
// A constant forwards iterator for ASCII encoded strings.
//
class ASCII_ConstIterator : public EncConstIterator
{
public:
   ASCII_ConstIterator(const std::vector<unsigned char> &str, uint32_t index = 0) :
      EncConstIterator(str, index)
   {}

   virtual uint32_t operator*() const;
   virtual EncConstIterator &operator++();

   virtual EncConstIterator  *Clone() const { return new ASCII_ConstIterator(m_str, m_index); }
};


// UTF_8_ConstIterator
//
// A constant forwards iterator for UTF_8 encoded strings.
//
class UTF_8_ConstIterator : public EncConstIterator
{
public:
   UTF_8_ConstIterator(const std::vector<unsigned char> &str, uint32_t index = 0) :
      EncConstIterator(str, index)
   {}

   virtual uint32_t operator*() const;
   virtual EncConstIterator &operator++();

   virtual EncConstIterator  *Clone() const { return new UTF_8_ConstIterator(m_str, m_index); }

private:
   void SkipToNext();
};

class EncodingConstIterator
{
public:
   EncodingConstIterator(const std::vector<unsigned char> &str, uint32_t index, eStringEncoding encoding) :
      m_encoding(encoding)
   {
      switch (encoding)
      {
      case eASCII:
         m_impl = new ASCII_ConstIterator(str, index);
         break;

      case eUTF_8:
         m_impl = new UTF_8_ConstIterator(str, index);
         break;
      }
   }

   EncodingConstIterator(const EncodingConstIterator &rhs) :
      m_encoding(rhs.m_encoding),
      m_impl(rhs.m_impl->Clone())
   {
   }

   ~EncodingConstIterator()
   {
      delete m_impl;
   }

   static EncodingConstIterator Begin(const std::vector<unsigned char> &str, eStringEncoding encoding)
   {
      return EncodingConstIterator(str, 0, encoding);
   }

   static EncodingConstIterator End(const std::vector<unsigned char> &str, eStringEncoding encoding)
   {
      return EncodingConstIterator(str, str.size(), encoding);
   }

   bool operator==(const EncodingConstIterator &rhs) const
   {
      return *m_impl == *rhs.m_impl && m_encoding == rhs.m_encoding;
   }

   bool operator!=(const EncodingConstIterator &rhs) const
   {
      return !(*m_impl == *rhs.m_impl) || m_encoding != rhs.m_encoding;
   }

   uint32_t operator*() const           { return m_impl->operator*();           }
   EncodingConstIterator &operator++()  { m_impl->operator++();  return *this;  }

   EncodingConstIterator &operator=(const EncodingConstIterator &rhs)
   {
      if (this != &rhs)
      {
         m_encoding = rhs.m_encoding;
         m_impl = rhs.m_impl->Clone();
      }

      return *this;
   }

private:
   eStringEncoding   m_encoding;
   EncConstIterator  *m_impl;
};

// @endcond

}

#endif
