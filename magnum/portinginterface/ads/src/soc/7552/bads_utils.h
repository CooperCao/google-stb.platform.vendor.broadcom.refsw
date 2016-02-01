/***************************************************************************
 *     (c)2005-2013 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 ***************************************************************************/
#ifndef BADS_UTILS_H__
#define BADS_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * ADS Function Prototypes Used Local 
 *****************************************************************************/
BERR_Code BADS_P_Range_Check(BADS_3x7x_ChannelHandle);
BERR_Code BADS_P_Set_CFL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_HS_u32, int32_t CFL_Frequency_i32);
BERR_Code BADS_P_Set_TL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_1S_u32, uint32_t Symbol_Rate);
BERR_Code BADS_P_Get_CFL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_HS_u32, int32_t *CFL_Frequency_pi32);
BERR_Code BADS_P_Get_TL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_1S_u32, uint32_t *TL_Frequency_pu32);
BERR_Code BADS_P_Get_CFL_Error(BADS_3x7x_ChannelHandle hChn, uint32_t F_HS_u32, int32_t *CFL_Error_pi32);
BERR_Code BADS_P_Get_VID_Error(BADS_3x7x_ChannelHandle hChn, uint32_t F_1S_u32, int32_t *VID_Error_pi32);
BERR_Code BADS_P_Get_CPL_Error(BADS_3x7x_ChannelHandle hChn, uint32_t Symbol_Rate_u32, int32_t *CPL_Error_pi32);
BERR_Code BADS_P_Get_TimingScan_Advanced_FFT(BADS_3x7x_ChannelHandle hChn, uint32_t Upper_Baud_Search, bool ReturnBin, uint32_t *TimingScanResult_pu32);
BERR_Code BADS_P_Get_CarrierScan_Advanced_FFT(BADS_3x7x_ChannelHandle hChn, uint32_t Symbol_Rate, int32_t *CarrierScanResult_pi32);
void BADS_P_ProgramFEC(BADS_3x7x_ChannelHandle hChn);
void BADS_P_AcquisitionPercentageTest(BADS_3x7x_ChannelHandle hChn);
BERR_Code BADS_P_Set_CWC_Auto(BADS_3x7x_ChannelHandle hChn, uint32_t Symbol_Rate, int32_t CWC_Offset_Freq, uint8_t *CWC_LengthResult_pu8);
void BADS_P_Get_AcquisitionScan_Settings(BADS_3x7x_ChannelHandle hChn);
void BADS_P_Set_ScanStatus_Params(BADS_3x7x_ChannelHandle hChn, uint8_t QamIndex_u8, uint8_t FECIndex_u8, int32_t CarrierOffset_i32, int32_t Carrier_Error_i32, uint32_t Symbol_Rate_u32); 
bool BADS_P_ADS_SLEEP(BADS_3x7x_ChannelHandle hChn, unsigned int Delay);	

#ifdef __cplusplus
}
#endif

#endif