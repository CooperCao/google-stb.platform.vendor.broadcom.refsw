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

#include "pg.h"
#include "pg_dummyData.h"

#include <stdint.h>
#include <memory>
#include <assert.h>
#include <algorithm>

#include "bsg_shape.h"

using namespace bsg;

namespace pg
{

static bool          s_oldBackdrop = false;
static bool          s_withBackdrop = true;
static bool          s_peelOffLongScrollAnim = true;
static bool          s_demo = false;


// Implement metrics in cpp for now as it reduces compilation times
Vec4 Metrics::GetBackgroundColour()       { return Vec4(0.012f, 0.231f, 0.298f, 1.0f); }

Vec2  Metrics::GetCurrentTimePositionOld(){ return Vec2(0.036f, 0.97f);                }
Vec2  Metrics::GetCurrentTimePosition()   { return Vec2(0.036f, 0.93f);                }
float Metrics::GetPanelXInset()           { return 0.1f;                               }
float Metrics::GetPanelYInset()           { return 0.1f;                               }

Vec4  Metrics::GetPanelColourEven()       { return Vec4(0.098f, 0.58f, 0.722f, 1.0f);  }
Vec4  Metrics::GetPanelColourOdd()        { return Vec4(0.020f, 0.325f, 0.416f, 1.0f); }

float Metrics::GetPanelTextScale()        { return 0.7f;                               }
float Metrics::GetPanelTextPoints()       { return 29.0f;                              }
Vec2  Metrics::GetPanelTextPosition()     { return Vec2(0.2f, 0.35f);                  }
Vec4  Metrics::GetPanelTextColour()       { return Vec4(1.0f);                         }

float Metrics::GetDescTextHeight()        { return 0.6f;                               }
float Metrics::GetDescTextLimit()         { return 27.0f;                              }
float Metrics::GetDescTextPoints()        { return 25.0f;                              }
Vec2  Metrics::GetDescTextPosition()      { return Vec2(-11.2f, -7.7f);                }
Vec4  Metrics::GetDescTextColour()        { return Vec3(0.9f).Lift(1.0f);              }

float Metrics::GetTitleTextHeight()       { return 0.7f;                               }
float Metrics::GetTitleTextPoints()       { return 29.0f;                              }
Vec2  Metrics::GetTitleTextPosition()     { return Vec2(-11.2f, -6.9f);                }
Vec4  Metrics::GetTitleTextColour()       { return Vec4(1.0f);                         }

float Metrics::GetMetaTextRight()         { return 16.0f;                              }
float Metrics::GetMetaTextSize()          { return 0.5f;                               }
Vec4  Metrics::GetMetaTextColour()        { return Vec3(0.9f).Lift(1.0f);              }

float Metrics::GetInfoTextHeight()        { return 0.6f;                               }
float Metrics::GetInfoTextPoints()        { return 27.0f;                              }
float Metrics::GetInfoTextLimit()         { return 10.5f;                              }
Vec2  Metrics::GetInfoTextPosition()      { return Vec2(-16.3f, -6.9f);                }
Vec4  Metrics::GetInfoTextColour()        { return Vec4(0.85f);                        }

float Metrics::GetPanelWidthForOneHour()  { return 11.0f;                              }
Time  Metrics::GetActiveRegionDuration()  { return Time(2.5f, Time::eHOURS);           }

float Metrics::GetTimeTextScale()         { return 0.6f;                               }
float Metrics::GetTimeTextPoints()        { return 29.0f;                              }
Vec2  Metrics::GetTimeTextPosition()      { return Vec2(-0.06f, 0.42f);                }
Vec4  Metrics::GetTimeTextColour()        { return Vec4(1.0f);                         }
Vec2  Metrics::GetTimeTickSize()          { return Vec2(0.2f, 0.2f);                   }
Vec4  Metrics::GetTimeTickColour()        { return Vec4(0.8f, 0.8f, 0.8f, 1.0f);       }

float Metrics::GetChannelTextScale()      { return 0.6f;                               }
float Metrics::GetChannelTextPoints()     { return 29.0f;                              }
Vec2  Metrics::GetChannelTextPosition()   { return Vec2(0.0f, 0.35f);                  }
Vec4  Metrics::GetChannelTextColour()     { return Vec4(1.0f);                         }
float Metrics::GetChannelTextWidth()      { return 4.9f;                               }

float Metrics::GetChannelSpacing()        { return 1.1f;                               }

Time  Metrics::GetPageScrollTime()        { return Time(0.4f, Time::eSECONDS);         }
Time  Metrics::GetDemoPageScrollTime()    { return Time(3.0f, Time::eSECONDS);         }
Time  Metrics::GetDemoPauseTime()         { return Time(3.0f, Time::eSECONDS);         }
Time  Metrics::GetLineScrollTime()        { return Time(0.2f, Time::eSECONDS);         }
Time  Metrics::GetHorizPageScrollTime()   { return Time(0.4f, Time::eSECONDS);         }
Time  Metrics::GetMaxHorizScrollDist()    { return Time(2.0f, Time::eHOURS);           }

bool  Metrics::GetDisableStenciling()     { return false;                              }
bool  Metrics::GetUseDebugCamera()        { return false;                              }
bool  Metrics::GetDoVisibilityCulling()   { return true;                               }

const char  *Metrics::GetNoScheduleText()          { return "Schedule unavailable";                            }
const char  *Metrics::GetNoScheduleDescription()   { return "There is no schedule data for this channel";      }
const char  *Metrics::GetNoProgrammeText()         { return "No information";                                  }
const char  *Metrics::GetNoProgrammeDescription()  { return "There is no information for this programme";      }

Vec4  Metrics::GetHighlightColour()       { return Vec4(1.0f, 0.753f, 0.0f, 1.0f);     }

const std::vector<uint32_t> &ShortenFormatter::Format(const std::vector<uint32_t> &str)
{
   float pointWidth = GetAdvanceX('.');

   float position   = 0.0f;

   m_result.clear();

   // Box is too small for anything sensible
   if (m_limit < pointWidth * 2.5f)
   {
      m_hasShortened = true;
      return m_result;
   }

   for (std::vector<uint32_t>::const_iterator readIter = str.begin(); readIter != str.end(); )
   {
      float width = GetAdvanceX(*readIter);

      if (position + width > m_limit - pointWidth * 3.0f)
      {
         m_result.push_back('.');
         m_result.push_back('.');
         readIter = str.end();
         m_hasShortened = true;
      }
      else
      {
         position += width;
         m_result.push_back(*readIter);
         ++readIter;
      }
   }

   return m_result;
}

/////////////////////////////////////////////////////////////////////

class CustomArgumentParser : public ArgumentParser
{
public:
   //! Process a command line argument.
   //! Return true if you recognize the argument and have handled it.
   //! Return false to indicate this is an option you don't recognize - an error.
   virtual bool ParseArgument(const std::string &arg)
   {     
      if (ApplicationOptions::ArgMatch(arg.c_str(), "+old"))
      {
         s_oldBackdrop = true;
         return true;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "+new2"))
      {
         s_withBackdrop = false;
         return true;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "+anim2"))
      {
         s_peelOffLongScrollAnim = false;
         return true;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "+demo"))
      {
         s_demo = true;
         return true;
      }
      
      return false;
   }

   //! Return a string containing usage descriptions of the extra arguments you can handle.
   virtual std::string UsageString() const
   {
      return "+old               run using old backdrop style\n"
             "+new2              run using a new style without the backdrop\n"
             "+anim2             scrolling animation for the long scrolls\n"
             "+demo              run in scrolling mode\n"
             ;
   }
};


///////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t Clamp(int32_t low, int32_t high, int32_t val)
{
   if (val < low)
      return low;

   if (val > high)
      return high;

   return val;
}

bool Selection::ChannelChange(int32_t dc, Controller *controller)
{
   int32_t newChannel = Clamp(0, (int32_t)m_maxChannel - 1, (int32_t)m_channel + dc);

   if ((int32_t)m_channel == newChannel)
      return false;

   controller->HighlightSelection(*this, false);

   m_channel = newChannel;

   controller->HighlightSelection(*this);

   return true;
}

bool Selection::MoveRight(Controller *controller)
{
   const Time  *newTime = controller->FindNextProgramme(*this);

   controller->HighlightSelection(*this, false);

   if (newTime != 0)
   {
      Time t = *newTime;

      if (t - m_time > Metrics::GetMaxHorizScrollDist())
         t = m_time + Metrics::GetMaxHorizScrollDist();

      m_time = t;
   }
   else
   {
      m_time = m_time + Metrics::GetMaxHorizScrollDist();
   }

   controller->HighlightSelection(*this);

   return true;
}

bool Selection::MoveLeft(Controller *controller)
{
   Time minTime = theApp->MinVisibleTime();

   if (m_time <= minTime)
      return false;

   if (GetProgrammeStart() < theApp->NowTimestamp())
      return false;

   const Time  *newTime = controller->FindPreviousProgramme(*this);

   controller->HighlightSelection(*this, false);

   if (newTime != 0)
   {
      Time t = *newTime;

      if (m_time - t > Metrics::GetMaxHorizScrollDist())
         t = m_time - Metrics::GetMaxHorizScrollDist();

      m_time = t;

      if (m_time < minTime)
         m_time = minTime;
   }
   else
   {
      m_time = m_time - Metrics::GetMaxHorizScrollDist();

      if (m_time < minTime)
         m_time = minTime;
   }

   controller->HighlightSelection(*this);

   return true;
}

void Selection::MoveToNow(Controller *controller)
{
   controller->HighlightSelection(*this, false);
   m_time = theApp->FrameTimestamp();
   controller->HighlightSelection(*this);
}

void Selection::AddTime(Controller *controller, const Time &offset)
{
   Time minTime = theApp->NowTimestamp();

   controller->HighlightSelection(*this, false);

   m_time = m_time + offset;

   if (m_time < minTime)
      m_time = minTime;

   controller->HighlightSelection(*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////

AppAnimation::AppAnimation() :
   m_currentKeyAnimation(0),
   m_currentKeyAnimationType(KeyEvent::eKEY_NONE)
{

}


bool AppAnimation::Set(bsg::AnimBindingBase *currentKeyAnimation,
                           bsg::KeyEvent::eKeyCode currentKeyAnimationType,
                           bool single)
{
   bool animationSet = false;

   if ((m_currentKeyAnimation == NULL) || (!single))
   {
      m_animList.Append(currentKeyAnimation);
      animationSet = true;
      m_currentKeyAnimationType = currentKeyAnimationType;
      m_currentKeyAnimation = currentKeyAnimation;
   }

   return animationSet;
}

/////////////////////////////////////////////////////////////////////

//

static Vec2 s_controlNoBackdrop1[] =
{
   Vec2(0.00f,  -0.14f),
   Vec2(0.10f,  -0.04f),
   Vec2(0.20f,  -0.15f),
   Vec2(0.30f,  -0.16f),
   Vec2(0.42f,  -0.04f),
   Vec2(0.35f,  -0.12f),
   Vec2(0.52f,  -0.5f),
   Vec2(0.65f,  -1.0f),
   Vec2(0.73f,  -1.0f),
   Vec2(0.85f,  -1.0f),
   Vec2(1.00f,  -1.0f)
};

static Vec2 s_controlNoBackdrop2[] =
{
   Vec2(0.05f,  -0.05f),
   Vec2(0.00f,  -0.01f),
   Vec2(0.10f,  -0.06f),
   Vec2(0.20f,  -0.15f),
   Vec2(0.35f,  -0.2f),
   Vec2(0.50f,  -0.3f),
   Vec2(0.65f,  -0.5f),
   Vec2(0.70f,  -0.5f),
   Vec2(0.80f,  -0.5f),
   Vec2(0.90f,  -0.5f),
   Vec2(1.00f,  -0.5f)
};

static Vec2 s_controlWithBackdrop1[] =
{
   Vec2(0.00f,  -0.20f),
   Vec2(0.10f,  -0.20f),
   Vec2(0.20f,  -0.15f),
   Vec2(0.30f,  -0.16f),
   Vec2(0.42f,  -0.04f),
   Vec2(0.35f,  -0.12f),
   Vec2(0.52f,  -0.16f),
   Vec2(0.65f,  -0.16f),
   Vec2(0.73f,  -0.07f),
   Vec2(0.85f,  -0.12f),
   Vec2(1.00f,  -0.14f)
};

static Vec2 s_controlWithBackdrop2[] =
{
   Vec2(0.05f,  -0.05f),
   Vec2(0.00f,  -0.01f),
   Vec2(0.10f,  -0.06f),
   Vec2(0.20f,  -0.11f),
   Vec2(0.35f,  -0.22f),
   Vec2(0.50f,  -0.04f),
   Vec2(0.65f,  -0.04f),
   Vec2(0.70f,  -0.07f),
   Vec2(0.80f,  -0.02f),
   Vec2(0.90f,  -0.10f),
   Vec2(1.00f,  -0.02f)
};

static Vec2 *SelectControl1()
{
   if (s_withBackdrop)
      return s_controlWithBackdrop1;
   else
      return s_controlNoBackdrop1;
}

static Vec2 *SelectControl2()
{
   if (s_withBackdrop)
      return s_controlWithBackdrop2;
   else
      return s_controlNoBackdrop2;
}

static Vec4 MkColor(uint32_t r, uint32_t g, uint32_t b)
{
   return Vec4(r / 255.0f, g / 255.0f, b / 255.0f, 0.7f);
}

uint32_t GetNumberOfControls()
{
   uint32_t number = 0;
   if(s_withBackdrop)
      number = sizeof(s_controlWithBackdrop1) / sizeof(Vec2);
   else
      number = sizeof(s_controlNoBackdrop1) / sizeof(Vec2);

   return number;
}

BCMGuilloche::BCMGuilloche() :
   m_pApp(0),
   m_numControls(GetNumberOfControls()),
   m_guilloche(m_numControls, SelectControl1(), SelectControl2(), 100, 10),
   //m_guillochePanel(m_guilloche, Vec2(-0.1f, 1.02f), 0.0f)
   m_guillochePanel(m_guilloche, Vec2(-0.1f, 1.02f), 0.0f)
{
   Vec4  botColor = MkColor(  0, 85, 104);
   Vec4  topColor = MkColor(227, 24, 55 );

   m_guilloche.SetColorRamp(botColor, botColor, topColor, topColor);

   if (s_withBackdrop)
      m_guilloche.SetScale(1.8f, 0.4f, 1.1f);
   else
      m_guilloche.SetScale(1.6f, 0.4f, 2.0f);
      

   m_guilloche.SetWidth(0.007f);
}

void BCMGuilloche::Init(Application *app)
{
   m_pApp = app;
   Time now = m_pApp->FrameTimestamp();

   AnimBindingLerpFloat *anim1 = new AnimBindingLerpFloat(&m_guilloche.GetOffsetX());
   anim1->Interpolator()->Init(now, now + 60.0f, BaseInterpolator::eREPEAT);
   anim1->Evaluator()->Init(0.0f, 1.0f);
   m_animList.Append(anim1);
}

void BCMGuilloche::Render()
{
   m_pApp->RenderSceneGraph(m_guillochePanel.GetRoot());
}

void BCMGuilloche::UpdateAnimationList(const Time &now)
{
   m_animList.UpdateTime(now);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

App *theApp = 0;

App::App(Platform &platform) :
   Application(platform),
   m_platform(platform),
   m_cameraNode(New),
   m_rootNode(New),
   m_descNode(New),
   m_stencilRectRoot(New),
   m_panelMaterial(New),
   m_oldBackground(s_oldBackdrop),
   m_pBackdrop(0),
   m_smallFont(New),
   m_descFont(New),
   m_titleFont(New),
   m_demoMode(false),
   m_helpMenu(0)
{
   theApp = this;
   m_controller = new Controller();
   LongScrolling::SetPeelOffAnimation(s_peelOffLongScrollAnim);

   if (!m_oldBackground)
   {
      m_guilloche.Init(this);
      // Create a backdrop with no animation
      m_pBackdrop = new BCMBackdrop(0, 0.0f, 0.0f, 1, 57, false, 0.99f, BACKDROP);
   }
   else
   {
      // Create a normal backdrop
      m_pBackdrop = new BCMBackdrop(64, 0.865f, 1.0f, 1, 57, false, 0.99f, BACKDROP);
   }

   m_smallFont->SetFontInPercent("DroidSans-Bold.ttf", 2.6f, GetOptions().GetHeight());

   m_descFont->Load("DroidSans-Bold.ttf", Metrics::GetDescTextPoints());
   m_titleFont->Load("DroidSans-Bold.ttf", Metrics::GetTitleTextPoints());

   Time epgStartTime = MinVisibleTime();

   DummyData   dummy;
   dummy.InitGridInfo(&m_database);

   m_selection = Selection(NowTimestamp(), 0, m_database.GetNumChannels());

   m_activeRegion = Region(epgStartTime, Metrics::GetActiveRegionDuration(), 0, 10);

   // Enlarge keep region all around
   m_keepRegion = m_activeRegion.Enlarged(m_activeRegion.GetDuration(), m_activeRegion.GetNumChannels());

   EffectHandle   panelEffect(New);
   panelEffect->Load("panel.bfx");
   m_panelMaterial->SetEffect(panelEffect);

   if (Metrics::GetDisableStenciling())
      m_panelMaterial->GetEffect()->GetPass(0)->State().SetEnableStencilTest(false);

   m_camera = CameraHandle(New);

   m_camera->SetNearClippingPlane(1.0f);
   m_camera->SetFarClippingPlane(50.0f);
   m_camera->SetFocalPlane(1.0f);
   m_camera->SetAspectRatio(GetWindowWidth(), GetWindowHeight());
   m_camera->SetYFov(30.0f);

   m_cameraNode->SetCamera(m_camera);
   m_cameraNode->SetPosition(Vec3(0.0f, 0.0f, 36.0f));

   if (Metrics::GetUseDebugCamera())
   {
      m_camera->SetFarClippingPlane(180.0f);
      m_cameraNode->SetPosition(Vec3(0.0f, 0.0f, 167.0f));
   }

   m_controller->Initialise(m_database, m_activeRegion, m_keepRegion);
   m_controller->HighlightSelection(m_selection);
   ChangeDescription();

   // Initialise long scrolling
   // between two different keep region
   bsg::Mat4 projectionMat;
   m_camera->MakeProjectionMatrix(&projectionMat);
   m_longScrolling.Init(this, 
                        m_controller, 
                        projectionMat, 
                        TimeToXCoord(m_activeRegion.GetDuration()), 
                        ChannelStride() * (float)m_activeRegion.GetNumChannels());

   m_rootNode->AppendChild(m_cameraNode);
   m_rootNode->AppendChild(m_controller->GetRoot());
   m_rootNode->AppendChild(m_descNode);

   if((s_withBackdrop) || (m_oldBackground))
      m_rootNode->AppendChild(m_pBackdrop->RootNode());

   m_guiDisplay.Init(m_controller->GetRoot());

   // Configure the backdrop
   Vec3 grey(0.4f, 0.4f, 0.4f);
   Vec3 white(1.0f, 1.0f, 1.0f);
   Vec3 black(0.0f, 0.0f, 0.0f);
   m_pBackdrop->AddShadedPanel(0.204f, 0.861f, m_pBackdrop->Blues(BCMBackdrop::eBCM_BL), black,
                                           black, m_pBackdrop->Blues(BCMBackdrop::eBCM_TR));
   m_pBackdrop->AddShadedPanel(0.861f, 0.865f, grey, white, grey, white);
   m_pBackdrop->AddShadedPanel(0.2f, 0.204f, white, grey, white, grey);

   if (m_oldBackground)
      m_pBackdrop->AddShadedPanel(0.0f, 0.2f, m_pBackdrop->Blues(BCMBackdrop::eBCM_TR), m_pBackdrop->Blues(BCMBackdrop::eBCM_TR),
                                          black, black);
   else
   {
      m_pBackdrop->AddShadedPanel(0.865f, 1.0f, black, black,
                                           black, black);
      m_pBackdrop->AddShadedPanel(0.0f, 0.2f, black, black,
                                           black, black);
   }

   Vec4  bg(Metrics::GetBackgroundColour());

   glClearColor(bg.X(), bg.Y(), bg.Z(), bg.W());
   glClearDepthf(1.0f);
   glClearStencil(0);

   if (!m_oldBackground)
   {
      // Making the help menu
      m_helpMenu = new HelpMenu(theApp, eHELP_BUTTON_RED, "Help", "DroidSans.ttf", Vec4(1.0f, 0.5f, 0.0f, 1.0f), 0.025f, 0.93f, 0.03f, true);
   
      m_helpMenu->SetMenuItemHeaderColour(Vec4(1.0f, 0.5f, 0.0f, 1.0f));
      m_helpMenu->SetMenuItemTextColour(Vec4(1.0f, 1.0f, 1.0f, 1.0f));

      m_helpMenu->AddMenuItem("FAV1", "3D view");
      m_helpMenu->AddMenuItem("FAV2", "Auto-scroll on/off");
      m_helpMenu->AddMenuItem(eHELP_BUTTON_GREEN, "Current time");
      m_helpMenu->AddMenuItem(eHELP_BUTTON_BLUE, "Next day");
      m_helpMenu->AddMenuItem(eHELP_BUTTON_YELLOW, "Previous day");
      m_helpMenu->AddMenuItem("Up", "Previous channel");
      m_helpMenu->AddMenuItem("Down", "Next channel");
      m_helpMenu->AddMenuItem("Channel Up", "Previous channel page");
      m_helpMenu->AddMenuItem("Channel Down", "Next channel page");
      m_helpMenu->AddMenuItem("Left", "Previous program");
      m_helpMenu->AddMenuItem("Right", "Next program");
      m_helpMenu->AddMenuItem(eHELP_BUTTON_RED, "Help Menu");
      m_helpMenu->AddMenuItem("Clear", "Exit");

      bool roundedCorners = true;
      m_helpMenu->SetMenuBackgroundColour(Vec4(0.0f,0.0f,0.0f,0.9f), roundedCorners);
      m_helpMenu->SetMenuPosition(eMENU_POSITION_BOT_RIGHT, Vec2(0.0f));

      unsigned int animationType = eMENU_ANIM_FADE | eMENU_ANIM_SCALE | eMENU_ANIM_MOVE_FROM_BOT_RIGHT; 
      m_helpMenu->SetAnimationType(animationType);
      m_helpMenu->FinaliseMenuLayout();
   }

   if (s_demo)
      StartDemoMode();
}

App::~App()
{
   delete m_controller;
   delete m_pBackdrop;
   delete PanelCache::GetPanelCache();
   delete m_helpMenu;
}

static const std::string &DayString(uint32_t day)
{
   static std::string days[31] = { "1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", "10th",
                                   "11th", "12th", "13th", "14th", "15th", "16th", "17th", "18th", "19th", "20th",
                                   "21st", "22nd", "23rd", "24th", "25th", "26th", "27th", "28th", "29th", "30th", "31st" };
   return days[day];
}

void App::DrawTime(const Vec2 &pos)
{
   std::string timeStr = FrameTimestamp().CalendarTimeString("%A %B ") + DayString(FrameTimestamp().CalendarDay() - 1) + 
                         FrameTimestamp().CalendarTimeString(" %Y - %X");

   DrawTextString(timeStr, pos[0], pos[1], m_smallFont, Vec4(1, 1, 1, 1));
}

bool App::UpdateFrame(int32_t * /*idleMs*/)
{
   Time now = FrameTimestamp();

   // If there is no animation running
   if (! m_appAnim.Running())
   {
      // Is the selection entirely in the too-old region?
      Time  lastProgEnd = m_selection.GetProgrammeEnd() - Time(1, Time::eHOURS);
      bool  scrolled    = false;

      while (m_selection.GetProgrammeEnd() < now && lastProgEnd != m_selection.GetProgrammeEnd())
      {
         lastProgEnd = m_selection.GetProgrammeEnd();
         m_selection.MoveRight(m_controller);
         scrolled = true;
      }

      if (scrolled)
         ChangeDescription();

      // Is the start time of the active region too old? (and we're not currently animating the guide)
      if (m_activeRegion.GetStartTime() < now - Time(30, Time::eMINUTES))
      {
         // Animate the start point
         Time minTime = MinVisibleTime();

         AnimBindingHermiteTime *anim = new AnimBindingHermiteTime(&m_activeRegion.GetStartTime());
         anim->Init(m_activeRegion.GetStartTime(), now, minTime, now + Time(1, Time::eSECONDS), BaseInterpolator::eLIMIT, this);
         m_appAnim.Set(anim, KeyEvent::eKEY_USER_3);

         m_keepRegion.UpdateStartTime(m_keepRegion.GetStartTime() + Time(30, Time::eMINUTES));
         m_controller->SetKeepRegion(m_keepRegion);
         m_controller->VisibilityCull(m_activeRegion);
         m_controller->HighlightSelection(m_selection, true);
      }
   }

   m_appAnim.UpdateTime(FrameTimestamp());

   if (m_oldBackground)
      m_pBackdrop->UpdateAnimationList(FrameTimestamp());
   else
      m_guilloche.UpdateAnimationList(FrameTimestamp());

   if (m_lastActiveRegion != m_activeRegion)
   {
      m_controller->SetActiveRegion(m_activeRegion);
      m_controller->VisibilityCull(m_activeRegion);

      m_lastActiveRegion = m_activeRegion;
   }

   m_controller->UpdateFrame();

   if (m_helpMenu)
      m_helpMenu->UpdateTime();

   return true;
}

void App::RenderFrame()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   // If there is no backdrop at all the Guilloche can be drawn first
   if ((!m_oldBackground) && (!s_withBackdrop))
      m_guilloche.Render();


   // If this is normal rendering
   if (!m_longScrolling.IsActive())
   {
      RenderSceneGraph(m_rootNode);
   }
   else
   {
      // Animating the long home scrolling
      bsg::AnimBindingBase * anim = m_longScrolling.RenderScene(m_rootNode, 
                                                            m_guiDisplay, 
                                                            FrameTimestamp(), 
                                                            TimeToXCoord(m_activeRegion.GetDuration()), 
                                                            this);
      // If animation has been created
      if (NULL != anim)
         m_appAnim.Set(anim, KeyEvent::eKEY_HOME);
   }

   // If there is a backdrop the Guilloche curves need 
   // to be drawn after it
   if ((!m_oldBackground) && (s_withBackdrop))
      m_guilloche.Render();
   
   if (m_oldBackground)
      DrawTime(Metrics::GetCurrentTimePositionOld());
   else
      DrawTime(Metrics::GetCurrentTimePosition());

   if (m_helpMenu)
      RenderSceneGraph(m_helpMenu->GetRootNode());

   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
   glClear(GL_COLOR_BUFFER_BIT);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void App::Notify(const Time &/*time*/)
{
   bsg::KeyEvent::eKeyCode currentKeyAnimationType = m_appAnim.GetKeyAnimation();
   m_appAnim.Reset();
   AnimationDoneNotifier::Reset();

   switch (currentKeyAnimationType)
   {
   case KeyEvent::eKEY_USER_1:
   case KeyEvent::eKEY_USER_2:   // Demo modes
      if (m_demoMode)
         NextDemoStep();
      break;
   case KeyEvent::eKEY_USER_3:   // 'Past' region scroll
      break;

   case KeyEvent::eKEY_GREEN:
   case KeyEvent::eKEY_HOME:
   case KeyEvent::eKEY_F5:
      {
         if (m_longScrolling.IsActive())
         {
            m_longScrolling.Stop();
            m_controller->HighlightSelection(m_selection);
         }
      }
      break;
/*         m_activeRegion.UpdateStartTime(MinVisibleTime());
         m_keepRegion = m_activeRegion.Enlarged(m_activeRegion.GetDuration(), m_activeRegion.GetNumChannels());
         m_controller->SetKeepRegion(m_keepRegion);
         m_selection.MoveToNow(m_controller);
         ChangeDescription();
         m_controller->VisibilityCull(m_activeRegion);
         m_controller->HighlightSelection(m_selection);

         AnimBindingHermiteQuaternionAngle *anim = new AnimBindingHermiteQuaternionAngle(&m_controller->GetPanelRotation());
         anim->Evaluator()->Init(Vec3(1.0f, 0.0f, 0.0f), 90.0f, 0.0f);
         anim->Interpolator()->Init(NowTimestamp(), NowTimestamp() + Metrics::GetHorizPageScrollTime() * 0.5f, 
                                    BaseInterpolator::eLIMIT);
         m_animList.Append(anim);
      }
      break;
*/

   default:
      break;
   }
}

void App::StartDemoMode()
{
   m_demoMode = true;
   m_demoState = DEMO_STATE_WAIT;
   m_demoKey = KeyEvent::eKEY_USER_2;
   NextDemoStep();
}

void App::NextDemoStep()
{
   assert(m_demoMode);
   assert(!m_appAnim.Running());

   // Go to the next demo state
   if (m_demoState == DEMO_STATE_WAIT)
      m_demoState = DEMO_STATE_SCROLL;
   else
      m_demoState = DEMO_STATE_WAIT;


   if (m_demoState == DEMO_STATE_WAIT)
   {
      // Add a pause
      static AnimatableFloat dummy;
      AnimBindingLerpFloat *anim = new AnimBindingLerpFloat(&dummy);
      anim->Init(0.0f, FrameTimestamp(), 1.0f, FrameTimestamp() + Metrics::GetDemoPauseTime(), BaseInterpolator::eLIMIT, this);
      m_appAnim.Set(anim, m_demoKey);
   }
   else  
   {
      if (m_demoKey == KeyEvent::eKEY_USER_2 && m_activeRegion.GetStartIndex() >= m_database.GetNumChannels() - m_activeRegion.GetNumChannels() - 1)
         m_demoKey = KeyEvent::eKEY_USER_1;
      else if (m_demoKey == KeyEvent::eKEY_USER_1 && m_activeRegion.GetStartIndex() <= 0)
         m_demoKey = KeyEvent::eKEY_USER_2;

      m_platform.PushKeyEvent(KeyEvent(m_demoKey, KeyEvent::eKEY_STATE_DOWN, FrameTimestamp()));
   }
}

void App::UpdateRegions(bsg::KeyEvent::eKeyCode keyScrollType)
{
   switch (keyScrollType)
   {
      case KeyEvent::eKEY_GREEN:
      case KeyEvent::eKEY_HOME:
      case KeyEvent::eKEY_F5:
         {
            UpdateRegionsToCurrentPrograms();
         }
         break;

      case KeyEvent::eKEY_BLUE:
      case KeyEvent::eKEY_F8:
         {
            UpdateRegionsToNextDay();
         }
         break;
      case KeyEvent::eKEY_YELLOW:
      case KeyEvent::eKEY_F7:
         {
            UpdateRegionsToPrevDay();
         }
         break;

      default:
         break;
   }
}

void App::UpdateRegionsToNextDay()
{
   Time oneDay(24.0f, Time::eHOURS);
   UpdateRegions(m_activeRegion.GetStartTime() + oneDay, oneDay);
}

void App::UpdateRegionsToPrevDay()
{
   Time minusOneDay(-24.0f, Time::eHOURS);
   Time targetTime = m_selection.GetTime() + minusOneDay;

   if (targetTime < MinVisibleTime())
   {
      targetTime = MinVisibleTime();
   }
   else
   {
      Time offset = targetTime - m_selection.GetTime();
      targetTime = m_activeRegion.GetStartTime() + offset;
   }

   UpdateRegions(targetTime, minusOneDay);
}

void App::UpdateRegionsToCurrentPrograms()
{
   Time offset = MinVisibleTime() - m_selection.GetTime();
   UpdateRegions(MinVisibleTime(), offset);
}

void App::UpdateRegions(const Time & targetTime, const Time &offset)
{
   // Update the regions to display the programs for the current time
   m_activeRegion.UpdateStartTime(targetTime);
   m_keepRegion = m_activeRegion.Enlarged(m_activeRegion.GetDuration(), m_activeRegion.GetNumChannels());
   m_controller->SetKeepRegion(m_keepRegion);
   m_selection.AddTime(m_controller, offset);
   ChangeDescription();
   m_controller->VisibilityCull(m_activeRegion);
   m_controller->HighlightSelection(m_selection);

   m_controller->SetActiveRegion(m_activeRegion);
}

void App::StopDemoMode()
{
   m_demoMode = false;
}

bool App::ScrollChannels(int32_t numLines, Time scrollTime, KeyEvent::eKeyCode keyCode)
{
   if (numLines < 0)
      return ScrollUp((uint32_t)-numLines, scrollTime, keyCode);
   else if (numLines > 0)
      return ScrollDown((uint32_t)numLines, scrollTime, keyCode);
   else
      return false;
}

bool App::ScrollUp(uint32_t numLines, Time scrollTime, KeyEvent::eKeyCode keyCode)
{
   Time now = NowTimestamp(); // Note: Not FrameTimestamp()

   if (m_activeRegion.GetStartIndex() > 0)
   {
      float targetChannel = std::max(0.0f, (float)m_activeRegion.GetStartIndex() - (float)numLines);
      float numScrollChannels = m_activeRegion.GetStartIndex() - targetChannel;

      if (numScrollChannels != numLines)
         scrollTime = scrollTime * ((float)numScrollChannels / numLines);

      m_keepRegion.UpdateStartChannel((int32_t)(m_keepRegion.GetStartIndex() - (float)numScrollChannels));
      m_controller->SetKeepRegion(m_keepRegion);
      m_controller->VisibilityCull(m_activeRegion);
      m_controller->HighlightSelection(m_selection);

      AnimBindingHermiteFloat *animStart = new AnimBindingHermiteFloat(&m_activeRegion.GetStartIndex());
      animStart->Init(m_activeRegion.GetStartIndex(), now, 
                     targetChannel, now + scrollTime, 
                     BaseInterpolator::eLIMIT, this);

      m_appAnim.Set(animStart, keyCode);

      return true;
   }
   return false;
}

bool App::ScrollDown(uint32_t numLines, Time scrollTime, KeyEvent::eKeyCode keyCode)
{
   Time now = NowTimestamp(); // Note: Not FrameTimestamp()

   if (m_activeRegion.GetStartIndex() < m_database.GetNumChannels() - numLines)
   {
      float targetChannel = std::min((float)m_database.GetNumChannels() - numLines, 
                                       m_activeRegion.GetStartIndex() + (float)numLines);
      float numScrollChannels = targetChannel - m_activeRegion.GetStartIndex();

      if (numScrollChannels != numLines)
         scrollTime = scrollTime * ((float)numScrollChannels / numLines);

      m_keepRegion.UpdateStartChannel((int32_t)(m_keepRegion.GetStartIndex() + (float)numScrollChannels));
      m_controller->SetKeepRegion(m_keepRegion);
      m_controller->VisibilityCull(m_activeRegion);
      m_controller->HighlightSelection(m_selection);

      AnimBindingHermiteFloat *animStart = new AnimBindingHermiteFloat(&m_activeRegion.GetStartIndex());
      animStart->Init(m_activeRegion.GetStartIndex(), now, 
         targetChannel, now + scrollTime, 
         BaseInterpolator::eLIMIT, this);

      m_appAnim.Set(animStart, keyCode);

      return true;
   }

   return false;
}

bool App::ScrollLeft(Time offset, Time scrollTime, KeyEvent::eKeyCode keyCode)
{
   if (offset.Milliseconds() < 0.0f)   // Don't scroll right if asked to scroll left
      return false;

   Time now = NowTimestamp(); // Note: Not FrameTimestamp()

   Time targetTime = m_activeRegion.GetStartTime() - offset;
   if (targetTime < MinVisibleTime())
      targetTime = MinVisibleTime();

   Time duration = m_activeRegion.GetStartTime() - targetTime;
   scrollTime = scrollTime * (duration.Hours() / offset.Hours());

   m_keepRegion.UpdateStartTime(m_keepRegion.GetStartTime() - (m_activeRegion.GetStartTime() - targetTime));
   m_controller->SetKeepRegion(m_keepRegion);
   m_controller->VisibilityCull(m_activeRegion);
   m_controller->HighlightSelection(m_selection);

   AnimBindingHermiteTime *animStart = new AnimBindingHermiteTime(&m_activeRegion.GetStartTime());
   animStart->Init(m_activeRegion.GetStartTime(), now, 
      targetTime, now + scrollTime, 
      BaseInterpolator::eLIMIT, this);

   m_appAnim.Set(animStart, keyCode);

   return true;
}

bool App::ScrollRight(Time offset, Time scrollTime, KeyEvent::eKeyCode keyCode)
{
   if (offset.Milliseconds() < 0.0f)   // Don't scroll left if asked to scroll right
      return false;

   Time now = NowTimestamp(); // Note: Not FrameTimestamp()

   m_keepRegion.UpdateStartTime(m_keepRegion.GetStartTime() + offset);
   m_controller->SetKeepRegion(m_keepRegion);
   m_controller->VisibilityCull(m_activeRegion);
   m_controller->HighlightSelection(m_selection);

   AnimBindingHermiteTime *animStart = new AnimBindingHermiteTime(&m_activeRegion.GetStartTime());
   animStart->Init(m_activeRegion.GetStartTime(), now, 
      m_activeRegion.GetStartTime() + offset, 
      now + scrollTime, 
      BaseInterpolator::eLIMIT, this);

   m_appAnim.Set(animStart, keyCode);

   return true;
}

void App::ChangeDescription()
{
   m_descNode->ClearGeometry();

   Time start = m_selection.GetProgrammeStart();
   Time end   = m_selection.GetProgrammeEnd();

   const ProgramInfo &progInfo = m_selection.GetProgInfo();

   const std::string &titleStr = progInfo.GetTitle();
   const std::string &descStr  = progInfo.GetDescription();

   std::string infoStr = start.CalendarTimeString("%A\n%B ") + DayString(start.CalendarDay() - 1) + 
                         start.CalendarTimeString("\n%H:%M - ") + end.CalendarTimeString("%H:%M");

   PrintHandle title(New);
   title->SetFont(m_titleFont);

   std::vector<std::string>  metas;

   if (progInfo.IsHD())
      metas.push_back("HD");
   if (progInfo.HasSubtitles())
      metas.push_back("Subtitled");
   if (progInfo.HasAudioDescription())
      metas.push_back("Audio Description");
   if (progInfo.HasSigning())
      metas.push_back("Signed");

   if (metas.size() > 0)
   {
      std::string metaStr = metas[0];

      for (uint32_t i = 1; i < metas.size(); ++i)
      {
         metaStr += ", ";
         metaStr += metas[i];
      }

      PrintHandle    metaText(New);

      metaText->SetFont(m_descFont, PrintOptions(PrintOptions::eTOP_RIGHT));
      metaText->SetText(metaStr, Metrics::GetMetaTextSize(), Vec2(Metrics::GetMetaTextRight(), Metrics::GetTitleTextPosition().Y()));
      metaText->SetUniform("u_textColor", Metrics::GetMetaTextColour());

      m_descNode->AppendGeometry(metaText);
   }

   title->SetText(titleStr, Metrics::GetTitleTextHeight(), Metrics::GetTitleTextPosition());
   title->SetUniform("u_textColor", Metrics::GetTitleTextColour());

   PrintHandle text(New);
   text->SetFont(m_descFont);
   text->SetFormatter(new PrintRaggedRight(Metrics::GetDescTextLimit()));
   text->SetText(descStr, Metrics::GetDescTextHeight(), Metrics::GetDescTextPosition());
   text->SetUniform("u_textColor", Metrics::GetDescTextColour());

   PrintHandle info(New);
   info->SetFont(m_descFont);
   info->SetText(infoStr, Metrics::GetInfoTextHeight(), Metrics::GetInfoTextPosition());
   info->SetUniform("u_textColor", Metrics::GetInfoTextColour());

   m_descNode->AppendGeometry(title);
   m_descNode->AppendGeometry(info);
   m_descNode->AppendGeometry(text);
}

void App::RotateGUI(KeyEvent::eKeyCode code,Time now)
{
   VectorOfAnim rotationAnimations = m_guiDisplay.Rotate(now, this);

   unsigned int nbAnims = rotationAnimations.size();

   for (unsigned int index=0; index < nbAnims; index++)
   {
      m_appAnim.Set(rotationAnimations[index], code, false);
   }

}

Time App::CalculateHowFarToScroll(const bool scrollRight) const
{
   // How far shall we scroll?
   Time target = m_activeRegion.GetStartTime() + Time(30, Time::eMINUTES);
   Time pStart = m_selection.GetTime();
   Time pStartRounded;
   if (pStart.CalendarMinute() > 30)
      pStartRounded = Time(pStart.CalendarYear(), pStart.CalendarMonth(), pStart.CalendarDay(), pStart.CalendarHour(), 30, 0);
   else
      pStartRounded = Time(pStart.CalendarYear(), pStart.CalendarMonth(), pStart.CalendarDay(), pStart.CalendarHour(), 0, 0);

   if (scrollRight)
      return (pStartRounded - target);
   else
      return (target - pStartRounded);
}

void App::MoveSelection(const bool moveSelectionRight, const KeyEvent::eKeyCode code)
{
   bool selectionMoved = false;

   if (moveSelectionRight)
      selectionMoved = m_selection.MoveRight(m_controller);
   else
      selectionMoved = m_selection.MoveLeft(m_controller);

   if (selectionMoved)
   {
      ChangeDescription();

      if (!m_activeRegion.ContainsTime(m_selection.GetProgrammeStart()) ||
            !m_activeRegion.ContainsTime(m_selection.GetProgrammeEnd()))
      {
         // if scrolling left and the start time is before the visible time
         if (!moveSelectionRight && (m_activeRegion.GetStartTime() <= MinVisibleTime()))
         {
            // No scrolling to be done
         }
         else
         {
            Time offset = CalculateHowFarToScroll(moveSelectionRight);
            if (moveSelectionRight)
               ScrollRight(offset, Metrics::GetHorizPageScrollTime(), code);
            else
               ScrollLeft(offset, Metrics::GetHorizPageScrollTime(), code);
         }
      }
   }
}

void App::KeyEventHandler(KeyEvents &queue)
{
   Time now = FrameTimestamp();

   if (queue.Pending())
   {
      KeyEvent ev = queue.Peek();

      if (ev.State() == KeyEvent::eKEY_STATE_DOWN)
      {
         // Ignore events when we're already animating
         switch (ev.Code())
         {
         case KeyEvent::eKEY_UP:
         case KeyEvent::eKEY_DOWN:
         case KeyEvent::eKEY_CH_UP:
         case KeyEvent::eKEY_CH_DOWN:
         case KeyEvent::eKEY_LEFT:
         case KeyEvent::eKEY_RIGHT:
         case KeyEvent::eKEY_FAV1:        // Rotate
         case KeyEvent::eKEY_F1:          // Rotate
         case KeyEvent::eKEY_USER_1:      // Demo modes
         case KeyEvent::eKEY_USER_2:      // Demo modes
         case KeyEvent::eKEY_GREEN:       // Display program at the current time
         case KeyEvent::eKEY_HOME:        // Display program at the current time
         case KeyEvent::eKEY_F5:          // Display program at the current time
         case KeyEvent::eKEY_BLUE:        // Move to the next day
         case KeyEvent::eKEY_F8:
         case KeyEvent::eKEY_YELLOW:      // Move to the previous day
         case KeyEvent::eKEY_F7:
            if (m_appAnim.Running())
               return;
            break;
         case KeyEvent::eKEY_FAV2:
         case KeyEvent::eKEY_F2:
            if ((m_appAnim.Running())     // if an animation is running
               && (!m_demoMode))
               return;                    // Don't start a new demo animation
            break;
         default:
            break;
         }

         switch (ev.Code())
         {
         case KeyEvent::eKEY_FAV2:
         case KeyEvent::eKEY_F2:
            if (m_demoMode)
               StopDemoMode();
            else
               StartDemoMode();
            queue.Pop();
            return;
            break;
         case KeyEvent::eKEY_USER_1:
         case KeyEvent::eKEY_USER_2:
            break;   // Handle below
         default:    // If any other key pressed stop the demo
            if (m_demoMode)
               StopDemoMode();
            break;
         }
         
         switch (ev.Code())
         {
         // Rotate the panels
         case KeyEvent::eKEY_FAV1:
         case KeyEvent::eKEY_F1:
         {
            RotateGUI(ev.Code(), now);
            break;
         }

         // Exit the application
         case KeyEvent::eKEY_EXIT :
         case KeyEvent::eKEY_ESC :
         case KeyEvent::eKEY_POWER :
            Stop(255);
            break;

         // Display the program for the current time
         // The animation will use eKEY_HOME for in the notify function
         case KeyEvent::eKEY_GREEN:
         case KeyEvent::eKEY_HOME:
         case KeyEvent::eKEY_F5:
            {
               if (m_keepRegion.ContainsTime(FrameTimestamp()))
               {
                  m_selection.MoveToNow(m_controller);
                  ChangeDescription();

                  if (!m_activeRegion.ContainsTime(m_selection.GetProgrammeStart()) ||
                     !m_activeRegion.ContainsTime(m_selection.GetProgrammeEnd()))
                  {
                     if (m_activeRegion.GetStartTime() <= MinVisibleTime())
                     {
                        // No scrolling to be done
                     }
                     else
                     {
                        bool scrollRight = false;
                        Time offset = CalculateHowFarToScroll(scrollRight);
                        ScrollLeft(offset, Metrics::GetHorizPageScrollTime(), ev.Code());
                     }
                  }
               }
               else
               {
                  // If the current program is out of the keep region
                  // we scroll from the current active region to 
                  // an active region containing the current programs
                  if (! m_longScrolling.IsActive())
                  {
                     m_longScrolling.Start(ev.Code());
                     m_controller->HighlightSelection(m_selection, false);
                  }
               }
            }
            break;

         // Display programs for a day later
         case KeyEvent::eKEY_BLUE:
         case KeyEvent::eKEY_F8:
            {
               if (! m_longScrolling.IsActive())
               {
                  m_longScrolling.Start(ev.Code());
                  m_controller->HighlightSelection(m_selection, false);
               }
            }
            break;

         // Display programs for a day earlier
         case KeyEvent::eKEY_YELLOW:
         case KeyEvent::eKEY_F7:
            {
               // If it is possible to scroll back in time
               // Not scrolling if the selection  is withing 30 minutes from the minimum visible time
               if (m_selection.GetProgrammeStart() > (MinVisibleTime() + Time(30, Time::eMINUTES)))
               {
                  if (! m_longScrolling.IsActive())
                  {
                     m_longScrolling.Start(ev.Code());
                     m_controller->HighlightSelection(m_selection, false);
                  }
               }
            }
            break;

         case KeyEvent::eKEY_RED:
         case KeyEvent::eKEY_F9:
            {
               if (m_helpMenu) 
                  m_helpMenu->ToggleMenu();
            }
            break;

         case KeyEvent::eKEY_UP:
            if (m_selection.ChannelChange(-1, m_controller))
            {
               ChangeDescription();
               if (!m_activeRegion.ContainsChannel(m_selection.GetChannel()))
                  ScrollUp((uint32_t)m_activeRegion.GetStartIndex() - m_selection.GetChannel(), 
                           Metrics::GetLineScrollTime(), ev.Code());
            }
            break;

         case KeyEvent::eKEY_DOWN:
            if (m_selection.ChannelChange(1, m_controller))
            {
               ChangeDescription();
               if (!m_activeRegion.ContainsChannel(m_selection.GetChannel()))
                  ScrollDown(m_selection.GetChannel() - ((uint32_t)m_activeRegion.GetStartIndex() + m_activeRegion.GetNumChannels() - 1), 
                              Metrics::GetLineScrollTime(), ev.Code());
            }
            break;

         // Automatic scroll in demo mode
         case KeyEvent::eKEY_USER_1:
         case KeyEvent::eKEY_CH_UP:
            if (m_selection.ChannelChange(-(int32_t)m_activeRegion.GetNumChannels(), m_controller))
            {
               ChangeDescription();
               if (!m_activeRegion.ContainsChannel(m_selection.GetChannel()))
                  ScrollUp(m_activeRegion.GetNumChannels(), 
                           m_demoMode ? Metrics::GetDemoPageScrollTime() : Metrics::GetPageScrollTime(),
                           ev.Code());
            }
            break;

         // Automatic scroll in demo mode
         case KeyEvent::eKEY_USER_2:
         case KeyEvent::eKEY_CH_DOWN:
            if (m_selection.ChannelChange((int32_t)m_activeRegion.GetNumChannels(), m_controller))
            {
               ChangeDescription();
               if (!m_activeRegion.ContainsChannel(m_selection.GetChannel()))
                  ScrollDown(m_activeRegion.GetNumChannels(), 
                              m_demoMode ? Metrics::GetDemoPageScrollTime() : Metrics::GetPageScrollTime(),
                              ev.Code());
            }
            break;

         case KeyEvent::eKEY_LEFT:
            {
               bool moveRight = false;
               MoveSelection(moveRight, ev.Code());
            }
            break;

         case KeyEvent::eKEY_RIGHT:
            {
               bool moveRight = true;
               MoveSelection(moveRight, ev.Code());
            }
            break;

         case KeyEvent::eKEY_1:
            {
               //RenderToFBO();
               //m_mode = BACKGROUND_FUZZY;
            }
            break;

         default : 
            break;
         }
      }

      queue.Pop();

      // Clear the queue if we are taking too long to process the events
      if (ev.GetTimestamp() < FrameTimestamp() - Time(1, Time::eSECONDS))
         queue.Clear();
   }
}

void App::ResizeHandler(uint32_t width, uint32_t height)
{
   m_camera->SetAspectRatio(width, height);
   m_pBackdrop->ResizeHandler(width, height);
   m_smallFont->SetFontInPercent("DroidSans-Bold.ttf", 2.6f, height);
   if (m_helpMenu)
      m_helpMenu->Resize();
}

bsg::Time App::MinVisibleTime() const
{
   Time now = FrameTimestamp();
   if (now.CalendarMinute() >= 30)
      return Time(now.CalendarYear(), now.CalendarMonth(), now.CalendarDay(), now.CalendarHour(), 30, 0);
   else
      return Time(now.CalendarYear(), now.CalendarMonth(), now.CalendarDay(), now.CalendarHour(), 0, 0);
}

void App::BackdropVisible(bool visible)
{ 
   if (visible) 
      m_pBackdrop->RootNode()->SetOpacity(1);
   else
      m_pBackdrop->RootNode()->SetOpacity(0);
}

MaterialHandle GetPanelMaterial()
{
   assert(theApp != 0);

   return theApp->GetPanelMaterial();
}

float TimeToXCoord(const bsg::Time &time) 
{ 
   return time.FloatHours() * Metrics::GetPanelWidthForOneHour();
}

extern float ChannelStride()
{
   return Metrics::GetChannelSpacing();
}

}

int main(int argc, char **argv)
{
   uint32_t ret = 0;

   try
   {
      ApplicationOptions   options;

      options.SetDisplayBits(32, 24, 8);        // We need destination alpha, depth & stencil
      options.SetDisplayDimensions(1280, 720);

      pg::CustomArgumentParser customParser;
      if (!options.ParseCommandLine(argc, argv, &customParser))
         return 1;

      Platform platform(options);
      pg::App  app(platform);

      ret = platform.Exec();
   }
   catch (Exception &e)
   {
      std::cerr << "Exception : " << e.Message() << "\n";
   }

   return ret;
}


