/***************************************************************************
* Copyright (C) 2004-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*   API name: Platform (private)
*   This file containes private API to implement the B_CONFIG_IMAGE user mode part
*
***************************************************************************/
#include "nexus_types.h"
#include "nexus_base.h"
#include "nexus_platform.h"
#include "nexus_platform_local_priv.h"
#include "bimg.h"
#include "blst_list.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "nexus_driver_ioctl.h"

BDBG_MODULE(nexus_platform_image);

typedef struct BIMG_LinuxEntry {
    BLST_S_ENTRY(BIMG_LinuxEntry)  link;
    const char *name;
    const BIMG_Interface *iface; /* parent interface */
    const void *context; /* context of the parent interface */
    void *image; /* opened image */
    bool external;
}BIMG_LinuxEntry;

BLST_S_HEAD(BIMG_InterfaceList, BIMG_LinuxEntry);

static struct {
    struct BIMG_InterfaceList active;
    bool stopped;
    NEXUS_PlatformImgInterface imgInterface;
} b_interfaces;

NEXUS_Error Nexus_Platform_P_Image_Init(const NEXUS_PlatformImgInterface *pImgInterface)
{
    BKNI_Memset(&b_interfaces, 0, sizeof(b_interfaces));
    BLST_S_INIT(&b_interfaces.active);
    b_interfaces.stopped = false;
    if (pImgInterface) {
        b_interfaces.imgInterface = *pImgInterface;
    }
    return NEXUS_SUCCESS;
}

void Nexus_Platform_P_Image_Shutdown(void)
{
    BIMG_LinuxEntry *entry;

    while((entry=BLST_S_FIRST(&b_interfaces.active))!=NULL) {
        BLST_S_REMOVE_HEAD(&b_interfaces.active, link);
        BKNI_Free(entry);
    }
    return ;
}


void Nexus_Platform_P_Image_Stop(int fd, int ioctl_no)
{
    BIMG_Ioctl ctl;
    int rc;

    BKNI_Memset(&ctl, 0, sizeof(ctl));
    ctl.req.req_type =  BIMG_Ioctl_Req_Type_Exit;

    b_interfaces.stopped = true;
    rc = ioctl(fd, ioctl_no, &ctl);
    if (rc) BERR_TRACE(rc);

    return;
}

NEXUS_Error Nexus_Platform_P_Image_Interfaces_Register(const BIMG_Interface *iface, const void *context, const char *id)
{
    BIMG_LinuxEntry *entry;

    entry = BKNI_Malloc(sizeof(*entry));
    if (!entry) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    entry->name = id;
    entry->iface = iface;
    entry->context = context;
    entry->image = NULL;
    entry->external = false;
    BLST_S_INSERT_HEAD(&b_interfaces.active, entry, link);
    return NEXUS_SUCCESS;
}

void Nexus_Platform_P_Image_Interfaces_Unregister(const BIMG_Interface *iface, const void *context)
{
    BIMG_LinuxEntry *entry;

    for(entry=BLST_S_FIRST(&b_interfaces.active);entry;entry=BLST_S_NEXT(entry, link)) {
        if (entry->iface == iface && entry->context == context) {
            BLST_S_REMOVE(&b_interfaces.active, entry, BIMG_LinuxEntry, link);
            BKNI_Free(entry);
            return;
        }
    }
    return;
}


NEXUS_Error Nexus_Platform_P_Image_Handler(int fd, int ioctl_no)
{
    BIMG_Ioctl ctl;
    int rc;
    BERR_Code mrc = 0;
    BIMG_LinuxEntry *entry;

    BKNI_Memset(&ctl, 0, sizeof(ctl));
    ctl.req.req_type =  BIMG_Ioctl_Req_Type_Start;

    for(;;) {
        void *tmp_pointer;
        const void *tmp_const_pointer;

        rc = ioctl(fd, ioctl_no, &ctl);
        if (b_interfaces.stopped) {
            break;
        }
        if (rc!=0) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        if (ctl.req.req_type == BIMG_Ioctl_Req_Type_Again) {
            continue;
        }
        for(entry=BLST_S_FIRST(&b_interfaces.active);entry;entry=BLST_S_NEXT(entry, link)) {
            if (!strcmp(entry->name,ctl.req.id)) {
                break;
            }
        }
        if (!entry) {
            BDBG_ERR(("unknown image ID '%s'", ctl.req.id));
            ctl.ack.result = -1;
            continue;
        }
        switch(ctl.req.req_type) {
        case BIMG_Ioctl_Req_Type_Open:
            BDBG_MSG((">>Open[%s]: %p %u", ctl.req.id, (void *)entry->context, (unsigned)ctl.req.data.open.image_id));
            if (b_interfaces.imgInterface.open) {
                mrc = (b_interfaces.imgInterface.open)(entry->name, &tmp_pointer, ctl.req.data.open.image_id);
                ctl.ack.data.open.image = (unsigned long)tmp_pointer;
                if (!mrc) {
                    entry->external = true;
                }
            }
            if (!entry->external) {
                mrc = entry->iface->open((void *)entry->context, &tmp_pointer, ctl.req.data.open.image_id);
                ctl.ack.data.open.image = (unsigned long)tmp_pointer;
            }
            ctl.ack.result = mrc;
            if (mrc==BERR_SUCCESS) {
                entry->image = (void *)(unsigned long)ctl.ack.data.open.image;
            }
            BDBG_MSG(("<<Open[%s]: %p %u %d %p", ctl.req.id, (void *)entry->context, (unsigned)ctl.req.data.open.image_id, ctl.ack.result, (void *)entry->image));
            break;
        case BIMG_Ioctl_Req_Type_Next:
            BDBG_MSG((">>Next[%s]: %p %u %u", ctl.req.id, (void *)entry->image, (unsigned)ctl.req.data.next.chunk, (unsigned)ctl.req.data.next.length));
            if (entry->external) {
                mrc = (b_interfaces.imgInterface.next)(entry->image, ctl.req.data.next.chunk, &tmp_const_pointer, (uint16_t)ctl.req.data.next.length);
            }
            else {
                mrc = entry->iface->next(entry->image, ctl.req.data.next.chunk, &tmp_const_pointer, (uint16_t)ctl.req.data.next.length);
            }
            ctl.ack.data.next.data = (unsigned long)tmp_const_pointer;
            ctl.ack.result = mrc;
            BDBG_MSG(("<<Next[%s]: %p %u %u %d %p", ctl.req.id, (void *)entry->image, (unsigned)ctl.req.data.next.chunk, (unsigned)ctl.req.data.next.length, ctl.ack.result, (void *)(unsigned long)ctl.ack.data.next.data ));
            break;
        case BIMG_Ioctl_Req_Type_Close:
            BDBG_MSG((">>Close[%s]: %p", ctl.req.id, (void *)entry->image));
            if (entry->external) {
                (b_interfaces.imgInterface.close)(entry->image);
            }
            else {
                entry->iface->close(entry->image);
            }
            entry->image = NULL;
            BDBG_MSG(("<<Close[%s]: %p", ctl.req.id, (void *)entry->image));
            ctl.ack.result = BERR_SUCCESS;
            break;
        default:
            BDBG_ERR(("invalid ctl.req.req_type: %d", ctl.req.req_type));
            break;
        }
    }
    return NEXUS_SUCCESS;
}

