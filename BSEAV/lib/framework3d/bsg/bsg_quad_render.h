/******************************************************************************
 *   (c)2013 Broadcom Corporation
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

#ifndef __BSG_QUAD_RENDER_H__
#define __BSG_QUAD_RENDER_H__

#include "bsg_camera.h"

#include <stdint.h>

namespace bsg
{

class Mat4;

// Information required for rendering a quad view (for large panel sizes when h/w only supports
// small buffer sizes.
class QuadRender
{
public:
   QuadRender(const IVec2 &sz);

   void  SetPanel(uint32_t p);
   void  Viewport(int32_t x, int32_t y, int32_t width, int32_t height);

   void  MakeProjectionMatrix(Mat4 *proj, const Camera *cam) const;

   Vec4  GetOffset()       const;
   IVec2 GetDimensions()   const;

   float GetAspectXoverY() const { return float(m_x) / m_y;                }

   uint32_t GetNumPanels() const { return m_x * m_y;                       }

   uint32_t GetPanel()     const { return m_panel;                         }
   bool     IsFirst()      const { return m_panel == 0;                    }
   bool     IsLast()       const { return m_panel == (GetNumPanels() - 1); }

private:
   uint32_t m_x;
   uint32_t m_y;

   // Requested viewport dimensions
   int32_t   m_vpX;
   int32_t   m_vpY;
   int32_t   m_vpWidth;
   int32_t   m_vpHeight;

   int32_t   m_virtualLeft;
   int32_t   m_virtualRight;
   int32_t   m_virtualBottom;
   int32_t   m_virtualTop;

   int32_t   m_clippedLeft;
   int32_t   m_clippedRight;
   int32_t   m_clippedBottom;
   int32_t   m_clippedTop;

   uint32_t  m_panel;
};

}

#endif

