/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_HUD_H__
#define __BSG_HUD_H__

#include "bsg_common.h"
#include "bsg_circular_index.h"
#include "bsg_font.h"
#include "bsg_key_events.h"

namespace bsg
{

// @cond

//! The head-up-display object.
//! Handles the drawing of the HUD menu, and the processing of key input whilst the HUD is visible.
class DevHud
{
   enum eIndex
   {
      eSWAP_INTERVAL = 0,
      eFPS_HUD,
      eSTEREO,
      eRATE_MULTIPLIER,
      eFRAME_GRAB
   };

public:
   DevHud();
   ~DevHud();

   //! Render the HUD now
   void Draw();

   //! Handle a KeyEvent, as the HUD is active
   void HandleKey(const KeyEvent &ev);

   //! Resize fonts etc.
   void Resize(uint32_t w, uint32_t h);

private:
   const char *Mark(eIndex item) const;

private:
#ifndef BSG_STAND_ALONE
   FontHandle       m_hudFont;
#endif
   CircularIndex    m_hudSel;
   int32_t          m_swapInterval;
   bool             m_stereo;
   float            m_rateMultiplier;
   bool             m_fpsHUD;
   bool             m_changed;
};

#ifndef BSG_STAND_ALONE

//! The head-up-display object.
//! Handles the drawing of the HUD menu, and the processing of key input whilst the HUD is visible.
class FpsHud
{
public:
   FpsHud();
   ~FpsHud();

   //! Render the HUD now
   void Draw();

   //! Resize fonts etc.
   void Resize(uint32_t w, uint32_t h);

private:
   FontHandle  m_fpsFont;
};

#else

//! Stub version of FpsHud
class FpsHud
{
public:
   FpsHud() {}
   ~FpsHud() {}

   void Draw() {}
   void Resize(uint32_t w, uint32_t h) {}
};

#endif

}

// @endcond

#endif /* __BSG_HUD_H__ */
