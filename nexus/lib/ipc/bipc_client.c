/******************************************************************************
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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "bstd.h"
#include "bipc_impl.h"
#include <errno.h>

BDBG_MODULE(bipc_client);

BDBG_OBJECT_ID(bipc_client);

bipc_t bipc_client_create(int recv_fd, int send_fd, const bipc_interface_descriptor * const interfaces[], size_t interface_count)
{
    bipc_t ipc;
    unsigned i;
    size_t size;
    BERR_Code rc;

    BDBG_ASSERT(interfaces);
    BDBG_ASSERT(interface_count>0);
    for(size=0,i=0;i<interface_count;i++) {
        BDBG_ASSERT(interfaces[i]);
        size += interfaces[i]->buf_size;
    }

    ipc = BKNI_Malloc(size + sizeof(b_ipc_pkt_in) + sizeof(b_ipc_pkt_out)+sizeof(*ipc));
    if(!ipc) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}

    BDBG_OBJECT_INIT(&ipc->t.client, bipc_client);
    ipc->t.client.ipc_buf_size = size;
    ipc->t.client.recv_fd = recv_fd;
    ipc->t.client.send_fd = send_fd;
    ipc->t.client.interfaces = interfaces;
    ipc->t.client.interface_count = interface_count;
    rc = BKNI_CreateMutex(&ipc->t.client.lock);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_mutex;}

    return ipc;
err_mutex:
    BKNI_Free(ipc);
err_alloc:
    return NULL;
}


void bipc_client_destroy(bipc_t ipc)
{
    BDBG_OBJECT_ASSERT(&ipc->t.client, bipc_client);
    BKNI_AcquireMutex(ipc->t.client.lock);
    BKNI_ReleaseMutex(ipc->t.client.lock);
    BKNI_DestroyMutex(ipc->t.client.lock);
    BDBG_OBJECT_DESTROY(&ipc->t.client, bipc_client);
    BKNI_Free(ipc);
    return;
}

void *bipc_client_begin(bipc_t ipc, const bipc_interface_descriptor *id)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(&ipc->t.client, bipc_client);
#if !BIPC_EXTERNAL_LOCK
    BKNI_AcquireMutex(ipc->t.client.lock);
#endif
    if(id) {
        bool found=false;
        for(i=0;i<ipc->t.client.interface_count;i++) {
            if(ipc->t.client.interfaces[i] == id) {
                found = true;
                break;
            }
        }
        if(!found) {
            return NULL;
        }
    }
    return ipc->t.client.buf + sizeof(b_ipc_pkt_in);
}



int bipc_client_send(bipc_t ipc, unsigned id, unsigned entry, size_t send_size, size_t  recv_offset, size_t recv_size)
{
    struct b_ipc_pkt_in *in;
    b_ipc_pkt_out out;
    int rc;

    BDBG_OBJECT_ASSERT(&ipc->t.client, bipc_client);
    in = (void *)ipc->t.client.buf;
    if(send_size + recv_size > ipc->t.client.ipc_buf_size) {
        rc = -1;
        (void)BERR_TRACE(BERR_NOT_SUPPORTED); goto done;
    }
    in->pkt_size = send_size + sizeof(*in);
    in->method = entry;
    in->instance = id;
    in->abi = sizeof(void *)*8;
    rc = b_safe_write(ipc->t.client.send_fd, in, in->pkt_size);
    if(rc!=(int)in->pkt_size) { (void)BERR_TRACE(BERR_NOT_SUPPORTED);goto done; }
    rc = b_safe_read(ipc->t.client.recv_fd, &out, sizeof(out));
    if(rc!=sizeof(out)) {
        if (rc != -ECONNRESET) {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        goto done;
    }
    if(out.pkt_size != recv_size + sizeof(out)) {
        rc = -1;
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);goto done; 
    }
    if(recv_size) {
        rc = b_safe_read(ipc->t.client.recv_fd, ipc->t.client.buf + sizeof(b_ipc_pkt_in) + recv_offset, recv_size);
        if(rc!=(int)recv_size) { (void)BERR_TRACE(BERR_NOT_SUPPORTED);goto done; }
    }
    rc = 0;
done:
    return rc;
}

void bipc_client_end(bipc_t ipc)
{
    BDBG_OBJECT_ASSERT(&ipc->t.client, bipc_client);

#if !BIPC_EXTERNAL_LOCK
    BKNI_ReleaseMutex(ipc->t.client.lock);
#endif
    return;
}

