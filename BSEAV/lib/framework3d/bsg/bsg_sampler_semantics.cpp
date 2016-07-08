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

#include "bsg_sampler_semantics.h"
#include "bsg_parse_utils.h"
#include "bsg_exception.h"

#include <string>
#include <sstream>

#include <stdint.h>

using namespace std;

namespace bsg
{

void GetToken(istringstream &iss, string &token, char delim)
{
   getline(iss, token, delim);

   token = ParseUtils::StripWhite(token);
}

GLenum StringToWrap(const string &s)
{
   if (s == "CLAMP")
      return GL_CLAMP_TO_EDGE;

   if (s == "REPEAT")
      return GL_REPEAT;

   if (s == "MIRROR")
      return GL_MIRRORED_REPEAT;

   return GL_FALSE;
}

GLenum StringToFilter(const string &s)
{
   if (s == "NEAREST")
      return GL_NEAREST;

   if (s == "LINEAR")
      return GL_LINEAR;

   if (s == "NEAREST_MIPMAP_NEAREST")
      return GL_NEAREST_MIPMAP_NEAREST;

   if (s == "NEAREST_MIPMAP_LINEAR")
      return GL_NEAREST_MIPMAP_LINEAR;

   if (s == "LINEAR_MIPMAP_NEAREST")
      return GL_LINEAR_MIPMAP_NEAREST;

   if (s == "LINEAR_MIPMAP_LINEAR")
      return GL_LINEAR_MIPMAP_LINEAR;

   return GL_FALSE;
}

static GLenum Target(SamplerSemantics::eType target)
{
   switch (target)
   {
   case SamplerSemantics::eSAMPLER_2D :
      return GL_TEXTURE_2D;

#ifdef BSG_USE_ES3
   case SamplerSemantics::eSAMPLER_3D :
      return GL_TEXTURE_3D;

   case SamplerSemantics::eSAMPLER_2D_ARRAY :
      return GL_TEXTURE_2D_ARRAY;
#endif

   case SamplerSemantics::eSAMPLER_CUBE :
      return GL_TEXTURE_CUBE_MAP;

   default: break;
   }

   return GL_FALSE;
}

void SamplerSemantics::Parse(const string &body, SamplerSemantics::eType target, const string &name)
{
   istringstream  iss(body);
   string         token;
   string         equal;
   string         semi;

   int32_t  unitValue         = -1;
   GLenum   targetValue       = Target(target);
   GLenum   wrapSValue        = GL_FALSE;
   GLenum   wrapTValue        = GL_FALSE;
   GLenum   minFilterValue    = GL_FALSE;
   GLenum   magFilterValue    = GL_FALSE;

   while (!iss.eof())
   {
      GetToken(iss, token, '=');

      if (token == "Unit")
      {
         string unit;
         GetToken(iss, unit, ';');

         istringstream ss(unit);
         ss >> unitValue;
      }
      else if (token == "Wrap")
      {
         string wrapS;
         string wrapT;

         GetToken(iss, wrapS, ',');
         wrapSValue = StringToWrap(wrapS);

         GetToken(iss, wrapT, ';');
         wrapTValue = StringToWrap(wrapT);
      }
      else if (token == "Filter")
      {
         string minFilter;
         string magFilter;

         GetToken(iss, minFilter, ',');
         minFilterValue = StringToFilter(minFilter);

         GetToken(iss, magFilter, ';');
         magFilterValue = StringToFilter(magFilter);
      }
   }

   if (unitValue < 0)
      BSG_THROW("Texture unit incorrect or missing");

   if (wrapSValue == GL_FALSE)
      BSG_THROW("Texture Wrap S incorrect or missing");

   if (wrapTValue == GL_FALSE)
      BSG_THROW("Texture Wrap T incorrect or missing");

   if (minFilterValue == GL_FALSE)
      BSG_THROW("Texture MinFilter incorrect or missing");

   if (magFilterValue == GL_FALSE)
      BSG_THROW("Texture MagFilter incorrect or missing");

   this->m_semantics[name] = GLSamplerState(unitValue, targetValue, wrapSValue, wrapTValue, minFilterValue, magFilterValue);
}

}
