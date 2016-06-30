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

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc.h"

BDBG_MODULE(BHDM_PACKET_SPD) ;


/******************************************************************************
Summary:
Get the length of  a string
*******************************************************************************/
static int _strlen(const unsigned char *s) {
	int i=0;
	while (*s++) i++;
	return i;
}



/******************************************************************************
Summary:
Set/Enable the Source Product Description Info Frame packet to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_SetSPDInfoFramePacket(
   const BHDM_Handle hHDMI		  /* [in] HDMI handle */
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BHDM_Packet PhysicalHdmiRamPacketId ;

	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* check length of vendor and description */
	PacketLength = _strlen(hHDMI->DeviceSettings.SpdVendorName) ;
	if (PacketLength > BAVC_HDMI_SPD_IF_VENDOR_LEN)
	{
		BDBG_ERR(("Tx%d: SPD Vendor Name Length %d larger than MAX: %d",
			hHDMI->eCoreId, PacketLength, BAVC_HDMI_SPD_IF_VENDOR_LEN)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	PacketLength = _strlen(hHDMI->DeviceSettings.SpdDescription) ;
	if (PacketLength > BAVC_HDMI_SPD_IF_DESC_LEN)
	{
		BDBG_ERR(("Tx%d: SPD Description Length %d larger than MAX: %d",
			hHDMI->eCoreId, PacketLength, BAVC_HDMI_SPD_IF_DESC_LEN)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}


	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eSPD_ID ;

	PacketType	  = BAVC_HDMI_PacketType_eSpdInfoFrame ;
	PacketVersion = BAVC_HDMI_PacketType_SpdInfoFrameVersion ;
	PacketLength  = 25 ;

	BKNI_Memcpy(hHDMI->PacketBytes+1, hHDMI->DeviceSettings.SpdVendorName,
		BAVC_HDMI_SPD_IF_VENDOR_LEN) ;
	BKNI_Memcpy(hHDMI->PacketBytes+9, hHDMI->DeviceSettings.SpdDescription,
		BAVC_HDMI_SPD_IF_DESC_LEN) ;
	hHDMI->PacketBytes[25] = hHDMI->DeviceSettings.eSpdSourceDevice ;


	BHDM_CHECK_RC(rc, BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes)) ;

	BDBG_MSG(("-------------------- NEW  SPD INFOFRAME -------------------")) ;
	BDBG_MSG(("Tx%d: Packet Type: 0x%02x  Version %d  Length: %d",
		hHDMI->eCoreId, PacketType, PacketVersion, PacketLength)) ;

	BDBG_MSG(("Tx%d: Vendor Name: %s", hHDMI->eCoreId,
		hHDMI->DeviceSettings.SpdVendorName)) ;

	BDBG_MSG(("Tx%d: Vendor Description: %s", hHDMI->eCoreId,
		hHDMI->DeviceSettings.SpdDescription)) ;

	BDBG_MSG(("Tx%d: Device Type: %s", hHDMI->eCoreId,
		BAVC_HDMI_SpdInfoFrame_SourceTypeToStr(
			hHDMI->DeviceSettings.eSpdSourceDevice))) ;
	BDBG_MSG(("--------------------- END SPD INFOFRAME ---------------------")) ;

done:
	return rc ;
}
