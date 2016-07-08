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

#include "bsg_effect_semantics.h"
#include "bsg_parse_utils.h"

using namespace std;

namespace bsg
{

#define MAP(A) m_parseMap.insert(pair<string, eSemantic>(#A, e##A))

bool EffectSemantics::m_parseMapInited = false;
std::map<std::string, EffectSemantics::eSemantic> EffectSemantics::m_parseMap;

EffectSemantics::EffectSemantics()
{
   if (!m_parseMapInited)
   {
      MAP(MATRIX4_MODEL);
      MAP(MATRIX4_VIEW);
      MAP(MATRIX4_PROJECTION);
      MAP(MATRIX4_MODEL_VIEW);
      MAP(MATRIX4_VIEW_PROJECTION);
      MAP(MATRIX4_MODEL_VIEW_PROJECTION);
      MAP(MATRIX3_INVT_MODEL);
      MAP(MATRIX3_INVT_VIEW);
      MAP(MATRIX3_INVT_MODEL_VIEW);
      MAP(SCALAR_OPACITY);
      MAP(SCALAR_USER);
      MAP(VECTOR2_USER);
      MAP(VECTOR3_USER);
      MAP(VECTOR4_USER);
      MAP(MATRIX2_USER);
      MAP(MATRIX3_USER);
      MAP(MATRIX4_USER);
#ifdef BSG_USE_ES3
      MAP(MATRIX2x3_USER);
      MAP(MATRIX2x4_USER);
      MAP(MATRIX3x2_USER);
      MAP(MATRIX3x4_USER);
      MAP(MATRIX4x2_USER);
      MAP(MATRIX4x3_USER);
#endif
      MAP(VECTOR4_SCREEN_SIZE);
      MAP(VECTOR4_QUAD_OFFSET);
      MAP(VATTR_POSITION);
      MAP(VATTR_NORMAL);
      MAP(VATTR_TANGENT);
      MAP(VATTR_BINORMAL);
      MAP(VATTR_TEXCOORD1);
      MAP(VATTR_TEXCOORD2);
      MAP(VATTR_TEXCOORD3);
      MAP(VATTR_COLOR);
      MAP(VATTR_USER1);
      MAP(VATTR_USER2);
      MAP(VATTR_USER3);
      MAP(VATTR_USER4);
      MAP(VATTR_USER5);
      MAP(VATTR_USER6);
  }

   m_parseMapInited = true;
}

void EffectSemantics::ParseLine(const std::string &semStr)
{
   int equal = semStr.find_first_of("=");
   string l = ParseUtils::StripWhite(semStr.substr(0, equal - 1));
   string r = ParseUtils::StripWhite(semStr.substr(equal + 1, semStr.npos));

   SetSemantic(l, MatchSemanticName(r));
}

void EffectSemantics::Parse(const std::string &semStr)
{
   int start = 0;
   int semi;

   do
   {
      semi = semStr.find_first_of(";", start);
      if (semi != (int)semStr.npos)
         ParseLine(semStr.substr(start, semi - start));

      start = semi + 1;
   }
   while (semi != (int)semStr.npos);
}

EffectSemantics::eSemantic EffectSemantics::MatchSemanticName(const std::string &name)
{
   std::map<std::string, EffectSemantics::eSemantic>::iterator iter;
   iter = m_parseMap.find(name);

   if (iter != m_parseMap.end())
      return iter->second;

   return eUNKNOWN_SEMANTIC;
}

void EffectSemantics::SetSemantic(const std::string &str, eSemantic sem)
{
   m_semantics.insert(pair<string, eSemantic>(str, sem));
}

EffectSemantics::eSemantic EffectSemantics::GetSemantic(const std::string &str) const
{
   std::map<std::string, EffectSemantics::eSemantic>::const_iterator iter;
   iter = m_semantics.find(str);

   if (iter != m_semantics.end())
      return iter->second;

   return eUNKNOWN_SEMANTIC;
}

std::string ToString(EffectSemantics::eSemantic semantic)
{
#define CASE(X)   case EffectSemantics::e##X: return #X;

   switch (semantic)
   {
      CASE(UNKNOWN_SEMANTIC)
      CASE(MATRIX4_MODEL)
      CASE(MATRIX4_VIEW)
      CASE(MATRIX4_PROJECTION)
      CASE(MATRIX4_MODEL_VIEW)
      CASE(MATRIX4_VIEW_PROJECTION)
      CASE(MATRIX4_MODEL_VIEW_PROJECTION)
      CASE(MATRIX3_INVT_MODEL)
      CASE(MATRIX3_INVT_VIEW)
      CASE(MATRIX3_INVT_MODEL_VIEW)
      CASE(SCALAR_OPACITY)
      CASE(SCALAR_USER)
      CASE(VECTOR2_USER)
      CASE(VECTOR3_USER)
      CASE(VECTOR4_USER)
      CASE(MATRIX2_USER)
      CASE(MATRIX3_USER)
      CASE(MATRIX4_USER)
#ifdef BSG_USE_ES3
      CASE(MATRIX2x3_USER);
      CASE(MATRIX2x4_USER);
      CASE(MATRIX3x2_USER);
      CASE(MATRIX3x4_USER);
      CASE(MATRIX4x2_USER);
      CASE(MATRIX4x3_USER);
#endif
      CASE(VECTOR4_SCREEN_SIZE)
      CASE(VECTOR4_QUAD_OFFSET)
      CASE(VATTR_POSITION)
      CASE(VATTR_NORMAL)
      CASE(VATTR_TANGENT)
      CASE(VATTR_BINORMAL)
      CASE(VATTR_TEXCOORD1)
      CASE(VATTR_TEXCOORD2)
      CASE(VATTR_TEXCOORD3)
      CASE(VATTR_COLOR)
      CASE(VATTR_USER1)
      CASE(VATTR_USER2)
      CASE(VATTR_USER3)
      CASE(VATTR_USER4)
      CASE(VATTR_USER5)
      CASE(VATTR_USER6)
   }

   return "";
}


}
