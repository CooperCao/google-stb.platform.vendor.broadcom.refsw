/***************************************************************************
 *  Copyright (C) 2006-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#ifndef NEXUS_IMG_IOCTL_H__
#define NEXUS_IMG_IOCTL_H__

#define BIMG_ID_MAX_LEN   16

typedef enum {
    BIMG_Ioctl_Req_Type_Open,   /* this request sent from the driver to user mode app */
    BIMG_Ioctl_Req_Type_Next,   /* this is request from the driver to user mode app */
    BIMG_Ioctl_Req_Type_Close,  /* this is request from the driver to user mode app */ 
    BIMG_Ioctl_Req_Type_Again   /* driver interrupted. no request. proxy must call again. */
} BIMG_Ioctl_Req_Type;

typedef enum  {
    BIMG_Ioctl_Ack_Type_Wait,   /* application waits for commands */
    BIMG_Ioctl_Ack_Type_Reply,  /* application responds to call from driver */
    BIMG_Ioctl_Ack_Type_Exit    /* exit is a special request from the user application, which is used to release another process which might wait inside ioctl */
} BIMG_Ioctl_Ack_Type;

typedef uint64_t BIMG_Ioctl_Pointer;

typedef struct {
    char id[BIMG_ID_MAX_LEN];
    uint32_t req_type;
    uint32_t sequence_no;
    union {
        struct {
            uint32_t image_id;
        } open;
        struct {
            BIMG_Ioctl_Pointer image;
            uint32_t chunk;
            uint32_t length;
        } next;
        struct {
            BIMG_Ioctl_Pointer image;
        } close;
    } data;
}BIMG_Ioctl_Req ;

typedef struct {
    int result;
    BIMG_Ioctl_Ack_Type ack_type;
    BIMG_Ioctl_Req_Type req_type; /* if ack==BIMG_Ioctl_Ack_Type_Reply this must match BIMG_Ioctl_Req.req_type */
    uint32_t sequence_no; /* if ack==BIMG_Ioctl_Ack_Type_Reply this must match BIMG_Ioctl_Req.sequence_no */
    union {
        struct {
            BIMG_Ioctl_Pointer image;
        } open;
        struct {
            BIMG_Ioctl_Pointer data;
            uint32_t length;
        } next;
        struct {
            uint32_t dummy;
        } close;
    } data;
}BIMG_Ioctl_Ack;

typedef struct {
    BIMG_Ioctl_Req req; /* set by driver, read by user space */
    BIMG_Ioctl_Ack ack; /* set by user space, read by driver */
}BIMG_Ioctl;

int nexus_img_interfaces_init(void);
void nexus_img_interfaces_shutdown(void);
int nexus_img_ioctl(unsigned int cmd, unsigned long arg);
int nexus_img_wait_until_ready(void);

#endif /* NEXUS_IMG_IOCTL_H__ */


