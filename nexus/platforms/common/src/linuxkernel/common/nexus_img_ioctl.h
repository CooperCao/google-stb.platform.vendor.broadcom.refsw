/***************************************************************************
 *     (c)2006-2011 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


#ifndef NEXUS_IMG_IOCTL_H__
#define NEXUS_IMG_IOCTL_H__

#define BIMG_ID_MAX_LEN   16

typedef enum {
    BIMG_Ioctl_Req_Type_Open,   /* this request sent from the driver to user mode app */
    BIMG_Ioctl_Req_Type_Next,   /* this is request from the driver to user mode app */
    BIMG_Ioctl_Req_Type_Close,  /* this is request from the driver to user mode app */ 
    BIMG_Ioctl_Req_Type_Start,  /* start is a special request, which should be used first time by application called into the driver */
    BIMG_Ioctl_Req_Type_Again,  /* driver interrupted. no request. proxy must call again. */
    BIMG_Ioctl_Req_Type_Exit    /* exit is a special request from the user application, which is used to release another process which might wait inside ioctl */
}BIMG_Ioctl_Req_Type ;

typedef struct {
    char id[BIMG_ID_MAX_LEN];
    uint32_t req_type;
    union {
        struct {
            uint32_t image_id;
        } open;
        struct {
            void *image;
            uint32_t chunk;
            uint32_t length;
        } next;
        struct {
            void *image;
        } close;
    } data; 
}BIMG_Ioctl_Req ;

typedef struct {
    int result;
    union {
        struct {
            void *image;
        } open;
        struct {
            const void *data;
        } next;
        struct {
            uint32_t dummy;
        } close;
    } data; 
}BIMG_Ioctl_Ack;

typedef struct {
    BIMG_Ioctl_Req req;
    BIMG_Ioctl_Ack ack;
}BIMG_Ioctl;

int nexus_img_interfaces_init(void);
void nexus_img_interfaces_shutdown(void);
int nexus_img_ioctl(unsigned int cmd, unsigned long arg);
int nexus_img_wait_until_ready(void);

#endif /* NEXUS_IMG_IOCTL_H__ */


