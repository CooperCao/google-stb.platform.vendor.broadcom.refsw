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
#ifndef _BTHD_ACQ_ISDBT_H__
#define _BTHD_ACQ_ISDBT_H__

#if __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Function prototypes
 ***************************************************************************/

BTHD_RESULT BTHD_P_IsdbtSetTMCC(BTHD_3x7x_ChnHandle,THD_TransmissionMode_t,THD_GuardInterval_t);
void BTHD_P_IsdbtSetOI(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_IsdbtSetFEC(BTHD_3x7x_ChnHandle);
BTHD_RESULT BTHD_P_IsdbtGetNotch(BTHD_3x7x_ChnHandle,THD_TransmissionMode_t);
BTHD_RESULT BTHD_P_IsdbtSetICE(BTHD_3x7x_ChnHandle,THD_TransmissionMode_t,THD_GuardInterval_t);
void BTHD_P_IsdbtStatus(BTHD_3x7x_ChnHandle) ;
void BTHD_P_IsdbtResetStatus(BTHD_3x7x_ChnHandle);
void BTHD_P_IsdbtResetLockSetClrFlag(BTHD_3x7x_ChnHandle);
void BTHD_P_IsdbtSetRsRt(BTHD_3x7x_ChnHandle, uint32_t, uint32_t);
BTHD_RESULT BTHD_P_IsdbtAcquire(BTHD_3x7x_ChnHandle);
BERR_Code BTHD_P_GetIsdbtSoftDecisionBuf(BTHD_3x7x_ChnHandle, int16_t, int16_t *, int16_t *, int16_t *); 
void BTHD_P_IsdbtSetFWFtt( BTHD_3x7x_ChnHandle,THD_FFTWindowMode_t,THD_TransmissionMode_t,THD_GuardInterval_t);
void BTHD_P_SetFW( BTHD_3x7x_ChnHandle,THD_FFTWindowMode_t,THD_TransmissionMode_t,THD_GuardInterval_t);

#ifdef __cplusplus
}
#endif

#endif /* BTHD_ACQ_ISDBT_H__ */

