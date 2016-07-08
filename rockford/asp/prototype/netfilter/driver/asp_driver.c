/*
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
 */
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <uapi/linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include "asp_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("IP reassembly,rx packet filter");

#undef PDEBUG
#undef PWARN
#undef PERR
#undef PINFO

#define DEBUG
#ifdef DEBUG
    #define PERR(fmt, args...) printk( KERN_ERR "ASPDRV: " fmt, ## args)
    #define PWARN(fmt, args...) printk( KERN_WARNING "ASPDRV: " fmt, ## args)
    #define PINFO(fmt, args...) printk( KERN_INFO "ASPDRV: " fmt, ## args)
    #define PDEBUG(fmt, args...) printk( KERN_DEBUG "ASPDRV: " fmt, ## args)
#else
    #define PERR(fmt, args...)
    #define PWARN(fmt, args...)
    #define PINFO(fmt, args...)
    #define PDEBUG(fmt, args...)
#endif

/*driver stuff*/
static dev_t asp_dev_devno;
static struct cdev *asp_dev_cdev=NULL;
static struct class *asp_dev_class=NULL;
static struct device *asp_dev_device=NULL;
static DEFINE_MUTEX(asp_dev_mutex);

/*net filter*/
static struct nf_hook_ops asp_dev_nfho;
static __u64 asp_dev_pkt_dropped=0;   /*number of packets dropped */
static __u64 asp_dev_pkt_toasp=0;     /*number of packets forwarded to asp */
static __u64 asp_dev_pkt_inspected=0; /*number of packets passed filter match  */



unsigned int asp_dev_nf_hook_func(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
    struct iphdr *ip = ip_hdr(skb);
    struct udphdr *udp=NULL;
    /* struct udphdr *tcp=NULL; */
    volatile unsigned i,j;

    if (skb->protocol == htons(ETH_P_IP) &&
        (ip->protocol == IPPROTO_TCP || ip->protocol == IPPROTO_UDP)
    {
        if(0xc0a80102 == ntohl(ip->saddr))
        {

            #if 0
            for(i=0;i<1000;i++)
            {
                /*for(j=0;j<1000;j++)
                {
                    asp_dev_pkt_counter++;
                }*/
                 asp_dev_pkt_counter++;
            }
            #endif
            /*udp=(struct udphdr *)(skb->data+ ip->ihl*4);*/
            /*udp = (struct udphdr *)((__u32 *)ip+ ip->ihl);*/
            /*PINFO("len=%d,src=%x,dst=%x,src_port==%x,dst_port=%x\n",
                   skb->len,ntohl(ip->saddr),ntohl(ip->daddr),
                   ntohs(udp->source),ntohs(udp->dest));*/
            /*print_hex_dump(KERN_NOTICE, "", DUMP_PREFIX_ADDRESS,
                  16, 1, skb->data, skb->len, 0);*/

        }
        /*kfree_skb(skb);*/
        return NF_ACCEPT;
    } else
        return NF_ACCEPT;

}


static int asp_dev_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int asp_dev_close(struct inode *inode, struct file *filp)
{
    return 0;
}


static int asp_dev_ioctl_check_cmd(unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    if (_IOC_TYPE(cmd) != ASP_DEV_MAGIC)
        return -ENOTTY;

    if (_IOC_NR(cmd) > ASP_DEV_IOC_MAXNR)
        return -ENOTTY;

    if (_IOC_DIR(cmd) & _IOC_READ)
        ret = !access_ok(VERIFY_WRITE, (void __user *)arg,
                         _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        ret = !access_ok(VERIFY_READ, (void __user *)arg,
                         _IOC_SIZE(cmd));

    if (ret)
        return -EFAULT;

    return 0;
}

static long asp_dev_ioctl(struct file *filp, unsigned int cmd,
                          unsigned long arg)
{
    int ret = 0;
    struct asp_ioc_params p;

    ret = asp_dev_ioctl_check_cmd(cmd, arg);
    if (ret)
        return ret;

    if (cmd < ASP_DEV_IOC_RESET)
    {
        if (copy_from_user(&p, (void __user *)(arg), sizeof(p)))
            return -EFAULT;
    }

    switch (cmd)
    {
    case ASP_DEV_IOC_ADDFILTER:

        PINFO("ASP_DEV_IOC_ADDFILTER:asp_ch=%d,proto%d,src ip=%llx,dsp ip =%llx,src port %d,dsp port =%d,actions =%d",
              p.asp_ch,p.protocol,p.src_ip,p.dst_ip,p.src_port,p.dst_port,p.action);
        ret = -EINVAL;
        ret = 0;
        break;


    default:
        PINFO("%d, not implemented\n",cmd);
        return -ENOTTY;

    }
    if (cmd < ASP_DEV_IOC_RESET)
    {
        if (copy_to_user((void __user *)(arg), &p, sizeof(p)))
            return -EFAULT;
    }
    return ret;
}

static const struct file_operations asp_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = asp_dev_open,
    .release        = asp_dev_close,
    .unlocked_ioctl = asp_dev_ioctl
};

int init_module(void)
{
    int ret = 0;
    int minor =0;

    ret = alloc_chrdev_region(&asp_dev_devno, 0, 1,ASP_DEV_NAME);
    if (ret)
    {
        PERR("couldn't alloc major devno\n");
        return ret;
    }
    PINFO("maj=%d min=%d\n", MAJOR(asp_dev_devno),MINOR(asp_dev_devno));

    asp_dev_cdev = cdev_alloc();
    if (asp_dev_cdev == NULL)
    {
        ret=-1;
        goto asp_dev_unreg_chrdev;
    }
    asp_dev_cdev->ops = &asp_dev_fops;
    asp_dev_cdev->owner = THIS_MODULE;
    ret = cdev_add(asp_dev_cdev, asp_dev_devno + minor, 1);
    if (ret)
    {
        PERR("cdev_add() failed (%d)\n", ret);
        goto asp_dev_cdev_del;
    }

    asp_dev_class = class_create(THIS_MODULE, ASP_CLASS_NAME);
    if (asp_dev_class == NULL)
    {
        ret = -1;
        goto asp_dev_cdev_del;
    }

    asp_dev_device = device_create(asp_dev_class, NULL, asp_dev_devno,NULL,ASP_DEV_NAME);
    if (IS_ERR(asp_dev_device))
    {
        PERR( "error creating device (%d)\n", ret);
        ret = PTR_ERR(asp_dev_device);
        goto asp_dev_del_class;
    }
    PINFO("Initialized Broadcom ASP driver\n");

    /*driver is now ready setup the netfilter */
    asp_dev_nfho.hook = asp_dev_nf_hook_func;
    asp_dev_nfho.hooknum = NF_INET_LOCAL_IN;   /* reassembled packets ready for ruting */
    asp_dev_nfho.pf = PF_INET;                 /* (PF_INET6) Protocol family packets */
    asp_dev_nfho.priority = NF_IP_PRI_FIRST;   /* set to highest priority over all other hook functions */
    nf_register_hook(&asp_dev_nfho);
    return ret;

asp_dev_del_class:
    class_destroy(asp_dev_class);
asp_dev_cdev_del:
    cdev_del(asp_dev_cdev);
asp_dev_unreg_chrdev:
    unregister_chrdev_region(asp_dev_devno,1);
    return ret;
}

void cleanup_module(void)
{
    nf_unregister_hook(&asp_dev_nfho);
    device_destroy(asp_dev_class,asp_dev_devno);
    class_destroy(asp_dev_class);
    cdev_del(asp_dev_cdev);
    unregister_chrdev_region(asp_dev_devno,1);
    PINFO("Cleaned up Broadcom ASP driver\n");
}
