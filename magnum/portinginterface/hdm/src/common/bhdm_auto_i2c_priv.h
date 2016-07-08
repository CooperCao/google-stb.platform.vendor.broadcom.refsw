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

#ifndef BHDM_AUTO_I2C_PRIV_H__
#define BHDM_AUTO_I2C_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Auto I2c channels run 0..3 */
#define BHDM_AUTO_I2C_P_NUM_CHX_REGISTERS \
	(BCHP_HDMI_TX_AUTO_I2C_CH1_REG0_CFG - BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_CFG)

/* REGx run 0..13 BSCx */
#define BHDM_AUTO_I2C_P_NUM_CH_REGX_REGISTERS \
	(BCHP_HDMI_TX_AUTO_I2C_CH0_REG1_CFG - BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_CFG)


/* Custom I2C Channels that can be used for read/write */
typedef enum BHDM_AUTO_I2C_P_CHANNEL_OFFSET
{
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e0,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e1,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e2,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e3,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e4,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e5,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e6,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e7,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e8,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e9,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e10,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e11,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e12,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_e13,
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET_eMax
}  BHDM_AUTO_I2C_P_CHANNEL_OFFSET ;


#define BHDM_AUTO_I2C_P_HDCP22_SLAVE_ADDR 0x74
	#define BHDM_AUTO_I2C_P_HDCP22_RXSTATUS_OFFSET 0x70

#define BHDM_AUTO_I2C_P_SCDC_SLAVE_ADDR 0x54
	#define BHDM_AUTO_I2C_P_SCDC_UPDATE0_OFFSET 0x10
			#define BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_STATUS 0x01
			#define BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_CED 0x02
			#define BHDM_AUTO_I2C_P_SCDC_UPDATE0_MASK_RR_Test 0x04

#define BHDM_AUTO_I2C_P_SCDC_STATUS_FLAGS_OFFSET 0x40
	#define BHDM_AUTO_I2C_P_SCDC_STATUS_FLAGS_LENGTH 0x2

	#define BHDM_AUTO_I2C_P_SCDC_CED_OFFSET 0x50
		#define BHDM_AUTO_I2C_P_SCDC_CED_LENGTH 7

	#define BHDM_AUTO_I2C_P_SCDC_TEST_CONFIG_0_OFFSET 0xC0
		#define BHDM_AUTO_I2C_P_SCDC_TEST_CONFIG_0_LENGTH 0x1
		#define BHDM_AUTO_I2C_P_SCDC_TEST_CONFIG_0_OFFSET_MASK_DELAY 0x7F
		#define BHDM_AUTO_I2C_P_SCDC_TEST_CONFIG_0_OFFSET_MASK_READ_REQUEST 0x80

#define BHDM_AUTO_I2C_P_MAX_READ_BYTES 8
#define BHDM_AUTO_I2C_P_MAX_WRITE_BYTES 32  /* offset + 31 data bytes */

#define BHDM_AUTO_I2C_P_WRITE 0
#define BHDM_AUTO_I2C_P_READ 1
#define BHDM_AUTO_I2C_P_READWRITE 2
#define BHDM_AUTO_I2C_P_WRITEREAD 3

#define BHDM_AUTO_I2C_P_TRIGGER_BY_SW  0
#define BHDM_AUTO_I2C_P_TRIGGER_BY_TIMER 1

#define BHDM_AUTO_I2C_P_SCL_SEL_375 0
#define BHDM_AUTO_I2C_P_SCL_SEL_390 1
#define BHDM_AUTO_I2C_P_SCL_SEL_187 2
#define BHDM_AUTO_I2C_P_SCL_SEL_200 3

#define BHDM_AUTO_I2C_P_DIV_CLK_BY_1 0
#define BHDM_AUTO_I2C_P_DIV_CLK_BY_4 1


/******************************************************************************
Summary:
Enumerated Type of different Auto I2c Modes available
NOTE: These enums are mapped to the MODE field within the RDB register HDMI_AUTO_I2C_CH0_CFG

Description:
The HDMI core can support up to four automated I2c modes

See Also:
*******************************************************************************/
typedef enum BHDM_AUTO_I2C_MODE
{
	BHDM_AUTO_I2C_MODE_eWrite,
	BHDM_AUTO_I2C_MODE_eRead,
	BHDM_AUTO_I2C_MODE_ePollScdcUpdate0,
	BHDM_AUTO_I2C_MODE_ePollHdcp22RxStatus,
	BHDM_AUTO_I2C_MODE_eMax
} BHDM_AUTO_I2C_MODE ;


/******************************************************************************
Summary:
Enumerated Type of different Auto I2c Read Data available.  The type of data read is stored to properly
intepret the read data after the Auto I2C Read is completed

Description:

See Also:
*******************************************************************************/

typedef enum BHDM_AUTO_I2C_P_READ_DATA
{
	BHDM_AUTO_I2C_P_READ_DATA_eNone,
	BHDM_AUTO_I2C_P_READ_DATA_eSCDC_TMDS_Config,
	BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Scrambler_Status,
	BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Config_0,
	BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Status_Flags,
	BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Err_Det,
	BHDM_AUTO_I2C_P_READ_DATA_eSCDC_Test_Config_0,
	BHDM_AUTO_I2C_P_READ_DATA_eEDID,
	BHDM_AUTO_I2C_P_READ_DATA_eMax
} BHDM_AUTO_I2C_P_READ_DATA ;

/******************************************************************************
Summary:
Enumeration of BHDM AUTO I2C Interrupt Masks
*******************************************************************************/
#define	MAKE_AUTO_I2C_INTR_ENUM(IntName)	BHDM_AUTO_I2C_INTR_e##IntName
#define	MAKE_AUTO_I2C_INTR_NAME(IntName)	"BHDM_AUTO_I2C_" #IntName

typedef enum BHDM_AUTO_I2C_INTERRUPT_MASK
{
	MAKE_AUTO_I2C_INTR_ENUM(Hdcp22RxStatus),
	MAKE_AUTO_I2C_INTR_ENUM(ScdcUpdate),
	MAKE_AUTO_I2C_INTR_ENUM(Write),
	MAKE_AUTO_I2C_INTR_ENUM(Read),
	MAKE_AUTO_I2C_INTR_ENUM(LAST)
 } BHDM_AUTO_I2C_INTERRUPT_MASK ;


typedef struct BHDM_AUTO_I2C_P_TriggerConfiguration
{
	uint8_t timerMs ;
	uint8_t bscxDataOffset ;
	BHDM_AUTO_I2C_MODE eMode ;
	uint8_t triggerSource ;  /* RDB Write (0) or Timer (1) */
	uint8_t enable ;
	bool activePolling ;
} BHDM_AUTO_I2C_P_TriggerConfiguration ;



BERR_Code BHDM_AUTO_I2C_P_SetBSCx(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel,  /* channel: CH0, CH1, CH2, or CH3 */
	BHDM_AUTO_I2C_P_CHANNEL_OFFSET eRegOffset, /* CHy_REGx it can range from 0 to 13 */
	uint8_t bscxOffset, /* BSCx offset register can range from 0 to 21; Maps to BSCx registers*/
	uint32_t data /* data to be written */
) ;

BERR_Code  BHDM_AUTO_I2C_P_AllocateResources(BHDM_Handle hHDMI) ;
BERR_Code  BHDM_AUTO_I2C_P_FreeResources(BHDM_Handle hHDMI) ;
BERR_Code BHDM_AUTO_I2C_P_EnableInterrupts(BHDM_Handle hHDMI);
BERR_Code BHDM_AUTO_I2C_P_DisableInterrupts(BHDM_Handle hHDMI);

void BHDM_AUTO_I2C_EnableReadChannel_isr(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel, uint8_t enable
) ;
BERR_Code BHDM_AUTO_I2C_P_ConfigureReadChannel_isr(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_MODE eAutoI2cMode,
	uint8_t slaveAddress, uint8_t slaveOffset, uint8_t length) ;

BERR_Code BHDM_AUTO_I2C_P_ConfigureWriteChannel_isr(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel,
	uint8_t slaveAddress, uint8_t slaveOffset, uint8_t *pBuffer, uint8_t length) ;

void  BHDM_AUTO_I2C_P_SetTriggerConfiguration_isr(const BHDM_Handle hHDMI, BHDM_AUTO_I2C_P_CHANNEL eChannel,
	const BHDM_AUTO_I2C_P_TriggerConfiguration *pstTriggerConfig) ;

void BHDM_AUTO_I2C_P_GetTriggerConfiguration_isrsafe(const BHDM_Handle hHDMI, BHDM_AUTO_I2C_P_CHANNEL eChannel,
	BHDM_AUTO_I2C_P_TriggerConfiguration *pstTriggerConfig) ;

/******************************************************************************
Summary:
Set up an Auto I2C Channel for reading

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eChannel - Channel to configure
	eAutoI2cMode - Auto Mode to use
	slaveAddress - i2c chip address to read
	slaveOffset - offset within chip to start reading from
	length - number of bytes to read

Output:
	Depending on eAutoI2cMode Data is read into HDMI_AUTO_I2C registers

Returns:
	BERR_SUCCESS - Auto I2C Channel Successfully Configured
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	o BHDM_AUTO_I2C_P_GetReadData
	o BHDM_AUTO_I2C_GetScdcData
	o BHDM_AUTO_I2C_GetHdcpRxStatusData
*******************************************************************************/
BERR_Code BHDM_AUTO_I2C_P_ConfigureReadChannel(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_MODE eAutoI2cMode,
	uint8_t slaveAddress, uint8_t slaveOffset, uint8_t length) ;


/******************************************************************************
Summary:
Set up an Auto I2C Channel for writing.
NOTE: I2c writes are scheduled for one time only

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eChannel - Channel to configure
	slaveAddress - i2c chip address to write
	slaveOffset - offset within chip to start writing from
	pBuffer - pointer to pBuffer containing data to write
	length - number of bytes in pBuffer to write (max of 31 bytes + address)

Output:
	<None>

Returns:
	BERR_SUCCESS - Auto I2C write sccessfully Configured
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_AUTO_I2C_P_ChannelStart
	BHDM_AUTO_I2C_P_ChannelStop
*******************************************************************************/
BERR_Code BHDM_AUTO_I2C_P_ConfigureWriteChannel(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel,
	uint8_t slaveAddress, uint8_t slaveOffset, uint8_t *pBuffer, uint8_t length) ;



/******************************************************************************
Summary:
Get the Read the generic data read by Auto I2c

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eChannel - Channel to read from (previously configured)
	length - size of bytes to read (NOTE: length is ignored for Hdcp Rx data)

Output:
	pBuffer - contains data from the read

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER

See Also:
	BHDM_AUTO_I2C_SetChannelRead
*******************************************************************************/
BERR_Code BHDM_AUTO_I2C_P_GetReadData(BHDM_Handle hHDMI,
	uint8_t *pBuffer, uint8_t length) ;


#ifdef __cplusplus
}
#endif

#endif /* BHDM_AUTO_I2C_PRIV_H__ */
