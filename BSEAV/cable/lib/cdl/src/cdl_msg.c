/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * Module Description:  OpenCable Common Download msg module, handles communication with app.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "cdl_common.h"
#include "b_cdl_lib.h"
#include "cdl_msg.h"
BDBG_MODULE(cdl_msg);

struct b_cdl_msg_ctl {
    sem_t msg_sem;	/* semaphore for the message */
    unsigned char to_app_pending;
    unsigned char from_app_pending;

    struct b_cdl_msg to_app_msg;
    struct b_cdl_msg from_app_msg;
#if defined (MPOD_SUPPORT) 
    NEXUS_Callback callback; /* called when CDL library needs to send a message
                                      * to app. App can use this callback to receive
                                      * cdl message in non-blocking way. see [B_CDL_MSG] 
                                      * for more description.
                                      */
    void * context;
#endif

};

static struct b_cdl_msg_ctl msg_ctl;


/*
 * called by cdl, send message to app,
 * to avoid deadlock, send is non-blocking, receiving is blocking
 *
 * return > 0 if succeed, =0 if busy, < 0 if error 
 */
int b_cdl_msg_send_to_app(struct b_cdl_msg * msg) {
     int ret;

     ret = sem_trywait(&msg_ctl.msg_sem);
     if (ret) {
         BDBG_WRN(("%s: cannot get semaphore, return %d", __FUNCTION__, ret));
         return 0;
     }
     BDBG_MSG(("%s: get sem", __FUNCTION__));
     if (msg_ctl.to_app_pending) {
         BDBG_WRN(("Message pending, should not happen"));
         /* reutrn -1; */
     }
     memcpy(&msg_ctl.to_app_msg, msg, sizeof(struct b_cdl_msg));
     msg_ctl.to_app_pending = 1;
     BDBG_MSG(("%s: msg %d sent to app", __FUNCTION__, msg->type));
#if defined (MPOD_SUPPORT)
     if (msg_ctl.callback) {
         msg_ctl.callback(msg_ctl.context, 0); /* param in NEXUS_Callback not used*/
     }
#endif
     return 1;
 }
/*
 * called by app, send msg to cdl
 * return > 0 if new msg , = 0 if no msg, < 0 if fail 
 */
int b_cdl_msg_send_to_cdl(struct b_cdl_msg * msg) {
    int ret = -1;
     ret = sem_trywait(&msg_ctl.msg_sem);
     if (ret) {
         BDBG_WRN(("%s: cannot get semaphore, return %d", __FUNCTION__, ret));
         return 0;
     }
     BDBG_MSG(("%s: get sem", __FUNCTION__));
     if (msg_ctl.from_app_pending) {
         BDBG_WRN(("Message pending, should not happen"));
         /* reutrn -1; */
     }

     memcpy(&msg_ctl.from_app_msg, msg, sizeof(struct b_cdl_msg));
     msg_ctl.from_app_pending = 1;
     BDBG_MSG(("%s: msg %d sent to cdl", __FUNCTION__, msg->type));

     return 0;
 }

/*
 * called by cdl, receive msg from app
 * return > 0 if new msg , = 0 if no msg or message does not match, < 0 if fail 
 */
int b_cdl_msg_recv_from_app(struct b_cdl_msg * msg) {
    int ret = -1;
    int value = 0;

    sem_getvalue(&msg_ctl.msg_sem, &value);
    if (value == 0) {
        /* message pending */
        if (msg_ctl.from_app_pending) {
            BDBG_MSG(("%s: msg pending", __FUNCTION__));
            /* message matches */
            if (msg->type == msg_ctl.from_app_msg.type) {

                memcpy(msg, &msg_ctl.from_app_msg, sizeof(struct b_cdl_msg));
                msg_ctl.from_app_pending = 0;
                /* release semaphore */
                sem_post(&msg_ctl.msg_sem);
                BDBG_MSG(("%s: msg %d recvd from app", __FUNCTION__, msg->type));

                return 1;
            }
        }
    }
    return 0;
}

/*
 * called by app, receive msg from cdl
 * return > 0 if new msg , = 0 if no msg or message does not match, < 0 if fail 
 */
int b_cdl_msg_recv_from_cdl(struct b_cdl_msg * msg) {
    int ret = -1;
    int value = 0;

    sem_getvalue(&msg_ctl.msg_sem, &value);
    if (value == 0) {
        /* message pending */
        if (msg_ctl.to_app_pending) {
            BDBG_MSG(("%s: msg pending", __FUNCTION__));
            /* message matches */
            if (msg->type == msg_ctl.to_app_msg.type || msg->type == B_CDL_MSG_TO_APP_ANY) {
                memcpy(msg, &msg_ctl.to_app_msg, sizeof(struct b_cdl_msg));
                msg_ctl.to_app_pending = 0;
                /* release semaphore */
                sem_post(&msg_ctl.msg_sem);
                BDBG_MSG(("%s: msg %d recvd from cdl", __FUNCTION__, msg->type));
                return 1;
            }
        }
    }
    return 0;
}

int cdl_msg_init() {
	int ret = -1;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
    memset(&msg_ctl, 0, sizeof(msg_ctl));
	if (sem_init(&msg_ctl.msg_sem, 0, 1) != 0){
		BDBG_ERR(("%s: sem_init failed", __FUNCTION__));
		return -1;
	}

	ret = 0;
	return ret;
}

int cdl_msg_uninit(){
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	return 0;
}

#if defined (MPOD_SUPPORT) 
int cdl_msg_set_callback(NEXUS_Callback callback, void * context) {
    msg_ctl.callback = callback;
    msg_ctl.context = context;
    return 1;
}
#endif
