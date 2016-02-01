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
* API Description:
*   API name: Frontend Analog
*    Private APIs for analog frontend access.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_frontend.h"

#ifndef NEXUS_FRONTEND_ANALOG_PRIV_H__
#define NEXUS_FRONTEND_ANALOG_PRIV_H__

/* Hook for VDEC to inform IFD of current video format */
void NEXUS_Frontend_NotifyAnalogVideoFormat_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_VideoFormat format);

/* Hook for AASD to retrieve current video format */
void NEXUS_Frontend_GetAnalogVideoFormat_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_VideoFormat *pFormat);

/* Hook for AASD to drive format switches */
NEXUS_Error NEXUS_Frontend_SetAnalogAVFormat_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_VideoFormat videoFormat,
    NEXUS_IfdAudioMode audioMode,
    bool carriersOnly);

/* Hook for frontend->AASD interrupts */
typedef enum NEXUS_FrontendAnalogAudioInterrupt
{
    NEXUS_FrontendAnalogAudioInterrupt_eTune,           /* New analog tune requested */
    NEXUS_FrontendAnalogAudioInterrupt_eUntune,         /* Tuned away (e.g. to digital) */
    NEXUS_FrontendAnalogAudioInterrupt_eFormatUpdate,   /* Video format update */
    NEXUS_FrontendAnalogAudioInterrupt_eMax
} NEXUS_FrontendAnalogAudioInterrupt;

typedef void (*NEXUS_FrontendAnalogAudioInterruptFunc)(void *pParam, NEXUS_FrontendAnalogAudioInterrupt interrupt);

void NEXUS_Frontend_SetAnalogAudioInterrupt_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendAnalogAudioInterruptFunc interruptFunc_isr,
    void *pFuncParam);

#endif /* #ifndef NEXUS_FRONTEND_ANALOG_PRIV_H__ */

