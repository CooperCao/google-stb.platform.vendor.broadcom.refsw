/*
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#define ASP_MSG_PRE_FMT  "%d: %s():: "
#define ASP_MSG_PRE_ARG __LINE__, __FUNCTION__

#include <linux/module.h>   /* For module stuff */
#include <linux/types.h>    /* For standard types (like size_t) */
#include <linux/errno.h>    /* For the error values */
#include <linux/kernel.h>   /* For various kernel APIs such as printk/panic/ */
#include <linux/fs.h>       /* For file operations */
#include <linux/device.h>   /* For char dev specific types */
#include <linux/miscdevice.h>   /* For handling misc devices */
#include <linux/init.h>     /* For __init/__exit */
#include <linux/uaccess.h>  /* For copy_to_user/put_user */
#include <linux/cdev.h>     /* For char device APIs. */
#include <linux/slab.h>     /* For kzalloc */
#include <linux/spinlock.h> /* For spin lock APIs. */
#include <linux/mutex.h>    /* For mutex (semaphore) APIs. */
#include <linux/list.h>     /* For link list apis. */
#include <asm/atomic.h>     /* For atomic variable. */
#include <linux/wait.h>     /* For wait q APIs. */
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <uapi/linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/net_namespace.h> /* #defines init_net as extern & exported symbol */
#include <net/route.h>
#include <linux/netdevice.h>
#include "asp_netfilter_drv.h"

typedef struct aspDeviceCtx
{
    dev_t                   firstDeviceNum;
    struct cdev             cdev;
    struct class            *pClass;
    struct device           *pDevice;
    bool                    cdevAdded;
    spinlock_t              lock;                   /* Spin lock to synchronize access to the list of context. */
    struct list_head        chCtxListHead;
    atomic_t                curChCount;
    struct nf_hook_ops      ipv4NetFilterHookOps;
    struct nf_hook_ops      ipv4NetFilterHookOps1;
    struct nf_hook_ops      ipv4NetFilterHookOps2;
    struct nf_hook_ops      ipv6NetFilterHookOps;
} AspDeviceCtx;
static AspDeviceCtx *g_pAspDeviceCtx = NULL;

typedef struct aspChannelCtx
{
    struct mutex            mutex;                  /* semaphore to serialize access to ioctl/read/poll calls on the same context. */
    AspDeviceCtx            *pAspDeviceCtx;
    struct list_head        chCtxListNode;
    /* TOD0 make this atomic. */
    atomic_t                socket5TupleInfoValid;  /* Gets set during ADD Socket5TupleInfo ioctl & cleared during REMOVE ioctl. */
    ASP_Socket5TupleInfo    socket5TupleInfo;
    struct sk_buff_head     rxPktQueueHead;
    wait_queue_head_t       rxPktWaitQueueHead;     /* Wait queue for waiting on new filtered pkts. */
    bool                    pktDropped;
    unsigned                droppedPktCount;
    ASP_PktMatchAction      action;
    atomic_t                numPktsToDrop;  /* -1 to drop all. */
    atomic_t                origNumPktsToDrop;  /* -1 to drop all. */
    atomic_t                pktPositionToDrop;
    unsigned                pktRcvd;
    bool                    startPktDrop;
} AspChannelCtx;

/*
 * The driver model is to have only 1 major & minor device. App can open multiple instances of this device each representing
 * one ASP Channel. Driver will allocate a new AspChannelCtx for each such open and associate it in the file_private.
 * Subsequent API calls then get this pointer via the file pointer.
 */

/* TODO:
 *  - Add Hdr offset support,
 *  - Try out search using exclusive ORs & study the performance.
 *  - Enhance unit test to include the sending packets, use telnet for TCP & udp,
 *  - Integrate with ASP Manager Test program.
 *  - Add sysfs entries for outputing stats, debug, etc.
 *  - IPv6 support.
 * */

/* Forward declarations. */
static int              ASP_Channel_Open(struct inode *, struct file *);
static int              ASP_Channel_Release(struct inode *, struct file *);
static long             ASP_Channel_Ioctl(struct file *, unsigned int, unsigned long);
static ssize_t          ASP_Channel_Read(struct file *, char *, size_t, loff_t *);
static unsigned int     ASP_Channel_Poll(struct file *, poll_table *);
#if 0
static ssize_t          ASP_Channel_Readv(struct kiocb *, const struct iovec *, unsigned long, loff_t);
#endif
static unsigned int aspIpv4NetfilterHookCallback(const struct nf_hook_ops *ops, struct sk_buff *pSkb, const struct nf_hook_state *state);
#if 0
static unsigned int aspIpv4NetfilterHookCallback1(const struct nf_hook_ops *ops, struct sk_buff *pSkb, const struct nf_hook_state *state);
#endif
static unsigned int aspIpv4NetfilterHookCallback2(const struct nf_hook_ops *ops, struct sk_buff *pSkb, const struct nf_hook_state *state);


static const struct file_operations g_aspFileOperations =
{
    .owner          = THIS_MODULE,
    .open           = ASP_Channel_Open,
    .release        = ASP_Channel_Release,
    .unlocked_ioctl = ASP_Channel_Ioctl,
    .read           = ASP_Channel_Read,
    .poll           = ASP_Channel_Poll
#if 0
    /* TODO: */
    .aio_read       = ASP_Channel_Readv,
#endif
};

static void module_cleanup(
    AspDeviceCtx *pAspDeviceCtx
    )
{

    if (pAspDeviceCtx == NULL) return;

    if (pAspDeviceCtx->ipv4NetFilterHookOps.hook) nf_unregister_hook(&pAspDeviceCtx->ipv4NetFilterHookOps);
    pAspDeviceCtx->ipv4NetFilterHookOps.hook = NULL;
    if (pAspDeviceCtx->ipv4NetFilterHookOps2.hook) nf_unregister_hook(&pAspDeviceCtx->ipv4NetFilterHookOps2);
    pAspDeviceCtx->ipv4NetFilterHookOps2.hook = NULL;


    /* delete the device from the file system. */
    if (pAspDeviceCtx->cdevAdded) cdev_del(&pAspDeviceCtx->cdev);
    pAspDeviceCtx->cdevAdded = false;

    if (pAspDeviceCtx->pDevice) device_destroy(pAspDeviceCtx->pClass, pAspDeviceCtx->firstDeviceNum);
    pAspDeviceCtx->pDevice = NULL;

    if (pAspDeviceCtx->pClass) class_destroy(pAspDeviceCtx->pClass);
    pAspDeviceCtx->pClass = NULL;

    unregister_chrdev_region(pAspDeviceCtx->firstDeviceNum, ASP_MINOR_DEVICES_COUNT);

    pr_info(ASP_MSG_PRE_FMT "pAspDeviceCtx=%p\n", ASP_MSG_PRE_ARG, pAspDeviceCtx);
}

static int ASP_Drv_Init(
    void
    )
{
    AspDeviceCtx *pAspDeviceCtx = NULL;
    int err;

    pAspDeviceCtx = (AspDeviceCtx *)kzalloc(sizeof(*pAspDeviceCtx), GFP_KERNEL);
    if (!pAspDeviceCtx)
    {
        pr_err(ASP_MSG_PRE_FMT "kzalloc() failed for %d bytes!\n", ASP_MSG_PRE_ARG, sizeof(*pAspDeviceCtx));
        return -ENOMEM;
    }
    g_pAspDeviceCtx = pAspDeviceCtx;
    spin_lock_init(&pAspDeviceCtx->lock);
    INIT_LIST_HEAD(&pAspDeviceCtx->chCtxListHead);

    /* Dynamically allocate the major device number. */
    {
        err = alloc_chrdev_region(&pAspDeviceCtx->firstDeviceNum, 0 /*firstMinor*/, ASP_MINOR_DEVICES_COUNT, ASP_DEVICE_NAME);
        if (err != 0)
        {
            pr_err(ASP_MSG_PRE_FMT "alloc_chrdev_region() failed: err=%d minor device count=%d, deviceName=%s\n", ASP_MSG_PRE_ARG, err, ASP_MINOR_DEVICES_COUNT, ASP_DEVICE_NAME);
            goto error;
        }
        pr_info(ASP_MSG_PRE_FMT "alloc_chrdev_region(): minor device count=%d, deviceName=%s: major=%d minor=%d\n", ASP_MSG_PRE_ARG, ASP_MINOR_DEVICES_COUNT, ASP_DEVICE_NAME, MAJOR(pAspDeviceCtx->firstDeviceNum), MINOR(pAspDeviceCtx->firstDeviceNum) );
    }

    /* Now dynamically create the device node in the /dev file system. This is in lieu of calling mknod. */
    {
        /* Create an entry in /sys/class/ASP_DEVICE_NAME first. */
        pAspDeviceCtx->pClass = class_create(THIS_MODULE, ASP_DEVICE_NAME);
        if (pAspDeviceCtx->pClass == NULL)
        {
            pr_err(ASP_MSG_PRE_FMT "class_create() failed to create /sys/class entry: className=%s\n", ASP_MSG_PRE_ARG, ASP_DEVICE_NAME);
            goto error;
        }

        /* Now create a device entry and register it w/ the sysfs. */
        pAspDeviceCtx->pDevice = device_create(pAspDeviceCtx->pClass, NULL /* parent */, pAspDeviceCtx->firstDeviceNum, NULL/*TODO: drvdata, may need to add this when sysfs support is added. */, ASP_DEVICE_NAME);
        if (IS_ERR(pAspDeviceCtx->pDevice))
        {
            pr_err(ASP_MSG_PRE_FMT "device_create() failed: err=%ld for deviceName=%s\n", ASP_MSG_PRE_ARG, PTR_ERR(pAspDeviceCtx->pDevice), ASP_DEVICE_NAME);
            pAspDeviceCtx->pDevice = NULL;
            goto error;
        }
        pr_info(ASP_MSG_PRE_FMT "device_create() success: created in /dev/%s & /sys/class/%s nodes!\n", ASP_MSG_PRE_ARG, ASP_DEVICE_NAME, ASP_DEVICE_NAME);
    }

    /* Register this character device w/ Kernel. */
    {
        /* Initialze the cdev device. */
        cdev_init(&pAspDeviceCtx->cdev, &g_aspFileOperations);
        pAspDeviceCtx->cdev.owner = THIS_MODULE;

        /* Add it to the Kernel. */
        err = cdev_add(&pAspDeviceCtx->cdev, pAspDeviceCtx->firstDeviceNum, ASP_MINOR_DEVICES_COUNT);
        if ( err != 0)
        {
            pr_err(ASP_MSG_PRE_FMT "cdev_add() failed to register device: err=%d deviceName=%s, major=%d minor=%d\n", ASP_MSG_PRE_ARG, err, ASP_DEVICE_NAME, MAJOR(pAspDeviceCtx->firstDeviceNum), MINOR(pAspDeviceCtx->firstDeviceNum));
            goto error;
        }
        pAspDeviceCtx->cdevAdded = true;
        /* At this point, driver open API can be invoked by a user app. So driver better be ready to go! */
    }

    /* ASP Device is initialized such that a user space app can now open a ASP Channel & send it any commands. */
    /* So we now register a callback function with the netfilter hook to receive all IP pkts for local i/f & filter any matching the ASP channel's 5-tuple. */
    {
        pAspDeviceCtx->ipv4NetFilterHookOps.hook = aspIpv4NetfilterHookCallback;
        pAspDeviceCtx->ipv4NetFilterHookOps.hooknum = NF_INET_LOCAL_IN;      /* We are only interested in the packets destinated for local interface. */
        pAspDeviceCtx->ipv4NetFilterHookOps.pf = AF_INET;                    /* AF_INET Protocol family. */
        pAspDeviceCtx->ipv4NetFilterHookOps.priority = NF_IP_PRI_FIRST;      /* Set to highest priority. */
        pAspDeviceCtx->ipv4NetFilterHookOps.owner = THIS_MODULE;
        pAspDeviceCtx->ipv4NetFilterHookOps.priv = pAspDeviceCtx;
        err = nf_register_hook(&pAspDeviceCtx->ipv4NetFilterHookOps);
        if ( err != 0)
        {
            pr_err(ASP_MSG_PRE_FMT "nf_register_hook() failed to register netfilter callback function: err=%d\n", ASP_MSG_PRE_ARG, err);
            goto error;
        }
        /* Note: kernel will start calling our hook function at this point. However, since driver initialization hasn't finished, */
        /* the list of ASP Channel contexts would be empty, so packets would just be handled back to the Netfilter logic of Kernel. */
    }

    /* TODO: Register another hook function for IPv6 packets as well. */

    pr_info(ASP_MSG_PRE_FMT "Success: g_pAspDeviceCtx=%p\n", ASP_MSG_PRE_ARG, g_pAspDeviceCtx);
    return 0;

error:
    pr_info(ASP_MSG_PRE_FMT "Failed: err=%d\n", ASP_MSG_PRE_ARG, err);
    module_cleanup(pAspDeviceCtx);
    if (pAspDeviceCtx) kfree(pAspDeviceCtx);
    return (err);
}

static void ASP_Drv_Uninit(
    void
    )
{
    AspDeviceCtx *pAspDeviceCtx = NULL;

    pAspDeviceCtx = g_pAspDeviceCtx;
    pr_info(ASP_MSG_PRE_FMT "pAspDeviceCtx=%p\n", ASP_MSG_PRE_ARG, pAspDeviceCtx);

    module_cleanup(pAspDeviceCtx);
    kfree(pAspDeviceCtx);
}

/*
 * Check if current task that has invoked the driver API has the correct privilege.
 *
 * Returns 0 if that is the case, errno of -EPERM otherwise.
 */
static int authenticateProcessContext(
    void
    )
{
    if (!capable(CAP_NET_ADMIN))
    {
        pr_err(ASP_MSG_PRE_FMT "EPERM: current task doesn't have superuser or CAP_NET_ADMIN privilege!\n", ASP_MSG_PRE_ARG);
        return -EPERM;
    }

    return 0;
}

/*
 * Open a per channel context and insert it in the list of contexts.
 *
 * Returns 0 upon success, -errno otherwise.
 */
static int ASP_Channel_Open(
    struct inode    *pInode,
    struct file     *pFile
    )
{
    int             err = 0;
    AspDeviceCtx    *pAspDeviceCtx = NULL;
    AspChannelCtx   *pAspChannelCtx = NULL;

    /* Make sure only privileged or super-user process can open a device context. */
    if ((err = authenticateProcessContext())) return (err);

    pAspDeviceCtx = g_pAspDeviceCtx;

    /* Allocate Channel specific context. */
    pAspChannelCtx = kzalloc(sizeof(*pAspChannelCtx), GFP_KERNEL);
    if (!pAspChannelCtx)
    {
        pr_err(ASP_MSG_PRE_FMT "kzalloc() failed for %d bytes!\n", ASP_MSG_PRE_ARG, sizeof(*pAspChannelCtx));
        return -ENOMEM;
    }
    pAspChannelCtx->pAspDeviceCtx = pAspDeviceCtx;
    atomic_inc(&pAspDeviceCtx->curChCount);
    mutex_init(&pAspChannelCtx->mutex);

    /* Insert Channel context in list of channel contexts. */
    /* Even though context is added to the list, it is not active until socket5tuple info is added for this context. */
    spin_lock_bh(&pAspDeviceCtx->lock);
    list_add(&pAspChannelCtx->chCtxListNode, &pAspDeviceCtx->chCtxListHead);
    spin_unlock_bh(&pAspDeviceCtx->lock);

    /* Store the channel ctx pointer so that we can retrieve it in the subsequent driver APIs. */
    pFile->private_data = pAspChannelCtx;

    if (0)
    {
        pAspDeviceCtx->ipv4NetFilterHookOps2.hook = aspIpv4NetfilterHookCallback2;
        pAspDeviceCtx->ipv4NetFilterHookOps2.hooknum = NF_INET_LOCAL_OUT;   /* Pre-routing hook before packets are passed to the input routing logic. */
        pAspDeviceCtx->ipv4NetFilterHookOps2.pf = AF_INET;                    /* AF_INET Protocol family. */
        pAspDeviceCtx->ipv4NetFilterHookOps2.priority = NF_IP_PRI_FIRST;      /* Set to highest priority. */
        pAspDeviceCtx->ipv4NetFilterHookOps2.owner = THIS_MODULE;
        pAspDeviceCtx->ipv4NetFilterHookOps2.priv = pAspDeviceCtx;
        err = nf_register_hook(&pAspDeviceCtx->ipv4NetFilterHookOps2);
        if ( err != 0)
        {
            pr_err(ASP_MSG_PRE_FMT "############################# nf_register_hook() failed to register netfilter callback function: err=%d\n", ASP_MSG_PRE_ARG, err);
        }
    pr_info(ASP_MSG_PRE_FMT ">>>>>>>>>>>>>>>>>>>>>>>>>>>>> pre-routing hook installed.................>>>>>>>>>>>>>>>>>>>", ASP_MSG_PRE_ARG);
        /* Note: kernel will start calling our hook function at this point. However, since driver initialization hasn't finished, */
        /* the list of ASP Channel contexts would be empty, so packets would just be handled back to the Netfilter logic of Kernel. */
    }
#if 0
    /* Register a callback function with the netfilter PRE_ROUTING hook for incoming packets. */
    /* It will filter any packets with source IP of ASP IP & change it to the host IP so that TCP layer can find a match for the exising CLIENT side socket. */
    if (0)
    {
        pAspDeviceCtx->ipv4NetFilterHookOps1.hook = aspIpv4NetfilterHookCallback1;
        pAspDeviceCtx->ipv4NetFilterHookOps1.hooknum = NF_INET_PRE_ROUTING;   /* Pre-routing hook before packets are passed to the input routing logic. */
        pAspDeviceCtx->ipv4NetFilterHookOps1.pf = AF_INET;                    /* AF_INET Protocol family. */
        pAspDeviceCtx->ipv4NetFilterHookOps1.priority = NF_IP_PRI_FIRST;      /* Set to highest priority. */
        pAspDeviceCtx->ipv4NetFilterHookOps1.owner = THIS_MODULE;
        pAspDeviceCtx->ipv4NetFilterHookOps1.priv = pAspDeviceCtx;
        err = nf_register_hook(&pAspDeviceCtx->ipv4NetFilterHookOps1);
        if ( err != 0)
        {
            pr_err(ASP_MSG_PRE_FMT "############################# nf_register_hook() failed to register netfilter callback function: err=%d\n", ASP_MSG_PRE_ARG, err);
        }
    pr_info(ASP_MSG_PRE_FMT ">>>>>>>>>>>>>>>>>>>>>>>>>>>>> pre-routing hook installed.................>>>>>>>>>>>>>>>>>>>", ASP_MSG_PRE_ARG);
        /* Note: kernel will start calling our hook function at this point. However, since driver initialization hasn't finished, */
        /* the list of ASP Channel contexts would be empty, so packets would just be handled back to the Netfilter logic of Kernel. */
    }
#endif

    /* All is well, so return success */
    err = 0;
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p pFile=%p curChCount=%d \n", ASP_MSG_PRE_ARG, pAspChannelCtx, pFile, atomic_read(&pAspDeviceCtx->curChCount) );
    return (err);
}

/*
 * Release resources associated with a channel context. This includes removing it from the global list of contexts.
 *
 * Note: Kernel ensures that it only calls release when there is no other API in progress for this context and
 * also when this pFile handle is no longer shared via the dup/fork/fd-passing w/ another process.
 *
 * Returns 0 upon success, -errno otherwise.
 */
static int ASP_Channel_Release(
    struct inode    *pInode,
    struct file     *pFile
    )
{
    int             err = 0;
    AspDeviceCtx    *pAspDeviceCtx = NULL;
    AspChannelCtx   *pAspChannelCtx = NULL;

    /* Make sure only privileged or super-user process can open a device context. */
    if ((err = authenticateProcessContext())) return (err);

    pAspDeviceCtx = g_pAspDeviceCtx;
    pAspChannelCtx = pFile->private_data;
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p curChCount=%d \n", ASP_MSG_PRE_ARG, pAspChannelCtx, atomic_read(&pAspDeviceCtx->curChCount) );

    /* Remove Channel context from the list of channel contexts. */
    spin_lock_bh(&pAspDeviceCtx->lock);
    list_del(&pAspChannelCtx->chCtxListNode);
    spin_unlock_bh(&pAspDeviceCtx->lock);

    atomic_dec(&pAspDeviceCtx->curChCount);

    /* Release any queued up packets incase ASP_CHANNEL_IOC_DEL_SOCKET_5TUPLE_INFO ioctl cmd wasn't called. */
    skb_queue_purge(&pAspChannelCtx->rxPktQueueHead);

    /* Clear the file pointer and release the object. */
    pFile->private_data = NULL;
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p curChCount=%d Done!\n", ASP_MSG_PRE_ARG, pAspChannelCtx, atomic_read(&pAspDeviceCtx->curChCount) );
    kfree(pAspChannelCtx);

    /* All is well, so return success */
    err = 0;
    return (err);
}

/*
 * Process the ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO command where it copies the socket 5 tuple info from user to per context structure.
 *
 * Note: since this function calls copy_from_user, it can sleep during that call. So caller can't hold any spin lock but semaphore/mutex
 * is fine.
 *
 * Returns 0 on success, -error otherwise.
 */
static int processSetSocket5TupleInfo_mutex_held(
    AspChannelCtx   *pAspChannelCtx,
    unsigned int    cmd,
    unsigned long   arg
    )
{
    int err = 0;

    if (copy_from_user(&pAspChannelCtx->socket5TupleInfo, (void __user *)(arg), sizeof(pAspChannelCtx->socket5TupleInfo)))
    {
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p copy_from_user failed for ioctl ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO\n",
                ASP_MSG_PRE_ARG, pAspChannelCtx);
        return -EFAULT;
    }

    /* Validate the params. */
    if (pAspChannelCtx->socket5TupleInfo.l4Protocol >= ASP_ChLayer4Protocol_eMax)
    {
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p Invalid param: l4Protocol=%d is > Max allowed (=%d) \n",
                ASP_MSG_PRE_ARG, pAspChannelCtx, pAspChannelCtx->socket5TupleInfo.l4Protocol, ASP_ChLayer4Protocol_eMax);
        return -EINVAL;
    }
    if (pAspChannelCtx->socket5TupleInfo.ipVersion >= ASP_ChLayer3IpVersion_eMax)
    {
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p Invalid param: ipVersion=%d is > Max allowed (=%d) \n",
                ASP_MSG_PRE_ARG, pAspChannelCtx, pAspChannelCtx->socket5TupleInfo.ipVersion, ASP_ChLayer3IpVersion_eMax);
        return -EINVAL;
    }
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p l4Protocol=%s ipVersion=%s srcIp:Port=%d.%d.%d.%d:%u dstIp:Port=%d.%d.%d.%d:%u aspIp=%d.%d.%d.%d \n",
            ASP_MSG_PRE_ARG, pAspChannelCtx,
            pAspChannelCtx->socket5TupleInfo.l4Protocol == ASP_ChLayer4Protocol_eTCP ? "TCP" :
            pAspChannelCtx->socket5TupleInfo.l4Protocol == ASP_ChLayer4Protocol_eUDP ? "UDP" :
            "other protocol",
            pAspChannelCtx->socket5TupleInfo.ipVersion == ASP_ChLayer3IpVersion_eIpv4 ? "IPv4" :
            pAspChannelCtx->socket5TupleInfo.ipVersion == ASP_ChLayer3IpVersion_eIpv6 ? "IPv4" :
            "other version",
            IPv4(pAspChannelCtx->socket5TupleInfo.srcIpAddr[0]), ntohs(pAspChannelCtx->socket5TupleInfo.srcPort),
            IPv4(pAspChannelCtx->socket5TupleInfo.dstIpAddr[0]), ntohs(pAspChannelCtx->socket5TupleInfo.dstPort),
            IPv4(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
           );
    err = 0;
    return err;
}

/*
 * Process the ASP_CHANNEL_IOC_SET_PKT_CONTROL command where it copies the socket 5 tuple info from user to per context structure.
 *
 * Note: since this function calls copy_from_user, it can sleep during that call. So caller can't hold any spin lock but semaphore/mutex
 * is fine.
 *
 * Returns 0 on success, -error otherwise.
 */
static int processPktControl_mutex_held(
    AspChannelCtx   *pAspChannelCtx,
    unsigned int    cmd,
    unsigned long   arg
    )
{
    int err = 0;
    ASP_ChannelPktControl pktControl;

    if (copy_from_user(&pktControl, (void __user *)(arg), sizeof(ASP_ChannelPktControl)))
    {
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p copy_from_user failed for ioctl ASP_CHANNEL_IOC_PKT_CONTROL\n",
                ASP_MSG_PRE_ARG, pAspChannelCtx);
        return -EFAULT;
    }

    /* Validate the params. */
    if (pktControl.action >= ASP_PktMatchAction_eMax)
    {
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p Invalid action param=%d is > Max allowed (=%d) \n",
                ASP_MSG_PRE_ARG, pAspChannelCtx, pktControl.action, ASP_PktMatchAction_eMax);
        return -EINVAL;
    }

    pAspChannelCtx->action = pktControl.action;
    atomic_set(&pAspChannelCtx->numPktsToDrop, pktControl.numPktsToDrop);
    atomic_set(&pAspChannelCtx->origNumPktsToDrop, pktControl.numPktsToDrop);
    atomic_set(&pAspChannelCtx->pktPositionToDrop, pktControl.pktPositionToDrop);
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p action=%d  numPktsToDrop=%d pktPositionToDrop=%d\n", ASP_MSG_PRE_ARG, pAspChannelCtx,pktControl.action, pktControl.numPktsToDrop, pktControl.pktPositionToDrop);
    err = 0;
    return err;
}
/*
 * API to return the socket 5 tuple info to user app.
 *
 * Returns 0 on success, -error otherwise.
 */
static int processGetSocket5TupleInfo_locked(
    AspChannelCtx   *pAspChannelCtx,
    unsigned int    cmd,
    unsigned long   arg
    )
{
    int err = 0;

    if (copy_to_user((void __user *)(arg), &pAspChannelCtx->socket5TupleInfo, sizeof(pAspChannelCtx->socket5TupleInfo)))
    {
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p copy_to_user failed for ioctl ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO\n",
                ASP_MSG_PRE_ARG, pAspChannelCtx);
        return -EFAULT;
    }

    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p l4Protocol=%s ipVersion=%s srcIp:Port=%d.%d.%d.%d:%u dstIp:Port=%d.%d.%d.%d:%u \n",
            ASP_MSG_PRE_ARG, pAspChannelCtx,
            pAspChannelCtx->socket5TupleInfo.l4Protocol == ASP_ChLayer4Protocol_eTCP ? "TCP" :
            pAspChannelCtx->socket5TupleInfo.l4Protocol == ASP_ChLayer4Protocol_eUDP ? "UDP" :
            "other protocol",
            pAspChannelCtx->socket5TupleInfo.ipVersion == ASP_ChLayer3IpVersion_eIpv4 ? "IPv4" :
            pAspChannelCtx->socket5TupleInfo.ipVersion == ASP_ChLayer3IpVersion_eIpv6 ? "IPv4" :
            "other version",
            IPv4(pAspChannelCtx->socket5TupleInfo.srcIpAddr[0]), ntohs(pAspChannelCtx->socket5TupleInfo.srcPort),
            IPv4(pAspChannelCtx->socket5TupleInfo.dstIpAddr[0]), ntohs(pAspChannelCtx->socket5TupleInfo.dstPort)
           );
    err = 0;
    return err;
}

/*
 * API to process various ioctl commands.
 *
 * Returns 0 on success, -error otherwise.
 */
static long ASP_Channel_Ioctl(
    struct file *pFile,
    unsigned int cmd,
    unsigned long arg
    )
{
    int             err = 0;
    AspDeviceCtx    *pAspDeviceCtx = NULL;
    AspChannelCtx   *pAspChannelCtx = NULL;

    /* Make sure only privileged or super-user process can open a device context. */
    if ((err = authenticateProcessContext())) return (err);

    /* Validate the command & its args. */
    {
        /* Note: For invalid ioctl command, we return ENOTTY instead of EINVAL as it causes C library to return 'inappropriate ioctl for device' to the app. */
        if (_IOC_TYPE(cmd) != ASP_DEVICE_MAGIC_TYPE)
        {
            return -ENOTTY;
        }

        if (_IOC_NR(cmd) > ASP_DEVICE_IOC_MAXNR)
        {
            return -ENOTTY;
        }

        if (_IOC_DIR(cmd) & _IOC_READ)
        {
            err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
        }
        else if (_IOC_DIR(cmd) & _IOC_WRITE)
        {
            pr_info(ASP_MSG_PRE_FMT "check if arg=0x%lx has correct access & size=%d",
                    ASP_MSG_PRE_ARG, arg, _IOC_SIZE(cmd));
            err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd)); /* Note: it doesn't really seem to verify if user had correct size on "arg" . */
        }

        if (err)
        {
            pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p cmd=0x%x cmd_number=%d access_ok failed!\n", ASP_MSG_PRE_ARG, pAspChannelCtx, cmd, _IOC_NR(cmd));
            return -EFAULT;
        }

    }

    pAspDeviceCtx = g_pAspDeviceCtx;
    pAspChannelCtx = pFile->private_data;
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p cmd=0x%x cmd_num=%d\n", ASP_MSG_PRE_ARG, pAspChannelCtx, cmd, _IOC_NR(cmd));

    /* Note: we only need to get mutex here & not spin lock. mutex allows us to possibly sleep during the copying */
    /* of arguments between user & kernel space. Also, we avoid the need of spin lock by atomically setting & clearing the */
    /* socket5TupleInfoValid flag that is also used by the hook callback function to check if a context contains a valid socket 5tuple info. */
    if (mutex_lock_interruptible(&pAspChannelCtx->mutex))
    {
        return -ERESTARTSYS; /* we got interrupted due to a signal while waiting, so ask upper layer to restart this read call. */
    }

    /* Now process the commands. */
    switch (cmd)
    {
        case ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO:
            err = processSetSocket5TupleInfo_mutex_held(pAspChannelCtx, cmd, arg);
            if (err == 0)
            {
                /* Socket 5 tuple is successfully set, so finish the per filter flow related initialization. */
                skb_queue_head_init(&pAspChannelCtx->rxPktQueueHead);
                init_waitqueue_head(&pAspChannelCtx->rxPktWaitQueueHead);

                /* Set the valid flag. hook callback function will no longer queue up any packets for this context. */
                atomic_set(&pAspChannelCtx->socket5TupleInfoValid, 1);
            }
            break;

        case ASP_CHANNEL_IOC_DEL_SOCKET_5TUPLE_INFO:
            skb_queue_purge(&pAspChannelCtx->rxPktQueueHead);

            /* Clear the valid flag. hook callback function can now queue matching packets for this context. */
            atomic_set(&pAspChannelCtx->socket5TupleInfoValid, 0);
            err = 0;
            break;

        case ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO:
            if (atomic_read(&pAspChannelCtx->socket5TupleInfoValid) == 1)
            {
                err = processGetSocket5TupleInfo_locked(pAspChannelCtx, cmd, arg);
            }
            else
            {
                err = -EINVAL;
                pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p Can't get socket5TupleInfo w/o adding it first.\n",
                        ASP_MSG_PRE_ARG, pAspChannelCtx);
            }
            break;

        case ASP_CHANNEL_IOC_PKT_CONTROL:
            err = processPktControl_mutex_held(pAspChannelCtx, cmd, arg);
            break;

        default:
            pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p cmd=0x%x cmd_number=%d not supported!\n", ASP_MSG_PRE_ARG, pAspChannelCtx, cmd, _IOC_NR(cmd));
            err = -ENOTTY;
    }
    mutex_unlock(&pAspChannelCtx->mutex);

    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p err=%d \n", ASP_MSG_PRE_ARG, pAspChannelCtx, err);
    return (err);
}

static void printIpv4SocketTupleInfo(
    const char *pString,
    ASP_Socket5TupleInfo *pSocket5TupleInfo
    )
{
    if (printk_ratelimit()) printk(KERN_INFO "%s: protocol: %s srcIp:Port -> dstIp:Port %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n",
            pString, pSocket5TupleInfo->l4Protocol == ASP_ChLayer4Protocol_eTCP ? "TCP": "UDP",
            IPv4(pSocket5TupleInfo->srcIpAddr[0]), ntohs(pSocket5TupleInfo->srcPort),
            IPv4(pSocket5TupleInfo->dstIpAddr[0]), ntohs(pSocket5TupleInfo->dstPort)
            );
}

/*
 * Match the socket 5-tuple related info of a incoming pkt to a ASP Channel's info.
 *
 * Returns true if match is found, false otherwise.
 */
static bool matchSocket5TupleInfo(
    ASP_Socket5TupleInfo *pPktSocket5TupleInfo,
    ASP_Socket5TupleInfo *pAspChSocket5TupleInfo
    )
{
    /* TODO: need to profile this & compare it w/ the comparision using exclusive OR. */
    if (pPktSocket5TupleInfo->l4Protocol != pAspChSocket5TupleInfo->l4Protocol)
        return false;

    if (pPktSocket5TupleInfo->ipVersion != pAspChSocket5TupleInfo->ipVersion)
        return false;

    if (pPktSocket5TupleInfo->srcPort != pAspChSocket5TupleInfo->srcPort)
        return false;

    if (pPktSocket5TupleInfo->dstPort != pAspChSocket5TupleInfo->dstPort)
        return false;

    if (pPktSocket5TupleInfo->srcIpAddr[0] != pAspChSocket5TupleInfo->srcIpAddr[0])
        return false;
    /* TODO: for IPv6, match additional address fields. */

    if (pPktSocket5TupleInfo->dstIpAddr[0] != pAspChSocket5TupleInfo->dstIpAddr[0])
        return false;

    return true;
}

/*
 * Extracts the socket 5-tuple related info from a socket buffer.
 *
 * Returns 1 if error, 0 otherwise.
 */
static int parseIpv4Packet(
    const struct sk_buff *pSkb,
    ASP_Socket5TupleInfo *pSocket5TupleInfo
    )
{
    struct iphdr *pIpHdr;

    if (!pSocket5TupleInfo || !pSkb)
    {
        return -1;
    }

    memset(pSocket5TupleInfo, 0, sizeof(*pSocket5TupleInfo));

    /* Parse IP Header. */
    pIpHdr = (struct iphdr *) skb_network_header(pSkb);
    pSocket5TupleInfo->srcIpAddr[0] = pIpHdr->saddr;
    pSocket5TupleInfo->dstIpAddr[0] = pIpHdr->daddr;
    pSocket5TupleInfo->ipVersion = ASP_ChLayer3IpVersion_eIpv4;

    /* Parse L4 Header. */
    if (pIpHdr->protocol == IPPROTO_TCP)
    {
        struct tcphdr *pTcpHdr;

        pSocket5TupleInfo->l4Protocol = ASP_ChLayer4Protocol_eTCP;
        pTcpHdr = (struct tcphdr *) skb_transport_header(pSkb);
        pSocket5TupleInfo->srcPort = pTcpHdr->source;
        pSocket5TupleInfo->dstPort = pTcpHdr->dest;
    }
    else if (pIpHdr->protocol == IPPROTO_UDP)
    {
        struct udphdr *pUdpHdr;

        pSocket5TupleInfo->l4Protocol = ASP_ChLayer4Protocol_eUDP;
        pUdpHdr = (struct udphdr *)skb_transport_header(pSkb);
        pSocket5TupleInfo->srcPort = pUdpHdr->source;
        pSocket5TupleInfo->dstPort = pUdpHdr->dest;
    }
    else
    {
        return -1;
    }

    return 0;
}

static uint16_t calculateIncrementalChecksum(uint16_t curCsum, uint32_t curValue, uint32_t newValue)
{
    /* This logic comes from RFC1624.
     * Notations:
        HC  - old checksum in header
        C   - one's complement sum of old header
        HC' - new checksum in header
        C'  - one's complement sum of new header
        m   - old value of a 16-bit field
        m'  - new value of a 16-bit field

     * Checksum calculation:
        HC' = ~(C + (-m) + m')
            = ~(~HC + ~m + m')
    */
    uint32_t tempSum;
    uint16_t newCsum;

    curCsum = ~curCsum;
    curValue = ~curValue;
    tempSum = (__u32)curCsum + (curValue>>16) + (curValue&0xffff) + (newValue>>16) + (newValue&0xffff);
    newCsum = htons(~( (__u16)(tempSum>>16) + (tempSum&0xffff) ));
    if (printk_ratelimit()) printk("cur check=0x%x newCsum=0x%x\n", curCsum, newCsum);
    return (newCsum);
}

/*
 * Callback function invoked by netfilter hook for all local IP packets.
 *
* Returns NF_STOLEN if pkt is matches with a ASP Channel socket tuple info, NF_ACCEPT otherwise.
 */
static unsigned int aspIpv4NetfilterHookCallback(
    const struct nf_hook_ops *ops,
    struct sk_buff *pSkb,
    const struct nf_hook_state *state
    )
{
    int                     pktStatus;
    ASP_Socket5TupleInfo    socket5TupleInfo;
    AspDeviceCtx            *pAspDeviceCtx;
    AspChannelCtx           *pAspChannelCtx;
    bool                    pktParsed = false;

    if (!pSkb || !ops) return NF_ACCEPT;
    pAspDeviceCtx = ops->priv;

    /*
     * Check if a packet matches w/ the socket tuple info of any of the currently opened ASP Channels.
     */
    pktStatus = NF_ACCEPT; /* By default, we let Kernel accept the packet. */

    /* Get the spin lock so that this list is not modified while we are trying to match the incoming packet. */
    spin_lock_bh(&pAspDeviceCtx->lock);

    /* Now traverse the list current ASP Channel Contexts & see if packet belongs to one of the contexts. */
    list_for_each_entry(pAspChannelCtx, &pAspDeviceCtx->chCtxListHead, chCtxListNode)
    {
        /* Check if socket 5tuple has been set for this channel (App may not yet have called the SET IOCTL method). */
        /* We use atomic variable for this valid flag as it is modified in ioctl APIs w/o holding the spin lock. */
        if (atomic_read(&pAspChannelCtx->socket5TupleInfoValid) == 0)
        {
            /* Info hasn't been set yet for this channel, so ignore it. */
            continue;
        }

        /* This channel has 5-tuple info, so let's first parse the incoming packet if we haven't already parsed this pkt. */
        if (!pktParsed && parseIpv4Packet(pSkb, &socket5TupleInfo))
        {
            /* Somehow we couldn't parse the packet, so let the packet thru back to the kernel. */
            break;
        }
        pktParsed = true;

        /* Now see if this parsed packet info matches with the channels's socket 5tuple info. */
        if (matchSocket5TupleInfo(&socket5TupleInfo, &pAspChannelCtx->socket5TupleInfo) == true)
        {
            /* Yes, the packet matches the socket flow info of this ASP Channel. */
#if 0
            printIpv4SocketTupleInfo("pkt matched: ", &socket5TupleInfo);
#endif
            pAspChannelCtx->pktRcvd++;
            if (socket5TupleInfo.l4Protocol == ASP_ChLayer4Protocol_eTCP)
            {
                if (pAspChannelCtx->action == ASP_PktMatchAction_eDropAllPkts)
                {
                    /* This is the default enum. */
                    /* Any matching packets for TCP protocol are dropped as they are retransmitted stale packets. */
                    /* This is because IP fragmentation doesn't happen for TCP connections. */
                    pktStatus = NF_DROP;
                    pAspChannelCtx->droppedPktCount++;
                    if (printk_ratelimit()) printk(KERN_INFO "Dropping TCP packet, droppedPktCount=%u\n", pAspChannelCtx->droppedPktCount);
                }
                else if (pAspChannelCtx->action == ASP_PktMatchAction_eDropNthPkts)
                {
                    if (atomic_read(&pAspChannelCtx->pktPositionToDrop) == pAspChannelCtx->pktRcvd)
                    {
                        pktStatus = NF_DROP;
                        pAspChannelCtx->droppedPktCount++;
                        if (printk_ratelimit()) printk(KERN_INFO ">>>>>> Dropping TCP packet# %d \n", pAspChannelCtx->pktRcvd);
                        atomic_dec(&pAspChannelCtx->numPktsToDrop);
                    }
                    else
                    {
                        pktStatus = NF_ACCEPT;
                    }
                }
                else if (pAspChannelCtx->action == ASP_PktMatchAction_eDropPktRate)
                {
                    if (pAspChannelCtx->pktRcvd % atomic_read(&pAspChannelCtx->pktPositionToDrop) == 0)
                    {
                        pAspChannelCtx->startPktDrop = true;
                    }
                    if (pAspChannelCtx->startPktDrop)
                    {
                        pktStatus = NF_DROP;
                        pAspChannelCtx->droppedPktCount++;
                        atomic_dec(&pAspChannelCtx->numPktsToDrop);
                        if (printk_ratelimit()) printk(KERN_INFO ">>>>>> Dropping pkt[ctx=%p]: pktRcvd=%u pktPositionToDrop=%u, remaining numPktsToDrop=%u \n",
                                pAspChannelCtx, pAspChannelCtx->pktRcvd, atomic_read(&pAspChannelCtx->pktPositionToDrop), atomic_read(&pAspChannelCtx->numPktsToDrop) );
                        if (atomic_read(&pAspChannelCtx->numPktsToDrop) == 0)
                        {
                            pAspChannelCtx->startPktDrop = false;
                            atomic_set(&pAspChannelCtx->numPktsToDrop, atomic_read(&pAspChannelCtx->origNumPktsToDrop));
                        }
                    }
                    else
                    {
                        pktStatus = NF_ACCEPT;
                    }
                }
                else if (pAspChannelCtx->action == ASP_PktMatchAction_eDropNPkts)
                {
                    if (atomic_read(&pAspChannelCtx->numPktsToDrop) != 0 &&
                        pAspChannelCtx->pktRcvd >= atomic_read(&pAspChannelCtx->pktPositionToDrop)
                        )
                    {
                        pktStatus = NF_DROP;
                        pAspChannelCtx->droppedPktCount++;
                        if (printk_ratelimit()) printk(KERN_INFO ">>>>>> Remaining to drop=%d packets starting at position=%u\n", atomic_read(&pAspChannelCtx->numPktsToDrop), pAspChannelCtx->pktRcvd);
                        atomic_dec(&pAspChannelCtx->numPktsToDrop);
                    }
                    else
                    {
                        pktStatus = NF_ACCEPT;
                    }
                }
                else if (pAspChannelCtx->action == ASP_PktMatchAction_eDropOnePkt)
                {
                    if (!pAspChannelCtx->pktDropped)
                    {
                        pktStatus = NF_DROP;
                        pAspChannelCtx->pktDropped = true;
                        pAspChannelCtx->droppedPktCount++;
                        if (printk_ratelimit()) printk(KERN_INFO ">>>>>> Dropping 1 TCP packet\n");
                    }
                    else
                    {
                        pktStatus = NF_ACCEPT;
                    }
                }
                else if (pAspChannelCtx->action == ASP_PktMatchAction_eNoDrops)
                {
                    pktStatus = NF_ACCEPT;
#if 0
                    if (printk_ratelimit()) printk(KERN_INFO "Not dropping TCP packet, droppedPktCount=%u\n", pAspChannelCtx->droppedPktCount);
#endif
                }
                else
                {
                    pktStatus = NF_DROP;
                    pAspChannelCtx->droppedPktCount++;
                    if (printk_ratelimit()) printk(KERN_INFO "Dropping TCP packet, droppedPktCount=%u\n", pAspChannelCtx->droppedPktCount);
                }
            }
            else
            {
                /* Queue up the skb associated w/ this packet. */
                /* TODO: may need to not queue up packets if queue size exceeds certain threshold */
                /* potentially indicating a user process has fallen way behind. */
                skb_queue_tail(&pAspChannelCtx->rxPktQueueHead, pSkb);

                /* Wake up any app thread waiting on read/poll. */
                wake_up_interruptible(&pAspChannelCtx->rxPktWaitQueueHead);

                /* Tell kernel that we have consumed this skb. It will be freed after app has consumed it during read API. */
                pktStatus = NF_STOLEN;
            }
            break;
        }
        else
        {
            /* parsed packet doesn't match w/ the socket 5tuple info of this channel, so continue w/ the next channel. */
            {
                /* Before that we check if the src IP matches the ASP IP, then we switch the src IP to the local IP. */
                /* This is to work in progress to get the local client working. */
                struct iphdr *pIpHdr;
                pIpHdr = (struct iphdr *) skb_network_header(pSkb);
                if (pIpHdr->saddr == pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
                {
                    if (printk_ratelimit()) printk(KERN_INFO "Switching Src IP from saddr %u.%u.%u.%u daddr %u.%u.%u.%u aspdr %u.%u.%u.%u\n",
                            IPv4(pIpHdr->saddr),
                            IPv4(pIpHdr->daddr),
                            IPv4(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
                            );
                    {
                        /* Recompute the TCP checksum. */
                        tcp_hdr(pSkb)->check = calculateIncrementalChecksum(ntohs(tcp_hdr(pSkb)->check), ntohl(pIpHdr->saddr), ntohl(pAspChannelCtx->socket5TupleInfo.dstIpAddr[0]) );
                    }
                    pIpHdr->saddr = pAspChannelCtx->socket5TupleInfo.dstIpAddr[0];
                    pSkb->csum = CHECKSUM_COMPLETE;
                }
                else {
                    if (0 && printk_ratelimit()) printk(KERN_INFO "NOT Switching Src IP from saddr %u.%u.%u.%u:%d daddr %u.%u.%u.%u:%d aspdr %u.%u.%u.%u\n",
                            IPv4(pIpHdr->saddr), ntohs(socket5TupleInfo.srcPort),
                            IPv4(pIpHdr->daddr), ntohs(socket5TupleInfo.dstPort),
                            IPv4(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
                            );
                }
            }
        }
    }

    /* Release the lock & return the pkt status to the kernel. */
    spin_unlock_bh(&pAspDeviceCtx->lock);
    return (pktStatus);
}

static unsigned int aspIpv4NetfilterHookCallback2(
    const struct nf_hook_ops *ops,
    struct sk_buff *pSkb,
    const struct nf_hook_state *state
    )
{
    int                     pktStatus;
    ASP_Socket5TupleInfo    socket5TupleInfo;
    AspDeviceCtx            *pAspDeviceCtx;
    AspChannelCtx           *pAspChannelCtx = NULL;
    bool                    pktParsed = false;

    if (!pSkb || !ops) return NF_ACCEPT;
    pAspDeviceCtx = ops->priv;

    /*
     * Check if a packet matches w/ the socket tuple info of any of the currently opened ASP Channels.
     */
    pktStatus = NF_ACCEPT; /* By default, we let Kernel accept the packet. */

    /* Get the spin lock so that this list is not modified while we are trying to match the incoming packet. */
    spin_lock_bh(&pAspDeviceCtx->lock);

    /* Now traverse the list current ASP Channel Contexts & see if packet belongs to one of the contexts. */
    list_for_each_entry(pAspChannelCtx, &pAspDeviceCtx->chCtxListHead, chCtxListNode)
    {
        if (pAspChannelCtx == NULL) break;

        /* Check if socket 5tuple has been set for this channel (App may not yet have called the SET IOCTL method). */
        /* We use atomic variable for this valid flag as it is modified in ioctl APIs w/o holding the spin lock. */
        if (atomic_read(&pAspChannelCtx->socket5TupleInfoValid) == 0)
        {
            /* Info hasn't been set yet for this channel, so ignore it. */
            break;
        }

        /* This channel has 5-tuple info, so let's first parse the incoming packet if we haven't already parsed this pkt. */
        if (!pktParsed && parseIpv4Packet(pSkb, &socket5TupleInfo))
        {
            /* Somehow we couldn't parse the packet, so let the packet thru back to the kernel. */
            break;
        }
        pktParsed = true;

        /* Now see if srcIp matches the aspIp. If it does, then Pkt must have come from ASP <TODO: validate this assumption & see if port# needs to be included. > */
        /* Then, replace the srcIp of aspIp w/ host's local IP which is dstIp here. */
        {
            struct iphdr *pIpHdr;
            struct tcphdr *pTcpHdr;

            pTcpHdr = (struct tcphdr *) skb_transport_header(pSkb);
            if (printk_ratelimit()) printk(KERN_INFO "pkt: sport=0x%x dport=0x%x entry srcPort=0x%x dstPort=0x%x",
                pTcpHdr->source, pTcpHdr->dest, pAspChannelCtx->socket5TupleInfo.srcPort, pAspChannelCtx->socket5TupleInfo.dstPort);

            pIpHdr = (struct iphdr *) skb_network_header(pSkb);
            if (pTcpHdr->dest == pAspChannelCtx->socket5TupleInfo.dstPort)
            {
                if (printk_ratelimit()) printk(KERN_INFO "2: Switching Dst IP from saddr %u.%u.%u.%u daddr %u.%u.%u.%u aspdr %u.%u.%u.%u\n",
                        IPv4(pIpHdr->saddr),
                        IPv4(pIpHdr->daddr),
                        IPv4(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
                        );
                pIpHdr->check = calculateIncrementalChecksum(ntohs(pIpHdr->check), ntohl(pIpHdr->daddr), ntohl(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0]) );
                tcp_hdr(pSkb)->check = calculateIncrementalChecksum(ntohs(tcp_hdr(pSkb)->check), ntohl(pIpHdr->saddr), ntohl(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0]) );
                pIpHdr->daddr = pAspChannelCtx->socket5TupleInfo.aspIpAddr[0];
                {
                    /* update the destination route to go to gphy so that packet can then reach the switch which will in turn forward it to ASP port. */
                    struct rtable *rt;
                    struct net_device *net;

                    net = dev_get_by_name(&init_net, "asp");
                    if (net)
                    {
                        rt = ip_route_output(dev_net(net), pIpHdr->daddr, 0,RT_TOS(pIpHdr->tos), 0);
                        if (rt)
                        {
                            skb_dst_drop(pSkb);
                            skb_dst_set(pSkb, &rt->dst);
                            if (printk_ratelimit()) printk(KERN_INFO ">>>> route update for Dst IP %u.%u.%u.%u\n", IPv4(pIpHdr->daddr));
                        }
                        else
                        {
                            if (printk_ratelimit()) printk(KERN_INFO "ip_route_output() failed for Dst IP %u.%u.%u.%u\n", IPv4(pIpHdr->daddr));
                        }
                    }
                    else
                    {
                        if (printk_ratelimit()) printk(KERN_INFO "dev_get_by_name() failed for Dst IP %u.%u.%u.%u\n", IPv4(pIpHdr->daddr));
                    }
                }
            }
            else {
                if (printk_ratelimit()) printk(KERN_INFO "2preroutting: NOT Switching Dst IP from saddr %u.%u.%u.%u daddr %u.%u.%u.%u aspdr %u.%u.%u.%u\n",
                        IPv4(pIpHdr->saddr),
                        IPv4(pIpHdr->daddr),
                        IPv4(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
                        );
            }
        }
    }

    /* Release the lock & return the pkt status to the kernel. */
    spin_unlock_bh(&pAspDeviceCtx->lock);
    return (pktStatus);
}

#if 0
static unsigned int aspIpv4NetfilterHookCallback1(
    const struct nf_hook_ops *ops,
    struct sk_buff *pSkb,
    const struct nf_hook_state *state
    )
{
    int                     pktStatus;
    ASP_Socket5TupleInfo    socket5TupleInfo;
    AspDeviceCtx            *pAspDeviceCtx;
    AspChannelCtx           *pAspChannelCtx = NULL;
    bool                    pktParsed = false;

    if (!pSkb || !ops) return NF_ACCEPT;
    pAspDeviceCtx = ops->priv;

    /*
     * Check if a packet matches w/ the socket tuple info of any of the currently opened ASP Channels.
     */
    pktStatus = NF_ACCEPT; /* By default, we let Kernel accept the packet. */

    /* Get the spin lock so that this list is not modified while we are trying to match the incoming packet. */
    spin_lock_bh(&pAspDeviceCtx->lock);

    /* Now traverse the list current ASP Channel Contexts & see if packet belongs to one of the contexts. */
    list_for_each_entry(pAspChannelCtx, &pAspDeviceCtx->chCtxListHead, chCtxListNode)
    {
        if (pAspChannelCtx == NULL) break;

        /* Check if socket 5tuple has been set for this channel (App may not yet have called the SET IOCTL method). */
        /* We use atomic variable for this valid flag as it is modified in ioctl APIs w/o holding the spin lock. */
        if (atomic_read(&pAspChannelCtx->socket5TupleInfoValid) == 0)
        {
            /* Info hasn't been set yet for this channel, so ignore it. */
            break;
        }

        /* This channel has 5-tuple info, so let's first parse the incoming packet if we haven't already parsed this pkt. */
        if (!pktParsed && parseIpv4Packet(pSkb, &socket5TupleInfo))
        {
            /* Somehow we couldn't parse the packet, so let the packet thru back to the kernel. */
            break;
        }
        pktParsed = true;

        /* Now see if srcIp matches the aspIp. If it does, then Pkt must have come from ASP <TODO: validate this assumption & see if port# needs to be included. > */
        /* Then, replace the srcIp of aspIp w/ host's local IP which is dstIp here. */
        {
            struct iphdr *pIpHdr;
            pIpHdr = (struct iphdr *) skb_network_header(pSkb);
#if 0
            if (pIpHdr->saddr == pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
#endif
            if (pIpHdr->saddr == pIpHdr->daddr)
            {
                if (printk_ratelimit()) printk(KERN_INFO "Switching Src IP from saddr %u.%u.%u.%u daddr %u.%u.%u.%u to aspdr %u.%u.%u.%u\n",
                        IPv4(pIpHdr->saddr),
                        IPv4(pIpHdr->daddr),
                        IPv4(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
                        );
                pIpHdr->saddr = pAspChannelCtx->socket5TupleInfo.dstIpAddr[0];
                pIpHdr->saddr = pAspChannelCtx->socket5TupleInfo.aspIpAddr[0];
            }
            else {
                if (1 && printk_ratelimit()) printk(KERN_INFO "NOT Switching Src IP from saddr %u.%u.%u.%u:%d daddr %u.%u.%u.%u:%d aspdr %u.%u.%u.%u\n",
                        IPv4(pIpHdr->saddr), ntohs(socket5TupleInfo.srcPort),
                        IPv4(pIpHdr->daddr), ntohs(socket5TupleInfo.dstPort),
                        IPv4(pAspChannelCtx->socket5TupleInfo.aspIpAddr[0])
                        );
            }
        }
    }

    /* Release the lock & return the pkt status to the kernel. */
    spin_unlock_bh(&pAspDeviceCtx->lock);
    return (pktStatus);
}
#endif

static ssize_t copyFromSkbToUserBuffer(
    AspChannelCtx *pAspChannelCtx,
    struct sk_buff  *pSkb,
    char            *pBuffer
    )
{
    ssize_t frameLength;

    /* TODO: include check for various offsets. */
    frameLength = pSkb->len /* includes IP Hdr onwards */ + (skb_network_header(pSkb) - skb_mac_header(pSkb)); /* + size of MAC header. */
    if (copy_to_user((void __user *)(pBuffer), skb_mac_header(pSkb), frameLength) != 0)
    {
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p copy_to_user failed for read!\n",
                ASP_MSG_PRE_ARG, pAspChannelCtx);
        return -EFAULT;
    }
    return (frameLength);
}

/*
 * Read API to copy 1 Pkt/frame worth of data.
 *
 * Returns # of bytes copies if success, -errno otherwise. So far there is no reason to return 0 (EOF).
 */
static ssize_t ASP_Channel_Read(
    struct file *pFile,
    char        *pBuffer,
    size_t      bufferSize,
    loff_t *    offset
    )
{
    int err = -1;
    AspDeviceCtx *pAspDeviceCtx = NULL;
    AspChannelCtx *pAspChannelCtx = NULL;
    struct sk_buff *pSkb;

    /* Make sure only privileged or super-user process can open a device context. */
    if ((err = authenticateProcessContext())) return (err);

    /* Validate input arguments. */
    {
        if (!access_ok(VERIFY_WRITE, (void __user *)pBuffer, bufferSize))
        {
            pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p access_ok failed on pBuffer=%p bufferSize=%u\n", ASP_MSG_PRE_ARG, pAspChannelCtx, pBuffer, (unsigned int)bufferSize);
            return -EFAULT;
        }
#define ASP_ETH_FRAME_SIZE 1518
        if (bufferSize < ASP_ETH_FRAME_SIZE)
        {
            /* This is an aberration from the regular read where even 1 byte of data can be returned. */
            /* However, in our model, we want to only return full frame to the app. So we ensure that user has provided 1 frame worth of buffer. */
            pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p bufferSize=%u < ASP_ETH_FRAME_SIZE (%d), app can only read 1 full frame!\n", ASP_MSG_PRE_ARG, pAspChannelCtx, (unsigned int)bufferSize, ASP_ETH_FRAME_SIZE);
            return -EFAULT;
        }
    }

    pAspDeviceCtx = g_pAspDeviceCtx;
    pAspChannelCtx = pFile->private_data;
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p read: pBuffer=%p bufferSize=%u\n", ASP_MSG_PRE_ARG, pAspChannelCtx, pBuffer, (unsigned int)bufferSize);

    /* Note: we only need to get mutex here & not spin lock. Even though mutex allows us to possibly sleep if no data is available during blocking read, */
    /* there is no reason to hold it while waiting for new data to show up. */
    /* Also, mutex allows this read to proceed before another thread tries to either issue another read or purge the skb queue via ioctl call. */
    if (mutex_lock_interruptible(&pAspChannelCtx->mutex))
    {
        return -ERESTARTSYS; /* we got interrupted due to a signal while waiting, so ask upper layer to restart this read call. */
    }

    /* See if we have any data available. */
    while (skb_queue_len(&pAspChannelCtx->rxPktQueueHead) == 0)
    {
        /* No filtered/matched packets available yet. Based on the blocking/non-blocking mode, we take the subsequent action. */

        /* release the mutex first as we will either return or sleep. We will re-acquire it below after we wakeup. */
        mutex_unlock(&pAspChannelCtx->mutex);

        /* if app is using non-blocking mode, then return EAGAIN. */
        if (pFile->f_flags & O_NONBLOCK)
        {
            pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p: no data available & non-blocking mode, returning -EAGAIN!\n", ASP_MSG_PRE_ARG, pAspChannelCtx);
            return -EAGAIN;
        }

        /* Otherwise, goto sleep by waiting on waitq. Netfilter hook callback will wake us up when the next frame arrives for this 5-tuple flow. */
        pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p: no data available, so going to sleep...\n", ASP_MSG_PRE_ARG, pAspChannelCtx);
        if (wait_event_interruptible(pAspChannelCtx->rxPktWaitQueueHead, (skb_queue_len(&pAspChannelCtx->rxPktQueueHead) > 0)) != 0)
        {
            return -ERESTARTSYS; /* we got interrupted due to a signal while waiting, so ask upper layer to restart this read call. */
        }

        /* woke up from sleeping, re-acquire the mutex and reloop to check if we still have data to read (& not read by some other app thread). */
        if (mutex_lock_interruptible(&pAspChannelCtx->mutex))
        {
            return -ERESTARTSYS; /* we got interrupted due to a signal while waiting, so ask upper layer to restart this read call. */
        }
    }
    mutex_unlock(&pAspChannelCtx->mutex);

    /* If we are here, there has to be data available. */
    /* Note: skb_queue_tail & skb_dequeue use skb's internal spin-lock to make these calls atomic. */
    pSkb = skb_dequeue(&pAspChannelCtx->rxPktQueueHead);
    if (pSkb)
    {
        err = copyFromSkbToUserBuffer(pAspChannelCtx, pSkb, pBuffer);
        if (err > 0 ) kfree_skb(pSkb);
    }
    else
    {
        /* Nothing is queued at this time, return -1 if non-blocking or block otherwise! */
        err = -1;
        pr_err(ASP_MSG_PRE_FMT "pAspChannelCtx=%p: we expected to have atleast 1 frame queued up, but we found none!\n", ASP_MSG_PRE_ARG, pAspChannelCtx);
    }

    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p returning=%d \n", ASP_MSG_PRE_ARG, pAspChannelCtx, err);
    return (err);
}

/*
 * Poll API to check if ASP Channel Context has any Pkts available to read.
 *
 * Returns # of bytes copies if success, -errno otherwise. So far there is no reason to return 0 (EOF).
 */
static unsigned int ASP_Channel_Poll(
    struct file *pFile,
    poll_table  *pPollTable
    )
{
    unsigned int    eventMask = 0;
    AspDeviceCtx    *pAspDeviceCtx = NULL;
    AspChannelCtx   *pAspChannelCtx = NULL;

    pAspDeviceCtx = g_pAspDeviceCtx;
    pAspChannelCtx = pFile->private_data;
    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p \n", ASP_MSG_PRE_ARG, pAspChannelCtx);

    /* Get the mutex to block other APIs. */
    if (mutex_lock_interruptible(&pAspChannelCtx->mutex))
    {
        return -ERESTARTSYS; /* we got interrupted due to a signal while waiting, so ask upper layer to restart this poll call. */
    }

    /* Tell poll logic about the waitQueueHead to wait on! */
    if (pPollTable)
    {
        /* Note: pPollTable can be NULL if Kernel has already found a fd that has some event available and thus it doesn't need to wait on us. */
        /* Instead, it just wants to know our event stats. */
        poll_wait(pFile, &pAspChannelCtx->rxPktWaitQueueHead, pPollTable);
    }

    /* Now check if we have any pkt ready to be read. */
    if (skb_queue_len(&pAspChannelCtx->rxPktQueueHead) != 0)
    {
        /* we atleast have 1 pkt ready for reading, so set the read event mask. */
        eventMask = POLLIN | POLLRDNORM;
    }
    mutex_unlock(&pAspChannelCtx->mutex);

    pr_info(ASP_MSG_PRE_FMT "pAspChannelCtx=%p eventMask=0x%x \n", ASP_MSG_PRE_ARG, pAspChannelCtx, eventMask);
    return (eventMask);
}

module_init(ASP_Drv_Init);
module_exit(ASP_Drv_Uninit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("NetFilter based driver to capture IP pkt pertaining to ASP offloaded connections");
MODULE_VERSION("0.1");
