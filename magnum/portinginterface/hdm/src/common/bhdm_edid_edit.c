/******************************************************************************
 * Copyright (C) 2019 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc.h"

BDBG_MODULE(BHDM_EDID_EDIT) ;


/******************************************************************************
uint8_t BHDM_EDID_P_CalcEdidBlockCheckSum
Summary:Verify the checksum on an EDID block
*******************************************************************************/
static uint8_t BHDM_EDID_P_CalcEdidBlockCheckSum(uint8_t *pEDID)
{
	uint8_t i ;
	uint8_t checksum = 0 ;


	for (i = 0 ; i < BHDM_EDID_BLOCKSIZE ; i++)
		checksum = checksum + (uint8_t) pEDID[i]  ;
	return (uint8_t) (256 - (checksum - (unsigned int) pEDID[BHDM_EDID_CHECKSUM])) ;
}


/******************************************************************************
Summary:
Remove CEA Data Block from EDID

Description:
This function will remove the specified CEA DB from an EDID extension block

Input:
	ucVsdbId - IEEE OUI of block to remove.
	           Can be part of a regular or extended data block

Output:
	ucBuffer - pointer to unsigned char buffer containing V3 Timing Extension

*******************************************************************************/
void BHDM_EDID_Remove_CeaVSDB(uint8_t *ucBuffer, const uint8_t *ucVsdbId)
{
	uint8_t
		DataOffset,
		NewDataOffset,
		DataBlockIndex,
		DataBlockTag,
		DataBlockLength,
		ExtendedTagCode ;
	uint8_t *pucNewEdidExt ;
	uint8_t uiNewEdidExtIndex ;
	uint8_t *ucpIEEE_RegId ;


	DataOffset = ucBuffer[BHDM_EDID_EXT_DATA_OFFSET] ;

	/* check if data blocks exist before any Detailed Descriptors */
	if ((DataOffset == 0) || (DataOffset == 4)) /* no data blocks exist. */
		return ;

	/* current offset (index) where Detailed Descriptors begin */
	/* offset will be updated if block(s) are removed */
	NewDataOffset = DataOffset ;

	/* Set the index to the start of Data Blocks */
	DataBlockIndex = BHDM_EDID_EXT_DATA_BLOCK_COLLECTION ;

	/* copy bytes before the Data Blocks to the new extension block */
	pucNewEdidExt = BKNI_Malloc(sizeof(uint8_t) * BHDM_EDID_BLOCKSIZE) ;
	if (pucNewEdidExt == NULL)
	{
		BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		return ;
	}

	BKNI_Memcpy(pucNewEdidExt, ucBuffer, DataBlockIndex) ;
	uiNewEdidExtIndex = DataBlockIndex ;

	BDBG_MSG(("Search for IEEE OUI: %02X %02X %02X",
		ucVsdbId[2], ucVsdbId[1], ucVsdbId[0])) ;

	while (DataBlockIndex < DataOffset)
	{
		/* get the Data Block Tag, Length */
		DataBlockTag    = ucBuffer[DataBlockIndex] >>    5 ;
		DataBlockLength = ucBuffer[DataBlockIndex]  & 0x1F ;

		switch (DataBlockTag)
		{
		case BHDM_EDID_CeaDataBlockTag_eVSDB :       /* VSDB */
			ucpIEEE_RegId = &ucBuffer[DataBlockIndex+1] ;
			if (BKNI_Memcmp(ucpIEEE_RegId, ucVsdbId, 3) == 0)
			{
				BDBG_MSG(("   Remove <%s> [%02X %02X %02X] @ Index %d",
					BAVC_HDMI_EDID_CeaTagToStr(DataBlockTag),
					ucVsdbId[2], ucVsdbId[1], ucVsdbId[0],
					DataBlockIndex)) ;
				NewDataOffset = NewDataOffset - (DataBlockLength + 1) ; /* include header */
				goto next_block ;
			}
			break ;

		case BHDM_EDID_CeaDataBlockTag_eExtendedDB:  /* VSDB in Extended DB */
			ExtendedTagCode = ucBuffer[DataBlockIndex+1];

			switch (ExtendedTagCode)
			{
			case BHDM_EDID_CeaExtendedDBTag_eVendorSpecificVideoDB:
				ucpIEEE_RegId = &ucBuffer[DataBlockIndex+2] ;

				if (BKNI_Memcmp(ucpIEEE_RegId, ucVsdbId, 3) == 0)
				{
					BDBG_MSG(("   Skip CEA Extended Tag <%s> [%02X %02X %02X] %d bytes @ Index %d",
						BAVC_HDMI_EDID_CeaExtendedTagToStr(ExtendedTagCode),
						ucVsdbId[2], ucVsdbId[1], ucVsdbId[0], DataBlockLength, DataBlockIndex)) ;
					NewDataOffset = NewDataOffset - (DataBlockLength + 1) ; /* include header */
					goto next_block ;
				}
				break ;

			case BHDM_EDID_CeaExtendedDBTag_eVideoCapabilityDB :
			case BHDM_EDID_CeaExtendedDBTag_eVESADisplayDeviceDB :
			case BHDM_EDID_CeaExtendedDBTag_eVESAVideoTimingBlockExtension :
			case BHDM_EDID_CeaExtendedDBTag_eColorimetryDB :
			case BHDM_EDID_CeaExtendedDBTag_eHDRStaticMetaDB :
			case BHDM_EDID_CeaExtendedDBTag_eVideoFormatPreferenceDB :
			case BHDM_EDID_CeaExtendedDBTag_eYCBCR420VideoDB :
			case BHDM_EDID_CeaExtendedDBTag_eYCBCR420CapabilityMapDB :
			case BHDM_EDID_CeaExtendedDBTag_eVendorSpecificAudioDB :
			case BHDM_EDID_CeaExtendedDBTag_eInfoFrameDB :
				BDBG_MSG(("   Copy CEA Extended Tag <%s> %d bytes @ TempBufIndex: %d",
					BAVC_HDMI_EDID_CeaExtendedTagToStr(ExtendedTagCode),
					DataBlockLength, uiNewEdidExtIndex)) ;
				break ;

			default :
				BDBG_WRN(("   Skip CEA Extended Tag <%d>; not supported",
					ExtendedTagCode)) ;
				NewDataOffset = NewDataOffset - (DataBlockLength + 1) ; /* include header */
				goto next_block ;
				break ;
			}

			break;

		case BHDM_EDID_CeaDataBlockTag_eVideoDB :   /* Video DB */
		case BHDM_EDID_CeaDataBlockTag_eAudioDB :   /* Audio DB */
		case BHDM_EDID_CeaDataBlockTag_eSpeakerDB : /* Speaker Allocation DB */
			break ;

		case BHDM_EDID_CeaDataBlockTag_eReserved0 :
		case BHDM_EDID_CeaDataBlockTag_eReserved5 :
		case BHDM_EDID_CeaDataBlockTag_eReserved6 :
		default : /* note any Unknown/Unsupported Tags */
			BDBG_WRN(("DB <%s> at index %d is not supported",
				   BAVC_HDMI_EDID_CeaTagToStr(DataBlockTag), DataBlockIndex)) ;
			goto next_block ;

			break ;
		}

/* copy_block */
		BDBG_MSG(("   Copy DB <%s> %d bytes @ TempBufIndex: %d",
			BAVC_HDMI_EDID_CeaTagToStr(DataBlockTag), DataBlockLength, uiNewEdidExtIndex)) ;
		BKNI_Memcpy(pucNewEdidExt + uiNewEdidExtIndex, ucBuffer + DataBlockIndex, DataBlockLength + 1) ;
		uiNewEdidExtIndex  += DataBlockLength + 1 ;

next_block:
		DataBlockIndex += DataBlockLength + 1;
	} /* while DataBlockIndex < DataOffset */

	/* copy the remaining bytes to the new EDID */
	BKNI_Memcpy(pucNewEdidExt + uiNewEdidExtIndex, ucBuffer + DataBlockIndex,
		BHDM_EDID_BLOCKSIZE - DataBlockIndex - 1 ) ;
	uiNewEdidExtIndex += (BHDM_EDID_BLOCKSIZE - DataBlockIndex) - 1 ;

	/* memset(0) the remaining bytes */
	BKNI_Memset(pucNewEdidExt + uiNewEdidExtIndex, 0, BHDM_EDID_BLOCKSIZE - uiNewEdidExtIndex - 1) ;

	/* update the new data offset where detailed timings now begin */
	pucNewEdidExt[BHDM_EDID_EXT_DATA_OFFSET] = NewDataOffset ;

	/* calculate checksum for new block */
	pucNewEdidExt[BHDM_EDID_CHECKSUM] = BHDM_EDID_P_CalcEdidBlockCheckSum(pucNewEdidExt) ;

#if 0
	/* display EDID for debug */
	BHDM_EDID_DEBUG_P_PrintEdidBlock(pucNewEdidExt) ;
#endif

	/* update EDID Timing extension with new extension */
	BKNI_Memcpy(ucBuffer, pucNewEdidExt, BHDM_EDID_BLOCKSIZE) ;

	BKNI_Free(pucNewEdidExt) ;
}
