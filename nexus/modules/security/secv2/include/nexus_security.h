/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/


#ifndef NEXUS_SECURITY__H_
#define NEXUS_SECURITY__H_

#include "nexus_keyslot.h"
#include "nexus_security_types.h"


#define NEXUS_SECURITY_PENDING    NEXUS_MAKE_ERR_CODE(0x109, (1))
#define NEXUS_SECURITY_HSM_ERROR  NEXUS_MAKE_ERR_CODE(0x109, (2))
#define NEXUS_SECURITY_STATE_ERROR  NEXUS_MAKE_ERR_CODE(0x109, (3))



/* Security module capabilities and configuration. */
typedef struct NEXUS_SecurityCapabilities
{
   struct
   {
       struct
       {
            unsigned major;
            unsigned minor;
            unsigned subminor;
       }zeus,bfw;
   }version;                 /* version information */

   struct{
        bool valid;          /* true if the firmware EPOCH is available */
        uint8_t value;       /* the EPOCH value ranging from 0 to 255*/
   }firmwareEpoch;           /* the EPOCH of the BSECK Firmware (BFW) */


   unsigned numKeySlotsForType[NEXUS_KeySlotType_eMax];   /* Number of each type of keyslot. */

} NEXUS_SecurityCapabilities;


void NEXUS_GetSecurityCapabilities(
    NEXUS_SecurityCapabilities *pCaps /* [out] */
    );


/*
    Associate a pidChannel handle with a specified bypass keyslot.
*/
NEXUS_Error NEXUS_SetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel,
                                              NEXUS_BypassKeySlot bypassKeySlot
                                              );
/*
    Get the Bypass keyslot associated with a particular pid channel.
*/
void NEXUS_GetPidChannelBypassKeyslot( NEXUS_PidChannelHandle pidChannel,
                                       NEXUS_BypassKeySlot *pBypassKeySlot /* [out]  the bypass keyslot associated with the specified pidChannel */
                                       );


#endif
