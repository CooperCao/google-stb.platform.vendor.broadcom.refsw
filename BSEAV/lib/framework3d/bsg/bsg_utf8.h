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

