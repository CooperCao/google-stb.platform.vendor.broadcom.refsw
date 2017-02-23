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

#include "vuartfops.h"
#include "eventqueue.h"
#include "poll.h"
#include "libfdt.h"
#include "parse_utils.h"
#include "tzioc_common_ring.h"
#include "tzioc_msg.h"
#include "tzioc_client.h"
#include "tzioc_mem.h"
#include "tzioc.h"
#include "tzioc_sys_msg.h"
#include "pgtable.h"

#define VIRTUAL_TXMEM_SIZE 0x20000 /* 128kB for transmitter buffer */
#define VIRTUAL_RXMEM_SIZE 0x1000 /* 4kB for receiver buffer */

using namespace VuartFops;

struct tzioc_ring_buf *rxBuf;
struct tzioc_ring_buf *txBuf;

bool isVuartEnabled;

VuartFile* VuartFile::instance = 0;
static uint8_t vuartFileMem[sizeof(VuartFile)];

static int ring_force_poke(
        struct tzioc_ring_buf *pRing,
        uintptr_t ulWrOffset,
        uint8_t *pData,
        uintptr_t ulSize);

uintptr_t __offset2addr1(uintptr_t ulOffset) {
    return (ulOffset);
}

int VuartFops::init(bool isVuartChosen) {
    if (!isVuartChosen)
        return 0;

    // Alloc vuart buffers
    struct tzioc_client *pClient =
        TzIoc::TzIocClient::clientFindById(TZIOC_CLIENT_ID_SYS);

    if (NULL == pClient) {
        err_msg("Failed to get TZIOC system client");
        goto ERR;
    }

    /* Allocate sys tzioc memory for tx and rx*/
    txBuf = (tzioc_ring_buf *)TzIoc::TzIocMem::alloc(pClient, VIRTUAL_TXMEM_SIZE);
    if (NULL == txBuf) {
        err_msg("Failed to alloc TZIOC memory");
        goto ERR;
    }

    rxBuf = (tzioc_ring_buf *)TzIoc::TzIocMem::alloc(pClient, VIRTUAL_RXMEM_SIZE);
    if (NULL == rxBuf) {
        err_msg("Failed to alloc TZIOC memory");
        goto ERR;
    }

    /* Configure tzioc ring tx and rx ring buffers*/
    txBuf->ulBuffOffset = (uintptr_t)txBuf + sizeof(struct tzioc_ring_buf);
    txBuf->ulBuffSize = VIRTUAL_TXMEM_SIZE - sizeof(struct tzioc_ring_buf);
    txBuf->ulWrOffset = txBuf->ulRdOffset = txBuf->ulBuffOffset;
    txBuf->pWrOffset2Addr = __offset2addr1;

    rxBuf->ulBuffOffset = (uintptr_t)rxBuf + sizeof(struct tzioc_ring_buf);
    rxBuf->ulBuffSize = VIRTUAL_RXMEM_SIZE - sizeof(struct tzioc_ring_buf);
    rxBuf->ulWrOffset = rxBuf->ulRdOffset = rxBuf->ulBuffOffset;
    rxBuf->pRdOffset2Addr = __offset2addr1;

    isVuartEnabled = true;
    return 0;
ERR:
    if (txBuf) {
        TzIoc::TzIocMem::free(pClient, txBuf);
        txBuf = 0;
    }

    if (rxBuf) {
        TzIoc::TzIocMem::free(pClient, rxBuf);
        rxBuf = 0;
    }
    return -EFAULT;
}

int VuartFops::peerUp(void) {
    int err = 0;
    if (!isVuartEnabled) {
        printf("vuart is not enabled\n");
        return 0;
    }
    struct tzioc_client *pClient =
        TzIoc::TzIocClient::clientFindById(TZIOC_CLIENT_ID_SYS);
    if (NULL == pClient) {
        err_msg("Failed to get TZIOC system client");
        goto ERR;
    }

    /* Send tx and rx buffer info message to nw */
    struct tzioc_msg_hdr hdr;
    struct sys_msg_vuart_on_cmd payload;

    hdr.ucType = (isVuartEnabled ? SYS_MSG_VUART_ON : SYS_MSG_VUART_OFF);
    hdr.ucOrig = TZIOC_CLIENT_ID_SYS;
    hdr.ucDest = TZIOC_CLIENT_ID_SYS;
    hdr.ulLen = sizeof(payload);

    /* Shared mem could be mapped to a different address */
    payload.rxFifoPaddr = TzIoc::vaddr2paddr((uintptr_t)rxBuf);
    payload.rxFifoSize = VIRTUAL_RXMEM_SIZE;
    payload.txFifoPaddr = TzIoc::vaddr2paddr((uintptr_t)txBuf);
    payload.txFifoSize = VIRTUAL_TXMEM_SIZE;

    err = TzIoc::TzIocMsg::send(
        pClient,
        &hdr, (uint8_t *)&payload);

    if (err) {
        printf("Failed to send vuart payload msg\n");
        goto ERR;
    }
    return 0;
ERR:
    return -EFAULT;
}

int VuartFops::peerDown(void) {
    /* Clean up after peer down */
    return 0;
}

void* VuartFile::operator new(size_t sz, void* where) {
    UNUSED(sz);
    VuartFile *rv = (VuartFile *)where;
    return rv;
}

void VuartFile::init() {
    if (isVuartEnabled) {
        instance = new(vuartFileMem) VuartFile();
    }
    else {
        instance = NULL;
    }
}

size_t VuartFile::write(const void *data, const size_t numBytes, const uint64_t offset) {
    uintptr_t ulWrOffset;
    int err = 0;
    UNUSED(offset);
    UNUSED(data);

    ulWrOffset = txBuf->ulWrOffset;

    err = ring_poke(txBuf, ulWrOffset, (uint8_t *)data, numBytes);

    if (-ENOSPC == err) {
        err = ring_force_poke(txBuf, ulWrOffset, (uint8_t *)data, numBytes);
        if (err) return 0;
    }

    if (err) return 0;

    ulWrOffset = ring_wrap(txBuf, ulWrOffset, numBytes);

    txBuf->ulWrOffset = ulWrOffset;

    return numBytes;
}

ssize_t VuartFile::writev(const iovec *iov, int iovcnt, const uint64_t offset) {

    ssize_t rv = 0;
    for (int i=0; i<iovcnt; i++) {
        rv += write(iov[i].iov_base, iov[i].iov_len, offset);
    }

    return rv;
}

void VuartFile::addWatcher(short pollEvent, short *pollResult, EventQueue *eq) {
    *pollResult = 0;
    if (pollEvent & POLLOUT) {
        *pollResult |= POLLOUT;
        eq->signal();
    }
}

size_t VuartFile::read(const void *data, const size_t numBytes, const uint64_t offset) {
    UNUSED(offset);
    int readCount = 0;
    uintptr_t ulRdOffset;
    int err = 0;

    readCount = numBytes;
    ulRdOffset = rxBuf->ulRdOffset;


    err = ring_peek(rxBuf, ulRdOffset, (uint8_t *)data, numBytes);
    if (err) return err;
    ulRdOffset = ring_wrap(rxBuf, ulRdOffset, numBytes);
    readCount = numBytes;

    rxBuf->ulRdOffset = ulRdOffset;
    return readCount;
}

ssize_t VuartFile::readv(const iovec *iov, int iovcnt, const uint64_t offset) {

    ssize_t rv = 0;
    for (int i=0; i<iovcnt; i++) {
        rv += read(iov[i].iov_base, iov[i].iov_len, offset);
    }

    return rv;
}

static int ring_force_poke(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulWrOffset,
    uint8_t *pData,
    uintptr_t ulSize)
{
    uintptr_t ulFreeSpace = 0;
    int err = 0;

    ulFreeSpace = ring_space(
            pRing,
            ulWrOffset,
            pRing->ulRdOffset);

    if (ulFreeSpace < ulSize) {
        pRing->ulRdOffset = ring_wrap(
            pRing,
            pRing->ulRdOffset,
            (ulSize - ulFreeSpace));
    }

    err = ring_poke(pRing, ulWrOffset, pData, ulSize);
    if (err) return err;

    return 0;
}
