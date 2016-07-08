/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 *****************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_frontend_ast.h"
#include "nexus_i2c.h"
#include "priv/nexus_i2c_priv.h"
#include "breg_i2c_priv.h"
#include "bi2c.h" /* status error codes */

BDBG_MODULE(nexus_frontend_ast);
BDBG_OBJECT_ID(NEXUS_AstDevice);

BTRC_MODULE_DECLARE(ChnChange_Tune);
BTRC_MODULE_DECLARE(ChnChange_TuneLock);

#if defined NEXUS_FRONTEND_73XX
#define PEAK_SCAN_SYM_RATE_MIN      BAST_G2_CONFIG_PEAK_SCAN_SYM_RATE_MIN
#define LEN_PEAK_SCAN_SYM_RATE_MIN  BAST_G2_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MIN
#define PEAK_SCAN_SYM_RATE_MAX      BAST_G2_CONFIG_PEAK_SCAN_SYM_RATE_MAX
#define LEN_PEAK_SCAN_SYM_RATE_MAX  BAST_G2_CONFIG_LEN_PEAK_SCAN_SYM_RATE_MAX
#endif

/***************************************************************************
Frontend Callback Prototypes
***************************************************************************/
NEXUS_Error NEXUS_Frontend_P_Ast_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
void NEXUS_Frontend_P_Ast_Untune(void *handle);

static NEXUS_Error NEXUS_Frontend_P_Ast_GetSatelliteStatus(void *handle, NEXUS_FrontendSatelliteStatus *pStatus);
static void NEXUS_Frontend_P_Ast_GetDiseqcSettings(void *handle, NEXUS_FrontendDiseqcSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Ast_SetDiseqcSettings(void *handle, const NEXUS_FrontendDiseqcSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Ast_GetDiseqcStatus( void *handle, NEXUS_FrontendDiseqcStatus *pStatus );
static NEXUS_Error NEXUS_Frontend_P_Ast_SendDiseqcMessage(void *handle, const uint8_t *pSendData, size_t sendDataSize, const NEXUS_CallbackDesc *pSendComplete);
static NEXUS_Error NEXUS_Frontend_P_Ast_GetDiseqcReply(void *handle, NEXUS_FrontendDiseqcMessageStatus *pStatus, uint8_t *pReplyBuffer, size_t pReplyBufferSize, size_t *pReplyLength);
static NEXUS_Error NEXUS_Frontend_P_Ast_ResetDiseqc(void *handle, uint8_t options);
static NEXUS_Error NEXUS_Frontend_P_Ast_SendDiseqcAcw(void *handle, uint8_t codeWord);
static void NEXUS_Frontend_P_Ast_ResetStatus(void *handle);
static void NEXUS_Frontend_P_Ast_Close(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_Ast_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length);
static void NEXUS_Frontend_P_Ast_LockEventHandler(void *pParam);
static void NEXUS_Frontend_P_Ast_DiseqcEventHandler(void *pParam);
static void NEXUS_Frontend_P_Ast_PeakscanEventHandler(void *pParam);
static NEXUS_Error NEXUS_Frontend_P_Ast_ReadSatelliteConfig( void *handle, unsigned id, void *buffer, unsigned bufferSize );
static NEXUS_Error NEXUS_Frontend_P_Ast_WriteSatelliteConfig( void *handle, unsigned id, const void *buffer, unsigned bufferSize );
static NEXUS_Error NEXUS_Frontend_P_Ast_SatellitePeakscan( void *handle, const NEXUS_FrontendSatellitePeakscanSettings *pSettings );
static NEXUS_Error NEXUS_Frontend_P_Ast_SatellitePeakscanPsd( void *handle, const NEXUS_FrontendSatellitePeakscanSettings *pSettings );
static NEXUS_Error NEXUS_Frontend_P_Ast_GetSatellitePeakscanResult( void *handle, NEXUS_FrontendSatellitePeakscanResult *pResult );
static NEXUS_Error NEXUS_Frontend_P_Ast_SatelliteToneSearch( void *handle, const NEXUS_FrontendSatelliteToneSearchSettings *pSettings );
static NEXUS_Error NEXUS_Frontend_P_Ast_GetSatelliteToneSearchResult( void *handle, NEXUS_FrontendSatelliteToneSearchResult *pResult );
static NEXUS_Error NEXUS_Frontend_P_Ast_RegisterExtension(NEXUS_FrontendHandle parentHandle, NEXUS_FrontendHandle extensionHandle);
static BERR_Code NEXUS_Frontend_P_Ast_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_Ast_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
static NEXUS_Error NEXUS_Frontend_P_Ast_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus );
static NEXUS_Error NEXUS_Frontend_P_Ast_GetSignalDetectStatus (void *handle, NEXUS_FrontendSatelliteSignalDetectStatus *pStatus);
static NEXUS_Error NEXUS_Frontend_P_Ast_ReadRegister(void *handle, unsigned address, uint32_t *pValue);
static NEXUS_Error NEXUS_Frontend_P_Ast_WriteRegister(void *handle, unsigned address, uint32_t value);
static void NEXUS_Frontend_P_Ast_GetType(void *handle, NEXUS_FrontendType *type);
static NEXUS_Error NEXUS_Frontend_P_Ast_GetBertStatus( void *handle, NEXUS_FrontendBertStatus *pStatus );


#ifdef TEST_NETWORK_TUNER
NEXUS_AstDevice *pAstDevice[3]; /* This allows us to test the network tuner */
#endif

/***************************************************************************
NEXUS -> PI Conversion Routines
***************************************************************************/
typedef struct NEXUS_AstModeEntry
{
    const NEXUS_FrontendSatelliteCodeRate *pCodeRate;
    BAST_Mode mode;
} NEXUS_AstModeEntry;

static const NEXUS_FrontendSatelliteCodeRate
    g_cr_scan = {0,0,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_1_4 = {1,4,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_1_2 = {1,2,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_2_3 = {2,3,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_3_4 = {3,4,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_5_6 = {5,6,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_6_7 = {6,7,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_7_8 = {7,8,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_5_11 = {5,11,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_3_5 = {3,5,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_4_5 = {4,5,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_9_10 = {9,10,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0},
    g_cr_8_9 = {8,9,NEXUS_FrontendSatelliteCodeRateMode_eStandard,0};

static const struct NEXUS_AstModeEntry
g_sds_dvb_modes[] = {
    {&g_cr_1_2, BAST_Mode_eDvb_1_2},
    {&g_cr_2_3, BAST_Mode_eDvb_2_3},
    {&g_cr_3_4, BAST_Mode_eDvb_3_4},
    {&g_cr_5_6, BAST_Mode_eDvb_5_6},
    {&g_cr_7_8, BAST_Mode_eDvb_7_8},
    {NULL, BAST_Mode_eDvb_scan}
},
g_sds_dss_modes[] = {
    {&g_cr_1_2, BAST_Mode_eDss_1_2},
    {&g_cr_2_3, BAST_Mode_eDss_2_3},
    {&g_cr_6_7, BAST_Mode_eDss_6_7},
    {NULL, BAST_Mode_eDss_scan}
},
g_sds_dcii_modes[] = {
    {&g_cr_1_2, BAST_Mode_eDcii_1_2},
    {&g_cr_2_3, BAST_Mode_eDcii_2_3},
    {&g_cr_3_5, BAST_Mode_eDcii_3_5},
    {&g_cr_3_4, BAST_Mode_eDcii_3_4},
    {&g_cr_4_5, BAST_Mode_eDcii_4_5},
    {&g_cr_5_6, BAST_Mode_eDcii_5_6},
    {&g_cr_7_8, BAST_Mode_eDcii_7_8},
    {&g_cr_5_11, BAST_Mode_eDcii_5_11},
    {NULL, BAST_Mode_eDcii_scan}
},
g_sds_qpsk_turbo_modes[] = {
    {&g_cr_1_2, BAST_Mode_eTurbo_Qpsk_1_2},
    {&g_cr_2_3, BAST_Mode_eTurbo_Qpsk_2_3},
    {&g_cr_3_4, BAST_Mode_eTurbo_Qpsk_3_4},
    {&g_cr_5_6, BAST_Mode_eTurbo_Qpsk_5_6},
    {&g_cr_7_8, BAST_Mode_eTurbo_Qpsk_7_8},
    {NULL, BAST_Mode_eTurbo_scan}
},
g_sds_8psk_turbo_modes[] = {
    {&g_cr_2_3, BAST_Mode_eTurbo_8psk_2_3},
    {&g_cr_3_4, BAST_Mode_eTurbo_8psk_3_4},
    {&g_cr_4_5, BAST_Mode_eTurbo_8psk_4_5},
    {&g_cr_5_6, BAST_Mode_eTurbo_8psk_5_6},
    {&g_cr_8_9, BAST_Mode_eTurbo_8psk_8_9},
    {NULL, BAST_Mode_eTurbo_scan}
},
g_sds_turbo_modes[] = {
    {&g_cr_1_2, BAST_Mode_eTurbo_Qpsk_1_2},
    {&g_cr_2_3, BAST_Mode_eTurbo_Qpsk_2_3},
    {&g_cr_3_4, BAST_Mode_eTurbo_Qpsk_3_4},
    {&g_cr_5_6, BAST_Mode_eTurbo_Qpsk_5_6},
    {&g_cr_7_8, BAST_Mode_eTurbo_Qpsk_7_8},
    {&g_cr_2_3, BAST_Mode_eTurbo_8psk_2_3},
    {&g_cr_3_4, BAST_Mode_eTurbo_8psk_3_4},
    {&g_cr_4_5, BAST_Mode_eTurbo_8psk_4_5},
    {&g_cr_5_6, BAST_Mode_eTurbo_8psk_5_6},
    {&g_cr_8_9, BAST_Mode_eTurbo_8psk_8_9},
    {NULL, BAST_Mode_eTurbo_scan}
},
g_sds_8psk_ldpc_modes[] = {
    {&g_cr_3_5, BAST_Mode_eLdpc_8psk_3_5},
    {&g_cr_2_3, BAST_Mode_eLdpc_8psk_2_3},
    {&g_cr_3_4, BAST_Mode_eLdpc_8psk_3_4},
    {&g_cr_5_6, BAST_Mode_eLdpc_8psk_5_6},
    {&g_cr_8_9, BAST_Mode_eLdpc_8psk_8_9},
    {&g_cr_9_10, BAST_Mode_eLdpc_8psk_9_10},
    {NULL, BAST_Mode_eLdpc_scan}
},
g_sds_qpsk_ldpc_modes[] = {
    {&g_cr_1_2, BAST_Mode_eLdpc_Qpsk_1_2},
    {&g_cr_3_5, BAST_Mode_eLdpc_Qpsk_3_5},
    {&g_cr_2_3, BAST_Mode_eLdpc_Qpsk_2_3},
    {&g_cr_3_4, BAST_Mode_eLdpc_Qpsk_3_4},
    {&g_cr_4_5, BAST_Mode_eLdpc_Qpsk_4_5},
    {&g_cr_5_6, BAST_Mode_eLdpc_Qpsk_5_6},
    {&g_cr_8_9, BAST_Mode_eLdpc_Qpsk_8_9},
    {&g_cr_9_10, BAST_Mode_eLdpc_Qpsk_9_10},
    {NULL, BAST_Mode_eLdpc_scan}
},
g_sds_ldpc_modes[] = {
    {&g_cr_1_2, BAST_Mode_eLdpc_Qpsk_1_2},
    {&g_cr_3_5, BAST_Mode_eLdpc_Qpsk_3_5},
    {&g_cr_2_3, BAST_Mode_eLdpc_Qpsk_2_3},
    {&g_cr_3_4, BAST_Mode_eLdpc_Qpsk_3_4},
    {&g_cr_4_5, BAST_Mode_eLdpc_Qpsk_4_5},
    {&g_cr_5_6, BAST_Mode_eLdpc_Qpsk_5_6},
    {&g_cr_8_9, BAST_Mode_eLdpc_Qpsk_8_9},
    {&g_cr_9_10, BAST_Mode_eLdpc_Qpsk_9_10},
    {&g_cr_3_5, BAST_Mode_eLdpc_8psk_3_5},
    {&g_cr_2_3, BAST_Mode_eLdpc_8psk_2_3},
    {&g_cr_3_4, BAST_Mode_eLdpc_8psk_3_4},
    {&g_cr_5_6, BAST_Mode_eLdpc_8psk_5_6},
    {&g_cr_8_9, BAST_Mode_eLdpc_8psk_8_9},
    {&g_cr_9_10, BAST_Mode_eLdpc_8psk_9_10},
    {NULL, BAST_Mode_eLdpc_scan}
},
g_blind_acquisition_mode[] = {
    {NULL, BAST_Mode_eBlindScan}
};

static BAST_Mode NEXUS_Frontend_P_Ast_GetMode(const struct NEXUS_AstModeEntry *pModes, const NEXUS_FrontendSatelliteCodeRate *pCodeRate)
{
    /* NOTE: there are not non-zero bitsPerSymbol values in the look up tables. if someone specifies a non-zero bitsPerSymbol,
    this algorithm will result in a scan. */
    for ( ;; pModes++ )
    {
        if ( NULL == pModes->pCodeRate )
        {
            BDBG_MSG(("Use scan for coderate %d:%d:%d", pCodeRate->numerator, pCodeRate->denominator, pCodeRate->bitsPerSymbol));
            return pModes->mode;
        }
        if ( pModes->pCodeRate->numerator == pCodeRate->numerator &&
             pModes->pCodeRate->denominator == pCodeRate->denominator &&
             pModes->pCodeRate->bitsPerSymbol == pCodeRate->bitsPerSymbol )
        {
            return pModes->mode;
        }
    }
}

void NEXUS_Frontend_P_Ast_GetDefaultSettings( NEXUS_FrontendAstSettings *pSettings )
{
    unsigned i;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    /* by default, everything is supported. specific chips should turn off what they don't support. */
    pSettings->capabilities.satellite = true;
    pSettings->capabilities.diseqc = true;
    for (i = 0;i < NEXUS_FrontendSatelliteMode_eMax; i++)
    {
        pSettings->capabilities.satelliteModes[i] = true;
    }
}

NEXUS_Error NEXUS_Frontend_P_Ast_RegisterEvents(NEXUS_AstDevice *pChannel)
{
    BERR_Code errCode;
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!pChannel->lockEvent) {
        errCode = BAST_GetLockStateChangeEventHandle(pChannel->astChannel, &pChannel->lockEvent);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            rc = NEXUS_UNKNOWN;
        }
    }

    if (!pChannel->lockEventCallback) {
        pChannel->lockEventCallback = NEXUS_RegisterEvent(pChannel->lockEvent,
                                                         NEXUS_Frontend_P_Ast_LockEventHandler,
                                                         pChannel);
        if ( NULL == pChannel->lockEventCallback )
        {
            errCode = BERR_TRACE(BERR_OS_ERROR);
            rc = NEXUS_UNKNOWN;
        }
    }
    if (!pChannel->lockAppCallback) {
        pChannel->lockAppCallback = NEXUS_TaskCallback_Create(pChannel->frontendHandle, NULL);
        if ( NULL == pChannel->lockAppCallback )
        {
            errCode = BERR_TRACE(BERR_OS_ERROR);
            rc = NEXUS_UNKNOWN;
        }
    }

    if (!pChannel->peakscanEvent) {
        errCode = BAST_GetPeakScanEventHandle(pChannel->astChannel, &pChannel->peakscanEvent);
        if ( errCode == BERR_NOT_SUPPORTED) {
            pChannel->peakscanEvent = NULL; /* used to determine whether peakscan is available */
        }
        else {
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                rc = NEXUS_UNKNOWN;
            }

            if (!pChannel->peakscanEventCallback) {
                pChannel->peakscanEventCallback = NEXUS_RegisterEvent(pChannel->peakscanEvent,
                                                                     NEXUS_Frontend_P_Ast_PeakscanEventHandler,
                                                                     pChannel);
                if ( NULL == pChannel->peakscanEventCallback )
                {
                    errCode = BERR_TRACE(BERR_OS_ERROR);
                    rc = NEXUS_UNKNOWN;
                }
            }
        }
    }

    if (pChannel->peakscanEvent) {
        if (!pChannel->peakscanAppCallback) {
            pChannel->peakscanAppCallback = NEXUS_TaskCallback_Create(pChannel->frontendHandle, NULL);
            if ( NULL == pChannel->peakscanAppCallback )
            {
                errCode = BERR_TRACE(BERR_OS_ERROR);
                rc = NEXUS_UNKNOWN;
            }
        }
    }

    if (!pChannel->ftmCallback) {
        pChannel->ftmCallback = NEXUS_TaskCallback_Create(pChannel->frontendHandle, NULL);
        if ( NULL == pChannel->ftmCallback )
        {
            errCode = BERR_TRACE(BERR_OS_ERROR);
            rc = NEXUS_UNKNOWN;
        }
    }

    if (pChannel->capabilities.diseqc)
    {
        bool deviceManagesDiseqc = false;
        if (pChannel->getDiseqcChannelHandle) {
            deviceManagesDiseqc = true;
        }
        if (!pChannel->diseqcAppCallback) {
            pChannel->diseqcAppCallback = NEXUS_TaskCallback_Create(pChannel->frontendHandle, NULL);
            if ( NULL == pChannel->diseqcAppCallback )
            {
                errCode = BERR_TRACE(BERR_OS_ERROR);
                rc = NEXUS_UNKNOWN;
            }
            if (deviceManagesDiseqc) {
                pChannel->setDiseqcAppCallback(pChannel->getDiseqcChannelHandleParam, pChannel->diseqcIndex, pChannel->diseqcAppCallback);
            }
        }
    }

    return rc;
}

NEXUS_Error NEXUS_Frontend_P_Ast_UnregisterEvents(NEXUS_AstDevice *pChannel)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_MSG(("NEXUS_Frontend_P_Ast_UnregisterEvents(%p)",(void*)pChannel));

    if (pChannel->lockAppCallback) {
        NEXUS_TaskCallback_Destroy(pChannel->lockAppCallback);
        pChannel->lockAppCallback = NULL;
    }

    pChannel->lockEvent = NULL;
    if (pChannel->lockEventCallback) {
        NEXUS_UnregisterEvent(pChannel->lockEventCallback);
        pChannel->lockEventCallback = NULL;
    }
    if (pChannel->lockAppCallback) {
        NEXUS_TaskCallback_Destroy(pChannel->lockAppCallback);
        pChannel->lockAppCallback = NULL;
    }

    if (pChannel->peakscanAppCallback) {
        NEXUS_TaskCallback_Destroy(pChannel->peakscanAppCallback);
        pChannel->peakscanAppCallback = NULL;
    }
    if (pChannel->peakscanEventCallback) {
        NEXUS_UnregisterEvent(pChannel->peakscanEventCallback);
        pChannel->peakscanEventCallback = NULL;
    }
    pChannel->peakscanEvent = NULL;

    if (pChannel->diseqcAppCallback) {
        NEXUS_TaskCallback_Destroy(pChannel->diseqcAppCallback);
        pChannel->diseqcAppCallback = NULL;
    }

    if (pChannel->ftmCallback) {
        NEXUS_TaskCallback_Destroy(pChannel->ftmCallback);
        pChannel->ftmCallback = NULL;
    }

    return rc;
}

NEXUS_Error NEXUS_Frontend_P_Ast_DelayedInitialization(NEXUS_FrontendHandle frontend)
{
    NEXUS_AstDevice *pDevice = frontend->pDeviceHandle;
    NEXUS_FrontendAstSettings *pSettings = &pDevice->settings;
    BAST_ChannelHandle dsqChannel;
    bool deviceManagesDiseqc = false;
    NEXUS_Error errCode;

    if (pSettings->getDiseqcChannelHandle) {
        deviceManagesDiseqc = true;
        pDevice->getDiseqcChannelHandle = pSettings->getDiseqcChannelHandle;
        pDevice->getDiseqcChannelHandleParam = pSettings->getDiseqcChannelHandleParam;
        pDevice->getDiseqcEventHandle = pSettings->getDiseqcEventHandle;
        pDevice->getDiseqcAppCallback = pSettings->getDiseqcAppCallback;
        pDevice->setDiseqcAppCallback = pSettings->setDiseqcAppCallback;
        dsqChannel = pSettings->getDiseqcChannelHandle(pSettings->getDiseqcChannelHandleParam, pSettings->diseqcIndex);
    } else {
        dsqChannel = pDevice->astChannel;
    }

    errCode = BAST_GetLockStateChangeEventHandle(pDevice->astChannel, &pDevice->lockEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_lock_event;
    }

    if (pSettings->capabilities.diseqc)
    {
        (void)BAST_ResetDiseqc(dsqChannel, 0);
    }

    pDevice->lockEventCallback = NEXUS_RegisterEvent(pDevice->lockEvent,
                                                     NEXUS_Frontend_P_Ast_LockEventHandler,
                                                     pDevice);
    if ( NULL == pDevice->lockEventCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_lock_event_callback;
    }

#if TEST_NETWORK_TUNER
    pAstDevice[pDevice->settings.channelIndex]=pDevice;
#endif

    /* reset diseqc with options 0. Currently this parameter is not used by the AST PI */
    if (pSettings->capabilities.diseqc)
    {
        if (deviceManagesDiseqc) {
            pDevice->diseqcEventCallback = pSettings->getDiseqcEventHandle(pSettings->getDiseqcChannelHandleParam, pSettings->diseqcIndex);
        } else {
            errCode = BAST_GetDiseqcEventHandle(dsqChannel, &pDevice->diseqcEvent);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_diseqc_event;
            }

            pDevice->diseqcEventCallback = NEXUS_RegisterEvent(pDevice->diseqcEvent,
                                                               NEXUS_Frontend_P_Ast_DiseqcEventHandler,
                                                               pDevice);
            if ( NULL == pDevice->diseqcEventCallback )
            {
                errCode = BERR_TRACE(errCode);
                goto err_diseqc_event_callback;
            }
        }

        BKNI_Memset(&pDevice->diseqcSettings,0,sizeof(pDevice->diseqcSettings));
        pDevice->diseqcSettings.enabled = true;
    }

    errCode = BAST_GetPeakScanEventHandle(pDevice->astChannel, &pDevice->peakscanEvent);
    if ( errCode == BERR_NOT_SUPPORTED) {
        pDevice->peakscanEvent = NULL; /* used to determine whether peakscan is available */
    }
    else {
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_peak_scan_event;
        }

        pDevice->peakscanEventCallback = NEXUS_RegisterEvent(pDevice->peakscanEvent,
                                                             NEXUS_Frontend_P_Ast_PeakscanEventHandler,
                                                             pDevice);
        if ( NULL == pDevice->peakscanEventCallback )
        {
            errCode = BERR_TRACE(BERR_OS_ERROR);
            goto err_peak_scan_event_callback;
        }
    }

    /* Set up I2C handle */
    {
        NEXUS_I2cCallbacks i2cCallbacks;
        NEXUS_I2c_InitCallbacks(&i2cCallbacks);
        i2cCallbacks.readNoAddr = NEXUS_Frontend_P_Ast_I2cReadNoAddr;
        i2cCallbacks.writeNoAddr = NEXUS_Frontend_P_Ast_I2cWriteNoAddr;
        pDevice->deviceI2cHandle = NEXUS_I2c_CreateHandle(NEXUS_MODULE_SELF, pDevice, &i2cCallbacks);
        if (!pDevice->deviceI2cHandle ) {
            errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_i2c;
        }
    }

    if (pSettings->capabilities.diseqc)
    {
         /* These must be created after the frontend handle has been created */
         pDevice->diseqcAppCallback = NEXUS_TaskCallback_Create(pDevice->frontendHandle, NULL);
         if ( NULL == pDevice->diseqcAppCallback )
         {
             errCode = BERR_TRACE(BERR_OS_ERROR);
             goto err_diseqc_app_callback;
         }
         if (deviceManagesDiseqc) {
             pSettings->setDiseqcAppCallback(pSettings->getDiseqcChannelHandleParam, pSettings->diseqcIndex, pDevice->diseqcAppCallback);
         }
     }

    if (pDevice->peakscanEvent) {
        pDevice->peakscanAppCallback = NEXUS_TaskCallback_Create(pDevice->frontendHandle, NULL);
        if ( NULL == pDevice->peakscanAppCallback )
        {
            errCode = BERR_TRACE(BERR_OS_ERROR);
            goto err_peak_scan_app_callback;
        }
    }

    pDevice->lockAppCallback = NEXUS_TaskCallback_Create(pDevice->frontendHandle, NULL);
    if ( NULL == pDevice->lockAppCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_lock_app_callback;
    }

    pDevice->ftmCallback = NEXUS_TaskCallback_Create(pDevice->frontendHandle, NULL);
    if ( NULL == pDevice->ftmCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_lock_app_callback;
    }

    /* fill in type information */
    {
        int i;
        unsigned base10[4] = {1000, 100, 10, 1};
        unsigned base16[4] = {0x1000, 0x100, 0x10, 1};
        unsigned family = 0, familyDec;
        uint16_t chipId;
        uint8_t chipVersion;
        uint32_t bondingOption;
        uint8_t apMicrocodeVersion;
        uint8_t hostConfigurationVersion;
        errCode = BAST_GetApVersion(pDevice->astHandle,
            &chipId,
            &chipVersion,
            &bondingOption,
            &apMicrocodeVersion,
            &hostConfigurationVersion);
        BKNI_Memset(&pDevice->type, 0, sizeof(pDevice->type));
        familyDec = pSettings->astChip;
        for (i=0; i<4; i++) {
            family += (familyDec / base10[i]) * base16[i];
            familyDec = familyDec % base10[i];
        }
        pDevice->type.chip.familyId = family;
        pDevice->type.chip.id = chipId;
    }

    return NEXUS_SUCCESS;

err_lock_app_callback:
    if (pDevice->peakscanEvent) {
        NEXUS_TaskCallback_Destroy(pDevice->peakscanAppCallback);
    }
err_peak_scan_app_callback:
    NEXUS_TaskCallback_Destroy(pDevice->diseqcAppCallback);
err_diseqc_app_callback:
    NEXUS_I2c_DestroyHandle(pDevice->deviceI2cHandle);
err_i2c:
    NEXUS_Frontend_P_Destroy(pDevice->frontendHandle);
    if (pDevice->peakscanEvent) {
        NEXUS_UnregisterEvent(pDevice->peakscanEventCallback);
    }
err_peak_scan_event_callback:
err_peak_scan_event:
    if (!deviceManagesDiseqc) {
        NEXUS_UnregisterEvent(pDevice->diseqcEventCallback);
    }
err_diseqc_event_callback:
err_diseqc_event:
    NEXUS_UnregisterEvent(pDevice->lockEventCallback);
err_lock_event_callback:
err_lock_event:
    return BERR_TRACE(NEXUS_NOT_INITIALIZED);
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Ast_Create( const NEXUS_FrontendAstSettings *pSettings )
{
    BERR_Code errCode;
    NEXUS_AstDevice *pDevice;

    BDBG_ASSERT(NULL != pSettings);

    if (!pSettings->delayedInitialization) {
        /* when delaying initialization, these values need to be set later -- NULL is OK. */
        BDBG_ASSERT(NULL != pSettings->astHandle);
        BDBG_ASSERT(NULL != pSettings->astChannel);
    }

    pDevice = BKNI_Malloc(sizeof(NEXUS_AstDevice));
    if ( NULL == pDevice )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(pDevice, 0, sizeof(*pDevice));

    pDevice->settings = *pSettings;     /* Save Settings */
    pDevice->astHandle = pSettings->astHandle;
    pDevice->astChannel = pSettings->astChannel;
    pDevice->astChip = pSettings->astChip;
    pDevice->channel = pSettings->channelIndex;
    pDevice->diseqcIndex = pSettings->diseqcIndex;

    /* Create Frontend Handle */
    pDevice->frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if ( NULL == pDevice->frontendHandle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_frontend_handle;
    }
    pDevice->frontendHandle->chip.id = pSettings->astChip;

    /* Set Capabilities */
    BDBG_ASSERT(pSettings->capabilities.satellite);
    pDevice->frontendHandle->capabilities = pSettings->capabilities;
    pDevice->capabilities = pSettings->capabilities;

    /* Install Hooks */
    pDevice->frontendHandle->close = NEXUS_Frontend_P_Ast_Close;
    pDevice->frontendHandle->untune = NEXUS_Frontend_P_Ast_Untune;
    pDevice->frontendHandle->getDiseqcReply = NEXUS_Frontend_P_Ast_GetDiseqcReply;
    pDevice->frontendHandle->getDiseqcSettings = NEXUS_Frontend_P_Ast_GetDiseqcSettings;
    pDevice->frontendHandle->sendDiseqcAcw = NEXUS_Frontend_P_Ast_SendDiseqcAcw;
    pDevice->frontendHandle->sendDiseqcMessage = NEXUS_Frontend_P_Ast_SendDiseqcMessage;
    pDevice->frontendHandle->setDiseqcSettings = NEXUS_Frontend_P_Ast_SetDiseqcSettings;
    pDevice->frontendHandle->getDiseqcStatus = NEXUS_Frontend_P_Ast_GetDiseqcStatus;
    pDevice->frontendHandle->resetDiseqc = NEXUS_Frontend_P_Ast_ResetDiseqc;
    pDevice->frontendHandle->getSatelliteStatus = NEXUS_Frontend_P_Ast_GetSatelliteStatus;
    pDevice->frontendHandle->getSoftDecisions = NEXUS_Frontend_P_Ast_GetSoftDecisions;
    pDevice->frontendHandle->tuneSatellite = NEXUS_Frontend_P_Ast_TuneSatellite;
    pDevice->frontendHandle->resetStatus = NEXUS_Frontend_P_Ast_ResetStatus;
    pDevice->frontendHandle->readSatelliteConfig = NEXUS_Frontend_P_Ast_ReadSatelliteConfig;
    pDevice->frontendHandle->writeSatelliteConfig = NEXUS_Frontend_P_Ast_WriteSatelliteConfig;
    pDevice->frontendHandle->satellitePeakscan = NEXUS_Frontend_P_Ast_SatellitePeakscan;
    pDevice->frontendHandle->getSatellitePeakscanResult = NEXUS_Frontend_P_Ast_GetSatellitePeakscanResult;
    pDevice->frontendHandle->satelliteToneSearch = NEXUS_Frontend_P_Ast_SatelliteToneSearch;
    pDevice->frontendHandle->getSatelliteToneSearchResult = NEXUS_Frontend_P_Ast_GetSatelliteToneSearchResult;
    pDevice->frontendHandle->registerExtension = NEXUS_Frontend_P_Ast_RegisterExtension;
    pDevice->frontendHandle->getFastStatus = NEXUS_Frontend_P_Ast_GetFastStatus;
    pDevice->frontendHandle->getType = NEXUS_Frontend_P_Ast_GetType;
    pDevice->frontendHandle->getSatelliteSignalDetectStatus = NEXUS_Frontend_P_Ast_GetSignalDetectStatus;
    pDevice->frontendHandle->readRegister = NEXUS_Frontend_P_Ast_ReadRegister;
    pDevice->frontendHandle->writeRegister = NEXUS_Frontend_P_Ast_WriteRegister;
    pDevice->frontendHandle->getBertStatus = NEXUS_Frontend_P_Ast_GetBertStatus;

    if (!pSettings->delayedInitialization) {
        NEXUS_Frontend_P_Ast_DelayedInitialization(pDevice->frontendHandle);
    }

    /* Success */
    BDBG_OBJECT_SET(pDevice, NEXUS_AstDevice);
    return pDevice->frontendHandle;

err_frontend_handle:
    BKNI_Free(pDevice);

    return NULL;
}

static void NEXUS_Frontend_P_Ast_Close(NEXUS_FrontendHandle handle)
{
    NEXUS_AstDevice *pDevice;
    bool deviceManagesDiseqc = false;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (pDevice->getDiseqcChannelHandle) {
        deviceManagesDiseqc = true;
    }
    /* Cleanup Channel */
    if (pDevice->peakscanEvent) {
        NEXUS_UnregisterEvent(pDevice->peakscanEventCallback);
        if (pDevice->peakscanAppCallback) {
            NEXUS_TaskCallback_Destroy(pDevice->peakscanAppCallback);
        }
    }
    if (pDevice->lockEventCallback) {
        NEXUS_UnregisterEvent(pDevice->lockEventCallback);
    }
    if (pDevice->lockAppCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->lockAppCallback);
    }
    if (pDevice->ftmCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->ftmCallback);
    }

    if (pDevice->capabilities.diseqc)
    {
        if (!deviceManagesDiseqc) {
            NEXUS_UnregisterEvent(pDevice->diseqcEventCallback);
        }
        if (pDevice->diseqcAppCallback) {
            NEXUS_TaskCallback_Destroy(pDevice->diseqcAppCallback);
        }
    }

    if (pDevice->deviceI2cHandle) {
        NEXUS_I2c_DestroyHandle(pDevice->deviceI2cHandle);
    }
    if (pDevice->frontendHandle) {
        NEXUS_Frontend_P_Destroy(pDevice->frontendHandle);
    }

    /* Call post-close callback */
    if ( pDevice->settings.closeFunction )
    {
        pDevice->settings.closeFunction(handle, pDevice->settings.pCloseParam);
    }

    BKNI_Memset(pDevice, 0, sizeof(*pDevice));
    BKNI_Free(pDevice);
}

NEXUS_Error NEXUS_Frontend_P_Ast_SetDevices( NEXUS_FrontendHandle handle, const NEXUS_FrontendAstDevices *pDevices )
{
    NEXUS_AstDevice *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pDevices);

    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    pDevice->settings.devices = *pDevices;

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Frontend_P_Ast_TuneSatellite( void *handle, const NEXUS_FrontendSatelliteSettings *pSettings )
{
    BERR_Code errCode;
    NEXUS_AstDevice *pDevice = handle;
    BAST_AcqSettings acqSettings;
    uint8_t networkSpec;
    bool nyquist20 = false;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_ASSERT(NULL != pSettings);

    if (pSettings->frequency == 0) {
        pDevice->lastSettings = *pSettings; /* save after all config, but before the acquire. */
        BAST_AbortAcq(pDevice->astChannel);
        return BERR_SUCCESS;
    }

    if (pSettings->nyquist20) {
        /* default value of deprecated member variable, check new field for changes first */
        if (pSettings->nyquistRolloff != NEXUS_FrontendSatelliteNyquistFilter_e20) {
            /* Validate values. AST only supports 20 or 35. */
            if (pSettings->nyquistRolloff == NEXUS_FrontendSatelliteNyquistFilter_e35) {
                nyquist20 = false;
            } else {
                BDBG_WRN(("This frontend only supports nyquist 20 and 35."));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        } else
            nyquist20 = true;
    } else {
        /* the app changed the deprecated variable. Assume this means old code and behave accordingly. */
        nyquist20 = false;
    }

    BKNI_Memset(&acqSettings, 0, sizeof(acqSettings));

    /* BAST_ACQSETTINGS_DEFAULT was applied in NEXUS_Frontend_GetDefaultSatelliteSettings */

    if ( pSettings->pnData )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_PN;
    if ( pSettings->prbs15 )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_PRBS15;
    if ( nyquist20 )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_NYQUIST_20;
    if ( pSettings->bertEnable )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_BERT_ENABLE;
    if ( pSettings->pnDataInvert )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_PN_INVERT;
    if ( pSettings->bertResyncDisable )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_BERT_RESYNC_DISABLE;
    if ( pSettings->reacquireDisable )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_REACQ_DISABLE;
    if ( pSettings->dciiSplit )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_DCII_SPLIT;
    if ( pSettings->dciiSplitQ )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_DCII_SPLIT_Q;
    if ( pSettings->oQpsk )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_OQPSK;
    if ( pSettings->rsDisable )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_RS_DISABLE;
    if ( pSettings->ldpcPilot )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_LDPC_PILOT;
    if ( pSettings->ldpcPilotPll )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_LDPC_PILOT_PLL;
    if ( pSettings->ldpcPilotScan )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_LDPC_PILOT_SCAN;
    if ( pSettings->tunerTestMode )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_TUNER_TEST_MODE;
    if ( pSettings->toneSearchMode )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_TONE_SEARCH_MODE;
    if ( pSettings->signalDetectMode )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_SIGNAL_DETECT_MODE;
    if ( pSettings->bypassTune )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_BYPASS_TUNE;
    if ( pSettings->bypassAcquire )
        acqSettings.acq_ctl |= BAST_ACQSETTINGS_BYPASS_ACQUIRE;
    if (pSettings->shortFrames) {
        BDBG_WRN(("This frontend does not support short frames."));
    }

    /* AST engineers prefer we always set this to scan. */
    if (pSettings->spectralInversion != NEXUS_FrontendSatelliteInversion_eScan) {
        BDBG_MSG(("forcing NEXUS_FrontendSatelliteInversion_eScan mode")); /* don't WRN */
    }
    acqSettings.acq_ctl |= BAST_ACQSETTINGS_SPINV_IQ_SCAN;

    /* This is added for backward compatibility */
    switch ( pSettings->mode )
    {
    case NEXUS_FrontendSatelliteMode_eQpskLdpc:
    case NEXUS_FrontendSatelliteMode_e8pskLdpc:
    case NEXUS_FrontendSatelliteMode_eLdpc:
        acqSettings.acq_ctl |= (BAST_ACQSETTINGS_NYQUIST_20 | BAST_ACQSETTINGS_SPINV_IQ_SCAN);
        break;
    default:
        break;
    }

    switch ( pSettings->mode )
    {
    case NEXUS_FrontendSatelliteMode_eDvb:
        BDBG_MSG(("Tune DVB"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_dvb_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eDss:
        BDBG_MSG(("Tune DSS"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_dss_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eDcii:
        BDBG_MSG(("Tune DCII"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_dcii_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eQpskLdpc:
        BDBG_MSG(("Tune QPSK LDPC"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_qpsk_ldpc_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_e8pskLdpc:
        BDBG_MSG(("Tune 8PSK LDPC"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_8psk_ldpc_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eLdpc:
        BDBG_MSG(("Tune LDCP"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_ldpc_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eQpskTurbo:
        BDBG_MSG(("Tune Turbo QPSK"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_qpsk_turbo_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_e8pskTurbo:
        BDBG_MSG(("Tune Turbo 8PSK"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_8psk_turbo_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eTurbo:
        BDBG_MSG(("Tune Turbo"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_sds_turbo_modes, &pSettings->codeRate);
        break;
    case NEXUS_FrontendSatelliteMode_eBlindAcquisition:
        BDBG_MSG(("Blind acquisition"));
        acqSettings.mode = NEXUS_Frontend_P_Ast_GetMode(g_blind_acquisition_mode, &pSettings->codeRate);
        break;
    default:
        /* turbo not supported */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("acqSettings.acq_ctl %x", acqSettings.acq_ctl));
    acqSettings.symbolRate = pSettings->symbolRate;
    BDBG_MSG(("Freq %u, Symbol Rate %u", pSettings->frequency, acqSettings.symbolRate));
    acqSettings.carrierFreqOffset = pSettings->carrierOffset;

    NEXUS_TaskCallback_Set(pDevice->lockAppCallback, &pSettings->lockCallback);

#if NEXUS_POWER_MANAGEMENT
    {
        BAST_ChannelHandle channelHandle;
        if (pDevice->getDiseqcChannelHandle) {
            channelHandle = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
        } else {
            channelHandle = pDevice->astChannel;
        }
        BDBG_ASSERT(channelHandle);

        errCode = BAST_PowerUp(pDevice->astChannel, BAST_CORE_SDS);
        if (pDevice->capabilities.diseqc && !errCode) {
            errCode = BAST_PowerUp(channelHandle, BAST_CORE_DISEQC);
        }
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    /* Restore the internal LNA (if configured to do so)
     * On some front-ends, this is required in order to be able to tune after the SDS core has been through a power cycle */
    if ( pDevice->settings.restoreInternalLnaFunction )
    {
        pDevice->settings.restoreInternalLnaFunction(handle, pDevice->settings.pRestoreParam);
    }
#endif

    /* Clear any previous signal/tone searches, or an in-progress acquisition, if any. */
    BAST_AbortAcq(pDevice->astChannel);

    /* Support external tuner if present */
    if ( pDevice->settings.devices.tuner )
    {
        errCode = NEXUS_Tuner_SetFrequency(pDevice->settings.devices.tuner, NEXUS_TunerMode_eDigital, pSettings->frequency);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    if (pSettings->networkSpec != pDevice->lastSettings.networkSpec) {
        networkSpec = (uint8_t)pSettings->networkSpec;
        BAST_SetNetworkSpec(pDevice->astHandle, networkSpec);
    }

#ifdef BCM73XX_CONFIG_LEN_EXT_TUNER_IF_OFFSET
    /* Set IF offset for external tuners */
    {
        uint32_t val = (uint32_t)pSettings->ifOffset;
        uint8_t buf[4];

        buf[0] = (val >> 24) & 0xFF;
        buf[1] = (val >> 16) & 0xFF;
        buf[2] = (val >> 8) & 0xFF;
        buf[3] = (val & 0xFF);
        BAST_WriteConfig(pDevice->astChannel, BCM73XX_CONFIG_EXT_TUNER_IF_OFFSET, buf, BCM73XX_CONFIG_LEN_EXT_TUNER_IF_OFFSET);
    }
#endif

    /* Warning, this overrides the search range for all channels of this AST device.  */
    errCode = BAST_SetSearchRange(pDevice->astHandle, pSettings->searchRange);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* If BERT mode is enabled OR if it has changed since the last time */
    if (pSettings->bertEnable || (pDevice->lastSettings.bertEnable != pSettings->bertEnable)) {
        /* BERT mode has changed */
        BAST_OutputTransportSettings xptSettings;
        BAST_GetOutputTransportSettings(pDevice->astChannel, &xptSettings);
        if ((pSettings->bertEnable) && (pSettings->mode != NEXUS_FrontendSatelliteMode_eDss)) {
            xptSettings.bXbert = true;
        } else {
            xptSettings.bXbert = false;
        }
        BAST_SetOutputTransportSettings(pDevice->astChannel, &xptSettings);
    }

#ifndef BAST_G3_CONFIG_TUNER_CT
#define BAST_G3_CONFIG_TUNER_CTL                 0x0008 /* tuner control settings, see BAST_G3_CONFIG_TUNER_CTL_* macros */
#define BAST_G3_CONFIG_LEN_TUNER_CTL             1
#define BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST      0x10 /* 1=bypass dft freq estimate */
#define BAST_G3_CONFIG_TUNER_CTL_LOCAL_DEFINES 1
#endif
#if 0
    if (pDevice->lastSettings.bypassFrequencyEstimation != pSettings->bypassFrequencyEstimation)
#endif
    {
        uint8_t tuner_ctl;
        BAST_ReadConfig(pDevice->astChannel, BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL);
        if (pSettings->bypassFrequencyEstimation)
            tuner_ctl |= BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST;
        else
            tuner_ctl &= ~BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST;
        BAST_WriteConfig(pDevice->astChannel, BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL);
    }
#ifdef BAST_G3_CONFIG_TUNER_CTL_LOCAL_DEFINES
#undef BAST_G3_CONFIG_TUNER_CTL
#undef BAST_G3_CONFIG_LEN_TUNER_CTL
#undef BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST
#undef BAST_G3_CONFIG_TUNER_CTL_LOCAL_DEFINES
#endif

    pDevice->lastSettings = *pSettings; /* save after all config, but before the acquire. */

    errCode = BAST_TuneAcquire(pDevice->astChannel, pSettings->frequency, &acqSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BTRC_TRACE(ChnChange_Tune, STOP);
    BTRC_TRACE(ChnChange_TuneLock, START);

    return BERR_SUCCESS;
}

void NEXUS_Frontend_P_Ast_Untune(void *handle)
{
    NEXUS_Error errCode;
    NEXUS_AstDevice *pDevice = handle;

    BDBG_MSG(("untune"));
    BDBG_ASSERT(handle);
#if NEXUS_POWER_MANAGEMENT
    if (pDevice->capabilities.diseqc) {
        BAST_ChannelHandle channelHandle;
        if (pDevice->getDiseqcChannelHandle) {
            channelHandle = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
        } else {
            channelHandle = pDevice->astChannel;
        }
        BDBG_ASSERT(channelHandle);
        BAST_PowerDown(channelHandle, BAST_CORE_DISEQC);
    }
    errCode = BAST_PowerDown(pDevice->astChannel, BAST_CORE_SDS);
    if (errCode) {errCode = BERR_TRACE(errCode);}
#else
    BSTD_UNUSED(pDevice);
    BSTD_UNUSED(errCode);
#endif
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetSatelliteStatus( void *handle, NEXUS_FrontendSatelliteStatus *pStatus )
{
    NEXUS_Error errCode;
    BAST_ChannelStatus astStatus;
    NEXUS_AstDevice *pDevice = handle;
    NEXUS_Time now;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_ASSERT(NULL != pStatus);

#if NEXUS_POWER_MANAGEMENT
    BAST_PowerUp(pDevice->astChannel, BAST_CORE_SDS);
#endif

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    errCode = BAST_GetApVersion(pDevice->astHandle,
        &pStatus->version.chipId,
        &pStatus->version.chipVersion,
        &pStatus->version.bondingOption,
        &pStatus->version.apMicrocodeVersion,
        &pStatus->version.hostConfigurationVersion);
    if (errCode) {
        if (errCode == BI2C_ERR_NO_ACK) {
            pStatus->statusError = NEXUS_FrontendSatelliteStatusError_eI2cNoAck;
            return 0;
        }
        else if (errCode == BAST_ERR_HAB_TIMEOUT) {
            pStatus->statusError = NEXUS_FrontendSatelliteStatusError_eHabTimeout;
            return 0;
        }
        else {
            /* unknown error. status function should fail. */
            return BERR_TRACE(errCode);
        }
    }

    errCode = BAST_GetChannelStatus(pDevice->astChannel, &astStatus);
    if ( errCode ) {
        /* if not locked, BAST_GetChannelStatus will fail. this is not an error. the status structure should just report not locked and no status available. */
        pStatus->statusError = NEXUS_FrontendSatelliteStatusError_eGetChannelStatusError;
        return 0;
    }

    NEXUS_Time_Get(&now);
    pStatus->timeElapsed = NEXUS_Time_Diff(&now, &pDevice->frontendHandle->resetStatusTime);

    pStatus->settings = pDevice->lastSettings; /* return last settings used in NEXUS_Frontend_TuneSatellite */
    pStatus->channel = pDevice->settings.channelIndex;
    pStatus->sampleClock = astStatus.sample_clock;
    pStatus->carrierError = astStatus.carrierError;
    pStatus->carrierOffset = astStatus.carrierOffset;
    pStatus->outputBitRate = astStatus.outputBitrate;
    pStatus->symbolRate = astStatus.symbolRate;
    pStatus->symbolRateError = astStatus.symbolRateError;
    pStatus->snrEstimate = astStatus.snrEstimate * 100/256;  /* convert to 1/100 units */
    pStatus->tunerLocked = astStatus.bTunerLocked;
    pStatus->demodLocked = astStatus.bDemodLocked;
    pStatus->bertLocked = astStatus.bBertLocked;
    pStatus->mpegErrors = astStatus.mpegErrors;
    pStatus->mpegCount = astStatus.mpegCount;
    pStatus->berErrorCount = astStatus.berErrors;
    pStatus->reacquireCount = astStatus.reacqCount;
    pStatus->ifAgc = astStatus.IFagc;
    pStatus->rfAgc = astStatus.RFagc;
    pStatus->agf = astStatus.agf;
    pStatus->frequency = astStatus.tunerFreq;

#if NEXUS_FRONTEND_4501
    {
    BAST_LnaStatus lnaStatus;
    BAST_GetLnaStatus(pDevice->settings.astHandle, &lnaStatus);
    pStatus->lnaStatus.intConfig = lnaStatus.int_config;
    pStatus->lnaStatus.extConfig = lnaStatus.ext_config;
    pStatus->lnaStatus.version = lnaStatus.version;
    pStatus->lnaStatus.agc0 = lnaStatus.agc0;
    pStatus->lnaStatus.agc1 = lnaStatus.agc1;
    }
#endif

    if (BAST_MODE_IS_LEGACY_QPSK(astStatus.mode))
    {
        pStatus->fecCorrected = astStatus.modeStatus.legacy.rsCorrCount;
        pStatus->fecUncorrected = astStatus.modeStatus.legacy.rsUncorrCount;
        pStatus->preViterbiErrorCount = astStatus.modeStatus.legacy.preVitErrCount;
        switch ( astStatus.modeStatus.legacy.spinv )
        {
        default:
            pStatus->spectralInversion = NEXUS_FrontendSatelliteInversion_eScan;
            break;
        case BAST_Spinv_eNormal:
            pStatus->spectralInversion = NEXUS_FrontendSatelliteInversion_eNormal;
            break;
        case BAST_Spinv_eIinv:
            pStatus->spectralInversion = NEXUS_FrontendSatelliteInversion_eI;
            break;
        case BAST_Spinv_eQinv:
            pStatus->spectralInversion = NEXUS_FrontendSatelliteInversion_eQ;
            break;
        }
        switch ( astStatus.modeStatus.legacy.phase )
        {
        default:
        case BAST_PhaseRotation_e0:
            pStatus->fecPhase = 0;
            break;
        case BAST_PhaseRotation_e90:
            pStatus->fecPhase = 90;
            break;
        case BAST_PhaseRotation_e180:
            pStatus->fecPhase = 180;
            break;
        case BAST_PhaseRotation_e270:
            pStatus->fecPhase = 270;
            break;
        }
    }
    else if ( BAST_MODE_IS_LDPC(astStatus.mode) )
    {
        pStatus->fecCorrected = astStatus.modeStatus.ldpc.corrBlocks;
        pStatus->fecUncorrected = astStatus.modeStatus.ldpc.badBlocks;
        pStatus->fecClean = astStatus.modeStatus.ldpc.totalBlocks -
            astStatus.modeStatus.ldpc.corrBlocks -
            astStatus.modeStatus.ldpc.badBlocks;
    }
    else if (BAST_MODE_IS_TURBO(astStatus.mode))
    {
        pStatus->fecCorrected = astStatus.modeStatus.turbo.corrBlocks;
        pStatus->fecUncorrected = astStatus.modeStatus.turbo.badBlocks;
        pStatus->fecClean = astStatus.modeStatus.turbo.totalBlocks -
            astStatus.modeStatus.turbo.corrBlocks -
            astStatus.modeStatus.turbo.badBlocks;
   }

    /* Check mode to get coderate */
    switch ( astStatus.mode )
    {
    case BAST_Mode_eDvb_scan:
    case BAST_Mode_eDss_scan:
    case BAST_Mode_eDcii_scan:
    case BAST_Mode_eLdpc_scan:
    case BAST_Mode_eTurbo_scan:
        pStatus->codeRate = g_cr_scan;
        break;

    case BAST_Mode_eDvb_1_2:
    case BAST_Mode_eDss_1_2:
    case BAST_Mode_eDcii_1_2:
    case BAST_Mode_eLdpc_Qpsk_1_2:
    case BAST_Mode_eTurbo_Qpsk_1_2:
        pStatus->codeRate = g_cr_1_2;
        break;

    case BAST_Mode_eDvb_2_3:
    case BAST_Mode_eDss_2_3:
    case BAST_Mode_eDcii_2_3:
    case BAST_Mode_eLdpc_Qpsk_2_3:
    case BAST_Mode_eLdpc_8psk_2_3:
    case BAST_Mode_eTurbo_Qpsk_2_3:
    case BAST_Mode_eTurbo_8psk_2_3:
        pStatus->codeRate = g_cr_2_3;
        break;

    case BAST_Mode_eDvb_3_4:
    case BAST_Mode_eDcii_3_4:
    case BAST_Mode_eLdpc_Qpsk_3_4:
    case BAST_Mode_eLdpc_8psk_3_4:
    case BAST_Mode_eTurbo_Qpsk_3_4:
    case BAST_Mode_eTurbo_8psk_3_4:
        pStatus->codeRate = g_cr_3_4;
        break;

    case BAST_Mode_eDcii_3_5:
    case BAST_Mode_eLdpc_Qpsk_3_5:
    case BAST_Mode_eLdpc_8psk_3_5:
        pStatus->codeRate = g_cr_3_5;
        break;

    case BAST_Mode_eDcii_4_5:
    case BAST_Mode_eLdpc_Qpsk_4_5:
    case BAST_Mode_eTurbo_8psk_4_5:
        pStatus->codeRate = g_cr_4_5;
        break;

    case BAST_Mode_eDvb_5_6:
    case BAST_Mode_eDcii_5_6:
    case BAST_Mode_eLdpc_Qpsk_5_6:
    case BAST_Mode_eLdpc_8psk_5_6:
    case BAST_Mode_eTurbo_Qpsk_5_6:
    case BAST_Mode_eTurbo_8psk_5_6:
        pStatus->codeRate = g_cr_5_6;
        break;

    case BAST_Mode_eDss_6_7:
        pStatus->codeRate = g_cr_6_7;
        break;

    case BAST_Mode_eDvb_7_8:
    case BAST_Mode_eDcii_7_8:
    case BAST_Mode_eTurbo_Qpsk_7_8:
        pStatus->codeRate = g_cr_7_8;
        break;

    case BAST_Mode_eDcii_5_11:
        pStatus->codeRate = g_cr_5_11;
        break;

    case BAST_Mode_eLdpc_Qpsk_8_9:
    case BAST_Mode_eLdpc_8psk_8_9:
    case BAST_Mode_eTurbo_8psk_8_9:
        pStatus->codeRate = g_cr_8_9;
        break;

    case BAST_Mode_eLdpc_Qpsk_9_10:
    case BAST_Mode_eLdpc_8psk_9_10:
        pStatus->codeRate = g_cr_9_10;
        break;

    default:
        break;
    }

    switch ( astStatus.mode )
    {
    default:
    case BAST_Mode_eDvb_scan:
    case BAST_Mode_eDvb_1_2:
    case BAST_Mode_eDvb_2_3:
    case BAST_Mode_eDvb_3_4:
    case BAST_Mode_eDvb_5_6:
    case BAST_Mode_eDvb_7_8:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eDvb;
        break;

    case BAST_Mode_eDss_scan:
    case BAST_Mode_eDss_1_2:
    case BAST_Mode_eDss_2_3:
    case BAST_Mode_eDss_6_7:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eDss;
        break;

    case BAST_Mode_eDcii_scan:
    case BAST_Mode_eDcii_1_2:
    case BAST_Mode_eDcii_2_3:
    case BAST_Mode_eDcii_3_4:
    case BAST_Mode_eDcii_3_5:
    case BAST_Mode_eDcii_4_5:
    case BAST_Mode_eDcii_5_6:
    case BAST_Mode_eDcii_7_8:
    case BAST_Mode_eDcii_5_11:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eDcii;
        break;

    case BAST_Mode_eLdpc_scan:
    case BAST_Mode_eLdpc_Qpsk_1_2:
    case BAST_Mode_eLdpc_Qpsk_2_3:
    case BAST_Mode_eLdpc_Qpsk_3_4:
    case BAST_Mode_eLdpc_Qpsk_3_5:
    case BAST_Mode_eLdpc_Qpsk_4_5:
    case BAST_Mode_eLdpc_Qpsk_5_6:
    case BAST_Mode_eLdpc_Qpsk_9_10:
    case BAST_Mode_eLdpc_Qpsk_8_9:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eQpskLdpc;
        break;

    case BAST_Mode_eLdpc_8psk_2_3:
    case BAST_Mode_eLdpc_8psk_3_4:
    case BAST_Mode_eLdpc_8psk_3_5:
    case BAST_Mode_eLdpc_8psk_5_6:
    case BAST_Mode_eLdpc_8psk_8_9:
    case BAST_Mode_eLdpc_8psk_9_10:
        pStatus->mode = NEXUS_FrontendSatelliteMode_e8pskLdpc;
        break;

    case BAST_Mode_eTurbo_scan:
    case BAST_Mode_eTurbo_Qpsk_1_2:
    case BAST_Mode_eTurbo_Qpsk_2_3:
    case BAST_Mode_eTurbo_Qpsk_3_4:
    case BAST_Mode_eTurbo_Qpsk_5_6:
    case BAST_Mode_eTurbo_Qpsk_7_8:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eTurboQpsk;
        break;

    case BAST_Mode_eTurbo_8psk_2_3:
    case BAST_Mode_eTurbo_8psk_3_4:
    case BAST_Mode_eTurbo_8psk_4_5:
    case BAST_Mode_eTurbo_8psk_5_6:
    case BAST_Mode_eTurbo_8psk_8_9:
        pStatus->mode = NEXUS_FrontendSatelliteMode_eTurbo8psk;
        break;
    }

    return BERR_SUCCESS;
}

static void NEXUS_Frontend_P_Ast_GetDiseqcSettings( void *handle, NEXUS_FrontendDiseqcSettings *pSettings )
{
    NEXUS_AstDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

#ifndef TEST_NETWORK_TUNER
    if (pDevice->capabilities.diseqc)
#endif
    {
        BDBG_ASSERT(NULL != pSettings);
        *pSettings = pDevice->diseqcSettings;
    }
}

static NEXUS_Error NEXUS_Frontend_P_Ast_SetDiseqcSettings( void *handle, const NEXUS_FrontendDiseqcSettings *pSettings )
{
    NEXUS_Error errCode;
    NEXUS_AstDevice *pDevice = handle;
    uint8_t buf[2];
    BAST_ChannelHandle channelHandle;

    BDBG_MSG(("NEXUS_Frontend_P_Ast_SetDiseqcSettings(%p,%p)", (void *)handle, (void *)pSettings));

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_ASSERT(NULL != pSettings);

    if (pDevice->getDiseqcChannelHandle) {
        channelHandle = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
    } else {
        channelHandle = pDevice->astChannel;
    }
    BDBG_ASSERT(channelHandle);
    BDBG_MSG(("NEXUS_Frontend_P_Ast_SetDiseqcSettings: channel: %p", (void *)channelHandle));

#if NEXUS_POWER_MANAGEMENT
    /* Ensure diseqc is powered on before reading from it.*/
    BAST_PowerUp(channelHandle, BAST_CORE_DISEQC);
#endif

    if(pDevice->astChip == 4506){
#if defined NEXUS_FRONTEND_4506
        errCode = BAST_ReadConfig(pDevice->astChannel, BCM4506_CONFIG_DISEQC_CTL2, buf, BCM4506_CONFIG_LEN_DISEQC_CTL2);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] &= ~(BCM4506_DISEQC_CTL2_TB_ENABLE | BCM4506_DISEQC_CTL2_TB_B | BCM4506_DISEQC_CTL2_ENVELOPE | BCM4506_DISEQC_CTL2_EXP_REPLY_DISABLE);

        if (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eUnmodulated) {
            buf[0] |= BCM4506_DISEQC_CTL2_TB_ENABLE;
        }
        else if (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eNominal) {
            buf[0] |= BCM4506_DISEQC_CTL2_TB_ENABLE | BCM4506_DISEQC_CTL2_TB_B;
        }

        if ( pSettings->toneMode == NEXUS_FrontendDiseqcToneMode_eEnvelope ) {
            buf[0] |= BCM4506_DISEQC_CTL2_ENVELOPE;
        }

        if (pSettings->framing == NEXUS_FrontendDiseqcFraming_eDontExpectReply) {
            buf[0] |= BCM4506_DISEQC_CTL2_EXP_REPLY_DISABLE;
        }

        errCode = BAST_WriteConfig(pDevice->astChannel, BCM4506_CONFIG_DISEQC_CTL2, buf, BCM4506_CONFIG_LEN_DISEQC_CTL2);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] = 0; buf[1] = 0;
        errCode = BAST_ReadConfig(pDevice->astChannel, BCM4506_CONFIG_DISEQC_PRETX_DELAY, buf, BCM4506_CONFIG_LEN_DISEQC_PRETX_DELAY);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] = (uint8_t)pSettings->preTransmitDelay;

        errCode = BAST_WriteConfig(pDevice->astChannel, BCM4506_CONFIG_DISEQC_PRETX_DELAY, buf, BCM4506_CONFIG_LEN_DISEQC_PRETX_DELAY);
        if (errCode) return BERR_TRACE(errCode);
#endif
    }
    else if(pDevice->astChip == 4501){
#if defined NEXUS_FRONTEND_4501
        errCode = BAST_ReadConfig(pDevice->astChannel, BCM4501_CONFIG_DISEQC_CTL2, buf, BCM4501_CONFIG_LEN_DISEQC_CTL2);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] &= ~(BCM4501_DISEQC_CTL2_TB_ENABLE | BCM4501_DISEQC_CTL2_TB_B | BCM4501_DISEQC_CTL2_ENVELOPE | BCM4501_DISEQC_CTL2_EXP_REPLY_DISABLE);

        if (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eUnmodulated) {
            buf[0] |= BCM4501_DISEQC_CTL2_TB_ENABLE;
        }
        else if (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eNominal) {
            buf[0] |= BCM4501_DISEQC_CTL2_TB_ENABLE | BCM4501_DISEQC_CTL2_TB_B;
        }

        if ( pSettings->toneMode == NEXUS_FrontendDiseqcToneMode_eEnvelope ) {
            buf[0] |= BCM4501_DISEQC_CTL2_ENVELOPE;
        }

        if (pSettings->framing == NEXUS_FrontendDiseqcFraming_eDontExpectReply) {
            buf[0] |= BCM4501_DISEQC_CTL2_EXP_REPLY_DISABLE;
        }

        errCode = BAST_WriteConfig(pDevice->astChannel, BCM4501_CONFIG_DISEQC_CTL2, buf, BCM4501_CONFIG_LEN_DISEQC_CTL2);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] = 0; buf[1] = 0;
        errCode = BAST_ReadConfig(pDevice->astChannel, BCM4501_CONFIG_DISEQC_PRETX_DELAY, buf, BCM4501_CONFIG_LEN_DISEQC_PRETX_DELAY);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] = (uint8_t)pSettings->preTransmitDelay;

        errCode = BAST_WriteConfig(pDevice->astChannel, BCM4501_CONFIG_DISEQC_PRETX_DELAY, buf, BCM4501_CONFIG_LEN_DISEQC_PRETX_DELAY);
        if (errCode) return BERR_TRACE(errCode);
#endif
    }
    else if(pDevice->astChip == 7346 || pDevice->astChip == 7358 || pDevice->astChip == 7344 || pDevice->astChip == 7360 || pDevice->astChip == 7362 || pDevice->astChip == 7228 || pDevice->astChip == 73625)  {
#if defined(NEXUS_FRONTEND_7346)
        BAST_DiseqcSettings diseqcSettings;
        BSTD_UNUSED(buf);
        errCode = BAST_GetDiseqcSettings(pDevice->astChannel, &diseqcSettings);
        if (errCode) return BERR_TRACE(errCode);

        diseqcSettings.bEnableToneburst = ((pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eUnmodulated) || (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eNominal));
        diseqcSettings.bToneburstB = (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eNominal);
        diseqcSettings.bEnvelope = (pSettings->toneMode == NEXUS_FrontendDiseqcToneMode_eEnvelope);
        switch (pSettings->framing) {
        case NEXUS_FrontendDiseqcFraming_eExpectReply:
            diseqcSettings.bOverrideFraming = true;
            diseqcSettings.bExpectReply = true;
            break;
        case NEXUS_FrontendDiseqcFraming_eDontExpectReply:
            diseqcSettings.bOverrideFraming = true;
            diseqcSettings.bExpectReply = false;
            break;
        case NEXUS_FrontendDiseqcFraming_eDefault:
        default:
            diseqcSettings.bOverrideFraming = false;
            diseqcSettings.bExpectReply = false;
            break;
        }

        diseqcSettings.preTxDelay = (uint8_t)pSettings->preTransmitDelay;

        errCode = BAST_SetDiseqcSettings(pDevice->astChannel, &diseqcSettings);
        if (errCode) return BERR_TRACE(errCode);
#endif
    }
    else if(pDevice->astChip == 4538) {
#if defined(NEXUS_FRONTEND_4538)
#if NEXUS_FRONTEND_HAS_A8299_DISEQC
#if !defined(NEXUS_FRONTEND_A8299_0_I2C_ADDR) || !defined(NEXUS_FRONTEND_A8299_1_I2C_ADDR)
#error "A8299 I2C addresses need to be defined for this platform"
#endif

        int channel = pDevice->diseqcIndex;
        uint8_t i2c_addr, shift, chipIdx, ctl;
        uint8_t A8299_control[2] = {0x88, 0x88};
        BAST_DiseqcSettings diseqcSettings;
        BREG_I2C_Handle i2cHandle;

        if (channel > 3) return BERR_TRACE(NEXUS_INVALID_PARAMETER); /* there are four diseqc controls (0..3), which are equal to the ADC assignment of the given frontend. */

        errCode = BAST_GetDiseqcSettings(channelHandle, &diseqcSettings);
        if (errCode) return BERR_TRACE(errCode);

        diseqcSettings.bEnableToneburst = ((pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eUnmodulated) || (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eNominal));
        diseqcSettings.bToneburstB = (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eNominal);
        diseqcSettings.bEnvelope = (pSettings->toneMode == NEXUS_FrontendDiseqcToneMode_eEnvelope);
        switch (pSettings->framing) {
        case NEXUS_FrontendDiseqcFraming_eExpectReply:
            diseqcSettings.bOverrideFraming = true;
            diseqcSettings.bExpectReply = true;
            break;
        case NEXUS_FrontendDiseqcFraming_eDontExpectReply:
            diseqcSettings.bOverrideFraming = true;
            diseqcSettings.bExpectReply = false;
            break;
        case NEXUS_FrontendDiseqcFraming_eDefault:
        default:
            diseqcSettings.bOverrideFraming = false;
            diseqcSettings.bExpectReply = false;
            break;
        }

        diseqcSettings.preTxDelay = (uint8_t)pSettings->preTransmitDelay;

        errCode = BAST_SetDiseqcSettings(channelHandle, &diseqcSettings);
        if (errCode) return BERR_TRACE(errCode);

        if (channel >= 2)
        {
            i2c_addr = NEXUS_FRONTEND_A8299_1_I2C_ADDR;
            chipIdx = 1;
        }
        else
        {
            i2c_addr = NEXUS_FRONTEND_A8299_0_I2C_ADDR;
            chipIdx = 0;
        }

        if ((channel & 1) == 0)
            shift = 0;
        else
            shift = 4;

        ctl = (pSettings->voltage == NEXUS_FrontendDiseqcVoltage_e18v) ? 0xC : 0x8;
        A8299_control[chipIdx] &= ~((0x0F) << shift);
        A8299_control[chipIdx] |= (ctl << shift);
        buf[0] = 0;
        buf[1] = A8299_control[chipIdx];

        i2cHandle = pDevice->settings.i2cRegHandle;
        BDBG_MSG(("A8299_%d: channel=%d, i2c_addr=0x%X, ctl=0x%X, handle: %p", chipIdx, channel, i2c_addr, buf[1], (void*)i2cHandle));
        BREG_I2C_WriteNoAddr(i2cHandle, i2c_addr, buf, 2);
        if (errCode) return BERR_TRACE(errCode);
#else
        BSTD_UNUSED(buf);
#endif
#endif
    }
    else {
#if defined(BCM73XX_CONFIG_DISEQC_CTL)
        errCode = BAST_ReadConfig(pDevice->astChannel, BCM73XX_CONFIG_DISEQC_CTL, buf, BCM73XX_CONFIG_LEN_DISEQC_CTL);
        if (errCode) return BERR_TRACE(errCode);

        buf[1] &= ~(BCM73XX_DISEQC_CTL_TONEBURST_ENABLE | BCM73XX_DISEQC_CTL_TONEBURST_B | BCM73XX_DISEQC_CTL_ENVELOPE | BCM73XX_DISEQC_CTL_EXP_REPLY_DISABLE);

        if (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eUnmodulated) {
            buf[1] |= BCM73XX_DISEQC_CTL_TONEBURST_ENABLE;
        }
        else if (pSettings->toneBurst == NEXUS_FrontendDiseqcToneBurst_eNominal) {
            buf[1] |= BCM73XX_DISEQC_CTL_TONEBURST_ENABLE | BCM73XX_DISEQC_CTL_TONEBURST_B;
        }

        if ( pSettings->toneMode == NEXUS_FrontendDiseqcToneMode_eEnvelope ) {
            buf[1] |= BCM73XX_DISEQC_CTL_ENVELOPE;
        }

        if (pSettings->framing == NEXUS_FrontendDiseqcFraming_eDontExpectReply) {
            buf[1] |= BCM73XX_DISEQC_CTL_EXP_REPLY_DISABLE;
        }

        errCode = BAST_WriteConfig(pDevice->astChannel, BCM73XX_CONFIG_DISEQC_CTL, buf, BCM73XX_CONFIG_LEN_DISEQC_CTL);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] = 0; buf[1] = 0;
        errCode = BAST_ReadConfig(pDevice->astChannel, BCM73XX_CONFIG_DISEQC_PRETX_DELAY, buf, BCM73XX_CONFIG_LEN_DISEQC_PRETX_DELAY);
        if (errCode) return BERR_TRACE(errCode);

        buf[0] = (uint8_t)pSettings->preTransmitDelay;

        errCode = BAST_WriteConfig(pDevice->astChannel, BCM73XX_CONFIG_DISEQC_PRETX_DELAY, buf, BCM73XX_CONFIG_LEN_DISEQC_PRETX_DELAY);
        if (errCode) return BERR_TRACE(errCode);

        /*After setting DISEQC_CTL, reset is needed to complete setting*/
        if ((pSettings->toneBurst != pDevice->diseqcSettings.toneBurst) ||
            (pSettings->toneMode!= pDevice->diseqcSettings.toneMode))
        {
            errCode = BAST_ResetDiseqc(pDevice->astChannel, 0);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
#else
    BSTD_UNUSED(buf);
#endif
    }

    #ifndef TEST_NETWORK_TUNER
    if (pDevice->capabilities.diseqc)
    #endif
    {
        #ifdef TEST_NETWORK_TUNER
        if (pDevice->settings.channelIndex==NEXUS_P_MAX_AST_CHANNELS-1)
        {
            /* For testing external tuner easily.  If we are SDS1 for 7325 or SDS2 for 7335, control diseqc from SDS0 or SDS1, respectively. */
            pDevice = pAstDevice[NEXUS_P_MAX_AST_CHANNELS-2];
        }
        #endif

        errCode = BAST_SetDiseqcTone(channelHandle, pSettings->toneEnabled);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        if((pDevice->astChip != 4501) && (pDevice->astChip != 4506) && (pDevice->astChip != 4538)){
            errCode = BAST_EnableDiseqcLnb(channelHandle, pSettings->lnbEnabled);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        errCode = BAST_SetDiseqcVoltage(channelHandle, (pSettings->voltage==NEXUS_FrontendDiseqcVoltage_e18v)?true:false);
        if ( errCode )
        {
            /* Restore original setting */
            (void) BAST_SetDiseqcTone(channelHandle, pDevice->diseqcSettings.toneEnabled);
            return BERR_TRACE(errCode);
        }

        pDevice->diseqcSettings = *pSettings;
    }

#if NEXUS_POWER_MANAGEMENT
    if (!pDevice->diseqcSettings.enabled) {
        BAST_PowerDown(channelHandle, BAST_CORE_DISEQC);
    }
#endif

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetDiseqcStatus( void *handle, NEXUS_FrontendDiseqcStatus *pStatus )
{
    NEXUS_AstDevice *pDevice = handle;
    BERR_Code rc;
    uint8_t temp;
    BAST_DiseqcStatus diseqcStatus;
    BAST_ChannelHandle channel;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (pDevice->getDiseqcChannelHandle) {
        channel = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
    } else {
        channel = pDevice->astChannel;
    }

    BDBG_CASSERT((NEXUS_FrontendDiseqcMessageStatus)BAST_DiseqcSendStatus_eBusy == NEXUS_FrontendDiseqcMessageStatus_eBusy);
    rc = BAST_GetDiseqcStatus(channel, &diseqcStatus);
    if (rc) return BERR_TRACE(rc);
    pStatus->sendStatus = diseqcStatus.status;

    if (pDevice->astChip != 4501) {
        /* HW requires that BAST_GetDiseqcVoltage be called before BAST_GetDiseqcTone */
        rc = BAST_GetDiseqcVoltage(channel, &temp);
        if (rc) {
            if (rc == BAST_ERR_DISEQC_BUSY) {
                /* If there is an outstanding message, the voltage cannot be read, but we do not want to return an error. */
                pStatus->voltage = 0;
            } else {
                return BERR_TRACE(rc);
            }
        } else {
            pStatus->voltage = temp;
        }
    }

    rc = BAST_GetDiseqcTone(channel, &pStatus->toneEnabled);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_SendDiseqcMessage( void *handle, const uint8_t *pSendData,
    size_t sendDataSize, const NEXUS_CallbackDesc *pSendComplete )
{
    NEXUS_AstDevice *pDevice = handle;
    BERR_Code errCode;
    BAST_ChannelHandle channel;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_ASSERT(NULL != pSendData);
    BDBG_ASSERT(sendDataSize > 0);

    if (pDevice->getDiseqcChannelHandle) {
        channel = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
        if (pDevice->getDiseqcAppCallback(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex) != pDevice->diseqcAppCallback) {
            pDevice->setDiseqcAppCallback(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex, pDevice->diseqcAppCallback);
        }
    } else {
        channel = pDevice->astChannel;
    }

    if (pDevice->capabilities.diseqc)
    {
        NEXUS_TaskCallback_Set(pDevice->diseqcAppCallback, pSendComplete);

#if NEXUS_POWER_MANAGEMENT
        BAST_PowerUp(channel, BAST_CORE_DISEQC);
#endif

        errCode = BAST_SendDiseqcCommand(channel, pSendData, sendDataSize);
        if ( errCode )
        {
            NEXUS_TaskCallback_Set(pDevice->diseqcAppCallback, NULL);
            return BERR_TRACE(errCode);
        }

        return BERR_SUCCESS;
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

static NEXUS_Error NEXUS_Frontend_P_Ast_ResetDiseqc( void *handle, const uint8_t options )
{
    NEXUS_AstDevice *pDevice = handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (pDevice->capabilities.diseqc)
    {
        BAST_ChannelHandle channel;

        if (pDevice->getDiseqcChannelHandle) {
            channel = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
        } else {
            channel = pDevice->astChannel;
        }

#if NEXUS_POWER_MANAGEMENT
        BAST_PowerUp(channel, BAST_CORE_DISEQC);
#endif

        errCode = BAST_ResetDiseqc(channel, options);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        return BERR_SUCCESS;
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetDiseqcReply( void *handle, NEXUS_FrontendDiseqcMessageStatus *pStatus, uint8_t *pReplyBuffer,
    size_t replyBufferSize, size_t *pReplyLength )
{
    NEXUS_AstDevice *pDevice = handle;
    BERR_Code errCode;
    BAST_DiseqcStatus status;
    BAST_ChannelHandle channel;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NULL != pReplyBuffer);
    BDBG_ASSERT(replyBufferSize > 0);
    BDBG_ASSERT(NULL != pReplyLength);

    if (pDevice->getDiseqcChannelHandle) {
        channel = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
    } else {
        channel = pDevice->astChannel;
    }

    NEXUS_TaskCallback_Set(pDevice->diseqcAppCallback, NULL);   /* just to be safe */

    BDBG_CASSERT((NEXUS_FrontendDiseqcMessageStatus)BAST_DiseqcSendStatus_eBusy == NEXUS_FrontendDiseqcMessageStatus_eBusy);
    errCode = BAST_GetDiseqcStatus(channel, &status);
    *pStatus = status.status;

    if (errCode || status.status != BAST_DiseqcSendStatus_eSuccess) {
        *pReplyLength = 0; /* indicate we wrote no bytes */
        return BERR_TRACE(errCode);
    }

    if (!status.bReplyExpected) {
        status.nReplyBytes = 0; /* force it */
    }

    *pReplyLength = replyBufferSize<status.nReplyBytes?replyBufferSize:status.nReplyBytes;
    if (*pReplyLength) {
        BKNI_Memcpy(pReplyBuffer, status.replyBuffer, *pReplyLength);
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_SendDiseqcAcw( void *handle, uint8_t codeWord )
{
    NEXUS_Error errCode;
    NEXUS_AstDevice *pDevice = handle;
    BAST_ChannelHandle channel;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (pDevice->getDiseqcChannelHandle) {
        channel = pDevice->getDiseqcChannelHandle(pDevice->getDiseqcChannelHandleParam, pDevice->diseqcIndex);
    } else {
        channel = pDevice->astChannel;
    }

    if (pDevice->capabilities.diseqc)
    {
        errCode = BAST_SendACW(channel, codeWord);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }


    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetSoftDecisions(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length)
{
#define TOTAL_AST_SOFTDECISIONS 15  /* What an odd number... */
    int j;
    size_t i;
    NEXUS_Error errCode;
    NEXUS_AstDevice *pDevice = handle;
    int16_t d_i[TOTAL_AST_SOFTDECISIONS], d_q[TOTAL_AST_SOFTDECISIONS];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    for( i=0; i<length; i += TOTAL_AST_SOFTDECISIONS )
    {
        errCode = BAST_GetSoftDecisionBuf(pDevice->astChannel, d_i, d_q);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        for ( j=0; j<TOTAL_AST_SOFTDECISIONS && i+j<length; j++ )
        {
            pDecisions[i+j].i = d_i[j] * 256 * 2;
            pDecisions[i+j].q = d_q[j] * 256 * 2;
        }
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_ReadSatelliteConfig( void *handle, unsigned id, void *buffer, unsigned bufferSize )
{
    NEXUS_AstDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    return BAST_ReadConfig(pDevice->astChannel, id, buffer, bufferSize);
}

static NEXUS_Error NEXUS_Frontend_P_Ast_WriteSatelliteConfig( void *handle, unsigned id, const void *buffer, unsigned bufferSize )
{
    NEXUS_AstDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    return BAST_WriteConfig(pDevice->astChannel, id, (void*)buffer, bufferSize);
}

/* Generic definitions required for peakscan and peakscan_psd.
 * If not available through the normal frontend includes, definitions added here.
 */

#ifndef BAST_G3_CONFIG_MISC_CTL
#define BAST_G3_CONFIG_MISC_CTL                  0x0001 /* miscellaneous acquisition settings, see BAST_G3_MISC_CTL_* macros */
#define BAST_G3_CONFIG_LEN_MISC_CTL                   1
#define BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD       0x08 /* enable PSD mode in PeakScan function */
#define BAST_G3_CONFIG_MISC_CTL_LOCAL_DEFINES         1
#endif

/* Helper function. Completion callback needs to already be set.
 * Divided to allow NEXUS_Frontend_P_Ast_StartSatellitePeakscanPsd to share peak scan start. */
static NEXUS_Error NEXUS_Frontend_P_Ast_StartSatellitePeakscan(void *handle, const NEXUS_FrontendSatellitePeakscanSettings *pSettings)
{
    NEXUS_AstDevice *pDevice = handle;
    NEXUS_SatellitePeakscanStatus *psStatus = &pDevice->peakscanStatus;
    BERR_Code errCode;
    uint8_t buf[8];

    BDBG_MSG(("NEXUS_Frontend_P_Ast_StartSatellitePeakscan"));

#if BCHP_CHIP ==7342 || BCHP_CHIP==7340 || BCHP_CHIP==7335 || BCHP_CHIP==7325
    /* set BLIND_SCAN_SYM_RATE_MIN configuration parameter */
    buf[0] = (uint8_t)((pSettings->minSymbolRate >> 24) & 0xFF);
    buf[1] = (uint8_t)((pSettings->minSymbolRate >> 16) & 0xFF);
    buf[2] = (uint8_t)((pSettings->minSymbolRate >> 8) & 0xFF);
    buf[3] = (uint8_t)(pSettings->minSymbolRate & 0xFF);
    errCode = BAST_WriteConfig(pDevice->astChannel, PEAK_SCAN_SYM_RATE_MIN, buf, LEN_PEAK_SCAN_SYM_RATE_MIN);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_WriteConfig(BLIND_SCAN_SYM_RATE_MIN) error %#x. Peak scan (blind scan) not initiated", errCode));
        return BERR_TRACE(errCode);
    }

    /* set BLIND_SCAN_SYM_RATE_MAX configuration parameter */
    buf[0] = (uint8_t)((pSettings->maxSymbolRate >> 24) & 0xFF);
    buf[1] = (uint8_t)((pSettings->maxSymbolRate >> 16) & 0xFF);
    buf[2] = (uint8_t)((pSettings->maxSymbolRate >> 8) & 0xFF);
    buf[3] = (uint8_t)(pSettings->maxSymbolRate & 0xFF);
    errCode = BAST_WriteConfig(pDevice->astChannel, PEAK_SCAN_SYM_RATE_MAX, buf, LEN_PEAK_SCAN_SYM_RATE_MAX);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_WriteConfig(BLIND_SCAN_SYM_RATE_MAX) error %#x. Peak scan (blind scan) not initiated", errCode));
        return BERR_TRACE(errCode);
    }
#else
    {
        uint8_t misc_ctl;
        BAST_ReadConfig(pDevice->astChannel, BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL);
        misc_ctl &= ~BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD;
        BAST_WriteConfig(pDevice->astChannel, BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL);
    }

    buf[0]=0;
    errCode = BAST_SetPeakScanSymbolRateRange(pDevice->astChannel,  pSettings->minSymbolRate, pSettings->maxSymbolRate );
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_WriteConfig(BLIND_SCAN_SYM_RATE_MAX) error %#x. Peak scan (blind scan) not initiated", errCode));
        return BERR_TRACE(errCode);
    }
#endif

    /* setup status variables */
    psStatus->curFreq = pSettings->frequency - pSettings->frequencyRange;
    psStatus->endFreq = pSettings->frequency + pSettings->frequencyRange;
    psStatus->stepFreq = pSettings->frequencyStep;
    psStatus->symRateCount = 0;
    psStatus->lastSymRate = 0;
    psStatus->maxPeakPower = 0;
    psStatus->maxPeakFreq = 0;
    psStatus->scanFinished = false;
    psStatus->singleScan = (pSettings->frequencyRange==0);

    if (psStatus->singleScan) {
        psStatus->endFreq = psStatus->curFreq+1;
    }

    pDevice->toneSearch = false;
    pDevice->psdSymbolSearch = false;
    errCode = BAST_PeakScan(pDevice->astChannel, psStatus->curFreq);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_PeakScan() error %#x. Peak scan not initiated", errCode));
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("Peak scan (blind scan) started at %uHz", psStatus->curFreq));

    /* the state machine is driven by pDevice->peakscanEventCallback and NEXUS_Frontend_P_Ast_PeakscanEventHandler */
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_SatellitePeakscan( void *handle, const NEXUS_FrontendSatellitePeakscanSettings *pSettings )
{
#if NEXUS_FRONTEND_73XX || NEXUS_FRONTEND_4506 || NEXUS_FRONTEND_7346 || NEXUS_FRONTEND_4538
    NEXUS_AstDevice *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (!pDevice->peakscanEvent) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

#if NEXUS_POWER_MANAGEMENT
    BAST_PowerUp(pDevice->astChannel, BAST_CORE_SDS);
#endif

    BDBG_MSG(("NEXUS_Frontend_P_Ast_SatellitePeakscan: %u %u %u  %u %u",
            pSettings->frequency, pSettings->frequencyRange, pSettings->frequencyStep,
            pSettings->minSymbolRate, pSettings->maxSymbolRate));

    if (pSettings->mode == NEXUS_FrontendSatellitePeakscanMode_ePowerSpectrumDensity)
        return NEXUS_Frontend_P_Ast_SatellitePeakscanPsd(handle, pSettings);

    if (pSettings->frequency - pSettings->frequencyRange < NEXUS_SATELLITE_PEAKSCAN_MIN_FREQ ||
        pSettings->frequency + pSettings->frequencyRange > NEXUS_SATELLITE_PEAKSCAN_MAX_FREQ) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* Clear any previous signal/tone searches, or an in-progress acquisition, if any. */
    BAST_AbortAcq(pDevice->astChannel);

    NEXUS_TaskCallback_Set(pDevice->peakscanAppCallback, &pSettings->peakscanCallback);

    return NEXUS_Frontend_P_Ast_StartSatellitePeakscan(handle, pSettings);

#else /* defined NEXUS_FRONTEND_73XX || defined NEXUS_FRONTEND_4506 || NEXUS_FRONTEND_7346 || NEXUS_FRONTEND_4538 */
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return NEXUS_NOT_SUPPORTED;
#endif
}

static NEXUS_Error NEXUS_Frontend_P_Ast_SatellitePeakscanPsd( void *handle, const NEXUS_FrontendSatellitePeakscanSettings *pSettings )
{
#if NEXUS_FRONTEND_4506 || NEXUS_FRONTEND_7346 || NEXUS_FRONTEND_4538
    NEXUS_AstDevice *pDevice = handle;
    NEXUS_SatellitePeakscanStatus *psStatus = &pDevice->peakscanStatus;
    BERR_Code errCode;
    uint8_t buf[8];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (!pDevice->peakscanEvent) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (pSettings->frequency - pSettings->frequencyRange < NEXUS_SATELLITE_PEAKSCAN_MIN_FREQ ||
        pSettings->frequency + pSettings->frequencyRange > NEXUS_SATELLITE_PEAKSCAN_MAX_FREQ) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    pDevice->peakscanSettings = *pSettings;

    /* Clear any previous signal/tone searches, or an in-progress acquisition, if any. */
    BAST_AbortAcq(pDevice->astChannel);

    NEXUS_TaskCallback_Set(pDevice->peakscanAppCallback, &pSettings->peakscanCallback);

    {
        uint8_t misc_ctl;
        BAST_ReadConfig(pDevice->astChannel, BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL);
        misc_ctl |= BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD;
        BAST_WriteConfig(pDevice->astChannel, BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL);
    }

    buf[0]=0;
    errCode = BAST_SetPeakScanSymbolRateRange(pDevice->astChannel, 0, 0);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_WriteConfig(BLIND_SCAN_SYM_RATE_MAX) error %#x. Peak scan (psd blind scan) not initiated", errCode));
        return BERR_TRACE(errCode);
    }

    /* setup status variables */
    psStatus->curFreq = pSettings->frequency - pSettings->frequencyRange;
    psStatus->endFreq = pSettings->frequency + pSettings->frequencyRange;
    psStatus->stepFreq = pSettings->frequencyStep;
    psStatus->symRateCount = 0;
    psStatus->lastSymRate = 0;
    psStatus->maxPeakPower = 0;
    psStatus->maxPeakFreq = 0;
    psStatus->scanFinished = false;
    psStatus->singleScan = (pSettings->frequencyRange==0);

    if (psStatus->singleScan) {
        psStatus->endFreq = psStatus->curFreq+1;
    }

    pDevice->toneSearch = false;
    pDevice->psdSymbolSearch = true;
    BKNI_Memset(&pDevice->psd,0,sizeof(pDevice->psd));
    errCode = BAST_PeakScan(pDevice->astChannel, psStatus->curFreq);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_PeakScan() error %#x. Peak scan not initiated", errCode));
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("Peak scan (psd blind scan) started at %uHz", psStatus->curFreq));

    /* the state machine is driven by pDevice->peakscanEventCallback and NEXUS_Frontend_P_Ast_PeakscanEventHandler */
    return NEXUS_SUCCESS;

#else /* defined NEXUS_FRONTEND_4506 || NEXUS_FRONTEND_7346 || NEXUS_FRONTEND_4538 */
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return NEXUS_NOT_SUPPORTED;
#endif
}

static NEXUS_Error NEXUS_Frontend_P_Ast_SatelliteToneSearch( void *handle, const NEXUS_FrontendSatelliteToneSearchSettings *pSettings )
{
#if NEXUS_FRONTEND_73XX || NEXUS_FRONTEND_4506 || NEXUS_FRONTEND_7346 || NEXUS_FRONTEND_4538
    NEXUS_AstDevice *pDevice = handle;
    NEXUS_SatellitePeakscanStatus *psStatus = &pDevice->peakscanStatus;
    BERR_Code errCode;
    uint8_t buf[8];

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (!pDevice->peakscanEvent) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

#if NEXUS_POWER_MANAGEMENT
    BAST_PowerUp(pDevice->astChannel, BAST_CORE_SDS);
#endif

    /* Clear any previous signal/tone searches, or an in-progress acquisition, if any. */
    BAST_AbortAcq(pDevice->astChannel);

    NEXUS_TaskCallback_Set(pDevice->peakscanAppCallback, &pSettings->completionCallback);

    {
        uint8_t misc_ctl;
        BAST_ReadConfig(pDevice->astChannel, BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL);
        misc_ctl &= ~BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD;
        BAST_WriteConfig(pDevice->astChannel, BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL);
    }

    buf[0]=0;
    errCode = BAST_SetPeakScanSymbolRateRange(pDevice->astChannel, 0, 0); /* tone search uses 0,0 */
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_WriteConfig() error %#x. Peak scan (tone search) not initiated", errCode));
        return BERR_TRACE(errCode);
    }

    /* setup status variables */
    psStatus->curFreq = pSettings->frequency - pSettings->frequencyRange;
    psStatus->endFreq = pSettings->frequency + pSettings->frequencyRange;
    psStatus->symRateCount = 0;
    psStatus->lastSymRate = 0;
    psStatus->maxPeakPower = 0;
    psStatus->minPeakPower = 0xFFFFFFFF;
    psStatus->maxPeakFreq = 0;
    psStatus->scanFinished = false;
    psStatus->minRatio.numerator = pSettings->minRatio.numerator;
    psStatus->minRatio.denominator = pSettings->minRatio.denominator;
    psStatus->singleScan = (pSettings->frequencyRange==0);

    {
        /* calculate the frequency step */
        BAST_ChannelStatus status;
        uint32_t fs;
        uint64_t p, q;
        BERR_Code rc;
        rc = BAST_GetChannelStatus(pDevice->astChannel, &status);
#define DFT_SIZE (512)
        fs = status.sample_clock;
        p = ((uint64_t)fs) * ((uint64_t)512);
        q = p / ((uint64_t)DFT_SIZE * (uint64_t)64);
        psStatus->binsize = ((int32_t)q)>>1; /* binsize is scaled 2^8 */
#undef DFT_SIZE
        psStatus->stepFreq = psStatus->binsize;
    }

    if (psStatus->singleScan) {
        psStatus->endFreq = psStatus->curFreq+1;
    }

    pDevice->toneSearch = true;
    pDevice->psdSymbolSearch = false;
    errCode = BAST_PeakScan(pDevice->astChannel, psStatus->curFreq);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(("BAST_PeakScan() error %#x. Peak scan (tone search) not initiated", errCode));
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("Peak scan (tone search) started at %uHz", psStatus->curFreq));

    /* the state machine is driven by pDevice->peakscanEventCallback and NEXUS_Frontend_P_Ast_PeakscanEventHandler */
    return NEXUS_SUCCESS;
#else /* defined NEXUS_FRONTEND_73XX || defined NEXUS_FRONTEND_4506 */
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return NEXUS_NOT_SUPPORTED;
#endif
}

#ifdef BAST_G3_CONFIG_MISC_CTL_LOCAL_DEFINES
#undef BAST_G3_CONFIG_MISC_CTL
#undef BAST_G3_CONFIG_LEN_MISC_CTL
#undef BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD
#undef BAST_G3_CONFIG_MISC_CTL_LOCAL_DEFINES
#endif

static NEXUS_Error NEXUS_Frontend_P_Ast_GetSatellitePeakscanResult( void *handle, NEXUS_FrontendSatellitePeakscanResult *pResult )
{
    NEXUS_AstDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (pDevice->peakscanEvent) {
        if (pDevice->peakscanStatus.scanFinished) {
            pResult->peakFrequency = pDevice->peakscanStatus.maxPeakFreq;
            pResult->symbolRate = pDevice->peakscanStatus.lastSymRate;
            pResult->lastFrequency= pDevice->peakscanStatus.curFreq;
            pResult->peakPower = pDevice->peakscanStatus.maxPeakPower;
            return NEXUS_SUCCESS;
        }
        else {
            return BERR_TRACE(NEXUS_NOT_INITIALIZED);
        }
    }
    else {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetSatelliteToneSearchResult( void *handle, NEXUS_FrontendSatelliteToneSearchResult *pResult )
{
    NEXUS_AstDevice *pDevice = handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (pDevice->peakscanEvent) {
        if (pDevice->peakscanStatus.scanFinished) {
            pResult->peakFrequency = pDevice->peakscanStatus.maxPeakFreq;
            pResult->lastFrequency= pDevice->peakscanStatus.curFreq;
            pResult->peakPower = pDevice->peakscanStatus.maxPeakPower;
            pResult->frequencyStep = pDevice->peakscanStatus.stepFreq;
            return NEXUS_SUCCESS;
        }
        else {
            return BERR_TRACE(NEXUS_NOT_INITIALIZED);
        }
    }
    else {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

static void NEXUS_Frontend_P_Ast_ResetStatus( void *handle )
{
    NEXUS_AstDevice *pDevice = handle;
#if NEXUS_POWER_MANAGEMENT
    BAST_PowerUp(pDevice->astChannel, BAST_CORE_SDS);
#endif
    BAST_ResetStatus(pDevice->astChannel);
}

BAST_ChannelHandle NEXUS_Frontend_P_Ast_GetChannel( NEXUS_FrontendHandle handle )
{
    NEXUS_AstDevice *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    if (handle->pParentFrontend) {
        handle = handle->pParentFrontend;
        BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    }
    pDevice = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    return pDevice->astChannel;
}

static void NEXUS_Frontend_P_Ast_LockEventHandler(void *pParam)
{
    NEXUS_AstDevice *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_MSG(("AST Lock Event"));

    BTRC_TRACE(ChnChange_TuneLock, STOP);
    NEXUS_TaskCallback_Fire(pDevice->lockAppCallback);
}

static void NEXUS_Frontend_P_Ast_DiseqcEventHandler(void *pParam)
{
    NEXUS_AstDevice *pDevice = pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_MSG(("AST Diseqc Event"));

    NEXUS_TaskCallback_Fire(pDevice->diseqcAppCallback);
}

static uint32_t NEXUS_Frontend_P_Ast_Log2Approx(int xint)
{
    int msb = 0;
    int log2Result;

    int OutputShift = 4; /* Log2 output has 4 LSB after the point. */

    if (xint <= 0)
        return 0;
    else {
        while (xint >> msb)
            msb++;
        msb--;

        if (msb - OutputShift > 0)
            log2Result = (xint >> (msb - OutputShift)) + ((msb - 1) << OutputShift);
        else
            log2Result = (xint << (OutputShift - msb)) + ((msb - 1) << OutputShift);

        return (log2Result);
    }
}

static uint32_t NEXUS_Frontend_P_Ast_Abs(int32_t x)
{
    if (x < 0)
        return (uint32_t)(1 - x);
    else
        return (uint32_t)x;
}

static void NEXUS_Frontend_P_Ast_PeakscanEventHandler(void *pParam)
{
    NEXUS_AstDevice *pDevice = pParam;
    BAST_PeakScanStatus astStatus;
    NEXUS_SatellitePeakscanStatus *psStatus = NULL;
    NEXUS_FrontendSatellitePeakscanSettings *psSettings = NULL;
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    psStatus = &pDevice->peakscanStatus;
    psSettings = &pDevice->peakscanSettings;

    if (psStatus->curFreq > psStatus->endFreq) {
        BDBG_WRN(("curFreq: %d > endFreq: %d, exiting scan",psStatus->curFreq,psStatus->endFreq));
        goto done;
    }

    if (pDevice->toneSearch) {

        /* get results of tone search scan */
        errCode = BAST_GetPeakScanStatus(pDevice->astChannel, &astStatus);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(("BAST_GetPeakScanStatus() (tone search) error %#x", errCode));
            errCode = BERR_TRACE(errCode);
            goto tone_step;
        }

        if (astStatus.status != 0) {
            BDBG_ERR(("BAST_GetPeakScanStatus() (tone search) scan status error %u", astStatus.status));
            goto tone_step;
        }

        BDBG_MSG(("%d Hz: Fb=%d, power=%#x, step=%d, min=%#x, max=%#x", astStatus.tunerFreq, astStatus.out, astStatus.peakPower, psStatus->stepFreq, psStatus->minPeakPower, psStatus->maxPeakPower));

        if (astStatus.peakPower > psStatus->maxPeakPower) { /* track min and max over the frequency range */
            psStatus->maxPeakPower = astStatus.peakPower;
            psStatus->maxPeakFreq = astStatus.tunerFreq;
            psStatus->maxPeakIndex = astStatus.out;
        }
        if (astStatus.peakPower < psStatus->minPeakPower) {
            psStatus->minPeakPower = astStatus.peakPower;
        }
        if (pDevice->peakscanStatus.singleScan) {
            psStatus->maxPeakIndex = astStatus.out;
            goto tone_done;
        }

tone_step:
        psStatus->curFreq += psStatus->stepFreq;

        if (psStatus->curFreq <= psStatus->endFreq) {
            errCode = BAST_PeakScan(pDevice->astChannel, psStatus->curFreq);
            if (errCode != BERR_SUCCESS) {
                BDBG_ERR(("BAST_PeakScan() (tone search) error %#x. Peak scan terminated", errCode));
                errCode = BERR_TRACE(errCode);
                goto done;
            }
            else {
                return;
            }
        }
        else {
#if 0
            if (psStatus->symRateCount > 0) {
                BDBG_WRN(("Potential signal found at %d Hz (%d sym/sec), but reached end of scan range",
                    psStatus->maxPeakFreq, psStatus->lastSymRate));
            }
            else {
                BDBG_WRN(("No signal found using peak scan"));
                psStatus->maxPeakFreq = 0;
                psStatus->lastSymRate = 0;
            }
#endif
            psStatus->curFreq -= psStatus->stepFreq;
        }
tone_done:
        /* calculate ratio */
        {
            uint64_t ratio, target_ratio;
#define TONE_SEARCH_RATIO_SCALE (1000)
            uint64_t max_pow = ((uint64_t)psStatus->maxPeakPower)*TONE_SEARCH_RATIO_SCALE;
            uint64_t min_pow = ((uint64_t)psStatus->minPeakPower);
            target_ratio = (((uint64_t)psStatus->minRatio.numerator)*TONE_SEARCH_RATIO_SCALE) / ((uint64_t)psStatus->minRatio.denominator);
            ratio = max_pow / min_pow;

            BDBG_MSG(("target_ratio: %u, min=" BDBG_UINT64_FMT ", max=" BDBG_UINT64_FMT " min=%#x, max=%#x", (unsigned)target_ratio, BDBG_UINT64_ARG(min_pow), BDBG_UINT64_ARG(max_pow), (unsigned)psStatus->minPeakPower, (unsigned)psStatus->maxPeakPower));
            if (max_pow > (min_pow * target_ratio)) {
                uint32_t binsize; /* 2^8 scale */
                uint64_t p, q;

                binsize = psStatus->binsize;
                p = ((uint64_t)psStatus->maxPeakIndex) * ((uint64_t)binsize);
                q = p / (uint64_t)256;
                psStatus->maxPeakFreq += (uint32_t)q;
            } else {
                psStatus->maxPeakFreq = 0;
                psStatus->lastSymRate = 0;
            }
        }

    } else if (pDevice->psdSymbolSearch) {
        static const int incrementThreshold  = 10;
        static const int inBandStepThreshold = 40;
        uint32_t val;
        uint32_t candidateBaudRate = 0;

        BDBG_MSG(("psdData_db[0]: %d, psdData_db[1]: %d, psdData_db[2]: %d",
                pDevice->psd.data_dB[0],
                pDevice->psd.data_dB[1],
                pDevice->psd.data_dB[2]
                ));
        BDBG_MSG(("psdFreq[0]: %d, psdFreq[1]: %d, psdFreq[2]: %d",
                pDevice->psd.freq[0],
                pDevice->psd.freq[1],
                pDevice->psd.freq[2]
                ));
        BDBG_MSG(("psdIndex: %d, risingFreq: %d, psdDataSize: %d",
                pDevice->psd.index,
                pDevice->psd.risingFreq,
                pDevice->psd.dataSize
                ));

        errCode = BAST_GetPeakScanStatus(pDevice->astChannel, &astStatus);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(("BAST_GetPeakScanStatus() (blind psd scan) error %#x", errCode));
            errCode = BERR_TRACE(errCode);
            goto blind_psd_step;
        }

        if (astStatus.status != 0) {
            BDBG_ERR(("BAST_GetPeakScanStatus() (blind psd scan) scan status error %u", astStatus.status));
            goto blind_psd_step;
        }

        BDBG_MSG(("%u Hz: Fb=%d, power=%#x", astStatus.tunerFreq, astStatus.out, astStatus.peakPower));

        pDevice->psd.dataSize++;
        pDevice->psd.freq[pDevice->psd.index] = psStatus->curFreq;
        val = NEXUS_Frontend_P_Ast_Log2Approx(astStatus.peakPower);
        pDevice->psd.raw_dB[pDevice->psd.rawIndex] = (12 * val) / 16;

        BDBG_MSG(("%u Hz: peak=%u, psd_dB=%u, raw_dB=%u, index=%d, risingType=%d, risingFreq=%u, freqPointIndex=%d",
                pDevice->psd.freq[pDevice->psd.index],
                astStatus.peakPower,
                pDevice->psd.data_dB[pDevice->psd.index],
                pDevice->psd.raw_dB[pDevice->psd.rawIndex],
                pDevice->psd.index,
                pDevice->psd.risingType,
                pDevice->psd.risingFreq,
                pDevice->psd.freqPointIndex
                ));

        if (pDevice->psd.dataSize >= 5)
        {
            int32_t  sortBuf[5];
            bool bSwapped = true;
            int i, j = 0, tmp;

            /* determine the median of the last 5 points */
            for (i = 0; i < 5; i++)
                sortBuf[i] = pDevice->psd.raw_dB[i];
            while (bSwapped)
            {
                bSwapped = false;
                j++;
                for (i = 0; i < (5 - j); i++)
                {
                    if (sortBuf[i] > sortBuf[i+1])
                    {
                        tmp = sortBuf[i];
                        sortBuf[i] = sortBuf[i+1];
                        sortBuf[i+1] = tmp;
                        bSwapped = true;
                    }
                }
            }
            pDevice->psd.data_dB[pDevice->psd.index] = sortBuf[2];
        }
        else
            pDevice->psd.data_dB[pDevice->psd.index] = 0;

        if (pDevice->psd.dataSize >= 3) {
            if (pDevice->psd.risingType > 0) {
                uint32_t step;
                /* already found a rising edge */
                pDevice->psd.freqPointIndex++;
                pDevice->psd.maxStep_D3 = pDevice->psd.maxStep_D2;
                pDevice->psd.maxStep_D2 = pDevice->psd.maxStep_D1;
                pDevice->psd.maxStep_D1 = pDevice->psd.maxStep;
                step = NEXUS_Frontend_P_Ast_Abs(pDevice->psd.data_dB[pDevice->psd.index] - pDevice->psd.data_dB[(pDevice->psd.index+2)%3]);
                if ((psStatus->stepFreq > pDevice->psd.maxStep) && (pDevice->psd.freqPointIndex > 3))
                    pDevice->psd.maxStep = step;
                if ((pDevice->psd.data_dB[(pDevice->psd.index+1)%3] - pDevice->psd.data_dB[pDevice->psd.index]) >= incrementThreshold) {
                    /* found a falling edge */
                    BDBG_MSG(("found a falling edge"));

                    candidateBaudRate = pDevice->psd.freq[pDevice->psd.index] - pDevice->psd.risingFreq + (2*psStatus->stepFreq);
                    if (candidateBaudRate >= 25000000)
                       candidateBaudRate -= 2500000;

                    BDBG_MSG(("found a falling edge: candidateBaudRate: %u, min: %u, max: %u, maxStep_D3: %u",
                            candidateBaudRate,
                            psSettings->minSymbolRate,
                            psSettings->maxSymbolRate,
                            pDevice->psd.maxStep_D3
                            ));

                    if ((candidateBaudRate >= psSettings->minSymbolRate) && (candidateBaudRate <= psSettings->maxSymbolRate) && (pDevice->psd.maxStep_D3 < (unsigned)inBandStepThreshold)) {
                        uint32_t minFb, maxFb, rangeFb, candidateCenterFreq;
                        BAST_ChannelStatus channelStatus;

found_candidate:
                        pDevice->psd.numCandidates++; /* for information only */
                        candidateCenterFreq = (pDevice->psd.freq[pDevice->psd.index] + pDevice->psd.risingFreq) / 2;
                        candidateCenterFreq -= (2*psStatus->stepFreq);

                        BDBG_MSG(("--> candidate found: Fc=%u, Fb=%u", candidateCenterFreq, candidateBaudRate));

                        errCode = BAST_GetChannelStatus(pDevice->astChannel, &channelStatus);
                        /* make sure we don't go below min_symbol_rate */
                        rangeFb = channelStatus.sample_clock / 128;
                        if (candidateBaudRate >= 15000000)
                            rangeFb *= 2;
                        if (candidateBaudRate >= 25000000)
                            rangeFb = 2000000;
                        else if (candidateBaudRate >= 5000000)
                           rangeFb = 1000000;
                        else
                           rangeFb = 500000;
                        if (rangeFb > candidateBaudRate)
                           minFb = psSettings->minSymbolRate;
                        else
                        {
                           minFb = candidateBaudRate - rangeFb;
                           if (minFb < psSettings->minSymbolRate)
                              minFb = psSettings->minSymbolRate;
                        }
                        maxFb = candidateBaudRate + rangeFb;
                        if (maxFb > psSettings->maxSymbolRate)
                            maxFb = psSettings->maxSymbolRate;
                        /* switch to peak scan */
                        {
                            NEXUS_FrontendSatellitePeakscanSettings peakscanSettings;
                            peakscanSettings.frequency = candidateCenterFreq;
                            peakscanSettings.frequencyStep = psStatus->stepFreq;
                            peakscanSettings.frequencyRange = 1000000;
                            peakscanSettings.minSymbolRate = minFb;
                            peakscanSettings.maxSymbolRate = maxFb;
                            /* re-use previous peakscanSettings.peakscanCallback */
                            errCode = NEXUS_Frontend_P_Ast_StartSatellitePeakscan(pDevice, &peakscanSettings);
                            return;
                        }
                    } else {
                        /* found a channel, but it is not valid, reset the rising edge type */
                        BDBG_MSG(("found a channel (Fb=%u), but it is not valid, reset the rising edge type", candidateBaudRate));
                        pDevice->psd.risingType = 0;
                        pDevice->psd.freqPointIndex = 0;
                    }
                } else {
                    /* already found a rising edge, no falling edge yet */
                    if ((pDevice->psd.data_dB[pDevice->psd.index] - pDevice->psd.data_dB[(pDevice->psd.index+1)%3]) >= incrementThreshold) {
                        /* found rising edge by crossing the incrementThreshold */
                        BDBG_MSG(("found rising edge by crossing the incrementThreshold"));
                        if ((pDevice->psd.freq[pDevice->psd.index] - pDevice->psd.risingFreq) <= 1000000) {
                            /* found a new rising edge that is 1MHz away from the existing risingType=1 edge */
                            goto found_new_rising_edge;
                        } else if ((pDevice->psd.freq[pDevice->psd.index] - pDevice->psd.risingFreq) >= 2000000) {
                           /* found a new rising edge 2MHz away without finding a falling edge;
                               for existing rising edge, assume it is also a falling edge */
                            BDBG_MSG(("found a new rising edge 2MHz away without finding a falling edge"));
                            candidateBaudRate = pDevice->psd.freq[pDevice->psd.index] - psStatus->stepFreq - pDevice->psd.risingFreq;
                            if ((candidateBaudRate >= psSettings->minSymbolRate) && (candidateBaudRate <= psSettings->maxSymbolRate) && (pDevice->psd.maxStep_D3 < (unsigned)inBandStepThreshold))
                                goto found_candidate;
                            pDevice->psd.risingType = 1;
                            pDevice->psd.risingFreq = pDevice->psd.freq[pDevice->psd.index];
                            pDevice->psd.freqPointIndex = 1;
                            pDevice->psd.maxStep = 0;
                        }
                    }
                }
            } else if ((pDevice->psd.data_dB[pDevice->psd.index] - pDevice->psd.data_dB[(pDevice->psd.index+1)%3]) >= incrementThreshold) {
found_new_rising_edge:
                BDBG_MSG(("found rising edge by crossing the incrementThreshold"));
                /* found rising edge by crossing the incrementThreshold */
                pDevice->psd.risingType = 1;
                pDevice->psd.risingFreq = pDevice->psd.freq[pDevice->psd.index];
                pDevice->psd.freqPointIndex = 1;
                pDevice->psd.maxStep = 0;
                pDevice->psd.maxStep_D1 = 0;
                pDevice->psd.maxStep_D2 = 0;
                pDevice->psd.maxStep_D3 = 0;
            }
        }
        pDevice->psd.index = (pDevice->psd.index + 1) % 3;
        pDevice->psd.rawIndex = (pDevice->psd.rawIndex + 1) % 5;
blind_psd_step:
        psStatus->curFreq += psStatus->stepFreq;

        if (psStatus->curFreq < psStatus->endFreq) {
            errCode = BAST_PeakScan(pDevice->astChannel, psStatus->curFreq);
            if (errCode != BERR_SUCCESS) {
                BDBG_ERR(("BAST_PeakScan() (blind scan) error %#x. Peak scan terminated", errCode));
                errCode = BERR_TRACE(errCode);
                goto done;
            }
            else {
                return;
            }
        }
        else {
            if (psStatus->symRateCount > 0) {
                BDBG_WRN(("Potential signal found at %d Hz (%d sym/sec), but reached end of scan range",
                    psStatus->maxPeakFreq, psStatus->lastSymRate));
            }
            else {
                BDBG_WRN(("No signal found using peak scan"));
                psStatus->maxPeakFreq = 0;
                psStatus->lastSymRate = 0;
            }
            psStatus->curFreq -= psStatus->stepFreq;
            goto done;
        }
    } else { /* standard symbol rate (peak) scan */

        /* get results of symbol rate scan */
        errCode = BAST_GetPeakScanStatus(pDevice->astChannel, &astStatus);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(("BAST_GetPeakScanStatus() (blind scan) error %#x", errCode));
            errCode = BERR_TRACE(errCode);
            goto blind_step;
        }

        if (astStatus.status != 0) {
            BDBG_ERR(("BAST_GetPeakScanStatus() (blind scan) scan status error %u", astStatus.status));
            goto blind_step;
        }

        BDBG_MSG(("%d Hz: Fb=%d, power=%#x", astStatus.tunerFreq, astStatus.out, astStatus.peakPower));

        if ((psStatus->symRateCount == 0) && ((astStatus.out == 0) || (astStatus.peakPower == 0)))
            goto blind_step;

        /* looser symbol rate match, per rockford/application/diags/satfe.c update */
        if ((astStatus.out - psStatus->lastSymRate) < (astStatus.out >> 9)) { /* approximately same symbol rate as last scan */
            psStatus->symRateCount++;
            if (psStatus->maxPeakPower < astStatus.peakPower) {
                psStatus->maxPeakPower = astStatus.peakPower;
                psStatus->maxPeakFreq = astStatus.tunerFreq;
            }
            if (pDevice->peakscanStatus.singleScan) {
                psStatus->lastSymRate = astStatus.out;
                goto done;
            }
        }
        else { /* symbol rate changed */
            if (psStatus->symRateCount > 0) {
                BDBG_MSG(("Potential signal found at %d Hz (%d sym/sec)", psStatus->maxPeakFreq, psStatus->lastSymRate));
                goto done;
            }
            else {
                /* save new symbol rate */
                psStatus->symRateCount = 0;
                psStatus->lastSymRate = astStatus.out;
                psStatus->maxPeakPower = astStatus.peakPower;
                psStatus->maxPeakFreq = astStatus.tunerFreq;
                if (pDevice->peakscanStatus.singleScan) {
                    goto done;
                }
            }
        }

blind_step:
        psStatus->curFreq += psStatus->stepFreq;

        if (psStatus->curFreq < psStatus->endFreq) {
            errCode = BAST_PeakScan(pDevice->astChannel, psStatus->curFreq);
            if (errCode != BERR_SUCCESS) {
                BDBG_ERR(("BAST_PeakScan() (blind scan) error %#x. Peak scan terminated", errCode));
                errCode = BERR_TRACE(errCode);
                goto done;
            }
            else {
                return;
            }
        }
        else {
            if (psStatus->symRateCount > 0) {
                BDBG_WRN(("Potential signal found at %d Hz (%d sym/sec), but reached end of scan range",
                    psStatus->maxPeakFreq, psStatus->lastSymRate));
            }
            else {
                BDBG_WRN(("No signal found using peak scan"));
                psStatus->maxPeakFreq = 0;
                psStatus->lastSymRate = 0;
            }
            psStatus->curFreq -= psStatus->stepFreq;
            goto done;
        }
    }

done:
    psStatus->scanFinished = true;
    NEXUS_TaskCallback_Fire(pDevice->peakscanAppCallback);

    return;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_RegisterExtension(NEXUS_FrontendHandle parentHandle, NEXUS_FrontendHandle extensionHandle)
{
    NEXUS_AstDevice *pDevice;

    BDBG_OBJECT_ASSERT(parentHandle, NEXUS_Frontend);
    pDevice = parentHandle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);

    if (extensionHandle == NULL) {
        extensionHandle = parentHandle;
    }
    else {
        BDBG_OBJECT_ASSERT(extensionHandle, NEXUS_Frontend);
    }

    /* recreate callbacks with the extension handle. this allows NEXUS_StopCallbacks to work. */
    if (pDevice->diseqcAppCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->diseqcAppCallback);
        pDevice->diseqcAppCallback = NEXUS_TaskCallback_Create(extensionHandle, NULL);
        if ( NULL == pDevice->diseqcAppCallback ) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }
    if (pDevice->peakscanAppCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->peakscanAppCallback);
        pDevice->peakscanAppCallback = NEXUS_TaskCallback_Create(extensionHandle, NULL);
        if ( NULL == pDevice->peakscanAppCallback ) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }
    if (pDevice->lockAppCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->lockAppCallback);
        pDevice->lockAppCallback = NEXUS_TaskCallback_Create(extensionHandle, NULL);
        if ( NULL == pDevice->lockAppCallback ) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }
    if (pDevice->ftmCallback) {
        NEXUS_TaskCallback_Destroy(pDevice->ftmCallback);
        pDevice->ftmCallback = NEXUS_TaskCallback_Create(extensionHandle, NULL);
        if ( NULL == pDevice->ftmCallback ) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
    }

    return 0;
}

/**
The following public API functions are implemented for AST only.
This means that a combo AST/SDS system will no longer work.
**/

NEXUS_AstDevice *NEXUS_Frontend_P_GetAstDeviceByChip(NEXUS_FrontendHandle handle, unsigned chipId)
{
    NEXUS_AstDevice *pDevice;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    if (handle->pParentFrontend) {
        handle = handle->pParentFrontend;
        BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    }
    if (handle->chip.id == chipId) {
        pDevice = handle->pDeviceHandle;
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
        return pDevice;
    }
    return NULL;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus )
{
    NEXUS_AstDevice *pDevice = handle;
    bool isLocked = false;
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_ASSERT(pStatus);
    pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnknown;
    rc = BAST_GetLockStatus(pDevice->astChannel, &isLocked);
    if (rc) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    /* AST only returns locked/unlocked, there is currently no "no signal" status. */
    pStatus->lockStatus = isLocked ? NEXUS_FrontendLockStatus_eLocked : NEXUS_FrontendLockStatus_eUnlocked;
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_Ast_ReadRegister(void *handle, unsigned address, uint32_t *pValue)
{
    NEXUS_AstDevice *astDevice = handle;
    BDBG_OBJECT_ASSERT(astDevice, NEXUS_AstDevice);
    if (astDevice) {
        return BAST_ReadRegister(astDevice->astChannel, address, pValue);
    }
    else {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

static NEXUS_Error NEXUS_Frontend_P_Ast_WriteRegister(void *handle, unsigned address, uint32_t value)
{
    NEXUS_AstDevice *astDevice = handle;
    BDBG_OBJECT_ASSERT(astDevice, NEXUS_AstDevice);
    if (astDevice) {
        return BAST_WriteRegister(astDevice->astChannel, address, &value);
    }
    else {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetSignalDetectStatus (void *handle, NEXUS_FrontendSatelliteSignalDetectStatus *pStatus)
{
    NEXUS_AstDevice *astDevice = handle;
    BDBG_OBJECT_ASSERT(astDevice, NEXUS_AstDevice);
    if (astDevice) {
        BERR_Code rc;
        BAST_SignalDetectStatus status;
        rc = BAST_GetSignalDetectStatus(astDevice->astChannel, &status);
        if (rc) {
            return BERR_TRACE(rc);
        } else {
            pStatus->enabled = status.bEnabled;
            pStatus->detectionComplete = status.bDone;
            pStatus->signalDetected = status.bTimingLoopLocked;
        }
    }
    else {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_Ast_GetType(void *handle, NEXUS_FrontendType *type)
{
    NEXUS_AstDevice *astDevice = handle;
    BDBG_OBJECT_ASSERT(astDevice, NEXUS_AstDevice);
    if (astDevice) {
        BKNI_Memcpy(type,(void *)&(astDevice->type),sizeof(*type));
    } else {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
}

static NEXUS_Error NEXUS_Frontend_P_Ast_GetBertStatus( void *handle, NEXUS_FrontendBertStatus *pStatus )
{
    NEXUS_Error errCode;
    BAST_ChannelStatus astStatus;
    NEXUS_AstDevice *pDevice = handle;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    errCode = BAST_GetChannelStatus(pDevice->astChannel, &astStatus);
    if ( errCode ) {
        /* if not locked, BAST_GetChannelStatus will fail. this is not an error. the status structure should just report not locked and no status available. */
        return 0;
    }

    pStatus->locked = astStatus.bBertLocked;
    pStatus->errorCount = (uint64_t)astStatus.berErrors;

    BDBG_WRN(("AST-based frontends can only return locked and errorCount from NEXUS_Frontend_GetBertStatus"));

    return NEXUS_SUCCESS;
}

/***************************************************************************
 * Module callback functions for I2C -- I2C Auto-synchronizes these.
 ***************************************************************************/
static BERR_Code NEXUS_Frontend_P_Ast_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length)
{
    NEXUS_AstDevice *pDevice = context;
    uint8_t dummy;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    return BAST_ReadMi2c(pDevice->astChannel, chipAddr<<1, &dummy, 0, pData, length);
}

static BERR_Code NEXUS_Frontend_P_Ast_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length)
{
    NEXUS_AstDevice *pDevice = context;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_AstDevice);
    return BAST_WriteMi2c(pDevice->astChannel, chipAddr<<1, (uint8_t *)((unsigned long)pData), length);
}

NEXUS_Error NEXUS_FrontendDevice_P_Ast_GetSatelliteCapabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCapabilities);

    BSTD_UNUSED(handle);

    /* Return empty values. Possibly hook up numChannels later. */
    BKNI_Memset(pCapabilities,0,sizeof(*pCapabilities));
    pCapabilities->numAdc = 0;
    pCapabilities->numChannels = 0;
    pCapabilities->externalBert = false;

    return rc;
}
