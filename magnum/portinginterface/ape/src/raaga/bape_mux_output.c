/***************************************************************************
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
*
* API Description:
*   API name: MuxOutput
*    Specific APIs related to Audio Transcoder Output (GenCdbItb)
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bchp_common.h"

#ifdef BCHP_XPT_PCROFFSET_REG_START
#include "bchp_xpt_pcroffset.h"
#endif

BDBG_MODULE(bape_mux_output);
BDBG_FILE_MODULE(bape_adts);

BDBG_OBJECT_ID(BAPE_MuxOutput);

#if BAPE_CHIP_HAS_POST_PROCESSING
#include "bdsp.h"
#define BAPE_ITB_ENTRY_TYPE_BASE_ADDRESS (0x20)
#define BAPE_ITB_ENTRY_TYPE_PTS_DTS      (0x21)
#define BAPE_ITB_ENTRY_TYPE_BIT_RATE     (0x60)
#define BAPE_ITB_ENTRY_TYPE_MUX_ESCR     (0x61)
#define BAPE_ITB_ENTRY_TYPE_ALGO_INFO    (0x62)
#define BAPE_ITB_ENTRY_TYPE_ASC          (0x63)
#define BAPE_ITB_ENTRY_TYPE_METADATA     (0x68)

/* Get Field Routines for ITB fields */
#define BAPE_ITB_WORD(Entry,Field) BAPE_ITB_##Entry##_##Field##_WORD
#define BAPE_ITB_MASK(Entry,Field) BAPE_ITB_##Entry##_##Field##_MASK
#define BAPE_ITB_SHIFT(Entry,Field) BAPE_ITB_##Entry##_##Field##_SHIFT

#define BAPE_ITB_GET_FIELD(Memory,Entry,Field)\
    ((((Memory)->words[BAPE_ITB_WORD(Entry,Field)] & \
       BAPE_ITB_MASK(Entry,Field)) >> \
      BAPE_ITB_SHIFT(Entry,Field)))

/* General Fields */
#define BAPE_ITB_GENERIC_ENTRY_TYPE_WORD      (0)
#define BAPE_ITB_GENERIC_ENTRY_TYPE_MASK      (0xFF000000)
#define BAPE_ITB_GENERIC_ENTRY_TYPE_SHIFT     (24)

/* Base Address Fields */
#define BAPE_ITB_BASE_ADDRESS_ERROR_WORD            (0)
#define BAPE_ITB_BASE_ADDRESS_ERROR_MASK            (0x00800000)
#define BAPE_ITB_BASE_ADDRESS_ERROR_SHIFT           (23)
#define BAPE_ITB_BASE_ADDRESS_CDB_ADDRESS_WORD      (1)
#define BAPE_ITB_BASE_ADDRESS_CDB_ADDRESS_MASK      (0xFFFFFFFF)
#define BAPE_ITB_BASE_ADDRESS_CDB_ADDRESS_SHIFT     (0)
#define BAPE_ITB_BASE_ADDRESS_FRAME_VALID_WORD    (2)
#define BAPE_ITB_BASE_ADDRESS_FRAME_VALID_MASK    (0x80000000)
#define BAPE_ITB_BASE_ADDRESS_FRAME_VALID_SHIFT   (31)
#define BAPE_ITB_BASE_ADDRESS_FRAME_LENGTH_WORD     (2)
#define BAPE_ITB_BASE_ADDRESS_FRAME_LENGTH_MASK     (0x0000FFFF)
#define BAPE_ITB_BASE_ADDRESS_FRAME_LENGTH_SHIFT    (0)

/* PTS_DTS Fields */
#define BAPE_ITB_PTS_DTS_DTS_VALID_WORD     (0)
#define BAPE_ITB_PTS_DTS_DTS_VALID_MASK     (0x00008000)
#define BAPE_ITB_PTS_DTS_DTS_VALID_SHIFT    (15)
#define BAPE_ITB_PTS_DTS_PTS_32_WORD        (0)
#define BAPE_ITB_PTS_DTS_PTS_32_MASK        (0x00000002)
#define BAPE_ITB_PTS_DTS_PTS_32_SHIFT       (1)
#define BAPE_ITB_PTS_DTS_DTS_32_WORD        (0)
#define BAPE_ITB_PTS_DTS_DTS_32_MASK        (0x00000001)
#define BAPE_ITB_PTS_DTS_DTS_32_SHIFT       (0)
#define BAPE_ITB_PTS_DTS_PTS_WORD           (1)
#define BAPE_ITB_PTS_DTS_PTS_MASK           (0xFFFFFFFF)
#define BAPE_ITB_PTS_DTS_PTS_SHIFT          (0)
#define BAPE_ITB_PTS_DTS_STC_UPPER_WORD     (2)
#define BAPE_ITB_PTS_DTS_STC_UPPER_MASK     (0xFFFFFFFF)
#define BAPE_ITB_PTS_DTS_STC_UPPER_SHIFT    (0)
#define BAPE_ITB_PTS_DTS_STC_LOWER_WORD     (3)
#define BAPE_ITB_PTS_DTS_STC_LOWER_MASK     (0xFFFFFFFF)
#define BAPE_ITB_PTS_DTS_STC_LOWER_SHIFT    (0)

/* BIT_RATE fields */
#define BAPE_ITB_BIT_RATE_SHR_WORD               (1)
#define BAPE_ITB_BIT_RATE_SHR_MASK               (0xFFFF0000)
#define BAPE_ITB_BIT_RATE_SHR_SHIFT              (16)
#define BAPE_ITB_BIT_RATE_TICKS_PER_BIT_WORD     (1)
#define BAPE_ITB_BIT_RATE_TICKS_PER_BIT_MASK     (0x0000FFFF)
#define BAPE_ITB_BIT_RATE_TICKS_PER_BIT_SHIFT    (0)
#define BAPE_ITB_BIT_RATE_SAMPLE_RATE_WORD       (2)
#define BAPE_ITB_BIT_RATE_SAMPLE_RATE_MASK       (0xFFFFFFFF)
#define BAPE_ITB_BIT_RATE_SAMPLE_RATE_SHIFT      (0)

/* ESCR_METADATA fields */
#define BAPE_ITB_ESCR_METADATA_ESCR_WORD         (1)
#define BAPE_ITB_ESCR_METADATA_ESCR_MASK         (0xFFFFFFFF)
#define BAPE_ITB_ESCR_METADATA_ESCR_SHIFT        (0)
#define BAPE_ITB_ESCR_METADATA_ORIGINAL_PTS_WORD    (2)
#define BAPE_ITB_ESCR_METADATA_ORIGINAL_PTS_MASK    (0xFFFFFFFF)
#define BAPE_ITB_ESCR_METADATA_ORIGINAL_PTS_SHIFT   (0)
#define BAPE_ITB_ESCR_METADATA_ORIGINAL_PTS_VALID_WORD    (3)
#define BAPE_ITB_ESCR_METADATA_ORIGINAL_PTS_VALID_MASK    (0x80000000)
#define BAPE_ITB_ESCR_METADATA_ORIGINAL_PTS_VALID_SHIFT   (31)

/* ALGO_INFO fields */
#define BAPE_ITB_ALGO_INFO_ALGO_ID_WORD         (0)
#define BAPE_ITB_ALGO_INFO_ALGO_ID_MASK         (0x000000FF)
#define BAPE_ITB_ALGO_INFO_ALGO_ID_SHIFT        (0)
#define BAPE_ITB_ALGO_INFO_HRD_SIZE_WORD        (1)
#define BAPE_ITB_ALGO_INFO_HRD_SIZE_MASK        (0xFFFF0000)
#define BAPE_ITB_ALGO_INFO_HRD_SIZE_SHIFT       (16)
#define BAPE_ITB_ALGO_INFO_CHANNEL_CONFIG_WORD  (1)
#define BAPE_ITB_ALGO_INFO_CHANNEL_CONFIG_MASK  (0x0000FF00)
#define BAPE_ITB_ALGO_INFO_CHANNEL_CONFIG_SHIFT (8)
#define BAPE_ITB_ALGO_INFO_BITS_PER_SAMPLE_WORD  (1)
#define BAPE_ITB_ALGO_INFO_BITS_PER_SAMPLE_MASK  (0x000000FF)
#define BAPE_ITB_ALGO_INFO_BITS_PER_SAMPLE_SHIFT (0)
#define BAPE_ITB_ALGO_INFO_BIT_RATE_WORD         (2)
#define BAPE_ITB_ALGO_INFO_BIT_RATE_MASK         (0xFFFFFFFF)
#define BAPE_ITB_ALGO_INFO_BIT_RATE_SHIFT        (0)
#define BAPE_ITB_ALGO_INFO_MAX_BIT_RATE_WORD     (3)
#define BAPE_ITB_ALGO_INFO_MAX_BIT_RATE_MASK     (0xFFFFFFFF)
#define BAPE_ITB_ALGO_INFO_MAX_BIT_RATE_SHIFT    (0)

/* ASC fields */
#define BAPE_ITB_ASC_NUM_ENTRIES_WORD            (0)
#define BAPE_ITB_ASC_NUM_ENTRIES_MASK            (0x0000FF00)
#define BAPE_ITB_ASC_NUM_ENTRIES_SHIFT           (8)
#define BAPE_ITB_ASC_ENTRY_NUMBER_WORD           (0)
#define BAPE_ITB_ASC_ENTRY_NUMBER_MASK           (0x000000FF)
#define BAPE_ITB_ASC_ENTRY_NUMBER_SHIFT          (0)
#define BAPE_ITB_ASC_ASC_SIZE_WORD               (1)
#define BAPE_ITB_ASC_ASC_SIZE_MASK               (0xFFFF0000)
#define BAPE_ITB_ASC_ASC_SIZE_SHIFT              (16)
#define BAPE_ITB_ASC_ASC0_WORD                   (1)
#define BAPE_ITB_ASC_ASC0_MASK                   (0x0000FFFF)
#define BAPE_ITB_ASC_ASC0_SHIFT                  (0)
#define BAPE_ITB_ASC_ASC1_WORD                   (2)
#define BAPE_ITB_ASC_ASC1_MASK                   (0xFFFFFFFF)
#define BAPE_ITB_ASC_ASC1_SHIFT                  (0)
#define BAPE_ITB_ASC_ASC2_WORD                   (3)
#define BAPE_ITB_ASC_ASC2_MASK                   (0xFFFFFFFF)
#define BAPE_ITB_ASC_ASC2_SHIFT                  (0)

/* WMA_SPECIFIC fields */
#define BAPE_ITB_WMA_SPECIFIC_NUM_ENTRIES_WORD            (0)
#define BAPE_ITB_WMA_SPECIFIC_NUM_ENTRIES_MASK            (0x0000FF00)
#define BAPE_ITB_WMA_SPECIFIC_NUM_ENTRIES_SHIFT           (8)
#define BAPE_ITB_WMA_SPECIFIC_ENTRY_NUMBER_WORD           (0)
#define BAPE_ITB_WMA_SPECIFIC_ENTRY_NUMBER_MASK           (0x000000FF)
#define BAPE_ITB_WMA_SPECIFIC_ENTRY_NUMBER_SHIFT          (0)
#define BAPE_ITB_WMA_SPECIFIC_ENCODE_OPTION_WORD          (1)
#define BAPE_ITB_WMA_SPECIFIC_ENCODE_OPTION_MASK          (0x0000FFFF)
#define BAPE_ITB_WMA_SPECIFIC_ENCODE_OPTION_SHIFT         (0)
#define BAPE_ITB_WMA_SPECIFIC_SAMPLES_PER_FRAME_WORD      (2)
#define BAPE_ITB_WMA_SPECIFIC_SAMPLES_PER_FRAME_MASK      (0xFFFF0000)
#define BAPE_ITB_WMA_SPECIFIC_SAMPLES_PER_FRAME_SHIFT     (16)
#define BAPE_ITB_WMA_SPECIFIC_BLOCK_ALIGN_WORD            (2)
#define BAPE_ITB_WMA_SPECIFIC_BLOCK_ALIGN_MASK            (0x0000FFFF)
#define BAPE_ITB_WMA_SPECIFIC_BLOCK_ALIGN_SHIFT           (0)
#define BAPE_ITB_WMA_SPECIFIC_SUPER_BLOCK_ALIGN_WORD      (3)
#define BAPE_ITB_WMA_SPECIFIC_SUPER_BLOCK_ALIGN_MASK      (0xFFFFFFFF)
#define BAPE_ITB_WMA_SPECIFIC_SUPER_BLOCK_ALIGN_SHIFT     (0)

/* METADATA fields */
#define BAPE_ITB_METADATA_STC_UPPER_WORD            (0)
#define BAPE_ITB_METADATA_STC_UPPER_MASK            (0x000003FF)
#define BAPE_ITB_METADATA_STC_UPPER_SHIFT           (0)
#define BAPE_ITB_METADATA_STC_LOWER_WORD            (1)
#define BAPE_ITB_METADATA_STC_LOWER_MASK            (0xFFFFFFFF)
#define BAPE_ITB_METADATA_STC_LOWER_SHIFT           (0)

static BERR_Code BAPE_MuxOutput_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_MuxOutput_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_MuxOutput_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_MuxOutput_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);
static void BAPE_MuxOutput_P_ParseItb(BAPE_MuxOutputHandle hMuxOutput, BAPE_FrameItbEntries **pCurrent, BAPE_FrameItbEntries **pNext, BAVC_AudioMetadataDescriptor *pMetadataDescriptor, BAPE_ItbEntry **pMetadata);
static void BAPE_MuxOutput_P_CopyToHost(void *pDest, void *pSource, size_t numBytes, BMMA_Block_Handle mmaBlock);
static BERR_Code BAPE_MuxOutput_P_AllocateStageResources(BAPE_MuxOutputHandle hMuxOutput, BDSP_ContextHandle dspContext, unsigned dspIndex);
static void BAPE_MuxOutput_P_DestroyStageResources(BAPE_MuxOutputHandle hMuxOutput);
static void BAPE_MuxOutput_P_GetUpstreamDeviceDetails(BAPE_PathConnection * pConnection, unsigned * pDspIndex, BDSP_ContextHandle * pDspContext);

static BERR_Code BAPE_MuxOutput_P_ParseAdtsMetadata(
    BAPE_MuxOutputHandle hMuxOutput,
    BAVC_AudioBufferDescriptor *pFrameDescriptor,
    BAVC_AudioMetadataDescriptor *pMetadataDescriptor
    );
static void BAPE_MuxOutput_P_ParseAdtsSegments(
    BAPE_MuxOutputHandle hMuxOutput,
    const uint8_t *pBuffer,
    size_t bufferLength,
    const uint8_t *pWrapBuffer,
    size_t wrapBufferLength
    );

/***************************************************************************
Summary:
    Get default settings for a MuxOutput stage
***************************************************************************/
void BAPE_MuxOutput_GetDefaultCreateSettings(
    BAPE_MuxOutputCreateSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->numDescriptors = BAPE_MUXOUTPUT_MAX_ITBDESCRIPTORS;
    pSettings->dspIndex = BAPE_INVALID_DEVICE_INDEX;
}

/***************************************************************************
Summary:
    Open a MuxOutput stage
***************************************************************************/
BERR_Code BAPE_MuxOutput_Create(
    BAPE_Handle hApe,
    const BAPE_MuxOutputCreateSettings *pSettings,
    BAPE_MuxOutputHandle *pHandle
    )
{
    BAPE_MuxOutputHandle hMuxOutput;
    BERR_Code errCode;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    unsigned dspIndex = BAPE_DEVICE_INVALID;
    unsigned dspIndexBase = BAPE_DEVICE_INVALID;
    BDSP_ContextHandle dspContext = NULL;
    size_t tempSize;

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);

    if ( pSettings->numDescriptors == 0 )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hMuxOutput = BKNI_Malloc(sizeof(BAPE_MuxOutput));
    if ( NULL == hMuxOutput )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(hMuxOutput, 0, sizeof(BAPE_MuxOutput));
    BDBG_OBJECT_SET(hMuxOutput, BAPE_MuxOutput);
    hMuxOutput->deviceHandle = hApe;
    hMuxOutput->dspIndex = BAPE_DEVICE_INVALID;
    hMuxOutput->createSettings = *pSettings;

    if ( pSettings->dspIndex != BAPE_DEVICE_INVALID )
    {
        dspIndex = pSettings->dspIndex;
    }
    else
    {
        dspIndex = BAPE_DEVICE_DSP_FIRST;
    }

    /* Shift the dsp index to the arm if dsp is not enabled, but arm audio is*/
    if ( dspIndex <= BAPE_DEVICE_DSP_LAST &&
         hMuxOutput->deviceHandle->dspHandle == NULL && hMuxOutput->deviceHandle->armHandle != NULL )
    {
        dspIndex = BAPE_DEVICE_ARM_FIRST;
    }

    if ( dspIndex >= BAPE_DEVICE_ARM_FIRST &&
         ( dspIndex >= (BAPE_DEVICE_ARM_FIRST + hMuxOutput->deviceHandle->numArms) ||
           hMuxOutput->deviceHandle->armHandle == NULL ) )
    {
        BDBG_ERR(("ARM device index %u is not available.  This system has %u ARM Audio Processors, arm handle %p.", dspIndex, hMuxOutput->deviceHandle->numArms, (void*)hMuxOutput->deviceHandle->armHandle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( dspIndex <= BAPE_DEVICE_DSP_LAST && (dspIndex >= hMuxOutput->deviceHandle->numDsps || hMuxOutput->deviceHandle->dspHandle == NULL ) )
    {
        BDBG_ERR(("DSP %u is not available.  This system has %u DSPs, dsp handle %p.", dspIndex, hMuxOutput->deviceHandle->numDsps, (void*)hMuxOutput->deviceHandle->dspHandle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( dspIndex >= BAPE_DEVICE_ARM_FIRST )
    {
        dspContext = hMuxOutput->deviceHandle->armContext;
        dspIndexBase = BAPE_DEVICE_ARM_FIRST;
    }
    else
    {
        dspContext = hMuxOutput->deviceHandle->dspContext;
        dspIndexBase = BAPE_DEVICE_DSP_FIRST;
    }

    if ( dspContext == NULL )
    {
        BDBG_ERR(("No DSP or ARM device available."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hMuxOutput->dspIndex = dspIndex;
    hMuxOutput->dspIndexBase = dspIndexBase;
    hMuxOutput->dspContext = dspContext;

    /* Technically, this has no output paths, but unless you set 1 below you get a compiler warning */
    BAPE_P_InitPathNode(&hMuxOutput->node, BAPE_PathNodeType_eMuxOutput, 0, 1, hApe, hMuxOutput);
    hMuxOutput->node.pName = "MuxOutput";
    hMuxOutput->state = BAPE_MuxOutputState_Init;

    BAPE_Connector_P_GetFormat(&hMuxOutput->node.connectors[0], &format);
    format.sampleRate = 0;
    if (!pSettings->useRDB)
    {
        format.source = BAPE_DataSource_eRave;
        format.type = BAPE_DataType_eRave;
    }
    else
    {
        format.source = BAPE_DataSource_eRdb;
        format.type = BAPE_DataType_eRdb;
    }
    errCode = BAPE_Connector_P_SetFormat(&hMuxOutput->node.connectors[0], &format);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_format; }

    BAPE_PathNode_P_GetInputCapabilities(&hMuxOutput->node, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x4);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x16);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eCompressedRaw);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&hMuxOutput->node, &caps);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_caps; }

    /* MuxOutput Specifics */
    hMuxOutput->node.allocatePathFromInput = BAPE_MuxOutput_P_AllocatePathFromInput;
    hMuxOutput->node.stopPathFromInput = BAPE_MuxOutput_P_StopPathFromInput;
    hMuxOutput->node.freePathFromInput = BAPE_MuxOutput_P_FreePathFromInput;
    hMuxOutput->node.removeInput = BAPE_MuxOutput_P_RemoveInputCallback;

    if (!pSettings->useRDB)
    {
        /* Save RAVE Context */
        BKNI_Memcpy(&hMuxOutput->contextMap, pSettings->pContextMap, sizeof(BAVC_XptContextMap));
        hMuxOutput->createSettings.pContextMap = &hMuxOutput->contextMap;
    }

    hMuxOutput->cdb.offset = BMMA_LockOffset(hMuxOutput->createSettings.cdb.block);
    if ( !hMuxOutput->cdb.offset )
    {
        errCode = BERR_TRACE(BERR_UNKNOWN);
        goto err_cdb_cached;
    }
    hMuxOutput->cdb.cached = BMMA_Lock(hMuxOutput->createSettings.cdb.block);
    if ( !hMuxOutput->cdb.cached )
    {
        errCode = BERR_TRACE(BERR_UNKNOWN);
        goto err_cdb_cached;
    }
    BMMA_FlushCache_isrsafe(hMuxOutput->createSettings.cdb.block, hMuxOutput->cdb.cached, pSettings->cdb.size);

    hMuxOutput->itb.offset = BMMA_LockOffset(hMuxOutput->createSettings.itb.block);
    if ( !hMuxOutput->itb.offset )
    {
        errCode = BERR_TRACE(BERR_UNKNOWN);
        goto err_cdb_cached;
    }
    hMuxOutput->itb.cached = BMMA_Lock(hMuxOutput->createSettings.itb.block);
    if ( !hMuxOutput->itb.cached )
    {
        errCode = BERR_TRACE(BERR_UNKNOWN);
        goto err_cdb_cached;
    }

    BMMA_FlushCache_isrsafe(hMuxOutput->createSettings.itb.block, hMuxOutput->itb.cached, pSettings->itb.size);

    /* Reset shadow pointers */
    hMuxOutput->descriptorInfo.uiCDBBufferShadowReadOffset = hMuxOutput->cdb.offset;
    hMuxOutput->descriptorInfo.uiITBBufferShadowReadOffset = hMuxOutput->itb.offset;

    tempSize = sizeof(BAVC_AudioBufferDescriptor)*pSettings->numDescriptors;
    BDSP_SIZE_ALIGN(tempSize);
    hMuxOutput->descriptorInfo.descriptors.block = BMMA_Alloc(hMuxOutput->createSettings.heaps.descriptor, tempSize, BDSP_ADDRESS_ALIGN, NULL);
    if ( NULL == hMuxOutput->descriptorInfo.descriptors.block )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_alloc_descriptors;
    }
    hMuxOutput->descriptorInfo.descriptors.cached = BMMA_Lock(hMuxOutput->descriptorInfo.descriptors.block);
    if ( !hMuxOutput->descriptorInfo.descriptors.cached )
    {
        errCode = BERR_TRACE(BERR_UNKNOWN);
        goto err_cache_descriptors;
    }

    tempSize = sizeof(BAVC_AudioMetadataDescriptor)*BAPE_MUXOUTPUT_MAX_METADATADESCRIPTORS;
    BDSP_SIZE_ALIGN(tempSize);
    hMuxOutput->descriptorInfo.metadata.block = BMMA_Alloc(hMuxOutput->createSettings.heaps.descriptor, tempSize, BDSP_ADDRESS_ALIGN, NULL);
    if ( NULL == hMuxOutput->descriptorInfo.metadata.block )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_alloc_metadata;
    }
    hMuxOutput->descriptorInfo.metadata.cached = BMMA_Lock(hMuxOutput->descriptorInfo.metadata.block);
    if ( !hMuxOutput->descriptorInfo.metadata.cached )
    {
        errCode = BERR_TRACE(BERR_UNKNOWN);
        goto err_cache_metadata;
    }

    BMMA_FlushCache_isrsafe(hMuxOutput->descriptorInfo.metadata.block, hMuxOutput->descriptorInfo.metadata.cached, sizeof(BAVC_AudioMetadataDescriptor)*BAPE_MUXOUTPUT_MAX_METADATADESCRIPTORS);

    #if 0
    /* Create Stage Handle */
    BDSP_Stage_GetDefaultCreateSettings(hApe->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eGenCdbItb] = true;
    errCode = BDSP_Stage_Create(hApe->dspContext, &stageCreateSettings, &hMuxOutput->hStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage_create;
    }
    hMuxOutput->node.connectors[0].hStage = hMuxOutput->hStage;
    #endif

    BLST_S_INSERT_HEAD(&hApe->muxOutputList, hMuxOutput, deviceListNode);

    *pHandle = hMuxOutput;
    return BERR_SUCCESS;

err_cache_metadata:
    BMMA_Free(hMuxOutput->descriptorInfo.metadata.block);
err_alloc_metadata:
    BMMA_Unlock(hMuxOutput->descriptorInfo.descriptors.block, hMuxOutput->descriptorInfo.descriptors.cached);
err_cache_descriptors:
    BMMA_Free(hMuxOutput->descriptorInfo.descriptors.block);
err_alloc_descriptors:
err_cdb_cached:
err_caps:
err_format:
    BDBG_OBJECT_DESTROY(hMuxOutput, BAPE_MuxOutput);
    BKNI_Free(hMuxOutput);

    return errCode;
}

BERR_Code BAPE_MuxOutput_P_AllocateStageResources(BAPE_MuxOutputHandle hMuxOutput, BDSP_ContextHandle dspContext, unsigned dspIndex)
{
    BERR_Code errCode;
    bool allocate = true;
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);

    if ( hMuxOutput->hStage && hMuxOutput->cdb.queue && hMuxOutput->itb.queue )
    {
        allocate = false;

        /* check existing resources for device compatibility */
        if ( dspContext != hMuxOutput->allocatedDspContext || dspIndex != hMuxOutput->allocatedDspIndex )
        {
            allocate = true;
        }
    }

    if ( allocate )
    {
        BDSP_StageCreateSettings stageCreateSettings;
        unsigned output, tmp;

        BDBG_MSG(("Allocate/Re-Allocate DSP Mux Resources"));

        /* cleanup any remaining resources */
        BAPE_MuxOutput_P_DestroyStageResources(hMuxOutput);

        /* Allocate Queues */
        if (hMuxOutput->createSettings.useRDB)
        {
            BDSP_QueueCreateSettings queueSettings;
            uint32_t numBuffers = 1;

            /* CDB */
            BDSP_Queue_GetDefaultSettings(dspContext, &queueSettings);
            queueSettings.dataType = BDSP_DataType_eRdbCdb;
            queueSettings.numBuffers = numBuffers;
            queueSettings.bufferInfo[0].bufferSize = hMuxOutput->createSettings.cdb.size;
            queueSettings.bufferInfo[0].buffer.hBlock = hMuxOutput->createSettings.cdb.block;
            queueSettings.bufferInfo[0].buffer.offset = hMuxOutput->cdb.offset;
            queueSettings.bufferInfo[0].buffer.pAddr = hMuxOutput->cdb.cached;

            BDBG_MSG(("Create CDB Queue, DSP cxt %p, idx %d", (void*)dspContext, (int)dspIndex));
            errCode = BDSP_Queue_Create(dspContext, dspIndex, &queueSettings, &hMuxOutput->cdb.queue);
            if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

            /* ITB */
            BDSP_Queue_GetDefaultSettings(dspContext, &queueSettings);
            queueSettings.dataType = BDSP_DataType_eRdbItb;
            queueSettings.numBuffers = numBuffers;
            queueSettings.bufferInfo[0].bufferSize = hMuxOutput->createSettings.itb.size;
            queueSettings.bufferInfo[0].buffer.hBlock = hMuxOutput->createSettings.itb.block;
            queueSettings.bufferInfo[0].buffer.offset = hMuxOutput->itb.offset;
            queueSettings.bufferInfo[0].buffer.pAddr = hMuxOutput->itb.cached;

            BDBG_MSG(("Create ITB Queue, DSP cxt %p, idx %d", (void*)dspContext, (int)dspIndex));
            errCode = BDSP_Queue_Create(dspContext, dspIndex, &queueSettings, &hMuxOutput->itb.queue);
            if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

            errCode = BDSP_Queue_GetBufferAddr(hMuxOutput->cdb.queue, numBuffers, &hMuxOutput->cdb.buffer);
            if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

            errCode = BDSP_Queue_GetBufferAddr(hMuxOutput->itb.queue, numBuffers, &hMuxOutput->itb.buffer);
            if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

            BDSP_Queue_Flush(hMuxOutput->cdb.queue);
            BDSP_Queue_Flush(hMuxOutput->itb.queue);

            hMuxOutput->cdb.bufferInterface.base = hMuxOutput->cdb.buffer.ui32BaseAddr;
            hMuxOutput->cdb.bufferInterface.end = hMuxOutput->cdb.buffer.ui32EndAddr;
            hMuxOutput->cdb.bufferInterface.read = hMuxOutput->cdb.buffer.ui32ReadAddr;
            hMuxOutput->cdb.bufferInterface.valid = hMuxOutput->cdb.buffer.ui32WriteAddr;
            hMuxOutput->cdb.bufferInterface.inclusive = false;
            hMuxOutput->itb.bufferInterface.base = hMuxOutput->itb.buffer.ui32BaseAddr;
            hMuxOutput->itb.bufferInterface.end = hMuxOutput->itb.buffer.ui32EndAddr;
            hMuxOutput->itb.bufferInterface.read = hMuxOutput->itb.buffer.ui32ReadAddr;
            hMuxOutput->itb.bufferInterface.valid = hMuxOutput->itb.buffer.ui32WriteAddr;
            hMuxOutput->itb.bufferInterface.inclusive = false;
        }
        else
        {
            hMuxOutput->cdb.bufferInterface.base = hMuxOutput->contextMap.CDB_Base;
            hMuxOutput->cdb.bufferInterface.end = hMuxOutput->contextMap.CDB_End;
            hMuxOutput->cdb.bufferInterface.read = hMuxOutput->contextMap.CDB_Read;
            hMuxOutput->cdb.bufferInterface.valid = hMuxOutput->contextMap.CDB_Valid;
            hMuxOutput->cdb.bufferInterface.inclusive = true;
            hMuxOutput->itb.bufferInterface.base = hMuxOutput->contextMap.ITB_Base;
            hMuxOutput->itb.bufferInterface.end = hMuxOutput->contextMap.ITB_End;
            hMuxOutput->itb.bufferInterface.read = hMuxOutput->contextMap.ITB_Read;
            hMuxOutput->itb.bufferInterface.valid = hMuxOutput->contextMap.ITB_Valid;
            hMuxOutput->itb.bufferInterface.inclusive = true;
        }

        /* Update shadow pointers for context to the current read pointer. */
        hMuxOutput->descriptorInfo.uiCDBBufferShadowReadOffset = BREG_ReadAddr(hMuxOutput->node.deviceHandle->regHandle, hMuxOutput->cdb.bufferInterface.read);
        hMuxOutput->descriptorInfo.uiITBBufferShadowReadOffset = BREG_ReadAddr(hMuxOutput->node.deviceHandle->regHandle, hMuxOutput->itb.bufferInterface.read);

        /* Reset Descriptor Offsets in case mux didn't consume all previous descriptors */
        hMuxOutput->descriptorInfo.uiDescriptorWriteOffset = 0;
        hMuxOutput->descriptorInfo.uiDescriptorReadOffset = 0;
        hMuxOutput->descriptorInfo.numOutstandingDescriptors = 0;

        /* Create Stage and Make connections */
        BDSP_Stage_GetDefaultCreateSettings(dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
        BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
        stageCreateSettings.algorithmSupported[BDSP_Algorithm_eGenCdbItb] = true;
        errCode = BDSP_Stage_Create(dspContext, &stageCreateSettings, &hMuxOutput->hStage);
        if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

        hMuxOutput->node.connectors[0].hStage = hMuxOutput->hStage;

        if (!hMuxOutput->createSettings.useRDB)
        {
            errCode = BDSP_Stage_AddRaveOutput(hMuxOutput->hStage, hMuxOutput->createSettings.pContextMap, &output);
            if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }
        }
        else
        {
            errCode = BDSP_Stage_AddQueueOutput(hMuxOutput->hStage, hMuxOutput->cdb.queue, &tmp);
            if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

            errCode = BDSP_Stage_AddQueueOutput(hMuxOutput->hStage, hMuxOutput->itb.queue, &tmp);
            if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }
        }

        hMuxOutput->allocatedDspContext = dspContext;
        hMuxOutput->allocatedDspIndex = dspIndex;
    }

    return BERR_SUCCESS;

cleanup:
    BAPE_MuxOutput_P_DestroyStageResources(hMuxOutput);
    return errCode;
}

void BAPE_MuxOutput_P_DestroyStageResources(BAPE_MuxOutputHandle hMuxOutput)
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    if ( hMuxOutput->hStage )
    {
        BDSP_Stage_RemoveAllInputs(hMuxOutput->hStage);
        BDSP_Stage_RemoveAllOutputs(hMuxOutput->hStage);
        BDSP_Stage_Destroy(hMuxOutput->hStage);
        hMuxOutput->hStage = NULL;
    }
    if( hMuxOutput->cdb.queue )
    {
        BDSP_Queue_Destroy(hMuxOutput->cdb.queue);
        hMuxOutput->cdb.queue = NULL;
    }
    if ( hMuxOutput->itb.queue )
    {
        BDSP_Queue_Destroy(hMuxOutput->itb.queue);
        hMuxOutput->itb.queue = NULL;
    }
    hMuxOutput->node.connectors[0].hStage = NULL;
    hMuxOutput->allocatedDspContext = NULL;
    hMuxOutput->allocatedDspIndex = 0;
}

/***************************************************************************
Summary:
    Close a MuxOutput stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_MuxOutput_Destroy(
    BAPE_MuxOutputHandle hMuxOutput
    )
{
    bool running;
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    running = BAPE_PathNode_P_IsActive(&hMuxOutput->node);
    BDBG_ASSERT(false == running);
    BDBG_ASSERT(NULL == hMuxOutput->input);
    BAPE_MuxOutput_P_DestroyStageResources(hMuxOutput);
    BLST_S_REMOVE(&hMuxOutput->node.deviceHandle->muxOutputList, hMuxOutput, BAPE_MuxOutput, deviceListNode);
    BMMA_Unlock(hMuxOutput->descriptorInfo.metadata.block, hMuxOutput->descriptorInfo.metadata.cached);
    BMMA_Free(hMuxOutput->descriptorInfo.metadata.block);
    BMMA_Unlock(hMuxOutput->descriptorInfo.descriptors.block, hMuxOutput->descriptorInfo.descriptors.cached);
    BMMA_Free(hMuxOutput->descriptorInfo.descriptors.block);
    BMMA_UnlockOffset(hMuxOutput->createSettings.cdb.block, hMuxOutput->cdb.offset);
    BMMA_Unlock(hMuxOutput->createSettings.cdb.block, hMuxOutput->cdb.cached);
    BMMA_UnlockOffset(hMuxOutput->createSettings.itb.block, hMuxOutput->itb.offset);
    BMMA_Unlock(hMuxOutput->createSettings.itb.block, hMuxOutput->itb.cached);

    BDBG_OBJECT_DESTROY(hMuxOutput, BAPE_MuxOutput);
    BKNI_Free(hMuxOutput);
}

/***************************************************************************
Summary:
    Get Default Start-time Settings for a MuxOutput object
***************************************************************************/
void BAPE_MuxOutput_GetDefaultStartSettings(
    BAPE_MuxOutputStartSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static BERR_Code BAPE_MuxOutput_P_ApplyDspSettings(
    BAPE_MuxOutputHandle hMuxOutput
    )
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_GenCdbItbConfigParams userConfig;

    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);

    if ( hMuxOutput->hStage )
    {
        errCode = BDSP_Stage_GetSettings(hMuxOutput->hStage, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        userConfig.eEnableEncode = hMuxOutput->state == BAPE_MuxOutputState_Started ? BDSP_AF_P_eEnable : BDSP_AF_P_eDisable;
        userConfig.ui32EncSTCAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_CHIP_GET_STC_ADDRESS(hMuxOutput->startSettings.stcIndex) );
        #if BAPE_CHIP_HAS_42BIT_STC
        userConfig.ui32EncSTCAddrUpper = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_CHIP_GET_STC_UPPER_ADDRESS(hMuxOutput->startSettings.stcIndex) );
        #endif
        userConfig.ui32A2PInMilliSeconds = hMuxOutput->startSettings.presentationDelay;
        userConfig.eSnapshotRequired = ( hMuxOutput->nonRealTimeIncrement.StcIncLo == 0 &&
                                         hMuxOutput->nonRealTimeIncrement.StcIncHi == 0 &&
                                         hMuxOutput->nonRealTimeIncrement.IncTrigger == 0 ) ? BDSP_AF_P_eEnable /* RT */ : BDSP_AF_P_eDisable /* NRT */;
        userConfig.eStcItbEntryEnable = BDSP_AF_P_eEnable;  /* Always enable the per-frame STC ITB entry */

        errCode = BDSP_Stage_SetSettings(hMuxOutput->hStage, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Start a MuxOutput Object

Description:
    Typically, this is called prior to BAPE_Decoder_Start() in order to
    guarantee all data is captured.

See Also:
    BAPE_MuxOutput_GetDefaultStartSettings
    BAPE_MuxOutput_Stop
***************************************************************************/
BERR_Code BAPE_MuxOutput_Start(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAPE_MuxOutputStartSettings *pSettings
    )
{
    unsigned dspIndex = BAPE_INVALID_DEVICE_INDEX;
    BDSP_ContextHandle dspContext = NULL;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);

    if ( hMuxOutput->state == BAPE_MuxOutputState_Started )
    {
        BDBG_ERR(("Already started."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( NULL == pSettings )
    {
        BAPE_MuxOutput_GetDefaultStartSettings(&hMuxOutput->startSettings);
        pSettings = &hMuxOutput->startSettings;
    }
    else
    {
        if ( pSettings->stcIndex >= BAPE_CHIP_MAX_STCS )
        {
            BDBG_ERR(("STC Index %u out of range.  Supported values are 0..%u", pSettings->stcIndex, BAPE_CHIP_MAX_STCS-1));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        BKNI_Memcpy(&hMuxOutput->startSettings, pSettings, sizeof(BAPE_MuxOutputStartSettings));
    }

    if ( NULL != pSettings->pNonRealTimeIncrement && BAPE_PathNode_P_IsActive(&hMuxOutput->node) )
    {
        BDBG_ERR(("To configure for Non-Realtime Transcode, BAPE_MuxOutput_Start must be called prior to starting transcoder input."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else if ( NULL != pSettings->pNonRealTimeIncrement )
    {
        if ( pSettings->pNonRealTimeIncrement->StcIncLo == 0 ||
             pSettings->pNonRealTimeIncrement->StcIncHi == 0 ||
             pSettings->pNonRealTimeIncrement->IncTrigger == 0 )
        {
            BDBG_ERR(("NRT was requested but an increment value was 0(StcIncLo 0x%x, StcIncHi 0x%x, IncTrigger 0x%x",
                pSettings->pNonRealTimeIncrement->StcIncLo,pSettings->pNonRealTimeIncrement->StcIncHi,pSettings->pNonRealTimeIncrement->IncTrigger));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        hMuxOutput->nonRealTimeIncrement.StcIncLo = pSettings->pNonRealTimeIncrement->StcIncLo;
        hMuxOutput->nonRealTimeIncrement.StcIncHi = pSettings->pNonRealTimeIncrement->StcIncHi;
        hMuxOutput->nonRealTimeIncrement.IncTrigger = pSettings->pNonRealTimeIncrement->IncTrigger;
    }
    else if (NULL == pSettings->pNonRealTimeIncrement )
    {
        hMuxOutput->nonRealTimeIncrement.StcIncLo = 0;
        hMuxOutput->nonRealTimeIncrement.StcIncHi = 0;
        hMuxOutput->nonRealTimeIncrement.IncTrigger = 0;
    }

    BDBG_MSG(("BAPE_MuxOutput_Start"));

    /* look for device index from upstream node */
    BAPE_MuxOutput_P_GetUpstreamDeviceDetails(BLST_S_FIRST(&hMuxOutput->node.upstreamList), &dspIndex, &dspContext);

    /* Use "default" device index determined curing _Create() */
    if ( dspIndex == BAPE_DEVICE_INVALID || dspContext == NULL )
    {
        dspIndex = hMuxOutput->dspIndex;
        dspContext = hMuxOutput->dspContext;
        BDBG_WRN(("Unable to find DSP index and context from upstream. Defaulting to DSP idx %d, context %p", (int)dspIndex, (void*)dspContext));
    }

    if ( dspIndex == BAPE_DEVICE_INVALID || dspContext == NULL )
    {
        BDBG_ERR(("Unable find an available device for mux."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    errCode = BAPE_MuxOutput_P_AllocateStageResources(hMuxOutput, dspContext, dspIndex);
    if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

    /* Start */
    hMuxOutput->state = BAPE_MuxOutputState_Started;
    hMuxOutput->sendEos = false;
    hMuxOutput->sendMetadata = true;

    return BERR_SUCCESS;
cleanup:
    BAPE_MuxOutput_P_DestroyStageResources(hMuxOutput);
    return errCode;
}

/***************************************************************************
Summary:
    Stop a MuxOutput Object

Description:
    Typically, this is called after BAPE_Decoder_Stop() in order to
    guarantee all data is captured.

See Also:
    BAPE_MuxOutput_Start
***************************************************************************/
void BAPE_MuxOutput_Stop(
    BAPE_MuxOutputHandle hMuxOutput
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);

    if ( hMuxOutput->state != BAPE_MuxOutputState_Started )
    {
        return;
    }

    BDBG_MSG(("BAPE_MuxOutput_Stop"));
    hMuxOutput->state = BAPE_MuxOutputState_Stopped;
    hMuxOutput->sendEos = true;

    errCode = BAPE_MuxOutput_P_ApplyDspSettings(hMuxOutput);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return;
    }
}

BERR_Code BAPE_MuxOutput_GetBufferDescriptors(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAVC_AudioBufferDescriptor **pBuffer, /* [out] pointer to BAVC_AudioBufferDescriptor structs */
    size_t *pSize, /* [out] number of BAVC_AudioBufferDescriptor elements in pBuffer */
    const BAVC_AudioBufferDescriptor **pBuffer2, /* [out] pointer to BAVC_AudioBufferDescriptor structs after wrap around */
    size_t *pSize2 /* [out] number of BAVC_AudioBufferDescriptor elements in pBuffer2 */
    )
{
    BREG_Handle hReg;
    BAPE_FrameItbEntries *pITBEntry;
    BAPE_FrameItbEntries *pITBEntryNext = NULL;
    BAPE_ItbEntry *pITBEntryMetadata = NULL;
    BAVC_AudioBufferDescriptor *pAudioDescriptor;
    BMMA_DeviceOffset uiCDBBaseOffset;
    BMMA_DeviceOffset uiCDBEndOffset;
    BMMA_DeviceOffset uiCDBValidOffset;
    BMMA_DeviceOffset uiCDBReadOffset;
    BMMA_DeviceOffset uiCDBEndOfFrameOffset;
    BMMA_DeviceOffset uiNextBase=0;
    unsigned uiTemp;
    BAPE_OutputDescriptorInfo *psOutputDescDetails;
    BAVC_AudioMetadataDescriptor stMetadata;
    BERR_Code ret = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBuffer2);
    BDBG_ASSERT(NULL != pSize);
    BDBG_ASSERT(NULL != pSize2);

    BDBG_ENTER(BAPE_MuxOutput_GetBufferDescriptors);

    hReg = hMuxOutput->node.deviceHandle->regHandle;

    psOutputDescDetails = &hMuxOutput->descriptorInfo;

    *pBuffer = NULL;
    *pBuffer2 = NULL;
    *pSize = 0;
    *pSize2 = 0;

    if (hMuxOutput->state == BAPE_MuxOutputState_Init)
    {
        return BERR_SUCCESS;
    }

    /* Read CDB Addresses */
    uiCDBBaseOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.base);
    uiCDBEndOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.end);
    uiCDBValidOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.valid);
    uiCDBReadOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.read);

    if ( uiCDBEndOffset != 0 && hMuxOutput->cdb.bufferInterface.inclusive )
    {
        uiCDBEndOffset += 1; /* end is inclusive */
    }

    BDBG_MSG(("CDB Base/End/Shadow Read/Read/Write = " BDBG_UINT64_FMT "/" BDBG_UINT64_FMT "/" BDBG_UINT64_FMT " (" BDBG_UINT64_FMT ")/" BDBG_UINT64_FMT "",
              BDBG_UINT64_ARG(uiCDBBaseOffset),
              BDBG_UINT64_ARG(uiCDBEndOffset),
              BDBG_UINT64_ARG(psOutputDescDetails->uiCDBBufferShadowReadOffset),
              BDBG_UINT64_ARG(uiCDBReadOffset),
              BDBG_UINT64_ARG(uiCDBValidOffset)
             ));

    while ( 1 )
    {
        /* Make sure input is valid */
        if (hMuxOutput->input == NULL)
        {
            BDBG_MSG(("%s hMuxOutput(%p)->input is NULL", BSTD_FUNCTION, (void *)hMuxOutput));
            break;
        }

        /* Check for Available ITB Entries */

        BAPE_MuxOutput_P_ParseItb(hMuxOutput, &pITBEntry, &pITBEntryNext, &stMetadata, &pITBEntryMetadata);
        if ( NULL == pITBEntry )
        {
            BDBG_MSG(("No more ITB Entries"));
            break;
        }

        if ( uiCDBValidOffset == psOutputDescDetails->uiCDBBufferShadowReadOffset && \
                  BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, FRAME_LENGTH) > 0 )
        {
            /* We ran out of CDB data */
            BDBG_MSG(("No more CDB Data"));
            break;
        }
        else
        {
            uint64_t uiDepthToNext;
            uint64_t uiDepthToValid;

            /* It is possible that the CDB Valid doesn't, yet, contain any of the next frame and
             * may still be in the middle of the current frame, so we need use the depth that is the
             * lesser of depth(cdb_read,cdb_next) depth(cdb_read,cdb_valid)
             */
            uiNextBase = BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, FRAME_LENGTH) +
                BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, CDB_ADDRESS);
            if ( uiNextBase >= uiCDBEndOffset )
            {
                uiNextBase -= (uiCDBEndOffset-uiCDBBaseOffset);
            }

            if ( uiNextBase >= psOutputDescDetails->uiCDBBufferShadowReadOffset )
            {
                uiDepthToNext = BAPE_ITB_GET_FIELD(&pITBEntryNext->baseAddress, BASE_ADDRESS, CDB_ADDRESS) - \
                                psOutputDescDetails->uiCDBBufferShadowReadOffset;
            }
            else
            {
                uiDepthToNext = uiCDBEndOffset - \
                                psOutputDescDetails->uiCDBBufferShadowReadOffset;
                uiDepthToNext += uiNextBase - uiCDBBaseOffset;
            }

            if ( uiCDBValidOffset >= \
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
                uiCDBEndOfFrameOffset = uiNextBase;
            }
        }

        /* Get Audio Descriptor for this ITB entry */
        uiTemp = (psOutputDescDetails->uiDescriptorWriteOffset + 1) % hMuxOutput->createSettings.numDescriptors;
        if ( uiTemp == psOutputDescDetails->uiDescriptorReadOffset )
        {
            BDBG_MSG(("Out of descriptors"));
            break;
        }
        pAudioDescriptor = &psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorWriteOffset];
        psOutputDescDetails->uiDescriptorWriteOffset = uiTemp;

        BKNI_Memset(pAudioDescriptor, 0, sizeof(BAVC_AudioBufferDescriptor));

        if ( uiCDBEndOfFrameOffset >= psOutputDescDetails->uiCDBBufferShadowReadOffset )
        {
            pAudioDescriptor->stCommon.uiLength = uiCDBEndOfFrameOffset - \
                                                  psOutputDescDetails->uiCDBBufferShadowReadOffset;
        }
        else
        {
            /* CDB Wrap occurs, so we need to split this picture into two descriptors.  We handle the first one here. */
            pAudioDescriptor->stCommon.uiLength = uiCDBEndOffset - \
                                                  psOutputDescDetails->uiCDBBufferShadowReadOffset;
        }

        /* Populate other fields iff this descriptor contains the beginning of the frame */
        if ( psOutputDescDetails->uiCDBBufferShadowReadOffset == \
             BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, CDB_ADDRESS) )
        {

            /* We're muxing the beginning of this frame */
            pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START;

            /* Determine if the original PTS is coded or interpolated from coded */
            if ( BAPE_ITB_GET_FIELD(&pITBEntry->escrMetadata, ESCR_METADATA, ORIGINAL_PTS_VALID) == 1 )
            {
               pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID;
            }

            pAudioDescriptor->stCommon.uiOriginalPTS = BAPE_ITB_GET_FIELD(&pITBEntry->escrMetadata, ESCR_METADATA, ORIGINAL_PTS);

            pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID;
            pAudioDescriptor->stCommon.uiPTS = BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, PTS_DTS, PTS_32);
            pAudioDescriptor->stCommon.uiPTS <<= 32;
            pAudioDescriptor->stCommon.uiPTS |= BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, PTS_DTS, PTS);

            pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID;
            pAudioDescriptor->stCommon.uiESCR = BAPE_ITB_GET_FIELD(&pITBEntry->escrMetadata, ESCR_METADATA, ESCR);

            pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID;
            pAudioDescriptor->stCommon.uiTicksPerBit = BAPE_ITB_GET_FIELD(&pITBEntry->bitRate, BIT_RATE, TICKS_PER_BIT);

            pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID;
            pAudioDescriptor->stCommon.iSHR = BAPE_ITB_GET_FIELD(&pITBEntry->bitRate, BIT_RATE, SHR);

            if ( 0 == BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, FRAME_VALID) )
            {
                pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME;
                BDBG_MSG(("Empty frame received in mux (len %u)", BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, FRAME_LENGTH)));
            }

            /* Parse Metadata Descriptor if found */
            if ( NULL != pITBEntryMetadata )
            {
                pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID;
                pAudioDescriptor->stCommon.uiSTCSnapshot = BAPE_ITB_GET_FIELD(pITBEntryMetadata, METADATA, STC_UPPER);
                pAudioDescriptor->stCommon.uiSTCSnapshot <<= 32;
                pAudioDescriptor->stCommon.uiSTCSnapshot |= BAPE_ITB_GET_FIELD(pITBEntryMetadata, METADATA, STC_LOWER);
                if ( hMuxOutput->nonRealTimeIncrement.StcIncLo != 0 &&
                     hMuxOutput->nonRealTimeIncrement.StcIncHi != 0 &&
                     hMuxOutput->nonRealTimeIncrement.IncTrigger != 0 )
                {
                    /* In NRT mode, we use 45kHz not 27Mhz.  Normalize to 27MHz. [27000/45 = 600 so there is no rounding error below] */
                    pAudioDescriptor->stCommon.uiSTCSnapshot = pAudioDescriptor->stCommon.uiSTCSnapshot * (27000/45);
                }
            }
        }

        /* Normalize the offset to 0 */
        pAudioDescriptor->stCommon.uiOffset = \
                                              psOutputDescDetails->uiCDBBufferShadowReadOffset - uiCDBBaseOffset;

        /* Invalidate this frame from the cache prior to the host accessing it */
        BMMA_FlushCache_isrsafe(hMuxOutput->createSettings.cdb.block,
                             (char *)hMuxOutput->cdb.cached + pAudioDescriptor->stCommon.uiOffset,
                             pAudioDescriptor->stCommon.uiLength);

        /* Set RAW data offset and length equal to frame offset and length.  When ADTS is parsed, add actual raw offset/length here */
        pAudioDescriptor->uiRawDataOffset = pAudioDescriptor->stCommon.uiOffset;
        pAudioDescriptor->uiRawDataLength = pAudioDescriptor->stCommon.uiLength;

        switch ( BAPE_FMT_P_GetAudioCompressionStd_isrsafe(&hMuxOutput->input->format) )
        {
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
            /* If Start Of Frame */
            if ( pAudioDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START )
            {
                uint8_t *pWrapBase;
                size_t wrapLength;
                if ( uiCDBEndOfFrameOffset > psOutputDescDetails->uiCDBBufferShadowReadOffset )
                {
                    pWrapBase = NULL;
                    wrapLength = 0;
                }
                else
                {
                    pWrapBase = hMuxOutput->cdb.cached;
                    wrapLength = uiCDBEndOfFrameOffset - uiCDBBaseOffset;
                    /* Make sure any wraparound data is also invalidated from the cache prior to accessing it. */
                    BMMA_FlushCache_isrsafe(hMuxOutput->createSettings.cdb.block, pWrapBase, wrapLength);
                }
                /* This function can not fail.  It will simply create a single segment equal to the
                   entire frame if a parse error occurs */
                BAPE_MuxOutput_P_ParseAdtsSegments(hMuxOutput,
                                                   (uint8_t *)hMuxOutput->cdb.cached+pAudioDescriptor->stCommon.uiOffset,
                                                   pAudioDescriptor->stCommon.uiLength,
                                                   (uint8_t *)pWrapBase,
                                                   wrapLength);
            }
            else
            {
                BDBG_MSG(("Not Parsing"));
            }
            /* Populate this segment's data */
            {
                unsigned segment = hMuxOutput->descriptorInfo.adtsSegment;
                if ( segment >= hMuxOutput->descriptorInfo.adtsNumSegments)
                {
                    BDBG_ERR(("ADTS segment parsing error"));
                    BKNI_Sleep(1000);   /* Let some debug data flush */
                    BDBG_ASSERT(segment < hMuxOutput->descriptorInfo.adtsNumSegments);
                }
                pAudioDescriptor->stCommon.uiOffset = hMuxOutput->descriptorInfo.adtsSegmentOffset[segment];
                pAudioDescriptor->stCommon.uiLength = hMuxOutput->descriptorInfo.adtsSegmentLength[segment];
                pAudioDescriptor->uiRawDataOffset = hMuxOutput->descriptorInfo.adtsSegmentRawOffset[segment];
                pAudioDescriptor->uiRawDataLength = hMuxOutput->descriptorInfo.adtsSegmentRawLength[segment];
                hMuxOutput->descriptorInfo.adtsSegment = segment+1;
            }
            break;
        default:
            break;
        }

        /* Advance read pointer appropriately */
        BDBG_MSG(("Advance CDB shadow read from " BDBG_UINT64_FMT " to " BDBG_UINT64_FMT " (len %lx)",BDBG_UINT64_ARG(psOutputDescDetails->uiCDBBufferShadowReadOffset),
                  BDBG_UINT64_ARG(psOutputDescDetails->uiCDBBufferShadowReadOffset+pAudioDescriptor->stCommon.uiLength),
                  (unsigned long)pAudioDescriptor->stCommon.uiLength));
        psOutputDescDetails->uiCDBBufferShadowReadOffset += \
                                                            pAudioDescriptor->stCommon.uiLength;
        if ( psOutputDescDetails->uiCDBBufferShadowReadOffset >= \
             uiCDBEndOffset )
        {
            psOutputDescDetails->uiCDBBufferShadowReadOffset -= \
                                                                ( uiCDBEndOffset - uiCDBBaseOffset );
        }

        if ( psOutputDescDetails->uiCDBBufferShadowReadOffset == uiNextBase )
        {
            pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_END;
        }

        BDBG_MSG(("Audio Descriptor Base/Length = %x/%lx Shadow Read " BDBG_UINT64_FMT "",
                  pAudioDescriptor->stCommon.uiOffset,
                  (unsigned long)pAudioDescriptor->stCommon.uiLength,
                  BDBG_UINT64_ARG(psOutputDescDetails->uiCDBBufferShadowReadOffset)
                 ));

        /* If we need to send metadata, send it on the first frame */
        if ( hMuxOutput->sendMetadata && (pAudioDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START) )
        {
            BAVC_AudioMetadataDescriptor *pMetadataDescriptor = &psOutputDescDetails->metadata.cached[0];
            BAVC_AudioBufferDescriptor *pFrameDescriptor;

            /* This is the first frame so we should always have another descriptor available. Assert for sanity. */
            uiTemp = (psOutputDescDetails->uiDescriptorWriteOffset + 1) % hMuxOutput->createSettings.numDescriptors;
            BDBG_ASSERT(uiTemp != psOutputDescDetails->uiDescriptorReadOffset);
            pFrameDescriptor = &psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorWriteOffset];
            psOutputDescDetails->uiDescriptorWriteOffset = uiTemp;

            /* The metadata descriptor must come before the first frame.  Swap them. */
            *pFrameDescriptor = *pAudioDescriptor;   /* Copy frame descriptor contents into second descriptor - we're about to overwrite the old one */

            BKNI_Memset(pAudioDescriptor, 0, sizeof(BAVC_AudioBufferDescriptor));
            /* SW7425-75: For Phase 2.0, only a single metadata descriptor will be sent at the beginning */
            /* TODO: Update this in Phase 3.0 to send a metadata descriptor on each RAP or when the metadata changes */
            pAudioDescriptor->stCommon.uiOffset = 0;
            pAudioDescriptor->stCommon.uiLength = sizeof( BAVC_AudioMetadataDescriptor );
            pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA;
            pAudioDescriptor->uiDataUnitType = BAVC_AudioMetadataType_eCommon;

            /* Populate metadata */
            BKNI_Memset(pMetadataDescriptor, 0, sizeof(*pMetadataDescriptor));

            if ( stMetadata.uiMetadataFlags & BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID )
            {
                pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID;
                pMetadataDescriptor->stBitrate = stMetadata.stBitrate;
            }

            /* Set Sample Rate */
            pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_SAMPLING_FREQUENCY_VALID;
            pMetadataDescriptor->stSamplingFrequency.uiSamplingFrequency = hMuxOutput->input->format.sampleRate;

            /* Set STC Snapshot */
            pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_TIMING_VALID;
            pMetadataDescriptor->stTiming = stMetadata.stTiming;

            /* Set Protocol */
            BDBG_MSG(("Setting protocol to %u", BAPE_FMT_P_GetAudioCompressionStd_isrsafe(&hMuxOutput->input->format)));
            pMetadataDescriptor->eProtocol = BAPE_FMT_P_GetAudioCompressionStd_isrsafe(&hMuxOutput->input->format);
            switch ( pMetadataDescriptor->eProtocol )
            {
            case BAVC_AudioCompressionStd_eAacAdts:
            case BAVC_AudioCompressionStd_eAacPlusAdts:
                /* ASSUMPTION: This is the first frame and does not need to handle CDB Wrap or partial frames.
                   That would make life much more difficult since metadata comes first, and the DSP should be doing this anyway... */
                ret = BAPE_MuxOutput_P_ParseAdtsMetadata(hMuxOutput, pFrameDescriptor, pMetadataDescriptor);
                if ( ret == BERR_SUCCESS )
                {
                    pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID;
                }
                else
                {
                    BDBG_WRN(("Unable to parse ADTS metadata."));
                    ret = BERR_SUCCESS;
                }
                break;
            case BAVC_AudioCompressionStd_eWmaStd:
                if ( stMetadata.uiMetadataFlags & BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID )
                {
                    BAPE_EncoderCodecSettings encoderCodecSettings;

                    pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID;
                    pMetadataDescriptor->uProtocolData = stMetadata.uProtocolData;  /* Metadata was parsed above */

                    BAPE_Encoder_GetCodecSettings(hMuxOutput->input->pParent->pHandle, BAVC_AudioCompressionStd_eWmaStd, &encoderCodecSettings);
                    pMetadataDescriptor->uProtocolData.stWmaStd.uiNumChannels = (encoderCodecSettings.codecSettings.wmaStd.channelMode==BAPE_ChannelMode_e1_0)?1:2;
                }
                else
                {
                    BDBG_WRN(("WMA Metadata not available"));
                }
                break;
            default:
                break;
            }
            BDBG_MSG(("Sending Metadata for codec %s (%u) - Flags %#x", BAPE_P_GetCodecName(pMetadataDescriptor->eProtocol), pMetadataDescriptor->eProtocol, pMetadataDescriptor->uiMetadataFlags));

            hMuxOutput->sendMetadata = false;
        }
    }

    if ( hMuxOutput->sendEos )
    {
       bool bDummyEOS = hMuxOutput->sendMetadata;

       /* Insert dummy METADATA descriptor if one hasn't already been sent */
       if ( hMuxOutput->sendMetadata )
       {
          BAVC_AudioMetadataDescriptor *pMetadataDescriptor = &psOutputDescDetails->metadata.cached[0];

          /* This is the first frame so we should always have another descriptor available. Assert for sanity. */
          uiTemp = (psOutputDescDetails->uiDescriptorWriteOffset + 1) % hMuxOutput->createSettings.numDescriptors;
          BDBG_ASSERT(uiTemp != psOutputDescDetails->uiDescriptorReadOffset);
          pAudioDescriptor = &psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorWriteOffset];
          psOutputDescDetails->uiDescriptorWriteOffset = uiTemp;

          BKNI_Memset(pAudioDescriptor, 0, sizeof(BAVC_AudioBufferDescriptor));

          pAudioDescriptor->stCommon.uiOffset = 0;
          pAudioDescriptor->stCommon.uiLength = sizeof( BAVC_AudioMetadataDescriptor );
          pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA;
          pAudioDescriptor->uiDataUnitType = BAVC_AudioMetadataType_eCommon;

          /* Populate metadata */
          BKNI_Memset(pMetadataDescriptor, 0, sizeof(*pMetadataDescriptor));

          hMuxOutput->sendMetadata = false;
       }

        BDBG_MSG(("EOS Required"));
        /* Get a descriptor for EOS */
        uiTemp = (psOutputDescDetails->uiDescriptorWriteOffset + 1) % hMuxOutput->createSettings.numDescriptors;
        if ( uiTemp == psOutputDescDetails->uiDescriptorReadOffset )
        {
            BDBG_MSG(("Out of descriptors, can't send EOS"));
        }
        else
        {
            pAudioDescriptor = &psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorWriteOffset];
            psOutputDescDetails->uiDescriptorWriteOffset = uiTemp;

            BKNI_Memset(pAudioDescriptor, 0, sizeof(BAVC_AudioBufferDescriptor));

            /* Decoder is stopped and we have run out of data. Fill the EOS entry in Audio descriptor */
            pAudioDescriptor->stCommon.uiFlags = BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;

            if ( bDummyEOS && ( hMuxOutput->nonRealTimeIncrement.StcIncLo != 0 &&
                                hMuxOutput->nonRealTimeIncrement.StcIncHi != 0 &&
                                hMuxOutput->nonRealTimeIncrement.IncTrigger != 0 ) )
            {
               pAudioDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID;
            }
            BDBG_MSG(("EOS %p", (void *)pAudioDescriptor));
            hMuxOutput->sendEos = false;
        }
    }

    /* Assign array(s) and count(s) */
    if ( psOutputDescDetails->uiDescriptorWriteOffset >= \
         psOutputDescDetails->uiDescriptorReadOffset )
    {
        *pBuffer = &psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorReadOffset];
        *pSize = psOutputDescDetails->uiDescriptorWriteOffset - psOutputDescDetails->uiDescriptorReadOffset;

        *pBuffer2 = NULL;
        *pSize2 = 0;
    }
    else
    {
        *pBuffer = &psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorReadOffset];
        *pSize = hMuxOutput->createSettings.numDescriptors - psOutputDescDetails->uiDescriptorReadOffset;

        *pBuffer2 = &psOutputDescDetails->descriptors.cached[0];
        *pSize2 = psOutputDescDetails->uiDescriptorWriteOffset;
    }

    BDBG_MSG(("pSize = %lu",(unsigned long)(*pSize)));
    for ( uiTemp=0;uiTemp < (*pSize);uiTemp++ )
        BDBG_MSG(("astDescriptors0[%d] = 0x%p (flags %#x, len %lu)",uiTemp,(void*)&psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorReadOffset+uiTemp],
                  psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorReadOffset+uiTemp].stCommon.uiFlags,
                  (unsigned long)psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorReadOffset+uiTemp].stCommon.uiLength));

    BDBG_MSG(("pSize2 = %lu",(unsigned long)(*pSize2)));
    for ( uiTemp=0;uiTemp < (*pSize2);uiTemp++ )
        BDBG_MSG(("astDescriptors1[%d] = 0x%p (flags %#x, len %lu)",uiTemp,(void*)&psOutputDescDetails->descriptors.cached[uiTemp],
                  psOutputDescDetails->descriptors.cached[uiTemp].stCommon.uiFlags,
                  (unsigned long)psOutputDescDetails->descriptors.cached[uiTemp].stCommon.uiLength));

    psOutputDescDetails->numOutstandingDescriptors = (*pSize) + (*pSize2);
    BDBG_MSG(("Returning %u descriptors", psOutputDescDetails->numOutstandingDescriptors));

    BDBG_LEAVE(BAPE_MuxOutput_GetBufferDescriptors);

    return ret;
}

BERR_Code BAPE_MuxOutput_ConsumeBufferDescriptors(
    BAPE_MuxOutputHandle hMuxOutput,
    unsigned numBufferDescriptors /* must be <= pSize+pSize2 returned by last BAPE_MuxOutput_GetBufferDescriptors call. */
    )
{
    BREG_Handle hReg;
    BERR_Code   ret = BERR_SUCCESS;
    BMMA_DeviceOffset uiCDBReadOffset;
    BMMA_DeviceOffset uiCDBEndOffset;
    BMMA_DeviceOffset uiCDBBaseOffset;
    BMMA_DeviceOffset uiCDBValidOffset;
    BMMA_DeviceOffset uiCDBDepth;
    uint64_t uiCDBIncrement=0;
    BAPE_OutputDescriptorInfo  *psOutputDescDetails;

    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(numBufferDescriptors > 0);

    BDBG_ENTER(BAPE_MuxOutput_ConsumeBufferDescriptors);

    BDBG_MSG(("BAPE_MuxOutput_ConsumeBufferDescriptors: uiNumBufferDescriptors = %d",numBufferDescriptors));
    hReg = hMuxOutput->node.deviceHandle->regHandle;
    psOutputDescDetails = &hMuxOutput->descriptorInfo;

    /* If a watchdog or stop/start event came while the mux is running, we may receive a request to drop more descriptors than we have.
       Warn and drop the ones that are actually outstanding */
    if ( numBufferDescriptors > psOutputDescDetails->numOutstandingDescriptors )
    {
        if ( 0 == psOutputDescDetails->numOutstandingDescriptors )
        {
            return BERR_SUCCESS;
        }
        else
        {
            BDBG_WRN(("Request to consume %u descriptors, but %u are available.  Consuming %u.",
                      numBufferDescriptors, psOutputDescDetails->numOutstandingDescriptors, psOutputDescDetails->numOutstandingDescriptors));
            numBufferDescriptors = psOutputDescDetails->numOutstandingDescriptors;
        }
    }
    psOutputDescDetails->numOutstandingDescriptors -= numBufferDescriptors;

    /* Read CDB Addresses */
    uiCDBBaseOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.base);
    uiCDBEndOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.end);
    uiCDBValidOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.valid);
    uiCDBReadOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.read);

    if ( uiCDBEndOffset != 0 && hMuxOutput->cdb.bufferInterface.inclusive )
    {
        uiCDBEndOffset += 1; /* end is inclusive */
    }

    if ( uiCDBValidOffset == uiCDBReadOffset )
    {
        uiCDBDepth = 0;
    }
    else if ( uiCDBValidOffset > uiCDBReadOffset )
    {
        uiCDBDepth = uiCDBValidOffset - uiCDBReadOffset;
    }
    else
    {
        uiCDBDepth = uiCDBEndOffset - uiCDBReadOffset;
        uiCDBDepth += uiCDBValidOffset - uiCDBBaseOffset;
    }

    while ( numBufferDescriptors )
    {
        if ( 0 == (psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorReadOffset].stCommon.uiFlags & (BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS|BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA)) )
        {
            /* Move CDB Read Offset */
            uiCDBIncrement += psOutputDescDetails->descriptors.cached[psOutputDescDetails->uiDescriptorReadOffset].stCommon.uiLength;
        }

        /* Move Descriptor Read Offset */
        psOutputDescDetails->uiDescriptorReadOffset++;
        psOutputDescDetails->uiDescriptorReadOffset %= hMuxOutput->createSettings.numDescriptors;

        numBufferDescriptors--;
    }

    if ( uiCDBIncrement > uiCDBDepth )
    {
        BDBG_ERR(("Attempting to consume " BDBG_UINT64_FMT " bytes from CDB when only " BDBG_UINT64_FMT " are present.", BDBG_UINT64_ARG(uiCDBIncrement), BDBG_UINT64_ARG(uiCDBDepth)));
        BDBG_ASSERT(uiCDBDepth >= uiCDBIncrement);
    }
    else
    {
        BDBG_MSG(("Consume " BDBG_UINT64_FMT " of " BDBG_UINT64_FMT " bytes from CDB", BDBG_UINT64_ARG(uiCDBIncrement), BDBG_UINT64_ARG(uiCDBDepth)));
    }

    uiCDBReadOffset += uiCDBIncrement;
    if ( uiCDBReadOffset >= uiCDBEndOffset )
    {
        uiCDBReadOffset -= ( uiCDBEndOffset - uiCDBBaseOffset );
    }

    BDBG_MSG(("BRAP_UpdateBufferDescriptors :uiDescriptorReadOffset = " BDBG_UINT64_FMT "",
              BDBG_UINT64_ARG(psOutputDescDetails->uiDescriptorReadOffset)));

    /* Update Actual ITB/CDB Read Pointers */
    BREG_WriteAddr(hReg, hMuxOutput->cdb.bufferInterface.read, uiCDBReadOffset);
    /* No need to compute the ITB amount - just consume all previously used ITB entries in one shot. */
    BREG_WriteAddr(hReg, hMuxOutput->itb.bufferInterface.read, psOutputDescDetails->uiITBBufferShadowReadOffset);

    BDBG_LEAVE (BAPE_MuxOutput_ConsumeBufferDescriptors);
    return ret;
}

BERR_Code BAPE_MuxOutput_GetBufferStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAVC_AudioBufferStatus *pBufferStatus    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(NULL != pBufferStatus);

    BKNI_Memset(pBufferStatus, 0, sizeof(*pBufferStatus));
    pBufferStatus->stCommon.hFrameBufferBlock = hMuxOutput->createSettings.cdb.block;
    pBufferStatus->stCommon.hMetadataBufferBlock = hMuxOutput->descriptorInfo.metadata.block;
    /* deprecated */
    pBufferStatus->stCommon.pFrameBufferBaseAddress = hMuxOutput->cdb.cached;
    pBufferStatus->stCommon.pMetadataBufferBaseAddress = hMuxOutput->descriptorInfo.metadata.cached;

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
void BAPE_MuxOutput_GetConnector(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &hMuxOutput->node.connectors[0];
}
#endif

BERR_Code BAPE_MuxOutput_AddInput(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( NULL != hMuxOutput->input )
    {
        BDBG_ERR(("Can not have more than one input"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_PathNode_P_AddInput(&hMuxOutput->node, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    hMuxOutput->input = input;
    return BERR_SUCCESS;
}

BERR_Code BAPE_MuxOutput_RemoveInput(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( input != hMuxOutput->input )
    {
        BDBG_ERR(("Input %s %s (%p) is not connected", input->pParent->pName, input->pName, (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    errCode = BAPE_PathNode_P_RemoveInput(&hMuxOutput->node, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    hMuxOutput->input = NULL;
    return BERR_SUCCESS;
}

BERR_Code BAPE_MuxOutput_RemoveAllInputs(
    BAPE_MuxOutputHandle hMuxOutput
    )
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    if ( hMuxOutput->input )
    {
        return BAPE_MuxOutput_RemoveInput(hMuxOutput, hMuxOutput->input);
    }
    return BERR_SUCCESS;
}

static void BAPE_MuxOutput_P_GetUpstreamDeviceDetails(
    BAPE_PathConnection * pConnection,
    unsigned * pDspIndex,
    BDSP_ContextHandle * pDspContext
    )
{
    if ( pConnection == NULL )
    {
        BDBG_WRN(("Unable to find upstream master task"));
        *pDspIndex = BAPE_DEVICE_INVALID;
        *pDspContext = NULL;
        return;
    }

    if ( pConnection->pSource->pParent->type ==  BAPE_PathNodeType_eMixer ||
         pConnection->pSource->pParent->type == BAPE_PathNodeType_eDecoder )
    {
        if ( pConnection->pSource->pParent->deviceIndex != BAPE_DEVICE_INVALID &&
             pConnection->pSource->pParent->deviceContext != NULL )
        {
            BDBG_MSG(("Getting DSP details from upstream node %p, type %s",
                      (void*)pConnection->pSource->pParent,
                      (pConnection->pSource->pParent->type == BAPE_PathNodeType_eMixer)?"MIXER":"DECODER"));
            BDBG_MSG(("  Using DSP idx %d, context %p",
                      (int)pConnection->pSource->pParent->deviceIndex,
                      (void*)pConnection->pSource->pParent->deviceContext));
            *pDspIndex = pConnection->pSource->pParent->deviceIndex;
            *pDspContext = pConnection->pSource->pParent->deviceContext;
        }
    }
    else
    {
        /* look for device index from upstream node */
        BAPE_MuxOutput_P_GetUpstreamDeviceDetails(BLST_S_FIRST(&pConnection->pSource->pParent->upstreamList), pDspIndex, pDspContext);
    }
}

static BERR_Code BAPE_MuxOutput_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BAPE_MuxOutputHandle hMuxOutput;
    unsigned input, output;
    BDSP_ContextHandle dspContext = NULL;
    unsigned dspIndex = BAPE_DEVICE_INVALID;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    hMuxOutput = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);

    if ( pConnection->pSource->pParent->type == BAPE_PathNodeType_eEncoder )
    {
        BAPE_EncoderHandle encoder = pConnection->pSource->pParent->pHandle;
        BAPE_EncoderSettings encoderSettings;
        BAPE_Encoder_GetSettings(encoder, &encoderSettings);
        switch ( encoderSettings.codec )
        {
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eDts:
            BDBG_ERR(("Muxoutput does not support codec type %d from encoder",encoderSettings.codec));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        default:
            break;
        }
    }

    /* if we found a device context upstream, use it. Else use our default */
    BAPE_MuxOutput_P_GetUpstreamDeviceDetails(pConnection, &dspIndex, &dspContext);
    if ( dspContext == NULL || dspIndex == BAPE_DEVICE_INVALID )
    {
        dspContext = hMuxOutput->dspContext;
        dspIndex = hMuxOutput->dspIndex - hMuxOutput->dspIndexBase;
    }

    if ( dspContext == NULL || dspIndex == BAPE_DEVICE_INVALID )
    {
        BDBG_ERR(("Unable to find the device context from parent connection or global device contexts"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

    errCode = BAPE_MuxOutput_P_AllocateStageResources(hMuxOutput, dspContext, dspIndex);
    if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

    /* connect input to mux */
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage,
                                        BDSP_DataType_eCompressedRaw,
                                        hMuxOutput->hStage,
                                        &output, &input);
    if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }


    errCode = BAPE_MuxOutput_P_ApplyDspSettings(hMuxOutput);
    if ( errCode ) { BERR_TRACE(errCode); goto cleanup; }

    return BERR_SUCCESS;
cleanup:
    BAPE_MuxOutput_P_DestroyStageResources(hMuxOutput);
    return errCode;
}

static void BAPE_MuxOutput_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    /* Invalidate task handle */
    BAPE_MuxOutputHandle hMuxOutput;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    hMuxOutput = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    if ( hMuxOutput->hStage )
    {
        BDSP_Stage_RemoveAllInputs(hMuxOutput->hStage);
    }
}

static void BAPE_MuxOutput_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BAPE_MuxOutput_P_StopPathFromInput(pNode, pConnection);
}

BERR_Code BAPE_MuxOutput_GetDelayStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAVC_AudioCompressionStd codec,
    BAPE_MuxOutputDelayStatus *pStatus    /* [out] */
    )
{
    unsigned frameTime;
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(NULL != pStatus);
    pStatus->endToEndDelay = 0;
    if ( NULL == hMuxOutput->input )
    {
        BDBG_ERR(("Not connected to any input.  Can not determine delay status."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /* Determine codec delay */
    switch (codec)
    {
    /* End to End delay (Time difference between when First PCM sample comes to Encode buffer
       To the time when reference decoder can start decoding) is 2*FrameTime. 32kHz is chosen to
       take care of worst case delay */
    case BAVC_AudioCompressionStd_eMpegL1:
    case BAVC_AudioCompressionStd_eMpegL2:
    case BAVC_AudioCompressionStd_eMpegL3:
        frameTime = 1152/32;
        break;
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eAc3Plus:
    case BAVC_AudioCompressionStd_eAc3Lossless:
        frameTime = 1536/32;
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
    default:
        frameTime = 2048/32;
        break;
    }
    if ( hMuxOutput->input->pParent->type == BAPE_PathNodeType_eDecoder )
    {
        /* Decoder input */
        pStatus->endToEndDelay = frameTime;
    }
    else
    {
        /* Encoder requires an extra frame of buffering */
        pStatus->endToEndDelay = frameTime * 2;
    }

    return BERR_SUCCESS;
}

static void BAPE_MuxOutput_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_MuxOutput_RemoveInput(pNode->pHandle, pConnector);
}

static void *bape_p_convert_itb_offset(BAPE_MuxOutputHandle hMuxOutput, BMMA_DeviceOffset offset)
{
    return (void*)((uint8_t*)hMuxOutput->itb.cached + (offset - hMuxOutput->itb.offset));
}

/* Get type of next entry */
static uint8_t BAPE_MuxOutput_P_GetNextItbEntryType(BAPE_MuxOutputHandle hMuxOutput, BMMA_DeviceOffset readOffset)
{
    void *pSource;
    BAPE_ItbEntry entry;

    pSource = bape_p_convert_itb_offset(hMuxOutput, readOffset);
    BAPE_MuxOutput_P_CopyToHost(&entry, pSource, sizeof(BAPE_ItbEntry), hMuxOutput->createSettings.itb.block);

    return BAPE_ITB_GET_FIELD(&entry,GENERIC,ENTRY_TYPE);
}

/* Read and hMuxOutput wraparound */
static size_t BAPE_MuxOutput_P_ReadItb(BAPE_MuxOutputHandle hMuxOutput, BMMA_DeviceOffset baseOffset, BMMA_DeviceOffset readOffset, BMMA_DeviceOffset endOffset, size_t depth, uint8_t *pDest, size_t length)
{
    void *pSource;

    if ( length <= depth )
    {
        pSource = bape_p_convert_itb_offset(hMuxOutput, readOffset);
        if ( readOffset + length > endOffset )
        {
            size_t preWrapAmount = endOffset-readOffset;
            size_t wrapAmount = length - preWrapAmount;
            /* Wraparound */
            BAPE_MuxOutput_P_CopyToHost(pDest, pSource, preWrapAmount, hMuxOutput->createSettings.itb.block);
            pSource = bape_p_convert_itb_offset(hMuxOutput, baseOffset);
            BAPE_MuxOutput_P_CopyToHost(pDest+preWrapAmount, pSource, wrapAmount, hMuxOutput->createSettings.itb.block);
        }
        else
        {
            /* No Wrap */
            BAPE_MuxOutput_P_CopyToHost(pDest, pSource, length, hMuxOutput->createSettings.itb.block);
        }

        return length;
    }
    else
    {
        return 0;
    }
}

static void BAPE_MuxOutput_P_ParseItb(BAPE_MuxOutputHandle hMuxOutput, BAPE_FrameItbEntries **pCurrent, BAPE_FrameItbEntries **pNext, BAVC_AudioMetadataDescriptor *pMetadataDescriptor, BAPE_ItbEntry **pMetadata)
{
    BREG_Handle hReg;
    BMMA_DeviceOffset uiITBBaseOffset;
    BMMA_DeviceOffset uiITBEndOffset;
    BMMA_DeviceOffset uiITBValidOffset;
    BMMA_DeviceOffset uiITBReadOffset;
    BMMA_DeviceOffset uiITBDepth;
    BMMA_DeviceOffset uiShadowReadOffset;
    BMMA_DeviceOffset uiNextEntryOffset;
    size_t uiAmountRead;
    uint8_t entryType;
    BAPE_OutputDescriptorInfo *psOutputDescDetails;
    BAPE_FrameItbEntries *pITBEntry, *pITBEntryNext;
    BAPE_ItbEntry *pITBEntryMetadata;

    hReg = hMuxOutput->node.deviceHandle->regHandle;
    psOutputDescDetails = &hMuxOutput->descriptorInfo;

    /* Invalidate pointers */
    *pCurrent = NULL;
    *pNext = NULL;
    *pMetadata = NULL;

    /* Setup Metadata */
    BKNI_Memset(pMetadataDescriptor, 0, sizeof(*pMetadataDescriptor));

    /* Read ITB Addresses */
    uiITBBaseOffset = BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.base);
    uiITBEndOffset = BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.end);
    uiITBValidOffset= BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.valid);
    uiITBReadOffset= BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.read);

    if ( uiITBEndOffset != 0 && hMuxOutput->itb.bufferInterface.inclusive )
    {
        uiITBEndOffset += 1; /* end is inclusive */
    }

    BDBG_MSG(("ITB Base/End/Shadow Read/Valid = " BDBG_UINT64_FMT "/" BDBG_UINT64_FMT "/" BDBG_UINT64_FMT " (" BDBG_UINT64_FMT ")/" BDBG_UINT64_FMT "",
              BDBG_UINT64_ARG(uiITBBaseOffset),
              BDBG_UINT64_ARG(uiITBEndOffset),
              BDBG_UINT64_ARG(psOutputDescDetails->uiITBBufferShadowReadOffset),
              BDBG_UINT64_ARG(uiITBReadOffset),
              BDBG_UINT64_ARG(uiITBValidOffset)
             ));


    uiShadowReadOffset = psOutputDescDetails->uiITBBufferShadowReadOffset;
    for ( ;; )
    {
        pITBEntry = NULL;
        pITBEntryNext = NULL;
        pITBEntryMetadata = NULL;

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

        BDBG_MSG(("ITB Depth: " BDBG_UINT64_FMT " bytes (Valid: " BDBG_UINT64_FMT ", Shadow Read: " BDBG_UINT64_FMT ")",
                  BDBG_UINT64_ARG(uiITBDepth),
                  BDBG_UINT64_ARG(uiITBValidOffset),
                  BDBG_UINT64_ARG(uiShadowReadOffset)
                 ));

        /* Don't attempt to read the next entry if it's invaild */
        if ( uiITBDepth < 16 )
        {
            BDBG_MSG(("No ITB entries available"));
            return;
        }

        /* Check for odd ITB entries and drop them */
        entryType = BAPE_MuxOutput_P_GetNextItbEntryType(hMuxOutput, uiShadowReadOffset);
        if ( entryType != BAPE_ITB_ENTRY_TYPE_BASE_ADDRESS )
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

        uiAmountRead = BAPE_MuxOutput_P_ReadItb(hMuxOutput, uiITBBaseOffset, uiShadowReadOffset, uiITBEndOffset, uiITBDepth,
                                                (uint8_t *)&psOutputDescDetails->itb.current, sizeof(BAPE_FrameItbEntries));
        if ( 0 == uiAmountRead )
        {
            /* We ran out of ITB entries */
            BDBG_MSG(("No more ITB Entries"));
            psOutputDescDetails->uiITBBufferShadowReadOffset = uiShadowReadOffset;
            return;
        }

        uiITBDepth -= uiAmountRead;
        pITBEntry = &psOutputDescDetails->itb.current;

        /* Check other fields for sanity */
        BDBG_ASSERT(BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, GENERIC, ENTRY_TYPE) == BAPE_ITB_ENTRY_TYPE_PTS_DTS);
        BDBG_ASSERT(BAPE_ITB_GET_FIELD(&pITBEntry->bitRate, GENERIC, ENTRY_TYPE) == BAPE_ITB_ENTRY_TYPE_BIT_RATE);
        BDBG_ASSERT(BAPE_ITB_GET_FIELD(&pITBEntry->escrMetadata, GENERIC, ENTRY_TYPE) == BAPE_ITB_ENTRY_TYPE_MUX_ESCR);

        BDBG_MSG(("*** ITB Dump (entry size = %lu bytes)***", (unsigned long)sizeof(BAPE_FrameItbEntries)));
        BDBG_MSG((" error = %1x", BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, ERROR)));
        BDBG_MSG((" valid = %1x", BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, FRAME_VALID)));
        BDBG_MSG((" cdb_address = %08x", BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, CDB_ADDRESS)));
        BDBG_MSG((" length = %04x", BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, FRAME_LENGTH)));
        BDBG_MSG((" pts[32] = %1x", BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, PTS_DTS, PTS_32)));
        BDBG_MSG((" pts[31:0] = %08x", BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, PTS_DTS, PTS)));
        BDBG_MSG((" stc[31:0] = %08x", BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, PTS_DTS, STC_LOWER)));
        BDBG_MSG((" ticks_per_bit = %04x", BAPE_ITB_GET_FIELD(&pITBEntry->bitRate, BIT_RATE, TICKS_PER_BIT)));
        BDBG_MSG((" shr = %4d", BAPE_ITB_GET_FIELD(&pITBEntry->bitRate, BIT_RATE, SHR)));
        BDBG_MSG((" samplerate = %08x", BAPE_ITB_GET_FIELD(&pITBEntry->bitRate, BIT_RATE, SAMPLE_RATE)));
        BDBG_MSG((" ui32ESCR = %08x", BAPE_ITB_GET_FIELD(&pITBEntry->escrMetadata, ESCR_METADATA, ESCR)));
        BDBG_MSG((" ui32OriginalPTS = %8x", BAPE_ITB_GET_FIELD(&pITBEntry->escrMetadata, ESCR_METADATA, ORIGINAL_PTS)));
        BDBG_MSG((" ui32OriginalPTSValid = %8x", BAPE_ITB_GET_FIELD(&pITBEntry->escrMetadata, ESCR_METADATA, ORIGINAL_PTS_VALID)));

        uiNextEntryOffset = uiShadowReadOffset + sizeof(BAPE_FrameItbEntries);
        if ( uiNextEntryOffset >= uiITBEndOffset )
        {
            uiNextEntryOffset -= (uiITBEndOffset-uiITBBaseOffset);
        }

        pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_TIMING_VALID;
        pMetadataDescriptor->stTiming.uiSTCSnapshot = BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, PTS_DTS, STC_UPPER);
        pMetadataDescriptor->stTiming.uiSTCSnapshot <<= 32;
        pMetadataDescriptor->stTiming.uiSTCSnapshot |= BAPE_ITB_GET_FIELD(&pITBEntry->ptsDts, PTS_DTS, STC_LOWER);

        while ( uiITBDepth >= sizeof(BAPE_FrameItbEntries) && NULL == pITBEntryNext )
        {
            /* Check for odd ITB entries and drop them */
            entryType = BAPE_MuxOutput_P_GetNextItbEntryType(hMuxOutput, uiNextEntryOffset);
            if ( entryType != BAPE_ITB_ENTRY_TYPE_BASE_ADDRESS )
            {
                switch ( entryType )
                {
                case BAPE_ITB_ENTRY_TYPE_ASC:
                    /* Parse WMA Algo-specific info */
                    if ( BAPE_FMT_P_GetAudioCompressionStd_isrsafe(&hMuxOutput->input->format) == BAVC_AudioCompressionStd_eWmaStd )
                    {
                        BAPE_ItbEntry wmaEntry;
                        uiAmountRead = BAPE_MuxOutput_P_ReadItb(hMuxOutput, uiITBBaseOffset, uiNextEntryOffset, uiITBEndOffset, uiITBDepth,
                                                                (uint8_t *)&wmaEntry, sizeof(wmaEntry));
                        if ( uiAmountRead != sizeof(wmaEntry) )
                        {
                            BDBG_MSG(("WMA entry unavailable"));
                            return;
                        }
                        pMetadataDescriptor->uProtocolData.stWmaStd.uiSamplesPerBlock = BAPE_ITB_GET_FIELD(&wmaEntry, WMA_SPECIFIC, SAMPLES_PER_FRAME);
                        pMetadataDescriptor->uProtocolData.stWmaStd.uiEncodeOptions = BAPE_ITB_GET_FIELD(&wmaEntry, WMA_SPECIFIC, ENCODE_OPTION);
                        pMetadataDescriptor->uProtocolData.stWmaStd.uiSuperBlockAlign = BAPE_ITB_GET_FIELD(&wmaEntry, WMA_SPECIFIC, SUPER_BLOCK_ALIGN);
                        pMetadataDescriptor->uProtocolData.stWmaStd.uiBlockAlign = BAPE_ITB_GET_FIELD(&wmaEntry, WMA_SPECIFIC, BLOCK_ALIGN);
                        pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID;
                        BDBG_MSG(("WMA Protocol Data - Samples Per Block %u SuperBlockAlign %u Options %#x",
                                  pMetadataDescriptor->uProtocolData.stWmaStd.uiSamplesPerBlock,
                                  pMetadataDescriptor->uProtocolData.stWmaStd.uiSuperBlockAlign,
                                  pMetadataDescriptor->uProtocolData.stWmaStd.uiEncodeOptions));
                    }
                    /* TODO: Parse AAC */
                    break;
                case BAPE_ITB_ENTRY_TYPE_ALGO_INFO:
                    {
                        BAPE_ItbEntry algoInfo;
                        uiAmountRead = BAPE_MuxOutput_P_ReadItb(hMuxOutput, uiITBBaseOffset, uiNextEntryOffset, uiITBEndOffset, uiITBDepth,
                                                                (uint8_t *)&algoInfo, sizeof(algoInfo));
                        if ( uiAmountRead != sizeof(algoInfo) )
                        {
                            BDBG_MSG(("ALGO_INFO entry unavailable"));
                            return;
                        }
                        pMetadataDescriptor->uiMetadataFlags |= BAVC_AUDIOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID;
                        pMetadataDescriptor->stBitrate.uiMax = BAPE_ITB_GET_FIELD(&algoInfo, ALGO_INFO, MAX_BIT_RATE);
                    }
                    break;
                case BAPE_ITB_ENTRY_TYPE_METADATA:
                    uiAmountRead = BAPE_MuxOutput_P_ReadItb(hMuxOutput, uiITBBaseOffset, uiNextEntryOffset, uiITBEndOffset, uiITBDepth,
                                                            (uint8_t *)&psOutputDescDetails->itb.metadata, sizeof(BAPE_ItbEntry));
                    if ( uiAmountRead != sizeof(BAPE_ItbEntry) )
                    {
                        BDBG_MSG(("METADATA entry unavailable"));
                        return;
                    }
                    pITBEntryMetadata = &(psOutputDescDetails->itb.metadata);
                    break;
                default:
                    BDBG_WRN(("Dropping ITB Entry type 0x%02x looking for next", entryType));
                    #if 0
                    BDBG_WRN(("uiNextEntryOffset 0x%08x base 0x%08x end 0x%08x shadow 0x%08x valid 0x%08x",uiNextEntryOffset,uiITBBaseOffset,uiITBEndOffset,uiShadowReadOffset,uiITBValidOffset));
                    #endif
                    break;
                }
                uiNextEntryOffset += 16;
                if ( uiNextEntryOffset >= uiITBEndOffset )
                {
                    uiNextEntryOffset -= (uiITBEndOffset-uiITBBaseOffset);
                }
                uiITBDepth -= 16;
                continue;
            }

            /* Found a base address entry.  Read the next entry. */
            uiAmountRead = BAPE_MuxOutput_P_ReadItb(hMuxOutput, uiITBBaseOffset, uiNextEntryOffset, uiITBEndOffset, uiITBDepth,
                                                    (uint8_t *)&psOutputDescDetails->itb.next, sizeof(BAPE_FrameItbEntries));
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
            if ( BAPE_ITB_GET_FIELD(&pITBEntryNext->baseAddress, BASE_ADDRESS, CDB_ADDRESS) ==
                 psOutputDescDetails->uiCDBBufferShadowReadOffset )
            {
                /* We have a next entry, and we've finished with the
                 * current entry, so move to the next entry
                 */
                uiShadowReadOffset = uiNextEntryOffset;
                psOutputDescDetails->uiITBBufferShadowReadOffset = uiNextEntryOffset;
                if ( BAPE_ITB_GET_FIELD(&pITBEntry->baseAddress, BASE_ADDRESS, FRAME_LENGTH) == 0 )
                {
                    *pCurrent = pITBEntry;
                    *pNext = pITBEntryNext;
                    *pMetadata = pITBEntryMetadata;
                    return;
                }
                BDBG_MSG(("Goto Next Entry"));
                continue;
            }

            *pCurrent = pITBEntry;
            *pNext = pITBEntryNext;
            *pMetadata = pITBEntryMetadata;
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


/**
Advance both bit and index by one through the byte array
**/
#define BADTS_ADVANCE(abit, aindex, asize) \
    do {if (abit) {(abit)--;} else {(abit)=7;if(++(aindex)>=asize) {goto err_eof;}} } while (0)

#define BADTS_GET_BITS(stream, val, len, bit, index, size) \
    do { unsigned i_tmp; (val)=0; for ( i_tmp=0; i_tmp < (len); i_tmp++) { (val) <<= 1; (val) |= ((((uint32_t)stream[(index)])>>(bit))&0x1); BADTS_ADVANCE(bit, index, size);} } while (0)

#define BADTS_SET_BITS(stream, val, len, bit, index, size) \
    do { unsigned i_tmp; for ( i_tmp=(len); i_tmp > 0; i_tmp-- ) { (stream[(index)] |= ((((val)>>(i_tmp-1))&0x1)<<(bit))); BADTS_ADVANCE(bit, index, size); } } while (0)

static BERR_Code BAPE_MuxOutput_P_ParseAdtsMetadata(
    BAPE_MuxOutputHandle hMuxOutput,
    BAVC_AudioBufferDescriptor *pFrameDescriptor,
    BAVC_AudioMetadataDescriptor *pMetadataDescriptor
    )
{
    /* CDB Data is assumed to be a bytestream */
    unsigned bit=7;
    unsigned index=0;
    unsigned size=pFrameDescriptor->stCommon.uiLength;
    uint32_t value;
    uint32_t adtsId;
    uint32_t adtsLayer;
    uint32_t adtsObject;
    uint32_t adtsRateIndex;
    uint32_t adtsChanCfg;
    uint8_t *pBytestream = (uint8_t *)hMuxOutput->cdb.cached + pFrameDescriptor->stCommon.uiOffset;

    BADTS_GET_BITS(pBytestream, value, 12, bit, index, size);

    /* Check syncword */
    if ( value != 0xfff )
    {
        BDBG_MSG(("Bad ADTS Syncword - expected 0xfff got 0x%x [offset=%u]", value, pFrameDescriptor->stCommon.uiOffset));
        goto err_parse;
    }
    /* Get ID */
    BADTS_GET_BITS(pBytestream, adtsId, 1, bit, index, size);
    BDBG_MSG(("ADTS ID: %u", adtsId));
    /* Get Layer */
    BADTS_GET_BITS(pBytestream, adtsLayer, 2, bit, index, size);
    BDBG_MSG(("ADTS Layer: %u", adtsLayer));
    /* Skip Protection */
    BADTS_ADVANCE(bit, index, size);
    /* Get Object Type */
    BADTS_GET_BITS(pBytestream, adtsObject, 2, bit, index, size);
    BDBG_MSG(("ADTS ObjectType: %u", adtsObject));
    /* Get Sample Rate Index */
    BADTS_GET_BITS(pBytestream, adtsRateIndex, 4, bit, index, size);
    BDBG_MSG(("ADTS SR Index: %u", adtsRateIndex));
    /* Skip Private Bit */
    BADTS_ADVANCE(bit, index, size);
    /* Get Channel Config */
    BADTS_GET_BITS(pBytestream, adtsChanCfg, 3, bit, index, size);
    BDBG_MSG(("ADTS Channel Config: %u", adtsChanCfg));

    /* That's enough to get us what we need.  Convert Object Type to value for ASConfig. */
    adtsObject += 1;

    /* Zero out Target Bytestream */
    bit=7;
    index=0;
    pBytestream = pMetadataDescriptor->uProtocolData.stAac.auiASC;
    size = BAVC_AUDIO_SPECIFIC_CONFIG_MAX_LENGTH;
    BKNI_Memset(pBytestream, 0, size);
    /* Write out payload */
    BADTS_SET_BITS(pBytestream, adtsObject, 5, bit, index, size);   /* audioObjectType:5 */
    BADTS_SET_BITS(pBytestream, adtsRateIndex, 4, bit, index, size);   /* samplingFrequency:4 */
    BADTS_SET_BITS(pBytestream, adtsChanCfg, 4, bit, index, size);  /* channelConfiguration:4 */
    /* TODO: SBR? Doesn't seem supported in ADTS (ADTS Object Type is 2 bits, max val 3 + 1 = 4 and SBR is 5) */
    /* Begin GASpecificConfig */
    BADTS_ADVANCE(bit, index, size);    /* frameLenthFlag:1 */
    BADTS_ADVANCE(bit, index, size);    /* dependsOnCoreCoder:1 */
    BADTS_ADVANCE(bit, index, size);    /* extensionFlag:1 */
    if ( adtsChanCfg == 0 )
    {
        /* Program Config element is required.  We don't support this. */
        BDBG_WRN(("PCE Not supported for ADTS."));
        goto err_parse;
    }
    /* Audio Object Types 6 and 20 are not supported, no layerNr. */
    /* No extension flag, so no other data */
    pMetadataDescriptor->uProtocolData.stAac.uiASCLengthBits = (8*index) + (7-bit);
    pMetadataDescriptor->uProtocolData.stAac.uiASCLengthBytes = (bit==7)?index:index+1;
#if 0
    {
        unsigned i;
        for ( i = 0; i < pMetadataDescriptor->uProtocolData.stAac.uiASCLengthBytes; i++ )
        {
            BDBG_WRN(("ASC[%u] = 0x%02x", i, pMetadataDescriptor->uProtocolData.stAac.auiASC[i]));
        }
    }
#endif
    return BERR_SUCCESS;
err_eof:
    BDBG_WRN(("ADTS EOF"));
err_parse:
    return BERR_INVALID_PARAMETER;
}

/**
Advance both bit and index by one through the byte array
**/
#define BADTS_ADVANCE_WRAP(astream1, astream2, abit, aindex, asize, asize2, awrap) \
    do {if (abit) {(abit)--;} else {(abit)=7;if(++(aindex)>=asize) {if ( (asize2) > 0 && ((astream2) != NULL) ) { (asize)=(asize2);(astream1)=(astream2);(asize2)=0;(astream2)=NULL; (aindex)=0; (awrap)=true;} else { goto err_eof;}}} } while (0)

#define BADTS_SKIP_BITS_WRAP(stream1, stream2, len, bit, index, size, size2, wrap) \
    do { unsigned i_tmp; for ( i_tmp=0; i_tmp < (len); i_tmp++) { BADTS_ADVANCE_WRAP(stream1, stream2, bit, index, size, size2, wrap);} } while (0)

#define BADTS_GET_BITS_WRAP(stream1, stream2, val, len, bit, index, size, size2, wrap) \
    do { unsigned i_tmp; (val)=0; for ( i_tmp=0; i_tmp < (len); i_tmp++) { (val) <<= 1; (val) |= ((((uint32_t)stream1[(index)])>>(bit))&0x1); BADTS_ADVANCE_WRAP(stream1, stream2, bit, index, size, size2, wrap);} } while (0)

#define BADTS_HANDLE_HEADER_WRAP() \
do {\
if ( wrapped )\
{\
    /* Wrapped in header, no actual data */\
    hMuxOutput->descriptorInfo.adtsSegmentOffset[0] = segmentOffset;\
    hMuxOutput->descriptorInfo.adtsSegmentLength[0] = segmentLength;\
    hMuxOutput->descriptorInfo.adtsSegmentRawOffset[0] = segmentOffset;\
    hMuxOutput->descriptorInfo.adtsSegmentRawLength[0] = 0;\
    segmentLength = s1;\
    segmentOffset = (pBuf1-pBase);\
    segment=1;\
    wrapped = false;\
}} while (0)

void BAPE_MuxOutput_P_ParseAdtsSegments(
    BAPE_MuxOutputHandle hMuxOutput,
    const uint8_t *pBuffer,
    size_t bufferLength,
    const uint8_t *pWrapBuffer,
    size_t wrapBufferLength
    )
{
    const uint8_t *pBase = hMuxOutput->cdb.cached;
    bool wrapped=false;
    bool protection;
    size_t s1=bufferLength,s2=wrapBufferLength;
    const uint8_t *pBuf1=pBuffer, *pBuf2=pWrapBuffer;
    unsigned bit=7;
    unsigned index=0;
    unsigned frameLength, numBlocks;
    unsigned i;
    unsigned segment, blockLength;
    unsigned segmentOffset, segmentLength;
    unsigned rawOffset, rawLength;
    uint32_t val;
    /* Initialize default segment info */
    hMuxOutput->descriptorInfo.adtsSegment = 0;
    hMuxOutput->descriptorInfo.adtsNumSegments = 0;
    segment = 0;
    segmentOffset = (pBuf1 - pBase);    /* Segment starts at beginning of first buffer */
    segmentLength = s1;                 /* Segment length is until wraparound by default */
    /* Parse header */
    /* adts_fixed_header */
    BADTS_GET_BITS_WRAP(pBuf1, pBuf2, val, 12, bit, index, s1, s2, wrapped);    /* syncword:12 */
    BADTS_HANDLE_HEADER_WRAP();
    if ( val != 0xfff )
    {
        BDBG_MODULE_MSG(bape_adts,("ADTS syncword mismatch expected 0xfff got 0x%03x", val));
        goto err_parse;
    }
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);              /* ID:1 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);              /* layer:2 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_GET_BITS_WRAP(pBuf1, pBuf2, val, 1, bit, index, s1, s2, wrapped);     /* protection_absent:1 */
    BADTS_HANDLE_HEADER_WRAP();
    protection = (val)?false:true;  /* Bit indicates absence of protection */
    BADTS_SKIP_BITS_WRAP(pBuf1, pBuf2, 2, bit, index, s1, s2, wrapped);         /* profile_ObjectType:2 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_SKIP_BITS_WRAP(pBuf1, pBuf2, 4, bit, index, s1, s2, wrapped);         /* sampling_frequency_index:4 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);              /* private_bit:1 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_SKIP_BITS_WRAP(pBuf1, pBuf2, 3, bit, index, s1, s2, wrapped);         /* channel_configuration:3 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);              /* original_copy:1 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);              /* home:1 */
    BADTS_HANDLE_HEADER_WRAP();
    /* adts_variable_header */
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);              /* copyright_identification_bit:1 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_ADVANCE_WRAP(pBuf1, pBuf2, bit, index, s1, s2, wrapped);              /* copyright_identification_start:1 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_GET_BITS_WRAP(pBuf1, pBuf2, val, 13, bit, index, s1, s2, wrapped);    /* aac_frame_length:13 */
    BADTS_HANDLE_HEADER_WRAP();
    frameLength = val;  /* In Bytes */
    if ( frameLength != bufferLength + wrapBufferLength )
    {
        BDBG_MODULE_MSG(bape_adts,("Frame Length error.  Expected %lu got %u", (unsigned long)bufferLength+wrapBufferLength, frameLength));
        goto err_parse;
    }
    BADTS_SKIP_BITS_WRAP(pBuf1, pBuf2, 11, bit, index, s1, s2, wrapped);        /* aac_buffer_fullness:11 */
    BADTS_HANDLE_HEADER_WRAP();
    BADTS_GET_BITS_WRAP(pBuf1, pBuf2, val, 2, bit, index, s1, s2, wrapped);     /* number_of_raw_data_blocks_in_frame:2 */
    BADTS_HANDLE_HEADER_WRAP();
    numBlocks = val;
    BDBG_ASSERT(bit == 7);  /* We should be byte aligned now.  If not something has gone horribly wrong. */
    /* handle protection prior to data if required */
    if ( protection )
    {
        if ( numBlocks > 0 )
        {
            /* adts_header_error_check */
            for ( i = 1; i <= numBlocks; i++ )
            {
                BADTS_SKIP_BITS_WRAP(pBuf1, pBuf2, 16, bit, index, s1, s2, wrapped);    /* raw_data_block_position(i):16 */
                BADTS_HANDLE_HEADER_WRAP();
            }
        }
        BADTS_SKIP_BITS_WRAP(pBuf1, pBuf2, 16, bit, index, s1, s2, wrapped);            /* crc_check:16 */
        BADTS_HANDLE_HEADER_WRAP();
    }

    numBlocks++;    /* Original numBlocks was inclusive.  That doesn't make sense for dividing. */
    if ( segment == 1 )
    {
        /* We have wrapped in the header */
        blockLength = frameLength - index - hMuxOutput->descriptorInfo.adtsSegmentLength[0];  /* Subtract header bytes pre and post-wrap */
    }
    else
    {
        blockLength = frameLength - index;  /* Subtract header bytes */
    }
    if ( (blockLength % numBlocks) != 0 )
    {
        BDBG_MODULE_MSG(bape_adts,("Raw Frame length not a multiple of number of blocks.  Fail."));
        goto err_parse;
    }
    blockLength /= numBlocks;
    if ( protection )
    {
        blockLength -= 2;   /* Remove 2-byte CRC that comes after each access unit */
    }

    /* First data segment will start here. */
    rawOffset = (pBuf1-pBase)+index;
    rawLength = blockLength;

    /* Actual first segment starts wherever the header parsing started and contains the header data also */
    segmentOffset = pBuf1-pBase;
    if ( segment == 1 )
    {
        /* Wrapped in header */
        segmentLength = frameLength-hMuxOutput->descriptorInfo.adtsSegmentLength[0];
    }
    else
    {
        segmentLength = frameLength;
    }
    BDBG_MODULE_MSG(bape_adts,("S1 %lu Segment Len %u Frame Len %u Raw Len %u nblk %u protection %u index %u", (unsigned long)s1, segmentLength, frameLength, rawLength, numBlocks, protection, index));

    /* Iterate through number of blocks (it's not inclusive anymore) */
    for ( i = 0; i < numBlocks; )
    {
        /* Find segment boundary */
        hMuxOutput->descriptorInfo.adtsSegmentOffset[segment] = segmentOffset;
        hMuxOutput->descriptorInfo.adtsSegmentRawOffset[segment] = rawOffset;
        if ( segmentLength >= s1 )
        {
            /* Truncate Segment */
             hMuxOutput->descriptorInfo.adtsSegmentLength[segment] = s1;
             segmentLength -= s1;
             /* Determine if raw payload wrapped (it might wrap in segment CRC region instead) */
             if ( rawLength + (rawOffset-segmentOffset) > s1 )
             {
                 hMuxOutput->descriptorInfo.adtsSegmentRawLength[segment] = s1-(rawOffset-segmentOffset);
                 rawLength -= hMuxOutput->descriptorInfo.adtsSegmentRawLength[segment];
             }
             else
             {
                 hMuxOutput->descriptorInfo.adtsSegmentRawLength[segment] = rawLength;
                 rawLength = 0;
             }
             /* Wrap pointers */
             s1 = s2;
             pBuf1 = pBuf2;
             s2 = 0;
             pBuf2 = NULL;
             /* Start new segment */
             segmentOffset = pBuf1 - pBase;
             rawOffset = segmentOffset;
        }
        else
        {
            /* No Wrap.  Use full segment */
            hMuxOutput->descriptorInfo.adtsSegmentLength[segment] = segmentLength;
            hMuxOutput->descriptorInfo.adtsSegmentOffset[segment] = segmentOffset;
            hMuxOutput->descriptorInfo.adtsSegmentRawOffset[segment] = rawOffset;
            hMuxOutput->descriptorInfo.adtsSegmentRawLength[segment] = rawLength;
            s1 -= segmentLength;
            pBuf1 += segmentLength;
            segmentOffset += segmentLength;
            rawOffset = segmentOffset;     /* The next RAW block starts with the next segment */
            segmentLength = 0;
            rawLength = 0;
        }

        /* Determine if we need to update lengths */
        if ( 0 == segmentLength )
        {
            rawLength = blockLength;
            segmentLength = (protection)?rawLength + 2:rawLength;
            i++;    /* We consumed a data block */
        }

        segment++;
    }
    BDBG_ASSERT(segment <= BAPE_MUXOUTPUT_MAX_ADTS_SEGMENTS);
    BDBG_MODULE_MSG(bape_adts,("ADTS Frame has %u segments", segment));
    hMuxOutput->descriptorInfo.adtsNumSegments = segment;
#if 1
    for ( segment = 0; segment < hMuxOutput->descriptorInfo.adtsNumSegments; segment++ )
    {
        BDBG_MODULE_MSG(bape_adts,("Segment %u length %u", segment, hMuxOutput->descriptorInfo.adtsSegmentLength[segment]));
    }
#endif
    return; /* Success */
err_parse:
err_eof:
    hMuxOutput->descriptorInfo.adtsSegment = 0;
    hMuxOutput->descriptorInfo.adtsNumSegments = 1;
    hMuxOutput->descriptorInfo.adtsSegmentOffset[0] = pBuffer - pBase;
    hMuxOutput->descriptorInfo.adtsSegmentLength[0] = bufferLength;
    hMuxOutput->descriptorInfo.adtsSegmentRawOffset[0] = pBuffer - pBase;
    hMuxOutput->descriptorInfo.adtsSegmentRawLength[0] = bufferLength;
    if ( pWrapBuffer != NULL )
    {
        hMuxOutput->descriptorInfo.adtsNumSegments = 2;
        hMuxOutput->descriptorInfo.adtsSegmentOffset[1] = pWrapBuffer - pBase;
        hMuxOutput->descriptorInfo.adtsSegmentLength[1] = wrapBufferLength;
        hMuxOutput->descriptorInfo.adtsSegmentRawOffset[1] = pWrapBuffer - pBase;
        hMuxOutput->descriptorInfo.adtsSegmentRawLength[1] = wrapBufferLength;
    }
}

bool BAPE_MuxOutput_P_IsRunning(BAPE_MuxOutputHandle hMuxOutput)
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    return hMuxOutput->state == BAPE_MuxOutputState_Started ? true : false;
}

void BAPE_MuxOutput_GetInterruptHandlers(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_MuxOutputInterruptHandlers *pInterrupts /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = hMuxOutput->interrupts;
}

BERR_Code BAPE_MuxOutput_SetInterruptHandlers(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAPE_MuxOutputInterruptHandlers *pInterrupts /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(NULL != pInterrupts);

    BKNI_EnterCriticalSection();
    hMuxOutput->interrupts = *pInterrupts;
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

void BAPE_MuxOutput_P_Overflow_isr(BAPE_MuxOutputHandle hMuxOutput)
{
    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    if ( hMuxOutput->interrupts.overflow.pCallback_isr )
    {
        hMuxOutput->interrupts.overflow.pCallback_isr(
            hMuxOutput->interrupts.overflow.pParam1,
            hMuxOutput->interrupts.overflow.param2);
    }
}

static void BAPE_MuxOutput_P_CopyToHost(void *pDest, void *pSource, size_t numBytes, BMMA_Block_Handle mmaBlock)
{
    BMMA_FlushCache_isrsafe(mmaBlock, pSource, numBytes);
    BKNI_Memcpy(pDest, pSource, numBytes);
}

BERR_Code BAPE_MuxOutput_GetStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_MuxOutputStatus *pStatus    /* [out] */
    )
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_GenCdbItbStreamInfo streamInfo;

    BDBG_OBJECT_ASSERT(hMuxOutput, BAPE_MuxOutput);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BAPE_MuxOutputStatus));
    if ( hMuxOutput->hStage )
    {
        BREG_Handle hReg;
        BMMA_DeviceOffset readOffset;
        BMMA_DeviceOffset endOffset;
        BMMA_DeviceOffset baseOffset;
        BMMA_DeviceOffset validOffset;

        errCode = BDSP_Stage_GetStatus(hMuxOutput->hStage, &streamInfo, sizeof(streamInfo));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        pStatus->numFrames = streamInfo.ui32NumFrames;
        pStatus->numErrorFrames = streamInfo.ui32ErrorFrames;
        pStatus->numDroppedOverflowFrames = streamInfo.ui32DroppedOverflowFrames;
        pStatus->decodePTS = streamInfo.ui32DecodePTS;
        pStatus->encodePTS = ((uint64_t)streamInfo.ui32EncodePTSUpper << 32) + streamInfo.ui32EncodePTSLower;
        pStatus->escr = streamInfo.ui32ESCR;
        pStatus->stcSnapshot = ((uint64_t)streamInfo.ui32STCUpper << 32) + streamInfo.ui32STCLower;

        hReg = hMuxOutput->node.deviceHandle->regHandle;
        if (hMuxOutput->cdb.bufferInterface.base != 0)
        {
            baseOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.base);
            endOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.end);
            validOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.valid);
            readOffset = BREG_ReadAddr(hReg, hMuxOutput->cdb.bufferInterface.read);

            if ( endOffset != 0 && hMuxOutput->cdb.bufferInterface.inclusive )
            {
                endOffset += 1; /* end is inclusive */
            }

            if ( validOffset == readOffset )
            {
                pStatus->data.fifoDepth = 0;
            }
            else if ( validOffset > readOffset )
            {
                pStatus->data.fifoDepth = validOffset - readOffset;
            }
            else
            {
                pStatus->data.fifoDepth = endOffset - readOffset;
                pStatus->data.fifoDepth += validOffset - baseOffset;
            }
            pStatus->data.fifoSize = endOffset - baseOffset;

            baseOffset = BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.base);
            endOffset = BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.end);
            validOffset = BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.valid);
            readOffset = BREG_ReadAddr(hReg, hMuxOutput->itb.bufferInterface.read);

            if ( endOffset != 0 && hMuxOutput->itb.bufferInterface.inclusive )
            {
                endOffset += 1; /* end is inclusive */
            }

            if ( validOffset == readOffset )
            {
                pStatus->index.fifoDepth = 0;
            }
            else if ( validOffset > readOffset )
            {
                pStatus->index.fifoDepth = validOffset - readOffset;
            }
            else
            {
                pStatus->index.fifoDepth = endOffset - readOffset;
                pStatus->index.fifoDepth += validOffset - baseOffset;
            }
            pStatus->index.fifoSize = endOffset - baseOffset;
        }
    }


    return BERR_SUCCESS;
}
#else
void BAPE_MuxOutput_GetDefaultCreateSettings(
    BAPE_MuxOutputCreateSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_MuxOutput_Create(
    BAPE_Handle hApe,
    const BAPE_MuxOutputCreateSettings *pSettings,
    BAPE_MuxOutputHandle *pHandle
    )
{
    BSTD_UNUSED(hApe);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_MuxOutput_Destroy(
    BAPE_MuxOutputHandle hMuxOutput
    )
{
   BSTD_UNUSED(hMuxOutput);
}

void BAPE_MuxOutput_GetDefaultStartSettings(
    BAPE_MuxOutputStartSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_MuxOutput_Start(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAPE_MuxOutputStartSettings *pSettings
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_MuxOutput_Stop(
    BAPE_MuxOutputHandle hMuxOutput
    )
{
    BSTD_UNUSED(hMuxOutput);
}

BERR_Code BAPE_MuxOutput_GetBufferDescriptors(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAVC_AudioBufferDescriptor **pBuffer, /* [out] pointer to BAVC_AudioBufferDescriptor structs */
    size_t *pSize, /* [out] number of BAVC_AudioBufferDescriptor elements in pBuffer */
    const BAVC_AudioBufferDescriptor **pBuffer2, /* [out] pointer to BAVC_AudioBufferDescriptor structs after wrap around */
    size_t *pSize2 /* [out] number of BAVC_AudioBufferDescriptor elements in pBuffer2 */
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pSize);
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pSize2);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_MuxOutput_ConsumeBufferDescriptors(
    BAPE_MuxOutputHandle hMuxOutput,
    unsigned numBufferDescriptors /* must be <= pSize+pSize2 returned by last BAPE_MuxOutput_GetBufferDescriptors call. */
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(numBufferDescriptors);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_MuxOutput_GetBufferStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAVC_AudioBufferStatus *pBufferStatus    /* [out] */
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(pBufferStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#if !B_REFSW_MINIMAL
void BAPE_MuxOutput_GetConnector(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector *pConnector
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(pConnector);
}
#endif

BERR_Code BAPE_MuxOutput_AddInput(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_MuxOutput_RemoveInput(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_MuxOutput_RemoveAllInputs(
    BAPE_MuxOutputHandle hMuxOutput
    )
{
    BSTD_UNUSED(hMuxOutput);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_MuxOutput_GetInterruptHandlers(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_MuxOutputInterruptHandlers *pInterrupts /* [out] */
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(pInterrupts);
}

BERR_Code BAPE_MuxOutput_SetInterruptHandlers(
    BAPE_MuxOutputHandle hMuxOutput,
    const BAPE_MuxOutputInterruptHandlers *pInterrupts /* [out] */
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(pInterrupts);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_MuxOutput_GetStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAPE_MuxOutputStatus *pStatus    /* [out] */
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_MuxOutput_GetDelayStatus(
    BAPE_MuxOutputHandle hMuxOutput,
    BAVC_AudioCompressionStd codec,
    BAPE_MuxOutputDelayStatus *pStatus    /* [out] */
    )
{
    BSTD_UNUSED(hMuxOutput);
    BSTD_UNUSED(codec);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
