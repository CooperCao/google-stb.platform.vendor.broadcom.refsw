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
#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc_hdmi.h"

BDBG_MODULE(BHDM_PACKET_DRM) ;


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

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eDRM_ID ;

	/* if no HDR support; then there is explicit SDR support only */
	/* do not send any DRM Packet */
	if ((pstDRMInfoFrame->eEOTF == BAVC_HDMI_DRM_EOTF_eSDR)
	&& (!hHDMI->AttachedEDID.HdrDB.valid))
	{
		BHDM_DisablePacketTransmission(hHDMI, BHDM_PACKET_eDRM_ID) ;
		BDBG_MSG(("Rx Does not support HDR; no DRM packet sent")) ;
		goto done ;
	}

	if (pstDRMInfoFrame->eEOTF == BAVC_HDMI_DRM_EOTF_eMax)
	{
		BDBG_WRN(("User requested to disable DRM packet transmission; no DRM packet sent")) ;
		BHDM_DisablePacketTransmission(hHDMI, BHDM_PACKET_eDRM_ID) ;
		goto done ;
	}

	PacketType    = BAVC_HDMI_PacketType_eDrmInfoFrame ;
	PacketVersion = BAVC_HDMI_PacketType_DrmInfoFrameVersion ;
	PacketLength  = 0 ;

	hHDMI->PacketBytes[1] = pstDRMInfoFrame->eEOTF ;
	hHDMI->PacketBytes[2] = pstDRMInfoFrame->eDescriptorId ;

	if (pstDRMInfoFrame->eDescriptorId == BAVC_HDMI_DRM_DescriptorId_eType1)
	{
		hHDMI->PacketBytes[3] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayPrimaries[0].X & 0x00FF) ;
		hHDMI->PacketBytes[4] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayPrimaries[0].X & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[5] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayPrimaries[0].Y & 0x00FF) ;
		hHDMI->PacketBytes[6] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayPrimaries[0].Y & 0xFF00) >> 8) ;


		hHDMI->PacketBytes[7] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayPrimaries[1].X & 0x00FF) ;
		hHDMI->PacketBytes[8] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayPrimaries[1].X & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[9] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayPrimaries[1].Y & 0x00FF) ;
		hHDMI->PacketBytes[10] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayPrimaries[1].Y & 0xFF00) >> 8) ;


		hHDMI->PacketBytes[11] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayPrimaries[2].X & 0x00FF) ;
		hHDMI->PacketBytes[12] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayPrimaries[2].X & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[13] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayPrimaries[2].Y & 0x00FF) ;
		hHDMI->PacketBytes[14] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayPrimaries[2].Y & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[15] =
			(uint8_t) (pstDRMInfoFrame->Type1.WhitePoint.X & 0x00FF) ;
		hHDMI->PacketBytes[16] =
			(uint8_t) ((pstDRMInfoFrame->Type1.WhitePoint.X & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[17] =
			(uint8_t) (pstDRMInfoFrame->Type1.WhitePoint.Y & 0x00FF) ;
		hHDMI->PacketBytes[18] =
			(uint8_t) ((pstDRMInfoFrame->Type1.WhitePoint.Y & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[19] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayMasteringLuminance.Max & 0x00FF) ;
		hHDMI->PacketBytes[20] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayMasteringLuminance.Max & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[21] =
			(uint8_t) (pstDRMInfoFrame->Type1.DisplayMasteringLuminance.Min & 0x00FF) ;
		hHDMI->PacketBytes[22] =
			(uint8_t) ((pstDRMInfoFrame->Type1.DisplayMasteringLuminance.Min & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[23] =
			(uint8_t) (pstDRMInfoFrame->Type1.MaxContentLightLevel & 0x00FF) ;
		hHDMI->PacketBytes[24] =
			(uint8_t) ((pstDRMInfoFrame->Type1.MaxContentLightLevel & 0xFF00) >> 8) ;

		hHDMI->PacketBytes[25] =
			(uint8_t) (pstDRMInfoFrame->Type1.MaxFrameAverageLightLevel & 0x00FF) ;
		hHDMI->PacketBytes[26] =
			(uint8_t) ((pstDRMInfoFrame->Type1.MaxFrameAverageLightLevel & 0xFF00) >> 8) ;

		PacketLength  = 26 ;
	}



	BHDM_CHECK_RC(rc, BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes)) ;

	BKNI_Memcpy(&hHDMI->DeviceSettings.stDRMInfoFrame, pstDRMInfoFrame,
		sizeof(BAVC_HDMI_DRMInfoFrame)) ;

	BDBG_MSG(("------------------- NEW  DRM INFOFRAME ------------------")) ;
	BDBG_MSG(("Tx%d: Packet Type: 0x%02x  Version %d  Length: %d", hHDMI->eCoreId,
		PacketType, PacketVersion, PacketLength)) ;
	BDBG_MSG(("Tx%d: Checksum            %#02x", hHDMI->eCoreId,
		hHDMI->PacketBytes[0])) ;

	BDBG_MSG(("Tx%d: DRM EOTF     %s", hHDMI->eCoreId,
		BAVC_HDMI_DRMInfoFrame_EOTFToStr(pstDRMInfoFrame->eEOTF))) ;

	BDBG_MSG(("Tx%d: DRM Descriptor ID   %s", hHDMI->eCoreId,
		BAVC_HDMI_DRMInfoFrame_DescriptorIdToStr(pstDRMInfoFrame->eDescriptorId))) ;

	{
		uint8_t i ;

		for (i = 1 ; i <= PacketLength ; i++)
		{
			BDBG_MSG(("Tx%d: Data Byte %02d = %#02x h", hHDMI->eCoreId,
				i, hHDMI->PacketBytes[i])) ;
		}
	}

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
		sizeof(BAVC_HDMI_DRMInfoFrame)) ;

	return rc ;
}
