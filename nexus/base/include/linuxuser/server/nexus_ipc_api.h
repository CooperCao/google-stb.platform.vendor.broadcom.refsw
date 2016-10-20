/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Platform (private)
*    Common part of all kernel servers
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _NEXUS_IPC_API_H_
#define _NEXUS_IPC_API_H_

#include "nexus_types.h"

#define NEXUS_IPC_ID(module, id) (id)
typedef struct NEXUS_Ipc_Header {
   size_t packet_size; /* size of the entire packet, including this header */
   unsigned version;
   unsigned client_id;
   unsigned function_id;
   int result; /* ipc system rc, not the function's rc */
} NEXUS_Ipc_Header;


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

/* manual IPC on main socket */
enum nexus_main_socket_message_type
{
    nexus_main_socket_message_type_disconnect,      /* client sends this for clean disconnect. no read. */
    nexus_main_socket_message_type_stop_callbacks,  /* client sends void *interface, then reads back lastStopCallbacksCount for sync. */
    nexus_main_socket_message_type_start_callbacks, /* client sends void *interface. no read. */
    nexus_main_socket_message_type_max
};

/* queue size in server and client. not required to be the same, but efficient. */
#define NEXUS_CALLBACK_QUEUE_SIZE 32

/* data passed from server to client on initialization */
struct nexus_client_init_data
{
    NEXUS_ClientConfiguration config;
    struct {
        /* client cannot call NEXUS_Heap_GetStatus to populate initial heap information */
        NEXUS_HeapHandle heap; /* must match config.heaps[] */
        unsigned offset;
        unsigned size;
        NEXUS_MemoryType memoryType;
    } heap[NEXUS_MAX_HEAPS];
};

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
