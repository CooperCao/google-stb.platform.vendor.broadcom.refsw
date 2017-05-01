/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BCM_BACKDROP_H__
#define __BCM_BACKDROP_H__

#include "bsg_application.h"
#include "bsg_animatable.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"
#include "bsg_animation_sequence.h"
#include "bsg_color.h"

#include <limits.h>

class BCMBackdrop
{
public:
   enum Corner
   {
      eBCM_BL = 0,
      eBCM_BR,
      eBCM_TL,
      eBCM_TR
   };

   //! numSquaresY relates to whole display, yBottom & yTop set the actual display range.
   BCMBackdrop(uint32_t numSquaresY, float yBottom, float yTop, uint32_t yBlankBottom, uint32_t yBlankTop,
               bool cycleColors = true, float zPos = 0.99f, int32_t sortPriority = INT_MIN);

   //! Destructor
   virtual ~BCMBackdrop();

   //! Call during the Application frame update to update the animations.
   virtual bool UpdateAnimationList(bsg::Time t);

   //! Handle resize event
   virtual void ResizeHandler(uint32_t width, uint32_t height);

   //! Return the handle to the root of the physical nodes (use to add the carousel to a graph,
   //! or for inspection of the graph)
   virtual const bsg::SceneNodeHandle RootNode();

   //! Add a central black panel with a colored corner fade
   void AddCenterPanel(float bottom, float top, Corner coloredCorner = eBCM_BR, Corner colorToUse = eBCM_TR);

   //! Add a new color panel
   void AddShadedPanel(float bottom, float top);
   void AddShadedPanel(float bottom, float top, const bsg::Vec3 &blCol, const bsg::Vec3 &brCol, const bsg::Vec3 &tlCol, const bsg::Vec3 &trCol);

   //! Return the backdrop colors
   bsg::Vec3 Blues(Corner corner) const;
   bsg::Vec3 Greens(Corner corner) const;
   bsg::Vec3 Purples(Corner corner) const;

   //! Return the current color value
   bsg::Vec3 GetCurrentColor(Corner corner) const;

private:
   bsg::GeometryHandle CreateMyQuad(const bsg::Vec2 &bl, const bsg::Vec2 &tr, float offset, const bsg::Vec2 &tcMult, bsg::MaterialHandle material);
   bsg::SurfaceHandle CreateMyQuad(const bsg::Vec2 &bl, const bsg::Vec2 &tr, float offset, const bsg::Vec2 &tcMult);

private:
   bsg::SceneNodeHandle   m_root;
   bsg::AnimationList     m_animList;
   bsg::AnimationSequence m_colorSequence[4];
   bsg::EffectHandle      m_gradientEffect;
   bsg::MaterialHandle    m_backdropMat;
   bsg::MaterialHandle    m_gradientMat;
   bsg::MaterialHandle    m_centerMat;
   bsg::Vec2              m_windowSizeDiv;

   bsg::Vec3              m_blues[4];
   bsg::Vec3              m_greens[4];
   bsg::Vec3              m_purples[4];

   std::vector<bsg::AnimatableVec3> m_currentColors;
   float                       m_zPos;
   Corner                      m_coloredCorner;
   Corner                      m_colorToUse;
   bool                        m_cycleColors;
   int32_t                     m_sortPriority;
};

#endif /* __BCM_BACKDROP_H__ */
