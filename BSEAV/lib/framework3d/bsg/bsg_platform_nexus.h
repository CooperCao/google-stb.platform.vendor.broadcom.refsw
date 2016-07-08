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
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_core_utils.h"

#include "nexus_hdmi_output.h"
#include "nexus_component_output.h"
#include "nexus_hdmi_output_hdcp.h"

#include "bchp_common.h"
#include "bchp_memc_arb_0.h"

#define IR_INPUT
#ifdef IR_INPUT
#include "nexus_ir_input.h"
#endif

#if NEXUS_HAS_INPUT_ROUTER
#include "nexus_input_client.h"
#endif

#include "default_nexus.h"
#include "display_nexus.h"

#include "bkni.h"

#include <sys/times.h>
#include <fcntl.h>
#include <unistd.h>
#include <list>

namespace bsg
{

class Platform;

// @cond

typedef struct DisplaySessionHandles
{
   NEXUS_HdmiOutputHandle  hdmi;
   NEXUS_DisplayHandle     display;
} DisplaySessionHandles;

class PlatformDataNexus : public Platform::PlatformData
{
public:
   PlatformDataNexus(Platform *platform) :
      m_platform(platform),
#ifdef IR_INPUT
      m_IRHandle(NULL),
#endif
      m_platformHandle(NULL),
#ifdef SINGLE_PROCESS
      m_nexusDisplay(NULL),
      m_sessionHandles(NULL),
      m_kbdThread(0),
      m_mouseThread(0),
      m_kbdFd(0),
      m_mouseFd(0),
#endif
      m_primaryProcess(true),
      m_idling(false),
      m_componentOn(false),
      m_compositeOn(false),
      m_hdmiOn(false),
      m_timeout(0),
      m_event(NULL)
   {
      m_lastCPUTicks = times(&m_lastCPUUsage);
   }

   ~PlatformDataNexus()
   {
      TermUSBInputs();
#ifdef SINGLE_PROCESS
      delete m_sessionHandles;
#endif
   }

   void InitRoutedInputs();
   void TermRoutedInputs();

   void InitUSBInputs();
   void TermUSBInputs();
   KeyEvent::eKeyCode MapKeyCode(unsigned int rawCode) const;

   void InitPanelOutput();
   void InitComponentOutput();
   void TermComponentOutput();
   void InitCompositeOutput(uint32_t w, uint32_t h);
   void TermCompositeOutput();
   void InitHDMIOutput(bool forceHDMI);
   void TermHDMIOutput();

public:
   Platform                      *m_platform;
#ifdef IR_INPUT
   NEXUS_IrInputHandle           m_IRHandle;
   NEXUS_IrInputMode             m_IRMode;
#endif
   std::list<NXPL_NativeWindowInfoEXT> m_winInfo;
   NXPL_PlatformHandle           m_platformHandle;
#ifdef SINGLE_PROCESS
   NEXUS_DisplayHandle           m_nexusDisplay;
   DisplaySessionHandles         *m_sessionHandles;
   pthread_t                     m_kbdThread;
   pthread_t                     m_mouseThread;
   int                           m_kbdFd;
   int                           m_mouseFd;
#else
#if NEXUS_HAS_INPUT_ROUTER
   NEXUS_InputClientHandle       m_inputRouterClient;
#endif
#endif

   NEXUS_PlatformConfiguration   m_platformConfig;
   bool                          m_primaryProcess;
   bool                          m_idling;
   bool                          m_componentOn;
   bool                          m_compositeOn;
   bool                          m_hdmiOn;
   int32_t                       m_timeout;
   BKNI_EventHandle              m_event;

   struct tms                    m_lastCPUUsage;
   clock_t                       m_lastCPUTicks;
};


class NexusPixmapData : public NativePixmapData
{
public:
   NexusPixmapData() :
      m_surface(NULL)
   {
   }

   virtual ~NexusPixmapData()
   {
   }

public:
   NEXUS_SurfaceHandle  m_surface;
};

// @endcond

}

