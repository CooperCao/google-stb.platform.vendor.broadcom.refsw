/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "planets.h"
#include "stars.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_animator.h"
#include "bsg_shape.h"
#include "body.h"

#include "../common/bcm_help_menu.h"

#include <iostream>
#include <istream>
#include <fstream>
#include <string>

#include <time.h>

using namespace bsg;
using namespace std;

const Body  *Info::m_earth = 0;

Planets     *Planets::m_instance = NULL;

static EffectHandle LoadEffect(const std::string &file)
{
   EffectHandle   effect(New);
   effect->Load(file);
   return effect;
}


class CameraView : public CameraCallback
{
public:
   virtual void OnViewTransform(const Mat4 &view, const Mat4 &iview)
   {
      Vec3  lightPos = GetTranslation(view);

      // Set the light position for all the materials.
      std::vector<MaterialHandle>   &materials = Planets::Instance()->GetLitMaterials();

      for (uint32_t i = 0; i < materials.size(); ++i)
         materials[i]->SetUniformValue("u_lightPos", lightPos);

      // Set the look vector
      Planets::Instance()->SetView(view, iview);
   }
};

class ScreenPosCallback : public CallbackOnRenderData
{
public:
   ScreenPosCallback(Vec2 *pos) :
      m_pos(pos)
   {}

   virtual bool OnRenderData(SemanticData &semData)
   {
      Vec4  pos  = semData.GetModelViewProjectionMatrix() * Vec4(0.0f, 0.0f, 0.0f, 1.0f);
      Vec3  ppos = pos.Proj();

      if (pos.Z() > 0.0f)
         *m_pos = ppos.Drop() * 0.5f + 0.5f;

      return true;
   }

private:
   Vec2  *m_pos;
};

Splash::Splash(FontHandle bigFont, FontHandle smallFont) :
   m_bigFont(bigFont),
   m_smallFont(smallFont),
   m_alpha(1.0f)
{
}

void Splash::Init(Planets &app)
{
   Time  now  = app.FrameTimestamp();
   float mult = app.GetRateMultiplier();

   AnimBindingLerpFloat *alpha = new AnimBindingLerpFloat(&m_alpha);
   alpha->Interpolator()->Init(now + 3.0f, now + 4.0f * mult, BaseInterpolator::eLIMIT);
   alpha->Evaluator()->Init(1.0f, 0.0f);

   app.AddAnimation(alpha);
}

void Splash::OnFrame(const Planets &app) const
{
   static const char *splashDescription =
      "Information:\n"
      "  Starfield derived from Smithsonian Astrophysical Observatory Star Catalog.\n"
      "  Distances and sizes not to scale.\n"
      "  Time runs at 1 hour per second.\n";

   static const char *splashCredit =
      "Texture Data Credits:\n"
      "  NASA, James Hastings-Trew, Grant Huchison,\n"
      "  Jens Meyer, Fridger Schrempp, Bjorn Jonsson,\n"
      "  Steve Albers, David Seal, Ivan Rivera, Chris Laurel.";

   if (m_alpha <= 0.0f)
      return;

   Vec4  color(1.0f, 1.0f, 1.0f, m_alpha);

   app.DrawTextString("The Planets", 0.3f, 0.8f, m_bigFont, color);
   app.DrawTextString(splashDescription, 0.3f, 0.6f, m_smallFont, color);
   app.DrawTextString(splashCredit,      0.3f, 0.4f, m_smallFont, color);
}

PlanetEffects::PlanetEffects(uint32_t geomResolution)
{
   m_surfaceEffect       = LoadEffect("planet.bfx");
   m_surfaceCloudsEffect = LoadEffect("planet_clouds.bfx");
   m_ringEffect          = LoadEffect("ring.bfx");
   m_atmosphereEffect    = LoadEffect("atmosphere.bfx");
   m_starEffect          = LoadEffect("star.bfx");
   m_sunEffect           = LoadEffect("sun.bfx");

   m_sphereSurface       = SphereFactory(Vec3(), 1.0f, geomResolution).MakeSurface();
   m_smallSphereSurface  = SphereFactory(Vec3(), 1.0f, geomResolution * 3 / 5).MakeSurface();
}

class Remover : public AnimationDoneNotifier
{
public:
   Remover(SceneNodeHandle node, const string &name1, const string &name2) :
      m_node(node),
      m_name1(name1),
      m_name2(name2)
   {}

   virtual void Notify(const Time &/*time*/)
   {
      m_node->RemoveGeometry(m_name1);
      m_node->RemoveGeometry(m_name2);
      delete this;
   }

private:
   bsg::SceneNodeHandle m_node;
   std::string          m_name1;
   std::string          m_name2;
};

Info::Info() :
   m_font(New),
   m_print1(New, "InfoText1"),
   m_print2(New, "InfoText2"),
   m_onColour (0.0f, 1.0f, 0.0f, 1.0f),
   m_offColour(0.0f, 1.0f, 0.0f, 0.0f)
{
   m_font->Load("DroidSans-Bold.ttf", 16);

   m_print1->SetFont(m_font);
   m_print2->SetFont(m_font);

   m_colourGroup.Append(m_print1->GetUniform<Vec4>("u_textColor"));
   m_colourGroup.Append(m_print2->GetUniform<Vec4>("u_textColor"));

   m_colourGroup.Set(m_offColour);

   m_info = "Name:\n"
            "Radius:\n"
            "Sidereal Rotation:\n"
            "Orbit Radius:\n"
            "Orbit Period:\n";
}

static void Plural(ostream &os, float val, const char *unit)
{
   os << val << " " << unit << (val == 1.0f ? "" : "s") << "\n";
}

void Info::Show(Planets &planets, const string &bodyName, bool on, const BodyBuilder &builder)
{
   const Body  *body    = builder.Find(bodyName);

   // One time initialisation of Earth for calculating au and year data
   if (m_earth == 0)
   {
      m_earth = builder.Find("Earth");
      if (m_earth == 0)
         BSG_THROW("Cannot find the Earth");
   }

   Time  now  = Application::Instance()->FrameTimestamp();
   float mult = Application::Instance()->GetRateMultiplier();

   Vec4 currentColour = m_colourGroup.Get();

   AnimBindingLerpVec4 *colorAnim = new AnimBindingLerpVec4(&m_colourGroup);

   if (on && body != 0)
   {
      stringstream   ss;

      ss.setf(ios::fixed);

      ss << bodyName << "\n";
      ss.precision(0);
      ss << body->GetRealRadius() << " km\n";
      ss.precision(1);

      float bodyDay = body->GetDay();

      if (fabs(bodyDay) < 100.0f)
         Plural(ss, bodyDay, "hour");
      else
         Plural(ss, bodyDay / 24.0f, "day");

      float bodyOrbitRadius  = body->GetRealOrbitRadius();
      float earthOrbitRadius = m_earth->GetRealOrbitRadius();

      if (bodyOrbitRadius != 0.0f)
      {
         if (bodyOrbitRadius > earthOrbitRadius * 0.5f)
            ss << bodyOrbitRadius / earthOrbitRadius << " au\n";
         else
            ss << bodyOrbitRadius << " km\n";
      }
      else
         ss << "n/a\n";

      float bodyYear  = body->GetYear();
      float earthYear = m_earth->GetYear();

      if (body->GetYear() != 0.0f)
      {
         if (bodyYear > earthYear * 0.5f)
            Plural(ss, bodyYear / earthYear, "year");
         else
            Plural(ss, bodyYear / 24.0f, "day");
      }
      else
      {
         ss << "n/a";
      }

      m_print1->SetText(m_info,   0.11f, Vec2(1.35f, 1.5f));
      m_print2->SetText(ss.str(), 0.11f, Vec2(2.2f, 1.5f));

      planets.GetTextRoot()->AppendGeometry(m_print1);
      planets.GetTextRoot()->AppendGeometry(m_print2);

      colorAnim->Interpolator()->Init(now, now + 1.0f * mult, BaseInterpolator::eLIMIT);
      colorAnim->Evaluator()->Init(currentColour, m_onColour);
   }
   else
   {
      // Note that removers delete themselves at the end
      colorAnim->Interpolator()->Init(now, now + 1.0f * mult, BaseInterpolator::eLIMIT, new Remover(planets.GetTextRoot(), "InfoText1", "InfoText2"));
      colorAnim->Evaluator()->Init(currentColour, m_offColour);
   }

   planets.AddAnimation(colorAnim);
}

Planets::Planets(Platform &platform, const ParseArgs &options) :
   Application(platform),
   m_rootNode(New),
   m_textNode(New),
   m_solarSystemNode(New),
   m_camNode(New),
   m_cameraLookAt(New),
   m_cameraPosition(New),
   m_camera(New),
   m_effects(options.GetGeomResolution()),
   m_upVector(0.0f, 1.0f, 0.0f),
   m_font(New),
   m_bigFont(New),
   m_bodyBuilder(0),
   m_sync(0),
   m_options(options),
   m_splash(m_bigFont, m_font),
   m_helpMenu(0)
{
   // Set up global instance
   if (m_instance != 0)
      BSG_THROW("Only one instance of Planets is allowed");

   m_instance = this;

   m_font->SetFontInPixels("DroidSans-Bold.ttf", 16.0f);
   m_bigFont->SetFontInPixels("DroidSans-Bold.ttf", 64.0f);

   SceneNodeHandle   stars;

   {
      ifstream in(FindResource("stars.txt").c_str());

      stars = Stars(in).CreateGeometry(m_effects);
   }

   {
      ifstream in(FindResource("solar.txt").c_str());

      m_bodyBuilder = new BodyBuilder(in);

      const Body  *root(m_bodyBuilder->GetRoot());

      if (root == 0)
         BSG_THROW("Root body not specified");

      m_solarSystemNode->AppendChild(root->CreateGraph(m_effects));
      m_rootNode->AppendChild(m_solarSystemNode);
   }

   m_rootNode->AppendChild(m_cameraPosition);
   m_rootNode->AppendChild(m_cameraLookAt);

   m_camera->SetClippingPlanes(1.0f, 2000.0f);
   m_camera->SetAspectRatio(GetWindowWidth(), GetWindowHeight());
   m_camera->SetYFov(40.0f);
   m_camera->SetCallback(new CameraView);

   m_camNode->SetCamera(m_camera);
   m_camNode->AppendChild(m_textNode);
   m_textNode->SetPosition(Vec3(0.0f, 0.0f, -5.0f));

   m_cameraPosition->SetPosition(Vec3(0.0f, 0.0f, -500.0f));
   m_cameraLookAt->SetPosition(Vec3(0.0f, 0.0f, 0.0f));

   // The camera is a constrained node dependent on the camera position and camera look-at nodes.
   ConstraintLookAtHandle  lookConstraint(New);
   lookConstraint->Install(lookConstraint, m_camNode, m_cameraPosition, m_cameraLookAt, m_upVector);

   // The stars "follow" the camera via a constraint
   ConstraintFollowPositionHandle   starsConstraint(New);
   starsConstraint->Install(starsConstraint, stars, m_cameraPosition);

   LoadScript();

   m_splash.Init(*this);

   if (!GetExtraOptions().GetOvernightTestMode())
   {
      // Making the help menu
      m_helpMenu = new HelpMenu(m_instance, eHELP_BUTTON_RED, "Help", "DroidSans.ttf", Vec4(1.0f, 0.5f, 0.0f, 1.0f), 0.025f, 0.93f, 0.03f, true);

      m_helpMenu->SetMenuItemHeaderColour(Vec4(1.0f, 0.3f, 0.0f, 1.0f));

      m_helpMenu->AddMenuItem("Menu", "Planet information");
      m_helpMenu->AddMenuItem("Right", "Look right");
      m_helpMenu->AddMenuItem("Left", "Look left");
      m_helpMenu->AddMenuItem("Down", "Move backward");
      m_helpMenu->AddMenuItem("Up", "Move forward and up");
      m_helpMenu->AddMenuItem("0", "Look at / move to the Sun");
      m_helpMenu->AddMenuItem("1", "Look at / move to Mercury");
      m_helpMenu->AddMenuItem("2", "Look at / move to Venus");
      m_helpMenu->AddMenuItem("3", "Look at / move to Earth");
      m_helpMenu->AddMenuItem("4", "Look at / move to Mars");
      m_helpMenu->AddMenuItem("5", "Look at / move to Jupiter");
      m_helpMenu->AddMenuItem("6", "Look at / move to Saturn");
      m_helpMenu->AddMenuItem("7", "Look at / move to Uranus");
      m_helpMenu->AddMenuItem("8", "Look at / move to Neptune");
      m_helpMenu->AddMenuItem("9", "Look at / move to Moon");
      m_helpMenu->AddMenuItem(eHELP_BUTTON_RED, "Help menu", eMENU_ID_1);
      m_helpMenu->AddMenuItem("Clear", "Stop demo", eMENU_ID_2);
      m_helpMenu->AddMenuItem("Clear", "Exit", eMENU_ID_3);

      m_helpMenu->SetMenuPosition(eMENU_POSITION_BOT_RIGHT, Vec2(0.0));
      unsigned int animationType = eMENU_ANIM_SCALE | eMENU_ANIM_MOVE_FROM_HELP_BTN;
      m_helpMenu->SetAnimationType(animationType);

      unsigned short menuToDisplay = 0;
      if (m_options.GetDemo())
      {
         menuToDisplay = eMENU_ID_1 | eMENU_ID_3;
      }
      else
      {
         menuToDisplay = eMENU_ID_1 | eMENU_ID_2;
      }
      m_helpMenu->SetMenuToDisplay(menuToDisplay);

      m_helpMenu->FinaliseMenuLayout();
   }
}

Planets::~Planets()
{
   m_instance = 0;

   m_command[0].Delete();
   m_command[1].Delete();

   delete m_bodyBuilder;
   delete m_helpMenu;
}

void Planets::LoadScript()
{
   ifstream ifs(FindResource("script.txt").c_str());
   m_commands.LoadScript(ifs, *this);
}

void Planets::ShowInfo(const string &body, bool status)
{
   m_info.Show(*this, body, status, *m_bodyBuilder);
}

// This happens before render frame
bool Planets::UpdateFrame(int32_t * /*idleTimeMs*/)
{
   // Update all the animations
   m_animList.UpdateTime(FrameTimestamp());

   if (m_helpMenu)
   {
      if (m_commands.Empty() && !m_options.GetDemo())
      {
         unsigned short menuToDisplay = eMENU_ID_3 | eMENU_ID_0 | eMENU_ID_1;
         m_helpMenu->SetMenuToDisplay(menuToDisplay);
      }

      m_helpMenu->UpdateTime();
   }

   return true;
}

void Planets::RenderFrame()
{
   static bool newCommand0 = false;
   static bool newCommand1 = false;

   static RenderOptions options;

   options.SetEnableViewFrustumCull(true);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   if (m_commands.Empty(0) && m_commands.Empty(1) && m_options.GetLoop())
      LoadScript();

   if (IsBeginFrame())
   {
      newCommand0 = m_command[0].Finished();
      newCommand1 = m_command[1].Finished();

      // No command currently running
      if (newCommand0)
      {
         m_command[0] = m_commands.Next(0);
         m_command[0].PreRenderInit();
      }

      if (newCommand1)
      {
         m_command[1] = m_commands.Next(1);
         m_command[1].PreRenderInit();
      }
   }

   // Render the scene -- will trigger callbacks to update camera info and navigation targets
   RenderSceneGraph(m_rootNode, options);

   if (IsEndFrame())
   {
      // A new command needs to set up the animations
      if (newCommand0)
         m_command[0].PostRenderInit();

      if (newCommand1)
         m_command[1].PostRenderInit();

      newCommand0 = false;
      newCommand1 = false;
      m_command[0].OnFrame();
      m_command[1].OnFrame();
   }

   if (m_helpMenu)
      RenderSceneGraph(m_helpMenu->GetRootNode());

   m_splash.OnFrame(*this);
}

void Planets::KeyEventHandler(KeyEvents &queue)
{
   static const char *target[] = { "Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Moon" };
   static string currentTarget = "";

   string newTarget = "";

   if (queue.Pending())
   {
      KeyEvent ev = queue.Pop();

      if (ev.State() == KeyEvent::eKEY_STATE_DOWN)
      {
         switch (ev.Code())
         {
         case KeyEvent::eKEY_EXIT :
         case KeyEvent::eKEY_ESC :
         case KeyEvent::eKEY_POWER :
            if (m_commands.Empty() || m_options.GetDemo())
            {
               Stop(255);
            }
            else
            {
               m_options.SetLoop(false);
               m_commands.Clear();
            }
            break;

         case KeyEvent::eKEY_0 :
         case KeyEvent::eKEY_1 :
         case KeyEvent::eKEY_2 :
         case KeyEvent::eKEY_3 :
         case KeyEvent::eKEY_4 :
         case KeyEvent::eKEY_5 :
         case KeyEvent::eKEY_6 :
         case KeyEvent::eKEY_7 :
         case KeyEvent::eKEY_8 :
         case KeyEvent::eKEY_9 :
            newTarget = target[ev.Code() - KeyEvent::eKEY_0];
            if (newTarget != currentTarget)
            {
               m_commands.Add(new CommandLookAt(1.0f, newTarget, m_cameraLookAt));
            }
            else
            {
               m_commands.Add(new CommandGoto(1.0f, newTarget, m_cameraPosition, m_bodyBuilder->Find(newTarget)->GetRadius() * 3.5f, Vec3()));
               newTarget = "";
            }
            break;

         case KeyEvent::eKEY_UP:
            m_commands.Add(new CommandMove(1.0f, m_cameraPosition, Vec3(0.0f, 10.0f, -10.0f)));
            break;

         case KeyEvent::eKEY_DOWN:
            m_commands.Add(new CommandMove(1.0f, m_cameraPosition, Vec3(0.0f, 0.0f,  10.0f)));
            break;

         case KeyEvent::eKEY_LEFT:
            m_commands.Add(new CommandRotate(1.0f, m_cameraLookAt, Vec3(0.0f, 10.0f, 0.0f)));
            break;

         case KeyEvent::eKEY_RIGHT:
            m_commands.Add(new CommandRotate(1.0f, m_cameraLookAt, Vec3(0.0f, -10.0f, 0.0f)));
            break;

         case KeyEvent::eKEY_MENU:
         case KeyEvent::eKEY_TAB:
         case KeyEvent::eKEY_F1:
            if (currentTarget != "")
               m_commands.Add(new CommandInfo(currentTarget, true));
            break;

         case KeyEvent::eKEY_F2:
            if (currentTarget != "")
               m_commands.Add(new CommandInfo(currentTarget, false));
            break;

         case KeyEvent::eKEY_RED:
         case KeyEvent::eKEY_F3:
             if (m_helpMenu)
               m_helpMenu->ToggleMenu();
            break;

         default :
            break;
         }

         if (newTarget != "")
            currentTarget = newTarget;
      }

      // Clear any pending events now
      queue.Clear();
   }
}

void Planets::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);

   if (m_helpMenu)
      m_helpMenu->Resize();
}

int main(int argc, char **argv)
{
   try
   {
      ApplicationOptions   options;
      ParseArgs            extraParse;

      options.SetDisplayDimensions(1280, 720);

      if (!options.ParseCommandLine(argc, argv, &extraParse))
         return 1;

      Platform platform(options);
      Planets  app(platform, extraParse);

      return platform.Exec();
   }
   catch (const Exception &e)
   {
      cerr << "Exception : " << e.Message() << "\n";
   }
}
