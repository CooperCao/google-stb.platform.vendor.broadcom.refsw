/***************************************************************************
*  Copyright (C) 2004-2018 Broadcom.  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
*
***************************************************************************/

#ifndef _NEXUS_IPC_API_H_
#define _NEXUS_IPC_API_H_

#include "nexus_types.h"

#define NEXUS_IPC_ID(module, id) (id)
typedef struct NEXUS_Ipc_Header {
   unsigned packet_size; /* size of the entire packet, including this header */
   unsigned version;
   unsigned function_id;
   unsigned unused;
} NEXUS_Ipc_Header;

#define NEXUS_IPC_SERVER_ERROR_PKT_LEN  1


/**
Intermediate physical addressed used to translate addresses between processes.
Instead of using a generic type, we use a typedef so that the auto-gen API is clear what is a translated address.
**/
typedef void *NEXUS_Ipc_DeviceAddress;

/* default usermode multiprocess paths */
#define B_UNIX_MAIN_SOCKET_STR "nexus_multiprocess"
#define B_UNIX_SOCKET_FORMAT_STR "nexus_module"
#define B_UNIX_SCHEDULER_SOCKET_FMT_STR "nexus_scheduler"

struct nexus_callback_data
{
    void *interface;
    NEXUS_CallbackDesc callback;
};

struct nexus_connect_info {
    unsigned abi; /* sizeof(void *)*8 */
    NEXUS_ClientAuthenticationSettings auth;
};

/* manual IPC on main socket */
enum nexus_main_socket_message_type
{
    nexus_main_socket_message_type_disconnect,      /* client sends this for clean disconnect. no read. */
    nexus_main_socket_message_type_stop_callbacks,  /* client sends void *interface, then reads back lastStopCallbacksCount for sync. */
    nexus_main_socket_message_type_start_callbacks, /* client sends void *interface. no read. */
    nexus_main_socket_message_type_max
};

/* Fixed size IPC buffer all [fixed size] arguments shall fit into this buffer */
#define NEXUS_P_IPC_BUFFER_SIZE 16384

/* queue size in server and client. not required to be the same, but efficient. */
#define NEXUS_CALLBACK_QUEUE_SIZE 32

/* nexus_socket.c */
typedef enum nexus_socket_type
{
    nexus_socket_type_main,
    nexus_socket_type_module,
    nexus_socket_type_scheduler
} nexus_socket_type;
/* return -1 on failure, return fd on success */
int b_nexus_socket_connect(nexus_socket_type socket_type, unsigned index);
int b_nexus_socket_listen(nexus_socket_type socket_type, unsigned index);
ssize_t b_nexus_read(int fd, void *buf, size_t buf_size);
ssize_t b_nexus_write(int fd, const void *buf, size_t buf_size);
int b_nexus_socket_close(int fd);

#endif /* _NEXUS_IPC_API_H_ */
