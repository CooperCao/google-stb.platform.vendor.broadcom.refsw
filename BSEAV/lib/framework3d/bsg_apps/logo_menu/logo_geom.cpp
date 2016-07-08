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

#include "logo_text.c"
#include "logo_pulse.c"
#include "logo_geom.h"
#include "logo_menu.h"
#include "bsg_geometry.h"
#include "bsg_surface.h"
#include "bsg_material.h"

using namespace bsg;

static uint32_t Offset(uint32_t n)
{
   return sizeof(float) * n;
}

GeometryHandle BroadcomLogo::InstanceLogo(MaterialHandle textMat, MaterialHandle pulseMat, bool reflected)
{
   GeometryHandle geom(New);

   if (m_textSurfHandle.IsNull())
   {
      // Make the text geom
      m_textSurfHandle = SurfaceHandle(New);

      m_textSurfHandle->SetDraw(GL_TRIANGLE_STRIP, sizeof(g_index_logo_text) / sizeof(uint16_t),
                                                   sizeof(g_vert_logo_text), g_vert_logo_text,
                                                   sizeof(g_index_logo_text), g_index_logo_text);

      m_textSurfHandle->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(0)));
      m_textSurfHandle->SetPointer(EffectSemantics::eVATTR_NORMAL,    GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(3)));
      m_textSurfHandle->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, 8 * sizeof(float), Offset(6)));

      m_textSurfHandle->SetBound(Bound(sqrtf((4 * 4) + (2 * 2) + (2 * 2)) , Vec3(0.0f, 0.0f, 0.0f)));
   }

   geom->AppendSurface(m_textSurfHandle,  textMat);

   if (m_pulseSurfHandle.IsNull())
   {
      m_pulseSurfHandle = SurfaceHandle(New);

      m_pulseSurfHandle->SetDraw(GL_TRIANGLE_STRIP, sizeof(g_index_logo_pulse) / sizeof(uint16_t),
                                                    sizeof(g_vert_logo_pulse),  g_vert_logo_pulse,
                                                    sizeof(g_index_logo_pulse), g_index_logo_pulse);

      m_pulseSurfHandle->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(0)));
      m_pulseSurfHandle->SetPointer(EffectSemantics::eVATTR_NORMAL,    GLVertexPointer(3, GL_FLOAT, 8 * sizeof(float), Offset(3)));
      m_pulseSurfHandle->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, 8 * sizeof(float), Offset(6)));

      m_pulseSurfHandle->SetBound(Bound(sqrtf((4 * 4) + (2 * 2) + (2 * 2)) , Vec3(0.0f, 0.0f, 0.0f)));
   }

   geom->AppendSurface(m_pulseSurfHandle, pulseMat);

   geom->SetReflected(reflected);

   return geom;
}
