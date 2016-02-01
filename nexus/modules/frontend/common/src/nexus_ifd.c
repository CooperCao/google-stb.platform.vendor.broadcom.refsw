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
*   API name: Frontend IFD
*    Generic APIs for an IF Demodulator, used demodulate an analog (NTSC/PAL)
*    input signal from a tuner device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_ifd);

/***************************************************************************
Summary:
	Get IFD Settings
See Also:
    NEXUS_Ifd_SetSettings
 ***************************************************************************/
void NEXUS_Ifd_GetSettings(
    NEXUS_IfdHandle handle,
    NEXUS_IfdSettings *pSettings    /* [out] */
    )
{
#if NEXUS_IFD_SUPPORT
#error not implemented
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif
}

/***************************************************************************
Summary:
	Set IFD Settings
See Also:
    NEXUS_Ifd_GetSettings
 ***************************************************************************/
void NEXUS_Ifd_SetSettings(
    NEXUS_IfdHandle handle,
    const NEXUS_IfdSettings *pSettings
    )
{
#if NEXUS_IFD_SUPPORT
#error not implemented
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif
}

/***************************************************************************
Summary:
	Get current IFD status
See Also:
    NEXUS_Ifd_SetSettings
 ***************************************************************************/
void NEXUS_Ifd_GetStatus(
    NEXUS_IfdHandle handle,
    NEXUS_IfdStatus *pStatus    /* [out] */
    )
{
#if NEXUS_IFD_SUPPORT
#error not implemented
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
#endif
}

void NEXUS_Ifd_P_GetDefaultSettings(
    NEXUS_IfdSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_IfdSettings));
    pSettings->enabled = true;
    pSettings->videoInputType = NEXUS_IfdInputType_eIf;
    pSettings->audioInputType = NEXUS_IfdInputType_eIf;
    pSettings->carrierFrequency = 45750000;
    pSettings->videoFormat = NEXUS_VideoFormat_eNtsc;
}

/***************************************************************************
Summary:
    Close an IFD handle
***************************************************************************/
void NEXUS_Ifd_Close(
    NEXUS_IfdHandle handle
    )
{
#if NEXUS_IFD_SUPPORT
#error not implemented
#else
    BSTD_UNUSED(handle);
#endif
}

