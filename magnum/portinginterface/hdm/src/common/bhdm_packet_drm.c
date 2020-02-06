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
#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc_hdmi.h"

BDBG_MODULE(BHDM_PACKET_DRM) ;

#define BHDM_DRM_PACKET_NUM_DISPLAY_PRIMARIES 3

/******************************************************************************
Summary:
Set/Enable the DRM Info Frame packet to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_SetDRMInfoFramePacket(
   const BHDM_Handle hHDMI,		  /* [in] HDMI handle */
   BAVC_HDMI_DRMInfoFrame *pstDRMInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BHDM_Packet PhysicalHdmiRamPacketId ;

	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* BHDM_SetDRMInfoFramePacket also called from BHDM_EnableDisplay
	    using the DRMInfoFrame stored in the HDMI handle
	*/

	if (BKNI_Memcmp(&hHDMI->DeviceSettings.stDRMInfoFrame, pstDRMInfoFrame,
				sizeof(*pstDRMInfoFrame)) == 0)
	{
		/* No change in software setting, so no need to change HW */
		BDBG_MSG(("Requested DRM Infoframe is the same as current setting, skip changing HW"));
		goto done ;
	}
	BDBG_MSG(("Requested DRM Infoframe is not the same as current setting, changing HW"));

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eDRM_ID ;

	/* if no HDR support; then there is explicit SDR support only */
	/* do not send any DRM Packet */
	if ((pstDRMInfoFrame->eEOTF == BAVC_HDMI_DRM_EOTF_eSDR)
	&&  (!hHDMI->AttachedEDID.HdrDB.valid))
	{
		BHDM_DisablePacketTransmission(hHDMI, BHDM_PACKET_eDRM_ID) ;
		BDBG_MSG(("Rx Does not support HDR; no DRM packet sent")) ;
		goto done ;
	}

	if (pstDRMInfoFrame->eEOTF == BAVC_HDMI_DRM_EOTF_eMax)
	{
		BDBG_WRN(("User requested to disable DRM packet transmission; no DRM packet sent")) ;

		/* clear current DRM packet settings, so transitioning from legacy to new EOTF mode will be updated */
		BKNI_Memset(&hHDMI->DeviceSettings.stDRMInfoFrame, 0, sizeof(hHDMI->DeviceSettings.stDRMInfoFrame)) ;
		BHDM_DisablePacketTransmission(hHDMI, BHDM_PACKET_eDRM_ID) ;
		goto done ;
	}

	PacketType    = BAVC_HDMI_PacketType_eDrmInfoFrame ;
	PacketVersion = BAVC_HDMI_PacketType_DrmInfoFrameVersion ;
	PacketLength  = 0 ;

	hHDMI->PacketBytes[1] = pstDRMInfoFrame->eEOTF ;
	hHDMI->PacketBytes[2] = pstDRMInfoFrame->eDescriptorId ;

	/* make sure all packet data is set to 0 */
	if (pstDRMInfoFrame->eEOTF == BAVC_HDMI_DRM_EOTF_eSDR
	|| pstDRMInfoFrame->eEOTF == BAVC_HDMI_DRM_EOTF_eHLG)
	{
		uint8_t i ;
		for (i = 2 ; i < BAVC_HDMI_PACKET_DATA_LENGTH ; i++ )
		{
#if BDBG_DEBUG_BUILD
			if (hHDMI->PacketBytes[i] != 0)
			{
				BDBG_WRN(("Non zero packet data at PacketBytes[%d] = %d....",
					i, hHDMI->PacketBytes[i])) ;
			}
#endif
			hHDMI->PacketBytes[i] = 0 ;
		}
		PacketLength = 26 ;
	}
	else if (pstDRMInfoFrame->eEOTF == BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084) /* HDR10 */
	{
		if (pstDRMInfoFrame->eDescriptorId == BAVC_HDMI_DRM_DescriptorId_eType1)
		{
		    unsigned i;
	        const BAVC_Point * points[BAVC_NUM_COLORIMETRY_POINTS];

            /* this maps GBRW -> 0123 per original CEA/SMPTE guidelines */
	        points[0] = &pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stGreen;
	        points[1] = &pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stBlue;
	        points[2] = &pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stRed;
	        points[3] = &pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stColorVolume.stWhitePoint;

	        for (i = 0; i < BAVC_NUM_COLORIMETRY_POINTS ; i++)
	        {
	            hHDMI->PacketBytes[3 + i * 4] = (uint8_t) (points[i]->ulX & 0x00FF) ;
                hHDMI->PacketBytes[4 + i * 4] = (uint8_t) ((points[i]->ulX & 0xFF00) >> 8) ;

                hHDMI->PacketBytes[5 + i * 4] = (uint8_t) (points[i]->ulY & 0x00FF);
                hHDMI->PacketBytes[6 + i * 4] = (uint8_t) ((points[i]->ulY & 0xFF00) >> 8) ;
	        }

	        /*
	         * internally we use 100 micronits units for max luma wherever we
	         * can, but it doesn't fit in uint16_t. CEA861 requires units of 1
	         * nits on HDMI interface, so we convert it here from 100 micronits
	         * to 1 nits
	         */
			hHDMI->PacketBytes[19] =
				(uint8_t) ((pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stLuminance.uiMax / 10000) & 0x00FF) ;
			hHDMI->PacketBytes[20] =
				(uint8_t) (((pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stLuminance.uiMax / 10000) & 0xFF00) >> 8) ;

			hHDMI->PacketBytes[21] =
				(uint8_t) (pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stLuminance.uiMin & 0x00FF) ;
			hHDMI->PacketBytes[22] =
				(uint8_t) ((pstDRMInfoFrame->stType1.stMasteringDisplayColorVolume.stLuminance.uiMin & 0xFF00) >> 8) ;

			hHDMI->PacketBytes[23] =
				(uint8_t) (pstDRMInfoFrame->stType1.stContentLightLevel.ulMax & 0x00FF) ;
			hHDMI->PacketBytes[24] =
				(uint8_t) ((pstDRMInfoFrame->stType1.stContentLightLevel.ulMax & 0xFF00) >> 8) ;

			hHDMI->PacketBytes[25] =
				(uint8_t) (pstDRMInfoFrame->stType1.stContentLightLevel.ulMaxFrameAvg & 0x00FF) ;
			hHDMI->PacketBytes[26] =
				(uint8_t) ((pstDRMInfoFrame->stType1.stContentLightLevel.ulMaxFrameAvg & 0xFF00) >> 8) ;

			PacketLength  = 26 ;
		}
		else
		{
			BDBG_ERR(("Unsupported Descriptor Type Id: %d",
				pstDRMInfoFrame->eDescriptorId)) ;
		}
	}
	else
	{
		BDBG_ERR(("Unsupported EOTF: %d", pstDRMInfoFrame->eEOTF)) ;
	}


	BHDM_CHECK_RC(rc, BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes)) ;

	/* Keep a copy of the  new DRM packet */
	BKNI_Memcpy(&hHDMI->DeviceSettings.stDRMInfoFrame, pstDRMInfoFrame,
		sizeof(*pstDRMInfoFrame)) ;

#if BDBG_DEBUG_BUILD
	{
		BDBG_Level level ;

		BDBG_GetModuleLevel("BHDM_PACKET_DRM", &level) ;
		if (level == BDBG_eMsg)
		{
			BDBG_MSG(("Tx%d: Packet Type: 0x%02x  Version %d  Length: %d", hHDMI->eCoreId,
				PacketType, PacketVersion, PacketLength)) ;

			BHDM_DisplayDRMInfoFramePacket( hHDMI, pstDRMInfoFrame) ;
		}
	}
#endif

done:
	return rc ;
}



/******************************************************************************
Summary:
Get the DRM Information Info frame sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_GetDRMInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_DRMInfoFrame *stDRMInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memcpy(stDRMInfoFrame, &(hHDMI->DeviceSettings.stDRMInfoFrame),
		sizeof(*stDRMInfoFrame)) ;

	return rc ;
}
