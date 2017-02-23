/******************************************************************************
 * Copyright (C) 2011-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#ifndef BIPC_IMPL_H__
#define BIPC_IMPL_H__

#include "blst_list.h"
#include "bkni_multi.h"
#include "bipc_client.h"
#include "bipc_client_priv.h"
#include "bipc_server.h"
#include "berr_ids.h"


typedef struct bipc_client {
    BDBG_OBJECT(bipc_client)
    BKNI_MutexHandle lock;
    int recv_fd;
    int send_fd;
    size_t ipc_buf_size;
    const bipc_interface_descriptor * const *interfaces;
    size_t interface_count;
    uint8_t buf[1]; /* variable length buffer */
} bipc_client ;  

typedef struct b_ipc_server_instance {
    const bipc_server_descriptor *interface; 
    void *object;
    unsigned generation; /* counter that is used to auto-close in reverse order */
} b_ipc_server_instance;

struct bipc_server_client;


typedef struct bipc_server {
    BDBG_OBJECT(bipc_server)
    BKNI_MutexHandle lock;
    BLST_D_HEAD(bipc_server_client_list, bipc_server_client) clients;
    bipc_server_create_settings create_settings;
    size_t ipc_buf_size;
    uint8_t buf[1]; /* variable length buffer */
} bipc_server ;

struct bipc {
    union {
       bipc_server server;
       bipc_client client;
    } t;
};

typedef struct b_ipc_pkt_in {
    unsigned pkt_size;
    unsigned instance;
    unsigned method;
    unsigned abi;
} b_ipc_pkt_in;

typedef struct b_ipc_pkt_out {
    unsigned pkt_size;
    unsigned unused;
} b_ipc_pkt_out;


#define BERR_END_OF_FILE BERR_MAKE_CODE(BERR_APP_ID, 1)

int b_safe_write(int fd, const void *buf, size_t buf_size);
int b_safe_read(int fd, void *buf, size_t buf_size);

#endif /* BIPC_IMPL_H__ */

