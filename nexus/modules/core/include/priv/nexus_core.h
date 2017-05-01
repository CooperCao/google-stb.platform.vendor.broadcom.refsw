/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#ifndef NEXUS_CORE_H
#define NEXUS_CORE_H

#include "nexus_platform_features.h"
#include "nexus_core_convert_local.h"
#include "nexus_base_object.h"
#include "priv/nexus_core_features.h"
#include "nexus_core_init.h"
#include "nexus_core_utils.h"
#include "nexus_memory.h"

/* NOTE: try to keep this list of header files generic. we don't want lots of
system-wide compilation dependencies. */
#include "bint.h"
#include "bint_plat.h"
#include "bchp.h"
#include "breg_mem.h"
#include "btmr.h"
#include "bmma.h"
#include "bmma_system.h"
#if !BMEM_DEPRECATED
#if !defined(BMMA_USE_STUB)
#include "bmma_bmem.h"
#endif
#include "bmem.h"
#endif /* !BMEM_DEPRECATED */
#include "bavc.h"
#include "bavc_hdmi.h"
#include "bpxl.h"
#include "bmrc_monitor.h"
#include "bbox.h"
#if NEXUS_ARM_AUDIO_SUPPORT
#include "btee_instance.h"
#endif
#include "bdtu.h"

#if B_HAS_TRC
#include "btrc.h"
void NEXUS_Core_Btrc_Report(const BTRC_Module *module, const char *moduleName);
#define NEXUS_BTRC_REPORT(module)  NEXUS_Core_Btrc_Report(BTRC_MODULE_HANDLE(module),#module)
#else
#define BTRC_MODULE_HANDLE(x)
#define BTRC_MODULE(x,y) extern int btrc_unused
#define BTRC_MODULE_DECLARE(x) extern int btrc_unused
#define NEXUS_BTRC_REPORT(L1_ISR) (void)0
#define BTRC_Module_Report(x)
#define BTRC_Module_Reset(x)
#define BTRC_TRACE(x, y)
#endif

#ifdef __cplusplus
#error
#endif

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Heap);

/*=************************
This private Core interface provides Magnum basemodules to Nexus modules.
They are accesible using the global variable g_pCoreHandles.

Core also provides an api for L1 interrupt management.
**************************/

typedef struct NEXUS_Core_MagnumHandles
{
    BREG_Handle         reg;
    BCHP_Handle         chp;
    BTMR_Handle         tmr;
    BINT_Handle         bint;
    const BINT_P_IntMap *bint_map;
#if !BMEM_DEPRECATED
    BMEM_ModuleHandle   mem;
#endif
    BMMA_Handle         mma;
    struct {
#if BMEM_DEPRECATED
        struct {
            unsigned deprecated;
        } *mem;
#else
        BMEM_Heap_Handle mem;/* magnum BMEM heap. deprecated  */
#endif
        BMMA_Heap_Handle mma; /* magnum BMMA heap  */
        NEXUS_HeapHandle nexus; /* nexus heap */
    } heap[NEXUS_MAX_HEAPS];
    unsigned defaultHeapIndex;
    struct {
        BMRC_Monitor_Handle rmm;
        BMRC_Handle mrc;
        BMRC_MonitorInterface mem_monitor;
        BDTU_Handle dtu;
    } memc[NEXUS_MAX_HEAPS]; /* index of this array is memc index, which must be < NEXUS_MAX_HEAPS */
    BCHP_MemoryLayout   memoryLayout;
    BBOX_Handle         box;
    const BBOX_Config  *boxConfig;
#if NEXUS_ARM_AUDIO_SUPPORT
    BTEE_InstanceHandle tee;
#else
    void *tee;
#endif
} NEXUS_Core_MagnumHandles;

/**
Summary:
Global variable for obtaining Core handles

Description:
This is usable after init
**/
extern const NEXUS_Core_MagnumHandles *g_NEXUS_pCoreHandles;

#define g_pCoreHandles g_NEXUS_pCoreHandles


/***************************************************************************
Summary:
Standby mode used in NEXUS_StandbySettings
***************************************************************************/
typedef enum NEXUS_StandbyMode
{
    NEXUS_StandbyMode_eOn,          /* Normal mode of operation. Also known as S0 mode. */
    NEXUS_StandbyMode_eActive,      /* Frontend and transport modules are running. All other modules are put to sleep.
                       The same wakeup devices as ePassive are available.
                       The application cannot put the MIPS to sleep in this mode. Also known as S1 mode*/
    NEXUS_StandbyMode_ePassive,     /* Low power mode where the AV cores are clock gated. Code remains resident.
                       IrInput, UhfInput, HdmiOutput (CEC), Gpio and Keypad are available to be configured as wakeup devices.
                       Application must call OS to put the MIPS to sleep. Also known as S2 mode. */
    NEXUS_StandbyMode_eDeepSleep,   /* Lowest power setting. Entire chip is power gated except for AON block. Code remains resident.
                       IrInput, UhfInput, HdmiOutput (CEC), Gpio and Keypad are available to be configured as wakeup devices.
                       Application must call OS to put the MIPS to sleep. Also known as S3 mode. */
    NEXUS_StandbyMode_eMax
} NEXUS_StandbyMode;

/***************************************************************************
Summary:
Settings used for module standby api
***************************************************************************/
typedef struct NEXUS_StandbySettings
{
    NEXUS_StandbyMode mode;
    struct {
        bool ir;
        bool uhf;
        bool keypad;
        bool gpio;
        bool nmi;
        bool cec;
        bool transport;
        unsigned timeout; /* in seconds */
    } wakeupSettings;
    bool openFrontend; /* If true, NEXUS_Platform_SetStandbySettings will initialize the frontend. */
} NEXUS_StandbySettings;

/***************************************************************************
Summary:
Enable an L1 interrupt.

Description:
This function will enable a level 1 (L1) interrupt.

Returns:
BERR_SUCCESS - The interrupt has been enabled.
BERR_INVALID_PARAMETER - An invalid parameter was passed.
BERR_OS_ERROR - An OS error occurred.

****************************************************************************/
BERR_Code NEXUS_Core_EnableInterrupt(
    unsigned irqNum
    );

/***************************************************************************
Summary:
Enable an L1 interrupt.

Description:
This function will enable a level 1 (L1) interrupt.

Returns:
BERR_SUCCESS - The interrupt has been enabled.
BERR_INVALID_PARAMETER - An invalid parameter was passed.
BERR_OS_ERROR - An OS error occurred.

****************************************************************************/
BERR_Code NEXUS_Core_EnableInterrupt_isr(
    unsigned irqNum
    );

/***************************************************************************
Summary:
Disable an L1 interrupt.

Description:
This function will disable a level 1 (L1) interrupt.

****************************************************************************/
void NEXUS_Core_DisableInterrupt(
    unsigned irqNum
    );

/***************************************************************************
Summary:
Disable an L1 interrupt.

Description:
This function will disable a level 1 (L1) interrupt.

Returns:
BERR_SUCCESS - The interrupt has been disabled.
BERR_INVALID_PARAMETER - An invalid parameter was passed.
BERR_OS_ERROR - An OS error occurred.

****************************************************************************/
void NEXUS_Core_DisableInterrupt_isr(
    unsigned irqNum
    );

/***************************************************************************
Summary:
Connect an L1 interrupt.

Description:
This function will register a callback function for a level 1 (L1)
interrupt.

Returns:
BERR_SUCCESS - The interrupt has been disabled.
BERR_INVALID_PARAMETER - An invalid parameter was passed.
BERR_OS_ERROR - An OS error occurred.

****************************************************************************/
BERR_Code NEXUS_Core_ConnectInterrupt(
    unsigned irqNum,
    NEXUS_Core_InterruptFunction pIsrFunc,
    void *pFuncParam,
    int iFuncParam
    );

/***************************************************************************
Summary:
Disconnect an L1 interrupt.

Description:
This function will un-register a callback function for a level 1 (L1)
interrupt.

****************************************************************************/
void NEXUS_Core_DisconnectInterrupt(
    unsigned irqNum
    );

/**
Summary:
Common Nexus-to-Magnum conversion functions
**/
NEXUS_Error       NEXUS_P_TransportType_ToMagnum_isrsafe(NEXUS_TransportType transportType, BAVC_StreamType *streamType);
NEXUS_Error       NEXUS_P_VideoFormat_ToMagnum_isrsafe(NEXUS_VideoFormat format, BFMT_VideoFmt *mformat);
NEXUS_VideoFormat NEXUS_P_VideoFormat_FromMagnum_isrsafe(BFMT_VideoFmt format);
NEXUS_VideoFormat NEXUS_P_VideoFormat_FromInfo_isrsafe(unsigned height, unsigned frameRate, bool interlaced);
NEXUS_AspectRatio NEXUS_P_AspectRatio_FromMagnum_isrsafe(BFMT_AspectRatio eAspectRatio);
BFMT_AspectRatio  NEXUS_P_AspectRatio_ToMagnum_isrsafe(NEXUS_AspectRatio aspectRatio);
BFMT_Orientation  NEXUS_P_VideoOrientation_ToMagnum_isrsafe(NEXUS_VideoOrientation orientation);
BAVC_StripeWidth  NEXUS_P_StripeWidth_ToMagnum_isrsafe(unsigned stripeWidth);
unsigned          NEXUS_P_StripeWidth_FromMagnum_isrsafe(BAVC_StripeWidth stripeWidth);

BAVC_MatrixCoefficients NEXUS_P_MatrixCoefficients_ToMagnum_isrsafe(NEXUS_MatrixCoefficients n_matrix);
NEXUS_MatrixCoefficients NEXUS_P_MatrixCoefficients_FromMagnum_isrsafe(BAVC_MatrixCoefficients m_matrix);
BAVC_ColorPrimaries NEXUS_P_ColorPrimaries_ToMagnum_isrsafe(NEXUS_ColorPrimaries n_color_primary);
NEXUS_ColorPrimaries NEXUS_P_ColorPrimaries_FromMagnum_isrsafe(BAVC_ColorPrimaries m_color_primary);
BAVC_TransferCharacteristics NEXUS_P_TransferCharacteristics_ToMagnum_isrsafe(NEXUS_TransferCharacteristics n_transfer);
NEXUS_TransferCharacteristics NEXUS_P_TransferCharacteristics_FromMagnum_isrsafe(BAVC_TransferCharacteristics m_transfer);

NEXUS_ColorSpace  NEXUS_P_ColorSpace_FromMagnum_isrsafe(BAVC_Colorspace colorSpace);
BAVC_Colorspace NEXUS_P_ColorSpace_ToMagnum_isrsafe(NEXUS_ColorSpace colorSpace);
NEXUS_ColorRange  NEXUS_P_ColorRange_FromMagnum_isrsafe(BAVC_ColorRange colorRange);
BAVC_ColorRange NEXUS_P_ColorRange_ToMagnum_isrsafe(NEXUS_ColorRange colorRange);
BAVC_HDMI_BitsPerPixel NEXUS_P_HdmiColorDepth_ToMagnum_isrsafe(NEXUS_HdmiColorDepth colorDepth);
BAVC_HDMI_DRM_EOTF NEXUS_P_VideoEotf_ToMagnum_isrsafe(NEXUS_VideoEotf eotf);
NEXUS_VideoEotf NEXUS_P_VideoEotf_FromMagnum_isrsafe(BAVC_HDMI_DRM_EOTF eotf);

NEXUS_VideoFrameRate NEXUS_P_FrameRate_FromMagnum_isrsafe(BAVC_FrameRateCode magnumFramerate);
NEXUS_Error NEXUS_P_FrameRate_ToMagnum_isrsafe(NEXUS_VideoFrameRate frameRate, BAVC_FrameRateCode *pMagnumFramerate);
void NEXUS_P_FrameRate_FromRefreshRate_isrsafe( unsigned frameRateInteger, NEXUS_VideoFrameRate *pNexusFrameRate );
unsigned NEXUS_P_RefreshRate_FromFrameRate_isrsafe(NEXUS_VideoFrameRate frameRate);
NEXUS_PixelFormat NEXUS_P_PixelFormat_FromMagnum_isrsafe(BPXL_Format magnumPixelFormat);
NEXUS_Error       NEXUS_P_PixelFormat_ToMagnum_isrsafe(NEXUS_PixelFormat nexusPixelFormat, BPXL_Format *magnumPixelFormat);

typedef struct NEXUS_PixelFormatConvertInfo {
    NEXUS_PixelFormatInfo info;
    BPXL_Format magnum;
    NEXUS_PixelFormat nexus;
} NEXUS_PixelFormatConvertInfo;

const NEXUS_PixelFormatConvertInfo *NEXUS_P_PixelFormat_GetConvertInfo_isrsafe(NEXUS_PixelFormat pixelFormat);

/**
Summary:
Easy Nexus lookup functions which can't be public because of coding convention.
See NEXUS_VideoFormat_GetInfo for public version.
**/
bool NEXUS_P_VideoFormat_IsSd(NEXUS_VideoFormat format);
bool NEXUS_P_VideoFormat_IsNotAnalogOutput(NEXUS_VideoFormat format);
bool NEXUS_P_VideoFormat_IsInterlaced(NEXUS_VideoFormat format);

NEXUS_Error NEXUS_Memory_P_ConvertAlignment(
    unsigned alignment,
    unsigned *pAlignmentExponent /* [out] */
    );

#if BMEM_DEPRECATED
void *NEXUS_Heap_GetMemHandle(NEXUS_HeapHandle heap) __attribute__ ((deprecated));
#else
/**
Summary:
Retrieve the heap for a cached address.
This function always has a second purpose of converting to an uncached address
**/
BMEM_Heap_Handle NEXUS_Core_P_AddressToHeap(
    void *pAddress,          /* cached address */
    void **ppUncachedAddress /* [out] optionally, uncached address. passing NULL is allowed. */
    );

/***************************************************************************
Summary:
Get the magnum MEM heap from the Nexus heap
***************************************************************************/
BMEM_Heap_Handle NEXUS_Heap_GetMemHandle(
    NEXUS_HeapHandle heap
    );
#endif /* !BMEM_DEPRECATED */

/**
Summary:
Set information about custom formats so that NEXUS_VideoFormat_GetInfo can return it.

Description:
See NEXUS_Display_SetCustomFormatSettings for the custom video format API.
This function only helps nexus state, not magnum state.
For now, only one custom video format is stored.
**/
void NEXUS_P_VideoFormat_SetInfo(
    NEXUS_VideoFormat customVideoFormat,
    const NEXUS_VideoFormatInfo *pVideoFmtInfo    /* [out] */
    );

void NEXUS_P_EotfToTransferCharacteristics_isrsafe(
    const NEXUS_VideoEotf eotf,
    NEXUS_TransferCharacteristics *tc,
    NEXUS_TransferCharacteristics *preferredTc);
NEXUS_VideoEotf NEXUS_P_TransferCharacteristicsToEotf_isrsafe(
    NEXUS_TransferCharacteristics tc,
    NEXUS_TransferCharacteristics preferredTc);
void NEXUS_P_ContentLightLevel_ToMagnum_isrsafe(
    NEXUS_ContentLightLevel * pCll,
    uint32_t *ulMaxContentLight,
    uint32_t *ulAvgContentLight);
void NEXUS_P_ContentLightLevel_FromMagnum_isrsafe(
    NEXUS_ContentLightLevel * pCll,
    uint32_t ulMaxContentLight,
    uint32_t ulAvgContentLight);
void NEXUS_P_MasteringDisplayColorVolume_ToMagnum_isrsafe(
    const NEXUS_MasteringDisplayColorVolume * pMdcv,
    BAVC_Point * pstDisplayPrimaries,
    BAVC_Point * pstWhitePoint,
    uint32_t *ulMaxDispMasteringLuma,
    uint32_t *ulMinDispMasteringLuma);

void NEXUS_P_MasteringDisplayColorVolume_FromMagnum_isrsafe(
    NEXUS_MasteringDisplayColorVolume * pMdcv,
    const BAVC_Point * pstDisplayPrimaries,
    const BAVC_Point * pstWhitePoint,
    uint32_t ulMaxDispMasteringLuma,
    uint32_t ulMinDispMasteringLuma);

/***************************************************************************
Summary:
Get the magnum MMA heap from the Nexus heap
***************************************************************************/
BMMA_Heap_Handle NEXUS_Heap_GetMmaHandle(
    NEXUS_HeapHandle heap
    );

BMMA_Block_Handle NEXUS_MemoryBlock_GetBlock_priv(NEXUS_MemoryBlockHandle block);

/***************************************************************************
Summary:
This function would either create new instance of NEXUS_MemoryBlockHandle or
return existing instance NEXUS_MemoryBlockHandle and bump reference count

Call NEXUS_MemoryBlock_Free_driver to decrement reference count yet not destroy local state
***************************************************************************/
NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_FromMma_priv(BMMA_Block_Handle block);
NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_P_CreateFromMma_priv(BMMA_Block_Handle mma_block);

NEXUS_Addr NEXUS_MemoryBlock_GetOffset_priv(NEXUS_MemoryBlockHandle block, unsigned blockOffset);

/* does not bump reference count, so NEXUS_MemoryBlock_Free_driver not required */
NEXUS_Error NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv(void *lockedMem, size_t size,
    NEXUS_MemoryBlockHandle *pBlock,
    unsigned *pOffset /* if NULL, don't calc */
    );

/***************************************************************************
Summary:
Get the Nexus heap from the magnum MMA heap
***************************************************************************/
NEXUS_HeapHandle NEXUS_Heap_GetHeapFromMmaHandle(
    BMMA_Heap_Handle mma
    );

/***************************************************************************
Summary:
Get default heap settings for a memory controller

Description:
The implementation of this function will have chip-specific information.
***************************************************************************/
void NEXUS_Heap_GetDefaultMemcSettings(
    unsigned memcIndex,
    NEXUS_Core_MemoryRegion *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Create a heap
***************************************************************************/
NEXUS_HeapHandle NEXUS_Heap_Create_priv(
    unsigned index, /* have to pass in the index because the heap[] array is sparse, so we need the debug information of where it's going */
    BREG_Handle reg,
    const NEXUS_Core_MemoryRegion *pSettings
    );

/***************************************************************************
Summary:
Destroy a heap
***************************************************************************/
void NEXUS_Heap_Destroy_priv(
    NEXUS_HeapHandle heap
    );

/* returns true if heap has driver mapping (cached and uncached) */
bool NEXUS_P_CpuAccessibleHeap(
    NEXUS_HeapHandle heap
    );

NEXUS_Error NEXUS_Heap_GetStatus_priv( NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus );
NEXUS_Error NEXUS_Heap_AddRegion_priv(NEXUS_HeapHandle heap, NEXUS_Addr base, size_t size);
NEXUS_Error NEXUS_Heap_RemoveRegion_priv(NEXUS_HeapHandle heap, NEXUS_Addr base, size_t size);
NEXUS_Error NEXUS_Heap_GetStatus_driver_priv( NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus );

typedef struct NEXUS_MemaoryRegion {
    NEXUS_Addr base;
    size_t length;
    bool boundary;
} NEXUS_MemoryRegion;

void NEXUS_Heap_GetFreeRegions_priv(NEXUS_HeapHandle heap, NEXUS_MemoryRegion *regions, unsigned numEntries, unsigned *pNumReturned);


NEXUS_OBJECT_CLASS_DECLARE(NEXUS_MemoryBlock);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_FrontendConnector);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_FrontendConnector);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_KeySlot);

typedef enum NEXUS_PowerManagementCore
{
    NEXUS_PowerManagementCore_eVbiDecoder,
    NEXUS_PowerManagementCore_eVdec, /* AnalogVideoDecoder */
    NEXUS_PowerManagementCore_eVideoDecoder, /* digital video decode */
    NEXUS_PowerManagementCore_eCcir656Input,
    NEXUS_PowerManagementCore_eHdDviInput,
    NEXUS_PowerManagementCore_eHdmiInput,
    NEXUS_PowerManagementCore_eAnalogFrontend, /* any internal IF demod */
    NEXUS_PowerManagementCore_eDigitalFrontend, /* any internal digital demod */
    NEXUS_PowerManagementCore_eTransport,
    NEXUS_PowerManagementCore_eDma,
    NEXUS_PowerManagementCore_eHsm,
    NEXUS_PowerManagementCore_eGraphics2D, /* M2MC */
    NEXUS_PowerManagementCore_eGraphics2DMemory, /* M2MC SRAM */
    NEXUS_PowerManagementCore_eGraphics3D, /* PX3D */
    NEXUS_PowerManagementCore_eGraphics3DMemory, /* Px3D/V3D SRAM */
    NEXUS_PowerManagementCore_ePictureDecoder, /* SID */
    NEXUS_PowerManagementCore_ePictureDecoderMemory, /* SID SRAM */
    NEXUS_PowerManagementCore_eImageInput, /* requires MFD HW */
    NEXUS_PowerManagementCore_eRfm,
    NEXUS_PowerManagementCore_eDfe,
    NEXUS_PowerManagementCore_eDs,
    NEXUS_PowerManagementCore_eThd,
    NEXUS_PowerManagementCore_eMax
} NEXUS_PowerManagementCore;

/**
Summary:
Set the state of a core to either powered up or down.

Description:
Internally, nexus will determine if the actual hardware can be powered up or down based
on a variety of chip-specific conditions. Setting a core to be powered down does not
guarantee that power will actually be turned off. It just instructs Nexus can power
can be turned off if possible.

This is a set function which does not fail. There is no need and no way for a module
to recover from the inability to power down. Inside, Nexus will just leave the core
powered up.
**/
void NEXUS_PowerManagement_SetCoreState( NEXUS_PowerManagementCore core, bool poweredUp );

void NEXUS_GetDefaultCommonModuleSettings(
    NEXUS_CommonModuleSettings *pSettings
    );

NEXUS_ModulePriority NEXUS_AdjustModulePriority(
    NEXUS_ModulePriority priority,
    const NEXUS_CommonModuleSettings *pSettings
    );

void NEXUS_VideoFormat_GetInfo_isrsafe(NEXUS_VideoFormat videoFormat, NEXUS_VideoFormatInfo *pInfo);

NEXUS_Error NEXUS_Core_HeapMemcIndex_isrsafe(unsigned heapIndex, unsigned *pMemcIndex);

unsigned NEXUS_Heap_GetMemcIndex_isrsafe(NEXUS_HeapHandle heap);

struct b_objdb_client;
void NEXUS_CoreModule_Uninit_Client_priv(struct b_objdb_client *client);

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#define NEXUS_NUM_DSP_ENCODER_PICTURE_BUFFERS 10
#else
#define NEXUS_NUM_DSP_ENCODER_PICTURE_BUFFERS 8
#endif

void NEXUS_Core_DumpHeaps_priv(void *context);
void NEXUS_Heap_GetRuntimeSettings_priv( NEXUS_HeapHandle heap, NEXUS_HeapRuntimeSettings *pSettings );
NEXUS_Error NEXUS_Heap_SetRuntimeSettings_priv( NEXUS_HeapHandle heap, const NEXUS_HeapRuntimeSettings *pSettings );

NEXUS_HeapHandle NEXUS_Heap_LookupForOffset_isrsafe(
    NEXUS_Addr offset
    );

#ifdef __cplusplus
}
#endif


#endif /* NEXUS_CORE_H */

