/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*****************************************************************************/
#ifndef BTNR_OB_3x7x_H__
#define BTNR_OB_3x7x_H__

#include "bchp.h"
#include "btnr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned short i2cAddr;             /* 7bit I2C address of Bcm7550 */
	BTMR_Handle hTmr;
	BMEM_Heap_Handle hHeap;
} BTNR_Ob_3x7x_Settings;

/***************************************************************************
Summary:
    This function returns the default settings for Bcm3x7x Tuner module.

Description:
    This function is responsible for returns the default setting for 
    BTNR module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BTNR_Ob_3x7x_Open()

****************************************************************************/
BERR_Code BTNR_Ob_3x7x_GetDefaultSettings(
    BTNR_Ob_3x7x_Settings *pDefSettings  /* [out] Returns default setting */
    );


/***************************************************************************
Summary:
    This function opens Bcm3x7x Tuner module.

Description:
    This function is responsible for opening Bcm3x7x BTNR module. When BTNR is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BTNR_Ob_Close(), BTNR_Ob_3x7x_GetDefaultSettings()

****************************************************************************/

BERR_Code BTNR_Ob_3x7x_Open(BTNR_Handle *phDev, 
		BTNR_Ob_3x7x_Settings *pSettings, 
		BREG_Handle hRegister);


/***************************************************************************
Summary:
    Function called once the event is sent to upper layer

	BTNR_Ob_3x7x_ProcessInterruptEvent
****************************************************************************/
BERR_Code BTNR_Ob_3x7x_ProcessInterruptEvent(BTNR_Handle hDev);

/***************************************************************************
Summary:
    Function called by upper to get the inetrrupt handle

	BTNR_Ob_3x7x_GetInterruptEventHandle
****************************************************************************/
BERR_Code BTNR_Ob_3x7x_GetInterruptEventHandle(BTNR_Handle h, BKNI_EventHandle* hEvent);


/***************************************************************************
Summary:
	BTNR_Ob_3x7x_P_TimerFunc_isr
****************************************************************************/
BERR_Code BTNR_Ob_3x7x_P_TimerFunc_isr(void *myParam1, int myParam2);


typedef struct
{
	int32_t		            Total_Mix_After_ADC;    /*Sum of mixer frequencies after ADC on eRequestMode*/
	int16_t		            PreADC_Gain_x256db ;    /*Gain in db*256 before ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t		            PostADC_Gain_x256db;    /*Gain in db*256 after ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t		            External_Gain_x256db;   /*Gain in db*256 external to chip (like external LNA) on eRequestMode: set to 0x8000 if unknown*/
} BTNR_Ob_3x7x_RfStatus_t;

/******************************************************************************
  BTNR_3x7x_Ob_Get_RF_Status()
  callback from Demod to Tuner through above layer
 ******************************************************************************/
BERR_Code BTNR_Ob_3x7x_Get_RF_Status(BTNR_Handle hTnrDev, BTNR_Ob_3x7x_RfStatus_t *RfCallbackStatus);



#ifdef __cplusplus
}
#endif
 
#endif



