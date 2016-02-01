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
*    Library functions for getting PSIP table data.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_PSIP_TABLE_H__
#define B_PSIP_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

B_Error B_PSIP_GetPAT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetPMT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetMGT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetTVCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetCVCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetSTT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetEIT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint16_t                sourceId,
    uint8_t                 section,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetETT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint16_t                pid,
    uint16_t                sourceId,
    uint16_t                eventId,
    B_PSIP_EtmIdType        etmType,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetRRT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetDCCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

B_Error B_PSIP_GetDCCSCT_P(
    B_PSIP_ApiSettings    * pApiSettings,
    uint8_t               * pBuffer,
    uint32_t              * pBufferLength,
    B_PSIP_Job            * pJob
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_PSIP_TABLE_H__ */


