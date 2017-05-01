/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "logo_menu.h"
#include "logo_geom.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_png.h"
#include "bsg_image_pkm.h"
#include "bsg_exception.h"
#include "bsg_cube_images.h"
#include "bsg_text.h"
#include "bsg_math.h"
#include "bsg_parse_utils.h"

#include <iostream>
#include <istream>
#include <fstream>

using namespace bsg;

const float carouselPaneX = 0.4f;
const uint32_t numNodes = 11;

#ifdef WIN32
#include <direct.h>
#ifndef getcwd
#define getcwd _getcwd
#endif
#else
#include <unistd.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Guilloche
//
static Vec2 s_control1[] =
{
   Vec2(0.00f,  -0.30f),
   Vec2(0.05f,  -0.25f),
   Vec2(0.10f,  -0.30f),
   Vec2(0.15f,  -0.35f),
   Vec2(0.10f,  -0.30f),
   Vec2(0.25f,  -0.35f),
   Vec2(0.30f,  -0.20f),
   Vec2(0.35f,  -0.15f),
   Vec2(0.40f,  -0.20f),
   Vec2(0.45f,  -0.20f),
   Vec2(0.50f,  -0.20f),
   Vec2(0.55f,   0.00f),
   Vec2(0.60f,   0.15f),
   Vec2(0.65f,   0.25f),
   Vec2(0.70f,   0.30f),
   Vec2(0.75f,   0.40f),
   Vec2(0.80f,   0.45f),
   Vec2(0.85f,   0.50f),
   Vec2(0.90f,   0.60f),
   Vec2(0.95f,   0.65f),
   Vec2(1.00f,   0.80f)
};

static Vec2 s_control2[] =
{
   Vec2(0.00f,  -0.50f),
   Vec2(0.05f,  -0.55f),
   Vec2(0.10f,  -0.50f),
   Vec2(0.15f,  -0.65f),
   Vec2(0.20f,  -0.50f),
   Vec2(0.25f,  -0.55f),
   Vec2(0.30f,  -0.50f),
   Vec2(0.35f,  -0.45f),
   Vec2(0.40f,  -0.40f),
   Vec2(0.45f,  -0.35f),
   Vec2(0.50f,  -0.45f),
   Vec2(0.55f,  -0.25f),
   Vec2(0.60f,  -0.35f),
   Vec2(0.65f,  -0.25f),
   Vec2(0.70f,  -0.15f),
   Vec2(0.65f,   0.00f),
   Vec2(0.80f,   0.15f),
   Vec2(0.85f,   0.25f),
   Vec2(0.90f,   0.35f),
   Vec2(0.99f,   0.40f),
   Vec2(1.00f,   0.50f)
};

static Vec4 MkColor(uint32_t r, uint32_t g, uint32_t b)
{
   return Vec4(r / 255.0f, g / 255.0f, b / 255.0f, 0.7f);
}

static float Rand(float low, float high)
{
   float    randf = (((float)rand() / RAND_MAX) * (high - low)) + low;

   return randf;
}

static Vec2 *Randomize(uint32_t numControls, Vec2 *arr)
{
   for (uint32_t i = 0; i < numControls; ++i)
   {
      arr[i].X() = arr[i].X() + Rand(-0.025f, 0.025f);
      arr[i].Y() = -arr[i].Y();
   }

   return arr;
}

BCMGuilloche::BCMGuilloche(Application &app) :
   m_app(app),
   m_numControls(sizeof(s_control1) / sizeof(Vec2)),
   m_guilloche(m_numControls, Randomize(m_numControls, s_control1), Randomize(m_numControls, s_control2), 300, 25),
   m_guillochePanel(m_guilloche, Vec2(-0.2f, 0.4f), 0.0f)
{
   Vec4  topColor = MkColor(  0, 85, 104);
   Vec4  botColor = MkColor(227, 24, 55 );

   m_guilloche.SetColorRamp(topColor, topColor, botColor, botColor);
   m_guilloche.SetScale(1.6f, 0.9f, 1.6f);
   m_guilloche.SetWidth(0.007f);

   Time now = m_app.FrameTimestamp();

   AnimBindingLerpFloat *anim1 = new AnimBindingLerpFloat(&m_guilloche.GetOffsetX());
   anim1->Interpolator()->Init(now, now + 60.0f, BaseInterpolator::eREPEAT);
   anim1->Evaluator()->Init(0.0f, 1.0f);
   m_animList.Append(anim1);
}

void BCMGuilloche::Render()
{
   m_app.RenderSceneGraph(m_guillochePanel.GetRoot());
}

void BCMGuilloche::UpdateAnimationList(const Time &now)
{
   m_animList.UpdateTime(now);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ParseArgs
//
class ParseArgs : public ArgumentParser
{
public:
   ParseArgs() :
      m_configFile("")
   {}

   virtual bool ParseArgument(const std::string &arg)
   {
      if (ApplicationOptions::ArgMatch(arg.c_str(), "config="))
      {
         std::vector<char> buff(arg.size());

         if (sscanf(arg.c_str(), "config=%s", &buff[0]) == 1)
         {
            m_configFile = std::string(&buff[0]);
            return true;
         }
      }

      return false;
   }

   virtual std::string UsageString() const
   {
      return "\n"
             "config=filename   specify configuration file\n";
   }

   const std::string &GetConfigFile() const { return m_configFile; }

private:
   std::string m_configFile;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// LogoMenu
//
LogoMenu::LogoMenu(Platform &platform, const ParseArgs &args) :
   Application(platform),
   m_carouselRoot(New),
   m_logoRoot(New),
   m_carouselCamNode(New),
   m_carouselCamera(New),
   m_logoCamNode(New),
   m_logoCamera(New),
   m_multiMenu(false),
   m_font(New),
   m_smallFont(New),
   m_guilloche(*this),
   m_helpMenu(0),
   m_configFile(args.GetConfigFile())
{
   m_font->SetFontInPercent("DroidSans-Bold.ttf", 4.0f, GetOptions().GetHeight());
   m_smallFont->SetFontInPercent("DroidSans-Bold.ttf", 2.0f, GetOptions().GetHeight());

   // Setup the cameras in each graph
   m_carouselCamera->SetNearClippingPlane(0.1f);
   m_carouselCamera->SetFarClippingPlane(100.0f);
   m_carouselCamera->SetFocalPlane(1.0f);
   m_carouselCamera->SetAspectRatio((float)(GetWindowWidth() * (1.0f - carouselPaneX)) / (float)GetWindowHeight());

   m_carouselCamNode->SetCamera(m_carouselCamera);

   m_logoCamera->SetNearClippingPlane(3.0f);
   m_logoCamera->SetFarClippingPlane(100.0f);
   m_logoCamera->SetFocalPlane(5.5f);
   m_logoCamera->SetAspectRatio((float)(GetWindowWidth() / 2) / (float)GetWindowHeight());

   m_logoCamNode->SetTransform(CameraTransformHelper::Lookat(Vec3(0.0f, 1.0f, 5.5f), Vec3(0.0f, 0.0f, 0.0f),
                                                             Vec3(0.0f, 1.0f, 0.0f)));
   m_logoCamNode->SetCamera(m_logoCamera);

   // Make the logo materials
   EffectHandle  colorEffect(New);
   colorEffect->Load("env_mapped.bfx");

   // Load the cube maps we will use for lighting
   TextureHandle cubeMapTexture(New);
   TextureHandle reflMapTexture(New);
   TextureHandle occMapTexture(New);

   cubeMapTexture->TexImageCube(CubeImages("interior_probe_2_irr",  "pkm", Image::eETC1));
   reflMapTexture->TexImageCube(CubeImages("interior_probe_2_refl", "pkm", Image::eETC1));

   occMapTexture->TexImage2D(Image("ambient_logo_256.png", Image::eRGB888));

   MaterialHandle textMat = MakeEnvMappedMaterial(colorEffect, cubeMapTexture, reflMapTexture, occMapTexture,
                                                  Vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

   MaterialHandle pulseMat = MakeEnvMappedMaterial(colorEffect, cubeMapTexture, reflMapTexture, occMapTexture,
                                                   Vec4(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);

   MaterialHandle textReflMat = MakeEnvMappedMaterial(colorEffect, cubeMapTexture, reflMapTexture, occMapTexture,
                                                   Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.3f);

   MaterialHandle pulseReflMat = MakeEnvMappedMaterial(colorEffect, cubeMapTexture, reflMapTexture, occMapTexture,
                                                   Vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.3f);

   // Make the logo node & geometry
   SceneNodeHandle logoNode(New);
   SceneNodeHandle logoXform(New);

   BroadcomLogo   logo;

   logoNode->AppendGeometry(logo.InstanceLogo(textMat, pulseMat));
   // Make y up
   logoNode->GetTransform().SetPosition(Vec3(0.0f, 0.8f, 0.0f));
   logoNode->GetTransform().SetRotation(Quaternion(90.0f, 1.0f, 0.0, 0.0f));

   logoXform->AppendChild(logoNode);
   m_logoRoot->AppendChild(logoXform);

   // Make the logo reflection node & geometry
   SceneNodeHandle reflNode(New);
   SceneNodeHandle reflXform(New);
   reflNode->AppendGeometry(logo.InstanceLogo(textReflMat, pulseReflMat, true));
   // Make y up
   reflNode->GetTransform().SetPosition(Vec3(0.0f, -1.4f, 0.0f));
   reflNode->GetTransform().SetScale(Vec3(1.0f, -1.0f, 1.0f));
   reflNode->GetTransform().SetRotation(Quaternion(-90.0f, 1.0f, 0.0, 0.0f));

   reflXform->AppendChild(reflNode);
   m_logoRoot->AppendChild(reflXform);

   // Setup the animations for the logo
   Time now = FrameTimestamp();

   AnimBindingLerpQuaternionAngle *anim = new AnimBindingLerpQuaternionAngle(&logoXform->GetTransform().GetRotation());
   anim->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eREPEAT);
   anim->Evaluator()->Init(Vec3(0.0f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(anim);

   AnimBindingLerpQuaternionAngle *refl_anim = new AnimBindingLerpQuaternionAngle(&reflXform->GetTransform().GetRotation());
   refl_anim->Interpolator()->Init(now, now + 10.0f, BaseInterpolator::eREPEAT);
   refl_anim->Evaluator()->Init(Vec3(0.0f, 1.0f, 0.0f), 0.0f, 360.0f);
   m_animList.Append(refl_anim);

   // Load the last demo we ran
   m_startAt = 0;
   FILE *fp = fopen("last_menu.txt", "r");
   if (fp != NULL)
   {
      if (fscanf(fp, "%d", (int*)&m_startAt) == 1)
         fclose(fp);
   }

   std::string loadName = m_configFile;

   if (loadName == "")
   {
      loadName = "menu_config.txt";

      std::string configFile = "menu_config_" + platform.GetPlatformName() + ".txt";

      FILE *configFp = fopen(configFile.c_str(), "r");
      if (configFp == NULL)
      {
         printf("Could not open %s, default to %s\n", configFile.c_str(), loadName.c_str());
      }
      else
      {
         loadName = configFile;
         fclose(configFp);
      }
   }

   // Make the menu carousel
   if (LoadMenu(loadName, m_startAt))
   {
      // Make the carousel we will use
      m_carousel = new MenuCarousel(6.0f, numNodes, 12.0f, 3.0f, 5.0f,
                                    m_font, Vec2(0.8f, 0.58f), Vec2(0.05f, 0.15f),
                                    2.5f, 0.17f);

      for (uint32_t i = 0; i < m_menuItems.size(); i++)
         m_carousel->AddEntry(m_menuItems[i].textureFile,
                              m_menuItems[i].quickText, m_menuItems[i].desc, m_menuItems[i].enabled);

      m_carouselRoot->AppendChild(m_carousel->RootNode());
   }

   // Ensure each graph has a camera
   m_carouselRoot->AppendChild(m_carouselCamNode);
   m_logoRoot->AppendChild(m_logoCamNode);

   // Making the help menu
   m_helpMenu = new HelpMenu(this, eHELP_BUTTON_RED, "Help", "DroidSans.ttf", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.025f, 0.93f, 0.93f, true);

   m_helpMenu->SetMenuItemHeaderColour(Vec4(1.0f, 1.0f, 1.0f, 1.0f));

   m_helpMenu->AddMenuItem("Select", "Start demo");
   m_helpMenu->AddMenuItem("Up", "Scroll up");
   m_helpMenu->AddMenuItem("Down", "Scroll down");
   m_helpMenu->AddMenuItem(eHELP_BUTTON_RED, "Help menu");
   m_helpMenu->AddMenuItem("Clear", "Exit");

   m_helpMenu->SetMenuPosition(eMENU_POSITION_TOP_RIGHT, Vec2(0.0));
   unsigned int animationType = eMENU_ANIM_SCALE | eMENU_ANIM_MOVE_FROM_HELP_BTN;
   m_helpMenu->SetAnimationType(animationType);

   m_helpMenu->FinaliseMenuLayout();

   // Init any global GL state
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

MaterialHandle LogoMenu::MakeEnvMappedMaterial(EffectHandle effect, TextureHandle irrad, TextureHandle refl,
                                              TextureHandle occlusion, const Vec4 &color, float dimScale)
{
   MaterialHandle mat(New);

   mat->SetEffect(effect);
   mat->SetTexture("u_tex", irrad);
   mat->SetTexture("u_refl_tex", refl);
   mat->SetTexture("u_occlusion", occlusion);
   mat->SetUniformValue("u_color", color);
   mat->SetUniformValue("u_dimScale", dimScale);

   return mat;
}

void LogoMenu::DrawTime(const Vec2 &pos)
{
   std::string timeStr = NowTimestamp().CalendarTimeString("%d %B %Y - %X");

   DrawTextString(timeStr, pos[0], pos[1], m_smallFont, Vec4(1, 1, 1, 0.7f));
}

bool LogoMenu::UpdateFrame(int32_t *idleMs)
{
   bool changed = false;

   Time  now = FrameTimestamp();

   // Update the animations
   changed = m_animList.UpdateTime(now);
   changed = m_carousel->UpdateAnimationList(now) || changed;

   if (!changed)
   {
      *idleMs = (int32_t)m_animList.TimeToStartMs(now);
      if (m_carousel->IsAnimating())
         *idleMs = 0;
   }

   m_guilloche.UpdateAnimationList(now);

   if (m_helpMenu)
      m_helpMenu->UpdateTime();

   return changed;
}

void LogoMenu::ResizeHandler(uint32_t width, uint32_t height)
{
   m_carouselCamera->SetAspectRatio((float)(width * (1.0f - carouselPaneX)) / (float)height);
   m_logoCamera->SetAspectRatio((float)(width / 2) / height);

   m_font->SetFontInPercent("DroidSans-Bold.ttf", 4.0f, height);
   m_smallFont->SetFontInPercent("DroidSans-Bold.ttf", 2.0f, height);

   m_carousel->Resize();

   if (m_helpMenu)
      m_helpMenu->Resize();
}

void LogoMenu::RenderFrame()
{
   // Clear all the buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   m_guilloche.Render();

   // Draw the spinning logo
   glViewport((GLint)(GetWindowWidth() * 0.06f), (GLint)(GetWindowHeight() * 0.028f),
      GetWindowWidth() / 2, GetWindowHeight());
   RenderSceneGraph(m_logoRoot);

   // Draw the carousel
   glViewport((GLint)(GetWindowWidth() * carouselPaneX), 0,
             (GLsizei)(GetWindowWidth() * (1.0f - carouselPaneX)), GetWindowHeight());
   RenderSceneGraph(m_carouselRoot);

   // Render text layers
   m_carousel->RenderText();

   DrawTime(Vec2(0.05f, 0.97f));

   if (m_helpMenu)
      RenderSceneGraph(m_helpMenu->GetRootNode());
}

void LogoMenu::KeyEventHandler(KeyEvents &queue)
{
   // Service pending key events
   while (queue.Pending())
   {
      KeyEvent ev = queue.Pop();

      if (ev.State() == KeyEvent::eKEY_STATE_DOWN)
      {
         switch (ev.Code())
         {
         case KeyEvent::eKEY_EXIT :
         case KeyEvent::eKEY_ESC :
         case KeyEvent::eKEY_POWER :
            Stop(255);
            break;
         case KeyEvent::eKEY_UP :
            m_carousel->Prev(FrameTimestamp(), Time(0.3f));
            break;
         case KeyEvent::eKEY_DOWN :
            m_carousel->Next(FrameTimestamp(), Time(0.3f));
            break;
         case KeyEvent::eKEY_OK :
         case KeyEvent::eKEY_ENTER :
            if (m_menuItems[m_carousel->CurrentIndex()].enabled)
               Stop(m_carousel->CurrentIndex());
            break;
         case KeyEvent::eKEY_RED :
         case KeyEvent::eKEY_F1 :
            if (m_helpMenu)
               m_helpMenu->ToggleMenu();
         default :
            break;
         }
      }
   }
}

void LogoMenu::MouseEventHandler(MouseEvents &queue)
{
   // Service pending mouse events
   while (queue.Pending())
   {
      MouseEvent ev = queue.Pop();

      if (ev.Type() == MouseEvent::eMOUSE_EVENT_BUTTON && ev.State() == MouseEvent::eMOUSE_STATE_DOWN)
      {
         switch (ev.Code())
         {
         case MouseEvent::eBTN_LEFT :
            if (m_menuItems[m_carousel->CurrentIndex()].enabled)
               Stop(m_carousel->CurrentIndex());
            break;
         default :
            break;
         }
      }
      else if (ev.Type() == MouseEvent::eMOUSE_EVENT_WHEEL)
      {
         IVec2 vec = ev.RelativeWheelVector();

         if (vec.Y() > 0)
            m_carousel->Prev(FrameTimestamp(), Time(0.3f));
         else if (vec.Y() < 0)
            m_carousel->Next(FrameTimestamp(), Time(0.3f));
      }
   }
}

static void Escape(char *s)
{
   uint32_t len = strlen(s);
   uint32_t res = 0;

   for (uint32_t src = 0; src < len; ++src, ++res)
   {
      if (s[src] != '\\')
      {
         s[res] = s[src];
      }
      else
      {
         src += 1;

         switch (s[src])
         {
         case 0:
            s[res] = '\0';
            break;

         case 'n':
            s[res] = '\n';
            break;

         case '\\':
            s[res] = '\\';
            break;

         default:
            s[res] = s[src];
            break;
         }
      }
   }

   s[res] = '\0';
}

void LogoMenu::HandleSpecialMenuCases(MenuItem *item)
{
   if (item)
   {
      if (item->quickText.find("Quake") != std::string::npos)
      {
         size_t splitPos1 = item->desc.find("#1#");
         size_t splitPos2 = item->desc.find("#2#");

         if (splitPos1 != std::string::npos && splitPos2 != std::string::npos)
         {
            std::string allGoodStr = item->desc.substr(0, splitPos1);
            std::string mouseKbdStr = item->desc.substr(splitPos1 + 3, splitPos2 - splitPos1 - 3);
            std::string demoStr = item->desc.substr(splitPos2 + 3, item->desc.length() - splitPos2 - 3);

            // Need to check the Quake installation
            std::string demoPakFile = item->folder + "/" + "demoq3/pak0.pk3";
            std::string pakFile = item->folder + "/" + "baseq3/pak0.pk3";

            if (ParseUtils::IsFile(pakFile))
            {
               // Test for mouse & keyboard
               if (!IsMouseAttached() || !IsKeyboardAttached())
               {
                  item->desc = mouseKbdStr;
                  item->args = ParseUtils::StripWhite(item->args) + " +demo Demo1";
               }
               else
                  item->desc = allGoodStr;
            }
            else if (ParseUtils::IsFile(demoPakFile))
            {
               item->args = ParseUtils::StripWhite(item->args) + " +set fs_basegame demoq3";

               // Test for mouse & keyboard
               if (!IsMouseAttached() || !IsKeyboardAttached())
               {
                  item->desc = mouseKbdStr;
                  item->args = ParseUtils::StripWhite(item->args) + " +demo Demo1";
               }
               else
                  item->desc = demoStr;
            }
            else
            {
               item->desc = "Quake 3 Arena demo is not correctly installed";
               item->enabled = false;
            }
         }
      }
   }
}

static bool GetLine(FILE *fp, std::vector<char> &buff)
{
   return fgets(&buff[0], buff.size(), fp) != NULL;
}

static void NoNewlines(std::vector<char> &buff)
{
   for (uint32_t i = 0; buff[i] != '\0' && i < buff.size(); ++i)
      if (buff[i] == '\n' || buff[i] == '\r')
         buff[i] = '\0';
}

static bool GetField(FILE *fp, std::vector<char> &buff, std::string &field)
{
   if (!GetLine(fp, buff))
      return false;

   NoNewlines(buff);
   Escape(&buff[0]);

   field = std::string(&buff[0]);

   return true;
}

bool LogoMenu::ParseMultiFormat(FILE *fp, std::vector<char> &buff, std::vector<MenuItem> &menuItems)
{
   do
   {
      MenuItem item;

      item.enabled = true;
      item.folder  = ".";

      if (!GetField(fp, buff, item.quickText))
         return false;

      if (!GetField(fp, buff, item.textureFile))
         return false;

      if (!GetField(fp, buff, item.desc))
         return false;

      while (GetLine(fp, buff) && buff[0] != '#')
      {
         item.exe += std::string(&buff[0]);
         buff[0] = '\0';
      }

      menuItems.push_back(item);

   } while (buff[0] == '#');

   return true;
}

bool LogoMenu::ParseOriginalFormat(FILE *fp, std::vector<char> &buff, std::vector<MenuItem> &menuItems)
{
   do
   {
      MenuItem item;
      char     *s = &buff[0];
      char     *p;

      if (*s == '-')
         continue;

      item.enabled = true;

      p = strstr(s, ",");
      *p = '\0';
      item.textureFile = s;
      s = p + 1;

      p = strstr(s, ",");
      *p = '\0';
      item.quickText = s;
      s = p + 1;

      p = strstr(s, ",");
      *p = '\0';
      Escape(s);
      item.desc = s;
      s = p + 1;

      p = strstr(s, ",");
      *p = '\0';
      item.folder = s;
      s = p + 1;

      p = strstr(s, ",");
      *p = '\0';
      item.exe = s;
      s = p + 1;

      item.args = s;

      HandleSpecialMenuCases(&item);

      menuItems.push_back(item);

   } while (GetLine(fp, buff));

   return true;
}

bool LogoMenu::LoadMenu(const std::string &file, uint32_t startAt)
{
   FILE *fp = fopen(file.c_str(), "r");

   if (fp == NULL)
   {
      BSG_THROW("Could not open " << file);
      return false;
   }

   std::vector<MenuItem>   menuItems;
   std::vector<char>       buff(4096);

   GetLine(fp, buff);

   m_multiMenu = buff[0] == '#';

   if (m_multiMenu)
      ParseMultiFormat(fp, buff, menuItems);
   else
      ParseOriginalFormat(fp, buff, menuItems);

   fclose(fp);

   // Re-order to account for startAt position
   m_menuItems.clear();

   for (uint32_t i = startAt; i < menuItems.size(); i++)
      m_menuItems.push_back(menuItems[i]);
   for (uint32_t i = 0; i < startAt; i++)
      m_menuItems.push_back(menuItems[i]);

   return true;
}

void LogoMenu::WriteDemoScript(uint32_t current)
{
   FILE *fp = fopen("demorun.sh", "w");
   if (fp != NULL)
   {
      if (m_multiMenu)
      {
         fprintf(fp, "%s\n", m_menuItems[current].exe.c_str());
      }
      else
      {
         std::vector<char> cwd(256);

         while (getcwd(&cwd[0], cwd.size()) == NULL)
         {
            std::cout << "Resizing\n";
            cwd.resize(cwd.size() * 2);
         }

         fprintf(fp, "cd %s\nexport LD_LIBRARY_PATH=.:../common:%s/../common\n%s %s\n",
            m_menuItems[current].folder.c_str(),
            &cwd[0],
            m_menuItems[current].exe.c_str(),
            m_menuItems[current].args.c_str());
      }
      fclose(fp);
   }
   else
   {
      BSG_THROW("Can't create demorun.sh -- is the folder write protected?");
   }

   fp = fopen("last_menu.txt", "w");
   if (fp != NULL)
   {
      int32_t indx = (int32_t)current + (int32_t)m_startAt - (numNodes / 2);
      while (indx < 0)
         indx += m_menuItems.size();
      indx = indx % m_menuItems.size();

      fprintf(fp, "%d\n", indx);
      fclose(fp);
   }
}


int main(int argc, char **argv)
{
   uint32_t ret = 0;

   try
   {
      ApplicationOptions   options;
      ParseArgs            parseArgs;

      options.SetDisplayDimensions(1280, 720);

      if (!options.ParseCommandLine(argc, argv, &parseArgs))
         return 1;

      Platform    platform(options);
      LogoMenu    app(platform, parseArgs);

      ret = platform.Exec();

      if (ret != 255)
         app.WriteDemoScript(ret);
   }
   catch (const Exception &e)
   {
      std::cerr << "Exception : " << e.Message() << "\n";
   }

   return ret;
}
