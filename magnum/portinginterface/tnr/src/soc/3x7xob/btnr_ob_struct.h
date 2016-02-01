/******************************************************************************
 *    (c)2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *****************************************************************************/

#ifndef _BTNR_OB_STRUCT_H__
#define _BTNR_OB_STRUCT_H__

#if __cplusplus
extern "C" {
#endif



typedef enum BTNR_Ob_BBSConnectMode_s
{
  BTNR_Ob_BBSConnectMode_Tune = 1, 
  BTNR_Ob_BBSConnectMode_ResetStatus = 2,
  BTNR_Ob_BBSConnectMode_EnableStatus = 4
} BTNR_Ob_BBSConnectMode_t;


typedef struct BTNR_Ob_3x7x_BBS_Params_t
{	
	uint32_t							    StartSturctureAddress;
	uint32_t								rfFreq;
	volatile BTNR_Ob_BBSConnectMode_t		BBSConnectMode;
	uint32_t								BBSAddress;
}BTNR_Ob_3x7x_BBS_Params_t;

/*This is the main structure by the tuning functions*/
typedef struct BTNR_Ob_3x7x_TuneParams_s
{
	BTNR_Ob_3x7x_BBS_Params_t		 BTNR_Ob_BBS_Params;
}BTNR_Ob_3x7x_TuneParams_t;



typedef enum BTNR_Ob_PowerStatus_s
{
    BTNR_Ob_ePower_Off = 0,
    BTNR_Ob_ePower_On  = 1,
    BTNR_Ob_ePower_Unknown = 0xFF
}  BTNR_Ob_PowerStatus_t;



/*This is the main structure by the tuning status*/
typedef struct BTNR_Ob_3x7x_TuneStatus_s
{
	BTNR_Ob_PowerStatus_t					PowerStatus;
	int32_t									Total_Mix_After_ADC;    /*Sum of mixer frequencies after ADC on eRequestMode*/
	int16_t									PreADC_Gain_x256db ;    /*Gain in db*256 before ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t									PostADC_Gain_x256db;    /*Gain in db*256 after ADC on eRequestMode: set to 0x8000 if unknown*/
	int16_t									External_Gain_x256db;   /*Gain in db*256 external to chip (like external LNA) on eRequestMode: set to 0x8000 if unknown*/

} BTNR_Ob_3x7x_TuneStatus_t;


#ifdef __cplusplus
}
#endif

#endif /* _BTNR_STRUCT_H__ */





