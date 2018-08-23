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

#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/slab.h>

#include "tzioc_drv.h"
#include "tzioc_client.h"
#include "tzioc_msg.h"
#include "tzioc_mem.h"
#include "tzioc_ioctl.h"

/* static pointer to ioctl module */
static struct tzioc_ioctl_module *pIoctlMod;

static int tzioc_ioctl_client_open(struct file *file, void *arg);
static int tzioc_ioctl_client_close(struct file *file, void *arg);
static int tzioc_ioctl_msg_send(struct file *file, void *arg);
static int tzioc_ioctl_mem_alloc(struct file *file, void *arg);
static int tzioc_ioctl_mem_free(struct file *file, void *arg);
static int tzioc_ioctl_call_smc(struct file *file, void *arg);

/* short-hand to get ioctl offset */
#define IOCTL_OFFSET(name)      (TZIOC_IOCTL_##name - TZIOC_IOCTL_FIRST)

int _tzioc_ioctl_module_init(void)
{
    /* alloc ioctl module */
    pIoctlMod = kzalloc(sizeof(struct tzioc_ioctl_module), GFP_KERNEL);
    if (!pIoctlMod) {
        LOGE("Failed to alloc TZIOC ioctl module");
        return -ENOMEM;
    }

    /* remember ioctl module in TZIOC device */
    tdev->pIoctlMod = pIoctlMod;

    memset(pIoctlMod->handlers, 0, sizeof(pIoctlMod->handlers));

    pIoctlMod->handlers[IOCTL_OFFSET(CLIENT_OPEN )] = tzioc_ioctl_client_open;
    pIoctlMod->handlers[IOCTL_OFFSET(CLIENT_CLOSE)] = tzioc_ioctl_client_close;
    pIoctlMod->handlers[IOCTL_OFFSET(MSG_SEND    )] = tzioc_ioctl_msg_send;
    pIoctlMod->handlers[IOCTL_OFFSET(MEM_ALLOC   )] = tzioc_ioctl_mem_alloc;
    pIoctlMod->handlers[IOCTL_OFFSET(MEM_FREE    )] = tzioc_ioctl_mem_free;
    pIoctlMod->handlers[IOCTL_OFFSET(CALL_SMC    )] = tzioc_ioctl_call_smc;

    LOGI("TZIOC ioctl module initialized");
    return 0;
}

int _tzioc_ioctl_module_deinit(void)
{
    /* reset ioctl module in TZIOC device */
    tdev->pIoctlMod = NULL;

    /* free ioctl module control block */
    kfree(pIoctlMod);

    LOGI("TZIOC ioctl module deinitialized");
    return 0;
}

int _tzioc_ioctl_do_ioctl(struct file *file, uint32_t cmd, void *arg)
{
    if (pIoctlMod->handlers[cmd - TZIOC_IOCTL_FIRST]) {
        return pIoctlMod->handlers[cmd - TZIOC_IOCTL_FIRST](file, arg);
    }
    else {
        LOGE("Unsupported TZIOC ioctl cmd 0x%x", (unsigned int)cmd);
        return -ENOTTY;
    }
}

static int tzioc_ioctl_client_open(struct file *file, void *arg)
{
    struct tzioc_ioctl_client_open_data clientOpenData;
    struct tzioc_client *pClient;
    char *pName;
    int msgQ;

    if (copy_from_user(
            &clientOpenData,
            (void *)arg,
            sizeof(clientOpenData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    pName = clientOpenData.name;
    msgQ = (int)clientOpenData.msgQ;

    if (pName[0] == '\0' ||
        msgQ < 0) {
        LOGE("Invalid args in TZIOC ioctl client open cmd");
        return -EINVAL;
    }

    pClient = _tzioc_user_client_open(
        pName,
        current,
        msgQ);

    if (pClient == NULL) {
        LOGE("Failed to open user client");
        clientOpenData.retVal = -EFAULT;
        goto ERR_RETURN;
    }

    clientOpenData.hClient   = (uintptr_t)pClient;
    clientOpenData.id        = (uint32_t)pClient->id;
    clientOpenData.smemStart = tdev->smemStart;
    clientOpenData.smemSize  = tdev->smemSize;
    clientOpenData.retVal    = 0;

 ERR_RETURN:
    if (copy_to_user(
            (void *)arg,
            &clientOpenData,
            sizeof(clientOpenData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    return 0;
}

static int tzioc_ioctl_client_close(struct file *file, void *arg)
{
    struct tzioc_ioctl_client_close_data clientCloseData;
    struct tzioc_client *pClient;

    if (copy_from_user(
            &clientCloseData,
            (void *)arg,
            sizeof(clientCloseData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    pClient = (struct tzioc_client *)clientCloseData.hClient;

    if (!pClient) {
        LOGE("Invalid args in TZIOC ioctl client close cmd");
        return -EINVAL;
    }

    if ((pClient->idx < 0) || (pClient->idx > TZIOC_CLIENT_NUM_MAX)) {
        LOGE("Invalid args in TZIOC ioctl client close cmd");
        return -EINVAL;
    }

    _tzioc_user_client_close(pClient);

    clientCloseData.retVal = 0;

    if (copy_to_user(
            (void *)arg,
            &clientCloseData,
            sizeof(clientCloseData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    return 0;
}

static int tzioc_ioctl_msg_send(struct file *file, void *arg)
{
    struct tzioc_ioctl_msg_send_data msgSendData;
    struct tzioc_client *pClient;
    struct tzioc_msg_hdr *pHdr;
    static uint8_t aucPayload[TZIOC_MSG_PAYLOAD_MAX];

    if (copy_from_user(
            &msgSendData,
            (void *)arg,
            sizeof(msgSendData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    pClient = (struct tzioc_client *)msgSendData.hClient;
    pHdr = &msgSendData.hdr;

    if (!pClient ||
        pHdr->ucOrig != pClient->id ||
        pHdr->ulLen > TZIOC_MSG_PAYLOAD_MAX) {
        LOGE("Invalid args in TZIOC ioctl client close cmd");
        return -EINVAL;
    }

    if (pHdr->ulLen) {
        if (copy_from_user(
                aucPayload,
                (void *)msgSendData.payloadAddr,
                pHdr->ulLen)) {
            LOGE("Failed to access msg payload");
            return -EFAULT;
        }
    }

    msgSendData.retVal = _tzioc_msg_send(
        pClient,
        pHdr,
        aucPayload);

    if (copy_to_user(
            (void *)arg,
            &msgSendData,
            sizeof(msgSendData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    return 0;
}

static int tzioc_ioctl_mem_alloc(struct file *file, void *arg)
{
    struct tzioc_ioctl_mem_alloc_data memAllocData;
    struct tzioc_client *pClient;
    void *pBuff;
    uint32_t ulBuffOffset;

    if (copy_from_user(
            &memAllocData,
            (void *)arg,
            sizeof(memAllocData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    pClient = (struct tzioc_client *)memAllocData.hClient;

    if (!pClient) {
        LOGE("Invalid args in TZIOC ioctl client close cmd");
        return -EINVAL;
    }

    pBuff = _tzioc_mem_alloc(
        pClient,
        memAllocData.size);

    if (!pBuff) {
        LOGE("Failed to alloc mem");
        memAllocData.retVal = -ENOMEM;
        goto ERR_RETURN;
    }

    ulBuffOffset = _tzioc_addr2offset((uintptr_t)pBuff);

    if (ulBuffOffset == -1) {
        LOGE("Failed to convert vaddr to offset");
        _tzioc_mem_free(
            pClient,
            pBuff);
        memAllocData.retVal = -EFAULT;
        goto ERR_RETURN;
    }

    memAllocData.buffOffset = ulBuffOffset;
    memAllocData.retVal = 0;

 ERR_RETURN:
    if (copy_to_user(
            (void *)arg,
            &memAllocData,
            sizeof(memAllocData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    return 0;
}

static int tzioc_ioctl_mem_free(struct file *file, void *arg)
{
    struct tzioc_ioctl_mem_free_data memFreeData;
    struct tzioc_client *pClient;
    void *pBuff;

    if (copy_from_user(
            &memFreeData,
            (void *)arg,
            sizeof(memFreeData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    pClient = (struct tzioc_client *)memFreeData.hClient;

    if (!pClient) {
        LOGE("Invalid args in TZIOC ioctl client close cmd");
        return -EINVAL;
    }

    pBuff = (void *)_tzioc_offset2addr(memFreeData.buffOffset);

    if ((uintptr_t)pBuff == -1) {
        LOGE("Failed to convert offset to vaddr");
        memFreeData.retVal = -EFAULT;
        goto ERR_RETURN;
    }

    _tzioc_mem_free(
        pClient,
        pBuff);

    memFreeData.retVal = 0;

 ERR_RETURN:
    if (copy_to_user(
            (void *)arg,
            &memFreeData,
            sizeof(memFreeData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    return 0;
}

static int tzioc_ioctl_call_smc(struct file *file, void *arg)
{
    struct tzioc_ioctl_call_smc_data callSmcData;
    struct tzioc_client *pClient;

    if (copy_from_user(
            &callSmcData,
            (void *)arg,
            sizeof(callSmcData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    pClient = (struct tzioc_client *)callSmcData.hClient;

    if (!pClient) {
        LOGE("Invalid args in TZIOC ioctl client close cmd");
        return -EINVAL;
    }

    _tzioc_call_smc(callSmcData.callnum);

    callSmcData.retVal = 0;

    if (copy_to_user(
            (void *)arg,
            &callSmcData,
            sizeof(callSmcData))) {
        LOGE("Failed to access TZIOC ioctl arguments");
        return -EFAULT;
    };

    return 0;
}
