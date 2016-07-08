/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef NEXUS_PLATFORM_PRIV_H__
#define NEXUS_PLATFORM_PRIV_H__

#include "nexus_platform_module.h"
#include "nexus_platform.h"
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

#ifdef NEXUS_FPGA_SUPPORT
#include "bfpga.h"
#endif

#if NEXUS_HAS_SPI_FRONTPANEL
#include "nexus_gpio.h"
#include "nexus_spi.h"
#endif

#ifndef NEXUS_NUM_XVD_DEVICES
/* NEXUS_NUM_XVD_DEVICES is used throughout platform code; this provides backward compatibility */
#define NEXUS_NUM_XVD_DEVICES NEXUS_MAX_XVD_DEVICES
#endif

#include "blst_queue.h"
#include "blst_slist.h"

/* CPU type, TODO: read mips 4380/5000 from sysfs
internally see http://www.sj.broadcom.com/projects/MIPS3300/. */
#if BCHP_CHIP==7422 || BCHP_CHIP==7425 || BCHP_CHIP==7435 || BCHP_CHIP==7429 || BCHP_CHIP==74295
 #define BMIPS5000_40NM 1
#elif BCHP_CHIP==7420
 #define BMIPS5000_65NM 1
#elif BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7231 || BCHP_CHIP==7584 || BCHP_CHIP==7563 || BCHP_CHIP==7362 || BCHP_CHIP==7228 ||  BCHP_CHIP==75635 || BCHP_CHIP==73625 || BCHP_CHIP==75845 || BCHP_CHIP==73465 || BCHP_CHIP==75525
 #define BMIPS4380_40NM 1
#elif BCHP_CHIP==7552 || BCHP_CHIP==7358 || BCHP_CHIP==7360
 #define BMIPS3300 1
#endif

#define NEXUS_NUM_L1_REGISTERS BINT_INTC_SIZE

typedef enum NEXUS_PlatformStandbyLockMode {
    NEXUS_PlatformStandbyLockMode_eNone,         /* Module does not need to be locked in any standby state */
    NEXUS_PlatformStandbyLockMode_ePassiveOnly,  /* Module is locked only in passive state. Unlocked otherwise */
    NEXUS_PlatformStandbyLockMode_eAlways,       /* Module is always locked in any standby state */
    NEXUS_PlatformStandbyLockMode_eMax
} NEXUS_PlatformStandbyLockMode;

typedef struct NEXUS_Platform_P_ModuleInfo {
    BLST_Q_ENTRY(NEXUS_Platform_P_ModuleInfo) link;
    NEXUS_ModuleHandle module;
    NEXUS_PlatformStandbyLockMode lock_mode;
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

#ifdef NEXUS_FPGA_SUPPORT
    BFPGA_Handle fpgaHandle;
#endif

    BLST_Q_HEAD(handle_head, NEXUS_Platform_P_ModuleInfo) handles;
    bool baseOnlyInit;
} NEXUS_PlatformHandles;

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

/**
Summary:
Memory layout of single memory controller
*/
typedef struct NEXUS_PlatformMemcMemory {
    uint64_t length; /* no MEMC if 0 */
    struct {
        NEXUS_Addr base; /* physical address. 0 is valid. */
        uint64_t length; /* no region if 0 */
    } region[NEXUS_NUM_MEMC_REGIONS];
} NEXUS_PlatformMemcMemory;

typedef struct NEXUS_PlatformOsRegion {
    NEXUS_Addr base; /* physical address */
    uint64_t length; /* no region if 0 */

    /* correlation of bmem to MEMC */
    unsigned memcIndex;
    unsigned subIndex;
    bool cma;
} NEXUS_PlatformOsRegion;

/**
Summary:
Memory layout of the board for each MEMC and each physical addressing region

Description:
These structure members are arranged to be backward compatible with NEXUS_Platform_P_GetHostMemory, which is implemented per OS.
**/
typedef struct NEXUS_PlatformMemory
{
    /* DDR sizes detected on the board, per MEMC, per physical addressing region */
    NEXUS_PlatformMemcMemory memc[NEXUS_MAX_MEMC];

    /* OS's report of usable memory by physical address */
    NEXUS_PlatformOsRegion osRegion[NEXUS_MAX_HEAPS];

    unsigned max_dcache_line_size; /* reported by OS. BMEM alignment must be >= this to avoid cache coherency bugs. */
} NEXUS_PlatformMemory;

/**
Summary:
Read board strapping option to determine MEMC configuration.

Description:
Sets pMemory->memc[].length.
May modify pSettings (platform settings) based on those strapping options.
Called from NEXUS_Platform_P_InitCore.
**/
NEXUS_Error NEXUS_Platform_P_ReadMemcConfig(
    NEXUS_PlatformMemory *pMemory, /* [out] */
    NEXUS_PlatformSettings *pSettings /* [out] */
    );

NEXUS_Error NEXUS_Platform_P_ReadGenericMemcConfig(
    NEXUS_PlatformMemory *pMemory,
    NEXUS_PlatformSettings *pSettings
    );

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
NEXUS_Error NEXUS_Platform_P_CalcSubMemc(const NEXUS_Core_PreInitState *preInitState, NEXUS_PlatformMemory *pMemory);

/***************************************************************************
Summary:
Read reserved memory
***************************************************************************/
uint32_t NEXUS_Platform_P_ReadReserved(
    uint32_t physicalAddress
    );

/***************************************************************************
Summary:
Write reserved memory
***************************************************************************/
void NEXUS_Platform_P_WriteReserved(
    uint32_t physicalAddress,
    uint32_t value
    );

/***************************************************************************
Summary:
Read core register
***************************************************************************/
uint32_t NEXUS_Platform_P_ReadCoreReg(
    uint32_t offset
    );

/***************************************************************************
Summary:
Write core register
***************************************************************************/
void NEXUS_Platform_P_WriteCoreReg(
    uint32_t offset,
    uint32_t value
    );

/***************************************************************************
Summary:
Read CMT Control Register
***************************************************************************/
uint32_t NEXUS_Platform_P_ReadCmtControl(void);

/***************************************************************************
Summary:
Write CMT Control Register
***************************************************************************/
void NEXUS_Platform_P_WriteCmtControl(
    uint32_t value
    );

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
    NEXUS_Core_InterruptFunction pIsrFunc,
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
Powers off internal cable modem (BNM)
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_ShutdownCM(void);

/***************************************************************************
Summary:
Returns chip number of the frontend
 ***************************************************************************/
#if NEXUS_HAS_FRONTEND
unsigned NEXUS_Platform_P_FrontendType(NEXUS_FrontendHandle h);
#endif

/***************************************************************************
Summary:
Returns power-on status of internal cable modem
 ***************************************************************************/
bool NEXUS_Platform_P_CMPowerIsOn(void);

/***************************************************************************
Summary:
Switch BSC_M3 and external interrupts for frontends on it to host control if host==true,
to BNM control otherwise.
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_HostFrontendPinmux(bool host);

/***************************************************************************
Summary:
True if pin-muxes for host control of BSC_M3 and the external
frontend(s) on that bus on 97125 platform.  False if BNM control.
 ***************************************************************************/
bool NEXUS_Platform_P_IsHostFrontendPinmux(void);

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
void NEXUS_Platform_P_ConfigureGisbTimeout(void);

/**
Internal standby functions
**/
#if NEXUS_POWER_MANAGEMENT
typedef struct NEXUS_PlatformStandbyState {
    NEXUS_PlatformStandbySettings settings;
    bool locked;
} NEXUS_PlatformStandbyState;

extern NEXUS_PlatformStandbyState g_standbyState;
#endif

/***************************************************************************
Summary:
Based on the RTS settings for each platform, framebuffer for each display
could be placed on any heaps. This API shall return the heap handle
for each frame buffer.
 ***************************************************************************/
NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex);

NEXUS_Error NEXUS_Platform_P_InitServer(void);
void NEXUS_Platform_P_UninitServer(void);

void NEXUS_Platform_P_TerminateProcess(unsigned pid);

/***************************************************************************
Summary:
    Initialize Nexus
Description:
    This will initialize all board-specifics
See Also:
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_Init( const NEXUS_PlatformSettings *pSettings,
                                           unsigned platformCheck,
                                           unsigned versionCheck,
                                           unsigned structSizeCheck );

/***************************************************************************
Summary:
Private function for frontend standby/resume
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_StandbyFrontend(
    bool enabled,
    const NEXUS_StandbySettings *pSettings
    );

/***************************************************************************
Summary:
Private function to set standby settings, with option ot reset wakeup status.
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_SetStandbySettings(
    const NEXUS_PlatformStandbySettings *pSettings,
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
extern NEXUS_PlatformMemory g_platformMemory;

NEXUS_Error NEXUS_Platform_P_AcquireObject(const struct b_objdb_client *client, const NEXUS_InterfaceName *type, void *object);
void NEXUS_Platform_P_ReleaseObject(const NEXUS_InterfaceName *type, void *object);

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
void NEXUS_P_GetDefaultMemoryRtsSettings(NEXUS_MemoryRtsSettings *pRtsSettings);
NEXUS_Error NEXUS_P_ApplyMemoryConfiguration(const NEXUS_Core_PreInitState *preInitState, const NEXUS_MemoryConfigurationSettings *pMemConfig, const NEXUS_MemoryRtsSettings *pRtsSettings, NEXUS_PlatformSettings *pSettings);
void NEXUS_P_SupportVideoDecoderCodec( NEXUS_MemoryConfigurationSettings *pSettings, NEXUS_VideoCodec codec );

enum nexus_memconfig_picbuftype
{
    nexus_memconfig_picbuftype_unsecure,
    nexus_memconfig_picbuftype_secure,
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
    NEXUS_VideoEncoderModuleSettings videoEncoder;
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

void NEXUS_Platform_P_AddBoardStatus(NEXUS_PlatformStatus *pStatus);
NEXUS_Error NEXUS_Platform_P_SetStandbyExclusionRegion(unsigned heapIndex);
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
BTEE_InstanceHandle NEXUS_Platform_P_CreateTeeInstance(void);
void NEXUS_Platform_P_DestroyTeeInstance(BTEE_InstanceHandle teeHandle);
#endif


#endif /* #ifndef NEXUS_PLATFORM_PRIV_H__ */

