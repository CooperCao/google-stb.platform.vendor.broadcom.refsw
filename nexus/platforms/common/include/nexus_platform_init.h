/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: Platform
*    Specific APIs to initialize the a board.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_PLATFORM_INIT_H__
#define NEXUS_PLATFORM_INIT_H__

#include "nexus_types.h"
#include "nexus_base_os.h"
#include "nexus_memory.h"
#include "nexus_platform_features.h"
#include "nexus_platform_common.h"
#include "nexus_platform_extint.h"
#include "nexus_platform_common_version.h"
#include "nexus_platform_version.h"
#include "nexus_platform_standby.h"
#include "nexus_platform_memconfig.h"
#if NEXUS_HAS_SURFACE
#include "nexus_surface_init.h"
#endif
#if NEXUS_HAS_TRANSPORT
#include "nexus_transport_init.h"
#endif
#if NEXUS_HAS_DISPLAY
#include "nexus_display_init.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_init.h"
#include "nexus_audio_dac.h"
#include "nexus_spdif_output.h"
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
#include "nexus_audio_dummy_output.h"
#endif
#if NEXUS_NUM_I2S_OUTPUTS
#include "nexus_i2s_output.h"
#endif
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder_init.h"
#endif
#if NEXUS_HAS_I2C
#include "nexus_i2c.h"
#endif
#if NEXUS_HAS_SPI
#include "nexus_spi.h"
#endif
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#else
typedef void *NEXUS_FrontendHandle; /* stub */
#endif
#if NEXUS_HAS_DISPLAY
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_svideo_output.h"
#include "nexus_ccir656_output.h"
#endif
#if NEXUS_HAS_RFM
#include "nexus_rfm.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_HDMI_DVO
#include "nexus_hdmi_dvo.h"
#endif
#if NEXUS_HAS_CEC
#include "nexus_cec.h"
#endif
#if NEXUS_HAS_SMARTCARD
#include "nexus_smartcard_init.h"
#endif
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_init.h"
#endif
#if NEXUS_HAS_SECURITY
#include "nexus_security_init.h"
#endif
#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d_init.h"
#endif
#if NEXUS_HAS_SAGE
#include "nexus_sage_init.h"
#endif

/* The following macros are for internal use, but are included in this public header file so that
we can provide a tagged NEXUS_Platform_Init to ensure binary compatibility because the nexus binary
and the application binary. */
#define NEXUS_PLATFORM_VERSION_UNITS (256)
#define NEXUS_PLATFORM_VERSION(maj,min) ((maj)*NEXUS_PLATFORM_VERSION_UNITS + (min))
#define NEXUS_P_GET_VERSION_P(p) NEXUS_PLATFORM_ ## p
#define NEXUS_P_GET_VERSION(p)  NEXUS_P_GET_VERSION_P(p)
#define NEXUS_P_GET_STRUCT_SIZES() (sizeof(NEXUS_PlatformSettings) + sizeof(NEXUS_PlatformConfiguration))

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************
Summary:
Run time configuration for the File Module
***************************************************************************/
#ifndef NEXUS_FILE_MAX_IOWORKERS
#define NEXUS_FILE_MAX_IOWORKERS 4
#endif
typedef struct NEXUS_FileModulePlatformSettings {
    unsigned workerThreads; /* number of the I/O worker threads  instaniated in the File module */
    NEXUS_ThreadSettings schedulerSettings[NEXUS_FILE_MAX_IOWORKERS];
} NEXUS_FileModulePlatformSettings;

typedef struct NEXUS_PlatformImgInterface {
    NEXUS_Error (*open)(const char *context, void **image, unsigned image_id);
    void (*close)(void *image);
    NEXUS_Error (*next)(void *image, unsigned chunk, const void **data, uint16_t length);
} NEXUS_PlatformImgInterface;

/***************************************************************************
Summary:
Heap Settings
*/
typedef struct NEXUS_PlatformHeapSettings {
    unsigned memcIndex; /* memory controller (MEMC) index. */
    unsigned subIndex;  /* addressing region within the MEMC. The only use is subIndex = 1 for MEMC0 memory above
                           the 256MB register hole or 760MB CMA barrier. Always 0 for MEMC1 and 2. */
    int size;           /* If size is >0, a fixed sized heap is created.
                           If size is -1, all remaining device memory in a matching bmem/cma region is allocated to that heap.
                           If size is 0 (default), no heap is created. */
    unsigned alignment; /* Minimum alignment of heap. In units of bytes. */
    bool guardBanding;  /* [deprecated] If true, add debug guard bands around each allocation.
                           Only applies to CPU-accessible heaps.
                           Set to false if your heap size calculations are precise with no extra space. */
    unsigned memoryType; /* see NEXUS_MEMORY_TYPE bitmasks and NEXUS_MemoryType macros in nexus_types.h */
    unsigned heapType;   /* see NEXUS_HEAP_TYPE bitmasks in nexus_types.h */
    bool optional;      /* if true, then allow nexus to init even if memory for this heap cannot be found */
    struct {
        bool first; /* if set to true, then this HEAP would be placed at beginning of the first region available on a given MEMC */
        bool sage; /* if set to true, then this HEAP should be placed to conform to SAGE requirements */
    } placement;
} NEXUS_PlatformHeapSettings;


/***************************************************************************
Summary:
Broadcom reference platform settings

Description:
The typical Broadcom settop/DTV reference platform has the following configurable features:
1) A set of I2C channels
2) An FPGA for routing transport to input bands
3) A series of frontend daughter card slots
4) A set of video and audio outputs with DAC settings which vary per platform

NEXUS_PlatformSettings allows NEXUS_Platform_Init to open these handles and configure
the devices according to typical defaults.
Your application can choose to disable this and open the handles itself.

If you allow Platform to open the handles, you cannot re-open the same handles in your application.
Instead, you must call NEXUS_Platform_GetConfiguration to retrieve the handles opened by NEXUS_Platform_Init.
***************************************************************************/
typedef struct NEXUS_PlatformSettings
{
    bool cachedMemory;      /* If true, the data cache will be enabled for driver memory clients that wish to use it.
                               If false, the cache will be disabled */
    bool openI2c;           /* If true, NEXUS_Platform_Init will open the I2C channels for this platform. */
    bool openFpga;          /* If true, NEXUS_Platform_Init will open the FPGA. This is needed for some frontends. */
    bool openFrontend;      /* If true, NEXUS_Platform_Init will scan the reference board for frontend devices. */
    bool openOutputs;       /* If true, NEXUS_Platform_Init will open video and audio outputs and configure the DAC's. */
    bool openInputs;        /* If true, NEXUS_Platform_Init will open analog video inputs configure the ADC's. */
    bool openCec;           /* If true, NEXUS_Platform_Init will open the CEC device. */
    bool checkPlatformType; /* If true, NEXUS_Platform_Init will verify nexus and app were compiled for the platform and version.
                               You can set false if your nexus platforms are binary compatibile. */
    NEXUS_ClientMode mode;  /* set handle verification mode for server app */
    struct {
        unsigned userId, groupId; /* drop to this user and group id after connecting to driver */
    } permissions;

    /* User-customized heap management.
    NEXUS_Platform_GetDefaultSettings populates the heap[] array for the reference board. It can be modified by the application.
    You can retrieve the NEXUS_HeapHandle's from NEXUS_PlatformConfiguration.heap[].
    Nexus creates heaps according to OS-reported bmem regions. 
    The heap[] array is sparse; unused heaps may appear in the middle of the array.
    */
    NEXUS_PlatformHeapSettings heap[NEXUS_MAX_HEAPS];

    NEXUS_PlatformImgInterface imgInterface;

    /* The reference platform exposes some module settings here for application configuration.
    Customer boards and applications may use this, or may implement their own custom platform code. */
#if NEXUS_HAS_TRANSPORT
    NEXUS_TransportModuleSettings transportModuleSettings;
#endif
#if NEXUS_HAS_DISPLAY
    NEXUS_DisplayModuleSettings displayModuleSettings;
#endif
#if NEXUS_HAS_AUDIO
    NEXUS_AudioModuleSettings audioModuleSettings;
#endif
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderModuleSettings videoDecoderModuleSettings;
#endif
#if NEXUS_HAS_SURFACE
    NEXUS_SurfaceModuleSettings surfacePlatformSettings;
#endif
#if NEXUS_HAS_SMARTCARD
    NEXUS_SmartcardModuleSettings smartCardSettings;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderModuleSettings videoEncoderSettings;
#endif
#if NEXUS_HAS_SECURITY
    NEXUS_SecurityModuleSettings securitySettings;
#endif
#if NEXUS_HAS_GRAPHICS2D
    NEXUS_Graphics2DModuleSettings graphics2DModuleSettings;
#endif
    NEXUS_PlatformStandbySettings standbySettings;

    NEXUS_FileModulePlatformSettings fileModuleSettings;
#if NEXUS_HAS_SAGE
    NEXUS_SageModuleSettings sageModuleSettings;
#endif
} NEXUS_PlatformSettings;


/***************************************************************************
Summary:
Get default settings to pass into NEXUS_Platform_Init.
***************************************************************************/
#define NEXUS_Platform_GetDefaultSettings(pSettings) NEXUS_Platform_GetDefaultSettings_tagged((pSettings), sizeof(NEXUS_PlatformSettings))

void NEXUS_Platform_GetDefaultSettings_tagged(
    NEXUS_PlatformSettings *pSettings, /* [out] */
    size_t size
    );


/***************************************************************************
Summary:
NEXUS_Platform_Init will initialize Nexus

Description:
NEXUS_Platform_MemConfigInit is a variation of NEXUS_Platform_Init which also includes memconfig settings.
NEXUS_Platform_Init uses default memconfig settings.

NEXUS_Platform_Init will initialize all board-specifics and then proceed to
initialize the nexus modules above it.  This is the main entry point
for all applications to start Nexus.

Nexus is a singleton. You cannot call NEXUS_Platform_Init a second time unless
you have first called NEXUS_Platform_Uninit.

Do not call NEXUS_Platform_Init_tagged directly. Instead, call NEXUS_Platform_Init or NEXUS_Platform_MemConfigInit.
This will perform basic version checking to make sure you have a properly configured system.

See Also:
NEXUS_Platform_Uninit
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_Init_tagged(
    const NEXUS_PlatformSettings *pSettings,     /* attr{null_allowed=y} Pass NULL for defaults */
    const NEXUS_MemoryConfigurationSettings *pMemConfig, /* attr{null_allowed=y} Pass NULL for defaults */
    unsigned platformCheck,                      /* set by NEXUS_Platform_Init macro. Only tested if NEXUS_PlatformSettings.checkPlatformType is true. */
    unsigned versionCheck,                       /* set by NEXUS_Platform_Init macro. Only tested if NEXUS_PlatformSettings.checkPlatformType is true.  */
    unsigned structSizeCheck                     /* set by NEXUS_Platform_Init macro */
    );

#if NEXUS_PLATFORM_NON_NUMERIC
#define NEXUS_Platform_Init(PSETTINGS) \
    NEXUS_Platform_Init_tagged((PSETTINGS), NULL, 0, NEXUS_P_GET_VERSION(NEXUS_PLATFORM), NEXUS_P_GET_STRUCT_SIZES())
#define NEXUS_Platform_MemConfigInit(PSETTINGS, PMEMCONFIG) \
    NEXUS_Platform_Init_tagged((PSETTINGS), PMEMCONFIG, 0, NEXUS_P_GET_VERSION(NEXUS_PLATFORM), NEXUS_P_GET_STRUCT_SIZES())
#else
#define NEXUS_Platform_Init(PSETTINGS) \
    NEXUS_Platform_Init_tagged((PSETTINGS), NULL, NEXUS_PLATFORM, NEXUS_P_GET_VERSION(NEXUS_PLATFORM), NEXUS_P_GET_STRUCT_SIZES())
#define NEXUS_Platform_MemConfigInit(PSETTINGS, PMEMCONFIG) \
    NEXUS_Platform_Init_tagged((PSETTINGS), PMEMCONFIG, NEXUS_PLATFORM, NEXUS_P_GET_VERSION(NEXUS_PLATFORM), NEXUS_P_GET_STRUCT_SIZES())
#endif

/***************************************************************************
Summary:
Uninitialize Nexus

Description:
The user is responsible for closing handles before calling NEXUS_Platform_Uninit.
Some handles can be automatically closed, but some cannot. It is best to explicitly
close all handles that you have opened.

After calling NEXUS_Platform_Uninit, you can all NEXUS_Platform_Init to bring up Nexus
again.
***************************************************************************/
/*
void NEXUS_Platform_Uninit(void) is defined in nexus_platform_client.h, which is included by nexus_platform.h
*/

/***************************************************************************
Summary:
***************************************************************************/
#define  NEXUS_GetDefaultMemoryConfigurationSettings(pSettings)  NEXUS_GetDefaultMemoryConfigurationSettings_tagged((pSettings), sizeof(NEXUS_MemoryConfigurationSettings))

void NEXUS_GetDefaultMemoryConfigurationSettings_tagged(
    NEXUS_MemoryConfigurationSettings *pSettings,
    size_t size
    );

/***************************************************************************
Summary:
NEXUS_GetPlatformCapabilities is callable before NEXUS_Platform_Init
***************************************************************************/
#define NEXUS_GetPlatformCapabilities(pCap) NEXUS_GetPlatformCapabilities_tagged((pCap), sizeof(NEXUS_PlatformCapabilities))

void NEXUS_GetPlatformCapabilities_tagged(
    NEXUS_PlatformCapabilities *pCap,
    size_t size
    );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef NEXUS_PLATFORM_INIT_H__ */

