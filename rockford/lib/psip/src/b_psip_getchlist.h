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
*    Library typedefs for PSIP getchlist state
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_PSIP_GETCHLIST_H__
#define B_PSIP_GETCHLIST_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Temporary data necessary to execute the B_PSIP_GetChannelList() 
    and B_PSIP_GetNumChannels() apis.
***************************************************************************/
typedef struct B_PSIP_StateData_GetChList
{
    uint8_t                * vctBuffer;         /* Dynamically allocated/freed by state machine */
    uint32_t                 vctBufferLen;      /* Size of temp VCT buffer */
    uint8_t                * patBuffer;         /* Dynamically allocated/freed by state machine */
    uint32_t                 patBufferLen;      /* Size of temp PAT buffer */
    uint8_t                * pmtBuffer;         /* Dynamically allocated/freed by state machine */
    uint32_t                 pmtBufferLen;      /* Size of temp PMT buffer */
    B_PSIP_Channel         * chList;            /* User provided list of channels to be filled in by API */
    uint16_t               * pLength;           /* User provided size of list of channels.  This will be replaced by the number of channels found on return. */
    B_PSIP_Channel           currentCh;         /* Currently found channel data to be copied to chList as state machine progresses. */
    uint16_t                 channelsFound;     /* Running count of number of channels found */
    uint16_t                 channelsSaved;     /* Running count of number of channels saved in list */
    uint8_t                  sectionIndex;      /* Current section we are searching */
    uint8_t                  sectionIndexMax;   /* Maximum number of sections to search */
    uint8_t                  channelIndex;      /* current channel index */
    B_PSIP_ChannelListType   type;
} B_PSIP_StateData_GetChList;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_GETCHLIST_H__ */


