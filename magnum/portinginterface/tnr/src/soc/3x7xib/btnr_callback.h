/*************************************************************************
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* [File Description:]
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BTNR_CALLBACK_ADS_H__
#define _BTNR_CALLBACK_ADS_H__

#if __cplusplus
extern "C" {
#endif

/*API Functions in bwfe_callback_to_ads.c*/
BERR_Code BTNR_P_Set_RF_Offset(BTNR_3x7x_ChnHandle hChn, int32_t RF_Offset, uint32_t Symbol_Rate);
BERR_Code BTNR_P_Get_RF_Status(BTNR_3x7x_ChnHandle hChn);
BERR_Code BTNR_P_LNA_AGC_Power(BTNR_3x7x_ChnHandle hTnr, uint16_t Mode);

#ifdef __cplusplus
}
#endif

#endif /* _BWFE_CALLACK_TO_ADS_H__ */