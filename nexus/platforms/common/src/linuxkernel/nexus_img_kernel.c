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
 * Module Description:
 * driver side of the Image Download interface
 *
 ***************************************************************************/
#include "nexus_platform_module.h"
#include "bkni.h"
#include "nexus_generic_driver_impl.h"
#include "nexus_img_kernel.h"
#include "nexus_img_ioctl.h"
#include "blst_slist.h"

BDBG_MODULE(nexus_img_kernel);

BDBG_OBJECT_ID(BIMG_Driver);
typedef struct BIMG_Driver {
    BDBG_OBJECT(BIMG_Driver);
    BLST_S_ENTRY(BIMG_Driver)  link;
    const char *name;
    void *image; /* currently opened image */
}BIMG_Driver;


BLST_S_HEAD(BIMG_InterfaceList, BIMG_Driver);

BDBG_OBJECT_ID(NEXUS_ImgState);
static struct NEXUS_ImgState {
    BDBG_OBJECT(NEXUS_ImgState)
    struct BIMG_InterfaceList active;
    bool stopped;
    bool ready;
    const BIMG_Driver *opened; /* we could only handle one interface at time so this used to serialize concurrent calls by serializing Open calls */
    BKNI_EventHandle req;
    BKNI_EventHandle ack;
    BKNI_EventHandle close;
    BIMG_Ioctl ioctl;
    BKNI_MutexHandle lock;
    uint8_t data[64*1024]; /* space to hold kernel copy of data */
} b_interfaces;

int
nexus_img_interfaces_init(void)
{
    int rc;
    BDBG_MSG(("initializing kernel image interface"));
    BDBG_OBJECT_SET(&b_interfaces, NEXUS_ImgState);
    BLST_S_INIT(&b_interfaces.active);
    BKNI_CreateEvent(&b_interfaces.req);
    BKNI_CreateEvent(&b_interfaces.ack);
    BKNI_CreateEvent(&b_interfaces.close);
    rc = BKNI_CreateMutex(&b_interfaces.lock);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return -rc; }
    b_interfaces.stopped = false;
    b_interfaces.ready = false;
    b_interfaces.opened = NULL;
    b_interfaces.ioctl.req.sequence_no = 0;
    return 0;
}

void
nexus_img_interfaces_shutdown(void)
{
    BIMG_Driver *iface;
    struct NEXUS_ImgState *interfaces = &b_interfaces;

    BDBG_OBJECT_ASSERT(interfaces, NEXUS_ImgState);
    BKNI_AcquireMutex(b_interfaces.lock);
    while((iface=BLST_S_FIRST(&b_interfaces.active))!=NULL) {
        BDBG_OBJECT_ASSERT(iface, BIMG_Driver);
        BDBG_MSG(("removing %p", (void *)iface));
        BLST_S_REMOVE_HEAD(&b_interfaces.active, link);
        BDBG_OBJECT_DESTROY(iface, BIMG_Driver);
        BKNI_Free(iface);
    }
    BKNI_ReleaseMutex(b_interfaces.lock);
    /* interface shutdown called at unload time, at this time shall be no application which are sleeping in the kernel, and waiting for the event */
    BKNI_DestroyEvent(b_interfaces.close);
    BKNI_DestroyEvent(b_interfaces.req);
    BKNI_DestroyEvent(b_interfaces.ack);
    BKNI_DestroyMutex(b_interfaces.lock);
    b_interfaces.ready = false;
    BDBG_OBJECT_UNSET(&b_interfaces, NEXUS_ImgState);
    return;
}

/* don't allow unpredictable IO delays to cause driver failure. wait an extra
long time, but give status while we wait. */
static BERR_Code b_loop_wait(BKNI_EventHandle event, unsigned seconds, unsigned linenumber)
{
    BERR_Code rc = BERR_UNKNOWN;
    unsigned i;
    for (i=0;i<seconds;i++) {
        rc = BKNI_WaitForEvent(event, 1000); /* wait for application to return */
        if (b_interfaces.stopped) {
            return BERR_TRACE(BERR_TIMEOUT);
        }
        if (rc==BERR_SUCCESS) {
            return rc;
        }
        else if (rc == BERR_TIMEOUT) {
            BDBG_WRN(("waiting for IMG interface to respond at line %d", linenumber));
        }
        else {
            return BERR_TRACE(rc);
        }
    }
    return rc;
}

int nexus_img_wait_until_ready(void)
{
    BERR_Code rc;
    struct NEXUS_ImgState *interfaces = &b_interfaces;

    BDBG_OBJECT_ASSERT(interfaces, NEXUS_ImgState);
    /* wait until proxy's IMG thread is pending in the ioctl.
    if it times out, then we fail because NEXUS_Platform_Init would likely fail on FW loading. */
    rc = b_loop_wait(b_interfaces.ack, 60, __LINE__);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

void *
Nexus_IMG_Driver_Create(const char *id)
{
    BIMG_Driver *iface;

    if (b_strlen(id)>=BIMG_ID_MAX_LEN) {
        BDBG_ERR(("length of the string '%s' exceeds %d", id, BIMG_ID_MAX_LEN));
        return NULL;
    }
    iface = BKNI_Malloc(sizeof(*iface));
    if (!iface) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_AcquireMutex(b_interfaces.lock);
    iface->name = id;
    BLST_S_INSERT_HEAD(&b_interfaces.active, iface, link);
    BDBG_MSG(("adding %p(%s)", (void *)iface, iface->name));
    iface->image = NULL;
    BDBG_OBJECT_SET(iface, BIMG_Driver);
    BKNI_ReleaseMutex(b_interfaces.lock);

    return iface;
}

void
Nexus_IMG_Driver_Destroy(void *interface)
{
    BIMG_Driver *iface=interface;
    BDBG_OBJECT_ASSERT(iface, BIMG_Driver);
    BDBG_MSG(("-removing %p(%s)", (void *)iface, iface->name));
    BKNI_AcquireMutex(b_interfaces.lock);
    BLST_S_REMOVE(&b_interfaces.active, iface, BIMG_Driver, link);
    BKNI_ReleaseMutex(b_interfaces.lock);
    BDBG_MSG(("+removing %p(%s)", (void *)iface, iface->name));
    BDBG_OBJECT_DESTROY(iface, BIMG_Driver);
    BKNI_Free(iface);
    return;
}

static BERR_Code
b_send_req(BIMG_Driver *iface, BIMG_Ioctl_Req_Type type)
{
    BERR_Code rc;
    struct NEXUS_ImgState *interfaces = &b_interfaces;
    uint32_t sequence_no;

    BDBG_OBJECT_ASSERT(iface, BIMG_Driver);
    BDBG_OBJECT_ASSERT(interfaces, NEXUS_ImgState);

    sequence_no = b_interfaces.ioctl.req.sequence_no;
    sequence_no++;
    BKNI_AcquireMutex(b_interfaces.lock);
    b_interfaces.ioctl.req.sequence_no = sequence_no;
    b_interfaces.ioctl.req.req_type = type;
    b_interfaces.ioctl.ack.result = -1;
    b_strncpy(b_interfaces.ioctl.req.id, iface->name, BIMG_ID_MAX_LEN-1);
    b_interfaces.ioctl.req.id[BIMG_ID_MAX_LEN-1] = '\0';
    BKNI_ReleaseMutex(b_interfaces.lock);

    BDBG_MSG(("+req_type %d, id '%s waiting for ack' %#x", type, iface->name, (unsigned)sequence_no));
    BKNI_SetEvent(b_interfaces.req); /* wakeup process which waits in the  ioctl */
    rc = b_loop_wait(b_interfaces.ack, 60, __LINE__);
    if (rc!=BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }
    BDBG_MSG(("+req_type %d, id '%s' rc=%d (%#x,%#x,%#x)", type, iface->name, b_interfaces.ioctl.ack.result, (unsigned)sequence_no, (unsigned)b_interfaces.ioctl.req.sequence_no, (unsigned)b_interfaces.ioctl.ack.sequence_no));
    if(sequence_no!=b_interfaces.ioctl.req.sequence_no || b_interfaces.ioctl.ack.sequence_no!=sequence_no) {
        BDBG_ERR(("not matching sequence_no:(%#x,%#x,%#x) req_type(%u,%u)", (unsigned)sequence_no, (unsigned)b_interfaces.ioctl.req.sequence_no, (unsigned)b_interfaces.ioctl.ack.sequence_no, (unsigned)type, (unsigned)b_interfaces.ioctl.ack.req_type));
        return BERR_TRACE(BERR_OS_ERROR);
    }
    /* check return code from the user space */
    if (b_interfaces.ioctl.ack.result!=BERR_SUCCESS) {
        if ( b_interfaces.ioctl.ack.result == BERR_INVALID_PARAMETER ) {
            /* This error is common on 740x platforms.  Silently return. */
            BDBG_MSG(("Invalid Parameter Error returned.  Firmware image not available?"));
            return b_interfaces.ioctl.ack.result;
        } else {
            return BERR_TRACE(b_interfaces.ioctl.ack.result);
        }
    }

    return BERR_SUCCESS;
}

int
nexus_img_ioctl(unsigned int cmd, unsigned long arg)
{
    int result;
    BERR_Code rc;
    struct NEXUS_ImgState *interfaces = &b_interfaces;
    BIMG_Ioctl_Ack ack;

    BDBG_OBJECT_ASSERT(interfaces, NEXUS_ImgState);
    if (b_interfaces.stopped) {
        BDBG_MSG(("stop req"));
        return -1;
    }
    result = copy_from_user_small(&ack, &((BIMG_Ioctl *)arg)->ack, sizeof(ack));
    /* coverity[dead_error_condition] - it's important to do this check in case it's not a dead condition in the future. */
    if (result) {
        return BERR_TRACE(result);
    }
    BDBG_MSG(("<req_type:[%s] %u(%u) interfaces:%p sequence_no:%#x,%#x", b_interfaces.ioctl.req.id, ack.req_type, b_interfaces.ioctl.req.req_type, (void *)b_interfaces.opened, (unsigned)b_interfaces.ioctl.req.sequence_no, (unsigned)ack.sequence_no));
    switch(ack.ack_type) {
    case BIMG_Ioctl_Ack_Type_Reply:
        if(ack.req_type==BIMG_Ioctl_Req_Type_Next) {
            if(ack.result == BERR_SUCCESS) { /* copy data to the private buffer while in the right context */
                unsigned length = ack.data.next.length;
                if(length <= sizeof(b_interfaces.data)) {
                    result = copy_from_user_small(b_interfaces.data, (void *)(unsigned long)ack.data.next.data, length);
                    if (result == length) {
                        /* If you ask copy_from_user to copy more data than is available it will */
                        /* return the number of bytes uncopied. */
                        /* We only flag an error if uncopied bytes is how many we asked to be copied */
                        BDBG_WRN(("couldn't copy %u bytes", length));
                        ack.result = BERR_OS_ERROR;
                    }
                } else {
                    ack.result = BERR_TRACE(BERR_OS_ERROR);
                }
            }
        }
        BKNI_AcquireMutex(b_interfaces.lock);
        b_interfaces.ioctl.ack = ack;
        BKNI_ReleaseMutex(b_interfaces.lock);
        BKNI_SetEvent(b_interfaces.ack); /* set an event */
        break;
    case BIMG_Ioctl_Ack_Type_Wait:
        if(b_interfaces.ioctl.req.sequence_no==0) {
            BKNI_SetEvent(b_interfaces.ack);
            BDBG_MSG(("start req"));
        }
        break;
    case BIMG_Ioctl_Ack_Type_Exit:
        b_interfaces.stopped = true;
        BDBG_MSG(("exit req"));
        BKNI_SetEvent(b_interfaces.req); /* set an event */
        return 0; /* return right the way */
    }
    rc=BKNI_WaitForEvent(b_interfaces.req, 1000);
    if (rc!=BERR_SUCCESS) {
        BIMG_Ioctl_Req req;
        BKNI_Memset(&req, 0, sizeof(req));
        req.req_type = BIMG_Ioctl_Req_Type_Again;
        result = copy_to_user_small((void*)&((BIMG_Ioctl *)arg)->req,&req,sizeof(req));
    } else {
        BKNI_AcquireMutex(b_interfaces.lock);
        BDBG_MSG((">req_type:[%s] %u(%u) interfaces:%p sequence_no:%#x", b_interfaces.ioctl.req.id, ack.req_type, b_interfaces.ioctl.req.req_type, (void *)b_interfaces.opened, (unsigned)b_interfaces.ioctl.req.sequence_no));
        result = copy_to_user_small((void*)&((BIMG_Ioctl *)arg)->req,&b_interfaces.ioctl.req,sizeof(b_interfaces.ioctl.req));
        BKNI_ReleaseMutex(b_interfaces.lock);
    }
    if (result) {
        return BERR_TRACE(result);
    }
    return 0;
}

static void
Nexus_IMG_Driver_P_Close(BIMG_Driver *iface)
{
    BKNI_AcquireMutex(b_interfaces.lock);
    iface->image = NULL;
    b_interfaces.opened = NULL;
    b_interfaces.ioctl.req.id[0] = '\0';
    BKNI_ReleaseMutex(b_interfaces.lock);
    return;
}


/*
* This function may be called from multiple threads, yet we could only process single
* data flow, so we let only one Open through and others have to wait.
* As result each call (open, close and next) could expects that the user application
* waits for the ioctl, if it doesn't error returned right the way.
* This should help with kernel waiting for application which went bye-bye.
*/
BERR_Code
Nexus_IMG_Driver_Open(void *context, void **image, unsigned image_id)
{
    BERR_Code rc;
    BIMG_Driver *iface = context;
    unsigned i;
    bool acquired;

    BDBG_OBJECT_ASSERT(iface, BIMG_Driver);

    BDBG_MSG(("open> context %p'%s' id %u", (void *)iface, iface->name, image_id));
    if (iface->image!=NULL) {
        /* open shall be matched with close */
        return BERR_TRACE(BERR_OS_ERROR);
    }
    for(acquired=false,i=0;i<600;i++) { /* try to get through for ~60 seconds */
        BKNI_AcquireMutex(b_interfaces.lock);
        if(b_interfaces.opened==NULL) {
            b_interfaces.opened=iface;
            acquired=true;
        } else {
            BDBG_WRN(("Detected concurrent access from '%s' (opened '%s')", iface->name, b_interfaces.opened->name));
        }
        BKNI_ReleaseMutex(b_interfaces.lock);
        if(acquired) {
            break;
        }
        BKNI_WaitForEvent(b_interfaces.close, 100);
    }
    if(!acquired) {
        BDBG_ERR(("user/kernel BIMG proxy is already used"));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }
    b_interfaces.ioctl.req.data.open.image_id = image_id;
    rc = b_send_req(iface, BIMG_Ioctl_Req_Type_Open);
    if (rc!=BERR_SUCCESS) {
        if ( rc == BERR_INVALID_PARAMETER ) {
            /* This error is common on 740x platforms.  Silently return. */
            BDBG_MSG(("Invalid Parameter Error returned.  Firmware image not available?"));
            Nexus_IMG_Driver_P_Close(iface);
            return rc;
        } else {
            return BERR_TRACE(rc);
        }
    }
    iface->image = (void *)(unsigned long)b_interfaces.ioctl.ack.data.open.image;
    BDBG_MSG(("open< context %p'%s' id %u image %p", (void *)iface, iface->name, image_id, iface->image));
    *image = iface;

    return BERR_SUCCESS;
}

static BERR_Code
Nexus_P_IMG_TestOpened(const BIMG_Driver *iface)
{
    BERR_Code rc = BERR_SUCCESS;
    BKNI_AcquireMutex(b_interfaces.lock);
    if(b_interfaces.opened != iface) {
        BDBG_ERR(("'%s' accessing interface when only opened '%s'", iface->name, b_interfaces.opened?b_interfaces.opened->name:"NULL"));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BKNI_ReleaseMutex(b_interfaces.lock);
    return rc;
}

BERR_Code
Nexus_IMG_Driver_Next(void *image, unsigned chunk, const void **data, uint16_t length)
{
    BERR_Code rc;
    BIMG_Driver *iface = image;
    BDBG_OBJECT_ASSERT(iface, BIMG_Driver);

    BDBG_ASSERT(data);

    *data = NULL;
    rc = Nexus_P_IMG_TestOpened(iface);
    if(rc!=BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }
    if (iface->image==NULL) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    BDBG_MSG(("next> image %p'%s' chunk %u length %u", (void *)iface, iface->name, chunk, (unsigned)length));
    b_interfaces.ioctl.req.data.next.chunk = chunk;
    b_interfaces.ioctl.req.data.next.length = length;
    b_interfaces.ioctl.req.data.next.image = (unsigned long)iface->image;

    rc = b_send_req(iface, BIMG_Ioctl_Req_Type_Next);
    if (rc!=BERR_SUCCESS) {
        Nexus_IMG_Driver_P_Close(iface);
        return BERR_TRACE(rc);
    }

    BDBG_MSG(("next< image %p'%s' chunk %u length %u data %p", (void *)iface, iface->name, chunk, (unsigned)length, (void *)(unsigned long)b_interfaces.ioctl.ack.data.next.data));
    /* data was already copied in the ioctl handler */
    *data = b_interfaces.data;
    return BERR_SUCCESS;
}

void
Nexus_IMG_Driver_Close(void *image)
{
    BIMG_Driver *iface = image;
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(iface, BIMG_Driver);

    BDBG_MSG(("close> image %p'%s'", (void *)iface, iface->name));
    rc = Nexus_P_IMG_TestOpened(iface);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);return;}

    if (iface->image==NULL) {
        BDBG_ERR(("'%s' already closed", iface->name));
        return;
    }
    rc = b_send_req(iface, BIMG_Ioctl_Req_Type_Close);
    BDBG_MSG(("close< image %p'%s'", (void *)iface, iface->name));
    Nexus_IMG_Driver_P_Close(iface);
    return;
}

