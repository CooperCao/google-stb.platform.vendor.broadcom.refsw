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
#include "bsg_semantic_data.h"
#include "bsg_application.h"
#include "bsg_exception.h"

namespace bsg
{

SemanticData::SemanticData() : 
   m_mvDirty(true),
   m_vpDirty(true),
   m_mvpDirty(true),
   m_imtDirty(true),
   m_ivtDirty(true),
   m_imvtDirty(true),
   m_opacity(1.0f)
{
}

float SemanticData::GetFloat(EffectSemantics::eSemantic semantic) const
{
   switch (semantic)
   {
   case EffectSemantics::eSCALAR_OPACITY :
      return GetOpacity();
      break;
   default :
      BSG_THROW("Invalid semantic");
      break;
   }

   return 1.0f;
}

const Vec4 &SemanticData::GetVec4(EffectSemantics::eSemantic semantic) const
{
   static Vec4 def;

   switch (semantic)
   {
   case EffectSemantics::eVECTOR4_SCREEN_SIZE :
      return GetScreenSize();
      break;

   case EffectSemantics::eVECTOR4_QUAD_OFFSET:
      return GetQuadOffset();
      break;

   default:
      BSG_THROW("Invalid semantic");
      break;
   }

   return def;
}

const Mat4 &SemanticData::GetMat4(EffectSemantics::eSemantic semantic) const
{
   static Mat4 def;

   switch (semantic)
   {
   case EffectSemantics::eMATRIX4_MODEL : 
      return m_modelMx;
      break;
   case EffectSemantics::eMATRIX4_VIEW : 
      return m_viewMx;
      break;
   case EffectSemantics::eMATRIX4_PROJECTION :
      return m_projMx;
      break;
   case EffectSemantics::eMATRIX4_MODEL_VIEW :
      return GetModelViewMatrix();
      break;
   case EffectSemantics::eMATRIX4_VIEW_PROJECTION :
      return GetViewProjectionMatrix();
      break;
   case EffectSemantics::eMATRIX4_MODEL_VIEW_PROJECTION :
      return GetModelViewProjectionMatrix();
      break;
   default :
      BSG_THROW("Invalid semantic");
      break;
   }

   return def;
}

const Mat3 &SemanticData::GetMat3(EffectSemantics::eSemantic semantic) const
{
   static Mat3 def;

   switch (semantic)
   {
   case EffectSemantics::eMATRIX3_INVT_MODEL :
      return GetInvTModelMatrix();
      break;
   case EffectSemantics::eMATRIX3_INVT_VIEW :
      return GetInvTViewMatrix();
      break;
   case EffectSemantics::eMATRIX3_INVT_MODEL_VIEW :
      return GetInvTModelViewMatrix();
      break;
   default :
      BSG_THROW("Invalid semantic");
      break;
   }

   return def;
}

const Vec4 &SemanticData::GetScreenSize() const
{
   const Application &app = *Application::Instance();

   float width  = (float)app.GetWindowWidth();
   float height = (float)app.GetWindowHeight();

   m_screenSize = Vec4(width, height, 1.0f / width, 1.0f / height);

   return m_screenSize;
}

const Vec4 &SemanticData::GetQuadOffset() const
{
   const Application &app = *Application::Instance();

   m_quadOffset = !app.IsQuad() ? Vec4(1.0f, 1.0f, 0.0f, 0.0f)    :
                                  app.GetQuadRender().GetOffset();

   return m_quadOffset;
}

void SemanticData::SetModelMatrix(const Mat4 &mx)
{
   if (m_modelMx != mx)
   {
      m_modelMx = mx;
      m_mvDirty  = true;
      m_mvpDirty = true;
      m_imtDirty  = true;
      m_ivtDirty  = true;
      m_imvtDirty = true;
   }
}

void SemanticData::SetViewMatrix(const Mat4 &mx)
{
   if (m_viewMx != mx)
   {
      m_viewMx    = mx;
      m_mvDirty   = true;
      m_vpDirty   = true;
      m_mvpDirty  = true;
      m_ivtDirty  = true;
      m_imvtDirty = true;
   }
}

void SemanticData::SetModelViewMatrix(const Mat4 &mx)
{
   if (m_modelViewMx != mx)
   {
      m_modelViewMx = mx;
      m_mvpDirty    = true;
      m_imvtDirty   = true;
   }
}

void SemanticData::SetProjMatrix(const Mat4 &mx)
{
   if (m_projMx != mx)
   {
      m_projMx   = mx;
      m_vpDirty  = true;
      m_mvpDirty = true;
   }
}

}

