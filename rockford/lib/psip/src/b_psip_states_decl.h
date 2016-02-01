/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
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
* Description:
*   API name: PSIP
*    Library function declarations for state machine.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_PSIP_STATES_DECL_H__
#define B_PSIP_STATES_DECL_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
GetTable State function declarations
***************************************************************************/
void B_PSIP_State_GetTable(B_PSIP_Job * pJob, B_PSIP_StateEvent event);

/***************************************************************************
Summary:
GetChList State function declarations
***************************************************************************/
void B_PSIP_State_GetChList_GetVCT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetChList_GetChData(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetChList_GetPidsPSI_PAT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetChList_GetPidsPSI_PMT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetChList_Done(B_PSIP_Job * pJob, B_PSIP_StateEvent event);

/***************************************************************************
Summary:
GetProgInfo State function declarations
***************************************************************************/
void B_PSIP_State_GetProgInfo_GetSTT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_GetVCT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_GetSourceId(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_GetMGT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_SearchMGT4EIT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_GetEIT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_SearchEIT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_SearchMGT4ETT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_GetETT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_SaveProgram(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetProgInfo_Done(B_PSIP_Job * pJob, B_PSIP_StateEvent event);

/***************************************************************************
Summary:
GetTableVer State function declarations
***************************************************************************/
void B_PSIP_State_GetTableVer_GetMGT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetTableVer_Done(B_PSIP_Job * pJob, B_PSIP_StateEvent event);

/***************************************************************************
Summary:
GetTime State function declarations
***************************************************************************/
void B_PSIP_State_GetTime_GetSTT(B_PSIP_Job * pJob, B_PSIP_StateEvent event);
void B_PSIP_State_GetTime_Done(B_PSIP_Job * pJob, B_PSIP_StateEvent event);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_STATES_DECL_H__ */

