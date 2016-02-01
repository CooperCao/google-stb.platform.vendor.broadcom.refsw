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

#ifndef __BSG_EFFECT_SEMANTICS_H__
#define __BSG_EFFECT_SEMANTICS_H__

#include "bsg_common.h"

#include <map>
#include <string>
#include <vector>
#include <stdint.h>

namespace bsg
{

// @cond

class EffectSemantics
{
public:
   enum eSemantic
   {
      eUNKNOWN_SEMANTIC = 0,
      eMATRIX4_MODEL,
      eMATRIX4_VIEW,
      eMATRIX4_PROJECTION,
      eMATRIX4_MODEL_VIEW,
      eMATRIX4_VIEW_PROJECTION,
      eMATRIX4_MODEL_VIEW_PROJECTION,
      eMATRIX3_INVT_MODEL,
      eMATRIX3_INVT_VIEW,
      eMATRIX3_INVT_MODEL_VIEW,
      eSCALAR_OPACITY,
      eSCALAR_USER,
      eVECTOR2_USER,
      eVECTOR3_USER,
      eVECTOR4_USER,
      eMATRIX2_USER,
      eMATRIX3_USER,
      eMATRIX4_USER,
#ifdef BSG_USE_ES3
      eMATRIX2x3_USER,
      eMATRIX2x4_USER,
      eMATRIX3x2_USER,
      eMATRIX3x4_USER,
      eMATRIX4x2_USER,
      eMATRIX4x3_USER,
#endif
      eVECTOR4_SCREEN_SIZE,
      eVECTOR4_QUAD_OFFSET,

      eVATTR_POSITION,
      eVATTR_NORMAL,
      eVATTR_TANGENT,
      eVATTR_BINORMAL,
      eVATTR_TEXCOORD1,
      eVATTR_TEXCOORD2,
      eVATTR_TEXCOORD3,
      eVATTR_COLOR,
      eVATTR_USER1,
      eVATTR_USER2,
      eVATTR_USER3,
      eVATTR_USER4,
      eVATTR_USER5,
      eVATTR_USER6,

      eVATTR_FIRST = eVATTR_POSITION,
      eVATTR_LAST  = eVATTR_USER6,
      eVATTR_COUNT = eVATTR_LAST - eVATTR_FIRST + 1
   };

public:
   EffectSemantics();

   void SetSemantic(const std::string &str, eSemantic sem);
   eSemantic GetSemantic(const std::string &str) const;

   //! Returns the required set of semantics
   const std::map<std::string, eSemantic> &RequiredSemantics() const { return m_semantics; }

   void Parse(const std::string &semStr);

private:
   static eSemantic MatchSemanticName(const std::string &name);
   void ParseLine(const std::string &semStr);

private:
   std::map<std::string, eSemantic>        m_semantics;

   static bool                             m_parseMapInited;
   static std::map<std::string, eSemantic> m_parseMap;
};

class AttributeSemantic
{
public:
   AttributeSemantic() :
      m_semantic(EffectSemantics::eUNKNOWN_SEMANTIC),
      m_index(0)
   {}

   AttributeSemantic(EffectSemantics::eSemantic s, uint32_t index) :
      m_semantic(s),
      m_index(index)
   {}

   EffectSemantics::eSemantic Semantic() const { return m_semantic;  }
   uint32_t                   Index()    const { return m_index;     }

private:
   EffectSemantics::eSemantic m_semantic;
   uint32_t                   m_index;
};

std::string ToString(EffectSemantics::eSemantic semantic);

// @endcond

typedef std::vector<AttributeSemantic>   AttributeSemantics;


}

#endif /* __BSG_EFFECT_SEMANTICS_H__ */

