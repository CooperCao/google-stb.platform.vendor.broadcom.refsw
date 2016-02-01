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

#ifndef __VIDEO_TEXTURING_DECODER_H__
#define __VIDEO_TEXTURING_DECODER_H__

#include "bsg_application.h"
#include "bsg_animatable.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"
#include "bsg_animation_sequence.h"
#include "bsg_video_decoder.h"

#include "../common/bcm_backdrop.h"
#include "../common/bcm_guilloche.h"
#include "../common/bcm_help_menu.h"

#include "menu_carousel.h"

class MenuItem
{
public:
   std::string text;
   std::string bfxFile;
};

class BCMGuilloche
{
public:
   BCMGuilloche(bsg::Application &app);

   void Render();
   void UpdateAnimationList(const bsg::Time &now);

private:
   bsg::Application     &m_app;
   uint32_t             m_numControls;
   Guilloche            m_guilloche;
   GuillochePanel       m_guillochePanel;
   bsg::AnimationList   m_animList;
};

class EffectCache
{
public:
   void Load(const std::vector<MenuItem> &items);

   const bsg::EffectHandle &GetEffect(uint32_t indx) const;

private:
   std::vector<bsg::EffectHandle>  m_cache;
};

class VideoTexturingApp;

class FullScreenAnimDone : public bsg::AnimationDoneNotifier
{
public:
   FullScreenAnimDone(VideoTexturingApp *app) : m_app(app) {}

protected:
   virtual void Notify(const bsg::Time &time);

private:
   VideoTexturingApp *m_app;
};

class VideoTexturingApp : public bsg::Application
{
public:
   enum
   {
      MaxDecodeBuffers = 3
   };

public:
   VideoTexturingApp(bsg::Platform &platform);
   ~VideoTexturingApp();

   friend class FullScreenAnimDone;

private:
   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void MouseEventHandler(bsg::MouseEvents &queue);

   virtual bool UpdateFrame(int32_t *idleMs); 
   virtual void ResizeHandler(uint32_t width, uint32_t height);

   void DrawTime(const bsg::Vec2 &pos);
   bsg::AnimBindingBase *GenerateNextAnim(bsg::Transform &next, bsg::SceneNodeHandle node);
   bool LoadMenu(const std::string &file);
   void ChangeFilter(uint32_t indx);
   void DoBenchmarking();

   void ToggleFullScreenVideo();
   void StartNonFullScreenTransition();

private:
   bsg::SceneNodeHandle   m_root;
   bsg::SceneNodeHandle   m_cameraNode;
   bsg::SceneNodeHandle   m_monitorCameraPosNode;
   bsg::SceneNodeHandle   m_monitorLookatNode;
   bsg::SceneNodeHandle   m_rootLookatNode;
   bsg::SceneNodeHandle   m_floatingCameraPosNode;
   bsg::SceneNodeHandle   m_floatingLookatNode;
   bsg::SceneNodeHandle   m_floatingCameraNode;
   bsg::SceneNodeHandle   m_vidNode;
   bsg::SceneNodeHandle   m_xformNode;
   bsg::SceneNodeHandle   m_bgNode;
   bsg::SceneNodeHandle   m_carouselRoot;
   bsg::CameraHandle      m_floatingCamera;

   bsg::ConstraintLerpPositionHandle   m_cameraPosConstraint;
   bsg::ConstraintLerpPositionHandle   m_cameraLookatConstraint;

   bsg::SceneNodeHandle   m_carouselCamNode;
   bsg::CameraHandle      m_carouselCamera;

   bsg::AnimationList     m_animList;
   bsg::AnimationSequence m_sequence;
   bsg::VideoDecoder      *m_videoDecoder;

   bsg::FontHandle         m_font;
   bsg::FontHandle         m_smallFont;
   bsg::FontHandle         m_midFont;

   bsg::Transform          m_curTransform;

   bsg::Auto<MenuCarousel> m_carousel;

   std::vector<MenuItem>            m_menuItems;
   EffectCache                      m_effectCache;
   bsg::EffectHandle                m_videoEffect;
   bsg::EffectHandle                m_rawVideoEffect;
   std::vector<bsg::MaterialHandle> m_videoMat;

   bsg::AnimatableFloat   m_animFloat;
   bsg::AnimatableFloat   m_animFadeOpacity;

   bsg::Time         m_lastKeyPressTime;
   bool              m_idling;

   BCMBackdrop       m_backdrop;
   BCMGuilloche      m_guilloche;

   HelpMenu          *m_helpMenu;

   bool                 m_fullScreen;
   FullScreenAnimDone   m_fullScreenAnimDone;
   int32_t              m_transitionCountdown;
   bool                 m_stopRendering;
   bool                 m_inTransition;
};


#endif /* __VIDEO_TEXTURING_DECODER_H__ */
