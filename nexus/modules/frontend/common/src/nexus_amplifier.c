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
 ***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_amplifier);

NEXUS_AmplifierHandle NEXUS_Amplifier_OpenStub( unsigned index )
{
    BSTD_UNUSED(index);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
	Get the settings of a frontend amplifier
See Also:
    NEXUS_Amplifier_SetSettings
 ***************************************************************************/
void NEXUS_Amplifier_GetSettings(
    NEXUS_AmplifierHandle handle,
    NEXUS_AmplifierSettings *pSettings  /* [out] */
    )
{
#if NEXUS_AMPLIFIER_SUPPORT
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif
}

/***************************************************************************
Summary:
	Set the settings of a frontend amplifier
See Also:
    NEXUS_Amplifier_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_Amplifier_SetSettings(
    NEXUS_AmplifierHandle handle,
    const NEXUS_AmplifierSettings *pSettings
    )
{
#if NEXUS_AMPLIFIER_SUPPORT
    BERR_Code errCode=BERR_SUCCESS;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

	if (pSettings->enabled != handle->settings.enabled) 
	{
		if (pSettings->enabled)
		{
			errCode = BLNA_DisablePowerSaver(handle->lnaHandle);
			if ( errCode )
			{
				return BERR_TRACE(errCode);
			}
		}
		else
		{
			errCode = BLNA_EnablePowerSaver(handle->lnaHandle);
			if ( errCode )
			{
				return BERR_TRACE(errCode);
			}
		}
	}

	if (pSettings->gainMode != handle->settings.gainMode)
	{
		switch ( pSettings->gainMode )
		{
		case NEXUS_AmplifierGainMode_eAutomatic:
			errCode = BLNA_EnableAutomaticGainControl(handle->lnaHandle,
													  pSettings->gainSettings.automatic.outputLevel,
													  pSettings->gainSettings.automatic.deltaValue);
			if ( errCode )
			{
				return BERR_TRACE(errCode);
			}
			break;
		case NEXUS_AmplifierGainMode_eManual:
			errCode = BLNA_EnableManualGainControl(handle->lnaHandle,
												   pSettings->gainSettings.manual.gain);
			if ( errCode )
			{
				return BERR_TRACE(errCode);
			}
			break;
		default:
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
	}
    handle->settings = *pSettings;
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************
Summary:
	Enable the external output for an amplifier port
See Also:
    
 ***************************************************************************/
NEXUS_Error NEXUS_Amplifier_EnableExternalDriver(
    NEXUS_AmplifierHandle handle,
    unsigned portNum,                       /* The port number you want to enable/disable */
    bool enabled                            /* If true, output will be enabled.  If false, it will be disabled */
    )
{
#if NEXUS_AMPLIFIER_SUPPORT
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != handle->lnaHandle);

    if ( enabled )
    {
        return BLNA_EnableExternalDriver(handle->lnaHandle, portNum);
    }
    else
    {
        return BLNA_DisableExternalDriver(handle->lnaHandle, portNum);
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(portNum);
    BSTD_UNUSED(enabled);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************
Summary:
	Set the amplifier mode for an InBand port
See Also:
    NEXUS_Amplifier_SetOutOfBandMode
 ***************************************************************************/
NEXUS_Error NEXUS_Amplifier_SetInBandMode(
    NEXUS_AmplifierHandle handle,
    unsigned inBandPortNum,
    NEXUS_AmplifierInBandMode inBandMode
    )
{
#if NEXUS_AMPLIFIER_SUPPORT
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != handle->lnaHandle);
    BDBG_ASSERT(inBandMode < NEXUS_AmplifierInBandMode_eMax);

    return BLNA_SetInBandMode(handle->lnaHandle, inBandPortNum,
                              (BLNA_InBandMode)inBandMode);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(inBandPortNum);
    BSTD_UNUSED(inBandMode);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************
Summary:
	Set the amplifier mode for an OutOfBand port
See Also:
    NEXUS_Amplifier_SetInBandMode
 ***************************************************************************/
NEXUS_Error NEXUS_Amplifier_SetOutOfBandMode(
    NEXUS_AmplifierHandle handle,
    unsigned outOfBandPortNum,
    NEXUS_AmplifierOutOfBandMode outOfBandMode
    )
{
#if NEXUS_AMPLIFIER_SUPPORT
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != handle->lnaHandle);
    BDBG_ASSERT(outOfBandMode < NEXUS_AmplifierOutOfBandMode_eMax);
    
    return BLNA_SetOutOfBandMode(handle->lnaHandle, outOfBandPortNum,
                                 (BLNA_OutOfBandMode)outOfBandMode);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(outOfBandPortNum);
    BSTD_UNUSED(outOfBandMode);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

/***************************************************************************
Summary:
    Close an amplifier handle
***************************************************************************/
static void NEXUS_Amplifier_P_Finalizer(
    NEXUS_AmplifierHandle handle
    )
{
#if NEXUS_AMPLIFIER_SUPPORT
    NEXUS_OBJECT_ASSERT(NEXUS_Amplifier, handle);
    BLNA_Close(handle->lnaHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_Amplifier, handle);
    BKNI_Free(handle);
#else
    BSTD_UNUSED(handle);
#endif
}
NEXUS_OBJECT_CLASS_MAKE(NEXUS_Amplifier, NEXUS_Amplifier_Close);

/***************************************************************************
Summary:
    Get the status of a frontend amplifier
***************************************************************************/
NEXUS_Error NEXUS_Amplifier_GetStatus(
    NEXUS_AmplifierHandle handle,
    NEXUS_AmplifierStatus *pStatus  /* [out] */
    )
{
#if NEXUS_AMPLIFIER_SUPPORT
    BLNA_Status lnaStatus;
    BERR_Code errCode=BERR_SUCCESS;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NEXUS_AMPLIFIER_NUM_INBAND_PORTS<=BLNA_MX_INBAND_PORTS);
    BDBG_ASSERT(NEXUS_AMPLIFIER_NUM_OUTOFBAND_PORTS<=BLNA_MX_OUTOFBAND_PORTS);
    BDBG_ASSERT(NEXUS_AMPLIFIER_NUM_EXTERNAL_DRIVER <=BLNA_MX_EXTDRV);
    BDBG_CASSERT(NEXUS_AmplifierInBandMode_eMax == (NEXUS_AmplifierInBandMode)BQOB_InBandMode_eLast);
    BDBG_CASSERT(NEXUS_AmplifierOutOfBandMode_eMax == (NEXUS_AmplifierOutOfBandMode)BQOB_OutOfBandMode_eLast);

    errCode = BLNA_GetStatus(handle->lnaHandle,&lnaStatus);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    else
    {
        unsigned i;
        pStatus->gainMode = lnaStatus.gainMode;
        BDBG_MSG(("gainMode = %u",pStatus->gainMode));
        for (i=0;i<NEXUS_AMPLIFIER_NUM_INBAND_PORTS;i++)
        {
            pStatus->inBandPortMode[i] = (NEXUS_AmplifierInBandMode)lnaStatus.inBandPorts[i];
            BDBG_MSG(("inBandPortMode[%u] = %u",i,pStatus->inBandPortMode[i]));
        }
        for (i=0;i<NEXUS_AMPLIFIER_NUM_OUTOFBAND_PORTS;i++)
        {
            pStatus->outOfBandPortMode[i] = (NEXUS_AmplifierOutOfBandMode)lnaStatus.outOfBandPorts[i];
            BDBG_MSG(("outOfBandPortMode[%u] = %u",i,pStatus->outOfBandPortMode[i]));
        }
        for (i=0;i<NEXUS_AMPLIFIER_NUM_EXTERNAL_DRIVER ;i++)
        {
            pStatus->isExternalDriverEnabled[i] = lnaStatus.isExternalDriverEnabled[i];
            BDBG_MSG(("isExternalDriverEnabled[%u] = %u",i,pStatus->isExternalDriverEnabled[i]));
        }
        pStatus->agcOutputLevel /* units 0.1 dB */ = 2 * lnaStatus.agcOutputLevel /* units 0.2 dB */;
        BDBG_MSG(("agcOutputLevel = %u",pStatus->agcOutputLevel));
        pStatus->agcDeltaVal /* units 0.1 dB */  = 2 * lnaStatus.agcDeltaVal /* units 0.2 dB */;
        BDBG_MSG(("agcDeltaVal = %u",pStatus->agcDeltaVal));
        pStatus->manualGainVal /* units 0.1 dB */  = 2 * lnaStatus.manualGainVal /* units 0.2 dB */;
        BDBG_MSG(("manualGainVal = %u",pStatus->manualGainVal));
        pStatus->gainBoostEnabled = lnaStatus.gainBoostEnabled;
        BDBG_MSG(("gainBoostEnabled = %u",pStatus->gainBoostEnabled));
        pStatus->superBoostEnabled = lnaStatus.superBoostEnabled;
        BDBG_MSG(("superBoostEnabled = %u",pStatus->superBoostEnabled));
        pStatus->tiltEnabled = lnaStatus.tiltEnabled;
        BDBG_MSG(("tiltEnabled = %u",pStatus->tiltEnabled));

        errCode = BLNA_GetLnaAgcRegVal(handle->lnaHandle,0,&pStatus->agcGain);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return NEXUS_NOT_SUPPORTED;
#endif
}


