/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: AudioCrc
*    CRC capture for Audio data
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_crc);

/***************************************************************************
Summary:
    Get Default CRC Open Settings

Description:
***************************************************************************/
void NEXUS_AudioCrc_GetDefaultOpenSettings(
    NEXUS_AudioCrcOpenSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Open Audio CRC 

Description:
    Open an Audio CRC.  Audio chain must be stopped to 
    perform this operation.
***************************************************************************/
NEXUS_AudioCrcHandle NEXUS_AudioCrc_Open(
    unsigned index,                                 /* index to CRC instance */
    const NEXUS_AudioCrcOpenSettings *pSettings     /* attr{null_allowed=y} Pass NULL for default settings */
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
    Close Audio CRC 

Description:
    Close this CRC.  Audio chain must be stopped, and CRC input must be 
    removed to perform this operation.
***************************************************************************/
void NEXUS_AudioCrc_Close(
    NEXUS_AudioCrcHandle handle
    )
{
    BSTD_UNUSED(handle);
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Get Default CRC Input Settings

Description:
***************************************************************************/
void NEXUS_AudioCrc_GetDefaultInputSettings(
    NEXUS_AudioCrcInputSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Set the input for this CRC 

Description:
    Add the input for this CRC.  Audio chain must be stopped to 
    perform this operation.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_SetInput(
    NEXUS_AudioCrcHandle handle,
    const NEXUS_AudioCrcInputSettings * pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Clear the input from this CRC 

Description:
    Removes the input from this CRC.  Audio chain should be stopped to 
    perform this operation.
***************************************************************************/
void NEXUS_AudioCrc_ClearInput(
    NEXUS_AudioCrcHandle handle
    )
{
    BSTD_UNUSED(handle);
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Get a buffer descriptor containing the CRC buffer(s).

Description:
    Get CRC entries. numEntries will always be populated ( >= 0 ), 
    even if CRC input is not connected. An error will also be
    returned if there is no active CRC input.

    NEXUS_AudioCrc_GetCrcEntries is non-destructive. You can safely call it 
    multiple times.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_GetCrcData(
    NEXUS_AudioCrcHandle handle,
    NEXUS_AudioCrcData *pData, /* [out] attr{nelem=numEntries;nelem_out=pNumReturned} pointer to CRC entries. */
    unsigned numEntries,
    unsigned *pNumReturned
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pData);
    BSTD_UNUSED(numEntries);
    BSTD_UNUSED(pNumReturned);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


