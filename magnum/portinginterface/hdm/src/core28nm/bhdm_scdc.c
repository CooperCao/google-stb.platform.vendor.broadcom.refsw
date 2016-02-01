/***************************************************************************
*     Copyright (c) 2003-2014, Broadcom Corporation
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
* Module Description:
* This module implements the SCDC functionality described in the HDMI 2.0 Specification
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bhdm_scdc.h"

BDBG_MODULE(BHDM_SCDC) ;


/******************************************************************************
Summary:
	struct containing offset and lenghts of data items read/write to the HDMI SCDC address
*******************************************************************************/
typedef struct BHDM_SCDC_DataMap
{
	const char *name ;
	uint8_t offset ;
	uint8_t length ;
	uint8_t structOffset ;
} BHDM_SCDC_DataMap ;


/******************************************************************************
Summary:
	Initialize the SCDC configuration
*******************************************************************************/
#if BHDM_HAS_HDMI_20_SUPPORT
BERR_Code BHDM_SCDC_Initialize(
   const BHDM_Handle hHDMI               /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_I2C_Handle i2cHandle ;
	uint8_t RR_Enable ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport == 0)
	{
		BDBG_WRN(("Attached Rx <%s> does not support SCDC (Status and Control Data Channel)",
			hHDMI->AttachedEDID.MonitorName)) ;
		rc = BERR_NOT_AVAILABLE ;
		goto done ;
	}

	if (!hHDMI->bAutoI2cMode)
	{
		i2cHandle = hHDMI->hI2cRegHandle ;
		RR_Enable = BHDM_SCDC_CONFIG_0_POLLING_ENABLE  ;
		/* TODO Disable RR_Enable per Review/Chris */
#if 0
		rc = BREG_I2C_Write(i2cHandle, BHDM_SCDC_I2C_ADDR, BHDM_SCDC_TMDS_CONFIG, &RR_Enable, 1) ;
		if (rc)
		{
			BERR_TRACE(rc) ;
			goto done ;
		}
#else
		/* eliminate compilation warning */
		BSTD_UNUSED(i2cHandle) ;
		BSTD_UNUSED(RR_Enable) ;
#endif
		hHDMI->bAutoI2cMode = true ;
	}

done:
	return rc ;

}

/******************************************************************************
Summary:
	Get the  Manufacturer Data which never changes (only need once after hotplug)
*******************************************************************************/
BERR_Code BHDM_SCDC_ReadManufacturerData(
   const BHDM_Handle hHDMI,               /* [in] HDMI handle */
   BHDM_SCDC_ManufacturerData *pManufacturerData /* [out] pointer to structure to hold SCDC Data */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BHDM_SCDC_ManufacturerData manufacturerData ;
	uint8_t i, items ;

#define SCDC_MANUFACTURER_DATA_OFFSET1(field) \
	((uint8_t *) &manufacturerData.field - (uint8_t *) &manufacturerData )

#define SCDC_MANUFACTURER_DATA_OFFSET(structure, field) \
	((uint8_t *) &structure.field - (uint8_t *) &structure )

	static const BHDM_SCDC_DataMap scdcManufacturerDataMap[] =
	{

		{BDBG_STRING("Manufcturer OUI"), BHDM_SCDC_MANUFACTURER_OUI_3, 3,
			SCDC_MANUFACTURER_DATA_OFFSET(manufacturerData, Manufacturer_OUI)},

		{BDBG_STRING("Device ID"), BHDM_SCDC_DEVICE_ID_STRING, BHDM_SCDC_DEVICE_ID_LENGTH,
			SCDC_MANUFACTURER_DATA_OFFSET(manufacturerData, Device_ID_String)},

		{BDBG_STRING("Hardware Rev"), BHDM_SCDC_DEVICE_ID_HARDWARE_REV, 1,
			SCDC_MANUFACTURER_DATA_OFFSET(manufacturerData, HardwareRev)},

		{BDBG_STRING("Software Major Rev"), BHDM_SCDC_DEVICE_ID_SOFTWARE_MAJOR_REV, 1,
			SCDC_MANUFACTURER_DATA_OFFSET(manufacturerData, SoftwareMajorRev)},

		{BDBG_STRING("Software Minor Rev"), BHDM_SCDC_DEVICE_ID_SOFTWARE_MINOR_REV, 1,
			SCDC_MANUFACTURER_DATA_OFFSET(manufacturerData, SoftwareMinorRev)},

		{BDBG_STRING("Manufacturer Speciific"), BHDM_SCDC_DEVICE_ID_MANUFACTURER_SPECIFIC,
			BHDM_SCDC_DEVICE_ID_MANUFACTURER_SPECIFIC_LEN,
			SCDC_MANUFACTURER_DATA_OFFSET(manufacturerData, ManufacturerSpecific)}
	} ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BDBG_ASSERT(pManufacturerData) ;
	BKNI_Memset(pManufacturerData, 0, sizeof(BHDM_SCDC_ManufacturerData)) ;
	BKNI_Memset(&manufacturerData, 0, sizeof(BHDM_SCDC_ManufacturerData)) ;

	if (hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport == 0)
	{
		BDBG_WRN(("Attached Rx <%s> does not support SCDC; cannot get Manufacturer Data",
			hHDMI->AttachedEDID.MonitorName)) ;
		rc = BERR_NOT_AVAILABLE ;
		goto done ;
	}

	i = 0 ;
	items = sizeof(scdcManufacturerDataMap) / sizeof(BHDM_SCDC_DataMap) ;

	for ( i = 0 ; i < items ; i++)
	{
		rc = BHDM_P_BREG_I2C_Read(hHDMI, BHDM_SCDC_I2C_ADDR, scdcManufacturerDataMap[i].offset,
			((uint8_t *) &manufacturerData + scdcManufacturerDataMap[i].structOffset),
			scdcManufacturerDataMap[i].length) ;

		if (rc)
		{
			rc = BERR_TRACE(rc) ;
			goto done ;
		}

		BDBG_MSG(("[0x%02x] %-20s  = 0x%02x",
			scdcManufacturerDataMap[i].offset, scdcManufacturerDataMap[i].name,
			* ((uint8_t *) &manufacturerData + scdcManufacturerDataMap[i].structOffset))) ;
	}

	BKNI_Memcpy(pManufacturerData, &manufacturerData, sizeof(BHDM_SCDC_ManufacturerData)) ;

done:
	return rc ;
}



/******************************************************************************
Summary:
	Get/return the basic Rx Status and Control Data (does not include Manufacturer Data which never changes)
*******************************************************************************/
void BHDM_SCDC_GetStatusControlData(
   const BHDM_Handle hHDMI,               /* [in] HDMI handle */
   BHDM_SCDC_StatusControlData *pStatusControlData /* [out] pointer to structure to hold SCDC Data */
)
{
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	*pStatusControlData = hHDMI->stStatusControlData ;
}


/******************************************************************************
Summary:
	Read the basic Rx Status and Control Data (does not include Manufacturer Data which never changes)
*******************************************************************************/
BERR_Code BHDM_SCDC_ReadStatusControlData(
   const BHDM_Handle hHDMI,               /* [in] HDMI handle */
   BHDM_SCDC_StatusControlData *pStatusControlData /* [out] pointer to structure to hold SCDC Data */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_I2C_Handle i2cHandle ;
	BHDM_SCDC_StatusControlData statusControlData ;
	uint8_t i, items, temp ;

#define SCDC_DATA_OFFSET(structure, field) \
	((uint8_t *) &structure.field - (uint8_t *) &structure )

	static const BHDM_SCDC_DataMap scdcStatusControlDataMap[] =
	{
		{"Sink Version", BHDM_SCDC_SINK_VERSION, 1, SCDC_DATA_OFFSET(statusControlData, SinkVersion)},
		{"Source Version", BHDM_SCDC_SOURCE_VERSION, 1, SCDC_DATA_OFFSET(statusControlData, SourceVersion)},
		{"Updated_0", BHDM_SCDC_UPDATE_0, 1,	SCDC_DATA_OFFSET(statusControlData, Update_0)},
		{"Updated_1", BHDM_SCDC_UPDATE_1, 1,	SCDC_DATA_OFFSET(statusControlData, Update_1)},


		{"TMDS Config", BHDM_SCDC_TMDS_CONFIG, 1, SCDC_DATA_OFFSET(statusControlData, TMDSConfig)},
		{"Scramble Status", BHDM_SCDC_SCRAMBLER_STATUS, 1, SCDC_DATA_OFFSET(statusControlData, RxScramblerStatus)},

		{"Config_0", BHDM_SCDC_CONFIG_0, 1, SCDC_DATA_OFFSET(statusControlData, Config_0)},

		{"Status_Flags_0", BHDM_SCDC_STATUS_FLAGS_0, 1, SCDC_DATA_OFFSET(statusControlData, StatusFlags_0)},
		{"Status_Flags_1", BHDM_SCDC_STATUS_FLAGS_1, 1, SCDC_DATA_OFFSET(statusControlData, StatusFlags_1)},

		{"Err_Det_0_L", BHDM_SCDC_ERR_DET_0_L,	 1, SCDC_DATA_OFFSET(statusControlData, ch[0].Err_L)},
		{"Err_Det_0_H", BHDM_SCDC_ERR_DET_0_H,	 1, SCDC_DATA_OFFSET(statusControlData, ch[0].Err_H)},

		{"Err_Det_1_L", BHDM_SCDC_ERR_DET_1_L,	 1, SCDC_DATA_OFFSET(statusControlData, ch[1].Err_L)},
		{"Err_Det_1_H", BHDM_SCDC_ERR_DET_1_H,	 1, SCDC_DATA_OFFSET(statusControlData, ch[1].Err_H)},

		{"Err_Det_2_L", BHDM_SCDC_ERR_DET_2_L,	 1, SCDC_DATA_OFFSET(statusControlData, ch[2].Err_L)},
		{"Err_Det_2_H", BHDM_SCDC_ERR_DET_2_H,	 1, SCDC_DATA_OFFSET(statusControlData, ch[2].Err_H)},
		{"Err_Det_Checksum", BHDM_SCDC_ERR_DET_CHECKSUM, 1, SCDC_DATA_OFFSET(statusControlData, ErrorDetectionChecksum)},

		{"Test_Config_0", BHDM_SCDC_TEST_CONFIG_0, 1, SCDC_DATA_OFFSET(statusControlData, TestConfig_0)}
	} ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BDBG_ASSERT(pStatusControlData) ;
	BKNI_Memset(pStatusControlData, 0, sizeof(BHDM_SCDC_StatusControlData)) ;

	if (hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport == 0)
	{
		BDBG_WRN(("Attached Rx <%s> does not support SCDC; Cannot read Status and Control Data)",
			hHDMI->AttachedEDID.MonitorName)) ;
		rc = BERR_NOT_AVAILABLE ;
		goto done ;
	}


	i = 0 ;
	items = sizeof(scdcStatusControlDataMap) / sizeof(BHDM_SCDC_DataMap) ;
	i2cHandle = hHDMI->hI2cRegHandle ;

	for ( i = 0 ; i < items ; i++)
	{
		rc = BHDM_P_BREG_I2C_Read(hHDMI, BHDM_SCDC_I2C_ADDR, scdcStatusControlDataMap[i].offset,
			((uint8_t *) &statusControlData + scdcStatusControlDataMap[i].structOffset), scdcStatusControlDataMap[i].length) ;
		if (rc)
		{
			rc = BERR_TRACE(rc) ;
			goto done ;
		}

		BDBG_MSG(("[0x%02x] %-20s  = 0x%02x",
			scdcStatusControlDataMap[i].offset, scdcStatusControlDataMap[i].name,
			* ((uint8_t *) &statusControlData + scdcStatusControlDataMap[i].structOffset))) ;
	}


	/* get derived values */
	for (i = 0 ; i < 3 ; i++)
	{
		statusControlData.ch[i].valid =
			statusControlData.ch[i].Err_H & BHDM_SCDC_ERR_DET_0_H_MASK_VALID ;
		statusControlData.ch[i].errorCount =
			(uint16_t) ((statusControlData.ch[i].Err_H & 0x7F) << 8)  | (uint16_t) (statusControlData.ch[i].Err_L) ;

		BDBG_MSG(("CH%d: Valid =  %d  Error Count %u", i,
			statusControlData.ch[i].valid, statusControlData.ch[i].errorCount)) ;
	}

	/* report any detected errors */
	if ((statusControlData.ch[0].valid && statusControlData.ch[0].errorCount)
	|| (statusControlData.ch[1].valid && statusControlData.ch[1].errorCount)
	|| (statusControlData.ch[2].valid && statusControlData.ch[2].errorCount))
	{
		BDBG_WRN(("CH0: Valid =  %s  Error Count %d",
			statusControlData.ch[0].valid ? "Yes" : "No", statusControlData.ch[0].errorCount)) ;
		BDBG_WRN(("CH1: Valid =  %s  Error Count %d",
			statusControlData.ch[1].valid ? "Yes" : "No", statusControlData.ch[1].errorCount)) ;
		BDBG_WRN(("CH2: Valid =  %s  Error Count %d",
			statusControlData.ch[2].valid ? "Yes" : "No", statusControlData.ch[2].errorCount)) ;
	}

	*pStatusControlData = statusControlData ;
	hHDMI->stStatusControlData = statusControlData ;

	/* clear update flag */
	temp = 1 ;
	rc = BREG_I2C_Write(i2cHandle, BHDM_SCDC_I2C_ADDR, BHDM_SCDC_UPDATE_0, &temp, 1) ;
	if (rc)
	{
		rc = BERR_TRACE(rc) ;
		goto done ;
	}

done:
	return rc ;
}


void BHDM_SCDC_GetScrambleConfiguration(BHDM_Handle hHDMI, BHDM_ScrambleConfig *pstScrambleConfig)
{
	BERR_Code rc ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint8_t rxStatusFlags ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	BKNI_Memset(pstScrambleConfig, 0, sizeof(BHDM_ScrambleConfig)) ;

	/* read the TX STB Scrambling configuration */
	Register = BREG_Read32(hRegister, BCHP_HDMI_SCRAMBLER_CTL + ulOffset) ;
	pstScrambleConfig->txScrambleEnable =
		BCHP_GET_FIELD_DATA(Register, HDMI_SCRAMBLER_CTL, ENABLE) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CLK_WORD_SEL + ulOffset) ;
	pstScrambleConfig->clockWordSelect =
		BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_TMDS_CLK_WORD_SEL, CLK_WORD_SEL) ;

	/* read the RX Scrambling configuriation (if supported by the Rx) */
	rxStatusFlags = 0 ;
	if ((hHDMI->AttachedEDID.RxHdmiForumVsdb.exists)
	&& (hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport))
	{
		rc =  BHDM_P_BREG_I2C_Read(hHDMI,
			BHDM_AUTO_I2C_P_SCDC_SLAVE_ADDR, BHDM_SCDC_SCRAMBLER_STATUS,
			&rxStatusFlags, 1) ;
		if (rc) {BERR_TRACE(rc); return ;}
	}

	pstScrambleConfig->rxStatusFlags_Scramble =
		rxStatusFlags & BHDM_SCDC_SCRAMBLER_STATUS_MASK_SCRAMBLING_STATUS ;

	/* report/store changes to Scramble Configuration */
	if (BKNI_Memcmp(&hHDMI->ScrambleConfig, pstScrambleConfig, sizeof(BHDM_ScrambleConfig)))
	{
		BDBG_LOG(("SCDC Scrambling Config (Tx/Rx) : %s/%s",
			pstScrambleConfig->txScrambleEnable ? "Yes" : "No",
			pstScrambleConfig->rxStatusFlags_Scramble ? "Yes" : "No")) ;

		hHDMI->ScrambleConfig = *pstScrambleConfig ;
	}
}


void BHDM_SCDC_DisableScrambleTx(BHDM_Handle hHDMI)
{
	BHDM_ScrambleConfig scrambleSettings ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(&scrambleSettings, 0, sizeof(BHDM_ScrambleConfig)) ;
	BHDM_SCDC_P_ConfigureScramblingTx_isrsafe(hHDMI, &scrambleSettings) ;

	/* enable Auto I2c only if Scrambling is being turned on */
	BHDM_AUTO_I2C_EnableReadChannel(hHDMI,
		BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0, 0) ;
}

void BHDM_SCDC_P_ReadStatusUpdates_isr(BHDM_Handle hHDMI)
{
	BERR_Code rc ;
	uint8_t slaveAddress, slaveOffset, length ;
	uint8_t maskBuffer ;

	slaveAddress = BHDM_AUTO_I2C_P_SCDC_SLAVE_ADDR ;

	/* TODO For Bringup process STATUS_UPDATE only; skip RR and CED */
	/* configure a separate read for the updated data indicated by the status */
	if (hHDMI->ScdcBuffer[0] & BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_STATUS)
	{
		slaveOffset = BHDM_AUTO_I2C_P_SCDC_STATUS_FLAGS_OFFSET ;
		length = BHDM_AUTO_I2C_P_SCDC_STATUS_FLAGS_LENGTH ;

		maskBuffer = BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_STATUS ;
		hHDMI->ePendingReadType = BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Status_Flags ;
	}
	else if (hHDMI->ScdcBuffer[0] & BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_CED)
	{
		slaveOffset = 	BHDM_AUTO_I2C_P_SCDC_CED_OFFSET ;
		length = BHDM_AUTO_I2C_P_SCDC_CED_LENGTH ;

		maskBuffer = BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_CED ;
		hHDMI->ePendingReadType = BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Err_Det ;
	}
	else if (hHDMI->ScdcBuffer[0] & BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_RR_Test)
	{
		slaveOffset = 	BHDM_AUTO_I2C_P_SCDC_TEST_CONFIG_0_OFFSET ;
		length = BHDM_AUTO_I2C_P_SCDC_TEST_CONFIG_0_LENGTH ;

		maskBuffer = BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_RR_Test ;
		hHDMI->ePendingReadType = BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Test_Config_0 ;
	}
	else if ((hHDMI->ScdcBuffer[0] == 0) && (hHDMI->ScdcBuffer[1] == 0))
	{
		BDBG_MSG(("All SCDC Updates have been processed...")) ;
		/* clear the update flag */
		goto done ;
	}
	else
	{
		BDBG_ERR(("Update Value of 0x%x is not supported", hHDMI->ScdcBuffer[0])) ;
		rc = BERR_TRACE(BERR_UNKNOWN) ;
		goto done ;
	}


	/* clear status that was indicated as updated */
	rc = BHDM_AUTO_I2C_P_ConfigureWriteChannel_isr(hHDMI,
		BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0,
		slaveAddress, slaveOffset, &maskBuffer, 1) ;
	if (rc)
	{
		BDBG_ERR(("Unable to Configure Write to clear SCDC Update")) ;
		rc = BERR_TRACE(rc) ;
	}

	/* now configure the Read Channel to read the actual data
	 * NOTE: the data can be read even if the previous ConfigureWriteChannel
	 * call to clear the status failed
	*/

	rc = BHDM_AUTO_I2C_P_ConfigureReadChannel_isr(hHDMI, BHDM_AUTO_I2C_MODE_eRead,
		slaveAddress, slaveOffset, length) ;
	if (rc)
	{
		BDBG_ERR(("Unable to Configure Read to get SCDC Update")) ;
		rc = BERR_TRACE(rc) ;
	}

done:
	return ;
}


/* function to read actual SCDC Status data that has been updated */
void BHDM_SCDC_P_ProcessUpdate_isr(const BHDM_Handle hHDMI)
{

	BDBG_ENTER(BHDM_SCDC_P_ProcessUpdate_isr) ;

	switch (hHDMI->ePendingReadType)
	{
	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_TMDS_Config :
		hHDMI->stStatusControlData.TMDSConfig = hHDMI->AutoI2CBuffer[0] ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Scrambler_Status :
		hHDMI->stStatusControlData.RxScramblerStatus = hHDMI->AutoI2CBuffer[0] ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Config_0 :
		hHDMI->stStatusControlData.Config_0 = hHDMI->AutoI2CBuffer[0] ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Status_Flags :
		hHDMI->stStatusControlData.StatusFlags_0 =
			(uint8_t) hHDMI->AutoI2CBuffer[0] ;
		hHDMI->stStatusControlData.StatusFlags_1 =
			(uint8_t) (hHDMI->AutoI2CBuffer[1]) ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Err_Det :
		hHDMI->stStatusControlData.ch[0].Err_L = hHDMI->AutoI2CBuffer[0] ;
		hHDMI->stStatusControlData.ch[0].Err_H = hHDMI->AutoI2CBuffer[1] ;
		hHDMI->stStatusControlData.ch[1].Err_L = hHDMI->AutoI2CBuffer[2] ;
		hHDMI->stStatusControlData.ch[1].Err_H = hHDMI->AutoI2CBuffer[3] ;
		hHDMI->stStatusControlData.ch[2].Err_L = hHDMI->AutoI2CBuffer[4] ;
		hHDMI->stStatusControlData.ch[2].Err_H = hHDMI->AutoI2CBuffer[5] ;
		hHDMI->stStatusControlData.ErrorDetectionChecksum
			= hHDMI->AutoI2CBuffer[6] ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Test_Config_0 :
		hHDMI->stStatusControlData.TestConfig_0 = (uint8_t) hHDMI->AutoI2CBuffer[0] ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eEDID :
		BDBG_ERR(("EDID Read incorrectly routed to SCDC Process Update function")) ;
		BERR_TRACE(BERR_INVALID_PARAMETER) ;
		break ;

	case BHDM_AUTO_I2C_P_READ_DATA_eNone :
	default :
		BDBG_WRN(("No or unknown pending SCDC Read %d", hHDMI->ePendingReadType)) ;
	}

	/* after reading the data; check if additional status must be read */
	hHDMI->ePendingReadType = BHDM_AUTO_I2C_P_READ_DATA_eNone ;
	BHDM_SCDC_P_ReadStatusUpdates_isr(hHDMI) ;

	BDBG_LEAVE(BHDM_SCDC_P_ProcessUpdate_isr) ;

}


/* function to read/copy the SCDC Update_N registers */
void BHDM_SCDC_P_ReadUpdate0Data_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, RegAddr, ulOffset ;
	uint32_t autoI2cChxOffset ;

	BDBG_ENTER(BHDM_SCDC_P_ReadUpdate0Data_isr) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	autoI2cChxOffset =
		BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS * BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0 ;

	RegAddr = BCHP_HDMI_TX_AUTO_I2C_CH0_RD_0 + ulOffset + autoI2cChxOffset ;

	Register = BREG_Read32(hRegister, RegAddr) ;
	hHDMI->stStatusControlData.Update_0 = (uint8_t) Register & 0x00000FF ;
	hHDMI->stStatusControlData.Update_1 = (uint8_t) ((Register & 0x0000FF00) >> 8) ;

	if (Register  == (uint32_t) 0)
	{
		BDBG_MSG(("No SCDC Updates Detected")) ;
		goto done ;
	}

	BHDM_SCDC_P_ReadStatusUpdates_isr(hHDMI) ;

done:
	BDBG_LEAVE(BHDM_SCDC_P_ReadUpdate0Data_isr) ;
}


void BHDM_SCDC_P_GetScrambleParams_isrsafe(const BHDM_Handle hHDMI, BHDM_ScrambleConfig *ScrambleSettings)
{

	BKNI_Memset(ScrambleSettings, 0,  sizeof(BHDM_ScrambleConfig)) ;

	/* if Rx does not support HDMI 2.0, no further configuration is required */
	/* but continue on so if scrambing was previously ON it can be turned OFF */
	if (hHDMI->AttachedEDID.RxHdmiForumVsdb.exists == false)
	{
		BDBG_MSG(("Attached Rx does not support HDMI 2.0")) ;
	}

	/* determine Scrambling Configuration */
	if (hHDMI->eTmdsMode == BHDM_P_TmdsMode_eHigh)
	{
		ScrambleSettings->rxTmdsConfig =
			  BHDM_SCDC_TMDS_CONFIG_MASK_SCRAMBLE
			| BHDM_SCDC_TMDS_CONFIG_MASK_BIT_CLOCK_RATIO  ;
		ScrambleSettings->txScrambleEnable = BHDM_SCDC_TMDS_CONFIG_MASK_SCRAMBLE;
		ScrambleSettings->clockWordSelect = 0x3 ;
	}
	/* else if scrambling support for Low TMDS Rates (i.e. < 340) */
	else if (hHDMI->AttachedEDID.RxHdmiForumVsdb.LTE_340MScrambleSupport)
	{
		ScrambleSettings->rxTmdsConfig =
			  BHDM_SCDC_TMDS_CONFIG_MASK_SCRAMBLE ;
		ScrambleSettings->txScrambleEnable = BHDM_SCDC_TMDS_CONFIG_MASK_SCRAMBLE ;
		ScrambleSettings->clockWordSelect = 0x0 ;
	}
	else
	{
		ScrambleSettings->rxTmdsConfig = 0x0 ;
		ScrambleSettings->txScrambleEnable = 0 ;
		ScrambleSettings->clockWordSelect = 0x0 ;
	}

}


static void BHDM_SCDC_P_ConfigureScramblingRx(const BHDM_Handle hHDMI)
{
	BERR_Code rc ;
	BHDM_ScrambleConfig ScrambleSettings ;

	BHDM_SCDC_P_GetScrambleParams_isrsafe(hHDMI, &ScrambleSettings) ;

	BDBG_MSG(("Set Scrambling RX:  %d <%s>",
		ScrambleSettings.rxTmdsConfig,
		ScrambleSettings.rxTmdsConfig ? "ON" : "OFF")) ;

	BDBG_MSG(("    Process Bit Clock Rate Change to eTmdsClock (%d) = %s  TmdsMode: %s  Rx TmdsConfig: %d",
		hHDMI->eTmdsClock, BHDM_P_TmdsClockToText_isrsafe(hHDMI->eTmdsClock),
		(hHDMI->eTmdsMode == BHDM_P_TmdsMode_eHigh) ? "High" : "Low",
		ScrambleSettings.rxTmdsConfig)) ;

	/* Writing to the Rx TMDS Bit Clock Ratio requires the TMDS lines to be disabled */
	/* turn off the TMDS lines prior to making the changes */
	BKNI_EnterCriticalSection() ;
		BHDM_P_EnableTmdsClock_isr(hHDMI, false) ;
		BHDM_P_EnableTmdsData_isr(hHDMI, false) ;
	BKNI_LeaveCriticalSection() ;

	/* remember the TMDS lines are temporarily disabled
	for the write of the BitClockRatio setting in the HDMI Rx */
	hHDMI->TmdsDisabledForBitClockRatioChange = true ;

	rc =  BREG_I2C_Write(hHDMI->hI2cRegHandle,
		BHDM_AUTO_I2C_P_SCDC_SLAVE_ADDR, BHDM_SCDC_TMDS_CONFIG,
		&ScrambleSettings.rxTmdsConfig, 1) ;
	if (rc)
	{
		rc = BERR_TRACE(rc) ;
		goto done ;
	}

	/* Allow the Rx to process the Scrambling Request prior to enabling/disabling Tx Scrambling */
	rc = BTMR_StartTimer(hHDMI->TimerTxScramble, BHDM_P_MILLISECOND * 75) ;
	if (rc)
	{
		rc = BERR_TRACE(rc) ;
		goto done ;
	}

done:
	return ;
}


void BHDM_SCDC_P_ConfigureScramblingTx_isrsafe(
	BHDM_Handle hHDMI, BHDM_ScrambleConfig *pstScrambleConfig)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	BDBG_MSG(("Set Scrambling TX:  %d <%s>",
		pstScrambleConfig->txScrambleEnable,
		pstScrambleConfig->txScrambleEnable ? "ON" : "OFF")) ;

	BDBG_MSG(("   SCDC TMDS Config: %x ; Clock Word Select: %x",
		pstScrambleConfig->rxTmdsConfig, pstScrambleConfig->clockWordSelect)) ;

	/* next configure (or clear) the STB Scrambling configuration */
	Register = BREG_Read32(hRegister, BCHP_HDMI_SCRAMBLER_CTL + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_SCRAMBLER_CTL, ENABLE) ;
		Register |= BCHP_FIELD_DATA(HDMI_SCRAMBLER_CTL, ENABLE, pstScrambleConfig->txScrambleEnable) ;
	BREG_Write32(hRegister, BCHP_HDMI_SCRAMBLER_CTL + ulOffset, Register) ;

	/* finally set Clock Word Select as needed */
	/* first <<<OVERRIDE>>> the incorrect default value of 0x3F0 with 0x3E0 */
	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CFG_0 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_2) ;
		Register |= BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_2, 0x3E0) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CFG_0 + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CLK_WORD_SEL + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_TX_PHY_TMDS_CLK_WORD_SEL, CLK_WORD_SEL) ;
		Register |= BCHP_FIELD_DATA(
			HDMI_TX_PHY_TMDS_CLK_WORD_SEL, CLK_WORD_SEL, pstScrambleConfig->clockWordSelect ) ;
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CLK_WORD_SEL + ulOffset, Register) ;

	hHDMI->ScrambleConfig = *pstScrambleConfig ;


	if (hHDMI->DeviceSettings.bEnableScdcMonitoring)
	{
		BDBG_MSG(("%s Auto I2c Channel %d for SCDC",
			pstScrambleConfig->txScrambleEnable ? "Enable" : "Disable",
			BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0)) ;

		/* enable Auto I2c only if Scrambling is being turned on */
		BHDM_AUTO_I2C_EnableReadChannel(hHDMI,
			BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0, pstScrambleConfig->txScrambleEnable) ;
	}
}


BERR_Code BHDM_SCDC_ConfigureScrambling(const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_SCDC_ConfigureScrambling);

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make sure Tx is not scrambling from a previous connection */
	BHDM_SCDC_DisableScrambleTx(hHDMI) ;

	if ((!hHDMI->AttachedEDID.RxHdmiForumVsdb.exists)
	|| (hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport == 0))
	{
		BDBG_MSG(("Attached Rx does not support SCDC (Status and Control Data Channel)")) ;

		goto done ;
	}

	BHDM_SCDC_P_ConfigureScramblingRx(hHDMI) ;

done:
	BDBG_LEAVE(BHDM_SCDC_ConfigureScrambling);
	return rc;
}
#endif
