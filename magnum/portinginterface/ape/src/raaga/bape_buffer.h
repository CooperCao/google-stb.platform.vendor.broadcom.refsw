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
 * Module Description: Simple Buffer Interface
 *
 ***************************************************************************/

#ifndef BAPE_BUFFER_H_
#define BAPE_BUFFER_H_

#include "bmma.h"
#include "bape_priv.h"

#define BAPE_MIN( v0, v1 )        (((v0) < (v1)) ? (v0) : (v1))
#if BAPE_DSP_SUPPORT
#define BAPE_FLUSHCACHE_ISRSAFE(block, mem, size) BMMA_FlushCache_isrsafe(block, mem, size)
#else
#define BAPE_FLUSHCACHE_ISRSAFE(block, mem, size) do {} while(0)
#endif

/***************************************************************************
Summary:
Simple Buffer Descriptor
***************************************************************************/
typedef struct BAPE_SimpleBufferDescriptor
{
    /* TBD7211 - add block and offset ??? */
    BMMA_Block_Handle block;    /* BMMA Block */
    BMMA_DeviceOffset offset;   /* offset @ location where pBuffer starts */
    BMMA_DeviceOffset wrapOffset; /* offset @ location where pWrapBuffer starts */
    void *pBuffer;              /* Buffer base address prior to wraparound */
    void *pWrapBuffer;          /* Buffer address after wraparound (NULL if no wrap has occurred) */

    unsigned bufferSize;        /* Buffer size before wraparound in bytes */
    unsigned wrapBufferSize;    /* Buffer size after wraparound in bytes */
} BAPE_SimpleBufferDescriptor;

typedef enum BAPE_BufferType
{
    BAPE_BufferType_eReadWrite,
    BAPE_BufferType_eReadOnly,
    BAPE_BufferType_eWriteOnly,
    BAPE_BufferType_eMax
} BAPE_BufferType;

/***************************************************************************
Summary:
Buffer Settings
***************************************************************************/
typedef struct BAPE_BufferSettings
{
    BAPE_BufferType type;               /* Role of this BAPE_Buffer interface with respect to the buffer at large.
                                           Default is ReadWrite */

    BMMA_Heap_Handle heap;              /* Optional - Heap to use for internal allocation */
    size_t bufferSize;                  /* Buffer size in bytes */

    BMMA_Block_Handle interfaceBlock;   /* Must be specified if buffer interface is used */
    BMMA_DeviceOffset interfaceOffset;  /* Must be specified if buffer interface is used */
    BAPE_BufferInterface * pInterface;  /* Optionally specify buffer interface to be shared by HW/SW/DSP.
                                           heap, pBuffer, bufferSize are intended to be NULL if
                                           buffer interface is used. */
    void *pBuffer;                      /* Optional - User can pass in an externally allocated buffer,
                                           Heap should be null in that case */
} BAPE_BufferSettings;
 
/***************************************************************************
Summary:
Buffer Get Default Settings
***************************************************************************/
void BAPE_Buffer_GetDefaultSettings(
    BAPE_BufferSettings *pSettings /*[out] */
    );

/***************************************************************************
Summary:
Buffer Open
***************************************************************************/
BERR_Code BAPE_Buffer_Open(
    BAPE_Handle deviceHandle,
    const BAPE_BufferSettings * pSettings, 
    BAPE_BufferHandle * pHandle /* [out] */
    );

/***************************************************************************
Summary:
Buffer Close
***************************************************************************/
void BAPE_Buffer_Close(
    BAPE_BufferHandle handle
    );

/***************************************************************************
Summary:
Buffer Read
***************************************************************************/
unsigned BAPE_Buffer_Read_isr(
    BAPE_BufferHandle handle, 
    BAPE_SimpleBufferDescriptor * pDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Buffer Read Complete
***************************************************************************/
unsigned BAPE_Buffer_ReadComplete_isr(
    BAPE_BufferHandle handle,
    unsigned size
    );

/***************************************************************************
Summary:
Get Buffer for writing - non destructive
***************************************************************************/
BERR_Code BAPE_Buffer_GetWriteBuffer_isr(
    BAPE_BufferHandle handle,
    BAPE_SimpleBufferDescriptor * pDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Buffer Write - if pData is NULL, just move ptrs
***************************************************************************/
unsigned BAPE_Buffer_Write_isr(
    BAPE_BufferHandle handle, 
    void * pData,
    unsigned size
    );

/***************************************************************************
Summary:
Buffer Write Complete
***************************************************************************/
unsigned BAPE_Buffer_WriteComplete_isr(
    BAPE_BufferHandle handle,
    unsigned size
    );

/***************************************************************************
Summary:
Buffer Flush
***************************************************************************/
void BAPE_Buffer_Flush_isr(
    BAPE_BufferHandle handle
    );

/***************************************************************************
Summary:
Buffer Get Depth
***************************************************************************/
unsigned BAPE_Buffer_GetBufferDepth_isr(
    BAPE_BufferHandle handle
    );

/***************************************************************************
Summary:
Buffer Get Free Space
***************************************************************************/
unsigned BAPE_Buffer_GetBufferFree_isr(
    BAPE_BufferHandle handle
    );

/***************************************************************************
Summary:
Buffer Enable Consumption
***************************************************************************/
void BAPE_Buffer_Enable_isr(
    BAPE_BufferHandle handle,
    bool enable
    );

typedef struct BAPE_BufferFormat
{
    bool            interleaved;
    bool            compressed;
    uint8_t         samplesPerDword; /* 1, 2, or 4 (8 bit only) */
    uint8_t         numChannels;     /* 1-8 */
    uint8_t         bitsPerSample;
} BAPE_BufferFormat;

/***************************************************************************
Summary:
Buffer report enable status
***************************************************************************/
bool BAPE_Buffer_IsEnabled_isrsafe(
    BAPE_BufferHandle handle
    );

/***************************************************************************
Summary:
Buffer Get Format
***************************************************************************/
BERR_Code BAPE_Buffer_GetFormat_isrsafe(
    BAPE_BufferHandle handle,
    BAPE_BufferFormat * pFormat
    );

/***************************************************************************
Summary:
Buffer Set Format
***************************************************************************/
BERR_Code BAPE_Buffer_SetFormat_isr(
    BAPE_BufferHandle handle,
    BAPE_BufferFormat * pFormat
    );

/***************************************************************************
Summary:
Buffer Group Open Settings
***************************************************************************/
typedef struct BAPE_BufferGroupOpenSettings
{
    BAPE_BufferType type;               /* Role of this BAPE_Buffer interface with respect to the buffer at large.
                                           Default is ReadWrite */

    BMMA_Heap_Handle heap;              /* Optional - Heap to use for internal allocation */
    size_t bufferSize;                  /* Buffer size in bytes (per buffer). must be non-zero unless buffers
                                           are allocated externally (see below) */

    unsigned numChannels;               /* number of buffers (channels or channel pairs) */
    bool interleaved;                   /* if true, numChannels specifies num channel pairs */
    bool bufferless;                    /* no local buffers. Will link to an parent buffer group and use its buffers */

    /* Optional - user may pass in pre allocated buffers */
    struct {
        BMMA_Block_Handle interfaceBlock;   /* BMMA block for buffer interface */
        BMMA_DeviceOffset interfaceOffset;  /* BMMA Device Offset */
        BAPE_BufferInterface * pInterface;  /* Buffer Interface to be shared by HW/SW/DSP. */
        void *pBuffer;                      /* Host addressable memory for buffer managed by buffer interface */
    } buffers[BAPE_Channel_eMax];
} BAPE_BufferGroupOpenSettings;

/***************************************************************************
Summary:
Buffer Group Format
***************************************************************************/
typedef struct BAPE_BufferGroupFormat
{
    bool compressed;                    /* reports if data is compressed */
    unsigned bitsPerSample;             /* bits per sample - 16, 24, 32. 0 indicates unknown */
    unsigned samplesPerDword;           /* samples per dword - typically 1 or 2. if 8bits/sample, then this could be 4 */
} BAPE_BufferGroupFormat;

/***************************************************************************
Summary:
Buffer Group Status
***************************************************************************/
typedef struct BAPE_BufferGroupStatus
{
    BAPE_BufferType type;               /* Role of this BAPE_Buffer interface with respect to the buffer at large.
                                           Default is ReadWrite */
    size_t bufferSize;                  /* Buffer size in bytes (per buffer). must be non-zero unless buffers
                                           are allocated externally (see below) */

    bool enabled;                       /* reports if consumption is enabled */

    bool interleaved;                   /* reports if buffers are interleaved or not */
    bool compressed;                    /* reports if data is compressed */
    unsigned numChannels;               /* number of buffers (channels or channel pairs) */
    unsigned bitsPerSample;             /* bits per sample - 16, 24, 32. 0 indicates unknown */
    unsigned samplesPerDword;           /* samples per dword - typically 1 or 2. if 8bits/sample, then this could be 4 */
} BAPE_BufferGroupStatus;

/***************************************************************************
Summary:
Buffer Get Default Settings
***************************************************************************/
void BAPE_BufferGroup_GetDefaultOpenSettings(
    BAPE_BufferGroupOpenSettings *pSettings /*[out] */
    );

/***************************************************************************
Summary:
Buffer Group Open
***************************************************************************/
BERR_Code BAPE_BufferGroup_Open(
    BAPE_Handle deviceHandle,
    const BAPE_BufferGroupOpenSettings * pSettings,
    BAPE_BufferGroupHandle * pHandle /* [out] */
    );

/***************************************************************************
Summary:
Buffer Group Close
***************************************************************************/
void BAPE_BufferGroup_Close(
    BAPE_BufferGroupHandle handle
    );

/***************************************************************************
Summary:
Buffer Group - link an output buffer group
***************************************************************************/
BERR_Code BAPE_BufferGroup_LinkOutput(
    BAPE_BufferGroupHandle pSource,
    BAPE_BufferGroupHandle pDest
    );

/***************************************************************************
Summary:
Buffer Group - unlink an output buffer group
***************************************************************************/
void BAPE_BufferGroup_UnLinkOutput(
    BAPE_BufferGroupHandle pSource,
    BAPE_BufferGroupHandle pDest
    );

/***************************************************************************
Summary:
Buffer Group - remove all linked output buffer groups
***************************************************************************/
void BAPE_BufferGroup_UnLinkAllOutputs(
    BAPE_BufferGroupHandle pSource
    );

/***************************************************************************
Summary:
Buffer Group Read - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_Read(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Buffer Group Read - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_Read_isr(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Buffer Group Read Complete
***************************************************************************/
BERR_Code BAPE_BufferGroup_ReadComplete(
    BAPE_BufferGroupHandle handle,
    size_t size
    );

/***************************************************************************
Summary:
Buffer Group Read Complete
***************************************************************************/
BERR_Code BAPE_BufferGroup_ReadComplete_isr(
    BAPE_BufferGroupHandle handle,
    size_t size
    );

/***************************************************************************
Summary:
Get Buffer Group descriptors for writing - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_GetWriteBuffers(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Get Buffer Group descriptors for writing - non destructive
***************************************************************************/
BERR_Code BAPE_BufferGroup_GetWriteBuffers_isr(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferDescriptor * pGroupDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Buffer Group Write
***************************************************************************/
BERR_Code BAPE_BufferGroup_WriteComplete(
    BAPE_BufferGroupHandle handle,
    size_t size
    );

/***************************************************************************
Summary:
Buffer Group Write
***************************************************************************/
BERR_Code BAPE_BufferGroup_WriteComplete_isr(
    BAPE_BufferGroupHandle handle,
    size_t size
    );

/***************************************************************************
Summary:
Buffer Group Flush
***************************************************************************/
void BAPE_BufferGroup_Flush(
    BAPE_BufferGroupHandle handle
    );

/***************************************************************************
Summary:
Buffer Group Flush
***************************************************************************/
void BAPE_BufferGroup_Flush_isr(
    BAPE_BufferGroupHandle handle
    );

/***************************************************************************
Summary:
Buffer Group Get Depth
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferDepth(
    BAPE_BufferGroupHandle handle
    );

/***************************************************************************
Summary:
Buffer Group Get Depth
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferDepth_isr(
    BAPE_BufferGroupHandle handle
    );

/***************************************************************************
Summary:
Buffer Group Get Free Space
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferFree(
    BAPE_BufferGroupHandle handle
    );

/***************************************************************************
Summary:
Buffer Group Get Free Space
***************************************************************************/
unsigned BAPE_BufferGroup_GetBufferFree_isr(
    BAPE_BufferGroupHandle handle
    );

/***************************************************************************
Summary:
Buffer Group Enable Consumption
***************************************************************************/
BERR_Code BAPE_BufferGroup_Enable(
    BAPE_BufferGroupHandle handle,
    bool enable
    );

/***************************************************************************
Summary:
Buffer Group Enable Consumption
***************************************************************************/
BERR_Code BAPE_BufferGroup_Enable_isr(
    BAPE_BufferGroupHandle handle,
    bool enable
    );

/***************************************************************************
Summary:
Buffer Group Set Format
***************************************************************************/
BERR_Code BAPE_BufferGroup_SetFormat(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupFormat * pFormat
    );

/***************************************************************************
Summary:
Buffer Group Set Format
***************************************************************************/
BERR_Code BAPE_BufferGroup_SetFormat_isr(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupFormat * pFormat
    );

/***************************************************************************
Summary:
Buffer Group Get Status
***************************************************************************/
void BAPE_BufferGroup_GetStatus_isrsafe(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupStatus * pStatus /* [out] */
    );

/***************************************************************************
Summary:
Buffer Group Interrupts
***************************************************************************/
typedef struct BAPE_BufferGroupInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } dataReady;
} BAPE_BufferGroupInterruptHandlers;

/***************************************************************************
Summary:
Buffer Group Get Interrupt Handlers
***************************************************************************/
void BAPE_BufferGroup_GetInterruptHandlers(
    BAPE_BufferGroupHandle handle,
    BAPE_BufferGroupInterruptHandlers *pInterrupts    /* [out] */
    );

/***************************************************************************
Summary:
Buffer Group Set Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_BufferGroup_SetInterruptHandlers(
    BAPE_BufferGroupHandle handle,
    const BAPE_BufferGroupInterruptHandlers *pInterrupts
    );

#endif /* #ifndef BAPE_BUFFER_H_ */
