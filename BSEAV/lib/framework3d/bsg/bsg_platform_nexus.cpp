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

#include "bsg_application.h"
#include "bsg_exception.h"
#include "bsg_platform_nexus.h"

#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <map>

#ifndef SINGLE_PROCESS
#include "nxclient.h"
#endif

namespace bsg
{

#define platform_data ((PlatformDataNexus*)(m_platform))
#if BCHP_CHIP==935230
#define RAW_EXIT 0x5da2a55a
#else
#define RAW_EXIT 0x0000d012
#endif


KeyEvent::eKeyCode PlatformDataNexus::MapKeyCode(unsigned int rawCode) const
{
   KeyEvent::eKeyCode code = KeyEvent::KEY_NONE;

#ifdef IR_INPUT

#if ((BCHP_CHIP==35230) || (BCHP_CHIP==35233))
   switch (rawCode)
   {
   case 0x5da2a55a : code = KeyEvent::KEY_EXIT; break;
   case 0x56a9a45b : code = KeyEvent::KEY_UP; break;
   case 0x57a8a45b : code = KeyEvent::KEY_DOWN; break;
   case 0x53aca45b : code = KeyEvent::KEY_LEFT; break;
   case 0x4ab5a45b : code = KeyEvent::KEY_RIGHT; break;
   case 0x1de2a55a : code = KeyEvent::KEY_1; break;
   case 0x857aa55a : code = KeyEvent::KEY_2; break;
   case 0x0df2a55a : code = KeyEvent::KEY_3; break;
   case 0x4db2a55a : code = KeyEvent::KEY_4; break;
   case 0x11eea55a : code = KeyEvent::KEY_5; break;
   case 0x47b8a55a : code = KeyEvent::KEY_6; break;
   case 0x0cf3a55a : code = KeyEvent::KEY_7; break;
   case 0x0ff0a55a : code = KeyEvent::KEY_8; break;
   case 0x0ef1a55a : code = KeyEvent::KEY_9; break;
      /* case 0x4cb3a55a : code = KeyEvent::KEY_0; break; */
   case 0x4cb3a55a : code = KeyEvent::KEY_OK; break;
      /* case 0x1ce3a55a : code = KeyEvent::KEY_POWER; break; */
   case 0x1ce3a55a : code = KeyEvent::KEY_MENU; break;
   case 0x0af5a55a : code = KeyEvent::KEY_VOL_UP; break;
   case 0x0bf4a55a : code = KeyEvent::KEY_VOL_DOWN; break;
   case 0x10efa45b : code = KeyEvent::KEY_CH_UP; break;
   case 0x11eea45b : code = KeyEvent::KEY_CH_DOWN; break;
   default : code = KeyEvent::KEY_UNKNOWN; break;
   }
#else

   if (m_IRMode == NEXUS_IrInputMode_eCirNec)
   {
      switch (rawCode)
      {
      case 0xb24dff00 : code = KeyEvent::KEY_EXIT; break;
      case 0xf50aff00 : code = KeyEvent::KEY_POWER; break;
      case 0xb14eff00 : code = KeyEvent::KEY_UP; break;
      case 0xf30cff00 : code = KeyEvent::KEY_DOWN; break;
      case 0xf40bff00 : code = KeyEvent::KEY_LEFT; break;
      case 0xb649ff00 : code = KeyEvent::KEY_RIGHT; break;
      case 0xf708ff00 : code = KeyEvent::KEY_OK; break;
      case 0xe01fff00 : code = KeyEvent::KEY_1; break;
      case 0xa15eff00 : code = KeyEvent::KEY_2; break;
      case 0xa05fff00 : code = KeyEvent::KEY_3; break;
      case 0xe41bff00 : code = KeyEvent::KEY_4; break;
      case 0xa55aff00 : code = KeyEvent::KEY_5; break;
      case 0xa45bff00 : code = KeyEvent::KEY_6; break;
      case 0xe817ff00 : code = KeyEvent::KEY_7; break;
      case 0xa956ff00 : code = KeyEvent::KEY_8; break;
      case 0xa857ff00 : code = KeyEvent::KEY_9; break;
      case 0xad52ff00 : code = KeyEvent::KEY_0; break;
      case 0xb748ff00 : code = KeyEvent::KEY_VOL_UP; break;
      case 0xb34cff00 : code = KeyEvent::KEY_VOL_DOWN; break;
      case 0xf609ff00 : code = KeyEvent::KEY_CH_UP; break;
      case 0xf20dff00 : code = KeyEvent::KEY_CH_DOWN; break;
      case 0xb04fff00 : code = KeyEvent::KEY_MENU; break;
      case 0xe11eff00 : code = KeyEvent::eKEY_PICTURE; break;
      case 0xe51aff00 : code = KeyEvent::eKEY_ASPECT; break;
      case 0xe916ff00 : code = KeyEvent::eKEY_PIP; break;
      case 0xed12ff00 : code = KeyEvent::eKEY_SWAP; break;
      case 0xf807ff00 : code = KeyEvent::eKEY_APPS; break;
      case 0xb946ff00 : code = KeyEvent::eKEY_FUNA; break;
      case 0xf10eff00 : code = KeyEvent::eKEY_GUIDE; break;
      case 0xa25dff00 : code = KeyEvent::eKEY_REVERSE; break;
      case 0xfd02ff00 : code = KeyEvent::eKEY_FAV1; break;
      case 0xfc03ff00 : code = KeyEvent::eKEY_FAV2; break;
      case 0xbd42ff00 : code = KeyEvent::eKEY_FAV3; break;
      case 0xbc43ff00 : code = KeyEvent::eKEY_FAV4; break;
      case 0xbb44ff00 : code = KeyEvent::eKEY_FREEZE; break;
      case 0xfb04ff00 : code = KeyEvent::eKEY_PHOTO; break;
      case 0xfa05ff00 : code = KeyEvent::eKEY_MUSIC; break;
      case 0xba45ff00 : code = KeyEvent::eKEY_VIDEO; break;
      case 0xbe41ff00 : code = KeyEvent::eKEY_AUDIO; break;
      case 0xb54aff00 : code = KeyEvent::eKEY_SOURCE; break;
      case 0xb44bff00 : code = KeyEvent::eKEY_WATCHTV; break;
      case 0xaf50ff00 : code = KeyEvent::eKEY_RED; break;
      case 0xef10ff00 : code = KeyEvent::eKEY_GREEN; break;
      case 0xee11ff00 : code = KeyEvent::eKEY_YELLOW; break;
      case 0xae51ff00 : code = KeyEvent::eKEY_BLUE; break;
      case 0xe21dff00 : code = KeyEvent::eKEY_PLAY; break;
      case 0xe31cff00 : code = KeyEvent::eKEY_PAUSE; break;
      case 0xa35cff00 : code = KeyEvent::eKEY_STOP; break;
      case 0xe619ff00 : code = KeyEvent::eKEY_REWIND; break;
      case 0xa659ff00 : code = KeyEvent::eKEY_FASTFORWARD; break;
      case 0xa758ff00 : code = KeyEvent::eKEY_BACK; break;
      case 0xe718ff00 : code = KeyEvent::eKEY_FORWARD; break;
      case 0xab54ff00 : code = KeyEvent::eKEY_RECORD; break;
      case 0xf00fff00 : code = KeyEvent::eKEY_INFO; break;
      case 0xbf40ff00 : code = KeyEvent::eKEY_SUBTITLE; break;
      case 0xfe01ff00 : code = KeyEvent::eKEY_MUTE; break;
      case 0xff00ff00 : code = KeyEvent::eKEY_SLEEP; break;
      case 0xf906ff00 : code = KeyEvent::eKEY_LAST; break;
      case 0xb847ff00 : code = KeyEvent::eKEY_TEXT; break;
      case 0xec13ff00 : code = KeyEvent::eKEY_ENT; break;
      case 0xac53ff00 : code = KeyEvent::eKEY_BULLET; break;
      default : code = KeyEvent::KEY_UNKNOWN; break;
      }
   }
   else if (m_IRMode == NEXUS_IrInputMode_eRemoteA)
   {
      switch (rawCode)
      {
      case 0x0000d012 : code = KeyEvent::KEY_EXIT; break;
      case 0x00009034 : code = KeyEvent::KEY_UP; break;
      case 0x00008035 : code = KeyEvent::KEY_DOWN; break;
      case 0x00007036 : code = KeyEvent::KEY_LEFT; break;
      case 0x00006037 : code = KeyEvent::KEY_RIGHT; break;
      case 0x0000e011 : code = KeyEvent::KEY_OK; break;
      case 0x0000f001 : code = KeyEvent::KEY_1; break;
      case 0x0000e002 : code = KeyEvent::KEY_2; break;
      case 0x0000d003 : code = KeyEvent::KEY_3; break;
      case 0x0000c004 : code = KeyEvent::KEY_4; break;
      case 0x0000b005 : code = KeyEvent::KEY_5; break;
      case 0x0000a006 : code = KeyEvent::KEY_6; break;
      case 0x00009007 : code = KeyEvent::KEY_7; break;
      case 0x00008008 : code = KeyEvent::KEY_8; break;
      case 0x00007009 : code = KeyEvent::KEY_9; break;
      case 0x00000000 : code = KeyEvent::KEY_0; break;
      case 0x0000600a : code = KeyEvent::KEY_POWER; break;
      case 0x0000300d : code = KeyEvent::KEY_VOL_UP; break;
      case 0x0000200e : code = KeyEvent::KEY_VOL_DOWN; break;
      case 0x0000500b : code = KeyEvent::KEY_CH_UP; break;
      case 0x0000400c : code = KeyEvent::KEY_CH_DOWN; break;
      case 0x00006019 : code = KeyEvent::KEY_MENU; break;
      default : code = KeyEvent::KEY_UNKNOWN; break;
      }
   }
#endif

#endif
   return code;
}

#if defined IR_INPUT && defined SINGLE_PROCESS
static void irCallback(void *pParam, int iParam)
{
   size_t numEvents = 1;
   NEXUS_Error rc = 0;
   bool overflow;
   Platform *app = (Platform*)pParam;
   PlatformDataNexus *pdn = (PlatformDataNexus*)(app->GetPlatformData());
   NEXUS_IrInputHandle irHandle = pdn->m_IRHandle;
   BSTD_UNUSED(iParam);

   static Time lastSentEventTime = Time::Now();

   while (numEvents && !rc)
   {
      NEXUS_IrInputEvent irEvent;
      rc = NEXUS_IrInput_GetEvents(irHandle, &irEvent, 1, &numEvents, &overflow);

      if (numEvents)
      {
         Time now = Time::Now();
         if (!irEvent.repeat || (irEvent.code != RAW_EXIT && (now - lastSentEventTime).Milliseconds() > 150))
         {
            // If using internal callback funcitons
            if (!(app->GetKeyEventHandlerCallback() || app->GetKeyEventHandlerCallback2()))
            {
               app->PushKeyEvent(KeyEvent(pdn->MapKeyCode(irEvent.code), now));
               if (((PlatformDataNexus*)(app->GetPlatformData()))->m_event)
                  BKNI_SetEvent(((PlatformDataNexus*)(app->GetPlatformData()))->m_event);
            }
            else
            {
               if (app->GetKeyEventHandlerCallback())
                  app->GetKeyEventHandlerCallback()(pdn->MapKeyCode(irEvent.code));
               else
               {
                  app->GetKeyEventHandlerCallback2()(pdn->MapKeyCode(irEvent.code), KeyEvent::eKEY_STATE_DOWN);
                  app->GetKeyEventHandlerCallback2()(pdn->MapKeyCode(irEvent.code), KeyEvent::eKEY_STATE_UP);
               }
            }

            lastSentEventTime = now;
         }
      }
   }
}
#endif

#ifdef SINGLE_PROCESS
static void NexusMemMinimum(NEXUS_MemoryConfigurationSettings *pSettings)
{
   unsigned i, j;
   for (i = 0; i < NEXUS_MAX_DISPLAYS; i++)
   {
      for (j = 0; j < NEXUS_NUM_VIDEO_WINDOWS; j++)
      {
         pSettings->display[i].window[j].used = i < 1 && j < 1;
         if (pSettings->display[i].window[j].used)
         {
            pSettings->display[i].maxFormat = NEXUS_VideoFormat_e1080i;
            pSettings->display[i].window[j].convertAnyFrameRate = false;
            pSettings->display[i].window[j].precisionLipSync = false;
            pSettings->display[i].window[j].capture = false;
            pSettings->display[i].window[j].deinterlacer = NEXUS_DeinterlacerMode_eNone;
         }
      }
   }

#if NEXUS_NUM_STILL_DECODES
   for (i = 0; i < NEXUS_NUM_STILL_DECODES; i++)
   {
      pSettings->stillDecoder[i].used = false;
   }
#endif

#if NEXUS_HAS_VIDEO_DECODER
   for (i = 0; i < NEXUS_NUM_VIDEO_DECODERS; i++)
   {
      pSettings->videoDecoder[i].used = false;
   }
#endif

#if NEXUS_HAS_VIDEO_ENCODER
   for (i = 0; i < NEXUS_MAX_VIDEO_ENCODERS; i++)
   {
      pSettings->videoEncoder[i].used = false;
   }
#endif
}

static bool InitMaxMemConfig(NEXUS_PlatformSettings *platformSettings, NEXUS_MemoryConfigurationSettings *memConfigSettings)
{
   NEXUS_PlatformConfiguration configuration;
   NEXUS_HeapHandle            heap3D;
   int                         heap3DIndex = -1;
   int                         saved = 0;
   int                         orig3DSize = 0;
   int                         target3DSize = 430 * 1024 * 1024; /* If we have at least 430 MB, we're good to go */

   NEXUS_Error err = NEXUS_NOT_SUPPORTED;

   /* Bring Nexus up in default config ONLY because we can't find which is the 3D heap without it. */
   NEXUS_SetEnv("NEXUS_BASE_ONLY_INIT", "y");
   err = NEXUS_Platform_Init(platformSettings);
   if (err != NEXUS_SUCCESS)
   {
      printf("NEXUS_Platform_Init() failed\n");
      return false;
   }
   NEXUS_SetEnv("NEXUS_BASE_ONLY_INIT", NULL);

   NEXUS_Platform_GetConfiguration(&configuration);

   heap3D = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);

   if (configuration.heap[NEXUS_MEMC0_GRAPHICS_HEAP] == heap3D)
   {
      heap3DIndex = NEXUS_MEMC0_GRAPHICS_HEAP;
   }
#ifdef NEXUS_MEMC1_GRAPHICS_HEAP
   else if (configuration.heap[NEXUS_MEMC1_GRAPHICS_HEAP] == heap3D)
   {
      heap3DIndex = NEXUS_MEMC1_GRAPHICS_HEAP;
   }
#endif
   else
   {
      printf("Couldn't find 3D heap\n");
      NEXUS_Platform_Uninit();
      return false;
   }

   orig3DSize = platformSettings->heap[heap3DIndex].size;

   // If we resize the heaps or not, we will reinitialise the platform anyway
   NEXUS_Platform_Uninit();

   /* Do we have enough 3D memory already? */
   if (orig3DSize < target3DSize)
   {
      /* If we still don't have enough, reduce the main heap - leaving at least 64MB */
      if (orig3DSize + saved < target3DSize && platformSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size > 64 * 1024 * 1024)
      {
         int need = target3DSize - (orig3DSize + saved);
         int avail = platformSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size - 64 * 1024 * 1024;
         int newSize;

         if (avail < need)
            newSize = 64 * 1024 * 1024;
         else
            newSize = platformSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size - need;

         saved += platformSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size - newSize;
         platformSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = newSize;

         /* The only use is subIndex = 1 for MEMC0 memory above
            the 256MB register hole or 760MB CMA barrier. Always 0 for MEMC1 and 2.*/
         if (heap3DIndex == NEXUS_MEMC0_GRAPHICS_HEAP)
         {
            // Fit the graphics heap before the main heap
            platformSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex=1;
            platformSettings->heap[heap3DIndex].subIndex = 0;
         }
      }

      /* Add the savings onto the 3D heap */
      platformSettings->heap[heap3DIndex].size = orig3DSize + saved;

      /* Minimise video memory usage */
      NEXUS_GetDefaultMemoryConfigurationSettings(memConfigSettings);
      NexusMemMinimum(memConfigSettings);
   }

   return true;
}

static void InitializeNexusSingle(bool secure)
{
   NEXUS_Error                         err = NEXUS_NOT_SUPPORTED;
   NEXUS_PlatformSettings              platformSettings;
   NEXUS_MemoryConfigurationSettings   memConfigSettings;

   char *env = getenv("V3D_USE_MAX_GRAPHICS_MEM");
   bool maxMem = env != NULL && atoi(env) == 1;

   NEXUS_Platform_GetDefaultSettings(&platformSettings);
   platformSettings.openFrontend = false;

   NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);

   bool initWithMemConfig = false;
   if (maxMem || secure)
      initWithMemConfig = true;

#if NEXUS_HAS_SAGE
   uint32_t secure_graphics_heap;

   if (secure)
   {
      uint32_t secure_graphics_memc;

      memConfigSettings.videoDecoder[0].secure = NEXUS_SecureVideo_eBoth;
      for (int j = 0; j < NEXUS_NUM_VIDEO_WINDOWS; j++)
         memConfigSettings.display[0].window[j].secure = NEXUS_SecureVideo_eBoth;

#if defined(NEXUS_MEMC2_SECURE_GRAPHICS_HEAP)
      secure_graphics_heap = NEXUS_MEMC2_SECURE_GRAPHICS_HEAP;
      secure_graphics_memc = 2;
#elif defined(NEXUS_MEMC1_SECURE_GRAPHICS_HEAP)
      secure_graphics_heap = NEXUS_MEMC1_SECURE_GRAPHICS_HEAP;
      secure_graphics_memc = 1;
#elif defined(NEXUS_MEMC0_SECURE_GRAPHICS_HEAP)
      secure_graphics_heap = NEXUS_MEMC0_SECURE_GRAPHICS_HEAP;
      secure_graphics_memc = 0;
#elif defined(NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP)
      secure_graphics_heap = NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP;
      secure_graphics_memc = 0;
#endif
      platformSettings.heap[secure_graphics_heap].size = 64 * 1024 * 1024;
      platformSettings.heap[secure_graphics_heap].memcIndex = secure_graphics_memc;
      platformSettings.heap[secure_graphics_heap].heapType = NEXUS_HEAP_TYPE_SECURE_GRAPHICS;
      platformSettings.heap[secure_graphics_heap].memoryType =
         NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED |
         NEXUS_MEMORY_TYPE_MANAGED |
         NEXUS_MEMORY_TYPE_SECURE;
   }
   else
#endif
   if (maxMem)
   {
      /* Hackery to maximize the amount of device memory we get if the V3D_USE_MAX_GRAPHICS_MEM env var is set */
      if (!InitMaxMemConfig(&platformSettings, &memConfigSettings))
      {
         printf("Couldn't allocate requested max-memory\n");
         return;
      }
   }

   if (initWithMemConfig)
   {
      err = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
      if (err != NEXUS_SUCCESS)
      {
         printf("NEXUS_Platform_MemConfigInit() failed\n");
         return;
      }
   }
   else
   {
      err = NEXUS_Platform_Init(&platformSettings);
      if (err != NEXUS_SUCCESS)
      {
         printf("NEXUS_Platform_Init() failed\n");
         return;
      }
   }

#if NEXUS_HAS_SAGE
   if (secure)
   {
      NEXUS_PlatformConfiguration platform_config;
      NEXUS_MemoryStatus memory_status_0, memory_status_1;
      NEXUS_Platform_GetConfiguration(&platform_config);
      /* Ensure secure graphics heap is also GFD0 accessible. In a multiprocess system, clients blit to large offscreen secure graphics heap
         and NSC copies to GFD0/GFD1 secure graphics. For this example, we keep it simple. */
      err = NEXUS_Heap_GetStatus(NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE), &memory_status_0);
      BDBG_ASSERT(!err);
      err = NEXUS_Heap_GetStatus(NEXUS_Platform_GetFramebufferHeap(0), &memory_status_1);
      BDBG_ASSERT(!err);
      if (memory_status_0.memcIndex != memory_status_1.memcIndex)
      {
         printf("Application does not support multiple secure graphics heaps\n");
         return;
      }
      BDBG_ASSERT(platform_config.heap[secure_graphics_heap] == NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE));
   }
#endif

   NEXUS_Memory_PrintHeaps();
}
#endif

#ifndef SINGLE_PROCESS
extern "C" {
   extern const char *__progname;
}
#endif

void Platform::InitializePlatform()
{
   m_platform = new PlatformDataNexus(this);
   platform_data->InitUSBInputs();

   // Initialise the Nexus platform
   NEXUS_Error                   err = NEXUS_NOT_SUPPORTED;

#ifndef SINGLE_PROCESS
   /* NX_CLIENT */
   NxClient_JoinSettings joinSettings;
   NxClient_GetDefaultJoinSettings(&joinSettings);
   snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", __progname);
   joinSettings.mode = NEXUS_ClientMode_eVerified;
   err = NxClient_Join(&joinSettings);

   if (err != NEXUS_SUCCESS)
      BSG_THROW("NxClient_Join : Failed to join an existing server");

   platform_data->m_primaryProcess = false;
   platform_data->InitRoutedInputs();

#else
   // Not multi-process, so we control the display
   InitializeNexusSingle(m_options.GetSecure());

   platform_data->m_primaryProcess = true;

#ifdef IR_INPUT
   if (platform_data->m_primaryProcess)
   {
      NEXUS_IrInputSettings irSettings;
      NEXUS_IrInput_GetDefaultSettings(&irSettings);

      platform_data->m_IRMode = NEXUS_IrInputMode_eCirNec;

      char *rem = getenv("BcmRemoteType");
      if (rem && strcmp("RemoteA", rem) == 0)
         platform_data->m_IRMode = NEXUS_IrInputMode_eRemoteA;
      else if (rem && strcmp("CirNec", rem) == 0)
         platform_data->m_IRMode = NEXUS_IrInputMode_eCirNec;

      irSettings.mode = platform_data->m_IRMode;
      irSettings.dataReady.callback = irCallback;
      irSettings.dataReady.context = this;
      platform_data->m_IRHandle = NEXUS_IrInput_Open(0, &irSettings);
   }
#endif

#endif // SINGLE_PROCESS

   NEXUS_PlatformStatus status;
   err = NEXUS_Platform_GetStatus(&status);
   if (err)
      BSG_THROW("NEXUS_Platform_GetStatus() failed\n");

   std::stringstream platformName;
   platformName << std::hex << status.chipId;

   m_platformName = "NEXUS" + platformName.str();

   BKNI_CreateEvent(&platform_data->m_event);
}

static void sSetBandwidth(const ApplicationOptions &options)
{
   int memFd = 0;
   uint32_t* addr = NULL;

   memFd = open("/dev/mem", O_RDWR|O_SYNC);

   if (memFd < 0)
      BSG_THROW("Failed to open /dev/mem");

   addr = (uint32_t*)mmap(0, BCHP_REGISTER_END, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, BCHP_PHYSICAL_OFFSET);

   if (addr == NULL)
      BSG_THROW("Failed to mmap register space");

   /* set the bandwidth */
   if (options.GetBandwidth() > 0)
   {
      uint32_t regvalue;

      const unsigned int client_0 = (options.GetBandwidth() / 4) * 3;       /* 75% of the bandwidth to client 0 */
      const unsigned int client_1 = options.GetBandwidth() / 4;             /* 25% to client 1 */

      const unsigned int jword_size_client_0 = 8;
      const unsigned int jword_size_client_1 = 4;
      const double fudge_factor_client_0 = 1.15;                    /* as client 0 has some ~16% of small transactions we need to inflate the number */

      const double mb_client_0 = (double)client_0 * 1073741824.0;
      const double mb_client_1 = (double)client_1 * 1073741824.0;
      const double transfer_size_client_0 = jword_size_client_0 * 32;
      const double transfer_size_client_1 = jword_size_client_1 * 32;

      const double transfers_sec_client_0 = (mb_client_0 / transfer_size_client_0) * fudge_factor_client_0;
      const double transfers_sec_client_1 = mb_client_1 / transfer_size_client_1;

      const unsigned int lockout_client_0 = (unsigned int)((1000000000000.0 / transfers_sec_client_0) / (1000.0 / options.GetMemFrequency()));
      const unsigned int lockout_client_1 = (unsigned int)((1000000000000.0 / transfers_sec_client_1) / (1000.0 / options.GetMemFrequency()));

      regvalue = addr[BCHP_MEMC_ARB_0_CLIENT_INFO_88/4];
      regvalue = (regvalue & ~BCHP_MEMC_ARB_0_CLIENT_INFO_88_RR_EN_MASK) | (BCHP_MEMC_ARB_0_CLIENT_INFO_88_RR_EN_DISABLED << BCHP_MEMC_ARB_0_CLIENT_INFO_88_RR_EN_SHIFT);
      regvalue = (regvalue & ~BCHP_MEMC_ARB_0_CLIENT_INFO_88_PR_TAG_MASK) | (0x43 << BCHP_MEMC_ARB_0_CLIENT_INFO_88_PR_TAG_SHIFT);
      regvalue = (regvalue & ~BCHP_MEMC_ARB_0_CLIENT_INFO_88_BO_VAL_MASK) | (lockout_client_0 << BCHP_MEMC_ARB_0_CLIENT_INFO_88_BO_VAL_SHIFT);
      addr[BCHP_MEMC_ARB_0_CLIENT_INFO_88/4] = regvalue;

      regvalue = addr[BCHP_MEMC_ARB_0_CLIENT_INFO_89/4];
      regvalue = (regvalue & ~BCHP_MEMC_ARB_0_CLIENT_INFO_89_RR_EN_MASK) | (BCHP_MEMC_ARB_0_CLIENT_INFO_89_RR_EN_DISABLED << BCHP_MEMC_ARB_0_CLIENT_INFO_89_RR_EN_SHIFT);
      regvalue = (regvalue & ~BCHP_MEMC_ARB_0_CLIENT_INFO_89_PR_TAG_MASK) | (0x44 << BCHP_MEMC_ARB_0_CLIENT_INFO_89_PR_TAG_SHIFT);
      regvalue = (regvalue & ~BCHP_MEMC_ARB_0_CLIENT_INFO_89_BO_VAL_MASK) | (lockout_client_1 << BCHP_MEMC_ARB_0_CLIENT_INFO_89_BO_VAL_SHIFT);
      addr[BCHP_MEMC_ARB_0_CLIENT_INFO_89/4] = regvalue;
   }
   else
   {
      /* set to default */
      /*      addr[BCHP_MEMC_ARB_0_CLIENT_INFO_88/4] = 0x87fff03c; */
      /*      addr[BCHP_MEMC_ARB_0_CLIENT_INFO_89/4] = 0x87fff03d; */
   }

   munmap(addr, BCHP_REGISTER_END);
   close(memFd);
}

#ifdef SINGLE_PROCESS
/* If format == 0, w & h are used to determine requested video format */
static NEXUS_DisplayHandle OpenNexusDisplay(NEXUS_VideoFormat format, uint32_t w, uint32_t h, bool secure)
{
   NEXUS_DisplayHandle     display = NULL;
   NEXUS_DisplaySettings   display_settings;
   NEXUS_GraphicsSettings  graphics_settings;

   /* Bring up display */
   NEXUS_Display_GetDefaultSettings(&display_settings);

   if (format != 0)
   {
      display_settings.format = format;
   }
   else
   {
      if (w <= 720 && h <= 480)
         display_settings.format = NEXUS_VideoFormat_eNtsc;
      else if (w <= 1280 && h <= 720)
         display_settings.format = NEXUS_VideoFormat_e720p;
      else if (w <= 1920 && h <= 1080)
         display_settings.format = NEXUS_VideoFormat_e1080p;
      else
         display_settings.format = NEXUS_VideoFormat_e3840x2160p24hz;
   }
   display_settings.displayType = NEXUS_DisplayType_eAuto;

   display = NEXUS_Display_Open(0, &display_settings);
   if (!display)
      printf("NEXUS_Display_Open() failed\n");

   NEXUS_Display_GetGraphicsSettings(display, &graphics_settings);
   graphics_settings.horizontalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;
   graphics_settings.verticalFilter = NEXUS_GraphicsFilterCoeffs_eBilinear;

   /* Disable blend with video plane */
   graphics_settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eOne;
   graphics_settings.destBlendFactor = NEXUS_CompositorBlendFactor_eZero;

   graphics_settings.secure = secure;

   NEXUS_Display_SetGraphicsSettings(display, &graphics_settings);

   return display;
}

void PlatformDataNexus::InitPanelOutput()
{
#if NEXUS_NUM_PANEL_OUTPUTS

   NEXUS_PlatformConfiguration   platform_config;

   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.panel[0])
   {
      char * b552 = getenv("panel_type");
      if (b552 && strcmp("B552", b552) == 0)
      {
         NEXUS_PanelOutputSettings     panelOutputSettings;
         NEXUS_PanelOutput_GetSettings(platform_config.outputs.panel[0], &panelOutputSettings);
         panelOutputSettings.frameRateMultiplier = 1;
         NEXUS_PanelOutput_SetSettings(platform_config.outputs.panel[0], &panelOutputSettings);
      }

      NEXUS_Display_AddOutput(m_nexusDisplay, NEXUS_PanelOutput_GetConnector(platform_config.outputs.panel[0]));

      NEXUS_BoardCfg_ConfigurePanel(true, true, true);
   }
#endif
}

void PlatformDataNexus::InitComponentOutput()
{
   m_componentOn = false;

#if NEXUS_NUM_COMPONENT_OUTPUTS

   NEXUS_PlatformConfiguration   platform_config;
   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.component[0])
   {
      NEXUS_Display_AddOutput(m_nexusDisplay, NEXUS_ComponentOutput_GetConnector(platform_config.outputs.component[0]));
      m_componentOn = true;
   }
#endif
}

void PlatformDataNexus::TermComponentOutput()
{
#if NEXUS_NUM_COMPONENT_OUTPUTS

   if (m_componentOn)
   {
      NEXUS_PlatformConfiguration   platform_config;
      NEXUS_Platform_GetConfiguration(&platform_config);

      if (platform_config.outputs.component[0])
         NEXUS_Display_RemoveOutput(m_nexusDisplay, NEXUS_ComponentOutput_GetConnector(platform_config.outputs.component[0]));
   }
#endif
}

void PlatformDataNexus::InitCompositeOutput(uint32_t w, uint32_t h)
{
   m_compositeOn = false;

#if NEXUS_NUM_COMPOSITE_OUTPUTS

   NEXUS_PlatformConfiguration   platform_config;

   if (w <= 720 && h <=480)
   {
      NEXUS_Platform_GetConfiguration(&platform_config);

      if (platform_config.outputs.composite[0])
      {
         NEXUS_Display_AddOutput(m_nexusDisplay, NEXUS_CompositeOutput_GetConnector(platform_config.outputs.composite[0]));
         m_compositeOn = true;
      }
   }
#else
   BSTD_UNUSED(w);
   BSTD_UNUSED(h);
#endif
}

void PlatformDataNexus::TermCompositeOutput()
{
#if NEXUS_NUM_COMPOSITE_OUTPUTS

   if (m_compositeOn)
   {
      NEXUS_PlatformConfiguration   platform_config;
      NEXUS_Platform_GetConfiguration(&platform_config);

      if (platform_config.outputs.composite[0])
         NEXUS_Display_RemoveOutput(m_nexusDisplay, NEXUS_CompositeOutput_GetConnector(platform_config.outputs.composite[0]));
   }

#endif
}

#if NEXUS_NUM_HDMI_OUTPUTS
static void hotplug_callback(void *pParam, int iParam)
{
   NEXUS_HdmiOutputStatus status;
   DisplaySessionHandles *displayHandles = (DisplaySessionHandles*)pParam;
   NEXUS_HdmiOutputHandle hdmi = displayHandles->hdmi;
   NEXUS_DisplayHandle    display = displayHandles->display;

   (void)iParam;

   NEXUS_HdmiOutput_GetStatus(hdmi, &status);
   printf("HDMI hotplug event: %s\n", status.connected?"connected":"not connected");

   /* the app can choose to switch to the preferred format, but it's not required. */
   if (status.connected)
   {
      NEXUS_DisplaySettings displaySettings;
      NEXUS_Display_GetSettings(display, &displaySettings);
      if (!status.videoFormatSupported[displaySettings.format])
      {
         fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
         displaySettings.format = status.preferredVideoFormat;
         NEXUS_Display_SetSettings(display, &displaySettings);
      }
   }
}
#endif

void PlatformDataNexus::InitHDMIOutput(bool forceHDMI)
{
   m_hdmiOn = false;

#if NEXUS_NUM_HDMI_OUTPUTS
   NEXUS_HdmiOutputSettings      hdmiSettings;
   NEXUS_PlatformConfiguration   platform_config;
   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.hdmi[0])
   {
      NEXUS_Display_AddOutput(m_nexusDisplay, NEXUS_HdmiOutput_GetVideoConnector(platform_config.outputs.hdmi[0]));

      if (!forceHDMI)
      {
         /* Install hotplug callback -- video only for now */
         m_sessionHandles = new DisplaySessionHandles;
         m_sessionHandles->hdmi = platform_config.outputs.hdmi[0];
         m_sessionHandles->display = m_nexusDisplay;

         NEXUS_HdmiOutput_GetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);
         hdmiSettings.hotplugCallback.callback = hotplug_callback;
         hdmiSettings.hotplugCallback.context  = m_sessionHandles;
         NEXUS_HdmiOutput_SetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);

         /* Force a hotplug to switch to a supported format if necessary */
         hotplug_callback(m_sessionHandles, 0);

         m_hdmiOn = true;
      }
   }
#else
   BSTD_UNUSED(forceHDMI);
#endif
}

void PlatformDataNexus::TermHDMIOutput()
{
#if NEXUS_NUM_HDMI_OUTPUTS
   NEXUS_HdmiOutputSettings      hdmiSettings;
   NEXUS_PlatformConfiguration   platform_config;
   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.hdmi[0])
   {
      /* Install hotplug callback -- video only for now */
      NEXUS_HdmiOutput_GetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);
      hdmiSettings.hotplugCallback.callback = NULL;
      hdmiSettings.hotplugCallback.context = NULL;
      hdmiSettings.hotplugCallback.param = 0;
      NEXUS_HdmiOutput_SetSettings(platform_config.outputs.hdmi[0], &hdmiSettings);

      NEXUS_Display_RemoveOutput(m_nexusDisplay, NEXUS_HdmiOutput_GetVideoConnector(platform_config.outputs.hdmi[0]));
   }
#endif
}

static NEXUS_VideoFormat SelectDisplayFormat(uint8_t hz, bool interlaced, uint32_t w, uint32_t h)
{
   int preferred = 0;
   NEXUS_AspectRatio monitorRatio = NEXUS_AspectRatio_e4x3;

#if NEXUS_NUM_HDMI_OUTPUTS
   NEXUS_PlatformConfiguration   platform_config;
   NEXUS_Platform_GetConfiguration(&platform_config);

   if (platform_config.outputs.hdmi[0])
   {
      NEXUS_HdmiOutputStatus  hdmiOutputStatus;

      NEXUS_Error rc = NEXUS_HdmiOutput_GetStatus(platform_config.outputs.hdmi[0], &hdmiOutputStatus);

      if (!rc && hdmiOutputStatus.connected)
      {
         for (int i = 0; i < NEXUS_VideoFormat_eMax; i++)
         {
            if (hdmiOutputStatus.videoFormatSupported[i])
            {
               NEXUS_VideoFormatInfo videoInfo;
               NEXUS_VideoFormat_GetInfo((NEXUS_VideoFormat)i, &videoInfo);
               if ((videoInfo.verticalFreq / 10 == hz) && (videoInfo.width == w) && (videoInfo.height == h))
               {
                  preferred = i;
                  monitorRatio = videoInfo.aspectRatio;
                  break;
               }
            }
         }

         // no exact match found, just look at width and height
         if (preferred == NEXUS_VideoFormat_eUnknown)
         {
            for (int i = 0; i < NEXUS_VideoFormat_eMax; i++)
            {
               if (hdmiOutputStatus.videoFormatSupported[i])
               {
                  NEXUS_VideoFormatInfo videoInfo;
                  NEXUS_VideoFormat_GetInfo((NEXUS_VideoFormat)i, &videoInfo);
                  if ((videoInfo.width == w) && (videoInfo.height == h))
                  {
                     preferred = i;
                     monitorRatio = videoInfo.aspectRatio;
                     break;
                  }
               }
            }
         }
      }
   }
#endif

   if (monitorRatio != NEXUS_AspectRatio_e16x9)
   {
      if (w <= 720 && h <= 480)
         preferred = NEXUS_VideoFormat_eNtsc;
      else if (w <= 720 && h <= 576)
         preferred = NEXUS_VideoFormat_ePal;
   }

   return (NEXUS_VideoFormat)preferred;
}
#endif // SINGLE_PROCESS

EGLNativeWindowType Platform::NewNativeWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
   NXPL_NativeWindowInfoEXT winInfo;
   NXPL_GetDefaultNativeWindowInfoEXT(&winInfo);

   if (w == 0)
      w = m_options.GetWidth();
   if (h == 0)
      h = m_options.GetHeight();

#ifdef SINGLE_PROCESS

   /* Init the window structure */
   winInfo.x = x;
   winInfo.y = y;
   winInfo.width = w;
   winInfo.height = h;

#else

   /* Init the window structure */
   winInfo.x = x;
   winInfo.y = y;
   winInfo.width = w;
   winInfo.height = h;
   winInfo.clientID = (uint32_t)m_options.GetClientID();

#endif // SINGLE_PROCESS

   if (m_options.GetStretch())
      winInfo.stretch = 1;
   else
      winInfo.stretch = 0;

   m_nativeWindows.push_back(NXPL_CreateNativeWindowEXT(&winInfo));

#ifndef SINGLE_PROCESS
   uint32_t clientID = NXPL_GetClientID(m_nativeWindows.front());
   if (clientID != 0)
      winInfo.clientID = clientID;
#endif

   platform_data->m_winInfo.push_back(winInfo);

   return m_nativeWindows.back();
}

void Platform::InitializePlatformDisplay()
{
   if (m_options.GetBandwidth() > 0)
      sSetBandwidth(m_options);

   /* bring up display */
   if (m_options.GetHeadless())
      NXPL_RegisterNexusDisplayPlatform(&platform_data->m_platformHandle, NULL);
   else
   {
#ifdef SINGLE_PROCESS
      uint32_t displayWidth  = m_options.GetWidth();
      uint32_t displayHeight = m_options.GetHeight();

      NEXUS_VideoFormat format = SelectDisplayFormat(m_options.GetDisplayRefreshRate(), m_options.GetDisplayInterlace(),
                                                     displayWidth, displayHeight);

      platform_data->m_nexusDisplay = OpenNexusDisplay(format, displayWidth, displayHeight, m_options.GetSecure());

      platform_data->InitPanelOutput();
      platform_data->InitCompositeOutput(displayWidth, displayHeight);
      platform_data->InitComponentOutput();
      platform_data->InitHDMIOutput(m_options.GetForceHDMI());

      NXPL_RegisterNexusDisplayPlatform(&platform_data->m_platformHandle, platform_data->m_nexusDisplay);
#else
      NXPL_RegisterNexusDisplayPlatform(&platform_data->m_platformHandle, NULL);
#endif // SINGLE_PROCESS
   }

   assert(m_nativeWindows.size() == 0);

   NewNativeWindow(m_options.GetOffsetX(), m_options.GetOffsetY(), m_options.GetWidth(), m_options.GetHeight());
}

void Platform::ResizeNativeWindow(uint32_t w, uint32_t h)
{
   // In theory this needs to resize m_nativeWindows to the size given.
   // TODO
   BSTD_UNUSED(w);
   BSTD_UNUSED(h);
}

void Platform::GoIdle(int32_t ms)
{
   platform_data->m_idling = true;
   platform_data->m_timeout = ms;
}

void Platform::RunPlatformMainLoop()
{
   /* Main message loop */
   while (m_exitCode == 0xF0F0F0F0)
   {
      if (platform_data->m_idling)
      {
         // Wait for an event (or timeout)
         //printf("Idle wait\n");
         BKNI_WaitForEvent(platform_data->m_event, platform_data->m_timeout);
         //printf("Event\n");
         platform_data->m_idling = false;
         platform_data->m_timeout = 0;
      }

      // We don't have a window resize event in Nexus, so we just have to check the window size each frame
      if (m_nativeWindows.front())
      {
         EGLSurface surf = m_context.GetSurface();
         if (surf != EGL_NO_SURFACE)
         {
            EGLint w, h;
            eglQuerySurface(m_context.GetDisplay(), m_context.GetSurface(), EGL_WIDTH, &w);
            eglQuerySurface(m_context.GetDisplay(), m_context.GetSurface(), EGL_HEIGHT, &h);

            if (m_windowWidth != (uint32_t)w || m_windowHeight != (uint32_t)h)
               Resize((uint32_t)w, (uint32_t)h);
         }
      }

      UpdateFrameTimestamp();

      if (m_keyEvents.Pending())
         m_app->KeyEventHandler(m_keyEvents);

      // If merging mouse moves, we might need to generate one
      if (m_mouseMoveMerging && m_relMousePos != IVec3())
      {
         // Add in a single mouse move
         PushMouseEvent(MouseEvent(m_relMousePos, m_absMousePos, Time::Now()));
      }

      if (m_mouseEvents.Pending())
         m_app->MouseEventHandler(m_mouseEvents);

      // Dispatch the frame render
      RenderFrameSequence();
   }
}

void Platform::TerminatePlatform()
{
   printf("BSG TerminatePlatform\n");

   BKNI_DestroyEvent(platform_data->m_event);

   platform_data->TermRoutedInputs();

#ifdef SINGLE_PROCESS

#ifdef IR_INPUT
   if (platform_data->m_primaryProcess)
      NEXUS_IrInput_Close(platform_data->m_IRHandle);
#endif

   NEXUS_Platform_Uninit();
#else
   NxClient_Uninit();
#endif

   delete m_platform;
   m_platform = NULL;
}

void Platform::TerminatePlatformDisplay()
{
   std::list<EGLNativeWindowType>::iterator iter;
   for (iter = m_nativeWindows.begin(); iter != m_nativeWindows.end(); ++iter)
      NXPL_DestroyNativeWindow(*iter);

   NXPL_UnregisterNexusDisplayPlatform(platform_data->m_platformHandle);

#ifdef SINGLE_PROCESS
   if (platform_data->m_nexusDisplay)
   {
      platform_data->TermComponentOutput();
      platform_data->TermCompositeOutput();
      platform_data->TermHDMIOutput();

      NEXUS_Display_Close(platform_data->m_nexusDisplay);
   }

   platform_data->m_nexusDisplay = 0;
#endif
}

void Platform::SetStereoscopic(bool on)
{
   static bool sentOne = false;

   if (on && (!sentOne || !m_stereo))
   {
#ifdef SINGLE_PROCESS
      NEXUS_PlatformConfiguration platformConfig;
      NEXUS_Platform_GetConfiguration(&platformConfig);

      NEXUS_HdmiOutputHandle hdmiOutput = platformConfig.outputs.hdmi[0];

      NEXUS_HdmiOutputVendorSpecificInfoFrame vsi;
      NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(hdmiOutput, &vsi);
      vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
      vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eSidexSideHalf;
      NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(hdmiOutput, &vsi);
#endif
      NXPL_SetDisplayType(platform_data->m_platformHandle, NXPL_3D_LEFT_RIGHT);

      sentOne = true;
   }
   else if (!on && (!sentOne || m_stereo))
   {
#ifdef SINGLE_PROCESS
      NEXUS_PlatformConfiguration platformConfig;
      NEXUS_Platform_GetConfiguration(&platformConfig);

      NEXUS_HdmiOutputHandle hdmiOutput = platformConfig.outputs.hdmi[0];

      NEXUS_HdmiOutputVendorSpecificInfoFrame vsi;
      NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(hdmiOutput, &vsi);
      vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone;
      NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(hdmiOutput, &vsi);
#endif
      NXPL_SetDisplayType(platform_data->m_platformHandle, NXPL_2D);

      sentOne = true;
   }

   m_stereo = on;
}

float Platform::EstimateCPUPercentage()
{
   struct tms cput;
   clock_t t = times(&cput);
   float pct;

   pct = 100.0f * ((float)(cput.tms_utime + cput.tms_stime + cput.tms_cutime + cput.tms_cstime) -
         (platform_data->m_lastCPUUsage.tms_utime + platform_data->m_lastCPUUsage.tms_stime +
          platform_data->m_lastCPUUsage.tms_cutime + platform_data->m_lastCPUUsage.tms_cstime)) /
          (float)(t - platform_data->m_lastCPUTicks);

   platform_data->m_lastCPUUsage = cput;
   platform_data->m_lastCPUTicks = t;

   return pct;
}

void Platform::BlitPixmap(NativePixmap *pixmap)
{
#ifdef SINGLE_PROCESS
   NEXUS_GraphicsSettings graphicsSettings;

   NEXUS_Display_GetGraphicsSettings(platform_data->m_nexusDisplay, &graphicsSettings);
   graphicsSettings.enabled = true;
   graphicsSettings.position.x = 0;
   graphicsSettings.position.y = 0;
   graphicsSettings.position.width = pixmap->GetWidth();
   graphicsSettings.position.height = pixmap->GetHeight();
   graphicsSettings.clip.width = pixmap->GetWidth();
   graphicsSettings.clip.height = pixmap->GetHeight();

   NEXUS_Display_SetGraphicsSettings(platform_data->m_nexusDisplay, &graphicsSettings);

   NEXUS_Display_SetGraphicsFramebuffer(platform_data->m_nexusDisplay, ((NexusPixmapData*)pixmap->GetNativePixmapData())->m_surface);
#else
   // TODO
   BSTD_UNUSED(pixmap);
#endif
}

void Platform::ConfigureVideoGraphicsBlending(eVideoGraphicsBlend type, float graphicsPlaneAlpha, uint32_t zOrder)
{
#ifdef SINGLE_PROCESS
   NEXUS_GraphicsSettings  graphics_settings;

   NEXUS_Display_GetGraphicsSettings(platform_data->m_nexusDisplay, &graphics_settings);

   if (type == eUSE_CONSTANT_ALPHA)
   {
      if (graphicsPlaneAlpha == 1.0f)
      {
         graphics_settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eOne;
         graphics_settings.destBlendFactor   = NEXUS_CompositorBlendFactor_eZero;
      }
      else if (graphicsPlaneAlpha == 0.0f)
      {
         graphics_settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eZero;
         graphics_settings.destBlendFactor   = NEXUS_CompositorBlendFactor_eOne;
      }
      else
      {
         graphics_settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eConstantAlpha;
         graphics_settings.destBlendFactor   = NEXUS_CompositorBlendFactor_eInverseConstantAlpha;
         graphics_settings.constantAlpha = (uint8_t)(graphicsPlaneAlpha * 255.0f);
      }
   }
   else
   {
      graphics_settings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eSourceAlpha;
      graphics_settings.destBlendFactor   = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
   }

   if (zOrder != ~0u)
      graphics_settings.zorder = zOrder;

   NEXUS_Display_SetGraphicsSettings(platform_data->m_nexusDisplay, &graphics_settings);
#else
   // Only uses the primary native window
   NXPL_NativeWindowInfoEXT &winInfo = platform_data->m_winInfo.front();
   uint32_t                 clientID = winInfo.clientID;

   if (clientID)
   {
      NEXUS_SurfaceComposition comp;
      NxClient_GetSurfaceClientComposition(clientID, &comp);

      if (type == eUSE_CONSTANT_ALPHA)
      {
         if (graphicsPlaneAlpha == 1.0f)
            comp.alphaBlend.a = NEXUS_BlendFactor_eOne;
         else if (graphicsPlaneAlpha == 0.0f)
            comp.alphaBlend.a = NEXUS_BlendFactor_eZero;
         else
         {
            comp.alphaBlend.a = NEXUS_BlendFactor_eConstantAlpha;
            comp.constantColor = (uint8_t)(graphicsPlaneAlpha * 255.0f) << 24;
         }

      }
      else
      {
         comp.alphaBlend.a = NEXUS_BlendFactor_eSourceAlpha;
      }

      comp.alphaBlend.b = NEXUS_BlendFactor_eOne;
      comp.alphaBlend.c = NEXUS_BlendFactor_eZero;
      comp.alphaBlend.d = NEXUS_BlendFactor_eZero;
      comp.alphaBlend.e = NEXUS_BlendFactor_eZero;

      if (zOrder != ~0u)
         comp.zorder = zOrder;

      NxClient_SetSurfaceClientComposition(clientID, &comp);
   }
#endif
}

// USB mouse and keyboard input
#define __EXPORTED_HEADERS__
#include <linux/input.h>
#undef __EXPORTED_HEADERS__

static KeyEvent::eKeyState stateMap[] =
{
   KeyEvent::eKEY_STATE_UP,
   KeyEvent::eKEY_STATE_DOWN,
   KeyEvent::eKEY_STATE_REPEAT
};

static MouseEvent::eMouseButtonState btnStateMap[] =
{
   MouseEvent::eMOUSE_STATE_UP,
   MouseEvent::eMOUSE_STATE_DOWN
};

static KeyEvent::eKeyCode keyMap[KEY_CNT];

static void InitKeyMap()
{
   keyMap[KEY_RESERVED] = KeyEvent::eKEY_RESERVED;
   keyMap[KEY_ESC] = KeyEvent::eKEY_ESC;
   keyMap[KEY_1] = KeyEvent::eKEY_1;
   keyMap[KEY_2] = KeyEvent::eKEY_2;
   keyMap[KEY_3] = KeyEvent::eKEY_3;
   keyMap[KEY_4] = KeyEvent::eKEY_4;
   keyMap[KEY_5] = KeyEvent::eKEY_5;
   keyMap[KEY_6] = KeyEvent::eKEY_6;
   keyMap[KEY_7] = KeyEvent::eKEY_7;
   keyMap[KEY_8] = KeyEvent::eKEY_8;
   keyMap[KEY_9] = KeyEvent::eKEY_9;
   keyMap[KEY_0] = KeyEvent::eKEY_0;
   keyMap[KEY_MINUS] = KeyEvent::eKEY_MINUS;
   keyMap[KEY_EQUAL] = KeyEvent::eKEY_EQUAL;
   keyMap[KEY_BACKSPACE] = KeyEvent::eKEY_BACKSPACE;
   keyMap[KEY_TAB] = KeyEvent::eKEY_TAB;
   keyMap[KEY_Q] = KeyEvent::eKEY_Q;
   keyMap[KEY_W] = KeyEvent::eKEY_W;
   keyMap[KEY_E] = KeyEvent::eKEY_E;
   keyMap[KEY_R] = KeyEvent::eKEY_R;
   keyMap[KEY_T] = KeyEvent::eKEY_T;
   keyMap[KEY_Y] = KeyEvent::eKEY_Y;
   keyMap[KEY_U] = KeyEvent::eKEY_U;
   keyMap[KEY_I] = KeyEvent::eKEY_I;
   keyMap[KEY_O] = KeyEvent::eKEY_O;
   keyMap[KEY_P] = KeyEvent::eKEY_P;
   keyMap[KEY_LEFTBRACE] = KeyEvent::eKEY_LEFTBRACE;
   keyMap[KEY_RIGHTBRACE] = KeyEvent::eKEY_RIGHTBRACE;
   keyMap[KEY_ENTER] = KeyEvent::eKEY_ENTER;
   keyMap[KEY_LEFTCTRL] = KeyEvent::eKEY_LEFTCTRL;
   keyMap[KEY_A] = KeyEvent::eKEY_A;
   keyMap[KEY_S] = KeyEvent::eKEY_S;
   keyMap[KEY_D] = KeyEvent::eKEY_D;
   keyMap[KEY_F] = KeyEvent::eKEY_F;
   keyMap[KEY_G] = KeyEvent::eKEY_G;
   keyMap[KEY_H] = KeyEvent::eKEY_H;
   keyMap[KEY_J] = KeyEvent::eKEY_J;
   keyMap[KEY_K] = KeyEvent::eKEY_K;
   keyMap[KEY_L] = KeyEvent::eKEY_L;
   keyMap[KEY_SEMICOLON] = KeyEvent::eKEY_SEMICOLON;
   keyMap[KEY_APOSTROPHE] = KeyEvent::eKEY_APOSTROPHE;
   keyMap[KEY_GRAVE] = KeyEvent::eKEY_GRAVE;
   keyMap[KEY_LEFTSHIFT] = KeyEvent::eKEY_LEFTSHIFT;
   keyMap[KEY_BACKSLASH] = KeyEvent::eKEY_BACKSLASH;
   keyMap[KEY_Z] = KeyEvent::eKEY_Z;
   keyMap[KEY_X] = KeyEvent::eKEY_X;
   keyMap[KEY_C] = KeyEvent::eKEY_C;
   keyMap[KEY_V] = KeyEvent::eKEY_V;
   keyMap[KEY_B] = KeyEvent::eKEY_B;
   keyMap[KEY_N] = KeyEvent::eKEY_N;
   keyMap[KEY_M] = KeyEvent::eKEY_M;
   keyMap[KEY_COMMA] = KeyEvent::eKEY_COMMA;
   keyMap[KEY_DOT] = KeyEvent::eKEY_DOT;
   keyMap[KEY_SLASH] = KeyEvent::eKEY_SLASH;
   keyMap[KEY_RIGHTSHIFT] = KeyEvent::eKEY_RIGHTSHIFT;
   keyMap[KEY_KPASTERISK] = KeyEvent::eKEY_KPASTERISK;
   keyMap[KEY_LEFTALT] = KeyEvent::eKEY_LEFTALT;
   keyMap[KEY_SPACE] = KeyEvent::eKEY_SPACE;
   keyMap[KEY_CAPSLOCK] = KeyEvent::eKEY_CAPSLOCK;
   keyMap[KEY_F1] = KeyEvent::eKEY_F1;
   keyMap[KEY_F2] = KeyEvent::eKEY_F2;
   keyMap[KEY_F3] = KeyEvent::eKEY_F3;
   keyMap[KEY_F4] = KeyEvent::eKEY_F4;
   keyMap[KEY_F5] = KeyEvent::eKEY_F5;
   keyMap[KEY_F6] = KeyEvent::eKEY_F6;
   keyMap[KEY_F7] = KeyEvent::eKEY_F7;
   keyMap[KEY_F8] = KeyEvent::eKEY_F8;
   keyMap[KEY_F9] = KeyEvent::eKEY_F9;
   keyMap[KEY_F10] = KeyEvent::eKEY_F10;
   keyMap[KEY_NUMLOCK] = KeyEvent::eKEY_NUMLOCK;
   keyMap[KEY_SCROLLLOCK] = KeyEvent::eKEY_SCROLLLOCK;
   keyMap[KEY_KP7] = KeyEvent::eKEY_KP7;
   keyMap[KEY_KP8] = KeyEvent::eKEY_KP8;
   keyMap[KEY_KP9] = KeyEvent::eKEY_KP9;
   keyMap[KEY_KPMINUS] = KeyEvent::eKEY_KPMINUS;
   keyMap[KEY_KP4] = KeyEvent::eKEY_KP4;
   keyMap[KEY_KP5] = KeyEvent::eKEY_KP5;
   keyMap[KEY_KP6] = KeyEvent::eKEY_KP6;
   keyMap[KEY_KPPLUS] = KeyEvent::eKEY_KPPLUS;
   keyMap[KEY_KP1] = KeyEvent::eKEY_KP1;
   keyMap[KEY_KP2] = KeyEvent::eKEY_KP2;
   keyMap[KEY_KP3] = KeyEvent::eKEY_KP3;
   keyMap[KEY_KP0] = KeyEvent::eKEY_KP0;
   keyMap[KEY_KPDOT] = KeyEvent::eKEY_KPDOT;
   keyMap[KEY_ZENKAKUHANKAKU] = KeyEvent::eKEY_ZENKAKUHANKAKU;
   keyMap[KEY_F11] = KeyEvent::eKEY_F11;
   keyMap[KEY_F12] = KeyEvent::eKEY_F12;
   keyMap[KEY_RO] = KeyEvent::eKEY_RO;
   keyMap[KEY_KATAKANA] = KeyEvent::eKEY_KATAKANA;
   keyMap[KEY_HIRAGANA] = KeyEvent::eKEY_HIRAGANA;
   keyMap[KEY_HENKAN] = KeyEvent::eKEY_HENKAN;
   keyMap[KEY_KATAKANAHIRAGANA] = KeyEvent::eKEY_KATAKANAHIRAGANA;
   keyMap[KEY_KPJPCOMMA] = KeyEvent::eKEY_KPJPCOMMA;
   keyMap[KEY_KPENTER] = KeyEvent::eKEY_KPENTER;
   keyMap[KEY_RIGHTCTRL] = KeyEvent::eKEY_RIGHTCTRL;
   keyMap[KEY_KPSLASH] = KeyEvent::eKEY_KPSLASH;
   keyMap[KEY_SYSRQ] = KeyEvent::eKEY_SYSRQ;
   keyMap[KEY_RIGHTALT] = KeyEvent::eKEY_RIGHTALT;
   keyMap[KEY_LINEFEED] = KeyEvent::eKEY_LINEFEED;
   keyMap[KEY_HOME] = KeyEvent::eKEY_HOME;
   keyMap[KEY_UP] = KeyEvent::eKEY_UP;
   keyMap[KEY_PAGEUP] = KeyEvent::eKEY_PAGEUP;
   keyMap[KEY_LEFT] = KeyEvent::eKEY_LEFT;
   keyMap[KEY_RIGHT] = KeyEvent::eKEY_RIGHT;
   keyMap[KEY_END] = KeyEvent::eKEY_END;
   keyMap[KEY_DOWN] = KeyEvent::eKEY_DOWN;
   keyMap[KEY_PAGEDOWN] = KeyEvent::eKEY_PAGEDOWN;
   keyMap[KEY_INSERT] = KeyEvent::eKEY_INSERT;
   keyMap[KEY_DELETE] = KeyEvent::eKEY_DELETE;
   keyMap[KEY_MACRO] = KeyEvent::eKEY_MACRO;
   keyMap[KEY_MUTE] = KeyEvent::eKEY_MUTE;
   keyMap[KEY_VOLUMEDOWN] = KeyEvent::eKEY_VOLUMEDOWN;
   keyMap[KEY_VOLUMEUP] = KeyEvent::eKEY_VOLUMEUP;
   keyMap[KEY_POWER] = KeyEvent::eKEY_POWER;
   keyMap[KEY_KPEQUAL] = KeyEvent::eKEY_KPEQUAL;
   keyMap[KEY_KPPLUSMINUS] = KeyEvent::eKEY_KPPLUSMINUS;
   keyMap[KEY_PAUSE] = KeyEvent::eKEY_PAUSE;
   keyMap[KEY_KPCOMMA] = KeyEvent::eKEY_KPCOMMA;
   keyMap[KEY_HANGUEL] = KeyEvent::eKEY_HANGUEL;
   keyMap[KEY_HANJA] = KeyEvent::eKEY_HANJA;
   keyMap[KEY_YEN] = KeyEvent::eKEY_YEN;
   keyMap[KEY_LEFTMETA] = KeyEvent::eKEY_LEFTMETA;
   keyMap[KEY_RIGHTMETA] = KeyEvent::eKEY_RIGHTMETA;
   keyMap[KEY_COMPOSE] = KeyEvent::eKEY_COMPOSE;
   keyMap[KEY_STOP] = KeyEvent::eKEY_STOP;
   keyMap[KEY_AGAIN] = KeyEvent::eKEY_AGAIN;
   keyMap[KEY_PROPS] = KeyEvent::eKEY_PROPS;
   keyMap[KEY_UNDO] = KeyEvent::eKEY_UNDO;
   keyMap[KEY_FRONT] = KeyEvent::eKEY_FRONT;
   keyMap[KEY_COPY] = KeyEvent::eKEY_COPY;
   keyMap[KEY_OPEN] = KeyEvent::eKEY_OPEN;
   keyMap[KEY_PASTE] = KeyEvent::eKEY_PASTE;
   keyMap[KEY_FIND] = KeyEvent::eKEY_FIND;
   keyMap[KEY_CUT] = KeyEvent::eKEY_CUT;
   keyMap[KEY_HELP] = KeyEvent::eKEY_HELP;
   keyMap[KEY_MENU] = KeyEvent::eKEY_MENU;
   keyMap[KEY_CALC] = KeyEvent::eKEY_CALC;
   keyMap[KEY_SETUP] = KeyEvent::eKEY_SETUP;
   keyMap[KEY_SLEEP] = KeyEvent::eKEY_SLEEP;
   keyMap[KEY_WAKEUP] = KeyEvent::eKEY_WAKEUP;
   keyMap[KEY_FILE] = KeyEvent::eKEY_FILE;
   keyMap[KEY_SENDFILE] = KeyEvent::eKEY_SENDFILE;
   keyMap[KEY_DELETEFILE] = KeyEvent::eKEY_DELETEFILE;
   keyMap[KEY_XFER] = KeyEvent::eKEY_XFER;
   keyMap[KEY_PROG1] = KeyEvent::eKEY_PROG1;
   keyMap[KEY_PROG2] = KeyEvent::eKEY_PROG2;
   keyMap[KEY_WWW] = KeyEvent::eKEY_WWW;
   keyMap[KEY_MSDOS] = KeyEvent::eKEY_MSDOS;
   keyMap[KEY_COFFEE] = KeyEvent::eKEY_COFFEE;
   keyMap[KEY_DIRECTION] = KeyEvent::eKEY_DIRECTION;
   keyMap[KEY_CYCLEWINDOWS] = KeyEvent::eKEY_CYCLEWINDOWS;
   keyMap[KEY_MAIL] = KeyEvent::eKEY_MAIL;
   keyMap[KEY_BOOKMARKS] = KeyEvent::eKEY_BOOKMARKS;
   keyMap[KEY_COMPUTER] = KeyEvent::eKEY_COMPUTER;
   keyMap[KEY_BACK] = KeyEvent::eKEY_BACK;
   keyMap[KEY_FORWARD] = KeyEvent::eKEY_FORWARD;
   keyMap[KEY_CLOSECD] = KeyEvent::eKEY_CLOSECD;
   keyMap[KEY_EJECTCD] = KeyEvent::eKEY_EJECTCD;
   keyMap[KEY_EJECTCLOSECD] = KeyEvent::eKEY_EJECTCLOSECD;
   keyMap[KEY_NEXTSONG] = KeyEvent::eKEY_NEXTSONG;
   keyMap[KEY_PLAYPAUSE] = KeyEvent::eKEY_PLAYPAUSE;
   keyMap[KEY_PREVIOUSSONG] = KeyEvent::eKEY_PREVIOUSSONG;
   keyMap[KEY_STOPCD] = KeyEvent::eKEY_STOPCD;
   keyMap[KEY_RECORD] = KeyEvent::eKEY_RECORD;
   keyMap[KEY_REWIND] = KeyEvent::eKEY_REWIND;
   keyMap[KEY_PHONE] = KeyEvent::eKEY_PHONE;
   keyMap[KEY_ISO] = KeyEvent::eKEY_ISO;
   keyMap[KEY_CONFIG] = KeyEvent::eKEY_CONFIG;
   keyMap[KEY_HOMEPAGE] = KeyEvent::eKEY_HOMEPAGE;
   keyMap[KEY_REFRESH] = KeyEvent::eKEY_REFRESH;
   keyMap[KEY_EXIT] = KeyEvent::eKEY_EXIT;
   keyMap[KEY_MOVE] = KeyEvent::eKEY_MOVE;
   keyMap[KEY_EDIT] = KeyEvent::eKEY_EDIT;
   keyMap[KEY_SCROLLUP] = KeyEvent::eKEY_SCROLLUP;
   keyMap[KEY_SCROLLDOWN] = KeyEvent::eKEY_SCROLLDOWN;
   keyMap[KEY_KPLEFTPAREN] = KeyEvent::eKEY_KPLEFTPAREN;
   keyMap[KEY_KPRIGHTPAREN] = KeyEvent::eKEY_KPRIGHTPAREN;
   keyMap[KEY_F14] = KeyEvent::eKEY_F14;
   keyMap[KEY_F15] = KeyEvent::eKEY_F15;
   keyMap[KEY_F16] = KeyEvent::eKEY_F16;
   keyMap[KEY_F17] = KeyEvent::eKEY_F17;
   keyMap[KEY_F18] = KeyEvent::eKEY_F18;
   keyMap[KEY_F19] = KeyEvent::eKEY_F19;
   keyMap[KEY_F20] = KeyEvent::eKEY_F20;
   keyMap[KEY_F21] = KeyEvent::eKEY_F21;
   keyMap[KEY_F22] = KeyEvent::eKEY_F22;
   keyMap[KEY_F23] = KeyEvent::eKEY_F23;
   keyMap[KEY_F24] = KeyEvent::eKEY_F24;
   keyMap[KEY_PLAYCD] = KeyEvent::eKEY_PLAYCD;
   keyMap[KEY_PAUSECD] = KeyEvent::eKEY_PAUSECD;
   keyMap[KEY_PROG3] = KeyEvent::eKEY_PROG3;
   keyMap[KEY_PROG4] = KeyEvent::eKEY_PROG4;
   keyMap[KEY_SUSPEND] = KeyEvent::eKEY_SUSPEND;
   keyMap[KEY_CLOSE] = KeyEvent::eKEY_CLOSE;
   keyMap[KEY_PLAY] = KeyEvent::eKEY_PLAY;
   keyMap[KEY_FASTFORWARD] = KeyEvent::eKEY_FASTFORWARD;
   keyMap[KEY_BASSBOOST] = KeyEvent::eKEY_BASSBOOST;
   keyMap[KEY_PRINT] = KeyEvent::eKEY_PRINT;
   keyMap[KEY_HP] = KeyEvent::eKEY_HP;
   keyMap[KEY_CAMERA] = KeyEvent::eKEY_CAMERA;
   keyMap[KEY_SOUND] = KeyEvent::eKEY_SOUND;
   keyMap[KEY_QUESTION] = KeyEvent::eKEY_QUESTION;
   keyMap[KEY_EMAIL] = KeyEvent::eKEY_EMAIL;
   keyMap[KEY_CHAT] = KeyEvent::eKEY_CHAT;
   keyMap[KEY_SEARCH] = KeyEvent::eKEY_SEARCH;
   keyMap[KEY_CONNECT] = KeyEvent::eKEY_CONNECT;
   keyMap[KEY_FINANCE] = KeyEvent::eKEY_FINANCE;
   keyMap[KEY_SPORT] = KeyEvent::eKEY_SPORT;
   keyMap[KEY_SHOP] = KeyEvent::eKEY_SHOP;
   keyMap[KEY_ALTERASE] = KeyEvent::eKEY_ALTERASE;
   keyMap[KEY_CANCEL] = KeyEvent::eKEY_CANCEL;
   keyMap[KEY_BRIGHTNESSDOWN] = KeyEvent::eKEY_BRIGHTNESSDOWN;
   keyMap[KEY_MEDIA] = KeyEvent::eKEY_MEDIA;
   keyMap[KEY_UNKNOWN] = KeyEvent::eKEY_UNKNOWN;
   keyMap[KEY_OK] = KeyEvent::eKEY_OK;
   keyMap[KEY_SELECT] = KeyEvent::eKEY_SELECT;
   keyMap[KEY_GOTO] = KeyEvent::eKEY_GOTO;
   keyMap[KEY_CLEAR] = KeyEvent::eKEY_CLEAR;
   keyMap[KEY_POWER2] = KeyEvent::eKEY_POWER2;
   keyMap[KEY_OPTION] = KeyEvent::eKEY_OPTION;
   keyMap[KEY_INFO] = KeyEvent::eKEY_INFO;
   keyMap[KEY_TIME] = KeyEvent::eKEY_TIME;
   keyMap[KEY_VENDOR] = KeyEvent::eKEY_VENDOR;
   keyMap[KEY_ARCHIVE] = KeyEvent::eKEY_ARCHIVE;
   keyMap[KEY_PROGRAM] = KeyEvent::eKEY_PROGRAM;
   keyMap[KEY_CHANNEL] = KeyEvent::eKEY_CHANNEL;
   keyMap[KEY_FAVORITES] = KeyEvent::eKEY_FAVORITES;
   keyMap[KEY_EPG] = KeyEvent::eKEY_EPG;
   keyMap[KEY_PVR] = KeyEvent::eKEY_PVR;
   keyMap[KEY_MHP] = KeyEvent::eKEY_MHP;
   keyMap[KEY_LANGUAGE] = KeyEvent::eKEY_LANGUAGE;
   keyMap[KEY_TITLE] = KeyEvent::eKEY_TITLE;
   keyMap[KEY_SUBTITLE] = KeyEvent::eKEY_SUBTITLE;
   keyMap[KEY_ANGLE] = KeyEvent::eKEY_ANGLE;
   keyMap[KEY_ZOOM] = KeyEvent::eKEY_ZOOM;
   keyMap[KEY_MODE] = KeyEvent::eKEY_MODE;
   keyMap[KEY_KEYBOARD] = KeyEvent::eKEY_KEYBOARD;
   keyMap[KEY_SCREEN] = KeyEvent::eKEY_SCREEN;
   keyMap[KEY_PC] = KeyEvent::eKEY_PC;
   keyMap[KEY_TV] = KeyEvent::eKEY_TV;
   keyMap[KEY_TV2] = KeyEvent::eKEY_TV2;
   keyMap[KEY_VCR] = KeyEvent::eKEY_VCR;
   keyMap[KEY_VCR2] = KeyEvent::eKEY_VCR2;
   keyMap[KEY_SAT] = KeyEvent::eKEY_SAT;
   keyMap[KEY_SAT2] = KeyEvent::eKEY_SAT2;
   keyMap[KEY_CD] = KeyEvent::eKEY_CD;
   keyMap[KEY_TAPE] = KeyEvent::eKEY_TAPE;
   keyMap[KEY_RADIO] = KeyEvent::eKEY_RADIO;
   keyMap[KEY_TUNER] = KeyEvent::eKEY_TUNER;
   keyMap[KEY_PLAYER] = KeyEvent::eKEY_PLAYER;
   keyMap[KEY_TEXT] = KeyEvent::eKEY_TEXT;
   keyMap[KEY_DVD] = KeyEvent::eKEY_DVD;
   keyMap[KEY_AUX] = KeyEvent::eKEY_AUX;
   keyMap[KEY_MP3] = KeyEvent::eKEY_MP3;
   keyMap[KEY_AUDIO] = KeyEvent::eKEY_AUDIO;
   keyMap[KEY_VIDEO] = KeyEvent::eKEY_VIDEO;
   keyMap[KEY_DIRECTORY] = KeyEvent::eKEY_DIRECTORY;
   keyMap[KEY_LIST] = KeyEvent::eKEY_LIST;
   keyMap[KEY_MEMO] = KeyEvent::eKEY_MEMO;
   keyMap[KEY_CALENDAR] = KeyEvent::eKEY_CALENDAR;
   keyMap[KEY_RED] = KeyEvent::eKEY_RED;
   keyMap[KEY_GREEN] = KeyEvent::eKEY_GREEN;
   keyMap[KEY_YELLOW] = KeyEvent::eKEY_YELLOW;
   keyMap[KEY_BLUE] = KeyEvent::eKEY_BLUE;
   keyMap[KEY_CHANNELUP] = KeyEvent::eKEY_CHANNELUP;
   keyMap[KEY_CHANNELDOWN] = KeyEvent::eKEY_CHANNELDOWN;
   keyMap[KEY_FIRST] = KeyEvent::eKEY_FIRST;
   keyMap[KEY_LAST] = KeyEvent::eKEY_LAST;
   keyMap[KEY_AB] = KeyEvent::eKEY_AB;
   keyMap[KEY_NEXT] = KeyEvent::eKEY_NEXT;
   keyMap[KEY_RESTART] = KeyEvent::eKEY_RESTART;
   keyMap[KEY_SLOW] = KeyEvent::eKEY_SLOW;
   keyMap[KEY_SHUFFLE] = KeyEvent::eKEY_SHUFFLE;
   keyMap[KEY_BREAK] = KeyEvent::eKEY_BREAK;
   keyMap[KEY_PREVIOUS] = KeyEvent::eKEY_PREVIOUS;
   keyMap[KEY_DIGITS] = KeyEvent::eKEY_DIGITS;
   keyMap[KEY_TEEN] = KeyEvent::eKEY_TEEN;
   keyMap[KEY_TWEN] = KeyEvent::eKEY_TWEN;
   keyMap[KEY_DEL_EOL] = KeyEvent::eKEY_DEL_EOL;
   keyMap[KEY_DEL_EOS] = KeyEvent::eKEY_DEL_EOS;
   keyMap[KEY_INS_LINE] = KeyEvent::eKEY_INS_LINE;
   keyMap[KEY_DEL_LINE] = KeyEvent::eKEY_DEL_LINE;
}

static MouseEvent::eMouseButtonCode btnMap[KEY_CNT];

static void InitBtnMap()
{
   btnMap[BTN_0] = MouseEvent::eBTN_0;
   btnMap[BTN_1] = MouseEvent::eBTN_1;
   btnMap[BTN_2] = MouseEvent::eBTN_2;
   btnMap[BTN_3] = MouseEvent::eBTN_3;
   btnMap[BTN_4] = MouseEvent::eBTN_4;
   btnMap[BTN_5] = MouseEvent::eBTN_5;
   btnMap[BTN_6] = MouseEvent::eBTN_6;
   btnMap[BTN_7] = MouseEvent::eBTN_7;
   btnMap[BTN_8] = MouseEvent::eBTN_8;
   btnMap[BTN_9] = MouseEvent::eBTN_9;
   btnMap[BTN_LEFT] = MouseEvent::eBTN_LEFT;
   btnMap[BTN_RIGHT] = MouseEvent::eBTN_RIGHT;
   btnMap[BTN_MIDDLE] = MouseEvent::eBTN_MIDDLE;
   btnMap[BTN_SIDE] = MouseEvent::eBTN_SIDE;
   btnMap[BTN_EXTRA] = MouseEvent::eBTN_EXTRA;
   btnMap[BTN_FORWARD] = MouseEvent::eBTN_FORWARD;
   btnMap[BTN_BACK] = MouseEvent::eBTN_BACK;
   btnMap[BTN_TASK] = MouseEvent::eBTN_TASK;
   btnMap[BTN_TRIGGER] = MouseEvent::eBTN_TRIGGER;
   btnMap[BTN_THUMB] = MouseEvent::eBTN_THUMB;
   btnMap[BTN_THUMB2] = MouseEvent::eBTN_THUMB2;
   btnMap[BTN_TOP] = MouseEvent::eBTN_TOP;
   btnMap[BTN_TOP2] = MouseEvent::eBTN_TOP2;
   btnMap[BTN_PINKIE] = MouseEvent::eBTN_PINKIE;
   btnMap[BTN_BASE] = MouseEvent::eBTN_BASE;
   btnMap[BTN_BASE2] = MouseEvent::eBTN_BASE2;
   btnMap[BTN_BASE3] = MouseEvent::eBTN_BASE3;
   btnMap[BTN_BASE4] = MouseEvent::eBTN_BASE4;
   btnMap[BTN_BASE5] = MouseEvent::eBTN_BASE5;
   btnMap[BTN_BASE6] = MouseEvent::eBTN_BASE6;
   btnMap[BTN_DEAD] = MouseEvent::eBTN_DEAD;
   btnMap[BTN_A] = MouseEvent::eBTN_A;
   btnMap[BTN_B] = MouseEvent::eBTN_B;
   btnMap[BTN_C] = MouseEvent::eBTN_C;
   btnMap[BTN_X] = MouseEvent::eBTN_X;
   btnMap[BTN_Y] = MouseEvent::eBTN_Y;
   btnMap[BTN_Z] = MouseEvent::eBTN_Z;
   btnMap[BTN_TL] = MouseEvent::eBTN_TL;
   btnMap[BTN_TR] = MouseEvent::eBTN_TR;
   btnMap[BTN_TL2] = MouseEvent::eBTN_TL2;
   btnMap[BTN_TR2] = MouseEvent::eBTN_TR2;
   btnMap[BTN_SELECT] = MouseEvent::eBTN_SELECT;
   btnMap[BTN_START] = MouseEvent::eBTN_START;
   btnMap[BTN_MODE] = MouseEvent::eBTN_MODE;
   btnMap[BTN_THUMBL] = MouseEvent::eBTN_THUMBL;
   btnMap[BTN_THUMBR] = MouseEvent::eBTN_THUMBR;
   btnMap[BTN_TOOL_PEN] = MouseEvent::eBTN_TOOL_PEN;
   btnMap[BTN_TOOL_RUBBER] = MouseEvent::eBTN_TOOL_RUBBER;
   btnMap[BTN_TOOL_BRUSH] = MouseEvent::eBTN_TOOL_BRUSH;
   btnMap[BTN_TOOL_PENCIL] = MouseEvent::eBTN_TOOL_PENCIL;
   btnMap[BTN_TOOL_AIRBRUSH] = MouseEvent::eBTN_TOOL_AIRBRUSH;
   btnMap[BTN_TOOL_FINGER] = MouseEvent::eBTN_TOOL_FINGER;
   btnMap[BTN_TOOL_MOUSE] = MouseEvent::eBTN_TOOL_MOUSE;
   btnMap[BTN_TOOL_LENS] = MouseEvent::eBTN_TOOL_LENS;
   btnMap[BTN_TOUCH] = MouseEvent::eBTN_TOUCH;
   btnMap[BTN_STYLUS] = MouseEvent::eBTN_STYLUS;
   btnMap[BTN_STYLUS2] = MouseEvent::eBTN_STYLUS2;
   btnMap[BTN_TOOL_DOUBLETAP] = MouseEvent::eBTN_TOOL_DOUBLETAP;
   btnMap[BTN_TOOL_TRIPLETAP] = MouseEvent::eBTN_TOOL_TRIPLETAP;
   btnMap[BTN_GEAR_UP] = MouseEvent::eBTN_GEAR_UP;
}

static void AddKeyboardEvent(Platform *plat, uint32_t code, uint32_t value, const Time &time)
{
   PlatformDataNexus *pdn = (PlatformDataNexus*)(plat->GetPlatformData());

   plat->PushKeyEvent(KeyEvent(keyMap[code], stateMap[value], time));

   if (pdn->m_event)
      BKNI_SetEvent(pdn->m_event);
}

static void AddMouseEvent(Platform *platform, uint32_t type, uint32_t code, int32_t value, const Time &time)
{
   PlatformDataNexus *pdn = (PlatformDataNexus*)(platform->GetPlatformData());

   IVec3 relPos;
   IVec3 absPos;

   MouseEvent event;

   absPos = platform->AbsMP();

   if (type == EV_REL)
   {
      switch (code)
      {
      case REL_X :
      case REL_HWHEEL : relPos.X() += value;
         break;
      case REL_Y :
      case REL_WHEEL :  relPos.Y() += value;
         break;
      case REL_Z :      relPos.Z() += value;
         break;
      default:
         break;
      }

      if (relPos != IVec3(0, 0, 0))
      {
         switch (code)
         {
         case REL_X :
         case REL_Y :
         case REL_Z :
            absPos += relPos;
            if (platform->IsMouseMoveMerging())
               platform->RelMP() += relPos;
            else
            {
               event = MouseEvent(relPos, absPos, time);
               platform->PushMouseEvent(event);
            }
            platform->SetAbsoluteMousePosition(absPos);
            if (pdn->m_event)
               BKNI_SetEvent(pdn->m_event);
            break;
         case REL_WHEEL :
         case REL_HWHEEL :
            event = MouseEvent(IVec2(relPos.X(), relPos.Y()), absPos, time);
            platform->PushMouseEvent(event);
            if (pdn->m_event)
               BKNI_SetEvent(pdn->m_event);
            break;
         }
      }
   }
   else if (type == EV_KEY)
   {
      event = MouseEvent(btnMap[code], btnStateMap[value], absPos, time);
      platform->PushMouseEvent(event);
      if (pdn->m_event)
         BKNI_SetEvent(pdn->m_event);
   }
}

#ifdef SINGLE_PROCESS

static void gotSIGUSR1(int sig, siginfo_t *info, void *ucontext)
{
   // Nothing, just unblock the IO
}

static void *KeyboardPollThread(void *plat)
{
   struct input_event ev[64];
   int                rd, i;
   Platform           *platform = (Platform*)plat;
   PlatformDataNexus  *pdn = (PlatformDataNexus*)platform->GetPlatformData();

   struct sigaction sa;
   sa.sa_handler = NULL;
   sa.sa_sigaction = gotSIGUSR1;
   sa.sa_flags = SA_SIGINFO;
   sigemptyset(&sa.sa_mask);

   if (sigaction(SIGUSR1, &sa, NULL) < 0) {
      BSG_THROW("Unable to install signal handler on KeyboardPollThread");
   }

   while (pdn->m_kbdFd != 0)
   {
      // Signal should abort this blocking read, rd will be 0 and the loop will quit
      rd = read(pdn->m_kbdFd, ev, sizeof(struct input_event) * 64);

      if (pdn->m_kbdFd == 0)
         return 0;

      if (rd < (int)sizeof(struct input_event))
         return 0;
      else
      {
         for (i = 0; i < rd / (int)sizeof(struct input_event); i++)
         {
            if (ev[i].type == EV_KEY)
            {
               // If using internal functions
               if (!(platform->GetKeyEventHandlerCallback() || platform->GetKeyEventHandlerCallback2()))
                  AddKeyboardEvent(platform, ev[i].code, ev[i].value, Time(ev[i].time.tv_sec, ev[i].time.tv_usec));
               else  if (platform->GetKeyEventHandlerCallback())
                        platform->GetKeyEventHandlerCallback()(keyMap[ev[i].code]);
                     else
                        platform->GetKeyEventHandlerCallback2()(keyMap[ev[i].code], stateMap[ev[i].value]);
            }
         }
      }
   }

   return NULL;
}

static void *MousePollThread(void *plat)
{
   struct input_event ev[64];
   int                rd, i;
   Platform           *platform = (Platform*)plat;
   PlatformDataNexus  *pdn = (PlatformDataNexus*)platform->GetPlatformData();

   struct sigaction sa;
   sa.sa_handler = NULL;
   sa.sa_sigaction = gotSIGUSR1;
   sa.sa_flags = SA_SIGINFO;
   sigemptyset(&sa.sa_mask);

   if (sigaction(SIGUSR1, &sa, NULL) < 0) {
      BSG_THROW("Unable to install signal handler on KeyboardPollThread");
   }

   while (pdn->m_mouseFd != 0)
   {
      // Signal should abort this blocking read, rd will be 0 and the loop will quit
      rd = read(pdn->m_mouseFd, ev, sizeof(struct input_event) * 64);

      if (pdn->m_mouseFd == 0)
         return 0;

      if (rd < (int)sizeof(struct input_event))
         return 0;
      else
      {
         // Check if we used external functions to handle mouse event callbacks
         bool useInternalEventHandler = !(platform->GetMouseButtonHandlerCallback() || platform->GetMouseMoveHandlerCallback());

         // Check that both external functions are registered if external callbacks are used
         if (!useInternalEventHandler &&
            (!platform->GetMouseButtonHandlerCallback() || !platform->GetMouseMoveHandlerCallback()))
            BSG_THROW("Both mouse button and move handler functions must be registered\n");

         for (i = 0; i < rd / (int)sizeof(struct input_event); i++)
         {
            if (useInternalEventHandler)
            {
               Time time(ev[i].time.tv_sec, ev[i].time.tv_usec);
               AddMouseEvent(platform, ev[i].type, ev[i].code, ev[i].value, time);
            }
            else
            {
               if (ev[i].type == EV_KEY)
                  platform->GetMouseButtonHandlerCallback()(btnMap[ev[i].code], btnStateMap[ev[i].value]);

               if (ev[i].type == EV_REL)
               {
                  if (ev[i].code == REL_X)
                     platform->GetMouseMoveHandlerCallback()(ev[i].value, 0);
                  else
                     platform->GetMouseMoveHandlerCallback()(0, ev[i].value);
               }
            }
         }
      }
   }

   return NULL;
}

#else

static void inputRouterCallback(void *pParam, int iParam)
{
   Platform          *platform = (Platform*)pParam;
   PlatformDataNexus *pdn = (PlatformDataNexus*)(platform->GetPlatformData());

   BSTD_UNUSED(iParam);

   NEXUS_InputRouterCode code[16];
   uint32_t              num;
   static Time           lastSentEventTime;

   do
   {
      num = 0;
      NEXUS_InputClient_GetCodes(pdn->m_inputRouterClient, code, 16, &num);

      Time now = Time::Now();

      for (uint32_t c = 0; c < num; c++)
      {
         switch (code[c].deviceType)
         {
         case NEXUS_InputRouterDevice_eIrInput :
            if (!code[c].data.irInput.repeat || (code[c].data.irInput.code != RAW_EXIT && (now - lastSentEventTime).Milliseconds() > 150))
            {
               /* pdn->m_IRMode = (NEXUS_IrInputMode)code[c].data.irInput.mode; */
               platform->PushKeyEvent(KeyEvent(pdn->MapKeyCode(code[c].data.irInput.code), now));
               lastSentEventTime = now;
               if (pdn->m_event)
                  BKNI_SetEvent(pdn->m_event);
            }
            break;
         case NEXUS_InputRouterDevice_eEvdev :
            if (code[c].data.evdev.type == EV_REL)
            {
               if ((code[c].data.evdev.code != 0) || (code[c].data.evdev.value != 0))
               {
                  switch (code[c].data.evdev.code)
                  {
                     case REL_X:
                     case REL_Y:
                     case REL_WHEEL:
                        AddMouseEvent(platform, EV_REL, code[c].data.evdev.code, code[c].data.evdev.value, now);
                     break;

                     case BTN_LEFT:
                     case BTN_RIGHT:
                     case BTN_MIDDLE:
                        AddMouseEvent(platform, EV_KEY, code[c].data.evdev.code, code[c].data.evdev.value, now);
                        break;

                     default:
                        break;
                  }
               }
            }
            else if (code[c].data.evdev.type == EV_KEY)
            {
               if (code[c].data.evdev.code == BTN_LEFT || code[c].data.evdev.code == BTN_RIGHT)
                  AddMouseEvent(platform, EV_KEY, code[c].data.evdev.code, 1, now);
               else
                  AddKeyboardEvent(platform, code[c].data.evdev.code, code[c].data.evdev.value, now);
            }
            break;
         default:
            break;
         }
      }
   }
   while (num != 0);
}

#endif

void PlatformDataNexus::InitUSBInputs()
{
   InitKeyMap();
   InitBtnMap();

#ifdef SINGLE_PROCESS
   // Identify kbd and mouse devices
   int  fd;
   char name[256] = "Unknown";

   for (uint32_t e = 0; e < 6; e++)
   {
      char devName[32];
      sprintf(devName, "/dev/event%d", e);

      if ((fd = open(devName, O_RDONLY)) > 0)
      {
         ioctl(fd, EVIOCGNAME(sizeof(name)), name);

         if (m_kbdFd == 0 && strstr(name, "Keyboard"))
         {
            m_kbdFd = fd;
         }
         else if (m_mouseFd == 0 && strstr(name, "Mouse"))
         {
            m_mouseFd = fd;
         }
         else
            close(fd);
      }
   }

   if (m_kbdFd != 0)
   {
      int rc = pthread_create(&m_kbdThread, NULL, KeyboardPollThread, (void *)m_platform);
      if (rc)
         BSG_THROW("Unable to create USB keyboard handling thread");
   }

   if (m_mouseFd != 0)
   {
      int rc = pthread_create(&m_mouseThread, NULL, MousePollThread, (void *)m_platform);
      if (rc)
         BSG_THROW("Unable to create USB mouse handling thread");
   }
#endif
}

void PlatformDataNexus::TermUSBInputs()
{
#ifdef SINGLE_PROCESS
   int mfd = m_mouseFd;
   int kfd = m_kbdFd;

   m_mouseFd = 0;
   m_kbdFd = 0;

   if (m_kbdThread)
   {
      pthread_kill(m_kbdThread, SIGUSR1);
      pthread_join(m_kbdThread, NULL);
   }

   if (m_mouseThread)
   {
      pthread_kill(m_mouseThread, SIGUSR1);
      pthread_join(m_mouseThread, NULL);
   }

   close(mfd);
   close(kfd);
#endif
}

void PlatformDataNexus::InitRoutedInputs()
{
   InitKeyMap();
   InitBtnMap();

#ifndef SINGLE_PROCESS
   int rc;

   NxClient_AllocSettings allocSettings;
   NxClient_AllocResults allocResults;

   m_IRMode = NEXUS_IrInputMode_eCirNec;

   NxClient_GetDefaultAllocSettings(&allocSettings);
   allocSettings.inputClient = 1;
   rc = NxClient_Alloc(&allocSettings, &allocResults);
   if (rc)
   {
      fprintf(stderr, "WARNING : Unable to alloc the input client\n");
      return;
   }

   if (allocResults.inputClient[0].id)
   {
      m_inputRouterClient = NEXUS_InputClient_Acquire(allocResults.inputClient[0].id);
      if (m_inputRouterClient == NULL)
      {
         fprintf(stderr, "WARNING : Unable to acquire the input client\n");
      }
      else
      {
         NEXUS_InputClientSettings settings;

         NEXUS_InputClient_GetSettings(m_inputRouterClient, &settings);

         settings.filterMask = (1 << NEXUS_InputRouterDevice_eIrInput) |
                                 (1 << NEXUS_InputRouterDevice_eEvdev);

         settings.codeAvailable.callback = inputRouterCallback;
         settings.codeAvailable.context = m_platform;

         NEXUS_InputClient_SetSettings(m_inputRouterClient, &settings);
      }
   }
   else
      fprintf(stderr, "WARNING : Unable to alloc the input client\n");
#endif
}

void PlatformDataNexus::TermRoutedInputs()
{
#ifndef SINGLE_PROCESS
   if (m_inputRouterClient)
      NEXUS_InputClient_Release(m_inputRouterClient);
#endif
}

bool Platform::IsMouseAttached() const
{
#ifdef SINGLE_PROCESS
   return platform_data->m_mouseFd != 0;
#else
   return true;
#endif
}

bool Platform::IsKeyboardAttached() const
{
#ifdef SINGLE_PROCESS
   return platform_data->m_kbdFd != 0;
#else
   return true;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


NativePixmapData::~NativePixmapData()
{
}

NativePixmap::NativePixmap(uint32_t w, uint32_t h, ePixmapFormat format) :
   m_width(w),
   m_height(h),
   m_format(format),
   m_eglPixmap(NULL)
{
   m_priv = new NexusPixmapData;

   BEGL_PixmapInfoEXT   pixInfo;
   EGLNativePixmapType  eglPixmap;
   NEXUS_SurfaceHandle  nexusSurf;

   NXPL_GetDefaultPixmapInfoEXT(&pixInfo);
   pixInfo.width  = w;
   pixInfo.height = h;
   pixInfo.secure = Platform::Instance()->GetOptions().GetSecure();

#ifdef BSG_VC5
/* Legacy from VC4 */
#define BEGL_BufferFormat_eYUV422_Texture BEGL_BufferFormat_eYUV422
#define BEGL_BufferFormat_eYV12_Texture BEGL_BufferFormat_eYV12
#endif

#ifdef BIG_ENDIAN_CPU
   switch (format)
   {
   case RGB565_TEXTURE     : pixInfo.format = BEGL_BufferFormat_eR5G6B5; break;
   case ABGR8888_TEXTURE   : pixInfo.format = BEGL_BufferFormat_eR8G8B8A8; break;
   case YUV422_TEXTURE     : pixInfo.format = BEGL_BufferFormat_eVUY224_Texture; break;
   case eYV12_TEXTURE      : pixInfo.format = BEGL_BufferFormat_eYV12_Texture; break;
   }
#else
   switch (format)
   {
   case RGB565_TEXTURE     : pixInfo.format = BEGL_BufferFormat_eR5G6B5; break;
   case ABGR8888_TEXTURE   : pixInfo.format = BEGL_BufferFormat_eA8B8G8R8; break;
   case YUV422_TEXTURE     : pixInfo.format = BEGL_BufferFormat_eYUV422_Texture; break;
   case eYV12_TEXTURE      : pixInfo.format = BEGL_BufferFormat_eYV12_Texture; break;
   }
#endif

   if (NXPL_CreateCompatiblePixmapEXT(
                  ((PlatformDataNexus*)Platform::Instance()->GetPlatformData())->m_platformHandle,
                  &eglPixmap, &nexusSurf, &pixInfo))
   {
      m_eglPixmap = eglPixmap;
      ((NexusPixmapData*)m_priv)->m_surface = nexusSurf;

      NEXUS_SurfaceStatus status;
      NEXUS_Surface_GetStatus(nexusSurf, &status);

      m_width  = status.width;
      m_height = status.height;
      m_stride = status.pitch;

      if (!pixInfo.secure)
      {
         NEXUS_SurfaceMemory  memory;
         NEXUS_Surface_GetMemory(nexusSurf, &memory);

         memset(memory.buffer, 0, m_stride * m_height);
         NEXUS_Surface_Flush(nexusSurf);
      }
   }
   else
      BSG_THROW("Unable to create native pixmap");

   if (nexusSurf == NULL)
      BSG_THROW("Unable to create native pixmap");
}

void *NativePixmap::GetPixelDataPtr() const
{
   NEXUS_SurfaceMemory  mem;

   NEXUS_Surface_GetMemory(((NexusPixmapData*)m_priv)->m_surface, &mem);

   return mem.buffer;
}

NativePixmap::~NativePixmap()
{
   if (m_eglPixmap)
      NXPL_DestroyCompatiblePixmap(((PlatformDataNexus*)Platform::Instance()->GetPlatformData())->m_platformHandle,
                                    m_eglPixmap);

   delete m_priv;
}

}
