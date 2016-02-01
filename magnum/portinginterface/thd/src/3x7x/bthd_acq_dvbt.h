/******************************************************************************
*     (c)2010-2013 Broadcom Corporation
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
 *****************************************************************************/
/***************************************************************************
*     (c)2005-2013 Broadcom Corporation
*  
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
#ifndef _BTHD_ACQ_DVBT_H__
#define _BTHD_ACQ_DVBT_H__

#if __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Function prototypes
 ***************************************************************************/
void BTHD_P_DvbtSetMode(BTHD_3x7x_ChnHandle,THD_TransmissionMode_t,THD_GuardInterval_t,THD_Qam_t,THD_CodeRate_t,THD_CodeRate_t,THD_DvbtHierarchy_t);
BTHD_RESULT BTHD_P_DvbtSetTPS(BTHD_3x7x_ChnHandle,THD_TransmissionMode_t,THD_GuardInterval_t);
void BTHD_P_DvbtSetOI(BTHD_3x7x_ChnHandle);
void BTHD_P_DvbtSetEq(BTHD_3x7x_ChnHandle,THD_CoChannelMode_t);
void BTHD_P_DvbtSetViterbi(BTHD_3x7x_ChnHandle,THD_CodeRate_t,THD_CodeRate_t);
BTHD_RESULT BTHD_P_DvbtSetFEC(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_DvbtGetNotch(BTHD_3x7x_ChnHandle,THD_TransmissionMode_t);
BTHD_RESULT BTHD_P_DvbtSetICE(BTHD_3x7x_ChnHandle,THD_TransmissionMode_t,THD_GuardInterval_t);
void BTHD_P_DvbtSetFrame(BTHD_3x7x_ChnHandle);
void BTHD_P_DvbtStatus(BTHD_3x7x_ChnHandle) ;
void BTHD_P_DvbtResetStatus(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_DvbtAcquire(BTHD_3x7x_ChnHandle);
BERR_Code BTHD_P_GetDvbtSoftDecisionBuf(BTHD_3x7x_ChnHandle, int16_t, int16_t *, int16_t *, int16_t *); 
void BTHD_P_DvbtSetFWFtt( BTHD_3x7x_ChnHandle,THD_FFTWindowMode_t,THD_TransmissionMode_t,THD_GuardInterval_t);

#ifdef __cplusplus
}
#endif

#endif /* BTHD_ACQ_DVBT_H__ */

