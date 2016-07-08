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
