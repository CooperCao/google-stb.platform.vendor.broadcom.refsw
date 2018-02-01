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
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#ifdef BCHP_AUD_FMM_BF_ESR_REG_START
#include "bchp_aud_fmm_bf_esr.h"
#endif

#ifdef BCHP_AUD_FMM_BF_ESR1_H_REG_START
#include "bchp_aud_fmm_bf_esr1_h.h"
#endif

#ifdef BCHP_AUD_FMM_BF_ESR2_H_REG_START
#include "bchp_aud_fmm_bf_esr2_h.h"
#endif

#if defined BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR
  #define BAPE_BF_ADDRESS_STRIDE (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR - BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR)
#elif defined BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR
  #define BAPE_BF_ADDRESS_STRIDE (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)
#else
  #warning "UNSUPPORTED CHIP - update this code"
#endif

BDBG_MODULE(bape_bf_priv);
BDBG_FILE_MODULE(bape_dfifo);
BDBG_FILE_MODULE(bape_sfifo);
BDBG_FILE_MODULE(bape_fcisp);
BDBG_FILE_MODULE(bape_fci);

typedef struct BAPE_SfifoGroup
{
    bool allocated;
    bool started;
    bool ppmCorrection;
    unsigned numChannelPairs;
    unsigned sampleRate;
    BAPE_Handle deviceHandle;
    uint32_t sfifoIds[BAPE_ChannelPair_eMax];
    uint32_t adaptRateIds[BAPE_ChannelPair_eMax];
    BAPE_SfifoGroupSettings settings;
} BAPE_SfifoGroup;

typedef struct BAPE_DfifoGroup
{
    bool allocated;
    bool started;
    unsigned numChannelPairs;
    BAPE_Handle deviceHandle;
    uint32_t dfifoIds[BAPE_ChannelPair_eMax];
    BAPE_DfifoGroupSettings settings;
} BAPE_DfifoGroup;

static BERR_Code BAPE_Sfifo_P_GetBuffer(BAPE_SfifoGroupHandle handle, BAPE_BufferDescriptor *pBuffers, unsigned chPair, unsigned bufferNum);
static BERR_Code BAPE_Sfifo_P_CommitData (BAPE_SfifoGroupHandle handle, unsigned numBytes, unsigned chPair, unsigned bufferNum );
static BERR_Code BAPE_Sfifo_P_GetQueuedBytes(BAPE_SfifoGroupHandle handle, unsigned *pQueuedBytes, unsigned chPair, unsigned bufferNum );
static void BAPE_Bf_P_SfifoStarting(BAPE_SfifoGroupHandle handle);
static void BAPE_Bf_P_SfifoStopping(BAPE_SfifoGroupHandle handle);
static bool BAPE_Sfifo_P_IsEnabled(BAPE_Handle hApe, unsigned sfifoId);
static unsigned BAPE_Bf_P_GetFirstRunningSfifo(BAPE_Handle hApe, const uint32_t *pSfifoIds);

void BAPE_SfifoGroup_P_GetDefaultCreateSettings(
    BAPE_SfifoGroupCreateSettings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->numChannelPairs = 1;
    pSettings->ppmCorrection = false;
}

static void BAPE_Sfifo_P_SetGroup(BAPE_Handle handle, uint32_t sfifoId, uint32_t groupId)
{
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(sfifoId < BAPE_CHIP_MAX_SFIFOS);
    BDBG_ASSERT(groupId < BAPE_CHIP_MAX_SFIFOS);

    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + (4*sfifoId);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID, groupId);
    BREG_Write32(handle->regHandle, regAddr, regVal);
}

BERR_Code BAPE_SfifoGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_SfifoGroupCreateSettings *pSettings,
    BAPE_SfifoGroupHandle *pHandle  /* [out] */
    )
{
    unsigned i, sfifo, adaptrate=(unsigned)-1;
    BERR_Code errCode;
    BAPE_SfifoGroupHandle handle=NULL;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(pSettings->numChannelPairs <= BAPE_ChannelPair_eMax);

    /* Sanity Check */
    if ( pSettings->numChannelPairs > 1 && pSettings->ppmCorrection )
    {
        BDBG_ERR(("PPM Correction is not supported for multichannel data"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MODULE_MSG(bape_sfifo, ("Create Sfifo Group, %lu ch pairs", (unsigned long)pSettings->numChannelPairs));
    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_SFIFO_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->sfifoGroups[i]);
        if ( !deviceHandle->sfifoGroups[i]->allocated )
        {
            handle = deviceHandle->sfifoGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        BDBG_ERR(("Not enough Audio Sfifo resources to support this usage case."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Now search for the correct number of resources */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eSfifo, pSettings->numChannelPairs, &sfifo);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_alloc_sfifo;
    }
    if ( pSettings->ppmCorrection )
    {
        errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eAdaptiveRate, pSettings->numChannelPairs, &adaptrate);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_alloc_adaptrate;
        }
    }

    /* Successfully allocated resources.  Initialize Group */
    handle->allocated = true;
    handle->started = false;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->ppmCorrection = pSettings->ppmCorrection;
    handle->deviceHandle = deviceHandle;
    handle->sampleRate = 0;
    BKNI_Memset(&handle->settings, 0, sizeof(handle->settings));
    handle->settings.stereoData = true;
    handle->settings.signedData = true;
    handle->settings.sampleRepeatEnabled = deviceHandle->settings.rampPcmSamples;
    handle->settings.dataWidth = 32;
    handle->settings.defaultSampleRate = 48000;
    BKNI_Memset(handle->sfifoIds, 0xff, sizeof(handle->sfifoIds));
    BKNI_Memset(handle->adaptRateIds, 0xff, sizeof(handle->adaptRateIds));
    for ( i = 0; i < pSettings->numChannelPairs; i++ )
    {
        handle->sfifoIds[i] = sfifo + i;
        BDBG_MODULE_MSG(bape_sfifo, ("  chPair[%lu] using sfifo %lu", (unsigned long)i, (unsigned long)handle->sfifoIds[i]));
        BAPE_Sfifo_P_SetGroup(handle->deviceHandle, sfifo+i, sfifo);    /* Set HW Grouping */
    }
    if ( handle->ppmCorrection )
    {
        for ( i = 0; i < pSettings->numChannelPairs; i++ )
        {
            handle->adaptRateIds[i] = adaptrate + i;
            /* Adapt Rate -> SFIFO linkage is done at start time */
        }
    }
    *pHandle = handle;
    return BERR_SUCCESS;

    err_alloc_adaptrate:
    BAPE_P_FreeFmmResource(deviceHandle, BAPE_FmmResourceType_eSfifo, pSettings->numChannelPairs, sfifo);
    err_alloc_sfifo:
    return errCode;
}

void BAPE_SfifoGroup_P_Destroy(
    BAPE_SfifoGroupHandle handle
    )
{
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(!handle->started);
    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);

    /* Release Resources */
    if ( handle->ppmCorrection )
    {
        BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eAdaptiveRate, handle->numChannelPairs, handle->adaptRateIds[0]);
    }
    /* Clear SFIFO Grouping */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_Sfifo_P_SetGroup(handle->deviceHandle, handle->sfifoIds[i], handle->sfifoIds[i]);
    }
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eSfifo, handle->numChannelPairs, handle->sfifoIds[0]);
    BKNI_Memset(handle->sfifoIds, 0xff, sizeof(handle->sfifoIds));
    BKNI_Memset(handle->adaptRateIds, 0xff, sizeof(handle->adaptRateIds));
    handle->allocated = false;
}

void BAPE_SfifoGroup_P_GetSettings(
    BAPE_SfifoGroupHandle handle,
    BAPE_SfifoGroupSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    *pSettings = handle->settings;
}

BERR_Code BAPE_SfifoGroup_P_SetSettings(
    BAPE_SfifoGroupHandle handle,
    const BAPE_SfifoGroupSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( handle->started )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

BERR_Code BAPE_SfifoGroup_P_Start(
    BAPE_SfifoGroupHandle handle,
    bool enableOnly                 /* If true, a separate call to BAPE_SfifoGroup_P_Run_isr is required to
                                    start data flow.  If false, data flow will start immediately. */
    )
{
    uint32_t regAddr, regVal;
    unsigned i;
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( handle->started )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    BDBG_MODULE_MSG(bape_sfifo, ("%s", BSTD_FUNCTION));

    /* Program each SFIFO */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BMMA_DeviceOffset base, end, wrpoint, writeOffset;
        unsigned watermark;

        regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + (4*handle->sfifoIds[i]);
        regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
        regVal &= ~(
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, REVERSE_ENDIAN)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARED_SBUF_ID)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, DMA_READ_DISABLE)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION)|
#ifdef BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_STCSYNC_ENABLE_MASK
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, STCSYNC_ENABLE)|
#endif
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE)|
                   BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE)
                   );
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID, 1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH, handle->settings.highPriority?1:0);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, handle->settings.dataWidth==32?0:handle->settings.dataWidth);
        if ( handle->settings.master )
        {
            BDBG_ASSERT(handle->settings.master->allocated);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARED_SBUF_ID, handle->settings.master->sfifoIds[i]);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF, 1);
        }
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL, 1);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, DMA_READ_DISABLE, handle->settings.bypassMemory?1:0);
        if ( handle->settings.sampleRepeatEnabled && deviceHandle->settings.rampPcmSamples )
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE, 1);
        }
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY, handle->settings.loopAround?1:0);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION, handle->settings.wrpointEnabled?1:0);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE, handle->settings.stereoData?0:1);
        if ( handle->settings.interleaveData )
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE, 1);
        }
        else
        {
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE, 1);
        }
        BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

        /* Sign handling is in the GRP register */
        regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + (4*handle->sfifoIds[i]);
        regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB, handle->settings.signedData?0:1);
        BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

        /* Setup Ringbuffer Registers */
        regAddr = BAPE_P_SFIFO_TO_RDADDR_REG(handle->sfifoIds[i]);
        base = handle->settings.bufferInfo[2*i].base;
        end = base + handle->settings.bufferInfo[2*i].length - 1;
        watermark = handle->settings.bufferInfo[2*i].watermark;
        wrpoint = handle->settings.bufferInfo[2*i].wrpoint;
        writeOffset = handle->settings.bufferInfo[2*i].writeOffset;
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Write is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, base);  /* leave buf empty for now */
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Base is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* End is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, end);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Freefull is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, (BMMA_DeviceOffset)watermark);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* WRPOINT is last */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, wrpoint);
        BAPE_Sfifo_P_CommitData (handle,  writeOffset, i  , 0 ); /* now adjust write pointer for existing data */

        if ( handle->settings.interleaveData )
        {
            base = end = wrpoint = writeOffset = 0;
            watermark = 0;
        }
        else
        {
            base = handle->settings.bufferInfo[(2*i)+1].base;
            end = base + handle->settings.bufferInfo[(2*i)+1].length - 1;
            watermark = handle->settings.bufferInfo[(2*i)+1].watermark;
            wrpoint = handle->settings.bufferInfo[(2*i)+1].wrpoint;
            writeOffset = handle->settings.bufferInfo[(2*i)+1].writeOffset;
        }
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Next RDADDR */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Write is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, base);  /* leave buf empty for now */
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Base is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* End is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, end);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Freefull is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, (BMMA_DeviceOffset)watermark);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* WRPOINT is last */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, wrpoint);
        BAPE_Sfifo_P_CommitData (handle,  writeOffset, i  , 1 ); /* now adjust write pointer for existing data */

#ifdef BCHP_AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi_ARRAY_BASE
        {
            /* Byte Swap if necessary */
            BAPE_Reg_P_FieldList regFieldList;
            regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi_ARRAY_BASE + (4*handle->sfifoIds[i]);
            BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);

            if (handle->settings.reverseEndian)
            {
                switch(handle->settings.dataWidth)
                {
                    case 32:
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_REORDER_ENABLE, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_3_SEL, 0);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_2_SEL, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_1_SEL, 2);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_0_SEL, 3);
                        break;
                    case 16:
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_REORDER_ENABLE, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_3_SEL, 0);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_2_SEL, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_1_SEL, 2);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_0_SEL, 3);
#else
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_REORDER_ENABLE, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_3_SEL, 2);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_2_SEL, 3);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_1_SEL, 0);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_0_SEL, 1);
#endif
                        break;
                    case 24:
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_REORDER_ENABLE, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_3_SEL, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_2_SEL, 2);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_1_SEL, 3);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_0_SEL, 0);
                        break;

                    default:
                        BDBG_WRN(("%s invalid data width value(%d)",BSTD_FUNCTION,handle->settings.dataWidth));
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_REORDER_ENABLE, 0);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_3_SEL, 3);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_2_SEL, 2);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_1_SEL, 1);
                        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_0_SEL, 0);
                        break;
                }
            }
            else
            {
                BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_REORDER_ENABLE, 0);
                BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_3_SEL, 3);
                BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_2_SEL, 2);
                BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_1_SEL, 1);
                BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_SOURCECH_BYTE_REORDERi, BYTE_0_SEL, 0);
            }
            BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);
        }
#endif
    }


    /* Enable all SFIFO's first. */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BDBG_MSG(("Enabling SFIFO %u", handle->sfifoIds[i]));
        BDBG_MODULE_MSG(bape_sfifo, ("  chPair[%lu]: enable Sfifo %lx", (unsigned long)i, (unsigned long)handle->sfifoIds[i]));
        regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + (4*handle->sfifoIds[i]);
        regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
        regVal |= BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE);
        BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    }

    if ( !enableOnly )
    {
        /* Run */
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            BDBG_MODULE_MSG(bape_sfifo, ("  chPair[%lu]: run Sfifo %lx", (unsigned long)i, (unsigned long)handle->sfifoIds[i]));
            regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + (4*handle->sfifoIds[i]);
            BREG_Write32(deviceHandle->regHandle, regAddr, 1);
        }
    }

    BAPE_Bf_P_SfifoStarting(handle);

    /* Enable Adaptive Rate Controllers if Required */
    if ( handle->ppmCorrection )
    {
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            BKNI_EnterCriticalSection();
#ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_1_CFG - BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG)*handle->adaptRateIds[i]);
            /* This register must be manipulated in critical section, it is programmed at sample rate changes as well */
            regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);

            regVal &= ~(BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, TRIWINDOW_WIDTH_SEL)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_MASTER_ENABLE));

            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL, handle->sfifoIds[i]));
            /* TODO: hardcoding window width field to 8 ie actual window width of 256. This has to be changed later
               to be taken from application */
            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, TRIWINDOW_WIDTH_SEL, 8));
            /* Earlier, for a decode channel FW was setting the enable flag.
            Now we're doing AdaptRate control for PCM channels also. So let the
            PI set this flag always */
            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE, 1));
            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_MASTER_ENABLE, 1));
#ifndef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0
            regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD, 0xffff);
#endif
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, regVal);

#ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0 + 4*handle->adaptRateIds[i];
            regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0, ADAPTIVE_RATE_THRESHOLD);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0, ADAPTIVE_RATE_THRESHOLD, 0xffff);
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, regVal);
#endif
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + 4*handle->adaptRateIds[i];
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, 0);
#else
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_ELEMENT_SIZE/8)*handle->adaptRateIds[i]);
            /* This register must be manipulated in critical section, it is programmed at sample rate changes as well */
            regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);

            regVal &= ~(BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPTIVE_SFIFO_SEL)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, TRIWINDOW_WIDTH_SEL)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, AUTOMATIC_RATE_ENABLE)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPTIVE_RATE_MASTER_ENABLE));

            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPTIVE_SFIFO_SEL, handle->sfifoIds[i]));
            /* TODO: hardcoding window width field to 8 ie actual window width of 256. This has to be changed later
               to be taken from application */
            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, TRIWINDOW_WIDTH_SEL, 8));
            /* Earlier, for a decode channel FW was setting the enable flag.
            Now we're doing AdaptRate control for PCM channels also. So let the
            PI set this flag always */
            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, AUTOMATIC_RATE_ENABLE, 1));
            regVal |= (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPTIVE_RATE_MASTER_ENABLE, 1));
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, regVal);

#ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLDi_ARRAY_BASE
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLDi_ARRAY_BASE + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLDi_ARRAY_ELEMENT_SIZE/8)*handle->adaptRateIds[i]);
            regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLDi, ADAPTIVE_RATE_THRESHOLD);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLDi, ADAPTIVE_RATE_THRESHOLD, 0xffff);
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, regVal);
#endif
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNTi_ARRAY_BASE + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNTi_ARRAY_ELEMENT_SIZE/8)*handle->adaptRateIds[i]);
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, 0);
#endif

            BAPE_SfifoGroup_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
            BKNI_LeaveCriticalSection();
        }
    }

    handle->started = true;
    return BERR_SUCCESS;
}

void BAPE_SfifoGroup_P_Stop(
    BAPE_SfifoGroupHandle handle
    )
{
    uint32_t regAddr, regVal;
    unsigned i;
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( !handle->started )
    {
        return;
    }
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    BAPE_Bf_P_SfifoStopping(handle);

    if ( handle->ppmCorrection )
    {
#ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG
        /* Workaround for HWAIO-20 (SW7425-681) - Program WRCNT to 0 prior to clearing master enable bit. */
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_1 - BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0)*handle->adaptRateIds[i]);
            BREG_Write32(deviceHandle->regHandle, regAddr, 0);
        }
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_1_CFG - BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG)*handle->adaptRateIds[i]);
            /* This register must be manipulated in critical section, it is programmed at sample rate changes as well */
            BKNI_EnterCriticalSection();
            regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);
            regVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_MASTER_ENABLE));
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, regVal);
            BKNI_LeaveCriticalSection();
        }
#else
        /* Workaround for HWAIO-20 (SW7425-681) - Program WRCNT to 0 prior to clearing master enable bit. */
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNTi_ARRAY_BASE + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNTi_ARRAY_ELEMENT_SIZE/8)*handle->adaptRateIds[i]);
            BREG_Write32(deviceHandle->regHandle, regAddr, 0);
        }
        for ( i = 0; i < handle->numChannelPairs; i++ )
        {
            regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_ELEMENT_SIZE/8)*handle->adaptRateIds[i]);
            /* This register must be manipulated in critical section, it is programmed at sample rate changes as well */
            BKNI_EnterCriticalSection();
            regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);
            regVal &= ~(BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, AUTOMATIC_RATE_ENABLE)|
                        BCHP_MASK (AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPTIVE_RATE_MASTER_ENABLE));
            BREG_Write32_isr(deviceHandle->regHandle, regAddr, regVal);
            BKNI_LeaveCriticalSection();
        }
#endif
    }
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t sfifo;
        unsigned timeout;

        sfifo = handle->sfifoIds[i];

        BDBG_MSG(("Disabling PLAY_RUN for SFIFO %u", sfifo));
        regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + (4*sfifo);
        BREG_Write32(deviceHandle->regHandle, regAddr, 0);

        /* Wait for group flow to stop */
        for ( timeout = 1000; timeout > 0; timeout-- )
        {
            regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON);
            if ( 0 == (regVal & (1<<sfifo)) )
            {
                break;
            }
            BKNI_Delay(1000); /* 1 millisecond */
        }
        if ( 0 == timeout )
        {
            regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON);
            BDBG_WRN(("Timeout waiting for SFIFO %u flow to stop [0x%08x]", sfifo, regVal));
        }

        /* Disable Source Channels */
        BDBG_MSG(("Disabling SFIFO %u", sfifo));
        regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + (4*sfifo);
        regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE);
        BREG_Write32(deviceHandle->regHandle, regAddr, regVal);

        /* Wait for the source channel to stop */
        for ( timeout = 1000; timeout > 0; timeout-- )
        {
            regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_ENABLE);
            if ( 0 == (regVal & (1<<sfifo)) )
            {
                break;
            }
            BKNI_Delay(1000); /* 1 millisecond */
        }
        if ( 0 == timeout )
        {
            regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_ENABLE);
            BDBG_WRN(("Timeout waiting for SFIFO %u enable status to clear [0x%08x]", sfifo, regVal));
        }
    }

    /* Reset Ringbuffers */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = BAPE_P_SFIFO_TO_RDADDR_REG(handle->sfifoIds[i]);
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Write is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Base is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* End is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Freefull is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* WRPOINT is last */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Next RDADDR */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Write is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Base is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* End is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Freefull is next */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* WRPOINT is last */
        BREG_WriteAddr(deviceHandle->regHandle, regAddr, 0);
    }

    handle->started = false;
}

void BAPE_SfifoGroup_P_Run_isr(
    BAPE_SfifoGroupHandle handle
    )
{
    uint32_t regAddr;
    unsigned i;
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( !handle->started )
    {
        BDBG_ERR(("SFIFO Group %p is not started, can not run.", (void *)handle));
        return;
    }
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BDBG_MSG(("Enabling PLAY_RUN for SFIFO %u", handle->sfifoIds[i]));
        regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + (4*handle->sfifoIds[i]);
        BREG_Write32(deviceHandle->regHandle, regAddr, 1);
    }
}

void BAPE_SfifoGroup_P_Halt_isr(
    BAPE_SfifoGroupHandle handle
    )
{
    uint32_t regAddr;
    unsigned i;
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( !handle->started )
    {
        BDBG_ERR(("SFIFO Group %p is not started, can not halt.", (void *)handle));
        return;
    }
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t sfifo;
        unsigned timeout;
        uint32_t regVal;

        sfifo = handle->sfifoIds[i];

        BDBG_MSG(("Disabling PLAY_RUN for SFIFO %u", handle->sfifoIds[i]));
        regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + (4*handle->sfifoIds[i]);
        BREG_Write32(deviceHandle->regHandle, regAddr, 0);

        /* Wait for group flow to stop */
        for ( timeout = 1000; timeout > 0; timeout-- )
        {
            regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON);
            if ( 0 == (regVal & (1<<sfifo)) )
            {
                break;
            }
            BKNI_Delay(1000); /* 1 millisecond */
        }
        if ( 0 == timeout )
        {
            regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON);
            BDBG_WRN(("Timeout waiting for SFIFO %u flow to stop [0x%08x]", sfifo, regVal));
        }
    }
}

void BAPE_SfifoGroup_P_SetSampleRate_isr(
    BAPE_SfifoGroupHandle handle,
    unsigned sampleRate
    )
{
    uint32_t regAddr, regVal;
    unsigned i;
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BKNI_ASSERT_ISR_CONTEXT();

    if ( !handle->ppmCorrection )
    {
        return;
    }

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
#ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG
        regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + 4*handle->adaptRateIds[i];

        regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);
        regVal &= ~(BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE));
        switch ( sampleRate )
        {
        case 32000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 0);
            break;
        default:
        case 48000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 1);
            break;
        case 96000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 2);
            break;
        case 192000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 3);
            break;
        case 44100:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 4);
            break;
        case 88200:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 5);
            break;
        case 176400:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 6);
            break;
        }
#else
        regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_ELEMENT_SIZE/8)*handle->adaptRateIds[i]);

        regVal = BREG_Read32_isr(deviceHandle->regHandle, regAddr);
        regVal &= ~(BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE));
        switch ( sampleRate )
        {
        case 32000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE, 0);
            break;
        default:
        case 48000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE, 1);
            break;
        case 96000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE, 2);
            break;
        case 192000:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE, 3);
            break;
        case 44100:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE, 4);
            break;
        case 88200:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE, 5);
            break;
        case 176400:
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_CFGi, ADAPT_SAMPLINGRATE, 6);
            break;
        }
#endif
        BREG_Write32_isr(deviceHandle->regHandle, regAddr, regVal);
    }
}

void BAPE_SfifoGroup_P_GetOutputFciIds_isrsafe(
    BAPE_SfifoGroupHandle handle,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    )
{
    unsigned i;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(NULL != pFciGroup);
    BAPE_FciIdGroup_Init(pFciGroup);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        pFciGroup->ids[i] = BAPE_FCI_BASE_SFIFO | handle->sfifoIds[i];    /* SFIFO FCI ID Base is 0x000*/
    }
}

BERR_Code BAPE_SfifoGroup_P_GetBuffer(
    BAPE_SfifoGroupHandle handle,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    )
{
    BERR_Code errCode;
    unsigned chPair;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(NULL != pBuffers);

    BKNI_Memset(pBuffers, 0, sizeof(BAPE_BufferDescriptor));

    /* TODO: Handle non-interleaved and multichannel */
    pBuffers->interleaved = handle->settings.interleaveData;
    if (pBuffers->interleaved)
    {
        pBuffers->numBuffers = handle->numChannelPairs;
    }
    else
    {
        pBuffers->numBuffers = handle->numChannelPairs * 2;
    }

    for (chPair = 0; chPair < handle->numChannelPairs; chPair++)
    {
        if (pBuffers->interleaved)
        {
            errCode = BAPE_Sfifo_P_GetBuffer(handle, pBuffers, chPair, 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }
        else
        {
            errCode = BAPE_Sfifo_P_GetBuffer(handle, pBuffers, chPair, 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }

            errCode = BAPE_Sfifo_P_GetBuffer(handle, pBuffers, chPair, 1);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }

    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Sfifo_P_GetBuffer(
    BAPE_SfifoGroupHandle handle,
    BAPE_BufferDescriptor *pBuffers,
    unsigned chPair,                /*0,1,2,3*/
    unsigned bufferNum              /*0,1*/
    )
{
    BMMA_DeviceOffset rd,wr,base,rdaddr,wraddr;
    unsigned bufferSize, sfifoId;
    unsigned padding = BAPE_CHIP_SFIFO_PADDING;

    sfifoId = handle->sfifoIds[chPair];
    bufferSize = handle->settings.bufferInfo[chPair*2 + bufferNum].length;
    rd = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_RDADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));
    wr = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_WRADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));
    base = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_BASEADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));

    BDBG_MSG(("PB GetBuffer: RDADDR " BDBG_UINT64_FMT "WRADDR " BDBG_UINT64_FMT "BASEADDR " BDBG_UINT64_FMT "Size %u", BDBG_UINT64_ARG(rd), BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(base), bufferSize));

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#else
    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#endif

    BDBG_CASSERT(BAPE_Channel_eLeft == 0);
    BDBG_CASSERT(BAPE_Channel_eRight == 1);
    BDBG_ASSERT(wraddr >= base);
    pBuffers->buffers[bufferNum].block = handle->settings.bufferInfo[chPair*2 + bufferNum].block;
    pBuffers->buffers[bufferNum].pBuffer = (uint8_t*)handle->settings.bufferInfo[chPair*2 + bufferNum].pBuffer + (wraddr - base);
    pBuffers->buffers[bufferNum].pWrapBuffer = handle->settings.bufferInfo[chPair*2 + bufferNum].pBuffer;
    BDBG_ASSERT(pBuffers->buffers[bufferNum].pBuffer);
    BDBG_ASSERT(pBuffers->buffers[bufferNum].pWrapBuffer);

    if ( wraddr > rdaddr )
    {
        pBuffers->bufferSize = bufferSize - (unsigned)(wraddr-base);
        pBuffers->wrapBufferSize = (unsigned)(rdaddr-base);
    }
    else if ( wraddr < rdaddr )
    {
        pBuffers->bufferSize = (unsigned)(rdaddr-wraddr);
    }
    else    /* equal */
    {
        if (
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
             BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
             BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP)
#else
             BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
             BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP)
#endif
           )
        {
            /* Toggle bits are equal, buffer is empty. */
            pBuffers->bufferSize = bufferSize - (unsigned)(wraddr-base);
            pBuffers->wrapBufferSize = bufferSize - pBuffers->bufferSize;
        }
        else
        {
            /* Toggle bit mismatch, buffer is full. */
            pBuffers->bufferSize = 0;
        }
    }

    /* Don't allow entire buffer to be filled, need gap between rd/wr for master/slave setups */
    if ( pBuffers->wrapBufferSize > padding )
    {
        pBuffers->wrapBufferSize -= padding;
        padding = 0;
    }
    else
    {
        padding -= pBuffers->wrapBufferSize;
        pBuffers->wrapBufferSize = 0;
    }
    if ( pBuffers->bufferSize > padding )
    {
        pBuffers->bufferSize -= padding;
        padding = 0;
    }
    else
    {
        pBuffers->bufferSize = 0;
    }

    /* Make sure wrap pointers are NULL if we have no data */
    if (pBuffers->interleaved)
    {
        if ( pBuffers->wrapBufferSize == 0 )
        {
            pBuffers->buffers[bufferNum*2].pWrapBuffer = NULL;
        }
        if ( pBuffers->bufferSize == 0 )
        {
            pBuffers->buffers[bufferNum*2].pBuffer = NULL;
        }
        if ( pBuffers->bufferSize == 0 && pBuffers->wrapBufferSize == 0 )
        {
            pBuffers->buffers[bufferNum*2].block = NULL;
        }
    }
    else
    {
        if ( pBuffers->wrapBufferSize == 0 )
        {
            pBuffers->buffers[bufferNum].pWrapBuffer = NULL;
        }
        if ( pBuffers->bufferSize == 0 )
        {
            pBuffers->buffers[bufferNum].pBuffer = NULL;
        }
        if ( pBuffers->bufferSize == 0 && pBuffers->wrapBufferSize == 0 )
        {
            pBuffers->buffers[bufferNum].block = NULL;
        }
    }

    if ( pBuffers->bufferSize > 0 )
    {
        BMMA_FlushCache(pBuffers->buffers[bufferNum].block, pBuffers->buffers[bufferNum].pBuffer, pBuffers->bufferSize);
    }
    if ( pBuffers->wrapBufferSize > 0 )
    {
        BMMA_FlushCache(pBuffers->buffers[bufferNum].block, pBuffers->buffers[bufferNum].pWrapBuffer, pBuffers->wrapBufferSize);
    }
    return BERR_SUCCESS;
}



BERR_Code BAPE_SfifoGroup_P_CommitData(
    BAPE_SfifoGroupHandle handle,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    )
{
    BERR_Code errCode;
    unsigned chPair;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    /* TODO: Handle multichannel and non-interleaved */
    for (chPair = 0; chPair < handle->numChannelPairs; chPair++)
    {
        if (handle->settings.interleaveData)
        {
            errCode = BAPE_Sfifo_P_CommitData(handle, numBytes, chPair, 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }
        else
        {
            errCode = BAPE_Sfifo_P_CommitData(handle, numBytes, chPair, 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
            errCode = BAPE_Sfifo_P_CommitData(handle, numBytes, chPair, 1);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Sfifo_P_CommitData (BAPE_SfifoGroupHandle handle,
    unsigned numBytes,                   /* Number of bytes written into the buffer */
    unsigned chPair,                     /*0,1,2,3*/
    unsigned bufferNum                   /*0,1*/
    )
{

    BMMA_DeviceOffset rd,wr,base,rdaddr,wraddr;
    unsigned sfifoId;
    unsigned bufferSize;

    sfifoId = handle->sfifoIds[chPair];
    bufferSize = handle->settings.bufferInfo[chPair*2 + bufferNum].length;
    rd = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_RDADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));
    wr = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_WRADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));
    base = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_BASEADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));

    BDBG_MSG(("PB Commit: RDADDR " BDBG_UINT64_FMT " WRADDR " BDBG_UINT64_FMT " BASEADDR " BDBG_UINT64_FMT " Size %u", BDBG_UINT64_ARG(rd), BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(base), bufferSize));

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#else
    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#endif

    /* Make sure the write pointer doesn't try and pass read */
    if ( rdaddr > wraddr )
    {
        if ( (wraddr + (BMMA_DeviceOffset)numBytes) > rdaddr )
        {
            BDBG_ERR(("Playback: Attempt to write more data than available in the buffer."));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        else if ( (wraddr + (BMMA_DeviceOffset)numBytes) == rdaddr )
        {
            if (
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
                 BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
                 BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP)
#else
                 BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
                 BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP)
#endif
                )
            {
                /* If the toggle bit is the same we will overflow. */
                BDBG_ERR(("Playback: Attempt to write more data than available in the buffer."));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }
    }
    else    /* rdaddr <= wraddr */
    {
        if ( (wraddr + (BMMA_DeviceOffset)numBytes) > (base + (BMMA_DeviceOffset)bufferSize) )
        {
            BDBG_ERR(("Playback: Attempt to write more data than available in the buffer."));
            BDBG_ERR(("wraddr " BDBG_UINT64_FMT ", rdaddr " BDBG_UINT64_FMT ", numBytes %u, base " BDBG_UINT64_FMT ", bufferSize %u", BDBG_UINT64_ARG(wraddr), BDBG_UINT64_ARG(rdaddr), numBytes, BDBG_UINT64_ARG(base), bufferSize));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    /* The request is legal.  Update the write pointer. */
    wraddr += numBytes;
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    wr &= ~BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
    if ( wraddr == ((BMMA_DeviceOffset)bufferSize + base) )
    {
        BDBG_MSG(("Inverting toggle bit - was " BDBG_UINT64_FMT "now " BDBG_UINT64_FMT, BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(wr ^ (unsigned)BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP, 1))));
        wr ^= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP, 1);   /* flip the toggle bit */
        wraddr = base;
    }
    wr |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR, wraddr);
#else
    wr &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
    if ( wraddr == ((BMMA_DeviceOffset)bufferSize + base) )
    {
        BDBG_MSG(("Inverting toggle bit - was " BDBG_UINT64_FMT " now " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(wr ^ (unsigned)BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP, 1))));
        wr ^= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP, 1);   /* flip the toggle bit */
        wraddr = base;
    }
    wr |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR, wraddr);
#endif
    BREG_WriteAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_WRADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE), wr);

    return BERR_SUCCESS;
}



BERR_Code BAPE_SfifoGroup_P_Flush(
    BAPE_SfifoGroupHandle handle
    )
{
    unsigned bufferNum;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    /* TODO: Handle multichannel */

    /* Flush by making write pointer equal to read pointer */
    if (handle->settings.interleaveData)
    {
        for (bufferNum = 0; bufferNum < handle->numChannelPairs; bufferNum++)
        {
            BREG_WriteAddr(handle->deviceHandle->regHandle,
                         BAPE_P_SFIFO_TO_WRADDR_REG(handle->sfifoIds[bufferNum]),
                         BREG_ReadAddr(handle->deviceHandle->regHandle,
                                     BAPE_P_SFIFO_TO_RDADDR_REG(handle->sfifoIds[bufferNum])));
        }
    }
    else
    {
        for (bufferNum = 0; bufferNum < handle->numChannelPairs; bufferNum++)
        {
            BREG_WriteAddr(handle->deviceHandle->regHandle,
                         BAPE_P_SFIFO_TO_WRADDR_REG(handle->sfifoIds[bufferNum]),
                         BREG_ReadAddr(handle->deviceHandle->regHandle,
                                     BAPE_P_SFIFO_TO_RDADDR_REG(handle->sfifoIds[bufferNum])));

            BREG_WriteAddr(handle->deviceHandle->regHandle,
                         BAPE_P_SFIFO_TO_WRADDR_REG(handle->sfifoIds[bufferNum]) + BAPE_P_RINGBUFFER_STRIDE,
                         BREG_ReadAddr(handle->deviceHandle->regHandle,
                                     BAPE_P_SFIFO_TO_RDADDR_REG(handle->sfifoIds[bufferNum] + BAPE_P_RINGBUFFER_STRIDE)));
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_SfifoGroup_P_GetQueuedBytes(
    BAPE_SfifoGroupHandle handle,
    unsigned *pQueuedBytes
    )
{
    BERR_Code errCode;
    uint32_t chPair;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(NULL != pQueuedBytes);

    /* TODO: Handle non-interleaved and multichannel */
    for (chPair = 0; chPair < handle->numChannelPairs; chPair++)
    {
        if (handle->settings.interleaveData)
        {
            errCode = BAPE_Sfifo_P_GetQueuedBytes(handle, pQueuedBytes, chPair, 0 );
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }
        else
        {
            errCode = BAPE_Sfifo_P_GetQueuedBytes(handle, pQueuedBytes, chPair, 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
            errCode = BAPE_Sfifo_P_GetQueuedBytes(handle, pQueuedBytes, chPair, 1);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
            }
        }

    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Sfifo_P_GetQueuedBytes(
    BAPE_SfifoGroupHandle handle,
    unsigned *pQueuedBytes,
    unsigned chPair,       /*0,1,2,3*/
    unsigned bufferNum     /*0,1*/
    )
{
    BMMA_DeviceOffset rd,wr,base,rdaddr,wraddr;
    unsigned bufferSize, sfifoId, queuedBytes;

    sfifoId = handle->sfifoIds[chPair];
    bufferSize = handle->settings.bufferInfo[chPair*2 + bufferNum].length;
    rd = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_RDADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));
    wr = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_WRADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));
    base = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_BASEADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#else
    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#endif

    if ( wraddr > rdaddr )
    {
        queuedBytes = wraddr-rdaddr;
    }
    else if ( wraddr < rdaddr )
    {
        queuedBytes = (wraddr-base)+(bufferSize-(rdaddr-base));
    }
    else    /* equal */
    {
        if (
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
             BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
             BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP)
#else
             BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
             BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP)
#endif
            )
        {
            /* Toggle bits are equal, buffer is empty. */
            queuedBytes = 0;
        }
        else
        {
            /* Toggle bit mismatch, buffer is full. */
            queuedBytes = bufferSize;
        }
    }
    *pQueuedBytes = queuedBytes;

    return BERR_SUCCESS;
}


void BAPE_SfifoGroup_P_GetReadAddress(
    BAPE_SfifoGroupHandle handle,
    unsigned chPair,       /*0,1,2,3*/
    unsigned bufferNum,     /*0,1*/
    BMMA_DeviceOffset *pReadPtr
    )
{
    BMMA_DeviceOffset read;
    unsigned sfifoId;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(NULL != pReadPtr);

    sfifoId = handle->sfifoIds[chPair];
    read = BREG_ReadAddr(handle->deviceHandle->regHandle, BAPE_P_SFIFO_TO_RDADDR_REG(sfifoId) + (bufferNum * BAPE_P_RINGBUFFER_STRIDE));

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    *pReadPtr = BCHP_GET_FIELD_DATA(read, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
#else
    *pReadPtr = BCHP_GET_FIELD_DATA(read, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
#endif
}

BERR_Code BAPE_SfifoGroup_P_SetFreemarkInterrupt(
    BAPE_SfifoGroupHandle handle,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    return BAPE_P_SetSourceChannelFreemarkInterrupt(handle->deviceHandle, handle->sfifoIds[0], callback_isr, pParam1, param2);
}

void BAPE_SfifoGroup_P_RearmFreemarkInterrupt(
    BAPE_SfifoGroupHandle handle
    )
{
    uint32_t regVal;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    regVal = 1<<handle->sfifoIds[0];
#ifdef BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK, regVal);
#else
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_REARM_FREE_MARK, regVal);
#endif
}

uint32_t BAPE_SfifoGroup_P_GetHwIndex(
    BAPE_SfifoGroupHandle handle,
    BAPE_ChannelPair channelPair
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( channelPair >= handle->numChannelPairs )
    {
        return 0xFFFFFFFF;
    }
    else
    {
        return handle->sfifoIds[channelPair];
    }
}

uint32_t BAPE_SfifoGroup_P_GetAdaptRateWrcntAddress(
    BAPE_SfifoGroupHandle handle,
    BAPE_ChannelPair channelPair
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( channelPair >= handle->numChannelPairs )
    {
        return 0xFFFFFFFF;
    }
    else
    {
        #ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNTi_ARRAY_BASE
        return BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNTi_ARRAY_BASE + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNTi_ARRAY_ELEMENT_SIZE/8)*handle->adaptRateIds[channelPair]);
        #else
        return BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + ((BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_1 - BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0)*handle->adaptRateIds[channelPair]);
        #endif
    }
}

void BAPE_DfifoGroup_P_GetDefaultCreateSettings(
    BAPE_DfifoGroupCreateSettings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->numChannelPairs = 1;
}

BERR_Code BAPE_DfifoGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DfifoGroupCreateSettings *pSettings,
    BAPE_DfifoGroupHandle *pHandle  /* [out] */
    )
{
    unsigned i, dfifo;
    BERR_Code errCode;
    BAPE_DfifoGroupHandle handle=NULL;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(pSettings->numChannelPairs <= BAPE_ChannelPair_eMax);

    BDBG_MODULE_MSG(bape_dfifo, ("Create Dfifo Group, %lu ch pairs", (unsigned long)pSettings->numChannelPairs));

    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_DFIFO_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->dfifoGroups[i]);
        if ( !deviceHandle->dfifoGroups[i]->allocated )
        {
            handle = deviceHandle->dfifoGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        BDBG_ERR(("%s: No free Dfifos.", BSTD_FUNCTION));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Now search for the correct number of resources */
    /* TODO: DFIFO's technically don't group so they could be allocated individually
       rather than as a group if that helps optimize resource usage */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eDfifo, pSettings->numChannelPairs, &dfifo);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_alloc_dfifo;
    }

    /* Successfully allocated resources.  Initialize Group */
    handle->allocated = true;
    handle->started = false;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->deviceHandle = deviceHandle;
    BKNI_Memset(&handle->settings, 0, sizeof(handle->settings));
    BAPE_FciIdGroup_Init(&handle->settings.input);
    handle->settings.dataWidth = 32;
    BKNI_Memset(handle->dfifoIds, 0xff, sizeof(handle->dfifoIds));
    for ( i = 0; i < pSettings->numChannelPairs; i++ )
    {
        handle->dfifoIds[i] = dfifo + i;
        /* DFIFO's don't support grouping, rather the inputs are grouped */
        BDBG_MODULE_MSG(bape_dfifo, ("  chPair[%lu] using dfifo %lu", (unsigned long)i, (unsigned long)handle->dfifoIds[i]));
    }
    *pHandle = handle;
    return BERR_SUCCESS;

    err_alloc_dfifo:
    return errCode;
}

void BAPE_DfifoGroup_P_Destroy(
    BAPE_DfifoGroupHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(!handle->started);
    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);

    /* Release Resources */
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eDfifo, handle->numChannelPairs, handle->dfifoIds[0]);
    BKNI_Memset(handle->dfifoIds, 0xff, sizeof(handle->dfifoIds));
    handle->allocated = false;
}

void BAPE_DfifoGroup_P_GetSettings(
    BAPE_DfifoGroupHandle handle,
    BAPE_DfifoGroupSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    *pSettings = handle->settings;
}

BERR_Code BAPE_DfifoGroup_P_SetSettings(
    BAPE_DfifoGroupHandle handle,
    const BAPE_DfifoGroupSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( handle->started )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    switch ( pSettings->dataWidth )
    {
    case 0:
    case 32:
#if BAPE_CHIP_DFIFO_SUPPORTS_16BIT_CAPTURE
    case 16:
#endif
        /* Supported */
        break;
    default:
        BDBG_ERR(("This chip does not support %u-bit DFIFO Capture", pSettings->dataWidth));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

static void BAPE_DfifoGroup_P_ApplySettings(
    BAPE_DfifoGroupHandle handle
    )
{
    uint32_t regAddr, regVal, regMask, regData;
    unsigned i;
    bool playFromCapture, captureToSfifo;
    BREG_Handle regHandle;

    regHandle = handle->deviceHandle->regHandle;

    BDBG_MODULE_MSG(bape_dfifo, ("%s", BSTD_FUNCTION));
    if ( NULL == handle->settings.linkedSfifo )
    {
        /* No SFIFO was provided.  This should capture to memory only. */
        BDBG_ASSERT(handle->settings.bypassMemory == false);
        playFromCapture = false;
        captureToSfifo = false;
        BDBG_MODULE_MSG(bape_dfifo, ("  Dfifo memory bypass mode"));
    }
    else
    {
        /* SFIFO provided.  Determine if we are bypassing memory (CAPTURE_TO_SOURCEFIFO) or capturing to the SFIFO ringbuffers (PLAY_FROM_CAPTURE). */
        captureToSfifo = handle->settings.bypassMemory;
        playFromCapture = !captureToSfifo;
        BDBG_MODULE_MSG(bape_dfifo, ("  Dfifo to Sfifo mode %lu, play from capture mode %lu", (unsigned long)captureToSfifo, (unsigned long)playFromCapture));
    }


    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        uint32_t sfifoId = (handle->settings.linkedSfifo) ? handle->settings.linkedSfifo->sfifoIds[i] : BAPE_Bf_P_GetFirstRunningSfifo(handle->deviceHandle, NULL);
        uint32_t fciId = handle->settings.input.ids[i];
        BMMA_DeviceOffset base, end, watermark;
#ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
        regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0;
        regMask =
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_ENABLE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, BUFFER_PAIR_ENABLE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, DESTFIFO_SIZE_DOUBLE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PLAY_FROM_CAPTURE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_TO_SOURCEFIFO)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PLAY_FROM_CAPTURE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, SOURCE_FIFO_ID)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, NOT_PAUSE_WHEN_FULL)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, FCI_CAP_ID)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_ID_HIGH)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, REVERSE_ENDIAN)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_SEQ_ID_VALID);
        regData =
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, BUFFER_PAIR_ENABLE, (handle->settings.interleaveData)?0:1) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, DESTFIFO_SIZE_DOUBLE, (handle->settings.interleaveData)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_TO_SOURCEFIFO, (captureToSfifo)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, PLAY_FROM_CAPTURE, (playFromCapture)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, SOURCE_FIFO_ID, sfifoId) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, FCI_CAP_ID, fciId) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_ID_HIGH, (handle->settings.highPriority)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, REVERSE_ENDIAN, (handle->settings.reverseEndian)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_SEQ_ID_VALID, 1);
#if BAPE_CHIP_DFIFO_SUPPORTS_16BIT_CAPTURE
        regMask |= BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE);
        if ( handle->settings.dataWidth == 16 )
        {
            regData |= BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_MODE, Compressed);
        }
        else
        {
            regData |= BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_MODE, PCM);
        }
#endif
#else
        #if BDBG_DEBUG_BUILD
        if ( handle->settings.linkedSfifo )
        {
            BDBG_MODULE_MSG(bape_fci, ("DFIFO %u setup for SFIFO ID %u", handle->dfifoIds[i], sfifoId));
            BDBG_MODULE_MSG(bape_fci, ("  chPair[%lu]: FCI %lx -> DFIFO ID %lu -> SFIFO ID %lu",(unsigned long)i, (unsigned long) fciId, (unsigned long)handle->dfifoIds[i],(unsigned long)sfifoId));
        }
        else
        {
            BDBG_MODULE_MSG(bape_fci, ("DFIFO %u setup for host or DSP capture", handle->dfifoIds[i]));
            BDBG_MODULE_MSG(bape_fci, ("  chPair[%lu]: FCI %lx -> DFIFO ID %lu",(unsigned long)i, (unsigned long) fciId, (unsigned long)handle->dfifoIds[i]));
        }
        #endif
        regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE;
        regAddr += handle->dfifoIds[i]*(BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_ELEMENT_SIZE/8);
        regMask =
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, BUFFER_PAIR_ENABLE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, DESTFIFO_SIZE_DOUBLE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, SOURCE_FIFO_ID)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, NOT_PAUSE_WHEN_FULL)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, FCI_CAP_ID)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_ID_HIGH)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, REVERSE_ENDIAN)|
        BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID);
        regData =
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, BUFFER_PAIR_ENABLE, (handle->settings.interleaveData)?0:1) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, DESTFIFO_SIZE_DOUBLE, (handle->settings.interleaveData)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO, (captureToSfifo)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE, (playFromCapture)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, SOURCE_FIFO_ID, sfifoId) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, FCI_CAP_ID, fciId) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_ID_HIGH, (handle->settings.highPriority)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, REVERSE_ENDIAN, (handle->settings.reverseEndian)?1:0) |
        BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID, 1);
#if BAPE_CHIP_DFIFO_SUPPORTS_16BIT_CAPTURE
        regMask |= BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE);
        if ( handle->settings.dataWidth == 16 )
        {
            regData |= BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE, Compressed);
        }
        else
        {
            regData |= BCHP_FIELD_ENUM(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE, PCM);
        }
#endif
#endif

        /* configure channel based on above settings */
        regVal = BREG_Read32(regHandle, regAddr);
        regVal = (regVal & ~regMask) | regData;
        BREG_Write32(regHandle, regAddr, regVal);

        /* program ringbuffer addresses */
        /* Setup Ringbuffer Registers */
        regAddr = BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i]);
        base = handle->settings.bufferInfo[2*i].base;
        end = base + handle->settings.bufferInfo[2*i].length - 1;
        watermark = handle->settings.bufferInfo[2*i].watermark;
        BREG_WriteAddr(regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Write is next */
        BREG_WriteAddr(regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Base is next */
        BREG_WriteAddr(regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* End is next */
        BREG_WriteAddr(regHandle, regAddr, end);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Freefull is next */
        BREG_WriteAddr(regHandle, regAddr, watermark);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* WRPOINT is last */
        BREG_WriteAddr(regHandle, regAddr, 0);
        if ( handle->settings.interleaveData )
        {
            base=end=watermark=0;
        }
        else
        {
            base = handle->settings.bufferInfo[(2*i)+1].base;
            end = base + handle->settings.bufferInfo[(2*i)+1].length - 1;
            watermark = handle->settings.bufferInfo[(2*i)+1].watermark;
        }
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Next RDADDR */
        BREG_WriteAddr(regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Write is next */
        BREG_WriteAddr(regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Base is next */
        BREG_WriteAddr(regHandle, regAddr, base);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* End is next */
        BREG_WriteAddr(regHandle, regAddr, end);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* Freefull is next */
        BREG_WriteAddr(regHandle, regAddr, watermark);
        regAddr += BAPE_BF_ADDRESS_STRIDE;   /* WRPOINT is last */
        BREG_WriteAddr(regHandle, regAddr, 0);
    }
}

static void BAPE_DfifoGroup_P_SetCaptureEnable(
    BAPE_DfifoGroupHandle handle,
    uint32_t value
    )
{
    uint32_t baseAddr, stride, mask, data, regVal, regAddr;
    unsigned i;
    BAPE_Handle deviceHandle;

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

#ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
    baseAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0;
    stride = 0;
    mask =
    BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_ENABLE)|
    BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_SEQ_ID_VALID);
    data =
    BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_ENABLE, value)|
    BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_SEQ_ID_VALID, value);
#else
    baseAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE;
    stride = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_ELEMENT_SIZE/8;
    mask =
    BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE)|
    BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID);
    data =
    BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE, value)|
    BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID, value);
#endif

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = baseAddr + (handle->dfifoIds[i]*stride);
        regVal = BREG_Read32(deviceHandle->regHandle, regAddr);
        regVal = (regVal & ~mask) | data;
        BREG_Write32(deviceHandle->regHandle, regAddr, regVal);
    }
}

static void BAPE_DfifoGroup_P_SetCaptureRun_isr(
    BAPE_DfifoGroupHandle handle,
    uint32_t value
    )
{
    uint32_t baseAddr;
    unsigned stride;
    unsigned i;

    /* These registers are not read/modify/write and thus are safe from either task or interrupt context */

    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);

#ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRL0
    baseAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRL0;
    stride = 0;
#else
    baseAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_BASE;
    stride = BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_ELEMENT_SIZE/8;
#endif

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BREG_Write32_isr(handle->deviceHandle->regHandle, baseAddr + (handle->dfifoIds[i])*stride, value);
    }
}

BERR_Code BAPE_DfifoGroup_P_Start(
    BAPE_DfifoGroupHandle handle,
    bool enableOnly                 /* If true, a separate call to BAPE_DfifoGroup_P_Run_isr is required to
                                    start data flow.  If false, data flow will start immediately. */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    /* Refresh Settings */
    BAPE_DfifoGroup_P_ApplySettings(handle);

    /* Enable */
    BAPE_DfifoGroup_P_SetCaptureEnable(handle, 1);

    handle->started = true;

    if ( !enableOnly )
    {
        BKNI_EnterCriticalSection();
        /* Run */
        BAPE_DfifoGroup_P_SetCaptureRun_isr(handle, 1);
        BKNI_LeaveCriticalSection();
    }

    return BERR_SUCCESS;
}

void BAPE_DfifoGroup_P_Stop(
    BAPE_DfifoGroupHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    BKNI_EnterCriticalSection();
    /* Halt */
    BAPE_DfifoGroup_P_SetCaptureRun_isr(handle, 0);
    BKNI_LeaveCriticalSection();

    /* Enable */
    BAPE_DfifoGroup_P_SetCaptureEnable(handle, 0);

    /* Reset Ringbuffer Registers to 0 */

    handle->started = false;
}

#if !B_REFSW_MINIMAL
void BAPE_DfifoGroup_P_Run_isr(
    BAPE_DfifoGroupHandle handle
    )
{
    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( !handle->started )
    {
        BDBG_ERR(("DFIFO Group %p is not started, can not run.", (void *)handle));
        return;
    }

    BAPE_DfifoGroup_P_SetCaptureRun_isr(handle, 1);
}

void BAPE_DfifoGroup_P_Halt_isr(
    BAPE_DfifoGroupHandle handle
    )
{
    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( !handle->started )
    {
        BDBG_ERR(("DFIFO Group %p is not started, can not halt.", (void *)handle));
        return;
    }
    BAPE_DfifoGroup_P_SetCaptureRun_isr(handle, 0);
}
#endif

typedef struct {
    BMMA_DeviceOffset rd, wr, base;
} BAPE_P_BufferPtrs;

BERR_Code BAPE_DfifoGroup_P_GetBuffer(
    BAPE_DfifoGroupHandle handle,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    )
{
    BMMA_DeviceOffset rd, wr, base, wrap;
    BAPE_P_BufferPtrs ptrs[BAPE_Channel_eMax];
    unsigned bufferSize, wrapBufferSize=0, fifoSize, i, minBufferSize=0x7fffffff;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(NULL != pBuffers);

    BKNI_Memset(pBuffers, 0, sizeof(*pBuffers));

    if ( handle->settings.interleaveData )
    {
        pBuffers->numBuffers = handle->numChannelPairs;
        pBuffers->interleaved = true;
    }
    else
    {
        pBuffers->numBuffers = 2*handle->numChannelPairs;
        pBuffers->interleaved = false;
    }
    fifoSize = handle->settings.bufferInfo[0].length;
    if ( !handle->started )
    {
        return BERR_SUCCESS;
    }

    /* Read registers, holding Critical Section */
    BKNI_EnterCriticalSection();
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        ptrs[2*i].rd = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i]));
        ptrs[2*i].wr = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_WRADDR_REG(handle->dfifoIds[i]));
        ptrs[2*i].base = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_BASEADDR_REG(handle->dfifoIds[i]));
        if ( handle->settings.interleaveData == false )
        {
            ptrs[(2*i)+1].rd = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i])+BAPE_P_RINGBUFFER_STRIDE);
            ptrs[(2*i)+1].wr = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_WRADDR_REG(handle->dfifoIds[i])+BAPE_P_RINGBUFFER_STRIDE);
            ptrs[(2*i)+1].base = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_BASEADDR_REG(handle->dfifoIds[i])+BAPE_P_RINGBUFFER_STRIDE);
        }
    }
    BKNI_LeaveCriticalSection();

    /* process the ptrs, converting to usable addresses */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        rd = ptrs[2*i].rd;
        wr = ptrs[2*i].wr;
        base = ptrs[2*i].base;

        /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
        wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
               BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

        /* Mask off toggle bits */
        rd = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
        wr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#else
        wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
               BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

        /* Mask off toggle bits */
        rd = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
        wr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#endif

        /* Get base address to read from */
        BDBG_ASSERT(rd >= base);
        pBuffers->buffers[2*i].pBuffer = (uint8_t*)handle->settings.bufferInfo[2*i].pBuffer + (rd - base);
        if ( !pBuffers->buffers[2*i].pBuffer )
        {
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }

        pBuffers->buffers[2*i].block = handle->settings.bufferInfo[2*i].block;

        /* Compute size of contiguous space */
        if ( wrap )
        {
            bufferSize = (uint64_t)fifoSize - (rd-base);
            wrapBufferSize = wr-base;
        }
        else
        {
            bufferSize = (wr - rd);
        }

        if ( bufferSize < minBufferSize )
        {
            minBufferSize = bufferSize;
        }

        if ( handle->settings.interleaveData == false )
        {
            rd = ptrs[(2*i)+1].rd;
            wr = ptrs[(2*i)+1].wr;
            base = ptrs[(2*i)+1].base;

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
            /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */
            wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
                   BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

            /* Mask off toggle bits */
            rd = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
            wr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#else
            /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */
            wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
                   BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

            /* Mask off toggle bits */
            rd = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
            wr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#endif

            /* Get base address to read from */
            BDBG_ASSERT(rd >= base);
            pBuffers->buffers[(2*i)+1].pBuffer = (uint8_t*)handle->settings.bufferInfo[(2*i)+1].pBuffer + (rd - base);
            if ( !pBuffers->buffers[(2*i)+1].pBuffer )
            {
                return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            }

            pBuffers->buffers[(2*i)+1].block = handle->settings.bufferInfo[(2*i)+1].block;

            /* Compute size of contiguous space */
            if ( wrap )
            {
                bufferSize = fifoSize - (rd-base);
                wrapBufferSize = wr-base;
            }
            else
            {
                bufferSize = (wr - rd);
            }

            if ( bufferSize < minBufferSize )
            {
                minBufferSize = bufferSize;
            }
        }
    }

    pBuffers->bufferSize = minBufferSize;
    pBuffers->wrapBufferSize = wrapBufferSize;
    return BERR_SUCCESS;
}

BERR_Code BAPE_DfifoGroup_P_CommitData_isr(
    BAPE_DfifoGroupHandle handle,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    )
{
    BMMA_DeviceOffset rd, wr, base, wrap, rdaddr, wraddr, wrapBit, prevReadAddr;
    unsigned fifoSize, i;
    BMMA_DeviceOffset rdaddrRegs[BAPE_Channel_eMax];

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BKNI_ASSERT_ISR_CONTEXT();

    if ( !handle->started )
    {
        return BERR_SUCCESS;
    }

    fifoSize = handle->settings.bufferInfo[0].length;
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        /* Read registers */
        rd = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i]));
        wr = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_WRADDR_REG(handle->dfifoIds[i]));
        base = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_BASEADDR_REG(handle->dfifoIds[i]));

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
        /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */
        wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
               BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

        /* Mask off toggle bits */
        rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
        wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#else
        /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */
        wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
               BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

        /* Mask off toggle bits */
        rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
        wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#endif

        /* Check for potential overflow */
        if ( wrap )
        {
            if (numBytes > ((base+fifoSize-rdaddr)+(wraddr-base)) )
            {
                BDBG_ERR(("Invalid number of bytes provided to BAPE_OutputCapture_ConsumeData [wrap]"));
                BDBG_ERR(("rd " BDBG_UINT64_FMT " wr " BDBG_UINT64_FMT " base " BDBG_UINT64_FMT " size %#x numBytes %#x", BDBG_UINT64_ARG(rd), BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(base), fifoSize, numBytes));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }
        else
        {
            if ( (rdaddr+numBytes) > wraddr )
            {
                BDBG_ERR(("Invalid number of bytes provided to BAPE_OutputCapture_ConsumeData [no wrap]"));
                BDBG_ERR(("rd " BDBG_UINT64_FMT " wr " BDBG_UINT64_FMT " base " BDBG_UINT64_FMT " size %#x numBytes %#x", BDBG_UINT64_ARG(rd), BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(base), fifoSize, numBytes));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }

        /* Update read pointer */
        prevReadAddr = rdaddr;
        rdaddr += numBytes;
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
        wrapBit = rd & BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#else
        wrapBit = rd & BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#endif

        if ( (rdaddr-base) >= fifoSize )
        {
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
            wrapBit ^= BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#else
            wrapBit ^= BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#endif
            rdaddr = base+numBytes - (base+fifoSize-prevReadAddr);
        }
        rdaddr |= wrapBit;
        rdaddrRegs[2*i] = rdaddr;

        if ( handle->settings.interleaveData == false )
        {
            /* Read registers */
            rd = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i])+BAPE_P_RINGBUFFER_STRIDE);
            wr = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_WRADDR_REG(handle->dfifoIds[i])+BAPE_P_RINGBUFFER_STRIDE);
            base = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_BASEADDR_REG(handle->dfifoIds[i])+BAPE_P_RINGBUFFER_STRIDE);

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
            /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */
            wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
                   BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

            /* Mask off toggle bits */
            rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
            wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#else
            /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */
            wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^
                   BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

            /* Mask off toggle bits */
            rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
            wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
#endif

            /* Check for potential overflow */
            if ( wrap )
            {
                if (numBytes > ((base+fifoSize-rdaddr)+(wraddr-base)) )
                {
                    BDBG_ERR(("Invalid number of bytes provided to BAPE_OutputCapture_ConsumeData [wrap]"));
                    BDBG_ERR(("rd " BDBG_UINT64_FMT " wr " BDBG_UINT64_FMT " base " BDBG_UINT64_FMT " size %#x numBytes %#x", BDBG_UINT64_ARG(rd), BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(base), fifoSize, numBytes));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }
            else
            {
                if ( (rdaddr+numBytes) > wraddr )
                {
                    BDBG_ERR(("Invalid number of bytes provided to BAPE_OutputCapture_ConsumeData [no wrap]"));
                    BDBG_ERR(("rd " BDBG_UINT64_FMT " wr " BDBG_UINT64_FMT " base " BDBG_UINT64_FMT " size %#x numBytes %#x", BDBG_UINT64_ARG(rd), BDBG_UINT64_ARG(wr), BDBG_UINT64_ARG(base), fifoSize, numBytes));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }

            /* Update read pointer */
            prevReadAddr = rdaddr;
            rdaddr += numBytes;
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
            wrapBit = rd & BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#else
            wrapBit = rd & BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#endif

            if ( (rdaddr-base) >= fifoSize )
            {
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
                wrapBit ^= BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#else
                wrapBit = rd & BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
#endif
                rdaddr = base+numBytes - (base+fifoSize-prevReadAddr);
            }

            rdaddr |= wrapBit;
            rdaddrRegs[(2*i)+1] = rdaddr;
        }
    }

    /* Move the actual read pointer after validation */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BREG_WriteAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i]), rdaddrRegs[2*i]);
        if ( false == handle->settings.interleaveData )
        {
            BREG_WriteAddr_isrsafe(handle->deviceHandle->regHandle, BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i])+BAPE_P_RINGBUFFER_STRIDE, rdaddrRegs[(2*i)+1]);
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_DfifoGroup_P_Flush_isr(
    BAPE_DfifoGroupHandle handle
    )
{
    BMMA_DeviceOffset regAddr, regVal, offset, wrap;
    unsigned i;

    /* To flush, first compute where the read address should be in terms
       of offset from base.  Then apply to all buffers. */
    regAddr = BAPE_P_DFIFO_TO_WRADDR_REG(handle->dfifoIds[0]);
    regVal = BREG_ReadAddr_isrsafe(handle->deviceHandle->regHandle, regAddr);
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    wrap = regVal & BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
    offset = (regVal & BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR)) - handle->settings.bufferInfo[0].base;
#else
    wrap = regVal & BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
    offset = (regVal & BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR, RINGBUF_RDADDR)) - handle->settings.bufferInfo[0].base;
#endif
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        regAddr = BAPE_P_DFIFO_TO_RDADDR_REG(handle->dfifoIds[i]);
        regVal = wrap | (offset+handle->settings.bufferInfo[2*i].base);
        BREG_WriteAddr_isrsafe(handle->deviceHandle->regHandle, regAddr, regVal);
        if ( handle->settings.interleaveData )
        {
            regAddr += BAPE_P_RINGBUFFER_STRIDE;
            regVal = wrap | (offset+handle->settings.bufferInfo[(2*i)+1].base);
            BREG_WriteAddr_isrsafe(handle->deviceHandle->regHandle, regAddr, regVal);
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_DfifoGroup_P_SetFullmarkInterrupt(
    BAPE_DfifoGroupHandle handle,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    )
{
    return BAPE_P_SetDfifoFullmarkInterrupt(handle->deviceHandle,
                                            handle->dfifoIds[0],
                                            callback_isr,
                                            pParam1,
                                            param2);
}

void BAPE_DfifoGroup_P_RearmFullmarkInterrupt_isr(
    BAPE_DfifoGroupHandle handle
    )
{
    uint32_t regVal;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BKNI_ASSERT_ISR_CONTEXT();

#ifdef BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK
    /* Clear the ESR status */
    regVal = BCHP_FIELD_DATA(AUD_FMM_BF_ESR2_H_STATUS_CLEAR, DEST_RINGBUF_0_EXCEED_FULLMARK, 1) << handle->dfifoIds[0];
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR2_H_STATUS_CLEAR, regVal);
    /* Rearm */
    regVal = (1<<BCHP_SHIFT(AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK, REARM_FULLMARK))<<handle->dfifoIds[0];
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK, regVal);
#else
    /* Clear the ESR status */
    regVal = BCHP_FIELD_DATA(AUD_FMM_BF_ESR_ESR4_STATUS_CLEAR, DEST_RINGBUF_0_EXCEED_FULLMARK, 1) << handle->dfifoIds[0];
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR4_STATUS_CLEAR, regVal);
    /* Rearm */
    regVal = (1<<BCHP_SHIFT(AUD_FMM_BF_CTRL_REARM_FULL_MARK, REARM_FULLMARK))<<handle->dfifoIds[0];
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_REARM_FULL_MARK, regVal);
#endif
}

BERR_Code BAPE_DfifoGroup_P_SetOverflowInterrupt(
    BAPE_DfifoGroupHandle handle,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    )
{
    return BAPE_P_SetDfifoOverflowInterrupt(handle->deviceHandle,
                                            handle->dfifoIds[0],
                                            callback_isr,
                                            pParam1,
                                            param2);
}

void BAPE_DfifoGroup_P_RearmOverflowInterrupt_isr(
    BAPE_DfifoGroupHandle handle
    )
{
    uint32_t regVal;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BKNI_ASSERT_ISR_CONTEXT();

#ifdef BCHP_AUD_FMM_BF_ESR1_H_STATUS_CLEAR
    /* Clear the ESR status */
    regVal = BCHP_FIELD_DATA(AUD_FMM_BF_ESR1_H_STATUS_CLEAR, DEST_RINGBUF_0_OVERFLOW, 1) << handle->dfifoIds[0];
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_STATUS_CLEAR, regVal);
#else
    /* Clear the ESR status */
    regVal = BCHP_FIELD_DATA(AUD_FMM_BF_ESR_ESR2_STATUS_CLEAR, DEST_RINGBUF_0_OVERFLOW, 1) << handle->dfifoIds[0];
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR_ESR2_STATUS_CLEAR, regVal);
#endif
}

uint32_t BAPE_DfifoGroup_P_GetHwIndex(
    BAPE_DfifoGroupHandle handle,
    BAPE_ChannelPair channelPair
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( channelPair >= handle->numChannelPairs )
    {
        return 0xFFFFFFFF;
    }
    else
    {
        return handle->dfifoIds[channelPair];
    }
}

/* FCI Splitter structs */

#if BAPE_CHIP_MAX_FCISPLITTER_GROUPS > 0
typedef struct BAPE_FciSplitterGroup
{
    bool allocated;
    bool started;
    unsigned numChannelPairs;
    unsigned sampleRate;
    BAPE_Handle deviceHandle;
    uint32_t fciSpIds[BAPE_ChannelPair_eMax];
    BAPE_FciSplitterGroupSettings settings;
} BAPE_FciSplitterGroup;

typedef struct BAPE_FciSplitterOutputGroup
{
    bool allocated;
    bool started;
    unsigned numChannelPairs;
    unsigned sampleRate;
    BAPE_Handle deviceHandle;
    uint32_t fciSpOutIds[BAPE_ChannelPair_eMax];
    BAPE_FciSplitterGroupHandle spHandle;
} BAPE_FciSplitterOutputGroup;

/* FCI Splitter Helper function declarations */
/* input */
static void      BAPE_FciSplitter_P_SetInputFci(BAPE_Handle handle, uint32_t fciSp, uint32_t fci);
static void      BAPE_FciSplitter_P_SetGroup(BAPE_Handle handle, uint32_t fciSp, uint32_t groupId);
static BERR_Code BAPE_FciSplitterGroup_P_AllocateInputResources(BAPE_FciSplitterGroupHandle handle);
static void      BAPE_FciSplitterGroup_P_FreeInputResources(BAPE_FciSplitterGroupHandle handle);
static unsigned  BAPE_FciSplitterGroup_P_GetConnectedOutputGroupIndexes(BAPE_FciSplitterGroupHandle handle);
static unsigned  BAPE_FciSplitterGroup_P_GetRunningOutputGroupIndexes(BAPE_FciSplitterGroupHandle handle);
static unsigned  BAPE_FciSplitterGroup_P_StopOutputGroups(BAPE_FciSplitterGroupHandle handle);
static BERR_Code BAPE_FciSplitterGroup_P_StartOutputGroups(BAPE_FciSplitterGroupHandle handle, unsigned groups);

/* output */
static void BAPE_FciSplitter_P_SetOutputGroupInputs(BAPE_Handle handle, uint32_t fciSpOutput, uint32_t fciSpInput, uint32_t count);
static void BAPE_FciSplitter_P_EnableOutputGroup(BAPE_Handle handle, uint32_t fciSpOutput, unsigned count, unsigned enable);
static BERR_Code BAPE_FciSplitterOutputGroup_P_Start(BAPE_FciSplitterOutputGroupHandle handle);
static void      BAPE_FciSplitterOutputGroup_P_Stop(BAPE_FciSplitterOutputGroupHandle handle);

/* FCI Splitter Public functions */

void BAPE_FciSplitterGroup_P_GetDefaultCreateSettings(
    BAPE_FciSplitterGroupCreateSettings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(BAPE_FciSplitterGroupCreateSettings));
    pSettings->numChannelPairs = 1;
}

BERR_Code BAPE_FciSplitterGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_FciSplitterGroupCreateSettings *pSettings,
    BAPE_FciSplitterGroupHandle *pHandle  /* [out] */
    )
{
    unsigned i;
    BAPE_FciSplitterGroupHandle handle=NULL;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);

    BDBG_MODULE_MSG(bape_fcisp, ("Create Fci Splitter Group"));
    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->fciSplitterGroups[i]);
        if ( !deviceHandle->fciSplitterGroups[i]->allocated )
        {
            handle = deviceHandle->fciSplitterGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        BDBG_MODULE_ERR(bape_fcisp, ("No more FCI Splitters available"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Successfully allocated resources.  Initialize Group */
    handle->allocated = true;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->deviceHandle = deviceHandle;
    handle->sampleRate = 0;
    BKNI_Memset(&handle->settings, 0, sizeof(handle->settings));
    BKNI_Memset(handle->fciSpIds, 0xff, sizeof(handle->fciSpIds));
    *pHandle = handle;
    return BERR_SUCCESS;
}

void BAPE_FciSplitterGroup_P_Destroy(
    BAPE_FciSplitterGroupHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);

    if ( BAPE_FciSplitterGroup_P_GetRunningOutputGroupIndexes(handle) )
    {
        BDBG_MODULE_WRN(bape_fcisp, ("Forcefully stopping downstream FCI splitter output groups during Destroy"));
        BAPE_FciSplitterGroup_P_Stop(handle);
    }

    handle->allocated = false;
}

void BAPE_FciSplitterGroup_P_GetSettings(
    BAPE_FciSplitterGroupHandle handle,
    BAPE_FciSplitterGroupSettings *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    *pSettings = handle->settings;
}

BERR_Code BAPE_FciSplitterGroup_P_SetSettings(
    BAPE_FciSplitterGroupHandle handle,
    const BAPE_FciSplitterGroupSettings *pSettings
    )
{
    BERR_Code errCode;
    unsigned running = 0;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    BDBG_MODULE_MSG(bape_fcisp, ("%s", BSTD_FUNCTION));

    running = BAPE_FciSplitterGroup_P_GetRunningOutputGroupIndexes(handle);
    if ( running )
    {
        BAPE_FciSplitterGroup_P_Stop(handle);
    }

    handle->settings = *pSettings;
    handle->numChannelPairs = handle->settings.numChannelPairs;

    if ( running )
    {
        errCode = BAPE_FciSplitterGroup_P_Start(handle);
        if ( errCode != BERR_SUCCESS )
        {
            return BERR_TRACE(BERR_UNKNOWN);
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_FciSplitterGroup_P_AllocateInputResources(BAPE_FciSplitterGroupHandle handle)
{
    BERR_Code errCode;
    unsigned i, fciSp;

    /* make sure we have enough consecutive channel pairs */
    errCode = BAPE_P_AllocateFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eFciSplitter, handle->numChannelPairs, &fciSp);
    if ( errCode )
    {
        #if BDBG_DEBUG_BUILD
        {
        unsigned allocatedChPairs = 0;
        for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_GROUPS; i++ )
        {
            BDBG_ASSERT(NULL != handle->deviceHandle->fciSplitterGroups[i]);
            if ( handle->deviceHandle->fciSplitterGroups[i]->allocated &&
                 handle->deviceHandle->fciSplitterGroups[i]->started )
            {
                allocatedChPairs += handle->deviceHandle->fciSplitterGroups[i]->numChannelPairs;
            }
        }
        BDBG_ASSERT(allocatedChPairs <= BAPE_CHIP_MAX_FCI_SPLITTERS);
        BDBG_MODULE_ERR(bape_fcisp, ("Not enough free FCI Splitter ch pairs available. requested %lu, free %lu",
                                     (unsigned long)handle->numChannelPairs, (unsigned long)(BAPE_CHIP_MAX_FCI_SPLITTERS - allocatedChPairs) ));
        }
        #endif
        return BERR_TRACE(errCode);
    }

    /* Success. Now set up the linkage */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        handle->fciSpIds[i] = fciSp + i;
        BDBG_MODULE_MSG(bape_fcisp, ("  chPair[%lu] using fci splitter %lu", (unsigned long)i, (unsigned long)handle->fciSpIds[i]));
        BAPE_FciSplitter_P_SetGroup(handle->deviceHandle, fciSp + i, fciSp);    /* Set HW Grouping */
        BDBG_MODULE_MSG(bape_fcisp, ("  set fci splitter %lu (chPair[%lu]) to fci input %x", (unsigned long)handle->fciSpIds[i], (unsigned long)i, handle->settings.input.ids[i]));
        BAPE_FciSplitter_P_SetInputFci(handle->deviceHandle, handle->fciSpIds[i], handle->settings.input.ids[i]);
    }

    return BERR_SUCCESS;
}

static void BAPE_FciSplitterGroup_P_FreeInputResources(BAPE_FciSplitterGroupHandle handle)
{
    unsigned i;

    /* Clear FCI Splitter Grouping */
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        BAPE_FciSplitter_P_SetGroup(handle->deviceHandle, handle->fciSpIds[i], handle->fciSpIds[i]);
        BAPE_FciSplitter_P_SetInputFci(handle->deviceHandle, handle->fciSpIds[i], 0x3ff);
    }
    /* Release Resources */
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eFciSplitter, handle->numChannelPairs, handle->fciSpIds[0]);
    BKNI_Memset(handle->fciSpIds, 0xff, sizeof(handle->fciSpIds));
}

BERR_Code BAPE_FciSplitterGroup_P_Start(
    BAPE_FciSplitterGroupHandle handle
    )
{
    BERR_Code errCode;
    unsigned connected = 0;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    /* if we are already running, stop/restart */
    if ( handle->started )
    {
        BAPE_FciSplitterGroup_P_Stop(handle);
    }

    BDBG_MODULE_MSG(bape_fcisp, ("%s %p", BSTD_FUNCTION, (void*)handle));

    connected = BAPE_FciSplitterGroup_P_GetConnectedOutputGroupIndexes(handle);
    if ( !connected )
    {
        BDBG_MODULE_WRN(bape_fcisp, ("No Fci Splitter output groups connected"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

    /* Allocate HW resources for FCI Splitter input
       (output resources are already allocated during their create routines) */
    errCode = BAPE_FciSplitterGroup_P_AllocateInputResources(handle);
    if ( errCode != BERR_SUCCESS )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_FciSplitterGroup_P_StartOutputGroups(handle, connected);
    if ( errCode != BERR_SUCCESS )
    {
        return BERR_TRACE(errCode);
    }

    handle->started = true;

    return BERR_SUCCESS;
}

void BAPE_FciSplitterGroup_P_Stop(
    BAPE_FciSplitterGroupHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);

    if ( !handle->started )
    {
        return;
    }

    BDBG_MODULE_MSG(bape_fcisp, ("%s %p", BSTD_FUNCTION, (void*)handle));

    BAPE_FciSplitterGroup_P_StopOutputGroups(handle);

    BAPE_FciSplitterGroup_P_FreeInputResources(handle);

    handle->started = false;
}

/* FCI Output Group */

BERR_Code BAPE_FciSplitterOutputGroup_P_Create(
    BAPE_FciSplitterGroupHandle spHandle,
    BAPE_FciSplitterGroupCreateSettings *pSettings,
    BAPE_FciSplitterOutputGroupHandle *pHandle        /* [out] */
    )
{
    unsigned i, fciSpOutput;
    BERR_Code errCode;
    BAPE_FciSplitterOutputGroupHandle handle=NULL;
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != spHandle);
    BDBG_ASSERT(spHandle->allocated);
    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(pSettings->numChannelPairs <= BAPE_ChannelPair_eMax);
    deviceHandle = spHandle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    BDBG_MODULE_MSG(bape_fcisp, ("Create Fci Splitter Output Group, %lu ch pairs", (unsigned long)pSettings->numChannelPairs));
    /* Find an available group handle */
    for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->fciSplitterOutputGroups[i]);
        if ( !deviceHandle->fciSplitterOutputGroups[i]->allocated )
        {
            handle = deviceHandle->fciSplitterOutputGroups[i];
            break;
        }
    }

    /* If none found, return error */
    if ( NULL == handle )
    {
        BDBG_MODULE_ERR(bape_fcisp, ("No more FCI Splitters available"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* we found a free slot. now make sure we have enough consecutive channel pairs */
    errCode = BAPE_P_AllocateFmmResource(deviceHandle, BAPE_FmmResourceType_eFciSplitterOutput, pSettings->numChannelPairs, &fciSpOutput);
    if ( errCode )
    {
        #if BDBG_DEBUG_BUILD
        {
        unsigned allocatedChPairs = 0;
        for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_GROUPS; i++ )
        {
            BDBG_ASSERT(NULL != deviceHandle->fciSplitterOutputGroups[i]);
            if ( deviceHandle->fciSplitterOutputGroups[i]->allocated )
            {
                allocatedChPairs += deviceHandle->fciSplitterOutputGroups[i]->numChannelPairs;
            }
        }
        BDBG_ASSERT(allocatedChPairs <= BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS);
        BDBG_MODULE_ERR(bape_fcisp, ("Not enough free FCI Splitter Output ch pairs available. requested %lu, free %lu",
                                     (unsigned long)pSettings->numChannelPairs, (unsigned long)(BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS - allocatedChPairs) ));
        }
        #endif
        errCode = BERR_TRACE(errCode);
        goto err_alloc_fcispout;
    }

    /* Successfully allocated resources.  Initialize Group */
    handle->allocated = true;
    handle->started = false;
    handle->numChannelPairs = pSettings->numChannelPairs;
    handle->deviceHandle = deviceHandle;
    handle->spHandle = spHandle;
    handle->sampleRate = 0;
    BKNI_Memset(handle->fciSpOutIds, 0xff, sizeof(handle->fciSpOutIds));
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        handle->fciSpOutIds[i] = fciSpOutput + i;
        BDBG_MODULE_MSG(bape_fcisp, ("  chPair[%lu] using fci sp output %lu", (unsigned long)i, (unsigned long)handle->fciSpOutIds[i]));
    }
    *pHandle = handle;
    return BERR_SUCCESS;

    err_alloc_fcispout:
    return errCode;
}

void BAPE_FciSplitterOutputGroup_P_Destroy(
    BAPE_FciSplitterOutputGroupHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_OBJECT_ASSERT(handle->deviceHandle, BAPE_Device);

    BDBG_MODULE_MSG(bape_fcisp, ("Destroy Fci Splitter Output Group %p, %lu ch pairs", (void*)handle, (unsigned long)handle->numChannelPairs));

    BAPE_FciSplitterOutputGroup_P_Stop(handle);

    /* Release Resources */
    BAPE_P_FreeFmmResource(handle->deviceHandle, BAPE_FmmResourceType_eFciSplitterOutput, handle->numChannelPairs, handle->fciSpOutIds[0]);
    BKNI_Memset(handle->fciSpOutIds, 0xff, sizeof(handle->fciSpOutIds));
    handle->allocated = false;
}

void BAPE_FciSplitterOutputGroup_P_GetOutputFciIds(
    BAPE_FciSplitterOutputGroupHandle handle,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    )
{
    unsigned i;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    BDBG_ASSERT(NULL != pFciGroup);
    BAPE_FciIdGroup_Init(pFciGroup);
    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        pFciGroup->ids[i] = BAPE_FCI_BASE_FCISP | handle->fciSpOutIds[i];
    }
}

/* FCI Splitter Helper functions */

static BERR_Code BAPE_FciSplitterOutputGroup_P_Start(
    BAPE_FciSplitterOutputGroupHandle handle
    )
{
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( handle->started )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    if ( handle->numChannelPairs != handle->spHandle->numChannelPairs )
    {
        BDBG_MODULE_MSG(bape_fcisp, ("%s: output group and fci splitter must have the same number of channel pairs.", BSTD_FUNCTION));
        BDBG_MODULE_MSG(bape_fcisp, ("%s: output group numChPairs %lu, fci splitter numChPairs %lu", BSTD_FUNCTION, (unsigned long)handle->numChannelPairs, (unsigned long)handle->spHandle->numChannelPairs));
        return BERR_SUCCESS;
    }

    BDBG_MODULE_MSG(bape_fcisp, ("Stop Fci Splitter Output Group %p, %lu ch pairs", (void*)handle, (unsigned long)handle->numChannelPairs));

    BAPE_FciSplitter_P_SetOutputGroupInputs(deviceHandle, handle->fciSpOutIds[0], handle->spHandle->fciSpIds[0], handle->numChannelPairs);
    BAPE_FciSplitter_P_EnableOutputGroup(deviceHandle, handle->fciSpOutIds[0], handle->numChannelPairs, 1);

    handle->started = true;
    return BERR_SUCCESS;
}

static void BAPE_FciSplitterOutputGroup_P_Stop(
    BAPE_FciSplitterOutputGroupHandle handle
    )
{
    BAPE_Handle deviceHandle;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(handle->allocated);
    if ( !handle->started )
    {
        return;
    }
    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    BDBG_MODULE_MSG(bape_fcisp, ("Stop Fci Splitter Output Group %p, %lu ch pairs", (void*)handle, (unsigned long)handle->numChannelPairs));

    BAPE_FciSplitter_P_EnableOutputGroup(deviceHandle, handle->fciSpOutIds[0], handle->numChannelPairs, 0);

    handle->started = false;
}

static void BAPE_FciSplitter_P_SetInputFci(BAPE_Handle handle, uint32_t fciSp, uint32_t fci)
{
    #if defined BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_BASE
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(fciSp < BAPE_CHIP_MAX_FCI_SPLITTERS);
    BDBG_ASSERT(fci <= 0x3ff);

    /*BDBG_MODULE_MSG(bape_fcisp, ("Set FciSp[%u] fci to source data from %x", fciSp, fci));*/

    BAPE_Reg_P_UpdateField(handle,
                           BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_BASE+(fciSp*BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_ELEMENT_SIZE/8),
                           AUD_MISC_FCI_SPLTR0_CTRLi, FCI_ID, fci);
    #else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(fciSp);
    BSTD_UNUSED(fci);
    #endif
}

static void BAPE_FciSplitter_P_SetGroup(BAPE_Handle handle, uint32_t fciSp, uint32_t groupId)
{
    #if defined BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_BASE
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(fciSp < BAPE_CHIP_MAX_FCI_SPLITTERS);
    BDBG_ASSERT(groupId < BAPE_CHIP_MAX_FCI_SPLITTERS);

    /*BDBG_MODULE_MSG(bape_fcisp, ("Set FciSp[%u] group to %x", fciSp, groupId));*/

    BAPE_Reg_P_UpdateField(handle,
                           BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_BASE+(fciSp*BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_ELEMENT_SIZE/8),
                           AUD_MISC_FCI_SPLTR0_CTRLi, GROUP_ID, groupId);

    #else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(fciSp);
    BSTD_UNUSED(groupId);
    #endif
}

void BAPE_FciSplitter_P_InitHw(BAPE_Handle handle)
{
    unsigned i;

    BAPE_FciSplitter_P_SetOutputGroupInputs(handle, 0, 0, 0xffffffff);

    for ( i = 0; i < BAPE_CHIP_MAX_FCI_SPLITTERS; i++ )
    {
        BAPE_FciSplitter_P_SetInputFci(handle, i, 0x3ff);
        BAPE_FciSplitter_P_SetGroup(handle, i, i);
    }
}

static void BAPE_FciSplitter_P_SetOutputGroupInputs(BAPE_Handle handle, uint32_t fciSpOutput, uint32_t fciSpInput, uint32_t count)
{
    #if defined BCHP_AUD_MISC_FCI_SPLTR0_OUT_CFG
    BAPE_Reg_P_FieldList fieldList;
    unsigned i;
    bool init = false;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(fciSpOutput < BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS);
    if ( count == 0xffffffff )
    {
        count = BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS;
        init = true;
    }
    else
    {
        BDBG_ASSERT(count <= BAPE_CHIP_MAX_FCI_SPLITTERS);
    }

    BAPE_Reg_P_InitFieldList(handle, &fieldList);
    for ( i = 0; i < count; i++ )
    {
        uint32_t input = init ? 0 : (fciSpInput+i);
        /*BDBG_MODULE_MSG(bape_fcisp, ("Set FciSpOutput[%u] to source FciSp %x", fciSpOutput + i, input));*/
        switch ( (fciSpOutput + i) )
        {
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 0
        case 0:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_0, input);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 1
        case 1:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_1, input);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 2
        case 2:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_2, input);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 3
        case 3:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_3, input);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 4
        case 4:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_4, input);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 5
        case 5:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_5, input);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 6
        case 6:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_6, input);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 7
        case 7:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_7, input);
            break;
        #endif
        default:
            BDBG_MODULE_ERR(bape_fcisp, ("Invalid Fci Splitter output pair %lu", (unsigned long)(fciSpOutput + i)));
            BERR_TRACE(BERR_INVALID_PARAMETER);

        }
    }
    BAPE_Reg_P_ApplyFieldList(&fieldList, BCHP_AUD_MISC_FCI_SPLTR0_OUT_CFG);
    #else
    BSTB_UNUSED(handle);
    BSTB_UNUSED(fciSpOutput);
    BSTB_UNUSED(fciSpInput);
    BSTB_UNUSED(count);
    #endif
}

static void BAPE_FciSplitter_P_EnableOutputGroup(BAPE_Handle handle, uint32_t fciSpOutput, unsigned count, unsigned enable)
{
    #if defined BCHP_AUD_MISC_FCI_SPLTR0_OUT_ENA
    BAPE_Reg_P_FieldList fieldList;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT((fciSpOutput+count) <= BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS);
    BDBG_ASSERT(count <= BAPE_CHIP_MAX_FCI_SPLITTERS);

    BAPE_Reg_P_InitFieldList(handle, &fieldList);
    for ( i = 0; i < count; i++ )
    {
        /*BDBG_MODULE_MSG(bape_fcisp, ("Set FciSpOutput[%u] enable %x", fciSpOutput + i, enable));*/
        switch ( (fciSpOutput + i) )
        {
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 0
        case 0:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_0, enable);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 1
        case 1:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_1, enable);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 2
        case 2:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_2, enable);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 3
        case 3:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_3, enable);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 4
        case 4:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_4, enable);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 5
        case 5:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_5, enable);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 6
        case 6:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_6, enable);
            break;
        #endif
        #if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 7
        case 7:
            BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_7, enable);
            break;
        #endif
        default:
            BDBG_MODULE_ERR(bape_fcisp, ("Invalid Fci Splitter output pair %lu", (unsigned long)(fciSpOutput + i)));
            BERR_TRACE(BERR_INVALID_PARAMETER);

        }
    }
    BAPE_Reg_P_ApplyFieldList(&fieldList, BCHP_AUD_MISC_FCI_SPLTR0_OUT_ENA);
    #else
    BSTB_UNUSED(handle);
    BSTB_UNUSED(fciSpOutput);
    BSTB_UNUSED(count);
    BSTB_UNUSED(enable);
    #endif
}

static unsigned BAPE_FciSplitterGroup_P_GetConnectedOutputGroupIndexes(BAPE_FciSplitterGroupHandle handle)
{
    unsigned i;
    unsigned connected = 0;
    BAPE_Handle deviceHandle;

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->fciSplitterOutputGroups[i]);
        if ( deviceHandle->fciSplitterOutputGroups[i]->spHandle == handle &&
             deviceHandle->fciSplitterOutputGroups[i]->allocated )
        {
            connected |= (1 << i);
        }
    }

    return connected;
}

static unsigned BAPE_FciSplitterGroup_P_GetRunningOutputGroupIndexes(BAPE_FciSplitterGroupHandle handle)
{
    unsigned i;
    unsigned running = 0;
    BAPE_Handle deviceHandle;

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS; i++ )
    {
        BDBG_ASSERT(NULL != deviceHandle->fciSplitterOutputGroups[i]);
        if ( deviceHandle->fciSplitterOutputGroups[i]->spHandle == handle &&
             deviceHandle->fciSplitterOutputGroups[i]->allocated &&
             deviceHandle->fciSplitterOutputGroups[i]->started )
        {
            running |= (1 << i);
        }
    }

    return running;
}

static unsigned BAPE_FciSplitterGroup_P_StopOutputGroups(BAPE_FciSplitterGroupHandle handle)
{
    unsigned i;
    unsigned running = 0;
    BAPE_Handle deviceHandle;

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    /* look up outputs connected to this Fci Splitter Group */
    running = BAPE_FciSplitterGroup_P_GetRunningOutputGroupIndexes(handle);

    if ( running != 0 )
    {
        BDBG_MODULE_MSG(bape_fcisp, ("Stop Fci Splitter output groups, mask %lu", (unsigned long)running));
        for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS; i++ )
        {
            BDBG_ASSERT(NULL != deviceHandle->fciSplitterOutputGroups[i]);
            if ( ((running >> i) & 1) )
            {
                BAPE_FciSplitterOutputGroup_P_Stop(deviceHandle->fciSplitterOutputGroups[i]);
            }
        }
    }

    return running;
}

static BERR_Code BAPE_FciSplitterGroup_P_StartOutputGroups(BAPE_FciSplitterGroupHandle handle, unsigned groups)
{
    unsigned i;
    BAPE_Handle deviceHandle;
    BERR_Code errCode;

    deviceHandle = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    if ( groups != 0 )
    {
        BDBG_MODULE_MSG(bape_fcisp, ("Start FCI Splitter output groups, mask %lu", (unsigned long)groups));
        for ( i = 0; i < BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS; i++ )
        {
            BDBG_ASSERT(NULL != deviceHandle->fciSplitterOutputGroups[i]);
            if ( ((groups >> i) & 1) )
            {
                errCode = BAPE_FciSplitterOutputGroup_P_Start(deviceHandle->fciSplitterOutputGroups[i]);
                if ( errCode != BERR_SUCCESS )
                {
                    BDBG_MODULE_ERR(bape_fcisp, ("Unable to start FCI Splitter output group %lu", (unsigned long)i));
                    BAPE_FciSplitterGroup_P_StopOutputGroups(handle);
                    return BERR_TRACE(BERR_UNKNOWN);
                }
            }
        }
    }

    return BERR_SUCCESS;
}

#else /* BAPE_CHIP_MAX_FCISPLITTER_GROUPS > 0 */

/* FCI Splitter Stubs */
typedef struct BAPE_FciSplitterGroup
{
    unsigned unused;
} BAPE_FciSplitterGroup;

typedef struct BAPE_FciSplitterOutputGroup
{
    unsigned unused;
} BAPE_FciSplitterOutputGroup;

void BAPE_FciSplitterGroup_P_GetDefaultCreateSettings(
    BAPE_FciSplitterGroupCreateSettings *pSettings    /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_FciSplitterGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_FciSplitterGroupCreateSettings *pSettings,
    BAPE_FciSplitterGroupHandle *pHandle  /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pHandle);
    BSTD_UNUSED(pSettings);
    return BERR_NOT_SUPPORTED;
}

void BAPE_FciSplitterGroup_P_Destroy(
    BAPE_FciSplitterGroupHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_FciSplitterGroup_P_GetSettings(
    BAPE_FciSplitterGroupHandle handle,
    BAPE_FciSplitterGroupSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_FciSplitterGroup_P_SetSettings(
    BAPE_FciSplitterGroupHandle handle,
    const BAPE_FciSplitterGroupSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_NOT_SUPPORTED;
}

BERR_Code BAPE_FciSplitterGroup_P_Start(
    BAPE_FciSplitterGroupHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_NOT_SUPPORTED;
}

void BAPE_FciSplitterGroup_P_Stop(
    BAPE_FciSplitterGroupHandle handle
    )
{
    BSTD_UNUSED(handle);
}

BERR_Code BAPE_FciSplitterOutputGroup_P_Create(
    BAPE_FciSplitterGroupHandle spHandle,
    BAPE_FciSplitterGroupCreateSettings *pSettings,
    BAPE_FciSplitterOutputGroupHandle *pHandle        /* [out] */
    )
{
    BSTD_UNUSED(spHandle);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_NOT_SUPPORTED;
}

void BAPE_FciSplitterOutputGroup_P_Destroy(
    BAPE_FciSplitterOutputGroupHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_FciSplitterOutputGroup_P_GetOutputFciIds(
    BAPE_FciSplitterOutputGroupHandle handle,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pFciGroup);
}

void BAPE_FciSplitter_P_InitHw(BAPE_Handle handle)
{
    BSTD_UNUSED(handle);
}
#endif /* BAPE_CHIP_MAX_FCISPLITTER_GROUPS > 0 */

BERR_Code BAPE_P_InitBfSw(
    BAPE_Handle handle
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    BDBG_MSG(("Allocating %u SFIFO Groups", BAPE_CHIP_MAX_SFIFO_GROUPS));
    handle->sfifoGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_SFIFO_GROUPS*sizeof(BAPE_SfifoGroup));
    if ( NULL == handle->sfifoGroups[0] )
    {
        BAPE_P_UninitBfSw(handle);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle->sfifoGroups[0], 0, BAPE_CHIP_MAX_SFIFO_GROUPS*sizeof(BAPE_SfifoGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_SFIFO_GROUPS; i++ )
    {
        handle->sfifoGroups[i] = handle->sfifoGroups[0] + i;
    }
    BDBG_MSG(("Allocating %u DFIFO Groups", BAPE_CHIP_MAX_DFIFO_GROUPS));
    handle->dfifoGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_DFIFO_GROUPS*sizeof(BAPE_DfifoGroup));
    if ( NULL == handle->dfifoGroups[0] )
    {
        BAPE_P_UninitBfSw(handle);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle->dfifoGroups[0], 0, BAPE_CHIP_MAX_DFIFO_GROUPS*sizeof(BAPE_DfifoGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_DFIFO_GROUPS; i++ )
    {
        handle->dfifoGroups[i] = handle->dfifoGroups[0] + i;
    }
    BDBG_MSG(("Allocating %u FCI Splitter Groups", BAPE_CHIP_MAX_FCISPLITTER_GROUPS));
    #if BAPE_CHIP_MAX_FCISPLITTER_GROUPS > 0
    handle->fciSplitterGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_FCISPLITTER_GROUPS*sizeof(BAPE_FciSplitterGroup));
    if ( NULL == handle->fciSplitterGroups[0] )
    {
        BAPE_P_UninitBfSw(handle);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle->fciSplitterGroups[0], 0, BAPE_CHIP_MAX_FCISPLITTER_GROUPS*sizeof(BAPE_FciSplitterGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_FCISPLITTER_GROUPS; i++ )
    {
        handle->fciSplitterGroups[i] = handle->fciSplitterGroups[0] + i;
    }
    #endif
    #if BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS > 0
    BDBG_MSG(("Allocating %u FCI Splitter Output Groups", BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS));
    handle->fciSplitterOutputGroups[0] = BKNI_Malloc(BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS*sizeof(BAPE_FciSplitterOutputGroup));
    if ( NULL == handle->fciSplitterOutputGroups[0] )
    {
        BAPE_P_UninitBfSw(handle);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle->fciSplitterOutputGroups[0], 0, BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS*sizeof(BAPE_FciSplitterOutputGroup));
    for ( i = 1; i < BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS; i++ )
    {
        handle->fciSplitterOutputGroups[i] = handle->fciSplitterOutputGroups[0] + i;
    }
    #endif
    return BERR_SUCCESS;
}

BERR_Code BAPE_P_InitBfHw(
    BAPE_Handle handle
    )
{
    uint32_t regAddr, endAddr, regVal;
    BREG_Handle regHandle;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    regHandle = handle->regHandle;

    BDBG_MSG(("Initializing BF registers"));

    /* Support for >2GB memory addressing */
    #ifdef BCHP_AUD_FMM_BF_CTRL_MISC_CONFIG_SCB0_BASE_START_MASK
    {
        BAPE_Reg_P_FieldList regFieldList;
        uint32_t chpMemsize;

        BAPE_Reg_P_InitFieldList(handle, &regFieldList);
        chpMemsize = handle->settings.memc[1].baseAddress >> 28;
        chpMemsize = chpMemsize ? chpMemsize - 1 : 0xF;
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_MISC_CONFIG, SCB0_BASE_START, 0x0);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_MISC_CONFIG, SCB0_BASE_END, chpMemsize);
        #ifdef BCHP_AUD_FMM_BF_CTRL_MISC_CONFIG_SCB1_BASE_END_MASK
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_MISC_CONFIG, SCB1_BASE_START, 0xF);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_MISC_CONFIG, SCB1_BASE_END, 0x0);
        #endif
        #ifdef BCHP_AUD_FMM_BF_CTRL_MISC_CONFIG_SCB2_BASE_END_MASK
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_MISC_CONFIG, SCB2_BASE_START, 0xF);
        BAPE_Reg_P_AddToFieldList(&regFieldList, AUD_FMM_BF_CTRL_MISC_CONFIG, SCB2_BASE_END, 0x0);
        #endif
        BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_FMM_BF_CTRL_MISC_CONFIG);
    }
    #endif

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    /* Clear Ringbuffer registers */
    regAddr = BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR;
    endAddr = regAddr + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR) * (BAPE_CHIP_MAX_SFIFOS + BAPE_CHIP_MAX_DFIFOS));
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FRMSTADDR
    endAddr += ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_FRMSTADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FRMSTADDR) * (BAPE_CHIP_MAX_SFIFOS + BAPE_CHIP_MAX_DFIFOS));
#endif
    BDBG_MSG(("Clearing ringbuffer registers from 0x%x to 0x%x", regAddr, endAddr-4));
    while ( regAddr < endAddr )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += 4;
    }
#else
    /* Clear Ringbuffer registers */
    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR;
    endAddr = regAddr + (BAPE_P_RINGBUFFER_STRIDE * 2 * (BAPE_CHIP_MAX_SFIFOS + BAPE_CHIP_MAX_DFIFOS));
    BDBG_MSG(("Clearing ringbuffer registers from 0x%x to 0x%x", regAddr, endAddr-4));
    while ( regAddr < endAddr )
    {
        BREG_WriteAddr(regHandle, regAddr, 0);
        regAddr += BAPE_BF_ADDRESS_STRIDE;
    }
#ifdef BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_FRMSTADDR
    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_FRMSTADDR;
    endAddr = regAddr + ((BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_1_FRMSTADDR-BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_FRMSTADDR)*2*(BAPE_CHIP_MAX_SFIFOS + BAPE_CHIP_MAX_DFIFOS));
    while ( regAddr < endAddr )
    {
        BREG_WriteAddr(regHandle, regAddr, 0);
        regAddr += (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_MI_VALID-BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_FRMSTADDR);
    }
#endif
#endif
    BDBG_MSG(("Resetting source channel group IDs to default"));
    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE;
    for ( i = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_START; i <= BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_END; i++ )
    {
        regVal = BREG_Read32(regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID, i);
        BREG_Write32(regHandle, regAddr, regVal);
        regAddr += (BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_ELEMENT_SIZE/8);
    }

    BAPE_FciSplitter_P_InitHw(handle);

    return BERR_SUCCESS;
}


void BAPE_P_UninitBfSw(
    BAPE_Handle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    if ( NULL != handle->sfifoGroups[0] )
    {
        BKNI_Free(handle->sfifoGroups[0]);
    }
    BKNI_Memset(handle->sfifoGroups, 0, sizeof(handle->sfifoGroups));
    if ( NULL != handle->dfifoGroups[0] )
    {
        BKNI_Free(handle->dfifoGroups[0]);
    }
    BKNI_Memset(handle->dfifoGroups, 0, sizeof(handle->dfifoGroups));
    #if BAPE_CHIP_MAX_FCISPLITTER_GROUPS > 0
    if ( NULL != handle->fciSplitterGroups[0] )
    {
        BKNI_Free(handle->fciSplitterGroups[0]);
    }
    BKNI_Memset(handle->fciSplitterGroups, 0, sizeof(handle->fciSplitterGroups));
    #endif
    #if BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS > 0
    if ( NULL != handle->fciSplitterOutputGroups[0] )
    {
        BKNI_Free(handle->fciSplitterOutputGroups[0]);
    }
    BKNI_Memset(handle->fciSplitterOutputGroups, 0, sizeof(handle->fciSplitterOutputGroups));
    #endif
}

static bool BAPE_Sfifo_P_IsEnabled(BAPE_Handle hApe, unsigned sfifoId)
{
    uint32_t regAddr;
    bool enabled;

    regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_BF_CTRL_SOURCECH_CFGi, sfifoId);
    enabled = BAPE_Reg_P_ReadField(hApe, regAddr, AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE) ? true : false;
    if ( enabled )
    {
        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_BF_CTRL_SOURCECH_CTRLi, sfifoId);
        enabled = BAPE_Reg_P_ReadField(hApe, regAddr, AUD_FMM_BF_CTRL_SOURCECH_CTRLi, PLAY_RUN) ? true : false;
    }

    return enabled;
}

static void BAPE_Dfifo_P_GetSfifoStatus(BAPE_Handle hApe, unsigned dfifoId, unsigned *pSfifoId, bool *pBound, bool *pEnabled)
{
    uint32_t regVal, regAddr;

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);
    BDBG_ASSERT(dfifoId < BAPE_CHIP_MAX_DFIFOS);
    BDBG_ASSERT(NULL != pSfifoId);
    BDBG_ASSERT(NULL != pEnabled);
    BDBG_ASSERT(NULL != pBound);

    regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_BF_CTRL_DESTCH_CFGi, dfifoId);
    regVal = BAPE_Reg_P_Read(hApe, regAddr);

    *pSfifoId = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_BF_CTRL_DESTCH_CFGi, SOURCE_FIFO_ID);
    *pBound = ((BCHP_GET_FIELD_DATA(regVal, AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO))||
               (BCHP_GET_FIELD_DATA(regVal, AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE))) ? true : false;
    *pEnabled = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE) ? true : false;
}

static void BAPE_Dfifo_P_SetSfifo(BAPE_Handle hApe, unsigned dfifoId, unsigned sfifoId)
{
    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);
    BDBG_ASSERT(dfifoId < BAPE_CHIP_MAX_DFIFOS);
    /* sfifoId = 0x1f is a valid setting, don't validate it */
    BAPE_Reg_P_UpdateField(hApe, BAPE_Reg_P_GetArrayAddress(AUD_FMM_BF_CTRL_DESTCH_CFGi, dfifoId), AUD_FMM_BF_CTRL_DESTCH_CFGi, SOURCE_FIFO_ID, sfifoId);
}

static void BAPE_Bf_P_SfifoStarting(BAPE_SfifoGroupHandle handle)
{
    unsigned dfifoId, sfifoId;
    bool dfifoEnabled, dfifoLinked;

    BDBG_ASSERT(NULL != handle);

    BDBG_MSG(("SFIFO Starting (Group Base %u, %u pairs)", handle->sfifoIds[0], handle->numChannelPairs));

    /* SW7425-3699: DFIFO[0] must always be programmed to a running SFIFO to
       avoid a data fetch error in the adaptive rate controllers */
    dfifoId = 0;
        BAPE_Dfifo_P_GetSfifoStatus(handle->deviceHandle, dfifoId, &sfifoId, &dfifoLinked, &dfifoEnabled);
        if ( sfifoId == 0x1f && !(dfifoLinked && dfifoEnabled) )
        {
            /* If a DFIFO is linked to a SFIFO group intentionally and enabled, don't mess with it */
            BDBG_MSG(("Reprogramming DFIFO %u for SFIFO %u", dfifoId, sfifoId));
            BAPE_Dfifo_P_SetSfifo(handle->deviceHandle, dfifoId, handle->sfifoIds[0]);
        }
        else if ( sfifoId != 0x1f )
        {
            BDBG_MSG(("DFIFO %u is already using running SFIFO %u", dfifoId, sfifoId));
        }
        else
        {
            BDBG_MSG(("DFIFO %u is linked to SFIFO %u for CAPTURE_TO_SOURCEFIFO", dfifoId, sfifoId));
        }
    }

static void BAPE_Bf_P_SfifoStopping(BAPE_SfifoGroupHandle handle)
{
    unsigned sfifoId, newSfifoId, dfifoId;
    bool dfifoLinked, dfifoEnabled;

    BDBG_MSG(("SFIFO Stopping (Group Base %u, %u pairs)", handle->sfifoIds[0], handle->numChannelPairs));

    newSfifoId = BAPE_Bf_P_GetFirstRunningSfifo(handle->deviceHandle, handle->sfifoIds);

    dfifoId = 0;
        BAPE_Dfifo_P_GetSfifoStatus(handle->deviceHandle, dfifoId, &sfifoId, &dfifoLinked, &dfifoEnabled);
        if ( !(dfifoLinked && dfifoEnabled) )
        {
            /* If a DFIFO is linked to a SFIFO group intentionally and enabled, don't mess with it */
            BDBG_MSG(("Reprogramming DFIFO %u for SFIFO %u", dfifoId, newSfifoId));
            BAPE_Dfifo_P_SetSfifo(handle->deviceHandle, dfifoId, newSfifoId);
        }
        else
        {
            BDBG_MSG(("DFIFO %u is linked to SFIFO %u for CAPTURE_TO_SOURCEFIFO", dfifoId, sfifoId));
        }
    }

static unsigned BAPE_Bf_P_GetFirstRunningSfifo(BAPE_Handle hApe, const uint32_t *pSfifoIds)
{
    unsigned sfifoId, i;
    const unsigned invalidSfifoIds[BAPE_ChannelPair_eMax] = {0x1f, 0x1f, 0x1f, 0x1f};

    if ( pSfifoIds == NULL )
    {
        pSfifoIds = &invalidSfifoIds[0];
    }

    for ( sfifoId = 0; sfifoId < BAPE_CHIP_MAX_SFIFOS; sfifoId++ )
    {
        if ( BAPE_Sfifo_P_IsEnabled(hApe, sfifoId) )
        {
            bool ignore = false;
            for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
            {
                if ( sfifoId == pSfifoIds[i] )
                {
                    ignore = true;
                    break;
                }
            }
            for ( i = 0; i < BAPE_CHIP_MAX_DFIFOS; i++ )
            {
                uint32_t regAddr, regVal;

                regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_BF_CTRL_DESTCH_CFGi, i);
                regVal = BAPE_Reg_P_Read(hApe, regAddr);
                if ( (regVal & BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE)) ||
                     (regVal & BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO)) )
                {
                    /* This SFIFO is being used as part of a capture to sfifo pr play from capture operation.  Don't use it.  */
                    ignore = true;
                    break;
                }
            }
            if ( !ignore )
            {
                break;
            }
        }
    }

    if ( sfifoId >= BAPE_CHIP_MAX_SFIFOS )
    {
        BDBG_MSG(("No Running Sfifo, DFIFOs will use 0x1f"));
        return 0x1f;
    }
    else
    {
        BDBG_MSG(("First Running Sfifo is %u, DFIFOs will use 0x%x", sfifoId, sfifoId));
        return sfifoId;
    }
}
