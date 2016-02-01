/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_DISPLAY_INIT_H__
#define NEXUS_DISPLAY_INIT_H__

#include "nexus_memory.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
The Display module's down modules
**/
typedef struct NEXUS_DisplayModuleDependencies
{
    NEXUS_ModuleHandle videoDecoder;
    NEXUS_ModuleHandle surface;
    NEXUS_ModuleHandle hdmiInput;
    NEXUS_ModuleHandle hdmiDvo;
    NEXUS_ModuleHandle hdmiOutput;
    NEXUS_ModuleHandle rfm;
    NEXUS_ModuleHandle pwm; /* needed for panel backlight control */
    NEXUS_ModuleHandle transport;
} NEXUS_DisplayModuleDependencies;

typedef enum NEXUS_VideoDacDetection
{
    NEXUS_VideoDacDetection_eAuto,
    NEXUS_VideoDacDetection_eOff,
    NEXUS_VideoDacDetection_eOn,
    NEXUS_VideoDacDetection_eMax
} NEXUS_VideoDacDetection;

typedef enum NEXUS_ComponentOutputSyncType
{
    /* consistent with CEA-770_3, clause 8.1.2 */
    NEXUS_ComponentOutputSyncType_eOnlyY,
    /* consistent with SMPTE 296M-1997, clause 12.7 */
    NEXUS_ComponentOutputSyncType_eAllChannels,
    NEXUS_ComponentOutputSyncType_eMax
} NEXUS_ComponentOutputSyncType;

/**
Summary:
Input from user for deinterlacer mode
**/
typedef enum NEXUS_DeinterlacerMode
{
    NEXUS_DeinterlacerMode_eNone, /* no deinterlacing, no memory */
    NEXUS_DeinterlacerMode_eBestQuality, /* requires more memory, has more latency */
    NEXUS_DeinterlacerMode_eLowestLatency, /* minimal latency and memory, but some deinterlacing */
    NEXUS_DeinterlacerMode_eMax
} NEXUS_DeinterlacerMode;

/**
Init time memory per display.
Can be set with NEXUS_MemoryConfigurationSettings at init time.
**/
typedef struct NEXUS_DisplayMemConfig
 {
    NEXUS_VideoFormat maxFormat; /* max display format for this display. if display is unused, leave it as eNone */
    struct {
        bool used;
        NEXUS_DeinterlacerMode deinterlacer;
        bool support3d; /* support duplicating to L/R eye in 3D mode */
        bool capture;
        bool convertAnyFrameRate; /* conversion between 50Hz and 60Hz inputs/outputs requires extra memory. */
        bool precisionLipSync; /* required if using SyncChannel */
        bool smoothScaling; /* deprecated. equivalent to 'capture' and defaults on where RTS allows. */
        unsigned userCaptureBufferCount; /* extra capture buffers for NEXUS_VideoWindowSettings.userCaptureBufferCount. */
        bool mtg; /* Allocate window memory for MPEG feeder timing generator sources.
                     An MTG MFD is driven at source rate, independent of display, which allows for better deinterlacing.
                     If a box mode supports any MTG MFD, every window has this 'mtg' boolean defaulted on which may allocate more capture buffer memory.
                     If you connect an MTG MFD to an MTG window, you get MTG. Otherwise, MTG is off. */
        NEXUS_SecureVideo secure; /* allocate picture buffer memory for unsecure, secure or both */
    } window[NEXUS_MAX_VIDEO_WINDOWS];
} NEXUS_DisplayMemConfig
;

/**
Summary:
Settings used in NEXUS_DisplayModule_Init
**/
typedef struct NEXUS_DisplayModuleSettings
{
    NEXUS_DisplayModuleDependencies modules;

    /* These global buffer counts are deprecated. Use NEXUS_DisplayModuleSettings.displayHeapSettings[] instead. */
    NEXUS_DisplayBufferTypeSettings fullHdBuffers;
    NEXUS_DisplayBufferTypeSettings hdBuffers;
    NEXUS_DisplayBufferTypeSettings sdBuffers;
    struct {
        /* deprecated method for getting runtime limits into system without per-window heaps */
        unsigned numDisplays, numWindowsPerDisplay;
    } legacy;

    /* displayHeapSettings[] will default to all zero.
    If the platform code detects this is all zero, it will set its defaults.
    If the display module code detects this is all zero, it will use the legacy settings. */
    NEXUS_DisplayHeapSettings displayHeapSettings[NEXUS_MAX_HEAPS];
    unsigned videoWindowHeapIndex[NEXUS_MAX_DISPLAYS][NEXUS_MAX_VIDEO_WINDOWS]; /* Set the heap index per video window. If >= NEXUS_MAX_HEAPS, then unused. */
    unsigned deinterlacerHeapIndex[NEXUS_MAX_DISPLAYS][NEXUS_MAX_VIDEO_WINDOWS]; /* same as above, but for deinterlacer. */
    struct {
        unsigned videoWindowHeapIndex[NEXUS_MAX_DISPLAYS][NEXUS_MAX_VIDEO_WINDOWS]; /* Set the heap index per video window. If >= NEXUS_MAX_HEAPS, then unused. */
        unsigned deinterlacerHeapIndex[NEXUS_MAX_DISPLAYS][NEXUS_MAX_VIDEO_WINDOWS]; /* same as above, but for deinterlacer. */
    } secure;
    unsigned primaryDisplayHeapIndex;    /* The heap given to BVDC_Open for general use.
                                            This is usually the heap index for videoWindowHeapIndex[0][0] i.e HD Main Video window */
    unsigned rdcHeapIndex;               /* The heap used by RDC for RUL's */
    bool dropFrame;                      /* Deprecated. See NEXUS_DisplaySettings.dropFrame. */

    uint32_t dacBandGapAdjust[NEXUS_MAX_VIDEO_DACS];  /*Adjustment to the video TDAC and QDAC bandgap setting.
                                            The default value is correct for most chipsets. However, there are
                                            some production runs that require an adjustment for correct amplitude,
                                            depends on the particular fab line that manufactured the chip. */
    NEXUS_VideoDacDetection dacDetection;

    struct {
        bool allowTeletext;              /* allocate extra memory for teletext VBI */
        bool allowVps;                   /* allocate extra memory for vps VBI */
        bool allowGemStar;               /* allocate extra memory for gemstar VBI */
        bool allowCgmsB;                 /* allocate extra memory for CGMS-B VBI */
        bool allowAmol;                  /* allocate extra memory for AMOL VBI */
        bool tteShiftDirMsb2Lsb;         /* If true, teletext encoder shift direction will be set to MSBToLSB. Otherwise, it
                                            will be set to LSBToMSB. The default value is FALSE. */
        unsigned ccir656InputBufferSize; /* Size of 656 VBI buffer in bytes. The default value is 0. */
    } vbi;

    bool vecSwap;                       /* Default = true.  If you require more than three SD outputs with concurrent HD output, you may need to set this to false.
                                           This is used only if NEXUS_DisplaySettings.vecIndex is -1. */
    bool handleDynamicRts;              /* unused */
    unsigned configurationId;           /* same as NEXUS_Core_Settings.boxMode */
    bool disableFrc;                    /* Default = false. Set to true to disable loading and running of FRC. */
    int encoderTgMapping[NEXUS_MAX_VIDEO_ENCODERS]; /* deprecated/unused */
#if NEXUS_NUM_VIDEO_DECODERS
    struct {
        struct {
            unsigned memcIndex;
            unsigned secondaryMemcIndex; /* for split Y/C buffer */
        } mfd, vfd;
    } videoImageInput[NEXUS_NUM_VIDEO_DECODERS];
#endif

    NEXUS_DisplayMemConfig  memConfig[NEXUS_MAX_DISPLAYS];
    struct {
        bool mosaic;
        bool hdDvi;
        bool ccir656;
    } memconfig;

    NEXUS_ComponentOutputSyncType componentOutputSyncType;

} NEXUS_DisplayModuleSettings;

struct NEXUS_Core_PreInitState;

/**
Summary:
Get defaults before calling NEXUS_DisplayModule_Init
**/
void NEXUS_DisplayModule_GetDefaultSettings(
    const struct NEXUS_Core_PreInitState *preInitState,
    NEXUS_DisplayModuleSettings *pSettings /* [out] */
    );

/**
Summary:
Initialize the Display module

Description:
This function is called by NEXUS_Platform_Init, not by applications.
If you want to modify these settings from your application, you can do this
through NEXUS_PlatformSettings as follows:

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.displayModuleSettings.xxx = xxx;
    NEXUS_Platform_Init(&platformSettings);

**/
NEXUS_ModuleHandle NEXUS_DisplayModule_Init(
    const NEXUS_DisplayModuleSettings *pSettings
    );

/**
Summary:
Uninitialize the Display module
**/
void NEXUS_DisplayModule_Uninit(void);

/**
Summary:
Get the settings that were used in NEXUS_DisplayModule_Init.

Description:
These cannot be changed without calling NEXUS_DisplayModule_Uninit then NEXUS_DisplayModule_Init.
This is for informational purposes.
**/
void NEXUS_DisplayModule_GetSettings(
    NEXUS_DisplayModuleSettings *pSettings /* [out] */
    );

#define NEXUS_DISPLAY_WINDOW_MAIN (0x1)
#define NEXUS_DISPLAY_WINDOW_PIP  (0x2)
#define NEXUS_DISPLAY_WINDOW_MONITOR (0x4)

#define NEXUS_DISPLAY_INPUT_DIGITAL (0x1000)
#define NEXUS_DISPLAY_INPUT_ANALOG  (0x2000)

/**
Summary:
Get the settings that were used in NEXUS_DisplayModule_Init.

Description:
These cannot be changed without calling NEXUS_DisplayModule_Uninit then NEXUS_DisplayModule_Init.
This is for informational purposes.
**/
NEXUS_Error NEXUS_DisplayModule_GetMemorySettings(
    unsigned configurationId,                           /* Configuration ID */
    uint32_t mask,                                      /* Must contain at least one window and at least one input */
    NEXUS_DisplayBufferTypeSettings *pFullHdBuffers,    /* [out] Full HD buffer requirements */
    NEXUS_DisplayBufferTypeSettings *pHdBuffers,        /* [out] HD buffer requirements */
    NEXUS_DisplayBufferTypeSettings *pSdBuffers,        /* [out] SD buffer requirements */
    unsigned *pHeapSize                                 /* [out] Heap size in bytes */
    );

/**
Summary:
Set the VideoDecoder module dependency

Description:
This allows for faster system boot time. The Display module and VideoDecoder module can init separately, then the link can be made after both are initialized.
**/
void NEXUS_DisplayModule_SetVideoDecoderModule(
    NEXUS_ModuleHandle videoDecoder /* Set to NULL or to the VideoDecoder module */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_INIT_H__ */
