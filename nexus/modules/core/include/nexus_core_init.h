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
#ifndef NEXUS_CORE_INIT_H__
#define NEXUS_CORE_INIT_H__

#include "bstd.h"
#include "nexus_types.h"
#include "nexus_memory.h"
#include "btee_instance.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum NEXUS_MemoryMapType {
    NEXUS_MemoryMapType_eUncached,
    NEXUS_MemoryMapType_eCached,
    NEXUS_MemoryMapType_eFake,
    NEXUS_MemoryMapType_eMax
} NEXUS_MemoryMapType;


/***************************************************************************
Summary:
Structure used by Platform to instruct Core how to create magnum heaps and their accompanying Nexus heap handles
****************************************************************************/
typedef struct NEXUS_Core_MemoryRegion
{
    unsigned memcIndex;     /* memory controller index */
    void *pvAddr;           /* virtual uncached address of memory region */
    NEXUS_Addr offset;      /* physical device offset (address) of memory region */
    size_t length;          /* length in bytes of the memory region */
    void *pvAddrCached;     /* virtual cached address of memory region, optional */
    unsigned memoryType;    /* bitmasdk of NEXUS_MEMORY_TYPE_XXX macros, found in nexus_types.h */
    unsigned heapType;      /* bitmask of NEXUS_HEAP_TYPE_XXX macros, found in nexus_types.h */
    unsigned alignment;     /* required alignment (in bytes) of allocations in this region */
    bool locked;            /* nexus does not allow new allocations from a locked heap. */
    bool guardBanding;      /* [deprecated] if true, use guard bands if possible. if false, force no guard banding for higher performance. */
    NEXUS_MemoryMapType cachedMapType;
    NEXUS_MemoryMapType uncachedMapType;
} NEXUS_Core_MemoryRegion;

/***************************************************************************
Summary:
Interface for handling OS interrupts
****************************************************************************/
typedef struct NEXUS_CoreInterruptInterface
{
    void (*pDisconnectInterrupt)(unsigned ulIrqNum);
    NEXUS_Error (*pConnectInterrupt)(unsigned irqNum, NEXUS_Core_InterruptFunction pIsrFunc, void *pFuncParam, int iFuncParam);
    NEXUS_Error (*pEnableInterrupt_isr)(unsigned irqNum);
    void  (*pDisableInterrupt_isr)(unsigned irqNum);
} NEXUS_CoreInterruptInterface;

/***************************************************************************
Summary:
Callback function prototype for NEXUS_CoreImageInterface
****************************************************************************/
typedef void*(*NEXUS_CoreImageInterface_Create)(
    const char *id
    );

/***************************************************************************
Summary:
Callback function prototype for NEXUS_CoreImageInterface
****************************************************************************/
typedef void (*NEXUS_CoreImageInterface_Destroy)(
    void *imageInterface
    );

/***************************************************************************
Summary:
Callback function prototype for NEXUS_CoreImageInterface
****************************************************************************/
typedef BERR_Code (*NEXUS_CoreImageInterface_Open)(
    void *context,  /* context of the image interface */
    void **image,  /* [out] pointer to the image context */
    unsigned image_id /* ID of the image */
    );

/***************************************************************************
Summary:
Callback function prototype for NEXUS_CoreImageInterface
****************************************************************************/
typedef void (*NEXUS_CoreImageInterface_Close)(
    void *image /* image context */
    );

/***************************************************************************
Summary:
Callback function prototype for NEXUS_CoreImageInterface
****************************************************************************/
typedef BERR_Code (*NEXUS_CoreImageInterface_Next)(
    void *image,  /* image context information, one returned by the 'open' call */
    unsigned chunk,  /* number of chunk, starts from 0, shall increment with each call */
    const void **data,  /*  [out] returns pointer to next piece of data, contents of this pointer is valid until succeding call to the 'next' function */
    uint16_t length /* number of bytes to read from the image,  length shall be less than 64K */
    );

/***************************************************************************
Summary:
Interface for Magnum's IMG interface used in NEXUS_Core_Settings.

Description:
The IMG interface is used for firmware loading in Magnum.
****************************************************************************/
typedef struct NEXUS_CoreImageInterface
{
    NEXUS_CoreImageInterface_Create create;   /* create image interface */
    NEXUS_CoreImageInterface_Destroy destroy; /* destroy the image interface  */
    NEXUS_CoreImageInterface_Open open;       /* open method */
    NEXUS_CoreImageInterface_Next next;       /* 'next' method */
    NEXUS_CoreImageInterface_Close close;     /* close method */
} NEXUS_CoreImageInterface;


/***************************************************************************
Summary:
Settings for the Core module
****************************************************************************/
typedef struct  NEXUS_Core_Memc_Information {
    NEXUS_Addr offset; /* offset e.g. physical address of first memory location addressable by MEMC */
    uint64_t length; /* size of the memory region addressable by MEMC */
} NEXUS_Core_Memc_Information;

/***************************************************************************
Summary:
Settings for the Core module
****************************************************************************/
typedef struct NEXUS_Core_Settings
{
    BREG_Handle regHandle;
    NEXUS_Core_MemoryRegion heapRegion[NEXUS_MAX_HEAPS]; /* heaps to allocate. set heapRegion[].length == 0 to terminate. */
    NEXUS_CoreInterruptInterface interruptInterface;
    NEXUS_CoreImageInterface imgInterface; /* Image interface */
    NEXUS_Core_Memc_Information memcRegion[NEXUS_MAX_MEMC];
    unsigned defaultHeapIndex;
    BTEE_InstanceHandle teeHandle;
} NEXUS_Core_Settings;

/***************************************************************************
Summary:
Get default settings for Core module
****************************************************************************/
void NEXUS_CoreModule_GetDefaultSettings(
    NEXUS_Core_Settings *pSettings /* [out] Default Settings */
    );

struct NEXUS_Core_PreInitState;
struct BINT_Settings;

/***************************************************************************
Summary:
Init the Core module
****************************************************************************/
NEXUS_ModuleHandle NEXUS_CoreModule_Init(
    const NEXUS_Core_Settings *pSettings,
    const struct NEXUS_Core_PreInitState *preInitState,
    const struct BINT_Settings *pIntSettings
    );

/***************************************************************************
Summary:
Uninit the Core module
****************************************************************************/
void NEXUS_CoreModule_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_CORE_INIT_H__ */
