/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *   See code
 *
 ***************************************************************************/
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "bfmt.h"
#include "bmma.h"
#include "bxvd.h"
#include "bxvd_platform.h"
#include "bavc.h"
#include "bxvd_vdec_info.h"
#include "bxvd_ppb.h"
#include "bxvd_priv.h"
#include "bxvd_reg.h"
#include "btmr.h"
#include "bxvd_pvr.h"
#include "bxvd_intr.h"
#include "bxvd_image.h"
#include "bchp_common.h"
#include "bxvd_status.h"
#include "bxvd_decoder.h"
#include "bbox_xvd.h"

#if BXVD_P_ENABLE_DRAM_PREF_INFO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if DEBUG_STILL_PICTURE
#include <stdio.h>
#endif

#include "bxvd_dbg.h"

#include "bxdm_dih.h"
#include "bxvd_dip.h"
#include "bxdm_pp.h"

BDBG_MODULE(BXVD);

/* This is temporary until defined in chp */
#ifndef BCHP_AVD0_INTR2_CPU_STATUS_AVD_ARC_INTR_SHIFT
#define BCHP_AVD0_INTR2_CPU_STATUS_AVD_ARC_INTR_SHIFT 2
#endif

/* Default settings. */
static const BXVD_Settings s_stBXVD_DefaultSettings =
{
   0,                         /* Decoder instance (always 0 for 7401, 7118) */
   BXVD_RaveEndianess_eBig,

   NULL,                      /* Decoder firmware memory heap handle where FW is loaded */
   NULL,                      /* Secure cabac memory heap handle */
   NULL,                      /* General purpose device memory heap handle */
   NULL,                      /* Decoder picture buffer memory heap handle */
   NULL,                      /* Decoder picture buffer 1 memory heap handle */

   NULL,                      /* FirmwareBlock where FW code and data is loaded */
   0,                         /* FirmwareBlockOffset in block where FW code is to be loaded */
   0,                         /* FirmwareBlockSize memory allocated for FW to be loaded */

   NULL,                      /* Debug timer */

   NULL,                      /* Set the BIMG interface to NULL by default */
   NULL,                      /* Set the BIMG context to NULL by default */

   NULL,                      /* Set the AVD Boot Callback to NULL by default */
   NULL,                      /* Set the AVD Boot Callback data pointer to NULL by default */

   NULL,                      /* Set the AVD Reset Callback to NULL by default */
   NULL,                      /* Set the AVD Reset Callback data pointer to NULL by default */
#ifdef BXVD_ENABLE_DVD_API
   BXVD_DisplayMgrMode_eUOD1, /* Set Display Manager mode to UOD 1.x (DVD Build)*/
#else
   BXVD_DisplayMgrMode_eSTB,  /* Set Display Manager mode to STB (non-DVD Build)*/
#endif
   {0,0,0,0,0},               /* Custom FW video memory configuration */
   0,                         /* Suggested size: 16 kb decoder debug log buffer size, 0 is default */
   false,                     /* SVC Inter Layer Bandwidth Optimized, disabled by default */
   0                          /* Number of channel contexts to pre-allocate */
};

static const  BAVC_VideoCompressionStd  VideoCmprStdList_H264[] =
{
   BAVC_VideoCompressionStd_eH264
};

/* Default channel settings */
static const BXVD_ChannelSettings s_stDefaultChannelSettings =
{
   BFMT_VERT_59_94Hz,
   0, /* PR20202: Initial removal delay */
   10, /* PR19284: Lip sync fix */
   BAVC_FrameRateCode_eUnknown, /* PR27373: User-settable default frame rate if unknown from stream */
   BXVD_DecodeResolution_eHD,
   (BAVC_VideoCompressionStd *)&VideoCmprStdList_H264,
   1,                          /* DisplayInterrupt */
   0,                          /* VDCRectangleNum */
   BXVD_DisplayInterrupt_eZero,
   BXVD_ChannelMode_eVideo,
   true,
   false,
   false,
   false,
   false,
   false,
   false,
   false,
   false,
   false,
   false,
   0,
   NULL, 0, 0,
   NULL, 0, 0,
   NULL, 0, 0,
   NULL, 0, 0,
   BXVD_1080pScanMode_eDefault,
   BXVD_PictureDropMode_eFrame            /* use frame drops */
};

#define BXVD_DECODESETTINGS_SIGNATURE INT32_C (0xeeeea1a1)

/* SWSTB-3450: moved the initialization to BXVD_S_GetDecodeDefaultSettings */

#if 0
/* Default channel settings */
static const BXVD_DecodeSettings s_stDefaultDecodeSettings =
{
   BAVC_VideoCompressionStd_eMPEG2,       /* Compression Standard */
   0,                                     /* Deprecated, MultiStreamId */
   0,                                     /* Deprecated, VideoSubStreamId */
   false,                                 /* Playback boolean */
   false,                                 /* CRCMode boolean */
   {0,0},                                 /* Deprecated, BAVC_XptOutput */
   0,                                     /* Deprecated, TimeBase */
   NULL,                                  /* Xpt ContextMap, must be specified */
   {NULL},                                /* Xpt ContextMap Extended, Optional */
   0,                                     /* Xpt ContextMap Extended Number, Optional */
   BXVD_DECODESETTINGS_SIGNATURE,         /* Signature, indicates BXVD_GetDecodeDefaultSettings
                                             being called */
   BXVD_STC_eZero,                        /* STC number, is STC0 for instance 0 and STC1 for instance 1 */
   BXVD_DisplayInterrupt_eZero,
   BXVD_DECODE_MODE_RECT_NUM_INVALID,     /* Rectangle Number, use channel number if invalid */
   false,                                 /* Disable DM ASTM mode */
   true,                                  /* Disable DM bVsyncModeOnPcrDiscontinuity */
   BXVD_HITSMode_eDisabled,               /* Disable MPEG HITS Mode */
   false,                                 /* Disable H264 Zero Delay Output Mode */
   BAVC_FrameRateCode_eUnknown,           /* FrameRate */
   0,                                     /* preroll rate of '0' */
   1,                                     /* SW7445-2421: uiPreRollNumerator */
   BXVD_ErrorHandling_eOff,               /* picture error handling is off */
   BXVD_FrameRateDetectionMode_eOff,      /* Frame Rate Detection (FRD) is off */
   false,                                 /* Disable Blu-ray decode mode */
   BXVD_ProgressiveOverrideMode_eTopBottom, /* TopBottom 480p/576p/1080p progressive override */
   BXVD_TimestampMode_eDecode,            /* timestamps are expected in display order */
   false,                                 /* Don't treat I Frame as RAP for AVC decode */
   false,                                 /* AVC error concealment disabled by default */
   false,                                 /* AVC I only field output mode, disabled by default */
   false,                                 /* SW3556-1058:: conditionally ignore the DPB output delay syntax */
   0,                                     /* SEI Message Flags */
   false,                                 /* P frame skip mode enabled */
   false,                                 /* External picture provider interface NOT in use */
   false,                                 /* Disable AVC Aspect Ratio Override mode */
   false,                                 /* Disable 3D decode for SVC protocol */
   false,                                 /* Disable SW Coefficient decode of AVC streams */
   false,                                 /* Ignore AVC Num Reorder Frames equal zero */
   false,                                 /* Disable Early Picture Delivery Mode */
   NULL,                                  /* SW7425-1064: with linked channels, BXVD_StartDecode will be called
                                           * once for both channel, hence the need to link the decode settings. */
   false,                                 /* Disable Userdata in BTP mode */
   false,                                 /* Disable NRT decode mode */
   BXVD_PCRDiscontinuityMode_eDrop,       /* SWSTB-68: ePCRDiscontinuityModeAtStartup default to drop */
   0,                                     /* SWSTB-439: default error level is 0 */

   {{          /* SWSTB-3450: default values for BXDM_PictureProvider_StartSettings. */
      false,
      BAVC_TransferCharacteristics_eUnknown,
      {
         0,
         0,
         {{0xFFFFFFFF,0xFFFFFFFF},{0xFFFFFFFF, 0xFFFFFFFF},{0xFFFFFFFF, 0xFFFFFFFF}},
         {0xFFFFFFFF, 0xFFFFFFFF},
         0xFFFFFFFF,
         0xFFFFFFFF,
         BAVC_TransferCharacteristics_eUnknown,
      },
   }}
};
#endif

/* Default device VDC interrupt settings */
static const BXVD_DeviceVdcInterruptSettings  s_stDefaultDevVdcIntrSettings =
{
   0,0,0,                                   /* Top/Bot/Frame field masks */
   BXVD_DisplayInterrupt_eZero,
   BXVD_DeviceVdcIntrSettingsFlags_None,
   0     /* To be removed */
};

/* Static array defining CDB and ITB sizes for various
   Buffer Configuration Modes */
static const BAVC_CdbItbConfig sCdbItbCfg[] =
{
   { { 0x00000000, 8, false }, {          0, 7, false }, false }   /*  BXVD_DecodeMode_eCustom         */
};

#if BDBG_DEBUG_BUILD

static const char * const sRaveEndianessNameLUT[BXVD_RaveEndianess_eMaxValue] =
{
   "BXVD_RaveEndianess_eBig",
   "BXVD_RaveEndianess_eLittle"
};

static const char * const sDisplayMgrModeNameLUT[BXVD_DisplayMgrMode_eMaxModes] =
{
   "BXVD_DisplayMgrMode_eSTB",
};

static const char * const sInstanceNameLUT[BXVD_MAX_INSTANCE_COUNT] =
{
   "[00]",
#if BXVD_MAX_INSTANCE_COUNT > 1
   "[01]",
#endif
};

static const char * const sChannelNameLUT[BXVD_MAX_INSTANCE_COUNT][BXVD_MAX_VIDEO_CHANNELS] =
{
   {
      "[00][00]",
      "[00][01]",
      "[00][02]",
      "[00][03]",
      "[00][04]",
      "[00][05]",
      "[00][06]",
      "[00][07]",
      "[00][08]",
      "[00][09]",
      "[00][10]",
      "[00][11]",
      "[00][12]",
      "[00][13]",
      "[00][14]",
      "[00][15]",
   },
#if BXVD_MAX_INSTANCE_COUNT > 1
   {
      "[01][00]",
      "[01][01]",
      "[01][02]",
      "[01][03]",
      "[01][04]",
      "[01][05]",
      "[01][06]",
      "[01][07]",
      "[01][08]",
      "[01][09]",
      "[01][10]",
      "[01][11]",
      "[01][12]",
      "[01][13]",
      "[01][14]",
      "[01][15]",
   },
#endif
};
#endif

#if BXVD_P_ENABLE_DRAM_PREF_INFO
/* Global performnce debug info */
int giPerfMemc, giPerfMemcClient;
#endif

/***************************************************************************
BXVD_Open: Opens and initializes XVD
****************************************a**********************************/
BERR_Code BXVD_Open(BXVD_Handle         *phXvd,
                    BCHP_Handle         hChip,
                    BREG_Handle         hRegister,
                    BINT_Handle         hInterrupt,
                    const BXVD_Settings *pDefSettings)
{
   BERR_Code rc;
   BXVD_P_Context *pXvd = (BXVD_P_Context*)NULL;

   BXVD_P_MemCfgMode eMemCfgMode;
   BTMR_Settings tmrSettings;
   BDBG_Level eDefaultDebugLevel;

#if BDBG_DEBUG_BUILD
   uint32_t uiGISBMask;
#endif

   BDBG_ENTER(BXVD_Open);

#if BXVD_PPB_EXTENDED
   /* This tests to see if private PPB crc element is at same offset as public PPB crc element */
   BDBG_CASSERT(offsetof(BXVD_P_PPB, crc) == offsetof(BXVD_PPB, crc));
#endif

   BDBG_ASSERT(phXvd);
   BDBG_ASSERT(hChip);
   BDBG_ASSERT(hRegister);
   BDBG_ASSERT(hInterrupt);

   /* Set handle to NULL in case the allocation fails */
   *phXvd = NULL;

   /* We need to at least have a General memory handle */
   if (!pDefSettings)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   else if (!pDefSettings->hGeneralHeap && !pDefSettings->hFirmwareHeap)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   BXVD_P_VALIDATE_PDEFSETTINGS(pDefSettings);

   pXvd = (BXVD_P_Context*)(BKNI_Malloc(sizeof(BXVD_P_Context)));
   if (!pXvd)
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   BDBG_REGISTER_INSTANCE(pXvd);
   rc = BDBG_GetLevel(&eDefaultDebugLevel);
   rc = BDBG_SetInstanceLevel(pXvd, eDefaultDebugLevel);

   /* Zero out the newly allocated context */
   BKNI_Memset((void*)pXvd, BXVD_P_MEM_ZERO, sizeof(BXVD_P_Context));

   BKNI_Memcpy((void *)&pXvd->stSettings,
               (void *)pDefSettings,
               sizeof(BXVD_Settings));

   /* validate instance id */
   if (pXvd->stSettings.uiAVDInstance >= BXVD_MAX_INSTANCE_COUNT)
   {
      BXVD_DBG_ERR(pXvd, ("BXVD_Open() - phXvd AVD Instance invalid = %d", pXvd->stSettings.uiAVDInstance));
      BKNI_Free(pXvd);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXVD_P_InitFreeChannelList(pXvd);
   if (BERR_SUCCESS != rc)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

#if BXVD_P_FW_DEBUG_DRAM_LOGGING
   if (pXvd->stSettings.uiDecoderDebugLogBufferSize == 0)
   {
      pXvd->stSettings.uiDecoderDebugLogBufferSize = 16*1024; /* Log size in bytes */
   }
#endif
   /* Set the decoder instance here. This will always be 0 for single decoder systems */
   pXvd->uDecoderInstance = pXvd->stSettings.uiAVDInstance;

   rc = BDBG_SetInstanceName(pXvd, sInstanceNameLUT[pXvd->uDecoderInstance]);

   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - phXvd = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)phXvd));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - hChip = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hChip));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - hRegister = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hRegister));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - hInterrupt = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hInterrupt));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - pDefSettings = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pDefSettings));

#if BDBG_DEBUG_BUILD
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.uiAVDInstance = %d", pXvd->stSettings.uiAVDInstance));

   if (pXvd->stSettings.eRaveEndianess < BXVD_RaveEndianess_eMaxValue)
   {
      BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.eRaveEndianess = %s (%d)",
                          sRaveEndianessNameLUT[pXvd->stSettings.eRaveEndianess],
                          pXvd->stSettings.eRaveEndianess));
   }
   else
   {
      BXVD_DBG_WRN(pXvd, ("BXVD_Open() - BXVD_Settings.eRaveEndianess = %s (%d)",
                          "Unknown/Invalid Value!",
                          pXvd->stSettings.eRaveEndianess));
   }

   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.hFirmwareHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.hFirmwareHeap));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.hCabacHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.hCabacHeap));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.hGeneralHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.hGeneralHeap));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.hPictureHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.hPictureHeap));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.hPictureHeap1 = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.hPictureHeap1));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.hFirmwareBlock = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.hFirmwareBlock));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.uiFirmwareBlockOffset = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.uiFirmwareBlockOffset));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.uiFirmwareBlockSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.uiFirmwareBlockSize));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.pImgInterface = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.pImgInterface));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.pImgContext = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.pImgContext));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.pAVDBootCallback = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.pAVDBootCallback));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.pAVDBootCallbackData = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.pAVDBootCallbackData));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.pAVDResetCallback = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.pAVDResetCallback));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.pAVDResetCallbackData = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.pAVDResetCallbackData));

   if (pXvd->stSettings.eDisplayMgrMode < BXVD_DisplayMgrMode_eMaxModes)
   {
      BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.eDisplayMgrMode = %s (%d)",
                          sDisplayMgrModeNameLUT[pXvd->stSettings.eDisplayMgrMode],
                          pXvd->stSettings.eDisplayMgrMode));
   }
   else
   {
      BXVD_DBG_WRN(pXvd, ("BXVD_Open() - BXVD_Settings.eDisplayMgrMode = %s (%d)",
                          "Unknown/Invalid Value!",
                          pXvd->stSettings.eDisplayMgrMode));
   }

   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.stFWMemConfig.uiGeneralHeapSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.stFWMemConfig.uiGeneralHeapSize));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.stFWMemConfig.uiCabacHeapSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.stFWMemConfig.uiCabacHeapSize));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.stFWMemConfig.uiPictureHeapSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.stFWMemConfig.uiPictureHeapSize));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.stFWMemConfig.uiPictureHeap1Size = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.stFWMemConfig.uiPictureHeap1Size));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - BXVD_Settings.uiDecoderDebugLogBufferSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->stSettings.uiDecoderDebugLogBufferSize));
#endif

   /* Each platform has a mask of supported decode protocols, used to validate in BXVD_Startdecode. */
   pXvd->uiSupportedProtocolsMask = BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS;

   /* Create timer */
   if (pXvd->stSettings.hTimerDev)
   {
      BXVD_DBG_MSG(pXvd, ("Creating timer in BXVD_Open"));
      rc = BTMR_GetDefaultTimerSettings(&tmrSettings);

      tmrSettings.type = BTMR_Type_eSharedFreeRun;
      tmrSettings.exclusive = false;

      if (BERR_SUCCESS == rc)
      {
         rc = BTMR_CreateTimer(pXvd->stSettings.hTimerDev,
                               &pXvd->hTimer,
                               &tmrSettings);

         if (BERR_SUCCESS != rc)
         {
            pXvd->hTimer = NULL;
            BXVD_DBG_WRN(pXvd, ("Error creating timer"));
         }

      }
   }

   /* Store handles in the context */
   pXvd->hChip = hChip;
   pXvd->hInterrupt = hInterrupt;
   pXvd->hReg = hRegister;

   /* Set handle type */
   pXvd->eHandleType = BXVD_P_HandleType_XvdMain;

   /* Set the heaps */
   pXvd->hGeneralHeap = pXvd->stSettings.hGeneralHeap;

   if (pXvd->stSettings.hPictureHeap)
   {
      pXvd->hPictureHeap = pXvd->stSettings.hPictureHeap;
      pXvd->hPictureHeap1 = pXvd->stSettings.hPictureHeap1;
   }
   else if (pXvd->stSettings.stFWMemConfig.uiPictureHeapSize != 0)
   {
      pXvd->hPictureHeap = pXvd->hGeneralHeap;
   }

   if (pXvd->stSettings.hFirmwareHeap)
   {
      pXvd->hFirmwareHeap = pXvd->stSettings.hFirmwareHeap;
   }
   else
   {
      pXvd->hFirmwareHeap = pXvd->hGeneralHeap;
   }

   if (pXvd->stSettings.hCabacHeap)
   {
      pXvd->hCabacHeap = pXvd->stSettings.hCabacHeap;
   }
   else
   {
      pXvd->hCabacHeap = pXvd->hGeneralHeap; /* Secure heap not specified, use general application heap */
   }

   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - hPictureHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->hPictureHeap));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - hGeneralHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->hGeneralHeap));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - hFirmwareHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->hFirmwareHeap));
   BXVD_DBG_MSG(pXvd, ("BXVD_Open() - hCabacHeap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvd->hCabacHeap));


#if !(BXVD_USE_CUSTOM_IMAGE)
   if (pXvd->stSettings.pImgInterface != &BXVD_IMAGE_Interface)
   {
      BXVD_DBG_WRN(pXvd, ("*******************"));
      BXVD_DBG_WRN(pXvd, ("You've linked in the default XVD BIMG interface and context."));
      BXVD_DBG_WRN(pXvd, ("However, you are providing your own version(s) to BXVD_Open()."));
      BXVD_DBG_WRN(pXvd, ("You should compile with BXVD_USE_CUSTOM_IMAGE=1 to prevent linkage"));
      BXVD_DBG_WRN(pXvd, ("of the default BIMG interface and context to reduce the binary size"));
      BXVD_DBG_WRN(pXvd, ("*******************"));
   }

#if !(BXVD_USE_CUSTOM_CONTEXT)
   if (pXvd->stSettings.pImgContext != BXVD_IMAGE_Context)
   {
      BXVD_DBG_WRN(pXvd, ("*******************"));
      BXVD_DBG_WRN(pXvd, ("You've linked in the default XVD BIMG context."));
      BXVD_DBG_WRN(pXvd, ("However, you are providing your own version to BXVD_Open()."));
      BXVD_DBG_WRN(pXvd, ("You should compile with BXVD_USE_CUSTOM_CONTEXT=1 to prevent linkage"));
      BXVD_DBG_WRN(pXvd, ("of the default BIMG context to reduce the binary size"));
      BXVD_DBG_WRN(pXvd, ("*******************"));
   }
#endif
#endif

   if ((pXvd->stSettings.pImgInterface == NULL) ||
       (pXvd->stSettings.pImgContext == NULL)) {
      BXVD_DBG_ERR(pXvd, ("*******************"));
      BXVD_DBG_ERR(pXvd, ("You've compiled with either BXVD_USE_CUSTOM_IMAGE=1 or BXVD_USE_CUSTOM_CONTEXT=1."));
      BXVD_DBG_ERR(pXvd, ("However, you have NOT provided your own version(s) of"));
      BXVD_DBG_ERR(pXvd, ("the BIMG interface and context to BXVD_Open()."));
      BXVD_DBG_ERR(pXvd, ("If you want to use the default BIMG, use BXVD_USE_CUSTOM_IMAGE=0 or BXVD_USE_CUSTOM_CONTEXT=0"));
      BXVD_DBG_ERR(pXvd, ("Otherwise, you MUST provide your own implementation of BIMG."));
      BXVD_DBG_ERR(pXvd, ("*******************"));
   }

   /* Set pointer back to parent structure */
   pXvd->stDecoderContext.hXvd = pXvd;

   /* Allocate the channel handle array */
   pXvd->ahChannel = (BXVD_ChannelHandle*)(BKNI_Malloc(sizeof(BXVD_ChannelHandle) * BXVD_MAX_VIDEO_CHANNELS));

   if (!pXvd->ahChannel)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   BKNI_Memset((void*)pXvd->ahChannel, 0x0, (sizeof(BXVD_ChannelHandle) * BXVD_MAX_VIDEO_CHANNELS));

   pXvd->hStillChannel = (BXVD_P_Channel*)(BKNI_Malloc(sizeof(BXVD_P_Channel)));

   if (!pXvd->hStillChannel)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* Allocate the multi-decode picture list */
   pXvd->pVDCPictureBuffers = (BAVC_XVD_Picture*)(BKNI_Malloc(sizeof(BAVC_XVD_Picture) * BXVD_MAX_VIDEO_CHANNELS));

   if (!pXvd->pVDCPictureBuffers)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   BKNI_Memset((void*)pXvd->pVDCPictureBuffers, 0x0, (sizeof(BAVC_XVD_Picture) * BXVD_MAX_VIDEO_CHANNELS));

   rc = BERR_TRACE(BKNI_CreateEvent(&(pXvd->stDecoderContext.hFWCmdDoneEvent)));
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(rc);
   }

   /*
    * Initialize stripe width and bank height prior to calling
    * platform specific register initialization and AVD core
    * boot. These are currently set to hardcoded values for all
    * platforms except the 7440 and 7405. These two platforms will
    * set the values in their specific register initialization code.
    */
   pXvd->uiDecode_StripeWidth = BXVD_P_AVD_INIT_STRIPE_WIDTH;
   pXvd->uiDecode_StripeMultiple = BXVD_P_AVD_INIT_STRIPE_MULTIPLE;

   /*
    * Initialize Chip Product Revision register address to be passed to FW.
    */
   pXvd->uiChip_ProductRevision = BXVD_P_CHIP_PRODUCT_REVISION;

#if BXVD_P_POWER_MANAGEMENT

   /* Use bchp power method if resource is defined */
#if BCHP_PWR_RESOURCE_AVD0

   /* Assume power (clocks) is off */
   pXvd->bHibernate = true;
   pXvd->PowerStateCurrent = BXVD_P_PowerState_ePwrOff;

   /* Wake up decoder */
   BXVD_P_SetHibernateState(pXvd, false);

#endif
#endif

   BXVD_P_INIT_REG_PTRS(pXvd);

   BXVD_P_GET_MEMORY_CONFIG(pXvd, eMemCfgMode);

   BXVD_P_ValidateHeaps(pXvd, eMemCfgMode);

#if BXVD_P_USE_XVD_PM1
   {
      /* Use legacy power management method, need reg pointers setup to access clock registers */
      bool bHibernateState;
      /* XVD should not be in hibernation state, verify the clocks are enabled */

      BXVD_P_GetHibernateState(pXvd, &bHibernateState);

      if (bHibernateState == true)
      {
         BXVD_DBG_WRN(pXvd, ("XVD clocks NOT enabled, clocks being enabled"));

         pXvd->PowerStateCurrent = BXVD_P_PowerState_eClkOff;
         /* Wake up decoder */
         BXVD_P_SetHibernateState(pXvd, false);
      }
      else
      {
         /* Clocks are enabled */
         pXvd->PowerStateCurrent = BXVD_P_PowerState_eOn;
      }
   }
#endif

   /* Validate the uiDecode_StripeWidth and uiDecode_StripeMultiple
    * values that may have been overridden by the platform's init reg
    * ptr code */
   if (pXvd->uiDecode_StripeWidth >= BXVD_P_STRIPE_WIDTH_NUM)
   {
      BXVD_DBG_ERR(pXvd, ("Unsupported stripe width: %d", pXvd->uiDecode_StripeWidth));
      BXVD_Close(pXvd);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   if (pXvd->uiDecode_StripeMultiple >= BXVD_P_STRIPE_MULTIPLE_NUM)
   {
      BXVD_DBG_ERR(pXvd, ("Unsupported stripe multiple: %d", pXvd->uiDecode_StripeMultiple));
      BXVD_Close(pXvd);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

#if BDBG_DEBUG_BUILD
   /* Check to see if AVD has access to the GISB */
   uiGISBMask = BREG_Read32(pXvd->hReg,
                            pXvd->stPlatformInfo.stReg.uiSunGisbArb_ReqMask);

   if (uiGISBMask & pXvd->stPlatformInfo.stReg.uiSunGisbArb_ReqMaskAVDMask)
   {
      BXVD_DBG_ERR(pXvd, ("***ERROR*** AVD access to GISB DISABLE! GISB Arbiter Request Mask: %08x", uiGISBMask));
   }
#endif

   /* Initialize memory */
   rc = BXVD_P_SETUP_FW_MEMORY(pXvd);
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(rc);
   }

   rc = BXVD_P_Boot(pXvd);
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(rc);
   }

   /* Only on HVD revision N and later cores */
#if BXVD_P_CORE_REVISION_NUM >= 14
   /* If FirmareBlock passed to BXVD_Open, then block must be unlocked so secure processor can now own the memory */
   if ((pXvd->stSettings.uiFirmwareBlockSize != 0) && (pXvd->stSettings.hFirmwareBlock != 0 ))
   {
      BMMA_Unlock(pXvd->hFWMemBlock, (void *)(pXvd->uiFWMemBaseVirtAddr - pXvd->stSettings.uiFirmwareBlockOffset));
      BMMA_UnlockOffset(pXvd->hFWMemBlock, (pXvd->FWMemBasePhyAddr - pXvd->stSettings.uiFirmwareBlockOffset));

      pXvd->uiFWMemBaseVirtAddr = 0;
      pXvd->FWMemBasePhyAddr = 0;
   }
#endif

   rc = BXVD_P_SetupFWSubHeap(pXvd);
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(rc);
   }

   rc = BXVD_Status_Open(pXvd, &pXvd->hXvdStatus);
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(pXvd);
      return BERR_TRACE(rc);
   }

   if (pXvd->stSettings.pAVDBootCallback == NULL)
   {
      rc = BXVD_P_OpenPartTwo(pXvd);

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   /* Give the user the new context */
   *phXvd = (BXVD_Handle)pXvd;

   BDBG_LEAVE(BXVD_Open);
   return BERR_TRACE(BERR_SUCCESS);
}

/******************************************************************************
BXVD_Close: Release allocated resources and close XVD
******************************************************************************/
BERR_Code BXVD_Close(BXVD_Handle hXvd)
{
   BERR_Code eStatus = BERR_SUCCESS;

   BDBG_ENTER(BXVD_Close);

   BDBG_ASSERT(hXvd);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BXVD_DBG_MSG(hXvd, ("BXVD_Close() - hXvd = 0x%0*lx, Decoder: %d", BXVD_P_DIGITS_IN_LONG, (long)hXvd, hXvd->uDecoderInstance));

#if BXVD_P_POWER_MANAGEMENT
   /* Wake up the decoder */
   BXVD_P_SetHibernateState(hXvd, false);
#endif

   /* We must teardown the still picture channel before we destroy the
    * callbacks because we need to get the response back from the
    * decoder when closing the channel */
   eStatus = BERR_TRACE(BXVD_P_TeardownStillPictureCompatibilityMode(hXvd));
   if (BERR_SUCCESS != eStatus)
   {
      BXVD_P_FreeXVDContext(hXvd);
      return BERR_TRACE(eStatus);
   }

   eStatus = BERR_TRACE(BXVD_P_DestroyInterrupts(hXvd));
   if (BERR_SUCCESS != eStatus)
   {
      BXVD_P_Reset740x(hXvd, (uint32_t)0);

      BXVD_P_FreeXVDContext(hXvd);
      return BERR_TRACE(eStatus);
   }

   BXVD_P_Reset740x(hXvd, (uint32_t)0);

   eStatus = BERR_TRACE(BXVD_P_TeardownFWSubHeap(hXvd));
   if (BERR_SUCCESS != eStatus)
   {
      BXVD_P_FreeXVDContext(hXvd);
      return BERR_TRACE(eStatus);
   }

   eStatus = BERR_TRACE(BXVD_P_TEAR_DOWN_FW_MEMORY(hXvd));
   if (BERR_SUCCESS != eStatus)
   {
      BXVD_P_FreeXVDContext(hXvd);

      return BERR_TRACE(eStatus);
   }

   if (hXvd->hXvdStatus)
   {
      eStatus = BXVD_Status_Close(hXvd->hXvdStatus);
      if(BERR_SUCCESS != eStatus)
      {
         BXVD_P_FreeXVDContext(hXvd);
         return BERR_TRACE(eStatus);
      }
   }

#if BXVD_P_POWER_MANAGEMENT

   /* Put the decoder to sleep  */
   BXVD_P_SET_POWER_STATE(hXvd, BXVD_P_PowerState_ePwrOff);

#endif

   BDBG_UNREGISTER_INSTANCE(hXvd);

   BXVD_P_FreeXVDContext(hXvd);

   /* Null the handle */
   hXvd = NULL;

   BDBG_LEAVE(BXVD_Close);
   return BERR_TRACE(eStatus);
}


/**************************************************************************
 Summary:
    API determines the size of the decoder firmware.
 Description:
    Determine decoder firware size for the decoder instance specified.

 Parameters:
   BREG_Handle    hRegister,             register handle
   uint32_t       uiDecoderInstance,     Decoder instance: 0, 1, 2
   int32_t        *DecoderFirmwareSize   pointer to decoder firware size being determined.

 Returns:
        BERR_SUCCESS  Decoder Firmware size determined successfully.
        BERR_INVALID_PARAMETER  Bad input parameter
**************************************************************************/

BERR_Code BXVD_GetDecoderFirmwareSize( BREG_Handle  hRegister,
                                       uint32_t     uiDecoderInstance,
                                       uint32_t     *uiDecoderFirmwareSize)

{
   BDBG_ENTER(BXVD_GetDecoderFirmwareSize);

   BDBG_ASSERT(hRegister);
   BSTD_UNUSED(hRegister);
   BSTD_UNUSED(uiDecoderInstance);

   *uiDecoderFirmwareSize = BXVD_P_FW_IMAGE_SIZE + BXVD_P_FW_IMAGE_SIGN_SIZE;

   BDBG_MSG(("BXVD_GetDecoderFirmwareSize: 0x%08x", *uiDecoderFirmwareSize));

   return (BERR_SUCCESS);
}


/**************************************************************************
 Summary: BXVD_GetHardwareCapabilties
    Returns the decoder HW video protocol capabilities

 Description:
    Using the specified video decoder handle, determine the supported
    video compression standards.

 Returns:
    BERR_SUCCESS:  Hardware capabilities determined successfully.
    BXVD_ERR_INVALID_HANDLE: BXVD_Handle not valid.
**************************************************************************/

BERR_Code BXVD_GetHardwareCapabilities(BXVD_Handle hXvd,
                                       BXVD_HardwareCapabilities *pCap)
{
   BAVC_VideoCompressionStd  eVideoCmprStd;

   BDBG_ENTER(BXVD_GetHardwareCapabilities);

   BDBG_ASSERT(hXvd);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Initialize compression standard capabilities array */

   BKNI_Memset((void*)(pCap->stCodecCapabilities), 0x0, (sizeof(BXVD_VidComprStd_Capabilities) * BAVC_VideoCompressionStd_eMax));

   for (eVideoCmprStd = BAVC_VideoCompressionStd_eH264; eVideoCmprStd < BAVC_VideoCompressionStd_eMax; eVideoCmprStd++)
   {
      pCap->bCodecSupported[eVideoCmprStd] = BXVD_P_IsDecodeProtocolSupported(hXvd, eVideoCmprStd);

      if (pCap->bCodecSupported[eVideoCmprStd] == true)
      {
         BXVD_P_GetVidCmprCapability(&(pCap->stCodecCapabilities[eVideoCmprStd]), eVideoCmprStd);
      }
   }

   if (BXVD_P_STC_MAX < BXVD_STC_eMax)
   {
      pCap->uiSTC_Count = BXVD_P_STC_MAX;
   }
   else
   {
      pCap->uiSTC_Count = (uint32_t)BXVD_STC_eMax;
   }

   if ( BXVD_P_CORE_REVISION_NUM >= 19 )
   {
      pCap->bIncludeRepeatedItbStartCodes = true;
   }
   else
   {
      pCap->bIncludeRepeatedItbStartCodes = false;
   }

   BDBG_LEAVE(BXVD_GetHardwareCapabilities);
   return BERR_TRACE(BERR_SUCCESS);
}


/******************************************************************************
BXVD_GetDefaultSettings: Get the default settings for the XVD device.
******************************************************************************/
BERR_Code BXVD_GetDefaultSettings(BXVD_Settings *pDefaultSettings)
{
   BDBG_ENTER(BXVD_GetDefaultSettings);

   BDBG_ASSERT(pDefaultSettings);

   BKNI_Memcpy((void*)pDefaultSettings,
               (void *)&s_stBXVD_DefaultSettings,
               sizeof(BXVD_Settings));

#if !(BXVD_USE_CUSTOM_IMAGE)
   pDefaultSettings->pImgInterface = &BXVD_IMAGE_Interface;
#if !(BXVD_USE_CUSTOM_CONTEXT)
   pDefaultSettings->pImgContext = (void *) BXVD_IMAGE_Context;
#endif
#endif

   BDBG_LEAVE(BXVD_GetDefaultSettings);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_GetChannelDefaultSettings: Gets the default settings of the desired XVD
                                channel.
****************************************************************************/
BERR_Code BXVD_GetChannelDefaultSettings(BXVD_Handle          hXvd,
                                         unsigned int         uiChannelNum,
                                         BXVD_ChannelSettings *pChnDefSettings)
{
   BDBG_ENTER(BXVD_GetChannelDefaultSettings);

   BSTD_UNUSED(uiChannelNum);
   BDBG_ASSERT(pChnDefSettings);

   if (hXvd != NULL)
   {
      /* Check handle type for correctness */
      if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
      {
         BDBG_ERR(("Invalid handle type passed to function"));
         return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
      }
   }

   BKNI_Memcpy((void *)pChnDefSettings,
               (void *)&s_stDefaultChannelSettings,
               sizeof(BXVD_ChannelSettings));

   /* coverity[read_parm_fld:FALSE] */
   BXVD_DBG_MSG(hXvd, ("pChnDefSettings->ui32MonitorRefreshRate = 0x%x (%d)",
                            pChnDefSettings->ui32MonitorRefreshRate,
                            pChnDefSettings->ui32MonitorRefreshRate));

   BDBG_LEAVE(BXVD_GetChannelDefaultSettings);

   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_GetDecodeDefaultSettings:  Gets the default settings of the desired
                                Decoder
****************************************************************************/
static BERR_Code BXVD_S_GetDecodeDefaultSettings
(
   BXVD_ChannelHandle   hXvdCh,              /* [in] XVD channel handle */
   BXVD_DecodeSettings  *pDecodeDefSettings, /* [out] default channel settings */
   bool                 bCompatibilityMode   /* [in] support backwards compatibility */
)
{
   uint32_t i;
   BDBG_ENTER(BXVD_S_GetDecodeDefaultSettings);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pDecodeDefSettings);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* SWSTB-3450: get rid of s_stDefaultDecodeSettings to make it easier to
    * add/initialize elements in the stXDMSettings sub structure.
    *
    * There was code  BXVD_StartDecode to initialize the default BXVD_DecodeSettings
    * structure if BXVD_GetDecodeDefaultSettings had not been called.
    * This code initialized a limited number of elements in the default BXVD_DecodeSettings.
    * To maintain backwards compatibility, only initialize this limited number of elements
    * when this routine is called from BXVD_StartDecode.  This compatibility mode is
    * required in at least one instance, the code in BXVD_DecodeStillPicture does NOT call
    * BXVD_GetDecodeDefaultSettings. BXVD_DecodeStillPicture could be updated to use
    * BXVD_GetDecodeDefaultSettings, but there may still be legacy customer applications
    * that don't call BXVD_GetDecodeDefaultSettings. */

   if ( false == bCompatibilityMode )
   {
      BKNI_Memset( (void *)pDecodeDefSettings, 0, sizeof(BXVD_DecodeSettings) );

      /* Set the default XVD default values. */
      pDecodeDefSettings->eVideoCmprStd = BAVC_VideoCompressionStd_eMPEG2;    /* Compression Standard */
      pDecodeDefSettings->ulMultiStreamId = 0;                                /* Deprecated, MultiStreamId */
      pDecodeDefSettings->ulVideoSubStreamId = 0;                             /* Deprecated, VideoSubStreamId */
      pDecodeDefSettings->bPlayback = false;                                  /* Playback boolean */
      pDecodeDefSettings->bCrcMode = false;                                   /* CRCMode boolean */
      pDecodeDefSettings->stDataXprtOutput.eXptOutputId = 0;                  /* Deprecated, BAVC_XptOutput */
      pDecodeDefSettings->stDataXprtOutput.eXptSourceId = 0;
      pDecodeDefSettings->eTimeBase = 0;                                      /* Deprecated, TimeBase */
      pDecodeDefSettings->pContextMap = NULL;                                 /* Xpt ContextMap, must be specified */
      for ( i=0; i < BXVD_NUM_EXT_RAVE_CONTEXT; i++ )                         /* Xpt ContextMap Extended, Optional */
         pDecodeDefSettings->aContextMapExtended[i] = NULL;

      pDecodeDefSettings->uiContextMapExtNum = 0;                             /* Xpt ContextMap Extended Number, Optional */
      pDecodeDefSettings->eSTC = BXVD_STC_eZero;                              /* STC number, is STC0 for instance 0 and STC1 for instance 1 */;                            /* STC used by this channel */

      pDecodeDefSettings->uiVDCRectangleNum = BXVD_DECODE_MODE_RECT_NUM_INVALID;     /* Rectangle Number, use channel number if invalid */

      pDecodeDefSettings->eHITSMode = BXVD_HITSMode_eDisabled;                /* Disable MPEG HITS Mode */
      pDecodeDefSettings->bZeroDelayOutputMode = false;                       /* Disable H264 Zero Delay Output Mode */
      pDecodeDefSettings->eDefaultFrameRate = BAVC_FrameRateCode_eUnknown;    /* FrameRate */


      pDecodeDefSettings->bBluRayDecode = false;                              /* Disable Blu-ray decode mode */

      pDecodeDefSettings->eProgressiveOverrideMode = BXVD_ProgressiveOverrideMode_eTopBottom; /* TopBottom 480p/576p/1080p progressive override */

      pDecodeDefSettings->eTimestampMode = BXVD_TimestampMode_eDecode;        /* timestamps are expected in display order */
      pDecodeDefSettings->bIFrameAsRAP = false;                               /* Don't treat I Frame as RAP for AVC decode */
      pDecodeDefSettings->bAVCErrorConcealmentMode = false;                   /* AVC error concealment disabled by default */            /* Enable AVC error concealment */
      pDecodeDefSettings->bIOnlyFieldOutputMode = false;                      /* AVC I only field output mode, disabled by default */
      pDecodeDefSettings->bIgnoreDPBOutputDelaySyntax = false;                /* SW3556-1058:: conditionally ignore the DPB output delay syntax */
      pDecodeDefSettings->uiSEIMessageFlags = 0;                              /* SEI Message Flags */
      pDecodeDefSettings->bPFrameSkipDisable = false;                         /* P frame skip mode enabled */

      pDecodeDefSettings->bExternalPictureProviderMode = false;               /* External picture provider interface NOT in use */
      pDecodeDefSettings->bAVCAspectRatioOverrideMode = false;                /* Disable AVC Aspect Ratio Override mode */
      pDecodeDefSettings->bSVC3DModeEnable = false;                           /* Disable 3D decode for SVC protocol */
      pDecodeDefSettings->bSWCoefAVCDecodeModeEnable = false;                 /* Disable SW Coefficient decode of AVC streams */
      pDecodeDefSettings->bIgnoreNumReorderFramesEqZero = false;              /* Ignore AVC Num Reorder Frames equal zero */

      pDecodeDefSettings->bEarlyPictureDeliveryMode = false;                  /* Disable Early Picture Delivery Mode */
      pDecodeDefSettings->pstEnhancedSettings = NULL;                         /* SW7425-1064: with linked channels, BXVD_StartDecode will be called */
      pDecodeDefSettings->bUserDataBTPModeEnable = false;                     /* Disable Userdata in BTP mode */
      pDecodeDefSettings->bNRTModeEnable = false;                             /* Disable NRT decode mode */

      pDecodeDefSettings->ePCRDiscontinuityModeAtStartup = BXVD_PCRDiscontinuityMode_eDrop;  /* SWSTB-68: ePCRDiscontinuityModeAtStartup default to drop */

   }/* end of if ( false == bCompatibilityMode )*/

   /* When this routine is called from BXVD_StartDecode, only initialize the
    * following settings. */

   pDecodeDefSettings->eDisplayInterrupt = BXVD_DisplayInterrupt_eZero;    /* FW PictDataRdy Interrupt to be used by this channel */
   pDecodeDefSettings->bAstmMode = false;                                  /* Disable DM ASTM mode */
   pDecodeDefSettings->bVsyncModeOnPcrDiscontinuity = true;                /* Disable DM bVsyncModeOnPcrDiscontinuity */
   pDecodeDefSettings->uiPreRollRate = 0;                                  /* preroll rate of '0' */
   pDecodeDefSettings->uiPreRollNumerator = 1;                             /* SW7445-2421: uiPreRollNumerator */
   pDecodeDefSettings->eErrorHandling = BXVD_ErrorHandling_eOff;           /* picture error handling is off */
   pDecodeDefSettings->uiErrorThreshold = 0;                               /* SWSTB-439: default error level is 0 */
   pDecodeDefSettings->eFrameRateDetectionMode = BXVD_FrameRateDetectionMode_eOff;  /* Frame Rate Detection (FRD) is off */
   pDecodeDefSettings->uiSignature = BXVD_DECODESETTINGS_SIGNATURE;        /* indicates BXVD_S_GetDecodeDefaultSettings has been called */

   /* SWSTB-3450: get the default XDM settings. */
   BXDM_PictureProvider_GetDefaultStartSettings_isrsafe( &(pDecodeDefSettings->stXDMSettings) );

   /* AVD0 should use STC0 and AVD1 should use STC1 to maintain
    * backwards compatibility with pre-mosaic firmware */
   if (hXvdCh->pXvd->uDecoderInstance == 0)
   {
      pDecodeDefSettings->eSTC = BXVD_STC_eZero;
   }
   else
   {
      pDecodeDefSettings->eSTC = BXVD_STC_eOne;
   }

   /* VDC Rectangle number defaults to channel number */
   pDecodeDefSettings->uiVDCRectangleNum = hXvdCh->ulChannelNum;

   BDBG_LEAVE(BXVD_S_GetDecodeDefaultSettings);
   return BERR_TRACE(BERR_SUCCESS);

}


/* SWSTB-3450: A wrapper around BXVD_S_GetDecodeDefaultSettings to support
 * backwards compatibility.  See BXVD_S_GetDecodeDefaultSettings for a more
 * detailed explanation. */

BERR_Code BXVD_GetDecodeDefaultSettings
(
   BXVD_ChannelHandle   hXvdCh,             /* [in] XVD channel handle */
   BXVD_DecodeSettings  *pDecodeDefSettings /* [out] default channel settings */
   )
{
   BDBG_ENTER(BXVD_GetDecodeDefaultSettings);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pDecodeDefSettings);

   BXVD_S_GetDecodeDefaultSettings( hXvdCh, pDecodeDefSettings, false  );

   BDBG_LEAVE(BXVD_GetDecodeDefaultSettings);

   return BERR_TRACE(BERR_SUCCESS);
}

/**************************************************************************
 Summary:
    API initializes video decoder firmware on secure systems
 Description:
    Firwamre init command sent to video decoder firmware.

 Parameters:
    BXVD_Handle  hXvd

 Returns:
    BERR_SUCCESS            Decoder Firmware initialized successfully.
    BERR_INVALID_PARAMETER  Bad input parameter
    BERR_TIMEOUT            Firmware startup or initialization timed-out
**************************************************************************/

BERR_Code BXVD_InitSecureFirmware(BXVD_Handle hXvd)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_InitSecureFirmware);

   BDBG_ASSERT(hXvd);

   BXVD_DBG_MSG(hXvd, ("BXVD_InitSecureFirmware() - hXvd = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd));

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXVD_P_InitDecoderFW(hXvd);
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(hXvd);
      return BERR_TRACE(rc);
   }

   rc = BXVD_P_OpenPartTwo(hXvd);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   return rc;
}

#if BXVD_P_POWER_MANAGEMENT

BERR_Code BXVD_Standby(BXVD_Handle hXvd)
{
   BXVD_ChannelHandle  hXvdCh;
   uint32_t            chanNum;

   BERR_Code rc = BERR_SUCCESS;

   BXVD_DBG_MSG(hXvd, ("BXVD_Standby decoder:%d", hXvd->uDecoderInstance));

   for ( chanNum = 0; chanNum < BXVD_MAX_VIDEO_CHANNELS; chanNum++)
   {
      hXvdCh = hXvd->ahChannel[chanNum];

      if (hXvdCh != NULL)
      {
         if (hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive)
         {
            BXVD_DBG_WRN(hXvdCh, ("BXVD_Standy() - Channel active: %d", chanNum));

            rc = BXVD_ERR_DECODER_ACTIVE;
            break;
         }
      }
   }

   for (chanNum = 0; chanNum < BXVD_MAX_VIDEO_CHANNELS; chanNum++)
   {
      hXvdCh = hXvd->ahChannel[chanNum];
      /*
       * Scram the timer created in BXVD_Open
       */
      if (NULL != hXvdCh && NULL != hXvd->hTimer)
      {
         BXVD_DBG_MSG(hXvdCh, ("nulling timer handle for channel %d", chanNum));
         rc = BXDM_PictureProvider_SetTimerHandle_isr(hXvdCh->hPictureProvider, NULL);
         if (rc != BERR_SUCCESS)
         {
            BXVD_DBG_WRN(hXvdCh, ("BXDM_PictureProvider_SetTimerHandle_isr failed for channel %d", chanNum));
            return BERR_TRACE(rc);
         }
      }
   }

   if (NULL != hXvd->hTimer)
   {
      BXVD_DBG_MSG(hXvd, ("destroying timer"));
      BTMR_DestroyTimer(hXvd->hTimer);
      hXvd->hTimer = NULL;
   }

   hXvd->PowerStateSaved = hXvd->PowerStateCurrent;

   /* If clocks are off, turn them on to reset decoder */
   if (hXvd->PowerStateCurrent == BXVD_P_PowerState_eClkOff)
   {
      BXVD_P_SET_POWER_STATE(hXvd, BXVD_P_PowerState_eOn);
   }

   /* Halt ARCs and reset decoder */
   rc = BERR_TRACE(BXVD_P_Reset740x(hXvd, hXvd->uDecoderInstance));

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BXVD_P_SET_POWER_STATE(hXvd, BXVD_P_PowerState_ePwrOff);

   return rc;
}

BERR_Code BXVD_Resume(BXVD_Handle hXvd)
{
   BERR_Code           rc;
   uint32_t            chanNum;
   BTMR_Settings       tmrSettings;
   BXVD_ChannelHandle  hXvdCh;

   BXVD_DBG_MSG(hXvd, ("BXVD_Resume decoder:%d", hXvd->uDecoderInstance));

   BXVD_P_SET_POWER_STATE(hXvd, BXVD_P_PowerState_eOn);

   hXvd->bWatchdogPending = true;

   hXvd->eAVDBootMode = BXVD_AVDBootMode_eWatchdog;

   rc = BXVD_ProcessWatchdog(hXvd);

   if (hXvd->stSettings.pAVDBootCallback == NULL)
   {
      if (hXvd->PowerStateSaved != BXVD_P_PowerState_eOn)
      {
         BXVD_DBG_MSG(hXvd, ("Decoder:%d, Set saved_Power_State:%d", hXvd->uDecoderInstance, hXvd->PowerStateSaved));

         BXVD_P_SET_POWER_STATE(hXvd, hXvd->PowerStateSaved);
      }

      /*
       * Restart timer
       */
      if (NULL != hXvd->stSettings.hTimerDev)
      {
         BXVD_DBG_MSG(hXvd, ("creating timer in BXVD_Resume"));
         hXvd->hTimer = NULL;
         rc = BTMR_GetDefaultTimerSettings(&tmrSettings);

         if (BERR_SUCCESS == rc)
         {
            tmrSettings.type = BTMR_Type_eSharedFreeRun;
            tmrSettings.exclusive = false;

            rc = BTMR_CreateTimer(hXvd->stSettings.hTimerDev,
                                  &hXvd->hTimer,
                                  &tmrSettings);

            if (BERR_SUCCESS != rc)
            {
               hXvd->hTimer = NULL;
               BXVD_DBG_WRN(hXvd, ("Error creating timer"));
            }

            if (NULL != hXvd->hTimer)
            {
               for (chanNum = 0; chanNum < BXVD_MAX_VIDEO_CHANNELS; chanNum++)
               {
                  hXvdCh = hXvd->ahChannel[chanNum];
                  if (NULL != hXvdCh)
                  {
                     BXVD_DBG_MSG(hXvdCh, ("setting timer handle for %d", chanNum));
                     rc = BXDM_PictureProvider_SetTimerHandle_isr(hXvdCh->hPictureProvider, hXvd->hTimer);
                     if (rc != BERR_SUCCESS)
                     {
                        BXVD_DBG_WRN(hXvdCh, ("Error setting picture provider timer handle for channel %d", chanNum));
                     }
                  }
               }
            }
         }
      }
   }

   return rc;
}

BERR_Code BXVD_ResumeRestartDecoder(BXVD_Handle hXvd)
{
   BERR_Code           rc;
   uint32_t            chanNum;
   BTMR_Settings       tmrSettings;
   BXVD_ChannelHandle  hXvdCh;

   BXVD_DBG_MSG(hXvd, ("BXVD_ResumeRestartDecoder:%d", hXvd->uDecoderInstance));

   rc = BXVD_P_InitDecoderFW(hXvd);
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(hXvd);
      return BERR_TRACE(rc);
   }

   rc = BXVD_P_RestartDecoder(hXvd);
   if(rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   if (hXvd->PowerStateSaved != BXVD_P_PowerState_eOn)
   {
      BXVD_DBG_MSG(hXvd, ("Decoder:%d, Set saved_Power_State:%d", hXvd->uDecoderInstance, hXvd->PowerStateSaved));

      BXVD_P_SET_POWER_STATE(hXvd, hXvd->PowerStateSaved);
   }

   /*
    * Restart timer
    */
   if (NULL != hXvd->stSettings.hTimerDev)
   {
      BXVD_DBG_MSG(hXvd, ("creating timer in BXVD_ResumeResterDecoder"));
      hXvd->hTimer = NULL;
      rc = BTMR_GetDefaultTimerSettings(&tmrSettings);

      tmrSettings.type = BTMR_Type_eSharedFreeRun;
      tmrSettings.exclusive = false;

      if (BERR_SUCCESS == rc)
      {
         rc = BTMR_CreateTimer(hXvd->stSettings.hTimerDev,
                               &hXvd->hTimer,
                               &tmrSettings);

         if (BERR_SUCCESS != rc)
         {
            hXvd->hTimer = NULL;
            BXVD_DBG_WRN(hXvd, ("Error creating timer"));
         }

         if (NULL != hXvd->hTimer)
         {
            for (chanNum = 0; chanNum < BXVD_MAX_VIDEO_CHANNELS; chanNum++)
            {
               hXvdCh = hXvd->ahChannel[chanNum];
               if (NULL != hXvdCh)
               {
                  BXVD_DBG_MSG(hXvdCh, ("setting timer handle for %d", chanNum));
                  rc = BXDM_PictureProvider_SetTimerHandle_isr(hXvdCh->hPictureProvider, hXvd->hTimer);
                  if (rc != BERR_SUCCESS)
                  {
                     BXVD_DBG_WRN(hXvdCh, ("Error setting picture provider timer handle for channel %d", chanNum));
                  }
               }
            }
         }
      }
   }

   return rc;
}

#endif /* BXVD_P_POWER_MANAGEMENT */

BERR_Code BXVD_ProcessWatchdog(BXVD_Handle hXvd)
{
   BXVD_ChannelHandle  hXvdCh;
   uint32_t            chanNum;

   BERR_Code rc;

   BDBG_ASSERT(hXvd);

   BDBG_ENTER(BXVD_ProcessWatchdog);

#if BXVD_P_POWER_MANAGEMENT
   hXvd->eWatchdogSavedPowerState = hXvd->PowerStateCurrent;
#endif

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if (hXvd->bWatchdogPending == false)
   {
      return BERR_SUCCESS;
   }

#if BXVD_P_POWER_MANAGEMENT
   if (hXvd->PowerStateCurrent != BXVD_P_PowerState_eOn)
   {
      /* Wake up the decoder */
      BXVD_P_SET_POWER_STATE(hXvd, BXVD_P_PowerState_eOn);
   }
#endif

   for ( chanNum = 0; chanNum < BXVD_MAX_VIDEO_CHANNELS; chanNum++)
   {
      hXvdCh = hXvd->ahChannel[chanNum];

      if (hXvdCh != NULL)
      {
         /* Only reset DM on Video channels, not Still Picture channel */
         if (hXvdCh->sChSettings.eChannelMode == BXVD_ChannelMode_eVideo)
         {
            BKNI_EnterCriticalSection();
            /* Reset Display manager state */
            BXDM_PictureProvider_WatchdogReset_isr(hXvdCh->hPictureProvider);
            BXVD_Decoder_WatchdogReset_isr( hXvdCh );
            BKNI_LeaveCriticalSection();
         }
      }
   }

   /* Only on HVD revision N and later cores */
#if BXVD_P_CORE_REVISION_NUM >= 14
   /* If FirmareBlock passed to BXVD_Open, then block must be locked again */
   if ((hXvd->stSettings.uiFirmwareBlockSize != 0) && (hXvd->stSettings.hFirmwareBlock != 0 ))
   {
      hXvd->uiFWMemBaseVirtAddr = (unsigned long) BMMA_Lock(hXvd->hFWMemBlock) + hXvd->stSettings.uiFirmwareBlockOffset;
      hXvd->FWMemBasePhyAddr = (BXVD_P_PHY_ADDR) BMMA_LockOffset(hXvd->hFWMemBlock) + hXvd->stSettings.uiFirmwareBlockOffset;
   }
#endif

   rc = BXVD_P_Boot(hXvd);
   if(rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Only on HVD revision N and later cores */
#if BXVD_P_CORE_REVISION_NUM >= 14
   /* If FirmareBlock passed to BXVD_Open, then block must be unlocked so secure processor can now own the memory */
   if ((hXvd->stSettings.uiFirmwareBlockSize != 0) && (hXvd->stSettings.hFirmwareBlock != 0 ))
   {
      BMMA_Unlock(hXvd->hFWMemBlock, (void *)(hXvd->uiFWMemBaseVirtAddr - hXvd->stSettings.uiFirmwareBlockOffset));
      BMMA_UnlockOffset(hXvd->hFWMemBlock, (hXvd->FWMemBasePhyAddr - hXvd->stSettings.uiFirmwareBlockOffset));

      hXvd->uiFWMemBaseVirtAddr = 0;
      hXvd->FWMemBasePhyAddr = 0;
   }
#endif

   if (hXvd->stSettings.pAVDBootCallback == NULL)
   {
      rc = BXVD_P_RestartDecoder(hXvd);
      if(rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }

#if BXVD_P_POWER_MANAGEMENT
      if ( hXvd->eWatchdogSavedPowerState != BXVD_P_PowerState_eOn)
      {
         /* Restore saved power state */
         BXVD_P_SET_POWER_STATE(hXvd, hXvd->eWatchdogSavedPowerState);
      }
#endif

      hXvd->bWatchdogPending = false;
   }

   BDBG_LEAVE(BXVD_ProcessWatchdog);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_ProcessWatchdogRestartDecoder(BXVD_Handle hXvd)
{
   BERR_Code rc;

   rc = BXVD_P_InitDecoderFW(hXvd);
   if (rc != BERR_SUCCESS)
   {
      BXVD_Close(hXvd);
      return BERR_TRACE(rc);
   }

   rc = BXVD_P_RestartDecoder(hXvd);
   if(rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

#if BXVD_P_POWER_MANAGEMENT
   if (hXvd->eWatchdogSavedPowerState != BXVD_P_PowerState_eOn)
   {
      /* Restore saved power state */
      BXVD_P_SET_POWER_STATE(hXvd, hXvd->eWatchdogSavedPowerState);
   }
#endif

   hXvd->bWatchdogPending = false;

   return rc;
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BXVD_CallbackFunc BXVD_Callback(void *pParm1, int parm2)
{
   BSTD_UNUSED(pParm1);
   BSTD_UNUSED(parm2);

   return (void *)0;
}
#endif

#if BDBG_DEBUG_BUILD

#define BXVD_S_MAX_FRAMERATE ( BAVC_FrameRateCode_eMax )

static const char * const sFrameRateCodeNameLUT[BXVD_S_MAX_FRAMERATE] =
{
   "BAVC_FrameRateCode_eUnknown",
   "BAVC_FrameRateCode_e23_976",
   "BAVC_FrameRateCode_e24",
   "BAVC_FrameRateCode_e25",
   "BAVC_FrameRateCode_e29_97",
   "BAVC_FrameRateCode_e30",
   "BAVC_FrameRateCode_e50",
   "BAVC_FrameRateCode_e59_94",
   "BAVC_FrameRateCode_e60",
   "BAVC_FrameRateCode_e14_985",
   "BAVC_FrameRateCode_e7_493",
   "BAVC_FrameRateCode_e10",
   "BAVC_FrameRateCode_e15",
   "BAVC_FrameRateCode_e20",
   "BAVC_FrameRateCode_e12_5", /* SW7584-331: add support for BAVC_FrameRateCode_e12_5 */
   "BAVC_FrameRateCode_e100",
   "BAVC_FrameRateCode_e119_88",
   "BAVC_FrameRateCode_e120",
   "BAVC_FrameRateCode_e19_98", /* SWSTB-378: Add support for 19.98fps */
   "BAVC_FrameRateCode_e7_5", /* SWSTB-1401: */
   "BAVC_FrameRateCode_e12", /* SWSTB-1401: */
   "BAVC_FrameRateCode_e11_988", /* SWSTB-1401: */
   "BAVC_FrameRateCode_e9_99", /* SWSTB-1401: */
};

static const char * const sDecodeResolutionNameLUT[BXVD_DecodeResolution_eMaxModes] =
{
   "BXVD_DecodeResolution_eHD",
   "BXVD_DecodeResolution_eSD",
   "BXVD_DecodeResolution_eCIF",
   "BXVD_DecodeResolution_eQCIF",
   "BXVD_DecodeResolution_e4K"
};

#if 0
/* need to conditionally define this for compatibility with older version of avc.h */
#ifndef BAVC_VideoCompressionStd_eMax
#define BAVC_VideoCompressionStd_eMax 14
#endif
#endif

#define BXVD_S_MAX_VIDEO_PROTOCOL 23

static const char * const sVideoCompressionStdNameLUT[BXVD_S_MAX_VIDEO_PROTOCOL] =
{
   "BAVC_VideoCompressionStd_eH264",
   "BAVC_VideoCompressionStd_eMPEG2",
   "BAVC_VideoCompressionStd_eH261",
   "BAVC_VideoCompressionStd_eH263",
   "BAVC_VideoCompressionStd_eVC1",
   "BAVC_VideoCompressionStd_eMPEG1",
   "BAVC_VideoCompressionStd_eMPEG2DTV",
   "BAVC_VideoCompressionStd_eVC1SimpleMain",
   "BAVC_VideoCompressionStd_eMPEG4Part2",
   "BAVC_VideoCompressionStd_eAVS",
   "BAVC_VideoCompressionStd_eMPEG2_DSS_PES",
   "BAVC_VideoCompressionStd_eSVC",
   "BAVC_VideoCompressionStd_eSVC_BL",
   "BAVC_VideoCompressionStd_eMVC",
   "BAVC_VideoCompressionStd_eVP6",
   "BAVC_VideoCompressionStd_eVP7",
   "BAVC_VideoCompressionStd_eVP8",
   "BAVC_VideoCompressionStd_eRV9",
   "BAVC_VideoCompressionStd_eSPARK",
   "BAVC_VideoCompressionStd_eMOTION_JPEG",
   "BAVC_VideoCompressionStd_eH265",
   "BAVC_VideoCompressionStd_eVP9",
   "BAVC_VideoCompressionStd_eAVS2"
};

static const char * const sChannelModeNameLUT[BXVD_ChannelMode_eMax] =
{
   "BXVD_ChannelMode_eVideo",
   "BXVD_ChannelMode_eStill"
};

static const char * const s1080pScanModeNameLUT[BXVD_1080pScanMode_eMax] =
{
   "BXVD_1080pScanMode_eDefault",
   "BXVD_1080pScanMode_eAdvanced"
};

static const char * const sPictureDropModeNameLUT[BXVD_PictureDropMode_eMax] =
{
   "BXVD_PictureDropMode_eField",
   "BXVD_PictureDropMode_eFrame"
};
#endif

/***************************************************************************
BXVD_OpenChannel: API used to open a video channel.
****************************************************************************/
BERR_Code BXVD_OpenChannel(BXVD_Handle                hXvd,
                           BXVD_ChannelHandle         *phXvdCh,
                           unsigned int               uiChannelNum,
                           const BXVD_ChannelSettings *pChDefSettings)
{
   BERR_Code rc = BERR_SUCCESS;
   BXVD_P_Channel       *pXvdCh = NULL;
   bool bStillMode = false;

   BXDM_PictureProvider_Settings stPictureProviderSettings;

   BDBG_Level eDefaultDebugLevel;

   uint32_t uiProtocolNum;

   BDBG_ENTER(BXVD_OpenChannel);

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(phXvdCh);

   BDBG_ASSERT(uiChannelNum<BXVD_MAX_VIDEO_CHANNELS);
   BDBG_ASSERT(pChDefSettings);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Set channel handle to NULL in case the allocation fails */
   *phXvdCh = NULL;

   /* validate channel number */
   if (uiChannelNum >= BXVD_MAX_VIDEO_CHANNELS)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Check if channel is already open */
   if (hXvd->ahChannel[uiChannelNum] != NULL)
   {
      BDBG_ERR(("BXVD_OpenChannel() - Channel [%d] already open. Cannot open again!", uiChannelNum));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   if (pChDefSettings->uiVideoCmprCount == 0)
   {
      BDBG_ERR(("BXVD_OpenChannel() - Channel Settings uiVideoCmprCount equals 0"));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

#if BXVD_P_POWER_MANAGEMENT
   if ((hXvd->uiOpenChannels == 0) &&  (hXvd->bHibernate == true))
   {
      /* Wake up the decoder */
      BXVD_P_SetHibernateState(hXvd, false);
   }
#endif

   if (pChDefSettings->eChannelMode == BXVD_ChannelMode_eStill)
   {
      bStillMode = true;
   }

   if (bStillMode)
   {
      if (hXvd->bStillChannelAllocated)
      {
         BDBG_ERR(("BXVD_OpenChannel() - Still channel already exists [%d]. Cannot create more than one",
                   hXvd->uiStillChannelNum));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      pXvdCh = hXvd->hStillChannel;
   }

   else
   {
      BXVD_P_GetChannelHandle(hXvd, &pXvdCh);

      if (!pXvdCh)
      {
         return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      }
   }

   BDBG_REGISTER_INSTANCE(pXvdCh);
   rc = BDBG_GetLevel(&eDefaultDebugLevel);
   rc = BDBG_SetInstanceLevel(pXvdCh, eDefaultDebugLevel);
   rc = BDBG_SetInstanceName(pXvdCh, sChannelNameLUT[hXvd->uDecoderInstance][uiChannelNum]);

   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - hXvd = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvd));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - phXvdCh = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)phXvdCh));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - uiChannelNum = %d", uiChannelNum));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - pChDefSettings = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pChDefSettings));

   /* Zero out the newly allocated context */
   BKNI_Memset((void*)pXvdCh, BXVD_P_MEM_ZERO, sizeof(BXVD_P_Channel));

   /* Set the defaults */
   BKNI_Memcpy((void *)&pXvdCh->sChSettings,
               (void *)pChDefSettings,
               sizeof(BXVD_ChannelSettings));

   /* Assigm ptr to the video compression standards list */

   pXvdCh->sChSettings.peVideoCmprStdList = &pXvdCh->asVideoCmprStdList[0];

   /* Copy the list */
   /* coverity[deref_ptr_in_call: FALSE] */
   BKNI_Memcpy(pXvdCh->sChSettings.peVideoCmprStdList,
               pChDefSettings->peVideoCmprStdList,
               pChDefSettings->uiVideoCmprCount * sizeof(BAVC_VideoCompressionStd));


   pXvdCh->sChSettings.uiVideoCmprCount = pChDefSettings->uiVideoCmprCount;

   /* Setup default channel heaps.
    *
    * We want the heaps that are specified at BXVD_OpenChannel() to
    * follow the same fall-back as the heaps specified at BXVD_Open().
    * So, if hChannelGeneralHeap is specified, but hChannelCabacHeap
    * and/or hChannelPictureHeap are not, then we want them to both
    * default to the hChannelGeneralHeap and NOT to XVD's sub-heaps.
    *
    * This still allows full flexibilty for specifying a separate heap
    * for each buffer type (context, cabac, and picture), but the
    * defaults are in-line with what users would expect from
    * BXVD_Open().
    *
    * Typically, on UMA platforms, the application would only need to
    * specify hChannelGerneralHeap if it wanted all channel memory
    * allocations (context, cabac, and picture buffers) to come from
    * that heap instead of XVD's sub-heaps.  We don't require the app
    * to explicitly specify the hChannelCabacHeap and
    * hChannelPictureHeap heaps, but there's no harm in doing so,
    * either.
    *
    * On non-UMA platforms, the application should specify both the
    * hChannelGeneralHeap and hChannelPictureHeap in order for channel
    * specific heap memory allocation to work properly. */
   if (pXvdCh->sChSettings.hChannelCabacBlock == NULL)
   {
      pXvdCh->sChSettings.hChannelCabacBlock = pXvdCh->sChSettings.hChannelGeneralBlock;
      pXvdCh->sChSettings.uiChannelCabacBlockOffset = pXvdCh->sChSettings.uiChannelGeneralBlockOffset;
      pXvdCh->sChSettings.uiChannelCabacBlockSize = pXvdCh->sChSettings.uiChannelGeneralBlockSize;
   }

   if (pXvdCh->sChSettings.hChannelPictureBlock == NULL)
   {
      pXvdCh->sChSettings.hChannelPictureBlock = pXvdCh->sChSettings.hChannelGeneralBlock;
      pXvdCh->sChSettings.uiChannelPictureBlockOffset = pXvdCh->sChSettings.uiChannelGeneralBlockOffset;
      pXvdCh->sChSettings.uiChannelPictureBlockSize = pXvdCh->sChSettings.uiChannelGeneralBlockSize;
   }

   for (uiProtocolNum = 0; uiProtocolNum < pXvdCh->sChSettings.uiVideoCmprCount; uiProtocolNum++)
   {
      if (pXvdCh->sChSettings.peVideoCmprStdList[uiProtocolNum] == BAVC_VideoCompressionStd_eVC1SimpleMain)
      {
         BDBG_MSG(("BXVD_OpenChannel() - Changing BAVC_VideoCompressionStd_eVC1SimpleMain to BAVC_VideoCompressionStd_eVC1"));

         pXvdCh->sChSettings.peVideoCmprStdList[uiProtocolNum] = BAVC_VideoCompressionStd_eVC1;
      }
   }

#if BDBG_DEBUG_BUILD
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.ui32MonitorRefreshRate = %d", pXvdCh->sChSettings.ui32MonitorRefreshRate));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.ulRemovalDelay = %ld", pXvdCh->sChSettings.ulRemovalDelay));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiVsyncDiffThreshDefault = %d", pXvdCh->sChSettings.uiVsyncDiffThreshDefault));
   if (pXvdCh->sChSettings.eDefaultFrameRate < BXVD_S_MAX_FRAMERATE)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.eDefaultFrameRate = %s (%d)",
                                 sFrameRateCodeNameLUT[pXvdCh->sChSettings.eDefaultFrameRate],
                                 pXvdCh->sChSettings.eDefaultFrameRate));
   }
   else
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.eDefaultFrameRate = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sChSettings.eDefaultFrameRate));
   }

   if (pXvdCh->sChSettings.eDecodeResolution < BXVD_DecodeResolution_eMaxModes)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.eDecodeResolution = %s (%d)",
                                 sDecodeResolutionNameLUT[pXvdCh->sChSettings.eDecodeResolution],
                                 pXvdCh->sChSettings.eDecodeResolution));
   }
   else
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.eDecodeResolution = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sChSettings.eDecodeResolution));
   }
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiVideoCmprCount = %d", pXvdCh->sChSettings.uiVideoCmprCount));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.peVideoCmprStdList = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.peVideoCmprStdList));
   if (pXvdCh->sChSettings.peVideoCmprStdList)
   {
      for (uiProtocolNum = 0; uiProtocolNum < pXvdCh->sChSettings.uiVideoCmprCount; uiProtocolNum++)
      {
         if (pXvdCh->sChSettings.peVideoCmprStdList[uiProtocolNum] < BXVD_S_MAX_VIDEO_PROTOCOL)
         {
            BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.peVideoCmprStdList[%d] = %s (%d)",
                                       uiProtocolNum,
                                       sVideoCompressionStdNameLUT[pXvdCh->sChSettings.peVideoCmprStdList[uiProtocolNum]],
                                       pXvdCh->sChSettings.peVideoCmprStdList[uiProtocolNum]));
         }
         else
         {
            BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.peVideoCmprStdList[%d] = %s (%d)",
                                       uiProtocolNum,
                                       "Unknown/Invalid Value!",
                                       pXvdCh->sChSettings.peVideoCmprStdList[uiProtocolNum]));
         }
      }
   }

   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiVDCRectangleNum = %d",
                              pXvdCh->sChSettings.uiVDCRectangleNum));

   if (pXvdCh->sChSettings.eChannelMode < BXVD_ChannelMode_eMax)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.eChannelMode = %s (%d)",
                                 sChannelModeNameLUT[pXvdCh->sChSettings.eChannelMode],
                                 pXvdCh->sChSettings.eChannelMode));
   }
   else
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.eChannelMode = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sChSettings.eChannelMode));
   }

   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bMPEG2BTPEnable = %d", pXvdCh->sChSettings.bMPEG2BTPEnable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bAVC41Enable = %d", pXvdCh->sChSettings.bAVC41Enable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bAVC51Enable = %d", pXvdCh->sChSettings.bAVC51Enable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bBluRayEnable = %d", pXvdCh->sChSettings.bBluRayEnable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bExcessDirModeEnable = %d", pXvdCh->sChSettings.bExcessDirModeEnable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bSVC3DModeEnable = %d", pXvdCh->sChSettings.bSVC3DModeEnable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.b1920PortraitModeEnable = %d", pXvdCh->sChSettings.b1920PortraitModeEnable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.b1200HDEnable = %d", pXvdCh->sChSettings.b1200HDEnable));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bSplitPictureBuffersEnable = %d", pXvdCh->sChSettings.bSplitPictureBuffersEnable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.bb10BitBuffersEnable = %d", pXvdCh->sChSettings.b10BitBuffersEnable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiExtraPictureMemoryAtoms = %d", pXvdCh->sChSettings.uiExtraPictureMemoryAtoms));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.hChannelGeneralBlock = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.hChannelGeneralBlock));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelGeneralBlockOffset = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelGeneralBlockOffset));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelGeneralBlockSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelGeneralBlockSize));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.hChannelCabacBlock = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.hChannelCabacBlock));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelCabacBlockOffset = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelCabacBlockOffset));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelCabacBlockSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelCabacBlockSize));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.hChannelPictureBlock = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.hChannelPictureBlock));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelPictureBlockOffset = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelPictureBlockOffset));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelPictureBlockSize = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelPictureBlockSize));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.hChannelPictureBlock1= 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.hChannelPictureBlock1));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelPictureBlockOffset1= 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelPictureBlockOffset1));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.uiChannelPictureBlockSize1= 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sChSettings.uiChannelPictureBlockSize1));

   if (pXvdCh->sChSettings.e1080pScanMode < BXVD_1080pScanMode_eMax)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.e1080pScanMode = %s (%d)",
                                 s1080pScanModeNameLUT[pXvdCh->sChSettings.e1080pScanMode],
                                 pXvdCh->sChSettings.e1080pScanMode));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.e1080pScanMode = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sChSettings.e1080pScanMode));
   }

   if (pXvdCh->sChSettings.ePictureDropMode < BXVD_PictureDropMode_eMax)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.ePictureDropMode = %s (%d)",
                                 sPictureDropModeNameLUT[pXvdCh->sChSettings.ePictureDropMode],
                                 pXvdCh->sChSettings.ePictureDropMode));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_OpenChannel() - BXVD_ChannelSettings.ePictureDropMode = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sChSettings.ePictureDropMode));
   }

#endif

   /* Store handles in the context */
   pXvdCh->pXvd = hXvd;

   /* Save channel number */
   pXvdCh->ulChannelNum = uiChannelNum;

   /* Set handle type */
   pXvdCh->eHandleType = BXVD_P_HandleType_XvdChannel;

   BXDM_PictureProvider_GetDefaultSettings(&stPictureProviderSettings);

   /* SWSTB-1380: XDM needs the heap handle when creating a debug fifo. */
#ifdef BXVD_ENABLE_BDBG_FIFO
   stPictureProviderSettings.hHeap = pXvdCh->pXvd->hGeneralHeap;
#endif

   rc = BXDM_PictureProvider_Create(&pXvdCh->hPictureProvider, &stPictureProviderSettings);

   if (rc != BERR_SUCCESS)
   {
      if (!bStillMode)
      {
         /* Keep channel context to use again later */
         BXVD_P_KeepChannelHandle(hXvd, pXvdCh);
      }

      return BERR_TRACE(rc);
   }

   BKNI_EnterCriticalSection();

   /* SW7445-2757: to aid in debug, create a unique serial number
    * each time a channel is opened.
    * - bump the open channel call count for this decoder instance
    * - combine the channel count with the decoder instance
    */
   hXvd->uiChannelOpenCalls++;

   pXvdCh->uiSerialNumber = ( hXvd->uiChannelOpenCalls & 0xFFFFFF );
   pXvdCh->uiSerialNumber |= ( hXvd->uDecoderInstance & 0xFF << 24 );

   rc = BXVD_P_InitChannel(pXvdCh);

   BKNI_LeaveCriticalSection();

   /* Update the open channel count */
   hXvd->uiOpenChannels++;

   if (rc != BERR_SUCCESS)
   {
      BXVD_CloseChannel(pXvdCh);
      return BERR_TRACE(rc);
   }

   /* Allocate memory based on whether this is a still or video decode channel */
   if (bStillMode)
   {
      /* Still Decode Channel */
      rc = BXVD_P_GetStillDecodeFWMemSize(hXvd,
                                          pXvdCh->sChSettings.eDecodeResolution,
                                          pXvdCh->sChSettings.peVideoCmprStdList,
                                          pXvdCh->sChSettings.uiVideoCmprCount,
                                          &(pXvdCh->sChSettings),
                                          &(pXvdCh->stDecodeFWMemSize));


      if (rc != BERR_SUCCESS)
      {
         BXVD_CloseChannel(pXvdCh);
         return BERR_TRACE(rc);
      }
   }
   else
   {
      /* Video Decode Channel memory size */
      rc = BXVD_P_GetDecodeFWMemSize(hXvd,
                                     pXvdCh->sChSettings.eDecodeResolution,
                                     pXvdCh->sChSettings.peVideoCmprStdList,
                                     pXvdCh->sChSettings.uiVideoCmprCount,
                                     &(pXvdCh->sChSettings),
                                     &(pXvdCh->stDecodeFWMemSize));
      if (rc != BERR_SUCCESS)
      {
         BXVD_CloseChannel(pXvdCh);
         return BERR_TRACE(rc);
      }
   }

   rc = BXVD_P_AllocateFWMem(hXvd,
                             pXvdCh,
                             &(pXvdCh->stDecodeFWMemSize),
                             &(pXvdCh->stDecodeFWBaseAddrs));

   if (rc != BERR_SUCCESS)
   {
      BXVD_CloseChannel(pXvdCh);
      return BERR_TRACE(rc);
   }

   /* Reduce by padding size sent to FW */
   pXvdCh->stDecodeFWMemSize.uiFWInnerLoopWorklistSize -= 128;

   /* Return the new channel context to the user */
   *phXvdCh = (BXVD_ChannelHandle)pXvdCh;

   hXvd->ahChannel[uiChannelNum] = *phXvdCh;

   if (bStillMode)
   {
      hXvd->bStillChannelAllocated = true;
      hXvd->uiStillChannelNum = uiChannelNum;
   }

   if (!bStillMode || !hXvd->bStillPictureCompatibilityMode)
   {
      /* We open the channel now only if we not in a still picture
       * compatibility mode, otherwise we open/close during
       * BXVD_DecodeStillPicture(), since we need to pass in the CDB
       * as the cabac bin buffer */
      rc = BXVD_P_HostCmdSendDecChannelOpen((BXVD_Handle)hXvd,
                                            pXvdCh,
                                            bStillMode,
                                            pXvdCh->sChSettings.eDecodeResolution,
                                            &(pXvdCh->stDecodeFWMemSize),
                                            &(pXvdCh->stDecodeFWBaseAddrs));

      if (rc == BERR_TIMEOUT)
      {
         hXvd->bWatchdogPending = true;
      }
   }

   BXVD_Status_OpenChannel(hXvd->hXvdStatus, pXvdCh);

   BXVD_DBG_MSG(pXvdCh, ("BXVD_OpenChannel() - pXvdCh = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh));

   BDBG_LEAVE(BXVD_OpenChannel);

   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_CloseChannel: API used to close a channel
***************************************************************************/
BERR_Code BXVD_CloseChannel(BXVD_ChannelHandle hXvdCh)
{
   BERR_Code rc = BERR_SUCCESS;
   BXVD_Handle hXvd = NULL;

   bool bWatchdogRequired = false;

   struct BXVD_P_InterruptCallbackInfo *pCallback;

   BDBG_ENTER(BXVD_CloseChannel);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hXvd = (BXVD_Handle) hXvdCh->pXvd;
   BDBG_ASSERT(hXvd);

   BXVD_DBG_MSG(hXvdCh, ("Closechannel Decoder: %d Ch: %d", hXvd->uDecoderInstance, hXvdCh->ulChannelNum));

   if ( hXvdCh->sChSettings.eChannelMode != BXVD_ChannelMode_eStill )
   {
      if ( hXvdCh->sDecodeSettings.bExternalPictureProviderMode == false )
      {
         BXDM_DisplayInterruptHandler_Handle hXdmDih;

         BXDM_PictureProvider_GetDIH_isrsafe( hXvdCh->hPictureProvider, &hXdmDih );

         if ( NULL != hXdmDih )
         {
            BERR_TRACE(BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(
                          hXdmDih,
                          BXDM_PictureProvider_GetPicture_isr,
                          hXvdCh->hPictureProvider));
         }
      }
   }

   /* Critical section to prevent ISR routines from using data about to be freed. */
   BKNI_EnterCriticalSection();

   /* NULL the channel entry to prevent DM from accessing the channel structure */
   hXvd->ahChannel[hXvdCh->ulChannelNum] = NULL;

   BKNI_LeaveCriticalSection();

   /* If the decoder is running, stop it now */
   if (hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive)
   {
      BXVD_DBG_MSG(hXvdCh, ("Decoder is active... calling BXVD_StopDecode"));
      rc = BXVD_StopDecode(hXvdCh);
   }

   if (hXvdCh->bDecoderChannelOpened)
   {
      rc = BERR_TRACE(BXVD_P_HostCmdSendDecChannelClose(hXvd, hXvdCh));

      if (rc == BERR_TIMEOUT)
      {
         bWatchdogRequired = true;
      }
   }

   BXVD_P_FreeFWMem(hXvd,
                    hXvdCh,
                    &(hXvdCh->stDecodeFWBaseAddrs));

   if ( NULL != hXvdCh->hPictureProvider )
   {
      rc = BERR_TRACE(BXDM_PictureProvider_Destroy( hXvdCh->hPictureProvider ));
   }

   /* SW7445-2757: to aid in debug, can print channel state at this time. */
   BXVD_Decoder_CloseChannel(hXvdCh);

   /* Adjust the number of active channels */
   hXvd->uiOpenChannels--;

   /* If still decode channel, we need to clear the flag. */
   if (hXvdCh->sChSettings.eChannelMode == BXVD_ChannelMode_eStill)
   {
      hXvd->bStillChannelAllocated = false;
   }

   /* Free the video compression standards list */

   hXvdCh->sChSettings.peVideoCmprStdList = NULL;
   hXvdCh->sChSettings.uiVideoCmprCount = 0;

   BXVD_Status_CloseChannel(hXvd->hXvdStatus, hXvdCh);

   BDBG_UNREGISTER_INSTANCE(hXvdCh);

   if (hXvdCh->sChSettings.eChannelMode != BXVD_ChannelMode_eStill)
   {
      /* Keep channel context to use again later */
      BXVD_P_KeepChannelHandle(hXvd, hXvdCh);
   }

   if (bWatchdogRequired == true)
   {
      hXvd->bWatchdogPending = true;

      /* Notify application if watchdog callback is registered */
      pCallback = &hXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eWatchdog];

      if (pCallback->BXVD_P_pAppIntCallbackPtr)
      {
         BKNI_EnterCriticalSection();

         pCallback->BXVD_P_pAppIntCallbackPtr(pCallback->pParm1,
                                              pCallback->parm2,
                                              0);
         BKNI_LeaveCriticalSection();
      }
      else
      {
         rc = BXVD_ProcessWatchdog(hXvd);
      }
   }

#if BXVD_P_POWER_MANAGEMENT
   if (hXvd->uiOpenChannels == 0)
   {
      /* Put decoder in hibernate state */
      BXVD_P_SetHibernateState(hXvd, true);
   }
#endif

   BDBG_LEAVE(BXVD_CloseChannel);
   return BERR_TRACE(rc);
}

#if BDBG_DEBUG_BUILD
static const char * const sTimebaseNameLUT[BAVC_Timebase_e3 + 1] =
{
   "BAVC_Timebase_e0",
   "BAVC_Timebase_e1",
   "BAVC_Timebase_e2",
   "BAVC_Timebase_e3"
};

static const char * const sSTCNameLUT[BXVD_STC_eMax] =
{
   "BXVD_STC_eZero",
   "BXVD_STC_eOne",
   "BXVD_STC_eTwo",
   "BXVD_STC_eThree",
   "BXVD_STC_eFour",
   "BXVD_STC_eFive",
   "BXVD_STC_eSix",
   "BXVD_STC_eSeven",
   "BXVD_STC_eEight",
   "BXVD_STC_eNine"
};

static const char * const sDisplayInterruptNameLUT[BXVD_DisplayInterrupt_eMax] =
{
   "BXVD_DisplayInterrupt_eZero",
   "BXVD_DisplayInterrupt_eOne",
   "BXVD_DisplayInterrupt_eTwo"
};

static const char * const sHITSModeNameLUT[BXVD_HITSMode_eClean + 1] =
{
   "BXVD_HITSMode_eDisabled",
   "BXVD_HITSMode_eLegacy",
   "BXVD_HITSMode_eClean"
};

static const char * const sErrorHandlingNameLUT[BXVD_ErrorHandling_ePrognostic + 1] =
{
   "BXVD_ErrorHandling_eOff",
   "BXVD_ErrorHandling_ePicture",
   "BXVD_ErrorHandling_ePrognostic"
};

static const char * const sFrameRateDetectionModeNameLUT[BXVD_FrameRateDetectionMode_eStable + 1] =
{
   "BXVD_FrameRateDetectionMode_eOff",
   "BXVD_FrameRateDetectionMode_eFast",
   "BXVD_FrameRateDetectionMode_eStable"
};

static const char * const sTimestampModeNameLUT[BXVD_TimestampMode_eMaxModes] =
{
   "BXVD_TimestampMode_eDecode",
   "BXVD_TimestampMode_eDisplay"
};
#endif

/***************************************************************************
BXVD_StartDecode: Starts decode on a given channel.
****************************************************************************/
BERR_Code BXVD_StartDecode(BXVD_ChannelHandle        hXvdChannel,
                           const BXVD_DecodeSettings *psDecodeSettings)
{
   uint32_t uiContextReg;
   uint32_t uiContextRegExtend[BXVD_NUM_EXT_RAVE_CONTEXT];

#if  !BXVD_P_RAVE_40BIT_ADDRESSABLE
   uint32_t CDBBase;
#else
   uint64_t CDBBase;
#endif

   uint32_t uiChannelMode;

#if !BXVD_P_FW_HIM_API
   uint32_t uiVirtAddr;
#endif

   uint32_t uiVecIndex;
   uint32_t i;
   bool bDQTEnabled = false; /* SW7425-2686 */

   BXVD_P_Context *pXvd;
   BXVD_P_Channel *pXvdCh;

   /* SW7425-3177: for mapping BAVC_VideoCompressionStd to BXVD_P_PPB_Protocol.
    * Will be passed to AVD via BXVD_P_HostCmdSendDecChannelStart() */
   BXVD_P_PPB_Protocol eProtocol;
   BERR_Code rc;

   BXVD_DisplayInterrupt eDisplayInterrupt;

   BDBG_ENTER(BXVD_StartDecode);

   BDBG_ASSERT(psDecodeSettings);

   BDBG_CASSERT((int)BBOX_XVD_DecodeResolution_eHD == (int)BXVD_DecodeResolution_eHD);
   BDBG_CASSERT((int)BBOX_XVD_DecodeResolution_eSD == (int)BXVD_DecodeResolution_eSD);
   BDBG_CASSERT((int)BBOX_XVD_DecodeResolution_eCIF == (int)BXVD_DecodeResolution_eCIF);
   BDBG_CASSERT((int)BBOX_XVD_DecodeResolution_eQCIF == (int)BXVD_DecodeResolution_eQCIF);
   BDBG_CASSERT((int)BBOX_XVD_DecodeResolution_e4K == (int)BXVD_DecodeResolution_e4K);

   /* Check handle type for correctness */
   if (hXvdChannel->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   pXvdCh = (BXVD_P_Channel*)hXvdChannel;
   BDBG_ASSERT(pXvdCh);

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - hXvdChannel = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvdChannel));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - psDecodeSettings = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)psDecodeSettings));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - eChannelType: %08x", pXvdCh->eChannelType));

   pXvd = pXvdCh->pXvd;
   BDBG_ASSERT(pXvd);

   if (pXvdCh->eDecoderState == BXVD_P_DecoderState_eActive)
   {
      BXVD_DBG_MSG(pXvdCh, ("Decoder is already active"));

      BXVD_StopDecode(pXvdCh);

      return BERR_TRACE(BXVD_ERR_DECODER_ACTIVE);
   }

   if (BXVD_P_IsDecodeProtocolSupported( pXvd, psDecodeSettings->eVideoCmprStd ) == false)
   {
      BXVD_DBG_WRN(pXvdCh, ("Video compression standard:%u not supported on this platform.", psDecodeSettings->eVideoCmprStd));

      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   if (psDecodeSettings->pContextMap == NULL)
   {
      BXVD_DBG_MSG(pXvdCh, ("NULL XPT Context pointer"));

      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Save decode settings */
   BKNI_Memcpy((void*)&(pXvdCh->sDecodeSettings),
               (void*)psDecodeSettings,
               sizeof(BXVD_DecodeSettings));

   /* Since GetDecodeDefaultSettings was added recently, we need to
    * check the signature of the BXVD_DecodeSettings struct to see if
    * the app called it.  Legacy apps won't call it, so we need to set
    * the recently added eSTC parameter accordingly to maintain backwards
    * compatibility with pre-mosaic firmware */
   if (pXvdCh->sDecodeSettings.uiSignature != BXVD_DECODESETTINGS_SIGNATURE)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - No signature found!  Using legacy defaults."));

      /* SWSTB-3450: call BXVD_S_GetDecodeDefaultSettings
       * instead of using  s_stDefaultDecodeSettings */

      BXVD_S_GetDecodeDefaultSettings( pXvdCh, &pXvdCh->sDecodeSettings, true  );
   }

   /* SW7425-1064: if this is the "enhanced" channel of a pair of linked channels,
    * use the interrupt number of the base channel.  Otherwise just use the number
    * for this channel.
    */
   if ( BXVD_P_ChannelType_eEnhanced == pXvdCh->eChannelType
       && NULL != pXvdCh->hXvdChLinked )
   {
      eDisplayInterrupt = pXvdCh->hXvdChLinked->sDecodeSettings.eDisplayInterrupt;

      /* TODO: is the following assignment needed? */
      /* pXvdCh->sDecodeSettings.eDisplayInterrupt = eDisplayInterrupt;*/

#if BXVD_P_USE_TWO_DECODERS
      /* If the XVD handles don't match, the two channels are being decoded
       * on separate decoders.  The "enhanced" decoder needs to be driven off
       * the same interrupt as the "base" decoder.  Normally this regsitration
       * is handled by a call to"BXVD_RegisterVdcDeviceInterrupt".
       */
      if ( pXvdCh->pXvd != pXvdCh->hXvdChLinked->pXvd )
      {
         BXVD_DisplayInterruptProvider_P_InterruptSettings stInterruptConfig;

         /* Save the interrupt settings for the enhanced channel,
          * these will be restored in BXVD_StopDecode.
          */
         BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration(
                     pXvdCh->pXvd->hXvdDipCh[ eDisplayInterrupt ],
                     &(pXvdCh->stEnhancedInterruptConfig)
                     );

         /* Get the interrupt settings for the base channel. */
         BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration(
                     pXvdCh->hXvdChLinked->pXvd->hXvdDipCh[ eDisplayInterrupt ],
                     &stInterruptConfig
                     );

         /* Set the interrupt settings on the enhanced channel to match the base channel. */
         rc = BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration(
                     pXvdCh->pXvd->hXvdDipCh[ eDisplayInterrupt ],
                     &stInterruptConfig
                     );

         /* TODO: Could this call fail? If so, what are the appropriate actions? */
         if (rc != BERR_SUCCESS )
         {
            BXVD_DBG_ERR(pXvdCh, ("BXVD_StartDecode->BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration returned %d on the base decoder", rc));
            return BERR_TRACE(rc);
         }
      }
#endif

   }
   else
   {
     eDisplayInterrupt = pXvdCh->sDecodeSettings.eDisplayInterrupt;
   }

   /* check for BAVC_VideoCompressionStd_eVC1SimpleMain and replace with BAVC_VideoCompressionStd_eVC1
      note, this is done in the saved settings so the working state will be saved  */
   if (pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eVC1SimpleMain)
   {
      pXvdCh->sDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eVC1;
   }

   /* If frame rate in decode settings is not valid, then use channel default frame rate */
   if (pXvdCh->sDecodeSettings.eDefaultFrameRate == BAVC_FrameRateCode_eUnknown)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - Unknown default frame rate(%d)!  Using channel default(%d)",
                                 pXvdCh->sChSettings.eDefaultFrameRate,
                                 pXvdCh->sDecodeSettings.eDefaultFrameRate));
      pXvdCh->sDecodeSettings.eDefaultFrameRate = pXvdCh->sChSettings.eDefaultFrameRate;
   }

#if BDBG_DEBUG_BUILD
   if (pXvdCh->sDecodeSettings.eVideoCmprStd < BXVD_S_MAX_VIDEO_PROTOCOL)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eVideoCmprStd = %s (%d)",
                                 sVideoCompressionStdNameLUT[pXvdCh->sDecodeSettings.eVideoCmprStd],
                                 pXvdCh->sDecodeSettings.eVideoCmprStd));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eVideoCmprStd = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eVideoCmprStd));
   }

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.ulMultiStreamId = %d (deprecated)", pXvdCh->sDecodeSettings.ulMultiStreamId));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.ulVideoSubStreamId = %d (deprecated)", pXvdCh->sDecodeSettings.ulVideoSubStreamId));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bPlayback = %d", pXvdCh->sDecodeSettings.bPlayback));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bCrcMode = %d", pXvdCh->sDecodeSettings.bCrcMode));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.stDataXprtOutput.eXptOutputId = %d (deprecated)", pXvdCh->sDecodeSettings.stDataXprtOutput.eXptOutputId));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.stDataXprtOutput.eXptSourceId = %d (deprecated)", pXvdCh->sDecodeSettings.stDataXprtOutput.eXptSourceId));
   if (pXvdCh->sDecodeSettings.eTimeBase <= BAVC_Timebase_e3)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eTimeBase = %s (%d)",
                                 sTimebaseNameLUT[pXvdCh->sDecodeSettings.eTimeBase],
                                 pXvdCh->sDecodeSettings.eTimeBase));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eTimeBase = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eTimeBase));
   }

   if (pXvdCh->sDecodeSettings.eTimestampMode < BXVD_TimestampMode_eMaxModes)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eTimestampMode = %s (%d)",
                                 sTimestampModeNameLUT[pXvdCh->sDecodeSettings.eTimestampMode],
                                 pXvdCh->sDecodeSettings.eTimestampMode));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eTimestampMode = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eTimestampMode));
   }

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bIFrameAsRAP = %d",
                              pXvdCh->sDecodeSettings.bIFrameAsRAP));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bAVCErrorConcealmentMode = %d",
                              pXvdCh->sDecodeSettings.bAVCErrorConcealmentMode));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bIOnlyFieldOutputMode = %d",
                              pXvdCh->sDecodeSettings.bIOnlyFieldOutputMode));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.pContextMap = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sDecodeSettings.pContextMap));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.pContextMap->CDB_Read = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sDecodeSettings.pContextMap->CDB_Read));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.uiSignature = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sDecodeSettings.uiSignature));
   for (i=0; i< pXvdCh->sDecodeSettings.uiContextMapExtNum; i++)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.aContextMapExtended[%d] = 0x%0*lx", i, BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sDecodeSettings.aContextMapExtended[i]));
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.aContextMapExtended[%d]->CDB_Read = 0x%0*lx", i, BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->sDecodeSettings.aContextMapExtended[i]->CDB_Read));
   }

   /* This checks to see if STC index is supported on this platform */
   if ((pXvdCh->sDecodeSettings.eSTC < BXVD_STC_eMax) && (pXvdCh->sDecodeSettings.eSTC < BXVD_P_STC_MAX))
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eSTC = %s (%d)",
                                 sSTCNameLUT[pXvdCh->sDecodeSettings.eSTC],
                                 pXvdCh->sDecodeSettings.eSTC));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eSTC = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eSTC));
   }
   if (eDisplayInterrupt < BXVD_DisplayInterrupt_eMax)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eDisplayInterrupt = %s (%d)",
                                 sDisplayInterruptNameLUT[eDisplayInterrupt],
                                 eDisplayInterrupt));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eDisplayInterrupt = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 eDisplayInterrupt));
   }
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.uiVDCRectangleNum = %d", pXvdCh->sDecodeSettings.uiVDCRectangleNum));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bAstmMode = %d", pXvdCh->sDecodeSettings.bAstmMode));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bVsyncModeOnPcrDiscontinuity = %d", pXvdCh->sDecodeSettings.bVsyncModeOnPcrDiscontinuity));
   if (pXvdCh->sDecodeSettings.eHITSMode <= BXVD_HITSMode_eClean)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eHITSMode = %s (%d)",
                                 sHITSModeNameLUT[pXvdCh->sDecodeSettings.eHITSMode],
                                 pXvdCh->sDecodeSettings.eHITSMode));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eHITSMode = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eHITSMode));
   }
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bZeroDelayOutputMode = %d", pXvdCh->sDecodeSettings.bZeroDelayOutputMode));
   if (pXvdCh->sDecodeSettings.eDefaultFrameRate < BXVD_S_MAX_FRAMERATE)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eDefaultFrameRate = %s (%d)",
                                 sFrameRateCodeNameLUT[pXvdCh->sDecodeSettings.eDefaultFrameRate],
                                 pXvdCh->sDecodeSettings.eDefaultFrameRate));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eDefaultFrameRate = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eDefaultFrameRate));
   }

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.uiPreRollRate = %d", pXvdCh->sDecodeSettings.uiPreRollRate));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.uiPreRollNumerator = %d", pXvdCh->sDecodeSettings.uiPreRollNumerator));

   if (pXvdCh->sDecodeSettings.eErrorHandling <= BXVD_ErrorHandling_ePrognostic)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eErrorHandling = %s (%d)",
                                 sErrorHandlingNameLUT[pXvdCh->sDecodeSettings.eErrorHandling],
                                 pXvdCh->sDecodeSettings.eErrorHandling));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eErrorHandling = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eErrorHandling));
   }

   /* SWSTB-439:  the error handling level. */
   BXVD_DBG_MSG(pXvdCh, ("%s() - BXVD_DecodeSettings.uiErrorThreshold = %d", BSTD_FUNCTION, pXvdCh->sDecodeSettings.uiErrorThreshold));

   if (pXvdCh->sDecodeSettings.eFrameRateDetectionMode <= BXVD_FrameRateDetectionMode_eStable)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eFrameRateDetectionMode = %s (%d)",
                                 sFrameRateDetectionModeNameLUT[pXvdCh->sDecodeSettings.eFrameRateDetectionMode],
                                 pXvdCh->sDecodeSettings.eFrameRateDetectionMode));
   }
   else
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.eFrameRateDetectionMode = %s (%d)",
                                 "Unknown/Invalid Value!",
                                 pXvdCh->sDecodeSettings.eFrameRateDetectionMode));
   }
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bIgnoreDPBOutputDelaySyntax = %d",
                              pXvdCh->sDecodeSettings.bIgnoreDPBOutputDelaySyntax));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.uiSEIMessageFlags = %d",
                              pXvdCh->sDecodeSettings.uiSEIMessageFlags));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bPFrameSkipDisable = %d",
                              pXvdCh->sDecodeSettings.bPFrameSkipDisable));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bExternalPictureProviderMode = %d",
                              pXvdCh->sDecodeSettings.bExternalPictureProviderMode));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bAVCAspectRatioOverridMode = %d",
                              pXvdCh->sDecodeSettings.bAVCAspectRatioOverrideMode));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bSVC3DModeEnable = %d",
                              pXvdCh->sDecodeSettings.bSVC3DModeEnable));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bSWCoefAVCDecodeModeEnable = %d",
                              pXvdCh->sDecodeSettings.bSWCoefAVCDecodeModeEnable));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bIgnoreNumReorderFramesEqZero = %d",
                         pXvdCh->sDecodeSettings.bIgnoreNumReorderFramesEqZero));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bEarlyPictureDeliveryMode = %d",
                         pXvdCh->sDecodeSettings.bEarlyPictureDeliveryMode));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bUserDataBTPModeEnable = %d",
                         pXvdCh->sDecodeSettings.bUserDataBTPModeEnable));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - BXVD_DecodeSettings.bNRTModeEnable = %d",
                         pXvdCh->sDecodeSettings.bNRTModeEnable));

#endif

#if BXVD_P_ENABLE_DRAM_PREF_INFO
   {
      char *pPerfMemc, *pPerfMemcClient;

      unsigned int   uiDDRStatCtrlReg;
      unsigned int   uiDDRStatCtrlVal;
      unsigned int   uiDDRStatCtrlEnableMask;
      unsigned int   uiDDRStatTimerReg;
      unsigned int   uiClientRead;
      unsigned int   uiCas;
      unsigned int   uiIntraPenality;
      unsigned int   uiPostPenality;

      if ( (pPerfMemcClient = getenv("bxvd_perf_memc_client")) != NULL )
      {
         sscanf(pPerfMemcClient, "%d", &giPerfMemcClient);

         BKNI_Printf("bxvd_perf_memc_client %d\n",  giPerfMemcClient);

         if ( (pPerfMemc = getenv("bxvd_perf_memc")) != NULL )
         {
            sscanf(pPerfMemc, "%d", &giPerfMemc);
         }
         else
         {
            giPerfMemc = 0;
         }

#if BXVD_P_MEMC_1_PRESENT
         if ( giPerfMemc == 1 )
         {
            uiDDRStatCtrlReg = BCHP_MEMC_DDR_1_STAT_CONTROL;
            uiDDRStatCtrlVal = giPerfMemcClient;
            uiDDRStatCtrlEnableMask = BCHP_MEMC_DDR_1_STAT_CONTROL_STAT_ENABLE_MASK;
            uiDDRStatTimerReg = BCHP_MEMC_DDR_1_STAT_TIMER;
            uiClientRead = BCHP_MEMC_DDR_1_STAT_CLIENT_SERVICE_TRANS_READ;
            uiCas = (BCHP_MEMC_DDR_1_STAT_CAS_CLIENT_0 + (giPerfMemcClient * 4));
            uiIntraPenality = BCHP_MEMC_DDR_1_STAT_CLIENT_SERVICE_INTR_PENALTY;
            uiPostPenality = BCHP_MEMC_DDR_1_STAT_CLIENT_SERVICE_POST_PENALTY;
         }
#endif
#if BXVD_P_MEMC_2_PRESENT
         else if ( giPerfMemc == 2)
         {
            uiDDRStatCtrlReg = BCHP_MEMC_DDR_2_STAT_CONTROL;
            uiDDRStatCtrlVal = giPerfMemcClient;
            uiDDRStatCtrlEnableMask = BCHP_MEMC_DDR_2_STAT_CONTROL_STAT_ENABLE_MASK;
            uiDDRStatTimerReg = BCHP_MEMC_DDR_2_STAT_TIMER;
            uiClientRead = BCHP_MEMC_DDR_2_STAT_CLIENT_SERVICE_TRANS_READ;
            uiCas = (BCHP_MEMC_DDR_2_STAT_CAS_CLIENT_0 + (giPerfMemcClient * 4));
            uiIntraPenality = BCHP_MEMC_DDR_2_STAT_CLIENT_SERVICE_INTR_PENALTY;
            uiPostPenality = BCHP_MEMC_DDR_2_STAT_CLIENT_SERVICE_POST_PENALTY;
         }
         else
#endif
         {
            giPerfMemc = 0;

            uiDDRStatCtrlReg = BCHP_MEMC_DDR_0_STAT_CONTROL;
            uiDDRStatCtrlVal = giPerfMemc;
            uiDDRStatCtrlEnableMask = BCHP_MEMC_DDR_0_STAT_CONTROL_STAT_ENABLE_MASK;
            uiDDRStatTimerReg = BCHP_MEMC_DDR_0_STAT_TIMER;
            uiClientRead = BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_TRANS_READ;
            uiCas = (BCHP_MEMC_DDR_0_STAT_CAS_CLIENT_0 + (giPerfMemcClient * 4));
            uiIntraPenality = BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_INTR_PENALTY;
            uiPostPenality = BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_POST_PENALTY;
         }

         BKNI_Printf("bxvd_perf_memc %d\n", giPerfMemc);

         rc = BXVD_P_HostCmdDramPerf(pXvd,
                                     uiDDRStatCtrlReg,
                                     uiDDRStatCtrlVal,
                                     uiDDRStatCtrlEnableMask,
                                     uiDDRStatTimerReg,
                                     uiClientRead,
                                     uiCas,
                                     uiIntraPenality,
                                     uiPostPenality);
      }
   }
#endif
#if  !BXVD_P_RAVE_40BIT_ADDRESSABLE
#define BXVD_XPT_WR_PTR_OFFSET 4
#else
#define BXVD_XPT_WR_PTR_OFFSET 8
#endif

   /* Read CDB Base register to verify that the CDB Base is 256 byte aligned. */
   uiContextReg = ((pXvdCh->sDecodeSettings.pContextMap->CDB_Read)+BXVD_XPT_WR_PTR_OFFSET);

#if  !BXVD_P_RAVE_40BIT_ADDRESSABLE
   CDBBase = BXVD_Reg_Read32( pXvdCh->pXvd, uiContextReg );

   if (CDBBase != (CDBBase & (~0xff)))
   {
      BXVD_DBG_WRN(pXvdCh, ("CBD Base not 256 byte aligned: 0x%08x", CDBBase));
   }
#else
   CDBBase = BXVD_Reg_Read64( pXvdCh->pXvd, uiContextReg );

   if (CDBBase != (CDBBase & (~0xff)))
   {
      BXVD_DBG_WRN(pXvdCh, ("CBD Base not 256 byte aligned: " BDBG_UINT64_FMT, BDBG_UINT64_ARG(CDBBase)));
   }
#endif

#if !BXVD_P_AVD_ARC600
   {
      uint32_t uiCDBEnd, uiITBEnd;

      /* Read CDB End register to verify that the CDB end is accessible by ARC */
      uiContextReg = ((pXvdCh->sDecodeSettings.pContextMap->CDB_Read)+8);
      uiCDBEnd = BXVD_Reg_Read32( pXvdCh->pXvd, uiContextReg );

      if (uiCDBEnd >= BXVD_P_ARC300_RAM_LIMIT)
      {
         BXVD_DBG_ERR(pXvdCh, ("CDB: %0x allocated in region greater than 768MB!", uiCDBEnd));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      /* Read CDB End register to verify that the CDB end is accessible by ARC */
      uiContextReg = ((pXvdCh->sDecodeSettings.pContextMap->CDB_Read)+44);
      uiITBEnd = BXVD_Reg_Read32( pXvdCh->pXvd, uiContextReg );

      if (uiITBEnd >= BXVD_P_ARC300_RAM_LIMIT)
      {
         BXVD_DBG_ERR(pXvdCh, ("ITB:%0x allocated in region greater than 768MB!", uiITBEnd));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

   }
#endif

   /* Calculate the register address for the appropriate context */
   uiContextReg = ((pXvdCh->sDecodeSettings.pContextMap->CDB_Read) - BXVD_XPT_WR_PTR_OFFSET);

#if BXVD_P_FW_HW_BASE_NON_ZERO
   uiContextReg -= BXVD_P_STB_REG_BASE;
#endif

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - XPT Rave Context reg base = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)uiContextReg));

   pXvdCh->ulXptCDB_Read = pXvdCh->sDecodeSettings.pContextMap->CDB_Read;

   if (pXvdCh->sDecodeSettings.uiContextMapExtNum != 0 )
   {
      for (i=0; i< pXvdCh->sDecodeSettings.uiContextMapExtNum; i++)
      {
         /* Read CDB Base register to verify that the CDB Base is 256 byte aligned. */
         uiContextRegExtend[i] = ((pXvdCh->sDecodeSettings.aContextMapExtended[i]->CDB_Read)+BXVD_XPT_WR_PTR_OFFSET);

#if  !BXVD_P_RAVE_40BIT_ADDRESSABLE
         CDBBase = BXVD_Reg_Read32( pXvdCh->pXvd, uiContextRegExtend[i]);

         if (CDBBase != (CDBBase & (~0xff)))
         {
            BXVD_DBG_MSG(pXvdCh, ("Extended[%d] CBD Base not 256 byte aligned: 0x%08x", i, CDBBase));
         }
#else
         CDBBase = BXVD_Reg_Read64( pXvdCh->pXvd, uiContextRegExtend[i] );

         if (CDBBase != (CDBBase & (~0xff)))
         {
            BXVD_DBG_MSG(pXvdCh, ("Extended[%d] CBD Base not 256 byte aligned: " BDBG_UINT64_FMT, i, BDBG_UINT64_ARG(CDBBase)));
         }
#endif
         /* Calculate the register address for the appropriate context */
         uiContextRegExtend[i] = ((pXvdCh->sDecodeSettings.aContextMapExtended[i]->CDB_Read)-BXVD_XPT_WR_PTR_OFFSET);

#if BXVD_P_FW_HW_BASE_NON_ZERO
         uiContextRegExtend[i] -= BXVD_P_STB_REG_BASE;
#endif
         BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - XPT Rave Context Extended[%d] reg base = 0x%08x",
                                    i, uiContextRegExtend[i]));

         /* Save extended CDB read pointer register address to restore when restart is needed */
         pXvdCh->aulXptCDB_Read_Extended[i] = pXvdCh->sDecodeSettings.aContextMapExtended[i]->CDB_Read;
      }
   }
   else
   {
      uiContextRegExtend[0] = 0;
   }

   /* SW7445-881: support bHostSparseMode for H265/HEVC  */
   if (((pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH264)
         || (pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH265))
       && (pXvdCh->stDecoderContext.bHostSparseMode == true))
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - Sparse mode TRUE"));

      if (pXvdCh->eCurrentSkipMode == BXVD_SkipMode_eDecode_IPB)
      {
         uiChannelMode = VDEC_CHANNEL_MODE_SPARSE_NOSKIP;
      }
      else if (pXvdCh->eCurrentSkipMode == BXVD_SkipMode_eDecode_Ref_Only)
      {
         uiChannelMode = VDEC_CHANNEL_MODE_SPARSE_REFONLY;
      }
      else if (pXvdCh->eCurrentSkipMode == BXVD_SkipMode_eDecode_IP_Only)
      {
         uiChannelMode = VDEC_CHANNEL_MODE_SPARSE_IPONLY;
      }
      else if (pXvdCh->eCurrentSkipMode == BXVD_SkipMode_eDecode_I_Only)
      {
         uiChannelMode = VDEC_CHANNEL_MODE_SPARSE_IONLY;
      }
      else
      {
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - FW ChanMode:%d CurSkipMode:%d", uiChannelMode, pXvdCh->eCurrentSkipMode));
   }
   else if (pXvdCh->stDecoderContext.bHostSparseMode == true)
   {
      BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - Sparse mode enabled on Non H264 stream, Sparse mode now disabled"));

      pXvdCh->stDecoderContext.bHostSparseMode = false;

      uiChannelMode = VDEC_CHANNEL_MODE_SPARSE_NORMAL;
   }
   else
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - Sparse mode FALSE"));
      uiChannelMode = VDEC_CHANNEL_MODE_SPARSE_NORMAL;
   }

   if (((pXvdCh->sDecodeSettings.eVideoCmprStd != BAVC_VideoCompressionStd_eVP8) &&
        (pXvdCh->sDecodeSettings.eVideoCmprStd != BAVC_VideoCompressionStd_eVP9)) &&
       (pXvdCh->sDecodeSettings.bZeroDelayOutputMode == true))
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_ZERO_DELAY;
   }

   /* SW7445-881: support bIFrameAsRAP for H265/HEVC  */
   if (((pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH264)
        || (pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH265))
       && (pXvdCh->sDecodeSettings.bIFrameAsRAP == true))
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_IFRAME_AS_RAP;
   }

   if ((pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH264) &&
       (pXvdCh->sDecodeSettings.bAVCErrorConcealmentMode == true))
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_ENA_ERROR_CONCEALMENT;
   }

   if ((pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH264) &&
       (pXvdCh->sDecodeSettings.bIOnlyFieldOutputMode == true))
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_ENA_IONLY_FIELD_OUTPUT;
   }

   /* SW7425-2686: check for either the original DQT or MP DQT. */
   bDQTEnabled = pXvdCh->stDecoderContext.bReversePlayback;
   bDQTEnabled |= ( pXvdCh->stTrickModeSettings.stGopTrickMode.eMode != BXVD_DQTTrickMode_eDisable );

   /* If DQT mode is enabled, tell the decoder to use all available resources. */
   if ( true == bDQTEnabled )
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_NON_LEGACY;
   }


   /* Set HITS mode if specifed in settings. */
   if (pXvdCh->sDecodeSettings.eHITSMode == BXVD_HITSMode_eLegacy)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_HITS;
   }
   else if (pXvdCh->sDecodeSettings.eHITSMode == BXVD_HITSMode_eClean)
   {
      uiChannelMode |= (VDEC_CHANNEL_MODE_HITS | VDEC_CHANNEL_MODE_CLEAN_HITS);
   }

   /* Set BD playback mode */
   if (pXvdCh->sDecodeSettings.bBluRayDecode)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_BLU_RAY_DECODE;
   }

   /* Set AVC in AVI mode */
   if (pXvdCh->sDecodeSettings.eTimestampMode == BXVD_TimestampMode_eDisplay)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_TIMESTAMP_DISPLAY_ORDER;
   }

   if (pXvdCh->sDecodeSettings.uiSEIMessageFlags & BXVD_DECODESETTINGS_SEI_MESSAGE_FRAMEPACKING)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_ENA_SEI_FRAME_PACK;
   }

   if (pXvdCh->sDecodeSettings.bPFrameSkipDisable)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_DISABLE_P_SKIP;
   }

   if ((pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH264) &&
       (pXvdCh->sDecodeSettings.bAVCAspectRatioOverrideMode == true))
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_FILE_FORMAT;
   }

   /*
    * Check if this channel is a still picture decode channel, if so,
    * then a VEC vsync interrupt is not associated with this channel.
    */
   if ( hXvdChannel->sChSettings.eChannelMode == BXVD_ChannelMode_eStill )
   {
      uiVecIndex = BXVD_P_VEC_UNUSED;
   }
   else
   {
      uiVecIndex = eDisplayInterrupt;
   }

   /* SW3556-1058:: conditionally ignore the DPB output delay syntax */
   if (pXvdCh->sDecodeSettings.bIgnoreDPBOutputDelaySyntax == true)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_DISABLE_DPB_OUTPUT_DELAY;
   }

   if (pXvdCh->sDecodeSettings.bSVC3DModeEnable == true)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_3D_SVC_DECODE;
   }

   if (pXvdCh->sDecodeSettings.bSWCoefAVCDecodeModeEnable == true)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_SW_COEF_AVC_DECODE;
   }

   if (pXvdCh->sDecodeSettings.bIgnoreNumReorderFramesEqZero == true)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_IGN_NUM_REORDR_FRM_ZERO;
   }

   if (pXvdCh->sDecodeSettings.bEarlyPictureDeliveryMode == true)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_EARLY_PIC_DELIVERY;
   }

   if (pXvdCh->sDecodeSettings.bUserDataBTPModeEnable == true)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_ENA_UD_BTP;
   }

   if (pXvdCh->sDecodeSettings.bNRTModeEnable == true)
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_NRT_ENABLE;
   }

   if (((pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eH265) ||
        (pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eVP9)) &&
       (pXvdCh->sChSettings.b10BitBuffersEnable != true))
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_OUTPUT_ALL_10BIT_TO_8BIT;
   }

   if ((pXvdCh->sDecodeSettings.eVideoCmprStd == BAVC_VideoCompressionStd_eVP8) &&
       (pXvdCh->sChSettings.uiExtraPictureMemoryAtoms != 0))
   {
      uiChannelMode |= VDEC_CHANNEL_MODE_NON_LEGACY;
   }

   /* SW7425-3177: map eVideoCmprStd to a BXVD_P_PPB_Protocol value. */
   rc = BXVD_P_MapToAVDProtocolEnum(
                  pXvd,
                  pXvdCh->sDecodeSettings.eVideoCmprStd,
                  &eProtocol );

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BXVD_P_DetermineChromaBufferBase(hXvdChannel, eProtocol);

#if BXVD_P_CAPTURE_RVC
   BXVD_P_StartRVCCapture((BXVD_Handle)pXvd, eProtocol);
#endif

   rc = BXVD_P_HostCmdSendDecChannelStart((BXVD_Handle)pXvd,
                                          hXvdChannel->ulChannelNum,
                                          eProtocol,
                                          uiChannelMode,
                                          uiContextReg,
                                          &uiContextRegExtend[0],
                                          uiVecIndex);

   if (rc == BERR_TIMEOUT)
   {
      pXvd->bWatchdogPending = true;

      return (rc);
   }

   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Critical section to prevent ISR routines from using data not yet initialized. */

   BKNI_EnterCriticalSection();

   /* SW7340-199: reset the "DM" state cached in the channel structure. */

   BKNI_Memset(&(pXvdCh->stPictureParameterInfo), 0, sizeof( BXVD_PictureParameterInfo ) );
   pXvdCh->bProgressiveStream_7411 = false;
   pXvdCh->uiPPBSerialNumber = 0;

   if ( eDisplayInterrupt == BXVD_DisplayInterrupt_eZero )
   {
      BXVD_P_SAVE_XVD_CHANNEL_DISPLAY_INFO_0(pXvdCh, pXvd);
   }
#if BXVD_P_PICTURE_DATA_RDY_2_SUPPORTTED
   else if ( eDisplayInterrupt == BXVD_DisplayInterrupt_eTwo)
   {
      BXVD_P_SAVE_XVD_CHANNEL_DISPLAY_INFO_2(pXvdCh, pXvd);
   }
#endif
   else
   {
      BXVD_P_SAVE_XVD_CHANNEL_DISPLAY_INFO_1(pXvdCh, pXvd);
   }

   /*
    * Initialize a few DM variables including the variable that
    * signifies end of all inits for DM
    */

#if BXVD_P_FW_HIM_API

   /* Using cached address space for Rev K core and later */

   pXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue =
      (BXVD_P_PictureDeliveryQueue *) BXVD_P_OFFSET_TO_VA(pXvdCh, pXvdCh->stDeliveryQueue.ulQueueOffset);

   pXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue =
      (BXVD_P_PictureReleaseQueue *) BXVD_P_OFFSET_TO_VA(pXvdCh, pXvdCh->stReleaseQueue.ulQueueOffset);

#else

   /* Using uncached address space for picture queue and DMS info */

   uiVirtAddr = pXvdCh->uiFWGenMemBaseUncachedVirtAddr + ((uint32_t)pXvdCh->ulPicBuf - pXvdCh->FWGenMemBasePhyAddr);
   pXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue = (BXVD_P_PictureDeliveryQueue *)uiVirtAddr;

   uiVirtAddr = pXvdCh->uiFWGenMemBaseUncachedVirtAddr + ((uint32_t)pXvdCh->ulPicRelBuf - pXvdCh->FWGenMemBasePhyAddr);
   pXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue = (BXVD_P_PictureDeliveryQueue *)uiVirtAddr;

   uiVirtAddr = pXvd->uiFWMemBaseUncachedVirtAddr + ((uint32_t)pXvdCh->ulPicInfoRelBuf - pXvd->FWMemBasePhyAddr);
   pXvdCh->stChBufferConfig.pAVD_PI2DMS_Buffer = ( BXVD_P_DM2DMSInfo *)uiVirtAddr;

#endif

   pXvd->auiActiveDecodes[pXvdCh->sChSettings.eChannelMode][eDisplayInterrupt]++;

   BXDM_PictureProvider_SetProtocol_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.eVideoCmprStd
            );

   BXDM_PictureProvider_SetCRCMode_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.bCrcMode
            );

   BXVD_Decoder_SetCRCMode_isr(
            pXvdCh,
            pXvdCh->sDecodeSettings.bCrcMode
            );
   {
      BXDM_PictureProvider_PulldownMode ePulldownMode;

      switch ( pXvdCh->sDecodeSettings.eProgressiveOverrideMode )
      {
         case BXVD_ProgressiveOverrideMode_eBottomTop:
            ePulldownMode = BXDM_PictureProvider_PulldownMode_eBottomTop;
            break;

         case BXVD_ProgressiveOverrideMode_eDisable:
            ePulldownMode = BXDM_PictureProvider_PulldownMode_eUseEncodedFormat;
            break;

         case BXVD_ProgressiveOverrideMode_eTopBottom:
            ePulldownMode = BXDM_PictureProvider_PulldownMode_eTopBottom;
            break;

         default:
            ePulldownMode = BXDM_PictureProvider_PulldownMode_eTopBottom;
            BXVD_DBG_ERR(pXvdCh, ("Unknown progressive override mode: 0x%x (%d)",
                     pXvdCh->sDecodeSettings.eProgressiveOverrideMode, pXvdCh->sDecodeSettings.eProgressiveOverrideMode));
            break;
      }

      BXDM_PictureProvider_Set480pPulldownMode_isr(
               pXvdCh->hPictureProvider,
               ePulldownMode
               );

      BXDM_PictureProvider_Set1080pPulldownMode_isr(
               pXvdCh->hPictureProvider,
               ePulldownMode
               );
   }

   BXVD_SetSTCSource_isr(
            pXvdCh,
            pXvdCh->sDecodeSettings.eSTC
            );

   BXDM_PictureProvider_SetRemovalDelay_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sChSettings.ulRemovalDelay
            );

   if ( pXvdCh->sDecodeSettings.uiPreRollRate > 1 )
   {
      BXDM_Picture_Rate stPreRollRate;
      BKNI_Memset(&stPreRollRate, 0, sizeof( BXDM_Picture_Rate ) );

      /* SW7445-2421: allows for rates > 50%. The error check is just in case initialization is wrong. */
      stPreRollRate.uiNumerator = ( pXvdCh->sDecodeSettings.uiPreRollNumerator != 0 ) ? pXvdCh->sDecodeSettings.uiPreRollNumerator : 1;

      stPreRollRate.uiDenominator = pXvdCh->sDecodeSettings.uiPreRollRate;

      BXDM_PictureProvider_SetPreRollRate_isr(
               pXvdCh->hPictureProvider,
               &stPreRollRate
               );
   }

   BXDM_PictureProvider_SetPlaybackMode_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.bPlayback
            );

   BXDM_PictureProvider_SetDefaultFrameRate_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.eDefaultFrameRate
            );

   BXDM_PictureProvider_SetFrameRateDetectionMode_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.eFrameRateDetectionMode
            );

   BXDM_PictureProvider_SetASTMMode_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.bAstmMode
            );

   BXDM_PictureProvider_SetVirtualTSMOnPCRDiscontinuityMode_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.bVsyncModeOnPcrDiscontinuity
            );

   {
      /* SWSTB-68: pass along the PCR mode from DecodeSettings. If additional modes are added
       * to BXDM_PictureProvider_PCRModes, a ...GetPCRMode... call will be needed here. */

      BXDM_PictureProvider_PCRModes stPCRModes;

      BKNI_Memset( &stPCRModes, 0, sizeof(BXDM_PictureProvider_PCRModes) );

      stPCRModes.ePCRDiscontinuityModeAtStartup = pXvdCh->sDecodeSettings.ePCRDiscontinuityModeAtStartup;

      BXDM_PictureProvider_SetPCRMode_isr( pXvdCh->hPictureProvider, &stPCRModes );
   }

   BXDM_PictureProvider_SetChannelSyncMode_isr(
            pXvdCh->hPictureProvider,
            true
            );

   /* SW7445-2490: With the addition of the SW STC, don't force vsync mode when DQT is enabled.*/
#if 0
   /* If DQT is enabled, force VSYNC mode. */
   if ( true == pXvdCh->stDecoderContext.bReversePlayback )
   {
      /* Set the video display mode */
      BXDM_PictureProvider_SetDisplayMode_isr(
            pXvdCh->hPictureProvider,
            BXDM_PictureProvider_DisplayMode_eVirtualTSM
            );
   }
#endif

   BXVD_SetErrorHandlingMode_isr(
            pXvdCh,
            pXvdCh->sDecodeSettings.eErrorHandling
            );

   /* SWSTB-439: set the error threshold, range is 0:100 . */
   BXDM_PictureProvider_SetErrorThreshold_isr(
            pXvdCh->hPictureProvider,
            pXvdCh->sDecodeSettings.uiErrorThreshold
            );

   {
      BXDM_PictureProvider_TrickMode ePictureProviderTrickMode = BXDM_PictureProvider_TrickMode_eAuto;

      /* SWDEPRECATED-1003: needed to turn off the FIC logic during certain trick modes  */

      if ( ( true == pXvdCh->stDecoderContext.bHostSparseMode )
           || ( BXVD_SkipMode_eDecode_IPB != pXvdCh->eCurrentSkipMode )
           || ( true == bDQTEnabled )
         )
      {
         ePictureProviderTrickMode = BXDM_PictureProvider_TrickMode_eSparsePictures;
      }

      BXDM_PictureProvider_SetTrickMode_isr(
            pXvdCh->hPictureProvider,
            ePictureProviderTrickMode
            );
   }


   {
      /* SW7405-4736: the uiInstanceId is used in debug messages to
       * differentiate one channel from another.  Currently it is printed
       * as a 8 bit value.  Set the upper nibble to distinguish between
       * multiple decoders.
       * SWSTB-4655: base the instance ID on pXvdCh->ulChannelNum instead
       * of pXvdCh->sDecodeSettings.uiVDCRectangleNum */

      uint32_t uiInstanceId = pXvdCh->ulChannelNum;

      uiInstanceId |= ( pXvd->uDecoderInstance & 0xF ) << 4 ;

      BXDM_PictureProvider_SetInstanceID_isr(
            pXvdCh->hPictureProvider,
            uiInstanceId
            );
   }

   BXVD_Decoder_StartDecode_isr( pXvdCh );

   {
      /* SW7425-1064: */
      bool bXmoInstalled;

      /* SW7425-1064: true if this is the base channel of a pair of channels with the XMO installed. */
      bXmoInstalled = (( BXVD_P_ChannelType_eBase == pXvdCh->eChannelType ) && ( NULL != pXvdCh->hXmo ));

      if ( true == bXmoInstalled )
      {
         BXDM_PictureProvider_ChannelChangeSettings stChannelChangeSettings;

         /* In order to release pictures properly when the XMO is installed, XDM must release all pictures
          * when StopDecode is call.  The following code forces "hold last picture" to false.
          * - get the current channel change settings.
          * - save the settings in the XMO structure
          * - set "bHoldLastPicture" to false
          * - set the channel change settings with the modified value
          */
         BXDM_PictureProvider_GetChannelChangeSettings_isr(
                        pXvdCh->hPictureProvider,
                        &stChannelChangeSettings );

         BXDM_PictureProvider_XMO_SaveChannelChangeSettings_isr(
                        pXvdCh->hXmo,
                        &stChannelChangeSettings );

         stChannelChangeSettings.bHoldLastPicture = false;

         BXDM_PictureProvider_SetChannelChangeSettings_isr(
                        pXvdCh->hPictureProvider,
                        &stChannelChangeSettings );

         /* Start channel 0 (base) of the XMO. */
         BXDM_PictureProvider_XMO_StartDecode_isr( pXvdCh->hXmo, BXDM_PP_XMO_Base_Index );
      }

      /* SW7425-1064: true if this is the enhanced channel of a pair of channels with the XMO installed. */
      bXmoInstalled = (( BXVD_P_ChannelType_eEnhanced == pXvdCh->eChannelType )
                           && ( NULL != pXvdCh->hXvdChLinked )
                           && ( NULL != pXvdCh->hXvdChLinked->hXmo ));

      if ( true == bXmoInstalled )
      {
         /* SW7425-1064: XDM on the enhanced channel is not used when running with the XMO.
          * Force XDM to release a picture if it is being held from the previous decode.
          * This is the only time to get the picture back for this decode.
          */
         BXDM_PictureProvider_FlushHeldPicture_isr( pXvdCh->hPictureProvider );

         /* Start channel 1 (enhanced) of the XMO. */
         BXDM_PictureProvider_XMO_StartDecode_isr( pXvdCh->hXvdChLinked->hXmo, BXDM_PP_XMO_Enhanced_Index );
      }
   }


   /* SW7250-168: when the channel-change-mode is changed from hold-last-picture
    * to mute while the video decoder is stopped, the held picture is released
    * by the call to BXDM_PictureProvider_StartDecode_isr.  The video decoder state
    * needs to be set to "active" before this call is made to avoid reporting that
    * a picture is being release on an inactive channel.  */
   pXvdCh->eDecoderState = BXVD_P_DecoderState_eActive;

   /* SW7425-1064: only need to start XDM on "standard" and "base" channels.
    * XDM for the "enhanced" channel is not used.
    * SWSTB-3450: switch to the new BXDM_PictureProvider_Start_isr API. */

   if ( BXVD_P_ChannelType_eEnhanced != pXvdCh->eChannelType )
   {
      BXDM_PictureProvider_Start_isr(
         pXvdCh->hPictureProvider,
         &psDecodeSettings->stXDMSettings );
   }

   BKNI_LeaveCriticalSection();

   /* Determine if the PictureProvider (XDM) needs to be registers/unregistered with the
    * DIH (Display Interrupt Handler) of it it needs to be re-enabled.  None of this needs
    * to happen for still pictures.
    */
   if ( pXvdCh->sChSettings.eChannelMode != BXVD_ChannelMode_eStill )
   {
      BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings stXdmDihPictureProviderSettings;
      BXDM_DisplayInterruptHandler_Handle hXdmDih;
      BXDM_DisplayInterruptHandler_Handle hXdmDihCurrent;

      stXdmDihPictureProviderSettings.uiVDCRectangleNumber = pXvdCh->sDecodeSettings.uiVDCRectangleNum;

      /* Determine which DIH to use. */
      if ( pXvd->hAppXdmDih[eDisplayInterrupt] != NULL )

      {
         hXdmDih = pXvd->hAppXdmDih[eDisplayInterrupt];

         BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - selecting pXvd->hAppXdmDih[%d], DIH:0x%0*lx",
                                 eDisplayInterrupt,
                                 BXVD_P_DIGITS_IN_LONG, (long)hXdmDih));
      }
      else if ( pXvd->hLinkedDecoderXdmDih != NULL )
      {
         hXdmDih = pXvd->hLinkedDecoderXdmDih;

         BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - selecting hLinkedDecoderXdmDih, DIH:0x%0*lx",
                               BXVD_P_DIGITS_IN_LONG, (long)hXdmDih));

      }
      else
      {
         hXdmDih = pXvd->hXdmDih[eDisplayInterrupt];

         BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - selecting pXvd->hXdmDih[%d], DIH:0x%0*lx",
                               eDisplayInterrupt,
                               BXVD_P_DIGITS_IN_LONG, (long)hXdmDih));
      }

      BXDM_PictureProvider_GetDIH_isrsafe( pXvdCh->hPictureProvider, &hXdmDihCurrent );

      if ( BXVD_P_ChannelType_eEnhanced == pXvdCh->eChannelType )
      {
         /* SW7425-1064: if this is the "enhanced" channel of a pair of linked channels,
          * only XDM for the base channel needs to be called.  Disable this instance
          * of XDM on the appropriate DIH.
          */
         if ( NULL != hXdmDihCurrent )
         {
            BKNI_EnterCriticalSection();

            BERR_TRACE(BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr(
                       hXdmDihCurrent,
                       BXDM_PictureProvider_GetPicture_isr,
                       pXvdCh->hPictureProvider,
                       BXDM_DisplayInterruptHandler_PictureProviderMode_eDisabled
                       ));

            BKNI_LeaveCriticalSection();

            BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - eEnhanced channel:: disabling XDM:0x%0*lx, on DIH:0x%0*lx",
                                  BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hPictureProvider,
                                  BXVD_P_DIGITS_IN_LONG, (long)hXdmDihCurrent));
         }
      }
      else if ( hXdmDih != hXdmDihCurrent )
      {
         if ( NULL != hXdmDihCurrent )
         {
            BERR_TRACE(BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(
                           hXdmDihCurrent,
                           BXDM_PictureProvider_GetPicture_isr,
                           pXvdCh->hPictureProvider));

            BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - removing XDM:0x%0*lx from DIH:0x%0*lx",
                                  BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hPictureProvider,
                                  BXVD_P_DIGITS_IN_LONG, (long)hXdmDihCurrent));
         }

         if ( pXvdCh->sDecodeSettings.bExternalPictureProviderMode == false )
         {
            /* External picture provider interface is being used for this decode.
             * Do not add this decode to the default DIH */
            BXDM_PictureProvider_SetDIH( pXvdCh->hPictureProvider, hXdmDih );

            rc = BXDM_DisplayInterruptHandler_AddPictureProviderInterface(
                     hXdmDih,
                     BXDM_PictureProvider_GetPicture_isr,
                     pXvdCh->hPictureProvider,
                     &stXdmDihPictureProviderSettings);

            BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - adding XDM:0x%0*lx to DIH:0x%0*lx",
                                  BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hPictureProvider,
                                  BXVD_P_DIGITS_IN_LONG, (long)hXdmDih));
            if (rc != BERR_SUCCESS)
            {
               BXVD_StopDecode(pXvdCh);
               return BERR_TRACE(rc);
            }
         }
      }
      else if ( pXvdCh->sDecodeSettings.bExternalPictureProviderMode == false )
      {
         /* SW7405-3984: the PictureProvider could have been disabled by the middleware.
          * It should always be enabled when StartDecode is called.  If it is an
          * If an external PictureProvider, it is up to the middleware to manage it.
          */
         BKNI_EnterCriticalSection();

         rc = BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr(
                     hXdmDih,
                     BXDM_PictureProvider_GetPicture_isr,
                     pXvdCh->hPictureProvider,
                     BXDM_DisplayInterruptHandler_PictureProviderMode_eEnabled
                     );

         BKNI_LeaveCriticalSection();

         BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - enabling XDM:0x%0*lx, on DIH:0x%0*lx",
                               BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hPictureProvider,
                               BXVD_P_DIGITS_IN_LONG, (long)hXdmDih));
         if (rc != BERR_SUCCESS)
         {
            BXVD_StopDecode(pXvdCh);
            return BERR_TRACE(rc);
         }

      }
   }

   /* Set skip mode if current set mode is not default IPB mode */

   if ((pXvdCh->stDecoderContext.bHostSparseMode == false) &&
       (  pXvdCh->eCurrentSkipMode != BXVD_SkipMode_eDecode_IPB))
   {
      /* Save current Skipmode */
      BXVD_SkipMode ePreservedSkipMode = pXvdCh->eCurrentSkipMode;

      /* Initial skip mode when decode is started is set to IPB (Normal) */
      pXvdCh->eCurrentSkipMode = BXVD_SkipMode_eDecode_IPB;

      /* Set it back to what it was set to since the last BXVD_StopDecode call. */
      if ( BERR_SUCCESS != BXVD_SetSkipPictureModeDecode(pXvdCh, ePreservedSkipMode) )
      {
         BXVD_DBG_WRN(pXvdCh, ("BXVD_StartDecode() - Error restoring skip picture mode"));
      }
   }

   /* PreserveCounters may have been set, now clear it */
   pXvdCh->bPreserveCounters = false;

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - pDisplayInfo: 0x%lx",
                              (unsigned long)pXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pDisplayInfo));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - pPictureDeliveryQueue: 0x%lx",
                              (unsigned long)pXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureDeliveryQueue));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - pPictureReleaseQueue: 0x%lx",
                              (unsigned long)pXvdCh->stChBufferConfig.AVD_DMS2PI_Buffer.pPictureReleaseQueue));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - pAVD_PI2DMS_Buffer: 0x%lx",
                              (unsigned long)pXvdCh->stChBufferConfig.pAVD_PI2DMS_Buffer));

   if ( BXVD_P_ChannelType_eEnhanced != pXvdCh->eChannelType )
   {
      BXVD_Status_StartDecode(pXvd->hXvdStatus, hXvdChannel);
   }

   {
      /* SW7425-1064: */

      bool bStartEnhancedChannel;

      bStartEnhancedChannel = ( BXVD_P_ChannelType_eBase == pXvdCh->eChannelType );
      bStartEnhancedChannel &= ( NULL != pXvdCh->hXvdChLinked );
      bStartEnhancedChannel &= ( NULL != psDecodeSettings->pstEnhancedSettings );

      if ( true == bStartEnhancedChannel )
      {
         BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - base channel, calling StartDecode on enhanced channel 0x%0*lx",
                                       BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hXvdChLinked ));
         rc = BXVD_StartDecode( pXvdCh->hXvdChLinked, psDecodeSettings->pstEnhancedSettings );

         if (rc != BERR_SUCCESS)
         {
            BXVD_DBG_ERR(pXvd, ("BXVD_StartDecoded failed on the enhanced channel, rc = %d", rc ));
            return BERR_TRACE(rc);
         }

      }

   }

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StartDecode() - exit" ));

   BDBG_LEAVE(BXVD_StartDecode);

   return BERR_TRACE(BERR_SUCCESS);
}


/***************************************************************************
BXVD_StopDecode: Stops the decode on a channel.
****************************************************************************/
BERR_Code BXVD_StopDecode(BXVD_ChannelHandle hXvdChannel)
{
   /* BERR_Code eStatus = BERR_SUCCESS; */
   BXVD_P_Context *pXvd;
   BXVD_P_Channel *pXvdCh = (BXVD_P_Channel*)hXvdChannel;

   BXVD_ChannelChangeMode ChChangeMode;
   BXDM_DisplayInterruptHandler_Handle hXdmDihCurrent;

   struct BXVD_P_InterruptCallbackInfo *pCallback;

   BERR_Code rc;
   BXDM_PictureProvider_Counters stCounters;

   BDBG_ENTER(BXVD_StopDecode);

   BDBG_ASSERT(pXvdCh);

   /* Check handle type for correctness */
   if (hXvdChannel->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   pXvd = pXvdCh->pXvd; /* Get XVD Device handle */

#if BXVD_P_CAPTURE_RVC
   BXVD_P_DumpRvc(pXvd);
#endif

   if ( BXVD_P_ChannelType_eEnhanced != pXvdCh->eChannelType )
   {
      BXVD_Status_StopDecode(pXvd->hXvdStatus, hXvdChannel);
   }

   if (pXvdCh->eDecoderState == BXVD_P_DecoderState_eNotActive)
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - Decoder is already inactive"));
      return BERR_TRACE(BXVD_ERR_DECODER_INACTIVE);
   }

   BKNI_EnterCriticalSection();

   BERR_TRACE(BXDM_PictureProvider_GetCounters_isr(
            pXvdCh->hPictureProvider,
            &stCounters
            ));

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - hXvdChannel = 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)hXvdChannel));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - Pictures Delayed: %d", stCounters.uiUnderflowCount));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - Decoder error count: %d", stCounters.uiDecodeErrorCount));
   BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - XDM:0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hPictureProvider));


   /* SW7425-1064: only need to stop XDM on "standard" and "base" channels.
    * XDM for the "enhanced" channel is not being used.
    */
   if ( BXVD_P_ChannelType_eEnhanced != pXvdCh->eChannelType )
   {
      /* SWSTB-3450: switch to the new BXDM_PictureProvider_Stop_isr API.
       * BXVD_StopSettings is not currently used. */
      BXVD_StopSettings stStopSettings;

      BXDM_PictureProvider_Stop_isr( pXvdCh->hPictureProvider, &stStopSettings );
   }

   /* SW7425-1064: when stopping, XDM cannot hold onto the last picture.  This is the only
    * time to get the picture back to the enhanced channel prior to disconnecting the interfaces.
    * To enforce this, the channel change mode was set to NOT "hold last picture" in StartDecode.
    * The following code restores the channel change mode it to the original value.
    * However if a flush, then the last picture should be held, so don't do anything here.
    */
   {
      BXDM_PictureProvider_ChannelChangeSettings stChannelChangeSettings;
      bool bRestoreChannelChangeSettings;

      bRestoreChannelChangeSettings = ( BXVD_P_ChannelType_eBase == pXvdCh->eChannelType );
      bRestoreChannelChangeSettings &= ( NULL != pXvdCh->hXmo );
      bRestoreChannelChangeSettings &= ( false == pXvdCh->bPreserveState );   /* will be true for a flush */

      if ( true == bRestoreChannelChangeSettings )
      {
         BXDM_PictureProvider_XMO_RetrieveChannelChangeSettings_isr(
                  pXvdCh->hXmo,
                  &stChannelChangeSettings );

         BXDM_PictureProvider_SetChannelChangeSettings_isr(
                  pXvdCh->hPictureProvider,
                  &stChannelChangeSettings );
      }
   }

   /* SW7425-1064: Shutdown the XMO filter when stopping the "master" channel.
    * Note: the base channel must be stopped before the enhanced channel.
    */
   if (( NULL != pXvdCh->hXmo ) && ( BXVD_P_ChannelType_eBase == pXvdCh->eChannelType ))
   {
      BXDM_PictureProvider_XMO_StopDecode_isr( pXvdCh->hXmo, 0 );
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - stopping XMO:0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hXmo ));
   }

   BXVD_Decoder_StopDecode_isr( pXvdCh );

   pXvd->auiActiveDecodes[pXvdCh->sChSettings.eChannelMode][pXvdCh->eDisplayInterrupt]--;

   if (pXvdCh->bPreserveState == false)
   {
      BXDM_Picture_FrameRateOverride  stFrameRateOverride;

      pXvdCh->eCurrentSkipMode = BXVD_SkipMode_eDecode_IPB;

      /* SW7405-4733: if state is NOT being preserved, the stPlaybackRate will be
       * set to 1/1 in BXDM_PictureProvider_StopDecode_isr.  Reset the pause state
       * here to keep the logic in sync.
       */
      pXvdCh->bPauseActive = false;

      /* SWDEPRECATED-1003: disable the frame rate override. */
      BKNI_Memset( &stFrameRateOverride, 0, sizeof ( BXDM_Picture_FrameRateOverride ));
      stFrameRateOverride.stRate.uiNumerator = 1;
      stFrameRateOverride.stRate.uiDenominator = 1;

      BERR_TRACE(BXDM_PictureProvider_SetFrameRateOverride_isr(
                  pXvdCh->hPictureProvider,
                  &stFrameRateOverride
                  ));
   }

   BKNI_LeaveCriticalSection();

   /* Close channel and free all outstanding pictures */
   rc = BXVD_P_HostCmdSendDecChannelStop((BXVD_P_Context*)pXvdCh->pXvd, pXvdCh->ulChannelNum);

   pXvdCh->eDecoderState = BXVD_P_DecoderState_eNotActive;

   if (rc == BERR_TIMEOUT)
   {
      pXvd->bWatchdogPending = true;

      /* Notify application if watchdog callback is registered */
      pCallback = &pXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_eWatchdog];

      if (pCallback->BXVD_P_pAppIntCallbackPtr)
      {
         BKNI_EnterCriticalSection();

         pCallback->BXVD_P_pAppIntCallbackPtr(pCallback->pParm1,
                                              pCallback->parm2,
                                              0);
         BKNI_LeaveCriticalSection();
      }
      else
      {
         rc = BXVD_ProcessWatchdog(pXvd);

         if (rc != BERR_SUCCESS)
         {
            BXVD_CloseChannel(pXvdCh);
            return BERR_TRACE(rc);
         }
      }
   }

   BERR_TRACE(BXVD_GetChannelChangeMode( pXvdCh, &ChChangeMode));

   /* SW7425-1064: XDM should already be disabled or disconnnectd for the "enhanced" channnel */

   if (( ChChangeMode == BXVD_ChannelChangeMode_eMute )
        && ( BXVD_P_ChannelType_eEnhanced != pXvdCh->eChannelType ))
   {
      BXDM_PictureProvider_GetDIH_isrsafe( pXvdCh->hPictureProvider, &hXdmDihCurrent );

      if ( NULL != hXdmDihCurrent )
      {
         BKNI_EnterCriticalSection();

         BERR_TRACE(BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr(
                       hXdmDihCurrent,
                       BXDM_PictureProvider_GetPicture_isr,
                       pXvdCh->hPictureProvider,
                       BXDM_DisplayInterruptHandler_PictureProviderMode_eDisabled
                       ));

         BKNI_LeaveCriticalSection();

         BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - disabling XDM:0x%0*lx on DIH:0x%0*lx",
                                    BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hPictureProvider,
                                    BXVD_P_DIGITS_IN_LONG, (long)hXdmDihCurrent ));
      }
   }

   /* SW7425-1064: Now that the "base" channel has been stopped, call
    * BXVD_StopDecode on the "enhanced" channel.
    */
   if ( BXVD_P_ChannelType_eBase == pXvdCh->eChannelType
         && NULL != pXvdCh->hXvdChLinked )
   {
      BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - base channel, calling StopDecode on enhanced channel 0x%0*lx",
                            BXVD_P_DIGITS_IN_LONG, (long)pXvdCh->hXvdChLinked ));

      BXVD_StopDecode( pXvdCh->hXvdChLinked );

#if BXVD_P_USE_TWO_DECODERS
      /* If the XVD handles don't match, the two channels are being decoded
       * on separate decoders.  The "enhanced" decoder interrupts were mucked
       * with in BXVD_StartDecode, restore the interrupts to their original state
       * TODO: this shouldn't need to happen on a flush.
       * TODO: is the correct time for the restore?
       */
      if ( pXvdCh->pXvd != pXvdCh->hXvdChLinked->pXvd )
      {
         /* Set the interrupt settings on the enhanced channel back to their original values. */
         rc = BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration(
                     pXvdCh->hXvdChLinked->pXvd->hXvdDipCh[ pXvdCh->sDecodeSettings.eDisplayInterrupt ],
                     &(pXvdCh->hXvdChLinked->stEnhancedInterruptConfig)
                     );

         /* TODO: Could this call fail? If so, what are the appropriate actions? */
         if (rc != BERR_SUCCESS )
         {
            BXVD_DBG_ERR(pXvdCh, ("BXVD_StopDecode->BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration returned %d on the base decoder", rc));
            return BERR_TRACE(rc);
         }
      }
#endif
   }

   BXVD_DBG_MSG(pXvdCh, ("BXVD_StopDecode() - exit" ));

   BDBG_LEAVE(BXVD_StopDecode);
   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_GetChannelStatus: Requests the channel status from the firmware.
****************************************************************************/
BERR_Code BXVD_GetChannelStatus(BXVD_ChannelHandle hXvdCh,
                                BXVD_ChannelStatus *psChannelStatus )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetChannelStatus);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetChannelStatus_isr(
      hXvdCh,
      psChannelStatus
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetChannelStatus);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetChannelStatus_isr(BXVD_ChannelHandle hXvdCh,
                                    BXVD_ChannelStatus *psChannelStatus )
{
   uint32_t uiPictureDeliveryCount;
   BXDM_PictureProvider_Counters stCounters;
   BXVD_Decoder_Counters         stDecoderCounters;
   const BXDM_Picture * pstPicture;
   BXVD_Handle hXvd;

   BDBG_ENTER(BXVD_GetChannelStatus_isr);
   BDBG_ASSERT(psChannelStatus);

   if (hXvdCh == NULL)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hXvd = hXvdCh->pXvd;

   if ( hXvd == NULL )
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   {
      /* SWSTB-788: move the calculation of the Delivery Queue depth from BXVD_GetChannelStatus_isr to
       * the XVD Decoder logic. BXVD_Decoder_P_GetStatus_isr() provides a method of retrieving the depth. */

      BXVD_Decoder_P_Status stDecoderStatus;

      BXVD_Decoder_P_GetStatus_isr( hXvdCh, &stDecoderStatus );

      uiPictureDeliveryCount = stDecoderStatus.uiPictureCountUnifiedQueue + stDecoderStatus.uiPictureCountDeliveryQueue;
   }

   /*
    * Copy the DM state information into the channel status structure.
    */

   BERR_TRACE(BXDM_PictureProvider_GetCounters_isr(
               hXvdCh->hPictureProvider,
               &stCounters
               ));

   BXDM_PictureProvider_GetCurrentPicturePtr_isr(
               hXvdCh->hPictureProvider,
               &pstPicture
               );

   BERR_TRACE(BXVD_Decoder_GetCounters_isr(
               hXvdCh,
               &stDecoderCounters
               ));

   psChannelStatus->uiDisplayManagerDroppedCount = stCounters.uiDisplayManagerDroppedCount;

   psChannelStatus->uiDecoderDroppedCount = stCounters.uiDecoderDroppedCount;

   psChannelStatus->uiDisplayedParityFailureCount = stCounters.uiDisplayedParityFailureCount;

   psChannelStatus->uiDisplayedCount = stCounters.uiDisplayedCount;

   psChannelStatus->uiPicturesDecodedCount = stCounters.uiPicturesDecodedCount;

   psChannelStatus->uiVsyncUnderflowCount = stCounters.uiVsyncUnderflowCount;

   psChannelStatus->uiDecodeErrorCount = stCounters.uiDecodeErrorCount;

   psChannelStatus->uiVsyncCount = stCounters.uiVsyncCount;

   psChannelStatus->uiIFrameCount = stCounters.uiIFrameCount;

   psChannelStatus->uiErrorIFrameCount = stCounters.uiErrorIFrameCount;

   /* SW7425-3558: use a pointer to save the overhead of a copy. */
   if ( NULL != pstPicture )
   {
      psChannelStatus->eVideoProtocol = pstPicture->stProtocol.eProtocol;
      psChannelStatus->eProtocolLevel = pstPicture->stProtocol.eLevel;
      psChannelStatus->eProtocolProfile = pstPicture->stProtocol.eProfile;
   }
   else
   {
      psChannelStatus->eVideoProtocol = 0;
      psChannelStatus->eProtocolLevel = 0;
      psChannelStatus->eProtocolProfile = 0;
   }

   psChannelStatus->uiPicturesReceivedCount = stCounters.uiPicturesReceivedCount;

   psChannelStatus->ulPictureDeliveryCount = uiPictureDeliveryCount;

   psChannelStatus->ulUnderflowCount = stCounters.uiUnderflowCount;

   psChannelStatus->uiDecoderInputOverflow = stDecoderCounters.uiDecoderInputOverflow;

#if !BXVD_P_HVD_PRESENT

   /*
   ** Retrieve the CABAC bin buffer depth for this channel.
   ** The register offset is calculated as follows;
   **   uiDecode_CabacBinDepth + ( ( 16 + ulChannelNum ) * 4 )
   **
   ** The *4 is since each location is 32-bits (4 bytes).
   */
   {
      uint32_t uiOffset;

      uiOffset = ( hXvdCh->ulChannelNum + 16 ) * 4;
      uiOffset += hXvd->stPlatformInfo.stReg.uiDecode_CabacBinDepth;

      psChannelStatus->uiCabacBinDepth = BXVD_Reg_Read32_isr( hXvdCh->pXvd, uiOffset );
   }

#else

   /* On Rev N and later decoders, Cabac bin fullness is in HIM space */
   BXVD_AVD_P_GET_CABAC_BIN_FULLNESS( hXvdCh, psChannelStatus->uiCabacBinDepth );

#endif

   BXVD_P_GET_VIDEO_DECODER_STATUS( hXvdCh, psChannelStatus->uiAVDStatusBlock );

   BDBG_LEAVE(BXVD_GetChannelStatus_isr);

   return BERR_TRACE(BERR_SUCCESS);

}

/***************************************************************************
BXVD_DisableForFlush: Stops decode on a channel.
****************************************************************************/
BERR_Code BXVD_DisableForFlush(BXVD_ChannelHandle hXvdChannel)
{
   BERR_Code rc;
   BXDM_PictureProvider_PreserveStateSettings stPreserveStateSettings;

   BDBG_ENTER(BXVD_DisableForFlush);

   /* Check handle type for correctness */
   if (hXvdChannel->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Save currently set channel change mode, then set to hold last frame for flush */
   BERR_TRACE(BXDM_PictureProvider_GetDefaultPreserveStateSettings_isrsafe(
            hXvdChannel->hPictureProvider,
            &stPreserveStateSettings
            ));

   BKNI_EnterCriticalSection();

   stPreserveStateSettings.bDisplay = true;

   BERR_TRACE(BXDM_PictureProvider_SetPreserveStateSettings_isr(
            hXvdChannel->hPictureProvider,
            &stPreserveStateSettings
            ));

   BKNI_LeaveCriticalSection();

   /* Don't reset decoder settings when stopping decode for a flush */
   hXvdChannel->bPreserveState = true;

   rc = BERR_TRACE(BXVD_StopDecode(hXvdChannel));

   hXvdChannel->bPreserveState = false;

   BDBG_LEAVE(BXVD_DisableForFlush);
   return BERR_TRACE( rc );
}


/***************************************************************************
BXVD_FlushDecode: Flushes the decode on a channel.
****************************************************************************/
BERR_Code BXVD_FlushDecode(BXVD_ChannelHandle hXvdChannel)
{
   BERR_Code rc;

   BAVC_XptContextMap  XptContextMap;
   BAVC_XptContextMap  aXptContextMap_Extended[BXVD_NUM_EXT_RAVE_CONTEXT];

   uint32_t i;

   BDBG_ENTER(BXVD_FlushDecode);

   /* Check handle type for correctness */
   if (hXvdChannel->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Reset XPT Rave CDB read register address */
   XptContextMap.CDB_Read = hXvdChannel->ulXptCDB_Read;

   hXvdChannel->sDecodeSettings.pContextMap = &XptContextMap;

   for (i = 0; i < hXvdChannel->sDecodeSettings.uiContextMapExtNum; i++)
   {
      hXvdChannel->sDecodeSettings.aContextMapExtended[i] = &aXptContextMap_Extended[i];
      aXptContextMap_Extended[i].CDB_Read = hXvdChannel->aulXptCDB_Read_Extended[i];
   }

   hXvdChannel->bPreserveState = true;

   rc = BERR_TRACE(BXVD_StartDecode(hXvdChannel,
                                    &hXvdChannel->sDecodeSettings));

   hXvdChannel->bPreserveState = false;

   BDBG_LEAVE(BXVD_FlushDecode);
   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_GetDecodeSettings: Returns the last decode settings for a specific
                        channel.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetDecodeSettings(const BXVD_ChannelHandle hXvdCh,
                                 BXVD_DecodeSettings      *psDecodeSettings)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetDecodeSettings);

   rc = BXVD_GetDecodeSettings_isrsafe( hXvdCh, psDecodeSettings );

   BDBG_LEAVE(BXVD_GetDecodeSettings);
   return rc;
}
#endif

BERR_Code BXVD_GetDecodeSettings_isrsafe(const BXVD_ChannelHandle hXvdCh,
                                 BXVD_DecodeSettings      *psDecodeSettings)
{
   BDBG_ENTER(BXVD_GetDecodeSettings_isrsafe);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(psDecodeSettings);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BKNI_Memcpy((void *)psDecodeSettings,
               (void *)&hXvdCh->sDecodeSettings,
               sizeof(BXVD_DecodeSettings));

   BDBG_LEAVE(BXVD_GetDecodeSettings_isrsafe);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_GetTotalChannels: API used to retrieve the max video channels supported.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetTotalChannels(BXVD_Handle hXvd,
                                unsigned    *puiTotalChannels)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetTotalChannels);

   rc = BXVD_GetTotalChannels_isr(
      hXvd,
      puiTotalChannels
      );

   BDBG_LEAVE(BXVD_GetTotalChannels);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetTotalChannels_isr(BXVD_Handle hXvd,
                                    unsigned    *puiTotalChannels)
{
   BDBG_ENTER(BXVD_GetTotalChannels_isr);
   BSTD_UNUSED(hXvd);

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(puiTotalChannels);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   *puiTotalChannels = BXVD_MAX_VIDEO_CHANNELS;

   BDBG_LEAVE(BXVD_GetTotalChannels_isr);
   return BERR_TRACE(BERR_SUCCESS);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
BXVD_GetRevision: API used to retrieve the 7401 FW version
****************************************************************************/
BERR_Code BXVD_GetRevision(BXVD_Handle       hXvd,
                           BXVD_RevisionInfo *psRevInfo)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetRevision);

   rc = BXVD_GetRevision_isr(
      hXvd,
      psRevInfo
      );

   BDBG_LEAVE(BXVD_GetRevision);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetRevision_isr(BXVD_Handle       hXvd,
                               BXVD_RevisionInfo *psRevInfo)
{
   BDBG_ENTER(BXVD_GetRevision_isr);

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(psRevInfo);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   *psRevInfo = hXvd->sRevisionInfo;

   BDBG_LEAVE(BXVD_GetRevision_isr);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_RegisterVdcInterrupt: This API is used to register the VDC interrupt
                           with the video firmware.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_RegisterVdcInterrupt(BXVD_ChannelHandle hXvdCh,
                                    BINT_Id            VDCIntId,
                                    BAVC_Polarity      eFieldPolarity)
{
   uint32_t value;
   BXVD_DisplayInterruptProvider_P_InterruptSettings stInterruptConfig;

   BERR_Code rc;

   BXVD_Handle hXvd;
   BDBG_ENTER(BXVD_RegisterVdcInterrupt);

   BDBG_ASSERT(hXvdCh);
   BSTD_UNUSED(eFieldPolarity);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hXvd = hXvdCh->pXvd;

   /* Compare the beginning of the strings */
   value = BCHP_INT_ID_GET_SHIFT(VDCIntId);

   /* Adjust the value so the parity of the next vsync can be recognized */
   value = 1 << value;

   /* Write the top field polarity registers */

   /* Check to see if the polarity changed and fix it */
   /* ssavekar: 09/20/2005: changing the code to set variables in
      structure "stRULIDMasks" */
   BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration( hXvd->hXvdDipCh[BXVD_DisplayInterrupt_eZero],
                                                              &stInterruptConfig );

   if (eFieldPolarity == BAVC_Polarity_eBotField)
   {
      stInterruptConfig.stRULIDMasks_0.ui32BottomFieldRULIDMask = value ;
   }

   if (eFieldPolarity == BAVC_Polarity_eTopField)
   {
      stInterruptConfig.stRULIDMasks_0.ui32TopFieldRULIDMask = value ;
   }

   if (eFieldPolarity == BAVC_Polarity_eFrame)
   {
      stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = value ;
   }

   /* Link DIH to DIP */
   /* TODO: Allow external DIH to be passed in */
   rc = BXVD_DisplayInterruptProvider_InstallCallback_DisplayInterrupt(
            hXvd->hXvdDipCh[BXVD_DisplayInterrupt_eZero],
            BXDM_DisplayInterruptHandler_Callback_isr,
            hXvd->hXdmDih[BXVD_DisplayInterrupt_eZero] );

   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }

   rc = BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration( hXvd->hXvdDipCh[BXVD_DisplayInterrupt_eZero],
                                                                   &stInterruptConfig );
   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }

   BDBG_LEAVE(BXVD_RegisterVdcInterrupt);

   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************************
BXVD_GetVdcDeviceInterruptDefaultSettings: Gets the DeviceVdcInterrupt default settings
***************************************************************************************/
BERR_Code BXVD_GetVdcDeviceInterruptDefaultSettings
(
   BXVD_Handle                      hXvd,
   BXVD_DeviceVdcInterruptSettings  *pDefDevVdcIntrSettings
   )
{
   BDBG_ENTER(BXVD_GetVdcDeviceInterruptDefaultSettings);
   BSTD_UNUSED(hXvd);

   BDBG_ASSERT(hXvd);

   BDBG_ASSERT(pDefDevVdcIntrSettings);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BKNI_Memcpy((void *)pDefDevVdcIntrSettings,
               (void *)&s_stDefaultDevVdcIntrSettings,
               sizeof(BXVD_DeviceVdcInterruptSettings));

   BDBG_LEAVE(BXVD_GetVdcDeviceInterruptDefaultSettings);

   return BERR_TRACE(BERR_SUCCESS);
}



/***************************************************************************
BXVD_RegisterVdcInterrupt: This API is used to register the VDC interrupt
                           with the video firmware.
****************************************************************************/
BERR_Code BXVD_RegisterVdcDeviceInterrupt
(
   BXVD_Handle                      hXvd,
   BXVD_DeviceVdcInterruptSettings  *pDevVdcIntrSettings
   )
{
   uint32_t uiValue;
   uint32_t uiRegMask = 0;

#if BXVD_P_RUL_DONE_MASK_64_BITS
   bool  bVdcFrameRulReg_0 = true;

   uint32_t uiVdcIntIdReg;
#endif

   BXVD_DisplayInterruptProvider_P_InterruptSettings stInterruptConfig;
   BXDM_DisplayInterruptHandler_Handle hXdmDih;

#if BXVD_P_POWER_MANAGEMENT
   bool bSavedHibernate = false;
#endif

   BERR_Code rc;

   BDBG_ENTER(BXVD_RegisterVdcInterrupt);

   BDBG_ASSERT(hXvd);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* If top/bottom field bits are to be treated as frame for Graphic processing, set frame mask */
   if ((pDevVdcIntrSettings->uiFlags & BXVD_DeviceVdcIntrSettingsFlags_UseFieldAsFrame) &&
       (pDevVdcIntrSettings->VDCIntId_Frame != 0))
   {
      BXVD_DBG_ERR(hXvd, ("Invalid FieldAsFrame mask, Frame bit specified for UseFieldAsFrame"));

      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration( hXvd->hXvdDipCh[pDevVdcIntrSettings->eDisplayInterrupt],
                                                              &stInterruptConfig );

   if (pDevVdcIntrSettings->VDCIntId_Topfield != 0)
   {
      /* Save Top field mask */
      uiValue = BCHP_INT_ID_GET_SHIFT(pDevVdcIntrSettings->VDCIntId_Topfield);

      uiValue = 1 << uiValue;

      uiRegMask = uiValue;

#if BXVD_P_RUL_DONE_MASK_64_BITS

      uiVdcIntIdReg = BCHP_INT_ID_GET_REG(pDevVdcIntrSettings->VDCIntId_Topfield);

      if (uiVdcIntIdReg == hXvd->stPlatformInfo.stReg.uiBvnf_Intr2_0_R5fStatus)
      {
         stInterruptConfig.stRULIDMasks_0.ui32TopFieldRULIDMask = uiValue;
         stInterruptConfig.stRULIDMasks_1.ui32TopFieldRULIDMask = 0;
      }
      else
      {
         stInterruptConfig.stRULIDMasks_0.ui32TopFieldRULIDMask = 0;;
         stInterruptConfig.stRULIDMasks_1.ui32TopFieldRULIDMask = uiValue;
      }

#else
      stInterruptConfig.stRULIDMasks_0.ui32TopFieldRULIDMask = uiValue;
      stInterruptConfig.stRULIDMasks_1.ui32TopFieldRULIDMask = 0;
#endif
   }
   else
   {
      stInterruptConfig.stRULIDMasks_0.ui32TopFieldRULIDMask = 0;
      stInterruptConfig.stRULIDMasks_1.ui32TopFieldRULIDMask = 0;
   }

   if (pDevVdcIntrSettings->VDCIntId_Botfield != 0)
   {
      /* Save bottom field mask */
      uiValue = BCHP_INT_ID_GET_SHIFT(pDevVdcIntrSettings->VDCIntId_Botfield);

      uiValue = 1 << uiValue;

#if BXVD_P_RUL_DONE_MASK_64_BITS

      uiVdcIntIdReg = BCHP_INT_ID_GET_REG(pDevVdcIntrSettings->VDCIntId_Botfield);

      if (uiVdcIntIdReg == hXvd->stPlatformInfo.stReg.uiBvnf_Intr2_0_R5fStatus)
      {
         stInterruptConfig.stRULIDMasks_0.ui32BottomFieldRULIDMask = uiValue;
         stInterruptConfig.stRULIDMasks_1.ui32BottomFieldRULIDMask = 0;
      }
      else
      {
         stInterruptConfig.stRULIDMasks_0.ui32BottomFieldRULIDMask = 0;
         stInterruptConfig.stRULIDMasks_1.ui32BottomFieldRULIDMask = uiValue;
      }
#else
      stInterruptConfig.stRULIDMasks_0.ui32BottomFieldRULIDMask = uiValue;
      stInterruptConfig.stRULIDMasks_1.ui32BottomFieldRULIDMask = 0;

      uiRegMask |= uiValue;
#endif
   }
   else
   {
      stInterruptConfig.stRULIDMasks_0.ui32BottomFieldRULIDMask = 0;
      stInterruptConfig.stRULIDMasks_1.ui32BottomFieldRULIDMask = 0;
   }

   if (pDevVdcIntrSettings->VDCIntId_Frame != 0)
   {
      /* Save bottom field mask */
      uiValue = BCHP_INT_ID_GET_SHIFT(pDevVdcIntrSettings->VDCIntId_Frame);

      uiValue = 1 << uiValue;

#if BXVD_P_RUL_DONE_MASK_64_BITS

      uiVdcIntIdReg = BCHP_INT_ID_GET_REG(pDevVdcIntrSettings->VDCIntId_Frame);

      if (uiVdcIntIdReg == hXvd->stPlatformInfo.stReg.uiBvnf_Intr2_0_R5fStatus)
      {
         stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = uiValue;
         stInterruptConfig.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask = 0;
      }
      else
      {
         bVdcFrameRulReg_0 = false;
         stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = 0;
         stInterruptConfig.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask = uiValue;
      }
#else
      stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = uiValue;
      stInterruptConfig.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask = 0;
#endif
      uiRegMask |= uiValue;
   }
   else
   {
      stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = 0;
      stInterruptConfig.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask = 0;
   }

   /* If top/bottom field bits are to be treated as frame for Graphic processing, set frame mask */
   if (pDevVdcIntrSettings->uiFlags & BXVD_DeviceVdcIntrSettingsFlags_UseFieldAsFrame)
   {
#if BXVD_P_RUL_DONE_MASK_64_BITS
      if (bVdcFrameRulReg_0 == true)
      {
         stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = uiRegMask;
         stInterruptConfig.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask = 0;
      }
      else
      {
         stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = 0;
         stInterruptConfig.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask = uiRegMask;
      }
#else
      stInterruptConfig.stRULIDMasks_0.ui32ProgressiveFieldRULIDMask = uiRegMask;
      stInterruptConfig.stRULIDMasks_1.ui32ProgressiveFieldRULIDMask = 0;
#endif

      stInterruptConfig.stRULIDMasks_0.ui32TopFieldRULIDMask = 0;
      stInterruptConfig.stRULIDMasks_0.ui32BottomFieldRULIDMask = 0;
      stInterruptConfig.stRULIDMasks_1.ui32TopFieldRULIDMask = 0;
      stInterruptConfig.stRULIDMasks_1.ui32BottomFieldRULIDMask = 0;
   }

#if BXVD_P_POWER_MANAGEMENT
   if (hXvd->bHibernate == true)
   {
      bSavedHibernate = true;

      /* Wake up decoder */
      BXVD_P_SetHibernateState(hXvd, false);
   }
#endif

   if (!(pDevVdcIntrSettings->uiFlags & BXVD_DeviceVdcIntrSettingsFlags_Linked))
   {
      /* Clear linked hXvdDih that maybe set */
      hXvd->hLinkedDecoderXdmDih = (BXDM_DisplayInterruptHandler_Handle) NULL;

      /* Link DIH to DIP */
      if (pDevVdcIntrSettings->hAppXdmDih == NULL)
      {
         hXdmDih = hXvd->hXdmDih[pDevVdcIntrSettings->eDisplayInterrupt];
         hXvd->hAppXdmDih[pDevVdcIntrSettings->eDisplayInterrupt] = NULL;
      }
      else
      {
         hXdmDih = pDevVdcIntrSettings->hAppXdmDih;
         hXvd->hAppXdmDih[pDevVdcIntrSettings->eDisplayInterrupt] = hXdmDih;
      }

      rc = BXVD_DisplayInterruptProvider_InstallCallback_DisplayInterrupt(
                hXvd->hXvdDipCh[pDevVdcIntrSettings->eDisplayInterrupt],
                BXDM_DisplayInterruptHandler_Callback_isr,
                hXdmDih );

      if (rc != BERR_SUCCESS )
      {
         return BERR_TRACE(rc);
      }
   }
   else if (pDevVdcIntrSettings->hAppXdmDih != NULL)
   {
      hXdmDih = pDevVdcIntrSettings->hAppXdmDih;
      hXvd->hAppXdmDih[pDevVdcIntrSettings->eDisplayInterrupt] = hXdmDih;
   }

   rc = BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration( hXvd->hXvdDipCh[pDevVdcIntrSettings->eDisplayInterrupt],
                                                                   &stInterruptConfig );
   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }

#if BXVD_P_POWER_MANAGEMENT

   /* If decoder was in hibernate state, or if no external XDN DIHs in use and all channel are closed,
      then put decoder in hibernate state */

   if ((bSavedHibernate == true) ||
       ((hXvd->uiOpenChannels == 0) &&
        ( hXvd->hAppXdmDih[BXVD_DisplayInterrupt_eZero] == NULL) &&
        (hXvd->hAppXdmDih[BXVD_DisplayInterrupt_eTwo] == NULL) &&
        (hXvd->hAppXdmDih[BXVD_DisplayInterrupt_eOne] == NULL)))
   {
      /* Put decoder in hibernate state */
      BXVD_P_SetHibernateState(hXvd, true);
   }

#endif

   BDBG_LEAVE(BXVD_RegisterVdcDeviceInterrupt);

   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_InstallDeviceInterruptCallback: Used to enable and install an application
                                     callback for Device relevant Interrupt.
****************************************************************************/
BERR_Code BXVD_InstallDeviceInterruptCallback(BXVD_Handle           hXvd,
                                              BXVD_DeviceInterrupt  eInterrupt,
                                              BXVD_CallbackFunc     fCallBack_isr,
                                              void                  *pParm1,
                                              int                   parm2)
{
#if BXVD_P_POWER_MANAGEMENT
   bool bSavedHibernate = false;
#endif

   BERR_Code rc;
   struct BXVD_P_InterruptCallbackInfo *callback;

   BDBG_ENTER(BXVD_InstallDeviceInterruptCallback);

   BDBG_ASSERT(hXvd);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if (eInterrupt >= BXVD_DeviceInterrupt_eMaxInterrupts) {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

#if BXVD_P_POWER_MANAGEMENT
   if (hXvd->bHibernate == true)
   {
      bSavedHibernate = true;

      /* Wake up decoder */
      BXVD_P_SetHibernateState(hXvd, false);
   }
#endif

   callback = &hXvd->stDeviceInterruptCallbackInfo[eInterrupt];

   switch (eInterrupt)
   {
      case BXVD_DeviceInterrupt_eWatchdog:

         rc = BERR_TRACE(BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_Watchdog_ISR));
         if (rc != BERR_SUCCESS )
         {
            return BERR_TRACE(rc);
         }
         break ;

      case BXVD_DeviceInterrupt_eVidInstrChecker:

         rc = BERR_TRACE(BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHReg_ISR));
         if (rc != BERR_SUCCESS )
         {
            return BERR_TRACE(rc);
         }

         rc = BERR_TRACE(BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHSCB_ISR));
         if (rc != BERR_SUCCESS )
         {
            return BERR_TRACE(rc);
         }

         rc = BERR_TRACE(BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHInstrRd_ISR));
         if (rc != BERR_SUCCESS )
         {
            return BERR_TRACE(rc);
         }

#if BXVD_P_AVD_ARC600
         /* VICH Inner Loop Instruction read violation */
         rc = BERR_TRACE(BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHILInstrRd_ISR));
         if (rc != BERR_SUCCESS)
         {
            return BERR_TRACE(rc);
         }

         if (hXvd->bSVCCapable == true)
         {
            /* VICH Base Layer Instruction read violation */
            rc = BERR_TRACE(BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_VICHBLInstrRd_ISR));
            if (rc != BERR_SUCCESS)
            {
               return BERR_TRACE(rc);
            }
         }
#endif
         break ;

      case BXVD_DeviceInterrupt_eStereoSeqError:

         rc = BERR_TRACE(BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR));
         if (rc != BERR_SUCCESS )
         {
            return BERR_TRACE(rc);
         }

         break ;

      default:
         break;
   }

   switch (eInterrupt)
   {
      case BXVD_DeviceInterrupt_eDisplayInterrupt0:
         BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eZero],
                  (BXDM_DisplayInterruptHandler_Display_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;

      case BXVD_DeviceInterrupt_eDisplayInterrupt1:
         BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eOne],
                  (BXDM_DisplayInterruptHandler_Display_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;

#if BXVD_P_PICTURE_DATA_RDY_2_SUPPORTTED
      case BXVD_DeviceInterrupt_eDisplayInterrupt2:
         BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eTwo],
                  (BXDM_DisplayInterruptHandler_Display_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
#endif

      case BXVD_DeviceInterrupt_ePictureDataReady0:
         BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eZero],
                  (BXDM_DisplayInterruptHandler_PictureDataReady_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;

      case BXVD_DeviceInterrupt_ePictureDataReady1:
         BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eOne],
                  (BXDM_DisplayInterruptHandler_PictureDataReady_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;

#if BXVD_P_PICTURE_DATA_RDY_2_SUPPORTTED
     case BXVD_DeviceInterrupt_ePictureDataReady2:
         BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eTwo],
                  (BXDM_DisplayInterruptHandler_PictureDataReady_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
#endif

      default:
         BKNI_EnterCriticalSection();
         callback->BXVD_P_pAppIntCallbackPtr = (BXVD_IntCallbackFunc)fCallBack_isr;
         callback->pParm1 = pParm1;
         callback->parm2 = parm2;
         BKNI_LeaveCriticalSection();
         break;
   }

#if BXVD_P_POWER_MANAGEMENT

   if (bSavedHibernate == true)
   {
      /* Put decoder back in hibernate state */
      BXVD_P_SetHibernateState(hXvd, true);
   }
#endif

   BDBG_LEAVE(BXVD_InstallDeviceInterruptCallback);

   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_UnInstallInterruptCallback: Used to disable and un-install an application
                                 callback for a device relevant Interrupt.
****************************************************************************/
BERR_Code BXVD_UnInstallDeviceInterruptCallback(BXVD_Handle           hXvd,
                                                BXVD_DeviceInterrupt  eInterrupt)
{
   struct BXVD_P_InterruptCallbackInfo *callback;


   BERR_Code eStatus = BERR_SUCCESS;

#if BXVD_P_POWER_MANAGEMENT
   bool bSavedHibernate = false;
#endif

   BDBG_ENTER(BXVD_UnInstallDeviceInterruptCallback);

   BDBG_ASSERT(hXvd);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if (eInterrupt >= BXVD_DeviceInterrupt_eMaxInterrupts) {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

#if BXVD_P_POWER_MANAGEMENT
   if (hXvd->bHibernate == true)
   {
      bSavedHibernate = true;

      /* Wake up decoder */
      BXVD_P_SetHibernateState(hXvd, false);
   }
#endif


   callback = &hXvd->stDeviceInterruptCallbackInfo[eInterrupt];

   switch (eInterrupt)
   {
      case BXVD_DeviceInterrupt_eWatchdog:
         eStatus = BERR_TRACE(BINT_DisableCallback((hXvd->stDecoderContext.pCbAVC_Watchdog_ISR)));
         break ;

      case BXVD_DeviceInterrupt_eVidInstrChecker:

         eStatus = BERR_TRACE(BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHReg_ISR));
         if (eStatus != BERR_SUCCESS )
         {
            return BERR_TRACE(eStatus);
         }

         eStatus = BERR_TRACE(BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHSCB_ISR));
         if (eStatus != BERR_SUCCESS )
         {
            return BERR_TRACE(eStatus);
         }

         eStatus = BERR_TRACE(BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHInstrRd_ISR));
         if (eStatus != BERR_SUCCESS )
         {
            return BERR_TRACE(eStatus);
         }

#if BXVD_P_AVD_ARC600
         /* VICH Inner Loop Instruction read violation */
         eStatus = BERR_TRACE(BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHILInstrRd_ISR));
         if (eStatus != BERR_SUCCESS)
         {
            return BERR_TRACE(eStatus);
         }

         if (hXvd->bSVCCapable == true)
         {
            /* VICH Base Layer Instruction read violation */
            eStatus = BERR_TRACE(BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_VICHBLInstrRd_ISR));
            if (eStatus!= BERR_SUCCESS)
            {
               return BERR_TRACE(eStatus);
            }
         }
#endif
         break;

      case BXVD_DeviceInterrupt_eStereoSeqError:

         eStatus = BERR_TRACE(BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_StereoSeqError_ISR));
         if (eStatus != BERR_SUCCESS )
         {
            return BERR_TRACE(eStatus);
         }

         break ;

      default:
         break;
   }

   switch (eInterrupt)
   {
      case BXVD_DeviceInterrupt_eDisplayInterrupt0:
         BXDM_DisplayInterruptHandler_UnInstallCallback_DisplayInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eZero]);
         break;

      case BXVD_DeviceInterrupt_eDisplayInterrupt1:
         BXDM_DisplayInterruptHandler_UnInstallCallback_DisplayInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eOne]);
         break;

#if BXVD_P_PICTURE_DATA_RDY_2_SUPPORTTED
      case BXVD_DeviceInterrupt_eDisplayInterrupt2:
         BXDM_DisplayInterruptHandler_UnInstallCallback_DisplayInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eTwo]);
         break;
#endif

      case BXVD_DeviceInterrupt_ePictureDataReady0:
         BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eZero]);
         break;

      case BXVD_DeviceInterrupt_ePictureDataReady1:
         BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eOne]);
         break;

#if BXVD_P_PICTURE_DATA_RDY_2_SUPPORTTED
      case BXVD_DeviceInterrupt_ePictureDataReady2:
         BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(
                  hXvd->hXdmDih[BXVD_DisplayInterrupt_eTwo]);
         break;
#endif

      default:
         break;
   }
   BKNI_EnterCriticalSection();
   callback->BXVD_P_pAppIntCallbackPtr = NULL;
   callback->pParm1 = NULL;
   callback->parm2 = 0;
   BKNI_LeaveCriticalSection();

#if BXVD_P_POWER_MANAGEMENT

   if (bSavedHibernate == true)
   {
      /* Put decoder back in hibernate state */
      BXVD_P_SetHibernateState(hXvd, true);
   }
#endif

   BDBG_LEAVE(BXVD_UnInstallDeviceInterruptCallback);

   return eStatus;
}

static void BXVD_S_FirstPtsReady_XDMCompatibility_isr(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo )
{
   BXVD_ChannelHandle hXvdCh = (BXVD_ChannelHandle) pPrivateContext;
   BXVD_Interrupt eInterrupt = (BXVD_Interrupt) iPrivateParam;
   struct BXVD_P_InterruptCallbackInfo *callback = &hXvdCh->stInterruptCallbackInfo[eInterrupt];
   BXDM_PictureProvider_PTSInfo stPTSInfo = *pstPTSInfo;

   /* The XVD FirstPTSReady callback is overloaded.  This logic implements the overloaded functionality
    * using the explicit XDM FirstPTSReady and FirstCodedPTSReady callbacks
    */
   if ( BXDM_PictureProvider_PTSType_eCoded == pstPTSInfo->ePTSType )
   {
      /* We already have a coded PTS, so we DISABLE the XDM FirstCodedPTSReady callback so that the
       * XVD FirstPTSReady callback is backwards compatible.
       */
      BXDM_PictureProvider_Callback_SetEnable_isr(
               hXvdCh->hPictureProvider,
               BXDM_PictureProvider_Callback_eFirstCodedPTSReady,
               false
               );
   }
   else
   {
      /* We don't have a coded PTS, so we ENABLE the XDM FirstCodedPTSReady callback so that the
       * XVD FirstPTSReady callback is backwards compatible.  When we get a coded PTS, XDM will
       * call back into the app accordingly
       */
      BXDM_PictureProvider_Callback_SetEnable_isr(
               hXvdCh->hPictureProvider,
               BXDM_PictureProvider_Callback_eFirstCodedPTSReady,
               true
               );
   }

   /* Forward PTS info to the FirstPTSReady app callback */
   if ( NULL != callback->BXVD_P_pAppIntCallbackPtr )
   {
      callback->BXVD_P_pAppIntCallbackPtr(
               callback->pParm1,
               callback->parm2,
               &stPTSInfo);
   }

   return;
}

static void BXVD_S_ClipEvent_XDMCompatibility_isr(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_ClipEventInfo *pstClipEventInfo )
{
   BXVD_ChannelHandle hXvdCh = (BXVD_ChannelHandle) pPrivateContext;
   BXVD_Interrupt eInterrupt = (BXVD_Interrupt) iPrivateParam;
   struct BXVD_P_InterruptCallbackInfo *callback = &hXvdCh->stInterruptCallbackInfo[eInterrupt];
   uint32_t uiPTS = pstClipEventInfo->uiPTS;

   /* Forward PTS info to the FirstPTSReady app callback */
   if ( NULL != callback->BXVD_P_pAppIntCallbackPtr )
   {
      callback->BXVD_P_pAppIntCallbackPtr(
               callback->pParm1,
               callback->parm2,
               &uiPTS);
   }

   return;
}

static void BXVD_S_RequestSTC_XDMCompatibility_isr(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo )
{
   BXVD_ChannelHandle hXvdCh = (BXVD_ChannelHandle) pPrivateContext;
   BXVD_Interrupt eInterrupt = (BXVD_Interrupt) iPrivateParam;
   struct BXVD_P_InterruptCallbackInfo *callback = &hXvdCh->stInterruptCallbackInfo[eInterrupt];
   uint32_t uiPTS = pstPTSInfo->ui32RunningPTS;

   /* Forward PTS info to the FirstPTSReady app callback */
   if ( NULL != callback->BXVD_P_pAppIntCallbackPtr )
   {
      callback->BXVD_P_pAppIntCallbackPtr(
               callback->pParm1,
               callback->parm2,
               &uiPTS);
   }

   return;
}

static void BXVD_S_DecodeError_XDMCompatibility_isr(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_DecodeErrorInfo *pstDecodeErrorInfo
         )
{
   BXVD_ChannelHandle hXvdCh = (BXVD_ChannelHandle) pPrivateContext;
   BXVD_Interrupt eInterrupt = (BXVD_Interrupt) iPrivateParam;
   struct BXVD_P_InterruptCallbackInfo *callback = &hXvdCh->stInterruptCallbackInfo[eInterrupt];
   uint32_t uiDecodeErrorCount = pstDecodeErrorInfo->uiDecodeErrorCount;

   /* Forward PTS info to the FirstPTSReady app callback */
   if ( NULL != callback->BXVD_P_pAppIntCallbackPtr )
   {
      callback->BXVD_P_pAppIntCallbackPtr(
               callback->pParm1,
               callback->parm2,
               &uiDecodeErrorCount);
   }

   return;
}

void BXVD_SetPictureParameterInfo_isrsafe(BXVD_PictureParameterInfo *pInfo, const BAVC_MFD_Picture *pstMFDPicture, const BXDM_Picture *pstUnifiedPicture)
{
   BKNI_Memset(pInfo, 0, sizeof(*pInfo));
   /*
    * Not all elements in "*pstPPInfo" are set.
    *
    * Two of associated PR's from V1:  PR28082 and PR31593.
    *
    * Note: in V1, the "PictureParameterInfo" structure was updated in
    * "BXVD_P_PreparePictureParametersDataStruct_isr()" and
    * "BXVD_P_PrepareDataStructForVDC_isr()"
    *
    */
   pInfo->eAspectRatio = pstMFDPicture->eAspectRatio;
   pInfo->eColorPrimaries = pstMFDPicture->eColorPrimaries;
   pInfo->eFrameRateCode = pstMFDPicture->eFrameRateCode;
   pInfo->eMatrixCoefficients = pstMFDPicture->eMatrixCoefficients;
   pInfo->eTransferCharacteristics = pstMFDPicture->eTransferCharacteristics;
   /*pInfo->ePreferredTransferCharacteristics = pstMFDPicture->ePreferredTransferCharacteristics;*/ /* SWSTB-1629 */
   pInfo->uiSampleAspectRatioX = pstMFDPicture->uiSampleAspectRatioX;
   pInfo->uiSampleAspectRatioY = pstMFDPicture->uiSampleAspectRatioY;
   pInfo->ulSourceHorizontalSize = pstMFDPicture->ulSourceHorizontalSize;
   pInfo->ulSourceVerticalSize = pstMFDPicture->ulSourceVerticalSize;
   pInfo->ulDisplayHorizontalSize = pstMFDPicture->ulDisplayHorizontalSize;
   pInfo->ulDisplayVerticalSize = pstMFDPicture->ulDisplayVerticalSize;
   pInfo->i32_HorizontalPanScan = pstMFDPicture->i32_HorizontalPanScan;
   pInfo->i32_VerticalPanScan = pstMFDPicture->i32_VerticalPanScan;

   pInfo->bFrameProgressive = pstMFDPicture->bFrameProgressive;

   pInfo->bStreamProgressive = ( BXDM_Picture_Sequence_eProgressive == pstUnifiedPicture->stPictureType.eSequence );

   pInfo->uiProfile = pstUnifiedPicture->stProtocol.eProfile;
   pInfo->uiLevel = pstUnifiedPicture->stProtocol.eLevel;

   /* SW7550-177: copy the Afd info into the picture parameter struct. */
   pInfo->bValidAfd = pstMFDPicture->bValidAfd;
   pInfo->uiAfd = pstMFDPicture->ulAfd;

   /* Copy HEVC HDR info.  SWSTB-2902: HDR data is copied directly from
    * PPB->Unified Picture->MFD structure. The data being returned
    * in the callback can come from either the Unified Picture or the MFD structure.
    * Previously it was coming from the MFD structure.  This is broken
    * when we are returning data for the first picture and it hasn't passed
    * TSM, i.e the MFD.HDR fields have not been updated in BXDM_PPOUT_P_CalculateVdcData_isr.
    * So copy the data here from the Unified Picture. */

   pInfo->ulAvgContentLight = pstUnifiedPicture->stHDR.ulAvgContentLight;
   pInfo->ulMaxContentLight = pstUnifiedPicture->stHDR.ulMaxContentLight;
   pInfo->stDisplayPrimaries[0] = pstUnifiedPicture->stHDR.stDisplayPrimaries[0];
   pInfo->stDisplayPrimaries[1] = pstUnifiedPicture->stHDR.stDisplayPrimaries[1];
   pInfo->stDisplayPrimaries[2] = pstUnifiedPicture->stHDR.stDisplayPrimaries[2];
   pInfo->stWhitePoint = pstUnifiedPicture->stHDR.stWhitePoint;
   pInfo->ulMaxDispMasteringLuma = pstUnifiedPicture->stHDR.ulMaxDispMasteringLuma;
   pInfo->ulMinDispMasteringLuma = pstUnifiedPicture->stHDR.ulMinDispMasteringLuma;
   pInfo->ePreferredTransferCharacteristics = pstUnifiedPicture->stHDR.uiTransferCharacteristics; /* SWSTB-1629 */

   pInfo->uiMacroBlockCntInter = pstUnifiedPicture->stStats.stMacroBlockCount.uiInterCoded;
   pInfo->uiMacroBlockCntIntra = pstUnifiedPicture->stStats.stMacroBlockCount.uiIntraCoded;
   pInfo->uiMacroBlockCntTotal = pstUnifiedPicture->stStats.stMacroBlockCount.uiTotal;

   pInfo->uiPictureCodingType = pstUnifiedPicture->stPictureType.eCoding;

   pInfo->uiBitRate = pstUnifiedPicture->stStats.uiBitRate;
   pInfo->uiLowDelayFlag = pstUnifiedPicture->stPictureType.bLowDelay;
   pInfo->uiVideoFormat = pstUnifiedPicture->stDisplayInfo.eVideoFormat;

   pInfo->uiFrameRateExtN = pstUnifiedPicture->stFrameRate.stExtension.uiNumerator;
   pInfo->uiFrameRateExtD = pstUnifiedPicture->stFrameRate.stExtension.uiDenominator;

   /* SW3556-836: dereference the fixed_frame_rate_flag */
   pInfo->uiFixedFrameRateFlag = ( BXDM_Picture_FrameRateType_eFixed == pstUnifiedPicture->stFrameRate.eType );

   /* SW7405-4378: return the unaltered width and height for use by the application. */
   pInfo->uiCodedSourceWidth = pstUnifiedPicture->stBufferInfo.stSource.uiWidth;
   pInfo->uiCodedSourceHeight = pstUnifiedPicture->stBufferInfo.stSource.uiHeight;

   /* SW7445-1023: report the depth of the picture buffers */
   pInfo->eBitDepth = pstMFDPicture->eBitDepth;
}

static void BXVD_S_PictureParameters_XDMCompatibility_isr(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BXDM_PictureProvider_Callback_PictureParameterInfo *pstPictureParameters
         )
{
   BXVD_ChannelHandle hXvdCh = (BXVD_ChannelHandle) pPrivateContext;
   BXVD_Interrupt eInterrupt = (BXVD_Interrupt) iPrivateParam;
   struct BXVD_P_InterruptCallbackInfo *callback = &hXvdCh->stInterruptCallbackInfo[eInterrupt];

   BDBG_ASSERT(pstPictureParameters);
   BDBG_ASSERT(pstPictureParameters->pstUnifiedPicture);
   BDBG_ASSERT(pstPictureParameters->pstMFDPicture);

   /* bProgressiveStream_7411 is sticky.  It is updated only when a
    * source change has been detected. A source change is a change in
    * any of the following parameters
    *
    *  - Frame Rate
    *  - Resolution
    *  - Aspect Ratio
    *  - Color Primaries
    */
   if ((hXvdCh->stPictureParameterInfo.eFrameRateCode != pstPictureParameters->pstMFDPicture->eFrameRateCode)
       || (hXvdCh->stPictureParameterInfo.ulSourceHorizontalSize != pstPictureParameters->pstMFDPicture->ulSourceHorizontalSize)
       || (hXvdCh->stPictureParameterInfo.ulSourceVerticalSize != pstPictureParameters->pstMFDPicture->ulSourceVerticalSize)
       || (hXvdCh->stPictureParameterInfo.eAspectRatio != pstPictureParameters->pstMFDPicture->eAspectRatio)
       || (hXvdCh->stPictureParameterInfo.eColorPrimaries != pstPictureParameters->pstMFDPicture->eColorPrimaries)
      )
   {
      hXvdCh->bProgressiveStream_7411 = ( BXDM_Picture_Sequence_eProgressive == pstPictureParameters->pstUnifiedPicture->stPictureType.eSequence );
   }

   BXVD_SetPictureParameterInfo_isrsafe(&hXvdCh->stPictureParameterInfo, pstPictureParameters->pstMFDPicture, pstPictureParameters->pstUnifiedPicture);
   hXvdCh->stPictureParameterInfo.bStreamProgressive_7411 = hXvdCh->bProgressiveStream_7411;

   /* Forward Picture Parameter info to the PictureParameter app callback */
   if ( NULL != callback->BXVD_P_pAppIntCallbackPtr )
   {
      callback->BXVD_P_pAppIntCallbackPtr(
               callback->pParm1,
               callback->parm2,
               &hXvdCh->stPictureParameterInfo);
   }

   return;
}

/***************************************************************************
BXVD_InstallInterruptCallback: Used to enable and install an application
                               callback for a channel relevant Interrupt.
****************************************************************************/
BERR_Code BXVD_InstallInterruptCallback(BXVD_ChannelHandle hXvdCh,
                                        BXVD_Interrupt     eInterrupt,
                                        BXVD_CallbackFunc  fCallBack_isr,
                                        void               *pParm1,
                                        int                parm2)
{
   struct BXVD_P_InterruptCallbackInfo *callback;

   BDBG_ENTER(BXVD_InstallInterruptCallback);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if (eInterrupt >= BXVD_Interrupt_eMaxInterrupts)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   if ( eInterrupt == BXVD_Interrupt_ePictureDataReady)
   {
      callback = &hXvdCh->pXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_ePictureDataReady0];
   }
   else
   {
      callback = &hXvdCh->stInterruptCallbackInfo[eInterrupt];
   }

   switch ( eInterrupt )
   {
      case BXVD_Interrupt_ePictureDataReady:
         BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(
                  hXvdCh->pXvd->hXdmDih[BXVD_DisplayInterrupt_eZero],
                  (BXDM_DisplayInterruptHandler_PictureDataReady_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      default:
         break;
   }

   BKNI_EnterCriticalSection();
   switch ( eInterrupt )
   {
      case BXVD_Interrupt_ePtsStcOffset:
         BXDM_PictureProvider_Callback_Install_StcPtsOffset_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_StcPtsOffset_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      case BXVD_Interrupt_eFirstPTSReady:
         BXDM_PictureProvider_Callback_Install_FirstPTSReady_isr(
                  hXvdCh->hPictureProvider,
                  BXVD_S_FirstPtsReady_XDMCompatibility_isr,
                  hXvdCh,
                  eInterrupt);

         BXDM_PictureProvider_Callback_Install_FirstCodedPTSReady_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_FirstCodedPTSReady_isr) fCallBack_isr,
                  pParm1,
                  parm2);

         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eFirstCodedPTSReady,
                  false
                  );
         break;
      case BXVD_Interrupt_eFirstPTSPassed:
         BXDM_PictureProvider_Callback_Install_FirstPTSPassed_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_FirstPTSPassed_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      case BXVD_Interrupt_ePTSError:
         BXDM_PictureProvider_Callback_Install_PTSError_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_PTSError_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      case BXVD_Interrupt_eIFrame:
         BXDM_PictureProvider_Callback_Install_IFrame_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_IFrame_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      case BXVD_Interrupt_ePictureParameters:
         BXDM_PictureProvider_Callback_Install_PictureParameters_isr(
                  hXvdCh->hPictureProvider,
                  BXVD_S_PictureParameters_XDMCompatibility_isr,
                  hXvdCh,
                  eInterrupt);
         break;
      case BXVD_Interrupt_eTSMPassInASTMMode:
         BXDM_PictureProvider_Callback_Install_TSMPassInASTMMode_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_TSMPassInASTMMode_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      case BXVD_Interrupt_eClipStart:
         BXDM_PictureProvider_Callback_Install_ClipStart_isr(
                  hXvdCh->hPictureProvider,
                  BXVD_S_ClipEvent_XDMCompatibility_isr,
                  hXvdCh,
                  eInterrupt);
         break;
      case BXVD_Interrupt_eClipStop:
         BXDM_PictureProvider_Callback_Install_ClipStop_isr(
                  hXvdCh->hPictureProvider,
                  BXVD_S_ClipEvent_XDMCompatibility_isr,
                  hXvdCh,
                  eInterrupt);
         break;
      case BXVD_Interrupt_ePictureMarker:
         BXDM_PictureProvider_Callback_Install_PictureMarker_isr(
                  hXvdCh->hPictureProvider,
                  BXVD_S_ClipEvent_XDMCompatibility_isr,
                  hXvdCh,
                  eInterrupt);
         break;
      case BXVD_Interrupt_eRequestSTC:
         BXDM_PictureProvider_Callback_Install_RequestSTC_isr(
                  hXvdCh->hPictureProvider,
                  BXVD_S_RequestSTC_XDMCompatibility_isr,
                  hXvdCh,
                  eInterrupt);
         break;
      case BXVD_Interrupt_ePPBParameters:
         BXDM_PictureProvider_Callback_Install_PictureUnderEvaluation_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_PictureUnderEvaluation_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      case BXVD_Interrupt_eTSMResult:
         BXDM_PictureProvider_Callback_Install_TSMResult_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_TSMResult_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;
      case BXVD_Interrupt_ePictureExtensionData:
         BXDM_PictureProvider_Callback_Install_PictureExtensionData_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_PictureExtensionData_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;

      case BXVD_Interrupt_eDecodeError:
         BXDM_PictureProvider_Callback_Install_DecodeError_isr(
                  hXvdCh->hPictureProvider,
                  BXVD_S_DecodeError_XDMCompatibility_isr,
                  hXvdCh,
                  eInterrupt);
         break;

      case BXVD_Interrupt_eChunkDone:
         BXDM_PictureProvider_Callback_Install_ChunkDone_isr(
                  hXvdCh->hPictureProvider,
                  (BXDM_PictureProvider_Callback_ChunkDone_isr) fCallBack_isr,
                  pParm1,
                  parm2);
         break;

      default:
         break;
   }
   callback->BXVD_P_pAppIntCallbackPtr = (BXVD_IntCallbackFunc)fCallBack_isr;
   callback->pParm1 = pParm1;
   callback->parm2 = parm2;

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_InstallInterruptCallback);

   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
BXVD_UnInstallInterruptCallback: Used to disable and un-install an application
                                 callback for a channel relevant interrupt.
****************************************************************************/
BERR_Code BXVD_UnInstallInterruptCallback(BXVD_ChannelHandle hXvdCh,
                                          BXVD_Interrupt     eInterrupt)
{
   struct BXVD_P_InterruptCallbackInfo *callback;

   BDBG_ENTER(BXVD_UnInstallInterruptCallback);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if (eInterrupt >= BXVD_Interrupt_eMaxInterrupts)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }


   if ( eInterrupt == BXVD_Interrupt_ePictureDataReady)
   {
      callback = &hXvdCh->pXvd->stDeviceInterruptCallbackInfo[BXVD_DeviceInterrupt_ePictureDataReady0];
   }
   else
   {
      callback = &hXvdCh->stInterruptCallbackInfo[eInterrupt];
   }

   BKNI_EnterCriticalSection();
   switch ( eInterrupt )
   {
      case BXVD_Interrupt_ePtsStcOffset:
         BXDM_PictureProvider_Callback_UnInstall_StcPtsOffset_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_ePictureDataReady:
         BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(
                  hXvdCh->pXvd->hXdmDih[BXVD_DisplayInterrupt_eZero]);
         break;
      case BXVD_Interrupt_eFirstPTSReady:
         BXDM_PictureProvider_Callback_UnInstall_FirstPTSReady_isr(
                  hXvdCh->hPictureProvider);
         BXDM_PictureProvider_Callback_UnInstall_FirstCodedPTSReady_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eFirstPTSPassed:
         BXDM_PictureProvider_Callback_UnInstall_FirstPTSPassed_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_ePTSError:
         BXDM_PictureProvider_Callback_UnInstall_PTSError_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eIFrame:
         BXDM_PictureProvider_Callback_UnInstall_IFrame_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_ePictureParameters:
         BXDM_PictureProvider_Callback_UnInstall_PictureParameters_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eTSMPassInASTMMode:
         BXDM_PictureProvider_Callback_UnInstall_TSMPassInASTMMode_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eClipStart:
         BXDM_PictureProvider_Callback_UnInstall_ClipStart_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eClipStop:
         BXDM_PictureProvider_Callback_UnInstall_ClipStop_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_ePictureMarker:
         BXDM_PictureProvider_Callback_UnInstall_PictureMarker_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eRequestSTC:
         BXDM_PictureProvider_Callback_UnInstall_RequestSTC_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_ePPBParameters:
         BXDM_PictureProvider_Callback_UnInstall_PictureUnderEvaluation_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eTSMResult:
         BXDM_PictureProvider_Callback_UnInstall_TSMResult_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_ePictureExtensionData:
         BXDM_PictureProvider_Callback_UnInstall_PictureExtensionData_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eDecodeError:
         BXDM_PictureProvider_Callback_UnInstall_DecodeError_isr(
                  hXvdCh->hPictureProvider);
         break;
      case BXVD_Interrupt_eChunkDone:
         BXDM_PictureProvider_Callback_UnInstall_ChunkDone_isr(
                  hXvdCh->hPictureProvider);
         break;
      default:
         break;
   }

   callback->BXVD_P_pAppIntCallbackPtr = NULL;
   callback->pParm1 = NULL;
   callback->parm2 = 0;
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_UnInstallInterruptCallback);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
 * BXVD_GetBufferConfig: returns the buffer requirements for CDB and ITB.
 ****************************************************************************/
BERR_Code BXVD_GetBufferConfig(BXVD_Handle       hXvd,
                               BAVC_CdbItbConfig *pCdbItbConfigInfo,
                               int32_t           *pPicBufLength)
{
   BERR_Code ret = BERR_SUCCESS;

   BDBG_ENTER (BXVD_GetBufferConfig);

   /* Check input parameters */
   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(pCdbItbConfigInfo);
   BDBG_ASSERT(pPicBufLength);

   if ((hXvd == NULL)||(pCdbItbConfigInfo == NULL)||(pPicBufLength == NULL))
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   *pCdbItbConfigInfo = sCdbItbCfg[0];

   *pPicBufLength = 0;

   BDBG_LEAVE (BXVD_GetBufferConfig);
   return BERR_TRACE(ret);
}

/***************************************************************************
 Summary: BXVD_GetFWMemConfigDefaultSettings

 Description:
    Returns the default values for BXVD_FWMemConfigSettings;

 Returns:
    BERR_SUCCESS  Values set successfully.
    BERR_INVALID_PARAMETER  Bad input parameter
**************************************************************************/
BERR_Code BXVD_GetFWMemConfigDefaultSettings(BXVD_FWMemConfigSettings  *pFWMemConfigSettings) /* [out] memory configuration settings */
{
   BDBG_ENTER (BXVD_GetFWMemConfigDefaultSettings);

   BDBG_ASSERT(pFWMemConfigSettings);

   if (pFWMemConfigSettings == NULL)
   {
      return BERR_INVALID_PARAMETER;
   }

   BKNI_Memset((void*)pFWMemConfigSettings, BXVD_P_MEM_ZERO, sizeof(BXVD_FWMemConfigSettings));

   BDBG_LEAVE (BXVD_GetFWMemConfigDefaultSettings);

   return BERR_SUCCESS;
}

/***************************************************************************
 Summary: BXVD_GetChannelMemoryParameters
    Returns the FW Memory needed for the specified channel settings.

 Description:
    Using the specified channel settings resolution, decode mode and system DDR information
    the FW memory configuration is determined and returned.

 Returns:
    BERR_SUCCESS  Memory configuration generated successfully.
    BERR_INVALID_PARAMETER  Bad input parameter
**************************************************************************/

BERR_Code BXVD_GetChannelMemoryParameters(const BXVD_ChannelSettings      *pChSettings,          /* [in] channel settings */
                                          const BXVD_FWMemConfigSettings  *pFWMemConfigSettings, /* [in] memory configuration settings */
                                          BXVD_FWMemConfig                *pFWMemConfig)         /* [out] memory configuration for channel */
{
   BXVD_P_Context *pXvd;

   BCHP_DramType ddrType;
   unsigned MemcIndex, memPartSize, memBusWidth, memDeviceWidth;
   unsigned stripeWidth, bankHeight;
   bool bDDRGroupageEnabled;


   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER (BXVD_GetChannelMemoryParameters);
   BDBG_ASSERT(pChSettings);
   BDBG_ASSERT(pFWMemConfigSettings);
   BDBG_ASSERT(pFWMemConfig);

   pXvd = (BXVD_P_Context*)(BKNI_Malloc(sizeof(BXVD_P_Context)));

   if (!pXvd)
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   BKNI_Memset((void*)pXvd, BXVD_P_MEM_ZERO, sizeof(BXVD_P_Context));

   BXVD_GetDefaultSettings(&pXvd->stSettings);

   pXvd->uDecoderInstance = pFWMemConfigSettings->uiAVDInstance;

   MemcIndex = pFWMemConfigSettings->MemcIndex;

   ddrType = pFWMemConfigSettings->pInfo->memc[MemcIndex].type;
   memPartSize = pFWMemConfigSettings->pInfo->memc[MemcIndex].deviceTech;
   memBusWidth = pFWMemConfigSettings->pInfo->memc[MemcIndex].width;
   memDeviceWidth = pFWMemConfigSettings->pInfo->memc[0].deviceWidth;
   bDDRGroupageEnabled = pFWMemConfigSettings->pInfo->memc[0].groupageEnabled;

   BXVD_P_DETERMINE_STRIPE_INFO(ddrType, memPartSize, memBusWidth, memDeviceWidth, bDDRGroupageEnabled,
                                &stripeWidth, &bankHeight);

   pXvd->uiDecode_StripeWidth = stripeWidth;
   pXvd->uiDecode_StripeMultiple = bankHeight;

   rc = BXVD_GetChannelMemoryConfig( pXvd, pChSettings, pFWMemConfig);

   if (rc != BERR_SUCCESS)
   {
      BDBG_ERR(("BXVD_GetChannelMemoryConfig failed"));
   }

   BKNI_Free(pXvd);

   BDBG_LEAVE (BXVD_GetChannelMemoryParameters);
   return BERR_TRACE(rc);
}

/***************************************************************************
 * BXVD_GetChannelMemoryConfig: returns the FW Memory configuation needed
 * for the specified channel settings.
 ****************************************************************************/
BERR_Code BXVD_GetChannelMemoryConfig( BXVD_Handle                hXvd,           /* [in] XVD handle */
                                       const BXVD_ChannelSettings *pChSettings,   /* [in] channel settings */
                                       BXVD_FWMemConfig           *pFWMemConfig)  /* [out] memory configuration for this channel */
{
   BXVD_P_DecodeFWMemSize  stDecodeFWMemSize;

   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER (BXVD_GetChannelMemoryConfig);

   /* Check input parameters */
   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(pChSettings);
   BDBG_ASSERT(pFWMemConfig);

#if BDBG_DEBUG_BUILD
   BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.eChannelMode: %s", pChSettings->eChannelMode ? "Still" : "Video"));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.eDecodeResolution = %d", pChSettings->eDecodeResolution));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.hSplitPictureBuffersEnable = %d", pChSettings->bSplitPictureBuffersEnable));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.bb10BitBuffersEnable = %d", pChSettings->b10BitBuffersEnable));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.uiVideoCmprCount = %d", pChSettings->uiVideoCmprCount));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.peVideoCmprStdList = %p",(void *)pChSettings->peVideoCmprStdList));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.uiExtraPictureMemoryAtoms = %d", pChSettings->uiExtraPictureMemoryAtoms));

   if (pChSettings->peVideoCmprStdList)
   {
      uint32_t uiIndex;

      for (uiIndex = 0; uiIndex < pChSettings->uiVideoCmprCount; uiIndex++)
      {
         if (pChSettings->peVideoCmprStdList[uiIndex] < BXVD_S_MAX_VIDEO_PROTOCOL)
         {
            BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.peVideoCmprStdList[%d] = %s (%d)",
                      uiIndex,
                      sVideoCompressionStdNameLUT[pChSettings->peVideoCmprStdList[uiIndex]],
                      pChSettings->peVideoCmprStdList[uiIndex]));
         }
         else
         {
            BDBG_MSG(("BXVD_GetChannelMemoryConfig() - BXVD_ChannelSettings.peVideoCmprStdList[%d] = %s (%d)",
                      uiIndex,
                      "Unknown/Invalid Value!",
                      pChSettings->peVideoCmprStdList[uiIndex]));
         }
      }
   }
#endif

   if (pChSettings->eChannelMode == BXVD_ChannelMode_eStill)
   {
      /* Get still picture decode FW memory sizes */
      rc = BXVD_P_GetStillDecodeFWMemSize(hXvd,
                                          pChSettings->eDecodeResolution,
                                          pChSettings->peVideoCmprStdList,
                                          pChSettings->uiVideoCmprCount,
                                          pChSettings,
                                          &stDecodeFWMemSize);
      if (rc != BERR_SUCCESS)
      {
         pFWMemConfig->uiGeneralHeapSize = 0;
         pFWMemConfig->uiCabacHeapSize = 0;
         pFWMemConfig->uiPictureHeapSize = 0;

         return BERR_TRACE(rc);
      }

      pFWMemConfig->uiGeneralHeapSize =  stDecodeFWMemSize.uiFWContextSize + stDecodeFWMemSize.uiFWInnerLoopWorklistSize;

      pFWMemConfig->uiCabacHeapSize = stDecodeFWMemSize.uiFWCabacSize + stDecodeFWMemSize.uiFWCabacWorklistSize + stDecodeFWMemSize.uiFWDirectModeSize;

      pFWMemConfig->uiPictureHeapSize = stDecodeFWMemSize.uiFWPicLumaBlockSize * stDecodeFWMemSize.uiFWPicBlockCount;
      pFWMemConfig->uiPictureHeap1Size = stDecodeFWMemSize.uiFWPicChromaBlockSize * stDecodeFWMemSize.uiFWPicBlockCount;
   }
   else
   {
      /* Get video picture decode FW memory sizes */
      rc = BXVD_P_GetDecodeFWMemSize(hXvd,
                                     pChSettings->eDecodeResolution,
                                     pChSettings->peVideoCmprStdList,
                                     pChSettings->uiVideoCmprCount,
                                     pChSettings,
                                     &stDecodeFWMemSize);

      if (rc != BERR_SUCCESS)
      {
         pFWMemConfig->uiGeneralHeapSize = 0;
         pFWMemConfig->uiCabacHeapSize = 0;
         pFWMemConfig->uiPictureHeapSize = 0;

         return BERR_TRACE(rc);
      }

      pFWMemConfig->uiGeneralHeapSize =  ((stDecodeFWMemSize.uiFWContextSize +
                                           stDecodeFWMemSize.uiFWInnerLoopWorklistSize +
                                           stDecodeFWMemSize.uiFWInterLayerMVSize + 4095) / 4096) * 4096;

      pFWMemConfig->uiCabacHeapSize = (((stDecodeFWMemSize.uiFWCabacSize +
                                         stDecodeFWMemSize.uiFWCabacWorklistSize +
                                         stDecodeFWMemSize.uiFWDirectModeSize + 4095) / 4096) * 4096);

      pFWMemConfig->uiPictureHeapSize = (((stDecodeFWMemSize.uiFWInterLayerPicSize +
                                           (stDecodeFWMemSize.uiFWPicLumaBlockSize * stDecodeFWMemSize.uiFWPicBlockCount) + 4095)) / 4096) * 4096;

      pFWMemConfig->uiPictureHeap1Size =
         (((stDecodeFWMemSize.uiFWPicChromaBlockSize * stDecodeFWMemSize.uiFWPicBlockCount) + 4095) / 4096) * 4096;
   }

   BDBG_MSG(("BXVD_GetChannelMemoryConfig(): General Heap Size: 0x%0x", pFWMemConfig->uiGeneralHeapSize));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig(): Cabac Heap Size: 0x%0x", pFWMemConfig->uiCabacHeapSize));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig(): Picture Heap Size: 0x%0x", pFWMemConfig->uiPictureHeapSize));
   BDBG_MSG(("BXVD_GetChannelMemoryConfig(): Picture Heap1 Size: 0x%0x", pFWMemConfig->uiPictureHeap1Size));

   BDBG_LEAVE (BXVD_GetChannelMemoryConfig);

   return rc;
}


/***************************************************************************
  BXVD_DecodeStillPicture: Decodes a still picture specified by RaveContext
  and type.
****************************************************************************/
BERR_Code BXVD_DecodeStillPicture(BXVD_ChannelHandle   hXvdCh,
                                  BXVD_DecodeStillMode ePictureType,
                                  BAVC_XptContextMap   *pContextMap)
{
   BXVD_Handle hXvd = NULL;
   BXVD_DecodeSettings stDecodeSettings;
   BERR_Code rc = BERR_SUCCESS;

   uint32_t i;

   BXVD_P_PictureReleaseQueue *pPictureReleaseQueue = NULL;

   BDBG_ENTER(BXVD_DecodeStillPicture);
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pContextMap);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if (hXvdCh->sChSettings.eChannelMode != BXVD_ChannelMode_eStill)
   {
      BXVD_DBG_ERR(hXvdCh, ("BXVD_DecodeStillPicture failed, channel[%d] is not a still decode channel",
                            hXvdCh->ulChannelNum));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   hXvd = hXvdCh->pXvd;

#if BXVD_P_FW_HIM_API
   /* Return previous still picture in release queue (if any) */
   if (hXvdCh->bStillPictureToRelease)
   {
      uint32_t  uiReleaseWriteOffset;
      uint32_t  uiQueueSize = sizeof(BXVD_P_PictureReleaseQueue);

      BKNI_EnterCriticalSection();

      BXVD_P_RELEASE_QUEUE_GET_ADDR( hXvdCh, pPictureReleaseQueue );

      BXVD_P_RELEASE_QUEUE_GET_SHADOW_WRITE_OFFSET( hXvdCh, uiReleaseWriteOffset );

      pPictureReleaseQueue->display_elements[uiReleaseWriteOffset - BXVD_P_INITIAL_OFFSET_DISPLAY_QUEUE] = hXvdCh->uiStillDisplayElementOffset;

      BMMA_FlushCache(hXvdCh->hFWGenMemBlock, (void *)pPictureReleaseQueue, uiQueueSize);

      BXVD_P_INCREMENT_2BASED_OFFSET( uiReleaseWriteOffset, 1 );

      BXVD_P_RELEASE_QUEUE_SET_SHADOW_WRITE_OFFSET( hXvdCh, uiReleaseWriteOffset );

      BKNI_LeaveCriticalSection();

      hXvdCh->bStillPictureToRelease = false;
      hXvdCh->uiStillDisplayElementOffset = 0;
   }
#endif

   /* Setup decode settings struct */
   BKNI_Memset(&stDecodeSettings,
               0,
               sizeof(BXVD_DecodeSettings));

   stDecodeSettings.pContextMap = pContextMap;
   switch (ePictureType)
   {
      case BXVD_DecodeModeStill_eMPEG_SD:
      case BXVD_DecodeModeStill_eMPEG_HD:
         stDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eMPEG2;
         break;

      case BXVD_DecodeModeStill_eAVC_SD:
      case BXVD_DecodeModeStill_eAVC_HD:
         stDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eH264;
         break;
      case BXVD_DecodeModeStill_eVC1_SD:
      case BXVD_DecodeModeStill_eVC1_HD:
         stDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eVC1;
         break;

      case BXVD_DecodeModeStill_eMPEG4Part2_SD:
      case BXVD_DecodeModeStill_eMPEG4Part2_HD:
         stDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eMPEG4Part2;
         break;

      case BXVD_DecodeModeStill_eVP8_SD:
      case BXVD_DecodeModeStill_eVP8_HD:
         stDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eVP8;
         break;

      case BXVD_DecodeModeStill_eVP9_SD:
      case BXVD_DecodeModeStill_eVP9_HD:
      case BXVD_DecodeModeStill_eVP9_4K:
         stDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eVP9;
         break;

#if BXVD_P_HVD_PRESENT
      case BXVD_DecodeModeStill_eHEVC_SD:
      case BXVD_DecodeModeStill_eHEVC_HD:
      case BXVD_DecodeModeStill_eHEVC_4K:
         stDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eH265;
         break;
#endif

      default:
         BXVD_DBG_ERR(hXvdCh, ("Invalid decode still mode specified: 0x%x (%d)",
                                    ePictureType, ePictureType));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Validate picture type */
   /* Make sure protocol is within channel's capabilities */
   for (i = 0; i < hXvdCh->sChSettings.uiVideoCmprCount; i++)
   {
      if (stDecodeSettings.eVideoCmprStd == hXvdCh->sChSettings.peVideoCmprStdList[i]) break;
   }
   if (i == hXvdCh->sChSettings.uiVideoCmprCount)
   {
      BXVD_DBG_ERR(hXvdCh, ("Still channel not capable of specified decode still mode (protocol): 0x%x (%d)",
                                 ePictureType, ePictureType));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Make sure resolution is within channel's capabilities */
   switch(hXvdCh->sChSettings.eDecodeResolution)
   {
      case BXVD_DecodeResolution_eHD:
         switch(ePictureType)
         {
            case BXVD_DecodeModeStill_eMPEG_SD:
            case BXVD_DecodeModeStill_eMPEG_HD:
            case BXVD_DecodeModeStill_eAVC_SD:
            case BXVD_DecodeModeStill_eAVC_HD:
            case BXVD_DecodeModeStill_eVC1_SD:
            case BXVD_DecodeModeStill_eVC1_HD:
            case BXVD_DecodeModeStill_eMPEG4Part2_SD:
            case BXVD_DecodeModeStill_eMPEG4Part2_HD:
            case BXVD_DecodeModeStill_eVP8_SD:
            case BXVD_DecodeModeStill_eVP8_HD:
            case BXVD_DecodeModeStill_eVP9_SD:
            case BXVD_DecodeModeStill_eVP9_HD:
            case BXVD_DecodeModeStill_eHEVC_SD:
            case BXVD_DecodeModeStill_eHEVC_HD:
               break;

            /* coverity[dead_error_begin: FALSE] */
            default:
               BXVD_DBG_ERR(hXvdCh, ("Still channel not capable of specified decode still mode (max resolution: HD): 0x%x (%d)",
                                          ePictureType, ePictureType));
               return BERR_TRACE(BERR_INVALID_PARAMETER);
         }
         break;

      case BXVD_DecodeResolution_eSD:
         switch(ePictureType)
         {
            case BXVD_DecodeModeStill_eMPEG_SD:
            case BXVD_DecodeModeStill_eAVC_SD:
            case BXVD_DecodeModeStill_eVC1_SD:
            case BXVD_DecodeModeStill_eMPEG4Part2_SD:
            case BXVD_DecodeModeStill_eVP8_SD:
            case BXVD_DecodeModeStill_eHEVC_SD:
               break;

            default:
               BXVD_DBG_ERR(hXvdCh, ("Still channel not capable of specified decode still mode (max resolution: SD): 0x%x (%d)",
                                          ePictureType, ePictureType));
               return BERR_TRACE(BERR_INVALID_PARAMETER);
         }
         break;

#if BXVD_P_HVD_PRESENT
      case BXVD_DecodeResolution_e4K:
         switch(ePictureType)
         {
            case BXVD_DecodeModeStill_eMPEG_SD:
            case BXVD_DecodeModeStill_eMPEG_HD:
            case BXVD_DecodeModeStill_eAVC_SD:
            case BXVD_DecodeModeStill_eAVC_HD:
            case BXVD_DecodeModeStill_eVC1_SD:
            case BXVD_DecodeModeStill_eVC1_HD:
            case BXVD_DecodeModeStill_eMPEG4Part2_SD:
            case BXVD_DecodeModeStill_eMPEG4Part2_HD:
            case BXVD_DecodeModeStill_eVP8_SD:
            case BXVD_DecodeModeStill_eVP8_HD:
            case BXVD_DecodeModeStill_eVP9_SD:
            case BXVD_DecodeModeStill_eVP9_HD:
            case BXVD_DecodeModeStill_eVP9_4K:
            case BXVD_DecodeModeStill_eHEVC_SD:
            case BXVD_DecodeModeStill_eHEVC_HD:
            case BXVD_DecodeModeStill_eHEVC_4K:

               break;

            /* coverity[dead_error_begin: FALSE] */
            default:
               BXVD_DBG_ERR(hXvdCh, ("Still channel not capable of specified decode still mode (max resolution: 4K): 0x%x (%d)",
                                          ePictureType, ePictureType));
               return BERR_TRACE(BERR_INVALID_PARAMETER);
         }
         break;
#endif
      default:
         BXVD_DBG_ERR(hXvdCh, ("Still channel not capable of specified decode still mode (resolution): 0x%x (%d)",
                                    ePictureType, ePictureType));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Disable interrupt to prevent race condition */
   if (hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {
      rc = BINT_DisableCallback(hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR);

      if (rc != BERR_SUCCESS)
      {
         return BERR_TRACE(rc);
      }
   }

   /* Call StartDecode with still context */
   rc = BERR_TRACE(BXVD_StartDecode(
                      hXvdCh,
                      &stDecodeSettings));

   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }

#if !BXVD_P_FW_HIM_API
   /* Return previous still picture in release queue (if any) */
   if (hXvdCh->bStillPictureToRelease)
   {
      uint32_t       uiReleaseWriteOffset;

      BXVD_P_RELEASE_QUEUE_GET_ADDR( hXvdCh, pPictureReleaseQueue );
      BXVD_P_RELEASE_QUEUE_GET_WRITE_OFFSET( hXvdCh, uiReleaseWriteOffset );

      pPictureReleaseQueue->display_elements[uiReleaseWriteOffset - BXVD_P_INITIAL_OFFSET_DISPLAY_QUEUE] = hXvdCh->uiStillDisplayElementOffset;

      BXVD_P_INCREMENT_2BASED_OFFSET( uiReleaseWriteOffset, 1 );

      BXVD_P_RELEASE_QUEUE_SET_WRITE_OFFSET( hXvdCh, uiReleaseWriteOffset );

      hXvdCh->bStillPictureToRelease = false;
      hXvdCh->uiStillDisplayElementOffset = 0;
   }
#endif

   /* Re-enable interrupt */
   if (hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR)
   {
      rc = BINT_EnableCallback(hXvd->stDecoderContext.pCbAVC_StillPicRdy_ISR);
   }

   /* The decoded picture will be retrieved by to the
    * BXVD_DeviceInterrupt_eDecodedStillBufferReady interrupt
    * handler */

   BDBG_LEAVE(BXVD_DecodeStillPicture);
   return BERR_TRACE(rc);
}

/***************************************************************************
Summary: This routine is used to reset the still picture decoder state.
****************************************************************************/
BERR_Code BXVD_DecodeStillPictureReset(BXVD_ChannelHandle  hXvdCh)
{
   BXVD_Handle hXvd = NULL;
   BERR_Code rc = BERR_SUCCESS;


   BDBG_ENTER(BXVD_DecodeStillPictureReset);
   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hXvd = hXvdCh->pXvd;

   /* Validate hXvdCh is a still picture channel */
   if (hXvdCh->sChSettings.eChannelMode == BXVD_ChannelMode_eStill)
   {
      rc = BXVD_StopDecode(hXvdCh);

   }

   /* Handle still picture compatibility mode */
   else if ((hXvdCh->ulChannelNum == 0) && hXvd->bStillPictureCompatibilityMode && hXvd->bStillChannelAllocated)
   {
      /* A compatibility still channel is being used */
      hXvdCh = hXvd->ahChannel[hXvd->uiStillChannelNum];
      BDBG_ASSERT(hXvdCh);

      rc =  BXVD_StopDecode(hXvdCh);
   }

   else
   {
      BXVD_DBG_ERR(hXvdCh, ("Channel specified not a still docode channel: %d", hXvdCh->ulChannelNum));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   return rc;
}

/***************************************************************************
  BXVD_SetSkipPictureModeDecode: Sets the skip picture mode (I, IP or IPB)
****************************************************************************/
BERR_Code BXVD_SetSkipPictureModeDecode(BXVD_ChannelHandle hXvdCh,
                                        BXVD_SkipMode      eSkipMode)
{
   BAVC_XptContextMap  XptContextMap;
   BAVC_XptContextMap  aXptContextMap_Extended[BXVD_NUM_EXT_RAVE_CONTEXT];

   uint32_t i;

   BERR_Code eStatus = BERR_SUCCESS;
   BXDM_PictureProvider_PreserveStateSettings stPreserveStateSettings;

   BDBG_ENTER(BXVD_SetSkipPictureModeDecode);
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(eSkipMode <= BXVD_SkipMode_eDecode_Ref_Only);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetSkipPictureModeDecode(%d -> %d)", hXvdCh->eCurrentSkipMode, eSkipMode));

   if (hXvdCh->eCurrentSkipMode != eSkipMode)
   {
      hXvdCh->eCurrentSkipMode = eSkipMode;

      /*
       * If Host Sparse mode is currently enabled and Decoder is active,
       * the decoder must be stopped and started to change skip mode.
       */

      if ((hXvdCh->stDecoderContext.bHostSparseMode == true) && (hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive))
      {
         BXVD_DBG_MSG(hXvdCh, ("BXVD_SetSkipPictureMode: Sparse mode enabled, Stopping and starting decoder"));

         /* Don't reset decoder settings when stopping decode for a sparse mode speed transition */
         hXvdCh->bPreserveState = true;

         /* Force channelChanleMode to hold last picture */
         BERR_TRACE(BXDM_PictureProvider_GetDefaultPreserveStateSettings_isrsafe(
                     hXvdCh->hPictureProvider,
                     &stPreserveStateSettings
                     ));

         BKNI_EnterCriticalSection();

         stPreserveStateSettings.bDisplay = true;
         stPreserveStateSettings.bCounters = true;

         BERR_TRACE(BXDM_PictureProvider_SetPreserveStateSettings_isr(
                  hXvdCh->hPictureProvider,
                  &stPreserveStateSettings
                  ));

         BKNI_LeaveCriticalSection();

         BXVD_StopDecode(hXvdCh);

         /* Decoder counters should not be cleared */
         hXvdCh->bPreserveCounters = true;

         /* Reset XPT Rave CDB read register address */
         XptContextMap.CDB_Read = hXvdCh->ulXptCDB_Read;
         hXvdCh->sDecodeSettings.pContextMap = &XptContextMap;

         for (i = 0; i < hXvdCh->sDecodeSettings.uiContextMapExtNum; i++)
         {
            hXvdCh->sDecodeSettings.aContextMapExtended[i] = &aXptContextMap_Extended[i];
            aXptContextMap_Extended[i].CDB_Read = hXvdCh->aulXptCDB_Read_Extended[i];
         }

         eStatus = BERR_TRACE(BXVD_StartDecode(hXvdCh, &hXvdCh->sDecodeSettings));

         hXvdCh->bPreserveState = false;
      }
      else
      {
         if (hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive)
         {
            eStatus = BXVD_P_HostCmdSetSkipPictureMode(hXvdCh->pXvd,
                                                       hXvdCh->ulChannelNum,
                                                       eSkipMode);
         }
      }
   }

   BDBG_LEAVE(BXVD_SetSkipPictureModeDecode);
   return BERR_TRACE(eStatus);
}

/***************************************************************************
  BXVD_GetSkipPictureModeConfig: Gets the current skip picture mode
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetSkipPictureModeConfig(BXVD_ChannelHandle hXvdCh,
                                        BXVD_SkipMode      *peSkipMode)
{
   BERR_Code eStatus = BERR_SUCCESS;

   BDBG_ENTER(BXVD_GetSkipPictureModeConfig);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   *peSkipMode = hXvdCh->eCurrentSkipMode;

   BDBG_LEAVE(BXVD_GetSkipPictureModeConfig);
   return BERR_TRACE(eStatus);
}
#endif

/***************************************************************************
BXVD_EnableInterrupt_isr: Enable or disable the specifed interrupt. ISR version
***************************************************************************/
BERR_Code BXVD_EnableInterrupt_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Interrupt eInterrupt,
   BXVD_InterruptEnable eEnable
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BXVD_EnableInterrupt_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Sanity check arguments */
   if (eEnable != BXVD_InterruptEnable_eDisable &&
       eEnable != BXVD_InterruptEnable_eEnable)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   switch (eInterrupt)
   {
      case BXVD_Interrupt_eVideoInstructionChecker:
         BXVD_DBG_ERR(hXvdCh, ("Enable/Disable interrupt not supported for VideoIntructionChecker"));
         rc = BERR_INVALID_PARAMETER;
         break;
      case BXVD_Interrupt_ePtsStcOffset:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eStcPtsOffset,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_ePictureDataReady:
         BXVD_DBG_ERR(hXvdCh, ("Enable/Disable interrupt not supported for PictureDataReady"));
         rc = BERR_INVALID_PARAMETER;
         break;
      case BXVD_Interrupt_eUserData:
         BXVD_DBG_ERR(hXvdCh, ("Enable/Disable interrupt not supported for UserData"));
         rc = BERR_INVALID_PARAMETER;
         break;
      case BXVD_Interrupt_ePicDepthLowerThreshold:
         BXVD_DBG_ERR(hXvdCh, ("Enable/Disable interrupt not supported for PicDepthLowerThreshold"));
         rc = BERR_INVALID_PARAMETER;
         break;
      case BXVD_Interrupt_ePicDepthHigherThreshold:
         BXVD_DBG_ERR(hXvdCh, ("Enable/Disable interrupt not supported for PicDepthHigherThreshold"));
         rc = BERR_INVALID_PARAMETER;
         break;
      case BXVD_Interrupt_eFirstPTSReady:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eFirstPTSReady,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eFirstPTSPassed:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eFirstPTSPassed,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_ePTSError:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_ePTSError,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_ePauseUntoPTS:
         BXVD_DBG_WRN(hXvdCh, ("BXVD_Interrupt_ePauseUntoPTS is DEPRECATED"));
         break;
      case BXVD_Interrupt_eDisplayUntoPTS:
         BXVD_DBG_WRN(hXvdCh, ("BXVD_Interrupt_ePauseUntoPTS is DEPRECATED"));
         break;
      case BXVD_Interrupt_ePTS1Match:
         BXVD_DBG_WRN(hXvdCh, ("BXVD_Interrupt_ePTS1Match is DEPRECATED"));
         break;
      case BXVD_Interrupt_ePTS2Match:
         BXVD_DBG_WRN(hXvdCh, ("BXVD_Interrupt_ePTS2Match is DEPRECATED"));
         break;
      case BXVD_Interrupt_eIFrame:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eIFrame,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_ePictureParameters:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_ePictureParameters,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eTSMPassInASTMMode:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eTSMPassInASTMMode,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eClipStart:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eClipStart,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eClipStop:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eClipStop,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_ePictureMarker:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_ePictureMarker,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eMarker:
         hXvdCh->stCallbackReq.bMarker = eEnable;
         break;
      case BXVD_Interrupt_eRequestSTC:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eRequestSTC,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_ePPBReceived:
         hXvdCh->stCallbackReq.bPPBReceived = eEnable;
         break;
      case BXVD_Interrupt_ePPBParameters:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_ePictureUnderEvaluation,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eDecodeError:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eDecodeError,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eTSMResult:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eTSMResult,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_ePictureExtensionData:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_ePictureExtensionData,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eChunkDone:
         BXDM_PictureProvider_Callback_SetEnable_isr(
                  hXvdCh->hPictureProvider,
                  BXDM_PictureProvider_Callback_eChunkDone,
                  eEnable
                  );
         break;
      case BXVD_Interrupt_eEndOfGOP:
         /* SW7425-2686: multi-pass DQT: indicates that the end of the GOP has been
          * reached and the system is beginning reverse playback.*/
         hXvdCh->stCallbackReq.bEndOfGOP = eEnable;
         break;
      default:
         rc = BERR_INVALID_PARAMETER;
         break;
   }

   BDBG_LEAVE(BXVD_EnableInterrupt_isr);

   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_EnableInterrupt: Enable or disable the specifed interrupt
***************************************************************************/
BERR_Code BXVD_EnableInterrupt(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Interrupt eInterrupt,
   BXVD_InterruptEnable eEnable
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BXVD_EnableInterrupt);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BKNI_EnterCriticalSection();
   rc = BXVD_EnableInterrupt_isr(hXvdCh, eInterrupt, eEnable);
   BKNI_LeaveCriticalSection();


   BDBG_LEAVE(BXVD_EnableInterrupt);
   return BERR_TRACE(rc);
}

/***************************************************************************
Summary:
        Link two decoders picture procssing

Description:
        Routine used to link the primary decoder picture processing
with the processing of the secondary decoders pictures. Two decoders
are needed to decode 12 CIF video streams which will be displayed on
a single display device.
***************************************************************************/
void BXVD_LinkDecoders(BXVD_Handle hXvd_Primary,
                       BXVD_Handle hXvd_Secondary,
                       BXVD_DisplayInterrupt eDisplayInterrupt)
{
   BDBG_ASSERT(hXvd_Primary);
   BDBG_ASSERT(hXvd_Secondary);

   /* Check handle type for correctness */
   if (hXvd_Primary->eHandleType != BXVD_P_HandleType_XvdMain ||
       hXvd_Secondary->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return;
   }

   hXvd_Primary->hXvd_Secondary = hXvd_Secondary;
   hXvd_Secondary->hLinkedDecoderXdmDih = hXvd_Primary->hXdmDih[eDisplayInterrupt];
   hXvd_Primary->Secondary_eDisplayInterrupt = eDisplayInterrupt;
}


/***************************************************************************
Summary:
        Unlink two decoders picture procssing

Description:
        Routine used to unlink the primary decoder picture processing
with the processing of the secondary decoders pictures.
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
void BXVD_UnlinkDecoders(BXVD_Handle hXvd_Primary)
{
   BDBG_ASSERT(hXvd_Primary);

   /* Check handle type for correctness */
   if (hXvd_Primary->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return;
   }

   hXvd_Primary->hXvd_Secondary = (BXVD_Handle) NULL;
}
#endif

/************************/
/* Display Manager APIs */
/************************/

/*********************************/
/* Display Manager APIs - Status */
/*********************************/

/***************************************************************************
BXVD_GetPTS: API used to get the running PTS.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetPTS
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PTSInfo       *pPTSInfo
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetPTS);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetPTS_isr(
      hXvdCh,
      pPTSInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetPTS);
   return BERR_TRACE(rc);
}
#endif

BERR_Code BXVD_GetPTS_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PTSInfo       *pPTSInfo
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PTSInfo stPTSInfo;
   BDBG_ENTER(BXVD_GetPTS_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pPTSInfo);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /*
    * Copy the "stCurrentPTSInfo" structure.
    */

   rc = BXDM_PictureProvider_GetCurrentPTSInfo_isr(
            hXvdCh->hPictureProvider,
            &stPTSInfo
            );

   *pPTSInfo = stPTSInfo;

   BDBG_LEAVE(BXVD_GetPTS_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BXVD_GetLastCodedPTS: API used to get the last coded PTS.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetLastCodedPTS
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PTSInfo       *pPTSInfo
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetLastCodedPTS);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetLastCodedPTS_isr(
      hXvdCh,
      pPTSInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetLastCodedPTS);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetLastCodedPTS_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PTSInfo       *pPTSInfo
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PTSInfo stPTSInfo;

   BDBG_ENTER(BXVD_GetLastCodedPTS_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pPTSInfo);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetLastCodedPTSInfo_isr(
            hXvdCh->hPictureProvider,
            &stPTSInfo
            );

   *pPTSInfo = stPTSInfo;

   BDBG_LEAVE(BXVD_GetLastCodedPTS_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
  BXVD_GetNextPTS: API used to get the next PTS.
****************************************************************************/
BERR_Code BXVD_GetNextPTS
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetNextPTS);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetNextPTS_isr(
      hXvdCh,
      pPTSInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetNextPTS);
   return rc;
}

BERR_Code BXVD_GetNextPTS_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PTSInfo stPTSInfo;

   BDBG_ENTER(BXVD_GetNextPTS_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pPTSInfo);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetNextPTSInfo_isr(
               hXvdCh->hPictureProvider,
               &stPTSInfo
               );

   *pPTSInfo = stPTSInfo;

   BDBG_LEAVE(BXVD_GetNextPTS_isr);

   return rc;
}

/***************************************************************************
  BXVD_GetIPictureFoundState: Retrieve I picture found status
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetIPictureFoundStatus
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbIPictureFound
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetIPictureFoundStatus);

   rc = BXVD_GetIPictureFoundStatus_isr(
      hXvdCh,
      pbIPictureFound
      );

   BDBG_LEAVE(BXVD_GetIPictureFoundStatus);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetIPictureFoundStatus_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbIPictureFound
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetIPictureFoundStatus_isr);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetIPictureFoundStatus_isr(
                     hXvdCh->hPictureProvider,
                     pbIPictureFound
                     );

   BDBG_LEAVE(BXVD_GetIPictureFoundStatus_isr);
   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
  BXVD_GetPictureTag: Get the current picture tag from DM.
***************************************************************************/
BERR_Code BXVD_GetPictureTag
(
   BXVD_ChannelHandle    hXvdCh,
   unsigned long        *pulPictureTag
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetPictureTag);

   rc = BXVD_GetPictureTag_isr(
      hXvdCh,
      pulPictureTag
      );

   BDBG_LEAVE(BXVD_GetPictureTag);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetPictureTag_isr
(
   BXVD_ChannelHandle    hXvdCh,
   unsigned long        *pulPictureTag
   )
{
   BERR_Code rc;
   uint32_t uiPictureTag;

   BDBG_ENTER(BXVD_GetPictureTag_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pulPictureTag);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Return current picture tag */
   rc = BXDM_PictureProvider_GetPictureTag_isr(
                     hXvdCh->hPictureProvider,
                     &uiPictureTag
                     );

   *pulPictureTag = uiPictureTag;

   BDBG_LEAVE(BXVD_GetPictureTag_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BXVD_GetGopTimeCode: Get the GOP timecode from DM and decode into HH:MM:SS:FF
***************************************************************************/
BERR_Code BXVD_GetGopTimeCode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_GopTimeCode *pGopTimeCode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetGopTimeCode);

   rc = BXVD_GetGopTimeCode_isr(
      hXvdCh,
      pGopTimeCode
      );

   BDBG_LEAVE(BXVD_GetGopTimeCode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetGopTimeCode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_GopTimeCode *pGopTimeCode
   )
{
   BERR_Code rc;
   BXDM_Picture_GopTimeCode stTimeCode;
   BDBG_ENTER(BXVD_GetGopTimeCode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pGopTimeCode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetCurrentTimeCode_isr(
            hXvdCh->hPictureProvider,
            &stTimeCode
            );

   pGopTimeCode->ulTimeCodeHours = stTimeCode.uiHours;
   pGopTimeCode->ulTimeCodeMinutes = stTimeCode.uiMinutes;
   pGopTimeCode->ulTimeCodeSeconds = stTimeCode.uiSeconds;
   pGopTimeCode->ulTimeCodePictures = stTimeCode.uiPictures;
   pGopTimeCode->bTimeCodeValid = stTimeCode.bValid;

   BDBG_LEAVE(BXVD_GetGopTimeCode_isr);
   return BERR_TRACE(rc);
}

/**********************************/
/* Display Manager APIs - Display */
/**********************************/

/***************************************************************************
BXVD_EnableMute: The Application can override the decoder mute with this API.
***************************************************************************/
BERR_Code BXVD_EnableMute
(
   BXVD_ChannelHandle hXvdCh,
   bool               bEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_EnableMute);

   BKNI_EnterCriticalSection();
   rc = BXVD_EnableMute_isr(
      hXvdCh,
      bEnable
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_EnableMute);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_EnableMute_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool               bEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_EnableMute_isr);
   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Save user request */
   rc = BXDM_PictureProvider_SetMuteMode_isr(
            hXvdCh->hPictureProvider,
            bEnable
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_EnableMute(%d)", bEnable));

   BDBG_LEAVE(BXVD_EnableMute_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BXVD_SetDisplayFieldMode: API used to set the display field mode.
****************************************************************************/
BERR_Code BXVD_SetDisplayFieldMode
(
   BXVD_ChannelHandle    hXvdCh,
   BXVD_DisplayFieldType eDisplayFieldType
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetDisplayFieldMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetDisplayFieldMode_isr(
      hXvdCh,
      eDisplayFieldType
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetDisplayFieldMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetDisplayFieldMode_isr
(
   BXVD_ChannelHandle    hXvdCh,
   BXVD_DisplayFieldType eDisplayFieldType
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_DisplayFieldMode eXDMDisplayFieldMode;
   BDBG_ENTER(BXVD_SetDisplayFieldMode_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( eDisplayFieldType )
   {
      case BXVD_DisplayFieldType_eBothField:
         eXDMDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eBothField;
         break;

      case BXVD_DisplayFieldType_eTopFieldOnly:
         eXDMDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eTopFieldOnly;
         break;

      case BXVD_DisplayFieldType_eBottomFieldOnly:
         eXDMDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eBottomFieldOnly;
         break;

      case BXVD_DisplayFieldType_eSingleField:
         eXDMDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eSingleField;
         break;

      case BXVD_DisplayFieldType_eAuto:
         eXDMDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eAuto;
         break;

      default:
      BXVD_DBG_ERR(hXvdCh, ("Invalid display field mode: 0x%x (%d)",
                                  eDisplayFieldType, eDisplayFieldType));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetDisplayFieldMode_isr(
            hXvdCh->hPictureProvider,
            eXDMDisplayFieldMode);

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetDisplayFieldMode(%d)", eDisplayFieldType));

   BDBG_LEAVE(BXVD_SetDisplayFieldMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
BXVD_GetDisplayFieldMode: Gets the video display mode status.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetDisplayFieldMode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayFieldType *peDisplayFieldType
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetDisplayFieldMode);

   rc = BXVD_GetDisplayFieldMode_isr(
      hXvdCh,
      peDisplayFieldType
      );

   BDBG_LEAVE(BXVD_GetDisplayFieldMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetDisplayFieldMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayFieldType *peDisplayFieldType
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_DisplayFieldMode eXDMDisplayFieldMode;

   BDBG_ENTER(BXVD_GetDisplayFieldMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peDisplayFieldType);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetDisplayFieldMode_isr(
            hXvdCh->hPictureProvider,
            &eXDMDisplayFieldMode);

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMDisplayFieldMode )
      {
         case BXDM_PictureProvider_DisplayFieldMode_eBothField:
            *peDisplayFieldType = BXVD_DisplayFieldType_eBothField;
            break;

         case BXDM_PictureProvider_DisplayFieldMode_eTopFieldOnly:
            *peDisplayFieldType = BXVD_DisplayFieldType_eTopFieldOnly;
            break;

         case BXDM_PictureProvider_DisplayFieldMode_eBottomFieldOnly:
            *peDisplayFieldType = BXVD_DisplayFieldType_eBottomFieldOnly;
            break;

         case BXDM_PictureProvider_DisplayFieldMode_eSingleField:
            *peDisplayFieldType = BXVD_DisplayFieldType_eSingleField;
            break;

         case BXDM_PictureProvider_DisplayFieldMode_eAuto:
            *peDisplayFieldType = BXVD_DisplayFieldType_eAuto;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM display field mode: 0x%x (%d)",
                     eXDMDisplayFieldMode, eXDMDisplayFieldMode));

            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_GetDisplayFieldMode_isr);
   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL  SWSTB-461 */

/***************************************************************************
  BXVD_SetChannelChangeMode: Tell DM how to handle channel changes.
****************************************************************************/
BERR_Code BXVD_SetChannelChangeMode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ChannelChangeMode eChChangeMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetChannelChangeMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetChannelChangeMode_isr(
      hXvdCh,
      eChChangeMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetChannelChangeMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetChannelChangeMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ChannelChangeMode eChChangeMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ChannelChangeSettings stNewChannelChangeSettings;

   BXDM_DisplayInterruptHandler_Handle hXdmDihCurrent;

   bool bSquirrelAwayTheSettings;

   BDBG_ENTER(BXVD_SetChannelChangeMode_isr);
   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetChannelChangeSettings_isr(
            hXvdCh->hPictureProvider,
            &stNewChannelChangeSettings);

   if ( BERR_SUCCESS == rc )
   {
      switch ( eChChangeMode )
      {
         case BXVD_ChannelChangeMode_eMute:
            stNewChannelChangeSettings.bHoldLastPicture = false;
            stNewChannelChangeSettings.bFirstPicturePreview = false;
            break;

         case BXVD_ChannelChangeMode_eLastFramePreviousChannel:
            stNewChannelChangeSettings.bHoldLastPicture = true;
            stNewChannelChangeSettings.bFirstPicturePreview = false;
            break;

         case BXVD_ChannelChangeMode_eMuteWithFirstPicturePreview:
            stNewChannelChangeSettings.bHoldLastPicture = false;
            stNewChannelChangeSettings.bFirstPicturePreview = true;
            break;

         case BXVD_ChannelChangeMode_eLastFramePreviousWithFirstPicturePreview:
            stNewChannelChangeSettings.bHoldLastPicture = true;
            stNewChannelChangeSettings.bFirstPicturePreview = true;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Invalid channel change parameter (%d)",
                     eChChangeMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      /* SW7425-1064: when the XMO is installed and active, the channel change mode is force
       * to NOT hold last picture.  After StopDecode is called, the channel change mode is
       * restored to the original value.  So if the XMO is currently active, save these
       * new settings in the XMO structure.  They will be applied to XDM in BXVD_StopDecode.
       */
      bSquirrelAwayTheSettings =  ( BXVD_P_DecoderState_eActive == hXvdCh->eDecoderState );
      bSquirrelAwayTheSettings &= ( BXVD_P_ChannelType_eBase == hXvdCh->eChannelType );
      bSquirrelAwayTheSettings &= ( NULL != hXvdCh->hXmo );

      if ( true == bSquirrelAwayTheSettings )
      {
         BXDM_PictureProvider_XMO_SaveChannelChangeSettings_isr(
                        hXvdCh->hXmo,
                        &stNewChannelChangeSettings );
      }
      else
      {
         rc = BXDM_PictureProvider_SetChannelChangeSettings_isr(
                  hXvdCh->hPictureProvider,
                  &stNewChannelChangeSettings );
      }

      if ((eChChangeMode == BXVD_ChannelChangeMode_eMute) &&
          (hXvdCh->eDecoderState == BXVD_P_DecoderState_eNotActive))
      {
         /* Channel change mode is now eMute, the channel is stopped,
          * disable picture provider. */
         BXDM_PictureProvider_GetDIH_isrsafe( hXvdCh->hPictureProvider, &hXdmDihCurrent );

         if ( NULL != hXdmDihCurrent )
         {

            BERR_TRACE(BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr(
                          hXdmDihCurrent,
                          BXDM_PictureProvider_GetPicture_isr,
                          hXvdCh->hPictureProvider,
                          BXDM_DisplayInterruptHandler_PictureProviderMode_eDisabled
                          ));
         }
      }

      BXVD_DBG_MSG(hXvdCh, ("BXVD_SetChannelChangeMode(%d)", eChChangeMode));
   }

   BDBG_LEAVE(BXVD_SetChannelChangeMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BXVD_GetChannelChangeMode: Retrieve current channel change mode
****************************************************************************/
BERR_Code BXVD_GetChannelChangeMode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ChannelChangeMode *peChChangeMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetChannelChangeMode);

   rc = BXVD_GetChannelChangeMode_isr(
      hXvdCh,
      peChChangeMode
      );

   BDBG_LEAVE(BXVD_GetChannelChangeMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetChannelChangeMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ChannelChangeMode *peChChangeMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ChannelChangeSettings eCurrentChannelChangeSettings;

   BDBG_ENTER(BXVD_GetChannelChangeMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peChChangeMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetChannelChangeSettings_isr(
            hXvdCh->hPictureProvider,
            &eCurrentChannelChangeSettings);

   if ( BERR_SUCCESS == rc )
   {
      if ( ( false == eCurrentChannelChangeSettings.bHoldLastPicture )
        && ( false == eCurrentChannelChangeSettings.bFirstPicturePreview ) )
      {
         *peChChangeMode = BXVD_ChannelChangeMode_eMute;
      } else if ( ( true == eCurrentChannelChangeSettings.bHoldLastPicture )
               && ( false == eCurrentChannelChangeSettings.bFirstPicturePreview ) )
      {
         *peChChangeMode = BXVD_ChannelChangeMode_eLastFramePreviousChannel;
      } else if ( ( false == eCurrentChannelChangeSettings.bHoldLastPicture )
               && ( true == eCurrentChannelChangeSettings.bFirstPicturePreview ) )
      {
         *peChChangeMode = BXVD_ChannelChangeMode_eMuteWithFirstPicturePreview;
      } else
      {
         *peChChangeMode = BXVD_ChannelChangeMode_eLastFramePreviousWithFirstPicturePreview;
      }
   }

   BDBG_LEAVE(BXVD_GetChannelChangeMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BXVD_SetInterpolationModeForStillContent: This API sets the SPIM for the
  video decoder.
****************************************************************************/
BERR_Code BXVD_SetInterpolationModeForStillContent
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_StillContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetInterpolationModeForStillContent);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetInterpolationModeForStillContent_isr(
      hXvdCh,
      eNewInterpolation
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetInterpolationModeForStillContent);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetInterpolationModeForStillContent_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_StillContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_SourceFormatOverride eSourceFormatOverride;
   BDBG_ENTER(BXVD_SetInterpolationModeForStillContent_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hXvdCh->eSavedSPIM = eNewInterpolation;

   switch ( eNewInterpolation )
   {
      case BXVD_StillContentInterpolationMode_eSingleField:
         eSourceFormatOverride = BXDM_PictureProvider_SourceFormatOverride_eInterlaced;
         break;

      case BXVD_StillContentInterpolationMode_eBothField:
      case BXVD_StillContentInterpolationMode_eFrame:
         eSourceFormatOverride = BXDM_PictureProvider_SourceFormatOverride_eProgressive;
       break;

      case BXVD_StillContentInterpolationMode_eDefault:
         eSourceFormatOverride = BXDM_PictureProvider_SourceFormatOverride_eDefault;
         break;

      default:
         BXVD_DBG_ERR(hXvdCh, ("Invalid SPIM Mode (%d)",
                  eNewInterpolation));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetSourceFormatOverride_isr(
            hXvdCh->hPictureProvider,
            eSourceFormatOverride
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetInterpolationModeForStillContent(%d)", eNewInterpolation));

   BDBG_LEAVE(BXVD_SetInterpolationModeForStillContent_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
  BXVD_GetInterpolationModeForStillContent: This function gets the still
  content interpolation mode(SPIM).
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetInterpolationModeForStillContent
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_StillContentInterpolationMode *peStillContentIntrpMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetInterpolationModeForStillContent);

   rc = BXVD_GetInterpolationModeForStillContent_isr(
      hXvdCh,
      peStillContentIntrpMode
      );

   BDBG_LEAVE(BXVD_GetInterpolationModeForStillContent);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetInterpolationModeForStillContent_isr
(
   BXVD_ChannelHandle   hXvdCh,
   BXVD_StillContentInterpolationMode *peStillContentIntrpMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_SourceFormatOverride eSourceFormatOverride;

   BDBG_ENTER(BXVD_GetInterpolationModeForStillContent_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peStillContentIntrpMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetSourceFormatOverride_isr(
            hXvdCh->hPictureProvider,
            &eSourceFormatOverride
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eSourceFormatOverride )
      {
         case BXDM_PictureProvider_SourceFormatOverride_eInterlaced:
            *peStillContentIntrpMode = BXVD_StillContentInterpolationMode_eSingleField;
            break;

         case BXDM_PictureProvider_SourceFormatOverride_eProgressive:
            *peStillContentIntrpMode = hXvdCh->eSavedSPIM;
            break;

         case BXDM_PictureProvider_SourceFormatOverride_eDefault:
            *peStillContentIntrpMode = BXVD_StillContentInterpolationMode_eDefault;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM SPIM Mode (%d)",
                     eSourceFormatOverride));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_GetInterpolationModeForStillContent_isr);
   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
  BXVD_SetInterpolationModeForMovingContent: This API sets the MPIM for
  the video decoder.
****************************************************************************/
BERR_Code BXVD_SetInterpolationModeForMovingContent
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MovingContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetInterpolationModeForMovingContent);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetInterpolationModeForMovingContent_isr(
      hXvdCh,
      eNewInterpolation
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetInterpolationModeForMovingContent);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetInterpolationModeForMovingContent_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MovingContentInterpolationMode eNewInterpolation
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ScanModeOverride eXDMScanModeOverride;
   BDBG_ENTER(BXVD_SetInterpolationModeForMovingContent_isr);
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(eNewInterpolation <= BXVD_MovingContentInterpolationMode_eProgressiveScanout);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( eNewInterpolation )
   {
      case BXVD_MovingContentInterpolationMode_eInterlacedScanout:
         eXDMScanModeOverride = BXDM_PictureProvider_ScanModeOverride_eInterlaced;
         break;

      case BXVD_MovingContentInterpolationMode_eProgressiveScanout:
         eXDMScanModeOverride = BXDM_PictureProvider_ScanModeOverride_eProgressive;
         break;

      case BXVD_MovingContentInterpolationMode_eDefault:
         eXDMScanModeOverride = BXDM_PictureProvider_ScanModeOverride_eDefault;
         break;

      default:
         BXVD_DBG_ERR(hXvdCh, ("Invalid MPIM Mode (%d)",
                  eNewInterpolation));
         return BERR_TRACE(BERR_INVALID_PARAMETER);

   }

   rc = BXDM_PictureProvider_SetScanModeOverride_isr(
            hXvdCh->hPictureProvider,
            eXDMScanModeOverride
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetInterpolationModeForMovingContent(%d)", eNewInterpolation));

   BDBG_LEAVE(BXVD_SetInterpolationModeForMovingContent_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BXVD_GetInterpolationModeForMovingContent: This function gets interpolation
  mode for moving content.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetInterpolationModeForMovingContent
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MovingContentInterpolationMode *peNewInterpolation
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetInterpolationModeForMovingContent);

   rc = BXVD_GetInterpolationModeForMovingContent_isr(
      hXvdCh,
      peNewInterpolation
      );

   BDBG_LEAVE(BXVD_GetInterpolationModeForMovingContent);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetInterpolationModeForMovingContent_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MovingContentInterpolationMode *peNewInterpolation
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ScanModeOverride eScanModeOverride;

   BDBG_ENTER(BXVD_GetInterpolationModeForMovingContent_isr);
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peNewInterpolation);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetScanModeOverride_isr(
            hXvdCh->hPictureProvider,
            &eScanModeOverride
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eScanModeOverride )
      {

         case BXDM_PictureProvider_ScanModeOverride_eInterlaced:
            *peNewInterpolation = BXVD_MovingContentInterpolationMode_eInterlacedScanout;
            break;

         case BXDM_PictureProvider_ScanModeOverride_eProgressive:
            *peNewInterpolation = BXVD_MovingContentInterpolationMode_eProgressiveScanout;
            break;

         case BXDM_PictureProvider_ScanModeOverride_eDefault:
            *peNewInterpolation = BXVD_MovingContentInterpolationMode_eDefault;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM MPIM Mode (%d)",
                     eScanModeOverride));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_GetInterpolationModeForMovingContent_isr);
   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
    BXVD_SetMonitorRefreshRate: Used to set the monitor refresh rate
****************************************************************************/
BERR_Code BXVD_SetMonitorRefreshRate
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t ui32MonitorRefreshRate
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetMonitorRefreshRate);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetMonitorRefreshRate_isr(
      hXvdCh,
      ui32MonitorRefreshRate
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetMonitorRefreshRate);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetMonitorRefreshRate_isr
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t ui32MonitorRefreshRate
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_MonitorRefreshRate eXDMMonitorRefreshRate;
   BDBG_ENTER(BXVD_SetMonitorRefreshRate_isr);
   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( ui32MonitorRefreshRate )
   {
      case BXVD_MONITOR_REFRESH_RATE_50Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e50Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_59_94Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_60Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e60Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_23_976Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_24Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e24Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_25Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e25Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_30Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e30Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_48Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e48Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_29_97Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_12_5Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_14_985Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_15Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e15Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_20Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e20Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_100Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e100Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_119_88Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e119_88Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_120Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e120Hz;
         break;
      case BXVD_MONITOR_REFRESH_RATE_7_493Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz;
         break;
      case BXVD_MONITOR_REFRESH_RATE_7_5Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz;
         break;
      case BXVD_MONITOR_REFRESH_RATE_9_99Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz;
         break;
      case BXVD_MONITOR_REFRESH_RATE_10Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e10Hz;
         break;
      case BXVD_MONITOR_REFRESH_RATE_11_988Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz;
         break;
      case BXVD_MONITOR_REFRESH_RATE_12Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e12Hz;
         break;
      case BXVD_MONITOR_REFRESH_RATE_19_98Hz:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz;
         break;

      case BXVD_MONITOR_REFRESH_RATE_INVALID:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_eUnknown;
         break;

      default:
         eXDMMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_eUnknown;
         BXVD_DBG_ERR(hXvdCh, ("Invalid monitor refresh rate specified (defaulting to \"Unknown\"): %d", ui32MonitorRefreshRate));
         break;
   }

   rc = BXDM_PictureProvider_SetMonitorRefreshRate_isr(
            hXvdCh->hPictureProvider,
            eXDMMonitorRefreshRate
            );

   hXvdCh->sChSettings.ui32MonitorRefreshRate = ui32MonitorRefreshRate;

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetMonitorRefreshRate(%d)", ui32MonitorRefreshRate));

   BDBG_LEAVE(BXVD_SetMonitorRefreshRate_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
    BXVD_GetMonitorRefreshRate: Used to get the current value of the monitor
  refresh rate
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetMonitorRefreshRate
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t *pui32MonitorRefreshRate
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetMonitorRefreshRate);

   rc = BXVD_GetMonitorRefreshRate_isr(
      hXvdCh,
      pui32MonitorRefreshRate
      );

   BDBG_LEAVE(BXVD_GetMonitorRefreshRate);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetMonitorRefreshRate_isr
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t *pui32MonitorRefreshRate
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_MonitorRefreshRate eXDMMonitorRefreshRate;

   BDBG_ENTER(BXVD_GetMonitorRefreshRate_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pui32MonitorRefreshRate);

   /* Check for NULL handle */
   if (hXvdCh == NULL)
   {
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetMonitorRefreshRate_isr(
            hXvdCh->hPictureProvider,
            &eXDMMonitorRefreshRate
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMMonitorRefreshRate )
      {
         case BXDM_PictureProvider_MonitorRefreshRate_e50Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_50Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e59_94Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_59_94Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e60Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_60Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e23_976Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_23_976Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e24Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_24Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e25Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_25Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e30Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_30Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e48Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_48Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e29_97Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_29_97Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_12_5Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_14_985Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e15Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_15Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e20Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_20Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e100Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_100Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e119_88Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_119_88Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_e120Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_120Hz;
            break;
         case BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_7_493Hz;
            break;
         case BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_7_5Hz;
            break;
         case BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_9_99Hz;
            break;
         case BXDM_PictureProvider_MonitorRefreshRate_e10Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_10Hz;
            break;
         case BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_11_988Hz;
            break;
         case BXDM_PictureProvider_MonitorRefreshRate_e12Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_12Hz;
            break;
         case BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_19_98Hz;
            break;

         case BXDM_PictureProvider_MonitorRefreshRate_eUnknown:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_INVALID;
            break;

         default:
            *pui32MonitorRefreshRate = BXVD_MONITOR_REFRESH_RATE_INVALID;
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM monitor refresh rate specified: %d", eXDMMonitorRefreshRate));
            rc = BERR_UNKNOWN;
            break;
      }
   }

   BDBG_LEAVE(BXVD_GetMonitorRefreshRate_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
  BXVD_EnableVideoFreeze: Used to freeze video while the decoder continues
  to run
****************************************************************************/
BERR_Code BXVD_EnableVideoFreeze
(
   BXVD_ChannelHandle hXvdCh
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_EnableVideoFreeze);

   BKNI_EnterCriticalSection();
   rc = BXVD_EnableVideoFreeze_isr(
      hXvdCh
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_EnableVideoFreeze);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_EnableVideoFreeze_isr
(
   BXVD_ChannelHandle hXvdCh
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_EnableVideoFreeze_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetFreeze_isr(
            hXvdCh->hPictureProvider,
            true);

   BXVD_DBG_MSG(hXvdCh, ("BXVD_EnableVideoFreeze()"));

   BDBG_LEAVE(BXVD_EnableVideoFreeze_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BXVD_DisableVideoFreeze: Used to re-enable previously frozen video
****************************************************************************/
BERR_Code BXVD_DisableVideoFreeze
(
   BXVD_ChannelHandle hXvdCh
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_DisableVideoFreeze);

   BKNI_EnterCriticalSection();
   rc = BXVD_DisableVideoFreeze_isr(
      hXvdCh
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_DisableVideoFreeze);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_DisableVideoFreeze_isr
(
   BXVD_ChannelHandle hXvdCh
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_DisaableVideoFreeze_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetFreeze_isr(
            hXvdCh->hPictureProvider,
            false);

   BXVD_DBG_MSG(hXvdCh, ("BXVD_DisableVideoFreeze()"));

   BDBG_LEAVE(BXVD_DisableVideoFreeze_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BXVD_GetVideoFreezeState: Used to get the video freeze state
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetVideoFreezeState
(
   BXVD_ChannelHandle hXvdCh,
   bool               *bVFState
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetVideoFreezeState);

   rc = BXVD_GetVideoFreezeState_isr(
      hXvdCh,
      bVFState
      );

   BDBG_LEAVE(BXVD_GetVideoFreezeState);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetVideoFreezeState_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool               *bVFState
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetVideoFreezeState_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(bVFState);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetFreeze_isr(
            hXvdCh->hPictureProvider,
            bVFState);

   BDBG_LEAVE(BXVD_GetVideoFreezeState_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_Set1080pScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode e1080pScanMode /* [in] The new 1080p scan mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_Set1080pScanMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_Set1080pScanMode_isr(
      hXvdCh,
      e1080pScanMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_Set1080pScanMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_Set1080pScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode e1080pScanMode /* [in] The new 1080p scan mode */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_1080pScanMode eXDM1080pScanMode;

   BDBG_ENTER(BXVD_Set1080pScanMode_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( e1080pScanMode )
   {
      case BXVD_1080pScanMode_eDefault:
         eXDM1080pScanMode = BXDM_PictureProvider_1080pScanMode_eDefault;
         break;

      case BXVD_1080pScanMode_eAdvanced:
         eXDM1080pScanMode = BXDM_PictureProvider_1080pScanMode_eAdvanced;
         break;

      default:
      BXVD_DBG_ERR(hXvdCh, ("Invalid 1080p scan mode specified: %d", e1080pScanMode));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_Set1080pScanMode_isr(
            hXvdCh->hPictureProvider,
            eXDM1080pScanMode
            );

   if (rc != BERR_SUCCESS )
   {
      return BERR_TRACE(rc);
   }

   hXvdCh->sChSettings.e1080pScanMode = e1080pScanMode;
   BXVD_DBG_MSG(hXvdCh, ("BXVD_Set1080pScanMode(%d)", e1080pScanMode));

   BDBG_LEAVE(BXVD_Set1080pScanMode_isr);
   return BERR_SUCCESS;
}

/***************************************************************************
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_Get1080pScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode *pe1080pScanMode /* [out] The current 1080p scan mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_Get1080pScanMode);

   rc = BXVD_Get1080pScanMode_isr(
      hXvdCh,
      pe1080pScanMode
      );

   BDBG_LEAVE(BXVD_Get1080pScanMode);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_Get1080pScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode *pe1080pScanMode /* [out] The current 1080p scan mode */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_1080pScanMode eXDM1080pScanMode;

   BDBG_ENTER(BXVD_Get1080pScanMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pe1080pScanMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_Get1080pScanMode_isr(
            hXvdCh->hPictureProvider,
            &eXDM1080pScanMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDM1080pScanMode )
      {
         case BXDM_PictureProvider_1080pScanMode_eDefault:
            *pe1080pScanMode = BXVD_1080pScanMode_eDefault;
            break;

         case BXDM_PictureProvider_1080pScanMode_eAdvanced:
            *pe1080pScanMode = BXVD_1080pScanMode_eAdvanced;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM 1080p scan mode: %d", eXDM1080pScanMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_Get1080pScanMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_Set240iScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_Set240iScanMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_Set240iScanMode_isr(
      hXvdCh,
      e240iScanMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_Set240iScanMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_Set240iScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_240iScanMode eXDM240iScanMode;
   BDBG_ENTER(BXVD_Set240iScanMode_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( e240iScanMode )
   {
      case BXVD_240iScanMode_eForceProgressive:
         eXDM240iScanMode = BXDM_PictureProvider_240iScanMode_eForceProgressive;
         break;

      case BXVD_240iScanMode_eUseEncodedFormat:
         eXDM240iScanMode = BXDM_PictureProvider_240iScanMode_eUseEncodedFormat;
         break;

      default:
      BXVD_DBG_ERR(hXvdCh, ("Invalid 240i scan mode specified: %d", e240iScanMode));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_Set240iScanMode_isr(
            hXvdCh->hPictureProvider,
            eXDM240iScanMode
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_Set240iScanMode(%d)", e240iScanMode));

   BDBG_LEAVE(BXVD_Set240iScanMode_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_Get240iScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_Get240iScanMode);

   rc = BXVD_Get240iScanMode_isr(
      hXvdCh,
      pe240iScanMode
      );

   BDBG_LEAVE(BXVD_Get240iScanMode);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_Get240iScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_240iScanMode eXDM240iScanMode;

   BDBG_ENTER(BXVD_Get240iScanMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pe240iScanMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_Get240iScanMode_isr(
            hXvdCh->hPictureProvider,
            &eXDM240iScanMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDM240iScanMode )
      {
         case BXDM_PictureProvider_240iScanMode_eForceProgressive:
            *pe240iScanMode = BXVD_240iScanMode_eForceProgressive;
            break;

           /* Assume non-AVC 240i/288i is coded correctly */
         case BXDM_PictureProvider_240iScanMode_eUseEncodedFormat:
            *pe240iScanMode = BXVD_240iScanMode_eUseEncodedFormat;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM 240i scan mode: %d", eXDM240iScanMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_Get240iScanMode_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_SetPictureDropMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode ePictureDropMode /* [in] the picture drop mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetPictureDropMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetPictureDropMode_isr(
      hXvdCh,
      ePictureDropMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetPictureDropMode);
   return BERR_TRACE(rc);
}

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_SetPictureDropMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode ePictureDropMode /* [in] the picture drop mode */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PictureDropMode eXDMPictureDropMode;
   BDBG_ENTER(BXVD_SetPictureDropMode_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( ePictureDropMode )
   {
      case BXVD_PictureDropMode_eField:
         eXDMPictureDropMode = BXDM_PictureProvider_PictureDropMode_eField;
         break;

      case BXVD_PictureDropMode_eFrame:
         eXDMPictureDropMode = BXDM_PictureProvider_PictureDropMode_eFrame;
         break;

      default:
         BXVD_DBG_ERR(hXvdCh, ("Invalid picture drop mode specified: %d", ePictureDropMode));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetPictureDropMode_isr(
            hXvdCh->hPictureProvider,
            eXDMPictureDropMode
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetPictureDropMode(%d)", ePictureDropMode));

   BDBG_LEAVE(BXVD_SetPictureDropMode_isr);

   return BERR_TRACE( rc );
}

/***************************************************************************
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetPictureDropMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetPictureDropMode);

   rc = BXVD_GetPictureDropMode_isr(
      hXvdCh,
      pePictureDropMode
      );

   BDBG_LEAVE(BXVD_GetPictureDropMode);
   return BERR_TRACE(rc);
}

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_GetPictureDropMode_isr(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_PictureDropMode eXDMPictureDropMode;

   BDBG_ENTER(BXVD_GetPictureDropMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pePictureDropMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetPictureDropMode_isr(
            hXvdCh->hPictureProvider,
            &eXDMPictureDropMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMPictureDropMode )
      {
         case BXDM_PictureProvider_PictureDropMode_eField:
            *pePictureDropMode = BXVD_PictureDropMode_eField;
            break;

         case BXDM_PictureProvider_PictureDropMode_eFrame:
            *pePictureDropMode = BXVD_PictureDropMode_eFrame;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM picture drop mode: %d", eXDMPictureDropMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_GetPictureDropMode_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************

Summary:
   SW7405-4703: API to Set Horizontal Overscan calculation mode

****************************************************************************/
BERR_Code BXVD_SetHorizontalOverscanMode(
   BXVD_ChannelHandle     hXvdCh,                  /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  eHorizOverscanMode /* [in] Horizontal Overscan mode  */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetHorizontalOverscanMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetHorizontalOverscanMode_isr(
      hXvdCh,
      eHorizOverscanMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetHorizontalOverscanMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetHorizontalOverscanMode_isr(
   BXVD_ChannelHandle     hXvdCh,                  /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  eHorizOverscanMode /* [in] Horizontal Overscan mode  */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_HorizontalOverscanMode eXDMHorizOverscanMode;
   BDBG_ENTER(BXVD_SetPictureDropMode_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( eHorizOverscanMode )
   {
      case BXVD_HorizontalOverscanMode_eAuto:
         eXDMHorizOverscanMode = BXDM_PictureProvider_HorizontalOverscanMode_eAuto;
         break;

      case BXVD_HorizontalOverscanMode_eDisable:
         eXDMHorizOverscanMode = BXDM_PictureProvider_HorizontalOverscanMode_eDisable;
         break;

      default:
         BXVD_DBG_ERR(hXvdCh, ("Invalid horizontal overscan mode specified: %d", eHorizOverscanMode));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetHorizontalOverscanMode_isr(
            hXvdCh->hPictureProvider,
            eXDMHorizOverscanMode
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetHorizontalOverscanMode_isr(%d)", eXDMHorizOverscanMode));

   BDBG_LEAVE(BXVD_SetHorizontalOverscanMode_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************

Summary:
   SW7405-4703: API to Get current Horizontal Overscan mode

****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetHorizontalOverscanMode
(
   BXVD_ChannelHandle           hXvdCh,               /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  *peHorizOverscanMode  /* [out] Horizontal Overscan mode  */
)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetHorizontalOverscanMode);

   rc = BXVD_GetHorizontalOverscanMode_isr(
      hXvdCh,
      peHorizOverscanMode
      );

   BDBG_LEAVE(BXVD_GetHorizontalOverscanMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetHorizontalOverscanMode_isr
(
   BXVD_ChannelHandle           hXvdCh,               /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  *peHorizOverscanMode  /* [out] Horizontal Overscan mode  */
)
{
   BERR_Code rc;
   BXDM_PictureProvider_HorizontalOverscanMode eXDMHorizOverscanMode;

   BDBG_ENTER(BXVD_GetHorizontalOverscanMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peHorizOverscanMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetHorizontalOverscanMode_isr(
            hXvdCh->hPictureProvider,
            &eXDMHorizOverscanMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMHorizOverscanMode )
      {
         case BXDM_PictureProvider_HorizontalOverscanMode_eAuto:
            *peHorizOverscanMode = BXVD_HorizontalOverscanMode_eAuto;
            break;

         case BXDM_PictureProvider_HorizontalOverscanMode_eDisable:
            *peHorizOverscanMode = BXVD_HorizontalOverscanMode_eDisable;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM Horizontal Overscan mode: %d", eXDMHorizOverscanMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_GetHorizontalOverscanMode_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/******************************/
/* Display Manager APIs - TSM */
/******************************/

/***************************************************************************
BXVD_SetDisplayOffset: API used to set the display offset.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_SetDisplayOffset
(
   BXVD_ChannelHandle hXvdCh,
   long               lDisplayOffsetValue
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetDisplayOffset);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetDisplayOffset_isr(
      hXvdCh,
      lDisplayOffsetValue
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetDisplayOffset);
   return BERR_TRACE(rc);
}
#endif

BERR_Code BXVD_SetDisplayOffset_isr
(
   BXVD_ChannelHandle hXvdCh,
   long               lDisplayOffsetValue
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetDisplayOffset_isr);
   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Set the display offset value */
   rc = BXDM_PictureProvider_SetPTSOffset_isr(
            hXvdCh->hPictureProvider,
            (uint32_t) lDisplayOffsetValue
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetDisplayOffset(%ld)", lDisplayOffsetValue));

   BDBG_LEAVE(BXVD_SetDisplayOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BXVD_GetDisplayOffset: API used to get the display offset.
****************************************************************************/
BERR_Code BXVD_GetDisplayOffset
(
   BXVD_ChannelHandle hXvdCh,
   long               *plDisplayOffsetValue
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetDisplayOffset);

   rc = BXVD_GetDisplayOffset_isr(
      hXvdCh,
      plDisplayOffsetValue);

   BDBG_LEAVE(BXVD_GetDisplayOffset);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetDisplayOffset_isr
(
   BXVD_ChannelHandle hXvdCh,
   long               *plDisplayOffsetValue
   )
{
   BERR_Code rc;
   uint32_t uiPTSOffset;

   BDBG_ENTER(BXVD_GetDisplayOffset_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(plDisplayOffsetValue);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* get the display offset value */
   rc = BXDM_PictureProvider_GetPTSOffset_isr(
            hXvdCh->hPictureProvider,
            &uiPTSOffset
            );

   *plDisplayOffsetValue = uiPTSOffset;

   BDBG_LEAVE(BXVD_GetDisplayOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BXVD_SetVideoDisplayMode: Used to set the display mode.
****************************************************************************/
BERR_Code BXVD_SetVideoDisplayMode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayMode   eDisplayMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetVideoDisplayMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetVideoDisplayMode_isr(
      hXvdCh,
      eDisplayMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetVideoDisplayMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetVideoDisplayMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayMode   eDisplayMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_DisplayMode eXDMDisplayMode;
   BDBG_ENTER(BXVD_SetVideoDisplayMode_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( eDisplayMode )
   {
      case BXVD_DisplayMode_eTSMMode:
         eXDMDisplayMode = BXDM_PictureProvider_DisplayMode_eTSM;
         break;

      case BXVD_DisplayMode_eVSYNCMode:
         eXDMDisplayMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
         break;

      default:
      BXVD_DBG_ERR(hXvdCh, ("Invalid video display mode: 0x%x (%d)",
                                 eDisplayMode, eDisplayMode));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* SW7445-2490: With the addition of the SW STC, don't force vsync mode when DQT is enabled.*/
#if 0
   /* If DQT is enabled, force VSYNC mode. */
   if ( true == hXvdCh->stDecoderContext.bReversePlayback )
   {
      eXDMDisplayMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
   }
#endif

   /* Set the video display mode */
   rc = BXDM_PictureProvider_SetDisplayMode_isr(
            hXvdCh->hPictureProvider,
            eXDMDisplayMode
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetVideoDisplayMode(%d)", eDisplayMode));

   BDBG_LEAVE(BXVD_SetVideoDisplayMode_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BXVD_GetVideoDisplayMode: Used to get the display mode.
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetVideoDisplayMode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayMode   *peDisplayMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetVideoDisplayMode);

   rc = BXVD_GetVideoDisplayMode_isr(
      hXvdCh,
      peDisplayMode
      );

   BDBG_LEAVE(BXVD_GetVideoDisplayMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetVideoDisplayMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayMode   *peDisplayMode
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_DisplayMode eXDMDisplayMode;
   BDBG_ENTER(BXVD_GetVideoDisplayMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peDisplayMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* Get the video display mode */
   rc = BXDM_PictureProvider_GetDisplayMode_isr(
            hXvdCh->hPictureProvider,
            &eXDMDisplayMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMDisplayMode )
      {
         case BXDM_PictureProvider_DisplayMode_eTSM:
            *peDisplayMode = BXVD_DisplayMode_eTSMMode;
            break;

         case BXDM_PictureProvider_DisplayMode_eVirtualTSM:
            *peDisplayMode = BXVD_DisplayMode_eVSYNCMode;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM video display mode: 0x%x (%d)",
                                       eXDMDisplayMode, eXDMDisplayMode));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_GetVideoDisplayMode_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
  BXVD_GetDisplayThresholds: API used to get display threshold values
****************************************************************************/
BERR_Code BXVD_GetDisplayThresholds
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayThresholds *pDispThresholds
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetDisplayThresholds);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetDisplayThresholds_isr(
      hXvdCh,
      pDispThresholds
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetDisplayThresholds);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetDisplayThresholds_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DisplayThresholds *pDispThresholds
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

   BDBG_ENTER(BXVD_GetDisplayThresholds_isr);

   /* Check for validity of input ptrs */
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pDispThresholds);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hXvdCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   pDispThresholds->ui32DiscardThreshold =
            eCurrentTSMThresholdSettings.uiTooEarlyThreshold;

   pDispThresholds->ui32VeryLateThreshold =
            eCurrentTSMThresholdSettings.uiTooLateThreshold;

   BDBG_LEAVE(BXVD_GetDisplayThresholds_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BXVD_SetDiscardThreshold: API used to supply the decoder with display
  discard threshold.
****************************************************************************/
BERR_Code BXVD_SetDiscardThreshold
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t           ui32DiscardThreshold
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetDiscardThreshold);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetDiscardThreshold_isr(
      hXvdCh,
      ui32DiscardThreshold
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetDiscardThreshold);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetDiscardThreshold_isr
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t           ui32DiscardThreshold
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

   BDBG_ENTER(BXVD_SetDiscardThreshold_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hXvdCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   if ( BERR_SUCCESS == rc )
   {
      eCurrentTSMThresholdSettings.uiTooEarlyThreshold = ui32DiscardThreshold;

      rc = BXDM_PictureProvider_SetTSMThresholdSettings_isr(
               hXvdCh->hPictureProvider,
               &eCurrentTSMThresholdSettings
               );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetDiscardThreshold(%d)", ui32DiscardThreshold));
   }

   BDBG_LEAVE(BXVD_SetDiscardThreshold_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
  BXVD_SetVeryLateThreshold:    API used to supply the decoder with display
  very late threshold.
****************************************************************************/
BERR_Code BXVD_SetVeryLateThreshold
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t           ui32VeryLateThreshold
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetVeryLateThreshold);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetVeryLateThreshold_isr(
      hXvdCh,
      ui32VeryLateThreshold
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetVeryLateThreshold);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetVeryLateThreshold_isr
(
   BXVD_ChannelHandle hXvdCh,
   uint32_t           ui32VeryLateThreshold
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;
   BDBG_ENTER(BXVD_SetVeryLateThreshold_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hXvdCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   if ( BERR_SUCCESS == rc )
   {
      eCurrentTSMThresholdSettings.uiTooLateThreshold = ui32VeryLateThreshold;

      rc = BXDM_PictureProvider_SetTSMThresholdSettings_isr(
               hXvdCh->hPictureProvider,
               &eCurrentTSMThresholdSettings
               );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetVeryLateThreshold(%d)", ui32VeryLateThreshold));
   }

   BDBG_LEAVE(BXVD_SetVeryLateThreshold_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BXVD_SetSTCInvalidFlag: Used to set the state of the STC invalid flag
****************************************************************************/
BERR_Code BXVD_SetSTCInvalidFlag
(
   BXVD_ChannelHandle hXvdCh,
   bool bStcInvalidFlag
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetSTCInvalidFlag);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetSTCInvalidFlag_isr(
      hXvdCh,
      bStcInvalidFlag
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetSTCInvalidFlag);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetSTCInvalidFlag_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool bStcInvalidFlag
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetSTCInvalidFlag_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetSTCValid_isr(
            hXvdCh->hPictureProvider,
            !bStcInvalidFlag);

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetSTCInvalidFlag(%d)", bStcInvalidFlag));

   BDBG_LEAVE(BXVD_SetSTCInvalidFlag_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BXVD_GetSTCInvalidFlag: Used to get the current state of the STC invalid flag
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetSTCInvalidFlag
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbStcInvalidFlag
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetStcInvalidFlag);

   rc = BXVD_GetSTCInvalidFlag_isr(
      hXvdCh,
      pbStcInvalidFlag
      );

   BDBG_LEAVE(BXVD_GetStcInvalidFlag);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetSTCInvalidFlag_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbStcInvalidFlag
   )
{
   BERR_Code rc;
   bool bCurrentSTCValid;
   BDBG_ENTER(BXVD_GetStcInvalidFlag_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbStcInvalidFlag);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetSTCValid_isr(
            hXvdCh->hPictureProvider,
            &bCurrentSTCValid);

   *pbStcInvalidFlag = !bCurrentSTCValid;

   BDBG_LEAVE(BXVD_GetStcInvalidFlag_isr);
   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
BXVD_SetPtsStcDiffThreshold: Set the Pts/Stc difference threshold
****************************************************************************/
BERR_Code BXVD_SetPtsStcDiffThreshold
(
   BXVD_ChannelHandle hXvdCh,
   long uiPtsStcDiffThreshold
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetPtsStcDiffThreshold);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetPtsStcDiffThreshold_isr(
      hXvdCh,
      uiPtsStcDiffThreshold
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetPtsStcDiffThreshold);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetPtsStcDiffThreshold_isr
(
   BXVD_ChannelHandle hXvdCh,
   long uiPtsStcDiffThreshold
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

   BDBG_ENTER(BXVD_SetPtsStcDiffThreshold_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hXvdCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   if ( BERR_SUCCESS == rc )
   {
      eCurrentTSMThresholdSettings.uiDeltaStcPtsDiffThreshold = uiPtsStcDiffThreshold;

      rc = BXDM_PictureProvider_SetTSMThresholdSettings_isr(
               hXvdCh->hPictureProvider,
               &eCurrentTSMThresholdSettings
               );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetPtsStcDiffThreshold(%ld)", uiPtsStcDiffThreshold));
   }

   BDBG_LEAVE(BXVD_SetPtsStcDiffThreshold_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
BXVD_GetPtsStcDiffThreshold: Get the Pts/Stc difference threshold
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetPtsStcDiffThreshold
(
   BXVD_ChannelHandle hXvdCh,
   long *puiPtsStcDiffThreshold
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetPtsStcDiffThreshold);

   rc = BXVD_GetPtsStcDiffThreshold_isr(
      hXvdCh,
      puiPtsStcDiffThreshold
      );

   BDBG_LEAVE(BXVD_GetPtsStcDiffThreshold);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetPtsStcDiffThreshold_isr
(
   BXVD_ChannelHandle hXvdCh,
   long *puiPtsStcDiffThreshold
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_TSMThresholdSettings eCurrentTSMThresholdSettings;

   BDBG_ENTER(BXVD_GetPtsStcDiffThreshold_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(puiPtsStcDiffThreshold);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetTSMThresholdSettings_isr(
            hXvdCh->hPictureProvider,
            &eCurrentTSMThresholdSettings
            );

   *puiPtsStcDiffThreshold = eCurrentTSMThresholdSettings.uiDeltaStcPtsDiffThreshold;

   BDBG_LEAVE(BXVD_GetPtsStcDiffThreshold_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
   BXVD_SetSTCSource: Set the STC source
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_SetSTCSource
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC eSTC               /* [in] STC Time base */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetSTCSource);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetSTCSource_isr(
      hXvdCh,
      eSTC
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetSTCSource);
   return BERR_TRACE(rc);
}
#endif

BERR_Code BXVD_SetSTCSource_isr(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC eSTC               /* [in] STC Time base */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetSTCSource_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if ((eSTC >= BXVD_STC_eMax) || (eSTC >= BXVD_P_STC_MAX))
   {
      BXVD_DBG_ERR(hXvdCh, ("Invalid STC: 0x%x (%d)",
                            eSTC, eSTC));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetSTCIndex_isr(
      hXvdCh->hPictureProvider,
      (uint32_t)eSTC
      );

   hXvdCh->sDecodeSettings.eSTC = eSTC;
   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetSTCSource(%d)", eSTC));

   BDBG_LEAVE(BXVD_SetSTCSource_isr);
   return BERR_TRACE( rc );
}


/***************************************************************************
   BXVD_GetSTCSource: Get the STC source
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetSTCSource
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC *peSTC             /* [out] STC Time base */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetSTCSource);

   rc = BXVD_GetSTCSource_isr(
      hXvdCh,
      peSTC
      );

   BDBG_LEAVE(BXVD_GetSTCSource);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetSTCSource_isr
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC *peSTC             /* [out] STC Time base */
   )
{
   BERR_Code rc;
   uint32_t uiXDMSTCIndex;

   BDBG_ENTER(BXVD_GetSTCSource_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peSTC);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetSTCIndex_isr(
            hXvdCh->hPictureProvider,
            &uiXDMSTCIndex
            );


   if ( BERR_SUCCESS == rc )
   {
      if ( ((BXVD_STC) uiXDMSTCIndex < BXVD_STC_eMax) && (uiXDMSTCIndex < BXVD_P_STC_MAX))
      {
         *peSTC = (BXVD_STC) uiXDMSTCIndex;
      }
      else
      {
         BXVD_DBG_ERR(hXvdCh, ("Unknown XDM STC Source: 0x%x (%d)",
                               uiXDMSTCIndex, uiXDMSTCIndex));

         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BDBG_LEAVE(BXVD_GetSTCSource_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
   BXVD_GetSTCValue: Get's the STC snapshot
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetSTCValue
(
   BXVD_Handle hXvd,                         /* [in] The XVD handle */
   BXVD_DisplayInterrupt eDisplayInterrupt,  /* [in] Display Interrupt number */
   BXVD_STC eSTC,                            /* [in] STC Time base */
   BXVD_STCInfo *pSTCInfo                    /* [out] STC info */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetSTCValue);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetSTCValue_isr(
      hXvd,
      eDisplayInterrupt,
      eSTC,
      pSTCInfo
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetSTCValue);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetSTCValue_isr
(
   BXVD_Handle hXvd,                         /* [in] The XVD handle */
   BXVD_DisplayInterrupt eDisplayInterrupt,  /* [in] Display Interrupt number */
   BXVD_STC eSTC,                            /* [in] STC Time base */
   BXVD_STCInfo *pSTCInfo                    /* [out] STC info */
   )
{
   BERR_Code rc;
   BXDM_DisplayInterruptInfo stDisplayInterruptInfo;

   BDBG_ENTER(BXVD_GetSTCValue_isr);

   BDBG_ASSERT(hXvd);
   BDBG_ASSERT(pSTCInfo);

   /* Check handle type for correctness */
   if (hXvd->eHandleType != BXVD_P_HandleType_XvdMain)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if ( eDisplayInterrupt >= BXVD_DisplayInterrupt_eMax )
   {
         BXVD_DBG_ERR(hXvd, ("Invalid Display Interrupt requested: %d",
                                  eDisplayInterrupt));
      return BERR_TRACE(BERR_NOT_SUPPORTED);
   }

   rc = BXVD_DisplayInterruptProvider_GetDisplayInterruptInfo_isr(
            hXvd->hXvdDipCh[eDisplayInterrupt],
                                                              &stDisplayInterruptInfo );

   if ( BERR_SUCCESS == rc )
   {
   pSTCInfo->eInterruptPolarity = stDisplayInterruptInfo.eInterruptPolarity;

   if ( ( eSTC >= stDisplayInterruptInfo.uiSTCCount )
        || ( false == stDisplayInterruptInfo.astSTC[eSTC].bValid ) )
   {
         BXVD_DBG_ERR(hXvd, ("Invalid STC requested: %d",
                                  eSTC));
         return BERR_TRACE(BERR_NOT_SUPPORTED);
   }

   pSTCInfo->ui32STCValue = stDisplayInterruptInfo.astSTC[eSTC].uiValue;
   }

   BDBG_LEAVE(BXVD_GetSTCValue_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
 ***************************************************************************/
BERR_Code BXVD_SetClipTime
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t    ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t    ui32StopTime /* [in] Stop display at the specified time/PTS */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetClipTime);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetClipTime_isr(
      hXvdCh,
      eClipTimeType,
      ui32StartTime,
      ui32StopTime
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetClipTime);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetClipTime_isr
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t    ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t    ui32StopTime /* [in] Stop display at the specified time/PTS */
   )
{
   BERR_Code rc;
   BXDM_PictureProvider_ClipTimeSettings stNewClipTimeSettings;

   BDBG_ENTER(BXVD_SetClipTime_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BKNI_Memset( &stNewClipTimeSettings, 0, sizeof( BXDM_PictureProvider_ClipTimeSettings) );

   stNewClipTimeSettings.uiStart = ui32StartTime;
   stNewClipTimeSettings.uiStop = ui32StopTime;
   stNewClipTimeSettings.bValid = true;
   stNewClipTimeSettings.eType = eClipTimeType;

   rc = BXDM_PictureProvider_SetClipTimeSettings_isr(
            hXvdCh->hPictureProvider,
            &stNewClipTimeSettings
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetClipTime(%d,%u,%u)", eClipTimeType, ui32StartTime, ui32StopTime));

   BDBG_LEAVE(BXVD_SetClipTime_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_SetTSMWaitForValidSTC
(
   BXVD_ChannelHandle hXvdCh /* [in] The XVD Channel handle */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetTSMWaitForValidSTC);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetTSMWaitForValidSTC_isr(
      hXvdCh
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetTSMWaitForValidSTC);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetTSMWaitForValidSTC_isr
(
   BXVD_ChannelHandle hXvdCh /* [in] The XVD Channel handle */
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BXVD_SetTSMWaitForValidSTC_isr);

   /* Check to make sure the RequestSTC callback is installed */
   if (hXvdCh->stInterruptCallbackInfo[BXVD_Interrupt_eRequestSTC].BXVD_P_pAppIntCallbackPtr == NULL)
   {
      BXVD_DBG_ERR(hXvdCh, ("BXVD_Interrupt_eRequestSTC callback must be installed"));
      return BERR_TRACE(BERR_NOT_INITIALIZED);
   }

   /* Enable the RequestSTC callback */
   rc = BXVD_EnableInterrupt_isr(hXvdCh,
                                 BXVD_Interrupt_eRequestSTC,
                                 BXVD_InterruptEnable_eEnable);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   /* Invalidate STC */
   rc = BXVD_SetSTCInvalidFlag_isr(hXvdCh,
                                   true);
   if (rc != BERR_SUCCESS)
   {
      return BERR_TRACE(rc);
   }

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetTSMWaitForValidSTC()"));

   BDBG_LEAVE(BXVD_SetTSMWaitForValidSTC_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
   BXVD_SetSwPcrOffset
****************************************************************************/
BERR_Code BXVD_SetSwPcrOffset
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetSwPcrOffset);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetSwPcrOffset_isr(
      hXvdCh,
      uiSwPcrOffset
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetSwPcrOffset);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetSwPcrOffset_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetSwPcrOffset_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetSoftwarePCROffset_isr(
            hXvdCh->hPictureProvider,
            uiSwPcrOffset
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetSwPcrOffset_isr(%d)", uiSwPcrOffset));

   BDBG_LEAVE(BXVD_SetSwPcrOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BXVD_GetSwPcrOffset
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetSwPcrOffset
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetSwPcrOffset);

   rc = BXVD_GetSwPcrOffset_isr(
      hXvdCh,
      puiSwPcrOffset
      );

   BDBG_LEAVE(BXVD_GetSwPcrOffset);
   return BERR_TRACE(rc);
}
#endif

BERR_Code BXVD_GetSwPcrOffset_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetSwPcrOffset_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(puiSwPcrOffset);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetSoftwarePCROffset_isr(
            hXvdCh->hPictureProvider,
            puiSwPcrOffset
            );

   BDBG_LEAVE(BXVD_GetSwPcrOffset_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BXVD_SetHwPcrOffsetEnable
****************************************************************************/
BERR_Code BXVD_SetHwPcrOffsetEnable
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetHwPcrOffsetEnable);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetHwPcrOffsetEnable_isr(
      hXvdCh,
      bHwPcrOffsetEnable
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetHwPcrOffsetEnable);
   return BERR_TRACE(rc);
}


BERR_Code BXVD_SetHwPcrOffsetEnable_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetHwPcrOffsetEnable_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetHardwarePCROffsetMode_isr(
            hXvdCh->hPictureProvider,
            bHwPcrOffsetEnable
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetHwPcrOffsetEnable_isr(%d)", bHwPcrOffsetEnable));

   BDBG_LEAVE(BXVD_SetHwPcrOffsetEnable_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BXVD_GetHwPcrOffsetEnable
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetHwPcrOffsetEnable
(
   BXVD_ChannelHandle hXvdCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetHwPcrOffsetEnable);

   rc = BXVD_GetHwPcrOffsetEnable_isr(
      hXvdCh,
      pbHwPcrOffsetEnable
      );

   BDBG_LEAVE(BXVD_GetHwPcrOffsetEnable);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetHwPcrOffsetEnable_isr
(
   BXVD_ChannelHandle hXvdCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetHwPcrOffsetEnable_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbHwPcrOffsetEnable);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetHardwarePCROffsetMode_isr(
            hXvdCh->hPictureProvider,
            pbHwPcrOffsetEnable
            );

   BDBG_LEAVE(BXVD_GetHwPcrOffsetEnable_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
   BXVD_SetVsyncPlaybackRate
   DEPRECATED: SW7400-2870: use BXVD_SetPlaybackRate(isr)
****************************************************************************/

BERR_Code BXVD_SetVsyncPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiVsyncPlaybackRate
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetVsyncPlaybackRate);

   BKNI_EnterCriticalSection();

   rc = BXVD_SetVsyncPlaybackRate_isr( hXvdCh, uiVsyncPlaybackRate );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetVsyncPlaybackRate);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetVsyncPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiVsyncPlaybackRate
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_Rate stPlaybackRate;
   BDBG_ENTER(BXVD_SetVsyncPlaybackRate_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* SW7400-2870:
    * XDM uses a single variable to store the vsync playback rate.  This value is set
    * both by this API and BXVD_PVR_EnablePause; the pause state has precedence.
    * As a result, if the system is paused when this API is called, the new playback
    * rate will be saved but not applied until the system is un-paused.
    */

   /* "uiVsyncPlaybackRate" is a percentage . */
   BKNI_Memset(&stPlaybackRate, 0, sizeof( BXDM_Picture_Rate ) );

   stPlaybackRate.uiNumerator = uiVsyncPlaybackRate;
   stPlaybackRate.uiDenominator = 100;

   /* Only apply the new playback rate if the system in NOT paused. */
   if ( false == hXvdCh->bPauseActive )
   {
      rc = BXDM_PictureProvider_SetPlaybackRate_isr(
            hXvdCh->hPictureProvider,
            &stPlaybackRate
            );
   }

   hXvdCh->stSavedPlaybackRate = stPlaybackRate;
   hXvdCh->bSavedPlaybackRateValid = true;

   BDBG_LEAVE(BXVD_SetVsyncPlaybackRate_isr);

   return BERR_TRACE( rc );
}

/***************************************************************************
   BXVD_GetVsyncPlaybackRate
   SW7400-2870:
   DEPRECATED: use BXVD_GetPlaybackRate(isr)
****************************************************************************/

BERR_Code BXVD_GetVsyncPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiVsyncPlaybackRate
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetVsyncPlaybackRate);

   BKNI_EnterCriticalSection();

   rc = BXVD_GetVsyncPlaybackRate_isr( hXvdCh, puiVsyncPlaybackRate );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetVsyncPlaybackRate);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetVsyncPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiVsyncPlaybackRate
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_Rate stCurrentPlaybackRate;
   BDBG_ENTER(BXVD_GetVsyncPlaybackRate_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(puiVsyncPlaybackRate);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   /* SW7400-2870: if not paused by a call to BXVD_PVR_EnablePause, retrieve
    * the playback rate from XDM.  If paused, return the valued saved in the
    * channel structure.
    */
   if ( false == hXvdCh->bPauseActive )
   {
      rc = BXDM_PictureProvider_GetPlaybackRate_isr(
                  hXvdCh->hPictureProvider,
                  &stCurrentPlaybackRate
                  );

      if ( BERR_SUCCESS != rc )
      {
         *puiVsyncPlaybackRate = 0;
         BXVD_DBG_WRN(hXvdCh, ("BXVD_GetVsyncPlaybackRate_isr() BXDM_PictureProvider_GetPlaybackRate_isr returned %d", rc ));
         return BERR_TRACE( rc );
      }
   }
   else
   {
      if ( true == hXvdCh->bSavedPlaybackRateValid )
      {
         stCurrentPlaybackRate = hXvdCh->stSavedPlaybackRate;
      }
      else
      {
         /* It should be impossible to hit this case.  If "bPauseActive" is true,
          * there should always be a saved playback rate.  Add this warning in the
          * event a logic bug creeps in.
          */
         stCurrentPlaybackRate.uiNumerator = 1;
         stCurrentPlaybackRate.uiDenominator = 1;
         BXVD_DBG_WRN(hXvdCh, ("BXVD_GetVsyncPlaybackRate_isr() bPauseActive but no saved playback rate."));
      }
   }

   /* Prevent a divide by '0'.*/
   if ( 0 == stCurrentPlaybackRate.uiDenominator )
   {
      *puiVsyncPlaybackRate = 0;
      BDBG_WRN(("BXVD_GetVsyncPlaybackRate_isr() uiDenominator == 0!"));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* "*puiVsyncPlaybackRate" is a percentage, multiply the numerator/demonintor ratio
    * by 100 to return the appropriate value.
    */
   *puiVsyncPlaybackRate = (stCurrentPlaybackRate.uiNumerator * 100) / stCurrentPlaybackRate.uiDenominator;

   BDBG_LEAVE(BXVD_GetVsyncPlaybackRate_isr);

   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
   SW7400-2870:
   BXVD_SetPlaybackRate
   see comment in bxvd.h
****************************************************************************/

BERR_Code BXVD_SetPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings stPlaybackRateSettings
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetPlaybackRate);

   BKNI_EnterCriticalSection();

   rc = BXVD_SetPlaybackRate_isr( hXvdCh, stPlaybackRateSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetPlaybackRate);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings stPlaybackRateSettings
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_Rate stXDMPlaybackRate;

   BDBG_ENTER(BXVD_SetPlaybackRate_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }


   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetPlaybackRate_isr(%d/%d)",
            stPlaybackRateSettings.uiNumerator,
            stPlaybackRateSettings.uiDenominator));

   /* Prevent a divide by '0'.*/
   if ( 0 == stPlaybackRateSettings.uiDenominator )
   {
      BDBG_WRN(("BXVD_SetPlaybackRate_isr90 uiDenominator == 0!"));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* XDM uses a single variable to store the vsync playback rate.  This value is set
    * both by this API and BXVD_PVR_EnablePause; the pause state has precedence.
    * As a result, if the system is paused when this API is called, the new playback
    * rate will be saved but not applied until the system is un-paused.
    */
   BKNI_Memset(&stXDMPlaybackRate, 0, sizeof( BXDM_Picture_Rate ) );

   stXDMPlaybackRate.uiNumerator    = stPlaybackRateSettings.uiNumerator;
   stXDMPlaybackRate.uiDenominator  = stPlaybackRateSettings.uiDenominator;

   /* Only apply the new playback rate if the system in NOT paused. */
   if ( false == hXvdCh->bPauseActive )
   {
      rc = BXDM_PictureProvider_SetPlaybackRate_isr(
                  hXvdCh->hPictureProvider,
                  &stXDMPlaybackRate
                  );
   }

   hXvdCh->stSavedPlaybackRate = stXDMPlaybackRate;
   hXvdCh->bSavedPlaybackRateValid = true;

   BDBG_LEAVE(BXVD_SetPlaybackRate_isr);

   return BERR_TRACE( rc );
}

/***************************************************************************
   BXVD_GetPlaybackRate
   see comment in bxvd.h
****************************************************************************/

BERR_Code BXVD_GetPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings * pstPlaybackRateSettings
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetPlaybackRate);

   BKNI_EnterCriticalSection();

   rc = BXVD_GetPlaybackRate_isr( hXvdCh, pstPlaybackRateSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetPlaybackRate);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings * pstPlaybackRateSettings
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_Rate stCurrentPlaybackRate;
   BDBG_ENTER(BXVD_GetPlaybackRate_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pstPlaybackRateSettings);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   if ( false == hXvdCh->bPauseActive )
   {
      rc = BXDM_PictureProvider_GetPlaybackRate_isr(
                  hXvdCh->hPictureProvider,
                  &stCurrentPlaybackRate
                  );

      if ( BERR_SUCCESS != rc )
      {
         pstPlaybackRateSettings->uiNumerator = 1;
         pstPlaybackRateSettings->uiDenominator = 1;
         BXVD_DBG_WRN(hXvdCh, ("BXVD_GetPlaybackRate_isr() BXDM_PictureProvider_GetPlaybackRate_isr returned %d", rc ));
         return BERR_TRACE( rc );
      }
   }
   else
   {
      if ( true == hXvdCh->bSavedPlaybackRateValid )
      {
         stCurrentPlaybackRate = hXvdCh->stSavedPlaybackRate;
      }
      else
      {
         /* It should be impossible to hit this case.  If "bPauseActive" is true,
          * there should always be a saved playback rate.  Add this warning in the
          * event a logic bug creeps in.
          */
         stCurrentPlaybackRate.uiNumerator = 1;
         stCurrentPlaybackRate.uiDenominator = 1;
         BXVD_DBG_WRN(hXvdCh, ("BXVD_GetPlaybackRate_isr() bPauseActive but no saved playback rate."));
      }
   }

   pstPlaybackRateSettings->uiNumerator = stCurrentPlaybackRate.uiNumerator;
   pstPlaybackRateSettings->uiDenominator = stCurrentPlaybackRate.uiDenominator;

   BDBG_LEAVE(BXVD_GetPlaybackRate_isr);

   return BERR_TRACE( rc );
}


/***************************************************************************
   BXVD_GetPPBParameterInfo
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetPPBParameterInfo(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PPBParameterInfo *pPPBParameterInfo
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetPPBParameterInfo);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetPPBParameterInfo_isr( hXvdCh, pPPBParameterInfo );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetPPBParameterInfo);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetPPBParameterInfo_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PPBParameterInfo *pPPBParameterInfo
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetPPBParameterInfo_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pPPBParameterInfo);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetCurrentPicture_isr(
            hXvdCh->hPictureProvider,
            pPPBParameterInfo
            );

   BDBG_LEAVE(BXVD_GetPPBParameterInfo_isr);
   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetPPBParameterQueueInfo(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_PPBParameterInfo* apstPPBParameterInfo[],
   uint32_t uiPPBParameterInfoCount,
   uint32_t *puiValidPPBParameterInfoCount
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetPPBParameterQueueInfo);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetPPBParameterQueueInfo_isr( hXvdCh,
                                           apstPPBParameterInfo,
                                           uiPPBParameterInfoCount,
                                           puiValidPPBParameterInfoCount );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetPPBParameterQueueInfo);
   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL  SWSTB-461 */

BERR_Code BXVD_GetPPBParameterQueueInfo_isr(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_PPBParameterInfo* apstPPBParameterInfo[],
   uint32_t uiPPBParameterInfoCount,
   uint32_t *puiValidPPBParameterInfoCount
   )
{
   BERR_Code rc = BERR_UNKNOWN;

   BDBG_ENTER(BXVD_GetPPBParameterQueueInfo_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(apstPPBParameterInfo);
   BDBG_ASSERT(puiValidPPBParameterInfoCount);
   BDBG_ASSERT(uiPPBParameterInfoCount > 0);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXVD_Decoder_GetPPBParameterQueueInfo_isr(
            hXvdCh,
            apstPPBParameterInfo,
            uiPPBParameterInfoCount,
            puiValidPPBParameterInfoCount);

   BDBG_LEAVE(BXVD_GetPPBParameterQueueInfo_isr);

   return BERR_TRACE(rc);
}

/***************************************************************************
    BXVD_SetErrorHandlingMode: Used to set the error handling mode.
****************************************************************************/
BERR_Code BXVD_SetErrorHandlingMode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling eErrorMode
)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_SetErrorHandlingMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_SetErrorHandlingMode_isr(
      hXvdCh,
      eErrorMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetErrorHandlingMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetErrorHandlingMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling eErrorMode
)
{
   BERR_Code rc;
   BXDM_PictureProvider_ErrorHandlingMode eXDMErrorHandlingMode;

   BDBG_ENTER(BXVD_SetErrorHandlingMode_isr);
   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   switch ( eErrorMode )
   {
      case BXVD_ErrorHandling_eOff:
         eXDMErrorHandlingMode = BXDM_PictureProvider_ErrorHandlingMode_eOff;
         break;

      case BXVD_ErrorHandling_ePicture:
         eXDMErrorHandlingMode = BXDM_PictureProvider_ErrorHandlingMode_ePicture;
         break;

      case BXVD_ErrorHandling_ePrognostic:
         eXDMErrorHandlingMode = BXDM_PictureProvider_ErrorHandlingMode_ePrognostic;
         break;

      default:
         BXVD_DBG_ERR(hXvdCh, ("Invalid Error Handling Mode: 0x%x (%d)",
                  eErrorMode, eErrorMode));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   rc = BXDM_PictureProvider_SetErrorHandlingMode_isr(
            hXvdCh->hPictureProvider,
            eXDMErrorHandlingMode
            );

   hXvdCh->sDecodeSettings.eErrorHandling = eErrorMode;
   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetErrorHandlingMode(%d)", eErrorMode));

   BDBG_LEAVE(BXVD_SetErrorHandlingMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
    BXVD_GetErrorHandlingMode: Used to get the error handling mode.
****************************************************************************/
BERR_Code BXVD_GetErrorHandlingMode
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling * peErrorMode
)
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_GetErrorHandlingMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_GetErrorHandlingMode_isr(
      hXvdCh,
      peErrorMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetErrorHandlingMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetErrorHandlingMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling * peErrorMode
)
{
   BERR_Code rc;
   BXDM_PictureProvider_ErrorHandlingMode eXDMErrorHandlingMode;

   BDBG_ENTER(BXVD_GetErrorHandlingMode_isr);
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(peErrorMode);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetErrorHandlingMode_isr(
            hXvdCh->hPictureProvider,
            &eXDMErrorHandlingMode
            );

   if ( BERR_SUCCESS == rc )
   {
      switch ( eXDMErrorHandlingMode )
      {
         case BXDM_PictureProvider_ErrorHandlingMode_eOff:
            *peErrorMode = BXVD_ErrorHandling_eOff;
            break;

         case BXDM_PictureProvider_ErrorHandlingMode_ePicture:
            *peErrorMode = BXVD_ErrorHandling_ePicture;
            break;

         case BXDM_PictureProvider_ErrorHandlingMode_ePrognostic:
            *peErrorMode = BXVD_ErrorHandling_ePrognostic;
            break;

         default:
            BXVD_DBG_ERR(hXvdCh, ("Unknown XDM Error Handling Mode: 0x%x (%d)",
                     eXDMErrorHandlingMode, eXDMErrorHandlingMode));

            return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   }

   BXVD_DBG_MSG(hXvdCh, ("BXVD_GetErrorHandlingMode(%d)", hXvdCh->sDecodeSettings.eErrorHandling ));

   BDBG_LEAVE(BXVD_GetErrorHandlingMode_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
    BXVD_[Set/Get]JitterToleranceImprovementEnable: Used to enable/disable
    Jitter Tolerance Improvement logic in XVD DM
****************************************************************************/

BERR_Code BXVD_SetJitterToleranceImprovementEnable(
   BXVD_ChannelHandle hXvdCh,
   bool bEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_SetJitterToleranceImprovementEnable );

   BKNI_EnterCriticalSection();
   rc = BXVD_SetJitterToleranceImprovementEnable_isr(
      hXvdCh,
      bEnable
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_SetJitterToleranceImprovementEnable );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_SetJitterToleranceImprovementEnable_isr(
   BXVD_ChannelHandle hXvdCh,
   bool bEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER( BXVD_SetJitterToleranceImprovementEnable_isr );

   BDBG_ASSERT( hXvdCh );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetJitterToleranceImprovementMode_isr(
            hXvdCh->hPictureProvider,
            bEnable
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetJitterToleranceImprovementEnable(%d)", bEnable));

   BDBG_LEAVE( BXVD_SetJitterToleranceImprovementEnable_isr );

   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetJitterToleranceImprovementEnable(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_GetJitterToleranceImprovementEnable );

   BKNI_EnterCriticalSection();
   rc = BXVD_GetJitterToleranceImprovementEnable_isr(
      hXvdCh,
      pbEnable
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_GetJitterToleranceImprovementEnable );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetJitterToleranceImprovementEnable_isr(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER( BXVD_GetJitterToleranceImprovementEnable_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pbEnable );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetJitterToleranceImprovementMode_isr(
            hXvdCh->hPictureProvider,
            pbEnable
            );

   BDBG_LEAVE( BXVD_GetJitterToleranceImprovementEnable_isr );

   return BERR_TRACE( rc );
}

/***************************************************************************
    BXVD_[Set/Get]MPEGPulldownOverride:
    Set the pulldown override mode for MPEG content.
****************************************************************************/

BERR_Code BXVD_SetMPEGPulldownOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride ePulldownOverride
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_SetMPEGPulldownOverride );

   BKNI_EnterCriticalSection();

   rc = BXVD_SetMPEGPulldownOverride_isr( hXvdCh, ePulldownOverride );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_SetMPEGPulldownOverride );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_SetMPEGPulldownOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride ePulldownOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_SetMPEGPulldownOverride_isr );

   BDBG_ASSERT( hXvdCh );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   hXvdCh->stDecoderContext.ePulldownOverride = ePulldownOverride;

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetMPEGPulldownOverride(%d)", ePulldownOverride));

   BDBG_LEAVE( BXVD_SetMPEGPulldownOverride_isr );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetMPEGPulldownOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride * pePulldownOverride
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_GetMPEGPulldownOverride );

   BKNI_EnterCriticalSection();

   rc = BXVD_GetMPEGPulldownOverride_isr( hXvdCh, pePulldownOverride );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_GetMPEGPulldownOverride );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetMPEGPulldownOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride * pePulldownOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_GetMPEGPulldownOverride_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pePulldownOverride );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   *pePulldownOverride = hXvdCh->stDecoderContext.ePulldownOverride;

   BDBG_LEAVE( BXVD_GetMPEGPulldownOverride_isr );

   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL  SWSTB-461 */

/***************************************************************************

   SW7405-4117: deinterlacer max height is used in conjuction with
   BXVD_DisplayFieldType_eAuto to choose either eSingleField or eBothField
   based on the steam height during slow motion (and preroll).

****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */

BERR_Code BXVD_SetDeinterlacerMaxHeight(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiMaxHeight
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_SetDeinterlacerMaxHeight );

   BKNI_EnterCriticalSection();

   rc = BXVD_SetDeinterlacerMaxHeight_isr( hXvdCh, uiMaxHeight );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_SetDeinterlacerMaxHeight );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_SetDeinterlacerMaxHeight_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiMaxHeight
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_SetDeinterlacerMaxHeight_isr );

   BDBG_ASSERT( hXvdCh );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetDeinterlacerMaxHeight_isr(
            hXvdCh->hPictureProvider,
            uiMaxHeight
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetDeinterlacerMaxHeight(%d)", uiMaxHeight));

   BDBG_LEAVE( BXVD_SetDeinterlacerMaxHeight_isr );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetDeinterlacerMaxHeight(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiMaxHeight
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_GetDeinterlacerMaxHeight );

   BKNI_EnterCriticalSection();

   rc = BXVD_GetDeinterlacerMaxHeight_isr( hXvdCh, puiMaxHeight );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_GetDeinterlacerMaxHeight );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetDeinterlacerMaxHeight_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiMaxHeight
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_GetDeinterlacerMaxHeight_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( puiMaxHeight );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetDeinterlacerMaxHeight_isr(
            hXvdCh->hPictureProvider,
            puiMaxHeight
            );

   BDBG_LEAVE( BXVD_GetDeinterlacerMaxHeight_isr );

   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
   SWDEPRECATED-1003:
   BXVD_SetFrameRateOverride
   see comment in bxvd.h
****************************************************************************/

BERR_Code BXVD_SetFrameRateOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_SetFrameRateOverride);

   BKNI_EnterCriticalSection();

   rc = BXVD_SetFrameRateOverride_isr( hXvdCh, pstFrameRateOverrideSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_SetFrameRateOverride);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_SetFrameRateOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_FrameRateOverride  stFrameRateOverride;

   BDBG_ENTER(BXVD_SetFrameRateOverride_isr);

   BDBG_ASSERT(hXvdCh);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetFrameRateOverride_isr() bValid:%d rate: %d/%d",
            pstFrameRateOverrideSettings->bValid,
            pstFrameRateOverrideSettings->stRate.uiNumerator,
            pstFrameRateOverrideSettings->stRate.uiDenominator
            ));

   BKNI_Memset( &stFrameRateOverride, 0, sizeof( BXDM_Picture_FrameRateOverride ));

   if ( true == pstFrameRateOverrideSettings->bValid )
   {
      if ( 0 == pstFrameRateOverrideSettings->stRate.uiNumerator
            || 0 == pstFrameRateOverrideSettings->stRate.uiDenominator
         )
      {
         /* Set the override to invalid if the specified rate is unknown.  */
         stFrameRateOverride.bValid = false;
         stFrameRateOverride.stRate.uiNumerator = 1;
         stFrameRateOverride.stRate.uiDenominator = 1;

         BDBG_ERR(("BXVD_SetFrameRateOverride_isr() invalid rate: %d/%d",
                        pstFrameRateOverrideSettings->stRate.uiNumerator,
                        pstFrameRateOverrideSettings->stRate.uiDenominator));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
      else
      {
         stFrameRateOverride.bValid = true;
         stFrameRateOverride.stRate.uiNumerator = pstFrameRateOverrideSettings->stRate.uiNumerator;
         stFrameRateOverride.stRate.uiDenominator = pstFrameRateOverrideSettings->stRate.uiDenominator;
         stFrameRateOverride.bTreatAsSingleElement = pstFrameRateOverrideSettings->bTreatAsSingleElement;
      }
   }
   else
   {
      stFrameRateOverride.bValid = false;
      stFrameRateOverride.stRate.uiNumerator = 1;
      stFrameRateOverride.stRate.uiDenominator = 1;
   }

   rc = BXDM_PictureProvider_SetFrameRateOverride_isr(
                  hXvdCh->hPictureProvider,
                  &stFrameRateOverride
                  );

   BDBG_LEAVE(BXVD_SetFrameRateOverride_isr);

   return BERR_TRACE( rc );
}

/***************************************************************************
   BXVD_GetFrameRateOverride
   see comment in bxvd.h
****************************************************************************/

BERR_Code BXVD_GetFrameRateOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_GetFrameRateOverride);

   BKNI_EnterCriticalSection();

   rc = BXVD_GetFrameRateOverride_isr( hXvdCh, pstFrameRateOverrideSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_GetFrameRateOverride);

   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetFrameRateOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_FrameRateOverride stCurrentFrameRateOverride;
   BDBG_ENTER(BXVD_GetFrameRateOverride_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pstFrameRateOverrideSettings);

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetFrameRateOverride_isr(
                  hXvdCh->hPictureProvider,
                  &stCurrentFrameRateOverride
                  );

   pstFrameRateOverrideSettings->bValid               = stCurrentFrameRateOverride.bValid;
   pstFrameRateOverrideSettings->stRate.uiNumerator   = stCurrentFrameRateOverride.stRate.uiNumerator;
   pstFrameRateOverrideSettings->stRate.uiDenominator = stCurrentFrameRateOverride.stRate.uiDenominator;
   pstFrameRateOverrideSettings->bTreatAsSingleElement = stCurrentFrameRateOverride.bTreatAsSingleElement;

   BDBG_LEAVE(BXVD_GetFrameRateOverride_isr);

   return BERR_TRACE( rc );
}

/***************************************************************************
   SW7422-72: API's for specifying the 3D orientation of pictures
   see comment in bxvd.h
****************************************************************************/

BERR_Code BXVD_Set3D(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_3DSetting * pst3DSettings
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_Set3D );

   BKNI_EnterCriticalSection();

   rc = BXVD_Set3D_isr( hXvdCh, pst3DSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_Set3D );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_Set3D_isr(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_3DSetting * pst3DSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_Set3D_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pst3DSettings );


   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_Set3D_isr(
            hXvdCh->hPictureProvider,
            pst3DSettings
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_Set3D pst3DSettings: 0x%0*lx", BXVD_P_DIGITS_IN_LONG, (long) pst3DSettings));

   BDBG_LEAVE( BXVD_Set3D_isr );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_Get3D(
   BXVD_ChannelHandle hXvdCh,
   BXVD_3DSetting * pst3DSettings
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_Get3D );

   BKNI_EnterCriticalSection();

   rc = BXVD_Get3D_isr( hXvdCh, pst3DSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_Get3D );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_Get3D_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_3DSetting * pst3DSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_Get3D_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pst3DSettings );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_Get3D_isr(
            hXvdCh->hPictureProvider,
            pst3DSettings
            );

   BDBG_LEAVE( BXVD_Get3D_isr );

   return BERR_TRACE( rc );
}


BERR_Code BXVD_GetDefault3D(
   BXVD_ChannelHandle hXvdCh,
   BXVD_3DSetting * pst3DSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_GetDefault3D );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pst3DSettings );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetDefault3D(
            hXvdCh->hPictureProvider,
            pst3DSettings
            );

#if BXVD_LEGACY_MVC_SUPPORT
      /* SW7422-72: the original MVC behavior on the DVD chips was to set "pNext" in the
       * MFD structure of frame 0 to point to frame 1.  When running on these older chip,
       * "bSetNextPointer" will convey to XDM that both "pEhanced" and "pNext" should be set.
       */
      pst3DSettings->bSetNextPointer = true;
#endif


   BDBG_LEAVE( BXVD_GetDefault3D );

   return BERR_TRACE( rc );
}

/***************************************************************************
   SW7425-1264: support for a synthesized STC; can create a clock that
   run backwards.
   see comment in bxvd.h
****************************************************************************/

BERR_Code BXVD_SetClockOverride(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_ClockOverride * pstClockOverride
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_SetClockOverride );

   BKNI_EnterCriticalSection();

   rc = BXVD_SetClockOverride_isr( hXvdCh, pstClockOverride );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_SetClockOverride );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_SetClockOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_ClockOverride * pstClockOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_SetClockOverride_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pstClockOverride );


   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetClockOverride_isr(
            hXvdCh->hPictureProvider,
            pstClockOverride
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_SetClockOverride:: Enable:%d  initialValue:%d load:%d stcDelta:%d)",
                  pstClockOverride->bEnableClockOverride,
                  pstClockOverride->uiStcValue,
                  pstClockOverride->bLoadSwStc,
                  pstClockOverride->iStcDelta
                  ));


   BDBG_LEAVE( BXVD_SetClockOverride_isr );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetClockOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ClockOverride * pstClockOverride
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_GetClockOverride );

   BKNI_EnterCriticalSection();

   rc = BXVD_GetClockOverride_isr( hXvdCh, pstClockOverride );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_GetClockOverride );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_GetClockOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ClockOverride * pstClockOverride
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_GetClockOverride_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pstClockOverride );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetClockOverride_isr(
            hXvdCh->hPictureProvider,
            pstClockOverride
            );

   BDBG_LEAVE( BXVD_GetClockOverride_isr );

   return BERR_TRACE( rc );
}

/***************************************************************************
   SW7425-1064: support for linking channels
   see comment in bxvd.h
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */

static BERR_Code BXVD_S_InstallXmo(
   BXVD_ChannelHandle hXvdChBase
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXVD_ChannelHandle hXvdChEnhanced;
   bool bInstallXmo;

   BDBG_ENTER( BXVD_S_InstallXmo );

   BDBG_ASSERT( hXvdChBase );

   /* Verify that the second channel is linked to the first. */
   hXvdChEnhanced = hXvdChBase->hXvdChLinked;

   if ( NULL == hXvdChEnhanced )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_S_InstallXmo:: handle for the enhanced channel is NULL"));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }

   /* If the XMO has been allocated and both channels are of the
    * correct type, link them to the XMO.
    */
   bInstallXmo = ( NULL != hXvdChBase->hXmo );
   bInstallXmo &= ( BXVD_P_ChannelType_eBase == hXvdChBase->eChannelType );
   bInstallXmo &= ( BXVD_P_ChannelType_eEnhanced == hXvdChEnhanced->eChannelType );

   if ( true == bInstallXmo )
   {
      BXDM_Decoder_Interface stDecoderInterface;
      void *pPrivateContext;

      /* Get the base decoder interface. */
      BXVD_Decoder_GetDMInterface(
               hXvdChBase,
               &stDecoderInterface,
               &pPrivateContext
               );

      /* Register the base decoder interface with XMO. */
      BXDM_PictureProvider_XMO_SetDecoderInterface(
               hXvdChBase->hXmo,
               BXDM_PP_XMO_Base_Index,
               &stDecoderInterface,
               pPrivateContext
               );

      /* Get the enhanced decoder interface. */
      BXVD_Decoder_GetDMInterface(
               hXvdChEnhanced,
               &stDecoderInterface,
               &pPrivateContext
               );

      /* Register the enhanced decoder interface with XMO. */
      BXDM_PictureProvider_XMO_SetDecoderInterface(
               hXvdChBase->hXmo,
               BXDM_PP_XMO_Enhanced_Index,
               &stDecoderInterface,
               pPrivateContext
               );

      /* Get the XMO decoder interface. */
      BXDM_PictureProvider_XMO_GetDMInterface(
               hXvdChBase->hXmo,
               &stDecoderInterface,
               &pPrivateContext
               );

      /* Bind XMO to XDM (the Picture Provider)
       * TODO: does BXDM_PictureProvider_SetDecoderInterface_isr
       * need to be an ISR call?
       */
      BKNI_EnterCriticalSection();

      BXDM_PictureProvider_SetDecoderInterface_isr(
               hXvdChBase->hPictureProvider,
               &stDecoderInterface,
               pPrivateContext
               );

      BKNI_LeaveCriticalSection();


   }     /* end of if ( true == bInstallXmo ) */
   else
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_S_InstallXmo:: install failed: hXmo: 0x%0*lx  base type:%d  enhanced type:%d",
                                     BXVD_P_DIGITS_IN_LONG, (long)hXvdChBase->hXmo,
                                     hXvdChBase->eChannelType,
                                     hXvdChEnhanced->eChannelType
                                     ));
      rc = BERR_INVALID_PARAMETER;
   }

Done:

   BDBG_LEAVE( BXVD_S_InstallXmo );

   return BERR_TRACE( rc );
}


static BERR_Code BXVD_S_UninstallXmo(
   BXVD_ChannelHandle hXvdChBase
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXVD_ChannelHandle hXvdChEnhanced;
   bool bInstallXmo;

   BDBG_ENTER( BXVD_S_UninstallXmo );

   BDBG_ASSERT( hXvdChBase );

   /* Verify that the second channel is linked to the first. */
   hXvdChEnhanced = hXvdChBase->hXvdChLinked;

   if ( NULL == hXvdChEnhanced )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_S_UninstallXmo:: handle for the enhanced channel is NULL"));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }

   /* If the XMO has been allocated and both channels are of the
    * correct type, link them to the XMO.
    */
   bInstallXmo = ( NULL != hXvdChBase->hXmo );
   bInstallXmo &= ( BXVD_P_ChannelType_eBase == hXvdChBase->eChannelType );
   bInstallXmo &= ( BXVD_P_ChannelType_eEnhanced == hXvdChEnhanced->eChannelType );

   if ( true == bInstallXmo )
   {
      BXDM_Decoder_Interface stDecoderInterface;
      void *pPrivateContext;

      /* Get the base decoder interface. */
      BXVD_Decoder_GetDMInterface(
               hXvdChBase,
               &stDecoderInterface,
               &pPrivateContext
               );

      /* Bind XMO to XDM (the Picture Provider)
       * TODO: does BXDM_PictureProvider_SetDecoderInterface_isr
       * need to be an ISR call?
       */
      BKNI_EnterCriticalSection();

      BXDM_PictureProvider_SetDecoderInterface_isr(
               hXvdChBase->hPictureProvider,
               &stDecoderInterface,
               pPrivateContext
               );

      BKNI_LeaveCriticalSection();


   }     /* end of if ( true == bInstallXmo ) */
   else
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_S_UninstallXmo:: install failed: hXmo: 0x%0*lx  base type:%d  enhanced type:%d",
                                     BXVD_P_DIGITS_IN_LONG, (long)hXvdChBase->hXmo,
                                     hXvdChBase->eChannelType,
                                     hXvdChEnhanced->eChannelType
                                     ));
      rc = BERR_INVALID_PARAMETER;
   }

Done:

   BDBG_LEAVE( BXVD_S_UninstallXmo );

   return BERR_TRACE( rc );
}

/***************************************************************************
Summary:


Description:
***************************************************************************/

BERR_Code BXVD_LinkChannels(
   BXVD_ChannelHandle hXvdChBase,
   BXVD_ChannelHandle hXvdChEnhanced
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXVD_LinkChannels );

   BDBG_ASSERT(hXvdChBase);
   BDBG_ASSERT(hXvdChEnhanced);

   /* Check base handle type for correctness */
   if ( hXvdChBase->eHandleType != BXVD_P_HandleType_XvdChannel )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_LinkChannels:: Invalid base handle type"));
      rc = BXVD_ERR_INVALID_HANDLE;
      goto Done;
   }

   /* Check enhanced handle type for correctness */
   if ( hXvdChEnhanced->eHandleType != BXVD_P_HandleType_XvdChannel )
   {
      BXVD_DBG_ERR( hXvdChEnhanced, ("BXVD_LinkChannels:: Invalid enhanced handle type"));
      rc = BXVD_ERR_INVALID_HANDLE;
      goto Done;
   }

   /* Linking/unlinking can only occur when the deocders are stopped. */
   if ( hXvdChBase->eDecoderState == BXVD_P_DecoderState_eActive )
   {
      BXVD_DBG_ERR(hXvdChBase, ("BXVD_LinkChannels:: base decoder is active, it must be stopped."));
      rc = BXVD_ERR_DECODER_ACTIVE;
      goto Done;
   }

   if ( hXvdChEnhanced->eDecoderState == BXVD_P_DecoderState_eActive )
   {
      BXVD_DBG_ERR(hXvdChEnhanced, ("BXVD_LinkChannels:: enhanced decoder is active, it must be stopped."));
      rc = BXVD_ERR_DECODER_ACTIVE;
      goto Done;
   }

   /* The streams being decoded on separate decoders, this is not currently supported. */
   if ( hXvdChBase->pXvd != hXvdChEnhanced->pXvd )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_LinkChannels:: linking channels on separate decoders is not supported."));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }

   /* Link each channel to the other and set the channel type. */

   hXvdChBase->hXvdChLinked   = hXvdChEnhanced;
   hXvdChBase->eChannelType   = BXVD_P_ChannelType_eBase;

   hXvdChEnhanced->hXvdChLinked  = hXvdChBase;
   hXvdChEnhanced->eChannelType  = BXVD_P_ChannelType_eEnhanced;


   /* Create the XMO filter. */
   rc = BXDM_PictureProvider_XMO_Create( &(hXvdChBase->hXmo) );

   if ( BERR_SUCCESS != rc )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_LinkChannels:: BXDM_PictureProvider_XMO_Create failed"));
      rc = BERR_OUT_OF_SYSTEM_MEMORY;
      goto Done;
   }

   /* Install the XMO filter. */
   rc = BXVD_S_InstallXmo( hXvdChBase );

   if ( BERR_SUCCESS != rc )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_LinkChannels:: BXVD_S_InstallXmo failed"));
      rc = BERR_INVALID_PARAMETER;
   }

Done:

   BDBG_LEAVE( BXVD_LinkChannels );

   return  BERR_TRACE(rc);

}


/***************************************************************************
Summary:

Description:
***************************************************************************/
BERR_Code BXVD_UnlinkChannels(
   BXVD_ChannelHandle hXvdChBase,
   BXVD_ChannelHandle hXvdChEnhanced
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXVD_UnlinkChannels );

   BDBG_ASSERT(hXvdChBase);
   BDBG_ASSERT(hXvdChEnhanced);

   /* Check base handle type for correctness */
   if ( hXvdChBase->eHandleType != BXVD_P_HandleType_XvdChannel )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_UnlinkChannels:: Invalid base handle type"));
      rc = BXVD_ERR_INVALID_HANDLE;
      goto Done;
   }

   /* Check enhanced handle type for correctness */
   if ( hXvdChEnhanced->eHandleType != BXVD_P_HandleType_XvdChannel )
   {
      BXVD_DBG_ERR( hXvdChEnhanced, ("BXVD_UnlinkChannels:: Invalid enhanced handle type"));
      rc = BXVD_ERR_INVALID_HANDLE;
      goto Done;
   }

   /* Linking/unlinking can only occur when the deocders are stopped. */
   if ( hXvdChBase->eDecoderState == BXVD_P_DecoderState_eActive )
   {
      BXVD_DBG_ERR(hXvdChBase, ("BXVD_UnlinkChannels:: base decoder is active, it must be stopped."));
      rc = BXVD_ERR_DECODER_ACTIVE;
      goto Done;
   }

   if ( hXvdChEnhanced->eDecoderState == BXVD_P_DecoderState_eActive )
   {
      BXVD_DBG_ERR(hXvdChEnhanced, ("BXVD_UnlinkChannels:: enhanced decoder is active, it must be stopped."));
      rc = BXVD_ERR_DECODER_ACTIVE;
      goto Done;
   }

   /* The streams being decoded on separate decoders, this is not currently supported. */
   if ( hXvdChBase->pXvd != hXvdChEnhanced->pXvd )
   {
      BXVD_DBG_ERR( hXvdChBase, ("BXVD_UnlinkChannels:: linking channels on separate decoders is not supported."));
      rc = BERR_INVALID_PARAMETER;
      goto Done;
   }

   /* Uninstall the filter. */
   BXVD_S_UninstallXmo( hXvdChBase );

   /* Free the memory and reset the channel state. */

   rc = BXDM_PictureProvider_XMO_Destroy( hXvdChBase->hXmo );
   hXvdChBase->hXmo = NULL;

   hXvdChBase->hXvdChLinked   = NULL;
   hXvdChBase->eChannelType   = BXVD_P_ChannelType_eStandard;

   hXvdChEnhanced->hXvdChLinked  = NULL;
   hXvdChEnhanced->eChannelType  = BXVD_P_ChannelType_eStandard;

Done:

   BDBG_LEAVE( BXVD_UnlinkChannels );

   return  BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

#define OMX_PROTOTYPE 1

#if OMX_PROTOTYPE
/***************************************************************************
   see comment in bxvd.h
****************************************************************************/


/***************************************************************************
Summary:


Description:
***************************************************************************/

/*
 * SW7425-4630
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_InstallFilter(
   BXVD_ChannelHandle hXvdCh,
   BXDM_Decoder_Interface *pstDecoderInterface,
   void *pPrivateFilterContext,
   BXVD_FilterSettings *pstFilterSettings,
   BXVD_FilterInterface ** pstFilterInterface
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXVD_InstallFilter );

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pstFilterInterface);

   /* Check XVD handle type for correctness */
   if ( hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel )
   {
      BXVD_DBG_ERR( hXvdCh, ("BXVD_InstallFilter::hXvdCh handle type"));
      rc = BXVD_ERR_INVALID_HANDLE;
      goto Done;
   }

#if 0
   /* Check filter handle type? */
   if ( pstFilterInterface->eHandleType != BXVD_P_HandleType_XvdChannel )
   {
      BXVD_DBG_ERR( pstFilterInterface, ("BXVD_InstallFilter:: Invalid enhanced handle type"));
      rc = BXVD_ERR_INVALID_HANDLE;
      goto Done;
   }
#endif

   /* a filter can only be installed when the deocder is stopped. */
   if ( hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive )
   {
      BXVD_DBG_ERR(hXvdCh, ("BXVD_InstallFilter:: decoder is active, it must be stopped."));
      rc = BXVD_ERR_DECODER_ACTIVE;
      goto Done;
   }

   BXDM_PictureProvider_InstallFilter(
         hXvdCh->hPictureProvider,
         pstDecoderInterface,
         pPrivateFilterContext,
         pstFilterSettings,
         pstFilterInterface
         );

Done:

   BDBG_LEAVE( BXVD_InstallFilter );

   return  BERR_TRACE(rc);

}


BERR_Code BXVD_UninstallFilter(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FilterInterface * pstFilterInterface
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXVD_UninstallFilter );

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pstFilterInterface);

   /* Check XVD handle type for correctness */
   if ( hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel )
   {
      BXVD_DBG_ERR( hXvdCh, ("BXVD_UninstallFilter::hXvdCh handle type %08x", hXvdCh->eHandleType ));
      rc = BXVD_ERR_INVALID_HANDLE;
      goto Done;
   }

   /* a filter can only be uninstalled when the deocder is stopped. */
   if ( hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive )
   {
      BXVD_DBG_ERR(hXvdCh, ("BXVD_UninstallFilter:: decoder is active, it must be stopped."));
      rc = BXVD_ERR_DECODER_ACTIVE;
      goto Done;
   }

   BXDM_PictureProvider_UninstallFilter( hXvdCh->hPictureProvider, pstFilterInterface );

Done:

   BDBG_LEAVE( BXVD_UninstallFilter );

   return BERR_TRACE(rc);

}

#endif /* !B_REFSW_MINIMAL SWSTB-461 */
#endif /* OMX_PROTOTYPE*/

/***************************************************************************
   SW7425-1064: support for managing XMO (XDM Merge Objec)
   see comment in bxvd.h
****************************************************************************/
#if 0
BERR_Code BXVD_CreateXmo(
   BXVD_ChannelHandle hXvdCh
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXVD_CreateXmo );

   rc = BXDM_PictureProvider_XMO_Create( &(hXvdCh->hXmo) );

   BDBG_LEAVE( BXVD_CreateXmo );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_DestroyXmo(
   BXVD_ChannelHandle hXvdCh
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXVD_DestroyXmo );

   if ( NULL != hXvdCh->hXmo
        && BXVD_P_ChannelType_eBase == hXvdCh->eChannelType
      )
   {
      rc = BXDM_PictureProvider_XMO_Destroy( hXvdCh->hXmo );
      hXvdCh->hXmo = NULL;
   }

   BDBG_LEAVE( BXVD_DestroyXmo );

   return BERR_TRACE( rc );
}
#endif

/***************************************************************************
Summary:
   SW7425-2686:  added for multi-pass DQT.  A "generic" API for setting
   trick modes.

****************************************************************************/

void BXVD_GetDefaultTrickModeSettings(
   BXVD_TrickModeSettings *pstTrickModeSettings
   )
{
   BDBG_ENTER( BXVD_GetDefaultTrickModeSettings );
   BDBG_ASSERT(pstTrickModeSettings);

   BKNI_Memset( pstTrickModeSettings, 0, sizeof(BXVD_TrickModeSettings));

   BDBG_LEAVE( BXVD_GetDefaultTrickModeSettings );
   return;
}

BERR_Code BXVD_SetTrickModeSettings(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   const BXVD_TrickModeSettings *pstTrickModeSettings
   )
{
   BERR_Code rc=BERR_SUCCESS;

   BDBG_ENTER( BXVD_SetTrickModeSettings );

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pstTrickModeSettings);

   if ( pstTrickModeSettings->stGopTrickMode.eMode >= BXVD_DQTTrickMode_eMax )
   {
      BXVD_DBG_ERR( hXvdCh, ("%s:: eMode of %d is >= BXVD_DQTTrickMode_eMax",
                                 BSTD_FUNCTION,
                                 pstTrickModeSettings->stGopTrickMode.eMode ));

      rc = BERR_INVALID_PARAMETER;
   }
   else if ( pstTrickModeSettings->stGopTrickMode.eTargetPTSType >= BXVD_PTSType_eMaxPTSType )
   {
      BXVD_DBG_ERR( hXvdCh, ("%s:: eTargetPTSType of %d is >= BXVD_PTSType_eMaxPTSType",
                                 BSTD_FUNCTION,
                                 pstTrickModeSettings->stGopTrickMode.eTargetPTSType ));

      rc = BERR_INVALID_PARAMETER;
   }
   else
   {
      hXvdCh->stTrickModeSettings = *pstTrickModeSettings;
   }

   BDBG_LEAVE( BXVD_SetTrickModeSettings );
   return BERR_TRACE(rc);
}

BERR_Code BXVD_GetTrickModeSettings(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   BXVD_TrickModeSettings *pstTrickModeSettings
   )
{
   BDBG_ENTER( BXVD_GetTrickModeSettings );

   *pstTrickModeSettings = hXvdCh->stTrickModeSettings;

   BDBG_LEAVE( BXVD_GetTrickModeSettings );
   return BERR_TRACE(BERR_SUCCESS);
}

/*******************/
/* Deprecated APIs */
/*******************/

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
  BXVD_SetTSMPassWindow: Sets the TSM Pass Display Threshold in number of
  decodable units of display decided by the Correct Display Algorithm
****************************************************************************/
BERR_Code BXVD_SetTSMPassWindow (BXVD_ChannelHandle hXvdCh,
                                 uint32_t ui32PassFractNumerator,
                                 uint32_t ui32PassFractDenominator)
{
   BDBG_ENTER(BXVD_SetTSMPassWindow);

   /* Check the arguments */
   BSTD_UNUSED(hXvdCh);
   BSTD_UNUSED(ui32PassFractNumerator);
   BSTD_UNUSED(ui32PassFractDenominator);

   BXVD_DBG_ERR(hXvdCh, ("BXVD_SetTSMPassWindow() is DEPRECATED"));

   BDBG_LEAVE(BXVD_SetTSMPassWindow);
   return BERR_TRACE(BERR_SUCCESS) ;
}

/***************************************************************************
  BXVD_DisplayUntoPTS: Decodes and displays until the specified PTS is reached.
****************************************************************************/
BERR_Code BXVD_DisplayUntoPTS(BXVD_ChannelHandle hXvdCh,
                              bool       bEnable,
                              uint32_t   ui32PTS,
                              uint32_t   ui32ThreshLoopAroundCntr)
{
   BDBG_ENTER(BXVD_DisplayUntoPTS);

   /* Check the arguments */
   BSTD_UNUSED(hXvdCh);
   BSTD_UNUSED(bEnable);
   BSTD_UNUSED(ui32PTS);
   BSTD_UNUSED(ui32ThreshLoopAroundCntr);

   BXVD_DBG_ERR(hXvdCh, ("BXVD_DisplayUntoPTS() is DEPRECATED"));

   BDBG_LEAVE(BXVD_DisplayUntoPTS);
   return BERR_TRACE(BERR_SUCCESS) ;
}

/***************************************************************************
BXVD_PauseUntoPTS: Decodes and displays until the specified PTS is reached.
****************************************************************************/
BERR_Code BXVD_PauseUntoPTS(BXVD_ChannelHandle hXvdCh,
                            bool        bEnable,
                            uint32_t    ui32PTS,
                            uint32_t    ui32ThreshLoopAroundCntr)
{
   BDBG_ENTER(BXVD_PauseUntoPTS);

   BSTD_UNUSED(hXvdCh);
   BSTD_UNUSED(bEnable);
   BSTD_UNUSED(ui32PTS);
   BSTD_UNUSED(ui32ThreshLoopAroundCntr);

   BXVD_DBG_ERR(hXvdCh, ("BXVD_PauseUntoPTS() is DEPRECATED"));

   BDBG_LEAVE(BXVD_PauseUntoPTS);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_SetPulldownMode(
   BXVD_ChannelHandle hXvdCh,      /* [in] The XVD Channel handle */
   BXVD_PulldownMode ePulldownMode /* [in] Pulldown state */
   )
{
   BDBG_ENTER(BXVD_SetPulldownMode);

   BSTD_UNUSED(hXvdCh);
   BSTD_UNUSED(ePulldownMode);

   BXVD_DBG_ERR(hXvdCh, ("BXVD_SetPulldownMode() is DEPRECATED"));

   BDBG_LEAVE(BXVD_SetPulldownMode);

   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
****************************************************************************/
BERR_Code BXVD_GetPulldownMode(
   BXVD_ChannelHandle hXvdCh,          /* [in] The XVD Channel handle */
   BXVD_PulldownMode *pePulldownMode   /* [in] Pulldown state */
   )
{
   BDBG_ENTER(BXVD_GetPulldownMode);

   BSTD_UNUSED(hXvdCh);
   BSTD_UNUSED(pePulldownMode);

   BXVD_DBG_ERR(hXvdCh, ("BXVD_GetPulldownMode() is DEPRECATED"));

   return BERR_TRACE(BERR_SUCCESS);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */
