/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BHDM_SCDC_H__
#define BHDM_SCDC_H__

#include "bhdm.h"
#include "bavc.h"

/*=************************ Module Overview ********************************
  The SCDC (Enhanced Display Identification Data) functions provide support
  for reading/interpretting the SCDC prom contained in the DVI/HDMI Receiver.

  The PROM has an I2C address of 0xA0.  These functions support the Enhanced
  DDC protocol as needed.
***************************************************************************/

/************************************************************************
Memory Requirements

The Memory Requirements
TBD
***************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#define BHDM_SCDC_I2C_ADDR 0x54
/* A8 - Write  /  A9 - Read */

/* use EDID return errors */

/* SCDC Offsets */
#define BHDM_SCDC_SINK_VERSION           0x01
#define BHDM_SCDC_SOURCE_VERSION      0x02

#define BHDM_SCDC_UPDATE_0      0x10
	#define BHDM_SCDC_UPDATE_0_MASK_STATUS_UPDATE      0x01
	#define BHDM_SCDC_UPDATE_0_MASK_CED_UPDATE      0x02
	#define BHDM_SCDC_UPDATE_0_MASK_RR_TEST      0x04

#define BHDM_SCDC_UPDATE_1      0x11


#define BHDM_SCDC_TMDS_CONFIG             0x20
	#define BHDM_SCDC_TMDS_CONFIG_MASK_SCRAMBLE             0x01
	#define BHDM_SCDC_TMDS_CONFIG_MASK_BIT_CLOCK_RATIO   0x2

#define BHDM_SCDC_SCRAMBLER_STATUS    0x21
	#define BHDM_SCDC_SCRAMBLER_STATUS_MASK_SCRAMBLING_STATUS    0x01


#define BHDM_SCDC_CONFIG_0        0x30
	/* RR Bit must be cleared to enable SCDC polling */
	#define BHDM_SCDC_CONFIG_0_POLLING_ENABLE 0


#define BHDM_SCDC_STATUS_FLAGS_0 0x40
	#define BHDM_SCDC_STATUS_FLAGS_0_MASK_CLOCK_DETECTED 0x01
	#define BHDM_SCDC_STATUS_FLAGS_0_MASK_CH0_LOCKED 0x02
	#define BHDM_SCDC_STATUS_FLAGS_0_MASK_CH1_LOCKED 0x04
	#define BHDM_SCDC_STATUS_FLAGS_0_MASK_CH2_LOCKED 0x08


#define BHDM_SCDC_STATUS_FLAGS_1 0x41

#define BHDM_SCDC_CED_OFFSET 0x50
#define BHDM_SCDC_CED_LENGTH  0x7
	#define BHDM_SCDC_ERR_DET_CH_MASK_VALID 0x80

#define BHDM_SCDC_TEST_CONFIG_0 0xC0
	#define BHDM_SCDC_TEST_CONFIG_0_MASK_TEST_RR 0x80
	#define BHDM_SCDC_TEST_CONFIG_0_MASK_TEST_RR_DELAY 0x7F

#define BHDM_SCDC_MANUFACTURER_OUI_3  0xD0
#define BHDM_SCDC_MANUFACTURER_OUI_2  0xD1
#define BHDM_SCDC_MANUFACTURER_OUI_1  0xD2

#define BHDM_SCDC_DEVICE_ID_STRING 0xD3
	#define BHDM_SCDC_DEVICE_ID_LENGTH 8

#define BHDM_SCDC_DEVICE_ID_HARDWARE_REV 0xDB
	#define BHDM_SCDC_DEVICE_ID_HARDWARE_REV_MASK_MAJOR 0xF0
	#define BHDM_SCDC_DEVICE_ID_HARDWARE_REV_MASK_MINOR 0x0F

#define BHDM_SCDC_DEVICE_ID_SOFTWARE_MAJOR_REV 0xDC
#define BHDM_SCDC_DEVICE_ID_SOFTWARE_MINOR_REV 0xDD

#define BHDM_SCDC_DEVICE_ID_MANUFACTURER_SPECIFIC 0xDE
	#define BHDM_SCDC_DEVICE_ID_MANUFACTURER_SPECIFIC_LEN 34


/******************************************************************************
Summary:
Status Data available from the SCDC (Status and Control Data Channel)

Description:
SCDC information includes TMDS configuration and lock status as well as error detection.
Information can be found in HDMI 2.0  Section 10.4.1.1

See Also:
	o BHDM_SCDC_Data
	o BHDM_SCDC_ReadManufacturerData
	o BHDM_SCDC_ManufacturerData

*******************************************************************************/
typedef struct   BHDM_SCDC_StatusControlData
{
	uint8_t SinkVersion ;
	uint8_t SourceVersion ;

	uint8_t Update_0 ;
	uint8_t Update_1 ;

	uint8_t TMDSConfig ;
	uint8_t RxScramblerStatus ;

	uint8_t Config_0 ;

	uint8_t StatusFlags_0 ;
	uint8_t StatusFlags_1 ;


	uint8_t TestConfig_0 ;

	struct {
		bool valid ;
		uint8_t Err_L ;	uint8_t Err_H ;
		uint16_t errorCount ;
	} ch[3] ;

	uint8_t ErrorDetectionChecksum ;

} BHDM_SCDC_StatusControlData ;


typedef struct   BHDM_SCDC_ManufacturerData
{
	uint8_t Manufacturer_OUI[3] ;
	uint8_t Device_ID_String[BHDM_SCDC_DEVICE_ID_LENGTH] ;

	uint8_t HardwareRev ;
	uint8_t SoftwareMajorRev ;
	uint8_t SoftwareMinorRev ;

	uint8_t ManufacturerSpecific[BHDM_SCDC_DEVICE_ID_MANUFACTURER_SPECIFIC_LEN] ;

} BHDM_SCDC_ManufacturerData ;


/******************************************************************************
Summary:
Retrieve the basic SCDC data

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pStatusControlData - pointer to BHDM_SCDC_Data structure to hold the SCDC DATA

Returns:
	BERR_SUCCESS            - Basic SCDC Data successfully retrieved
	BERR_INVALID_PARAMETER  - Invalid function parameter.
	BHDM_SCDC_NOT_FOUND      - No SCDC Available

See Also:
	o BHDM_Handle
	o BHDM_SCDC_Data
	o BHDM_SCDC_ReadManufacturerData
	o BHDM_SCDC_ManufacturerData

*******************************************************************************/
BERR_Code BHDM_SCDC_Initialize(
   const BHDM_Handle hHDMI               /* [in] HDMI handle */
) ;


/******************************************************************************
Summary:
Retrieve the basic SCDC data

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pStatusControlData - pointer to BHDM_SCDC_Data structure to hold the SCDC DATA

Returns:
	BERR_SUCCESS            - Basic SCDC Data successfully retrieved
	BHDM_SCDC_NOT_FOUND      - No SCDC Available

See Also:
	o BHDM_Handle

*******************************************************************************/
BERR_Code BHDM_SCDC_ReadStatusControlData(
   const BHDM_Handle hHDMI,               /* [in] HDMI handle */
   BHDM_SCDC_StatusControlData *pStatusControlData /* [out] pointer to structure to hold Basic
                                            SCDC Data */
) ;


/******************************************************************************
Summary:
Retrieve the Manufacturer SCDC data from the SCDC Channel

Input:
	hHDMI - The HDMI device handle that the application created earlier
	during the system initialization sequence.

Output:
	pManufacturerData - pointer to BHDM_SCDC_ManufacturerData structure to hold the Manufacturer Specific
	Data

Returns:
	BERR_SUCCESS            - Basic SCDC Data successfully retrieved
	BERR_INVALID_PARAMETER  - Invalid function parameter.
	BHDM_SCDC_NOT_FOUND      - No SCDC Available

See Also:
	o BHDM_Handle
	o BHDM_SCDC_Data
	o BHDM_SCDC_ReadStatusControlData
	o BHDM_SCDC_ManufacturerData

*******************************************************************************/
BERR_Code BHDM_SCDC_ReadManufacturerData(
   const BHDM_Handle hHDMI,               /* [in] HDMI handle */
   BHDM_SCDC_ManufacturerData *pManufacturerData /* [out] pointer to structure to hold Manufacturer
                                            SCDC Data */
) ;

#ifdef __cplusplus
}
#endif

#endif /* BHDM_SCDC_H__ */
