/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __LOGO_MENU_H__
#define __LOGO_MENU_H__

#include "bsg_application.h"
#include "bsg_animatable.h"
#include "bsg_animator.h"
#include "bsg_font.h"
#include "bsg_text.h"

#include "../common/bcm_backdrop.h"
#include "../common/bcm_guilloche.h"
#include "../common/bcm_help_menu.h"

#include "menu_carousel.h"

namespace bsg
{
   class SceneNode;
   class Camera;
}

class MenuItem
{
public:
   std::string textureFile;
   std::string quickText;
   std::string desc;
   std::string folder;
   std::string exe;
   std::string args;
   bool        enabled;
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

class ParseArgs;

class LogoMenu : public bsg::Application
{
public:
   LogoMenu(bsg::Platform &platform, const ParseArgs &args);

   void WriteDemoScript(uint32_t current);

private:
   // Overridden methods
   virtual bool UpdateFrame(int32_t *idleMs);
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void MouseEventHandler(bsg::MouseEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);

   // Private methods
   bool LoadMenu(const std::string &file, uint32_t startAt);
   bsg::MaterialHandle MakeEnvMappedMaterial(bsg::EffectHandle effect, bsg::TextureHandle irrad,
                                        bsg::TextureHandle refl, bsg::TextureHandle occlusion,
                                        const bsg::Vec4 &color, float dimScale);
   void DrawTime(const bsg::Vec2 &pos);
   void HandleSpecialMenuCases(MenuItem *item);

   bool ParseOriginalFormat(FILE *fp, std::vector<char> &buff, std::vector<MenuItem> &menuItems);
   bool ParseMultiFormat(FILE *fp, std::vector<char> &buff, std::vector<MenuItem> &menuItems);

private:
   bsg::SceneNodeHandle    m_carouselRoot;
   bsg::SceneNodeHandle    m_logoRoot;
   //bsg::SceneNodeHandle m_backdropRoot;
   uint32_t                m_startAt;

   bsg::SceneNodeHandle    m_carouselCamNode;
   bsg::CameraHandle       m_carouselCamera;

   bsg::SceneNodeHandle    m_logoCamNode;
   bsg::CameraHandle       m_logoCamera;

   bsg::AnimationList      m_animList;

   bsg::Auto<MenuCarousel> m_carousel;

   std::vector<MenuItem>   m_menuItems;
   bool                    m_multiMenu;

   bsg::FontHandle         m_font;
   bsg::FontHandle         m_smallFont;

//   BCMBackdrop             m_backdrop;
   BCMGuilloche            m_guilloche;

   HelpMenu                *m_helpMenu;

   std::string             m_configFile;
};

#endif /* __LOGO_MENU_H__ */
