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

#ifndef __BSG_SEMANTIC_DATA_H__
#define __BSG_SEMANTIC_DATA_H__

#include "bsg_common.h"
#include "bsg_effect_semantics.h"
#include "bsg_matrix.h"

#include <stdio.h>

namespace bsg
{

// @cond
class SemanticData
{
public:
   SemanticData();

   float GetOpacity() const { return m_opacity; }
   void SetOpacity(const float &val) { m_opacity = val; }

   const Mat4 &GetMat4(EffectSemantics::eSemantic semantic) const;
   const Mat3 &GetMat3(EffectSemantics::eSemantic semantic) const;
   const Vec4 &GetVec4(EffectSemantics::eSemantic semantic) const;

   float GetFloat(EffectSemantics::eSemantic semantic) const;

   void SetModelMatrix(const Mat4 &mx);

   //! Warning -- by setting the modelview directly, you imply that the model and view
   //!            have been set appropriately
   void SetModelViewMatrix(const Mat4 &mx);

   //! Set the view matrix
   void SetViewMatrix(const Mat4 &mx);

   //! Set the projection matrix
   void SetProjMatrix(const Mat4 &mx);

   const Mat4 &GetModelMatrix() const
   {
      return m_modelMx;
   }

   const Mat4 &GetProjMatrix() const
   {
      return m_projMx;
   }

   const Mat4 &GetModelViewMatrix() const
   {
      if (m_mvDirty)
      {
         m_modelViewMx = m_viewMx * m_modelMx;
         m_mvDirty = false;
      }
      return m_modelViewMx;
   }

   const Mat4 &GetViewProjectionMatrix() const
   {
      if (m_vpDirty)
      {
         m_viewProjMx = m_projMx * m_viewMx;
         m_vpDirty = false;
      }
      return m_viewProjMx;
   }

   const Mat4 &GetModelViewProjectionMatrix() const
   {
      if (m_mvpDirty)
      {
         m_modelViewProjMx = m_projMx * GetModelViewMatrix();
         m_mvpDirty = false;
      }
      return m_modelViewProjMx;
   }

   const Mat3 &GetInvTModelMatrix() const
   {
      if (m_imtDirty)
      {
         m_invtModelMx = Invert(m_modelMx.Drop()).Transpose();
         m_imtDirty = false;
      }
      return m_invtModelMx;
   }

   const Mat3 &GetInvTViewMatrix() const
   {
      if (m_ivtDirty)
      {
         m_invtViewMx = Invert(m_viewMx.Drop()).Transpose();
         m_ivtDirty = false;
      }
      return m_invtViewMx;
   }

   const Mat3 &GetInvTModelViewMatrix() const
   {
      if (m_imvtDirty)
      {
         m_invtModelViewMx = Invert(GetModelViewMatrix().Drop()).Transpose();
         m_imvtDirty = false;
      }
      return m_invtModelViewMx;
   }

   const Vec4 &GetScreenSize() const;

private:
   mutable bool   m_mvDirty;
   mutable bool   m_vpDirty;
   mutable bool   m_mvpDirty;
   mutable bool   m_imtDirty;
   mutable bool   m_ivtDirty;
   mutable bool   m_imvtDirty;

   Mat4           m_modelMx;
   Mat4           m_viewMx;
   Mat4           m_projMx;
   mutable Mat4   m_modelViewMx;
   mutable Mat4   m_viewProjMx;
   mutable Mat4   m_modelViewProjMx;
   mutable Mat3   m_invtModelMx;
   mutable Mat3   m_invtViewMx;
   mutable Mat3   m_invtModelViewMx;

   float          m_opacity;

   mutable Vec4   m_screenSize;
};

// @endcond
}

#endif /* __BSG_SEMANTIC_DATA_H__ */

