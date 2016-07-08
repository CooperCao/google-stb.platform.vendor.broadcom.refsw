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
*
* API Description:
*   API name: Dma
*    Specific APIs related to memory to memory DMA.
*
***************************************************************************/

#ifndef NEXUS_DMA_H__
#define NEXUS_DMA_H__

#include "nexus_dma_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************************************************
The Nexus Dma interface provides M2M (mem-to-mem) DMA functionality to applications.

The caller is responsible to maintain cache coherency using NEXUS_FlushCache.
All nexus memory pointers provided to the caller are cached addresses, so you should assume that cache coherency is an issue.

There are potentially three CPU caches you should be aware of:
    1) L1 cache - this is a write-back cache
    2) L2 cache - this is a write-back cache
    3) RAC (read ahead cache) - this is a read-only cache which are "read ahead" up to a 4K boundary

NEXUS_FlushCache will do a wback_invalidate of the L1 and L2 caches and an invalidate of the RAC.

The following cache flush rules will allow your app to maintain cache coherency.

Rule 1: After DMA writes to RAM, you must flush the cache before the CPU reads from that memory. That is:

    1. DMA writes to RAM
    2. flush cache for that RAM
    3. CPU reads from that RAM

Rule 2: After the CPU writes to RAM, you must flush the cache before DMA does any access to that memory. That is:

    1. CPU writes to RAM
    2. flush cache for that RAM
    3. DMA reads from that RAM

There is a danger created by the RAC's 4K-bounded prefetch that requires special attention.
The danger is in reversing the order of steps 1 & 2 in Rule 1 if an allocation is not front and back aligned to 4K boundaries
(i.e. no adjacent allocation shares addresses within a 4K block of the front or back of this allocation).
If an allocation is not so aligned, then a cached read in an adjacent allocation may populate the RAC before DMA updates it.
The RAC will then contain stale data that the application does not invalidate. A critical error could easily result.
By default, the magnum MEM heap (which underlies all nexus heaps) is aligned to the max of the L1 and L2 cache line size (e.g. 64
or 128 bytes). So there is no "adjacent allocation" danger for these caches.
However, the MEM heap is not aligned to the RAC's 4K prefetch. So the "adjacent allocation" danger does occur for the RAC.

This danger can appear in a variety of ways. One basic scenario is:

    1. CPU writes to RAM
    2. flush cache for that RAM
    3. DMA reads from that RAM
    4. a CPU read from an adjacent allocation (within the 4K boundary) occurs in another thread; the RAC is populated for both the adjacent
       allocation and the memory about to be written to in step 5
    5. DMA writes to RAM
       >>app mistakenly thinks that it doesn't need to flush here because it already flushed in step 2.
    6. CPU reads stale data from the RAC for that RAM

Even though the multiple accesses make the scenario more complicated, the root of this problem is that the application mistakenly believed
that steps 1 & 2 in Rule 1 could be reversed. They cannot. In this case, a flush before the DMA read and another flush after the DMA write are both required.

In nexus, interfaces like Recpump and Message fall into Rule 1. Nexus internally flushes the cache in the GetBuffer call.
Interfaces like Playpump fall into Rule 2. Nexus internally flushes the cache in the ReadComplete call.
Interfaces like Surface typically combine both CPU reads and writes and DMA reads and writes. Nexus cannot efficiently flush before and after all DMA transactions.
Therefore, Nexus defaults all surface allocations to 4K alignment so that the RAC coherency danger does not spill over from a surface
to an adjacent allocation or from an adjacent allocation into the surface. However, any combination of blits and CPU writes/reads within the surface need to follow
both cache flush rules to avoid problems.

See nexus/examples/graphics/bad_rac_blit.c for a demonstration of the problem and the recommended solutions.
**************************************************************************/

/***************************************************************************
Summary:
DMA channel settings

Description:
See NEXUS_Dma_Open for restrictions on changing these settings if you are using more than one instance of a NEXUS_Dma channel.
***************************************************************************/
typedef struct NEXUS_DmaSettings
{
    NEXUS_DmaEndianMode endianMode; /* endian mode */
    NEXUS_DmaSwapMode swapMode;     /* byteswap mode */
    NEXUS_DmaCoreType coreType;     /* which DMA core to use */
    NEXUS_HeapHandle boundsHeap;    /* optional heap to bounds check DMA transfers */

    unsigned totalBlocks;           /* deprecated */
    unsigned maxBlocksPerTransfer;  /* deprecated */
} NEXUS_DmaSettings;

/***************************************************************************
Summary:
Get default settings for the structure.
***************************************************************************/
void NEXUS_Dma_GetDefaultSettings(
    NEXUS_DmaSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open a DMA channel

Description:
Nexus DMA supports virtualization by allowing NEXUS_Dma_Open to be 
called more than once with the same index.
***************************************************************************/
NEXUS_DmaHandle NEXUS_Dma_Open(  /* attr{destructor=NEXUS_Dma_Close}  */
    unsigned index, /* M2M_DMA controller index */
    const NEXUS_DmaSettings *pSettings  /* attr{null_allowed=y} may be NULL for default settings */
    );

/***************************************************************************
Summary:
Close a DMA channel
***************************************************************************/
void NEXUS_Dma_Close(
    NEXUS_DmaHandle handle
    );

/***************************************************************************
Summary:
Get current settings from a DMA channel
***************************************************************************/
NEXUS_Error NEXUS_Dma_GetSettings(
    NEXUS_DmaHandle handle,
    NEXUS_DmaSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the current settings of a DMA channel

Description:
See NEXUS_Dma_Open for restrictions on changing these settings if you are using more than one instance of a NEXUS_Dma channel.
***************************************************************************/
NEXUS_Error NEXUS_Dma_SetSettings(
    NEXUS_DmaHandle handle,
    const NEXUS_DmaSettings *pSettings
    );

/**
Summary:
Handle for a DMA job.
**/
typedef struct NEXUS_DmaJob *NEXUS_DmaJobHandle;

/***************************************************************************
Summary:
DMA Job Settings

Description:
See Also:
NEXUS_Dma_GetDefaultJobSettings
NEXUS_DmaJob_Create
***************************************************************************/
typedef struct NEXUS_DmaJobSettings
{
    unsigned numBlocks;             /* maximum number of blocks in this job. for scatter-gather jobs, this can be > 1 */
    unsigned busyWait;              /* units of microseconds. NEXUS_DmaJob_ProcessBlocks does a busy loop waiting for DMA completion of each block up to this 
                                       time if block size is <= busyWaitThreshold. Use 0 for no busy wait. */
    unsigned busyWaitThreshold;     /* units of bytes. see 'busyWait' for usage. */
    NEXUS_KeySlotHandle keySlot;    /* key slot for security.  NULL(default) if not encrypting or decrypting data */
    NEXUS_BypassKeySlot bypassKeySlot; /* if keySlot is NULL, this is used for SVP regions. */

    /* for XPT_DMA-based platforms, dataFormat and timestampType are per-channel, not per-job */
    NEXUS_DmaDataFormat dataFormat;
    NEXUS_TransportTimestampType timestampType; /* 4-byte timestamp preprended to each packet. valid only for NEXUS_DmaDataFormat_eMpeg */

    NEXUS_CallbackDesc completionCallback; /* if set, this callback will fire when the job completes */
    bool useRPipe;

    /* SHARF-specific settings. in SHARF mode, keySlot and dataFormat have 
       no effect and are replaced by SHARF-specific settings */
    struct {
        bool useBspKey; /* if true, SHARF will use a secure key supplied by BSP directly 
                           for data encryption/decryption. if false, SHARF will use the key
                           that prepends the descriptor data */
        NEXUS_DmaSharfMode mode;
        unsigned shaContext; /* SHARF HW can hold the intermediate or final SHA-1 digest 
                                for up to 3 contexts across all SHARF DMA channels. This 
                                allows interleaving SHARF DMA operations. 
                                Valid values are 0,1 or 2. */
    } sharf;
} NEXUS_DmaJobSettings;

/***************************************************************************
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_DmaJob_Create
***************************************************************************/
void NEXUS_DmaJob_GetDefaultSettings(
    NEXUS_DmaJobSettings *pSettings
    );

/***************************************************************************
Summary:
Create a DMA Job

Description:
A DMA Job is a context of DMA descriptors and its assocatied completion callback & status.
You can have more than one active job on a DMA channel.
***************************************************************************/
NEXUS_DmaJobHandle NEXUS_DmaJob_Create( /* attr{destructor=NEXUS_DmaJob_Destroy} */
    NEXUS_DmaHandle dmaHandle,
    const NEXUS_DmaJobSettings *pSettings /* attr{null_allowed=y} may be NULL for default settings */
    );

/***************************************************************************
Summary:
Destroy a DMA Job
***************************************************************************/
void NEXUS_DmaJob_Destroy(
    NEXUS_DmaJobHandle handle
    );

/***************************************************************************
Summary:
Get current settings from a DMA job
***************************************************************************/
void NEXUS_DmaJob_GetSettings(
    NEXUS_DmaJobHandle handle,
    NEXUS_DmaJobSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set new settings for a DMA job

Description:
Some DmaJobSettings (e.g. numBlocks) cannot be changed and require a new job to be created
***************************************************************************/
NEXUS_Error NEXUS_DmaJob_SetSettings(
    NEXUS_DmaJobHandle handle,
    const NEXUS_DmaJobSettings *pSettings
    );

/***************************************************************************
Summary:
DMA Job Block Descriptor
***************************************************************************/
typedef struct NEXUS_DmaJobBlockSettings
{
    const void *pSrcAddr;   /* attr{memory=cached} Source address */
    void *pDestAddr;        /* attr{memory=cached} Destination address - may be the same as source address */
    size_t blockSize;       /* in bytes */

    bool cached;            /* Use cache flush on source and destination addresses.
                               Requires that device memory have server-side memory mapping.
                               Source flush happens before read; destination flush happens before and after write;
                               therefore, you can be more efficient and flexible by setting this false and calling
                               NEXUS_FlushCache in your app only as required. See above for general rules for cache flush. */
    bool resetCrypto;       /* Should the crypto operation reset on this block? This flag has an effect only when paired with scatterGatherCryptoStart */

    bool scatterGatherCryptoStart; /* If true, this block starts a scatter-gather crypto op */
    bool scatterGatherCryptoEnd;   /* If true, this block ends a scatter-gather crypto op */
    bool securityBtp; /* if true, this block is a security BTP descriptor */

    struct {
        bool keyPresent;    /* if true, a crypto key prepends the data */
        bool digestPresent; /* if true, a hash digest or MAC prepends the data */
    } sharf;
} NEXUS_DmaJobBlockSettings;

/***************************************************************************
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
***************************************************************************/
void NEXUS_DmaJob_GetDefaultBlockSettings(
    NEXUS_DmaJobBlockSettings *pSettings /* [out] */
    );

/* deprecated */
#define NEXUS_DmaJob_SetBlockSettings(handle,blockIndex,pSettings) NEXUS_NOT_SUPPORTED
#define NEXUS_DmaJob_Start(handle) NEXUS_NOT_SUPPORTED

#define NEXUS_DMA_QUEUED NEXUS_MAKE_ERR_CODE(0x100, 1)

/***************************************************************************
Summary:
Prepare and run a DMA transaction

Description:
This function prepares and starts a DMA job operation.  If function returns NEXUS_SUCCESS, then
DMA transaction is completed and new transaction could be started right away.
However if function returns NEXUS_DMA_QUEUED, then caller should wait
for the completion callback or poll the current status with NEXUS_DmaJob_GetStatus
to ensure the transfer is complete before using the same job a second
time.

NEXUS_DmaJob_ProcessBlocks is used for fast batch DMA processing. It should be used
instead of multiple calls to NEXUS_DmaJob_SetBlockSettings and NEXUS_Error NEXUS_DmaJob_Start.

See Also:
    NEXUS_DmaJob_GetStatus
Returns:
  On Success
    NEXUS_SUCCESS - DMA completed
    NEXUS_DMA_QUEUED - DMA job queued for processing
***************************************************************************/
NEXUS_Error NEXUS_DmaJob_ProcessBlocks(
    NEXUS_DmaJobHandle handle,
    const NEXUS_DmaJobBlockSettings *pSettings, /* attr{nelem=nBlocks;reserved=2} pointer to array of DMA blocks */
    unsigned nBlocks                            /* Must be < NEXUS_DmaJobSettings.numBlocks */
    );

/***************************************************************************
Summary:
DMA Job Block Descriptor that uses offsets (physical addresses) for source and destination
***************************************************************************/
typedef struct NEXUS_DmaJobBlockOffsetSettings
{
    NEXUS_Addr srcOffset;     /* source offset */
    NEXUS_Addr destOffset;    /* destination offset - may be the same as source offset */
    size_t blockSize;       /* in bytes */

    bool resetCrypto;       /* Should the crypto operation reset on this block? This flag has an effect only when paired with scatterGatherCryptoStart */

    bool scatterGatherCryptoStart; /* If true, this block starts a scatter-gather crypto op */
    bool scatterGatherCryptoEnd;   /* If true, this block ends a scatter-gather crypto op */
    bool securityBtp; /* if true, this block is a security BTP descriptor */

    struct {
        bool keyPresent;    /* if true, a crypto key prepends the data */
        bool digestPresent; /* if true, a hash digest or MAC prepends the data */
    } sharf;
} NEXUS_DmaJobBlockOffsetSettings;

/***************************************************************************
Summary:
Offset-version of NEXUS_DmaJob_GetDefaultBlockSettings()
***************************************************************************/
void NEXUS_DmaJob_GetDefaultBlockOffsetSettings(
    NEXUS_DmaJobBlockOffsetSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Offset-version of NEXUS_DmaJob_ProcessBlocks()
***************************************************************************/
NEXUS_Error NEXUS_DmaJob_ProcessBlocksOffset(
    NEXUS_DmaJobHandle handle,
    const NEXUS_DmaJobBlockOffsetSettings *pSettings, /* attr{nelem=nBlocks;reserved=2} pointer to array of DMA blocks */
    unsigned nBlocks                                  /* Must be < NEXUS_DmaJobSettings.numBlocks */
    );

/***************************************************************************
Summary:
DMA Job State returned by NEXUS_DmaJobStatus
***************************************************************************/
typedef enum NEXUS_DmaJobState
{
    NEXUS_DmaJobState_eComplete,   /* completed */
    NEXUS_DmaJobState_eFailed,     /* deprecated */
    NEXUS_DmaJobState_eInProgress, /* queued in HW */
    NEXUS_DmaJobState_ePending,    /* deprecated */
    NEXUS_DmaJobState_eIdle,       /* not yet started */
    NEXUS_DmaJobState_eUnknown,
    NEXUS_DmaJobState_eMax
} NEXUS_DmaJobState;

/***************************************************************************
Summary:
Status returned by NEXUS_DmaJob_GetStatus
***************************************************************************/
typedef struct NEXUS_DmaJobStatus
{
    NEXUS_DmaJobState currentState;
} NEXUS_DmaJobStatus;

/***************************************************************************
Summary:
Get the status of a DMA job

Description:
This is only required if you are polling and not using NEXUS_DmaJobSettings.completionCallback.
***************************************************************************/
NEXUS_Error NEXUS_DmaJob_GetStatus(
    NEXUS_DmaJobHandle handle,
    NEXUS_DmaJobStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DMA_H__ */

