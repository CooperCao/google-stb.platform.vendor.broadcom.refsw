/***************************************************************************
*     (c)2008 Broadcom Corporation
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
* Description: Sample header file for an App Lib
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_Dsmcc_LIB_PRIV_INBAND_H__
#define B_Dsmcc_LIB_PRIV_INBAND_H__

#include "nexus_platform_features.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_message.h" 
#include "nexus_platform.h"
#include "nexus_parser_band.h"

/***************************************************************************
Summary:
Private handle for Dsmcc App Lib
***************************************************************************/
struct b_dsmcc_lib_priv_inband
{
    NEXUS_MessageHandle msg;
    BKNI_EventHandle message_ready_event;
};

void b_dsmcc_lib_priv_inband_stop(B_Dsmcc_P_Handle hDsmcc);
/* Return 0 if succeed, otherwise fail. */
int b_dsmcc_lib_priv_inband_start(B_Dsmcc_P_Handle h, int module_id);

/* return >=0 if succeed, < 0 if fail */
int b_dsmcc_lib_priv_inband_open(B_Dsmcc_P_Handle hDsmcc, B_Dsmcc_Settings *pDsmccSettings);
/* return  0 if succeed, otherwise if fail */
int b_dsmcc_lib_priv_inband_close(B_Dsmcc_P_Handle hDsmcc);
/* return 0 if succeed, otherwise fail */
int b_dsmcc_lib_priv_inband_get_buffer(B_Dsmcc_P_Handle hDsmcc, unsigned char ** buffer, size_t * size);
/* return 0 if succeed, otherwise fail */
int b_dsmcc_lib_priv_inband_read_complete(B_Dsmcc_P_Handle hDsmcc, size_t size);
/* return >=0 if succeed, otherwise fail */
int b_dsmcc_lib_priv_inband_init(void);
/* return >= if succeed, otherwise fail */
int b_dsmcc_lib_priv_inband_uninit(void);

#endif /* #ifndef B_Dsmcc_LIB_PRIV_INBAND_H__ */
