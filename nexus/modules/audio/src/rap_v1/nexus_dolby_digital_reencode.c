/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
*   API name: DolbyDigitalReencode
*    Specific APIs related to Dolby Digital Reencoding used in Dolby MS11
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_dolby_digital_reencode);

void NEXUS_DolbyDigitalReencode_GetDefaultSettings(
    NEXUS_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    NEXUS_Ac3Encode_GetDefaultSettings(&pSettings->encodeSettings);
}

NEXUS_DolbyDigitalReencodeHandle NEXUS_DolbyDigitalReencode_Open(
    const NEXUS_DolbyDigitalReencodeSettings *pSettings
    )
{
	BSTD_UNUSED(pSettings);
	(void)BERR_TRACE(BERR_NOT_SUPPORTED);
	return NULL;
}

void NEXUS_DolbyDigitalReencode_Close(
    NEXUS_DolbyDigitalReencodeHandle handle
    )
{
	BSTD_UNUSED(handle);
}

void NEXUS_DolbyDigitalReencode_GetSettings(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    )
{
	BSTD_UNUSED(handle);
	BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_DolbyDigitalReencode_SetSettings(
    NEXUS_DolbyDigitalReencodeHandle handle,
    const NEXUS_DolbyDigitalReencodeSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_AudioInput NEXUS_DolbyDigitalReencode_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_DolbyDigitalReencodeConnectorType connectorType
    )
{
	BSTD_UNUSED(handle);
	BSTD_UNUSED(connectorType);
	(void)BERR_TRACE(BERR_NOT_SUPPORTED);
	return NULL;
}

NEXUS_Error NEXUS_DolbyDigitalReencode_AddInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInput input
    )
{
	BSTD_UNUSED(handle);
	BSTD_UNUSED(input);
	return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveAllInputs(
    NEXUS_DolbyDigitalReencodeHandle handle
    )
{
	BSTD_UNUSED(handle);
	return BERR_TRACE(BERR_NOT_SUPPORTED);
}

