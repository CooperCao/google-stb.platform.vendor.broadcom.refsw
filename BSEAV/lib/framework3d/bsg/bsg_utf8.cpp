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
// Returns the current character, if valid or nullptr otherwise
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
// Returns the current character, if valid or nullptr otherwise
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
