/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_H__
#define __PG_H__

#include "bsg_application.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"
#include "bsg_material.h"
#include "bsg_time.h"
#include "bsg_print.h"
#include "bsg_vector.h"

#include "pg_info.h"
#include "pg_controller.h"
#include "pg_region.h"

#include "pg_longscrolling.h"
#include "pg_gui_display.h"

#include "../common/bcm_backdrop.h"
#include "../common/bcm_guilloche.h"
#include "../common/bcm_help_menu.h"

namespace pg
{

bsg::MaterialHandle GetPanelMaterial();

extern float TimeToXCoord(const bsg::Time &time);
extern float ChannelStride();

// Placeholder for layout information
class Metrics
{
public:
   // Scale font texture sizes based on display resolution
   static void SetPointScale(float scale);

   static bsg::Vec4  GetBackgroundColour();
   static bsg::Vec2  GetCurrentTimePosition();
   static bsg::Vec2  GetCurrentTimePositionOld();

   static float      GetPanelXInset();
   static float      GetPanelYInset();

   static bsg::Vec4  GetPanelColourEven();
   static bsg::Vec4  GetPanelColourOdd();

   static float      GetPanelTextScale();
   static float      GetPanelTextPoints();
   static bsg::Vec2  GetPanelTextPosition();
   static bsg::Vec4  GetPanelTextColour();

   static float      GetDescTextHeight();
   static bsg::Vec2  GetDescTextPosition();
   static bsg::Vec4  GetDescTextColour();
   static float      GetDescTextLimit();
   static float      GetDescTextPoints();

   static float      GetTitleTextHeight();
   static bsg::Vec2  GetTitleTextPosition();
   static bsg::Vec4  GetTitleTextColour();
   static float      GetTitleTextPoints();

   static float      GetMetaTextRight();
   static float      GetMetaTextSize();
   static bsg::Vec4  GetMetaTextColour();

   static float      GetInfoTextHeight();
   static bsg::Vec2  GetInfoTextPosition();
   static float      GetInfoTextLimit();
   static bsg::Vec4  GetInfoTextColour();
   static float      GetInfoTextPoints();

   static float      GetPanelWidthForOneHour();
   static bsg::Time  GetActiveRegionDuration();

   static float      GetTimeTextScale();
   static float      GetTimeTextPoints();
   static bsg::Vec2  GetTimeTextPosition();
   static bsg::Vec4  GetTimeTextColour();
   static bsg::Vec2  GetTimeTickSize();
   static bsg::Vec4  GetTimeTickColour();

   static float      GetChannelTextScale();
   static float      GetChannelTextPoints();
   static bsg::Vec2  GetChannelTextPosition();
   static bsg::Vec4  GetChannelTextColour();
   static float      GetChannelTextWidth();

   static float      GetChannelSpacing();

   static bsg::Time  GetPageScrollTime();
   static bsg::Time  GetDemoPageScrollTime();
   static bsg::Time  GetDemoPauseTime();

   static bsg::Time  GetLineScrollTime();
   static bsg::Time  GetHorizPageScrollTime();
   static bsg::Time  GetMaxHorizScrollDist();

   static bool       GetDisableStenciling();
   static bool       GetUseDebugCamera();
   static bool       GetDoVisibilityCulling();

   static const char *GetNoScheduleText();
   static const char *GetNoScheduleDescription();
   static const char *GetNoProgrammeText();
   static const char *GetNoProgrammeDescription();

   static bsg::Vec4  GetHighlightColour();

private:
   static float m_pointScale;
};

enum Priority
{
   // Smaller priorities are drawn earlier
   PANEL_STENCIL           = -10000,
   FRAME_STENCIL           = -9000,
   PANEL_GEOM_BASE         = -8000,
   BACKDROP                = -7000,
   UNSET                   = 0,
   PANEL_TEXT_BASE         = 10000,
   FRAME_TEXT              = 11000,
   HIGHLIGHT               = 20000,
   OVERLAY                 = 100000
};

class App;

// Used to truncate text within a box -- adds ".." on the end if it is shortened.
// If the text is too short, it returns "-" and if it is too short for that ""
class ShortenFormatter : public bsg::PrintFormatter
{
public:
   ShortenFormatter(float limit) :
      m_limit(limit),
      m_hasShortened(false)
   {}

   virtual const std::vector<uint32_t> &Format(const std::vector<uint32_t> &str);

   bool GetHasShortened() const { return m_hasShortened; }

private:
   std::vector<uint32_t>   m_result;
   float                   m_limit;
   bool                    m_hasShortened;
};

class Selection
{
public:
   Selection() :
      m_channel(0),
      m_maxChannel(0),
      m_changed(false)
   {}

   Selection(const bsg::Time &time, uint32_t channel, uint32_t maxChannel) :
      m_time(time),
      m_channel(channel),
      m_maxChannel(maxChannel),
      m_changed(false)
   {}

   const bsg::Time   &GetTime() const     { return m_time;           }
   uint32_t          GetChannel() const   { return m_channel;        }

   bool ChannelChange(int32_t dc, Controller *controller);
   bool MoveRight(Controller *controller);
   bool MoveLeft(Controller *controller);
   void MoveToNow(Controller *controller);
   void AddTime(Controller *controller, const bsg::Time &offset);

   void Highlight(Controller *controller);

   const bsg::Time &GetProgrammeStart() const   { return m_progInfo.GetStartTime();   }
   const bsg::Time &GetProgrammeEnd() const     { return m_progInfo.GetEndTime();     }

   const ProgramInfo &GetProgInfo() const       { return m_progInfo;    }
   void SetProgInfo(const pg::ProgramInfo &val) { m_progInfo = val;     }

   bool HasChanged() const { return m_changed; }

private:
   bsg::Time   m_time;
   uint32_t    m_channel;
   uint32_t    m_maxChannel;
   ProgramInfo m_progInfo;
   bool        m_changed;
};

// Helper class to manage the main animation of PG application
class AppAnimation
{
public:
   // Default constructor
   AppAnimation();

   // Add an animation to the animation list
   // if single is true this function only add
   // the animation if there is already a main animation running
	bool Set(bsg::AnimBindingBase *m_currentKeyAnimation,
            bsg::KeyEvent::eKeyCode m_currentKeyAnimationType,
            bool single = true);

   // Reset flags showing an main animation running
   void Reset()   { m_currentKeyAnimation = NULL; m_currentKeyAnimationType = bsg::KeyEvent::eKEY_NONE; };

   // return true if a main animation is running
   bool Running() const { return (m_currentKeyAnimation != NULL); };

   // Retrieves the key associated to an animation
   bsg::KeyEvent::eKeyCode GetKeyAnimation() { return m_currentKeyAnimationType; };

   // Returns the animation list used by the application
   bsg::AnimationList & GetAnimList() { return m_animList; }

   // Update the time for the animation in the animation list
   bool UpdateTime(bsg::Time time) { return m_animList.UpdateTime(time); };

private:
   // Constructor by copy not implemented
   AppAnimation(const AppAnimation &);
   // Assignment operator not implemented
   const AppAnimation &operator=(const AppAnimation &);

   // Main animation list for this application
   bsg::AnimationList      m_animList;
   // Flag used to check if a main animation is running
   bsg::AnimBindingBase    *m_currentKeyAnimation;
   // Key associated with the main animation running
   bsg::KeyEvent::eKeyCode m_currentKeyAnimationType;
};

class BCMGuilloche
{
public:
   BCMGuilloche();
   void Init(bsg::Application *app);
   void Render();
   void UpdateAnimationList(const bsg::Time &now);
   const bsg::SceneNodeHandle &RootNode() { return m_guillochePanel.GetRoot(); };
private:
   bsg::Application     *m_pApp;
   uint32_t             m_numControls;
   Guilloche            m_guilloche;
   GuillochePanel       m_guillochePanel;
   bsg::AnimationList   m_animList;
};

class App : public bsg::Application, public bsg::AnimationDoneNotifier
{
public:
   App(bsg::Platform &platform);
   ~App();

   // Overridden methods
   virtual bool UpdateFrame(int32_t *idleMs);
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual void ResizeHandler(uint32_t width, uint32_t height);

   virtual void Notify(const bsg::Time &time); // From AnimationDoneNotifier

   bsg::MaterialHandle &GetPanelMaterial() { return m_panelMaterial; }
   bsg::AnimationList &AnimList() { return m_appAnim.GetAnimList(); }

   void StartDemoMode();
   void NextDemoStep();
   void StopDemoMode();
   bsg::Time MinVisibleTime() const;

   void BackdropVisible(bool visible);

   // Update the active and keep regions according to a key type
   void UpdateRegions(bsg::KeyEvent::eKeyCode keyScrollType);

private:
   enum DemoStates {
      DEMO_STATE_WAIT = 0,
      DEMO_STATE_SCROLL = 1
   };

   void DrawTime(const bsg::Vec2 &pos);

   bool ScrollUp(uint32_t numLines, bsg::Time scrollTime, bsg::KeyEvent::eKeyCode keyCode);
   bool ScrollDown(uint32_t numLines, bsg::Time scrollTime, bsg::KeyEvent::eKeyCode keyCode);
   bool ScrollChannels(int32_t numLines, bsg::Time scrollTime, bsg::KeyEvent::eKeyCode keyCode);
   bool ScrollLeft(bsg::Time offset, bsg::Time scrollTime, bsg::KeyEvent::eKeyCode keyCode);
   bool ScrollRight(bsg::Time offset, bsg::Time scrollTime, bsg::KeyEvent::eKeyCode keyCode);
   void ChangeDescription();

   // Update the active and keep regions to display the current programs
   void UpdateRegionsToCurrentPrograms();
   // Update the active and keep regions to display the next day programs
   void UpdateRegionsToNextDay();
   // Update the active and keep regions to display the previous day programs
   void UpdateRegionsToPrevDay();
   // Helper function to update the active an keep regions
   void UpdateRegions(const bsg::Time & targetTime, const bsg::Time &offset);

   void RotateGUI(const bsg::KeyEvent::eKeyCode code, const bsg::Time now);
   bsg::Time CalculateHowFarToScroll(const bool scrollRight) const;
   void MoveSelection(const bool moveSelectionRight, const bsg::KeyEvent::eKeyCode code);

   bsg::Platform           &m_platform;

   GridInfo                m_database;
   bsg::CameraHandle       m_camera;
   bsg::SceneNodeHandle    m_cameraNode;
   bsg::SceneNodeHandle    m_rootNode;
   bsg::SceneNodeHandle    m_descNode;
   bsg::SceneNodeHandle    m_stencilRectRoot;

   bsg::MaterialHandle     m_panelMaterial;

   Selection               m_selection;

   Controller              *m_controller;
   Region                  m_activeRegion;
   Region                  m_keepRegion;
   Region                  m_lastActiveRegion;

   AppAnimation            m_appAnim;

   bool                    m_oldBackground;
   BCMBackdrop             *m_pBackdrop;
   BCMGuilloche            m_guilloche;

   bsg::FontHandle         m_smallFont;
   bsg::PrintFontHandle    m_descFont;
   bsg::PrintFontHandle    m_titleFont;

   bool                    m_demoMode;
   DemoStates              m_demoState;
   bsg::KeyEvent::eKeyCode m_demoKey;

   GUIDisplay              m_guiDisplay;

   LongScrolling           m_longScrolling;

   HelpMenu                *m_helpMenu;
};

extern App *theApp;
}

#endif
