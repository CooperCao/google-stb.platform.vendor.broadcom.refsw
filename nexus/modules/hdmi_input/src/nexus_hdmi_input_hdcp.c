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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_hdmi_input_module.h"
#include "nexus_hdmi_input_hdcp.h"
#include "bhdr_hdcp.h"
#if NEXUS_HAS_SECURITY
#include "bhsm.h"
#include "bhsm_keyladder.h"
#include "breg_endian.h"
#if (NEXUS_SECURITY_API_VERSION==2)
#include "bhsm_hdcp1x.h"
#endif
#endif

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
#include "nexus_sage.h"
#include "priv/nexus_sage_priv.h"
#include "nexus_sage_types.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
#include "priv/nexus_sage_priv.h" /* get access to NEXUS_Sage_GetSageLib_priv() */
#include "bhdcplib_hdcp2x.h"
#include "bhdm_hdcp.h"
#endif


BDBG_MODULE(nexus_hdmi_input_hdcp);

#if NEXUS_HAS_SECURITY
#include "priv/nexus_security_priv.h"

#define LOCK_SECURITY() NEXUS_Module_Lock(g_NEXUS_hdmiInputModuleSettings.modules.security)
#define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_hdmiInputModuleSettings.modules.security)


#if !NEXUS_NUM_HDMI_INPUTS
#error Platform must define NEXUS_NUM_HDMI_INPUTS
#endif



extern NEXUS_ModuleHandle g_NEXUS_hdmiInputModule;
extern NEXUS_HdmiInputModuleSettings g_NEXUS_hdmiInputModuleSettings;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
#define LOCK_SAGE() NEXUS_Module_Lock(g_NEXUS_hdmiInputModuleSettings.modules.sage)
#define UNLOCK_SAGE() NEXUS_Module_Unlock(g_NEXUS_hdmiInputModuleSettings.modules.sage)


/* Hdcp 2.2 related Private APIs */
static void NEXUS_HdmiInput_P_Hdcp2xAuthenticationStatusUpdate(void *pContext);
static void NEXUS_HdmiInput_P_SageTATerminatedCallback_isr(void);
static void NEXUS_HdmiInput_P_SageWatchdogEventhandler(void *pContext);
static void NEXUS_HdmiInput_P_SageIndicationEventHandler(void *pContext);
static void NEXUS_HdmiInput_P_SageIndicationCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument,
    uint32_t indication_id,
    uint32_t value
);
static void NEXUS_HdmiInput_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
);
#endif



void NEXUS_HdmiInput_P_HdcpStateChange_isr(void *context, int param2, void *data)
{
    NEXUS_HdmiInputHandle hdmiInput ;
    BSTD_UNUSED(param2) ;
    BSTD_UNUSED(data) ;

    hdmiInput = (NEXUS_HdmiInputHandle) context;
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    NEXUS_IsrCallback_Fire_isr(hdmiInput->hdcpRxChanged) ;
}


static NEXUS_Error NEXUS_HdmiInput_P_HdcpKeyLoad(NEXUS_HdmiInputHandle hdmiInput)
{
#if (NEXUS_SECURITY_API_VERSION==1) /* diversify HDCP key loading */
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    BHSM_Handle hHsm ;
    uint8_t i;
    BHSM_EncryptedHdcpKeyStruct  EncryptedHdcpKeys;

    BDBG_ASSERT(g_NEXUS_hdmiInputModuleSettings.modules.security);

    LOCK_SECURITY();
	NEXUS_Security_GetHsm_priv(&hHsm);
	BDBG_LOG(("Begin Loading Encrypted keyset..."));

	EncryptedHdcpKeys.Alg = hdmiInput->hdcpKeyset.alg;
	EncryptedHdcpKeys.cusKeySel  = hdmiInput->hdcpKeyset.custKeySel;
	EncryptedHdcpKeys.cusKeyVarL = hdmiInput->hdcpKeyset.custKeyVarL;
	EncryptedHdcpKeys.cusKeyVarH = hdmiInput->hdcpKeyset.custKeyVarH;

	for (i = 0; i < NEXUS_HDMI_HDCP_NUM_KEYS  ; i++ )
	{
            EncryptedHdcpKeys.HdcpKeyLo  = hdmiInput->hdcpKeyset.privateKey[i].hdcpKeyLo;
            EncryptedHdcpKeys.HdcpKeyHi  = hdmiInput->hdcpKeyset.privateKey[i].hdcpKeyHi;
            errCode = BHSM_FastLoadEncryptedHdcpKey(hHsm, i, &EncryptedHdcpKeys ) ;

            if (errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BHSM_FastLoadEncryptedHdcpKey errCode: %x", errCode )) ;
                BERR_TRACE(errCode) ;
                break ;
            }
            BDBG_MSG(("Loaded Encrypted Key  %02d of %d  %#08x%08x",
                i + 1, BAVC_HDMI_HDCP_N_PRIVATE_KEYS,
                hdmiInput->hdcpKeyset.privateKey[i].hdcpKeyHi,
                hdmiInput->hdcpKeyset.privateKey[i].hdcpKeyLo)) ;
        }

        BDBG_LOG(("Done  Loading Encrypted keyset...")) ;

    UNLOCK_SECURITY();
    return errCode ;
#else
  #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,0)
    BERR_Code rc = BERR_SUCCESS;
    BHSM_Hdcp1xRouteKey hdcpConf;
    BHSM_Handle hHsm ;
    unsigned i = 0;
    uint32_t otpKeyIndex;

    LOCK_SECURITY();
    NEXUS_Security_GetHsm_priv(&hHsm);
    UNLOCK_SECURITY();

    BKNI_Memset( &hdcpConf, 0, sizeof(hdcpConf) );
    otpKeyIndex = hdmiInput->hdcpKeyset.privateKey[0].caDataLo;
    BREG_LE32( otpKeyIndex );

    hdcpConf.algorithm = BHSM_CryptographicAlgorithm_e3DesAba;
    hdcpConf.root.type = BHSM_KeyLadderRootType_eOtpDirect;
    hdcpConf.root.otpKeyIndex = otpKeyIndex;

    for( i =0; i< NEXUS_HDMI_HDCP_NUM_KEYS; i++ )
    {
        hdcpConf.hdcpKeyIndex = i;
        hdcpConf.key.high = hdmiInput->hdcpKeyset.privateKey[i].hdcpKeyHi;
        hdcpConf.key.low  = hdmiInput->hdcpKeyset.privateKey[i].hdcpKeyLo;

        LOCK_SECURITY();
        rc = BHSM_Hdcp1x_RouteKey( hHsm, &hdcpConf );
        UNLOCK_SECURITY();

        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    }

    return BERR_SUCCESS;
  #else
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return BERR_SUCCESS;
  #endif
#endif
}


/**
Summary:
Get HDCP Default Keyset
**/
void NEXUS_HdmiInput_HdcpGetDefaultKeyset(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHdcpKeyset *pKeyset /* [out] */
    )
{
	static const uint8_t hdcpSpecB1RxBksv[NEXUS_HDMI_HDCP_KSV_LENGTH] =
		{0x14, 0xF7, 0x61, 0x03, 0xB7} ;

	static const
		NEXUS_HdmiInputHdcpKey hdcpSpecB1RxKeySet[NEXUS_HDMI_HDCP_NUM_KEYS] =
	{
			/* LSB.. MSB */
			{ 0, 0, 0, 0, 0x691e138f, 0x58a44d00},		{ 0, 0, 0, 0, 0x0950e658, 0x35821f00},
			{ 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00},		{ 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300},
			{ 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00},		{ 0, 0, 0, 0, 0x818f4878, 0xc98be000},
			{ 0, 0, 0, 0, 0x412c11c8, 0x64d0a000},		{ 0, 0, 0, 0, 0x44202428, 0x5a9db300},
			{ 0, 0, 0, 0, 0x6b56adbd, 0xb228b900},		{ 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100},
			{ 0, 0, 0, 0, 0x589d5e20, 0xf8005600},		{ 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00},
			{ 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000},		{ 0, 0, 0, 0, 0x059f4be5, 0x61125600},
			{ 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400},		{ 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00},
			{ 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000},		{ 0, 0, 0, 0, 0x98547846, 0x8e2f4800},
			{ 0, 0, 0, 0, 0x38472762, 0x25ae6600},		{ 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00},
			{ 0, 0, 0, 0, 0x543a7b76, 0x31d2e200},		{ 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00},
			{ 0, 0, 0, 0, 0xf7227bbf, 0x82603200},		{ 0, 0, 0, 0, 0x6bce3035, 0x461bf600},
			{ 0, 0, 0, 0, 0x6b97d7f0, 0x09043600},		{ 0, 0, 0, 0, 0xf9498d61, 0x05e1a100},
			{ 0, 0, 0, 0, 0x063405d1, 0x9d8ec900},		{ 0, 0, 0, 0, 0x90614294, 0x67c32000},
			{ 0, 0, 0, 0, 0xc34facce, 0x51449600},		{ 0, 0, 0, 0, 0x8a8ce104, 0x45903e00},
			{ 0, 0, 0, 0, 0xfc2d9c57, 0x10002900},		{ 0, 0, 0, 0, 0x80b1e569, 0x3b94d700},
			{ 0, 0, 0, 0, 0x437bdd5b, 0xeac75400},		{ 0, 0, 0, 0, 0xba90c787, 0x58fb7400},
			{ 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300},		{ 0, 0, 0, 0, 0xae119a15, 0x5e070300},
			{ 0, 0, 0, 0, 0x01fb788a, 0x40d30500},		{ 0, 0, 0, 0, 0xb34da0d7, 0xa5590000},
			{ 0, 0, 0, 0, 0x409e2c4a, 0x633b3700},		{ 0, 0, 0, 0, 0x412056b4, 0xbb732500}
	} ;


    BSTD_UNUSED(hdmiInput);

    BKNI_Memset(pKeyset, 0, sizeof(NEXUS_HdmiInputHdcpKeyset)) ;

    /* following should be set by app */
    pKeyset->alg = 1 ;
    pKeyset->custKeySel = 0 ;
    pKeyset->custKeyVarH = 0 ;
    pKeyset->custKeyVarL = 0 ;

    /* copy HDCP Test Key Set from HDCP Specification  to default Key Set */
    /* !!! FYI This Test Key Set *IS NOT* compatible with production transmitter */
    BKNI_Memcpy(pKeyset->privateKey, hdcpSpecB1RxKeySet,
        sizeof(NEXUS_HdmiInputHdcpKey) * NEXUS_HDMI_HDCP_NUM_KEYS) ;

    /* copy HDCP BKsv from HDCP Specification to default Bksv */
    BKNI_Memcpy(&pKeyset->rxBksv, hdcpSpecB1RxBksv, NEXUS_HDMI_HDCP_KSV_LENGTH) ;

}


/**
Summary:
Initialize/Load HDCP Key Set
**/
NEXUS_Error NEXUS_HdmiInput_HdcpSetKeyset(
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_HdmiInputHdcpKeyset *pKeyset
    )
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    BHDR_HDCP_Status hdcpStatus ;
    BHDR_HDCP_Settings hdcpSettings ;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    /* Install callbacks */
    BDBG_ASSERT(pKeyset) ;

    BDBG_CASSERT(BAVC_HDMI_HDCP_KSV_LENGTH == NEXUS_HDMI_HDCP_KSV_LENGTH);
    BDBG_CASSERT(BAVC_HDMI_HDCP_N_PRIVATE_KEYS == NEXUS_HDMI_HDCP_NUM_KEYS);
    BDBG_CASSERT(sizeof(BHDR_HDCP_EncryptedKeyStruct) == sizeof(NEXUS_HdmiInputHdcpKey));


    /* update the HDCP BKSV value */

    BKNI_Memcpy(&hdmiInput->hdcpKeyset.rxBksv,
	    pKeyset->rxBksv.data, BAVC_HDMI_HDCP_KSV_LENGTH);

    /* update the HDCP Private keyset value */
    BKNI_Memcpy(hdmiInput->hdcpKeyset.privateKey,
	    pKeyset->privateKey,
	    sizeof( NEXUS_HdmiInputHdcpKey) * NEXUS_HDMI_HDCP_NUM_KEYS) ;

    hdmiInput->hdcpKeyset.alg = pKeyset->alg ;
    hdmiInput->hdcpKeyset.custKeySel = pKeyset->custKeySel ;
    hdmiInput->hdcpKeyset.custKeyVarH = pKeyset->custKeyVarH ;
    hdmiInput->hdcpKeyset.custKeyVarL = pKeyset->custKeyVarL ;

    /* use storage type to determine load type */
    BHDR_HDCP_GetStatus(hdmiInput->hdr, &hdcpStatus) ;


    BHDR_HDCP_GetSettings(hdmiInput->hdr, &hdcpSettings) ;
        BKNI_Memcpy(&hdcpSettings.rxBksv, &hdmiInput->hdcpKeyset.rxBksv,
            NEXUS_HDMI_HDCP_KSV_LENGTH) ;
    BHDR_HDCP_SetSettings(hdmiInput->hdr, &hdcpSettings) ;

   /* always reload HDCP Keys whenever requested */

    BHDR_HDCP_EnableKeyLoading(hdmiInput->hdr) ;

    switch (hdcpStatus.eKeyStorage)
    {
    case NEXUS_HdmiInputHdcpKeyStorage_eOtpRAM :
        /* load Keyset into OTP RAM  */
        errCode = NEXUS_HdmiInput_P_HdcpKeyLoad(hdmiInput) ;
        break ;

    case NEXUS_HdmiInputHdcpKeyStorage_eOtpROM :
        /* keys are already loaded in OTP ROM; nothing to do */
        errCode = NEXUS_SUCCESS ;
        break ;

    default :
        errCode = NEXUS_UNKNOWN ;
        BDBG_ERR(("Unknown HDCP Key Storage... unable to load keyset")) ;
    }

    /* disable key loading if key loading was successful */
    if (!errCode)
	    BHDR_HDCP_DisableKeyLoading(hdmiInput->hdr) ;

    return errCode ;
}




/**
Summary:
Get HDCP Default Settings
**/
void NEXUS_HdmiInput_HdcpGetDefaultSettings(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHdcpSettings *pSettings /* [out] */
    )
{
    BHDR_HDCP_Settings hdcpDefaultSettings ;

    BSTD_UNUSED(hdmiInput);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_HdmiInputHdcpSettings)) ;

    BHDR_HDCP_GetDefaultSettings(&hdcpDefaultSettings) ;

    BDBG_MSG(("Default HDCP Repeater Setting %d", hdcpDefaultSettings.bRepeater)) ;

    pSettings->repeater = hdcpDefaultSettings.bRepeater ;
    pSettings->maxDepthSupported = hdcpDefaultSettings.uiMaxLevels ;
    pSettings->maxDeviceCountSupported = hdcpDefaultSettings.uiMaxDevices ;
    NEXUS_CallbackDesc_Init(&pSettings->hdcpRxChanged) ;
}


void NEXUS_HdmiInput_HdcpGetSettings(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHdcpSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = hdmiInput->hdcpSettings;
}


/**
Summary:
Set HDCP Settings
**/
NEXUS_Error NEXUS_HdmiInput_HdcpSetSettings(
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_HdmiInputHdcpSettings *pSettings
    )
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    BERR_Code rc ;
    BHDR_HDCP_Status hdcpStatus ;
    BHDR_HDCP_Settings hdcpSettings ;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_ASSERT(NULL != pSettings);

    /* Install callbacks */
    BDBG_ASSERT(pSettings) ;

    BDBG_CASSERT(BAVC_HDMI_HDCP_KSV_LENGTH == NEXUS_HDMI_HDCP_KSV_LENGTH);
    BDBG_CASSERT(BAVC_HDMI_HDCP_N_PRIVATE_KEYS == NEXUS_HDMI_HDCP_NUM_KEYS);
    BDBG_CASSERT(sizeof(BHDR_HDCP_EncryptedKeyStruct) == sizeof(NEXUS_HdmiInputHdcpKey));

    BKNI_Memset(&hdcpStatus, 0, sizeof(BHDR_HDCP_Status)) ;

    NEXUS_IsrCallback_Set(hdmiInput->hdcpRxChanged, &pSettings->hdcpRxChanged) ;

    rc = BHDR_HDCP_InstallHdcpStatusChangeCallback(hdmiInput->hdr,
    	NEXUS_HdmiInput_P_HdcpStateChange_isr, hdmiInput, 0);
    if (rc) {BERR_TRACE(rc); goto error;}


    /* enable key loading as needed */
    BHDR_HDCP_GetSettings(hdmiInput->hdr, &hdcpSettings) ;
        BKNI_Memcpy(&hdcpSettings.rxBksv, &hdmiInput->hdcpKeyset.rxBksv,
            NEXUS_HDMI_HDCP_KSV_LENGTH) ;
        hdcpSettings.bRepeater = pSettings->repeater ;
		hdcpSettings.uiMaxDevices = pSettings->maxDeviceCountSupported;
		hdcpSettings.uiMaxLevels = pSettings->maxDepthSupported;
    BHDR_HDCP_SetSettings(hdmiInput->hdr, &hdcpSettings) ;

    BKNI_Memcpy(&hdmiInput->hdcpSettings, pSettings, sizeof(NEXUS_HdmiInputHdcpSettings)) ;

error:
    return errCode ;
}


/**
Summary:
Get HDCP Status
**/
NEXUS_Error NEXUS_HdmiInput_HdcpGetStatus(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHdcpStatus *pStatus /* [out] */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BHDR_HDCP_Status stHdrStatus;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_ASSERT(NULL != pStatus);

    BDBG_CASSERT(BHDR_HDCP_OtpState_eMax == (BHDR_HDCP_OtpState)NEXUS_HdmiInputHdcpKeySetOtpState_eMax) ;
    BDBG_CASSERT(BHDR_HDCP_Version_eMax == (BHDR_HDCP_Version)NEXUS_HdcpVersion_eMax) ;
    BDBG_CASSERT(BHDR_HDCP_KeyStorage_eMax == (BHDR_HDCP_KeyStorage)NEXUS_HdmiInputHdcpKeyStorage_eMax) ;

    rc = BHDR_HDCP_GetStatus(hdmiInput->hdr, &stHdrStatus) ;
    if (rc != BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto done;
    }

    pStatus->version = (NEXUS_HdcpVersion) stHdrStatus.eVersion;
    pStatus->hdcpState = NEXUS_HdmiInputHdcpState_eUnknown;	/* default for 1.x */
    pStatus->eKeyStorage = (NEXUS_HdmiInputHdcpKeyStorage) stHdrStatus.eKeyStorage;
    pStatus->eOtpState = (NEXUS_HdmiInputHdcpKeySetOtpState) stHdrStatus.eOtpState;

    pStatus->eAuthState = (NEXUS_HdmiInputHdcpAuthState) stHdrStatus.eAuthState;

    pStatus->programmedCrc = stHdrStatus.programmedCrc;
    pStatus->computedCrc = stHdrStatus.calculatedCrc;
    BKNI_Memcpy(pStatus->anValue, stHdrStatus.anValue, NEXUS_HDMI_HDCP_AN_LENGTH);
    BKNI_Memcpy(pStatus->aksvValue, stHdrStatus.aksvValue, NEXUS_HDMI_HDCP_KSV_LENGTH);
    BKNI_Memcpy(pStatus->bksvValue, stHdrStatus.bksvValue, NEXUS_HDMI_HDCP_KSV_LENGTH);

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
    {
        BHDCPlib_Hdcp2x_AuthenticationStatus stAuthenticationStatus;

        rc = BHDCPlib_Hdcp2x_GetAuthenticationStatus(hdmiInput->hdcpHandle, &stAuthenticationStatus);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("Error retrieving HDCP2.x authentication status"));
            rc = BERR_TRACE(rc);
            goto done;
        }
        pStatus->hdcp2xContentStreamControl = stAuthenticationStatus.eContentStreamTypeFromUpstream;

        if (stAuthenticationStatus.eHdcpState == BHDCPlib_State_eReceiverAuthenticated)
        {
            pStatus->hdcpState = NEXUS_HdmiInputHdcpState_eAuthenticated;
        }
        else if ((stAuthenticationStatus.eHdcpState == BHDCPlib_State_eEncryptionEnabled)
            || (stAuthenticationStatus.eHdcpState == BHDCPlib_State_eLinkAuthenticated))
        {
            pStatus->hdcpState = NEXUS_HdmiInputHdcpState_eRepeaterAuthenticated;
        }
        else {
            pStatus->hdcpState = NEXUS_HdmiInputHdcpState_eUnauthenticated;
        }
    }
#endif

done:

    return rc;
}

/**
Summary:
Load Downstream KSV FIFO
**/
NEXUS_Error NEXUS_HdmiInput_HdcpLoadKsvFifo(
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_HdmiHdcpDownStreamInfo *pDownstreamInfo,
    const NEXUS_HdmiHdcpKsv *pDownstreamKsvs, /* attr{nelem=numDevices} */
    unsigned numDevices
	)
{
    NEXUS_Error rc = NEXUS_SUCCESS ;
    BDBG_CASSERT(sizeof(BHDR_HDCP_RepeaterDownStreamInfo) == sizeof(NEXUS_HdmiHdcpDownStreamInfo)) ;

    /* warn of numDevices/ pDowstreamInfo inconsistency */
    if (numDevices != pDownstreamInfo->devices)
    {
        BDBG_WRN((" !!! ")) ;
        BDBG_WRN(("numDevices %d is not equal to pDownstreamInfo->devices %d",
            numDevices, pDownstreamInfo->devices)) ;
        BDBG_WRN((" !!! ")) ;
    }

#if BHDR_CONFIG_HDCP_REPEATER
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
    {
        BHDR_HDCP_Status stHdcpStatus;

        rc = BHDR_HDCP_GetStatus(hdmiInput->hdr, &stHdcpStatus) ;
        if (rc != BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            goto done;
        }

        /* Authenticate with upstream using HDCP 2.x */
        if (stHdcpStatus.eVersion == BHDR_HDCP_Version_e2x)
        {
            BHDCPlib_ReceiverIdListData hdcp2xReceiverIdListData;

            hdcp2xReceiverIdListData.deviceCount = (uint8_t) pDownstreamInfo->devices;
            hdcp2xReceiverIdListData.depth = (uint8_t) pDownstreamInfo->depth;
            hdcp2xReceiverIdListData.maxDevsExceeded = (uint8_t) pDownstreamInfo->maxDevicesExceeded;
            hdcp2xReceiverIdListData.maxCascadeExceeded = (uint8_t) pDownstreamInfo->maxDepthExceeded;
            hdcp2xReceiverIdListData.hdcp2LegacyDeviceDownstream = 0;
            hdcp2xReceiverIdListData.hdcp1DeviceDownstream = 1; /* yes */
            hdcp2xReceiverIdListData.downstreamIsRepeater = (uint8_t) pDownstreamInfo->isRepeater;

            if (pDownstreamInfo->devices >= BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT)
            {
                /* ignore the rest of the list, if any */
                BKNI_Memcpy(hdcp2xReceiverIdListData.rxIdList, &pDownstreamKsvs,
                            BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
            }
            else
            {
                /*****************
                * If downstream device is repeater device, the ReceiverIdList should contain
                * contain 1 additional entry, which is the ReceiverId of the repeater device.
                * This additional entry is not accounted in the device count
                *******************/
                BKNI_Memcpy(hdcp2xReceiverIdListData.rxIdList, &pDownstreamKsvs,
                    pDownstreamInfo->devices*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);

                if (pDownstreamInfo->isRepeater) {
                    BKNI_Memcpy(hdcp2xReceiverIdListData.rxIdList + pDownstreamInfo->devices*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH,
                        &pDownstreamInfo->repeaterKsv, BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
                }
            }

            /* upload HDCP 2.x receiverId List */
            rc = BHDCPlib_Hdcp2x_Rx_UploadReceiverIdList(hdmiInput->hdcpHandle, &hdcp2xReceiverIdListData);
            if (rc != BERR_SUCCESS)
            {
                BDBG_ERR(("Error (%d) uploading ReceiverIdList", rc));
                rc = BERR_TRACE(rc);
            }

            goto done;  /* skip the rest */
        }
        else {
            /* fall through */
        }
    }
#endif
		/* upload HDCP 1.x KSV FIFO */
        BHDR_HDCP_LoadRepeaterKsvFifo(hdmiInput->hdr,
            (BHDR_HDCP_RepeaterDownStreamInfo *) pDownstreamInfo,
            (uint8_t *) pDownstreamKsvs) ;
		goto done;
#else
        BDBG_WRN(("HDCP Repeater Support is DISABLED!!!")) ;
        BSTD_UNUSED(hdmiInput) ;
        BSTD_UNUSED(pDownstreamInfo) ;
        BSTD_UNUSED(pDownstreamKsvs) ;
		goto done;
#endif

done:

    return rc ;
}


#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
NEXUS_Error NEXUS_HdmiInput_LoadHdcp2xReceiverIdList_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_Hdcp2xReceiverIdListData *pData,
    bool downstreamIsRepeater
)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    BHDR_HDCP_Status stHdcpStatus;
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    errCode = BHDR_HDCP_GetStatus(hdmiInput->hdr, &stHdcpStatus) ;
    if (errCode != BERR_SUCCESS) {
        errCode = BERR_TRACE(errCode);
        goto done;
    }

    /* Authenticate with upstream using HDCP 2.x */
    if (stHdcpStatus.eVersion == BHDR_HDCP_Version_e2x)
    {
        BHDCPlib_ReceiverIdListData hdcp2xReceiverIdListData;

        hdcp2xReceiverIdListData.deviceCount = (uint8_t) pData->deviceCount;
        hdcp2xReceiverIdListData.depth = (uint8_t) pData->depth;
        hdcp2xReceiverIdListData.maxDevsExceeded = (uint8_t) pData->maxDevsExceeded;
        hdcp2xReceiverIdListData.maxCascadeExceeded = (uint8_t) pData->maxCascadeExceeded;
        hdcp2xReceiverIdListData.hdcp2LegacyDeviceDownstream = (uint8_t) pData->hdcp2xLegacyDeviceDownstream;
        hdcp2xReceiverIdListData.hdcp1DeviceDownstream = (uint8_t) pData->hdcp1DeviceDownstream;
        hdcp2xReceiverIdListData.downstreamIsRepeater = (uint8_t) downstreamIsRepeater;

        if (pData->deviceCount + hdcp2xReceiverIdListData.downstreamIsRepeater >= BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT)
        {
            /* ignore the rest of the list, if any */
            BKNI_Memcpy(hdcp2xReceiverIdListData.rxIdList, &pData->rxIdList,
                        BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
        }
        else
        {
            /*****************
            * If downstream device is repeater device, the ReceiverIdList will
            * contain 1 additional entry, which is the ReceiverId of the repeater device.
            * This additional entry is not accounted in the device count
            *******************/
            BKNI_Memcpy(hdcp2xReceiverIdListData.rxIdList, &pData->rxIdList,
                (pData->deviceCount + hdcp2xReceiverIdListData.downstreamIsRepeater)*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
        }

        /* upload receiverId List */
        errCode = BHDCPlib_Hdcp2x_Rx_UploadReceiverIdList(hdmiInput->hdcpHandle, &hdcp2xReceiverIdListData);
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("Error (%d) uploading ReceiverIdList", errCode));
            errCode = BERR_TRACE(errCode);
            goto done;
        }
    }
    else {
        NEXUS_HdmiHdcpDownStreamInfo downStream  ;
        NEXUS_HdmiHdcpKsv *pKsvs = NULL ;

        /* populate downstream info */
        downStream.devices = (uint8_t) pData->deviceCount;
        downStream.depth = (uint8_t) pData->depth;
        downStream.maxDevicesExceeded = (uint8_t) pData->maxDevsExceeded;
        downStream.maxDepthExceeded = (uint8_t) pData->maxCascadeExceeded;
        downStream.isRepeater = downstreamIsRepeater;

        /* allocate space to hold ksvs for the downstream devices */
        pKsvs = BKNI_Malloc((downStream.devices) * NEXUS_HDMI_HDCP_KSV_LENGTH) ;
        if (pKsvs == NULL) {
            BDBG_ERR(("Error allocating memory for KSV FIFO"));
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto done;
        }

        if (pData->deviceCount >= BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT)
        {
            BKNI_Memset(downStream.repeaterKsv.data, 0, sizeof(NEXUS_HDMI_HDCP_KSV_LENGTH));
            BKNI_Memcpy(pKsvs, &pData->rxIdList, BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
        }
        else
        {
            /*****************
            * If downstream device is repeater device, the ReceiverIdList will
            * contain 1 additional entry, which is the ReceiverId of the repeater device.
            * This additional entry is not accounted in the device count
            *******************/
            BKNI_Memcpy(downStream.repeaterKsv.data,
                &pData->rxIdList[pData->deviceCount*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH], NEXUS_HDMI_HDCP_KSV_LENGTH);
            BKNI_Memcpy(pKsvs, &pData->rxIdList, pData->deviceCount * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
        }

        errCode = NEXUS_HdmiInput_HdcpLoadKsvFifo(hdmiInput, &downStream, pKsvs, downStream.devices) ;
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("Error (%d) uploading KSV FIFO list", errCode));
            errCode = BERR_TRACE(errCode);
            if (pKsvs) BKNI_Free(pKsvs) ;
            goto done;
        }

        if (pKsvs) BKNI_Free(pKsvs) ;
    }

done:

    return errCode ;
}


NEXUS_Error NEXUS_HdmiInput_UpdateHdcp2xRxCaps_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    bool downstreamDeviceAttached
   )
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

	/* Set RxCaps HW register to use later in the authentication process */
	errCode = BHDR_HDCP_SetHdcp2xRxCaps(hdmiInput->hdr, BHDCPLIB_HDCP22_RXCAPS_VERSION,
						BHDCPLIB_HDCP22_RXCAPS_RECEIVER_CAPABILITY_MASK, downstreamDeviceAttached?1:0);
	if (errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Error setting RxCaps"));
		errCode = BERR_TRACE(errCode);
		goto done;
	}

done:

	return errCode;
}


NEXUS_Error NEXUS_HdmiInput_LoadHdcpTA_priv(
    void *buf, size_t length
)
{
    NEXUS_Error rc = BERR_SUCCESS ;

    /* initialize */
    g_hdmiInputTABlock.buf = NULL;
    g_hdmiInputTABlock.len = 0;

#if SAGE_VERSION >= SAGE_VERSION_CALC(3,0)
{
    NEXUS_SageMemoryBlock blk = {0};
    NEXUS_SageImageHolder holder = {"HDMIRX FC", SAGE_IMAGE_FirmwareID_eSage_HDMIRX_FC, NULL};

    BDBG_LOG(("%s: allocate %u bytes for HDCP_TA buffer", BSTD_FUNCTION, (unsigned)length));
    if(length != 0)
    {
        /* use SAGE allocator */
        LOCK_SAGE();
        g_hdmiInputTABlock.buf = NEXUS_Sage_Malloc_priv(length);
        UNLOCK_SAGE();
        if (g_hdmiInputTABlock.buf == NULL) {
            rc = BERR_OUT_OF_DEVICE_MEMORY;
            BDBG_ERR(("%s - Error allocating %u bytes memory for HDCP22_TA buffer",
                      BSTD_FUNCTION, (unsigned)length));
            BERR_TRACE(rc);
            goto done;
        }

        /* copy the TA buffer and save for later use */
        g_hdmiInputTABlock.len = length;
        BKNI_Memcpy(g_hdmiInputTABlock.buf, buf, length);
    }

    /* If there's an HDMI Input FC, load/store that for later use */
    holder.raw = &blk;
    LOCK_SAGE();
    rc = NEXUS_Sage_LoadImage_priv(&holder);
    UNLOCK_SAGE();
    if (rc == NEXUS_NOT_AVAILABLE) {
        /* File is not present, not an error, it is optional */
        rc = BERR_SUCCESS;
        goto done;
    } else if (rc != BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto done;
    }

    g_hdmiInputFCBlock.buf = blk.buf;
    g_hdmiInputFCBlock.len = blk.len;
}

#else
    BSTD_UNUSED(buf);
    BSTD_UNUSED(length);
    goto done;
#endif

done:

    return rc;
}


NEXUS_Error NEXUS_HdmiInput_P_InitHdcp2x(NEXUS_HdmiInputHandle hdmiInput)
{
    BHDCPlib_Dependencies hdcpDependencies;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    NEXUS_SageStatus sageStatus;
    BSAGElib_Handle sagelibHandle;
    BSAGElib_ClientSettings sagelibClientSettings;
    BKNI_EventHandle hdcplibEvent;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    /* get status so we block until Sage is running */
    errCode = NEXUS_Sage_GetStatus(&sageStatus);
    if (errCode) return BERR_TRACE(errCode);

    /* retrieve Sagelib Handle */
    LOCK_SAGE();
    NEXUS_Sage_GetSageLib_priv(&sagelibHandle);
    UNLOCK_SAGE();


    /*****
         Create event to handle watchdog from SAGE
        *****/
    errCode = BKNI_CreateEvent(&g_NEXUS_hdmiInputSageData.eventWatchdogRecv);
    if (errCode != BERR_SUCCESS) {
        BDBG_ERR(( "Error creating sage eventWatchdogRecv" ));
        errCode = BERR_TRACE(errCode);
        goto err_hdcp;
    }

    /* register event */
    g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback =
        NEXUS_RegisterEvent(g_NEXUS_hdmiInputSageData.eventWatchdogRecv,
            NEXUS_HdmiInput_P_SageWatchdogEventhandler, hdmiInput);
    if (g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback == NULL)
    {
        BDBG_ERR(( "NEXUS_RegisterEvent(eventWatchdogRecv) failed!" ));
        errCode = BERR_TRACE(NEXUS_OS_ERROR);
        goto err_hdcp;
    }

    NEXUS_Module_Lock(g_NEXUS_hdmiInputModuleSettings.modules.sage);
    NEXUS_Sage_AddWatchdogEvent_priv(g_NEXUS_hdmiInputSageData.eventWatchdogRecv);
    NEXUS_Module_Unlock(g_NEXUS_hdmiInputModuleSettings.modules.sage);


    /* Initialize all SAGE related handles/Data if not yet initialized */
    if (g_NEXUS_hdmiInputSageData.sagelibClientHandle == NULL)
    {

        /****
              * create event to call BSAGElib_Client_DispatchResponseCallbacks
                when responseRecv_isr callback fire */
        errCode = BKNI_CreateEvent(&g_NEXUS_hdmiInputSageData.eventResponseRecv);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(( "Error creating sage eventResponseRecv" ));
            errCode = BERR_TRACE(errCode);
            goto err_hdcp;
        }

        /******
              * create event for IndicationRecv_isr callback */
        errCode = BKNI_CreateEvent(&g_NEXUS_hdmiInputSageData.eventIndicationRecv);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(( "Error creating sage eventIndicationRecv" ));
            errCode = BERR_TRACE(errCode);
            goto err_hdcp;
        }

        /* register event */
        g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback=
            NEXUS_RegisterEvent(g_NEXUS_hdmiInputSageData.eventIndicationRecv,
                NEXUS_HdmiInput_P_SageIndicationEventHandler, &g_NEXUS_hdmiInputSageData);

        if (g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback == NULL)
        {
            BDBG_ERR(( "NEXUS_RegisterEvent(eventIndicationRecv) failed!" ));
            errCode = BERR_TRACE(NEXUS_OS_ERROR);
            goto err_hdcp;
        }

        /*****
             Create event to handle TATerminate Int from SAGE
        *****/
        errCode = BKNI_CreateEvent(&g_NEXUS_hdmiInputSageData.eventTATerminated);
        if (errCode != BERR_SUCCESS) {
            BDBG_ERR(( "Error creating sage eventTATerminated" ));
            errCode = BERR_TRACE(errCode);
            goto err_hdcp;
        }

        /* register event - piggy back to SAGE WatchdogEventHandler */
        g_NEXUS_hdmiInputSageData.eventTATerminatedCallback =
            NEXUS_RegisterEvent(g_NEXUS_hdmiInputSageData.eventTATerminated,
                NEXUS_HdmiInput_P_SageWatchdogEventhandler, hdmiInput);
        if (g_NEXUS_hdmiInputSageData.eventTATerminatedCallback== NULL)
        {
            BDBG_ERR(( "NEXUS_RegisterEvent(eventTATerminated) failed!" ));
            errCode = BERR_TRACE(NEXUS_OS_ERROR);
            goto err_hdcp;
        }


        /* Open sagelib client */
        BSAGElib_GetDefaultClientSettings(sagelibHandle, &sagelibClientSettings);
        sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_HdmiInput_P_SageIndicationCallback_isr;
        sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_HdmiInput_P_SageResponseCallback_isr;
        sagelibClientSettings.i_rpc.taTerminate_isr = (BSAGElib_Rpc_TATerminateCallback) NEXUS_HdmiInput_P_SageTATerminatedCallback_isr;
        errCode = BSAGElib_OpenClient(sagelibHandle, &g_NEXUS_hdmiInputSageData.sagelibClientHandle,
                                    &sagelibClientSettings);
        if (errCode != BERR_SUCCESS)
        {
            BERR_TRACE(errCode);
            goto err_hdcp;
        }
    }


    /* Open HDCPlib */
    BHDCPlib_GetDefaultDependencies(&hdcpDependencies);
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
    hdcpDependencies.hHdr = hdmiInput->hdr;
#endif
    hdcpDependencies.hSagelibClientHandle = g_NEXUS_hdmiInputSageData.sagelibClientHandle;
    hdcpDependencies.eVersion = BHDM_HDCP_Version_e2_2;
    hdcpDependencies.eCoreType = BHDCPlib_CoreType_eRx;
    hdcpDependencies.pHdcpTA = g_hdmiInputTABlock.buf;
    hdcpDependencies.hdcpTASize = g_hdmiInputTABlock.len;
    hdcpDependencies.sageResponseReceivedEvent = g_NEXUS_hdmiInputSageData.eventResponseRecv;

    errCode = BHDCPlib_Open(&hdmiInput->hdcpHandle, &hdcpDependencies);
    BDBG_MSG(("BHDCPlib_Open (for HDCP2.2) <<< RX(%p)", (void *)(hdmiInput->hdcpHandle)));
    if (errCode != BERR_SUCCESS)
    {
        errCode = BERR_TRACE(errCode);
        goto err_hdcp;
    }


    /* get HDCP2x_AuthenticationStatus Event Handle */
    errCode = BHDCPlib_Hdcp2x_GetEventHandle(hdmiInput->hdcpHandle, BHDCPLIB_Hdcp2x_EventIndication, &hdcplibEvent);
    if (errCode != BERR_SUCCESS) {
        errCode = BERR_TRACE(errCode);
        goto err_hdcp;
    }
    hdmiInput->hdcp2xAuthenticationStatusCallback = NEXUS_RegisterEvent(hdcplibEvent,
                                NEXUS_HdmiInput_P_Hdcp2xAuthenticationStatusUpdate, hdmiInput);
    if (hdmiInput->hdcp2xAuthenticationStatusCallback == NULL) {
        goto err_hdcp;
    }

    if (g_hdmiInputFCBlock.buf && g_hdmiInputFCBlock.len) {
        errCode = BHDCPlib_Hdcp2x_SetBinFeatCert(hdmiInput->hdcpHandle, g_hdmiInputFCBlock.buf,
                                                 g_hdmiInputFCBlock.len);
        if (errCode != BERR_SUCCESS) {
            errCode = BERR_TRACE(errCode);
            goto err_hdcp;
        }
    }

    return NEXUS_SUCCESS;


err_hdcp:
    if (g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback);
        g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventWatchdogRecv) {
        NEXUS_Module_Lock(g_NEXUS_hdmiInputModuleSettings.modules.sage);
        NEXUS_Sage_RemoveWatchdogEvent_priv(g_NEXUS_hdmiInputSageData.eventWatchdogRecv);
        NEXUS_Module_Unlock(g_NEXUS_hdmiInputModuleSettings.modules.sage);
        BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventWatchdogRecv);
        g_NEXUS_hdmiInputSageData.eventWatchdogRecv = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventTATerminatedCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiInputSageData.eventTATerminatedCallback);
        g_NEXUS_hdmiInputSageData.eventTATerminatedCallback = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventTATerminated) {
        BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventTATerminated);
        g_NEXUS_hdmiInputSageData.eventTATerminated= NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventResponseRecv) {
        BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventResponseRecv);
        g_NEXUS_hdmiInputSageData.eventResponseRecv = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback);
        g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventIndicationRecv) {
        BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventIndicationRecv);
        g_NEXUS_hdmiInputSageData.eventIndicationRecv = NULL;
    }

    if (hdmiInput->hdcp2xAuthenticationStatusCallback != NULL) {
        NEXUS_UnregisterEvent(hdmiInput->hdcp2xAuthenticationStatusCallback);
        hdmiInput->hdcp2xAuthenticationStatusCallback = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.sagelibClientHandle) {
        BSAGElib_CloseClient(g_NEXUS_hdmiInputSageData.sagelibClientHandle);
        g_NEXUS_hdmiInputSageData.sagelibClientHandle = NULL;
    }


    return NEXUS_NOT_SUPPORTED;

}


void NEXUS_HdmiInput_P_UninitHdcp2x(NEXUS_HdmiInputHandle hdmiInput)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

	if (g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback) {
		NEXUS_UnregisterEvent(g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback);
		g_NEXUS_hdmiInputSageData.eventWatchdogRecvCallback = NULL;
	}

    if (g_NEXUS_hdmiInputSageData.eventWatchdogRecv) {
        NEXUS_Module_Lock(g_NEXUS_hdmiInputModuleSettings.modules.sage);
        NEXUS_Sage_RemoveWatchdogEvent_priv(g_NEXUS_hdmiInputSageData.eventWatchdogRecv);
        NEXUS_Module_Unlock(g_NEXUS_hdmiInputModuleSettings.modules.sage);
        BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventWatchdogRecv);
        g_NEXUS_hdmiInputSageData.eventWatchdogRecv = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventTATerminatedCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiInputSageData.eventTATerminatedCallback);
        g_NEXUS_hdmiInputSageData.eventTATerminatedCallback = NULL;
    }

    if (g_NEXUS_hdmiInputSageData.eventTATerminated) {
        BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventTATerminated);
        g_NEXUS_hdmiInputSageData.eventTATerminated= NULL;
    }

	if (hdmiInput->hdcp2xAuthenticationStatusCallback != NULL) {
		NEXUS_UnregisterEvent(hdmiInput->hdcp2xAuthenticationStatusCallback);
		hdmiInput->hdcp2xAuthenticationStatusCallback = NULL;
	}

	BHDCPlib_Close(hdmiInput->hdcpHandle);

	/* Close sagelibClientHandle */
	if (g_NEXUS_hdmiInputSageData.eventResponseRecv) {
		BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventResponseRecv);
		g_NEXUS_hdmiInputSageData.eventResponseRecv = NULL;
	}

	if (g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback) {
		NEXUS_UnregisterEvent(g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback);
		g_NEXUS_hdmiInputSageData.eventIndicationRecvCallback = NULL;
	}

	if (g_NEXUS_hdmiInputSageData.eventIndicationRecv) {
		BKNI_DestroyEvent(g_NEXUS_hdmiInputSageData.eventIndicationRecv);
		g_NEXUS_hdmiInputSageData.eventIndicationRecv = NULL;
	}

	if (g_NEXUS_hdmiInputSageData.sagelibClientHandle) {
		BSAGElib_CloseClient(g_NEXUS_hdmiInputSageData.sagelibClientHandle);
		g_NEXUS_hdmiInputSageData.sagelibClientHandle = NULL;
	}

	BKNI_Memset(&g_NEXUS_hdmiInputSageData, 0, sizeof(g_NEXUS_hdmiInputSageData));

    return;
}


static void NEXUS_HdmiInput_P_Hdcp2xAuthenticationStatusUpdate(void *pContext)
{
    NEXUS_HdmiInputHandle hdmiInput = pContext;
    BERR_Code rc = BERR_SUCCESS;
    BHDCPlib_Hdcp2x_AuthenticationStatus stAuthenticationStatus;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    rc = BHDCPlib_Hdcp2x_GetAuthenticationStatus(hdmiInput->hdcpHandle, &stAuthenticationStatus);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error retrieving HDCP2.x authentication status"));
        rc = BERR_TRACE(rc);
        goto done;
    }

    BDBG_LOG(("Hdcp2x Authentication status: %s",
        stAuthenticationStatus.linkAuthenticated?"AUTHENTICATED":"NOT AUTHENTICATED"));

    /* fire callback */
    BKNI_EnterCriticalSection();
    NEXUS_IsrCallback_Fire_isr(hdmiInput->hdcpRxChanged);
    BKNI_LeaveCriticalSection();


done:
	return;

}


/* The ISR callback is registered in HSI and will be fire uppon TA terminated interrupt */
static void NEXUS_HdmiInput_P_SageTATerminatedCallback_isr(void)
{
    BDBG_WRN(("%s: SAGE TATerminate interrupt", BSTD_FUNCTION));

    BKNI_SetEvent_isr(g_NEXUS_hdmiInputSageData.eventTATerminated);
}


/* This event handler is called whenever watchdogEvent registered event is set.
 * The watchdogEvent is set inside NEXUS_HdmiInput_P_SageWatchdogIntHandler_isr() */
static void NEXUS_HdmiInput_P_SageWatchdogEventhandler(void *pContext)
{
    NEXUS_HdmiInputHandle hdmiInput = pContext;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    BDBG_ERR(("%s: SAGE Hdcp2.x Recovery Process - Reopen/initialize HDCPlib and sage rpc handles", BSTD_FUNCTION));

    /* Reinitialized SAGE RPC handles (now invalid) */
    errCode = BHDCPlib_Hdcp2x_ProcessWatchDog(hdmiInput->hdcpHandle);
    if (errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: Error process recovery attempt in HDCPlib", BSTD_FUNCTION));
        errCode = BERR_TRACE(errCode);
        goto done;

    }

done:

    return;
}

static void NEXUS_HdmiInput_P_SageIndicationEventHandler(void *pContext)
{
    NEXUS_HdmiInput_SageData *sageData = (NEXUS_HdmiInput_SageData *) pContext;
    NEXUS_HdmiInputIndicationData receivedIndication;
    BERR_Code rc = BERR_SUCCESS;
    unsigned i=0;

    while (g_NEXUS_hdmiInputSageData.indicationReadPtr != g_NEXUS_hdmiInputSageData.indicationWritePtr)
    {
        i = g_NEXUS_hdmiInputSageData.indicationReadPtr;
        receivedIndication = sageData->indicationData[i];

        BKNI_EnterCriticalSection();
            if (++g_NEXUS_hdmiInputSageData.indicationReadPtr == NEXUS_HDMI_INPUT_SAGE_INDICATION_QUEUE_SIZE)
            {
                g_NEXUS_hdmiInputSageData.indicationReadPtr = 0;
            }
        BKNI_LeaveCriticalSection();

        /* Now pass the callback information to HDCPlib */
        rc = BHDCPlib_Hdcp2x_ReceiveSageIndication(receivedIndication.hHDCPlib, &receivedIndication.sageIndication);
        if (rc) BERR_TRACE(rc);
    }

    return;
}


static void NEXUS_HdmiInput_P_SageIndicationCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument,
    uint32_t indication_id,
    uint32_t value
)
{
    /* Save information for later use */
    unsigned i = g_NEXUS_hdmiInputSageData.indicationWritePtr;

    g_NEXUS_hdmiInputSageData.indicationData[i].sageIndication.rpcRemoteHandle = sageRpcHandle;
    g_NEXUS_hdmiInputSageData.indicationData[i].sageIndication.indication_id = indication_id;
    g_NEXUS_hdmiInputSageData.indicationData[i].sageIndication.value = value;
    g_NEXUS_hdmiInputSageData.indicationData[i].hHDCPlib =
                             (BHDCPlib_Handle) async_argument;

    if (++g_NEXUS_hdmiInputSageData.indicationWritePtr == NEXUS_HDMI_INPUT_SAGE_INDICATION_QUEUE_SIZE)
    {
        g_NEXUS_hdmiInputSageData.indicationWritePtr = 0;
    }

    if (g_NEXUS_hdmiInputSageData.indicationWritePtr == g_NEXUS_hdmiInputSageData.indicationReadPtr)
    {
        BDBG_ERR(("Indication queue overflow - increase queue size"));
    }

    BKNI_SetEvent_isr(g_NEXUS_hdmiInputSageData.eventIndicationRecv);
    return;
}


static void NEXUS_HdmiInput_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(g_NEXUS_hdmiInputSageData.eventResponseRecv);
    return;
}


NEXUS_Error NEXUS_HdmiInput_SetHdcp2xBinKeys(
    NEXUS_HdmiInputHandle handle,
    const uint8_t *pBinFileBuffer,  /* attr{nelem=length} pointer to encrypted key buffer */
    uint32_t length                 /* size of data in pBinFileBuffer in bytes */
)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiInput);

    if (length == 0)
    {
        BDBG_ERR(("Invalid size provided")) ;
        errCode = NEXUS_INVALID_PARAMETER ;
        BERR_TRACE(errCode);
        goto done ;
    }

    errCode = BHDCPlib_Hdcp2x_SetBinKeys(handle->hdcpHandle, pBinFileBuffer, length);

    if ( errCode != BERR_SUCCESS ) {
        errCode = BERR_TRACE(errCode);
    }

done:

    return errCode;
}
#endif		/* NEXUS_HAS_SAGE */




/******************************************************/
#else	/* NEXUS_HAS_SECURITY */

void NEXUS_HdmiInput_P_HdcpProcessChange(void *pContext)
{
	BSTD_UNUSED(pContext) ;
}


void NEXUS_HdmiInput_HdcpGetDefaultKeyset(
	NEXUS_HdmiInputHandle hdmiInput,
	NEXUS_HdmiInputHdcpKeyset *pKeyset /* [out] */
)
{
	BSTD_UNUSED(hdmiInput) ;
	BSTD_UNUSED(pKeyset) ;
}



NEXUS_Error NEXUS_HdmiInput_HdcpSetKeyset(
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_HdmiInputHdcpKeyset *pKeyset
)
{
    BSTD_UNUSED(hdmiInput) ;
    BSTD_UNUSED(pKeyset) ;
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/* No security - use stubs for HDCP */
void NEXUS_HdmiInput_HdcpGetDefaultSettings(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHdcpSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(hdmiInput);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_HdmiInputHdcpSettings)) ;
}


void NEXUS_HdmiInput_HdcpGetSettings(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHdcpSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(hdmiInput);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_HdmiInputHdcpSettings)) ;
}

NEXUS_Error NEXUS_HdmiInput_HdcpSetSettings(
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_HdmiInputHdcpSettings *pSettings
    )
{
    BSTD_UNUSED(hdmiInput);
    BSTD_UNUSED(pSettings);

    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NEXUS_SUCCESS ;
}



NEXUS_Error NEXUS_HdmiInput_HdcpGetStatus(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHdcpStatus *pStatus /* [out] */
    )
{
    BSTD_UNUSED(hdmiInput);

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_HdmiInputHdcpStatus)) ;
    BERR_TRACE(BERR_NOT_SUPPORTED) ;
    return NEXUS_SUCCESS ;
}

NEXUS_Error NEXUS_HdmiInput_HdcpLoadKsvFifo(
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_HdmiHdcpDownStreamInfo *pDownstreamInfo,
    const NEXUS_HdmiHdcpKsv *pDownstreamKsvs, /* attr{nelem=numDevices} */
    unsigned numDevices
	)
{
    BSTD_UNUSED(hdmiInput) ;
    BSTD_UNUSED(pDownstreamInfo) ;
    BSTD_UNUSED(pDownstreamKsvs) ;
    BSTD_UNUSED(numDevices) ;

    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NEXUS_SUCCESS ;
}

#endif		/* NEXUS_HAS_SECURITY */


#if !NEXUS_HAS_SECURITY || !NEXUS_HAS_SAGE || !defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
NEXUS_Error NEXUS_HdmiInput_SetHdcp2xBinKeys(
    NEXUS_HdmiInputHandle handle,
    const uint8_t *pBinFileBuffer,  /* attr{nelem=length} pointer to encrypted key buffer */
    uint32_t length                 /* size of data in pBinFileBuffer in bytes */
)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiInput);
    BSTD_UNUSED(pBinFileBuffer);
    BSTD_UNUSED(length);

    return NEXUS_SUCCESS ;
}
#endif
