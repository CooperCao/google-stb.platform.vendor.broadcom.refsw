/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/

#include "tzioc_msg.h"

#include "libfdt.h"
#include "parse_utils.h"
#include "lib_printf.h"

// Static non-const data from TzIocMsg class
spinlock_t TzIoc::TzIocMsg::sndLock;
spinlock_t TzIoc::TzIocMsg::rcvLock;
struct tzioc_msg_cb TzIoc::TzIocMsg::msgCB;

// Exported msg control block to common code
struct tzioc_msg_cb *pTziocMsgCB;

void TzIoc::TzIocMsg::init(void *devTree)
{
    // Get parent tzioc node
    int parentOffset = fdt_subnode_offset(devTree, 0, "tzioc");
    int propLen;

    // Parse #address-cells property of parent node
    unsigned long adCellSize;
    const struct fdt_property *fpAdCells =
        fdt_get_property(devTree, parentOffset, "#address-cells", &propLen);

    if ((!fpAdCells) || (propLen < sizeof(int)))
        adCellSize = 1;
    else
        adCellSize = parseInt((void *)fpAdCells->data, propLen);

    // Parse #size-cells property of parent node
    unsigned long szCellSize;
    const struct fdt_property *fpSzCells =
        fdt_get_property(devTree, parentOffset, "#size-cells", &propLen);

    if ((!fpSzCells) || (propLen < sizeof(int)))
        szCellSize = 1;
    else
        szCellSize = parseInt((void *)fpAdCells->data, propLen);

    int adByteSize = adCellSize * sizeof(int);
    int szByteSize = szCellSize * sizeof(int);

    // Get t2n-ring node
    int nodeOffset = fdt_subnode_offset(devTree, parentOffset, "t2n-ring");
    if (nodeOffset < 0) {
        kernelHalt("No t2n-ring node found in the device tree");
    }

    // Parse 'reg' property
    const struct fdt_property *fpT2NReg =
        fdt_get_property(devTree, nodeOffset, "reg", &propLen);

    if ((!fpT2NReg) || (propLen != adByteSize + szByteSize))
        kernelHalt("Invalid reg property in device tree t2n-ring node.");

    const char *t2nRegData = fpT2NReg->data;
    uint32_t t2nOffset = (uint32_t)((adCellSize == 1) ?
         parseInt(t2nRegData, adByteSize) :
         parseInt64(t2nRegData, adByteSize));
    t2nRegData += adByteSize;

    uint32_t t2nSize = (uint32_t)((szCellSize == 1) ?
         parseInt(t2nRegData, szByteSize) :
         parseInt64(t2nRegData, szByteSize));
    t2nRegData += szByteSize;

    printf("TzIoc T2N ring at 0x%x, size 0x%x\n",
           (unsigned int)t2nOffset, (unsigned int)t2nSize);

    // Get n2t-ring node
    nodeOffset = fdt_subnode_offset(devTree, parentOffset, "n2t-ring");
    if (nodeOffset < 0) {
        kernelHalt("No n2t-ring node found in the device tree");
    }

    // Parse 'reg' property
    const struct fdt_property *fpN2TReg =
        fdt_get_property(devTree, nodeOffset, "reg", &propLen);

    if ((!fpN2TReg) || (propLen != adByteSize + szByteSize))
        kernelHalt("Invalid reg property in device tree n2t-ring node.");

    const char *n2tRegData = fpN2TReg->data;
    uint32_t n2tOffset = (uint32_t)((adCellSize == 1) ?
         parseInt(n2tRegData, adByteSize) :
         parseInt64(n2tRegData, adByteSize));
    n2tRegData += adByteSize;

    uint32_t n2tSize = (uint32_t)((szCellSize == 1) ?
         parseInt(n2tRegData, szByteSize) :
         parseInt64(n2tRegData, szByteSize));
    n2tRegData += szByteSize;

    printf("TzIoc N2T ring at 0x%x, size 0x%x\n",
           (unsigned int)n2tOffset, (unsigned int)n2tSize);

    // Init spinlocks
    spinlock_init("TzIocMsg.sndLock", &sndLock);
    spinlock_init("TzIocMsg.rcvLock", &rcvLock);

    // Init shared memory
    __tzioc_ring_init(
        &psmem->t2nRing,
        t2nOffset,
        t2nSize,
        TZIOC_RING_CREATE | TZIOC_RING_WRITE,
        _tzioc_offset2addr);

    __tzioc_ring_init(
        &psmem->n2tRing,
        n2tOffset,
        n2tSize,
        TZIOC_RING_CREATE | TZIOC_RING_READ,
        _tzioc_offset2addr);

    // Init msg control block
    msgCB.pSndRing = &psmem->t2nRing;
    msgCB.pRcvRing = &psmem->n2tRing;

    // Export msg control block to common code
    pTziocMsgCB = &msgCB;

    printf("TzIoc msg module initialized\n");
}

int TzIoc::TzIocMsg::send(
    struct tzioc_client *pClient,
    struct tzioc_msg_hdr *pHdr,
    uint8_t *pPayload)
{
    spin_lock(&sndLock);
    int err = __tzioc_msg_send(pHdr, pPayload);
    spin_unlock(&sndLock);

    if (err) {
        printf("TzIoc msg send failed, client %d\n", pClient->id);
        return err;
    }
    return err;
}

int TzIoc::TzIocMsg::receive(
    struct tzioc_client *pClient,
    struct tzioc_msg_hdr *pHdr,
    uint8_t *pPayload,
    uint32_t ulSize)
{
    spin_lock(&rcvLock);
    int err = __tzioc_msg_receive(pHdr, pPayload, ulSize);
    spin_unlock(&rcvLock);

    if (err &&
        err != -ENOMSG &&
        err != -ENOSPC) {
        printf("TzIoc msg receive failed, client %d\n", pClient->id);
        return err;
    }
    return err;
}
