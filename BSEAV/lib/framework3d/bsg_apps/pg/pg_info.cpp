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

#include "pg_info.h"

namespace pg
{

// Look for [tok, tok,...] patterns where tok can be
// AD, SL, S
void Description::ParseString(const std::string &desc)
{
   enum eState
   {
      eNORMAL,
      eINBRACKET,
      eFOUND_S,
      eFOUND_A
   };

   eState state = eNORMAL;

   for (uint32_t i = 0; i < desc.size(); ++i)
   {
      char  c = desc[i];

      switch (state)
      {
      case eNORMAL:
         if (c == '[')
            state = eINBRACKET;
         else
            m_result += c;
         break;

      case eINBRACKET:
         if (c == 'S')
            state = eFOUND_S;
         else if (c == 'A')
            state = eFOUND_A;
         break;

      case eFOUND_S:
         if (c == 'L')
            m_hasSigning = true;
         else
            m_hasSubtitles = true;

         state = eINBRACKET;
         break;

      case eFOUND_A:
         if (c == 'D')
         {
            m_hasAudioDescription = true;
         }
         state = eINBRACKET;
         break;
      }

      switch (state)
      {
      case eINBRACKET:
      case eFOUND_S:
      case eFOUND_A:
         if (c == ']')
            state = eNORMAL;
         break;

      case eNORMAL:
         break;
      }
   }
}

Description::Description() :
   m_hasAudioDescription(false),
   m_hasSubtitles(false),
   m_hasSigning(false)
{}

Description::Description(const std::string &desc) :
   m_hasAudioDescription(false),
   m_hasSubtitles(false),
   m_hasSigning(false)
{
   ParseString(desc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string ProgramInfo::StripTrailingDots(const std::string &str) const
{
   if (str == "")
      return "";

   std::string::const_iterator   last = str.end();

   --last;

   while (*last == '.')
   {
      if (last != str.begin())
         --last;
      else
         return "";
   }

   return std::string(str.begin(), last + 1);
}

uint32_t GridInfo::AddChannel(const ChannelInfo &channel)
{
   uint32_t chIndx = m_channels.size();
   m_channels.push_back(channel); 
   return chIndx;
}

}
