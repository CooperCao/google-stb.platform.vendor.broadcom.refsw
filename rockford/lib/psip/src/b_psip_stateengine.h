/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* Description:
*   API name: PSIP
*    Library typedefs and macros for PSIP state machine 
*
***************************************************************************/
#ifndef B_PSIP_STATEENGINE_H__
#define B_PSIP_STATEENGINE_H__

#ifdef __cplusplus
extern "C" {
#endif

void B_PSIP_StateEngine(
	B_PSIP_Job * pJob, 
	B_PSIP_StateEvent event, 
	const char * func, 
	const uint32_t line);

/***************************************************************************
Summary:
Macro to advance the state engine.
***************************************************************************/
#define STATE_ENGINE(pJob,event) B_PSIP_StateEngine((pJob),(event),BSTD_FUNCTION,__LINE__)

/***************************************************************************
Summary:
Macro to change the current state in the state machine.
***************************************************************************/
#define CHANGE_STATE(pJob,newState) do { \
                                        (pJob)->stateLast = (pJob)->state; \
                                        (pJob)->state = (newState); \
                                        BDBG_MSG(("job:%p ---> %s:%d - change from state:%d to state:%d", \
                                                  (void*)pJob, BSTD_FUNCTION, __LINE__, (pJob)->stateLast, (pJob)->state)); \
                                    }while(0)

/* #define CHANGE_STATE(pJob,newState) ((pJob)->state = (newState)) */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_STATEENGINE_H__ */

