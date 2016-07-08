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

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(BHDM_PACKET) ;

#define BHDM_P_WORDS_PER_RAM_PACKET 9
#define BHDM_P_NUM_CONFIGURABLE_RAM_PACKETS 14

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
/*
 * As per Chris Pasqualino:
 * ========================
 * This GBD specification based on GBD format 1, 12-bit data, defined as an RGB expression
 * of xvYCC709 coordinates.  This profile provides the min/max non-gamma corrected limited
 * range RGB values in the transmitted stream.
 *
 * Header Byte 1 = 0x81 = 1_000_0001b:
 *	Affected_Gamut_Seq_Num = 1	This is the first gamut provided
 *	GBD_profile = 0 		P0 Profile
 *	Next_Field = 1			This GBD data applies to the next field
 *
 * Header Byte 2 = 0x31 = 0_0_11_0001b:
 *	Current_Gamut_Seq_Num = 1	This gamut number being transmitted.
 *	Packet_Seq = 3			The only packet in the sequence
 *	No_Current_GBD = 0		A GBD is applicable to next video field.
 *
 * From the header, we know this is a profile 0 GBD.  This is important because it means
 * that there are no length parameters stored in the subpacket data. Thus, the MSB of the
 * first byte in subpacket 0 indicates the format of the remainder of the packet.
 *
 * Subpacket 0, Byte 0 = 0x92 = 1_0_0_10_010b:
 *	Format_Flag = 1 		GBD data is a range description
 *	GBD_Color_Precision = 2 	12-bit precision on GBD data
 *	GBD_Color_Space = 2 	RGB Expression of xvYCC709 coordinates
 *
 * The HDMI spec states in section E.4 that for Format_Flag = 1, the Packet_Range_Data
 * shall be stored in the following order:
 *	Min_Red_Data
 *	Max_Red_Data
 *	Min_Green_Data
 *	Max_Green_Data
 *	Min_Blue_Data
 *	Max_Blue_Data
 */
#define BHDM_Packet_GamutMetadataHB1  0x81
#define BHDM_Packet_GamutMetadataHB2  0x31
static const uint8_t BHDM_Packet_GamutMetadataSubPkt[] = {
		0x92, 0x9B, 0x52, 0xF4, 0x8D,
		0x72, 0x96, 0x8C, 0xC2, 0x92
};
#endif

/***************************************************************************
BHDM_P_InitializePacketRAM
Summary: Configure/Initialize the Packet RAM with NULL Packets
****************************************************************************/
BERR_Code BHDM_InitializePacketRAM(
   const BHDM_Handle hHDMI		  /* [in] HDMI handle */
)
{
	BERR_Code	rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t	Register, ulOffset ;
	uint32_t RegAddr ;
	uint8_t Packet ;

	/*CP*  13 Enable/Configure RAM Packets - HDMI_TODO Add PUBLIC FUNCTION(s)??? */

	/* This will configure a NULL packet for sending.
	   This is just to get things up and running. */

	/* set	 RAM_ENABLE  */
	/* set	 ACTIVE_RAM_PACKETS = 14'd1   - Enable RAM Packet 0 */

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* Zero (NULL) out all RAM Packets */
	for (Packet = 0 ; Packet < BHDM_P_NUM_CONFIGURABLE_RAM_PACKETS; Packet++)
	{
		for (Register = 0 ; Register < BHDM_P_WORDS_PER_RAM_PACKET ; Register++)
		{
			RegAddr = BCHP_HDMI_RAM_GCP_0
				+ Packet * BHDM_P_WORDS_PER_RAM_PACKET * 4
				+ Register * 4 ;

			BREG_Write32(hRegister, RegAddr + ulOffset, (uint32_t) 0) ;
		}
	}

	/* Enable the Packet Ram */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset) ;
	Register |= BCHP_FIELD_DATA(HDMI_RAM_PACKET_CONFIG, RAM_ENABLE, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset,  Register) ;

	return rc ;
}


BERR_Code BHDM_EnablePacketTransmission(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_Packet PacketId)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t XmitPacketN ;
	uint8_t timeoutMs ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* set the transmission bit */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset) ;
	Register |= 1 << (uint8_t) PacketId ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset, Register) ;

	/* wait until RAM Packet Status starts transmitting (RAM_PACKET_X = 0) */
	timeoutMs = 10 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_STATUS + ulOffset) ;
		XmitPacketN = Register & (1 << (uint8_t) PacketId) ;
		if (XmitPacketN)
			break ;

		BDBG_MSG(("Tx%d: Attempting start of RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		BKNI_Sleep(1) ;
	} while ( timeoutMs-- ) ;

	if (!XmitPacketN)
	{
		BDBG_ERR(("Tx%d: Timeout Error starting RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		rc = BERR_TRACE(BERR_TIMEOUT) ;
		goto done ;
	}

done:
	return rc ;
}



BERR_Code BHDM_DisablePacketTransmission(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_Packet PacketId)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t XmitPacketN ;
	uint8_t timeoutMs ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* clear the transmission bit */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset) ;
	Register &= ~(1 << (uint8_t) PacketId) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset, Register) ;

	/* wait until RAM Packet Status stops transmitting (RAM_PACKET_X = 0) */
	timeoutMs = 10 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_STATUS + ulOffset) ;
		XmitPacketN = Register & (1 << (uint8_t) PacketId) ;
		if (!XmitPacketN)
			break ;

		BDBG_MSG(("Tx%d: Attempting stop of RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		BKNI_Sleep(1) ;
	} while ( timeoutMs-- ) ;

	if (XmitPacketN)
	{
		BDBG_ERR(("Tx%d: Timeout Error stopping RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		rc = BERR_TRACE(BERR_TIMEOUT) ;
		goto done ;
	}

done:
	return rc ;
}



BERR_Code BHDM_P_WritePacket(
	const BHDM_Handle hHDMI, BHDM_Packet PhysicalHdmiRamPacketId,
	uint8_t PacketType, uint8_t PacketVersion, uint8_t PacketLength,
	uint8_t *PacketBytes)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;

	uint32_t Register, ulOffset ;

	uint32_t PacketRegisterOffset ;
	uint32_t checksum ;
	uint8_t i ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* zero out the unused packet bytes */
	for (i = PacketLength + 1 ; i < BHDM_NUM_PACKET_BYTES; i++)
		PacketBytes[i] = 0 ;

	/* calculate Packet Checksum only on CEA-861 InfoFrame Packets */
	if (PacketType > BHDM_INFOFRAME_PACKET_TYPE)
	{
		/* calculate checksum */
		checksum = 0 ;
		checksum = PacketType + PacketVersion + PacketLength ;
		for (i = 1 ; i <= PacketLength ; i++)
			checksum = checksum + PacketBytes[i] ;

		PacketBytes[0] = 256 - (uint8_t) (checksum % 256)  ;
#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Infoframe Checksum Byte: %#X",
			hHDMI->eCoreId, PacketBytes[0])) ;
#endif
	}
	else
	{
		/* do not modify PB0; may be used for some other purpose */
		BDBG_WRN(("Tx%d: Writing non-Infoframe Packet Type %#x",
			hHDMI->eCoreId, PacketType)) ;
	}

#ifdef BCHP_PWR_RESOURCE_HDMI_TX
	BCHP_PWR_AcquireResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX);
#endif
	/* disable transmission of the RAM Packet */
	BHDM_CHECK_RC(rc, BHDM_DisablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;


	/* calculate the offset of where the RAM for Packet ID begins */
	PacketRegisterOffset =
		   BCHP_HDMI_RAM_GCP_0
		+ BHDM_P_WORDS_PER_RAM_PACKET * 4 * PhysicalHdmiRamPacketId ;

	/* create/write the header use HDMI_RAM_PACKET_1_0 for Register field names */
	Register = BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_0, PacketType)
			 | BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_1, PacketVersion)
			 | BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_2, PacketLength) ;
	BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;

#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
	BDBG_MSG(("Tx%d: Packet Type %#X Packet %d Loading...", hHDMI->eCoreId,
		PacketType, PhysicalHdmiRamPacketId)) ;
	BDBG_MSG(("Tx%d: Addr [%#x] = %x", hHDMI->eCoreId,
		PacketRegisterOffset, Register)) ;
#endif

	/* load the Packet Bytes */
	for (i = 0 ; i < BHDM_NUM_PACKET_BYTES; i = i + 7)
	{
		PacketRegisterOffset += 4 ;

		/* create/write the subpacket data use HDMI_RAM_PACKET_1_1 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_0, PacketBytes[i])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_1, PacketBytes[i+1])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_2, PacketBytes[i+2])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_3, PacketBytes[i+3]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;
#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId,
			PacketRegisterOffset, Register)) ;
#endif

		PacketRegisterOffset += 4 ;

		/* create/write the subpacket data use HDMI_RAM_PACKET_1_1 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_2, SUBPACKET_1_BYTE_4, PacketBytes[i+4])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_2, SUBPACKET_1_BYTE_5, PacketBytes[i+5])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_2, SUBPACKET_1_BYTE_6, PacketBytes[i+6]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;
#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId,
			PacketRegisterOffset, Register)) ;
#endif
	}

	/* reenable the packet transmission */
	BHDM_CHECK_RC(rc, BHDM_EnablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;
#ifdef BCHP_PWR_RESOURCE_HDMI_TX
	BCHP_PWR_ReleaseResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX);
#endif
done:
	return rc ;
}



/******************************************************************************
Summary:
	Create/Set a user defined packet
*******************************************************************************/
BERR_Code BHDM_SetUserDefinedPacket(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	BHDM_Packet PhysicalHdmiRamPacketId,

	uint8_t PacketType,
	uint8_t PacketVersion,
	uint8_t PacketLength,

	uint8_t *PacketData
)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	uint8_t i ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (PacketLength > BHDM_NUM_PACKET_BYTES)
	{
		BDBG_ERR(("Tx%d: User Defined Packet Length larger than HDMI Packet Size %d",
			hHDMI->eCoreId, BHDM_NUM_PACKET_BYTES)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	if (PhysicalHdmiRamPacketId < BHDM_Packet_eUser1)
	{
		BDBG_ERR(("Tx%d: Pre-defined Packets cannot be modified; use BHDM_Packet_eUserX",
			hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;

	}

	for (i = 0 ; i < BHDM_Packet_eUser1;  i++)
	{
		if (PacketType == (BHDM_Packet) i)
		{
			BDBG_ERR(("Tx%d: User Packet Type cannot be pre-defined Packet Type: %d",
				hHDMI->eCoreId, i)) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
		}
	}


	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;


	/* copy the user defined data */
	for (i = 0 ; i < PacketLength;	i++)
		hHDMI->PacketBytes[i] = PacketData[i] ;

	BDBG_MSG(("Tx%d: Packet Bytes: %s", hHDMI->eCoreId, hHDMI->PacketBytes)) ;

	BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes) ;

done:
	return rc ;
}


#if BHDM_CONFIG_HDMI_1_3_SUPPORT
/******************************************************************************
Summary:
Set/Enable the Gamut Metadata packet to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_P_SetGamutMetadataPacket(
	const BHDM_Handle hHDMI		  /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;

	BHDM_Packet PhysicalHdmiRamPacketId ;
	uint32_t PacketRegisterOffset ;
	uint32_t Register, ulOffset;
	uint32_t i;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eGamutMetadata_ID;

	hHDMI->PacketBytes[0] = BAVC_HDMI_PacketType_eGamutMetadataPacket ;
	hHDMI->PacketBytes[1] = BHDM_Packet_GamutMetadataHB1;
	hHDMI->PacketBytes[2] = BHDM_Packet_GamutMetadataHB2;

	for (i=0; i<sizeof(BHDM_Packet_GamutMetadataSubPkt); i++) {
		hHDMI->PacketBytes[i+3] = BHDM_Packet_GamutMetadataSubPkt[i];
	}

	/* disable transmission of the RAM Packet */
	BHDM_CHECK_RC(rc, BHDM_DisablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;

	/* calculate the offset of where the RAM for Packet ID begins */
	PacketRegisterOffset =
		  BCHP_HDMI_RAM_GCP_0
		+ BHDM_P_WORDS_PER_RAM_PACKET * 4 * PhysicalHdmiRamPacketId ;

	/* load the Packet Bytes */
	for (i = 0 ; i < BHDM_NUM_PACKET_BYTES; i = i + 7)
	{
		/* create/write the subpacket data use HDMI_RAM_PACKET_1_0 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_0, hHDMI->PacketBytes[i+0])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_1, hHDMI->PacketBytes[i+1])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_2, hHDMI->PacketBytes[i+2]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;

#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId, PacketRegisterOffset, Register)) ;
#endif
		PacketRegisterOffset += 4 ;

		/* create/write the subpacket data use HDMI_RAM_PACKET_1_1 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_0, hHDMI->PacketBytes[i+3])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_1, hHDMI->PacketBytes[i+4])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_2, hHDMI->PacketBytes[i+5])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_3, hHDMI->PacketBytes[i+6]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;

#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId, PacketRegisterOffset, Register)) ;
#endif
		PacketRegisterOffset += 4 ;
	}

	/* reenable the packet transmission */
	BHDM_CHECK_RC(rc, BHDM_EnablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;

done:
	return rc ;
}

#endif
