/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BHSI_PRIV_H__
#define BHSI_PRIV_H__

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include  "bmem.h"

#include "berr_ids.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Definitions */

#if 0
#define BHSI_DUMP(BUF, LEN, NAME)         \
    {                                     \
        uint32_t i;                       \
        BDBG_LOG(("%s", (NAME)));         \
        BDBG_MSG(("\t%u bytes:", (LEN))); \
        for (i = 0; i < (LEN); i++) {     \
            BDBG_MSG(("\t\t%.3d - %u \n", \
                      i, (BUF)[i]));      \
        }                                 \
    }
#else
#define BHSI_DUMP(BUF, LEN, NAME)
#endif

/* End of Definitions */

/* Enum Types */


/***************************************************************************
Summary:
Structure that defines Host Secure module handle.

Description:
Structure that defines Host Secure module handle.

See Also:
BHSI_Open()

****************************************************************************/

/***************************************************************************
Summary:
Structure that defines Host Sage Interface handle.

Description:
Structure that defines Host Sage Interface handle.

See Also:
BHSI_Open()

****************************************************************************/
 typedef struct BHSI_P_Handle
{
    BHSI_Settings       settings;        /* current settings */
    BREG_Handle         regHandle;       /* register handle */
    BINT_Handle         interruptHandle; /* interrupt handle */
    bool                bIsOpen;         /* Is Module opened */
    bool                bReceivePending; /* data is pending. BHSI_Receive() is allowed */
    bool                bCallbacksEnabled;/* callbacks are currently enabled */
    BINT_CallbackHandle oLoadIntCallback;/* OLOAD Interrupt Callback */
    BINT_CallbackHandle drdyIntCallback; /* DRDY Interrupt Callback */
    BKNI_EventHandle    oLoadWaitEvent;  /* OLOAD wait event, fired once SHI has finish consuming the buffers OR reset is called */
    uint32_t            sendSeq;         /* send sequence ID */
} BHSI_P_Handle;


/* End of Host Secure Private Data Structures */


/* same definition as counterpart: BSHI_Msg */
typedef struct BHSI_Msg {
    uint32_t size;
    uint8_t payload;
} BHSI_Msg;

#ifdef __cplusplus
}
#endif

#endif /* BHSI_PRIV_H__ */
