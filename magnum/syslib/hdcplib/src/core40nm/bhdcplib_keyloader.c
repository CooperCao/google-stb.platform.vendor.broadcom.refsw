/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bkni.h"
#include "bhdcplib_keyloader.h"
#include "breg_endian.h"

#include "bhsm.h"
#include "bhsm_keyladder.h"
#if (BHSM_API_VERSION==2)
#include "bhsm_hdcp1x.h"
#endif


#define BHDCPLib_KEY_OFFSET 0x80

BDBG_MODULE(BHDCPLIB_KEYLOADER) ;



BERR_Code BHDCPlib_FastLoadEncryptedHdcpKeys(
	BHDCPlib_Handle hHDCPlib)
{
#if (BHSM_API_VERSION==1)
	BERR_Code errCode = BERR_SUCCESS;
	uint8_t i;
	BHSM_EncryptedHdcpKeyStruct * EncryptedHdcpKeys ;

	BDBG_ASSERT(hHDCPlib) ;
	BDBG_MSG(("HDCP Key Loader URSR 14.4")) ;

	/* Get HDCP Encrypted Keys */
	EncryptedHdcpKeys = (BHSM_EncryptedHdcpKeyStruct *)
		&hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure ;

	for (i =0; i< BAVC_HDMI_HDCP_N_PRIVATE_KEYS; i++)
	{
		/* skip keys that are not specified to be used by the RxBksv */
		if (!(hHDCPlib->stHdcpConfiguration.RxInfo.RxBksv[i / 8] & (1 << (i % 8))))
			continue ;

		/* byte swap 32bits values for BigEndian platforms */
		BREG_LE32(EncryptedHdcpKeys[i].CaDataLo);
		BREG_LE32(EncryptedHdcpKeys[i].CaDataHi);
		BREG_LE32(EncryptedHdcpKeys[i].TCaDataLo);
		BREG_LE32(EncryptedHdcpKeys[i].TCaDataHi);
		BREG_LE32(EncryptedHdcpKeys[i].HdcpKeyLo);
		BREG_LE32(EncryptedHdcpKeys[i].HdcpKeyHi);

		hHDCPlib->stDependencies.lockHsm();
		errCode = BHSM_FastLoadEncryptedHdcpKey(
			hHDCPlib->stDependencies.hHsm, i + BHDCPLib_KEY_OFFSET, &(EncryptedHdcpKeys[i]) ) ;
		hHDCPlib->stDependencies.unlockHsm();
		if (errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BHSM_FastLoadEncryptedHdcpKey errCode: %x", errCode )) ;
			BERR_TRACE(errCode) ;
			break ;
		}
	}

	return( errCode );
#else  /* secv2 */
  #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(5,0)
	BERR_Code rc = BERR_SUCCESS;
    BHSM_Hdcp1xRouteKey hdcpConf;
    unsigned i = 0;
    uint32_t otpKeyIndex;

    BKNI_Memset( &hdcpConf, 0, sizeof(hdcpConf) );

    otpKeyIndex = hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].CaDataLo;
    BREG_LE32( otpKeyIndex );

    hdcpConf.algorithm = BHSM_CryptographicAlgorithm_e3DesAba;
    hdcpConf.root.type = BHSM_KeyLadderRootType_eOtpDirect;
    hdcpConf.root.otpKeyIndex = otpKeyIndex;

	for( i =0; i< BAVC_HDMI_HDCP_N_PRIVATE_KEYS; i++ )
    {
        hdcpConf.hdcpKeyIndex  = i;
        hdcpConf.key.high = hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[i].HdcpKeyHi;
        hdcpConf.key.low  = hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[i].HdcpKeyLo;

		hHDCPlib->stDependencies.lockHsm();
        rc = BHSM_Hdcp1x_RouteKey( hHDCPlib->stDependencies.hHsm, &hdcpConf );
		hHDCPlib->stDependencies.unlockHsm();

        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    }

    return BERR_SUCCESS;
  #else
    BSTD_UNUSED(hHDCPlib);
    BERR_TRACE( BERR_NOT_SUPPORTED );
    return BERR_SUCCESS;
  #endif
#endif
}
