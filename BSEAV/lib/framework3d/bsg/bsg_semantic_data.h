/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
