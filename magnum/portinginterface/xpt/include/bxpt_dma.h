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
 *****************************************************************************/

#ifndef BXPT_DMA_H__
#define BXPT_DMA_H__

#include "bstd.h"
#include "bkni.h"
#include "bchp.h"
#include "breg_mem.h"
#include "bmma.h"
#include "bint.h"
#include "berr_ids.h"
#include "bxpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
Module Overview:
    XPT_Dma is a XPT-sub API for mem-to-mem DMA transfers.
    It replaces the BMMD portinginterface module on previous platforms,
    and has very similar similar public API as BMMD.

    BMMD utilizes the MEM_DMA HW block, which was independent of the XPT
    HW block. BXPT_Dma utilizes the XPT HW block and therefore shares
    HW resources with the rest of the BXPT portinginterface.

    In XPT_Dma, a "channel" refers to a set of dedicated HW that is required
    for mem-to-mem DMA transfers: an MCPB channel and a WDMA channel.
    Opening a XPT_Dma channel with a channel number reserves the MCPB channel
    with the same channel number for mem-to-mem DMA transfers.

    In XPT_Dma, a "context" is a SW abstraction that refers to a single DMA job:
    a chain of DMA descriptors and its associated completion callback. Multiple
    contexts can be opened from the same channel.

    After a context has been created, it is "enqueued", which queues up the
    context in HW.

    A single context can only perform a DMA transfer at a time.
    That is, after a context has been enqueued, it cannot be enqueued again
    until it has finished. To perform multiple DMA transfers in succession, you
    create multiple contexts and enqueue them in the desired order. The
    completion callbacks will fire in the same order.

    New contexts can be enqueued while another context is currently active,
    i.e. while the HW is currently busy. The portinginterface handles
    serialization and synchronization with HW.
*******************************************************************************/

/* MCPB and WDMA descriptor format */
#if (!BXPT_HAS_MCPB_VER_3)
#define MCPB_DW_READADDRHI     0
#define MCPB_DW_READADDRLO     1
#define MCPB_DW_NEXTDESCADDRLO 2
#define MCPB_DW_READSIZE       3
#define MCPB_DW_FLAGS0         4
#define MCPB_DW_FLAGS1         5
#define MCPB_DW_PIDCHANNEL     6
#define MCPB_DW_NEXTDESCADDRHI 7 /* this word is unused on < MCPB v3 */
#else
#define MCPB_DW_READADDRHI     0
#define MCPB_DW_READADDRLO     1
#define MCPB_DW_READSIZE       2
#define MCPB_DW_FLAGS0         3
#define MCPB_DW_FLAGS1         4
#define MCPB_DW_PIDCHANNEL     5
#define MCPB_DW_NEXTDESCADDRHI 6
#define MCPB_DW_NEXTDESCADDRLO 7
#endif

#if (!BXPT_HAS_MCPB_VER_3)
#define WDMA_DW_WRITEADDRHI    0
#define WDMA_DW_WRITEADDRLO    1
#define WDMA_DW_WRITESIZE      2
#define WDMA_DW_NEXTDESCADDRLO 3
#else
#define WDMA_DW_WRITEADDRHI    0
#define WDMA_DW_WRITEADDRLO    1
#define WDMA_DW_WRITESIZE      2
#define WDMA_DW_RESERVED3      3
#define WDMA_DW_RESERVED4      4
#define WDMA_DW_RESERVED5      5
#define WDMA_DW_NEXTDESCADDRHI 6
#define WDMA_DW_NEXTDESCADDRLO 7
#endif

/*******************************************************************************
Summary:
    XPT_Dma return codes
*******************************************************************************/
#define BXPT_DMA_QUEUED   BERR_MAKE_CODE(BERR_XPT_ID, 32)


/*******************************************************************************
Summary:
    Data scrambling mode
*******************************************************************************/
typedef enum BXPT_Dma_ScramMode
{
    BXPT_Dma_ScramMode_eBlock = 0, /* scramble all data as a generic block */
    BXPT_Dma_ScramMode_eMpeg,      /* only scramble the payload of 188-byte MPEG2-TS packets */
    BXPT_Dma_ScramMode_eDss,       /* only scramble the payload of 130-byte DSS packets */
    BXPT_Dma_ScramMode_eMax
} BXPT_Dma_ScramMode;


/*******************************************************************************
Summary:
    XPT_Dma settings
*******************************************************************************/
typedef struct BXPT_Dma_Settings
{
    BXPT_Dma_ScramMode scramMode; /* scrambling mode */
    bool timestampEnabled; /* true if input stream contains 4 byte timestamp prepended to every packet */
} BXPT_Dma_Settings;


/*******************************************************************************
Summary:
    Get default XPT_Dma settings
*******************************************************************************/
void BXPT_Dma_GetDefaultSettings(
    BXPT_Dma_Settings *pSettings /* [out] */
    );


/*******************************************************************************
Summary:
    Opaque XPT_Dma channel handle
*******************************************************************************/
typedef struct BXPT_Dma_Handle_Tag *BXPT_Dma_Handle;

/*******************************************************************************
Summary:
    Create a XPT_DMA handle with a given channel number and settings

Description:
    The channel number specified to this function will reserve the MCPB channel
    with the same channel number for mem-to-mem DMA transfers.
*******************************************************************************/
BERR_Code BXPT_Dma_OpenChannel(
    BXPT_Handle hXpt,                   /* XPT handle */
    BXPT_Dma_Handle *phDma,             /* [out] XPT_Dma handle */
    unsigned channelNum,                /* channel number */
    const BXPT_Dma_Settings *pSettings  /* settings. may be NULL */
    );

/* for backward compatibility */
#define BXPT_Dma_Open BXPT_Dma_OpenChannel


/*******************************************************************************
Summary:
    Get current channel settings
*******************************************************************************/
BERR_Code BXPT_Dma_GetSettings(
    BXPT_Dma_Handle hDma,
    BXPT_Dma_Settings *pSettings /* [out] */
    );


/*******************************************************************************
Summary:
    Set new channel settings
*******************************************************************************/
BERR_Code BXPT_Dma_SetSettings(
    BXPT_Dma_Handle hDma,
    const BXPT_Dma_Settings *pSettings
    );


/*******************************************************************************
Summary:
    Endian format of source data to be read from memory
*******************************************************************************/
typedef enum BXPT_Dma_EndianMode
{
    BXPT_Dma_EndianMode_eLittle,
    BXPT_Dma_EndianMode_eBig,
    BXPT_Dma_EndianMode_eMax
} BXPT_Dma_EndianMode;


/*******************************************************************************
Summary:
    Endian translation method of data to be written to memory
*******************************************************************************/
typedef enum BXPT_Dma_SwapMode
{
    BXPT_Dma_SwapMode_eNone,
    BXPT_Dma_SwapMode_eByte, /* swap  8-bit bytes (0x00112233 -> 0x33221100) */
    BXPT_Dma_SwapMode_eWord, /* swap 16-bit words (0x00112233 -> 0x22330011) */
    BXPT_Dma_SwapMode_eMax
} BXPT_Dma_SwapMode;


/*******************************************************************************
Summary:
    Context settings

See Also:
    BXPT_Dma_ContextHandle
    BXPT_Dma_Context_Enqueue()
*******************************************************************************/
typedef struct BXPT_Dma_ContextSettings
{
    unsigned maxNumBlocks; /* maximum number of blocks in this context.
                              this parameter is used to allocate memory from the system heap */

    BXPT_Dma_EndianMode endianMode; /* data endianness */
    BXPT_Dma_SwapMode swapMode;     /* endian translation method */
    unsigned pidChannelNum;         /* pid channel number for scrambling in XPT security.
                                       must be greater than or equal to
                                       (BXPT_DMA_PID_CHANNEL_NUM_START + 0) */
    bool useRPipe;

    struct {
        uint64_t offset; /* bounded memory offset (0=no bounds check) */
        uint32_t size;   /* bounded memory size  (0=no bounds check) */
    } memoryBounds; /* used to verify that transfers do not violate memory bounds */

    /* callback that fires in ISR-context when this context has completed. can be NULL */
    void (*callback_isr)(void *pParm1, int parm2);
    void *pParm1;
    int pParm2;
} BXPT_Dma_ContextSettings;

/*******************************************************************************
Summary:
    Pid channels used for DMA
*******************************************************************************/

#ifndef BXPT_P_MEMDMA_PID_CHANNEL_START
#define BXPT_P_MEMDMA_PID_CHANNEL_START 768
#endif

#ifndef BXPT_P_PID_TABLE_SIZE
#define BXPT_P_PID_TABLE_SIZE 1024
#endif

#define BXPT_DMA_PID_CHANNEL_NUM_START (BXPT_P_MEMDMA_PID_CHANNEL_START) /* first pid channel number that is reserved for DMA */
#define BXPT_DMA_NUM_PID_CHANNELS (BXPT_P_PID_TABLE_SIZE-BXPT_P_MEMDMA_PID_CHANNEL_START)

/*******************************************************************************
Summary:
    Get default context settings
*******************************************************************************/
void BXPT_Dma_Context_GetDefaultSettings(
    BXPT_Dma_ContextSettings *pSettings /* [out] */
    );


/*******************************************************************************
Summary:
    Context block settings. A context block is a single DMA descriptor.
*******************************************************************************/
typedef struct BXPT_Dma_ContextBlockSettings
{
    uint64_t src;  /* source physical address */
    uint64_t dst;  /* destination physical address */
    uint32_t size; /* block size in bytes */

    bool resetCrypto;  /* if true, discontinue crypto from previous block and re-initialize */
    bool sgScramStart; /* if true, then this block indicates the start of scatter-gather scrambling operation */
    bool sgScramEnd;   /* if true, then this block indicates the end of scatter-gather scrambling operation */
    bool securityBtp;  /* if true, then this block is a BTP descriptor */

    const void* srcPtr; /* unused */
    void* dstPtr;       /* unused */
} BXPT_Dma_ContextBlockSettings;


/*******************************************************************************
Summary:
    Get default context block settings
*******************************************************************************/
void BXPT_Dma_Context_GetDefaultBlockSettings(
    BXPT_Dma_ContextBlockSettings *pSettings /* [out] */
    );


/*******************************************************************************
Summary:
    Opaque XPT_Dma context handle

Description:
    An XPT_Dma context represents a single DMA job to be performed: a chain of
    DMA descriptors and its associated completion callback.

    A single XPT_Dma context can only be used for a single DMA job at a time.
    That is, after a context has been enqueued, it cannot be enqueued again
    before it has finished. To perform multiple DMA jobs in succession, you
    create multiple contexts and enqueue them in the desired order.

    There is no XPT_Dma-imposed limit to how many contexts can be created.
*******************************************************************************/
typedef struct BXPT_Dma_Context *BXPT_Dma_ContextHandle;


/*******************************************************************************
Summary:
    Create an XPT_Dma context handle
*******************************************************************************/
BXPT_Dma_ContextHandle BXPT_Dma_Context_Create(
    BXPT_Dma_Handle hDma,
    const BXPT_Dma_ContextSettings *pSettings
    );


/*******************************************************************************
Summary:
    Get current context settings
*******************************************************************************/
BERR_Code BXPT_Dma_Context_GetSettings(
    BXPT_Dma_ContextHandle hCtx,
    BXPT_Dma_ContextSettings *pSettings /* [out] */
    );


/*******************************************************************************
Summary:
    Set new context settings

Description:
    Context settings can only be changed when the context is idle. Otherwise,
    this function will fail.
    Changing BXPT_Dma_ContextSettings.maxNumBlocks is not allowed. To change
    maxNumBlocks, a new context must be created.
*******************************************************************************/
BERR_Code BXPT_Dma_Context_SetSettings(
    BXPT_Dma_ContextHandle hCtx,
    const BXPT_Dma_ContextSettings *pSettings
    );


/*******************************************************************************
Summary:
    Enqueue a context with an array of context block settings

Description:
    If no other contexts are currently active, then the context starts
    immediately.
    Otherwise, the context is processed after all previously-queued contexts
    have completed.

    The context has to be idle in order to be enqueued.

Returns:
    BERR_SUCCESS        if context was completed
    BXPT_DMA_QUEUED     if context is in progress or is queued
    BERR_NOT_SUPPORTED  if context was not idle
*******************************************************************************/
BERR_Code BXPT_Dma_Context_Enqueue(
    BXPT_Dma_ContextHandle hCtx,
    const BXPT_Dma_ContextBlockSettings *pSettings, /* array of BXPT_Dma_ContextBlockSettings */
    unsigned numBlocks                              /* number of BXPT_Dma_ContextBlockSettings elements.
                                                       must be <= BXPT_Dma_ContextSettings.maxNumBlocks */
    );

/*******************************************************************************
Summary:
    Configure a pidchannel for a context
*******************************************************************************/
BERR_Code BXPT_Dma_Context_ConfigurePidChannel(
    BXPT_Dma_ContextHandle hCtx,
    unsigned pidChannelNum,
    unsigned pid,
    bool enable
    );

/*******************************************************************************
Summary:
    Context state
*******************************************************************************/
typedef enum BXPT_Dma_ContextState
{
    BXPT_Dma_ContextState_eIdle,       /* idle or complete */
    BXPT_Dma_ContextState_eInProgress  /* queued in HW. the HW may not yet have started
                                          the context if there are other active contexts */
} BXPT_Dma_ContextState;


/*******************************************************************************
Summary:
    Context status
*******************************************************************************/
typedef struct BXPT_Dma_ContextStatus
{
    BXPT_Dma_ContextState state;
} BXPT_Dma_ContextStatus;


/*******************************************************************************
Summary:
    Get current status of an XPT_DMA context
*******************************************************************************/
BERR_Code BXPT_Dma_Context_GetStatus(
    BXPT_Dma_ContextHandle hCtx,
    BXPT_Dma_ContextStatus *pStatus /* [out] */
    );


/*******************************************************************************
Summary:
    Destroy an XPT_Dma context

Description:
    If there are no currently active contexts, then the memory allocated for
    DMA descriptors is freed immediately.

    If there are currently active contexts, then this function does not wait
    for them to finish. The completion callback will not fire. The contexts
    may or may not finish; this behavior is undefined.
    The memory allocated for DMA descriptors may not be freed immediately.
*******************************************************************************/
void BXPT_Dma_Context_Destroy(
    BXPT_Dma_ContextHandle hCtx
    );


/*******************************************************************************
Summary:
    Close XPT_Dma handle

Description:
    This function immediately stops the HW, destroys any undestroyed contexts,
    and frees all resources.
*******************************************************************************/
void BXPT_Dma_CloseChannel(
    BXPT_Dma_Handle hDma
    );

/* Internal API. User code should NOT call this. */
void BXPT_Dma_P_EnableInterrupts(BXPT_Handle hXpt);
BERR_Code BXPT_Dma_P_OpenChannel(BXPT_Handle hXpt, BCHP_Handle hChp, BREG_Handle hReg, BMMA_Heap_Handle hMmaHeap, BINT_Handle hInt, BXPT_Dma_Handle *phDma, unsigned channelNum, const BXPT_Dma_Settings *pSettings);

#ifdef __cplusplus
}
#endif

#endif
