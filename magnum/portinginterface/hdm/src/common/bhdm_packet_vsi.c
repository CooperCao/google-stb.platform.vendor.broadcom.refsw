/******************************************************************************
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
 ******************************************************************************/

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc.h"

BDBG_MODULE(BHDM_PACKET_VSI) ;

/******************************************************************************
Summary:
	Create/Set a Vendor Specific Info Frame
*******************************************************************************/
BERR_Code BHDM_SetVendorSpecificInfoFrame(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	const BAVC_HDMI_VendorSpecificInfoFrame *pVendorSpecificInfoFrame)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;

	uint8_t packetByte5 ;

	BHDM_Packet PhysicalHdmiRamPacketId = BHDM_Packet_eVendorSpecific_ID ;
	BAVC_HDMI_VSInfoFrame_HDMIVideoFormat eHdmiVideoFormat ;
	BAVC_HDMI_VSInfoFrame_HDMIVIC eHdmiVic ;

	BAVC_HDMI_VendorSpecificInfoFrame NewVSI ;

	PacketType = BAVC_HDMI_PacketType_eVendorSpecificInfoFrame ;
	PacketVersion = BAVC_HDMI_PacketType_VendorSpecificInfoFrameVersion ;
	PacketLength = 0 ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_eReserved ;

	/* BHDM_SetVendorSpecificInfoFrame packet also called from BHDM_EnableDisplay
	    using the VSI stored in the HDMI handle
	*/

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	BKNI_Memcpy(&NewVSI, pVendorSpecificInfoFrame,
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;


	/* ready the Packet Data structure used for transmissIon of the Vendor Specific Data */
	/* Copy the IEEE Reg ID; skip over the checksum byte */
	BKNI_Memcpy(hHDMI->PacketBytes + 1, NewVSI.uIEEE_RegId,
		BAVC_HDMI_IEEE_REGID_LEN) ;
	PacketLength = BAVC_HDMI_IEEE_REGID_LEN;

	/* Set the HDMI Video Format */
	if (pVendorSpecificInfoFrame->eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat)
	{
		eHdmiVideoFormat =
			BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat ;
	}
	else
	{
		/* check for non-3D HDMI Video Format e.g. 4K; use current VideoFmt */
		switch(hHDMI->DeviceSettings.eInputVideoFmt)
		{
#if BHDM_HAS_HDMI_20_SUPPORT
		case BFMT_VideoFmt_e3840x2160p_50Hz :
		case BFMT_VideoFmt_e3840x2160p_60Hz :
			eHdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;
			break ;
#endif

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
		case BFMT_VideoFmt_e3840x2160p_30Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2997_30Hz ;
			break ;

		case BFMT_VideoFmt_e3840x2160p_25Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_25Hz ;

			break ;

		case BFMT_VideoFmt_e3840x2160p_24Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2398_24Hz ;

			break ;

		case BFMT_VideoFmt_e4096x2160p_24Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_SMPTE_24Hz ;

			break ;
#endif

		default :
			eHdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_eReserved ;
			break ;
		}
	}

	hHDMI->PacketBytes[4] = eHdmiVideoFormat << 5 ;
	PacketLength++ ;


	NewVSI.eHdmiVic = eHdmiVic ;
	NewVSI.eHdmiVideoFormat = eHdmiVideoFormat ;
	/* Set the Extended Resolution or 3D Format */
	switch (eHdmiVideoFormat)
	{
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone :
	default :
		BDBG_MSG(("Tx%d: HDMI Normal Resolution", hHDMI->eCoreId)) ;
		goto done;

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution :
		BDBG_MSG(("Tx%d: HDMI Extended Resolution", hHDMI->eCoreId )) ;
		packetByte5 = eHdmiVic ;
		break ;
#endif

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat :
		BDBG_MSG(("Tx%d: HDMI 3D Format", hHDMI->eCoreId)) ;
		packetByte5 = NewVSI.e3DStructure << 4 ;
		break ;
	}

	hHDMI->PacketBytes[5] = packetByte5 ;
	PacketLength++ ;

	/* if 3D_Structure = sidebysideHalf, 3D_Ext_Data is added to HDMI VSI for additional info */
	if ((NewVSI.eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat)
	&& (NewVSI.e3DStructure == BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideHalf))
	{
		hHDMI->PacketBytes[6] = NewVSI.e3DExtData << 4;
		PacketLength++;
	}

done:

    if (pVendorSpecificInfoFrame->bDolbyVisionEnabled)
    {
        PacketLength = 24;
    }

	/* update current device settings with new information on VendorSpecificInfoFrame */
	BKNI_Memcpy(&hHDMI->DeviceSettings.stVendorSpecificInfoFrame, &NewVSI,
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes) ;

	return rc ;
}



/******************************************************************************
Summary:
	Get currently transmitted Vendor Specific Info Frame
*******************************************************************************/
void  BHDM_GetVendorSpecificInfoFrame(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	BAVC_HDMI_VendorSpecificInfoFrame *pVendorSpecficInfoFrame)
{
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(pVendorSpecficInfoFrame,
		0, sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	BKNI_Memcpy(pVendorSpecficInfoFrame, &(hHDMI->DeviceSettings.stVendorSpecificInfoFrame),
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;
}
