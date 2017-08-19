/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_frontend_customer_extension.h"
#include "nexus_frontend_ast.h"
#if NEXUS_FRONTEND_7346
#include "bast_g3.h"
#endif
#if NEXUS_FRONTEND_4538
#include "bast_4538.h"
#endif

BDBG_MODULE(nexus_frontend_directv);

struct NEXUS_FrontendCustomerData {
    struct {
        NEXUS_FrontendFtmSettings settings;
    } ftm;

    NEXUS_Frontend_DirecTV_FrontendSettings frontendSettings;
};

static NEXUS_Error NEXUS_Frontend_DirecTV_P_SetFtmTxPower( NEXUS_AstDevice *pDevice, uint8_t ftmTxPower )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (ftmTxPower > 0x1f) {
        return NEXUS_INVALID_PARAMETER;
    }
    
    switch (pDevice->astChip) {
#if NEXUS_FRONTEND_4506 
        case 4506: {
            if (BCM4506_CONFIG_LEN_FTM_TX_POWER > 1) {
                /* Just making sure message length didn't change unexpectedly */
                rc = NEXUS_UNKNOWN;
                break;
            }
            rc = BAST_WriteConfig(pDevice->astChannel, BCM4506_CONFIG_FTM_TX_POWER, &ftmTxPower, BCM4506_CONFIG_LEN_FTM_TX_POWER);
            break;
        }
#endif            
#if NEXUS_FRONTEND_7346
        case 7346: {
            if (BAST_G3_CONFIG_LEN_FTM_TX_POWER > 1) {
                /* Just making sure message length didn't change unexpectedly */
                rc = NEXUS_UNKNOWN;
                break;
            }
            rc = BAST_WriteConfig(pDevice->astChannel, BAST_G3_CONFIG_FTM_TX_POWER, &ftmTxPower, BAST_G3_CONFIG_LEN_FTM_TX_POWER);
            break;
        }
#endif
#if NEXUS_FRONTEND_73XX
        case 7342: {
            if (BCM73XX_CONFIG_LEN_FTM_TX_POWER > 1) {
                /* Just making sure message length didn't change unexpectedly */
                rc = NEXUS_UNKNOWN;
                break;
            }
            rc = BAST_WriteConfig(pDevice->astChannel, BCM73XX_CONFIG_FTM_TX_POWER, &ftmTxPower, BCM73XX_CONFIG_LEN_FTM_TX_POWER);
            break;
        }
#endif
#if NEXUS_FRONTEND_4538
        case 4538: {
            if (BAST_4538_CONFIG_LEN_FTM_TX_POWER > 1) {
                /* Just making sure message length didn't change unexpectedly */
                rc = NEXUS_UNKNOWN;
                break;
            }
            rc = BAST_WriteConfig(pDevice->astChannel, BAST_4538_CONFIG_FTM_TX_POWER, &ftmTxPower, BAST_4538_CONFIG_LEN_FTM_TX_POWER);
            break;
        }
#endif
        default:
            BDBG_ERR(("Unsupported chip: %d", pDevice->astChip));
            rc = NEXUS_INVALID_PARAMETER;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_DirecTV_P_GetFtmTxPower( NEXUS_AstDevice *pDevice, uint8_t *ftmTxPower )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    *ftmTxPower = 0;

    switch (pDevice->astChip) {
#if NEXUS_FRONTEND_4506 
        case 4506: {
            uint8_t buf[BCM4506_CONFIG_LEN_FTM_TX_POWER];
            rc = BAST_ReadConfig(pDevice->astChannel, BCM4506_CONFIG_FTM_TX_POWER, buf, BCM4506_CONFIG_LEN_FTM_TX_POWER);
            *ftmTxPower = buf[0] & 0x1f;
            break;
        }
#endif            
#if NEXUS_FRONTEND_7346
        case 7346: {
            uint8_t buf[BAST_G3_CONFIG_LEN_FTM_TX_POWER];
            rc = BAST_ReadConfig(pDevice->astChannel, BAST_G3_CONFIG_FTM_TX_POWER, buf, BAST_G3_CONFIG_LEN_FTM_TX_POWER);
            *ftmTxPower = buf[0] & 0x1f;
            break;
        }
#endif
#if NEXUS_FRONTEND_73XX
        case 7342: {
            uint8_t buf[BCM73XX_CONFIG_LEN_FTM_TX_POWER];
            rc = BAST_ReadConfig(pDevice->astChannel, BCM73XX_CONFIG_FTM_TX_POWER, buf, BCM73XX_CONFIG_LEN_FTM_TX_POWER);
            *ftmTxPower = buf[0] & 0x1f;
            break;
        }
#endif
#if NEXUS_FRONTEND_4538
        case 4538: {
            uint8_t buf[BAST_4538_CONFIG_LEN_FTM_TX_POWER];
            rc = BAST_ReadConfig(pDevice->astChannel, BAST_4538_CONFIG_FTM_TX_POWER, buf, BAST_4538_CONFIG_LEN_FTM_TX_POWER);
            *ftmTxPower = buf[0] & 0x1f;
            break;
        }
#endif
        default:
            BDBG_ERR(("Unsupported chip: %d", pDevice->astChip));
            rc = BERR_INVALID_PARAMETER;
    }

    return rc;
}

static struct NEXUS_FrontendCustomerData *NEXUS_Frontend_P_GetData(NEXUS_AstDevice *pDevice)
{
    struct NEXUS_FrontendCustomerData *pData;

    if (!pDevice->customerData) {
        uint8_t ftmTxPower;
        
        pDevice->customerData = BKNI_Malloc(sizeof(*pData));
        BKNI_Memset(pDevice->customerData, 0, sizeof(*pData));
        pData = pDevice->customerData;

        /* Fill our ftmTxPower variable with value from the frontend */
        if (NEXUS_Frontend_DirecTV_P_GetFtmTxPower(pDevice, &ftmTxPower) != NEXUS_SUCCESS) {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
        } else {
            pData->ftm.settings.ftmTxPower = ftmTxPower;
        }
    }
    return pDevice->customerData;
}

NEXUS_Error NEXUS_Frontend_DirecTV_SendFTMMessage( NEXUS_FrontendHandle handle, const uint8_t *pSendCommand, size_t sendCommmandSize )
{
    BERR_Code rc = 0;
    struct NEXUS_FrontendCustomerData *pData;
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(handle, handle->chip.id);
    if (!pDevice) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }
    pData = NEXUS_Frontend_P_GetData(pDevice);

    if (!pData->ftm.settings.enabled) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }

    rc = BAST_WriteFtm(pDevice->astHandle, (uint8_t*)pSendCommand, sendCommmandSize);
    if (rc) {
        rc = BERR_TRACE(rc);
    }

errComplete:
    return rc;
}

NEXUS_Error NEXUS_Frontend_DirecTV_ReceiveFTMMessage( NEXUS_FrontendHandle handle, uint8_t *pReceiveCommand, size_t receiveCommmandSize, size_t *pDataReceived )
{
    uint8_t n;
    BERR_Code rc = 0;
    struct NEXUS_FrontendCustomerData *pData;
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(handle, handle->chip.id);
    if (!pDevice) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }
    pData = NEXUS_Frontend_P_GetData(pDevice);

    if (!pData->ftm.settings.enabled) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }

    if (receiveCommmandSize < 128) {
        /* limit of 128 taken from (size&0x7f)+1 logic in bast_xxx.c files */
        BDBG_WRN(("NEXUS_Frontend_DirecTV_ReceiveFTMMessage may overflow a message buffer of %d bytes", receiveCommmandSize));
    }

    rc = BAST_ReadFtm(pDevice->astHandle, pReceiveCommand, &n);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto errComplete;
    }

    *pDataReceived = n;

errComplete:
    return rc;
}

NEXUS_Error NEXUS_Frontend_DirecTV_ResetFTM( NEXUS_FrontendHandle handle )
{
    BERR_Code rc = 0;
    struct NEXUS_FrontendCustomerData *pData;
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(handle, handle->chip.id);
    if (!pDevice) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }
    pData = NEXUS_Frontend_P_GetData(pDevice);

    if (!pData->ftm.settings.enabled) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }

    rc = BAST_ResetFtm(pDevice->astHandle);
    if (rc) {
        rc = BERR_TRACE(rc);
    }

errComplete:
    return rc;
}

void NEXUS_Frontend_DirecTV_GetFTMSettings( NEXUS_FrontendHandle handle, NEXUS_FrontendFtmSettings *pSettings )
{
    struct NEXUS_FrontendCustomerData *pData;
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(handle, handle->chip.id);
    if (!pDevice) {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
        goto errComplete;
    }
    pData = NEXUS_Frontend_P_GetData(pDevice);

    *pSettings = pData->ftm.settings;

errComplete:
    return;
}

NEXUS_Error NEXUS_Frontend_DirecTV_SetFTMSettings( NEXUS_FrontendHandle handle, const NEXUS_FrontendFtmSettings *pSettings )
{
    BERR_Code rc = 0;
    struct NEXUS_FrontendCustomerData *pData;
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(handle, handle->chip.id);
    if (!pDevice) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }
    pData = NEXUS_Frontend_P_GetData(pDevice);

    if (pSettings->enabled != pData->ftm.settings.enabled) {
        if (pSettings->enabled) {
            BAST_PowerUpFtm(pDevice->astHandle);
        } else {
            BAST_PowerDownFtm(pDevice->astHandle);
        }
        pData->ftm.settings.enabled = pSettings->enabled;
    }

    if (pSettings->ftmTxPower != pData->ftm.settings.ftmTxPower) {
        rc = NEXUS_Frontend_DirecTV_P_SetFtmTxPower(pDevice, pSettings->ftmTxPower);
        if (rc) {
            rc = BERR_TRACE(rc);
        } else {
            pData->ftm.settings.ftmTxPower = pSettings->ftmTxPower;
        }
    }

    if (pSettings->dataReady.callback == NULL) {
        NEXUS_TaskCallback_Set(pDevice->ftmCallback, NULL);
    } else {
        NEXUS_TaskCallback_Set(pDevice->ftmCallback, &pSettings->dataReady);
    }
    pData->ftm.settings.dataReady = pSettings->dataReady;

errComplete:    
    return rc;
}

void NEXUS_Frontend_P_UninitExtension( NEXUS_FrontendHandle handle )
{
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(handle, handle->chip.id);
    if (!pDevice) {
        return;
    }
    if (pDevice->ftmCallback) {
        /* User should have cleared the callback, but do it just in case */
        NEXUS_TaskCallback_Set(pDevice->ftmCallback, NULL);
    }
    if (pDevice->customerData) {
        BKNI_Free(pDevice->customerData);
        pDevice->customerData = NULL;
    }
}

NEXUS_Error NEXUS_Frontend_DirecTV_SetAmcScramblingSequence( NEXUS_FrontendHandle frontend, 
                                                             uint32_t xseed, 
                                                             uint32_t plhdrscr1, 
                                                             uint32_t plhdrscr2, 
                                                             uint32_t plhdrscr3 )
{
    BERR_Code rc = 0;
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(frontend, frontend->chip.id);
    if (!pDevice) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }

    rc = BAST_SetAmcScramblingSeq(pDevice->astChannel, xseed, plhdrscr1, plhdrscr2, plhdrscr3);
    if (rc) {
        rc = BERR_TRACE(rc);
    }

errComplete:
    return rc;
}

NEXUS_Error NEXUS_Frontend_DirecTV_SetFskChannel( NEXUS_FrontendHandle frontend, 
                                                  NEXUS_DirecTV_FskChannel fskTxChannel,
                                                  NEXUS_DirecTV_FskChannel fskRxChannel )
{
    BERR_Code rc = 0;
    /* There is only one FSK configuration for 4538 so no need to call BAST_SetFskChannel() */
#if NEXUS_FRONTEND_4538
    return rc;
#endif
    NEXUS_AstDevice *pDevice = NEXUS_Frontend_P_GetAstDeviceByChip(frontend, frontend->chip.id);
    if (!pDevice) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }

    BDBG_CASSERT(NEXUS_DirecTV_FskChannel_eCh1 == (NEXUS_DirecTV_FskChannel)BAST_FskChannel_e1);

#if !defined NEXUS_FRONTEND_4506 && !defined NEXUS_FRONTEND_73XX && !defined NEXUS_FRONTEND_7346
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif

    if (fskTxChannel >= NEXUS_DirecTV_FskChannel_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (fskRxChannel >= NEXUS_DirecTV_FskChannel_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    rc = BAST_SetFskChannel(pDevice->astHandle, fskTxChannel, fskRxChannel);

    if (rc) {
        rc = BERR_TRACE(rc);
    }

errComplete:
    return rc;
}

void
NEXUS_Frontend_DirecTV_GetDefaultSettings(NEXUS_FrontendHandle frontend, NEXUS_Frontend_DirecTV_FrontendSettings * pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error 
NEXUS_Frontend_DirecTV_SetSettings(NEXUS_FrontendHandle frontend, const NEXUS_Frontend_DirecTV_FrontendSettings * const pSettings)
{
    BERR_Code rc = 0;
    struct NEXUS_FrontendCustomerData *pData;
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDeviceByChip(frontend, frontend->chip.id);

    if (!astDevice){
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto errComplete;
    }

    pData = NEXUS_Frontend_P_GetData(astDevice);

#if BCHP_CHIP == 7346
    /* The following set of configuration settings are valid for the 7346 only */
    if (frontend->chip.id != 7346) {
        goto frontendNot7346;
    }

    if (pData->frontendSettings.bypassLnaGain != pSettings->bypassLnaGain) {
        uint8_t tuner_ctl;
        BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL);
        if (pSettings->bypassLnaGain) {
            tuner_ctl |= BAST_G3_CONFIG_TUNER_CTL_LNA_BYPASS;
        } else {
            tuner_ctl &= ~BAST_G3_CONFIG_TUNER_CTL_LNA_BYPASS;
        }
        BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL);
    }

    if (pSettings->daisyGain.override) {
        if (pData->frontendSettings.daisyGain.value != pSettings->daisyGain.value) {
            uint8_t gain = pSettings->daisyGain.value;
            BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_DAISY_GAIN, &gain, BAST_G3_CONFIG_LEN_TUNER_DAISY_GAIN);
        }
    }

    if ((pSettings->bbClipThreshold) || (pSettings->lnaClipThreshold)) {
        uint8_t value[BAST_G3_CONFIG_LEN_TUNER_AGC_THRESH];
        BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_THRESH, value, BAST_G3_CONFIG_LEN_TUNER_AGC_THRESH);
        if (pSettings->bbClipThreshold) {
            value[0] = (pSettings->bbClipThreshold >> 8) & 0xFF;
            value[1] = (pSettings->bbClipThreshold     ) & 0xFF;
        }
        if (pSettings->lnaClipThreshold) {
            value[2] = (pSettings->lnaClipThreshold >> 8) & 0xFF;
            value[3] = (pSettings->lnaClipThreshold     ) & 0xFF;
        }
        BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_THRESH, value, BAST_G3_CONFIG_LEN_TUNER_AGC_THRESH);
    }

    if ((pSettings->bbWindow) || (pSettings->lnaWindow)) {
        uint8_t value[BAST_G3_CONFIG_LEN_TUNER_AGC_WIN_LEN];
        BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_WIN_LEN, value, BAST_G3_CONFIG_LEN_TUNER_AGC_WIN_LEN);
        if (pSettings->bbWindow) {
            value[0] = (pSettings->bbWindow >> 8) & 0xFF;
            value[1] = (pSettings->bbWindow     ) & 0xFF;
        }
        if (pSettings->lnaWindow) {
            value[2] = (pSettings->lnaWindow >> 8) & 0xFF;
            value[3] = (pSettings->lnaWindow     ) & 0xFF;
        }
        BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_WIN_LEN, value, BAST_G3_CONFIG_LEN_TUNER_AGC_WIN_LEN);
    }

    if ((pSettings->bbAmpThreshold) || (pSettings->lnaAmpThreshold)) {
        uint8_t value[BAST_G3_CONFIG_LEN_TUNER_AGC_AMP_THRESH];
        BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_AMP_THRESH, value, BAST_G3_CONFIG_LEN_TUNER_AGC_AMP_THRESH);
        if (pSettings->bbAmpThreshold) {
            value[0] = (pSettings->bbAmpThreshold) & 0xFF;
        }
        if (pSettings->lnaAmpThreshold) {
            value[1] = (pSettings->lnaAmpThreshold) & 0xFF;
        }
        BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_AMP_THRESH, value, BAST_G3_CONFIG_LEN_TUNER_AGC_AMP_THRESH);
    }

    if ((pSettings->bbLoopCoeff) || (pSettings->lnaLoopCoeff)) {
        uint8_t value[BAST_G3_CONFIG_LEN_TUNER_AGC_LOOP_COEFF];
        BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_LOOP_COEFF, value, BAST_G3_CONFIG_LEN_TUNER_AGC_LOOP_COEFF);
        if (pSettings->bbLoopCoeff) {
            value[0] = (pSettings->bbLoopCoeff) & 0xFF;
        }
        if (pSettings->lnaLoopCoeff) {
            value[1] = (pSettings->lnaLoopCoeff) & 0xFF;
        }
        BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_AGC_LOOP_COEFF, value, BAST_G3_CONFIG_LEN_TUNER_AGC_LOOP_COEFF);
    }
frontendNot7346:
#endif /* BCHP_CHIP == 7346 */

    pData->frontendSettings = *pSettings;
errComplete:
    return rc;
}



