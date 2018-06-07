/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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

/* Following macros are also defined in nexus_hdmi_input_hdcp_keyloader.c.
 * Both places must have the same definitions. */

#define BCRYPT_ALG_CUST_KEY            (0x01)
#define BCRYPT_ALG_GLOBAL_KEY          (0x02)
#define BCRYPT_ALG_OTP_KEY_DIRECT      (0x81)
#define BCRYPT_ALG_OTP_KEY_ASKM        (0x82)

#define BCRYPT_MASK_KEY2_CHIP_FAMILY   (0x0)
#define BCRYPT_MASK_KEY2_FIXED         (0x1)

#define BCRYPT_STB_OWNER_ID_OTP        (0x0)
#define BCRYPT_STB_OWNER_ID_ONE        (0x1)

#define BCRYPT_GLOBAL_OWNER_ID_MSP0    (0x0)
#define BCRYPT_GLOBAL_OWNER_ID_MSP1    (0x1)
#define BCRYPT_GLOBAL_OWNER_ID_ONE     (0x2)


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
        if (EncryptedHdcpKeys[i].Alg == BCRYPT_ALG_GLOBAL_KEY) {
            BDBG_ERR(( "***********************************************************************" ));
            BDBG_ERR(( "***                                                                 ***" ));
            BDBG_ERR(( "***   ERROR:                                                        ***" ));
            BDBG_ERR(( "***   The encrypted HDCP key is not compatible with this chip.      ***" ));
            BDBG_ERR(( "***   If the command-line BCrypt has been used (vs. GUI Bcrypt),    ***" ));
            BDBG_ERR(( "***   make sure to set Version to 'V1' in the tool config file.     ***" ));
            BDBG_ERR(( "***                                                                 ***" ));
            BDBG_ERR(( "***********************************************************************" ));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
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
    switch( hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].Alg )
    {
        case BCRYPT_ALG_GLOBAL_KEY:
            {
            uint32_t tmp;
            uint8_t maskKey2;
            uint8_t stbOwnerId;
            uint8_t globalOwnerId;

            hdcpConf.root.type = BHSM_KeyLadderRootType_eGlobalKey;
            hdcpConf.root.askm.caVendorId = (hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].CaDataHi)>>16;

            /* TCaDataLo is filled with following four bytes:
             *   Mask Key2 select (MSB) | STB Owner ID Select | Global Owner ID Select | Global Key Index (LSB)
             */
            tmp = hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].TCaDataLo;
            maskKey2 = tmp >> 24;
            switch( maskKey2 )
            {
                case BCRYPT_MASK_KEY2_CHIP_FAMILY:
                    hdcpConf.root.askm.caVendorIdScope = BHSM_KeyLadderCaVendorIdScope_eChipFamily; break;
                case BCRYPT_MASK_KEY2_FIXED:
                    hdcpConf.root.askm.caVendorIdScope = BHSM_KeyLadderCaVendorIdScope_eFixed;      break;
                default:
                    BDBG_ERR(( "Invalid maskKey2: 0x%x", maskKey2 ));
                    return BERR_TRACE( BERR_INVALID_PARAMETER );
            }

            stbOwnerId = (tmp >> 16)&0xff;
            switch( stbOwnerId )
            {
                case BCRYPT_STB_OWNER_ID_OTP:
                    hdcpConf.root.askm.stbOwnerSelect = BHSM_KeyLadderStbOwnerIdSelect_eOtp; break;
                case BCRYPT_STB_OWNER_ID_ONE:
                    hdcpConf.root.askm.stbOwnerSelect = BHSM_KeyLadderStbOwnerIdSelect_eOne; break;
                default:
                    BDBG_ERR(( "Invalid stbOwnerId: 0x%x", stbOwnerId ));
                    return BERR_TRACE( BERR_INVALID_PARAMETER );
            }

            globalOwnerId = (tmp >> 8)&0xff;
            switch( globalOwnerId )
            {
                case BCRYPT_GLOBAL_OWNER_ID_MSP0:
                    hdcpConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0; break;
                case BCRYPT_GLOBAL_OWNER_ID_MSP1:
                    hdcpConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1; break;
                case BCRYPT_GLOBAL_OWNER_ID_ONE:
                    hdcpConf.root.globalKey.owner = BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne;  break;
                default:
                    BDBG_ERR(( "Invalid globalOwnerId: 0x%x", globalOwnerId ));
                    return BERR_TRACE( BERR_INVALID_PARAMETER );
            }

            hdcpConf.root.globalKey.index = tmp & 0xff;
            }
            break;
        case BCRYPT_ALG_OTP_KEY_DIRECT:
            otpKeyIndex = hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].CaDataLo;
            BREG_LE32( otpKeyIndex );

            hdcpConf.root.type = BHSM_KeyLadderRootType_eOtpDirect;
            hdcpConf.root.otpKeyIndex = otpKeyIndex;
            break;
        case BCRYPT_ALG_OTP_KEY_ASKM:
            {
            uint32_t tmp;
            uint8_t maskKey2;
            uint8_t stbOwnerId;

            hdcpConf.root.type = BHSM_KeyLadderRootType_eOtpAskm;
            hdcpConf.root.otpKeyIndex = (hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].CaDataLo);
            hdcpConf.root.askm.caVendorId = (hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].CaDataHi)>>16;

            /* TCaDataLo is filled with following four bytes:
             *   Mask Key2 select (MSB) | STB Owner ID Select | Reserved | Reserved (LSB)
             */
            tmp = hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].TCaDataLo;
            maskKey2 = tmp >> 24;
            switch( maskKey2 )
            {
                case BCRYPT_MASK_KEY2_CHIP_FAMILY:
                    hdcpConf.root.askm.caVendorIdScope = BHSM_KeyLadderCaVendorIdScope_eChipFamily; break;
                case BCRYPT_MASK_KEY2_FIXED:
                    hdcpConf.root.askm.caVendorIdScope = BHSM_KeyLadderCaVendorIdScope_eFixed;      break;
                default:
                    BDBG_ERR(( "Invalid maskKey2: 0x%x", maskKey2 ));
                    return BERR_TRACE( BERR_INVALID_PARAMETER );
            }

            stbOwnerId = (tmp >> 16)&0xff;
            switch( stbOwnerId )
            {
                case BCRYPT_STB_OWNER_ID_OTP:
                    hdcpConf.root.askm.stbOwnerSelect = BHSM_KeyLadderStbOwnerIdSelect_eOtp; break;
                case BCRYPT_STB_OWNER_ID_ONE:
                    hdcpConf.root.askm.stbOwnerSelect = BHSM_KeyLadderStbOwnerIdSelect_eOne; break;
                default:
                    BDBG_ERR(( "Invalid stbOwnerId: 0x%x", stbOwnerId ));
                    return BERR_TRACE( BERR_INVALID_PARAMETER );
            }

            }
            break;
        case BCRYPT_ALG_CUST_KEY:
            BDBG_ERR(( "Invalid Alg: 0x%x", hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].Alg ));
            BDBG_ERR(( "***********************************************************************" ));
            BDBG_ERR(( "***                                                                 ***" ));
            BDBG_ERR(( "***   ERROR:                                                        ***" ));
            BDBG_ERR(( "***   The encrypted HDCP key is not compatible with this chip.      ***" ));
            BDBG_ERR(( "***   Make sure to use the command-line BCrypt (vs. GUI BCrypt),    ***" ));
            BDBG_ERR(( "***   and set Version to 'V2' in the tool config file.              ***" ));
            BDBG_ERR(( "***                                                                 ***" ));
            BDBG_ERR(( "***********************************************************************" ));
            break;
        default:
            BDBG_ERR(( "Invalid Alg: 0x%x", hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure[0].Alg ));
			return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    hdcpConf.algorithm = BHSM_CryptographicAlgorithm_e3DesAba;

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
