/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/
#ifndef NEXUS_PLATFORM_H__
#define NEXUS_PLATFORM_H__

#include "nexus_types.h"
#include "nexus_base_os.h"
#include "nexus_memory.h"
#include "nexus_platform_features.h"
#include "nexus_platform_extint.h"
#include "nexus_platform_init.h"
#include "nexus_platform_standby.h"
#include "nexus_platform_server.h"
#ifdef NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#endif
#include "nexus_platform_compat.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************
Summary:
Get settings that were passed into NEXUS_Platform_Init along with modifications
made by Nexus. Be aware this is a different meaning of GetSettings than the
standard definition.

Description:
For some settings, 0 means that Nexus should select a default. NEXUS_Platform_GetSettings will
not return that 0 but will return the default that was chosen. In this sense, this
function works more like GetStatus.
***************************************************************************/
void NEXUS_Platform_GetSettings(
    NEXUS_PlatformSettings *pSettings /* [out] */
    );

/* generic max is useful for binary compatibility between varying platforms */
#define NEXUS_MAX_CONFIG_HANDLES 8
#ifndef NEXUS_MAX_FRONTENDS
#define NEXUS_MAX_FRONTENDS 32
#endif

/***************************************************************************
Summary:
Broadcom reference platform configuration

Description:
The broadcom reference platforms will open board-specific handles
and place the handles in this structure for use by the application.
***************************************************************************/
typedef struct NEXUS_PlatformConfiguration {
    NEXUS_I2cHandle i2c[NEXUS_MAX_CONFIG_HANDLES];
    NEXUS_FrontendHandle frontend[NEXUS_MAX_FRONTENDS];

    struct {
        NEXUS_ComponentOutputHandle component[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_CompositeOutputHandle composite[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_SvideoOutputHandle svideo[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_Ccir656OutputHandle ccir656[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_RfmHandle rfm[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_AudioDacHandle audioDacs[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_AudioDummyOutputHandle audioDummy[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_SpdifOutputHandle spdif[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_I2sOutputHandle i2s[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_I2sMultiOutputHandle i2sMulti[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_HdmiOutputHandle hdmi[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_HdmiDvoHandle hdmiDvo[NEXUS_MAX_CONFIG_HANDLES];
        NEXUS_CecHandle cec[NEXUS_MAX_CONFIG_HANDLES];
    } outputs;

    struct {
        NEXUS_VideoFormat maxOutputFormat; /* maximum supported video output format */
        NEXUS_VideoFormat maxOutputFormatSd; /* maximum supported standard definition video output format */
    } video;

    bool supportedDisplay[NEXUS_MAX_DISPLAYS]; /* report which displays can be opened */
    unsigned numWindowsPerDisplay; /* 1 = main only, 2 = PIP-capable */
    bool supportedDecoder[NEXUS_MAX_VIDEO_DECODERS]; /* This refers to regular decoders, not mosaic decoders. */

    NEXUS_HeapHandle heap[NEXUS_MAX_HEAPS];
    NEXUS_SpiHandle spi[NEXUS_MAX_CONFIG_HANDLES];
} NEXUS_PlatformConfiguration;

/***************************************************************************
Summary:
Get configured handles from the platform layer

Description:
The reference nexus platform code will initialize board-specific handles
based upon settings provided to NEXUS_Platform_Init().  Those handles
may be retrieved by an application via this funciton.

See Also:
NEXUS_Platform_Init
***************************************************************************/
void NEXUS_Platform_GetConfiguration(
    NEXUS_PlatformConfiguration *pConfiguration /* [out] */
    );


/***************************************************************************
Summary:
Get the input band for a streamer.

Description:
The assignment of streamer input to input band depends on board layout and possibly FPGA routing.
If FPGA configuration is required for your platform to achieve this routing, you must set openI2c
and openFpga to be true in NEXUS_PlatformSettings.
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_GetStreamerInputBand(
    unsigned index, /* index of the streamer input */
    NEXUS_InputBand *pInputBand /* [out] */
    );

/***************************************************************************
Summary:
Initialize the frontend if NEXUS_PlatformSettings.openFrontend = false.

Description:
If NEXUS_PlatformSettings.openFrontend = true (which is default), then NEXUS_Platform_Init
will call NEXUS_Platform_InitFrontend automatically. For faster system boot time, you can
set openFrontend = false and then call NEXUS_Platform_InitFrontend after the system has achieved
some baseline state (e.g. after the app has put graphics on the screen).
***************************************************************************/
NEXUS_Error NEXUS_Platform_InitFrontend(void);

/***************************************************************************
Summary:
Initialize a particular frontend.

Description:
This will create and initialize the specified frontend.
Use NEXUS_Platform_UninitFrontend to uninit the frontend along with any others
that have been created.
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Platform_OpenFrontend(
    unsigned id /* platform assigned ID for this frontend. See NEXUS_FrontendUserParameters.id.
                   See nexus_platform_frontend.c for ID assignment and/or see
                   nexus_platform_features.h for possible platform-specific macros.
                */
    );

/***************************************************************************
Summary:
Uninitialize all frontends.

Description:
This will uninit whatever was created by NEXUS_Platform_OpenFrontend,
NEXUS_Platform_InitFrontend, or NEXUS_Platform_Init.
NEXUS_Platform_UninitFrontend is called automatically by NEXUS_Platform_Uninit.
***************************************************************************/
void NEXUS_Platform_UninitFrontend(void);

/***************************************************************************
Summary:
Get the current reference software release version.
***************************************************************************/
void NEXUS_Platform_GetReleaseVersion( /* attr{local=true} */
    char *pVersionString, /* [out] returns a NULL-terminated string <= size */
    unsigned size /* maximum size data written to pVersionString, including null terminator */
    );

/***************************************************************************
Summary:
Each module's estimated maximum memory use

All units are bytes allocated from the given MEMC, unless noted differently.
***************************************************************************/
typedef struct NEXUS_PlatformEstimatedMemory
{
    struct {
        struct {
            unsigned general;
            unsigned secure;
        } videoDecoder;
        struct {
            unsigned general;
            unsigned secure;
            unsigned firmware;
            unsigned index;
            unsigned data;
        } videoEncoder;
        struct {
            unsigned general;
        } audio;
        struct {
            unsigned general;
        } display;
    } memc[NEXUS_MAX_MEMC];
} NEXUS_PlatformEstimatedMemory;

/***************************************************************************
Summary:
Platform status and general information
***************************************************************************/
typedef struct NEXUS_PlatformStatus
{
    unsigned chipId;       /* main chip, in hex, e.g. 0x7445. This ID is also called the product ID or bondout ID. */
    unsigned chipRevision; /* revision of the main chip, in hex, e.g. 0x0010 = A0, 0x0021 = B1 */
    unsigned familyId;     /* the family of this chip. this usually corresponds to the BCHP_CHIP used to compile the SW.
                              SW compiled for the familyId can often run on all chipId's within that family. */
    struct {
        unsigned major, minor;
    } boardId;
    unsigned boxMode;
    NEXUS_DisplayModuleStatus displayModuleStatus;
    NEXUS_PlatformEstimatedMemory estimatedMemory;
    struct {
        unsigned size;
    } memc[NEXUS_MAX_MEMC];
} NEXUS_PlatformStatus;

/***************************************************************************
Summary:
Get platform status
***************************************************************************/
NEXUS_Error NEXUS_Platform_GetStatus(
    NEXUS_PlatformStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Read a register

Description:
This function is provided for special cases and is not generally supported. Indiscriminate use will result in system failure.
It requires the application to know the register map, usually by including RDB header files.

Nexus will add the memory-mapped base pointer for register space. This means you can use RDB register addresses directly.
***************************************************************************/
NEXUS_Error NEXUS_Platform_ReadRegister(
    uint32_t address,
    uint32_t *pValue /* [out] */
    );

/***************************************************************************
Summary:
Write a register

Description:
This function is provided for special cases and is not generally supported. Indiscriminate use will result in system failure.
It requires the application to know the register map, usually by including RDB header files.
***************************************************************************/
void NEXUS_Platform_WriteRegister(
    uint32_t address,
    uint32_t value
    );

/***************************************************************************
Summary:
Returns heap that is accessible by GFD HW block for the specified display.
If you use this heap in your framebuffer's NEXUS_SurfaceCreateSettings.heap, the GFD will be able to read from it.

Description:
If you pass NEXUS_OFFSCREEN_SURFACE, the function will return the "graphics heap" for off-screen surface allocation.
This is provided as a convenience for multi-platform graphics applications.
It will have eApplication mapping, be usable by VC4 (3D graphics) and M2MC (2D graphics), but may not be GFD0 accessible.

For platforms with secondary graphics heaps, use NEXUS_SECONDARY_OFFSCREEN_SURFACE. Secondary graphics heaps
will have eApplication mapping and are generally only usable by M2MC, not VC4.
If there is no secondary graphics heap, platform code can return either NULL or the primary graphics heap.

For platforms with secure graphics, NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE will return the largest region for off-screen
secure graphics.
***************************************************************************/
NEXUS_HeapHandle NEXUS_Platform_GetFramebufferHeap(
    unsigned displayIndex
    );
#define NEXUS_OFFSCREEN_SURFACE (0x100)
#define NEXUS_SECONDARY_OFFSCREEN_SURFACE (0x101)
#define NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE (0x102)

/**
Summary:
Settings for NEXUS_Platform_CreateHeap
**/
typedef struct NEXUS_PlatformCreateHeapSettings
{
    unsigned offset; /* physical address */
    unsigned size; /* in bytes */
    NEXUS_MemoryType memoryType; /* requested memory mapping */
    unsigned alignment; /* required alignment (in bytes) of allocations in this region */
    bool locked; /* if true, nexus is not allowed to allocate from this heap */
    void *userAddress; /* if set, use this instead of doing an internal mmap for NEXUS_MemoryType_eApplication. attr{kind=null_ptr} */
} NEXUS_PlatformCreateHeapSettings;

/**
Summary:
Get default settings for NEXUS_Platform_CreateHeap
**/
void NEXUS_Platform_GetDefaultCreateHeapSettings(
    NEXUS_PlatformCreateHeapSettings *pSettings /* [out] */
    );

/**
Summary:
Create a heap at runtime which is outside of all existing nexus heaps.

Description:
A system may have memory which nexus is not given access to at NEXUS_Platform_Init.
NEXUS_Platform_CreateHeap can create a heap on that memory.
There must be no overlap in offset or virtual address space with existing heaps.
**/
NEXUS_HeapHandle NEXUS_Platform_CreateHeap(
    const NEXUS_PlatformCreateHeapSettings *pSettings
    );

/**
Summary:
Destroy a heap created by NEXUS_Platform_CreateHeap
**/
void NEXUS_Platform_DestroyHeap(
    NEXUS_HeapHandle heap
    );

/**
Summary:
This function attempts to add requested amount of memory to the specified heap

Description:
Only heaps of type NEXUS_MEMORY_TYPE_NOT_MAPPED or NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED support this operation
**/
NEXUS_Error NEXUS_Platform_GrowHeap(
    NEXUS_HeapHandle heap,
    size_t numBytes /* number of bytes to be added into the heap */
    );

/**
Summary:
This function attempts removes unused memory from the specified heap

Description:
This function only works for heaps that where previously grown by NEXUS_Platform_GrowHeap
**/
void NEXUS_Platform_ShrinkHeap(
    NEXUS_HeapHandle heap,
    size_t continuousBytes, /* don't shrunk heap if it has less then specified amount of continuous memory */
    size_t lowThreshold /* don't remove ranges that are smaller then specified size */
    );


/* backward compatibility; use NEXUS_GetVideoEncoderCapabilities instead. */
#ifndef NEXUS_ENCODER_DISPLAY_IDX
#define NEXUS_ENCODER_DISPLAY_IDX NEXUS_Platform_GetVideoEncoderDisplay(0)
#endif
#ifndef NEXUS_ENCODER0_DISPLAY_IDX
#define NEXUS_ENCODER0_DISPLAY_IDX NEXUS_Platform_GetVideoEncoderDisplay(0)
#endif
#ifndef NEXUS_ENCODER1_DISPLAY_IDX
#define NEXUS_ENCODER1_DISPLAY_IDX NEXUS_Platform_GetVideoEncoderDisplay(1)
#endif
#ifndef NEXUS_ENCODER2_DISPLAY_IDX
#define NEXUS_ENCODER2_DISPLAY_IDX NEXUS_Platform_GetVideoEncoderDisplay(2)
#endif
#ifndef NEXUS_ENCODER3_DISPLAY_IDX
#define NEXUS_ENCODER3_DISPLAY_IDX NEXUS_Platform_GetVideoEncoderDisplay(3)
#endif
unsigned NEXUS_Platform_GetVideoEncoderDisplay( /* attr{local=yes} */
    unsigned videoEncoderIndex
    );

/**
Summary:
Allow a handle to be used any other client in the system.

Description:
This is the equivalent of granting NEXUS_ClientMode_eVerified access to any client for this handle.
The handle type and value will be checked, but ownership will not.

The handle must be owned by the caller in order to change the sharing setting.

This does not make sharing of settings safe. If two or more clients change settings, erratic behavior may result.
**/
NEXUS_Error NEXUS_Platform_SetSharedHandle(
    NEXUS_PlatformAnyObject object, /* any Nexus handle */
    bool shared
    );

NEXUS_Error NEXUS_Platform_SetThermalScaling_driver(
    unsigned scale_factor,
    unsigned num_trip_points
    );

/* If a heap's security is toggled on/off at run time (external to Nexus), this notifies
Nexus so it can add or remove any additional monitoring. This does not actually make a heap secure
or unsecure.

NEXUS_HeapRuntimeSettings.secure = true should be set before actually making it secure.
NEXUS_HeapRuntimeSettings.secure = false should be set after actually making it unsecure.
*/
void NEXUS_Platform_GetHeapRuntimeSettings(
    NEXUS_HeapHandle heap,
    NEXUS_HeapRuntimeSettings *pSettings
    );

NEXUS_Error NEXUS_Platform_SetHeapRuntimeSettings(
    NEXUS_HeapHandle heap,
    const NEXUS_HeapRuntimeSettings *pSettings
    );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef NEXUS_PLATFORM_H__ */

