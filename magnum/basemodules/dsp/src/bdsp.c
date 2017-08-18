/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#include "bdsp.h"
#include "bdsp_priv.h"

BDBG_MODULE(bdsp);

void BDSP_Close(
    BDSP_Handle handle      /* Handle to be closed */
    )
{
    BDBG_OBJECT_ASSERT(handle, BDSP_Device);
    BDBG_ASSERT(NULL != handle->close);
    handle->close(handle->pDeviceHandle);
}

void BDSP_GetStatus(
    BDSP_Handle handle,
    BDSP_Status *pStatus             /* [out] Current Status */
    )
{

    BDBG_OBJECT_ASSERT(handle, BDSP_Device);

    if ( handle->getStatus )
    {
        handle->getStatus(handle->pDeviceHandle, pStatus);
    }
    else
    {
        BKNI_Memset(pStatus, 0, sizeof(BDSP_Status));
    }
    return;
}

BERR_Code BDSP_Standby(
    BDSP_Handle             handle,         /* [in] DSP device handle */
    BDSP_StandbySettings    *pSettings  /* [in] standby settings */
    )
{

    BDBG_OBJECT_ASSERT(handle, BDSP_Device);

    if ( handle->powerStandby)
    {
        return handle->powerStandby(handle->pDeviceHandle, pSettings);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_Resume(
    BDSP_Handle             handle      /* [in] DSP device handle */
    )
{

    BDBG_OBJECT_ASSERT(handle, BDSP_Device);

    if ( handle->powerResume)
    {
        return handle->powerResume(handle->pDeviceHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_GetAlgorithmInfo(
    BDSP_Handle handle,
    BDSP_Algorithm algorithm, /* [in] */
    BDSP_AlgorithmInfo *pInfo /* [out] */
    )
{

    BDBG_OBJECT_ASSERT(handle, BDSP_Device);

    if ( handle->getAlgorithmInfo )
    {
        handle->getAlgorithmInfo(algorithm, pInfo);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Allocate an external interrupt.

Description:
    This function allocates one interrupt handle that can be used to interrupt DSP.

Returns:
    BERR_SUCCESS - If allocation is successful, otherwise error

See Also:
    BDSP_FreeExternalInterrupt
    BDSP_GetExternalInterruptInfo
***************************************************************************/
BERR_Code BDSP_AllocateExternalInterrupt(
    BDSP_Handle hDsp,
    uint32_t    dspIndex,
    BDSP_ExternalInterruptHandle *pInterruptHandle /* [out] */
    )
{
    BERR_Code   ErrCode = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hDsp, BDSP_Device);

    if ( hDsp->allocateExternalInterrupt)
    {
        ErrCode = hDsp->allocateExternalInterrupt(hDsp->pDeviceHandle, dspIndex, pInterruptHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return ErrCode;
}


/***************************************************************************
Summary:
    Frees an allocated external interrupt.

Description:
    This function frees one interrupt handle that was already allocated.

Returns:
    BERR_SUCCESS if sucessful else error

See Also:
    BDSP_AllocateExternalInterrupt
    BDSP_GetExternalInterruptInfo
***************************************************************************/
BERR_Code BDSP_FreeExternalInterrupt(
            BDSP_ExternalInterruptHandle    hInterrupt
            )
{
    BDSP_Handle handle;
    BERR_Code   ErrCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hInterrupt, BDSP_ExternalInterrupt);

    handle = hInterrupt->hDsp;

    if(handle->freeExternalInterrupt)
    {
        ErrCode = handle->freeExternalInterrupt(hInterrupt->pExtInterruptHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return ErrCode;
}

/***************************************************************************
Summary:
    Retrieve external interrupt information.

Description:
    This function provides the interrupt register and the bit that can be used to interrupt DSP.

Returns:
    BERR_SUCCESS - If successful, otherwise error

See Also:
    BDSP_FreeExternalInterrupt
    BDSP_AllocateExternalInterrupt
***************************************************************************/
BERR_Code BDSP_GetExternalInterruptInfo(
    BDSP_ExternalInterruptHandle hInterrupt,
    BDSP_ExternalInterruptInfo **pInfo /* [out] */
    )
{
    BDSP_Handle handle;
    BERR_Code   ErrCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hInterrupt, BDSP_ExternalInterrupt);

    handle = hInterrupt->hDsp;

    if(handle->getExternalInterruptInfo)
    {
        ErrCode = handle->getExternalInterruptInfo(hInterrupt->pExtInterruptHandle, pInfo);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return ErrCode;

}

/***************************************************************************
Summary:
    Process audio capture data

Description:
    This function must be called in a background thread at a frequent
    interval (e.g. 10ms). for the data to be copied into the capture buffers.

Returns:
    BERR_SUCCESS - If audio capture is successful

See Also:
    BDSP_AudioCapture_GetBuffer
    BDSP_AudioCapture_ConsumeData
***************************************************************************/
BERR_Code BDSP_ProcessAudioCapture(
    BDSP_Handle hDsp
    )
{
    BERR_Code   ErrCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDsp, BDSP_Device);

    if(hDsp->processAudioCapture)
    {
        ErrCode = hDsp->processAudioCapture(hDsp->pDeviceHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return ErrCode;
}
