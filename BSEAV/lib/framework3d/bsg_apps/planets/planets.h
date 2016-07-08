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

#ifndef __PLANETS_H__
#define __PLANETS_H__

#include "bsg_application.h"
#include "bsg_animation_list.h"
#include "bsg_animator.h"
#include "bsg_print.h"

#include "command.h"

class BodyBuilder;
class HelpMenu;

class ParseArgs : public bsg::ArgumentParser
{
public:
   ParseArgs() :
      m_loop(false),
      m_useLOD(false),
      m_demo(false),
      m_tilt(false),
      m_overnightTests(false),
      m_geomResolution(50)
   {}

   virtual bool ParseArgument(const std::string &arg)
   {
      typedef bsg::ApplicationOptions Opt;

      return Opt::FlagMatch(&m_loop, arg.c_str(), "loop")          ||
             Opt::FlagMatch(&m_useLOD, arg.c_str(), "lod")         ||
             Opt::FlagMatch(&m_demo, arg.c_str(), "demo")          ||
             Opt::FlagMatch(&m_tilt, arg.c_str(), "tilt")          ||
             Opt::FlagMatch(&m_overnightTests, arg.c_str(), "old") ||
             Opt::UIntMatch(&m_geomResolution, arg.c_str(), "geom=%d");
   }

   virtual std::string UsageString() const
   {
      return "\n"
             "+loop     repeat script indefinitely\n"
             "+demo     demo mode -- quit on exit button\n"
             "+lod      use LOD scheme for planets\n"
             "+tilt     use axial and orbit tilts\n"
             "geom=X    set resultion of geometry (default 50)\n"
             "+old      Remove changes for overnight test consistency";
   }

   bool     GetLoop()                const { return m_loop;           }
   void     SetLoop(bool loop)             { m_loop = loop;           }
   bool     GetUseLOD()              const { return m_useLOD;         }
   bool     GetDemo()                const { return m_demo;           }
   bool     GetTilt()                const { return m_tilt;           }
   bool     GetOvernightTestMode()   const { return m_overnightTests; }
   uint32_t GetGeomResolution()      const { return m_geomResolution; }

private:
   bool     m_loop;
   bool     m_useLOD;
   bool     m_demo;
   bool     m_tilt;
   bool     m_overnightTests;
   uint32_t m_geomResolution;
};

class PlanetEffects
{
public:
   PlanetEffects(uint32_t geomResolution);

   bsg::EffectHandle      GetSurfaceEffect()       const { return m_surfaceEffect;       }
   bsg::EffectHandle      GetSurfaceCloudsEffect() const { return m_surfaceCloudsEffect; }
   bsg::EffectHandle      GetRingEffect()          const { return m_ringEffect;          }
   bsg::EffectHandle      GetAtmosphereEffect()    const { return m_atmosphereEffect;    }
   bsg::EffectHandle      GetStarEffect()          const { return m_starEffect;          }
   bsg::EffectHandle      GetSunEffect()           const { return m_sunEffect;           }

   bsg::SurfaceHandle     GetSphereSurface()       const { return m_sphereSurface;       }
   bsg::SurfaceHandle     GetSmallSphereSurface()  const { return m_smallSphereSurface;  }

private:
   bsg::EffectHandle   m_surfaceEffect;
   bsg::EffectHandle   m_surfaceCloudsEffect;
   bsg::EffectHandle   m_ringEffect;
   bsg::EffectHandle   m_atmosphereEffect;
   bsg::EffectHandle   m_starEffect;
   bsg::EffectHandle   m_sunEffect;

   bsg::SurfaceHandle  m_sphereSurface;
   bsg::SurfaceHandle  m_smallSphereSurface;
};

class Info
{
public:
   Info();
   void Show(Planets &planets, const std::string &bodyName, bool on, const BodyBuilder &builder);

private:
   std::string          m_info;

   bsg::PrintFontHandle m_font;
   bsg::PrintHandle     m_print1;
   bsg::PrintHandle     m_print2;
   bsg::Vec4            m_onColour;
   bsg::Vec4            m_offColour;

   bsg::AnimatableGroup<bsg::Vec4> m_colourGroup;

   static const Body    *m_earth;
};

class Splash
{
public:
   Splash(bsg::FontHandle bigFont, bsg::FontHandle smallFont);
   void Init(Planets &app);
   void OnFrame(const Planets &app) const;

private:
   bsg::FontHandle      m_bigFont;
   bsg::FontHandle      m_smallFont;
   bsg::AnimatableFloat m_alpha;
};

class Planets : public bsg::Application
{
public:
   Planets(bsg::Platform &platform, const ParseArgs &options);
   ~Planets();

   virtual void RenderFrame();
   virtual bool UpdateFrame(int32_t *idleTimeMs);

   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);

   void AddAnimation(bsg::AnimBindingBase *anim)  { m_animList.Append(anim); }

   static Planets *Instance()
   {
      if (m_instance == NULL)
         BSG_THROW("Planets application has not been created.  No instance available");

      return m_instance;
   }

   void SetLightPos(const bsg::Vec3 &pos)
   {
      m_lightPos = pos;
   }

   void SetView(const bsg::Mat4 &view, const bsg::Mat4 &iview)
   {
      m_viewToWorld = view;
      m_worldToView = iview;
   }

   const bsg::Mat4 &GetWorldToView() const
   {
      return m_worldToView;
   }

   const bsg::Mat4 &GetViewToWorld() const
   {
      return m_viewToWorld;
   }

   bsg::Vec3 &GetUpVector()
   {
      return m_upVector;
   }

   bsg::SceneNodeHandle   GetCameraLookAt()   const { return m_cameraLookAt;    }
   bsg::SceneNodeHandle   GetCameraPosition() const { return m_cameraPosition;  }
   bsg::SceneNodeHandle   GetRoot()           const { return m_rootNode;        }
   bsg::SceneNodeHandle   GetTextRoot()       const { return m_textNode;        }

   const ParseArgs &GetExtraOptions() const { return m_options; }

   uint32_t *GetSync()
   {
      return &m_sync;
   }

   void ShowInfo(const std::string &body, bool status);

   void AddMaterial(const bsg::MaterialHandle &material)
   {
      m_litMaterials.push_back(material);
   }

   std::vector<bsg::MaterialHandle> &GetLitMaterials() { return m_litMaterials; }

private:
   void LoadScript();

private:
   bsg::SceneNodeHandle   m_rootNode;
   bsg::SceneNodeHandle   m_textNode;
   bsg::SceneNodeHandle   m_solarSystemNode;
   bsg::SceneNodeHandle   m_camNode;
   bsg::SceneNodeHandle   m_cameraLookAt;
   bsg::SceneNodeHandle   m_cameraPosition;

   bsg::CameraHandle      m_camera;

   PlanetEffects          m_effects;
   bsg::AnimationList     m_animList;

   bsg::Vec3              m_upVector;
   bsg::Vec3              m_lightPos;
   bsg::Mat4              m_viewToWorld;
   bsg::Mat4              m_worldToView;

   CommandQueue      m_commands;
   CommandProxy      m_command[2];

   bsg::FontHandle   m_font;
   bsg::FontHandle   m_bigFont;
   BodyBuilder       *m_bodyBuilder;
   uint32_t          m_sync;

   ParseArgs         m_options;
   Info              m_info;
   Splash            m_splash;

   std::vector<bsg::MaterialHandle>   m_litMaterials;

   static Planets    *m_instance;

   HelpMenu          *m_helpMenu;
};

#endif /* __PLANETS_H__ */
