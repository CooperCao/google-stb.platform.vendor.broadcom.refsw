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
