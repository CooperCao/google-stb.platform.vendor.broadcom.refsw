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
*    Library typedefs for PSIP getproginfo state
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_PSIP_GETPROGINFO_H__
#define B_PSIP_GETPROGINFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Temporary data necessary to execute the B_PSIP_GetProgInfo() api.
***************************************************************************/
typedef struct B_PSIP_StateData_GetProgInfo
{
    uint8_t        * sttBuffer;             /* Dynamically allocated/freed by state machine */
    uint32_t         sttBufferLen;          /* Size of temp STT buffer */
    uint8_t        * vctBuffer;             /* Dynamically allocated/freed by state machine */
    uint32_t         vctBufferLen;          /* Size of temp VCT buffer */
    uint8_t        * mgtBuffer;             /* Dynamically allocated/freed by state machine */
    uint32_t         mgtBufferLen;          /* Size of temp MGT buffer */
    uint8_t        * eitBuffer;             /* Dynamically allocated/freed by state machine */
    uint32_t         eitBufferLen;          /* Size of temp EIT buffer */
    uint8_t        * ettBuffer;             /* Dynamically allocated/freed by state machine */
    uint32_t         ettBufferLen;          /* Size of temp ETT buffer */
    B_PSIP_Program * progList;              /* User provided list of programs to be filled in by API */
    uint16_t       * pLength;               /* User provided size of list of programs.  This will be replaced by the number of programs found on return. */
    B_PSIP_Channel   currentCh;             /* Currently found channel data to be copied to chList as state machine progresses. */
    B_PSIP_Program   currentProg;           /* Currently found program data to be copied to progList as state machine progresses. */
    uint16_t         progsFound;            /* Running count of number of programs found */
    uint8_t          sectionIndexVCT;       /* Current VCT section we are searching */
    uint8_t          sectionIndexMaxVCT;    /* Maximum number of VCT sections to search */
    uint8_t          sectionIndexEIT;       /* Current EIT section we are searching */
    uint8_t          sectionIndexMaxEIT;    /* Maximum number of EIT sections to search */
    uint16_t         pidEIT;                /* PID of current EIT to search */
    uint16_t         pidETT;                /* PID of current ETT to search */
    uint16_t         channelIndex;          /* current channel index in VCT */
    uint16_t         programIndex;          /* current program index in EIT */
    uint16_t         tableIndex;            /* current MGT table index */
    uint16_t         eventIndex;            /* current EIT/ETT table index */
    B_Time           timeEITStart;          /* time when EIT table collection begins */
    uint32_t         timeUTCStart;          /* start time associated with search for program data */
    uint32_t         timeUTCEnd;            /* end time associated with search for program data */
} B_PSIP_StateData_GetProgInfo;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_GETPROGINFO_H__ */


