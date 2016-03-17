/******************************************************************************
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
 ******************************************************************************/

#include "bstd.h"
#include "breg_endian.h"

#include "bhdm_config.h"
#include "bhdm.h"
#include "bhdm_priv.h"
#include "bhdm_hdcp.h"
#if BHDM_CONFIG_MHL_SUPPORT
#include "bhdm_mhl_priv.h"
#include "bhdm_mhl_hdcp_priv.h"
#endif

BDBG_MODULE(BHDM_HDCP) ;

static unsigned int BHDM_HDCP_P_NumberOfSetBits(const unsigned char *bytes, int nbytes) ;

static BERR_Code BHDM_HDCP_P_CheckRevokedKsvList(
	const uint8_t *pRevokedKsvList,
	const uint16_t uiRevokedKsvCount,
	uint8_t *pRxKsvList,
	uint16_t uiRxDeviceCount,
	bool *bRevoked) ;


#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
static BERR_Code BHDM_HDCP_P_AutoRiLinkIntegrityCheck(
	const BHDM_Handle hHDMI     /* [in] HDMI handle */
) ;

static BERR_Code BHDM_HDCP_P_AutoPjLinkIntegrityCheck(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
) ;

static BERR_Code BHDM_HDCP_P_ConfigureAutoRi(
	const BHDM_Handle hHDMI     /* [in] HDMI handle */
) ;

static BERR_Code BHDM_HDCP_P_ConfigureAutoPj(
	const BHDM_Handle hHDMI     /* [in] HDMI handle */
) ;

static BERR_Code BHDM_HDCP_P_EnableAutoRiPjChecking (
	const BHDM_Handle hHDMI,      /* [in] HDMI handle */
	uint8_t uiPjChecking
) ;

static BERR_Code BHDM_HDCP_P_DisableAutoRiPjChecking (
	const BHDM_Handle hHDMI		/* [in] HDMI handle */
) ;
#endif


/******************************************************************************
unsigned int BHDM_HDCP_P_NumberOfSetBits
Summary: Get the number of Set Bits
*******************************************************************************/
static unsigned int BHDM_HDCP_P_NumberOfSetBits(const unsigned char *bytes, int nbytes)
{
	int i, j ;
	int bit ;
	int count = 0 ;
	uint8_t byteToCheck;

	count = 0 ;
	for (i = 0; i < nbytes; i++)
	{
		bit = 1 ;
		byteToCheck = bytes[i];
		for (j = 0; j < 8 ; j++)
		{
			if (bit & byteToCheck)
				count++ ;
			bit = bit << 1 ;
		}
	}
	return count ;
} /* end BHDM_HDCP_P_NumberOfSetBits */


/******************************************************************************
BERR_Code BHDM_HDCP_P_CheckRevokedKsvList
Summary: Check if retrieved Ksv(s) are on the list of revoked Ksvs
*******************************************************************************/
BERR_Code BHDM_HDCP_P_CheckRevokedKsvList(
	const uint8_t *pRevokedKsvList,
	const uint16_t uiRevokedKsvCount,
	uint8_t *pRxKsvList,
	uint16_t uiRxDeviceCount,
	bool *bRevokedKsv)
{
	uint16_t i, j ;
	uint16_t uiRxKsvIndex ;
	uint16_t uiRevokedKsvIndex ;

	uint8_t k ;

	*bRevokedKsv = false ;

	/* check each retrieved KSV against the Revoked KSV List */
	for (i = 0 ; i < uiRxDeviceCount ; i++)                  /* for each RxKsv */
	{
		uiRxKsvIndex = i * BHDM_HDCP_KSV_LENGTH ;

		/* display each KSV to check debugging */
		BDBG_MSG(("Checking RxBksv (Device %d): %02x %02x %02x %02x %02x",
			i, pRxKsvList[uiRxKsvIndex + 4],
			pRxKsvList[uiRxKsvIndex + 3], pRxKsvList[uiRxKsvIndex + 2],
			pRxKsvList[uiRxKsvIndex + 1], pRxKsvList[uiRxKsvIndex + 0]));

		for (j = 0 ; j < uiRevokedKsvCount; j++)        /* for each Revoked Ksv */
		{
			/*
			* SRM messages, which contain the Revoked Keys are stored in big endian
			* format, therefore the comparison must be reveresed for the retrieved
			* Ksvs which are in little endian format
			*/

			/* set the Revoked Ksv byte index to the end of the stored key
			* i.e. at the LSB
			*/
			uiRevokedKsvIndex = j * BHDM_HDCP_KSV_LENGTH + BHDM_HDCP_KSV_LENGTH - 1 ;

			for (k = 0 ; k < BHDM_HDCP_KSV_LENGTH; k++) /* for each Ksv Byte */
			{
#if 0
				/* debug KSV revocation */
				BDBG_MSG(("Compare Ksv LSB %d: %02x - %02x", k,
					pRxKsvList[uiRxKsvIndex + k],               /* little endian */
					pRevokedKsvList[uiRevokedKsvIndex - k])) ;  /* big endian */
#endif

				if (pRxKsvList[uiRxKsvIndex + k] != pRevokedKsvList[uiRevokedKsvIndex - k])
					break ;  /* no match; go to next Revoked Key */

				/* all bytes matched */
				if (k + 1 == BHDM_HDCP_KSV_LENGTH)
				{
					BDBG_WRN(("RxBksv appears in revoked list")) ;
					*bRevokedKsv = true ;
					goto done ;
				}
			}
		}
	}

done:
	return BERR_SUCCESS ;
}


/******************************************************************************
bool BHDM_HDCP_P_RegisterAccessAllowed
	check that
	1) HDCP 1.x registers are ony accessed when HDCP version 1.x is enabled, or
	2) HDCP 2.x registers are ony accessed when HDCP version 2.x is enabled
*******************************************************************************/

static bool BHDM_HDCP_P_RegisterAccessAllowed(
	const BHDM_Handle hHDMI,
	uint8_t offset
)
{
	bool authenticated ;
	bool accessAllowed = true ;

	BHDM_HDCP_IsLinkAuthenticated(	hHDMI, &authenticated) ;

	/* if not authenticated, any register can be accessed */
	if ((!authenticated) || (hHDMI->HdcpVersion == BHDM_HDCP_Version_eUnused))
	{
#if 0
		/* message for i2c register access debuggingitg */
		BDBG_MSG((" Authenticated: %d Register Access at offset %#x allowed",
			authenticated, offset)) ;
#endif
		accessAllowed = true ;
		goto done ;
	}

	if (((hHDMI->HdcpVersion >= BHDM_HDCP_Version_e2_2) && (offset < BHDM_HDCP_2_2_OFFSET_START))
	||  ((hHDMI->HdcpVersion < BHDM_HDCP_Version_e2_2) && (offset >= BHDM_HDCP_2_2_OFFSET_START)))
	{
		accessAllowed = false ;
		BDBG_MSG(("Tx%d Disable access to HDCP %s register %#x while HDCP %s link is enabled",
			hHDMI->eCoreId, (hHDMI->HdcpVersion >= BHDM_HDCP_Version_e2_2) ? "1.x" : "2.x",
			offset, (hHDMI->HdcpVersion >= BHDM_HDCP_Version_e2_2) ? "2.x" : "1.x")) ;
		goto done ;
	}

	BDBG_MSG(("Authenticated: %s I2c Offset %#x  %s",
		authenticated ? "yes" : "no" ,
		offset, accessAllowed ? "yes" : "no" )) ;

done:
	return accessAllowed ;

}

/******************************************************************************
BERR_Code BHDM_HDCP_ReadRxBksv
Summary: Read the HDCP Bksv value from the receiver
*******************************************************************************/
BERR_Code BHDM_HDCP_ReadRxBksv(
   const BHDM_Handle hHDMI,              /* [in] HDMI handle */
   const uint8_t *pRevokedKsvList, /* [in] pointer to Revoked KSV List */
   const uint16_t uiRevokedKsvCount /* [in] number of KSVs in Revoked Ksv List */
)
{
	BERR_Code   rc = BERR_SUCCESS;

	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset ;
	uint32_t    BksvRegisterValue ;
	unsigned char   RxBksv[] = { 0x00, 0x01, 0x02, 0x03, 0x04 }; /* LSB...MSB */
	/*
	**	FYI B1 Bksv Test Value from HDCP Specification
	**     RxBksv[] = { 0xcd, 0x1a, 0xf2, 0x1e, 0x51 };
	*/
	uint8_t BCaps ;
	uint8_t RxDeviceAttached ;
	bool bRevoked ;

	BDBG_ENTER(BHDM_HDCP_ReadRxBksv) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		BDBG_WRN(("Tx%d: No DVI/HDMI Device Found", hHDMI->eCoreId)) ;
		goto done ;
	}

	/* read the BCaps first */
	BCaps = 0 ;
	BHDM_CHECK_RC(rc, BHDM_HDCP_GetRxCaps(hHDMI, &BCaps)) ;


#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
	BCaps |= BHDM_HDCP_RxCaps_eHdcpRepeater ;
#endif


	/* We can't access I2C if control has been handed over to HW */
	if (hHDMI->bAutoRiPjCheckingEnabled)
	{
		BKNI_Memcpy(RxBksv, hHDMI->HDCP_RxKsv, BHDM_HDCP_KSV_LENGTH) ;
	}
	else
	{
		/* try to read the RxBksv */
#if BHDM_CONFIG_MHL_SUPPORT
		rc = BHDM_MHL_P_Hdcp_GetRxBksv(hHDMI, RxBksv);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to get RxBksv."));
			goto done;
		}
#else
		/* check if HDCP offset can be accessed if not, return not available */
		if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_RX_BKSV0))
		{
			BDBG_ERR(("I2c access of Rx Bksv0 (%#x) is unavailable", BHDM_HDCP_RX_BKSV0)) ;
			rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
			goto done ;
		}

		rc = BHDM_P_BREG_I2C_Read(hHDMI, BHDM_HDCP_RX_I2C_ADDR,
			BHDM_HDCP_RX_BKSV0, RxBksv, BHDM_HDCP_KSV_LENGTH ) ;

		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Tx%d: Bksv I2C read error", hHDMI->eCoreId));
			rc = BERR_TRACE(BHDM_HDCP_RX_BKSV_I2C_READ_ERROR) ;
			goto done;
		}
#endif
	}

	/* Verify RxBksv has 20 zeros & 20 ones */
	if (BHDM_HDCP_P_NumberOfSetBits(RxBksv, BHDM_HDCP_KSV_LENGTH) != 20)
	{
		BDBG_ERR(("Tx%d: Valid RxBksv contain 20 1s and 20 0s", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_HDCP_RX_BKSV_ERROR) ;
		goto done ;
	}

	/* display changes in KSV for debugging */
	if (BKNI_Memcmp(RxBksv, hHDMI->HDCP_RxKsv, BHDM_HDCP_KSV_LENGTH))
	{
		BDBG_MSG(("Tx%d: RxBksv = %02x %02x %02x %02x %02x", hHDMI->eCoreId,
				RxBksv[4], RxBksv[3], RxBksv[2], RxBksv[1], RxBksv[0]));
	}


	BKNI_Memcpy(hHDMI->HDCP_RxKsv, RxBksv, BHDM_HDCP_KSV_LENGTH) ;


	/* check the retrieved KSV against the Revoked KSV List */
	if (uiRevokedKsvCount)
	{
		BHDM_HDCP_P_CheckRevokedKsvList(
			(uint8_t *) pRevokedKsvList, uiRevokedKsvCount, RxBksv, 1, &bRevoked) ;

		if (bRevoked)
		{
			rc = BERR_TRACE(BHDM_HDCP_RX_BKSV_REVOKED) ;
			goto done ;
		}
	}

	/* write the 4 LSBs RxBksv to the transmitter... */
	BksvRegisterValue =
		  RxBksv[0]
		| RxBksv[1] <<  8
		| RxBksv[2] << 16
		| RxBksv[3] << 24 ;

	Register = BCHP_FIELD_DATA(HDMI_BKSV0, I_BKSV_31_0, BksvRegisterValue) ;
	BREG_Write32(hRegister, BCHP_HDMI_BKSV0 + ulOffset, Register) ;
#if 0
	BDBG_MSG(("Tx%d: BKSV0 Register: 0x%08X", hHDMI->eCoreId, Register)) ;
#endif


	/*
	-- write the 1 MSB RxBksv to the transmitter...
	-- also check if we are authenticating with a repeater
	*/
	BksvRegisterValue = RxBksv[4];
	Register = BCHP_FIELD_DATA(HDMI_BKSV1, I_BKSV_39_32, BksvRegisterValue)
	        |  BCHP_FIELD_DATA(HDMI_BKSV1, BCAPS_7_0, BCaps) ;
#if 0
	BDBG_MSG(("Tx%d: BKSV1 Register Value: 0x%08X", hHDMI->eCoreId, Register)) ;
#endif
	BREG_Write32(hRegister, BCHP_HDMI_BKSV1 + ulOffset, Register) ;

done:
	BDBG_LEAVE(BHDM_HDCP_ReadRxBksv) ;
	return rc ;
} /* end BHDM_HDCP_ReadRxBksv */


#if BHDM_CONFIG_DEBUG_FORCE_FIFO_ERROR
	/* Tested FIFO Recovery with RB 20120326 */
BERR_Code BHDM_HDCP_DEBUG_P_ForceValidVideoFailure(const BHDM_Handle hHDMI)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RM_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RM_CONTROL, TRACKING_RANGE) ;
	Register |= BCHP_FIELD_DATA(HDMI_RM_CONTROL, TRACKING_RANGE, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RM_CONTROL + ulOffset, Register) ;

#if 0
	BREG_Write32(hRegister, BCHP_HDMI_RM_SKIP_REPEAT_GAP + ulOffset, 0x1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RM_SKIP_REPEAT_NUMBER + ulOffset, 0x30) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RM_SKIP_REPEAT_CONTROL + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RM_SKIP_REPEAT_CONTROL, ENABLE) ;
	BREG_Write32(hRegister, BCHP_HDMI_RM_SKIP_REPEAT_CONTROL + ulOffset, Register) ;
		Register |= BCHP_FIELD_DATA(HDMI_RM_SKIP_REPEAT_CONTROL, ENABLE, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RM_SKIP_REPEAT_CONTROL + ulOffset, Register) ;
#endif

#if 0
	BDBG_WRN(("Tx%d: Force FIFO Error", hHDMI->eCoreId)) ;
	Register = BREG_Read32(hRegister, BCHP_HDMI_RM_OFFSET + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RM_OFFSET, OFFSET_ONLY) ;
	{
		uint32_t offset ;

		offset = BCHP_GET_FIELD_DATA(Register, HDMI_RM_OFFSET, OFFSET) ;
		BDBG_WRN(("Tx%d: !!!!!!!!!!!!!!!!!!!!!offset %d", hHDMI->eCoreId, offset)) ;
		offset = (offset * 5000) / 5001 ;
		BDBG_WRN(("Tx%d: !!!!!!!!!!!!!! offset (1percent) %d", hHDMI->eCoreId, offset)) ;
		Register &= BCHP_MASK(HDMI_RM_OFFSET, OFFSET) ;

		Register |= BCHP_FIELD_DATA(HDMI_RM_OFFSET, OFFSET, offset) ;
		Register |= BCHP_FIELD_DATA(HDMI_RM_OFFSET, OFFSET_ONLY, 0) ;
		BREG_Write32(hRegister, BCHP_HDMI_RM_OFFSET + ulOffset, Register) ;
	}

	BKNI_Sleep(3) ;
	BHDM_CHECK_RC(rc, BHDM_HDCP_CheckForValidVideo(hHDMI)) ;

	BDBG_LOG(("Tx%d: DONE DONE DONE Valid Video OK AT LINE %d", hHDMI->eCoreId, __LINE__)) ;
#endif


done:
	if(rc)
	{
        rc = BERR_TRACE(rc);
		BDBG_ERR(("Tx%d: Video NOT VALID!!!!!!!! AT LINE %d", hHDMI->eCoreId, __LINE__)) ;
	}
	return rc ;

}
#endif


/******************************************************************************
BERR_Code BHDM_HDCP_GenerateAn
Summary: Generate the HDCP An value for HDCP calculations
*******************************************************************************/
BERR_Code BHDM_HDCP_GenerateAn(
	const BHDM_Handle hHDMI,              /* [in] HDMI handle */
	BHDM_HDCP_AnSelect AnSelection) /* [in] HDCP An type value to use */
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset;
	uint32_t	AnMsb, AnLsb ;
	uint8_t i ;
	uint8_t timeoutFrames ;
	uint8_t AnReady ;
	uint8_t AnValue[BHDM_HDCP_AN_LENGTH] ;
	bool 	bTestAnGenerated = false ;
	bool bHdcpCapable ;

#define NUM_TEST_AN_VALUES 4
static const uint8_t ucHdcpSpecTestAnValues[NUM_TEST_AN_VALUES][BHDM_HDCP_AN_LENGTH] =
	{
		{ 0x03, 0x04, 0x07, 0x0c, 0x13, 0x1c, 0x27, 0x34},  /* A1/B1 */
		{ 0xe5, 0x0f, 0xd1, 0x3a, 0xa5, 0x62, 0x5e, 0x44},  /* A1/B2 */
		{ 0x07, 0x6e, 0xc6, 0x01, 0xbb, 0xc2, 0xbe, 0x83},  /* A2/B1 */
		{ 0x4d, 0xa7, 0x06, 0x54, 0x17, 0xf7, 0x51, 0x03}   /* A2/B2 */
	} ;


	BDBG_ENTER(BHDM_HDCP_GenerateAn) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->bHdcpAnRequest)
	{
		BDBG_ERR(("Tx%d: HDCP An value already requested", hHDMI->eCoreId)) ;
		return BHDM_HDCP_MULTIPLE_AN_REQUEST ;
	}

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* new authentication attempt */
	BDBG_MSG(("HDCP Authentication Request (URSR %s %d)",
		BHDM_P_GetVersion(), BCHP_CHIP)) ;


	/* For platforms with both HDCP 1.x and HDCP 2.x capable. Make sure to select HDCP 1.x cipher */
#if BHDM_CONFIG_HAS_HDCP22
	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_CFG0  + ulOffset);
		Register &= ~(BCHP_MASK(HDMI_HDCP2TX_CFG0, HDCP_VERSION_SELECT));
		Register |= BCHP_FIELD_DATA(HDMI_HDCP2TX_CFG0, HDCP_VERSION_SELECT, 0x0);
	BREG_Write32(hRegister, BCHP_HDMI_HDCP2TX_CFG0	+ ulOffset, Register) ;
#endif


#if BHDM_CONFIG_DEBUG_PJ_CHECKING
	BDBG_WRN(("Tx%d: !!!!! Pj Check Test... video will be solid green or magenta color !!!!! ", hHDMI->eCoreId)) ;
	BDBG_WRN(("Tx%d: !!!!! Ri Sequence should be consistent on every authentication  !!!!! ", hHDMI->eCoreId)) ;
	Register = BREG_Read32(hRegister,  BCHP_HDMI_CP_TST + ulOffset) ;
	Register &=
		~(BCHP_MASK(HDMI_CP_TST, I_TST_FORCE_VIDEO_ALL_ONES)
		| BCHP_MASK(HDMI_CP_TST, I_TST_FORCE_VIDEO_ALL_ZEROS)) ;

	Register |= BCHP_FIELD_DATA(HDMI_CP_TST,  I_TST_FORCE_VIDEO_ALL_ZEROS, 1) ;

	BREG_Write32(hRegister, BCHP_HDMI_CP_TST + ulOffset, Register) ;
	AnSelection = BHDM_HDCP_AnSelect_eTestA1B1An ;
#endif


	bHdcpCapable = false ;
	BHDM_CHECK_RC(rc, BCHP_GetFeature(hHDMI->hChip, BCHP_Feature_eHdcpCapable, &bHdcpCapable)) ;
	if (!bHdcpCapable)
	{
		BDBG_ERR(("#############################")) ;
		BDBG_ERR(("HDCP is DISABLED on this part")) ;
		BDBG_ERR(("#############################")) ;
		rc = BERR_TRACE(BERR_NOT_SUPPORTED) ;
		goto done ;
	}

	/* clear previously received values; for debug reporting */
	BKNI_Memset(hHDMI->HDCP_RxKsv, 0, BHDM_HDCP_KSV_LENGTH) ;
	hHDMI->RxBCaps = 0 ;



#if BHDM_CONFIG_DEBUG_FORCE_FIFO_ERROR
	/* Tested FIFO Recovery w/ RB 20120326 */
	BHDM_CHECK_RC(rc, BHDM_HDCP_DEBUG_P_ForceValidVideoFailure(hHDMI)) ;
#endif

	Register = BREG_Read32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_SCHEDULER_CONTROL, ALWAYS_VERT_KEEP_OUT) ;

	Register |= BCHP_FIELD_DATA(HDMI_SCHEDULER_CONTROL, ALWAYS_VERT_KEEP_OUT, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset, Register) ;

	hHDMI->bHdcpAnRequest = true ;
 	do
	{
		/* set up for Test An values if requested */
		if (AnSelection != BHDM_HDCP_AnSelect_eRandomAn)
		{
			/* for HDCP Test An values, make sure the influence bits are off */
			Register = BREG_Read32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset) ;
			Register &= ~BCHP_MASK(HDMI_CP_CONFIG, AN_INFLUENCE_MODE) ;
			Register |= BCHP_FIELD_DATA(HDMI_CP_CONFIG, AN_INFLUENCE_MODE, 0) ;
			BREG_Write32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset, Register) ;

			/* Set An Selection and TestAn Enable */
			Register = BREG_Read32(hRegister, BCHP_HDMI_CP_TST + ulOffset) ;
			Register &=
				~( BCHP_MASK(HDMI_CP_TST, I_TST_AN_SEL_1_0)
				 | BCHP_MASK(HDMI_CP_TST, I_TST_MODE_AN_ENABLE)) ;
			Register |=
				  BCHP_FIELD_DATA(HDMI_CP_TST, I_TST_AN_SEL_1_0, AnSelection)
				| BCHP_FIELD_DATA(HDMI_CP_TST, I_TST_MODE_AN_ENABLE, 1) ;
			BREG_Write32(hRegister, BCHP_HDMI_CP_TST + ulOffset, Register) ;
		}

		/* Check for completion of the generated HDCP An Value */
		/* should complete within 1-2 frames */
		timeoutFrames = 6 ;
		do
		{
			/* request the authentication values */
			/* Set AUTH_REQUEST_BIT only - all other bits must be zero */
			Register = BCHP_FIELD_DATA(HDMI_HDCP_CTL, I_AUTH_REQUEST, 1) ;
			BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

			/* HDCP An value *should* be generated after one frame;  */
			/* Wait up to  3+ frames */
			/* NOTE: Video flow from BVN to HDMI must be stable when generating An value */
			BHDM_CHECK_RC(rc, BKNI_WaitForEvent(hHDMI->BHDM_EventHDCP, BHDM_HDCP_CONFIG_AN_TIMEOUT_MS)) ;

 			Register = BREG_Read32(hRegister, BCHP_HDMI_CP_STATUS + ulOffset) ;
			AnReady = BCHP_GET_FIELD_DATA(Register, HDMI_CP_STATUS, O_AN_READY) ;
			if (AnReady)
				break ;

			BDBG_WRN(("Tx%d: Waitng for HDCP An Value", hHDMI->eCoreId)) ;
		} while ( timeoutFrames-- ) ;

		if (!AnReady)
		{
			BDBG_ERR(("Tx%d: Timeout Error waiting for HDCP An Value", hHDMI->eCoreId)) ;

			rc = BERR_TRACE(BERR_TIMEOUT);
			goto done ;
		}


		/* read the generated HDCP An value / write to the HDCP Rx */
		AnMsb = BREG_Read32(hRegister, BCHP_HDMI_AN1 + ulOffset) ; /* 32 MSB */
		AnLsb = BREG_Read32(hRegister, BCHP_HDMI_AN0 + ulOffset) ; /* 32 LSB */

		/* copy the AnValue to a uint8_t string for single I2C write to the Rx */
		AnValue[0] =  AnLsb        & 0xFF ;
		AnValue[1] = (AnLsb >>  8) & 0xFF ;
		AnValue[2] = (AnLsb >> 16) & 0xFF ;
		AnValue[3] = (AnLsb >> 24) & 0xFF ;

		AnValue[4] =  AnMsb        & 0xFF ;
		AnValue[5] = (AnMsb >>  8) & 0xFF ;
		AnValue[6] = (AnMsb >> 16) & 0xFF ;
		AnValue[7] = (AnMsb >> 24) & 0xFF ;

		/* verify the generated An value is not one of the test values if we are generating random numbers */
		if (AnSelection == BHDM_HDCP_AnSelect_eRandomAn)  /* for Test An Values */
		{
			for (i = 0 ; i < NUM_TEST_AN_VALUES ; i++)
			{
#if 0
				BDBG_MSG(("Tx%d: Compare generated RandomAn to TestAn %02x%02x%02x%02x%02x%02x%02x%02x ",
					hHDMI->eCoreId,
					ucHdcpSpecTestAnValues[i][7], ucHdcpSpecTestAnValues[i][6],
					ucHdcpSpecTestAnValues[i][5], ucHdcpSpecTestAnValues[i][4],
					ucHdcpSpecTestAnValues[i][3], ucHdcpSpecTestAnValues[i][2],
					ucHdcpSpecTestAnValues[i][1], ucHdcpSpecTestAnValues[i][0])) ;
#endif
				if (!BKNI_Memcmp(AnValue, ucHdcpSpecTestAnValues[i], BHDM_HDCP_AN_LENGTH))
				{
					bTestAnGenerated = true ;
					break ;
				}
			}
		}

		BDBG_MSG(("Tx%d: AnValue = %02x%02x%02x%02x%02x%02x%02x%02x ", hHDMI->eCoreId,
			AnValue[7], AnValue[6],	AnValue[5], AnValue[4],
			AnValue[3], AnValue[2],	AnValue[1], AnValue[0])) ;
	} while ((AnSelection == BHDM_HDCP_AnSelect_eRandomAn) && (bTestAnGenerated)) ;


#if BHDM_CONFIG_HDCP_FORCE_ENC_SIGNAL
	/*
	    For HDCP Compliance Test ...
	    make sure the RDB Auth is set so at least the ENC_DIS signal is always sent
	    when MUX_VSYNC is enabled after authentication, ENC_EN will be transmitted
	  */

	/* Set SET_RDB_AUTHENTICATED_BIT only - all other bits must be zero */
	Register = BCHP_FIELD_DATA(HDMI_HDCP_CTL, I_SET_RDB_AUTHENTICATED, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;
#endif

#if BHDM_CONFIG_MHL_SUPPORT
	rc = BHDM_MHL_P_Hdcp_SendAnValue(hHDMI, (uint8_t *)AnValue);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed to send AnValue."));
	}
#else
	/* write the generate An value to the Receiver */
	BHDM_CHECK_RC(rc, BREG_I2C_Write(hHDMI->hI2cRegHandle,
		BHDM_HDCP_RX_I2C_ADDR,	BHDM_HDCP_RX_AN0, (uint8_t *) AnValue, BHDM_HDCP_AN_LENGTH)) ;
#endif

done:
	hHDMI->bHdcpAnRequest = false ;
	BDBG_LEAVE(BHDM_HDCP_GenerateAn) ;
	return rc ;
} /* end BHDM_HDCP_GenerateAn */


/******************************************************************************
BERR_Code BHDM_HDCP_WriteTxAksvToRx
Summary: Write the HDCP Aksv value to the receiver
*******************************************************************************/
BERR_Code BHDM_HDCP_WriteTxAksvToRx(
   const BHDM_Handle hHDMI,           /* [in] HDMI handle */
   const uint8_t *pTxAksv       /* [in] pointer HDCP Key Set Aksv Value */
)
{
	static const uint8_t HdcpSpecTestSet1Aksv[BHDM_HDCP_KSV_LENGTH] =
		{0x14, 0xF7, 0x61, 0x03, 0xB7} ;

	BERR_Code   rc = BERR_SUCCESS;
	uint8_t RxCaps;
	uint8_t AinfoByte;

	BDBG_ENTER(BHDM_HDCP_WriteTxAksvToRx) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* Verify TxAksv has 20 zeros & 20 ones */

	/* display KSV for debugging */
	BDBG_MSG(("Tx%d: TxAksv = %02x %02x %02x %02x %02x", hHDMI->eCoreId,
			pTxAksv[4], pTxAksv[3], pTxAksv[2], pTxAksv[1], pTxAksv[0]));

	/* check for valid KSV */
	if (BHDM_HDCP_P_NumberOfSetBits((uint8_t *) pTxAksv, BHDM_HDCP_KSV_LENGTH) != 20)
	{
		BDBG_ERR(("Tx%d: Valid TxAksv must contain 20 1s and 20 0s", hHDMI->eCoreId)) ;

		/* display KSV for debugging */
		BDBG_ERR(("Tx%d: TxAksv = %02x %02x %02x %02x %02x", hHDMI->eCoreId,
			pTxAksv[4], pTxAksv[3], pTxAksv[2], pTxAksv[1], pTxAksv[0]));
		rc = BERR_TRACE(BHDM_HDCP_TX_AKSV_ERROR) ;
		goto done ;
	}

	/* make sure KSV is not from the Test Key Set*/
	if (!BKNI_Memcmp(HdcpSpecTestSet1Aksv, pTxAksv, BHDM_HDCP_KSV_LENGTH))
	{
		BDBG_ERR(("\n\n\n")) ;
		BDBG_ERR(("******************************")) ;
		BDBG_ERR(("Provided TxAksv = %02x %02x %02x %02x %02x part of HDCP1.x Test Keys",
				pTxAksv[4], pTxAksv[3], pTxAksv[2], pTxAksv[1], pTxAksv[0]));
		BDBG_ERR(("The HDCP keys provided are HDCP 1.x non-production keys. HDCP1.x Authentication failed.")) ;
		BDBG_ERR(("HDCP1.x authentication with Production HDMI Receivers (production DTV) requires production HDCP keys."));
		BDBG_ERR(("Please contact your HDCP development team for assistance.")) ;
		BDBG_ERR(("******************************\n\n\n")) ;
		rc = BERR_TRACE(BHDM_HDCP_TX_AKSV_ERROR) ;
		goto done ;
	}


	/* Write the AInfo value */
	BHDM_CHECK_RC(rc, BHDM_HDCP_GetRxCaps(hHDMI, &RxCaps)) ;

    if ((RxCaps & BHDM_HDCP_RxCaps_eHDCP_1_1_Features)
	&& (hHDMI->HdcpOptions.PjChecking)
	&& (hHDMI->HdcpOptions.numPjFailures <= BHDM_HDCP_MAX_PJ_LINK_FAILURES_BEFORE_DISABLE_HDCP_1_1))
	{
		AinfoByte = BHDM_HDCP_RX_ENABLE_1_1_FEATURES ;

#if BHDM_CONFIG_MHL_SUPPORT
		rc = BHDM_MHL_P_Hdcp_SendAinfoByte(hHDMI, &AinfoByte);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to send AinfoByte."));
			goto done;
		}
#else

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
		if (!hHDMI->bAutoRiPjCheckingEnabled)
		{
			BHDM_CHECK_RC(rc, BREG_I2C_Write(hHDMI->hI2cRegHandle,
				BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_AINFO, (uint8_t *) &AinfoByte, 1)) ;
		}
#else
		BHDM_CHECK_RC(rc, BREG_I2C_Write(hHDMI->hI2cRegHandle,
			BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_AINFO, (uint8_t *) &AinfoByte, 1)) ;
#endif

#endif
	}

#if BHDM_CONFIG_MHL_SUPPORT
	rc = BHDM_MHL_P_Hdcp_SendTxAksv(hHDMI, pTxAksv);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed to send TxAksv."));
		goto done;
	}
#else
	/* Write the TxAksv value to the HDCP Rx */
	rc = BREG_I2C_Write(hHDMI->hI2cRegHandle, BHDM_HDCP_RX_I2C_ADDR,
		BHDM_HDCP_RX_AKSV0, (uint8_t *) pTxAksv, BHDM_HDCP_KSV_LENGTH) ;
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Tx%d: Aksv I2C write error", hHDMI->eCoreId));
		rc = BERR_TRACE(BHDM_HDCP_TX_AKSV_I2C_WRITE_ERROR) ;
		goto done;
	}
#endif

	/*
	consider the authentication request started once the TxAksv
	is written to the Rx; clear the authenticated link bit
	*/
	hHDMI->AbortHdcpAuthRequest = 0 ;

	/* clear AuthenticatedLink Variable */
	hHDMI->HDCP_AuthenticatedLink = 0 ;


done :
	BDBG_LEAVE(BHDM_HDCP_WriteTxAksvToRx) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_HDCP_EnableSerialKeyLoad(
Summary: Enable HDMI transmitter core to receive the HDCP Keys serially.
*******************************************************************************/
BERR_Code BHDM_HDCP_EnableSerialKeyLoad(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset ;

	BDBG_ENTER(BHDM_HDCP_EnableSerialKeyLoad) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_CP_CONFIG, I_ENABLE_RDB_KEY_LOAD) ;
	Register |= BCHP_FIELD_DATA(HDMI_CP_CONFIG, I_ENABLE_RDB_KEY_LOAD, 0x0) ;
	BREG_Write32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_CP_CONFIG, I_KEY_BASE_ADDRESS_9_0) ;
	Register |= BCHP_FIELD_DATA(HDMI_CP_CONFIG, I_KEY_BASE_ADDRESS_9_0, 0x80) ;
	BREG_Write32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset, Register) ;

	hHDMI->AbortHdcpAuthRequest = 0 ;

	BDBG_LEAVE(BHDM_HDCP_EnableSerialKeyLoad) ;
	return rc ;
}




/******************************************************************************
BERR_Code BHDM_HDCP_AuthenticateLink
Summary: Authenticate the HDCP Link; verify TxR0 and RxR0 values are equal
*******************************************************************************/
BERR_Code BHDM_HDCP_AuthenticateLink
(
	const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;

	uint32_t    Register, ulOffset ;
	uint8_t     timeoutMs ;

	uint8_t
		RdbAuthenticated ,
		SchedulerAuthenticated,
		CoreAuthenticated ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;
	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* Before checking Authentication... */
	/* Reset Ri counter and previous Ri Values */
	hHDMI->HDCP_RiCount = 0 ;

	hHDMI->HDCP_Ri2SecsAgo = 0x01 ;
	hHDMI->HDCP_Ri4SecsAgo = 0x01 ;
	hHDMI->HDCP_Ri6SecsAgo = 0x01 ;

	hHDMI->HDCP_AuthenticatedLink = 0 ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	hHDMI->bAutoRiPjCheckingEnabled = false;
#endif

	BDBG_MSG(("HDCP Ri check configured for %s read",
		BHDM_CONFIG_HDCP_RI_SHORT_READ ? "SHORT" : "NORMAL" )) ;

	if ((rc = BHDM_HDCP_RiLinkIntegrityCheck(hHDMI)) != BERR_SUCCESS)
	{
		BDBG_ERR(("Tx%d: HDCP Authentication Failed", hHDMI->eCoreId));
		goto done ;
	}


	/* SET_RDB_AUTHENTICATED must be asserted to ensure ENC_EN/ENC_DIS
	signal is sent either via MUX_VSYNC or by the HW */

	/* Set SET_RDB_AUTHENTICATED_BIT only - all other bits must be zero */
	Register = BCHP_FIELD_DATA(HDMI_HDCP_CTL, I_SET_RDB_AUTHENTICATED, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;


	/* wait up to 50ms for the core to authenticate; should authenticate within 1 field */
	timeoutMs = 5 ;
	do
	{
		/* make sure HDCP request is still active... hotplug will abort request  */
		if (hHDMI->AbortHdcpAuthRequest)
		{
			BDBG_ERR(("Tx%d: HDCP Authentication Request Aborted....", hHDMI->eCoreId)) ;
			rc = BERR_TRACE(BHDM_HDCP_AUTH_ABORTED) ;
			goto done ;
		}

		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_STATUS + ulOffset) ;

		RdbAuthenticated = BCHP_GET_FIELD_DATA(Register, HDMI_CP_STATUS, RDB_AUTHENTICATED) ;
		SchedulerAuthenticated = BCHP_GET_FIELD_DATA(Register, HDMI_CP_STATUS, AUTHENTICATED_OK) ;
		CoreAuthenticated = BCHP_GET_FIELD_DATA(Register, HDMI_CP_STATUS, CORE_AUTHENTICATED) ;

		if (RdbAuthenticated
		&&  SchedulerAuthenticated
		&&  CoreAuthenticated)
			break ;

		BDBG_WRN(("Tx%d: Waiting for TxCore to Authenticate %d %d %d", hHDMI->eCoreId,
			RdbAuthenticated, SchedulerAuthenticated, CoreAuthenticated)) ;

		BKNI_Sleep(10) ;
	} while ( timeoutMs-- ) ;

	if (!RdbAuthenticated)
	{
		BDBG_ERR(("Tx%d: HDCP Tx RDB not authenticated", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_HDCP_AUTHENTICATE_ERROR) ;
		goto done ;
	}
	else if (!SchedulerAuthenticated)
	{
		BDBG_ERR(("Tx%d: HDCP Tx Scheduler not authenticated", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_HDCP_AUTHENTICATE_ERROR) ;
		goto done ;
	}
	else if (!CoreAuthenticated)
	{
		BDBG_ERR(("Tx%d: HDCP Tx Core not authenticated", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_HDCP_AUTHENTICATE_ERROR) ;
		goto done ;
	}


	hHDMI->HDCP_AuthenticatedLink = 1 ;
	BDBG_MSG(("Tx%d: Receiver AUTHENTICATED", hHDMI->eCoreId)) ;


	/* Reset the Ri values again so when Video transmission begins */
	/* HDCP doesn't think Ri is not advancing */
	hHDMI->HDCP_Ri2SecsAgo = 0x0102 ;
	hHDMI->HDCP_Ri4SecsAgo = 0x0203 ;
	hHDMI->HDCP_Ri6SecsAgo = 0x0304 ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
#if BHDM_CONFIG_MHL_SUPPORT
	if (!hHDMI->bMhlMode)
#endif
	{
		if (hHDMI->DeviceSettings.bEnableAutoRiPjChecking)
		{
			/* Configure/setup auto Ri */
			BHDM_HDCP_P_ConfigureAutoRi(hHDMI);

			/* Configure/setup auto Pj */
			if (hHDMI->HdcpOptions.PjChecking)
				BHDM_HDCP_P_ConfigureAutoPj(hHDMI);

	                /* Cache RxBCaps and RxKSV before we switch over to auto HW checking. */
	                BHDM_CHECK_RC(rc, BHDM_HDCP_GetRxCaps(hHDMI, &hHDMI->RxBCaps)) ;
	                BHDM_CHECK_RC(rc, BHDM_HDCP_GetRxKsv(hHDMI, &hHDMI->HDCP_RxKsv[0])) ;

	                /* If non-repeater, switch over from software to auto HW checking now. */
	                if ((hHDMI->RxBCaps & BHDM_HDCP_RxCaps_eHdcpRepeater) == 0)
	                {
	                        BHDM_HDCP_P_EnableAutoRiPjChecking(hHDMI, hHDMI->HdcpOptions.PjChecking) ;
	                }
		}
	}
#endif

done:
	return rc ;
} /* end BHDM_HDCP_AuthenticateLink */




/******************************************************************************
BERR_Code BHDM_HDCP_ClearAuthentication(
Summary: Clear the Authenticated link between the Transmitter and the Receiver.
*******************************************************************************/
BERR_Code BHDM_HDCP_ClearAuthentication(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset ;

	BDBG_ENTER(BHDM_HDCP_ClearAuthentication) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* Clear the MuxVsync Bit First (eliminate momentary snow at the Rx) */
	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_CP_CONFIG, I_MUX_VSYNC) ;
	Register |= BCHP_FIELD_DATA(HDMI_CP_CONFIG, I_MUX_VSYNC, 0) ;
	BREG_Write32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset, Register) ;

	/* Wait long enough for the current frame to complete... */
	BKNI_Sleep(20) ;  /* wait */


#if BHDM_CONFIG_HDCP_FORCE_ENC_SIGNAL
	/*
	Clearing the I_CLEAR_RDB_AUTHENTICATED_BIT has been removed to fix HDCP
	Compliance Test Issue... the CLEAR_RDB_AUTHENTICATED should never asserted

	The Compliance Test checks to make sure either ENC_DIS or ENC_EN
	is always transmitted.	Setting  the core as always authenticated will
	allow the MUX_VSYNC to control the ENC_DIS/ENC_EN signal
	*/

#else
	/* Set the Tx Authentication Cleared Bit */

	/* Set CLEAR_RDB_AUTHENTICATED BIT only - all other bits must be zero */
	Register = BCHP_FIELD_DATA(HDMI_HDCP_CTL, I_CLEAR_RDB_AUTHENTICATED, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

	Register = BCHP_FIELD_DATA(HDMI_HDCP_CTL, I_CLEAR_RDB_AUTHENTICATED, 0) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

#endif

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	BHDM_HDCP_P_DisableAutoRiPjChecking (hHDMI);
#endif
	/* clear AuthenticatedLink Variable */
	hHDMI->HDCP_AuthenticatedLink = 0 ;

	/* Reset Ri counter */
	hHDMI->HDCP_RiCount = 0 ;

	BDBG_LEAVE(BHDM_HDCP_ClearAuthentication) ;
	return rc ;

} /* end BHDM_HDCP_ClearAuthentication */




/******************************************************************************
BERR_Code BHDM_HDCP_XmitEncrypted
Summary:Start transmitting video encrypted with HDCP.
*******************************************************************************/
BERR_Code BHDM_HDCP_XmitEncrypted(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset ;
	uint16_t JRate ;

	BDBG_ENTER(BHDM_HDCP_XmitEncrypted) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	if ((hHDMI->HdcpVersion == BHDM_HDCP_Version_e1_1) /* same as hHDMI->HdcpVersion == BHDM_HDCP_Version_e1_0*/
	||	(hHDMI->HdcpVersion == BHDM_HDCP_Version_eUnused))
	{
		BDBG_MSG(("Tx%d: Using HDCP 1.1 Features", hHDMI->eCoreId)) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CFG + ulOffset) ;
		Register &= ~BCHP_MASK(HDMI_CP_INTEGRITY_CFG, I_ALWAYS_REKEY_ON_VSYNC) ;
		BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CFG + ulOffset, Register) ;
	}
	else
	{
		BDBG_MSG(("Tx%d: Using HDCP 1.1 with Optional Features", hHDMI->eCoreId)) ;

		/* enable ReKey on Vsync for HDCP 1.1 Receivers */
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CFG + ulOffset) ;
		JRate = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CFG, J_RATE_7_0) ;
		BDBG_MSG(("Tx%d: Pj Rate: %d frames", JRate)) ;

		Register &= ~BCHP_MASK(HDMI_CP_INTEGRITY_CFG, I_ALWAYS_REKEY_ON_VSYNC) ;
		Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CFG, I_ALWAYS_REKEY_ON_VSYNC, 1) ;
		BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CFG + ulOffset, Register) ;
	}


	/* enable (toggle) the Vsync for encryption */
	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_CP_CONFIG, I_MUX_VSYNC) ;

	Register |= BCHP_FIELD_DATA(HDMI_CP_CONFIG, I_MUX_VSYNC, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset, Register) ;

	BDBG_LEAVE(BHDM_HDCP_XmitEncrypted) ;
	return rc ;
} /* end BHDM_HDCP_XmitEncrypted */




/******************************************************************************
BERR_Code BHDM_HDCP_XmitClear
Summary: Start transmitting video without HDCP.
*******************************************************************************/
BERR_Code BHDM_HDCP_XmitClear(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset ;

	BDBG_ENTER(BHDM_HDCP_XmitClear) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_CP_CONFIG, I_MUX_VSYNC) ;
	Register |= BCHP_FIELD_DATA(HDMI_CP_CONFIG, I_MUX_VSYNC, 0) ;
	BREG_Write32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset, Register) ;

	BDBG_LEAVE(BHDM_HDCP_XmitClear) ;
	return rc ;
} /* end BHDM_HDCP_XmitClear */


#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
/******************************************************************************
BERR_Code BHDM_HDCP_P_AutoRiLinkIntegrityCheck
Summary: Use hardware Ri checking to verify the protected link is still valid.
*******************************************************************************/
BERR_Code BHDM_HDCP_P_AutoRiLinkIntegrityCheck(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code	rc = BERR_SUCCESS;
	BREG_Handle hRegister ;

	uint32_t	Register, ulOffset ;
	uint16_t	Ri ;
#if BHDM_CONFIG_DEBUG_PJ_CHECKING
	uint8_t  Pj ;
#endif

	BDBG_ENTER(BHDM_HDCP_P_AutoRiLinkIntegrityCheck) ;
	BDBG_ASSERT( hHDMI );

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* Request to abort authentication */
	if (hHDMI->AbortHdcpAuthRequest)
	{
		BDBG_ERR(("Tx%d: HDCP Authentication Request Aborted....", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_HDCP_AUTH_ABORTED) ;
		goto done ;
	}

	/* Ri mismatch interrupt(s) fired. Either on the 127th frame (A) or the 0th frame (B) */
	if (hHDMI->HDCP_AutoRiMismatch_A || hHDMI->HDCP_AutoRiMismatch_B)
	{
		if (hHDMI->HDCP_AutoRiMismatch_A)
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_STATUS_2 + ulOffset) ;
			Ri = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CHK_STATUS_2, O_RI_A_SOURCE) ;
			BKNI_Memcpy(&hHDMI->HDCP_TxRi, &Ri, 2) ;

			Ri = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CHK_STATUS_2, O_RI_A_SINK) ;
			BKNI_Memcpy(&hHDMI->HDCP_RxRi, &Ri, 2) ;

			BDBG_WRN(("Tx%d: HDCP Ri Failure. Sink update Ri' too early", hHDMI->eCoreId));
			BDBG_WRN(("Tx%d: LIC: Frame 127th: TxRi= %04x <> RxRi= %04x		  R%d !=",
				hHDMI->eCoreId, hHDMI->HDCP_TxRi, hHDMI->HDCP_RxRi, hHDMI->HDCP_RiCount)) ;
			hHDMI->HDCP_AutoRiMismatch_A = false;
		}

		if (hHDMI->HDCP_AutoRiMismatch_B)
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_STATUS_3 + ulOffset) ;
			Ri = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CHK_STATUS_3, O_RI_B_SOURCE) ;
			BKNI_Memcpy(&hHDMI->HDCP_TxRi, &Ri, 2) ;

			Ri = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CHK_STATUS_3, O_RI_B_SINK) ;
			BKNI_Memcpy(&hHDMI->HDCP_RxRi, &Ri, 2) ;

			BDBG_WRN(("Tx%d: HDCP Ri Failure. Sink update Ri' too late", hHDMI->eCoreId));
			BDBG_WRN(("Tx%d: LIC: Frame 0th: TxRi= %04x <> RxRi= %04x 		R%d !=",
				hHDMI->eCoreId, hHDMI->HDCP_TxRi, hHDMI->HDCP_RxRi, hHDMI->HDCP_RiCount)) ;
			hHDMI->HDCP_AutoRiMismatch_B = false;
		}

		/* Clear/Disable the HDCP Authentication */
		BHDM_HDCP_ClearAuthentication(hHDMI) ;

		rc = BERR_TRACE(BHDM_HDCP_LINK_RI_FAILURE) ;
		goto done;
	}

	/* No Ri mismatch interrupt was fired. Update Ri values */
	else
	{
		/* Read TxRi at the 0th frame. This is the start of the next 128 frames period, thus the updated Ri value */
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY + ulOffset) ;
		Ri = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY, O_RI_15_0) ;

		BKNI_Memcpy(&hHDMI->HDCP_TxRi, &Ri, 2) ;

		if (! BKNI_Memcmp(&hHDMI->HDCP_TxRi, &hHDMI->HDCP_Ri2SecsAgo, 2))	/* No Ri change from 2secs ago */
		{
			/*
			** The TxRi has not changed, so there is no need to read the RxRi.
			** just verify the TxRi value has changed in the last 6 seconds
			*/

			if (BKNI_Memcmp(&hHDMI->HDCP_TxRi, &hHDMI->HDCP_Ri4SecsAgo, 2)) /* Ri changed from 4secs ago */
			{
				BKNI_Memcpy(&hHDMI->HDCP_Ri4SecsAgo, &hHDMI->HDCP_Ri2SecsAgo, 2) ;
				rc = BERR_SUCCESS ;
				goto done ;
			}

			if (BKNI_Memcmp(&hHDMI->HDCP_TxRi, &hHDMI->HDCP_Ri6SecsAgo, 2)) /* Ri changed from 6secs ago */
			{
				BKNI_Memcpy(&hHDMI->HDCP_Ri6SecsAgo, &hHDMI->HDCP_Ri4SecsAgo, 2) ;
				rc = BERR_SUCCESS ;
				goto done ;
			}

			/*
			** Ri hasn't changed for six seconds
			** Clear the Authentication and return a LINK FAILURE
			*/
			BDBG_ERR(("Tx%d: Ri HDCP Link Integrity (Ri) values are not changing", hHDMI->eCoreId)) ;
			BDBG_ERR(("Tx%d: HDCP Link will shutdown", hHDMI->eCoreId)) ;

			/* Clear/Disable the HDCP Authentication */
			BHDM_HDCP_ClearAuthentication(hHDMI) ;

			rc = BERR_TRACE(BHDM_HDCP_LINK_RI_FAILURE) ;
			goto done ;
		}


		if	((hHDMI->HDCP_TxRi == hHDMI->HDCP_Ri2SecsAgo)  /* No Ri change in 2secs */
		&& (hHDMI->HDCP_TxRi == hHDMI->HDCP_Ri4SecsAgo)  /* No Ri change in 4secs */
		&& (hHDMI->HDCP_TxRi == hHDMI->HDCP_Ri6SecsAgo)) /* No Ri change in 6secs */
		{
			/*
			** if Ri has not changed in 6 seconds  (3 Consecutive Ri reads)
			** the HDCP link is no longer valid
			*/
			rc = BERR_TRACE(BHDM_HDCP_LINK_RI_FAILURE) ;
			goto done ;
		}

		/* Print the Ri value at the last frame of the previous 128 frames period for debug information */
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_STATUS_2 + ulOffset) ;
		Ri = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CHK_STATUS_2, O_RI_A_SOURCE) ;

		BDBG_MSG(("Tx%d: LIC: At 127th frame:  Tx/Rx Ri= %04x 		R%d",
			hHDMI->eCoreId,	Ri, hHDMI->HDCP_RiCount-1)) ;
		BDBG_MSG(("Tx%d: LIC: At 0th frame:  Tx/Rx Ri= %04x			R%d",
			hHDMI->eCoreId, hHDMI->HDCP_TxRi, hHDMI->HDCP_RiCount)) ;

#if BHDM_CONFIG_DEBUG_PJ_CHECKING
		/* Pj values at  Ri updates should equal the LSB of the Ri value */
		/* Note the video pixel must be zero for this test */
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY + ulOffset) ;
		Pj = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY, O_PJ_7_0) ;

		BDBG_MSG(("Tx%d:  			  %04x", hHDMI->eCoreId, Pj)) ;
#endif
		/*	increment  the RiCount only if Ri Changed in the last 2 secs */
		if (hHDMI->HDCP_TxRi != hHDMI->HDCP_Ri2SecsAgo)
			hHDMI->HDCP_RiCount++ ;

		/*
		** RiNSecsAgo values are initialized
		** in BHDM_HDCP_AuthenticateLink
		*/
		hHDMI->HDCP_Ri6SecsAgo = hHDMI->HDCP_Ri4SecsAgo ;
		hHDMI->HDCP_Ri4SecsAgo = hHDMI->HDCP_Ri2SecsAgo ;
		hHDMI->HDCP_Ri2SecsAgo = hHDMI->HDCP_TxRi ;

		/* Return Link is Valid */
		rc = BERR_SUCCESS ;
		goto done ;

	}

done:
	BDBG_LEAVE(BHDM_HDCP_P_AutoRiLinkIntegrityCheck) ;
	return rc ;

}


/******************************************************************************
BERR_Code BHDM_HDCP_P_AutoPjLinkIntegrityCheck
Summary: Use hardware Pj checking to verify the protected link is still valid.
*******************************************************************************/
BERR_Code BHDM_HDCP_P_AutoPjLinkIntegrityCheck(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset ;
	uint8_t 	Pj ;

	BDBG_ENTER(BHDM_HDCP_P_AutoPjLinkIntegrityCheck) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT

	/* Request to abort authentication */
	if (hHDMI->AbortHdcpAuthRequest)
	{
		BDBG_ERR(("Tx%d: HDCP Authentication Request Aborted....", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_HDCP_AUTH_ABORTED) ;
		goto done ;
	}

	/* Read Tx Pj updated value */
	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_STATUS_1 + ulOffset) ;
	Pj = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CHK_STATUS_1, O_PJ_SOURCE) ;
	BKNI_Memcpy(&hHDMI->HDCP_TxPj, &Pj, 1) ;

	/* Pj values mismatched */
	if (hHDMI->HDCP_AutoPjMismatch)
	{
		Pj = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY_CHK_STATUS_1, O_PJ_SINK) ;
		BKNI_Memcpy(&hHDMI->HDCP_RxPj, &Pj, 1) ;

		BDBG_WRN(("Tx%d: Pj LIC: TxPj= %02x <> RxPj= %02x !=", hHDMI->eCoreId,
			hHDMI->HDCP_TxPj, hHDMI->HDCP_RxPj)) ;

		/* could be a pixel transmission error; ignore if errs < MAX MisMatch */
		if (++hHDMI->HDCP_PjMismatchCount < BHDM_HDCP_MAX_PJ_MISMATCH)
		{
			BDBG_WRN(("Tx%d: Pixel Transmission Error?? Pj Mis-match Count %d",
				hHDMI->eCoreId, hHDMI->HDCP_PjMismatchCount)) ;
		}
		else   /* otherwise fail the link */
		{
			rc = BERR_TRACE(BHDM_HDCP_LINK_PJ_FAILURE) ;

			/* Clear/Disable the HDCP Authentication */
			BHDM_HDCP_ClearAuthentication(hHDMI) ;
			goto done;
		}
	}


	/* Pjs match... */
	BDBG_MSG(("Tx%d: Pj LIC: Tx/Rx Pj: %02x", hHDMI->eCoreId, Pj)) ;
	hHDMI->HDCP_PjMismatchCount = 0 ;

	/* Return Link is Valid */
	rc = BERR_SUCCESS ;;

#endif

done:
	BDBG_LEAVE(BHDM_HDCP_P_AutoPjLinkIntegrityCheck) ;
	return rc ;

}
#endif

/******************************************************************************
BERR_Code BHDM_HDCP_RiLinkIntegrityCheck
Summary: Use the HDCP Ri values to verify the protected link is still valid.
*******************************************************************************/
BERR_Code BHDM_HDCP_RiLinkIntegrityCheck(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t    Register, ulOffset ;
	uint16_t    Ri ;
#if BHDM_CONFIG_DEBUG_PJ_CHECKING
	uint8_t  Pj ;
#endif
	uint8_t RetryCount = 0;

	BDBG_ENTER(BHDM_HDCP_RiLinkIntegrityCheck) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	/* Enable auto Ri checking */
	if (hHDMI->DeviceSettings.bEnableAutoRiPjChecking && hHDMI->bAutoRiPjCheckingEnabled)
        {
		rc = BHDM_HDCP_P_AutoRiLinkIntegrityCheck(hHDMI);
		goto done;
	}
#endif

#if 0
	/* TE for SimplayHD Test 7-11 appears to get confused if we attempt to read R0
	 * more than once. So don't retry if this LIC is for R0.
	 *
	 * Discovered that the new Quantum Data PS-980 has an internal problem with the
	 * 1st (short-form) access to R0 but the 2nd attempt works fine. This is a
	 * good thing to have in general, anyway.
	 */

	if (hHDMI->HDCP_RiCount == 0)
			RetryCount = BHDM_HDCP_MAX_I2C_RETRY;
#endif

	/* No Auto Ri/Pj checking is disabled or not supported */
	do
	{
		RetryCount++;

		if (hHDMI->AbortHdcpAuthRequest)
		{
			BDBG_ERR(("Tx%d: HDCP Authentication Request Aborted....", hHDMI->eCoreId)) ;
			rc = BERR_TRACE(BHDM_HDCP_AUTH_ABORTED) ;
			goto done ;
		}

		/* Read the Tx Ri value contained in the upper 16 bits of the Register */
		/* also get the Pj value */
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY + ulOffset) ;
		Ri = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY, O_RI_15_0) ;


		BKNI_Memcpy(&hHDMI->HDCP_TxRi, &Ri, 2) ;

		if (! BKNI_Memcmp(&hHDMI->HDCP_TxRi, &hHDMI->HDCP_Ri2SecsAgo, 2))   /* No Ri change from 2secs ago */
		{
			/*
			** The TxRi has not changed, so there is no need to read the RxRi.
			** just verify the TxRi value has changed in the last 6 seconds
			*/

			if (BKNI_Memcmp(&hHDMI->HDCP_TxRi, &hHDMI->HDCP_Ri4SecsAgo, 2)) /* Ri changed from 4secs ago */
			{
				BKNI_Memcpy(&hHDMI->HDCP_Ri4SecsAgo, &hHDMI->HDCP_Ri2SecsAgo, 2) ;
				rc = BERR_SUCCESS ;
				goto done ;
			}

			if (BKNI_Memcmp(&hHDMI->HDCP_TxRi, &hHDMI->HDCP_Ri6SecsAgo, 2)) /* Ri changed from 6secs ago */
			{
				BKNI_Memcpy(&hHDMI->HDCP_Ri6SecsAgo, &hHDMI->HDCP_Ri4SecsAgo, 2) ;
				rc = BERR_SUCCESS ;
				goto done ;
			}

			/*
			** Ri hasn't changed for six seconds
			** Clear the Authentication and return a LINK FAILURE
			*/
			BDBG_ERR(("Tx%d: Ri HDCP Link Integrity (Ri) values are not changing", hHDMI->eCoreId)) ;
			BDBG_ERR(("Tx%d: HDCP Link will shutdown", hHDMI->eCoreId)) ;

			/* Clear/Disable the HDCP Authentication */
			BHDM_HDCP_ClearAuthentication(hHDMI) ;

			rc = BERR_TRACE(BHDM_HDCP_LINK_RI_FAILURE) ;

			goto done ;
		}

#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
		hHDMI->HDCP_RxRi = hHDMI->HDCP_TxRi ;
		BDBG_MSG(("Tx%d: LIC: Tx/Rx Ri= %04x  %8d", hHDMI->eCoreId,
			hHDMI->HDCP_TxRi, hHDMI->HDCP_RiCount++)) ;
		return BERR_SUCCESS ;
#endif


/*
** All HDCP Rx are required to support Short or Fast Read Configuration
** where a read without an offset indicates the default I2C offset of the
** device should be used.   In the case of the HDCP Rx this is offset
** 0x08 (Ri).  Some early DVI receivers did not properly implement the
** Short Read resulting in Authentication errors
**
** Enable the following macro in bhdm_config.h to enable the HDCP Ri Short Read
** and eliminate compliance test reports of long Ri reads.
*/
#if BHDM_CONFIG_MHL_SUPPORT
		rc = BHDM_MHL_P_Hdcp_GetRxRi(hHDMI, &hHDMI->HDCP_RxRi);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to get RxRi."));
			continue;
		}
#else

		/* check if HDCP offset can be accessed if not, return not available */
		if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_RX_RI0))
		{
			BDBG_ERR(("I2c access of Rx Ri0 (%#x) is unavailable", BHDM_HDCP_RX_RI0)) ;
			rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
			goto done ;
		}


#if BHDM_CONFIG_HDCP_RI_SHORT_READ
		{
			rc = BHDM_P_BREG_I2C_ReadNoAddr(hHDMI,
				BHDM_HDCP_RX_I2C_ADDR, (uint8_t *) &hHDMI->HDCP_RxRi, 2) ;
			if (rc) {rc = BERR_TRACE(rc) ; }
		}
#else
		{
			rc = BHDM_P_BREG_I2C_Read(hHDMI,
				BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_RI0, (uint8_t *) &hHDMI->HDCP_RxRi, 2) ;
			if (rc) {rc = BERR_TRACE(rc) ; }
		}
#endif

		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Tx%d: Ri LIC: RxRi I2C read failure %x", hHDMI->eCoreId, rc)) ;
			continue ;
		}
#endif

		/* little endian number */
		BREG_LE16(hHDMI->HDCP_RxRi) ;


		/* if RIs match... */
		if  (hHDMI->HDCP_TxRi == hHDMI->HDCP_RxRi)
		{
			/* make sure the Ri values are changing...*/

			if  ((hHDMI->HDCP_TxRi == hHDMI->HDCP_Ri2SecsAgo)  /* No Ri change in 2secs */
			&& (hHDMI->HDCP_TxRi == hHDMI->HDCP_Ri4SecsAgo)  /* No Ri change in 4secs */
			&& (hHDMI->HDCP_TxRi == hHDMI->HDCP_Ri6SecsAgo)) /* No Ri change in 6secs */
			{
				/*
				** if Ri has not changed in 6 seconds  (3 Consecutive Ri reads)
				** the HDCP link is no longer valid
				*/
				rc = BERR_TRACE(BHDM_HDCP_LINK_RI_FAILURE) ;
				goto done ;
			}

			BDBG_MSG(("Tx%d: LIC: Tx/Rx Ri= %04x          R%d", hHDMI->eCoreId,
				hHDMI->HDCP_TxRi, hHDMI->HDCP_RiCount)) ;


#if BHDM_CONFIG_DEBUG_PJ_CHECKING
			/* read the Integrity register a 2nd time to ensure Pj value has updated */
			/* Pj values at  Ri updates should equal the LSB of the Ri value */
			/* Note the video pixel must be zero for this test */
			Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY + ulOffset) ;
			Pj = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY, O_PJ_7_0) ;

			BDBG_MSG(("Tx%d:                %04x", hHDMI->eCoreId, Pj)) ;
#endif
			/*  increment  the RiCount only if Ri Changed in the last 2 secs */
			if (hHDMI->HDCP_TxRi != hHDMI->HDCP_Ri2SecsAgo)
				hHDMI->HDCP_RiCount++ ;

			/*
			** RiNSecsAgo values are initialized
			** in BHDM_HDCP_AuthenticateLink
			*/
			hHDMI->HDCP_Ri6SecsAgo = hHDMI->HDCP_Ri4SecsAgo ;
			hHDMI->HDCP_Ri4SecsAgo = hHDMI->HDCP_Ri2SecsAgo ;
			hHDMI->HDCP_Ri2SecsAgo = hHDMI->HDCP_TxRi ;


			/* Return Link is Valid */
			rc = BERR_SUCCESS ;
			goto done ;
		}

		/* Ri values don't match; try reading again up to 4 times */

		/*
		** BDBG_MSG Display the different Ri Values read
		** Add a pattern "!=" to the display message so differences can
		** be easily found in any generated logs.
		**
		** Most of the time this will be a transition from one Ri
		** value to the next;
		** the next Ris read should result in equal Ri values
		*/
		BDBG_ERR(("Tx%d: LIC: TxRi= %04x <> RxRi= %04x         R%d !=", hHDMI->eCoreId,
			hHDMI->HDCP_TxRi, hHDMI->HDCP_RxRi, hHDMI->HDCP_RiCount)) ;

		rc = BERR_TRACE(BHDM_HDCP_LINK_RI_FAILURE) ;
	} while (RetryCount < BHDM_HDCP_MAX_I2C_RETRY) ;

	/* Clear/Disable the HDCP Authentication */
	BHDM_HDCP_ClearAuthentication(hHDMI) ;

done:
	BDBG_LEAVE(BHDM_HDCP_RiLinkIntegrityCheck) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_HDCP_PjLinkIntegrityCheck
Summary: Use the HDCP Ri values to verify the protected link is still valid.
*******************************************************************************/
BERR_Code BHDM_HDCP_PjLinkIntegrityCheck(
   const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;

	uint32_t    Register, ulOffset ;
	uint8_t 	Pj ;
	uint8_t RetryCount = 0;

	BDBG_ENTER(BHDM_HDCP_PjLinkIntegrityCheck) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	if (hHDMI->DeviceSettings.bEnableAutoRiPjChecking && hHDMI->bAutoRiPjCheckingEnabled)
	{
		rc = BHDM_HDCP_P_AutoPjLinkIntegrityCheck(hHDMI);
		goto done;
	}
#endif

	do
	{
		RetryCount++;

		/* Read the Tx Ri value contained in the upper 16 bits of the Register */
		/* also get the Pj value */
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY + ulOffset) ;
		Pj = BCHP_GET_FIELD_DATA(Register, HDMI_CP_INTEGRITY, O_PJ_7_0) ;

		/*
		** There is no guarantee the connected Rx supports the HDCP I2C Short
		** Read format, therefore use a standard I2C read for the Receiver
		*/

		/* check if HDCP offset can be accessed if not, just continue like other error conditions
		*/
		if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_RX_RI0))
		{
			continue;
		}

#if BHDM_CONFIG_MHL_SUPPORT
		rc = BHDM_MHL_P_Hdcp_GetRxPj(hHDMI, &hHDMI->HDCP_RxPj);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to get RxPj."));
			continue;
		}
#else
		rc = BHDM_P_BREG_I2C_Read(hHDMI,
			BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_PJ, (uint8_t *) &hHDMI->HDCP_RxPj, 1) ;

		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Tx%d: Pj LIC: Rx Pj I2C read failure %d", hHDMI->eCoreId, rc));
			continue  ;
		}
#endif
		/* Pjs match... */
		if  (hHDMI->HDCP_RxPj == Pj)
		{
			BDBG_MSG(("Tx%d: Pj LIC: Tx/Rx Pj: %02x", hHDMI->eCoreId, Pj)) ;
			hHDMI->HDCP_PjMismatchCount = 0 ;
			hHDMI->HdcpOptions.numPjFailures = 0;

			/* Return Link is Valid */
			rc = BERR_SUCCESS ;
			goto done ;
		}


		/* Pj values don't match; try reading again up to MAX_I2C_RETRY times */

		/*
		** BDBG_MSG Display the different Pj Values read
		** Add a pattern "!=" to the display message so differences can
		** be easily found in any generated logs.
		**
		** Most of the time this will be a transition from one Pj
		** value to the next;
		** the next Pjs read should result in equal Pj values
		*/
		BDBG_WRN(("Tx%d: Pj LIC: TxPj= %02x <> RxPj= %02x %d !=", hHDMI->eCoreId,
			Pj, hHDMI->HDCP_RxPj, RetryCount)) ;
	} while (RetryCount < BHDM_HDCP_MAX_I2C_RETRY) ;

	/* could be a pixel transmission error; ignore if errs < MAX MisMatch */
	if (++hHDMI->HDCP_PjMismatchCount < BHDM_HDCP_MAX_PJ_MISMATCH)
	{
		BDBG_WRN(("Tx%d: Pixel Transmission Error?? Pj Mis-match Count %d", hHDMI->eCoreId,
			hHDMI->HDCP_PjMismatchCount)) ;
	}
	else   /* otherwise fail the link */
	{
		rc = BERR_TRACE(BHDM_HDCP_LINK_PJ_FAILURE) ;
		hHDMI->HdcpOptions.numPjFailures++;
		BDBG_ERR(("Tx%d: %d consecutive Pj link failures", hHDMI->eCoreId,
			hHDMI->HdcpOptions.numPjFailures));

		/* Clear/Disable the HDCP Authentication */
		BHDM_HDCP_ClearAuthentication(hHDMI) ;
	}


done:
	BDBG_LEAVE(BHDM_HDCP_PjLinkIntegrityCheck) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_HDCP_GetRxCaps
Summary: Retrieve the Rx Capabilities.
*******************************************************************************/
BERR_Code BHDM_HDCP_GetRxCaps(
   const BHDM_Handle hHDMI,         /* [in] HDMI handle */
   uint8_t *pRxCapsReg     /* [out] HDCP Rx Capability */
)
{
	BERR_Code   rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_HDCP_GetRxCaps) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* if get RxCaps is being used to determine if the KSV FIFO is ready */
	/* return an HDCP Abort if a HP has occurred while waiting for KsvFifoRdy bit */
	if ((hHDMI->AbortHdcpAuthRequest) && (hHDMI->HDCP_AuthenticatedLink))
	{
		BDBG_ERR(("Tx%d: HDCP Authentication Request Aborted....", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_HDCP_AUTH_ABORTED) ;
		hHDMI->AbortHdcpAuthRequest = 0 ;
		goto done ;
	}

	/* check if HDCP offset can be accessed if not, return not available */
	if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_RX_BCAPS))
	{
		BDBG_ERR(("I2c access of Rx Bcaps (%#x) is unavailable", BHDM_HDCP_RX_BCAPS)) ;
		rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
		goto done ;
	}


#if BHDM_CONFIG_MHL_SUPPORT
	rc = BHDM_MHL_P_Hdcp_GetRxBCaps(hHDMI, pRxCapsReg);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed to get RxBCaps."));
		goto done;
	}
#else
#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	/* We can't access I2C if control has been handed over to HW */
	if (hHDMI->bAutoRiPjCheckingEnabled)
	{
		*pRxCapsReg = hHDMI->RxBCaps;
	}
	else
	{
		BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
			BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BCAPS, pRxCapsReg, 1)) ;
	}
#else
	BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
		BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BCAPS, pRxCapsReg, 1)) ;
#endif
#endif /* BHDM_CONFIG_MHL_SUPPORT */


#if 0
	/* debug to force Pj values */
	/* if Rx does not support Pj auth errors will occur */
	*pRxCapsReg	|= BHDM_HDCP_RxCaps_eHDCP_1_1_Features ;
#endif

#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
	*pRxCapsReg |=BHDM_HDCP_RxCaps_eHdcpRepeater ;
#endif

	/* update settings only if RxCaps has changed */
	if (hHDMI->RxBCaps == *pRxCapsReg)
		goto done ;

	BDBG_MSG(("Tx%d: %s Bcaps (%#x): %#x", hHDMI->eCoreId,
		(*pRxCapsReg & BHDM_HDCP_RxCaps_eHdcpRepeater) ? "Repeater" : "Receiver",
		BHDM_HDCP_RX_BCAPS, *pRxCapsReg)) ;

#if BHDM_CONFIG_DEBUG_HDCP_BCAPS
	BDBG_MSG(("-----------------------------------")) ;
	BDBG_MSG(("Tx%d: HDMI Rsvd(7):  %d", hHDMI->eCoreId,
		*pRxCapsReg & BHDM_HDCP_RxCaps_eHdmiCapable ? 1 : 0)) ;
	BDBG_MSG(("Tx%d: Repeater(6):   %d", hHDMI->eCoreId,
		*pRxCapsReg & BHDM_HDCP_RxCaps_eHdcpRepeater ? 1 : 0)) ;
	if (*pRxCapsReg & BHDM_HDCP_RxCaps_eHdcpRepeater)
		BDBG_MSG(("Tx%d:    KSV FIFO Rdy (5): %d", hHDMI->eCoreId,
			*pRxCapsReg & BHDM_HDCP_RxCaps_eKsvFifoReady ? 1 : 0)) ;
	BDBG_MSG(("Tx%d: Fast I2C(4):   %d", hHDMI->eCoreId,
		*pRxCapsReg & BHDM_HDCP_RxCaps_eI2c400KhzSupport ? 1 : 0)) ;
	BDBG_MSG(("Tx%d: HDCP 1.1 with Optional Features(1):   %d", hHDMI->eCoreId,
		*pRxCapsReg & BHDM_HDCP_RxCaps_eHDCP_1_1_Features ? 1 : 0)) ;
	BDBG_MSG(("Tx%d: FastReauth(0): %d", hHDMI->eCoreId,
		*pRxCapsReg & BHDM_HDCP_RxCaps_eFastReauth ? 1 : 0)) ;
#endif

	/* If attached Rx supports HDCP 1.1 with optional features, option Pj checking desired and
	the number of Pj link failures <= the max failures allow, enable HDCP 1.1 with optional features */
	if ((*pRxCapsReg & BHDM_HDCP_RxCaps_eHDCP_1_1_Features)
	&& (hHDMI->HdcpOptions.PjChecking)
	&& (hHDMI->HdcpOptions.numPjFailures <= BHDM_HDCP_MAX_PJ_LINK_FAILURES_BEFORE_DISABLE_HDCP_1_1))
	{
		/* tell the Rx the our HDMI core supports HDCP 1.1 with optional features as well */
		hHDMI->HdcpVersion = BHDM_HDCP_Version_e1_1_Optional_Features ;

		/* turn on the callbacks for Pj interrupts */
		BHDM_CHECK_RC(rc, BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(HDCP_PJ)] )) ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
		BHDM_CHECK_RC(rc, BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(HDCP_PJ_MISMATCH)] )) ;
#endif

	}
	else
	{
		hHDMI->HdcpVersion = BHDM_HDCP_Version_e1_1 ;
		/* HDCP 1.1 only; disable the Pj interrupts */
		BHDM_CHECK_RC(rc, BINT_DisableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(HDCP_PJ)] )) ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
		BHDM_CHECK_RC(rc, BINT_DisableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(HDCP_PJ_MISMATCH)] )) ;
#endif
	}


	hHDMI->RxBCaps = *pRxCapsReg ;

done:

	BDBG_LEAVE(BHDM_HDCP_GetRxCaps) ;
	return rc ;
} /* end BHDM_HDCP_GetRxCaps */




/******************************************************************************
BERR_Code BHDM_HDCP_GetRxStatus
Summary: Check the status of the attached receiver.
*******************************************************************************/
BERR_Code BHDM_HDCP_GetRxStatus(
   const BHDM_Handle hHDMI,                /* [in] HDMI handle */
   uint16_t *pRxStatusReg          /* [out] HDCP Rx Status */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	uint16_t Status ;

	BDBG_ENTER(BHDM_HDCP_GetRxStatus) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
    /* We can't access I2C if control has been handed over to HW */
    if (hHDMI->bAutoRiPjCheckingEnabled)
    {
            *pRxStatusReg = hHDMI->RxStatus;
            goto done;
    }
#endif

	/* check if HDCP offset can be accessed if not, return not available */
	if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_RX_BSTATUS))
	{
		BDBG_ERR(("I2c access of Rx Bstatus (%#x) is unavailable", BHDM_HDCP_RX_BSTATUS)) ;
		rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
		goto done ;
	}


#if BHDM_CONFIG_MHL_SUPPORT
	rc = BHDM_MHL_P_Hdcp_GetRxStatus(hHDMI, &Status);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed to get RxStatus."));
		goto done;
	}
#else
	BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
		BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BSTATUS, (uint8_t *) &Status, 2)) ;
#endif


	BREG_LE16(Status) ;

	BDBG_MSG(("Tx%d: Rx BStatus (0x41): %#x", hHDMI->eCoreId, Status)) ;
	BDBG_MSG(("--------------------------------")) ;
	BDBG_MSG(("Tx%d: HDMI Mode:            %d", hHDMI->eCoreId,
		Status & BHDM_HDCP_RxStatus_eHdmiMode ? 1 : 0)) ;
	BDBG_MSG(("Tx%d: MAX Cascade Exceeded: %d", hHDMI->eCoreId,
		Status & BHDM_HDCP_RxStatus_eMaxRepeatersExceeded ? 1 : 0)) ;
	BDBG_MSG(("Tx%d: Max Devs Exceeded:    %d", hHDMI->eCoreId,
		Status & BHDM_HDCP_RxStatus_eMaxDevicesExceeded ? 1 : 0)) ;

	*pRxStatusReg = Status ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	hHDMI->RxStatus =  *pRxStatusReg ;
#endif


done:
	BDBG_LEAVE(BHDM_HDCP_GetRxStatus) ;
	return rc ;
} /* end BHDM_HDCP_GetRxStatus */



/******************************************************************************
BERR_Code BHDM_HDCP_CheckForRepeater
Summary: Check if the attached receiver is an HDCP Receiver.
*******************************************************************************/
BERR_Code BHDM_HDCP_CheckForRepeater(
   const BHDM_Handle hHDMI,                /* [in] HDMI handle */
   uint8_t *uiRepeater          /* [out] HDCP Rx Status */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	uint8_t RxCaps ;

	BDBG_ENTER(BHDM_HDCP_CheckForRepeater) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_CHECK_RC(rc, BHDM_HDCP_GetRxCaps(hHDMI, &RxCaps)) ;
	if (RxCaps & BHDM_HDCP_RxCaps_eHdcpRepeater)
	{
		BDBG_MSG(("Tx%d: HDCP Repeater Found", hHDMI->eCoreId)) ;
		*uiRepeater = 1 ;
	}
	else
	{
		BDBG_MSG(("Tx%d: HDCP Standard Receiver Found", hHDMI->eCoreId)) ;
		*uiRepeater = 0 ;
	}
#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
		*uiRepeater = 1 ;
		BDBG_WRN(("Tx%d: HDCP Repeater Found", hHDMI->eCoreId)) ;
#endif
done:
	BDBG_LEAVE(BHDM_HDCP_CheckForRepeater) ;
	return rc ;
} /* end BHDM_HDCP_CheckForRepeater */


/******************************************************************************
BERR_Code BHDM_HDCP_GetRepeaterDepth
Summary: Get the depth (number of levels) of HDCP Repeaters
*******************************************************************************/
BERR_Code BHDM_HDCP_GetRepeaterDepth(
   const BHDM_Handle hHDMI,      /* [in] HDMI handle */
   uint8_t *pRepeaterDepth /* [out] pointer to hold Levels of HDCP Repeaters */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	uint16_t    BStatus = 0 ;

	BDBG_ENTER(BHDM_HDCP_GetRepeaterDepth) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_CHECK_RC(rc, BHDM_HDCP_GetRxStatus(hHDMI, &BStatus)) ;

	/* shift the BStatus Register to get the Repeater Depth; HDCP Spec Table 2-4 */
	BStatus = (uint16_t) (BStatus >> 8) ;

	BStatus = BStatus & 0x07 ;
	*pRepeaterDepth = (uint8_t) BStatus ;
	BDBG_MSG(("Tx%d: Repeater Depth: %X", hHDMI->eCoreId, BStatus)) ;

done:
	BDBG_LEAVE(BHDM_HDCP_GetRepeaterDepth) ;
	return rc ;
} /* end BHDM_HDCP_GetRepeaterDepth */




/******************************************************************************
BERR_Code BHDM_HDCP_GetRepeaterDeviceCount
Summary: Get the number of devices attached to the HDCP Repeater.
*******************************************************************************/
BERR_Code BHDM_HDCP_GetRepeaterDeviceCount(
   const BHDM_Handle hHDMI,   /* [in] HDMI handle */
   uint8_t *pNumDevices /* [out] pointer to # of devices attached to the HDCP
                           Repeater */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	uint16_t    BStatus = 0 ;

	BDBG_ENTER(BHDM_HDCP_GetRepeaterDeviceCount) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_CHECK_RC(rc, BHDM_HDCP_GetRxStatus(hHDMI, &BStatus)) ;

	/* AND the Device Count bits in the BStatus Register to get the Device Count */
	/* See HDCP Spec */
	BStatus = (uint8_t) (BStatus & 0x7F) ;
	BDBG_MSG(("Tx%d: Repeater Device Count: %X", hHDMI->eCoreId, BStatus)) ;

	/* copy the Repeater Depth Value */
	*pNumDevices = (uint8_t) BStatus ;

done :
	BDBG_LEAVE(BHDM_HDCP_GetRepeaterDeviceCount) ;
	return rc ;
} /* end BHDM_HDCP_GetRepeaterDeviceCount */




/******************************************************************************
BERR_Code BHDM_HDCP_ReadRepeaterKsvFIFO
Summary: Read the KSV list from the attached repeater.
*******************************************************************************/
BERR_Code BHDM_HDCP_ReadRxRepeaterKsvFIFO(
   const BHDM_Handle hHDMI,      /* [in] HDMI handle */
   uint8_t *pRxKsvList,    /* [out] pointer to memory to hold KSV List */
   uint16_t uiRxDeviceCount,  /* [in ] number of Ksvs in the Rx KSV List */
   const uint8_t *pRevokedKsvList,  /* [in] pointer to Revoked KSV List */
   const uint16_t uiRevokedKsvCount /* [in] number of KSVs in Revoked Ksv List */
)
{
	BERR_Code   rc = BERR_SUCCESS;

	uint16_t uiBufSize ;
	bool bRevoked ;
	uint16_t iNumKsvBytes ;
	uint16_t i ;
	uint8_t NumOfSetBits ;


#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
	uint8_t TestKsvFifo[] =
	{
		0x2e, 0x17, 0x6a, 0x79, 0x35,
		0x0f, 0xe2, 0x71, 0x8e, 0x47,
		0xa6, 0x97, 0x53, 0xe8, 0x74
	} ;
#endif

	BDBG_ENTER(BHDM_HDCP_ReadRxRepeaterKsvFIFO) ;
	BDBG_ASSERT( hHDMI );

	uiBufSize = uiRxDeviceCount * BHDM_HDCP_KSV_LENGTH ;

	/* Read the KSV FIFO */
	/* NOTE: the KSV_FIFO offset in the receiver does not auto-increment when read...
	** all values come from the same offset
	*/

	/* check if HDCP offset can be accessed if not, return not available */
	if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_REPEATER_KSV_FIFO))
	{
		BDBG_ERR(("I2c access of Repeater KSV FIFO (%#x) is unavailable", BHDM_HDCP_REPEATER_KSV_FIFO)) ;
		rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
		goto done ;
	}


#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
	pRxKsvList = &TestKsvFifo[0] ;
#else
#if BHDM_CONFIG_MHL_SUPPORT
	BSTD_UNUSED(uiBufSize);

	rc = BHDM_MHL_P_Hdcp_GetRepeaterKsvFifo(hHDMI, pRxKsvList, uiRxDeviceCount);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed to get RepeaterKsvFifo."));
		goto done;
	}
#else
	BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
		BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_REPEATER_KSV_FIFO, pRxKsvList, uiBufSize )) ;
#endif
#endif /* BHDM_CONFIG_MHL_SUPPORT */

	BDBG_MSG(("Downstream Devices KSVs: %d", uiRxDeviceCount)) ;
	for (i = 0 ; i < uiRxDeviceCount; i++)
	{
		NumOfSetBits = BHDM_HDCP_P_NumberOfSetBits(&pRxKsvList[i*5], BHDM_HDCP_KSV_LENGTH) ;
		if (NumOfSetBits == 20)
		{
			BDBG_MSG(("Valid KSV[%d]: = %02X %02X %02X %02X %02X \n",
				i, pRxKsvList[i*5+4], pRxKsvList[i*5+3], pRxKsvList[i*5+2],
				pRxKsvList[i*5+1], pRxKsvList[i*5])) ;
		}
		else
		{
			BDBG_ERR(("Invalid KSV[%d] %02X %02X %02X %02X %02X - NumberOfSetBits=%d \n", i,
				pRxKsvList[i*5+4], pRxKsvList[i*5+3], pRxKsvList[i*5+2],
				pRxKsvList[i*5+1], pRxKsvList[i*5], NumOfSetBits)) ;

			/* report an error to the user indicating failed KSVs */
			/* this condition is likely to happen on a QD 882EA device for HDCP Compliance Testing */
			/* the DUT/repeater declaration on the QD 882EA should indicate support for 16 downstream KSVs */
			BDBG_ERR(("=================================")) ;
			BDBG_ERR(("Please confirm the attached device supports the number downstream devices reported")) ;
			BDBG_ERR(("Devices Reported: %d", uiRxDeviceCount)) ;
			BDBG_ERR(("=================================")) ;

			rc = BERR_TRACE(BHDM_HDCP_RX_BKSV_ERROR) ;
			goto done;
		}
	}


	/* keep a copy of the attached ksv list */
	/* free any previous list that may have been allocated */
	if (hHDMI->HDCP_RepeaterKsvList)
		BKNI_Free(hHDMI->HDCP_RepeaterKsvList) ;


	hHDMI->HDCP_RepeaterDeviceCount = uiRxDeviceCount ;
	iNumKsvBytes = (uint16_t) (hHDMI->HDCP_RepeaterDeviceCount * BHDM_HDCP_KSV_LENGTH) ;

	/* there could be no devices attached to the repeater; finished */
	if (!iNumKsvBytes)
	{
		hHDMI->HDCP_RepeaterKsvList  = (uint8_t *) 0  ;
		goto done ;
	}

	hHDMI->HDCP_RepeaterKsvList  = (uint8_t *) BKNI_Malloc(sizeof(uint8_t) * iNumKsvBytes) ;
	BKNI_Memcpy(hHDMI->HDCP_RepeaterKsvList, pRxKsvList, iNumKsvBytes) ;


	/* check if the Repeater's Ksv List has any revoked Ksvs */
	if (uiRevokedKsvCount)
	{
		BHDM_HDCP_P_CheckRevokedKsvList(
			(uint8_t *) pRevokedKsvList, uiRevokedKsvCount,
			pRxKsvList, uiRxDeviceCount, &bRevoked) ;

		if (bRevoked)
		{
			rc = BERR_TRACE(BHDM_HDCP_RX_BKSV_REVOKED) ;
			goto done ;
		}
	}

done:
	BDBG_LEAVE(BHDM_HDCP_ReadRxRepeaterKsvFIFO) ;
	return rc ;
} /* end BHDM_HDCP_ReadRepeaterKsvFIFO */



/******************************************************************************
BERR_Code BHDM_HDCP_WriteTxKsvFIFO
Summary: Write the KSV list retrieved from the attached repeater to the
Transmitter core
*******************************************************************************/
BERR_Code BHDM_HDCP_WriteTxKsvFIFO(
	const BHDM_Handle hHDMI,
	uint8_t *KsvList,
	uint16_t uiRxDeviceCount)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;

	uint8_t
		i, j ;

	uint8_t
		timeoutMs,
		Ksv[5] ;

	uint32_t
		Register, ulOffset,
		BksvRegisterValue,
		RepeaterBusy ;

#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
	uint8_t SHA_1[] =
	{
		0x86, 0xd5, 0xcb, 0x0f,
		0xef, 0x07, 0xc1, 0xef,
		0x1d, 0x0a, 0xd7, 0xcc,
		0xda, 0x6d, 0x18, 0xb1,
		0x5e, 0xff, 0xb3, 0x1f
	} ;

#endif

	BDBG_ENTER(BHDM_HDCP_WriteTxKsvFIFO) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	for (i = 0 ; i < uiRxDeviceCount; i++)
	{

		/* wait until the Tx Repeater core is ready for a Ksv */
		timeoutMs = 10 ;
		do
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_CP_STATUS + ulOffset) ;
			RepeaterBusy = BCHP_GET_FIELD_DATA(Register, HDMI_CP_STATUS, O_REPEATER_IS_BUSY) ;
			if (!RepeaterBusy)
				break ;

			BKNI_Sleep(1) ;
			BDBG_WRN(("Tx%d: Tx Repeater Core Busy!", hHDMI->eCoreId)) ;
		} while ( timeoutMs-- ) ;


		/* copy the next Ksv */
		for (j = 0 ; j < BHDM_HDCP_KSV_LENGTH ; j++)
			Ksv[j] = KsvList[BHDM_HDCP_KSV_LENGTH * i  + j] ;

		/* write the 4 LSBs RxBksv to the transmitter... */
		BksvRegisterValue =
			  Ksv[0]
			| Ksv[1] <<  8
			| Ksv[2] << 16
			| Ksv[3] << 24 ;

		BREG_Write32(hRegister, BCHP_HDMI_KSV_FIFO_0 + ulOffset, BksvRegisterValue) ;
#if 0
		BDBG_MSG(("Tx%d:  Key[%d] %X", hHDMI->eCoreId, i, BksvRegisterValue)) ;
#endif

		/* write the 1 MSB RxBksv to the transmitter... */
		BksvRegisterValue = Ksv[4] ;
		BREG_Write32(hRegister, BCHP_HDMI_KSV_FIFO_1 + ulOffset, BksvRegisterValue) ;
#if 0
		BDBG_MSG(("Tx%d:  Key[%d] MSB %X", hHDMI->eCoreId, i, BksvRegisterValue)) ;
#endif
	}

	BDBG_LEAVE(BHDM_HDCP_WriteTxKsvFIFO) ;
	return rc ;
}

/******************************************************************************
BERR_Code BHDM_HDCP_IsLinkAuthenticated
Summary: Check if the current link is authenticated.
*******************************************************************************/
BERR_Code BHDM_HDCP_IsLinkAuthenticated(const BHDM_Handle hHDMI, bool *bAuthenticated)
{
#if BHDM_CONFIG_HAS_HDCP22
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;
	bool uiAuthenticatedOK = false;
	bool uiHdcp2Authenticated = false;

	BDBG_ENTER(BHDM_HDCP_IsLinkAuthenticated);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	hRegister = hHDMI->hRegister;
	ulOffset = hHDMI->ulOffset;

	if (hHDMI->HdcpVersion == BHDM_HDCP_Version_e2_2)
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_STATUS + ulOffset);
		uiAuthenticatedOK = BCHP_GET_FIELD_DATA(Register, HDMI_HDCP2TX_STATUS, AUTHENTICATED_OK);

		Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset);
		uiHdcp2Authenticated = (bool) BCHP_GET_FIELD_DATA(Register, HDMI_HDCP2TX_AUTH_CTL, HDCP2_AUTHENTICATED) ;

		*bAuthenticated = (uiAuthenticatedOK && uiHdcp2Authenticated);
	}
	else {
		*bAuthenticated = hHDMI->HDCP_AuthenticatedLink == 1 ;
	}

	BDBG_MSG(("HdcpVersion: %d IsLinkAuthenticated Check %d",
		hHDMI->HdcpVersion, *bAuthenticated)) ;

#else

	BDBG_ENTER(BHDM_HDCP_IsLinkAuthenticated);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	*bAuthenticated = hHDMI->HDCP_AuthenticatedLink == 1 ;
#endif

	BDBG_LEAVE(BHDM_HDCP_IsLinkAuthenticated);
	return BERR_SUCCESS;
}


#define DUMP_CP_STATUS 	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_STATUS + ulOffset) ; \
	BDBG_MSG(("Tx%d: CP_STATUS: %04X", hHDMI->eCoreId, Register)) ;

/******************************************************************************
BERR_Code BHDM_HDCP_InitializeRepeaterAuthentication
Summary: Initialize the HDCP Core to authenticate the attached repeater
*******************************************************************************/
BERR_Code BHDM_HDCP_InitializeRepeaterAuthentication(const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;

	uint32_t Register, ulOffset ;
	uint16_t BStatus ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* check if HDCP offset can be accessed if not, return not available */
	if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_RX_BSTATUS))
	{
		BDBG_ERR(("I2c access of Rx Bstatus (%#x) is unavailable", BHDM_HDCP_RX_BSTATUS)) ;
		rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
		goto done ;
	}


	/* load the initial values to the core */
	/* BCaps was already written by BHDM_ReadRxBksv */
#if BHDM_CONFIG_MHL_SUPPORT
	rc = BHDM_MHL_P_Hdcp_GetRxStatus(hHDMI, &BStatus);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed to get RxStatus."));
		goto done;
	}
#else
	BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
		BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BSTATUS, (unsigned char *) &BStatus, 2)) ;
#endif

	BREG_LE16(BStatus) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_BKSV1 + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_BKSV1, I_BSTATUS_15_0) ;

	Register |=  BCHP_FIELD_DATA(HDMI_BKSV1, I_BSTATUS_15_0, BStatus) ;
	BREG_Write32(hRegister, BCHP_HDMI_BKSV1 + ulOffset, Register) ;

#if 0
	BDBG_MSG(("Tx%d: Bksv1: %X", hHDMI->eCoreId, Register)) ;
#endif

	DUMP_CP_STATUS	;

	/* initilize Repeater logic */
	Register = BCHP_MASK(HDMI_HDCP_CTL, I_INIT_REPEATER) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

	DUMP_CP_STATUS	;
done:
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_HDCP_RepeaterAuthentication
Summary: Initialize the HDCP Core to authenticate the attached repeater
*******************************************************************************/
BERR_Code BHDM_HDCP_RepeaterAuthenticateLink(const BHDM_Handle hHDMI, uint8_t *RepeaterAuthenticated)
{
	BERR_Code rc ;
	BREG_Handle hRegister ;
	uint8_t
		i,
		timeoutMs,
		RepeaterBusy,
		Ksv[5] ;


	uint32_t Register, ulOffset;
	uint32_t BksvRegisterValue ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* wait for core to finish V calculation */
	timeoutMs = 10 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_STATUS + ulOffset) ;
		RepeaterBusy = BCHP_GET_FIELD_DATA(Register, HDMI_CP_STATUS, O_REPEATER_IS_BUSY) ;
		if (!RepeaterBusy)
			break ;

		BKNI_Sleep(1) ;
		BDBG_MSG(("Tx%d: Waiting for TxCore Repeater V Calculation...", hHDMI->eCoreId)) ;
	} while ( timeoutMs-- ) ;


	if (RepeaterBusy)
	{
		BDBG_ERR(("Tx%d: TxCore failed Repeater V Calculation", hHDMI->eCoreId));
		*RepeaterAuthenticated = 0 ;
		goto done ;
	}


	/*
	-- now read/load the results 160 byte SHA-1 Hash from the receiver...
	-- use the Ksv to store each 32 bit part of the V value
	*/

	/* check if HDCP offset can be accessed if not, return not available */
	if (!BHDM_HDCP_P_RegisterAccessAllowed(hHDMI, BHDM_HDCP_REPEATER_SHA1_V_H0))
	{
		BDBG_ERR(("I2c access of Repeater SHA1_V_H0 (%#x) is unavailable", BHDM_HDCP_REPEATER_SHA1_V_H0)) ;
		rc = BERR_TRACE(BERR_NOT_AVAILABLE) ;
		goto done ;
	}


	for (i = 0; i < 5; i++)
	{
#if BHDM_CONFIG_REPEATER_SIMULATION_TEST
		BKNI_Memcpy(&BksvRegisterValue, (uint8_t *) &SHA_1[i*4],  4) ;
#else

#if BHDM_CONFIG_MHL_SUPPORT
		rc = BHDM_MHL_P_Hdcp_GetRepeaterSha(hHDMI, Ksv, i);
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to get Ksv."));
			goto done;
		}
#else
		BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
			BHDM_HDCP_RX_I2C_ADDR,	BHDM_HDCP_REPEATER_SHA1_V_H0 + (i * 4), Ksv, 4 )) ;
#endif
		BksvRegisterValue =
		  Ksv[0]
		| (Ksv[1] <<  8)
		| (Ksv[2] << 16)
		| (Ksv[3] << 24) ;
#endif
		BDBG_MSG(("Tx%d: SHA-1 H%d Key = %X", hHDMI->eCoreId, i, BksvRegisterValue)) ;

		BREG_Write32(hRegister, BCHP_HDMI_V + ulOffset, BksvRegisterValue) ;
	}


	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_STATUS + ulOffset) ;
	if (Register & BCHP_MASK(HDMI_CP_STATUS, O_V_MATCH))
	{
		BDBG_MSG(("Tx%d: Repeater AUTHENTICATED", hHDMI->eCoreId)) ;
		*RepeaterAuthenticated = 1 ;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
#if BHDM_CONFIG_MHL_SUPPORT
			if (!hHDMI->bMhlMode)
#endif
			{
                if (hHDMI->DeviceSettings.bEnableAutoRiPjChecking) {
                        BHDM_HDCP_P_EnableAutoRiPjChecking(hHDMI, hHDMI->HdcpOptions.PjChecking) ;
                }
			}
#endif
	}
	else
	{
		BDBG_ERR(("Tx%d: Repeater FAILED Authentication", hHDMI->eCoreId)) ;
		*RepeaterAuthenticated = 0 ;
	}
	DUMP_CP_STATUS	;

done:
	return BERR_SUCCESS ;
}


/******************************************************************************
BERR_Code BHDM_HDCP_GetRxKsv
Summary: Return KSV information obtained from the authentication
*******************************************************************************/
BERR_Code BHDM_HDCP_GetRxKsv(const BHDM_Handle hHDMI,
	uint8_t *AttachedDeviceKsv
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t RxDeviceAttached ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	/* make sure the Rx has been read;  */
	BHDM_CHECK_RC(rc, BHDM_HDCP_ReadRxBksv(hHDMI, (uint8_t *) 0, 0)) ;

	/* copy the KSV of the attached device */
	BKNI_Memcpy(AttachedDeviceKsv, hHDMI->HDCP_RxKsv, BHDM_HDCP_KSV_LENGTH) ;

done:
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_HDCP_GetRepeaterKsvFifo
Summary: Return KSV information obtained from the authentication
*******************************************************************************/
BERR_Code BHDM_HDCP_GetRepeaterKsvFifo(const BHDM_Handle hHDMI,
	uint8_t *DownstreamKsvsBuffer, uint16_t DownstreamKsvsBufferSize
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint16_t iNumKsvBytes ;
	uint8_t RxDeviceAttached ;
	bool bAuthenticated = false;;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	/* make sure link is authenticated */
	rc = BHDM_HDCP_IsLinkAuthenticated(hHDMI, &bAuthenticated) ;
	if (!bAuthenticated)
	{
		BDBG_WRN(("Tx%d: Repeater KSV FIFO available only after authentication",
			hHDMI->eCoreId)) ;
		if (rc) {rc = BERR_TRACE(rc) ;}
		goto done ;
	}

	/* copy the Downstream KSV list */
	if (hHDMI->HDCP_RepeaterKsvList)
	{
		iNumKsvBytes =
			(uint16_t) hHDMI->HDCP_RepeaterDeviceCount * BHDM_HDCP_KSV_LENGTH ;

		if (DownstreamKsvsBufferSize < iNumKsvBytes)
		{
			BDBG_ERR(("Tx%d: Allocated buffer of %d bytes too small for KSV list of %d bytes",
				hHDMI->eCoreId, DownstreamKsvsBufferSize, iNumKsvBytes)) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
		}

		BKNI_Memcpy(DownstreamKsvsBuffer, hHDMI->HDCP_RepeaterKsvList, iNumKsvBytes) ;
	}

done:
	return rc ;
}


#if !defined(BHDM_CONFIG_DISABLE_HDCP_AUTH_REPEATER_DEVCOUNT0)
/******************************************************************************
BERR_Code BHDM_HDCP_ForceVCalculation
Summary: Force the V Calculation for attached repeaters with device count zero
*******************************************************************************/
BERR_Code BHDM_HDCP_ForceVCalculation(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset) ;
	Register |= BCHP_MASK(HDMI_HDCP_CTL, I_FORCE_VCALC) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

	Register &= ~BCHP_MASK(HDMI_HDCP_CTL, I_FORCE_VCALC) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

	return BERR_SUCCESS ;
}
#endif


/******************************************************************************
BERR_Code BHDM_HDCP_GetOptions
Summary: Get current options used for HDCP Authtentication etc.
*******************************************************************************/
BERR_Code BHDM_HDCP_GetOptions(
	const BHDM_Handle hHDMI, BHDM_HDCP_OPTIONS *HdcpOptions)
{
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memcpy(HdcpOptions, &hHDMI->HdcpOptions, sizeof(BHDM_HDCP_OPTIONS)) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
BERR_Code BHDM_HDCP_SetOptions
Summary: Set options used for HDCP Authtentication etc.
*******************************************************************************/
BERR_Code BHDM_HDCP_SetOptions(
	const BHDM_Handle hHDMI, BHDM_HDCP_OPTIONS *HdcpOptions)
{
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memcpy(&hHDMI->HdcpOptions, HdcpOptions, sizeof(BHDM_HDCP_OPTIONS)) ;
	return BERR_SUCCESS ;
}



BERR_Code BHDM_HDCP_P_DEBUG_PjForceVideo(const BHDM_Handle hHDMI, uint8_t value)
{
#if BDBG_DEBUG_BUILD
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister,  BCHP_HDMI_CP_TST + ulOffset) ;
	Register &=
		~(BCHP_MASK(HDMI_CP_TST, I_TST_FORCE_VIDEO_ALL_ONES)
		| BCHP_MASK(HDMI_CP_TST, I_TST_FORCE_VIDEO_ALL_ZEROS)) ;

	if (value)
		Register |= BCHP_FIELD_DATA(HDMI_CP_TST,  I_TST_FORCE_VIDEO_ALL_ONES, 1) ;
	else
		Register |= BCHP_FIELD_DATA(HDMI_CP_TST,  I_TST_FORCE_VIDEO_ALL_ZEROS, 1) ;

	BREG_Write32(hRegister, BCHP_HDMI_CP_TST + ulOffset, Register) ;
#else
    BSTD_UNUSED(hHDMI);
    BSTD_UNUSED(value);
#endif
	return BERR_SUCCESS ;
}


BERR_Code BHDM_HDCP_P_DEBUG_PjCleanVideo(const BHDM_Handle hHDMI, uint8_t value)
{
#if BDBG_DEBUG_BUILD
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BSTD_UNUSED(value) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister,  BCHP_HDMI_CP_TST + ulOffset) ;
	Register &=
		~(BCHP_MASK(HDMI_CP_TST, I_TST_FORCE_VIDEO_ALL_ONES)
		| BCHP_MASK(HDMI_CP_TST, I_TST_FORCE_VIDEO_ALL_ZEROS)) ;

	BREG_Write32(hRegister, BCHP_HDMI_CP_TST + ulOffset, Register) ;
#else
    BSTD_UNUSED(hHDMI);
    BSTD_UNUSED(value);
#endif
	return BERR_SUCCESS ;
}


#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
/******************************************************************************
BERR_Code BHDM_HDCP_P_ConfigureAutoRi
Summary: Configure HDMI HDCP RI register for Auto (hardware) Ri Checking feature.
*******************************************************************************/
BERR_Code BHDM_HDCP_P_ConfigureAutoRi(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_3 + ulOffset) ;
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_DATA));

#if BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_ADDR_OFFSET, 0xb)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_DATA, 0x0)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_ADDR_OFFSET, 0xa)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_DATA, 0xd3);
#else
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_ADDR_OFFSET, 0xb)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_0_WR_DATA, 0x0)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_ADDR_OFFSET, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_3, RI_IIC_REG_1_WR_DATA, 0x8);
#endif
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_3 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_4 + ulOffset) ;
#if BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_ADDR_OFFSET)
                        | BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_DATA));

	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_ADDR_OFFSET, 0x9)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_DATA, 0x81)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_ADDR_OFFSET, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_DATA, 0x8);
#else
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_EN));

	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_ADDR_OFFSET, 0xb)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_2_WR_DATA, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_4, RI_IIC_REG_3_WR_EN, 0x0);
#endif
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_4 + ulOffset, Register) ;

#if BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT
	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_8 + ulOffset) ;
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_4_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_4_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_4_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_5_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_5_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_5_WR_DATA));

	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_4_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_4_WR_ADDR_OFFSET, 0xb)
                        | BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_8, RI_IIC_REG_4_WR_DATA, 0x1);
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_8 + ulOffset, Register) ;
#endif

	return BERR_SUCCESS;
}


/******************************************************************************
BERR_Code BHDM_HDCP_P_ConfigureAutoPj
Summary: Configure HDMI HDCP PJ register for Auto (hardware) Pj Checking feature.
*******************************************************************************/
BERR_Code BHDM_HDCP_P_ConfigureAutoPj(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_5 + ulOffset) ;
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_DATA));

#if BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_ADDR_OFFSET, 0xb)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_DATA, 0x0)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_ADDR_OFFSET, 0xa)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_DATA, 0xd3);
#else
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_ADDR_OFFSET, 0xb)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_0_WR_DATA, 0x0)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_ADDR_OFFSET, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_5, PJ_IIC_REG_1_WR_DATA, 0xa);
#endif
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_5 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_6 + ulOffset) ;
#if BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_ADDR_OFFSET)
                        | BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_DATA));

	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_ADDR_OFFSET, 0x9)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_DATA, 0x81)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_ADDR_OFFSET, 0x1)
                        | BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_DATA, 0xa);
#else
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_EN));

	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_ADDR_OFFSET, 0xb)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_2_WR_DATA, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_6, PJ_IIC_REG_3_WR_EN, 0x0);
#endif
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_6 + ulOffset, Register) ;


#if BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT
	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_7 + ulOffset) ;
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_4_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_4_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_4_WR_DATA)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_5_WR_EN)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_5_WR_ADDR_OFFSET)
			| BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_5_WR_DATA));

	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_4_WR_EN, 0x1)
			| BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_4_WR_ADDR_OFFSET, 0xb)
                        | BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_7, PJ_IIC_REG_4_WR_DATA, 0x1);
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_7 + ulOffset, Register) ;
#endif

	return BERR_SUCCESS;
}


/******************************************************************************
BERR_Code BHDM_HDCP_P_EnableAutoRiPjChecking
Summary: Enable Auto (hardware) Ri/Pj Checking feature
*******************************************************************************/
static BERR_Code BHDM_HDCP_P_EnableAutoRiPjChecking (const BHDM_Handle hHDMI, uint8_t uiPjChecking)
{
	BERR_Code rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset;
	uint8_t uiMode;

#if !(BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT)
	uint32_t dataTransferFormat=3, cnt1=1, cnt2=2;
#endif

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* Set up I2C HW first */
	BHDM_CHECK_RC(rc, BREG_I2C_Read(hHDMI->hI2cRegHandle,
		BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BSTATUS, (uint8_t *) &hHDMI->RxStatus, 2)) ;
	BREG_LE16(hHDMI->RxStatus) ;

	BHDM_CHECK_RC(rc, BREG_I2C_Read(hHDMI->hI2cRegHandle,
		BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BCAPS, &hHDMI->RxBCaps, 1)) ;

#if !(BHDM_CONFIG_HDCP_ADVANCED_HW_AUTO_RI_PJ_SUPPORT)
	rc = BREG_I2C_SetupHdmiHwAccess(hHDMI->hI2cRegHandle, dataTransferFormat, cnt1, cnt2);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Tx%d: Error setting up I2C HW for auto Ri/Pj link integrity check",
			hHDMI->eCoreId));
		rc = BERR_TRACE(rc) ;
		goto done;
	}
#endif

	if (uiPjChecking)
		uiMode = 2;
	else
		uiMode = 1;

	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_2 + ulOffset) ;
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_2, IIC_REG_RD_ADDR_OFFSET));
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_2, IIC_REG_RD_ADDR_OFFSET, 0xc);
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_2 + ulOffset, Register) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_1 + ulOffset) ;
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_1, CLEAR_RI_MISMATCH, 1);
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_1, CLEAR_PJ_MISMATCH, 1);
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_1 + ulOffset, Register) ;
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_1, CLEAR_RI_MISMATCH));
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_1, CLEAR_PJ_MISMATCH));
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_1 + ulOffset, Register) ;

	hHDMI->HDCP_AutoRiMismatch_A = false;
	hHDMI->HDCP_AutoRiMismatch_B = false;

	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_1, CHECK_MODE));
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_1, CHECK_MODE, uiMode);
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_1 + ulOffset, Register) ;

	hHDMI->bAutoRiPjCheckingEnabled = true;

done:
	return rc;
}


/******************************************************************************
BERR_Code BHDM_HDCP_P_DisableAutoRiPjChecking
Summary: Disable Auto (hardware) Ri/Pj Checking feature
*******************************************************************************/
static BERR_Code BHDM_HDCP_P_DisableAutoRiPjChecking (const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset;
	uint8_t uiMode=0;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_1 + ulOffset) ;
	Register &= ~(BCHP_MASK(HDMI_CP_INTEGRITY_CHK_CFG_1, CHECK_MODE));
	Register |= BCHP_FIELD_DATA(HDMI_CP_INTEGRITY_CHK_CFG_1, CHECK_MODE, uiMode);
	BREG_Write32(hRegister, BCHP_HDMI_CP_INTEGRITY_CHK_CFG_1 + ulOffset, Register) ;

	hHDMI->bAutoRiPjCheckingEnabled = false;
	return BERR_SUCCESS;
}

#endif


void BHDM_HDCP_P_ResetSettings_isr(BHDM_Handle hHDMI)
{
	BDBG_ENTER(BHDM_HDCP_P_ResetSettings_isr);

	hHDMI->AbortHdcpAuthRequest = 1 ;
	hHDMI->HDCP_AuthenticatedLink = 0;
	hHDMI->HdcpOptions.numPjFailures = 0;
	hHDMI->HdcpVersion = BHDM_HDCP_Version_eUnused;

	/* enable HDCP Pj Checking by default */
	hHDMI->HdcpOptions.PjChecking = true ;

	BDBG_LEAVE(BHDM_HDCP_P_ResetSettings_isr);
}


BERR_Code BHDM_HDCP_GetHdcpVersion(const BHDM_Handle hHDMI, BHDM_HDCP_Version *eVersion)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t RxCaps ;
	uint8_t uiHdcp2Version;
	uint8_t RxDeviceAttached;
	bool authenticated ;


#if BDBG_DEBUG_BUILD
	uint8_t version = 0 ;
#endif

	BDBG_ENTER(BHDM_HDCP_GetHdcpVersion);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);


	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));

	if (!RxDeviceAttached)
	{
		BDBG_ERR(("Tx%d: No DVI/HDMI Device attached to HDMI port", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	/* return cached version whenever the link is AUTHENTICATED */
	rc = BHDM_HDCP_IsLinkAuthenticated(hHDMI, &authenticated) ;
	if (rc != BERR_SUCCESS || authenticated)
	{
		goto done ;
	}


	/**************************/
	/* check for HDCP 2.x Rx device */
	/**************************/

	/* access to register space is allowed here */
	rc = BHDM_P_BREG_I2C_Read(hHDMI,
		BHDM_HDCP_RX_I2C_ADDR,	BHDM_HDCP_RX_HDCP2VERSION, &uiHdcp2Version, 1) ;

	if (rc == BERR_SUCCESS)
	{
		/* If 3rd bit set, Rx support HDCP 2.2 */
		if (uiHdcp2Version & 0x04)
		{
#if BHDM_CONFIG_HAS_HDCP22
			hHDMI->HdcpVersion = BHDM_HDCP_Version_e2_2 ;
			goto done ;
#else
			/* just report the Rx supports HDCP 2.2; even though this platform cannot */
			BDBG_MSG(("Attached Rx supports HDCP 2.x")) ;
			/* almost all devices support 1.x, but check BCaps */
#endif
		}
	}
	else
	{
#if BHDM_CONFIG_HAS_HDCP22
		/* HDMI 2.0 Platforms expect HDCP 2.x support */
		/* Warn if attached Rx does not support HDCP 2.x */
		BDBG_ERR(("Tx%d: (i2c err code= %x)  Unable to read HDCP2VERSION - Rx does not support HDCP 2.x",
			hHDMI->eCoreId, rc)) ;
#endif
	}


	/**************************/
	/* check for HDCP 1.x Rx device */
	/**************************/

	/* check for HDCP 1.x Support */
#if BHDM_CONFIG_MHL_SUPPORT
	rc = BHDM_MHL_P_Hdcp_GetRxBCaps(hHDMI, &RxCaps);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Failed to get RxBCaps."));
		rc = BERR_TRACE(rc) ;
		goto done;
	}
#elif BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	/* We can't access I2C if control has been handed over to HW */
	{
		if (hHDMI->bAutoRiPjCheckingEnabled)
		{
			RxCaps = hHDMI->RxBCaps;
		}
		else
		{
			BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
				BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BCAPS, &RxCaps, 1)) ;
		}
	}
#else
	{
		BHDM_CHECK_RC(rc, BHDM_P_BREG_I2C_Read(hHDMI,
			BHDM_HDCP_RX_I2C_ADDR, BHDM_HDCP_RX_BCAPS, &RxCaps, 1)) ;
	}
#endif

	if (RxCaps & BHDM_HDCP_RxCaps_eHDCP_1_1_Features) {
		hHDMI->HdcpVersion = BHDM_HDCP_Version_e1_1_Optional_Features ;
	}
	else {
		hHDMI->HdcpVersion = BHDM_HDCP_Version_e1_1 ;
	}

done:
#if BDBG_DEBUG_BUILD
	switch (hHDMI->HdcpVersion)
	{
	case BHDM_HDCP_Version_e1_1 :
	case BHDM_HDCP_Version_e1_1_Optional_Features :
		version =  1 ;
		break ;

	case BHDM_HDCP_Version_e2_2 :
		version = 2 ;
		break ;

	default :
		version = 0 ;
	}

	BDBG_MSG(("Attached Rx supports up to HDCP %d.x", version)) ;
#endif

	*eVersion = hHDMI->HdcpVersion;

	BDBG_LEAVE(BHDM_HDCP_GetHdcpVersion);
	return rc;
}


#if BHDM_CONFIG_HAS_HDCP22
/* function to read/copy the HDCP RxStatus registers */
void BHDM_HDCP_P_ReadHdcp22RxStatus_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, RegAddr, ulOffset ;
	uint32_t autoI2cChxOffset ;

	BDBG_ENTER(BHDM_HDCP_P_ReadHdcp22RxStatus_isr) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	autoI2cChxOffset =
		BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus ;

	RegAddr = BCHP_HDMI_TX_AUTO_I2C_CH0_RD_0 + ulOffset + autoI2cChxOffset ;

	Register = BREG_Read32(hRegister, RegAddr) ;
	if (Register  == (uint32_t) 0)
	{
		BDBG_MSG(("No HDCP Updates Detected")) ;
		goto done ;
	}

	hHDMI->Hdcp22RxStatusBuffer[0] = (uint8_t) Register & 0x00000FF ;
	hHDMI->Hdcp22RxStatusBuffer[1] = (uint8_t) ((Register & 0x0000FF00) >> 8) ;

	BDBG_MSG(("HDCP Rx Status %x",
		hHDMI->Hdcp22RxStatusBuffer[1], hHDMI->Hdcp22RxStatusBuffer[0])) ;

done:
	BDBG_LEAVE(BHDM_HDCP_P_ReadHdcp22RxStatus_isr) ;
}


BERR_Code BHDM_HDCP_UpdateHdcp2xAuthenticationStatus(const BHDM_Handle hHDMI, const bool authenticated)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDM_HDCP_UpdateHdcp2xAuthenticationStatus);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	hRegister = hHDMI->hRegister;
	ulOffset = hHDMI->ulOffset;

	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset);
		Register &= ~BCHP_MASK(HDMI_HDCP2TX_AUTH_CTL, HDCP2_AUTHENTICATED);
		Register |=	BCHP_FIELD_DATA(HDMI_HDCP2TX_AUTH_CTL, HDCP2_AUTHENTICATED, authenticated);
	BREG_Write32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset, Register) ;

	BDBG_LEAVE(BHDM_HDCP_UpdateHdcp2xAuthenticationStatus);
	return BERR_SUCCESS;
}


BERR_Code BHDM_HDCP_EnableHdcp2xEncryption(const BHDM_Handle hHDMI, const bool enable)
{
	uint32_t Register, ulOffset;
	bool uiAuthenticatedOk = false;
	bool uiHdcp2Authenticated = false;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDM_HDCP_EnableHdcp2xEncryption);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	hRegister = hHDMI->hRegister;
	ulOffset = hHDMI->ulOffset;

	BDBG_MSG(("bReAuthRequestPending = %d", hHDMI->bReAuthRequestPending));
	if (hHDMI->bReAuthRequestPending && enable)
	{
		BDBG_MSG(("Pending REAUTH_REQ. Skip enabling HDCP 2.x encryption"));
		goto done;
	}

	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_STATUS + ulOffset);
	uiAuthenticatedOk = (bool) BCHP_GET_FIELD_DATA(Register, HDMI_HDCP2TX_STATUS, AUTHENTICATED_OK) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset);
	uiHdcp2Authenticated = (bool) BCHP_GET_FIELD_DATA(Register, HDMI_HDCP2TX_AUTH_CTL, HDCP2_AUTHENTICATED) ;

	if  ((!uiAuthenticatedOk || !uiHdcp2Authenticated) && (enable))
	{
		BDBG_MSG(("HDCP2.x Authentication not yet completed. Cannot enable HDCP2.x encryption"));
		goto done;
	}

	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset);
		Register &= ~BCHP_MASK(HDMI_HDCP2TX_AUTH_CTL, ENABLE_HDCP2_ENCRYPTION);
		Register |=	BCHP_FIELD_DATA(HDMI_HDCP2TX_AUTH_CTL, ENABLE_HDCP2_ENCRYPTION, enable);
	BREG_Write32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset, Register) ;

done:
	BDBG_LEAVE(BHDM_HDCP_EnableHdcp2xEncryption);
	return BERR_SUCCESS;
}


BERR_Code BHDM_HDCP_SetHdcp2xTxCaps(const BHDM_Handle hHDMI, const uint8_t version, const uint16_t txCapsMask)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDM_HDCP_SetHdcp2xTxCaps);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	hRegister = hHDMI->hRegister;
	ulOffset = hHDMI->ulOffset;

	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_TXCAPS  + ulOffset);
		Register &= ~(BCHP_MASK(HDMI_TX_AUTO_I2C_HDCP2TX_TXCAPS , VERSION)
					| BCHP_MASK(HDMI_TX_AUTO_I2C_HDCP2TX_TXCAPS , TRANSMITTER_CAPABILITY_MASK));

		Register |=	BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_TXCAPS , VERSION, version)
					| BCHP_FIELD_DATA(HDMI_TX_AUTO_I2C_HDCP2TX_TXCAPS , TRANSMITTER_CAPABILITY_MASK, txCapsMask);
	BREG_Write32(hRegister, BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_TXCAPS  + ulOffset, Register) ;

	BDBG_LEAVE(BHDM_HDCP_SetHdcp2xTxCaps);
	return BERR_SUCCESS;
}


BERR_Code BHDM_HDCP_KickStartHdcp2xCipher(const BHDM_Handle hHDMI)
{
	uint32_t Register, ulOffset;
	BREG_Handle hRegister;

	BDBG_ENTER(BHDM_HDCP_KickStartHdcp2xCipher);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI);

	hRegister = hHDMI->hRegister;
	ulOffset = hHDMI->ulOffset;

	/* Set HDCP_VERSION_SELECT */
	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_CFG0  + ulOffset);
		Register &= ~(BCHP_MASK(HDMI_HDCP2TX_CFG0, HDCP_VERSION_SELECT));
		Register |= BCHP_FIELD_DATA(HDMI_HDCP2TX_CFG0, HDCP_VERSION_SELECT, 0x1);
	BREG_Write32(hRegister, BCHP_HDMI_HDCP2TX_CFG0	+ ulOffset, Register) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset);
		Register &= ~BCHP_MASK(HDMI_HDCP2TX_AUTH_CTL, AKE_INIT);
		Register |=	BCHP_FIELD_DATA(HDMI_HDCP2TX_AUTH_CTL, AKE_INIT, 0x1);
	BREG_Write32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset, Register) ;

	/* Clear ReAuthRequestPending flag */
	hHDMI->bReAuthRequestPending = false;

	BDBG_LEAVE(BHDM_HDCP_KickStartHdcp2xCipher);
	return BERR_SUCCESS;
}


#endif
