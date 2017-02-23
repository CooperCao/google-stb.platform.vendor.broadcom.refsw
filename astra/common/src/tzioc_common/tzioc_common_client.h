/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef TZIOC_COMMON_CLIENT_H
#define TZIOC_COMMON_CLIENT_H

#ifndef TZIOC_CLIENT_DEFINES
#define TZIOC_CLIENT_DEFINES

#define TZIOC_CLIENT_NAME_LEN_MAX       32
#define TZIOC_CLIENT_PATH_LEN_MAX       128

#endif /* TZIOC_CLIENT_DEFINES */

#define TZIOC_CLIENT_NUM_MAX            32
#define TZIOC_CLIENT_ID_MAX             255

/* system uses fixed client id of 0 */
#ifndef TZIOC_CLIENT_ID_SYS
#define TZIOC_CLIENT_ID_SYS             0
#endif

/* uappd uses fixed client id of 1 */
#ifndef TZIOC_CLIENT_ID_UAPPD
#define TZIOC_CLIENT_ID_UAPPD           1
#endif

/* msg processing function pointer */
#ifndef TZIOC_MSG_PROC_PFN
#define TZIOC_MSG_PROC_PFN

#include "tzioc_common_msg.h"

typedef int (*tzioc_msg_proc_pfn)(
    tzioc_msg_hdr *pHdr,
    uintptr_t ulPrivData);

#endif

/* TZIOC client */
struct tzioc_client {
    char name[TZIOC_CLIENT_NAME_LEN_MAX];
    uint8_t idx;
    uint8_t id;
    bool kernel;
    void *psmem;

    /* kernel client only */
    tzioc_msg_proc_pfn msgProc;
    uintptr_t privData;

    /* user client only */
    uintptr_t task;
    int msgQ;
};

/* client control block */
struct tzioc_client_cb {
    /* client pinters */
    struct tzioc_client *pClients[TZIOC_CLIENT_NUM_MAX];

    /* last assigned client id */
    uint8_t lastId;
};

#ifdef __cplusplus
extern "C" {
#endif

struct tzioc_client *__tzioc_client_find_by_id(uint8_t id);
struct tzioc_client *__tzioc_client_find_by_name(const char *pName);
struct tzioc_client *__tzioc_client_find_by_task(uintptr_t task);
struct tzioc_client *__tzioc_client_find_by_name_and_task(
    const char *pName,
    uintptr_t task);

int __tzioc_kernel_client_open(
    struct tzioc_client *pClient,
    const char *pName,
    tzioc_msg_proc_pfn pMsgProc,
    uintptr_t ulPrivData);

int __tzioc_kernel_client_close(
    struct tzioc_client *pClient);

int __tzioc_user_client_open(
    struct tzioc_client *pClient,
    const char *pName,
    uintptr_t task,
    int msgQ);

int __tzioc_user_client_close(
    struct tzioc_client *pClient);

#ifdef __cplusplus
}
#endif

/* To be initialized by the specific OS */
extern struct tzioc_client_cb *pTziocClientCB;

#endif /* TZIOC_COMMON_CLIENT_H */
