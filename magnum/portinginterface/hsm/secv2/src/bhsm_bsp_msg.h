/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BHSM_BSP_MSG_H__
#define BHSM_BSP_MSG_H__

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bint.h"
#include "bhsm.h"

#define BHSM_P_MAILBOX_BYTE_SIZE (384)
#define BHSM_P_MAILBOX_WORD_SIZE (BHSM_P_MAILBOX_BYTE_SIZE/4)


typedef struct BHSM_P_BspMsg* BHSM_BspMsg_h;

typedef struct
{
    unsigned unused;

} BHSM_BspMsgInit_t;


typedef struct
{
    /* output */
    uint8_t *pSend;                    /* send pointed.    location in mailbox after the header.*/
    uint8_t *pReceive;                 /* receive pointer. location in mailbox after the header. */

} BHSM_BspMsgCreate_t;


typedef struct
{
    /* input */
    unsigned     component;
    unsigned     command;

    uint32_t     continualMode;

    struct{
        bool enable;
        unsigned rsaKeyId;                   /* the Rsa Slot that the command is signed against. */
        BSTD_DeviceOffset signatureOffset;   /* memory offset to signature. Valid of "enable" is true. */
    }signedCommand;

} BHSM_BspMsgConfigure_t;


/* To be called only during HSM module initialisation/unit. */
BERR_Code BHSM_BspMsg_Init( BHSM_Handle hHsm, BHSM_BspMsgInit_t *pParam );
void BHSM_BspMsg_Uninit( BHSM_Handle hHsm );

/* Create/Destroy a BSP interface component. Only one instance can exist at any one time. */
BHSM_BspMsg_h  BHSM_BspMsg_Create( BHSM_Handle hHsm, BHSM_BspMsgCreate_t *pParam );
BERR_Code BHSM_BspMsg_Destroy( BHSM_BspMsg_h hMsg );

BERR_Code BHSM_BspMsg_Configure( BHSM_BspMsg_h hMsg, BHSM_BspMsgConfigure_t *pParam );

/* Submit the Message to the BSP interface. Function blocks until a response is received (or it times out) */
BERR_Code BHSM_BspMsg_SubmitCommand( BHSM_BspMsg_h hMsg, uint16_t *pBspStatus );

#ifdef __cplusplus
}
#endif

#endif /* BHSM_BSP_MSG_H__ */
