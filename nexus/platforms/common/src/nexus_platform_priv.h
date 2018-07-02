/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
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
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef NEXUS_PLATFORM_PRIV_H__
#define NEXUS_PLATFORM_PRIV_H__

#include "nexus_platform_module.h"
#include "nexus_platform_features.h"
#include "nexus_platform_local_priv.h"
#include "nexus_memory.h"
#include "priv/nexus_core.h"
#include "priv/nexus_core_preinit.h"
#include "nexus_core_init.h"
#include "priv/nexus_base_platform.h"
#include "b_objdb.h"
#if NEXUS_POWER_MANAGEMENT
#include "nexus_platform_standby.h"
#endif

#include "blst_queue.h"
#include "blst_slist.h"

/* CPU type, TODO: read mips 4380/5000 from sysfs
internally see http://www.sj.broadcom.com/projects/MIPS3300/. */
#if BCHP_CHIP==7422 || BCHP_CHIP==7425 || BCHP_CHIP==7435 || BCHP_CHIP==7429 || BCHP_CHIP==74295
 #define BMIPS5000_40NM 1
#elif BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7231 || BCHP_CHIP==7584 || BCHP_CHIP==7563 || BCHP_CHIP==7362 || BCHP_CHIP==75635 || BCHP_CHIP==73625 || BCHP_CHIP==75845 || BCHP_CHIP==73465 || BCHP_CHIP==75525
 #define BMIPS4380_40NM 1
#endif

#define NEXUS_NUM_L1_REGISTERS BINT_INTC_SIZE

typedef struct NEXUS_Platform_P_ModuleInfo {
    BLST_Q_ENTRY(NEXUS_Platform_P_ModuleInfo) link;
    NEXUS_ModuleHandle module;
    NEXUS_ModuleStandbyLevel standby_level;
    bool powerdown, locked;
    NEXUS_Error (*standby)(bool, const NEXUS_StandbySettings *);
    void (*uninit)(void);
} NEXUS_Platform_P_ModuleInfo;

typedef struct NEXUS_MemoryRtsSettings
{
    unsigned boxMode;
#if NEXUS_MAX_VIDEO_DECODERS
    struct {
        unsigned avdIndex;
        unsigned mfdIndex;
    } videoDecoder[NEXUS_MAX_VIDEO_DECODERS];
#endif
#if NEXUS_MAX_XVD_DEVICES
    struct {
        unsigned memcIndex;
        unsigned secondaryMemcIndex; /* for split buffer systems */
        bool splitBufferHevc;
    } avd[NEXUS_MAX_XVD_DEVICES];
#endif
} NEXUS_MemoryRtsSettings;

/***************************************************************************
 * Container for platform handles
 ***************************************************************************/
typedef struct NEXUS_PlatformHandles
{
    NEXUS_PlatformConfiguration config;
    NEXUS_MemoryRtsSettings rtsSettings;
    NEXUS_PlatformEstimatedMemory estimatedMemory;

    NEXUS_ModuleHandle core;
    NEXUS_ModuleHandle videoDecoder;
    NEXUS_ModuleHandle audio;
    NEXUS_ModuleHandle transport;
    NEXUS_ModuleHandle display;
    NEXUS_ModuleHandle file;
    NEXUS_ModuleHandle playback;
    NEXUS_ModuleHandle surface;
    NEXUS_ModuleHandle hdmiOutput;
    NEXUS_ModuleHandle hdmiDvo;
    NEXUS_ModuleHandle hdmiInput;
    NEXUS_ModuleHandle cec;
    NEXUS_ModuleHandle rfm;
    NEXUS_ModuleHandle frontend;
    NEXUS_ModuleHandle security;
    NEXUS_ModuleHandle sage;
    NEXUS_ModuleHandle dma;
    NEXUS_ModuleHandle i2c;
    NEXUS_ModuleHandle gpio;
    NEXUS_ModuleHandle pwm;
    NEXUS_ModuleHandle videoEncoder;
#if NEXUS_HAS_SCM
    NEXUS_ModuleHandle scm;
#endif
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_ModuleHandle pictureDecoder;
#endif
    NEXUS_ModuleHandle graphics2D;
    NEXUS_ModuleHandle graphics3D;
    NEXUS_ModuleHandle smartcard;
    BLST_Q_HEAD(handle_head, NEXUS_Platform_P_ModuleInfo) handles;
    bool baseOnlyInit;
} NEXUS_PlatformHandles;

typedef struct NEXUS_P_PlatformInternalSettings {
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderModuleInternalSettings videoEncoderSettings;
#endif
    unsigned unused;
} NEXUS_P_PlatformInternalSettings;



/***************************************************************************
Summary:
Initialize any OS-specifics including memory mapping and interrupt handling.
***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitOS(void);
NEXUS_Error NEXUS_Platform_P_UninitOS(void);
void NEXUS_Platform_P_MonitorOS(void);

/***************************************************************************
Summary:
Initialize OS-specific memory mapping.
This should not be called if you are also calling NEXUS_Platform_P_InitOS.
***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitOSMem(void);
void NEXUS_Platform_P_UninitOSMem(void);

/**
Summary:
Initialize core module

Description:
This calls into nexus_platform_core.c, which initializes the nexus_core module.
From here, calls into platform-specific code is made. See NEXUS_Platform_P_InitBoard.
**/
NEXUS_Error NEXUS_Platform_P_InitCore(
    const NEXUS_Core_PreInitState *state,
    NEXUS_PlatformSettings *pSettings
    );

/**
Summary:
Uninitialize the core module
**/
void NEXUS_Platform_P_UninitCore(void);

/**
Summary:
Init board-specific state. See nexus_platform_$(NEXUS_PLATFORM).c.
**/
NEXUS_Error NEXUS_Platform_P_InitBoard(void);

/**
Summary:
Uninit board-specific state. See nexus_platform_$(NEXUS_PLATFORM).c.
**/
void NEXUS_Platform_P_UninitBoard(void);


void NEXUS_Platform_P_GetPlatformHeapSettings(
    NEXUS_PlatformSettings *pSettings, unsigned boxMode
    );

void NEXUS_Platform_P_GetDefaultHeapSettings(
    NEXUS_PlatformSettings *pSettings, unsigned boxMode
    );

/***************************************************************************
Summary:
    Map physical register address space into virtual space
 ***************************************************************************/
void *NEXUS_Platform_P_MapRegisterMemory(
    unsigned long offset,
    unsigned long length
    );

/***************************************************************************
Summary:
    Unmap a virtual address mapped by NEXUS_Platform_P_MapRegisterMemory
 ***************************************************************************/
void NEXUS_Platform_P_UnmapRegisterMemory(
    void *pMem,
    unsigned long length
    );

/**
Summary:
Get the memory regions that the OS says can be used by nexus/magnum
**/
NEXUS_Error NEXUS_Platform_P_GetHostMemory(NEXUS_PlatformMemory *pMemory);
NEXUS_Error NEXUS_Platform_P_CalcSubMemc(const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformMemoryLayout *pMemory);

/***************************************************************************
Summary:
    Reset any pending L1 interrupts
 ***************************************************************************/
void NEXUS_Platform_P_ResetInterrupts(void);

/***************************************************************************
Summary:
    Connect an L1 interrupt to the OS
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_ConnectInterrupt(
    unsigned irqNum,
    NEXUS_Core_InterruptFunction pIsrFunc_isr,
    void *pFuncParam,
    int iFuncParam
    );

/***************************************************************************
Summary:
    Disconnect an L1 interrupt from the OS
 ***************************************************************************/
void NEXUS_Platform_P_DisconnectInterrupt(
    unsigned irqNum
    );

/***************************************************************************
Summary:
    enable an L1 interrupt
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_EnableInterrupt(
    unsigned irqNum
    );

/***************************************************************************
Summary:
    Enable an L1 interrupt
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_EnableInterrupt_isr(
    unsigned irqNum
    );

/***************************************************************************
Summary:
    Disable an L1 interrupt
 ***************************************************************************/
void NEXUS_Platform_P_DisableInterrupt(
    unsigned irqNum
    );

/***************************************************************************
Summary:
    Disable an L1 interrupt in isr context
 ***************************************************************************/
void NEXUS_Platform_P_DisableInterrupt_isr(
    unsigned irqNum
    );

/***************************************************************************
Summary:
Configure pin-muxes in a platform-specific way
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void);

/***************************************************************************
Summary:
Configure VCXO in a platform-specific way
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitVcxo(void);

/***************************************************************************
Summary:
    Enable L1 interrupts for a platform
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitInterrupts(void);
void NEXUS_Platform_P_UninitInterrupts(void);

/***************************************************************************
Summary:
    Post-initialization board configuration
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_Config(
    const NEXUS_PlatformSettings *pSettings
    );

/***************************************************************************
Summary:
    Shutdown for NEXUS_Platform_P_Config
 ***************************************************************************/
void NEXUS_Platform_P_Shutdown(void);

/***************************************************************************
Summary:
    This function initializes magnum sub-system that allows to call magnum functtions.
Note:
   This function could be called multiped times, but should initialize magnum only once
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_Magnum_Init(void);

/***************************************************************************
Summary:
    This function un-initializes magnum sub-system.
 ***************************************************************************/
void NEXUS_Platform_P_Magnum_Uninit(void);

/***************************************************************************
Summary:
Common function to map user-accessible Platform settings to internal Core settings.
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_SetCoreModuleSettings(
    const NEXUS_PlatformSettings *pSettings, /* settings based in by the user */
    const NEXUS_PlatformMemory *pMemory,
    NEXUS_Core_Settings *pCoreSettings /* [out] this structure should already be initialized. this function will populate the heapRegion[] array. */
    );

/***************************************************************************
Summary:
Configure GISB timeout checking
 ***************************************************************************/
bool NEXUS_Platform_P_IsGisbTimeoutAvailable(void);
void NEXUS_Platform_P_ConfigureGisbTimeout(void);

/**
Internal standby functions
**/
#if NEXUS_POWER_MANAGEMENT
typedef struct NEXUS_PlatformStandbyState {
    NEXUS_StandbySettings settings;
    bool locked;
} NEXUS_PlatformStandbyState;

extern NEXUS_PlatformStandbyState g_standbyState;
#endif

#if !NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX
/***************************************************************************
Summary:
Based on the RTS settings for each platform, framebuffer for each display
could be placed on any heaps. This API shall return the heap handle
for each frame buffer.
 ***************************************************************************/
NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex);
#endif

NEXUS_Error NEXUS_Platform_P_InitServer(void);
void NEXUS_Platform_P_UninitServer(void);

/***************************************************************************
Summary:
Private function to set standby settings, with option ot reset wakeup status.
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_SetStandbySettings(
    const NEXUS_StandbySettings *pSettings,
    bool resetWakeupStatus
    );

/***************************************************************************
Summary:
Initialize Thermal Monitor
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitializeThermalMonitor(void);

/***************************************************************************
Summary:
Uninitialize Thermal Monitor
 ***************************************************************************/
void NEXUS_Platform_P_UninitializeThermalMonitor(void);

/***************************************************************************
Summary:
Global variables for platform state
 ***************************************************************************/
extern NEXUS_PlatformHandles g_NEXUS_platformHandles;
extern NEXUS_PlatformSettings g_NEXUS_platformSettings;
extern NEXUS_P_PlatformInternalSettings g_NEXUS_platformInternalSettings;
extern NEXUS_PlatformMemory g_platformMemory;

struct b_objdb_client *nexus_p_platform_objdb_client(NEXUS_ClientHandle client);

/**
process any deferred cleanup in modules
**/
void NEXUS_Platform_P_SweepModules(void);

/***************************************************************************
Summary:
Private functions for Contiguous Memory Allocation (CMA)
***************************************************************************/
#if NEXUS_CPU_ARM && !defined(NEXUS_USE_CMA)
/* by default, ARM uses CMA. But you can build for "bmem" static reservation as well with NEXUS_CFLAGS="-DNEXUS_USE_CMA=0". */
#define NEXUS_USE_CMA 1
#endif
#if NEXUS_USE_CMA
NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings(const NEXUS_PlatformSettings *pSettings, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings);
NEXUS_Addr NEXUS_Platform_P_AllocCma(const NEXUS_PlatformMemory *pMemory, unsigned memcIndex, unsigned subIndex, unsigned size, unsigned alignment);
void NEXUS_Platform_P_FreeCma(const NEXUS_PlatformMemory *pMemory, unsigned memcIndex, unsigned subIndex, NEXUS_Addr addr, unsigned size);
#endif
NEXUS_Error NEXUS_Platform_P_AddDynamicRegion(NEXUS_Addr addr, unsigned size);
NEXUS_Error NEXUS_Platform_P_RemoveDynamicRegion(NEXUS_Addr addr, unsigned size);

/* requiring NEXUS_Core_PreInitState be passed in a few key places to encode proper state initialization (that is, we must already be in preinit state) */
void NEXUS_Platform_Priv_GetDefaultSettings(const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformSettings *pSettings );
void NEXUS_P_GetDefaultMemoryConfigurationSettings(const NEXUS_Core_PreInitState *preInitState, NEXUS_MemoryConfigurationSettings *pSettings);
void NEXUS_P_GetDefaultMemoryRtsSettings(const NEXUS_Core_PreInitState *preInitState, NEXUS_MemoryRtsSettings *pRtsSettings);
NEXUS_Error NEXUS_P_ApplyMemoryConfiguration(const NEXUS_Core_PreInitState *preInitState, const NEXUS_MemoryConfigurationSettings *pMemConfig, const NEXUS_MemoryRtsSettings *pRtsSettings, NEXUS_PlatformSettings *pSettings, NEXUS_P_PlatformInternalSettings *pInternalSettings);
void NEXUS_P_SupportVideoDecoderCodec( NEXUS_MemoryConfigurationSettings *pSettings, NEXUS_VideoCodec codec );
bool nexus_p_has_secure_decoder_on_memc(const NEXUS_Core_PreInitState *preInitState, const NEXUS_MemoryRtsSettings *pRtsSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig, unsigned memcIndex);

enum nexus_memconfig_picbuftype
{
    nexus_memconfig_picbuftype_glr,
    nexus_memconfig_picbuftype_urr,
    nexus_memconfig_picbuftype_urrt,
    nexus_memconfig_picbuftype_max
};

typedef struct NEXUS_MemoryConfiguration
{
    struct {
        unsigned size; /* in bytes */
        unsigned heapIndex;
    } pictureBuffer[NEXUS_MAX_MEMC][nexus_memconfig_picbuftype_max];
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderModuleSettings videoDecoder;
#endif
#if NEXUS_HAS_DISPLAY
    NEXUS_DisplayModuleSettings display;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderModuleInternalSettings videoEncoder;
#endif
} NEXUS_MemoryConfiguration;

/* optional functions into platform-specific code to modify settings. */
struct NEXUS_PlatformSpecificOps
{
    void (*modifyDefaultMemoryConfigurationSettings)(NEXUS_MemoryConfigurationSettings *pSettings);
    void (*modifyDefaultMemoryRtsSettings)(NEXUS_MemoryRtsSettings *pRtsSettings);
};
void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps);

const NEXUS_Core_PreInitState *NEXUS_Platform_P_PreInit(void);
void NEXUS_Platform_P_PreUninit(void);
extern const NEXUS_Core_PreInitState *g_pPreInitState;

#include "blst_queue.h"

/* read box mode from device tree or env variable */
unsigned NEXUS_Platform_P_ReadBoxMode(void);
/* read board id from device tree */
unsigned NEXUS_Platform_P_ReadBoardId(void);
/* read pmap id from device tree or env variable */
unsigned NEXUS_Platform_P_ReadPMapId(void);
/* read pmap settings from device tree */
BCHP_PmapSettings * NEXUS_Platform_P_ReadPMapSettings(void);
void NEXUS_Platform_P_FreePMapSettings(BCHP_PmapSettings *pMapSettings);

NEXUS_Error NEXUS_Platform_P_SetStandbyExclusionRegion(unsigned heapIndex);
bool NEXUS_Platform_P_ModuleInStandby(NEXUS_ModuleHandle module);
void NEXUS_Platform_P_Os_SystemUpdate32_isrsafe(const NEXUS_Core_PreInitState *preInitState, uint32_t reg, uint32_t mask, uint32_t value, bool systemRegister);
NEXUS_Error NEXUS_Platform_P_UpdateIntSettings(BINT_Settings *pIntSettings);

typedef struct NEXUS_Platform_Os_Config {
    bool virtualIrqSupported;
    bool gpioSupported;
} NEXUS_Platform_Os_Config;

#if B_REFSW_SYSTEM_MODE_CLIENT
void NEXUS_Platform_P_ClientMapMemory(void *pMem, size_t length, uint64_t offset, bool cached);
void NEXUS_Platform_P_ClientUnmapMemory(void *pMem, size_t length);
#endif
NEXUS_Error b_get_client_default_heaps(NEXUS_ClientConfiguration *config, struct b_objdb_client_default_heaps *default_heaps);
void nexus_platform_p_update_all_mmap_access(void);

#if NEXUS_USE_CMA
#if B_REFSW_SYSTEM_MODE_CLIENT
#define NEXUS_Platform_P_InitCma() (NEXUS_SUCCESS)
#define NEXUS_Platform_P_UnInitCma()
#else
NEXUS_Error NEXUS_Platform_P_InitCma(void);
void NEXUS_Platform_P_UnInitCma(void);
#endif
#endif

#if NEXUS_TEE_SUPPORT
#include "btee_instance.h"
BTEE_InstanceHandle NEXUS_Platform_P_CreateTeeInstance(void);
void NEXUS_Platform_P_DestroyTeeInstance(BTEE_InstanceHandle teeHandle);
#endif

#if NEXUS_CPU_ARM
#define NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX 1
#endif

void NEXUS_Platform_GetDefaultClientConfiguration(NEXUS_ClientConfiguration *pConfig);
bool NEXUS_Platform_P_IsOs64(void);
bool NEXUS_Platform_P_LazyUnmap(void);

NEXUS_Error nexus_platform_p_add_proc(NEXUS_ModuleHandle module, const char *filename, const char *module_name, void (*dbgPrint)(void));
void nexus_platform_p_remove_proc(NEXUS_ModuleHandle module, const char *filename);

/***************************************************************************
Summary:
Check the environment to see if we should enable SAGE logging
 ***************************************************************************/
bool NEXUS_Platform_P_EnableSageLog(void);

/***************************************************************************
Summary:
Initialize the Universal UART Interface
 ***************************************************************************/
void NEXUS_Platform_P_InitUUI(void);

/***************************************************************************
Summary:
Platform-specific UART IDs
 ***************************************************************************/
struct NEXUS_Platform_P_UartId {const unsigned id;const char *name;};
extern const struct NEXUS_Platform_P_UartId NEXUS_Platform_P_UartIds[];

/***************************************************************************
Summary:
Set 656 daughter card output format
 ***************************************************************************/
#if NEXUS_NUM_656_OUTPUTS && defined(NEXUS_HAS_656_DAUGHTER_CARD)
void NEXUS_Platform_P_Ccir656Daughtercard_SetFormat(NEXUS_VideoFormat fmt);
#endif

#endif /* #ifndef NEXUS_PLATFORM_PRIV_H__ */
