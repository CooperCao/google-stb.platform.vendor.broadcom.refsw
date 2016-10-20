/***************************************************************************
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
 ***************************************************************************/

#include "bavc_hdmi.h"
#include "bhdcplib.h"
#include "bhdcplib_priv.h"
#include "bhdcplib_keyloader.h"
#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
#include "bhdcplib_hdcp2x.h"
#include "bhdm_auto_i2c.h"
#endif


BDBG_MODULE(BHDCPLIB) ;
BDBG_OBJECT_ID(HDCPLIB) ;


#define	BHDCPLIB_CHECK_RC( rc, func )	              \
do                                                \
{										          \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										      \
		goto done;							      \
	}										      \
} while(0)


/* HDCP default settings */
static const BHDCPlib_Configuration stHdcpDefaultConfiguration =
{
	BHDM_HDCP_AnSelect_eRandomAn,
	{
		{0x14, 0xF7, 0x61, 0x03, 0xB7},
		{		/* HDCP Test Keys */
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x691e138f, 0x58a44d00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0950e658, 0x35821f00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x818f4878, 0xc98be000},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412c11c8, 0x64d0a000},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x44202428, 0x5a9db300},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b56adbd, 0xb228b900},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x589d5e20, 0xf8005600},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x059f4be5, 0x61125600},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x98547846, 0x8e2f4800},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x38472762, 0x25ae6600},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x543a7b76, 0x31d2e200},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf7227bbf, 0x82603200},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6bce3035, 0x461bf600},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b97d7f0, 0x09043600},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf9498d61, 0x05e1a100},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x063405d1, 0x9d8ec900},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x90614294, 0x67c32000},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xc34facce, 0x51449600},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x8a8ce104, 0x45903e00},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xfc2d9c57, 0x10002900},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x80b1e569, 0x3b94d700},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x437bdd5b, 0xeac75400},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xba90c787, 0x58fb7400},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xae119a15, 0x5e070300},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x01fb788a, 0x40d30500},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb34da0d7, 0xa5590000},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x409e2c4a, 0x633b3700},
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412056b4, 0xbb732500}
		}
	},
	{{0x11,0x11,0x11,0x11,0x11}, 0, 0, false},
	100					/* msWaitForValidVideo */,
	0 					/* msWaitForRxR0Margin */,
	100				 	/* msIntervalKsvFifoReadyCheck */,
	0					/* msWaitForKsvFifoMargin */,
	BHDM_HDCP_REPEATER_MAX_DEVICE_COUNT		/* uiMaxDeviceCount */,
	BHDM_HDCP_REPEATER_MAX_DEPTH			/* uiMaxDepth */
#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
	,
	BHDCPlib_Hdcp2xContentStreamType_eType0
#endif
};


typedef struct _BHDCPLIB_P_HDM_HdcpErrors
{
	BERR_Code hdmHdcpError;
	BHDCPlib_HdcpError hdcplibHdcpError;

} BHDCPLIB_P_HDM_HdcpErrors;

static const BHDCPLIB_P_HDM_HdcpErrors BHDCPlib_SupportedHdcpErrors[BHDCPlib_HdcpError_eCount]=
{
	{BERR_SUCCESS,							BHDCPlib_HdcpError_eSuccess},
	{BHDM_HDCP_RX_BKSV_ERROR,				BHDCPlib_HdcpError_eRxBksvError},
	{BHDM_HDCP_RX_BKSV_REVOKED,				BHDCPlib_HdcpError_eRxBksvRevoked},
	{BHDM_HDCP_RX_BKSV_I2C_READ_ERROR,		BHDCPlib_HdcpError_eRxBksvI2cReadError},
	{BHDM_HDCP_TX_AKSV_ERROR,				BHDCPlib_HdcpError_eTxAksvError},
	{BHDM_HDCP_TX_AKSV_I2C_WRITE_ERROR, 	BHDCPlib_HdcpError_eTxAksvI2cWriteError},
	{BHDM_HDCP_RECEIVER_AUTH_ERROR, 		BHDCPlib_HdcpError_eReceiverAuthenticationError},
	{BHDM_HDCP_REPEATER_AUTH_ERROR,			BHDCPlib_HdcpError_eRepeaterAuthenticationError},
	{BHDM_HDCP_RX_DEVICES_EXCEEDED, 		BHDCPlib_HdcpError_eRxDevicesExceeded},
	{BHDM_HDCP_REPEATER_DEPTH_EXCEEDED, 	BHDCPlib_HdcpError_eRepeaterDepthExceeded},
	{BHDM_HDCP_REPEATER_FIFO_NOT_READY, 	BHDCPlib_HdcpError_eRepeaterFifoNotReady},
	{BHDM_HDCP_REPEATER_DEVCOUNT_0, 		BHDCPlib_HdcpError_eRepeaterDeviceCount0},
	{BHDM_HDCP_LINK_FAILURE,				BHDCPlib_HdcpError_eRepeaterLinkFailure},
	{BHDM_HDCP_LINK_RI_FAILURE, 			BHDCPlib_HdcpError_eLinkRiFailure},
	{BHDM_HDCP_LINK_PJ_FAILURE, 			BHDCPlib_HdcpError_eLinkPjFailure},
	{BHDM_HDCP_FIFO_UNDERFLOW,				BHDCPlib_HdcpError_eFifoUnderflow},
	{BHDM_HDCP_FIFO_OVERFLOW,				BHDCPlib_HdcpError_eFifoOverflow},
	{BHDM_HDCP_MULTIPLE_AN_REQUEST, 		BHDCPlib_HdcpError_eMultipleAnRequest}
};


#if BDBG_DEBUG_BUILD
static const char * const BHDCPlib_StateText[] =
{
	"UnPowered",
	"Unauthenticated",
	"WaitForValidVideo",
	"InitializeAuthentication",
	"WaitForReceiverAuthentication",
	"ReceiverR0Ready",
	"R0LinkFailure",
	"ReceiverAuthenticated",	/* Part 1 Completed; R0 Match */
	"WaitForRepeaterReady",
	"CheckForRepeaterReady",
	"RepeaterReady",
	"LinkAuthenticated",		/* Part 2 Completed; Include down stream devices */
	"EncryptionEnabled",		/* Part 3 Ri Link Integrity Checks Match */
	"RepeaterAuthenticationFailure",
	"RiLinkIntegrityFailure",
	"PjLinkIntegrityFailure"
} ;
#endif

static void BHDCPlib_P_ShowStateChange(BHDCPlib_Handle hHDCPlib, BHDCPlib_State currentState)
{
	if (currentState != hHDCPlib->stHdcpStatus.eAuthenticationState)
	{
		BDBG_MSG(("HDCP State Change: %s (%d) --> %s (%d)",
			BHDCPlib_StateText[currentState], currentState,
			BHDCPlib_StateText[hHDCPlib->stHdcpStatus.eAuthenticationState],
			hHDCPlib->stHdcpStatus.eAuthenticationState)) ;
	}
	return ;
}


static BERR_Code BHDCPlib_P_CheckRepeaterReady(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;
	uint16_t msElapsedTime;
	BHDCPlib_State currentState = hHDCPlib->stHdcpStatus.eAuthenticationState;

	BDBG_ENTER(BHDCPlib_P_CheckRepeaterReady);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	rc = BHDCPlib_GetReceiverInfo(hHDCPlib, &hHDCPlib->stHdcpConfiguration.RxInfo);
	if (rc != BERR_SUCCESS)
		goto done;

	if (hHDCPlib->stHdcpConfiguration.RxInfo.uiRxBCaps & BHDM_HDCP_RxCaps_eKsvFifoReady)
	{
		hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eRepeaterReady;
		goto done;
	}

	/* Calculate elapsed time */
	msElapsedTime = hHDCPlib->uiKsvFifoReadyCount
		* hHDCPlib->stHdcpConfiguration.msIntervalKsvFifoReadyCheck;

	/* the minimum wait time for KSV FIFO Ready is 5seconds */
	if (msElapsedTime >= 5000 +
		hHDCPlib->stHdcpConfiguration.msWaitForKsvFifoMargin)
	{
		BDBG_ERR(("HDCP Auth Failure; Repeater KSV FIFO not ready")) ;
		hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eRepeaterAuthenticationFailure;
		rc = BHDM_HDCP_REPEATER_FIFO_NOT_READY ;
		goto done;

	}
	BDBG_WRN(("Wait for KSV FIFO Ready; timeout in %d ms", (5000+
				hHDCPlib->stHdcpConfiguration.msWaitForKsvFifoMargin - msElapsedTime)));
	hHDCPlib->uiKsvFifoReadyCount++;

	/* Update HDCP Authentication state */
	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eWaitForRepeaterReady;

	/* Restart periodic timer for msIntervalKsvFifoReadyCheck */
	hHDCPlib->bRepeaterReadyTimerExpired = false;
	BTMR_StopTimer(hHDCPlib->hTimer);
	BTMR_StartTimer(hHDCPlib->hTimer, hHDCPlib->stHdcpConfiguration.msIntervalKsvFifoReadyCheck * 1000) ;

done:

	/* Clear authentication if error */
	if (rc != BERR_SUCCESS)
		BHDM_HDCP_ClearAuthentication(hHDCPlib->stDependencies.hHdm);

	BHDCPlib_P_ShowStateChange(hHDCPlib, currentState) ;

	BDBG_LEAVE(BHDCPlib_P_CheckRepeaterReady);
	return rc;

}


static void BHDCPlib_P_TimerExpiration_isr (BHDCPlib_Handle hHDCPlib, int parm2)
{
	BDBG_ENTER(BHDCPlib_P_TimerExpiration_isr);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);
	BSTD_UNUSED(parm2);

	switch (hHDCPlib->stHdcpStatus.eAuthenticationState)
	{
	case BHDCPlib_State_eWaitForReceiverAuthentication :
		hHDCPlib->bR0ReadyTimerExpired = true;
		break ;

	case BHDCPlib_State_eWaitForRepeaterReady:
		hHDCPlib->bRepeaterReadyTimerExpired = true;
		break;

	default :
		BDBG_WRN(("Unknown Timer <%d> expiration...",
			hHDCPlib->stHdcpStatus.eAuthenticationState)) ;
	}

	BDBG_LEAVE(BHDCPlib_P_TimerExpiration_isr);
}


BERR_Code BHDCPlib_Open(BHDCPlib_Handle *hHDCPlib, const BHDCPlib_Dependencies *pstDependencies)
{
	BERR_Code rc = BERR_SUCCESS;
	BHDCPlib_Handle hHandle;

	BDBG_ENTER(BHDCPlib_Open);

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
	/* Open HDCP 2.2 handle */
	if (pstDependencies->eVersion == BHDM_HDCP_Version_e2_2) {
		return BHDCPlib_P_Hdcp2x_Open(hHDCPlib, pstDependencies);
	}
#endif

	BDBG_ASSERT(pstDependencies->hHdm);
	BDBG_ASSERT(pstDependencies->hHsm);
	BDBG_ASSERT(pstDependencies->hTmr);

	/* Alloc memory from the system */
	hHandle = BKNI_Malloc(sizeof(BHDCPlib_P_Handle));
	if (hHandle == NULL)
	{
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("BHDCPlib_Open: BKNI_malloc() failed"));
		goto done;
	}
	BKNI_Memset(hHandle, 0, sizeof(BHDCPlib_P_Handle));
	BDBG_OBJECT_SET(hHandle, HDCPLIB);

	BKNI_Memcpy(&hHandle->stDependencies, pstDependencies, sizeof(BHDCPlib_Dependencies)) ;

	hHandle->hTmr = pstDependencies->hTmr ;
	hHandle->stTmrSettings.type =  BTMR_Type_eCountDown ;
	hHandle->stTmrSettings.cb_isr = (BTMR_CallbackFunc) BHDCPlib_P_TimerExpiration_isr ;
	hHandle->stTmrSettings.pParm1 = hHandle;
	hHandle->stTmrSettings.parm2 = 0 ;
	hHandle->stTmrSettings.exclusive = false ;

	rc = BTMR_CreateTimer(hHandle->hTmr, &hHandle->hTimer, &hHandle->stTmrSettings) ;
	if(rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(BERR_LEAKED_RESOURCE);
		goto done ;
	}

	/* Initialize HDCP authentication state, error status */
	hHandle->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eUnauthenticated;
	hHandle->stHdcpStatus.eHdcpError = BHDCPlib_HdcpError_eSuccess;

	/* Set default HDCP configuration information*/
	BHDCPlib_GetDefaultConfiguration(&hHandle->stHdcpConfiguration);

	/* Initialize revoked KSV list */
	hHandle->RevokedKsvList.Ksvs = NULL;
	hHandle->RevokedKsvList.uiNumRevokedKsvs = 0;

done:
	if (rc != BERR_SUCCESS)
	{
		if (!hHandle)
		{
			BKNI_Free(hHandle);
			hHandle=NULL;
		}
		BDBG_OBJECT_DESTROY(hHandle, HDCPLIB);
	}

	*hHDCPlib = hHandle;

	BDBG_LEAVE(BHDCPlib_Open);
	return rc ;

}


BERR_Code BHDCPlib_Close(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_Close);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
	if (hHDCPlib->stDependencies.eVersion == BHDM_HDCP_Version_e2_2) {
		return BHDCPlib_P_Hdcp2x_Close(hHDCPlib);
	}
#endif

	rc = BTMR_DestroyTimer(hHDCPlib->hTimer);
	if (rc != BERR_SUCCESS)
		BDBG_ERR(("Error destroying timer"));


	/* free memory associated with the Revoked KSV list, if any */
	if (hHDCPlib->RevokedKsvList.Ksvs)
	{
		BKNI_Free(hHDCPlib->RevokedKsvList.Ksvs);
		hHDCPlib->RevokedKsvList.Ksvs = NULL;
		hHDCPlib->RevokedKsvList.uiNumRevokedKsvs = 0;
	}

	/* free memory associated with the HDMIlib Handle */
	BKNI_Memset(hHDCPlib, 0, sizeof(BHDCPlib_P_Handle));
	BDBG_OBJECT_DESTROY(hHDCPlib, HDCPLIB);
	BKNI_Free( (void *) hHDCPlib) ;


	BDBG_LEAVE(BHDCPlib_Close);
	return rc;
}


BERR_Code BHDCPlib_GetDefaultDependencies(BHDCPlib_Dependencies *pDefaultDependencies)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_GetDefaultDependencies);

	/* Set all Dependencies to NULL */
	BKNI_Memset(pDefaultDependencies, 0, sizeof(*pDefaultDependencies));

	pDefaultDependencies->eVersion = BHDM_HDCP_Version_e1_1;

	BDBG_LEAVE(BHDCPlib_GetDefaultDependencies);
	return rc;
}


BERR_Code BHDCPlib_InitializeReceiverAuthentication(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code   rc = BERR_SUCCESS;
	uint8_t ucDeviceAttached=0;

	BDBG_ENTER(BHDCPlib_InitializeReceiverAuthentication) ;
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB) ;
	BDBG_ASSERT(hHDCPlib->stDependencies.hHdm) ;


	rc = BHDM_RxDeviceAttached(hHDCPlib->stDependencies.hHdm, &ucDeviceAttached) ;
	if (!ucDeviceAttached)
	{
		hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eUnPowered;
		goto done;
	}

	/* Retrieve Receiver information before starting the authentication process */
	rc = BHDCPlib_GetReceiverInfo(hHDCPlib, &hHDCPlib->stHdcpConfiguration.RxInfo);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error getting receiver info"));
		goto done;
	}

	if (hHDCPlib->RevokedKsvList.Ksvs) {
		/* Read the Bksv from the Rx,  compare revoked list against retrieved Rx information */
		rc = BHDM_HDCP_ReadRxBksv(hHDCPlib->stDependencies.hHdm,
			(const uint8_t *) hHDCPlib->RevokedKsvList.Ksvs,
			hHDCPlib->RevokedKsvList.uiNumRevokedKsvs);
		if (rc != BERR_SUCCESS)
			goto done;
	}


	/* Update the RxBksv in BHDCPlib_Configuration */
	rc = BHDM_HDCP_GetRxKsv(hHDCPlib->stDependencies.hHdm, (uint8_t *) hHDCPlib->stHdcpConfiguration.RxInfo.RxBksv);
	if (rc != BERR_SUCCESS)
		goto done;


	/* Generate/Write the Authentication An value */
	rc = BHDM_HDCP_GenerateAn(hHDCPlib->stDependencies.hHdm, hHDCPlib->stHdcpConfiguration.eAnSelection) ;
	if (rc != BERR_SUCCESS)
		goto done;


	/* Write the Tx Aksv to the Receiver */
	rc = BHDM_HDCP_WriteTxAksvToRx(hHDCPlib->stDependencies.hHdm, hHDCPlib->stHdcpConfiguration.TxKeySet.TxAksv);
	if (rc != BERR_SUCCESS)
		goto done;


	/* load the transmitter HDCP Keys */
	rc = BHDM_HDCP_EnableSerialKeyLoad(hHDCPlib->stDependencies.hHdm);
	if (rc != BERR_SUCCESS)
		goto done;


	rc = BHDCPlib_FastLoadEncryptedHdcpKeys(hHDCPlib);
	if (rc != BERR_SUCCESS)
		goto done;


	/* Initiate timer countdown for 100ms + msWaitForR0Margin before setting Receiver Ready */
	hHDCPlib->bR0ReadyTimerExpired = false;
	BTMR_StopTimer(hHDCPlib->hTimer);
	BTMR_StartTimer(hHDCPlib->hTimer, (100+hHDCPlib->stHdcpConfiguration.msWaitForRxR0Margin) * 1000) ;

	/* Update HDCP authentication state */
	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eWaitForReceiverAuthentication;

done:

	if (rc != BERR_SUCCESS)
		hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eUnauthenticated;

	BDBG_LEAVE(BHDCPlib_InitializeReceiverAuthentication) ;
	return rc ;

}



BERR_Code BHDCPlib_AuthenticateReceiver(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_AuthenticateReceiver) ;
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* Authenticate the Link  */
	rc = BHDM_HDCP_AuthenticateLink(hHDCPlib->stDependencies.hHdm) ;
	if ( rc != BERR_SUCCESS)
	{
		hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eR0LinkFailure;
		goto done;
	}

	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eReceiverAuthenticated;

	rc = BHDM_HDCP_XmitEncrypted(hHDCPlib->stDependencies.hHdm);
	if (rc != BERR_SUCCESS)
		goto done;

done:

	BDBG_LEAVE(BHDCPlib_AuthenticateReceiver) ;
	return rc;

}



BERR_Code BHDCPlib_InitializeRepeaterAuthentication(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_InitializeRepeaterAuthentication) ;
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	/* Set default configuration	*/
	hHDCPlib->uiKsvFifoReadyCount = 1;

	/* Enable/Initiate periodic timer once every msIntervalKsvFifoReadyCheck */
	hHDCPlib->bRepeaterReadyTimerExpired = false;
	BTMR_StopTimer(hHDCPlib->hTimer);
	BTMR_StartTimer(hHDCPlib->hTimer, hHDCPlib->stHdcpConfiguration.msIntervalKsvFifoReadyCheck * 1000) ;

	/* Update state machine	*/
	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eWaitForRepeaterReady;

	BDBG_LEAVE(BHDCPlib_InitializeRepeaterAuthentication) ;
	return rc ;
}



BERR_Code BHDCPlib_AuthenticateRepeater(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;
	uint16_t BStatus;
	uint8_t RepeaterLevels;
	uint8_t DeviceCount;
	uint16_t uiNumKsvBytes;
	uint8_t KsvListMemoryAllocated = 0 ;
	uint8_t *KsvFifoList = NULL ;

	BDBG_ENTER(BHDCPlib_AuthenticateRepeater) ;
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* check Repeater Values */
	BHDCPLIB_CHECK_RC(rc,
		BHDM_HDCP_GetRxStatus(hHDCPlib->stDependencies.hHdm, &BStatus)) ;
	BHDCPLIB_CHECK_RC(rc,
		BHDM_HDCP_GetRepeaterDepth(hHDCPlib->stDependencies.hHdm, &RepeaterLevels)) ;
	BHDCPLIB_CHECK_RC(rc,
		BHDM_HDCP_GetRepeaterDeviceCount(hHDCPlib->stDependencies.hHdm, &DeviceCount)) ;

	BDBG_MSG(("RXStatus: %X, Depth: %d, Devices: %d",
		BStatus, RepeaterLevels, DeviceCount)) ;


	/* check if the number of repeater levels has been exceeded */
	if (BStatus & BHDM_HDCP_RxStatus_eMaxRepeatersExceeded)
	{
		BDBG_ERR(("%d Levels of Repeaters exceed the MAX allowed of %d",
			RepeaterLevels, hHDCPlib->stHdcpConfiguration.uiMaxDepth)) ;
		rc = BHDM_HDCP_REPEATER_DEPTH_EXCEEDED ;
		goto done;
	}

	/* check if the number of receiver devices has been exceeded */
	if (BStatus & BHDM_HDCP_RxStatus_eMaxDevicesExceeded)
	{
		BDBG_ERR(("Number of Devices: %d exceeds the MAX allowed of %d",
			DeviceCount, hHDCPlib->stHdcpConfiguration.uiMaxDeviceCount)) ;
		rc = BHDM_HDCP_RX_DEVICES_EXCEEDED ;
		goto done ;
	}

	/* initialize the Repeater Authentication */
	BHDM_HDCP_InitializeRepeaterAuthentication(hHDCPlib->stDependencies.hHdm);

	if (DeviceCount)
	{
		/* allocate a buffer to hold the Ksv List */
		uiNumKsvBytes = (uint16_t) (DeviceCount * BHDM_HDCP_KSV_LENGTH) ;
		KsvFifoList = (uint8_t *) BKNI_Malloc(sizeof(uint8_t) * uiNumKsvBytes) ;
		KsvListMemoryAllocated = 1 ;

		/* read the Ksv List */
		/* pass the revoked list for checking against downstream Rx devices */
		BHDCPLIB_CHECK_RC(rc,
			BHDM_HDCP_ReadRxRepeaterKsvFIFO(hHDCPlib->stDependencies.hHdm,
				KsvFifoList, DeviceCount,
				(uint8_t *) &hHDCPlib->RevokedKsvList.Ksvs,
				(uint16_t) hHDCPlib->RevokedKsvList.uiNumRevokedKsvs)) ;

		/* write the Ksvs from the Rx (Repeater) to the Transmitter core for verification */
		BHDCPLIB_CHECK_RC(rc,
			BHDM_HDCP_WriteTxKsvFIFO(hHDCPlib->stDependencies.hHdm,
				KsvFifoList, DeviceCount)) ;

	}
	else	/* handle zero devices attached to repeater */
	{
#if BHDM_CONFIG_DISABLE_HDCP_AUTH_REPEATER_DEVCOUNT0
		/* do not allow authentication with repeaters that have device count of 0 */
		BDBG_WRN(("Auth Disabled for Repeaters with Device Count of 0")) ;
		rc = BHDM_HDCP_REPEATER_DEVCOUNT_0 ;
		goto done;
#else
		/* force V calculation for repeater with zero attached devices */
		BHDM_HDCP_ForceVCalculation(hHDCPlib->stDependencies.hHdm) ;
#endif

	}

	/* check SHA-1 Hash Verification (V).... */
	BHDCPLIB_CHECK_RC(rc,
		BHDM_HDCP_RepeaterAuthenticateLink(hHDCPlib->stDependencies.hHdm,
			&hHDCPlib->stHdcpConfiguration.RxInfo.uiIsAuthenticated)) ;

	if (!hHDCPlib->stHdcpConfiguration.RxInfo.uiIsAuthenticated)
	{
		BDBG_ERR(("Repeater failed to authenticate")) ;
		rc = BHDM_HDCP_REPEATER_AUTH_ERROR ;
		goto done ;
	}

	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eLinkAuthenticated;

done:
	/* release allocated memory for Ksv list */
	if (KsvListMemoryAllocated)
		BKNI_Free(KsvFifoList) ;

	if (rc != BERR_SUCCESS)
		hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eRepeaterAuthenticationFailure;

	BDBG_LEAVE(BHDCPlib_AuthenticateRepeater) ;
	return rc ;
}


BERR_Code BHDCPlib_TransmitEncrypted(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;
	BHDCPlib_State currentState = hHDCPlib->stHdcpStatus.eAuthenticationState;

	BDBG_ENTER(BHDCPlib_TransmitEncrypted);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	rc = BHDM_HDCP_XmitEncrypted(hHDCPlib->stDependencies.hHdm);
	if (rc != BERR_SUCCESS)
		goto done;

	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eEncryptionEnabled;
	BHDCPlib_P_ShowStateChange(hHDCPlib, currentState) ;

done:

	BDBG_LEAVE(BHDCPlib_TransmitEncrypted);
	return rc;
}

BERR_Code BHDCPlib_TransmitClear(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;
	BHDCPlib_State currentState = hHDCPlib->stHdcpStatus.eAuthenticationState;

	BDBG_ENTER(BHDCPlib_TransmitClear);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	rc = BHDM_HDCP_XmitClear(hHDCPlib->stDependencies.hHdm);
	if (rc != BERR_SUCCESS)
		goto done;

	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eUnauthenticated;

	BHDCPlib_P_ShowStateChange(hHDCPlib, currentState) ;

done:

	BDBG_LEAVE(BHDCPlib_TransmitClear);
	return rc;
}

void BHDCPlib_ProcessEvent(BHDCPlib_Handle hHDCPlib, BHDCPlib_Event *stHdmiEvent)
{

	BERR_Code rc = BERR_SUCCESS ;
	uint8_t ucRxSense;
	uint8_t ucDeviceAttached;
	uint8_t ucErrorDetected = 0 ;
	BHDCPlib_State currentState = hHDCPlib->stHdcpStatus.eAuthenticationState;

	BDBG_ENTER(BHDCPlib_ProcessEvent);

	switch (stHdmiEvent->event)
	{
	case BHDM_EventHDCPRiValue:
		if (BHDM_HDCP_RiLinkIntegrityCheck(hHDCPlib->stDependencies.hHdm) != BERR_SUCCESS)
		{
			hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eRiLinkIntegrityFailure;
			ucErrorDetected = 1 ;
		}

		break;

	case BHDM_EventHDCPPjValue:
		if (BHDM_HDCP_PjLinkIntegrityCheck(hHDCPlib->stDependencies.hHdm) != BERR_SUCCESS)
		{
			hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_ePjLinkIntegrityFailure;
			ucErrorDetected = 1 ;
		}

		break;

	case BHDM_EventHDCPRepeater:
		hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eRepeaterAuthenticationFailure;
		ucErrorDetected = 1 ;
		break;

	case BHDM_EventHotPlug:
		BDBG_MSG(("Hotplug event occurred"));
		rc = BHDM_RxDeviceAttached(hHDCPlib->stDependencies.hHdm, &ucDeviceAttached) ;
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Error getting Hot Plug Status "));
			BERR_TRACE(rc);
		}

        if (hHDCPlib->stDependencies.eVersion == BHDM_HDCP_Version_e2_2)
        {
#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
            rc = BHDCPlib_P_Hdcp2x_StopAuthentication(hHDCPlib);
            if (rc != BERR_SUCCESS)
            {
                BDBG_ERR(("Error disabling HDCP 2.x authentication"));
                BERR_TRACE(rc);
            }
#else
            BDBG_ERR(("HDCP 2.x is not supported, invalid mode of operation"));
            BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
        }
        else
        {
            if (!ucDeviceAttached)
                hHDCPlib->stHdcpStatus.eAuthenticationState =
                    BHDCPlib_State_eUnPowered;
            else
                hHDCPlib->stHdcpStatus.eAuthenticationState =
                    BHDCPlib_State_eUnauthenticated;
        }

        break;

	default:
		BDBG_WRN(("Invalid Event")) ;
		break;
	}

	if (ucErrorDetected)
	{
		rc = BHDM_GetReceiverSense(hHDCPlib->stDependencies.hHdm, &ucRxSense);
		if (!ucRxSense)
		{
			BDBG_WRN(("HdcpLibState: (%d) '%s' generated due to Rx powered off",
				hHDCPlib->stHdcpStatus.eAuthenticationState,
				BHDCPlib_StateText[hHDCPlib->stHdcpStatus.eAuthenticationState]	)) ;
			hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eUnPowered;
		}
	}

	BHDCPlib_P_ShowStateChange(hHDCPlib, currentState) ;

	BDBG_LEAVE(BHDCPlib_ProcessEvent);
	return;
}


BERR_Code BHDCPlib_SetRevokedKSVs(BHDCPlib_Handle hHDCPlib, BHDCPlib_RevokedKsvList *stKsvList)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_SetRevokedKSVs);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	if (hHDCPlib->RevokedKsvList.Ksvs)
	{
		BKNI_Free(hHDCPlib->RevokedKsvList.Ksvs);
		hHDCPlib->RevokedKsvList.Ksvs = NULL;
		hHDCPlib->RevokedKsvList.uiNumRevokedKsvs = 0;
	}

	if (!stKsvList->uiNumRevokedKsvs)
	{
		BDBG_WRN(("No Revoked KSVs specified."));
		goto done;
	}

	/* Store the Revoked KSVs in the handle */
	hHDCPlib->RevokedKsvList.Ksvs =
		BKNI_Malloc(sizeof(uint8_t) * stKsvList->uiNumRevokedKsvs * BAVC_HDMI_HDCP_KSV_LENGTH);

	BKNI_Memcpy(hHDCPlib->RevokedKsvList.Ksvs, stKsvList->Ksvs,
					(sizeof(uint8_t) * stKsvList->uiNumRevokedKsvs * BAVC_HDMI_HDCP_KSV_LENGTH));

	hHDCPlib->RevokedKsvList.uiNumRevokedKsvs = stKsvList->uiNumRevokedKsvs;

done:
	BDBG_LEAVE(BHDCPlib_SetRevokedKSVs);
	return rc;
}


BERR_Code BHDCPlib_SetConfiguration(BHDCPlib_Handle hHDCPlib, BHDCPlib_Configuration * stHdcpConfiguration)
{
	BDBG_ENTER(BHDCPlib_SetConfiguration);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* Copy Hdcp Link data into HDMlib handle 	*/
	BKNI_Memcpy(&hHDCPlib->stHdcpConfiguration, stHdcpConfiguration, sizeof(BHDCPlib_Configuration));


	BDBG_LEAVE(BHDCPlib_SetConfiguration);
	return BERR_SUCCESS;
}

BERR_Code BHDCPlib_GetDefaultConfiguration(BHDCPlib_Configuration *stHdcpConfiguration)
{
	BDBG_ENTER(BHDCPlib_GetDefaultConfiguration);
	BDBG_ASSERT(stHdcpConfiguration);

	/* Copy default Hdcp configuration */
	BKNI_Memcpy(stHdcpConfiguration, &stHdcpDefaultConfiguration, sizeof(BHDCPlib_Configuration));

	BDBG_LEAVE(BHDCPlib_GetDefaultConfiguration);
	return BERR_SUCCESS;
}

BERR_Code BHDCPlib_GetConfiguration(BHDCPlib_Handle hHDCPlib, BHDCPlib_Configuration *stHdcpConfiguration)
{
	BDBG_ENTER(BHDCPlib_GetConfiguration);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* Copy Hdcp Link data from HDCPlib handle */
	BKNI_Memcpy(stHdcpConfiguration, &hHDCPlib->stHdcpConfiguration, sizeof(BHDCPlib_Configuration));


	BDBG_LEAVE(BHDCPlib_GetConfiguration);
	return BERR_SUCCESS;
}


BERR_Code BHDCPlib_GetReceiverInfo(BHDCPlib_Handle hHDCPlib, BHDCPlib_RxInfo *stRxHdcpInfo)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_GetReceiverInfo);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* Retrieve RxBCaps */
	rc = BHDM_HDCP_GetRxCaps(hHDCPlib->stDependencies.hHdm,
				&stRxHdcpInfo->uiRxBCaps) ;
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error getting Rx BCaps"));
		goto done;
	}

	/* Retrieve the RxBksv */
	rc = BHDM_HDCP_GetRxKsv(hHDCPlib->stDependencies.hHdm,
				(uint8_t *) stRxHdcpInfo->RxBksv);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error getting Rx BCaps"));
		goto done;
	}

	/* Check for Repeater */
	stRxHdcpInfo->bIsHdcpRepeater =
		stRxHdcpInfo->uiRxBCaps & BHDM_HDCP_RxCaps_eHdcpRepeater ;

done:

	BDBG_LEAVE(BHDCPlib_GetReceiverInfo);
	return rc;

}


#if !B_REFSW_MINIMAL
BERR_Code BHDCPlib_GetAuthenticationState(BHDCPlib_Handle hHDCPlib, BHDCPlib_State *eAuthenticationState)
{
	BDBG_ENTER(BHDCPlib_GetAuthenticationState);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	/* Retrieve the current HDCP authentication state */
	*eAuthenticationState = hHDCPlib->stHdcpStatus.eAuthenticationState;

	BDBG_LEAVE(BHDCPlib_GetAuthenticationState);
	return BERR_SUCCESS;
}
#endif


BERR_Code BHDCPlib_GetHdcpStatus(BHDCPlib_Handle hHDCPlib, BHDCPlib_Status *stHdcpStatus)
{
	BDBG_ENTER(BHDCPlib_GetHdcpStatus);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	/* Retrieve the current HDCP status  */
	BKNI_Memcpy(stHdcpStatus, &hHDCPlib->stHdcpStatus, sizeof(BHDCPlib_Status));


	BDBG_LEAVE(BHDCPlib_GetHdcpStatus);
	return BERR_SUCCESS;
}


bool BHDCPlib_LinkReadyForEncryption(BHDCPlib_Handle hHDCPlib)
{
	bool brc = false;

	BDBG_ENTER(BHDCPlib_LinkReadyForEncryption);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

	/* link is ready for encryption if it is authenticated or already encrypting */
	if ((hHDCPlib->stHdcpStatus.eAuthenticationState == BHDCPlib_State_eLinkAuthenticated)
	|| (hHDCPlib->stHdcpStatus.eAuthenticationState == BHDCPlib_State_eEncryptionEnabled))
		brc = true;

	BDBG_LEAVE(BHDCPlib_LinkReadyForEncryption);
	return brc;
}


BERR_Code BHDCPlib_StartAuthentication(BHDCPlib_Handle hHDCPlib)
{
	BHDCPlib_State currentState = hHDCPlib->stHdcpStatus.eAuthenticationState;
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_StartAuthentication);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
	if (hHDCPlib->stDependencies.eVersion == BHDM_HDCP_Version_e2_2) {
		rc = BHDCPlib_P_Hdcp2x_StartAuthentication(hHDCPlib);
		BHDCPlib_P_ShowStateChange(hHDCPlib, currentState) ;
		return rc ;
	}
#endif

	/* Initialize HDCP Authentication state */
	hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eWaitForValidVideo;

	BHDCPlib_P_ShowStateChange(hHDCPlib, currentState) ;

	BDBG_LEAVE(BHDCPlib_StartAuthentication);
	return rc;
}


/******************************

Possible return errors:

BHDM_NO_RX_DEVICE
BHDM_HDCP_RX_BKSV_ERROR
BHDM_HDCP_RX_BKSV_REVOKED
BHDM_HDCP_TX_AKSV_ERROR

BHDM_HDCP_AUTH_ABORTED
BHDM_HDCP_AUTHENTICATE_ERROR
BHDM_HDCP_LINK_RI_FAILURE
BHDM_HDCP_LINK_PJ_FAILURE

BHDM_HDCP_REPEATER_FIFO_NOT_READY
BHDM_HDCP_REPEATER_DEVCOUNT_0
BHDM_HDCP_REPEATER_DEPTH_EXCEEDED
BHDM_HDCP_RX_DEVICES_EXCEEDED


TBD:

BHDM_HDCP_RX_NO_HDCP_SUPPORT
BHDM_HDCP_RECEIVER_AUTH_ERROR
BHDM_HDCP_REPEATER_AUTH_ERROR
BHDM_HDCP_NO_AUTHENTICATED_LINK

**********************************/
BERR_Code BHDCPlib_ProcessAuthentication(BHDCPlib_Handle hHDCPlib, BHDCPlib_Status *stHdcpStatus)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t i;
	bool bErrFound = false;
	BHDCPlib_State currentState = hHDCPlib->stHdcpStatus.eAuthenticationState;

	BDBG_ENTER(BHDCPlib_ProcessAuthentication);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);


	/* Always recommend a delay of 1 ms between states unless specified below */
	hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 1;

	switch (hHDCPlib->stHdcpStatus.eAuthenticationState)
	{
	case BHDCPlib_State_eUnPowered:
		hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 500 ;
		break;

	case BHDCPlib_State_eUnauthenticated:
		hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 100;
		break;

	case BHDCPlib_State_eWaitForValidVideo:
		if (BHDM_CheckForValidVideo(hHDCPlib->stDependencies.hHdm))
		{
			hHDCPlib->stHdcpStatus.msRecommendedWaitTime = hHDCPlib->stHdcpConfiguration.msWaitForValidVideo;
		}
		else {
			hHDCPlib->stHdcpStatus.eAuthenticationState =
					BHDCPlib_State_eInitializeAuthentication;
		}
		break;

	case BHDCPlib_State_eInitializeAuthentication:
		/* Attempting first part of authentication */
		rc = BHDCPlib_InitializeReceiverAuthentication(hHDCPlib);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("first part of the authentication process failed")) ;
			goto done;
		}
		break;

	case BHDCPlib_State_eWaitForReceiverAuthentication:
		if (hHDCPlib->bR0ReadyTimerExpired)
			hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eReceiverR0Ready;
		else
			hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 100 + hHDCPlib->stHdcpConfiguration.msWaitForRxR0Margin;

		break;

	case BHDCPlib_State_eReceiverR0Ready:
		rc = BHDCPlib_AuthenticateReceiver(hHDCPlib);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to authenticate receiver"));
			goto done;
		}
		break;

	case BHDCPlib_State_eR0LinkFailure:
		hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 2000;
		break;

	case BHDCPlib_State_eReceiverAuthenticated:
		/* Proceed to part 2 of authentication if attached device is a repeater */
		if (hHDCPlib->stHdcpConfiguration.RxInfo.bIsHdcpRepeater)
		{
			rc = BHDCPlib_InitializeRepeaterAuthentication(hHDCPlib);
			if (rc != BERR_SUCCESS)
			{
				BDBG_ERR(("Second part of the authentication process failed"));
				goto done;
			}
		}
		else
			hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eLinkAuthenticated;

		break;

	case BHDCPlib_State_eWaitForRepeaterReady:
		if (hHDCPlib->bRepeaterReadyTimerExpired)
			hHDCPlib->stHdcpStatus.eAuthenticationState = BHDCPlib_State_eCheckForRepeaterReady;
		else
			/* Recommend waiting 100 ms for Repeater Fifo ready */
			hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 100;

		break;

	case BHDCPlib_State_eCheckForRepeaterReady:
		/* Check for repeater FIFO ready */
		rc = BHDCPlib_P_CheckRepeaterReady(hHDCPlib);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Repeater KSV FIFO not ready"));
			goto done;
		}
		break;

	case BHDCPlib_State_eRepeaterReady:
		/* Repeater Ready, start authenticate repeater */
		rc = BHDCPlib_AuthenticateRepeater(hHDCPlib);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to authenticate repeater"));
			goto done;
		}
		break;

	case BHDCPlib_State_eLinkAuthenticated:
		/* HDCP AUTHENTICATION SUCCESS - link is now authenticated */

		/* Recommend 10ms for checking encryption enabled */
		hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 10;
		break;


	case BHDCPlib_State_eEncryptionEnabled:
		/* Transmitting Encrypted Video	*/
		hHDCPlib->stHdcpStatus.msRecommendedWaitTime = 2000;
		goto done;
		break;

	case BHDCPlib_State_eRepeaterAuthenticationFailure:
		/* Repeater Error */
		rc = BHDM_HDCP_LINK_FAILURE;
		break;

	case BHDCPlib_State_eRiLinkIntegrityFailure:
		/* Ri Link Failure */
		rc = BHDM_HDCP_LINK_RI_FAILURE;
		break;

	case BHDCPlib_State_ePjLinkIntegrityFailure:
		/* Pj Link Failure */
		rc = BHDM_HDCP_LINK_PJ_FAILURE ;
		break;

	default:
		BDBG_WRN(("Invalid HDCP Authentication state: %d", hHDCPlib->stHdcpStatus.eAuthenticationState)) ;
		break;

	}


done:
	/* HDCP error */
	for (i=0; i < sizeof(BHDCPlib_SupportedHdcpErrors)/sizeof(BHDCPLIB_P_HDM_HdcpErrors); i++)
	{
		if (rc == BHDCPlib_SupportedHdcpErrors[i].hdmHdcpError)
		{
			hHDCPlib->stHdcpStatus.eHdcpError = BHDCPlib_SupportedHdcpErrors[i].hdcplibHdcpError;
			bErrFound = true;
			break;
		}
	}

	if (!bErrFound)
	{
		BDBG_ERR(("Unable to find matching BHDM_HDCP return code for HDCPlib return code %x", rc));
		BDBG_ERR(("Current Authentication State: %s (%d)",
			BHDCPlib_StateText[currentState], currentState)) ;

		hHDCPlib->stHdcpStatus.eHdcpError = BHDCPlib_HdcpError_eReceiverAuthenticationError;
	}

	/* Save current hdcp status  */
	BKNI_Memcpy(stHdcpStatus, &hHDCPlib->stHdcpStatus, sizeof(BHDCPlib_Status));

	BHDCPlib_P_ShowStateChange(hHDCPlib, currentState) ;

	BDBG_LEAVE(BHDCPlib_ProcessAuthentication);
	return rc;

}


BERR_Code BHDCPlib_DisableAuthentication(BHDCPlib_Handle hHDCPlib)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDCPlib_DisableAuthentication);
	BDBG_OBJECT_ASSERT(hHDCPlib, HDCPLIB);

#if BHDCPLIB_HAS_HDCP_2X_SUPPORT && defined(BHDCPLIB_HAS_SAGE)
	if (hHDCPlib->stDependencies.eVersion == BHDM_HDCP_Version_e2_2) {
		return BHDCPlib_P_Hdcp2x_StopAuthentication(hHDCPlib);
	}
#endif

	/* clear HDCP Authentication */
	BHDM_HDCP_ClearAuthentication(hHDCPlib->stDependencies.hHdm) ;

	/* Transmit clear data */
	rc = BHDCPlib_TransmitClear(hHDCPlib);

	BDBG_LEAVE(BHDCPlib_DisableAuthentication);
	return rc;
}

