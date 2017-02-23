/***************************************************************************
 *  Copyright (C) 2002-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

/*================== Module Overview =====================================
This module is used to program the FPGA on the 97038 reference design.

It requires the EBI bus to be configured such that the FPGA's registers
are memory mapped.  Normally, this is done in the boot ROM, but can be
configured as part of the application software using the following.

Sample Code:

#include "bchp_ebi.h"
#include "boardmap.h"
static void BFramework_InitFpga(const BFramework_Info *pFrameworkInfo)
{
	BREG_Write32(pFrameworkInfo->hRegister, BCHP_EBI_CS_BASE_3, CPU_PHYS_FPGA_BASE|BCHP_EBI_CS_BASE_3_size_SIZE_32MB);
	BREG_Write32(pFrameworkInfo->hRegister, BCHP_EBI_CS_CONFIG_3, 
			BCHP_FIELD_DATA(EBI_CS_CONFIG_3, enable, 1) | 
			BCHP_FIELD_DATA(EBI_CS_CONFIG_3, ta_wait, 1) |
#if BSTD_CPU_ENDIAN==BSTD_ENDIAN_LITTLE
			 BCHP_FIELD_DATA(EBI_CS_CONFIG_3, le, 1)
#else
			0
#endif
			);
}
=============== End of Module Overview ===================================*/

#ifndef BFPGA__
#define BFPGA__

#ifdef NEXUS_FPGA_SUPPORT

#include "breg_mem.h"
#include "breg_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
This structure is used to communicate the read only information from
the FPGA.
*/
typedef struct
{
	uint8_t board_cfg; /* board config from BOARD_FPGA_CFG[3:0] pins */
	uint8_t fpga_ver; /* FPGA version high and low */
	uint8_t strap_pins; /* value strapped from BOARD_CFG[3:0] pins */
} BFPGA_info;

/*
Summary:
This enumeration describes the possible TS input sources that can
be configured in the FPGA.
*/
typedef enum
{
	BFPGA_TsSelect_eQam_Ds1=0, /* 3250 DS1 */
	BFPGA_TsSelect_eMb4500=0, /* MB 4500 SDS */
	BFPGA_TsSelect_eQam_Oob=1, /* 3250 OOB */
	BFPGA_TsSelect_eMb3517=1, /* MB 3517 VSB/QAM */
	BFPGA_TsSelect_eHsx_1=2, /* 7038 HSX 1 */
	BFPGA_TsSelect_eRmx_0=2, /* 7401 RMX 0 */
	BFPGA_TsSelect_eHsx_2=3, /* 7038 HSX 2 */
	BFPGA_TsSelect_eRmx_1=3, /* 7401 RMX 1 */
	BFPGA_TsSelect_eEnc_Ts0=4, /* 7041 TS 0 */
	BFPGA_TsSelect_e1394=5, /* 1394 */
	BFPGA_TsSelect_eLvds_2=6, /* LVDS JP2803 or ASI J1801 (Streamer 2) */
	BFPGA_TsSelect_eLvds_1=7, /* LVDS JP2801 (Streamer 1) */
	BFPGA_TsSelect_eQam_Ds2=8, /* 3250 DS 2 */
	BFPGA_TsSelect_eSlot0_Ts2=8, /* Slot0 TS2 */
	BFPGA_TsSelect_eEnc_Ts1=9, /* 7041 TS 0 */
	BFPGA_TsSelect_eSlot0_Ts3=9,  /* Slot0 TS3 */
	BFPGA_TsSelect_ePod=10, /* POD (reserved for BFPGA_OutputSelect_ePod) */
	BFPGA_TsSelect_eVsb_1=11, /* VSB 1 */
	BFPGA_TsSelect_eSlot1_Ts4=11, /* Slot1 TS4 */
	BFPGA_TsSelect_eVsb_2=12, /* VSB 2 */
	BFPGA_TsSelect_eSlot1_Ts5=12, /* Slot1 TS5 */
	BFPGA_TsSelect_e8psk_1 = BFPGA_TsSelect_eVsb_1, /* 8PSK 1 */
	BFPGA_TsSelect_e8psk_2 = BFPGA_TsSelect_eVsb_2, /* 8PSK 2 */
	BFPGA_TsSelect_eReserved_1=13, /* reserved */
	BFPGA_TsSelect_eMultiStreamPod=14, /* Multi-stream POD mode (valid for BFPGA_OutputSelect_ePod only) */
	BFPGA_TsSelect_eSlot2_Ts6=14, /* Slot1 TS6 */
	BFPGA_TsSelect_eDisable=15 /* disable */
} BFPGA_TsSelect;

/*
Summary:
This enumeration describes all of the TS outputs that
the FPGA supports.

Description:
The order of this enum is used for the FPGA configuration register address for most of the enums, therefore order matters.
*/
typedef enum
{
	BFPGA_OutputSelect_ePkt0, /* Packet 0 */
	BFPGA_OutputSelect_ePkt1, /* Packet 1 */
	BFPGA_OutputSelect_ePkt2, /* Packet 2 */
	BFPGA_OutputSelect_ePkt3, /* Packet 3 */
	BFPGA_OutputSelect_e1394, /* 1394 */
	BFPGA_OutputSelect_eTest, /* Test */
	BFPGA_OutputSelect_ePod,  /* POD */
	BFPGA_OutputSelect_eAvc,  /* AVC Daughter card */
	BFPGA_OutputSelect_ePkt4  /* Packet 4 */
} BFPGA_OutputSelect;

/*
Summary:
This enumeration describes the clock polarities.
*/
typedef enum
{
	BFPGA_ClockPolarity_ePositiveEdge, /* Clock on Positive Edge */
	BFPGA_ClockPolarity_eNegativeEdge /* Clock on Negative Edge */
} BFPGA_ClockPolarity;

/*
Summary:
This handle is used for all calls to the fgpa module.
*/
typedef struct BFPGA_Impl *BFPGA_Handle; /* Opaique Handle */

/*
Summary:
	Required default settings structure for FPGA module.

Description:
	The default setting structure defines the default configuration of
	FPGA when the device is opened.

*/
typedef struct
{
	uint32_t regBase;					/* FPGA EBI: base address of where FPGA is located */
	unsigned short i2cAddr;				/* FPGA I2C: 7bit I2C address of FPGA, */
} BFPGA_Settings;

/*
Summary:
This function creates and returns a fpga handle.

Description:
Before this function is called, the FPGA must be memory mapped on
the EBI bus (see module overview), and a register handle must be
created.
*/
BERR_Code BFPGA_Open(
	BFPGA_Handle	*phFpga, 			/* [out] returns handle to fpga */
	BREG_Handle 	hReg, 				/* [in] Handle to Memory Mapped register */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BFPGA_Settings *pDefSettings	/* [in] Default settings */
	);

/*
Summary:
This function closes a fpga handle.
*/
void BFPGA_Close(
	BFPGA_Handle hFpga /* handle to fpga */
	);

/*
Summary:
This function returns the default settings

Description:
*/
BERR_Code BFPGA_GetDefaultSettings(
	BFPGA_Settings *pDefSettings		/* [out] Returns default setting */
	);

/*
Summary:
This function returns the FPGA info structure.
*/
BERR_Code BFPGA_GetInfo( 
	BFPGA_Handle hFpga, /* handle to fpga */
	BFPGA_info *pInfo /* [out] pointer to info structure that will be filled with data */
	);

/*
Summary:
This function resets the FPGA.
*/
BERR_Code BFPGA_Reset(	
	BFPGA_Handle hFpga /* handle to fpga */
	);

/*
Summary:
This function configures a specified output of the FPGA.

Description:
Each output of the FPGA can be assigned an input.  The same
input can be mapped to any number of outputs, but for obvious
reasons an output can only have a single input.
*/
BERR_Code BFPGA_SetTsOutput( 
	BFPGA_Handle hFpga,	/* handle to fpga */
	BFPGA_OutputSelect outputSelect, /* Selects which of the FPGA TS outputs to configure */
	BFPGA_TsSelect tsSelect /* Selects the ts stream to send to selected output */
	);

/*
Summary:
This function returns the input source for a specified output of the FPGA.
*/
BERR_Code BFPGA_GetTsOutput( 
	BFPGA_Handle hFpga,	/* handle to fpga */
	BFPGA_OutputSelect outputSelect, /* Selects which of the FPGA TS outputs to configure */
	BFPGA_TsSelect *p_tsSelect, /* [out] Returns the ts stream that is routed to selected output */
	bool *p_softwareConfigured /* [out] Returns if the ts stream is configured by software (if not it is configured by the jumper setting) */
	);

/*
Summary:
This function configures the clock polarity for a specified output of the FPGA.
*/
BERR_Code BFPGA_SetClockPolarity( 
	BFPGA_Handle hFpga, /* handle to fpga */
	BFPGA_OutputSelect outputSelect, /* Selects which of the FPGA TS outputs to configure */
	BFPGA_ClockPolarity inputClockPolarity, /* Selects the input clock polarity */
	BFPGA_ClockPolarity outputClockPolarity /* Selects the output clock polarity */
	);

/*
Summary:
This function returns the clock polarity for a specified output of the FPGA.
*/
BERR_Code BFPGA_GetClockPolarity( 
	BFPGA_Handle hFpga, /* handle to fpga */
	BFPGA_OutputSelect outputSelect, /* Selects which of the FPGA TS outputs to configure */
	BFPGA_ClockPolarity *p_inputClockPolarity, /* [out] Returns the input clock polarity */
	BFPGA_ClockPolarity *p_outputClockPolarity /* [out] Returns the output clock polarity */
	);

#ifdef __cplusplus
}
#endif

#endif

#endif

