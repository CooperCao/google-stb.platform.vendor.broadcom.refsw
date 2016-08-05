/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bape.h"
#include "bape_priv.h"

#ifdef BCHP_AIO_INTH_REG_START
    #include "bchp_int_id_aio_inth.h"
#endif

#ifdef BCHP_AUD_INTH_REG_START
    #include "bchp_int_id_aud_inth.h"
#endif

BDBG_MODULE(bape_mai_input);
BDBG_FILE_MODULE(bape_mai_input_detail);

BDBG_OBJECT_ID(BAPE_MaiInput);

#if BAPE_CHIP_MAX_MAI_INPUTS > 0

typedef struct BAPE_MaiInput
{
    BDBG_OBJECT(BAPE_MaiInput)
    BAPE_Handle deviceHandle;
    BAPE_MaiInputSettings settings;
    unsigned index;
    BAPE_InputPortObject inputPort;
    uint32_t offset;
    BAPE_MaiInputFormatDetectionSettings clientFormatDetectionSettings;
    bool localFormatDetectionEnabled;
    bool formatDetectionEnabled;
    bool hwAutoMuteEnabled;
    uint32_t outFormatEna;      /* last value written to OUT_FORMAT_ENA field. */
    struct
    {
        BAPE_MclkSource mclkSource;
        unsigned pllChannel;    /* only applies if mclkSource refers to a PLL */
        unsigned mclkFreqToFsRatio;
    } mclkInfo;
    bool enable;
    char name[12];   /* MAI IN %d */

    BINT_CallbackHandle maiRxCallback;

    BAPE_MaiInputFormatDetectionStatus  lastFormatDetectionStatus;
} BAPE_MaiInput;


#if BCHP_CHIP == 7425 || BCHP_CHIP == 7422 || BCHP_CHIP == 7435
#define BAPE_MAI_INPUT_CAPTURE_ID(chPair) (5+(chPair))
#elif BCHP_CHIP == 35230 || BCHP_CHIP == 35233 || BCHP_CHIP == 35125 || BCHP_CHIP == 35126
#define BAPE_MAI_INPUT_CAPTURE_ID(chPair) (6+(chPair))
#elif defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
/* 7429-style chips do not need this hardcoded */
#else
#error MAI Input Capture ID not defined
#endif

/* Build some abstract register names and map them to the chip's real names. */
#if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
    #define BAPE_P_HDMI_RX_MAI_FORMAT_REGADDR    BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT
    #define BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME         AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT
    #define BAPE_P_HDMI_RX_STATUS_REGADDR        BCHP_AUD_FMM_IOP_IN_HDMI_0_STATUS
    #define BAPE_P_HDMI_RX_STATUS_REGNAME             AUD_FMM_IOP_IN_HDMI_0_STATUS
    #define BAPE_P_HDMI_RX_CONFIG_REGADDR        BCHP_AUD_FMM_IOP_IN_HDMI_0_CONFIG
    #define BAPE_P_HDMI_RX_CONFIG_REGNAME             AUD_FMM_IOP_IN_HDMI_0_CONFIG
#endif
#if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
    #define BAPE_P_HDMI_RX_MAI_FORMAT_REGADDR   BCHP_HDMI_RCVR_CTRL_MAI_FORMAT /* Use HDMI register */
    #define BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME       SPDIF_RCVR_CTRL_MAI_FORMAT /* but use SPDIF field and enum definitions */
    #define BAPE_P_HDMI_RX_STATUS_REGADDR       BCHP_HDMI_RCVR_CTRL_STATUS
    #define BAPE_P_HDMI_RX_STATUS_REGNAME           SPDIF_RCVR_CTRL_STATUS
    #define BAPE_P_HDMI_RX_CONFIG_REGADDR       BCHP_HDMI_RCVR_CTRL_CONFIG
    #define BAPE_P_HDMI_RX_CONFIG_REGNAME           SPDIF_RCVR_CTRL_CONFIG
#endif

/* Static function prototypes */
static BERR_Code    BAPE_MaiInput_P_OpenHw(BAPE_MaiInputHandle handle);
static BERR_Code    BAPE_MaiInput_P_ApplySettings(BAPE_MaiInputHandle handle);
static void         BAPE_MaiInput_P_DetectInputChange_isr (BAPE_MaiInputHandle handle);
static BERR_Code    BAPE_MaiInput_P_GetFormatDetectionStatus_isr(BAPE_MaiInputHandle handle, BAPE_MaiInputFormatDetectionStatus *pStatus );
static void         BAPE_P_MAI_RX_isr (void * pParm1, int iParm2);
static void         BAPE_MaiInput_P_SetReceiverOutputFormat_isr (BAPE_MaiInputHandle handle, BAPE_MaiInputFormatDetectionStatus *pFormatDetectionStatus);
static BERR_Code    BAPE_MaiInput_P_SetFormatDetection_isr(BAPE_MaiInputHandle handle);
static void         BAPE_MaiInput_P_UpdateFormat_isr (BAPE_MaiInputHandle handle, BAPE_MaiInputFormatDetectionStatus *pFormatDetectionStatus, BAPE_FMT_Descriptor *pFormat);

/* Input port callbacks */
static void         BAPE_MaiInput_P_Enable(BAPE_InputPort inputPort);
static void         BAPE_MaiInput_P_Disable(BAPE_InputPort inputPort);
static BERR_Code    BAPE_MaiInput_P_ConsumerAttached_isr(BAPE_InputPort inputPort, BAPE_PathNode *pConsumer, BAPE_FMT_Descriptor *pFormat);
static void         BAPE_MaiInput_P_ConsumerDetached_isr(BAPE_InputPort inputPort, BAPE_PathNode *pConsumer);

/***************************************************************************
        Public APIs: From bape_input.h
***************************************************************************/
void BAPE_MaiInput_GetDefaultSettings(
    BAPE_MaiInputSettings *pSettings        /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->errorInsertion = BAPE_SpdifInputErrorInsertion_eNone;
    pSettings->ignoreCompressedParity = true;
    pSettings->ignorePcmParity = true;
    pSettings->ignoreValidity = true;
    pSettings->stcIndex = 0;
    #if defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR
    #if 0
    pSettings->hardwareMutingEnabled = true;
    #endif
    #endif

}

/**************************************************************************/

BERR_Code BAPE_MaiInput_Open(
    BAPE_Handle hApe,
    unsigned index,
    const BAPE_MaiInputSettings *pSettings,
    BAPE_MaiInputHandle *phandle
    )
{
    BERR_Code errCode;
    BAPE_FMT_Descriptor format;
    BAPE_MaiInputHandle handle;
    BAPE_MaiInputSettings defaultSettings;

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);
    BDBG_ASSERT(NULL != phandle);

    BDBG_MSG(("%s: Opening MAI Input: %u", __FUNCTION__, index));

    *phandle = NULL;

    if ( index >= BAPE_CHIP_MAX_MAI_INPUTS )
    {
        BDBG_ERR(("Request to open MAI %d but chip only has %u MAI inputs", index, BAPE_CHIP_MAX_MAI_INPUTS ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( hApe->maiInputs[index] )
    {
        BDBG_ERR(("MAI input %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Allocate the device structure, then fill in all the fields. */
    handle = BKNI_Malloc(sizeof(BAPE_MaiInput));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_MaiInput));
    BDBG_OBJECT_SET(handle, BAPE_MaiInput);
    BAPE_P_InitInputPort(&handle->inputPort, BAPE_InputPortType_eMai, index, handle);
    handle->maiRxCallback = NULL;
    handle->deviceHandle = hApe;
    handle->index = index;
    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
    {
        uint32_t regVal;
        #if BAPE_CHIP_MAX_FCI_SPLITTERS
        BAPE_FciSplitterGroupCreateSettings fciSpCreateSettings;
        BAPE_FciSplitterGroup_P_GetDefaultCreateSettings(&fciSpCreateSettings);
        errCode = BAPE_FciSplitterGroup_P_Create(handle->deviceHandle, &fciSpCreateSettings, &handle->inputPort.fciSpGroup);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Could not allocate FCI Splitter"));
            BAPE_MaiInput_Close(handle);
            return BERR_TRACE(errCode);
        }
        #endif

        #if 0
        /* set up the input FCIs and group */
        for ( i = 0; i < 4; i++ )
        {
            BAPE_Reg_P_UpdateField(handle->deviceHandle, BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_BASE+(i*BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_ELEMENT_SIZE), AUD_MISC_FCI_SPLTR0_CTRLi, FCI_ID, BAPE_FCI_BASE_INPUT | (regVal+i));
            BAPE_Reg_P_UpdateField(handle->deviceHandle, BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_BASE+(i*BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_ELEMENT_SIZE), AUD_MISC_FCI_SPLTR0_CTRLi, GROUP_ID, i);
        }

        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &fieldList);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_0, 0);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_1, 1);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_2, 2);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_CFG, OUT_SEL_3, 3);
        BAPE_Reg_P_ApplyFieldList(&fieldList, BCHP_AUD_MISC_FCI_SPLTR0_OUT_CFG);

        #if 0
        BAPE_Reg_P_InitFieldList(handle->deviceHandle, &fieldList);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_0, 1);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_1, 1);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_2, 1);
        BAPE_Reg_P_AddToFieldList(&fieldList, AUD_MISC_FCI_SPLTR0_OUT_ENA, OUT_ENA_3, 1);
        BAPE_Reg_P_ApplyFieldList(&fieldList, BCHP_AUD_MISC_FCI_SPLTR0_OUT_ENA);
        #endif

        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRight] = 0;
        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRightSurround] = 1;
        handle->inputPort.streamId[BAPE_ChannelPair_eCenterLfe] = 2;
        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRightRear] = 3;
        #else
        regVal = BAPE_Reg_P_Read(hApe, BCHP_AUD_FMM_IOP_IN_HDMI_0_CAPTURE_FCI_ID_TABLE);
        regVal = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_IOP_IN_HDMI_0_CAPTURE_FCI_ID_TABLE, START_FCI_ID);

        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRight] = regVal;
        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRightSurround] = regVal+1;
        handle->inputPort.streamId[BAPE_ChannelPair_eCenterLfe] = regVal+2;
        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRightRear] = regVal+3;
        #endif
    }
    #endif
    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY

        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRight] = BAPE_MAI_INPUT_CAPTURE_ID(BAPE_ChannelPair_eLeftRight);
        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRightSurround] = BAPE_MAI_INPUT_CAPTURE_ID(BAPE_ChannelPair_eLeftRightSurround);
        handle->inputPort.streamId[BAPE_ChannelPair_eCenterLfe] = BAPE_MAI_INPUT_CAPTURE_ID(BAPE_ChannelPair_eCenterLfe);
        handle->inputPort.streamId[BAPE_ChannelPair_eLeftRightRear] = BAPE_MAI_INPUT_CAPTURE_ID(BAPE_ChannelPair_eLeftRightRear);
    #endif
    handle->inputPort.enable = BAPE_MaiInput_P_Enable;
    handle->inputPort.disable = BAPE_MaiInput_P_Disable;
    handle->inputPort.consumerAttached_isr = BAPE_MaiInput_P_ConsumerAttached_isr;
    handle->inputPort.consumerDetached_isr = BAPE_MaiInput_P_ConsumerDetached_isr;

    {   /* Start Critical Section */
        BKNI_EnterCriticalSection();
        BAPE_InputPort_P_GetFormat_isr(&handle->inputPort, &format);
        format.source = BAPE_DataSource_eFci;
        format.type = BAPE_DataType_ePcmStereo;
        format.sampleRate = 48000;
        (void)BAPE_InputPort_P_SetFormat_isr(&handle->inputPort, &format);
        BKNI_LeaveCriticalSection();
    }   /* End Critical Section */

    BKNI_Snprintf(handle->name, sizeof(handle->name), "MAI IN %u", index);
    handle->inputPort.pName = handle->name;

    /* Currently we only support one MAI input, this can be expanded later if we support more */
    #if BAPE_CHIP_MAX_MAI_INPUTS  > 1
        #error "Need to support more MAI inputs"
    #endif
    handle->offset = 0;

    BDBG_ASSERT(handle->offset == 0);

    /* Init to specified settings */
    if ( NULL == pSettings )
    {
        BAPE_MaiInput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    errCode = BAPE_MaiInput_P_OpenHw(handle);
    if ( errCode )
    {
        BAPE_MaiInput_Close(handle);
        return BERR_TRACE(errCode);
    }

    #if defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR
    handle->hwAutoMuteEnabled = pSettings->hardwareMutingEnabled;
    #endif

    errCode = BAPE_MaiInput_SetSettings(handle, pSettings);
    if ( errCode )
    {
        BAPE_MaiInput_Close(handle);
        return BERR_TRACE(errCode);
    }

    *phandle = handle;
    handle->deviceHandle->maiInputs[index] = handle;
    return BERR_SUCCESS;
}

/**************************************************************************/

void BAPE_MaiInput_Close(
    BAPE_MaiInputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    /* Make sure we're not still connected to anything */
    if ( BAPE_InputPort_P_HasConsumersAttached(&handle->inputPort) )
    {
        BDBG_ERR(("Cannot close MAI input %p (%d)", (void *)handle, handle->index));
        #if BDBG_DEBUG_BUILD
        {
            BAPE_PathNode * pConsumer;
            for ( pConsumer = BLST_S_FIRST(&handle->inputPort.consumerList);
                pConsumer != NULL;
                pConsumer = BLST_S_NEXT(pConsumer, consumerNode) )
            {
                BDBG_ERR(("  still connected to %s", pConsumer->pName));
            }

        }
        #endif
        BDBG_ASSERT(!BAPE_InputPort_P_HasConsumersAttached(&handle->inputPort));
        return;
    }

    if ( handle->maiRxCallback ) {
        BINT_DestroyCallback(handle->maiRxCallback);
    }

    if ( handle->inputPort.fciSpGroup )
    {
        BAPE_FciSplitterGroup_P_Destroy(handle->inputPort.fciSpGroup);
        handle->inputPort.fciSpGroup = NULL;
    }

    handle->deviceHandle->maiInputs[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_MaiInput);
    BKNI_Free(handle);
}

/**************************************************************************/

void BAPE_MaiInput_GetSettings(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputSettings *pSettings        /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_SetSettings(
    BAPE_MaiInputHandle handle,
    const BAPE_MaiInputSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(NULL != pSettings);
    handle->settings = *pSettings;

    return BAPE_MaiInput_P_ApplySettings(handle);
}

/**************************************************************************/

void BAPE_MaiInput_GetInputPort(
    BAPE_MaiInputHandle handle,
    BAPE_InputPort *pPort
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(NULL != pPort);
    *pPort = &handle->inputPort;
}

/**************************************************************************/

void BAPE_MaiInput_GetFormatDetectionSettings(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputFormatDetectionSettings *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(NULL != pSettings);

    {  /* Start Critical Section */
        BKNI_EnterCriticalSection();
        *pSettings = handle->clientFormatDetectionSettings;
        BKNI_LeaveCriticalSection();
    }  /* End Critical Section */
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_SetFormatDetectionSettings(
    BAPE_MaiInputHandle handle,
    const BAPE_MaiInputFormatDetectionSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(NULL != pSettings);

    {  /* Start Critical Section */
        BKNI_EnterCriticalSection();
        handle->clientFormatDetectionSettings = *pSettings;
        BAPE_MaiInput_P_SetFormatDetection_isr(handle);
        BKNI_LeaveCriticalSection();
    }  /* End Critical Section */


    return BERR_SUCCESS;
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_GetFormatDetectionStatus(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputFormatDetectionStatus *pStatus
    )
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BAPE_MaiInput_GetFormatDetectionStatus);

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pStatus);

    if ( handle->formatDetectionEnabled )
    {
        /* If Format Detection is enabled, we can just return a copy of whats
         * in the handle's lastFormatDectionStatus structure (since it's being
         * kept up to date). */
        BDBG_CASSERT(sizeof (*pStatus) == sizeof (handle->lastFormatDetectionStatus));
        BKNI_Memcpy(pStatus, &handle->lastFormatDetectionStatus, sizeof (*pStatus));
    }
    else
    {
        /* Format Detection is not enabled, so build a fresh format detection status
         * structure by reading the hardware.  */
        {  /* Start Critical Section */
            BKNI_EnterCriticalSection();
            BAPE_MaiInput_P_GetFormatDetectionStatus_isr(handle, pStatus);
            BKNI_LeaveCriticalSection();
        }  /* End Critical Section */
    }

    BDBG_LEAVE(BAPE_MaiInput_GetFormatDetectionStatus);
    return ret;
}

/***************************************************************************
        BAPE Internal APIs: From bape_fmm_priv.h
***************************************************************************/

BERR_Code BAPE_MaiInput_P_PrepareForStandby(BAPE_Handle hApe)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    maiInputIndex;

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);

    /* For each opened MaiInput... */
    for ( maiInputIndex=0 ; maiInputIndex<BAPE_CHIP_MAX_MAI_INPUTS ; maiInputIndex++ )
    {
        if ( hApe->maiInputs[maiInputIndex] )       /* If this MaiInput is open... */
        {
            BAPE_MaiInputHandle handle = hApe->maiInputs[maiInputIndex];

            /* If we already have a callback (interrupt), make sure that it's disabled, then destroy it. */
            if( handle->maiRxCallback )
            {
                errCode = BINT_DisableCallback(handle->maiRxCallback);
                if (errCode != BERR_SUCCESS)
                {
                    BDBG_ERR(("Unable to Disable MAI RX callback"));
                }

                BINT_DestroyCallback(handle->maiRxCallback);
                handle->maiRxCallback = NULL;
            }
        }
    }
    return errCode;
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_P_ResumeFromStandby(BAPE_Handle hApe)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    maiInputIndex;

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);

    /* For each opened MaiInput, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( maiInputIndex=0 ; maiInputIndex<BAPE_CHIP_MAX_MAI_INPUTS ; maiInputIndex++ )
    {
        if ( hApe->maiInputs[maiInputIndex] )       /* If this MaiInput is open... */
        {
            BAPE_MaiInputHandle handle = hApe->maiInputs[maiInputIndex];

            /* Put the HW into the generic open state. */
            errCode = BAPE_MaiInput_P_OpenHw(handle);
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now apply changes for the settings struct. */
            errCode = BAPE_MaiInput_SetSettings(handle, &handle->settings);
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now restore the dynamic stuff from the values saved in the device struct. */
            errCode = BAPE_MaiInput_SetFormatDetectionSettings(handle, &handle->clientFormatDetectionSettings);
            if ( errCode ) return BERR_TRACE(errCode);
        }
    }
    return errCode;
}

/* work around to reset FCI splitter and HDMI IN - works around FCI splitter locking up HDMI IN when grouping is used */
static void BAPE_MaiInput_P_FciSpReset(BAPE_MaiInputHandle handle)
{
    #if defined BCHP_AUD_MISC_INIT_SPLTR0_LOGIC_INIT_MASK
    unsigned mode = BAPE_Reg_P_ReadField(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL, AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL, MODE);

    BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BCHP_AUD_MISC_INIT, AUD_MISC_INIT, SPLTR0_LOGIC_INIT, Init);
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL, AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL, MODE, (mode == 2) ? 0 : 2);
    BAPE_Reg_P_UpdateField(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL, AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL, MODE, mode);
    BAPE_Reg_P_UpdateEnum(handle->deviceHandle, BCHP_AUD_MISC_INIT, AUD_MISC_INIT, SPLTR0_LOGIC_INIT, Inactive);
    #else
    BSTD_UNUSED(handle);
    #endif
}

/***************************************************************************
        Private callbacks: Protyped above
***************************************************************************/

#if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
static void BAPE_MaiInput_P_SetMclk_isr(BAPE_MaiInputHandle handle, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BAPE_Reg_P_FieldList regFieldList;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(handle->offset == 0);

    /* Save the settings in case we need to re-apply them later. */
    handle->mclkInfo.mclkSource         = mclkSource;
    handle->mclkInfo.pllChannel         = pllChannel;
    handle->mclkInfo.mclkFreqToFsRatio  = mclkFreqToFsRatio;

    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);

    switch ( mclkSource )
    {
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_PLL0_ch1
    case BAPE_MclkSource_ePll0:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL0_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_PLL1_ch1
    case BAPE_MclkSource_ePll1:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL1_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_PLL2_ch1
    case BAPE_MclkSource_ePll2:
        switch ( pllChannel )
        {
        case 0: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch1); break;
        case 1: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch2); break;
        case 2: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, PLL2_ch3); break;
        default: (void) BERR_TRACE(BERR_NOT_SUPPORTED); break;
        }
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen0
    case BAPE_MclkSource_eNco0:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen0);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen1
    case BAPE_MclkSource_eNco1:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen1);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen2
    case BAPE_MclkSource_eNco2:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen2);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen3
    case BAPE_MclkSource_eNco3:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen3);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen4
    case BAPE_MclkSource_eNco4:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen4);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen5
    case BAPE_MclkSource_eNco5:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen6);
        break;
#endif
#ifdef BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0_PLLCLKSEL_Mclk_gen6
    case BAPE_MclkSource_eNco6:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, PLLCLKSEL, Mclk_gen6);
        break;
#endif
    default:
        BDBG_ERR(("Unsupported clock source %u for MAI IN %u", mclkSource, handle->index));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    switch ( mclkFreqToFsRatio )
    {
    case 128: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, MCLK_RATE, MCLK_128fs_SCLK_64fs); break;
    case 256: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, MCLK_RATE, MCLK_256fs_SCLK_64fs); break;
    case 384: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, MCLK_RATE, MCLK_384fs_SCLK_64fs); break;
    case 512: BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0, MCLK_RATE, MCLK_512fs_SCLK_64fs); break;
    default:
        BDBG_ERR(("Unsupported MCLK Rate of %uFs", mclkFreqToFsRatio));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        break;
    }
    BAPE_Reg_P_ApplyFieldList(&regFieldList, BCHP_AUD_FMM_IOP_IN_HDMI_0_MCLK_CFG_0);
}

#define BAPE_NUM_OUTPUTS    10
void BAPE_MaiInput_P_ConfigureClockSource_isr(BAPE_MaiInputHandle handle)
{
    BAPE_PathNode * pConsumer;
    BAPE_Mixer *pMixer = NULL;
    BAPE_OutputPort mai = NULL;
    BAPE_OutputPort alternate[BAPE_NUM_OUTPUTS];
    BAPE_MclkSource mclkSource = BAPE_MclkSource_eNone;
    unsigned i,k, j=0;

    BKNI_Memset(alternate, 0, sizeof(BAPE_OutputPort)*BAPE_NUM_OUTPUTS);

    BDBG_MSG(("Looking for clock source to drive MAI input..."));
    for ( pConsumer = BLST_S_FIRST(&handle->inputPort.consumerList);
        pConsumer != NULL;
        pConsumer = BLST_S_NEXT(pConsumer, consumerNode) )
    {
        BAPE_OutputPort outputs[BAPE_NUM_OUTPUTS];
        unsigned numFound;
        BAPE_PathNode_P_GetConnectedOutputs(pConsumer, BAPE_NUM_OUTPUTS, &numFound, outputs);
        /* try to use Mai output as our reference, if present */
        for ( i=0; i<numFound; i++ )
        {
            if ( outputs[i]->type == BAPE_OutputPortType_eMaiOutput )
            {
                mai = outputs[i];
                BDBG_MSG(("  Found mai output %s", mai->pName));
            }
            else
            {
                alternate[j] = outputs[i];
                BDBG_MSG(("  Found alternate output %s", alternate[j]->pName));
                ++j;
            }
        }
    }

    /* prioritize NCOs, then PLLs... */
    for ( k=0; k<2; k++ )
    {
        if ( mai != NULL )
        {
            pMixer = (BAPE_Mixer*)mai->mixer;
            if ( pMixer &&
                 ( (k==0)?BAPE_MCLKSOURCE_IS_NCO(pMixer->mclkSource):BAPE_MCLKSOURCE_IS_PLL(pMixer->mclkSource) ) )
            {
                if ( BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) != 0 &&
                     handle->inputPort.format.sampleRate >= BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) &&
                     (handle->inputPort.format.sampleRate % BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer)) == 0 )
                {
                    BDBG_MSG(("Mai output is an %s clock match for mai input", (k==0)?"NCO":"PLL"));
                    mclkSource = pMixer->mclkSource;
                }
            }
        }

        for ( i=0; i<j; i++ )
        {
            if ( mclkSource == BAPE_MclkSource_eNone && alternate[i] != NULL )
            {
                pMixer = (BAPE_Mixer*)alternate[i]->mixer;
                if ( pMixer &&
                     ( (k==0)?BAPE_MCLKSOURCE_IS_NCO(pMixer->mclkSource):BAPE_MCLKSOURCE_IS_PLL(pMixer->mclkSource) ) )
                {
                    if ( BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) != 0 &&
                         handle->inputPort.format.sampleRate >= BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) &&
                         (handle->inputPort.format.sampleRate % BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer)) == 0 )
                    {
                        BDBG_MSG(("%s output is an %s clock match for mai input", alternate[i]->pName, (k==0)?"NCO":"PLL"));
                        mclkSource = pMixer->mclkSource;
                    }
                }
            }
        }
    }

    if ( mclkSource == BAPE_MclkSource_eNone )
    {
        #if BAPE_CHIP_MAX_NCOS > 0
        BDBG_WRN(("No clock source found downstream. Defaulting to NCO0 . This may indidate a problem."));
        mclkSource = BAPE_MclkSource_eNco0;
        #elif BAPE_CHIP_MAX_PLLS > 0
        BDBG_WRN(("No clock source found downstream. Defaulting to PLL0 . This may indidate a problem."));
        mclkSource = BAPE_MclkSource_ePll0;
        #else
        BDBG_WRN(("No clock sources found. Cannot configure Mai input clock"));
        #endif
    }

    if ( BAPE_MCLKSOURCE_IS_NCO(mclkSource) )
    {
        BAPE_NcoConfiguration ncoConfig;
        unsigned mclkFreqToFsRatio = 1;
        BAPE_P_GetNcoConfiguration(handle->deviceHandle, mclkSource - BAPE_MclkSource_eNco0, &ncoConfig);
        mclkFreqToFsRatio = ncoConfig.frequency / (handle->inputPort.format.sampleRate == 0 ? 48000: handle->inputPort.format.sampleRate);
        BDBG_MSG(("Success. Setting mai input to use clock source %lu, ch %lu, mclkFreqToFsRatio %lu", (unsigned long)mclkSource, (unsigned long)0, (unsigned long)mclkFreqToFsRatio));
        BAPE_MaiInput_P_SetMclk_isr(handle, mclkSource, 0, mclkFreqToFsRatio);
    }
    else if ( BAPE_MCLKSOURCE_IS_PLL(mclkSource) )
    {
        BDBG_WRN(("Currently MAI IN requires a downstream output that uses an NCO (typically MAI output or Dummy output"));
    }
}
#else
void BAPE_MaiInput_P_ConfigureClockSource_isr(BAPE_MaiInputHandle handle)
{
    BSTD_UNUSED(handle);
}
#endif

static void BAPE_MaiInput_P_Enable(BAPE_InputPort inputPort)
{
    BAPE_MaiInputHandle handle;
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    if ( handle->enable && inputPort->fciSpGroup )
    {
        /* we have a second client asking to enable.
           restart fci splitter to attach that output */
        BDBG_MSG(("Enabling %s (restart FCI Splitter only)", handle->name));
        BAPE_FciSplitterGroup_P_Stop(handle->inputPort.fciSpGroup);
        errCode = BAPE_FciSplitterGroup_P_Start(handle->inputPort.fciSpGroup);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_ERR(("Unable to start MAI Input FCI Splitter"));
        }
        BAPE_MaiInput_P_FciSpReset(handle);
        return;
    }
    else
    {
        BDBG_ASSERT(false == handle->enable);
    }

    BDBG_MSG(("Enabling %s", handle->name));

    /* Group the input channel pairs, then enable capture */

    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN  /* Local (HDMI-specific) STREAM_CFG register */
    {
        unsigned i;
        unsigned numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&inputPort->format);

        uint32_t regAddr;

        /* Set Capture Group ID's for the correct number of channel pairs */
        for ( i = 0; i < BAPE_FMT_P_GetNumChannelPairs_isrsafe(&inputPort->format); i++ )
        {
            regAddr = BAPE_Reg_P_GetArrayAddress(                 AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, i);
            BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, CAP_GROUP_ID, 0);
        }
        for ( ; i <= BCHP_AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i_ARRAY_END; i++ )
        {
            regAddr = BAPE_Reg_P_GetArrayAddress(                 AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, i);
            BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, CAP_GROUP_ID, i);
        }

        /* If we have an FCI splitter, enable it here */
        if ( handle->inputPort.fciSpGroup )
        {
            BAPE_FciSplitterGroupSettings fciGroupSettings;
            unsigned numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&inputPort->format);
            BAPE_FciSplitterGroup_P_Stop(handle->inputPort.fciSpGroup);

            BAPE_FciSplitterGroup_P_GetSettings(handle->inputPort.fciSpGroup, &fciGroupSettings);
            BAPE_InputPort_P_GetFciIds(inputPort, &fciGroupSettings.input);
            fciGroupSettings.numChannelPairs = numChannelPairs;
            errCode = BAPE_FciSplitterGroup_P_SetSettings(handle->inputPort.fciSpGroup, &fciGroupSettings);
            if ( errCode != BERR_SUCCESS )
            {
                BDBG_ERR(("Unable to set FCI Splitter Settings"));
            }
            errCode = BAPE_FciSplitterGroup_P_Start(handle->inputPort.fciSpGroup);
            if ( errCode != BERR_SUCCESS )
            {
                BDBG_ERR(("Unable to start MAI Input FCI Splitter"));
            }
            BAPE_MaiInput_P_FciSpReset(handle);
        }

        /* Enable capture */
        for ( i = 0; i < numChannelPairs; i++ )
        {
            regAddr = BAPE_Reg_P_GetArrayAddress(                 AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, i);
            BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, CAP_ENA, 1);
        }
    }
    #elif defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY  /* Shared pool of STREAM_CFG registers in IOP */
        BAPE_Iop_P_EnableCapture( handle->deviceHandle, inputPort->streamId[0], BAPE_FMT_P_GetNumChannelPairs_isrsafe(&inputPort->format));
    #else
        #error "Unknown MAI register format"
    #endif /* if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN... BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY */

    /* Enable output FIFO */
    {  /* Start Critical Section */
        BKNI_EnterCriticalSection();

        /* Configure Mclk, needed for Locked mode */
        BAPE_MaiInput_P_ConfigureClockSource_isr(handle);

        if ( inputPort->halted )
        {
            BDBG_MSG(("Input %s is halted. can't enable, disabling instead", handle->name ));
            /* This handles the unlikely case of input being halted before the consumer start was complete.
             * Since the input is halted, do a disable instead of enabling... which is the same thing
             * that happens when a halt occurs after enabling.  */
            BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_P_HDMI_RX_CONFIG_REGADDR,
                                                               BAPE_P_HDMI_RX_CONFIG_REGNAME, OUTFIFO_ENA, Disable);
            handle->enable = false;
        }
        else
        {
            /* Normal case.  Input is not halted, so enable the input. */
            BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_P_HDMI_RX_CONFIG_REGADDR,
                                                               BAPE_P_HDMI_RX_CONFIG_REGNAME, OUTFIFO_ENA, Enable);
            handle->enable = true;
        }

        BKNI_LeaveCriticalSection();
    }  /* End Critical Section */
}

/**************************************************************************/

static void BAPE_MaiInput_P_Disable(BAPE_InputPort inputPort)
{
    BAPE_MaiInputHandle handle;

    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    if ( !handle->enable )
    {
        return;
    }

    /* Check if we have multiple consumers */
    if ( BAPE_InputPort_P_GetNumConsumersAttached(inputPort) > 1 )
    {
        return;
    }

    BDBG_MSG(("Disabling %s", handle->name));

    /* Disable the output FIFO. */
    {  /* Start Critical Section */
        BKNI_EnterCriticalSection();
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_P_HDMI_RX_CONFIG_REGADDR,
                                                           BAPE_P_HDMI_RX_CONFIG_REGNAME, OUTFIFO_ENA, Disable);
        handle->enable = false;
        BKNI_LeaveCriticalSection();
    }  /* End Critical Section */

    /* Disable capture and ungroup the channel pairs */
    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN  /* Local (HDMI-specific) STREAM_CFG register */
    {
        uint32_t regAddr;
        unsigned i;

        /* Disable capture */
        for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
        {
            regAddr = BAPE_Reg_P_GetArrayAddress(                 AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, i);
            BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, CAP_ENA, 0);
        }

        if ( handle->inputPort.fciSpGroup )
        {
            BAPE_FciSplitterGroup_P_Stop(handle->inputPort.fciSpGroup);
        }

        /* Ungroup */
        for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
        {
            regAddr = BAPE_Reg_P_GetArrayAddress(                 AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, i);
            BAPE_Reg_P_UpdateField(handle->deviceHandle, regAddr, AUD_FMM_IOP_IN_HDMI_0_CAP_STREAM_CFG_i, CAP_GROUP_ID, i);
        }
    }
    #elif defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY  /* Shared pool of STREAM_CFG registers in IOP */
        BAPE_Iop_P_DisableCapture( handle->deviceHandle, inputPort->streamId[0], BAPE_ChannelPair_eMax );
    #else
        #error "Unknown MAI register format"
    #endif /* if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN, BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY */
}

/**************************************************************************/

static void BAPE_MaiInput_P_Halt_isr(BAPE_InputPort inputPort)
{
    BAPE_MaiInputHandle handle;

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    BDBG_MSG(("Halting %s", handle->name));

    /* Remember the halted state. */
    BAPE_InputPort_P_Halt_isr(inputPort);

    /* Disable the output FIFO. */
    if ( handle->enable )
    {
        BAPE_Reg_P_UpdateEnum_isr(handle->deviceHandle, BAPE_P_HDMI_RX_CONFIG_REGADDR,
                                                           BAPE_P_HDMI_RX_CONFIG_REGNAME, OUTFIFO_ENA, Disable);
    }
}

/**************************************************************************/

static BERR_Code BAPE_MaiInput_P_ConsumerAttached_isr(BAPE_InputPort inputPort, BAPE_PathNode *pConsumer, BAPE_FMT_Descriptor *pFormat)
{
    BAPE_MaiInputHandle handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    BDBG_MSG(("Attaching consumer %s", pConsumer->pName));

    switch ( pConsumer->type )
    {
    case BAPE_PathNodeType_eDecoder:
    case BAPE_PathNodeType_eInputCapture:
        break;
    default:
        BDBG_ERR(("Node %s is not a valid consumer for MAI Input", pConsumer->pName));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_ASSERT(BAPE_InputPort_P_ConsumerIsAttached(inputPort, pConsumer));

    /* Enable format detection interupts.  After this, the current input format
     * will be maintained in the handle's lastFormatDetectionStatus struct.  */
    handle->localFormatDetectionEnabled = true;
    BAPE_MaiInput_P_SetFormatDetection_isr(handle);

    /* Update the InputPort's fields to match the current input format. */
    BAPE_InputPort_P_GetFormat_isr(inputPort, pFormat);
    BAPE_MaiInput_P_UpdateFormat_isr (handle, &handle->lastFormatDetectionStatus, pFormat);
    BAPE_MaiInput_P_SetReceiverOutputFormat_isr (handle, &handle->lastFormatDetectionStatus);
    errCode = BAPE_InputPort_P_SetFormat_isr(inputPort, pFormat);

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_MaiInput_P_ConsumerDetached_isr(BAPE_InputPort inputPort, BAPE_PathNode *pConsumer)
{
    BAPE_MaiInputHandle handle;

    BSTD_UNUSED(pConsumer);

    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    handle = inputPort->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    if ( BAPE_InputPort_P_GetNumConsumersAttached(inputPort) <= 1 )
    {
        /* Disable local format detection. */
        handle->localFormatDetectionEnabled = false;
        BAPE_MaiInput_P_SetFormatDetection_isr(handle);
    }
    else
    {
        BDBG_MSG(("Detached one consumer %s (restart FCI Splitter only)", pConsumer->pName));
        BAPE_MaiInput_P_FciSpReset(handle);
    }

    return;
}

/***************************************************************************
        Private functions: Protyped above
***************************************************************************/

static BERR_Code BAPE_MaiInput_P_OpenHw(BAPE_MaiInputHandle handle)
{
    uint32_t regAddr;
    BAPE_Handle hApe;
    BAPE_Reg_P_FieldList regFieldList;
    BERR_Code errCode = BERR_SUCCESS;

    /* Currently we only support one MAI input, this can be expanded later if we support more */
    #if BAPE_CHIP_MAX_MAI_INPUTS > 1
        #error "Need to support more MAI inputs"
    #endif

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    hApe = handle->deviceHandle;
    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);

    /* Taken from RAP PI (regarding ALLOW_NZ_STUFFING=Nonzero_Ok) -->
    PR 35668: Some Blu Ray DVD Players send Non Zero values between compressed
    frames. This was getting treated as PCM data and causing confusion in 3563
    and a workaround was put. In 3548 this has been fixed in hardware.
    Enabling the bit here.
    */

    BAPE_Reg_P_InitFieldList(hApe, &regFieldList);

    regAddr = BAPE_P_HDMI_RX_CONFIG_REGADDR;

    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, OUT_FORMAT_ENA, ALL);
    handle->outFormatEna = BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_CONFIG_REGNAME, OUT_FORMAT_ENA, ALL);  /* Remember OUT_FORMAT_ENA setting. */

    BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, ALLOW_NZ_STUFFING, Nonzero_OK);
    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, SOURCE_SEL, MAI);
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, HDMI_SEL, HDMI0);
    #endif /* if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY */

    BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

    /* Clear the interrupts before enabling any callbacks */
    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
        /* Clear and unmask L3 interrupts. */
        BAPE_Reg_P_Write(hApe, BCHP_AUD_FMM_IOP_IN_HDMI_0_ESR_MASK_SET,     0xffffffff);
        BAPE_Reg_P_Write(hApe, BCHP_AUD_FMM_IOP_IN_HDMI_0_ESR_STATUS_CLEAR, 0xffffffff);
        BAPE_Reg_P_Write(hApe, BCHP_AUD_FMM_IOP_IN_HDMI_0_ESR_MASK_CLEAR,  BCHP_MASK( AUD_FMM_IOP_IN_HDMI_0_ESR_MASK_CLEAR, HDMIRX_CHAN_STAT_MASK) |
                                                                           BCHP_MASK( AUD_FMM_IOP_IN_HDMI_0_ESR_MASK_CLEAR, HDMIRX_PC_CHANGE_MASK) |
                                                                           /* this interrupt is coming repeatedly in Low Latency Passthrough HBR mode
                                                                           BCHP_MASK( AUD_FMM_IOP_IN_HDMI_0_ESR_MASK_CLEAR, HDMIRX_COMP_CHANGE_MASK) | */
                                                                           BCHP_MASK( AUD_FMM_IOP_IN_HDMI_0_ESR_MASK_CLEAR, HDMIRX_MAI_FMT_CHANGE_MASK) );

        /* Clear and unmask L2 interrupt. */
        BAPE_Reg_P_Write(hApe, BCHP_AUD_FMM_IOP_MISC_ESR_MASK_SET,     BCHP_MASK( AUD_FMM_IOP_MISC_ESR_MASK_SET,     IOP_INTERRUPT_IN_HDMI_0));
        BAPE_Reg_P_Write(hApe, BCHP_AUD_FMM_IOP_MISC_ESR_STATUS_CLEAR, BCHP_MASK( AUD_FMM_IOP_MISC_ESR_STATUS_CLEAR, IOP_INTERRUPT_IN_HDMI_0));
        BAPE_Reg_P_Write(hApe, BCHP_AUD_FMM_IOP_MISC_ESR_MASK_CLEAR,   BCHP_MASK( AUD_FMM_IOP_MISC_ESR_MASK_CLEAR,   IOP_INTERRUPT_IN_HDMI_0));

        BAPE_Reg_P_UpdateEnum_isr(hApe, BCHP_AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL,
                                                           AUD_FMM_IOP_IN_HDMI_0_OUT_RATE_CTRL, MODE, Locked);

    #endif /* if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN */
    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
        /* Clear and unmask L2 interrupts. */
        BAPE_Reg_P_Write(hApe, BCHP_HDMI_RCVR_ESR_MASK_SET,     0xffffffff);
        BAPE_Reg_P_Write(hApe, BCHP_HDMI_RCVR_ESR_STATUS_CLEAR, 0xffffffff);
        BAPE_Reg_P_Write(hApe, BCHP_HDMI_RCVR_ESR_MASK_CLEAR, BCHP_MASK( SPDIF_RCVR_ESR_MASK_CLEAR, SPDIFRX_CHAN_STAT_MASK )   |
                                                              BCHP_MASK( SPDIF_RCVR_ESR_MASK_CLEAR, SPDIFRX_PC_CHANGE_MASK )   |
                                                              BCHP_MASK( SPDIF_RCVR_ESR_MASK_CLEAR, SPDIFRX_COMP_CHANGE_MASK ) |
                                                              BCHP_MASK( SPDIF_RCVR_ESR_MASK_CLEAR, SPDIFRX_MAI_FMT_CHANGE_MASK ));

        BAPE_Reg_P_UpdateEnum_isr(hApe, BCHP_HDMI_RCVR_CTRL_OUT_RATE_CTRL,
                                                            SPDIF_RCVR_CTRL_OUT_RATE_CTRL, MODE, Locked);
    #endif /* if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY */

    {
        #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
            BINT_Id             intId = BCHP_INT_ID_AUD_IOP;
        #endif /* if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN */
        #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
            BINT_Id             intId = BCHP_INT_ID_HDMIRX;
        #endif /* if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY */

        errCode = BINT_CreateCallback( &handle->maiRxCallback,
                                       hApe->intHandle,
                                       intId,
                                       BAPE_P_MAI_RX_isr,
                                       (void*)handle,
                                       0 /* Not used*/
                                      );
    }
    if (errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Unable to create MAI RX callback"));
        return errCode;
    }

    return errCode;
}

/**************************************************************************/

static BERR_Code BAPE_MaiInput_P_ApplySettings(BAPE_MaiInputHandle handle)
{
    BAPE_Reg_P_FieldList regFieldList;
    const BAPE_MaiInputSettings *pSettings = &handle->settings;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_Reg_P_InitFieldList(handle->deviceHandle, &regFieldList);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, IGNORE_VALID_PCM, pSettings->ignoreValidity);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, IGNORE_PERR_PCM,  pSettings->ignorePcmParity);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, IGNORE_PERR_COMP, pSettings->ignoreCompressedParity);
    BAPE_Reg_P_AddToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, TIMEBASE_SEL,     pSettings->stcIndex);

    switch ( pSettings->errorInsertion )
    {
    case BAPE_SpdifInputErrorInsertion_eNone:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, INSERT_MODE, No_insert);
        break;
    case BAPE_SpdifInputErrorInsertion_eZero:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, INSERT_MODE, Insert_zero);
        break;
    case BAPE_SpdifInputErrorInsertion_eRepeat:
        BAPE_Reg_P_AddEnumToFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGNAME, INSERT_MODE, Insert_repeat);
        break;
    default:
        BDBG_ERR(("Invalid Error Insertion Mode %d", pSettings->errorInsertion));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    {  /* Start Critical Section */
        BKNI_EnterCriticalSection();
        BAPE_Reg_P_ApplyFieldList(&regFieldList, BAPE_P_HDMI_RX_CONFIG_REGADDR);
        BKNI_LeaveCriticalSection();
    }  /* End Critical Section */

    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_P_MAI_RX_isr (
        void * pParm1, /* [in] channel handle */
        int    iParm2  /* [in] Not used */
)
{
    BAPE_MaiInputHandle handle = NULL;
    uint32_t                        ui32IntStatus=0;
    uint32_t                        ui32MaskStatus=0;

    BDBG_ENTER (BAPE_P_MAI_RX_isr);

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_ASSERT (pParm1);
    BSTD_UNUSED(iParm2);

    handle = (BAPE_MaiInputHandle) pParm1;

    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
        ui32IntStatus = BAPE_Reg_P_Read_isr(handle->deviceHandle,  BCHP_AUD_FMM_IOP_IN_HDMI_0_ESR_STATUS);
        ui32MaskStatus = BAPE_Reg_P_Read_isr(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_ESR_MASK);
    #endif
    #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
        ui32IntStatus = BAPE_Reg_P_Read_isr(handle->deviceHandle,  BCHP_HDMI_RCVR_ESR_STATUS);
        ui32MaskStatus = BAPE_Reg_P_Read_isr(handle->deviceHandle, BCHP_HDMI_RCVR_ESR_MASK);
    #endif

    ui32IntStatus &= ~ui32MaskStatus;

    BDBG_MSG(("MAI_RX_ISR: ESR_STATUS (unmasked): 0x%x", ui32IntStatus));

    if (ui32IntStatus)
    {
        #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
            BAPE_Reg_P_Write_isr(handle->deviceHandle, BCHP_AUD_FMM_IOP_MISC_ESR_STATUS_CLEAR, BCHP_MASK( AUD_FMM_IOP_MISC_ESR_STATUS_CLEAR, IOP_INTERRUPT_IN_HDMI_0));
            BAPE_Reg_P_Write_isr(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_ESR_STATUS_CLEAR, ui32IntStatus);
        #endif
        #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
            BAPE_Reg_P_Write_isr(handle->deviceHandle, BCHP_HDMI_RCVR_ESR_STATUS_CLEAR, ui32IntStatus);
        #endif

        BAPE_MaiInput_P_DetectInputChange_isr(handle);
    }
    BDBG_LEAVE (BAPE_P_MAI_RX_isr);
    return;
}

/**************************************************************************/

static BERR_Code BAPE_MaiInput_P_SetFormatDetection_isr(
    BAPE_MaiInputHandle handle
    )
{
    BERR_Code ret = BERR_SUCCESS;
    bool formatDetectionRequired = false;

    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);


    /* There are two reasons for enabling format detection.  First, when the input is enabled, we need
     * to use format detection for setting up the receiver's output format, and to halt the outfifo
     * in the case of a format change that can't be handled on-the-fly.  Secondly, the client (Nexus)
     * can enable format detection to keep track of the current input format.
     * So start by checking to see if anybody needs format detection on. */

    /* Check to see if our client needs format detection. */
    if ( handle->clientFormatDetectionSettings.enabled == true &&
         (handle->clientFormatDetectionSettings.formatChangeIntInput.pCallback_isr ||
          handle->clientFormatDetectionSettings.formatChangeIntDecode.pCallback_isr) ){

        formatDetectionRequired = true;
    }

    /* Now see if we need it internally.  */
    if ( handle->localFormatDetectionEnabled )
    {
        formatDetectionRequired = true;
    }

    /* If it's not enabled and somebody needs it, then turn it on. */
    if (! handle->formatDetectionEnabled && formatDetectionRequired )
    {
        /* Make a call to BAPE_MaiInput_P_GetFormatDetectionStatus_isr() to get a
         * fresh snapshot of the current state of things from the hardware
         * (so that we can detect a change when interrupts occur later.   */
        BAPE_MaiInput_P_GetFormatDetectionStatus_isr(handle, &handle->lastFormatDetectionStatus);

        BINT_EnableCallback_isr(handle->maiRxCallback);
        handle->formatDetectionEnabled = true;
    }

    /* If format detection is enabled, but nobody needs it, then turn if off.  */
    if ( handle->formatDetectionEnabled && ! formatDetectionRequired )
    {
        if( handle->maiRxCallback )
        {
            handle->formatDetectionEnabled = false;
            ret = BINT_DisableCallback_isr(handle->maiRxCallback);
            if (ret != BERR_SUCCESS)
            {
                BDBG_ERR(("Unable to Disable MAI RX callback"));
            }
        }
    }

    return BERR_SUCCESS;
}

/**************************************************************************/

static BERR_Code BAPE_MaiInput_P_GetFormatDetectionStatus_isr(BAPE_MaiInputHandle handle, BAPE_MaiInputFormatDetectionStatus *pStatus)
{
    uint32_t        receiverStatusRegVal = 0;
    uint32_t        maiFormatRegVal = 0;
    uint32_t        maiFormatRegAudioFormat = 0;
    uint32_t        maiFormatRegSampleRate = 0;
    uint32_t        burstPreamble = 0;
    unsigned int    i;

    BDBG_ENTER(BAPE_MaiInput_P_GetFormatDetectionStatus_isr);

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);
    BDBG_ASSERT(pStatus);

    /* Clear out the destination buffer. */
    BKNI_Memset(pStatus, 0, sizeof (*pStatus));

    /* Set some default values in case we don't find a reason to change them. */
    pStatus->codec            = BAVC_AudioCompressionStd_eMax;
    pStatus->signalPresent    = true;

    /* Let them know if the format change interrupts are enabled.  */
    pStatus->detectionEnabled   = false;
    if ( handle->clientFormatDetectionSettings.enabled == true &&
        (handle->clientFormatDetectionSettings.formatChangeIntInput.pCallback_isr ||
         handle->clientFormatDetectionSettings.formatChangeIntDecode.pCallback_isr) )
    {
        pStatus->detectionEnabled   = true;
    }

    /* Read the HDMI Receiver's MAI_FORMAT and STATUS registers.  */
    maiFormatRegVal         = BAPE_Reg_P_Read(handle->deviceHandle, BAPE_P_HDMI_RX_MAI_FORMAT_REGADDR);
    receiverStatusRegVal    = BAPE_Reg_P_Read(handle->deviceHandle, BAPE_P_HDMI_RX_STATUS_REGADDR);

    /* Gather some of the fields from the MAI Format register. */
    pStatus->sampleWidth = BCHP_GET_FIELD_DATA(maiFormatRegVal , BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_WIDTH);

    /* Get the audio format from the MAI Format register, then convert to the BAPE_MaiInputFormat enum. */
    maiFormatRegAudioFormat = BCHP_GET_FIELD_DATA(maiFormatRegVal, BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT);
    {
        struct
        {
            uint32_t                hwAudioFormat;
            BAPE_MaiInputFormat     maiInputFormat;
            bool                    compressed;     /* true => stream is compressed */
            bool                    hbr;            /* true => HBR stream           */
            unsigned                numPcmChannels;   /* Total number of PCM audio channels, 0 for comnpressed streams */
        } maiInputFormatInfo[] =
        {    /* hwAudioFormat                                                                                      maiInputFormat                              compressed   hbr    numPcmChannels */


            {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, Idle),                               BAPE_MaiInputFormat_eIdle,                      false,    false,   0 },
            #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
                {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, Linear_PCM_audio_stereo_mode),   BAPE_MaiInputFormat_eSpdifLinearPcm,            false,    false,   2 },
            #endif
            #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
                {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, HDMI_linearPCM_stereo),          BAPE_MaiInputFormat_eHdmiPcmStereo,             false,    false,   2 },
                {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, SPDIF_linearPCM),                BAPE_MaiInputFormat_eSpdifLinearPcm,            false,    false,   2 },
            #endif
            {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, SPDIF_linearPCM_stereo),             BAPE_MaiInputFormat_eSpdifPcmStereo,            false,    false,   2 },
            {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, SPDIF_linearPCM_6_channel),          BAPE_MaiInputFormat_eSpdifPcm6Channel,          false,    false,   6 },
            {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, SPDIF_linearPCM_8_channel),          BAPE_MaiInputFormat_eSpdifPcm8Channel,          false,    false,   8 },

            {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, HBR_compressed_8_channel),           BAPE_MaiInputFormat_eHbrCompressed,              true,     true,   0 },

            #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY
                {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, HDMI_nonlinearPCM),              BAPE_MaiInputFormat_eHdmiNonLinearPcm,           true,    false,   0 },
                {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, SPDIF_nonlinearPCM),             BAPE_MaiInputFormat_eSpdifNonLinearPcm,          true,    false,   0 },
            #endif
            #if defined BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN
                {BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, AUDIO_FORMAT, Compressed_audio_2_channel),     BAPE_MaiInputFormat_eSpdifNonLinearPcm,          true,    false,   0 },
            #endif
        };

        /* Loop through the table, looking for a match to the MAI Format audio format bits. */
        for ( i=0 ; i<BAPE_P_NUM_ELEMS(maiInputFormatInfo) ; i++ )
        {
            if ( maiInputFormatInfo[i].hwAudioFormat == maiFormatRegAudioFormat )  break;
        }

        if ( i < BAPE_P_NUM_ELEMS(maiInputFormatInfo) )   /* Found a match, use table entry. */
        {
            pStatus->compressed         = maiInputFormatInfo[i].compressed;
            pStatus->hbr                = maiInputFormatInfo[i].hbr;
            pStatus->numPcmChannels     = maiInputFormatInfo[i].numPcmChannels;
        }
        else  /* MAI Format's audio format not listed in table.  */
        {
            BDBG_ERR (("MAI Format: 0x%08x not valid => BAVC_AudioCompressionStd_eMax!!!", maiFormatRegAudioFormat));
            pStatus->compressed      = false;
            pStatus->hbr             = false;
            pStatus->numPcmChannels     = 0;
        }
    }

    /* Get the sample rate from the MAI Format register and convert to an integer. */
    maiFormatRegSampleRate = BCHP_GET_FIELD_DATA(maiFormatRegVal, BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE);

    /* HBR is always 192k, ignore the value in MAI_FORMAT */
    if ( pStatus->hbr )
    {
        pStatus->sampleRate = 192000;
    }
    else
    {
        switch (maiFormatRegSampleRate)
        {
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_8000Hz):      pStatus->sampleRate =  8000;      break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_11025Hz):     pStatus->sampleRate =  11025;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_12000Hz):     pStatus->sampleRate =  12000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_16000Hz):     pStatus->sampleRate =  16000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_22050Hz):     pStatus->sampleRate =  22050;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_24000Hz):     pStatus->sampleRate =  24000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_32000Hz):     pStatus->sampleRate =  32000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_44100Hz):     pStatus->sampleRate =  44100;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_48000Hz):     pStatus->sampleRate =  48000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_64000Hz):     pStatus->sampleRate =  64000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_88200Hz):     pStatus->sampleRate =  88200;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_96000Hz):     pStatus->sampleRate =  96000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_12800Hz):     pStatus->sampleRate = 128000;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_17640Hz):     pStatus->sampleRate = 176400;     break;
            case BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_MAI_FORMAT_REGNAME, SAMPLE_RATE, Rate_19200Hz):     pStatus->sampleRate = 192000;     break;
            default:                                                                                 pStatus->sampleRate =      0;     break;
        }
    }

    if ( pStatus->sampleRate == 0 )
    {
        BDBG_MSG (("MAI FORMAT : Sample Rate is Unknown"));
    }
    else
    {
        BDBG_MSG (("MAI FORMAT : Sample Rate is %u", pStatus->sampleRate ));
    }

    if ( ! pStatus->compressed )
    {
        if ( pStatus->numPcmChannels > 0 )
        {
            pStatus->codec = BAVC_AudioCompressionStd_ePcm;
            BDBG_MSG (("MAI_Format: Uncompressed => BAVC_AudioCompressionStd_ePcm"));
        }
        else
        {
            pStatus->signalPresent    = false;
            pStatus->codec = BAVC_AudioCompressionStd_eMax;
        }
    }
    else  /* must be compressed */
    {
            BDBG_MSG (("MAI_Format: Compressed... Need to check Preamble C bits"));
            pStatus->pcValid        = BCHP_GET_FIELD_DATA(receiverStatusRegVal, BAPE_P_HDMI_RX_STATUS_REGNAME, BPC_VALID);

            if (!pStatus->pcValid)   /* If Preamble C is not valid */
            {
                pStatus->codec = BAVC_AudioCompressionStd_eMax;
                BDBG_MSG (("Preamble C: Not yet valid: BAVC_AudioCompressionStd_eMax"));
            }
            else /* if preamble C is valid... */
            {
                burstPreamble = BCHP_GET_FIELD_DATA(receiverStatusRegVal, BAPE_P_HDMI_RX_STATUS_REGNAME, BURST_PREAM_C);

                BAPE_InputPort_P_BurstPreambleToCodec_isr(burstPreamble, &pStatus->codec );

            } /* End if preambleC is valid */
    } /* End if not linear PCM */

    BDBG_LEAVE(BAPE_MaiInput_P_GetFormatDetectionStatus_isr);

    return BERR_SUCCESS;
}

/**************************************************************************/

static void BAPE_MaiInput_P_DetectInputChange_isr (BAPE_MaiInputHandle    handle)
{
    #if defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR
    uint32_t formatCounter = 0;
    uint32_t formatCounterReturn = 0;
    unsigned changeCount = 0;
    #endif

    BAPE_MaiInputFormatDetectionStatus  oldFormatDetectionStatus, *pOldFormatDetectionStatus, *pNewFormatDetectionStatus;
    BERR_Code errCode;

    BDBG_ENTER(BAPE_MaiInput_P_DetectInputChange_isr);

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    pOldFormatDetectionStatus = &oldFormatDetectionStatus;
    pNewFormatDetectionStatus = &handle->lastFormatDetectionStatus;

    #if defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR
    do
    #endif
    {
        /* HW Mute detector */
        #if defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR
        if ( handle->hwAutoMuteEnabled )
        {
            formatCounter = BAPE_Reg_P_ReadField_isr(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR, AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR, VAL);
            formatCounterReturn = BAPE_Reg_P_ReadField_isr(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR_RETURN, AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR_RETURN, VAL);
            changeCount = 10; /* allow up to 10 format changes in a row, fail after that */
        }
        #endif

        /* Make a copy of the current "old" format detection snapshot. */
        BKNI_Memcpy_isr(pOldFormatDetectionStatus, pNewFormatDetectionStatus, sizeof handle->lastFormatDetectionStatus);

        /* Now build a "new" format detection snapshot by reading the hardware. */
        BAPE_MaiInput_P_GetFormatDetectionStatus_isr(handle, pNewFormatDetectionStatus);

        BAPE_MaiInput_P_SetReceiverOutputFormat_isr (handle, pNewFormatDetectionStatus);

        BDBG_MSG(("MAI FORMAT DETECT"));
        if (BKNI_Memcmp_isr(pNewFormatDetectionStatus, pOldFormatDetectionStatus, sizeof (*pNewFormatDetectionStatus)) != 0)
        {
            BAPE_FMT_Descriptor format;

            BDBG_MSG(("MAI FORMAT CHANGED:"));
            BDBG_MSG(("  new: comp %u, hbr %u, numChs %u, codec %u", (uint32_t)pNewFormatDetectionStatus->compressed,(uint32_t)pNewFormatDetectionStatus->hbr,(uint32_t)pNewFormatDetectionStatus->numPcmChannels,(uint32_t)pNewFormatDetectionStatus->codec));
            BDBG_MSG(("  old: comp %u, hbr %u, numChs %u, codec %u", (uint32_t)pOldFormatDetectionStatus->compressed,(uint32_t)pOldFormatDetectionStatus->hbr,(uint32_t)pOldFormatDetectionStatus->numPcmChannels,(uint32_t)pOldFormatDetectionStatus->codec));
            BAPE_InputPort_P_GetFormat_isr(&handle->inputPort, &format);
            BAPE_MaiInput_P_UpdateFormat_isr (handle, pNewFormatDetectionStatus, &format);
            /* Let the consumer know about the format change. */
            errCode = BAPE_InputPort_P_SetFormat_isr(&handle->inputPort, &format);
            if ( errCode )
            {
                /* The consumer can't handle the format change on-the-fly, so halt the capture.
                 * Nexus will need to stop and restart the decoder. */
                BDBG_MSG(("Halting MAI"));
                BAPE_MaiInput_P_Halt_isr(&handle->inputPort);
            }

            /* Notify our client (probably Nexus) that there has been a format change.  */
            if ( handle->clientFormatDetectionSettings.enabled == true )
            {
                if ( handle->clientFormatDetectionSettings.formatChangeIntInput.pCallback_isr )
                {
                    BDBG_MSG(("%s - Calling format change callback (InputCapture) ", __FUNCTION__));
                    handle->clientFormatDetectionSettings.formatChangeIntInput.pCallback_isr(
                                              handle->clientFormatDetectionSettings.formatChangeIntInput.pParam1,
                                              handle->clientFormatDetectionSettings.formatChangeIntInput.param2);
                }
                if ( handle->clientFormatDetectionSettings.formatChangeIntDecode.pCallback_isr )
                {
                    BDBG_MSG(("%s - Calling format change callback (Decode)", __FUNCTION__));
                    handle->clientFormatDetectionSettings.formatChangeIntDecode.pCallback_isr(
                                              handle->clientFormatDetectionSettings.formatChangeIntDecode.pParam1,
                                              handle->clientFormatDetectionSettings.formatChangeIntDecode.param2);
                }
            }

            /* Done with the important stuff,  now print out each of the fields and indicate whether they've changed. */
            #if BDBG_DEBUG_BUILD
            #define BAPE_PRINT_CHANGE(name, pfmt, old, new)                                  \
                (  (old) != (new)                                                            \
                    ?      BDBG_MODULE_MSG(bape_mai_input_detail, ("%s: " pfmt " -> " pfmt , name,   (old),  (new )))    \
                    :      BDBG_MODULE_MSG(bape_mai_input_detail, ("%s: " pfmt, name, (new) ))                           \
                    )

            BDBG_MODULE_MSG(bape_mai_input_detail, ("--------MAI Input Format Change Detection ---- begin ----"));

            BAPE_PRINT_CHANGE("  Codec from Preamble C", "%s",     BAPE_P_GetCodecName(pOldFormatDetectionStatus->codec),                   BAPE_P_GetCodecName(pNewFormatDetectionStatus->codec)  );
            BAPE_PRINT_CHANGE("  Num Chans (PCM only) ", "%u",     pOldFormatDetectionStatus->numPcmChannels,   pNewFormatDetectionStatus->numPcmChannels );
            BAPE_PRINT_CHANGE("  Sample Rate          ", "%u",     pOldFormatDetectionStatus->sampleRate,       pNewFormatDetectionStatus->sampleRate );
            BAPE_PRINT_CHANGE("  Sample Width         ", "%u",     pOldFormatDetectionStatus->sampleWidth,      pNewFormatDetectionStatus->sampleWidth);
            BAPE_PRINT_CHANGE("  Signal Present       ", "%s",     pOldFormatDetectionStatus->signalPresent    ? "TRUE" : "false", pNewFormatDetectionStatus->signalPresent    ? "TRUE" : "false" );
            BAPE_PRINT_CHANGE("  Compressed Flag      ", "%s",     pOldFormatDetectionStatus->compressed       ? "TRUE" : "false",       pNewFormatDetectionStatus->compressed ? "TRUE" : "false" );
            BAPE_PRINT_CHANGE("  Hi-bitrate (HBR) Flag", "%s",     pOldFormatDetectionStatus->hbr              ? "TRUE" : "false",       pNewFormatDetectionStatus->hbr        ? "TRUE" : "false" );
            BAPE_PRINT_CHANGE("  Preamble C Valid     ", "%s",     pOldFormatDetectionStatus->pcValid          ? "TRUE" : "false", pNewFormatDetectionStatus->pcValid          ? "TRUE" : "false" );
            BAPE_PRINT_CHANGE("  Detection Enabled    ", "%s",     pOldFormatDetectionStatus->detectionEnabled ? "TRUE" : "false", pNewFormatDetectionStatus->detectionEnabled ? "TRUE" : "false" );

            BDBG_MODULE_MSG(bape_mai_input_detail, ("--------MAI Input Format Change Detection ---- end ----"));
            #endif
        }
        #if BDBG_DEBUG_BUILD
        else
        {
            BDBG_MSG(("  no change"));
        }
        #endif

        /* HW Mute detection - only available on 7445 D0 28nm and newer chips */
        #if defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR
        if ( handle->hwAutoMuteEnabled )
        {
            BDBG_MSG(("--------MAI Input Updating Format Counter from %d to %d", formatCounterReturn, formatCounter));
            formatCounterReturn = formatCounter;
            BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR_RETURN,
                                       AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR_RETURN, VAL, formatCounterReturn);

            /* Re-read the format, in case it changed again */
            formatCounter = BAPE_Reg_P_ReadField_isr(handle->deviceHandle, BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR, AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR, VAL);
            changeCount--;
            if ( changeCount == 0 )
            {
                BDBG_ERR(("--------ERROR: MAI Input Format Changed more than 10 times, format may not be set correctly at this point."));
            }
        }
        #endif

    }
    #if defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT_CNTR
    while ( formatCounter != formatCounterReturn && changeCount > 0 );
    #endif

    BDBG_LEAVE (BAPE_MaiInput_P_DetectInputChange_isr);
    return;
}


/**************************************************************************/

static void BAPE_MaiInput_P_SetReceiverOutputFormat_isr (BAPE_MaiInputHandle handle, BAPE_MaiInputFormatDetectionStatus *pFormatDetectionStatus)
{
    uint32_t  outFormatEna;

    BDBG_ENTER(BAPE_MaiInput_P_SetReceiverOutputFormat_isr);

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    outFormatEna =  BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_CONFIG_REGNAME, OUT_FORMAT_ENA, ALL);

    if ( outFormatEna != handle->outFormatEna )
    {
        handle->outFormatEna = outFormatEna; /* Update the saved state in the handle. */

        BDBG_MSG (("Switching HDMI Receiver output format to %s",
                   (outFormatEna ==  BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_CONFIG_REGNAME, OUT_FORMAT_ENA, COMP) ) ? "COMP" :
                   (outFormatEna ==  BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_CONFIG_REGNAME, OUT_FORMAT_ENA, PES) )  ? "PES"  :
                   (outFormatEna ==  BAPE_P_BCHP_ENUM(BAPE_P_HDMI_RX_CONFIG_REGNAME, OUT_FORMAT_ENA, PCM) )  ? "PCM"  :
                                                                                                            "Unknown" ));

        BAPE_Reg_P_UpdateField_isr(handle->deviceHandle,
                                  BAPE_P_HDMI_RX_CONFIG_REGADDR,
                                  BAPE_P_HDMI_RX_CONFIG_REGNAME, OUT_FORMAT_ENA, outFormatEna);

    }
    #if BAPE_CHIP_MAI_INPUT_HBR_SUPPORT
    BDBG_MSG(("Setting HBR_MODE to %u", pFormatDetectionStatus->hbr ? 1 : 0));
    BAPE_Reg_P_UpdateField_isr(handle->deviceHandle, BAPE_P_HDMI_RX_CONFIG_REGADDR,
                               BAPE_P_HDMI_RX_CONFIG_REGNAME, HBR_MODE, pFormatDetectionStatus->hbr?1:0);

    #else
    BSTD_UNUSED(pFormatDetectionStatus);
    #endif

    BDBG_LEAVE (BAPE_MaiInput_P_SetReceiverOutputFormat_isr);
    return;
}

/**************************************************************************/

static void BAPE_MaiInput_P_UpdateFormat_isr (BAPE_MaiInputHandle handle, BAPE_MaiInputFormatDetectionStatus *pFormatDetectionStatus, BAPE_FMT_Descriptor *pFormat)
{
    BAPE_InputPort  inputPort;
    BDBG_ENTER(BAPE_MaiInput_P_UpdateFormat_isr);

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(handle, BAPE_MaiInput);

    inputPort = &handle->inputPort;
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);

    pFormat->sampleRate = pFormatDetectionStatus->sampleRate;
    if ( pFormatDetectionStatus->hbr )
    {
        pFormat->type = BAPE_DataType_eIec61937x16;
        BAPE_FMT_P_SetAudioCompressionStd_isrsafe(pFormat, pFormatDetectionStatus->codec);
    }
    else if ( pFormatDetectionStatus->compressed )
    {
        if ( pFormat->sampleRate == 176400 || pFormat->sampleRate == 192000 )
        {
            pFormat->type = BAPE_DataType_eIec61937x4;
            BAPE_FMT_P_SetAudioCompressionStd_isrsafe(pFormat, pFormatDetectionStatus->codec);
        }
        else
        {
            pFormat->type = BAPE_DataType_eIec61937;
            BAPE_FMT_P_SetAudioCompressionStd_isrsafe(pFormat, pFormatDetectionStatus->codec);
        }
    }
    else
    {
        if ( pFormatDetectionStatus->numPcmChannels == 8 )
        {
            pFormat->type = BAPE_DataType_ePcm7_1;
        }
        else if ( pFormatDetectionStatus->numPcmChannels == 6 )
        {
            pFormat->type = BAPE_DataType_ePcm5_1;
        }
        else if ( pFormatDetectionStatus->numPcmChannels == 0 )
        {
            pFormat->type = BAPE_DataType_eUnknown;     /* can't determine format type. */
        }
        else
        {
            pFormat->type = BAPE_DataType_ePcmStereo;
        }
    }

    BDBG_MSG(( "Updated format fields: "  BAPE_FMT_P_TO_PRINTF_ARGS(pFormat)));

    BDBG_LEAVE (BAPE_MaiInput_P_UpdateFormat_isr);
    return;
}

/***************************************************************************
    Define stub functions for when there are no MAI inputs.
***************************************************************************/
#else /* BAPE_CHIP_MAX_MAI_INPUTS > 0 */
    /* No MAI inputs, just use stubbed out functions. */

/**************************************************************************/
void BAPE_MaiInput_GetDefaultSettings(
    BAPE_MaiInputSettings *pSettings        /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_Open(
    BAPE_Handle hApe,
    unsigned index,
    const BAPE_MaiInputSettings *pSettings,
    BAPE_MaiInputHandle *phandle              /* [out] */
    )
{
    BSTD_UNUSED(hApe);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(phandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**************************************************************************/

void BAPE_MaiInput_Close(
    BAPE_MaiInputHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/**************************************************************************/

void BAPE_MaiInput_GetSettings(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputSettings *pSettings        /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_SetSettings(
    BAPE_MaiInputHandle handle,
    const BAPE_MaiInputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**************************************************************************/

void BAPE_MaiInput_GetInputPort(
    BAPE_MaiInputHandle handle,
    BAPE_InputPort *pPort
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pPort);
}

/**************************************************************************/

void BAPE_MaiInput_GetFormatDetectionSettings(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputFormatDetectionSettings *pSettings   /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_SetFormatDetectionSettings(
    BAPE_MaiInputHandle handle,
    const BAPE_MaiInputFormatDetectionSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_GetFormatDetectionStatus(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputFormatDetectionStatus *pStatus
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_P_PrepareForStandby(BAPE_Handle hApe)
{
    BSTD_UNUSED(hApe);
    return BERR_SUCCESS;
}

/**************************************************************************/

BERR_Code BAPE_MaiInput_P_ResumeFromStandby(BAPE_Handle hApe)
{
    BSTD_UNUSED(hApe);
    return BERR_SUCCESS;
}

#endif
