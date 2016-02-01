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

#ifndef __BSG_COLOR_H__
#define __BSG_COLOR_H__

#include "bsg_common.h"

#include <memory.h>
#include <stdint.h>

namespace bsg
{

// @cond
class Color
{
public:
   Color() { memset(m_color, 0, 4); }
   Color(uint8_t color[4]) { memcpy(m_color, color, 4); }
   Color(uint8_t r, uint8_t g, uint8_t b) { m_color[0] = r; m_color[1] = g; m_color[2] = b; m_color[3] = 255; }
   Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { m_color[0] = r; m_color[1] = g; m_color[2] = b; m_color[3] = a; }
   Color(float r, float g, float b) { m_color[0] = (uint8_t)(r * 255.0f); m_color[1] = (uint8_t)(g * 255.0); m_color[2] = (uint8_t)(b * 255.0f); m_color[3] = 255; }
   Color(float r, float g, float b, float a) { m_color[0] = (uint8_t)(r * 255.0f); m_color[1] = (uint8_t)(g * 255.0); m_color[2] = (uint8_t)(b * 255.0f); m_color[3] = (uint8_t)(a * 255.0f); }

   uint8_t R() const { return m_color[0]; }
   uint8_t G() const { return m_color[1]; }
   uint8_t B() const { return m_color[2]; }
   uint8_t A() const { return m_color[3]; }

   float FR() const { return (float)m_color[0] / 255.0f; }
   float FG() const { return (float)m_color[1] / 255.0f; }
   float FB() const { return (float)m_color[2] / 255.0f; }
   float FA() const { return (float)m_color[3] / 255.0f; }

   const uint8_t *Vec() const { return m_color; }
   uint8_t *Vec() { return m_color; }

   const float *VecF() const
   {
      m_colorF[0] = FR();
      m_colorF[1] = FG();
      m_colorF[2] = FB();
      m_colorF[3] = FA();
      return m_colorF; 
   }
   float *VecF()
   {
      m_colorF[0] = FR();
      m_colorF[1] = FG();
      m_colorF[2] = FB();
      m_colorF[3] = FA();
      return m_colorF; 
   }

private:
   uint8_t        m_color[4];
   mutable float  m_colorF[4];
};
// @endcond
}

#endif /* __BSG_COLOR_H__ */

