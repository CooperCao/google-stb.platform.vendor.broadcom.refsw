/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhdr.h"
#include "bhdr_priv.h"
#include "bhdr_debug.h"

BDBG_MODULE(BHDR_PACKET_PRIV) ;


 /******************************************************************************
Summary:
Data structure used to map RAM Packet Types to physical packet memory (registers).
As of 01/01/2007 these are the known Packet Types

Packet Type Value Packet Type
	0x00 Null
	0x01 Audio Clock Regeneration (N/CTS)
	0x02 Audio Sample
	0x03 General Control
	0x04 AudioContentProtection Packet
	0x05 ISRC1 Packet
	0x06 ISRC2 Packet
	0x07 Reserved for DSD Packet

0x80+InfoFrame Type EIA/CEA-861B InfoFrame
	0x81 Vendor-Specific InfoFrame
	0x82 AVI InfoFrame
	0x83 Source Product Descriptor InfoFrame
	0x84 Audio InfoFrame
	0x85 MPEG Source InfoFrame


The list below can be modified, but is best left in its current configuration.  It maps which HDMI
Rx Registers will get which HDMI Packets e.g. Packet 4 will receive the AVI Infoframe Packet.
The eUnused packet is unassigned and therefore no packet will be written to these registers.

In the event of errors in the stream, the packet memory (registers) may have updates since
random data *might* match the packet types listed above.  Infoframe packets types have
checksums which can be used to filter bad packets.
*******************************************************************************/
typedef struct BHDR_P_RamPacketMap
{
	BAVC_HDMI_PacketType ePacketType ;
	bool bMonitorStoppedPackets ;
} BHDR_P_RamPacketMap ;

#define ACR_PACKET_LOCATION 1

static const BHDR_P_RamPacketMap RamPacketMap[BHDR_P_NUM_PACKETS] =
{
	/* Packet 00 */ {BAVC_HDMI_PacketType_eGamutMetadataPacket, false},

	/*
	    the clock regeneration packet is ideally used for debug purposes
	    future designs may pass this information directly to the audio core

	    DO NOT MOVE the AudioClockRegeneration Packet from Packet 01
	    other code in this unit depend on this physical location
	 */

	/* Packet 01 */ {BAVC_HDMI_PacketType_eAudioClockRegeneration, false},

	/* Packet 02 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 03 */ {BAVC_HDMI_PacketType_eGeneralControl, false},

	/* Packet 04 */ {BAVC_HDMI_PacketType_eAviInfoFrame, true},
	/* Packet 05 */ {BAVC_HDMI_PacketType_eSpdInfoFrame, false},
	/* Packet 06 */ {BAVC_HDMI_PacketType_eAudioInfoFrame, false },
	/* Packet 07 */ {BAVC_HDMI_PacketType_eVendorSpecificInfoframe, true},
	/* Packet 08 */ {BAVC_HDMI_PacketType_eMpegInfoFrame, false},

	/* Packet 09 */ {BAVC_HDMI_PacketType_eAudioContentProtection, true},
	/* Packet 10 */ {BAVC_HDMI_PacketType_eISRC1, true},
	/* Packet 11 */ {BAVC_HDMI_PacketType_eISRC2, true},

	/* Packet 12 */ {BAVC_HDMI_PacketType_eDirectStream, false},

	/* Packet 13 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 14 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 15 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 16 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 17 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 18 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 19 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 20 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 21 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 22 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 23 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 24 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 25 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 26 */ {BAVC_HDMI_PacketType_eUnused, false},
	/* Packet 27 */ {BAVC_HDMI_PacketType_eUnused, false}
}  ;



#define BHDR_P_GET_PACKET_RAM_INDEX_BY_TYPE(packetType, index) \
	do \
	{ \
		int i; \
		for (i=0; i<BHDR_P_NUM_PACKETS; i++) \
		{ \
			if (RamPacketMap[i].ePacketType == packetType) \
			{ \
				index = i; \
				break; \
			} \
		} \
	} while(0)

BERR_Code BHDR_P_GetPacketRamIndexByType(
	BHDR_Handle hHDR, BAVC_HDMI_PacketType ePacketType,  uint8_t *PacketIndex)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

	BSTD_UNUSED(hHDR) ;

	for (i=0; i < BHDR_P_NUM_PACKETS; i++)
	{
		if (RamPacketMap[i].ePacketType == ePacketType)
		{
			*PacketIndex = i ;
			break;
		}
	}

	if (i == BHDR_P_NUM_PACKETS)
	{
		BDBG_ERR(("Invalid Packet Type")) ;
		rc = BERR_INVALID_PARAMETER ;
	}

	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_InitializePacketRAM_isr(
   BHDR_Handle hHDR
)
{

	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t uiPacketBit ;
	uint8_t PacketNum ;

	uint32_t RegAddr ;


	BDBG_ENTER(BHDR_P_InitializePacketRAM_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;


	/* default Packet Stop Detection to use VSync vs HSync */
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_SYNC_SELECT + ulOffset, 0xFFFFFFF) ;

	/* clear any enabled packet stop detection  */
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT + ulOffset, 0) ;


	BHDR_P_ENABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;

		/* Zero out all Packet Registers */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset) ;
		Register |= BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CFG , INIT_RAM_PACKET) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset, Register) ;

		Register &= ~ BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CFG , INIT_RAM_PACKET) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CFG + ulOffset, Register) ;


	BHDR_P_DISABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;



	/* assign the Packet Types to the physical packet RAM; use the
	     HDMI Rx Interrupt table to get Phys Packet number and Packet Type
	 */

	for (PacketNum= 0; PacketNum < BHDR_P_NUM_PACKETS ; PacketNum++)
	{
		/* skip Unused Packet Types */
		if (RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eUnused)
			continue ;

		/* assign the Packet Type to the RAM Packet Number using the RamPacketMap */
		/* NOTE: there is a default mapping at reset/power up */

		RegAddr = BCHP_HDMI_RX_0_RAM_PACKET_TYPES_0  + ulOffset
			+ (uint8_t) (PacketNum / 4) * 4 ;

		Register = BREG_Read32(hRegister, RegAddr) ;
		Register &= ~(0xFF << ((PacketNum % 4) * 8)) ;
		Register |= (RamPacketMap[PacketNum].ePacketType << ((PacketNum % 4) * 8)) ;
		BREG_Write32(hRegister, RegAddr, Register) ;

		/* Enable reception of this Packet */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_UPDATE_CONFIG + ulOffset) ;
		uiPacketBit = 1 << PacketNum ;
		Register |= uiPacketBit ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_UPDATE_CONFIG + ulOffset, Register) ;

		/* Enable the IRQ for reception of this Packet */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_IRQ_ENABLE + ulOffset) ;
		uiPacketBit = 1 << PacketNum ;
		Register |= uiPacketBit ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_0_RAM_PACKET_IRQ_ENABLE + ulOffset, Register) ;


#if 0
		BDBG_MSG(("Mask %#08X for Packet %d",
			(0xFF << ((PacketNum % 4) * 8)), PacketNum)) ;
#endif
	}


	/* clear individual locks */
	for (PacketNum = 0 ; PacketNum < BHDR_P_NUM_PACKETS; PacketNum++)
	{
		BHDR_P_ClearPacketRAMLock_isr(hHDR, PacketNum) ;
	}

	/* clear internal Packet Ram used for status */
	BHDR_P_ClearPacketMemory_isr(hHDR) ;

	BDBG_LEAVE(BHDR_P_InitializePacketRAM_isr) ;
	return rc ;
}




/******************************************************************************
Summary:
Copy the Packet Bytes from the H/W registers
*******************************************************************************/
static void BHDR_P_GetPacketRAM_isr(
	BREG_Handle hRegister, uint32_t PacketRegisterOffset, uint8_t *DataBytes)
{
	uint32_t Register ;
	uint8_t i ;

	for (i = 0 ; i < BAVC_HDMI_PB_LENGTH; i = i + 7)
	{
		Register = BREG_Read32(hRegister, PacketRegisterOffset) ;
		DataBytes[i+0] = (uint8_t) (Register & 0x000000FF) ;
		DataBytes[i+1] = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
		DataBytes[i+2] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;
		DataBytes[i+3] = (uint8_t) ((Register & 0xFF000000) >> 24) ;

		PacketRegisterOffset +=  4 ;

		Register = BREG_Read32(hRegister, PacketRegisterOffset) ;
		DataBytes[i+4] = (uint8_t) (Register & 0x000000FF) ;
		DataBytes[i+5] = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
		DataBytes[i+6] = (uint8_t) ((Register & 0x00FF0000) >> 16) ;

		PacketRegisterOffset +=  4 ;
	}
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ClearPacketRAM_isr(
   BHDR_Handle hHDR, uint8_t PacketNum
)
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t RegAddr ;

	BDBG_ENTER(BHDR_P_ClearPacketRAM_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	if ((RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eUnused)
	|| (RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eNull))
		goto done ;


	BHDR_P_ENABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;

		/* Zero out all Registers for this Packet */
		RegAddr = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER  + ulOffset
			+ PacketNum * 4 * BHDR_P_PACKET_WORDS  ;

		for (Register = 0 ; Register < BHDR_P_PACKET_WORDS ; Register++)
		{
			BREG_Write32(hRegister, RegAddr, (uint32_t) 0) ;
			RegAddr = RegAddr + 4 ;
		}

	BHDR_P_DISABLE_PACKET_WRITES(hRegister, Register, ulOffset) ;

done:
	BDBG_LEAVE(BHDR_P_ClearPacketRAM_isr) ;
	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_P_ClearPacketMemory_isr(BHDR_Handle hHDR)
{
	BDBG_ENTER(BHDR_P_ClearPacketMemory_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset(&hHDR->AviInfoFrame, 0, sizeof(BAVC_HDMI_AviInfoFrame));
	BKNI_Memset(&hHDR->AudioInfoFrame, 0, sizeof(BAVC_HDMI_AudioInfoFrame));
	BKNI_Memset(&hHDR->SPDInfoFrame, 0, sizeof(BAVC_HDMI_SPDInfoFrame));
	BKNI_Memset(&hHDR->AudioContentProtection, 0, sizeof(BAVC_HDMI_ACP)) ;
	BKNI_Memset(&hHDR->AudioClockRegenerationPacket,
		0,  sizeof(BAVC_HDMI_AudioClockRegenerationPacket)) ;
	BKNI_Memset(&hHDR->VSInfoFrame, 0, sizeof(BAVC_HDMI_VendorSpecificInfoFrame));

#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
	BKNI_Memset(&hHDR->GamutPacket, 0, sizeof(BAVC_HDMI_GamutPacket)) ;
#endif
#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
	BKNI_Memset(&hHDR->ISRC, 0, sizeof(BAVC_HDMI_ISRC)) ;
#endif

	BDBG_LEAVE(BHDR_P_ClearPacketMemory_isr) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ClearPacketRAMLock_isr(
   BHDR_Handle hHDR,	uint8_t PacketNum
)
{

	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;

	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t uiPacketLockedBit ;

	BDBG_ENTER(BHDR_P_ClearPacketRAMLock_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	uiPacketLockedBit = 1 << PacketNum ;

	/* Clear Locked Packet by writing 0 followed by 1 */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset) ;

	Register |= uiPacketLockedBit ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset, Register) ;

	Register &= ~uiPacketLockedBit ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_P_ClearPacketRAMLock_isr) ;
	return rc ;
}


/******************************************************************************
Summary:
*******************************************************************************/
static BERR_Code BHDR_P_CalculateInfoframeChecksum_isr(
	BAVC_HDMI_Packet *Packet, uint8_t *checksum)
{
	uint8_t i ;
	uint32_t temp = 0 ;
	uint32_t chksum ;

	temp = Packet->Type + Packet->Length + Packet->Version  ;

	for (i = 0 ; i < BAVC_HDMI_PB_LENGTH; i++)
		temp = temp + Packet->DataBytes[i]  ;

	chksum = temp % 256 ;

	*checksum = (uint8_t) chksum ;

	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
	Copy Packet Data
*******************************************************************************/
static BERR_Code BHDR_P_GetPacketData_isr(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_Packet *Packet
)
{
	BREG_Handle hRegister ;
	uint32_t ulOffset ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t PacketRegisterOffset ;


	BDBG_ENTER(BHDR_GetPacketData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister= hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;


	/* calculate the offset of where the Packet RAM begins */
	PacketRegisterOffset = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER + ulOffset
		+ BHDR_P_PACKET_WORDS * 4 * PacketNum ;

	/* read the Header Bytes (HBn) first */
	Register = BREG_Read32(hRegister, PacketRegisterOffset) ;
	Packet->Type    = (uint8_t)  (Register & 0x000000FF) ;
#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
	if (Packet->Type == BAVC_HDMI_PacketType_eGamutMetadataPacket)
	{
		hHDR->GamutPacket.Type = Packet->Type ;

		rc = BHDR_P_GetGamutPacketData_isr(
			hHDR, PacketNum, &hHDR->GamutPacket) ;
		goto done ;
	}
#endif

	Packet->Version = (uint8_t) ((Register & 0x0000FF00) >>  8) ;
	Packet->Length  = (uint8_t) ((Register & 0x00FF0000) >> 16) ;

	/* advance to the next packet register */
	PacketRegisterOffset +=  4 ;

	/* now read the Packet Bytes (PBn) */
	BHDR_P_GetPacketRAM_isr(hRegister, PacketRegisterOffset, (uint8_t *) Packet->DataBytes) ;

#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
done:
#endif
	BDBG_LEAVE(BHDR_P_GetPacketData_isr) ;
	return rc ;
}


/******************************************************************************
Summary:
	Copy Packet Data
*******************************************************************************/
BERR_Code BHDR_GetPacketData(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_Packet *Packet
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDR_GetPacketData) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_EnterCriticalSection() ;
		rc = BHDR_P_GetPacketData_isr(hHDR, PacketNum, Packet) ;
	BKNI_LeaveCriticalSection() ;

	return rc ;
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ProcessReceivedPackets_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t LockedPackets  ;
	uint32_t Register ;
#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
	uint8_t AvMute ;
#endif
	uint32_t i ;
	uint8_t checksum ;

	BDBG_ENTER(BHDR_P_ProcessReceivedPackets_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	LockedPackets = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_STATUS + ulOffset) ;
	for (i = 0 ; i < BHDR_P_NUM_PACKETS; i++)
	{
		/* process only packets that are listed as active in the RAM Packet Map */
		if (RamPacketMap[i].ePacketType == BAVC_HDMI_PacketType_eUnused)
			continue ;

		if ((1 << i) & LockedPackets)
		{
			BHDR_P_GetPacketData_isr(hHDR, i, &hHDR->RamPacket) ;

#if BDBG_DEBUG_BUILD
			/* debug support to display received packets -- skip reporting GCP and Clock Recovery Packets; too many! */
			if ((hHDR->RamPacket.Type != BAVC_HDMI_PacketType_eAudioClockRegeneration)
			&& (hHDR->RamPacket.Type != BAVC_HDMI_PacketType_eGeneralControl)
			&& (hHDR->RamPacket.Type != BAVC_HDMI_PacketType_eNull))
			{
				BDBG_MSG(("RAM_PACKET_%02d received '%s' Packet (0x%02x)",
					i, BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
					hHDR->RamPacket.Type)) ;
			}
#endif

			if (hHDR->RamPacket.Type > BAVC_HDMI_INFOFRAME_PACKET_TYPE)
			{
				BHDR_P_CalculateInfoframeChecksum_isr(&hHDR->RamPacket, &checksum) ;
				if (checksum)  /* there's an error in the packet */
				{
					BDBG_WRN(("CH%d '%s' Packet (0x%02x) has checksum error",
						hHDR->eCoreId,
						BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
						hHDR->RamPacket.Type)) ;

					/*
					ClearLockedPacket may not be needed since the packet
					already contains an error; a clean error-free packet will
					overwrite once the lock is released

					in any case this is a rare occurence

					*/
					BHDR_P_ClearPacketRAMLock_isr(hHDR, i) ;

					continue ;
				}
			}

			switch (hHDR->RamPacket.Type)
			{
			case BAVC_HDMI_PacketType_eNull :
				/* do nothing */
				break ;

			case BAVC_HDMI_PacketType_eSpdInfoFrame    :

#if BHDR_CONFIG_DEBUG_INFO_PACKET_SPD
				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
#endif
				BHDR_P_ParseSPDInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->SPDInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->SPDInfoFrame) ;
				}
				break ;

			case BAVC_HDMI_PacketType_eVendorSpecificInfoframe :

#if BHDR_CONFIG_DEBUG_INFO_PACKET_VENDOR_SPECIFIC
				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
#endif
				rc = BHDR_P_ParseVendorSpecificInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;
				if (rc) /* error in packet; do not callback with UNKNOWN VSI packet */
					break  ;

				hHDR->VSInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->VSInfoFrame) ;
				}
				break ;



			case BAVC_HDMI_PacketType_eAviInfoFrame :

#if BHDR_CONFIG_DEBUG_INFO_PACKET_AVI
				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
#endif
				BHDR_P_ParseAviInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AviInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->AviInfoFrame) ;
				}
				else
				{
					BDBG_WRN(("CH%d NO HDMI Packet Change Callback installed!!!",
						hHDR->eCoreId)) ;
				}

				break ;

			case BAVC_HDMI_PacketType_eAudioInfoFrame :

#if BHDR_CONFIG_DEBUG_INFO_PACKET_AUDIO
				BHDR_P_DEBUG_DumpPacketRam_isr(hHDR, i, &hHDR->RamPacket) ;
#endif
				BHDR_P_ParseAudioInfoFrameData_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AudioInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->AudioInfoFrame) ;
				}

				break ;

			case BAVC_HDMI_PacketType_eGeneralControl :
				/* AvMute in the GCP is handled at Isr time */

#if BHDR_CONFIG_CLEAR_AVMUTE_AFTER_N_S
				/* when muted check for continued GCPs every 1s */
				if (hHDR->AvMute)
				{
					Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_MISC_STATUS + ulOffset) ;
					AvMute = (uint8_t) BCHP_GET_FIELD_DATA(Register,
						HDMI_RX_0_MISC_STATUS, PKT_PROCESSOR_AV_MUTE) ;

					/* GCP Packets with AvMute set are still coming; restart our timer */
					BTMR_ReadTimer_isr(hHDR->timerGeneric, &hHDR->AvMuteTimeStamp) ;

					BDBG_MSG(("GCPs with Set_AVMUTE are still active; Restart auto Clear_AVMUTE timer...")) ;
				}
#endif

				BHDR_P_ParseGeneralControlPacket_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->GeneralControlPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;
				break ;

			case BAVC_HDMI_PacketType_eAudioClockRegeneration :
				BHDR_P_ParseAudioClockRegeneration_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AudioClockRegenerationPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				break ;

			case BAVC_HDMI_PacketType_eAudioContentProtection :
				BHDR_P_ParseAudioContentProtection_isr(hHDR, &hHDR->RamPacket) ;

				hHDR->AudioContentProtection.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->AudioContentProtection) ;
				}

				break ;

	 		case BAVC_HDMI_PacketType_eISRC1                :
			case BAVC_HDMI_PacketType_eISRC2                :
#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
				if (hHDR->RamPacket.Type == BAVC_HDMI_PacketType_eISRC1)
					BHDR_P_ParseISRC1_isr(hHDR, &hHDR->RamPacket) ;
				else
					BHDR_P_ParseISRC2_isr(hHDR, &hHDR->RamPacket) ;


				/* callbacks for ISRC packet change are handled in the parse function */
				hHDR->ISRC.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;
#else
				BDBG_WRN(("****")) ;
				BDBG_WRN(("Support for ISRC packets is disabled; see bhdr_config.h to enable")) ;
				BDBG_WRN(("****")) ;
#endif
				break ;

			case BAVC_HDMI_PacketType_eGamutMetadataPacket :
#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
				hHDR->GamutPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eUpdated ;

				hHDR->GamutPacket.Type = hHDR->RamPacket.Type ;
				BHDR_P_ParseGamutMetadataPacket_isr(hHDR, &hHDR->GamutPacket) ;

				if (hHDR->pfPacketChangeCallback)
				{
					hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
						hHDR->RamPacket.Type, &hHDR->GamutPacket) ;
				}

				break ;
#else
				/* fall through */
#endif

			case BAVC_HDMI_PacketType_eMpegInfoFrame   :
			default :
				BDBG_WRN(("   CH%d No handler for (%s) Packet (0x%02x)",
					hHDR->eCoreId,
					BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
					hHDR->RamPacket.Type)) ;
				break ;

			}

			/* enable packet stop detection  */
			if (RamPacketMap[i].bMonitorStoppedPackets)
			{
				BDBG_MSG(("RAM_PACKET_%02d enabled  '%s' Packet (0x%02x) Stop Detection",
					i, BAVC_HDMI_PacketTypeToStr_isrsafe(hHDR->RamPacket.Type),
					hHDR->RamPacket.Type)) ;

				/* clear the packet stop detect status */
				Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_CLEAR_DETECT + ulOffset) ;
				Register |= (uint32_t) 1 << i ;       /* write 1 */
				BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_CLEAR_DETECT + ulOffset, Register) ;
				Register &= ~((uint32_t) 1 << i) ;  /* write 0 */
				BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_CLEAR_DETECT + ulOffset, Register) ;

				/* Enable Packet Stop Detection in case the packet stops */
				Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT + ulOffset) ;
				Register |= (uint32_t) 1 << i ;
				BREG_Write32(hRegister, BCHP_HDMI_RX_0_ENABLE_PACKET_STOP_DETECT  + ulOffset, Register) ;
			}

			/* now that the packet is processed; clear its lock so we get the next change */
			BHDR_P_ClearPacketRAMLock_isr(hHDR, i) ;
		}
	}

	/* always return success */
	BDBG_LEAVE(BHDR_P_ProcessReceivedPackets_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_P_ProcessStoppedPackets_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset  ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t StoppedPackets  ;
	uint32_t PacketNum ;
	uint32_t Register ;

	int ePacketType ;
	void *pvPacketData ;


	BDBG_ENTER(BHDR_P_ProcessStoppedPackets_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	StoppedPackets = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_STOP_DETECT_STATUS + ulOffset) ;
	for (PacketNum = 0 ; PacketNum < BHDR_P_NUM_PACKETS; PacketNum++)
	{
		/* check if monitoring was enabled on the this stopped packet */
		if (!RamPacketMap[PacketNum].bMonitorStoppedPackets)
			continue ;

		/* process only packets that are listed as active in the RAM Packet Map */
		if ((RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eUnused)
		|| (RamPacketMap[PacketNum].ePacketType == BAVC_HDMI_PacketType_eNull))
			continue ;

		/* skip packets which are still coming */
		if (!( ( (uint32_t) 1 << PacketNum) & StoppedPackets))
			continue ;

		ePacketType = (int) RamPacketMap[PacketNum].ePacketType ;
		BDBG_WRN(("CH%d RAM_PACKET_%02d has STOPPED receiving '%s' Packets (0x%02x)",
			hHDR->eCoreId,
			PacketNum, BAVC_HDMI_PacketTypeToStr_isrsafe(ePacketType), ePacketType)) ;

		pvPacketData = NULL ;
		/* handle stopped packets appropriately */
		switch (ePacketType)
		{
		case BAVC_HDMI_PacketType_eAviInfoFrame :
			hHDR->AviInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AviInfoFrame;

			/* call  the callback functions for Format Change notification  */
			if (hHDR->pfVideoFormatChangeCallback)
			{
				hHDR->pfVideoFormatChangeCallback(hHDR->pvVideoFormatChangeParm1,
					BAVC_HDMI_PacketType_eAviInfoFrame, &hHDR->AviInfoFrame) ;
			}
			break ;

		case BAVC_HDMI_PacketType_eAudioInfoFrame :
			hHDR->AudioInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AudioInfoFrame ;
			break ;

		case BAVC_HDMI_PacketType_eSpdInfoFrame       :
			hHDR->SPDInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->SPDInfoFrame ;
			break ;


		case BAVC_HDMI_PacketType_eVendorSpecificInfoframe:
			hHDR->VSInfoFrame.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			hHDR->VSInfoFrame.eHdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;
			pvPacketData = &hHDR->VSInfoFrame ;

			break ;

		/* loss of audio ??? */
		case BAVC_HDMI_PacketType_eAudioClockRegeneration :
			hHDR->AudioClockRegenerationPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AudioClockRegenerationPacket ;
			break ;


		/* Audio Not Protected */
		case BAVC_HDMI_PacketType_eAudioContentProtection :
			hHDR->AudioContentProtection.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->AudioContentProtection ;
			break ;


		case BAVC_HDMI_PacketType_eAudioSample          :
			/* Audio packets handled by hw */
			continue ;
			break ;

		case BAVC_HDMI_PacketType_eGeneralControl       :
			hHDR->GeneralControlPacket.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;
			pvPacketData = &hHDR->GeneralControlPacket ;
			BDBG_WRN(("CH%d General Control Packet stopped...  ", hHDR->eCoreId)) ;

			/* reset the DC count threshold to zero */
			Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset) ;
			Register &= ~BCHP_MASK(HDMI_RX_0_HDMI_13_FEATURES_CFG_1, GCP_CD_FRAME_COUNT_THRESHOLD) ;
			BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDMI_13_FEATURES_CFG_1 + ulOffset,  Register) ;

			break ;

		case BAVC_HDMI_PacketType_eISRC1                :
		case BAVC_HDMI_PacketType_eISRC2                :
#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
			hHDR->ISRC.ePacketStatus = BAVC_HDMI_PacketStatus_eStopped ;

			/* reset ISRC1_PacketReceived; it is no longer valid */
			hHDR->ISRC.ISRC1_PacketReceived = false ;
			/* call  the ISRC callback functions for indicate packet has stopped */
			if (hHDR->pfISRCChangeCallback)
			{
				hHDR->pfISRCChangeCallback(hHDR->pvISRCChangeParm1,
					BAVC_HDMI_PacketType_eISRC1, &hHDR->ISRC) ;
			}
#endif
			break ;


		case BAVC_HDMI_PacketType_eDirectStream      :
		case BAVC_HDMI_PacketType_eGamutMetadataPacket   :
		case BAVC_HDMI_PacketType_eMpegInfoFrame   :
			BDBG_WRN(("CH%d HDMI '%s' Packet (%#02x) is not supported",
				hHDR->eCoreId,
				BAVC_HDMI_PacketTypeToStr_isrsafe(ePacketType), ePacketType)) ;
			continue ;
			break ;

		default :
			BDBG_WRN(("CH%d Unknown Packet Type %#02x Stopped",
				hHDR->eCoreId, ePacketType)) ;
			continue ;
		}

		if (hHDR->pfPacketChangeCallback && pvPacketData)
		{
			hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
				ePacketType, pvPacketData) ;
		}

		/* clear Packet RAM in case the same packet starts transmitting again */
		/* we will then be notified of the resumption of the packet */

		BHDR_P_ClearPacketRAM_isr(hHDR, PacketNum) ;
		BHDR_P_ClearPacketRAMLock_isr(hHDR, PacketNum) ;

	}

	BDBG_LEAVE(BHDR_P_ProcessStoppedPackets_isr) ;
	return rc ;
}



/******************************************************************************
Summary:
Parse General Control Packet (GCP) data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseGeneralControlPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	uint8_t temp ;
	BAVC_HDMI_GcpData stNewGcpPacket ;

	BDBG_ENTER(BHDR_P_ParseGeneralControlPacket_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset(&stNewGcpPacket, 0, sizeof(BAVC_HDMI_GcpData)) ;

	/* Keep a raw copy of the HDMI Packet structure  */
	BKNI_Memcpy(&stNewGcpPacket.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;


	/* check for SetAvMute */
	temp = Packet->DataBytes[0] ;

#if BHDR_CONFIG_DEBUG_GCP_DC || BHDR_CONFIG_DEBUG_GCP_AV_MUTE
	BDBG_LOG(("-------------------- START PARSE GCP PACKET----------------------")) ;
#endif

	if ((temp) && (temp != hHDR->GeneralControlPacket.uiMuteStatus))  /* AvMute Change */
	{
		stNewGcpPacket.uiMuteStatus = (uint32_t) temp ;

		/* check for SetAvMute */
		temp = Packet->DataBytes[0] & 0x01 ;  /* 0x01 = SetAvMute  */
		stNewGcpPacket.SetAvMute = (uint32_t) temp ;

		/* check for ClearAvMute */
		temp = Packet->DataBytes[0] & 0x10 ; /* 0x10 = ClearAvMute */
		stNewGcpPacket.ClearAvMute = (uint32_t) temp ;

#if BHDR_CONFIG_DEBUG_GCP_AV_MUTE
		/* Either Clear_AvMute or Set_AvMute should be set but not both */
		if (stNewGcpPacket.SetAvMute)
		{
			BDBG_WRN(("CH%d Set_AVMUTE (%#02x) is set; PB0: %#02x",
				hHDR->eCoreId, stNewGcpPacket.SetAvMute, Packet->DataBytes[0])) ;
		}

		if (stNewGcpPacket.ClearAvMute)
		{
			BDBG_WRN(("CH%d Clear_AVMUTE (%#02x) is set; PB0: %#02x",
				hHDR->eCoreId, stNewGcpPacket.ClearAvMute, Packet->DataBytes[0])) ;
		}
#endif
	}
	else if (!temp)  /* no AvMute information */
	{
#if BHDR_CONFIG_DEBUG_GCP_AV_MUTE
		BDBG_WRN(("CH%d No AvMute info in current GCP Packet; PB0: %#02x",
				hHDR->eCoreId, Packet->DataBytes[0])) ;
#endif
		/* no mute information in current GCP packet; keep current setting*/
		stNewGcpPacket.uiMuteStatus = hHDR->GeneralControlPacket.uiMuteStatus ;
	}

	/* check for color depth info */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x0F ;

	/* Ignore GCP Packets that do not contain Color Depth Info  */
	/* hardware handles Mute/Unmute data  */

	stNewGcpPacket.eColorDepth = (uint32_t) temp ;


	temp = Packet->DataBytes[1] ;
	temp = temp & 0xF0 ;
	temp = temp >> 4 ;
	stNewGcpPacket.ePixelPacking = (uint32_t) temp ;

	temp = Packet->DataBytes[2] ;
	temp = temp & 0x01 ;
	stNewGcpPacket.DefaultPhase = (uint32_t) temp ;

	/* Ignore GCP if there is no difference in the Color Depth */
	if ((hHDR->bDeepColorMode) && (!stNewGcpPacket.eColorDepth))  /* packet with no DC info; keep current DC */
	{
#if BHDR_CONFIG_DEBUG_GCP_DC
		/* informative message indicating GCP with no DC message;  which is legal */
		BDBG_LOG(("In %s DC Mode; Received GCP with no DC information",
			BAVC_HDMI_BitsPerPixelToStr(hHDR->GeneralControlPacket.eColorDepth))) ;
#endif
		stNewGcpPacket.eColorDepth = hHDR->GeneralControlPacket.eColorDepth ;
		goto done ;
	}

	if ((!hHDR->bDeepColorMode)
	&& (   (stNewGcpPacket.eColorDepth == BAVC_HDMI_GCP_ColorDepth_e24bpp)
		|| (stNewGcpPacket.eColorDepth == BAVC_HDMI_GCP_ColorDepth_eDepthNotIndicated)))
	{
		stNewGcpPacket.eColorDepth = (uint32_t) BAVC_HDMI_GCP_ColorDepth_e24bpp ;
	}

	if (hHDR->GeneralControlPacket.eColorDepth == stNewGcpPacket.eColorDepth)
		goto done ; /* no color depth change */


	switch (stNewGcpPacket.eColorDepth)
	{
	case BAVC_HDMI_GCP_ColorDepth_eDepthNotIndicated :
	case BAVC_HDMI_GCP_ColorDepth_e24bpp :
		BDBG_MSG(("CH%d Color Depth: 24 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = false ;
		/* Pre HDMI 1.3 GCP; no Deep Color Info specified (default 24Bit) ...*/
		goto done ;
		break ;

	case BAVC_HDMI_GCP_ColorDepth_e30bpp :
		BDBG_MSG(("CH%d Color Depth: 30 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = true ;
		break ;

	case BAVC_HDMI_GCP_ColorDepth_e36bpp :
		BDBG_MSG(("CH%d Color Depth: 36 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = true ;
		break ;

	case BAVC_HDMI_GCP_ColorDepth_e48bpp :
		BDBG_MSG(("CH%d Color Depth: 48 bpp", hHDR->eCoreId)) ;
		hHDR->bDeepColorMode = true ;
		break ;

	default :
		BDBG_MSG(("CH%d Incorrectly Specified Color Depth: %#x",
			hHDR->eCoreId, hHDR->GeneralControlPacket.eColorDepth)) ;
	}


#if BHDR_CONFIG_DEBUG_GCP_DC
	BDBG_LOG(("CH%d Pixel Packing Phase: %#x", hHDR->eCoreId, hHDR->GeneralControlPacket.ePixelPacking)) ;
	switch (hHDR->GeneralControlPacket.ePixelPacking)
	{

	case BAVC_HDMI_GCP_PixelPacking_ePhase4_10P4 :
		BDBG_LOG(("\tCH%d Phase4 Pixel Packing (10P4)", hHDR->eCoreId)) ;
		break ;

	case BAVC_HDMI_GCP_PixelPacking_ePhase1_10P1_12P1_16P1 :
		BDBG_LOG(("\tCH%d Phase1 Pixel Packing (10P1 12P1 16P1)", hHDR->eCoreId)) ;
		break ;

	case BAVC_HDMI_GCP_PixelPacking_ePhase2_10P2_12P2 :
		BDBG_LOG(("\tCH%d Phase2 Pixel Packing (10P2 12P2)", hHDR->eCoreId)) ;
		break ;

	case BAVC_HDMI_GCP_PixelPacking_ePhase3_10P3 :
		BDBG_LOG(("\tCH%d Phase3 Pixel Packing (10P2)", hHDR->eCoreId)) ;
		break ;

	default :
		BDBG_WRN(("CH%d Incorrectly Specified Pixel Packing Phase %#x",
			hHDR->eCoreId, hHDR->GeneralControlPacket.ePixelPacking)) ;
	}
#endif

done :
#if BHDR_CONFIG_DEBUG_GCP_DC || BHDR_CONFIG_DEBUG_GCP_AV_MUTE
	BDBG_LOG(("--------------------  END  PARSE GCP PACKET----------------------")) ;
#endif

	/* copy the new packet to the handle for use later */
	BKNI_Memcpy(&hHDR->GeneralControlPacket, &stNewGcpPacket, sizeof(BAVC_HDMI_GcpData)) ;

	BDBG_LEAVE(BHDR_P_ParseGeneralControlPacket_isr) ;

	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
Parse AVI Info Frame data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseAviInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	BERR_Code rc = BERR_SUCCESS ;
	BAVC_HDMI_AviInfoFrame stNewAviInfoFrame ;
	uint8_t temp ;

	BDBG_ENTER(BHDR_P_ParseAviInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;


	if (hHDR->DeviceSettings.bParseAVI == false)
	{
		/* return with just a raw copy of the HDMI Packet structure  */
		BKNI_Memcpy(&hHDR->AviInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;
		return rc ;
	}

	BKNI_Memset(&stNewAviInfoFrame, 0, sizeof(BAVC_HDMI_AviInfoFrame)) ;

	/* Keep a raw copy of the HDMI Packet structure  */
	BKNI_Memcpy(&stNewAviInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;


	/* parse all fields and store in the local HDMI structure */

 	/* AVI Infoframe Data Byte 1 */
	/* AviInfoFrame.ePixelEncoding */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x60 ;
	temp = temp >> 5 ;
	stNewAviInfoFrame.ePixelEncoding = temp ;

	/* AviInfoFrame.eActiveInfo*/
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x10 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.eActiveInfo = temp ;

	/* AviInfoFrame.eBarInfo */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x0C ;
	temp = temp >> 2 ;
	stNewAviInfoFrame.eBarInfo = temp ;

 	/* AviInfoFrame.eScanInfo */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x03 ;
	stNewAviInfoFrame.eScanInfo = temp ;


 	/* AVI Infoframe Data Byte 2 */
	/* AviInfoFrame.eColorimetry */
 	temp = Packet->DataBytes[2];
	temp = temp & 0xC0 ;
	temp = temp >> 6 ;
	stNewAviInfoFrame.eColorimetry = temp ;

	/* AviInfoFrame.ePictureAspectRatio */
 	temp = Packet->DataBytes[2];
	temp = temp & 0x30 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.ePictureAspectRatio = temp ;

	/* AviInfoFrame.eActiveFormatAspectRatio */
 	temp = Packet->DataBytes[2] ;
	temp = temp & 0x0F ;
	stNewAviInfoFrame.eActiveFormatAspectRatio = temp ;

 	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eITContent */
 	temp = Packet->DataBytes[3] ;
	temp = temp & 0x80 ;
	temp = temp >> 7 ;
	stNewAviInfoFrame.eITContent= temp ;

 	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eExtendedColorimetry */
 	temp = Packet->DataBytes[3] ;
	temp = temp & 0x70 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.eExtendedColorimetry= temp ;


 	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eRGBQuantizationRange */
 	temp = Packet->DataBytes[3] ;
	temp = temp & 0x0c ;
	temp = temp >> 2 ;
	stNewAviInfoFrame.eRGBQuantizationRange= temp ;

 	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eScaling */
 	temp = Packet->DataBytes[3] ;
	temp = temp & 0x03 ;
	stNewAviInfoFrame.eScaling = temp ;


 	/* AVI Infoframe Data Byte 4 */
	/* AviInfoFrame.VideoIdCode */
 	temp = Packet->DataBytes[4] ;
	temp = temp & 0x7F ;
	stNewAviInfoFrame.VideoIdCode = temp ;


 	/* AVI Infoframe Data Byte 5 */
	/* AviInfoFrame.PixelRepeat */
 	temp = Packet->DataBytes[5] ;
	temp = temp & 0x0F ;
	stNewAviInfoFrame.PixelRepeat = temp ;

 	/* AVI Infoframe Data Byte 5 */
	/* AviInfoFrame.eContentType */
 	temp = Packet->DataBytes[5] ;
	temp = temp & 0x30 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.eContentType = temp ;

 	/* AVI Infoframe Data Byte 5 */
	/* AviInfoFrame.eYccQuantizationRange */
 	temp = Packet->DataBytes[5] ;
	temp = temp & 0xC0 ;
	temp = temp >> 6 ;
	stNewAviInfoFrame.eYccQuantizationRange = temp ;

	if (hHDR->AviInfoFrame.eBarInfo != BAVC_HDMI_AviInfoFrame_BarInfo_eInvalid)
	{
		/* get bar info */
		stNewAviInfoFrame.TopBarEndLineNumber =
			(uint16_t) ((Packet->DataBytes[7] << 8) | Packet->DataBytes[6]) ;

		stNewAviInfoFrame.BottomBarStartLineNumber =
			(uint16_t) ((Packet->DataBytes[9] << 8) | Packet->DataBytes[8]) ;

		stNewAviInfoFrame.LeftBarEndPixelNumber =
			(uint16_t) ((Packet->DataBytes[11] << 8) | Packet->DataBytes[10]) ;

		stNewAviInfoFrame.RightBarEndPixelNumber =
			(uint16_t) ((Packet->DataBytes[13] << 8) | Packet->DataBytes[12]) ;
	}


#if BHDR_CONFIG_DEBUG_INFO_PACKET_AVI
	BHDR_P_DEBUG_AviInfoFrame(&hHDR->AviInfoFrame, &stNewAviInfoFrame) ;
#endif

	/* copy the new packet */
	BKNI_Memcpy(&hHDR->AviInfoFrame, &stNewAviInfoFrame, sizeof(BAVC_HDMI_AviInfoFrame)) ;

	/* call  the callback functions for Format Change notification  */
	if (hHDR->pfVideoFormatChangeCallback)
	{
		hHDR->pfVideoFormatChangeCallback(hHDR->pvVideoFormatChangeParm1,
			hHDR->iVideoFormatChangeParm2, &hHDR->AviInfoFrame) ;
	}

	BDBG_LEAVE(BHDR_P_ParseAviInfoFrameData_isr) ;
	return rc ;
}


/******************************************************************************
Summary:
Parse Audio Info Frame data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseAudioInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	uint8_t temp ;
	BAVC_HDMI_AudioInfoFrame stNewAudioInfoFrame ;

	BDBG_ENTER(BHDR_P_ParseAudioInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* zero out the declared Audio Infoframe Structure */
	BKNI_Memset(&stNewAudioInfoFrame, 0, sizeof(stNewAudioInfoFrame)) ;

	/* keep a copy of the raw HDMI Packet structure */
	BKNI_Memcpy(&stNewAudioInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet));

	/* parse the various fields in the packet */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x07 ;
	stNewAudioInfoFrame.ChannelCount = temp ;

	temp = Packet->DataBytes[1] ;
	temp = temp & 0xF0 ;
	temp = temp >> 4 ;
	stNewAudioInfoFrame.CodingType = temp ;


	temp = Packet->DataBytes[2] ;
	temp = temp & 0x03,
	 stNewAudioInfoFrame.SampleSize = temp ;

	temp = Packet->DataBytes[2] ;
	temp = temp & 0x1C ;
	temp = temp >> 2 ;
	stNewAudioInfoFrame.SampleFrequency = temp ;


	temp = Packet->DataBytes[4] ;
	temp = temp & 0x1F ;
	stNewAudioInfoFrame.SpeakerAllocation = temp ;


	temp = Packet->DataBytes[5] ;
	temp = temp & 0x78 ;
	temp = temp >> 3 ;
	stNewAudioInfoFrame.LevelShift = temp ;

	temp = Packet->DataBytes[5] ;
	temp = temp & 0x80 ;
	temp = temp >> 7 ;
	stNewAudioInfoFrame.DownMixInhibit = temp ;

#if BHDR_CONFIG_DEBUG_INFO_PACKET_AUDIO
	BHDR_P_DEBUG_AudioInfoFrame(&hHDR->AudioInfoFrame, &stNewAudioInfoFrame) ;
#endif

	/* copy the new packet to the handle for use later */
	BKNI_Memcpy(&hHDR->AudioInfoFrame, &stNewAudioInfoFrame, sizeof(BAVC_HDMI_AudioInfoFrame)) ;

	/* make sure MAI bus reflects any updated channel count info */
	BHDR_P_ConfigureAudioMaiBus_isr(hHDR) ;

	/* call  the callback functions for Audio Change notification  */
	if (hHDR->pfAudioFormatChangeCallback)
	{
		hHDR->pfAudioFormatChangeCallback(hHDR->pvAudioFormatChangeParm1,
			hHDR->iAudioFormatChangeParm2, &hHDR->AudioData);
	}

	BDBG_LEAVE(BHDR_P_ParseAudioInfoFrameData_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
Parse SPD Info Frame data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseSPDInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	BAVC_HDMI_SPDInfoFrame stNewSpdInfoFrame ;

	BDBG_ENTER(BHDR_P_ParseSPDInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset(&stNewSpdInfoFrame, 0, sizeof(BAVC_HDMI_SPDInfoFrame)) ;

	/* Keep a raw copy of the SPD packet  */
	BKNI_Memcpy(&stNewSpdInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;


	BKNI_Memcpy(stNewSpdInfoFrame.VendorName,
		&Packet->DataBytes[BAVC_HDMI_SPD_IF_VENDOR_OFFSET],
		BAVC_HDMI_SPD_IF_VENDOR_LEN) ;

	stNewSpdInfoFrame.VendorName[BAVC_HDMI_SPD_IF_VENDOR_LEN] = '\0' ;

	BKNI_Memcpy(stNewSpdInfoFrame.ProductDescription,
		&Packet->DataBytes[BAVC_HDMI_SPD_IF_DESC_OFFSET],
		BAVC_HDMI_SPD_IF_DESC_LEN) ;

	stNewSpdInfoFrame.ProductDescription[BAVC_HDMI_SPD_IF_DESC_LEN] = '\0' ;

	stNewSpdInfoFrame.SourceDeviceInfo =
		Packet->DataBytes[BAVC_HDMI_SPD_IF_DEVICE_INFO_OFFSET] ;

#if BHDR_CONFIG_DEBUG_INFO_PACKET_SPD
	BHDR_P_DEBUG_SpdInfoFrame(&hHDR->SPDInfoFrame, &stNewSpdInfoFrame) ;
#endif

	/* copy the new packet struct data */
	BKNI_Memcpy(&hHDR->SPDInfoFrame, &stNewSpdInfoFrame, sizeof(BAVC_HDMI_SPDInfoFrame)) ;


	BDBG_LEAVE(BHDR_P_ParseSPDInfoFrameData_isr) ;
	return BERR_SUCCESS ;
}


static void BHDR_P_ParseVSI_0xA05DE7_isr(BAVC_HDMI_VendorSpecificInfoFrame *stNewVsInfoFrame , BAVC_HDMI_Packet *Packet)
{
	/* parse the various fields in the packet */

	uint8_t temp ;
	/* HDMI Video Format */
	temp = Packet->DataBytes[4] ;
	temp = temp & 0xF0 ;
	temp = temp >> 4 ;
	stNewVsInfoFrame->eHdmiVideoFormat = temp ;

	BDBG_WRN(("VideoFormat: %d", stNewVsInfoFrame->eHdmiVideoFormat)) ;

	/* HDMI 3D Structure */
	temp = Packet->DataBytes[5] ;
	temp = temp & 0xF0 ;
	temp = temp >> 4 ;
	stNewVsInfoFrame->e3DStructure = temp ;
	BDBG_WRN(("3D Struct: %d", stNewVsInfoFrame->e3DStructure)) ;

	/* get the 3D Ext Data regardless if we know it is valid or not */
	temp = Packet->DataBytes[6] ;
	temp = temp & 0xF0 ;
	temp = temp >> 4 ;
	stNewVsInfoFrame->e3DExtData = temp ;


}


BERR_Code BHDR_P_ParseVendorSpecificInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{
	uint8_t temp ;
	static const uint8_t HDMI_IEEE_0x00C003[] = {0x03, 0x0C, 0x00} ;
	static const uint8_t HDMI_IEEE_0xA05DE7[] = {0xE7, 0x5D, 0xA0} ;

	BAVC_HDMI_VendorSpecificInfoFrame stNewVsInfoFrame ;

	BDBG_ENTER(BHDR_P_ParseVendorSpecificInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	BKNI_Memset(&stNewVsInfoFrame, 0, sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	/* Keep a raw copy of the SPD packet  */
	BKNI_Memcpy(&stNewVsInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;


	/* copy the IEEE Reg ID value */
	BKNI_Memcpy_isr(&stNewVsInfoFrame.uIEEE_RegId,
		&Packet->DataBytes[ BAVC_HDMI_VS_IEEE_REGID_OFFSET],
		BAVC_HDMI_IEEE_REGID_LEN) ;

	if (BKNI_Memcmp(&stNewVsInfoFrame.uIEEE_RegId, HDMI_IEEE_0x00C003,
		BAVC_HDMI_IEEE_REGID_LEN))
	{
		BDBG_MSG(("Proprietary IEEE RegId: <0x%02X%02X%02X>",
			stNewVsInfoFrame.uIEEE_RegId[2], stNewVsInfoFrame.uIEEE_RegId[1],
			stNewVsInfoFrame.uIEEE_RegId[0])) ;


		if (!BKNI_Memcmp(&stNewVsInfoFrame.uIEEE_RegId, &HDMI_IEEE_0xA05DE7,
			BAVC_HDMI_IEEE_REGID_LEN))
		{
			BHDR_P_ParseVSI_0xA05DE7_isr(&stNewVsInfoFrame, Packet) ;
			goto done ;
		}
		else
		{
			BDBG_MSG(("IEEE RegId: <0x%02x%02X%02X> is UNSUPPORTED",
				stNewVsInfoFrame.uIEEE_RegId[2], stNewVsInfoFrame.uIEEE_RegId[1],
				stNewVsInfoFrame.uIEEE_RegId[0])) ;
			return BERR_UNKNOWN ;
		}
	}

	/* parse the various fields in the packet */

	/* HDMI Video Format */
	temp = Packet->DataBytes[4] ;
	temp = temp & 0xE0 ;
	temp = temp >> 5 ;
	stNewVsInfoFrame.eHdmiVideoFormat = temp ;

	switch(stNewVsInfoFrame.eHdmiVideoFormat)
	{
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution :
	/* HDMI VIC */
		temp = Packet->DataBytes[5] ;
		stNewVsInfoFrame.eHdmiVic = temp ;
		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat :
	/* OR  HDMI 3D Structure */
		temp = Packet->DataBytes[5] ;
		temp = temp & 0xF0 ;
		temp = temp >> 4 ;
		stNewVsInfoFrame.e3DStructure = temp ;

		/* get the 3D Ext Data regardless if we know it is valid or not */
		temp = Packet->DataBytes[6] ;
		temp = temp & 0xF0 ;
		temp = temp >> 4 ;
		stNewVsInfoFrame.e3DExtData = temp ;

		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone :
		/* do nothing */
		break ;

	default :
		BDBG_WRN(("VSI HDMI Video Format Reserved (%d)",
			stNewVsInfoFrame.eHdmiVideoFormat)) ;
	}

done:
#if BHDR_CONFIG_DEBUG_INFO_PACKET_VENDOR_SPECIFIC
	BHDR_P_DEBUG_VsInfoFrame(& hHDR->VSInfoFrame, &stNewVsInfoFrame) ;
#endif

	/* copy the new packet to the handle for use later */
	BKNI_Memcpy(&hHDR->VSInfoFrame, &stNewVsInfoFrame,
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	BDBG_LEAVE(BHDR_P_ParseVendorSpecificInfoFrameData_isr) ;

	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
Parse Audio Content Protection  from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseAudioContentProtection_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseAudioContentProtection_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Firstly don't  parse all fields but store in the local HDMI structure */
	BKNI_Memcpy(&hHDR->AudioContentProtection.stPacket, Packet, sizeof(BAVC_HDMI_Packet));


	hHDR->AudioContentProtection.ACP_Type = Packet->Version;
	for (i =0; i<BAVC_HDMI_PB_LENGTH; i++)
	{
		hHDR->AudioContentProtection.ACP_Type_Dependent[i] = Packet->DataBytes[i] ;
	}

	/* call  the callback functions for Audio Content Protection Change notification  */
	if (hHDR->pfAudioContentProtectionChangeCallback)
	{
		hHDR->pfAudioContentProtectionChangeCallback(
			hHDR->pvAudioContentProtectionChangeParm1,
			hHDR->iAudioContentProtectionChangeParm2,
			&hHDR->AudioContentProtection);
	}

	BDBG_LEAVE(BHDR_P_ParseAudioContentProtection_isr) ;
	return BERR_SUCCESS ;
}


#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
/******************************************************************************
Summary:
Parse ISRC1 from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseISRC1_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseISRC1_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Firstly don't  parse all fields but store in the local HDMI structure */
	BKNI_Memcpy(&hHDR->ISRC.stISRC1_Packet, Packet, sizeof(BAVC_HDMI_Packet));

	hHDR->ISRC.ISRC1_PacketReceived = true  ;

	hHDR->ISRC.ISRC_Cont = Packet->Version & 0x80 ;
	hHDR->ISRC.ISRC_Valid = Packet->Version & 0x40 ;
	hHDR->ISRC.ISRC_Status = Packet->Version & 0x07 ;

	for (i =0; i < BAVC_HDMI_PB_LENGTH; i++)
	{
		hHDR->ISRC.ISRC_UPC_EAN[i] = Packet->DataBytes[i] ;
	}

	/* call callback if there is no ISRC2 packet, otherwise wait for the Recd ISRC2 packet to call the cb */
	if (!hHDR->ISRC.ISRC_Cont)
	{
		/* call  the callback functions for ISRC Change notification  */
		if (hHDR->pfISRCChangeCallback)
		{
			hHDR->pfISRCChangeCallback(hHDR->pvISRCChangeParm1,
				hHDR->iISRCChangeParm2, &hHDR->ISRC);
		}
	}

	BDBG_LEAVE(BHDR_P_ParseISRC1_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
Parse ISRC2 from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseISRC2_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseISRC2_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Store a copy of the packet first before parsing the fields  */
	BKNI_Memcpy(&hHDR->ISRC.stISRC2_Packet, Packet, sizeof(BAVC_HDMI_Packet));


	/* there is no Header data in ISRC2; only packet byte data */

	for (i =0; i < BAVC_HDMI_PB_LENGTH; i++)
	{
		hHDR->ISRC.ISRC_UPC_EAN[BAVC_HDMI_IRSC2_PACKET_OFFSET + i]
			= Packet->DataBytes[i] ;
	}


	/* always call  the callback function for ISRC Change notification  when 2nd part of ISRC packet is sent */
	if (hHDR->pfISRCChangeCallback)
	{
		hHDR->pfISRCChangeCallback(hHDR->pvISRCChangeParm1,
			hHDR->iISRCChangeParm2, &hHDR->ISRC);
	}

	BDBG_LEAVE(BHDR_P_ParseISRC2_isr) ;
	return BERR_SUCCESS ;
}
#endif

BERR_Code BHDR_P_ParseAudioClockRegeneration_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

#if BHDR_CONFIG_DEBUG_ACR_PACKET
	BAVC_HDMI_AudioClockRegenerationPacket stNewAudioClockRegenerationPacket ;
#endif
	uint32_t N, CTS ;

	BDBG_ENTER(BHDR_P_ParseAudioClockRegeneration_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Keep a raw copy of the HDMI Packet structure  */
	BKNI_Memcpy(&hHDR->AudioClockRegenerationPacket.stPacket, Packet,
		sizeof(BAVC_HDMI_Packet)) ;

	/* display modified N and CTS values for debug purposes */
	CTS =
		  (uint32_t) ((Packet->DataBytes[1] & 0xF) << 16)
		|(uint32_t) ((Packet->DataBytes[2] << 8))
		|(uint32_t) ((Packet->DataBytes[3])) ;

	N =
		  (uint32_t) ((Packet->DataBytes[4] & 0xF) << 16)
		|(uint32_t) ((Packet->DataBytes[5] << 8))
		|(uint32_t) ((Packet->DataBytes[6])) ;


	/* For audio sample freq, CTS and N from packet are more exact than the value from register. */
	if ((hHDR->AudioClockRegenerationPacket.N != N)
	|| (hHDR->AudioClockRegenerationPacket.CTS > (CTS + 20))
	|| ((hHDR->AudioClockRegenerationPacket.CTS + 20) < CTS))
	{
#if BHDR_CONFIG_DEBUG_ACR_PACKET
		BDBG_MSG(("RAM_PACKET_%02d received '%s' Packet (0x%02x)",
			ACR_PACKET_LOCATION, BAVC_HDMI_PacketTypeToStr_isrsafe(Packet->Type),
			Packet->Type)) ;

		stNewAudioClockRegenerationPacket.CTS = CTS ;
		stNewAudioClockRegenerationPacket.N = N ;
		/* always report significant changes in audio clock recovery packet */
		BHDR_P_DEBUG_AcrPacket(
			&hHDR->AudioClockRegenerationPacket,
			&stNewAudioClockRegenerationPacket) ;
#endif

		hHDR->AudioClockRegenerationPacket.CTS = CTS ;
		hHDR->AudioClockRegenerationPacket.N = N ;
	}

	BDBG_LEAVE(BHDR_P_ParseAudioClockRegeneration_isr) ;
	return BERR_SUCCESS ;
}


#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
BERR_Code BHDR_P_GetGamutPacketData_isr(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_GamutPacket *Packet )
{
	BREG_Handle hRegister ;
	uint32_t ulOffset ;

	uint32_t Register ;
	uint32_t PacketRegisterOffset ;

	BDBG_ENTER(BHDR_P_GetGamutPacketData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister= hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	/* calculate the offset of where the Packet RAM begins */
	PacketRegisterOffset = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER + ulOffset
		+ BHDR_P_PACKET_WORDS * 4 * PacketNum ;

	/* read the Header Bytes (HBn) first */

	Register = BREG_Read32(hRegister, PacketRegisterOffset) ;

	hHDR->GamutPacket.NextField               = ((Register & 0x00008000) >> 15) ;
	hHDR->GamutPacket.Profile                 = ((Register & 0x00007000) >> 12) ;
	hHDR->GamutPacket.AffectedGamutSeqNumber  = ((Register & 0x00000F00) >>  8) ;
	hHDR->GamutPacket.NoCurrentGamut          = ((Register & 0x00800000) >> 23) ;
	hHDR->GamutPacket.PacketSeq               = ((Register & 0x00300000) >> 20) ;
	hHDR->GamutPacket.CurrentGamutSeqNumber   = ((Register & 0x000F0000) >> 16) ;

	/* advance to the next packet register */
	PacketRegisterOffset +=  4 ;

	/* now read the Packet Bytes (PBn) */
	BHDR_P_GetPacketRAM_isr(hRegister, PacketRegisterOffset, (uint8_t *) Packet->DataBytes) ;

#if BHDR_CONFIG_DEBUG_GAMUT_PACKET
	{
		static char *ucProfile[] = {"P0", "P1", "P2", "P3"} ;
		static char *ucPacketSequence[] =
			{"Intermediate", "First", "Last", "Only"} ;

		BDBG_WRN(("Next Field:           %d", hHDR->GamutPacket.NextField)) ;
		BDBG_WRN(("No Current GBD:       %d", hHDR->GamutPacket.NoCurrentGamut)) ;
		BDBG_WRN(("GBD Profile:          %s", ucProfile[hHDR->GamutPacket.Profile])) ;
		BDBG_WRN(("GBD Affected Gamut Sequence Number:    %d",
			hHDR->GamutPacket.AffectedGamutSeqNumber)) ;
		BDBG_WRN(("GBD Current Gamut Sequence Number:     %d",
			hHDR->GamutPacket.CurrentGamutSeqNumber)) ;
		BDBG_WRN(("GBD Packet Sequence:  %s Packet",
			ucPacketSequence[hHDR->GamutPacket.PacketSeq])) ;

	}
#endif
	BDBG_LEAVE(BHDR_P_GetGamutPacketData_isr) ;
	return BERR_SUCCESS ;
}



BERR_Code BHDR_P_ParseGamutMetadataPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_GamutPacket *Packet)
{
	uint8_t temp ;
	uint8_t index ;
	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseGamutMetadataPacket_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* First, store original contents in the local HDMI structure */
	BKNI_Memcpy(&hHDR->GamutPacket, Packet, sizeof(BAVC_HDMI_Packet));


	/* Format Flag */
	temp = Packet->DataBytes[0] ;
	temp = temp & 0x80 ;
	if (temp)
		hHDR->GamutPacket.Format = BAVC_HDMI_GamutFormat_eRange ;
	else
		hHDR->GamutPacket.Format = BAVC_HDMI_GamutFormat_eVerticesFacets ;

	/* facet mode */
	temp = Packet->DataBytes[0] ;
	hHDR->GamutPacket.bFacets = (bool) (temp & 0x40) ;

	/* color precision */
	temp = Packet->DataBytes[0] & 0x18 ;
	temp = temp >> 3 ;
	hHDR->GamutPacket.eColorPrecision = (BAVC_HDMI_GamutColorPrecision) temp ;

	/* color space */
	temp = Packet->DataBytes[0] & 0x07 ;

	hHDR->GamutPacket.eColorSpace = (BAVC_HDMI_GamutColorspace) temp ;

#if BHDR_CONFIG_DEBUG_GAMUT_PACKET
	{
		static char *ucPacketFormat[] = {"Vertices/Facets", "Range Data"} ;
		static char *ucColorPrecision[] = {"8 Bit", "10 Bit", "12 bit"} ;
		static char *ucColorSpace[] = {
			"ITU-R BT.709 (using RGB)",
			"xvYCC601 (IEC 61966-2-4  SD) (using YCbCr)",
			"xvYCC709 (IEC 61966-2-4  HD) (using YCbCr)",
			"XYZ (see above)" } ;

		BDBG_WRN(("GBD Format: %s %s - %s",
			ucColorPrecision[hHDR->GamutPacket.eColorPrecision],
			ucPacketFormat[hHDR->GamutPacket.Format],
			ucColorSpace[hHDR->GamutPacket.eColorSpace])) ;
	}
#endif


	if (hHDR->GamutPacket.Format == BAVC_HDMI_GamutFormat_eVerticesFacets)
	{
		/* Number_Vertices */
		hHDR->GamutPacket.uiNumberVertices =
			  (uint16_t)  (Packet->DataBytes[1] << 8)
			|(uint16_t)  (Packet->DataBytes[2]) ;

		if (hHDR->GamutPacket.uiNumberVertices != 4)
		{
			BDBG_ERR(("GBD Vertices:        %d invalid; Assuming 4",
				hHDR->GamutPacket.uiNumberVertices)) ;
			hHDR->GamutPacket.uiNumberVertices = 4 ;
		}
#if BHDR_CONFIG_DEBUG_GAMUT_PACKET
		BDBG_WRN(("GBD Vertices:        %d",
			hHDR->GamutPacket.uiNumberVertices )) ;
#endif

		if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e8Bit)
		{
			for (i = 0 ; i < 4 ; i++)  /* for each pixel value black/red/green/blue */
			{
				index = 3 + (3 * i ) ;
				hHDR->GamutPacket.uiY[i]   = Packet->DataBytes[index] ;
				hHDR->GamutPacket.uiCb[i] = Packet->DataBytes[index + 1] ;
				hHDR->GamutPacket.uiCr[i]  = Packet->DataBytes[index + 2] ;
			}
		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e10Bit)
		{
			index = 3;

			/* point 0: Black Y, Cb, Cr */
			hHDR->GamutPacket.uiY[0]   =
				  (uint16_t) (Packet->DataBytes[index]	<< 2)
				| (uint16_t) ((Packet->DataBytes[index + 1] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiCb[0] =
				(uint16_t) ((Packet->DataBytes[index + 1] & 0x3F) << 4)
				| (uint16_t) ((Packet->DataBytes[index + 2] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiCr[0] =
				(uint16_t) ((Packet->DataBytes[index + 2] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[index + 3] & 0xFC) >> 2) ;


			/* point 1: Red Y, Cb, Cr	*/
			hHDR->GamutPacket.uiY[1]   =
				  (uint16_t) ((Packet->DataBytes[index + 3] & 0x03) << 8)
				| (uint16_t) (Packet->DataBytes[index + 4]) ;

			hHDR->GamutPacket.uiCb[1] =
				  (uint16_t) (Packet->DataBytes[index+5]  << 2)
				| (uint16_t) ((Packet->DataBytes[index + 6] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiCr[1] =
				(uint16_t) ((Packet->DataBytes[index + 6] & 0x3F) << 4)
				| (uint16_t) ((Packet->DataBytes[index + 7] & 0xF0) >> 4) ;

			/* point 2: Green Y, Cb, Cr */
			hHDR->GamutPacket.uiY[2] =
				(uint16_t) ((Packet->DataBytes[index + 7] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[index + 8] & 0xFC) >> 2) ;

			hHDR->GamutPacket.uiCb[2]	=
				  (uint16_t) ((Packet->DataBytes[index + 8] & 0x03) << 8)
				| (uint16_t) (Packet->DataBytes[index + 9]) ;

			hHDR->GamutPacket.uiCr[2] =
				  (uint16_t) (Packet->DataBytes[index+10]  << 2)
				| (uint16_t) ((Packet->DataBytes[index + 11] & 0xC0) >> 6) ;


			/* point 3: Blue Y, Cb, Cr	*/
			hHDR->GamutPacket.uiY[3] =
				(uint16_t) ((Packet->DataBytes[index + 11] & 0x3F) << 4)
				| (uint16_t) ((Packet->DataBytes[index + 12] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiCb[3] =
				(uint16_t) ((Packet->DataBytes[index + 12] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[index + 13] & 0xFC) >> 2) ;

			hHDR->GamutPacket.uiCr[3]	=
				  (uint16_t) ((Packet->DataBytes[index + 13] & 0x03) << 8)
				| (uint16_t) (Packet->DataBytes[index + 14]) ;

		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e12Bit)
		{
			index = 3;
			for (i = 0 ; i < 4 ; i++)  /* for each pixel value black/red/green/blue */
			{
				hHDR->GamutPacket.uiY[i]   =
					  (uint16_t) (Packet->DataBytes[index]	<< 4)
					| (uint16_t) ((Packet->DataBytes[index+1] & 0xF0) >> 4) ;

				hHDR->GamutPacket.uiCb[i] =
					(uint16_t) ((Packet->DataBytes[index+1] & 0x0F) << 8)
					| (uint16_t) (Packet->DataBytes[index+2]) ;

				hHDR->GamutPacket.uiCr[i]	=
					  (uint16_t) (Packet->DataBytes[index+3]  << 4)
					| (uint16_t) ((Packet->DataBytes[index+4] & 0xF0) >> 4) ;

				hHDR->GamutPacket.uiY[i] =
					(uint16_t) ((Packet->DataBytes[index+4] & 0x0F) << 8)
					| (uint16_t) (Packet->DataBytes[index+5]) ;

				hHDR->GamutPacket.uiCb[i]	=
					  (uint16_t) (Packet->DataBytes[index+6]  << 4)
					| (uint16_t) ((Packet->DataBytes[index+7] & 0xF0) >> 4) ;

				hHDR->GamutPacket.uiCr[i] =
					(uint16_t) ((Packet->DataBytes[index+7] & 0x0F) << 8)
					| (uint16_t) (Packet->DataBytes[index+8]) ;

				index += 9;
			}
		}

#if BHDR_CONFIG_DEBUG_GAMUT_PACKET
		for (i=0; i<4; i++)
		{
			BDBG_MSG(("Point %d Y= %3d	Cb= %3d, Cr= %3d",
				i+1, hHDR->GamutPacket.uiY[i],
				hHDR->GamutPacket.uiCb[i], hHDR->GamutPacket.uiCr[i]))	;
		}
#endif

		/* FACET DATA not supported by HDMI 1.3 */

	}
	else if (hHDR->GamutPacket.Format == BAVC_HDMI_GamutFormat_eRange)
	{
		if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e8Bit)
		{
			/* RED Data */
			hHDR->GamutPacket.uiMinRedData = (uint16_t) Packet->DataBytes[1]  ;
			hHDR->GamutPacket.uiMaxRedData = (uint16_t) Packet->DataBytes[2] ;


			/* GREEN Data */
			hHDR->GamutPacket.uiMinGreenData = (uint16_t) Packet->DataBytes[3] ;
			hHDR->GamutPacket.uiMaxGreenData = (uint16_t) Packet->DataBytes[4] ;


			/* BLUE Data */
			hHDR->GamutPacket.uiMinBlueData = (uint16_t) Packet->DataBytes[5]  ;
			hHDR->GamutPacket.uiMaxBlueData = (uint16_t) Packet->DataBytes[6] ;
		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e10Bit)
		{
			/* RED Data */
			hHDR->GamutPacket.uiMinRedData =
				  (uint16_t) (Packet->DataBytes[1]  << 2)
				| (uint16_t) ((Packet->DataBytes[2] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiMaxRedData =
				  (uint16_t) ((Packet->DataBytes[2] & 0x3F)  << 4)
				| (uint16_t) ((Packet->DataBytes[3] & 0xF0) >> 4) ;


			/* GREEN Data */
			hHDR->GamutPacket.uiMinGreenData =
				  (uint16_t) ((Packet->DataBytes[3] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[4] & 0xFC) >> 2) ;

			hHDR->GamutPacket.uiMaxGreenData =
				  (uint16_t) ((Packet->DataBytes[4] & 0x03)  << 8)
				| (uint16_t) (Packet->DataBytes[5]) ;


			/* BLUE Data */
			hHDR->GamutPacket.uiMinBlueData =
				  (uint16_t) (Packet->DataBytes[6]  << 2)
				| (uint16_t) ((Packet->DataBytes[7] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiMaxBlueData =
				  (uint16_t) ((Packet->DataBytes[7] & 0x3F)  << 4)
				| (uint16_t) ((Packet->DataBytes[8] & 0xF0) >> 4) ;

		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e12Bit)
		{
			/* RED Data */
			hHDR->GamutPacket.uiMinRedData =
				  (uint16_t) (Packet->DataBytes[1]  << 4)
				| (uint16_t) ((Packet->DataBytes[2] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiMaxRedData =
				  (uint16_t) ((Packet->DataBytes[2] & 0x0F)  << 8)
				| (uint16_t) (Packet->DataBytes[3]) ;


			/* GREEN Data */
			hHDR->GamutPacket.uiMinGreenData =
				  (uint16_t) (Packet->DataBytes[4]  << 4)
				| (uint16_t) ((Packet->DataBytes[5] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiMaxGreenData =
				  (uint16_t) ((Packet->DataBytes[5] & 0x0F)  << 8)
				| (uint16_t) (Packet->DataBytes[6]) ;


			/* BLUE Data */
			hHDR->GamutPacket.uiMinBlueData =
				  (uint16_t) (Packet->DataBytes[7]  << 4)
				| (uint16_t) ((Packet->DataBytes[8] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiMaxBlueData =
				  (uint16_t) ((Packet->DataBytes[8] & 0x0F)  << 8)
				| (uint16_t) (Packet->DataBytes[9]) ;
		}
		else
		{
			BDBG_ERR(("Unknown Gamut Color Precision Format",
				hHDR->GamutPacket.eColorPrecision)) ;
		}

#if BHDR_CONFIG_DEBUG_GAMUT_PACKET
		BDBG_MSG(("RED RANGE:     %d to %d",
			hHDR->GamutPacket.uiMinRedData, hHDR->GamutPacket.uiMaxRedData)) ;
		BDBG_MSG(("GREEN RANGE:   %d to %d",
			hHDR->GamutPacket.uiMinGreenData, hHDR->GamutPacket.uiMaxGreenData)) ;
		BDBG_MSG(("BLUE RANGE:    %d to %d",
			hHDR->GamutPacket.uiMinBlueData, hHDR->GamutPacket.uiMaxBlueData)) ;
#endif
	}


	/* notify users of gamut change */
	BDBG_WRN(("Gamut Change Notification")) ;

	/* call  the callback functions for Format Change notification  */
	if (hHDR->pfGamutChangeCallback)
	{
		hHDR->pfGamutChangeCallback(hHDR->pvGamutChangeParm1,
			hHDR->iGamutChangeParm2, &hHDR->GamutPacket) ;
	}

	BDBG_LEAVE(BHDR_P_ParseGamutMetadataPacket_isr) ;
	return BERR_SUCCESS ;
 }



#endif


