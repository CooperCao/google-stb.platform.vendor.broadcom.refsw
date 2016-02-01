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

#include "bsg_quad_render.h"
#include "bsg_application.h"

#include <algorithm>

namespace bsg
{

// Calculate offsets and viewports for tiled rendering
//
QuadRender::QuadRender(const IVec2 &sz) :
   m_x(sz.X()),
   m_y(sz.Y()),
   m_vpX(0),
   m_vpY(0),
   m_vpWidth(Application::Instance()->GetWindowWidth()),
   m_vpHeight(Application::Instance()->GetWindowHeight()),
   m_virtualLeft(0),
   m_virtualRight(0),
   m_virtualBottom(0),
   m_virtualTop(0),
   m_clippedLeft(0),
   m_clippedRight(0),
   m_clippedBottom(0),
   m_clippedTop(0),
   m_panel(0)
{
}

void QuadRender::SetPanel(uint32_t p)
{
   m_panel = p;
}

void QuadRender::Viewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
   // Record requested size
   m_vpX      = x;
   m_vpY      = y;
   m_vpWidth  = width;
   m_vpHeight = height;

   int32_t scrWidth  = Application::Instance()->GetWindowWidth();
   int32_t scrHeight = Application::Instance()->GetWindowHeight();

   int32_t  xo = m_panel % m_x;
   int32_t  yo = m_panel / m_x;

   // The viewport scaled to the expanded screen size
   m_virtualLeft   = x * m_x;
   m_virtualRight  = (x + width  - 1) * m_x;
   m_virtualBottom = y * m_y;
   m_virtualTop    = (y + height - 1) * m_y;

   // The bounds of the current quad in expanded screen space
   int32_t  clipLeft   = xo * (scrWidth - 1);
   int32_t  clipBottom = yo * (scrHeight - 1);
   int32_t  clipRight  = clipLeft   + (scrWidth  - 1);
   int32_t  clipTop    = clipBottom + (scrHeight - 1);

   // The viewport clipped to the current quad
   m_clippedLeft   = std::max(m_virtualLeft,   clipLeft);
   m_clippedRight  = std::min(m_virtualRight,  clipRight);
   m_clippedBottom = std::max(m_virtualBottom, clipBottom);
   m_clippedTop    = std::min(m_virtualTop,    clipTop);

   // Really set the viewport
   ::glViewport(m_clippedLeft   - clipLeft,
                m_clippedBottom - clipBottom,
                std::max(0, m_clippedRight  - m_clippedLeft   + 1),
                std::max(0, m_clippedTop    - m_clippedBottom + 1));
}

void QuadRender::MakeProjectionMatrix(Mat4 *proj, const Camera *cam) const
{
   cam->MakeQuadProjectionMatrix(proj,
                                 (float)(m_clippedLeft   - m_virtualLeft)   / m_vpWidth  / m_x,  // The parametric distance across the
                                 (float)(m_clippedRight  - m_virtualLeft)   / m_vpWidth  / m_x,  // viewport for l, r, t, b.
                                 (float)(m_clippedBottom - m_virtualBottom) / m_vpHeight / m_y,
                                 (float)(m_clippedTop    - m_virtualBottom) / m_vpHeight / m_y,
                                 (float)m_x / m_y);
}

Vec4 QuadRender::GetOffset() const
{
   return Vec4(float(m_x), float(m_y), (m_x - 1.0f) - 2.0f * (m_panel % m_x), (m_y - 1.0f) - 2.0f * (m_panel / m_x));
}

IVec2 QuadRender::GetDimensions() const
{
   return IVec2(m_x, m_y);
}

}
