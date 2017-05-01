/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_hud.h"
#include "bsg_application.h"
#include "bsg_font.h"
#include "bsg_fonts.h"

#include <algorithm>

namespace bsg
{

static const char *OnOff(bool flag, bool disabled = false)
{
   if (disabled)
      return "disabled";

   return flag ? "on" : "off";
}

const uint32_t NUM_MENU_ITEMS = 5;

DevHud::DevHud() :
   m_swapInterval(1),
   m_stereo(false),
   m_rateMultiplier(1.0f),
   m_fpsHUD(false),
   m_changed(true)
{
   m_hudSel = CircularIndex(0, NUM_MENU_ITEMS - 1);
}

DevHud::~DevHud()
{
}

const char *DevHud::Mark(eIndex item) const
{
   return (m_hudSel.Current() == item) ? "> " : "  ";
}

static void PrintInfo(std::stringstream &str, const Application &app)
{
   uint32_t w   = app.GetWindowWidth();
   uint32_t h   = app.GetWindowHeight();

   str << w << "x" << h << "  ";

   str << app.GetOptions().GetColorBits() << "bpp\n";
   str << app.GetFPS() << " fps\n\n";
}

#ifndef BSG_STAND_ALONE

void DevHud::Draw()
{
   const Application &app = *Application::Instance();

   // Test for first time
   if (m_hudFont.IsNull())
   {
      m_hudFont = FontHandle(New);
      m_hudFont->SetFontInPercent(FontMem::GetInfo("DroidSans-Bold"), 3.0f, app.GetWindowHeight());
   }

   // Capture the current state of the settings and display
   m_swapInterval   = app.GetSwapInterval();
   m_stereo         = app.IsStereo();
   m_rateMultiplier = app.GetRateMultiplier();
   m_fpsHUD         = app.GetShowFpsHUD();

   std::stringstream str;
   str.setf(std::ios::fixed, std::ios::floatfield);
   str.precision(1);

   PrintInfo(str, app);

   str << "Swap interval ("   << m_swapInterval   << ")\n";
   str << "Fps HUD ("         << OnOff(m_fpsHUD)  << ")\n";
   str << "Stereo 3D ("       << OnOff(m_stereo)  << ")\n";
   str << "Rate multiplier (" << m_rateMultiplier << ")\n";

   str << "Frame-grab now\n";

   app.DrawTextString(str.str(), 0.1f, 0.9f, m_hudFont, Vec4(0.1f, 1.0f, 0.1f, 1.0f));

   std::stringstream s;
   s << "\n\n\n";
   for (int32_t i = 0; i < ((int32_t)m_hudSel.Current()); i++)
      s << "\n";
   s << ">";

   app.DrawTextString(s.str(), 0.08f, 0.9f, m_hudFont, Vec4(0.1f, 1.0f, 0.1f, 1.0f));
}

#else

void DevHud::Draw()
{
   if (m_changed)
   {
      const Application &app = *Application::Instance();

      std::stringstream str;
      str.setf(std::ios::fixed, std::ios::floatfield);
      str.precision(1);

      PrintInfo(str, app);

      // Capture the current state of the settings and display
      m_swapInterval   = app.GetSwapInterval();
      m_stereo         = app.IsStereo();
      m_rateMultiplier = app.GetRateMultiplier();
      m_fpsHUD         = app.GetShowFpsHUD();

      str << Mark(eSWAP_INTERVAL)   << "Swap interval ("   << m_swapInterval   << ")\n";
      str << Mark(eFPS_HUD)         << "Fps HUD ("         << "disabled"       << ")\n";
      str << Mark(eSTEREO)          << "Stereo 3D ("       << OnOff(m_stereo)  << ")\n";
      str << Mark(eRATE_MULTIPLIER) << "Rate multiplier (" << m_rateMultiplier << ")\n";
      str << Mark(eFRAME_GRAB)      << "Frame-grab disabled\n";

      std::cout << str.str();

      m_changed = false;
   }
}

#endif

static float StepUp(float x)
{
   float l     = log10(x);
   float step  = pow(10.0f, floor(l) - 1.0f);

   return std::max(step, 0.1f);
}

static float StepDown(float x)
{
   float l      = log10(x);
   float step   = pow(10.0f, floor(l) - 1.0f);
   float stepUp = StepUp(x -step);

   step = std::min(stepUp, step);

   return std::max(step, 0.1f);
}

void DevHud::HandleKey(const KeyEvent &ev)
{
   Application &app = *Application::Instance();

   bool  oldChanged = m_changed;

   m_changed = true;

   if (ev.State() == KeyEvent::eKEY_STATE_DOWN)
   {
      switch (ev.Code())
      {
      case KeyEvent::eKEY_EXIT :
      case KeyEvent::eKEY_ESC :
      case KeyEvent::eKEY_POWER :
         app.SetShowHUD(false);
#ifdef BSG_STAND_ALONE
         std::cout << "HUD off\n";
#endif
         break;
      case KeyEvent::eKEY_UP :
         m_hudSel--;
         break;
      case KeyEvent::eKEY_DOWN :
         m_hudSel++;
         break;
      case KeyEvent::eKEY_LEFT :
         switch (m_hudSel.Current())
         {
         case eSWAP_INTERVAL :
            m_swapInterval = m_swapInterval - 1;
            m_swapInterval = std::max(m_swapInterval, 0);
            app.SetSwapInterval(m_swapInterval);
            break;

         case eFPS_HUD :
            m_fpsHUD = !m_fpsHUD;
            app.SetShowFpsHUD(m_fpsHUD);
            break;

         case eSTEREO :
            m_stereo = !m_stereo;
            app.SetStereoscopic(m_stereo);
            break;

         case eRATE_MULTIPLIER :
            m_rateMultiplier -= StepDown(m_rateMultiplier);
            if (m_rateMultiplier <= 0.0f)
               m_rateMultiplier = 0.1f;
            app.SetRateMultiplier(m_rateMultiplier);
            break;

         default:
            break;
         }
         break;

      case KeyEvent::eKEY_RIGHT :
         switch (m_hudSel.Current())
         {
         case eSWAP_INTERVAL :
            m_swapInterval++;
            app.SetSwapInterval(m_swapInterval);
            break;

         case eFPS_HUD :
            m_fpsHUD = !m_fpsHUD;
            app.SetShowFpsHUD(m_fpsHUD);
            break;

         case eSTEREO :
            m_stereo = !m_stereo;
            app.SetStereoscopic(m_stereo);
            break;

         case eRATE_MULTIPLIER :
            m_rateMultiplier += StepUp(m_rateMultiplier);
            app.SetRateMultiplier(m_rateMultiplier);
            break;

         default :
            break;
         }
         break;

      case KeyEvent::eKEY_OK :
      case KeyEvent::eKEY_ENTER :
         if (m_hudSel.Current() == eFRAME_GRAB)
         {
            app.InitiateFrameGrab();
         }
         break;

      default :
         m_changed = oldChanged;
         break;
      }
   }
}

void DevHud::Resize(uint32_t /*w*/, uint32_t h)
{
#ifndef BSG_STAND_ALONE
   if (!m_hudFont.IsNull())
      m_hudFont->SetFontInPercent(FontMem::GetInfo("DroidSans-Bold"), 3.0f, h);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BSG_STAND_ALONE

FpsHud::FpsHud()
{
}

FpsHud::~FpsHud()
{
}

void FpsHud::Draw()
{
   const Application &app = *Application::Instance();

   if (m_fpsFont.IsNull())
   {
      m_fpsFont = FontHandle(New);
      m_fpsFont->SetFontInPercent(FontMem::GetInfo("DroidSans-Bold"), 3.0f, app.GetWindowHeight());
   }

   std::stringstream str;
   str.setf(std::ios::fixed, std::ios::floatfield);
   str.precision(1);
   str << app.GetFPS() << " fps\n\n";
   app.DrawTextString(str.str(), 0.85f, 0.9f, m_fpsFont, Vec4(0.1f, 1.0f, 0.1f, 1.0f));
}

void FpsHud::Resize(uint32_t /*w*/, uint32_t h)
{
   if (!m_fpsFont.IsNull())
      m_fpsFont->SetFontInPercent(FontMem::GetInfo("DroidSans-Bold"), 3.0f, h);
}

#endif /* BSG_STAND_ALONE */

}
