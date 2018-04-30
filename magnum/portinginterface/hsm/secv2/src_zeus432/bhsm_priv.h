/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"

#ifndef BHSM_PRIVATE__H
#define BHSM_PRIVATE__H

#define FLASHMAP_VERSION_V1 1
#define FLASHMAP_VERSION_V2 2
#define FLASHMAP_VERSION_V3 3
#define FLASHMAP_VERSION_V4 4
#define FLASHMAP_VERSION_V5 5
#define FLASHMAP_VERSION FLASHMAP_VERSION_V5

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

typedef struct
{
    struct {
        struct {
            unsigned major;
            unsigned minor;
            unsigned subminor;
        }zeus, bfw;
    }version;

    struct{
        bool valid;          /* true if the firmware EPOCH is available */
        uint8_t value;       /* the EPOCH value ranging from 0 to 255*/
    }firmwareEpoch;           /* the EPOCH of the BSECK Firmware (BFW) */

    bool verified;
}BHSM_BfwVersion;



typedef struct BHSM_P_Handle
{
   BDBG_OBJECT( BHSM_P_Handle )

   struct{ /* pointers to sub-module data. */
       struct BHSM_KeySlotModule *pKeyslots;
       void* pKeyLadders;
       void* pRegionVerification;
       void* pHash;
       void* pRvRsa;
       void* pRsa;
       struct BHSM_RvRegionModule *pRvRegions;
       struct BHSM_OtpKeyModule *pOtpKey;
   }modules;
   void* pBspMessageModule;

    BREG_Handle  regHandle;
    BCHP_Handle  chipHandle;
    BINT_Handle  interruptHandle;

    bool            bfwVersionValid;   /* true if "bfwVersion" is valid */
    BHSM_BfwVersion bfwVersion;

}BHSM_P_Handle;


bool isBfwVersion_GreaterOrEqual( BHSM_Handle hHsm, unsigned major, unsigned minor, unsigned subMinor ); /* Returns true if current BFW is greater than or equal to specified value */

#endif
