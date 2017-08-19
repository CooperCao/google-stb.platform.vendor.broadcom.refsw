/******************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bvee.h"
#include "bvee_channel.h"
#include "bvee_priv.h"
#include "bdsp_raaga.h"
#include "bvee_dsp_utils_priv.h"
#include "bdsp_video_encode_task.h"

BDBG_MODULE(bvee_channel);

BDBG_OBJECT_ID(BVEE_Channel);

#define STRIPE_WIDTH_64     (64)
#define STRIPE_WIDTH_128    (128)

#define DSP_INDEX 0
#define BVEE_DATA_UNIT_START_CODE      0x00000001

#define BDSP_RAAGA_P_FIFO_BASE_OFFSET    0
#define BDSP_RAAGA_P_FIFO_END_OFFSET     4
#define BDSP_RAAGA_P_FIFO_READ_OFFSET    12 /*8*/
#define BDSP_RAAGA_P_FIFO_WRITE_OFFSET   8  /*12*/

#define BVEE_UserData_P_PacketDescriptor_MAX_PER_FIELD BVEE_FW_P_UserData_PacketType_eMax

#define BVEE_P_UserData_PacketType_UNSUPPORTED 0xFFFF

static const uint16_t BVEE_P_UserData_PacketTypeLUT[BUDP_DCCparse_Format_LAST + 1] =
{
BVEE_P_UserData_PacketType_UNSUPPORTED, /* BUDP_DCCparse_Format_Unknown */
BVEE_FW_P_UserData_PacketType_eSCTE_20, /* BUDP_DCCparse_Format_DVS157 */
BVEE_FW_P_UserData_PacketType_eATSC_A53, /* BUDP_DCCparse_Format_ATSC53 */
BVEE_FW_P_UserData_PacketType_eSCTE_21, /* BUDP_DCCparse_Format_DVS053 */
BVEE_P_UserData_PacketType_UNSUPPORTED, /* BUDP_DCCparse_Format_SEI */
BVEE_P_UserData_PacketType_UNSUPPORTED, /* BUDP_DCCparse_Format_SEI2 */
BVEE_P_UserData_PacketType_UNSUPPORTED, /* BUDP_DCCparse_Format_LAST */
};
static const uint32_t BVEE_P_FrameRateLUT[BDSP_VF_P_EncodeFrameRate_eMax] =
{
BDSP_VF_P_EncodeFrameRate_eUnknown,  /* Unknown */
BDSP_VF_P_EncodeFrameRate_e23_97,  /* 23.97 */
BDSP_VF_P_EncodeFrameRate_e24,     /* 24 */
BDSP_VF_P_EncodeFrameRate_e25,     /* 25 */
BDSP_VF_P_EncodeFrameRate_e29_97,  /* 29.97 */
BDSP_VF_P_EncodeFrameRate_e30,     /* 30 */
BDSP_VF_P_EncodeFrameRate_e50,     /* 50 */
BDSP_VF_P_EncodeFrameRate_e59_94,  /* 59.94 */
BDSP_VF_P_EncodeFrameRate_e60,     /* 60 */
BDSP_VF_P_EncodeFrameRate_e14_985, /* 14.985 */
BDSP_VF_P_EncodeFrameRate_e7_493,  /* 7.493 */
BDSP_VF_P_EncodeFrameRate_e10,     /* 10 */
BDSP_VF_P_EncodeFrameRate_e15
};

void BVEE_Channel_GetDefaultOpenSettings(
BVEE_ChannelOpenSettings *pSettings
)
{
BDBG_ASSERT(NULL != pSettings);
BKNI_Memset(pSettings, 0, sizeof(*pSettings));

pSettings->maxQueuedPictures = BVEE_MAX_PICTURE_BUFFERS;
pSettings->codec = BAVC_VideoCompressionStd_eH264;

pSettings->resolution.width = BVEE_DEFAULT_PICT_WIDTH;
pSettings->resolution.height = BVEE_DEFAULT_PICT_HEIGHT;
pSettings->bufferHeap = NULL;
pSettings->enableExternalTrigger = false;

return;
}
static uint8_t BVEE_Channel_P_GetNextItbEntryType(BVEE_ChannelHandle handle, uint32_t readOffset)
{
uint8_t *pSource;
BVEE_P_ITBEntry *pEntry_Cached;

pSource = (uint8_t *)handle->outputbuffer.pITBBufferCached + (readOffset - handle->outputbuffer.uiITBBufferOffset);
pEntry_Cached = (BVEE_P_ITBEntry *)pSource;

/* invalidates cache */
BMMA_FlushCache(handle->outputbuffer.hITBBufferMmaBlock, pEntry_Cached, sizeof(BVEE_P_ITBEntry));

return BVEE_ITB_GET_FIELD(&pEntry_Cached->fields.baseAddress,GENERIC,ENTRY_TYPE);
}
static size_t BVEE_Channel_P_ReadItb(BVEE_ChannelHandle handle, uint32_t baseOffset,
uint32_t readOffset, uint32_t endOffset,
uint32_t depth, uint8_t *pDest, size_t length)
{
uint8_t *pSource;
void *pSource_Cached;

if ( length <= depth )
{

pSource = (uint8_t*)handle->outputbuffer.pITBBufferCached + (readOffset - handle->outputbuffer.uiITBBufferOffset);
pSource_Cached = pSource;

if ( readOffset + length > endOffset )
{
size_t preWrapAmount = endOffset-readOffset;
size_t wrapAmount = length - preWrapAmount;

/* Wraparound */
BMMA_FlushCache(handle->outputbuffer.hITBBufferMmaBlock,pSource_Cached, preWrapAmount);
BKNI_Memcpy(pDest, pSource_Cached, preWrapAmount);

pSource = (uint8_t *)handle->outputbuffer.pITBBufferCached + (baseOffset - handle->outputbuffer.uiITBBufferOffset);
pSource_Cached = pSource;

BMMA_FlushCache(handle->outputbuffer.hITBBufferMmaBlock,pSource_Cached, wrapAmount);
BKNI_Memcpy(pDest+preWrapAmount, pSource_Cached, wrapAmount);
}
else
{
/* No Wrap */
BMMA_FlushCache(handle->outputbuffer.hITBBufferMmaBlock, pSource_Cached, length);
BKNI_Memcpy(pDest, pSource_Cached, length);
}

return length;
}
else
{
return 0;
}
}
void BVEE_Channel_P_ParseItb(BVEE_ChannelHandle handle, BVEE_P_ITBEntry **pCurrent, BVEE_P_ITBEntry **pNext)
{

uint32_t uiITBDepth;
uint32_t uiITBBaseOffset;
uint32_t uiITBEndOffset;
uint32_t uiITBValidOffset;
uint32_t uiShadowReadOffset;
uint32_t uiNextEntryOffset;
BVEE_ChannelOutputDescriptorInfo *psOutputDescDetails = &handle->veeoutput;
BVEE_P_ITBEntry *pITBEntry, *pITBEntryNext;
size_t uiAmountRead;
uint8_t entryType;

*pCurrent = NULL;
*pNext = NULL;

/* ITB Pointers */
uiITBBaseOffset = BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sItbBuffer.ui32BaseAddr);
uiITBEndOffset = BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sItbBuffer.ui32EndAddr);

uiITBValidOffset = BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sItbBuffer.ui32WriteAddr);
if ( uiITBValidOffset >= uiITBEndOffset )
{
uiITBValidOffset = uiITBBaseOffset + ( uiITBValidOffset - uiITBEndOffset );
}

uiShadowReadOffset = psOutputDescDetails->uiITBBufferShadowReadOffset;
for ( ;; )
{
pITBEntry = NULL;
pITBEntryNext = NULL;

if ( uiITBValidOffset >= uiShadowReadOffset )
{
uiITBDepth = uiITBValidOffset - \
uiShadowReadOffset;
}
else
{
uiITBDepth = uiITBEndOffset - uiShadowReadOffset;
uiITBDepth += uiITBValidOffset - uiITBBaseOffset;
}

BDBG_MSG(("ITB Depth: %d bytes (Valid: %08x, Shadow Read: %08x)",
uiITBDepth,
uiITBValidOffset,
uiShadowReadOffset
));

/* Check for odd ITB entries and drop them */
entryType = BVEE_Channel_P_GetNextItbEntryType(handle, uiShadowReadOffset);
if ( entryType != BVEE_ITB_ENTRY_TYPE_BASE_ADDRESS )
{
/* this should never happen. We should be in sync looking for this entry drops should happen looking for the next one */
BDBG_WRN(("Dropping ITB Entry type 0x%02x looking for first", entryType));
uiShadowReadOffset += 16;
if ( uiShadowReadOffset >= uiITBEndOffset )
{
uiShadowReadOffset -= (uiITBEndOffset-uiITBBaseOffset);
}
continue;
}
uiAmountRead = BVEE_Channel_P_ReadItb(handle, uiITBBaseOffset, uiShadowReadOffset, uiITBEndOffset, uiITBDepth,
(uint8_t *)&psOutputDescDetails->itb.current, sizeof(BVEE_P_ITBEntry));
if ( 0 == uiAmountRead )
{
/* We ran out of ITB entries */
BDBG_MSG(("No more ITB Entries"));
psOutputDescDetails->uiITBBufferShadowReadOffset = uiShadowReadOffset;
return;
}

uiITBDepth -= uiAmountRead;
pITBEntry = &psOutputDescDetails->itb.current;


uiNextEntryOffset = uiShadowReadOffset + sizeof(BVEE_P_ITBEntry);
if ( uiNextEntryOffset >= uiITBEndOffset )
{
uiNextEntryOffset -= (uiITBEndOffset-uiITBBaseOffset);
}

while ( uiITBDepth >= sizeof(BVEE_P_ITBEntry) && NULL == pITBEntryNext )
{
/* Check for odd ITB entries and drop them */
entryType = BVEE_Channel_P_GetNextItbEntryType(handle, uiNextEntryOffset);
if ( entryType != BVEE_ITB_ENTRY_TYPE_BASE_ADDRESS )
{
uiNextEntryOffset += 16;
if ( uiNextEntryOffset >= uiITBEndOffset )
{
uiNextEntryOffset -= (uiITBEndOffset-uiITBBaseOffset);
}
uiITBDepth -= 16;
continue;
}

/* Found a base address entry.  Read the next entry. */
uiAmountRead = BVEE_Channel_P_ReadItb(handle, uiITBBaseOffset, uiNextEntryOffset, uiITBEndOffset, uiITBDepth,
(uint8_t *)&psOutputDescDetails->itb.next, sizeof(BVEE_P_ITBEntry));
if ( 0 == uiAmountRead )
{
/* We ran out of ITB entries */
BDBG_MSG(("Next ITB entry unavailable"));
return;
}

uiITBDepth -= uiAmountRead;
pITBEntryNext = &psOutputDescDetails->itb.next;
}

/* Figure out how much CDB data we have for the current Frame */
if ( NULL != pITBEntryNext )
{
/* Goto next frame's ITB Entry */
if ( BVEE_ITB_GET_FIELD(&pITBEntryNext->fields.baseAddress, BASE_ADDRESS, CDB_ADDRESS) ==
psOutputDescDetails->uiCDBBufferShadowReadOffset )
{
/* We have a next entry, and we've finished with the
* current entry, so move to the next entry
*/
uiShadowReadOffset = uiNextEntryOffset;
psOutputDescDetails->uiITBBufferShadowReadOffset = uiNextEntryOffset;
BDBG_MSG(("Goto Next Entry"));
continue;
}

*pCurrent = pITBEntry;
*pNext = pITBEntryNext;
return;
}
else
{
/* We ran out of ITB entries */
BDBG_MSG(("Next ITB entry unavailable"));
return;
}
}
}
BERR_Code BVEE_Channel_P_AllocatePictParamBuffer(BVEE_ChannelHandle handle)
{
unsigned int i=0;
BMMA_Heap_Handle  mmaHandle = NULL;
uint32_t uiTempSize=0;

if (handle->opensettings.bufferHeap!= NULL)
{
BDBG_MSG(("%s Frame buffer from %p (Opensettings)",BSTD_FUNCTION, (void*)handle->opensettings.bufferHeap));
mmaHandle = handle->opensettings.bufferHeap;
}
else
{
BDBG_MSG(("%s Frame buffer from default", BSTD_FUNCTION));
mmaHandle = handle->devicehandle->mmahandle;
}
handle->capturepicture = BKNI_Malloc(sizeof(BVEE_CapBufferMemory)* handle->opensettings.maxQueuedPictures);
for(i=0;i<handle->opensettings.maxQueuedPictures;i++)
{
handle->capturepicture[i].hPpBufferMmaBlock = BMMA_Alloc(mmaHandle, sizeof(BVENC_VF_sPicParamBuff), 32, 0);
if(NULL == handle->capturepicture[i].hPpBufferMmaBlock)
{
return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
}
handle->capturepicture[i].uiPpBufferOffset = BMMA_LockOffset(handle->capturepicture[i].hPpBufferMmaBlock);
handle->capturepicture[i].pPpBufferCached  = BMMA_Lock(handle->capturepicture[i].hPpBufferMmaBlock);

handle->capturepicture[i].bValid = false;
}
if((handle->opensettings.resolution.height>480)&&(handle->opensettings.resolution.width>640))
{
/* reference buffer size for HD resolution */
uiTempSize = (BVEE_H264_ENCODE_REF_LUMAFRAME_BUF_SIZE_HD+BVEE_H264_ENCODE_REF_CHROMAFRAME_BUF_SIZE_HD)
*BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL;
BDBG_MSG(("Allocating HD buffer size %d",uiTempSize));
}
else
{
/* reference buffer size */
uiTempSize = (BVEE_H264_ENCODE_REF_LUMAFRAME_BUF_SIZE+BVEE_H264_ENCODE_REF_CHROMAFRAME_BUF_SIZE)
*BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL;
BDBG_MSG(("Allocating SD buffer size %d",uiTempSize));
}

/* Video FW requires the buffers to be 1024 byte alligned for DMA */
handle->hRefFrameBaseAddrMmaBlock = BMMA_Alloc(mmaHandle,uiTempSize, 1024, 0);
if( NULL == handle->hRefFrameBaseAddrMmaBlock)
{
BDBG_ERR(("%s: Unable to Allocate memory for Video encoder reference buffer!",BSTD_FUNCTION));
return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
}
handle->uiRefFrameBaseAddrOffset = (uint32_t)BMMA_LockOffset(handle->hRefFrameBaseAddrMmaBlock);

/* Reset shadow pointers */
handle->veeoutput.uiDescriptorReadOffset = 0;
handle->veeoutput.uiDescriptorWriteOffset = 0;
handle->veeoutput.uiMetadataDescriptorReadOffset = 0;
handle->veeoutput.uiMetadataDescriptorWriteOffset = 0;

/* Allocate Video buffer descriptors */
handle->veeoutput.hDescriptorsMmaBlock = BMMA_Alloc(mmaHandle,sizeof(BAVC_VideoBufferDescriptor)*BVEE_MAX_VIDEODESCRIPTORS, 32,0);
if ( NULL == handle->veeoutput.hDescriptorsMmaBlock )
{
BDBG_ERR(("Error allocating encoder descriptors"));
return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
}

handle->veeoutput.pstDescriptorsCached = (BAVC_VideoBufferDescriptor *)BMMA_Lock(handle->veeoutput.hDescriptorsMmaBlock);

/* Allocate Video Metadata descriptors */
handle->veeoutput.hMetadataMmaBlock = BMMA_Alloc(mmaHandle, sizeof(BAVC_VideoMetadataDescriptor)*BVEE_MAX_METADATADESCRIPTORS, 32,0);
if ( NULL == handle->veeoutput.hMetadataMmaBlock )
{
BDBG_ERR(("Error allocating Meta data buffers"));
return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
}
handle->veeoutput.pstMetadataCached = (BAVC_VideoMetadataDescriptor *)BMMA_Lock(handle->veeoutput.hMetadataMmaBlock);

/* Allocate user data queue */
if ( 0 != BVEE_FW_P_USERDATA_QUEUE_LENGTH )
{
handle->userdata.savebuffer_cc.uiDescriptorBufferSize = BVEE_FW_P_USERDATA_QUEUE_LENGTH * BVEE_FW_P_UserData_PacketDescriptor_MAX_LENGTH * BVEE_UserData_P_PacketDescriptor_MAX_PER_FIELD;

handle->userdata.savebuffer_cc.hDescriptorBufferMmaBlock = BMMA_Alloc(
mmaHandle,
handle->userdata.savebuffer_cc.uiDescriptorBufferSize,
32,
0
);

if ( NULL == handle->userdata.savebuffer_cc.hDescriptorBufferMmaBlock)
{
BDBG_ERR(("Error allocating savebuffer_cc user data queue"));
return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
}

/* Convert to cached */
handle->userdata.savebuffer_cc.pDescriptorBufferCached = BMMA_Lock(handle->userdata.savebuffer_cc.hDescriptorBufferMmaBlock);

handle->userdata.savebuffer_cc.uiNumDescriptors = 0;
handle->userdata.savebuffer_cc.bIsFree = true;
}

return BERR_SUCCESS;
}

BERR_Code BVEE_Channel_P_DeAllocatePictParamBuffer(BVEE_ChannelHandle handle)
{
BERR_Code errcode = BERR_SUCCESS;
unsigned int i=0;

BMMA_Unlock(handle->veeoutput.hMetadataMmaBlock,(void*)handle->veeoutput.pstMetadataCached);
BMMA_Free(handle->veeoutput.hMetadataMmaBlock);

BMMA_Unlock(handle->veeoutput.hDescriptorsMmaBlock,(void*)handle->veeoutput.pstDescriptorsCached);
BMMA_Free(handle->veeoutput.hDescriptorsMmaBlock);

for(i=0;i<handle->opensettings.maxQueuedPictures;i++)
{
BMMA_UnlockOffset(handle->capturepicture[i].hPpBufferMmaBlock,handle->capturepicture[i].uiPpBufferOffset);
BMMA_Unlock(handle->capturepicture[i].hPpBufferMmaBlock,handle->capturepicture[i].pPpBufferCached);
BMMA_Free(handle->capturepicture[i].hPpBufferMmaBlock);
}

BMMA_UnlockOffset(handle->hRefFrameBaseAddrMmaBlock, handle->uiRefFrameBaseAddrOffset);
BMMA_Free(handle->hRefFrameBaseAddrMmaBlock);

BKNI_Free(handle->capturepicture);

BMMA_Unlock(handle->userdata.savebuffer_cc.hDescriptorBufferMmaBlock,(void*)handle->userdata.savebuffer_cc.pDescriptorBufferCached);
BMMA_Free(handle->userdata.savebuffer_cc.hDescriptorBufferMmaBlock);
handle->userdata.savebuffer_cc.pDescriptorBufferCached = NULL;

return errcode;
}
BERR_Code BVEE_Channel_P_AllocateOutputBuffers(BVEE_ChannelHandle handle)
{
BMMA_Heap_Handle mmaHandle = NULL;

if (handle->opensettings.bufferHeap!= NULL)
{
BDBG_MSG(("%s CDB ITB buffers from %p (Opensettings)",BSTD_FUNCTION, (void*)handle->opensettings.bufferHeap));
mmaHandle = handle->opensettings.bufferHeap;
}
else
{
BDBG_MSG(("%s CDB ITB buffers from default", BSTD_FUNCTION));
mmaHandle = handle->devicehandle->mmahandle;
}


/* Allocate CDB */
handle->outputbuffer.hCDBBufferMmaBlock = BMMA_Alloc(mmaHandle, (uint32_t)BVEE_H264_ENCODE_OUTPUT_CDB_SIZE, 16,0);
if ( NULL == handle->outputbuffer.hCDBBufferMmaBlock )
{
BDBG_ERR(("Error allocating CDB buffer"));
return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
}

/* Convert to cached */
handle->outputbuffer.pCDBBufferCached = BMMA_Lock(handle->outputbuffer.hCDBBufferMmaBlock);
/* Get the HW address */
handle->outputbuffer.uiCDBBufferOffset = (uint32_t)BMMA_LockOffset(handle->outputbuffer.hCDBBufferMmaBlock);

/* Allocate ITB */
handle->outputbuffer.hITBBufferMmaBlock = BMMA_Alloc(mmaHandle, (uint32_t)BVEE_H264_ENCODE_OUTPUT_ITB_SIZE, 16,0);
if ( NULL == handle->outputbuffer.hITBBufferMmaBlock )
{
BDBG_ERR(("Error allocating ITB buffer"));
return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
}

/* Convert to cached */
handle->outputbuffer.pITBBufferCached = BMMA_Lock(handle->outputbuffer.hITBBufferMmaBlock);
/* Get the HW address */
handle->outputbuffer.uiITBBufferOffset = (uint32_t)BMMA_LockOffset(handle->outputbuffer.hITBBufferMmaBlock);


return BERR_SUCCESS;
}

BERR_Code BVEE_Channel_P_DeAllocateOutputBuffers(BVEE_ChannelHandle handle)
{

BMMA_UnlockOffset(handle->outputbuffer.hCDBBufferMmaBlock,handle->outputbuffer.uiCDBBufferOffset);
BMMA_Unlock(handle->outputbuffer.hCDBBufferMmaBlock,(void*)handle->outputbuffer.pCDBBufferCached);
BMMA_Free(handle->outputbuffer.hCDBBufferMmaBlock);
handle->outputbuffer.pCDBBufferCached = NULL;

BMMA_UnlockOffset(handle->outputbuffer.hITBBufferMmaBlock,handle->outputbuffer.uiITBBufferOffset);
BMMA_Unlock(handle->outputbuffer.hITBBufferMmaBlock,(void*)handle->outputbuffer.pITBBufferCached);
BMMA_Free(handle->outputbuffer.hITBBufferMmaBlock);
handle->outputbuffer.pITBBufferCached = NULL;

return BERR_SUCCESS;
}

BERR_Code BVEE_Channel_P_CreateQueue(BVEE_ChannelHandle handle)
{
BERR_Code errCode;
BDSP_QueueCreateSettings queueSettings;

BDSP_Queue_GetDefaultSettings(handle->devicehandle->dspContext, &queueSettings);
queueSettings.dataType = BDSP_DataType_eRdbCdb;
queueSettings.numBuffers = 1;
queueSettings.bufferInfo[0].bufferSize = BVEE_H264_ENCODE_OUTPUT_CDB_SIZE;
queueSettings.bufferInfo[0].buffer.hBlock = handle->outputbuffer.hCDBBufferMmaBlock;
queueSettings.bufferInfo[0].buffer.pAddr  = handle->outputbuffer.pCDBBufferCached;
queueSettings.bufferInfo[0].buffer.offset = handle->outputbuffer.uiCDBBufferOffset;

errCode = BDSP_Queue_Create(
handle->devicehandle->dspContext,
DSP_INDEX,
&queueSettings,
&handle->cdbqueue
);

if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("errCode: CDB Queue Create failed"));
BDBG_ASSERT( !errCode );
}
/* populate CDB buffer pointer addresses */
errCode = BDSP_Queue_GetBufferAddr(handle->cdbqueue, queueSettings.numBuffers, &(handle->outputbuffer.sCdbBuffer));
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("errCode: CDB Queue Address Retreival failed"));
BDBG_ASSERT( !errCode );
}

BDSP_Queue_GetDefaultSettings(handle->devicehandle->dspContext, &queueSettings);
queueSettings.dataType = BDSP_DataType_eRdbItb;
queueSettings.numBuffers = 1;
queueSettings.bufferInfo[0].bufferSize = BVEE_H264_ENCODE_OUTPUT_ITB_SIZE;
queueSettings.bufferInfo[0].buffer.hBlock = handle->outputbuffer.hITBBufferMmaBlock;
queueSettings.bufferInfo[0].buffer.pAddr  = handle->outputbuffer.pITBBufferCached;
queueSettings.bufferInfo[0].buffer.offset = handle->outputbuffer.uiITBBufferOffset;

errCode = BDSP_Queue_Create(
handle->devicehandle->dspContext,
DSP_INDEX,
&queueSettings,
&handle->itbqueue
);
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("errCode: ITB Queue Create failed"));
BDBG_ASSERT( !errCode );
}
/* populate ITB buffer pointer addresses */
errCode = BDSP_Queue_GetBufferAddr(handle->itbqueue, queueSettings.numBuffers, &(handle->outputbuffer.sItbBuffer));
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("errCode: ITB Queue Address Retreival failed"));
BDBG_ASSERT( !errCode );
}

return errCode;
}

BERR_Code BVEE_Channel_P_DestroyQueue(BVEE_ChannelHandle handle)
{
if (handle->cdbqueue)
{
BDSP_Queue_Destroy(handle->cdbqueue);
}

if (handle->itbqueue)
{
BDSP_Queue_Destroy(handle->itbqueue);
}

return BERR_SUCCESS;
}

static void BVEE_Channel_P_LinkStages(BVEE_ChannelHandle handle)
{
BERR_Code errCode;
unsigned OutputIndex;

if ( handle->hPrimaryStage )
{
errCode = BDSP_Stage_AddQueueOutput(
handle->hPrimaryStage,
handle->cdbqueue,
&OutputIndex
);
if ( errCode )
{
BERR_TRACE(errCode);
return;
}

errCode = BDSP_Stage_AddQueueOutput(
handle->hPrimaryStage,
handle->itbqueue,
&OutputIndex
);
if ( errCode )
{
BERR_TRACE(errCode);
return;
}

}
}

static void BVEE_Channel_P_UnlinkStages(BVEE_ChannelHandle handle)
{
if ( handle->hPrimaryStage )
{
BDSP_Stage_RemoveAllOutputs(handle->hPrimaryStage);
}
}
static BERR_Code BVEE_Channel_P_ValidateSettings(
BVEE_ChannelHandle handle,
const BVEE_ChannelStartSettings *pSettings,
BVEE_ChannelStartSettings *pOutputSettings
)
{
BDSP_Algorithm videoAlgorithm;
BDSP_AlgorithmInfo algoInfo;
BERR_Code errCode;

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pSettings);
BDBG_ASSERT(NULL != pOutputSettings);

/* Start by copying the existing settings */
BKNI_Memcpy(pOutputSettings, pSettings, sizeof(BVEE_ChannelStartSettings));

/* Check for valid input */
if ( NULL == pSettings->pContextMap )
{
BDBG_ERR(("Must specify an input to encode"));
return BERR_TRACE(BERR_INVALID_PARAMETER);
}

/* Store local copy of the RAVE context map in case it goes out of scope after start. */
if ( pSettings->pContextMap )
{
BKNI_Memcpy(&handle->contextmap, pSettings->pContextMap, sizeof(BAVC_XptContextMap));
pOutputSettings->pContextMap = &handle->contextmap;
}


videoAlgorithm = BVEE_P_GetCodecVideoType(pSettings->codec);

/* Check for FW availability */
errCode = BDSP_GetAlgorithmInfo(handle->devicehandle->dspHandle, videoAlgorithm, &algoInfo);
if ( errCode )
{
return BERR_TRACE(errCode);
}
else if ( !algoInfo.supported )
{
BDBG_ERR(("Codec %s (%u) DSP algorithm %s (%u) is not supported.",
BVEE_P_GetCodecName(pSettings->codec), pSettings->codec,
algoInfo.pName, videoAlgorithm));
return BERR_TRACE(BERR_NOT_SUPPORTED);
}
if((pSettings->ui32EncodPicHeight%16)||(pSettings->ui32EncodPicWidth%16))
{
BDBG_ERR(("Supports 16 pixel alignment only"));
return BERR_TRACE(BERR_NOT_SUPPORTED);
}

return BERR_SUCCESS;
}

BERR_Code BVEE_Channel_Open(
BVEE_Handle deviceHandle,
unsigned index,
const BVEE_ChannelOpenSettings *pSettings,
BVEE_ChannelHandle *pHandle
)
{
BERR_Code errCode;
BVEE_ChannelOpenSettings defaults;
BVEE_ChannelHandle handle;
BDSP_TaskCreateSettings dspSettings;
BDSP_StageCreateSettings stageCreateSettings;

BDBG_OBJECT_ASSERT(deviceHandle, BVEE_Device);

if ( NULL == pSettings )
{
BDBG_WRN (("pSettings is NULL. Using Defaults with memPicHandle as NULL"));
BVEE_Channel_GetDefaultOpenSettings(&defaults);
pSettings = &defaults;
}

if ( index >= BVEE_MAX_CHANNELS )
{
BDBG_ERR(("This chip only supports %u channels. Cannot open channel %u", BVEE_MAX_CHANNELS, index));
return BERR_TRACE(BERR_INVALID_PARAMETER);
}

if ( deviceHandle->channels[index] )
{
BDBG_ERR(("Channel %d already open", index));
return BERR_TRACE(BERR_INVALID_PARAMETER);
}

handle = BKNI_Malloc(sizeof(BVEE_Channel));
if ( NULL == handle )
{
return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
}
BKNI_Memset(handle, 0, sizeof(BVEE_Channel));

handle->devicehandle = deviceHandle;
handle->index = index;
BKNI_Snprintf(handle->name, sizeof(handle->name), "Channel %u", index);
handle->state = BVEE_ChannelState_eStopped;
BKNI_Memcpy(&handle->opensettings, pSettings, sizeof(handle->opensettings));
BDBG_OBJECT_SET(handle, BVEE_Channel);

BDSP_Task_GetDefaultCreateSettings(deviceHandle->dspContext, &dspSettings);

dspSettings.numSrc = 1; /* one input */
dspSettings.numDst = 1; /* one output */
errCode = BDSP_Task_Create(deviceHandle->dspContext, &dspSettings, &handle->task);
if ( errCode )
{
errCode = BERR_TRACE(errCode);
goto err_task_create;
}

/* Allocate Memory for descriptor Buffers*/
errCode = BVEE_Channel_P_AllocatePictParamBuffer (handle);
if ( errCode )
{
errCode = BERR_TRACE(errCode);
goto err_alloc_framebuf;
}

BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eVideoEncode, &stageCreateSettings);

errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hPrimaryStage);
if ( errCode )
{
errCode = BERR_TRACE(errCode);
goto err_stage;
}

#if BDSP_ENCODER_ACCELERATOR_SUPPORT
errCode = BDSP_Stage_SetAlgorithm(handle->hPrimaryStage, BDSP_Algorithm_eX264Encode);
#else
errCode = BDSP_Stage_SetAlgorithm(handle->hPrimaryStage, BDSP_Algorithm_eH264Encode);
#endif
if ( errCode )
{
errCode = BERR_TRACE(errCode);
goto err_stage;
}

/* Allocate Memory for output Buffers*/
errCode = BVEE_Channel_P_AllocateOutputBuffers(handle);
if ( errCode )
{
errCode = BERR_TRACE(errCode);
goto err_alloc_framebuf;
}

errCode = BVEE_Channel_P_CreateQueue(handle);
if ( errCode )
{
errCode = BERR_TRACE(errCode);
goto err_queue;
}

/* Success */
*pHandle = handle;
deviceHandle->channels[index] = handle;

return BERR_SUCCESS;

err_queue:
err_stage:
err_task_create:
err_alloc_framebuf:
BVEE_Channel_Close (handle);

return errCode;
}

BERR_Code BVEE_Channel_Close(BVEE_ChannelHandle handle)
{
BERR_Code errCode = BERR_SUCCESS;

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

if ( handle->state != BVEE_ChannelState_eStopped )
{
BDBG_WRN(("Implicitly stopping channel %u on shutdown.", handle->index));
BVEE_Channel_Stop(handle);
}

errCode = BVEE_Channel_P_DeAllocatePictParamBuffer(handle);
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("BVEE_Channel_P_DeAllocatePictParamBuffer:Failure"));
}

errCode = BVEE_Channel_P_DeAllocateOutputBuffers(handle);
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("BVEE_Channel_P_DeAllocateOutputBuffers:Failure"));
}

errCode = BVEE_Channel_P_DestroyQueue(handle);
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("BVEE_Channel_P_DestroyQueue:Failure"));
}

if (handle->hPrimaryStage)
{
BDSP_Stage_RemoveAllOutputs(handle->hPrimaryStage);
}

/* Cleanup */
handle->devicehandle->channels[handle->index] = NULL;

if ( handle->task )
{
BDSP_Task_Destroy(handle->task);
}

if ( handle->hPrimaryStage )
{
BDSP_Stage_Destroy(handle->hPrimaryStage);
}

BDBG_OBJECT_DESTROY(handle, BVEE_Channel);
BKNI_Free(handle);
return errCode;
}

void BVEE_Channel_GetDefaultStartSettings(BVEE_ChannelStartSettings *pSettings)
{
BKNI_Memset(pSettings, 0, sizeof(*pSettings));
pSettings->codec = BAVC_VideoCompressionStd_eH264;
pSettings->pContextMap = NULL;
pSettings->nonRealTime = false;
pSettings->frameRate = BAVC_FrameRateCode_e30;
pSettings->eAspectRatio = BVEE_AspectRatio_e4_3;
/*Using graphics surface handle as output of BVN.
Do we support planar format for it?*/
pSettings->pxlformat = BAVC_YCbCrType_e4_2_2;
pSettings->sendMetadata = false;
pSettings->sendEos = false;
/*Default dsp task user config*/
pSettings->ui32TargetBitRate = 400000;
pSettings->bDblkEnable = false;
pSettings->bSubPelMvEnable = false;
pSettings->bVarModeOptEnable = false;
pSettings->eGopStruct = BVEE_VideoGopStruct_eIP;
pSettings->eProfileIDC = BVEE_VideoH264Profile_eBaseline;
pSettings->ui32LevelIdc = 40;
pSettings->ui32IntraPeriod = 30;
pSettings->ui32IDRPeriod = 30;
pSettings->bRateControlEnable = 1;
pSettings->ui32EncodPicHeight = 240;
pSettings->ui32EncodPicWidth = 320;
pSettings->stcIndx = 0;
pSettings->bSendCC= false;
pSettings->ccPacketType = BVEE_FW_P_UserData_PacketType_eATSC_A53;

pSettings->bDblkEnable = false;
pSettings->eProfileIDC = BVEE_VideoH264Profile_eBaseline;

#if BDSP_ENCODER_ACCELERATOR_SUPPORT
pSettings->eProfileIDC = BDSP_Raaga_VideoH264Profile_eMain;
#endif

return;
}

void BVEE_Channel_GetStartSettings(BVEE_ChannelHandle handle, BVEE_ChannelStartSettings *pSettings)
{
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pSettings);

*pSettings = handle->startsettings;

return;
}

static BERR_Code BVEE_Channel_P_Start(
BVEE_ChannelHandle handle
)
{
BERR_Code errCode;
unsigned i=0;
uint32_t offset,ui32UsedSize,ui32LumaStripeHeight,ui32ChromaStripeHeight,ui32LumaBufferSize,ui32ChromaBufferSize;
size_t cdbLength;
BDSP_sVEncoderIPConfig *psVencCfg;
const BVEE_ChannelStartSettings *pSettings;
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
BDSP_Raaga_VideoBX264UserConfig userConfig;
#else
BDSP_Raaga_VideoBH264UserConfig userConfig;
#endif
BDSP_TaskStartSettings taskStartSettings;
uint8_t *pTemp;

BDBG_MSG(("BVEE_Channel_P_Start(%p) [index %u]", (void*)handle, handle->index));

pSettings = &handle->startsettings;

BDSP_Queue_Flush(handle->cdbqueue);
BDSP_Queue_Flush(handle->itbqueue);

handle->veeoutput.uiCDBBufferShadowReadOffset = BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sCdbBuffer.ui32BaseAddr);
handle->veeoutput.uiITBBufferShadowReadOffset = BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sItbBuffer.ui32BaseAddr);
offset = BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sCdbBuffer.ui32BaseAddr);

/*Convert CDB_Base offset to Cached address*/
pTemp = (uint8_t *)handle->outputbuffer.pCDBBufferCached + (offset - handle->outputbuffer.uiCDBBufferOffset);
handle->veeoutput.pCdbBaseCached = pTemp;


/* Flush the CDB data from the cache prior to the HW filling it */
cdbLength = BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sCdbBuffer.ui32EndAddr)-offset;
BMMA_FlushCache(handle->outputbuffer.hCDBBufferMmaBlock, handle->veeoutput.pCdbBaseCached, cdbLength);

errCode = BDSP_Stage_SetAlgorithm(handle->hPrimaryStage, BVEE_P_GetCodecVideoType(handle->startsettings.codec));
if ( errCode )
{
errCode = BERR_TRACE(errCode);
goto err_cdb_offset;
}

BVEE_Channel_P_LinkStages(handle);

/* Setup Task Parameters */
BDSP_Task_GetDefaultStartSettings(handle->task, &taskStartSettings);

taskStartSettings.primaryStage = handle->hPrimaryStage;
if (handle->startsettings.nonRealTime)
{
taskStartSettings.realtimeMode = BDSP_TaskRealtimeMode_eNonRealTime;
BDBG_MSG(("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++NRT mode enabled"));
}
else
{
BDBG_MSG(("------------------------------------------------------------------NRT mode NOT enabled"));
}
taskStartSettings.psVEncoderIPConfig =(BDSP_sVEncoderIPConfig*)BKNI_Malloc(sizeof(BDSP_sVEncoderIPConfig));
if (NULL == taskStartSettings.psVEncoderIPConfig)
{
return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
}
BKNI_Memset(taskStartSettings.psVEncoderIPConfig, 0, sizeof(BDSP_sVEncoderIPConfig));

psVencCfg = taskStartSettings.psVEncoderIPConfig;

psVencCfg->sEncoderParams.eEncodeFrameRate = BVEE_P_FrameRateLUT[pSettings->frameRate];
psVencCfg->sEncoderParams.ui32Frames2Accum = 1; /*handle->opensettings.maxQueuedPictures;TBD:*/
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
psVencCfg->sEncoderParams.IsGoBitInterruptEnabled = 0;
#else
psVencCfg->sEncoderParams.IsGoBitInterruptEnabled = 1;
#endif
if (handle->startsettings.nonRealTime)
{
psVencCfg->sEncoderParams.IsNrtModeEnabled = 1;
}
else
{
psVencCfg->sEncoderParams.IsNrtModeEnabled = 0;
}
psVencCfg->sEncoderParams.ui32StcAddr = BVEE_CHP_GET_STC_ADDR(pSettings->stcIndx);
psVencCfg->sEncoderParams.ui32StcAddr_hi = BVEE_CHP_GET_STC_ADDR_HI(pSettings->stcIndx);

psVencCfg->MaxFrameWidth = handle->opensettings.resolution.width;
psVencCfg->MaxFrameHeight = handle->opensettings.resolution.height;

for(i=0;i<BDSP_FWMAX_VIDEO_BUFF_AVAIL;i++)
{
psVencCfg->sPPBs[i].ui32DramBufferAddress = handle->capturepicture[i].uiPpBufferOffset;
psVencCfg->sPPBs[i].ui32BufferSizeInBytes = sizeof(BVENC_VF_sPicParamBuff);
}

if((handle->startsettings.ui32EncodPicHeight>480)&&(handle->startsettings.ui32EncodPicWidth>640))
{  /*Luma,Chroma settings for HD resolution*/
ui32LumaStripeHeight = BVEE_H264_ENCODE_REF_LUMASTRIPE_HEIGHT_HD;
ui32ChromaStripeHeight = BVEE_H264_ENCODE_REF_CHROMASTRIPE_HEIGHT_HD;
ui32LumaBufferSize = BVEE_H264_ENCODE_REF_LUMAFRAME_BUF_SIZE_HD;
ui32ChromaBufferSize = BVEE_H264_ENCODE_REF_CHROMAFRAME_BUF_SIZE_HD;
if((handle->startsettings.bDblkEnable)||
(handle->startsettings.bSubPelMvEnable))
{
BDBG_ERR((" %s Band width not enough to support HD encoding", BSTD_FUNCTION));
}
}
else
{  /*Luma,Chroma settings for Tablet resolution*/
ui32LumaStripeHeight = BVEE_H264_ENCODE_REF_LUMASTRIPE_HEIGHT;
ui32ChromaStripeHeight = BVEE_H264_ENCODE_REF_CHROMASTRIPE_HEIGHT;
ui32LumaBufferSize = BVEE_H264_ENCODE_REF_LUMAFRAME_BUF_SIZE;
ui32ChromaBufferSize = BVEE_H264_ENCODE_REF_CHROMAFRAME_BUF_SIZE;
}
ui32UsedSize = 0;
psVencCfg->sReferenceBuffParams.ui32ChromaStripeHeight = ui32ChromaStripeHeight;
psVencCfg->sReferenceBuffParams.ui32LumaStripeHeight = ui32LumaStripeHeight;
psVencCfg->sReferenceBuffParams.ui32NumBuffAvl = BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL;
BDBG_MSG((" Y:%d Cr:%d Ysize:%d Crsize:%d",  psVencCfg->sReferenceBuffParams.ui32LumaStripeHeight,
psVencCfg->sReferenceBuffParams.ui32ChromaStripeHeight, ui32LumaBufferSize,  ui32ChromaBufferSize));
for(i=0; i<BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL ; i++)
{
psVencCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffLuma.ui32DramBufferAddress
= (uint32_t)(handle->uiRefFrameBaseAddrOffset + ui32UsedSize);
psVencCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffLuma.ui32BufferSizeInBytes
= ui32LumaBufferSize;
ui32UsedSize += ui32LumaBufferSize;

psVencCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffChroma.ui32DramBufferAddress
= (uint32_t)(handle->uiRefFrameBaseAddrOffset + ui32UsedSize);
psVencCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffChroma.ui32BufferSizeInBytes
= ui32ChromaBufferSize;
ui32UsedSize += ui32ChromaBufferSize;
}
/*copy the external interrupt configuration to task create settings*/
BKNI_Memcpy(&taskStartSettings.extInterruptConfig, &pSettings->extIntCfg, sizeof(BDSP_ExtInterruptConfig));

/* Set any user config param before starting the task */
errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &userConfig, sizeof(userConfig));
if(errCode != BERR_SUCCESS)
{
BERR_TRACE(errCode);
}

userConfig.ui32FrameRate = BVEE_P_FrameRateLUT[handle->startsettings.frameRate];
userConfig.ui32TargetBitRate = handle->startsettings.ui32TargetBitRate; /*400000*/
userConfig.eDblkEnable = handle->startsettings.bDblkEnable;
userConfig.eSubPelEnable = handle->startsettings.bSubPelMvEnable;
userConfig.eVarModeDecsOptEnable = handle->startsettings.bVarModeOptEnable;
userConfig.eGopStruct = handle->startsettings.eGopStruct; /*IP*/
userConfig.eProfileIDC = handle->startsettings.eProfileIDC; /*66*/
userConfig.ui32LevelIdc = handle->startsettings.ui32LevelIdc; /*40*/
userConfig.ui32IntraPeriod = handle->startsettings.ui32IntraPeriod; /*30*/
userConfig.ui32IDRPeriod = handle->startsettings.ui32IDRPeriod; /*30*/
userConfig.eRateControlEnable = handle->startsettings.bRateControlEnable; /*1*/
userConfig.ui32EncodPicHeight = handle->startsettings.ui32EncodPicHeight; /*240*/
userConfig.ui32EncodPicWidth = handle->startsettings.ui32EncodPicWidth; /*320*/
userConfig.ui32End2EndDelay = handle->startsettings.ui32End2EndDelay;
userConfig.eSendCC = handle->startsettings.bSendCC;
errCode = BDSP_Stage_SetSettings(handle->hPrimaryStage, &userConfig, sizeof(userConfig));
if(errCode != BERR_SUCCESS)
{
BERR_TRACE(errCode);
goto err_stages;
}
/*
errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &userConfig, sizeof(userConfig));

BDBG_ERR(("userConfig.eDblkEnable:%d",userConfig.eDblkEnable));
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
BDBG_ERR(("userConfig.encodingMode:%d",userConfig.encodingMode));
BDBG_ERR(("userConfig.InputYUVFormat:%d",userConfig.InputYUVFormat));
#endif
BDBG_ERR(("userConfig.eProfileIDC:%d",userConfig.eProfileIDC));

BDBG_ERR((" userConfig.sRCConfig.RCMaxQP[0]:%d", userConfig.sRCConfig.RCMaxQP[0]));
BDBG_ERR((" userConfig.sRCConfig.RCMinQP[0]:%d", userConfig.sRCConfig.RCMinQP[0]));
BDBG_ERR(("userConfig.ui32IntraPeriod:%d",userConfig.ui32IntraPeriod));
BDBG_ERR(("userConfig.sRCConfig.SeinitialQp:%d",userConfig.sRCConfig.SeinitialQp));
BDBG_ERR(("strpe width :%d ui32DDR3Width :%d", psVencCfg->StripeWidth, ui32DDR3Width ));
BDBG_ERR(("-------------------------------------------------userConfig.eSendCC:%d", userConfig.eSendCC));*/

errCode = BVEE_Channel_SetInterruptHandlers(handle, &handle->interrupts);
if(errCode != BERR_SUCCESS)
{
BERR_TRACE(errCode);
}

errCode = BDSP_Task_Start(handle->task, &taskStartSettings);

if ( errCode )
{
(void)BERR_TRACE(errCode);
goto err_start_task;
}

handle->state = BVEE_ChannelState_eStarted;

if (taskStartSettings.psVEncoderIPConfig)
BKNI_Free(taskStartSettings.psVEncoderIPConfig);

return BERR_SUCCESS;

err_start_task:
if (taskStartSettings.psVEncoderIPConfig)
BKNI_Free(taskStartSettings.psVEncoderIPConfig);

handle->state = BVEE_ChannelState_eStopped;

err_stages:
BVEE_Channel_P_UnlinkStages(handle);
err_cdb_offset:
return errCode;
}

BERR_Code BVEE_Channel_Start(BVEE_ChannelHandle handle,
const BVEE_ChannelStartSettings *pSettings)
{
BERR_Code errCode;

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pSettings);

BDBG_MSG(("BVEE_Channel_Start(%p) [index %u]", (void*)handle, handle->index));

if ( NULL == handle->devicehandle->dspContext )
{
BDBG_ERR(("DSP Not avaliable"));
return BERR_TRACE(BERR_NOT_SUPPORTED);
}

if ( handle->state != BVEE_ChannelState_eStopped )
{
BDBG_ERR(("Already running, cannot start"));
return BERR_TRACE(BERR_INVALID_PARAMETER);
}

/* Sanity check settings */
errCode = BVEE_Channel_P_ValidateSettings(handle, pSettings, &handle->startsettings);
if ( errCode )
{
return BERR_TRACE(errCode);
}

if(handle->devicehandle->VEEWatchdogFlag == true)
{
/* Create and Destroy the Queues, as Watchdog has Occured */
BVEE_Channel_P_DestroyQueue(handle);
BVEE_Channel_P_CreateQueue(handle);

/* Reset shadow pointers */
handle->veeoutput.uiDescriptorReadOffset = 0;
handle->veeoutput.uiDescriptorWriteOffset = 0;
handle->veeoutput.uiMetadataDescriptorReadOffset = 0;
handle->veeoutput.uiMetadataDescriptorWriteOffset = 0;

/*Reset the VEE Watchdog Flag */
handle->devicehandle->VEEWatchdogFlag = false;
}

errCode = BVEE_Channel_P_Start(handle);
if ( errCode )
{
return BERR_TRACE(errCode);
}

/* CDB/ITB register map initialized now i.e, in BVEE_Channel_P_Start */
handle->bContextValid = 1;

/* Success */
return BERR_SUCCESS;
}

static BERR_Code BVEE_Channel_P_Stop(
BVEE_ChannelHandle handle
)
{
unsigned int i=0;

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

if ( NULL == handle->task )
{
BDBG_MSG(("BVEE_Channel_P_Stop: Channel %u already stopped.", handle->index));
return BERR_SUCCESS;
}

#if BVEE_DISABLE_DSP
#warning Task Start is Disabled!
BDBG_ERR(("NOT STOPPING DSP"));
#else
BDSP_Task_Stop(handle->task);
#endif

BVEE_Channel_P_UnlinkStages(handle);


for(i=0;i<handle->opensettings.maxQueuedPictures;i++)
{
handle->capturepicture[i].bValid = false;
}

return BERR_SUCCESS;
}

void BVEE_Channel_Stop(BVEE_ChannelHandle handle)
{
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

BDBG_MSG(("BVEE_Channel_Stop(%p) [index %u]", (void*)handle, handle->index));

switch ( handle->state )
{
case BVEE_ChannelState_eStopped:
BDBG_WRN(("Channel %u Already Stopped.", handle->index));
return;
case BVEE_ChannelState_ePaused:
/*TBD*/
break;
default:
break;
}

/* Stop the task first */
handle->state = BVEE_ChannelState_eStopped;
handle->startsettings.sendEos = true;

/* Serialize with critical section prior to stopping the task,
guarantees isrs are not updating while we stop (they check the state first) */
BKNI_EnterCriticalSection();
BKNI_LeaveCriticalSection();

BVEE_Channel_P_Stop(handle);
}

void BVEE_Channel_GetInterruptHandlers(
BVEE_ChannelHandle handle,
BVEE_ChannelInterruptHandlers *pInterrupts     /* [out] */
)
{
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

BDBG_ASSERT(NULL != pInterrupts);
*pInterrupts = handle->interrupts;
}

BERR_Code BVEE_Channel_SetInterruptHandlers(
BVEE_ChannelHandle handle,
const BVEE_ChannelInterruptHandlers *pInterrupts
)
{
BERR_Code errCode;
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pInterrupts);

BKNI_EnterCriticalSection();
if (handle->task)
{
BDSP_AudioInterruptHandlers interrupts;

BDSP_AudioTask_GetInterruptHandlers_isr(handle->task, &interrupts);
interrupts.vencDataDiscarded.pCallback_isr = pInterrupts->vencDataDiscarded.pCallback_isr;
interrupts.vencDataDiscarded.pParam1 = pInterrupts->vencDataDiscarded.pParam1;
interrupts.vencDataDiscarded.param2 = pInterrupts->vencDataDiscarded.param2;

errCode = BDSP_AudioTask_SetInterruptHandlers_isr(handle->task, &interrupts);
if ( errCode )
{
BKNI_LeaveCriticalSection();
return BERR_TRACE(errCode);
}
}
handle->interrupts = *pInterrupts;
BKNI_LeaveCriticalSection();
return BERR_SUCCESS;
}

BERR_Code BVEE_Channel_EnqueuePicture_isr(
BVEE_ChannelHandle handle,
const BVEE_PictureDescriptor *pPicture
)
{
BERR_Code errCode=BERR_SUCCESS;
unsigned i=0;
uint32_t BuffYAddr;
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
uint32_t BuffCAddr;
#endif
BVENC_VF_sPicParamBuff *pPpb_Cached = NULL;
BVEE_CapBufferMemory *temp_cappict = NULL;

BDBG_ENTER(BVEE_Channel_EnqueuePicture_isr);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pPicture);

for(i=0;i<handle->opensettings.maxQueuedPictures;i++)
{
if(handle->capturepicture[i].bValid == false)
{
temp_cappict = &handle->capturepicture[i];
temp_cappict->bValid = true;
break;
}
}

if (temp_cappict == NULL)
{
return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
}

if(i == handle->opensettings.maxQueuedPictures)
{
return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
}

/* Save the picture desc details from Nexus */
temp_cappict->hPDescMmaBlock = pPicture->hImageMmaBlock;
temp_cappict->uiPDescOffset = pPicture->offset;
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
temp_cappict->hLumaBlock = pPicture->hLumaBlock;
temp_cappict->ulPictureId = pPicture->ulPictureId;
temp_cappict->ulLumaOffset = pPicture->ulLumaOffset;
#endif

pPpb_Cached = (BVENC_VF_sPicParamBuff*)temp_cappict->pPpBufferCached;
if (pPpb_Cached == NULL)
{
return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
}

BDBG_MSG(("Picture:FrameRate %d\n" "\t\t ui32CurrentPTS %d \n" "\t\t height %d \n" "\t\t width %d \n"
"\t\t polarity %d \n" "\t\t sarHorz %d \n" "\t\t sarVert %d \n" "\t\t repeat %d \n" "\t\t ignore %d \n"
"\t\t bIgnorePicture %d \n" "\t\t bStallStc %d \n",
pPicture->frameRate,pPicture->originalPts.ui32CurrentPTS,pPicture->height,pPicture->width,pPicture->polarity
,pPicture->sarHorizontal,pPicture->sarVertical,pPicture->repeat,pPicture->ignore,pPicture->bIgnorePicture,pPicture->bStallStc));

if (pPicture->hImageMmaBlock) {
BuffYAddr = BMMA_GetOffset_isr(pPicture->hImageMmaBlock)+pPicture->offset;
}
else {
BuffYAddr = 0;
}
/*BREG_Read32(handle->devicehandle->regHandle, handle->outputbuffer.sCdbBuffer.ui32BaseAddr);
BDBG_ERR(("s:%ld", pPicture->STC_Lo));*/
pPpb_Cached->ui32CaptureTimeStamp = pPicture->STC_Lo;
pPpb_Cached->ui32BuffAddrY = BuffYAddr;
pPpb_Cached->sPictureMetaData.ui32PicHeight = pPicture->height;
pPpb_Cached->sPictureMetaData.ui32PicWidth = pPicture->width;
pPpb_Cached->sPictureMetaData.eFrameType = pPicture->polarity;
pPpb_Cached->sPictureMetaData.eChromaSampling = handle->startsettings.pxlformat;
pPpb_Cached->sPictureMetaData.eEncodeFrameRate = BVEE_P_FrameRateLUT[pPicture->frameRate];
pPpb_Cached->sPictureMetaData.ui32OrignalPtsHigh = 0;/*since clock is 45KHz, PTS high will be zero*/
pPpb_Cached->sPictureMetaData.ui32OrignalPtsLow = pPicture->originalPts.ui32CurrentPTS;
pPpb_Cached->sPictureMetaData.eStallStc = pPicture->bStallStc;
pPpb_Cached->sPictureMetaData.eIgnorePicture = pPicture->bIgnorePicture;

pPpb_Cached->sPictureMetaData.ui32AspectRatioIdc = 0xFF;
pPpb_Cached->sPictureMetaData.ui32SARWidth = pPicture->sarHorizontal;
pPpb_Cached->sPictureMetaData.ui32SARHeight = pPicture->sarVertical;

#if BDSP_ENCODER_ACCELERATOR_SUPPORT
BuffYAddr = (uint32_t)(BMMA_GetOffset_isr(pPicture->hLumaBlock)+pPicture->ulLumaOffset);
pPpb_Cached->ui32BuffAddrY = BuffYAddr;

BuffCAddr = (uint32_t)(BMMA_GetOffset_isr(pPicture->hChromaBlock)+pPicture->ulChromaOffset);
pPpb_Cached->ui32BuffAddrUV = BuffCAddr;

pPpb_Cached->sPictureMetaData.bStriped = pPicture->bStriped;
pPpb_Cached->sPictureMetaData.ulStripeWidth = pPicture->ulStripeWidth;
pPpb_Cached->sPictureMetaData.ulLumaNMBY = pPicture->ulLumaNMBY;
pPpb_Cached->sPictureMetaData.ulChromaNMBY = pPicture->ulChromaNMBY;
pPpb_Cached->sPictureMetaData.ePolarity = pPicture->ePolarity;
pPpb_Cached->sPictureMetaData.ulSTCSnapshotLo = pPicture->ulSTCSnapshotLo;
pPpb_Cached->sPictureMetaData.ulSTCSnapshotHi = pPicture->ulSTCSnapshotHi;
pPpb_Cached->sPictureMetaData.ulPictureId = pPicture->ulPictureId;
pPpb_Cached->sPictureMetaData.bCadenceLocked = pPicture->bCadenceLocked;
pPpb_Cached->sPictureMetaData.ePicStruct = (BVENC_VF_sPicStruct)pPicture->ePicStruct;
pPpb_Cached->sPictureMetaData.ui32BuffAddr2H1VY = (uint32_t)NULL;
pPpb_Cached->sPictureMetaData.ui32BuffAddr2H1VUV = (uint32_t)NULL;
pPpb_Cached->sPictureMetaData.ui32BuffAddr2H2VY = (uint32_t)NULL;
pPpb_Cached->sPictureMetaData.ui32BuffAddr2H2VUV = (uint32_t)NULL;
#endif
/*
BDBG_MSG(("VEE Enqueue Luma MMA 0x%x offset = 0x%x pic id = 0x%x PPB = 0x%x", pPicture->hLumaBlock,pPicture->ulLumaOffset ,pPicture->ulPictureId,temp_cappict->uiPpBufferOffset));
*/
#if 0
{
unsigned char *pLuma;

pLuma = (unsigned char *)BMMA_Lock(pPicture->hLumaBlock);
BDBG_MSG(("Luma %d %d %d %d %d", pLuma[0], pLuma[1], pLuma[2], pLuma[3], pLuma[4]));
BDBG_MSG(("Luma %d %d %d %d %d", pLuma[128], pLuma[129], pLuma[130], pLuma[131], pLuma[132]));
BMMA_Unlock(pPicture->hLumaBlock,(void*)pLuma);
}
#endif

/*
BDBG_ERR(("EnQ :: YPhy = 0x%x, PpBoffset = 0x%x", pPpb_Cached->ui32BuffAddrY, (uint32_t)temp_cappict->uiPpBufferOffset));
*/
BMMA_FlushCache(temp_cappict->hPpBufferMmaBlock, (void *)pPpb_Cached,sizeof(BVENC_VF_sPicParamBuff));
errCode = BDSP_VideoEncode_putPicture_isr( (void *)handle->task, (uint32_t)temp_cappict->uiPpBufferOffset);
if(errCode == BERR_SUCCESS)
{
BDBG_LEAVE(BVEE_Channel_EnqueuePicture_isr);
return BERR_SUCCESS;
}
else
{
BDBG_ERR(("BDSP_VideoEncode_putPicture_isr failed.."));
}


BDBG_LEAVE(BVEE_Channel_EnqueuePicture_isr);
return errCode;
}
BERR_Code BVEE_Channel_DequeuePicture_isr(
BVEE_ChannelHandle handle,
BVEE_PictureDescriptor *pPicture )
{
BERR_Code errcode= BERR_SUCCESS;
unsigned i=0;
uint32_t pPpb_Offset;   /* from Raaga */
uint32_t pPpbCap_Offset;/* VEE internal */

BDBG_ENTER(BVEE_Channel_DequeuePicture_isr);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pPicture);

BKNI_Memset(pPicture, 0, sizeof(BVEE_PictureDescriptor));

errcode = BDSP_VideoEncode_getPictureBuffer_isr( (void *)handle->task, &pPpb_Offset);

for(i=0;i<handle->opensettings.maxQueuedPictures;i++)
{
pPpbCap_Offset = handle->capturepicture[i].uiPpBufferOffset;
if((pPpbCap_Offset == pPpb_Offset)&&
(handle->capturepicture[i].bValid == true))
{
/*Give back the address*/
pPicture->hImageMmaBlock = handle->capturepicture[i].hPDescMmaBlock;
pPicture->offset = handle->capturepicture[i].uiPDescOffset;
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
pPicture->hLumaBlock = handle->capturepicture[i].hLumaBlock;
pPicture->ulPictureId = handle->capturepicture[i].ulPictureId;
pPicture->ulLumaOffset = handle->capturepicture[i].ulLumaOffset;
/*
BDBG_ERR(("VEE dequeue Luma MMA 0x%x offset = 0x%x pic id = 0x%x PPB = 0x%x", pPicture->hLumaBlock,pPicture->ulLumaOffset ,pPicture->ulPictureId,pPpb_Offset));
BDBG_ERR(("DEQ :: YPhy = 0x%x, PpBoffset = 0x%x", (uint32_t)(BMMA_GetOffset_isr(pPicture->hLumaBlock)+pPicture->ulLumaOffset), pPpb_Offset));
*/
#endif

handle->capturepicture[i].bValid = false;
break;
}
}


return errcode;
}

BERR_Code BVEE_Channel_GetBufferDescriptors(
BVEE_ChannelHandle handle,
const BAVC_VideoBufferDescriptor **pBuffer,
size_t *pSize,
const BAVC_VideoBufferDescriptor **pBuffer2,
size_t *pSize2
)
{
BVEE_P_ITBEntry *pITBEntry;
BVEE_P_ITBEntry *pITBEntryNext = NULL;

BAVC_VideoBufferDescriptor *pVideoDescriptor;
BVEE_ChannelOutputDescriptorInfo *psOutputDescDetails;
uint32_t uiCDBBaseOffset;
uint32_t uiCDBEndOffset;
uint32_t uiCDBValidOffset;
uint32_t uiCDBEndOfFrameOffset;
uint32_t uiTemp;

BDBG_ENTER(BVEE_Channel_GetBufferDescriptors);

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pBuffer);
BDBG_ASSERT(NULL != pBuffer2);
BDBG_ASSERT(NULL != pSize);
BDBG_ASSERT(NULL != pSize2);

*pBuffer = NULL;
*pSize = 0;
*pBuffer2 = NULL;
*pSize2 = 0;
psOutputDescDetails = &handle->veeoutput;

/* return if CDB/ITB context is not initialized */
if(!handle->bContextValid)
return BERR_SUCCESS;

/* Snapshot the CDB/ITB read/valid pointers
* NOTE: We MUST snapshot the CDB pointers first,
* and then the ITB pointers so that we can properly
* detect the end of the current frame.  If we read
* the ITB first, and then the CDB, it is possible a
* new ITB entry has been written in between the reads,
* and the CDB write pointer now includes the beginning
* of the next frame */
{ /* CDB Pointers */
uiCDBBaseOffset = BREG_Read32(handle->devicehandle->regHandle,handle->outputbuffer.sCdbBuffer.ui32BaseAddr);
uiCDBEndOffset = BREG_Read32(handle->devicehandle->regHandle,handle->outputbuffer.sCdbBuffer.ui32EndAddr);

uiCDBValidOffset = BREG_Read32(handle->devicehandle->regHandle,handle->outputbuffer.sCdbBuffer.ui32WriteAddr);

if ( uiCDBValidOffset >= uiCDBEndOffset )
{
uiCDBValidOffset = uiCDBBaseOffset + ( uiCDBValidOffset - uiCDBEndOffset );
}

}

while ( handle->state != BVEE_ChannelState_eStopped )
{
/* Check for Available ITB Entries */
/* TODO: We may want to copy ITB entries into local memory
* so that we can handle ITB wrap easily
*/
/* Check for Available CDB Data */
if ( uiCDBValidOffset == \
psOutputDescDetails->uiCDBBufferShadowReadOffset )
{
/* We ran out of CDB data */
BDBG_MSG(("No more CDB Data"));
break;
}
/* Check for Available ITB Entries */
/* TODO: We may want to copy ITB entries into local memory
* so that we can handle ITB wrap easily
*/

BVEE_Channel_P_ParseItb(handle, &pITBEntry, &pITBEntryNext);
if ( NULL == pITBEntry )
{
BDBG_MSG(("No more ITB Entries"));
break;
}

if ( NULL == pITBEntryNext )
{
/* We can not work unless we have the next ITB entry to determine real frame size.
Full frame of data is required. */
BDBG_MSG(("Partial Frame in CDB"));
break;
}
else
{
uint32_t uiDepthToNext;
uint32_t uiDepthToValid;

/* It is possible that the CDB Valid doesn't, yet, contain any of the next frame and
* may still be in the middle of the current frame, so we need use the depth that is the
* lesser of depth(cdb_read,cdb_next) depth(cdb_read,cdb_valid)
*/
if ( BVEE_ITB_GET_FIELD(&pITBEntryNext->fields.baseAddress, BASE_ADDRESS, CDB_ADDRESS) > \
psOutputDescDetails->uiCDBBufferShadowReadOffset )
{
uiDepthToNext = BVEE_ITB_GET_FIELD(&pITBEntryNext->fields.baseAddress, BASE_ADDRESS, CDB_ADDRESS) - \
psOutputDescDetails->uiCDBBufferShadowReadOffset;
}
else
{
uiDepthToNext = uiCDBEndOffset - \
psOutputDescDetails->uiCDBBufferShadowReadOffset;
uiDepthToNext += BVEE_ITB_GET_FIELD(&pITBEntryNext->fields.baseAddress, BASE_ADDRESS, CDB_ADDRESS) - uiCDBBaseOffset;
}

if ( uiCDBValidOffset > \
psOutputDescDetails->uiCDBBufferShadowReadOffset )
{
uiDepthToValid = uiCDBValidOffset - \
psOutputDescDetails->uiCDBBufferShadowReadOffset;
}
else
{
uiDepthToValid = uiCDBEndOffset - \
psOutputDescDetails->uiCDBBufferShadowReadOffset;
uiDepthToValid += uiCDBValidOffset - uiCDBBaseOffset;
}

if ( uiDepthToValid < uiDepthToNext )
{
uiCDBEndOfFrameOffset = uiCDBValidOffset;
}
else
{
uiCDBEndOfFrameOffset = BVEE_ITB_GET_FIELD(&pITBEntryNext->fields.baseAddress, BASE_ADDRESS, CDB_ADDRESS);;
}
}

/* Get Video Descriptor for this ITB entry */
uiTemp = (psOutputDescDetails->uiDescriptorWriteOffset + 1) % BVEE_MAX_ITBDESCRIPTORS;
if ( uiTemp == psOutputDescDetails->uiDescriptorReadOffset )
{
BDBG_MSG(("Out of descriptors"));
break;
}
pVideoDescriptor = &psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorWriteOffset];
psOutputDescDetails->uiDescriptorWriteOffset = uiTemp;

BKNI_Memset(pVideoDescriptor, 0, sizeof(BAVC_VideoBufferDescriptor));

if ( uiCDBEndOfFrameOffset > psOutputDescDetails->uiCDBBufferShadowReadOffset )
{
pVideoDescriptor->stCommon.uiLength = uiCDBEndOfFrameOffset - \
psOutputDescDetails->uiCDBBufferShadowReadOffset;
}
else
{
/* CDB Wrap occurs, so we need to split this picture into two descriptors.  We handle the first one here. */
pVideoDescriptor->stCommon.uiLength = uiCDBEndOffset - \
psOutputDescDetails->uiCDBBufferShadowReadOffset;
}

/* Populate other fields if this descriptor contains the beginning of the frame */
if ( psOutputDescDetails->uiCDBBufferShadowReadOffset == \
BVEE_ITB_GET_FIELD(&pITBEntry->fields.baseAddress, BASE_ADDRESS, CDB_ADDRESS) )
{

/* We're muxing the beginning of this frame */
pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START;

if ( 0 != BVEE_P_ITBEntry_GetIFrame(pITBEntry) )
{
pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP;
}

pVideoDescriptor->stCommon.uiOriginalPTS = BVEE_ITB_GET_FIELD(&pITBEntry->fields.escrMetadata, ESCR_METADATA, ORIGINAL_PTS);

pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID;
pVideoDescriptor->stCommon.uiPTS = BVEE_ITB_GET_FIELD(&pITBEntry->fields.ptsDts, PTS_DTS, PTS_32);
pVideoDescriptor->stCommon.uiPTS <<= 32;
pVideoDescriptor->stCommon.uiPTS |= BVEE_ITB_GET_FIELD(&pITBEntry->fields.ptsDts, PTS_DTS, PTS);

pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID;
pVideoDescriptor->uiDTS = BVEE_P_ITBEntry_GetDTS(pITBEntry);

pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID;
pVideoDescriptor->stCommon.uiESCR = BVEE_ITB_GET_FIELD(&pITBEntry->fields.escrMetadata, ESCR_METADATA, ESCR);

pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID;
pVideoDescriptor->stCommon.uiTicksPerBit = BVEE_ITB_GET_FIELD(&pITBEntry->fields.bitRate, BIT_RATE, TICKS_PER_BIT);

pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID;
pVideoDescriptor->stCommon.iSHR = BVEE_ITB_GET_FIELD(&pITBEntry->fields.bitRate, BIT_RATE, SHR);

pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID;
pVideoDescriptor->stCommon.uiSTCSnapshot = BVEE_ITB_GET_FIELD(&pITBEntry->fields.bitRate, BIT_RATE, STC_UPPER);
pVideoDescriptor->stCommon.uiSTCSnapshot <<= 32;
pVideoDescriptor->stCommon.uiSTCSnapshot |= BVEE_ITB_GET_FIELD(&pITBEntry->fields.bitRate, BIT_RATE, STC_LOWER);

/* DataUnit detection.TODO:Populate video descriptors for all NALU */
switch ( handle->startsettings.codec )
{
case BAVC_VideoCompressionStd_eH264:
{
uint8_t *pCDBBufferCached =  (uint8_t*) handle->veeoutput.pCdbBaseCached;
uint8_t *uiCurrentByte = &pCDBBufferCached[psOutputDescDetails->uiCDBBufferShadowReadOffset-uiCDBBaseOffset];
uint32_t uiDataUnitStartCode = (uiCurrentByte[0]<<24)|(uiCurrentByte[1]<<16)|(uiCurrentByte[2])<<16|uiCurrentByte[3];
uint8_t data[5];

/* CDB Wraparound */
if ( psOutputDescDetails->uiCDBBufferShadowReadOffset + 5 >= \
uiCDBEndOffset )
{
size_t length = 5;
uint32_t readOffset = psOutputDescDetails->uiCDBBufferShadowReadOffset;
uint32_t endOffset = uiCDBEndOffset;
size_t preWrapAmount = endOffset - readOffset;
size_t wrapAmount = length - preWrapAmount;

/* pCDBBufferCached points to the CDB base. So, position it appropriately */
BKNI_Memcpy(data, pCDBBufferCached+(psOutputDescDetails->uiCDBBufferShadowReadOffset-uiCDBBaseOffset), preWrapAmount);
BKNI_Memcpy(data+preWrapAmount, pCDBBufferCached, wrapAmount);

uiCurrentByte = data;
uiDataUnitStartCode = (uiCurrentByte[0]<<24)|(uiCurrentByte[1]<<16)|(uiCurrentByte[2])<<16|uiCurrentByte[3];

}

if(uiDataUnitStartCode == BVEE_DATA_UNIT_START_CODE)
{
pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START;
pVideoDescriptor->uiDataUnitType = uiCurrentByte[4];
}
}
break;
default:
break;
}
}

/* Normalize the offset to 0 */
pVideoDescriptor->stCommon.uiOffset = \
psOutputDescDetails->uiCDBBufferShadowReadOffset - uiCDBBaseOffset;

/* Invalidate this frame from the cache prior to the host accessing it */
BMMA_FlushCache(handle->outputbuffer.hCDBBufferMmaBlock, \
(char *)handle->veeoutput.pCdbBaseCached + pVideoDescriptor->stCommon.uiOffset,  \
pVideoDescriptor->stCommon.uiLength);

/* Advance read pointer appropriately */
psOutputDescDetails->uiCDBBufferShadowReadOffset += \
pVideoDescriptor->stCommon.uiLength;
if ( psOutputDescDetails->uiCDBBufferShadowReadOffset >= \
uiCDBEndOffset )
{
psOutputDescDetails->uiCDBBufferShadowReadOffset -= \
( uiCDBEndOffset - uiCDBBaseOffset );
}

/* If we need to send metadata, send it on the first frame */
if ( handle->startsettings.sendMetadata && (pVideoDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START) )
{
BAVC_VideoMetadataDescriptor *pMetadataDescriptor = &psOutputDescDetails->pstMetadataCached[0];
BAVC_VideoBufferDescriptor *pFrameDescriptor;

/* This is the first frame so we should always have another descriptor available. Assert for sanity. */
uiTemp = (psOutputDescDetails->uiDescriptorWriteOffset + 1) % BVEE_MAX_VIDEODESCRIPTORS;
BDBG_ASSERT(uiTemp != psOutputDescDetails->uiDescriptorReadOffset);
pFrameDescriptor = &psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorWriteOffset];
psOutputDescDetails->uiDescriptorWriteOffset = uiTemp;

/* The metadata descriptor must come before the first frame.  Swap them. */
*pFrameDescriptor = *pVideoDescriptor;   /* Copy frame descriptor contents into second descriptor - we're about to overwrite the old one */

BKNI_Memset(pVideoDescriptor, 0, sizeof(BAVC_VideoBufferDescriptor));

pVideoDescriptor->stCommon.uiOffset = 0;
pVideoDescriptor->stCommon.uiLength = sizeof( BAVC_VideoMetadataDescriptor );
pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA;
pVideoDescriptor->uiDataUnitType = BAVC_VideoMetadataType_eCommon;

/* Populate metadata */
BKNI_Memset(pMetadataDescriptor, 0, sizeof(*pMetadataDescriptor));

/* Set Protocol */
BDBG_MSG(("Setting protocol to %u", handle->startsettings.codec));
switch ( handle->startsettings.codec )
{
case BAVC_VideoCompressionStd_eH264:
/* Populate Coded Dimension */
pMetadataDescriptor->uiMetadataFlags |= BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_DIMENSION_CODED_VALID;
pMetadataDescriptor->stDimension.coded.uiHeight = ( ( BVEE_P_ITBEntry_GetMetadata(pITBEntry) >> 0 ) & 0x7F ) << 4;
pMetadataDescriptor->stDimension.coded.uiWidth = ( ( BVEE_P_ITBEntry_GetMetadata(pITBEntry) >> 7 ) & 0x7F ) << 4;
break;
default:
break;
}

handle->startsettings.sendMetadata = false;
}
}

if ( handle->startsettings.sendEos )
{
BDBG_MSG(("EOS Required"));
/* Get a descriptor for EOS */
uiTemp = (psOutputDescDetails->uiDescriptorWriteOffset + 1) % BVEE_MAX_VIDEODESCRIPTORS;
if ( uiTemp == psOutputDescDetails->uiDescriptorReadOffset )
{
BDBG_MSG(("Out of descriptors, can't send EOS"));
}
else
{
pVideoDescriptor = &psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorWriteOffset];
psOutputDescDetails->uiDescriptorWriteOffset = uiTemp;

BKNI_Memset(pVideoDescriptor, 0, sizeof(BAVC_VideoBufferDescriptor));

/* Decoder is stopped and we have run out of data. Fill the EOS entry in Video descriptor */
pVideoDescriptor->stCommon.uiFlags = BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;
BDBG_MSG(("EOS %p", (void*)pVideoDescriptor));
handle->startsettings.sendEos = false;
}
}

/* Assign array(s) and count(s) */
if ( psOutputDescDetails->uiDescriptorWriteOffset >= \
psOutputDescDetails->uiDescriptorReadOffset )
{
*pBuffer = &psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorReadOffset];
*pSize = psOutputDescDetails->uiDescriptorWriteOffset - psOutputDescDetails->uiDescriptorReadOffset;

*pBuffer2 = NULL;
*pSize2 = 0;
}
else
{
*pBuffer = &psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorReadOffset];
*pSize = BVEE_MAX_VIDEODESCRIPTORS - psOutputDescDetails->uiDescriptorReadOffset;

*pBuffer2 = &psOutputDescDetails->pstDescriptorsCached[0];
*pSize2 = psOutputDescDetails->uiDescriptorWriteOffset;
}
/*
BDBG_MSG(("BVEE_Channel_GetBufferDescriptors pBuffer = 0x%x size = %d, pBuffer2 = 0x%x size = %d", *pBuffer, *pSize, *pBuffer2, *pSize2));
*/
BDBG_LEAVE(BVEE_Channel_GetBufferDescriptors);
return BERR_TRACE( BERR_SUCCESS );
}


BERR_Code BVEE_Channel_ConsumeBufferDescriptors(
BVEE_ChannelHandle handle,
unsigned numBufferDescriptors
)
{
BREG_Handle regHandle;
BERR_Code   ret = BERR_SUCCESS;
uint32_t uiCDBReadOffset;
uint32_t uiCDBEndOffset;
uint32_t uiCDBBaseOffset;
uint32_t uiITBReadOffset;
uint32_t uiITBEndOffset;
uint32_t uiITBBaseOffset;
BVEE_ChannelOutputDescriptorInfo  *psOutputDescDetails;

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

BDBG_ASSERT(numBufferDescriptors > 0);

BDBG_ENTER(BVEE_Channel_ConsumeBufferDescriptors);

BDBG_MSG(("BVEE_Channel_ConsumeBufferDescriptors: uiNumBufferDescriptors = %d",numBufferDescriptors));
regHandle = handle->devicehandle->regHandle;
psOutputDescDetails = &handle->veeoutput;

/* Read CDB Addresses */
uiCDBBaseOffset = BREG_Read32(regHandle, handle->outputbuffer.sCdbBuffer.ui32BaseAddr);
uiCDBEndOffset = BREG_Read32(regHandle, handle->outputbuffer.sCdbBuffer.ui32EndAddr);
uiCDBReadOffset = BREG_Read32(regHandle, handle->outputbuffer.sCdbBuffer.ui32ReadAddr);
uiCDBEndOffset++;


/* Read ITB Addresses */
uiITBBaseOffset = BREG_Read32(regHandle, handle->outputbuffer.sItbBuffer.ui32BaseAddr);
uiITBEndOffset = BREG_Read32(regHandle, handle->outputbuffer.sItbBuffer.ui32EndAddr);
uiITBReadOffset= BREG_Read32(regHandle, handle->outputbuffer.sItbBuffer.ui32ReadAddr);
uiITBEndOffset++;

while ( numBufferDescriptors )
{
if( 0 != (psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorReadOffset].stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA))
{
/*Do nothing for now*/
}
else if ( 0 == (psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorReadOffset].stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS) )
{
/* Move CDB Read Offset */
uiCDBReadOffset += psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorReadOffset].stCommon.uiLength;
if ( uiCDBReadOffset >= uiCDBEndOffset )
{
uiCDBReadOffset -= ( uiCDBEndOffset - uiCDBBaseOffset );
}

if ( 0 != (psOutputDescDetails->pstDescriptorsCached[psOutputDescDetails->uiDescriptorReadOffset].stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )
{
/* Move ITB Read Offset */
uiITBReadOffset += sizeof( BVEE_P_ITBEntry );
if ( uiITBReadOffset >= uiITBEndOffset )
{
uiITBReadOffset -= ( uiITBEndOffset - uiITBBaseOffset );
}
}
}

/* Move Descriptor Read Offset */
psOutputDescDetails->uiDescriptorReadOffset++;
psOutputDescDetails->uiDescriptorReadOffset %= BVEE_MAX_ITBDESCRIPTORS;

numBufferDescriptors--;
}

BDBG_MSG(("UpdateBufferDescriptors :uiDescriptorReadOffset = %d",\
psOutputDescDetails->uiDescriptorReadOffset));

/* Update Actual ITB/CDB Read Pointers */
BREG_Write32(regHandle, handle->outputbuffer.sCdbBuffer.ui32ReadAddr, uiCDBReadOffset);
BREG_Write32(regHandle, handle->outputbuffer.sItbBuffer.ui32ReadAddr, uiITBReadOffset);

BDBG_LEAVE(BVEE_Channel_ConsumeBufferDescriptors);

return ret;
}
BERR_Code BVEE_Channel_GetBufferStatus(
BVEE_ChannelHandle handle,
BAVC_VideoBufferStatus *pBufferStatus
)
{
BDBG_ENTER(BVEE_Channel_GetBufferStatus);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pBufferStatus);
BKNI_Memset(pBufferStatus, 0, sizeof(BAVC_VideoBufferStatus));

pBufferStatus->stCommon.pFrameBufferBaseAddress = handle->veeoutput.pCdbBaseCached;
pBufferStatus->stCommon.pMetadataBufferBaseAddress = handle->veeoutput.pstMetadataCached;
pBufferStatus->stCommon.hFrameBufferBlock = handle->outputbuffer.hCDBBufferMmaBlock;
pBufferStatus->stCommon.hMetadataBufferBlock = handle->veeoutput.hMetadataMmaBlock;
pBufferStatus->stCommon.hIndexBufferBlock = handle->outputbuffer.hITBBufferMmaBlock;

BDBG_LEAVE(BVEE_Channel_GetBufferStatus);
return BERR_SUCCESS;
}

BERR_Code BVEE_Channel_GetDataSyncSettings(
BVEE_ChannelHandle handle,
BVEE_DatasyncSettings *EncDataSyncSettings
)
{
BERR_Code   ret = BERR_SUCCESS;
BDSP_VideoEncodeTaskDatasyncSettings  DataSyncSettings;
BDBG_ENTER(BVEE_Channel_GetDataSyncSettings);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

ret = BDSP_VideoEncodeTask_GetDatasyncSettings(handle->hPrimaryStage, &DataSyncSettings);
if(ret != BERR_SUCCESS)
{
BDBG_ERR(("Err in BVEE_Channel_GetDataSyncSettings"));
}

EncDataSyncSettings->eEnableStc = DataSyncSettings.eEnableStc;
EncDataSyncSettings->ui32StcAddress = DataSyncSettings.ui32StcAddress;
BDBG_LEAVE(BVEE_Channel_GetDataSyncSettings);
return ret;
}

BERR_Code BVEE_Channel_SetDataSyncSettings(
BVEE_ChannelHandle handle,
BVEE_DatasyncSettings EncDataSyncSettings
)
{
BERR_Code   ret = BERR_SUCCESS;
BDSP_VideoEncodeTaskDatasyncSettings  DataSyncSettings;
BDBG_ENTER(BVEE_Channel_SetDataSyncSettings);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

DataSyncSettings.eEnableStc = EncDataSyncSettings.eEnableStc;
DataSyncSettings.ui32StcAddress = EncDataSyncSettings.ui32StcAddress;

ret = BDSP_VideoEncodeTask_SetDatasyncSettings(handle->hPrimaryStage, &DataSyncSettings);
if(ret != BERR_SUCCESS)
{
BDBG_ERR(("Err in BVEE_Channel_SetDataSyncSettings"));
}

BDBG_LEAVE(BVEE_Channel_SetDataSyncSettings);
return ret;
}

BERR_Code BVEE_Channel_AllocExtInterrupt(
BVEE_ChannelHandle handle,
BVEE_ExtInterruptHandle *pIntHandle
)
{
BERR_Code   ret = BERR_SUCCESS;
BDBG_ENTER(BVEE_Channel_AllocExtInterrupt);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

ret = BDSP_AllocateExternalInterrupt(handle->devicehandle->dspHandle, DSP_INDEX,(BDSP_ExternalInterruptHandle *)pIntHandle);
if(ret != BERR_SUCCESS)
{
BDBG_ERR(("Err in BVEE_Channel_AllocExtInterrupt"));
}

BDBG_LEAVE(BVEE_Channel_AllocExtInterrupt);
return ret;
}

BERR_Code BVEE_Channel_GetExtInterruptInfo(
BVEE_ChannelHandle handle,
BVEE_ExtInterruptHandle pIntHandle,
BVEE_ExtInterruptInfo *pExtIntInfo
)
{
BERR_Code   ret = BERR_SUCCESS;
BVEE_ExtInterruptInfo *pInfo ;
BDBG_ENTER(BVEE_Channel_GetExtInterruptInfo);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

ret = BDSP_GetExternalInterruptInfo((BDSP_ExternalInterruptHandle)pIntHandle, (BDSP_ExternalInterruptInfo**)&pInfo);
if(ret != BERR_SUCCESS)
{
BDBG_ERR(("Err in BVEE_Channel_GetExtInterruptInfo"));
}
*pExtIntInfo = *pInfo;
BDBG_LEAVE(BVEE_Channel_GetExtInterruptInfo);
return ret;
}

BERR_Code BVEE_Channel_FreeExtInterrupt(
BVEE_ChannelHandle handle,
BVEE_ExtInterruptHandle pIntHandle
)
{
BERR_Code   ret = BERR_SUCCESS;
BDBG_ENTER(BVEE_Channel_FreeExtInterrupt);
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);

ret = BDSP_FreeExternalInterrupt((BDSP_ExternalInterruptHandle)pIntHandle);
if(ret != BERR_SUCCESS)
{
BDBG_ERR(("Err in BVEE_Channel_FreeExtInterrupt"));
}

BDBG_LEAVE(BVEE_Channel_FreeExtInterrupt);
return ret;
}
void BVEE_Channel_GetTriggerInfo(
BVEE_ChannelHandle handle,
BVEE_ChannelTriggerInfo *pInfo  /* [out] */
)
{
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BSTD_UNUSED(pInfo);
}

void BVEE_Channel_InitPictureDescriptor(
BVEE_ChannelHandle handle,
BVEE_PictureDescriptor *pPicture    /* [out] */
)
{
BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BSTD_UNUSED(pPicture);
}


/*************/
/* User Data */
/*************/
/* BVEE_Channel_UserData_AddBuffers_isr - adds user data field info structs(s) to user data queue for stream insertion */
BERR_Code
BVEE_Channel_UserData_AddBuffers_isr(
BVEE_ChannelHandle hVeeCh,
const BUDP_Encoder_FieldInfo *pstUserDataFieldInfo, /* Pointer to first field info descriptor */
size_t uiCount, /* Count of user data field buffer info structs */
size_t *puiQueuedCount /* Count of user data field info structs queued by encoder (*puiQueuedCount <= uiCount) */
)
{
BERR_Code rc = BERR_SUCCESS;
unsigned i;
BVEE_CCInfo *pstCCInfo;
void *pTemp;  /* temp ptr to point to the previously stored CC data */

BDBG_ENTER( BVEE_Channel_UserData_AddBuffers_isr );

BDBG_ASSERT( hVeeCh );
BDBG_ASSERT( pstUserDataFieldInfo );
BDBG_ASSERT( puiQueuedCount );


/* update the *puiQueuedCount irrespective of data availability */
*puiQueuedCount = uiCount;

/* if CC is not enabled then return */
if (!hVeeCh->startsettings.bSendCC)
return BERR_SUCCESS;
#if BVEE_P_DUMP_USERDATA_LOG
{
BVEE_FW_UserData_PacketType ePacketType;
unsigned uiSourceDescNum = 0;
BDBG_ERR(("Start Packet Type %d", hVeeCh->startsettings.ccPacketType));
for ( ePacketType = 0; ePacketType < BVEE_FW_P_UserData_PacketType_eMax; ePacketType++ )
{
for ( uiSourceDescNum = 0; uiSourceDescNum < pstUserDataFieldInfo->uiNumDescriptors; uiSourceDescNum++ )
{
const BUDP_Encoder_PacketDescriptor *pPacketDescriptor = &pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum];
for ( i = 0; i < pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].data.stDvs157.stCC.uiNumLines; i++ )
{
BDBG_ERR(("From XUD: Fmt %d %d %d:%d", \
pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].ePacketFormat,
pPacketDescriptor->data.stDvs157.stCC.astLine[i].seq.field_number,
pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_1,
pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_2));
}
}
}
}
#endif

if ( BVEE_ChannelState_eStarted == hVeeCh->state )
{
/*Read FW UserData Queue*/
{
BVEE_FW_UserData_PacketType ePacketType;
unsigned uiSourceDescNum = 0;

BDBG_ASSERT( BVEE_FW_P_UserData_PacketType_eMax >= pstUserDataFieldInfo->uiNumDescriptors );

/* return if no data is received */
if (pstUserDataFieldInfo->uiNumDescriptors <= 0)
return BERR_INVALID_PARAMETER;


/* FW wants packets to be inserted in ascending packet type order, so for now, just do a brute force traverse of all the descriptors
* process them in ascending packet type order
*/
#if BVEE_P_DUMP_USERDATA_LOG
if ( NULL == hVeeCh->userdata.hUserDataLog )
{
char fname[256];
sprintf(fname, "out/BVCE_USERDATA_LOG.log");
hVeeCh->userdata.hUserDataLog = fopen(fname, "wb");
if ( NULL == hVeeCh->userdata.hUserDataLog )
{
BDBG_ERR(("Error Creating BVEE User Data Log Dump File (%s)", fname));
}
}

if ( NULL != hVeeCh->userdata.hUserDataLog )
{
fprintf( hVeeCh->userdata.hUserDataLog, "++++++++++++++++++++++++++++++++++[%d][%d][%d]\n", pstUserDataFieldInfo->uiNumDescriptors, pstUserDataFieldInfo->ePolarity, pstUserDataFieldInfo->uiStgPictureId);
}
#endif

if (hVeeCh->userdata.savebuffer_cc.bIsFree)
{
/* buffer is free so store this CC info */
pstCCInfo = &hVeeCh->userdata.savebuffer_cc;


pstCCInfo->uiNumDescriptors = pstUserDataFieldInfo->uiNumDescriptors;
pTemp = pstCCInfo->pDescriptorBufferCached;
for (uiSourceDescNum = 0; uiSourceDescNum < pstUserDataFieldInfo->uiNumDescriptors; uiSourceDescNum++)
{
pstCCInfo->uiStgId[uiSourceDescNum] = pstUserDataFieldInfo->uiStgPictureId;
pstCCInfo->ePacketFormat[uiSourceDescNum] = pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].ePacketFormat;
pstCCInfo->uiNumCCLines[uiSourceDescNum]  = pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].data.stDvs157.stCC.uiNumLines;

BKNI_Memcpy (pTemp, \
pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].data.stDvs157.stCC.astLine, \
pstCCInfo->uiNumCCLines[uiSourceDescNum] * sizeof(BUDP_DCCparse_ccdata));
pTemp = (uint8_t *)pTemp + (pstCCInfo->uiNumCCLines[uiSourceDescNum] * sizeof(BUDP_DCCparse_ccdata));
}

/* mark as occupied */
hVeeCh->userdata.savebuffer_cc.bIsFree = false;

/* return now, in next call send this savedbuffer & received CC, combine them and send it to FW */
return BERR_SUCCESS;
}


/* combine CC in the savebuffer & the current one and send it to FW*/
{
BDSP_Raaga_Video_DCCparse_ccdata cc_packet;

cc_packet.cc_chunk_count = 0;

/* FW expects the chunk count to be sent only once with sum of  savebuffer & current CC's num lines */
for ( uiSourceDescNum = 0; uiSourceDescNum < hVeeCh->userdata.savebuffer_cc.uiNumDescriptors; uiSourceDescNum++ )
{
/* skip the CC packet types that does not match the required one */
if (hVeeCh->startsettings.ccPacketType != BVEE_P_UserData_PacketTypeLUT[hVeeCh->userdata.savebuffer_cc.ePacketFormat[uiSourceDescNum]])
continue;

cc_packet.cc_chunk_count += hVeeCh->userdata.savebuffer_cc.uiNumCCLines[uiSourceDescNum];
}

for ( uiSourceDescNum = 0; uiSourceDescNum < pstUserDataFieldInfo->uiNumDescriptors; uiSourceDescNum++ )
{
/* skip the CC packet types that does not match the required one */
if (hVeeCh->startsettings.ccPacketType != BVEE_P_UserData_PacketTypeLUT[pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].ePacketFormat])
continue;

cc_packet.cc_chunk_count += pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].data.stDvs157.stCC.uiNumLines;
}

#if BVEE_P_DUMP_USERDATA_LOG
fprintf (hVeeCh->userdata.hUserDataLog, "chunk count=%d\n",cc_packet.cc_chunk_count);
#endif

/* first send savedbuffer cc to FW */
pstCCInfo = &hVeeCh->userdata.savebuffer_cc;
for ( ePacketType = 0; ePacketType < BVEE_FW_P_UserData_PacketType_eMax; ePacketType++)
{
/* Intialize to the start of the cached CC descriptor buffer */
pTemp = pstCCInfo->pDescriptorBufferCached;
for ( uiSourceDescNum = 0; uiSourceDescNum < pstCCInfo->uiNumDescriptors; uiSourceDescNum++ )
{
/* skip the CC packet types that does not match the required one */
if (hVeeCh->startsettings.ccPacketType != BVEE_P_UserData_PacketTypeLUT[pstCCInfo->ePacketFormat[uiSourceDescNum]])
{
/* move the cached CC ptr accordingly */
pTemp = (uint8_t *)pstCCInfo->pDescriptorBufferCached + (pstCCInfo->uiNumCCLines[uiSourceDescNum] * sizeof(BUDP_DCCparse_ccdata));
continue;
}

/* Only add packet types that FW understands */
if ( ePacketType == BVEE_P_UserData_PacketTypeLUT[pstCCInfo->ePacketFormat[uiSourceDescNum]])
{
for ( i = 0; i < pstCCInfo->uiNumCCLines[uiSourceDescNum]; i++ )
{
const BUDP_DCCparse_ccdata *pCCData = (BUDP_DCCparse_ccdata *)pTemp;

cc_packet.stgId = pstCCInfo->uiStgId[uiSourceDescNum];
cc_packet.bIsAnalog = pCCData->bIsAnalog;
cc_packet.polarity = pCCData->polarity;
cc_packet.format = pCCData->format;
cc_packet.cc_valid = pCCData->cc_valid;
cc_packet.cc_priority = pCCData->cc_priority;
cc_packet.line_offset = pCCData->line_offset;
cc_packet.seq.field_number = pCCData->seq.field_number;
cc_packet.cc_data_1 = pCCData->cc_data_1;
cc_packet.cc_data_2 = pCCData->cc_data_2;
cc_packet.active_format = pCCData->active_format;

BDSP_VideoEncode_putCcData_isr(hVeeCh->task, (void*)&cc_packet);
/* set chunk count to zero for remaining ISR to Raaga */
cc_packet.cc_chunk_count = 0;

#if BVEE_P_DUMP_USERDATA_LOG
fprintf (hVeeCh->userdata.hUserDataLog, "Analog=%d Polarity=%d Format=%d CC_Valid=%d CC_Pirority=%d Line_offset=%d Field_num=%d ", \
cc_packet.bIsAnalog,
cc_packet.polarity,
cc_packet.format,
cc_packet.cc_valid,
cc_packet.cc_priority,
cc_packet.line_offset,
cc_packet.seq.field_number);

fprintf (hVeeCh->userdata.hUserDataLog, "cc_data_1=0x%X cc_data_2=0x%X Active_format=%d\n", \
cc_packet.cc_data_1,
cc_packet.cc_data_2,
cc_packet.active_format);
#endif
pTemp = (uint8_t *)pTemp + sizeof(BUDP_DCCparse_ccdata);
}
}
/* move the cached CC ptr accordingly */
pTemp = (uint8_t *)pstCCInfo->pDescriptorBufferCached + (pstCCInfo->uiNumCCLines[uiSourceDescNum] * sizeof(BUDP_DCCparse_ccdata));
}
}

/* mark savebuffer as free */
hVeeCh->userdata.savebuffer_cc.bIsFree = true;

/* now send current cc which is received from XUD lib */
for ( ePacketType = 0; ePacketType < BVEE_FW_P_UserData_PacketType_eMax; ePacketType++ )
{
for ( uiSourceDescNum = 0; uiSourceDescNum < pstUserDataFieldInfo->uiNumDescriptors; uiSourceDescNum++ )
{
/* skip the CC packet types that does not match the required one */
if (hVeeCh->startsettings.ccPacketType != BVEE_P_UserData_PacketTypeLUT[pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].ePacketFormat])
continue;

/* Only add packet types that FW understands */
if ( ePacketType == BVEE_P_UserData_PacketTypeLUT[pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].ePacketFormat] )
{
const BUDP_Encoder_PacketDescriptor *pPacketDescriptor = &pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum];
BDSP_Raaga_Video_DCCparse_ccdata cc_packet;

for ( i = 0; i < pstUserDataFieldInfo->stPacketDescriptor[uiSourceDescNum].data.stDvs157.stCC.uiNumLines; i++ )
{
cc_packet.stgId = pstUserDataFieldInfo->uiStgPictureId;
cc_packet.bIsAnalog = pPacketDescriptor->data.stDvs157.stCC.astLine[i].bIsAnalog;
cc_packet.polarity = pPacketDescriptor->data.stDvs157.stCC.astLine[i].polarity;
cc_packet.format = pPacketDescriptor->data.stDvs157.stCC.astLine[i].format;
cc_packet.cc_valid = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_valid;
cc_packet.cc_priority = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_priority;
cc_packet.line_offset = pPacketDescriptor->data.stDvs157.stCC.astLine[i].line_offset;
cc_packet.seq.field_number = pPacketDescriptor->data.stDvs157.stCC.astLine[i].seq.field_number;
cc_packet.cc_data_1 = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_1;
cc_packet.cc_data_2 = pPacketDescriptor->data.stDvs157.stCC.astLine[i].cc_data_2;
cc_packet.active_format = pPacketDescriptor->data.stDvs157.stCC.astLine[i].active_format;
cc_packet.cc_chunk_count = 0;
BDSP_VideoEncode_putCcData_isr(hVeeCh->task, (void*)&cc_packet);

#if BVEE_P_DUMP_USERDATA_LOG
fprintf (hVeeCh->userdata.hUserDataLog, "Analog=%d Polarity=%d Format=%d CC_Valid=%d CC_Pirority=%d Line_offset=%d Field_num=%d ", \
cc_packet.bIsAnalog,
cc_packet.polarity,
cc_packet.format,
cc_packet.cc_valid,
cc_packet.cc_priority,
cc_packet.line_offset,
cc_packet.seq.field_number);

fprintf (hVeeCh->userdata.hUserDataLog, "cc_data_1=0x%X cc_data_2=0x%X Active_format=%d\n", \
cc_packet.cc_data_1,
cc_packet.cc_data_2,
cc_packet.active_format);
#endif
}
}
}
}
}

hVeeCh->userdata.uiQueuedBuffers++;

pstUserDataFieldInfo = BUDP_ENCODER_FIELDINFO_GET_NEXT (pstUserDataFieldInfo);
}

}
else
{
rc = BERR_TRACE( BERR_INVALID_PARAMETER );
}
BDBG_LEAVE( BVEE_Channel_UserData_AddBuffers_isr );

return BERR_TRACE( rc );
}


BERR_Code
BVEE_Channel_UserData_GetStatus_isr(
BVEE_ChannelHandle hVceCh,
BVEE_Channel_UserData_Status *pstUserDataStatus
)
{
BERR_Code rc = BERR_SUCCESS;

BSTD_UNUSED(hVceCh);
BSTD_UNUSED(pstUserDataStatus);

return BERR_TRACE( rc );
}

BERR_Code BVEE_Channel_GetSettings(
BVEE_ChannelHandle handle,
BVEE_ChannelSettings *pSettings
)
{
BERR_Code errCode = BERR_SUCCESS;
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
BDSP_Raaga_VideoBX264UserConfig userConfig;
#else
BDSP_Raaga_VideoBH264UserConfig userConfig;
#endif

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pSettings);

if (handle->task && handle->hPrimaryStage)
{
errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &userConfig, sizeof(userConfig));
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("BDSP_Task_GetStageSettings failed in BVEE_Channel_GetSettings"));
BERR_TRACE(errCode);
}

pSettings->ui32TargetBitRate = userConfig.ui32TargetBitRate;
pSettings->frameRate = (BAVC_FrameRateCode)userConfig.ui32FrameRate;
pSettings->bSceneChangeEnable = userConfig.eSceneChangeEnable;
}
else
{
BDBG_ERR(("Err in BVEE_Channel_GetSettings"));
}

return errCode;
}

BERR_Code BVEE_Channel_SetSettings(
BVEE_ChannelHandle handle,
const BVEE_ChannelSettings *pSettings
)
{
BERR_Code errCode = BERR_SUCCESS;
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
BDSP_Raaga_VideoBX264UserConfig userConfig;
#else
BDSP_Raaga_VideoBH264UserConfig userConfig;
#endif

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pSettings);

if (handle->task && handle->hPrimaryStage)
{
errCode = BDSP_Stage_GetSettings(handle->hPrimaryStage, &userConfig, sizeof(userConfig));
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("BDSP_Task_GetStageSettings failed in BVEE_Channel_SetSettings"));
BERR_TRACE(errCode);
}

userConfig.ui32TargetBitRate = pSettings->ui32TargetBitRate;
userConfig.ui32FrameRate = BVEE_P_FrameRateLUT[pSettings->frameRate];
userConfig.eSceneChangeEnable = pSettings->bSceneChangeEnable;

errCode= BDSP_Stage_SetSettings(handle->hPrimaryStage, &userConfig, sizeof(userConfig));
if(errCode != BERR_SUCCESS)
{
BDBG_ERR(("BDSP_Task_SetStageSettings failed in BVEE_Channel_SetSettings"));
BERR_TRACE(errCode);
}
}
else
{
BDBG_ERR(("Err in BVEE_Channel_SetSettings"));
}

return errCode;
}

void BVEE_Channel_GetStatus(
BVEE_ChannelHandle handle,
BVEE_ChannelStatus *pStatus     /* [out] */
)
{
BERR_Code errCode;
#if BDSP_ENCODER_ACCELERATOR_SUPPORT
BDSP_Raaga_VideoX264EncoderInfo streamInfo;
#else
BDSP_Raaga_VideoH264EncoderInfo streamInfo;
#endif

BDBG_OBJECT_ASSERT(handle, BVEE_Channel);
BDBG_ASSERT(NULL != pStatus);

BKNI_Memset(pStatus, 0, sizeof(BVEE_ChannelStatus));

errCode = BDSP_Stage_GetStatus(handle->hPrimaryStage, &streamInfo, sizeof(streamInfo));
if ( errCode )
{
BDBG_ERR(("BDSP_Stage_GetStatus failed with 0x%x", errCode));
return;
}

pStatus->ui32TotalFramesRecvd               = streamInfo.TotalFramesRecvd;
pStatus->ui32TotalFramesEncoded             = streamInfo.TotalFramesEncoded;
pStatus->ui32TotalFramesDropedForFRC        = streamInfo.TotalFramesDropedForFRC;
pStatus->ui32TotalFramesDropedForLipSynch   = streamInfo.TotalFramesDropedForLipSynch;
pStatus->ui32CdbFullCounter                 = streamInfo.ui32CdbFullCounter;
pStatus->ui32RelinquishCounter              = streamInfo.ui32RelinquishCounter;
pStatus->ui32EncodedPTS                     = streamInfo.uiEncodedPTS;
pStatus->ui32StcValue                       = streamInfo.ui32StcValue;
}
