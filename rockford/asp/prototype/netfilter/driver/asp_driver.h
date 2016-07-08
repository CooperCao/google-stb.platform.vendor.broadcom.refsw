/******************************************************************************
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
 *****************************************************************************/

#ifndef __BCM_ASP_DRIVER_H__
#define __BCM_ASP_DRIVER_H__

#include <asm/types.h>

#define ASP_DEV_NAME        "brcm_asp"
#define ASP_CLASS_NAME      "brcm_asp"

#define ASP_DEV_MAGIC       'c'
#define ASP_DEV_IOC_ADDFILTER   _IOWR(ASP_DEV_MAGIC, 1, int)
#define ASP_DEV_IOC_DELFILTER   _IOWR(ASP_DEV_MAGIC, 2, int)
#define ASP_DEV_IOC_GETFILTER   _IOWR(ASP_DEV_MAGIC, 3, int)
#define ASP_DEV_IOC_RESET       _IOWR(ASP_DEV_MAGIC, 4, int)
#define ASP_DEV_IOC_MAXNR   4

struct asp_ioc_rule{
        __u32 version;           /*  4=IpV4,6=IPV6 */
        __u32 protocol;          /*  0x06=tcp,0x11 udp */
        __u32 src_ip[4];         /* [in/out] source ip address (BE) */
        __u32 dst_ip[4];         /* [in/out] destination ip address (BE) */
        __u16 src_port;          /* [in/out] source ip address (BE)*/
        __u16 dst_port;          /* [in/out] destination ip address (BE) */
        __u32 action;            /* [in/out] 0 forward to n/w stack,1 drop,2 forward to asp,
                                    3 forward to application   */
};

struct asp_ioc_params {
    __u32 asp_ch;                   /* [in] asp channel index, 0-31 */
    struct asp_ioc_rule asp_rule;
};

#endif /* __BCM_ASP_DRIVER_H__ */
