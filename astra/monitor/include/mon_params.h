/******************************************************************************
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
 *****************************************************************************/

#ifndef _MON_PARAMS_H_
#define _MON_PARAMS_H_

#include <stddef.h>
#include <stdint.h>

/*
 * Parameter types
 *
 * Note: These parameter header types are compatible with that used
 * in BL31 to allow legacy Bolt be used with the new monitor code.
 */
#define MONITOR_PARAMS          0x03
#define BRCMSTB_PARAMS          0x04

/*
 * Current monitor parameters version
 *
 * Note: Version 1 of monitor parameters represents BL31 parameters.
 * BL31 does NOT support brcmstb specific parameters.
 */
#define MONITOR_PARAMS_VERSION  0x03

/*
 * Image flags
 */
#define IMG_NSEC_MASK           (0x01 << 0)
#define IMG_NSEC                IMG_NSEC_MASK
#define IMG_SEC                 0x00

#define IMG_AARCH32_MASK        (0x01 << 1)
#define IMG_AARCH32             IMG_AARCH32_MASK
#define IMG_AARCH64             0x00

#define IMG_NSEC_EL1_MASK       (0x01 << 1)
#define IMG_NSEC_EL1            IMG_NSEC_EL1_MASK
#define IMG_NSEC_EL2            0x00

/*
 * Header for parameters structures
 *
 * Note: This parameter header struct is compatible with that used
 * in BL31 to allow legacy Bolt be used with the new monitor code.
 */
typedef struct param_hdr {
    uint8_t  type;
    uint8_t  version;
    uint16_t reserved;
} param_hdr_t;

/*
 * Image information
 */
typedef struct img_info {
    uint64_t base;
    uint32_t size;
    uint32_t flags;
} img_info_t;

/*
 * Mailbox information
 */
typedef struct mbox_info {
    uint64_t addr;
    uint32_t flags;
    uint16_t size;
    uint8_t  sgi;
    uint8_t  reserved;
} mbox_info_t;

/*
 * Monitor parameters
 */
typedef struct mon_params {
    param_hdr_t hdr;

    /* Number of clusters and cpus */
    uint8_t     num_clusters;
    uint8_t     num_cpus;

    /* Secure and normal world cpus masks */
    uint8_t     tz_cpus_mask;
    uint8_t     nw_cpus_mask;

    /* Monitor image info (self) */
    img_info_t  mon_img_info;

    /* Secure world image info and parameters (device tree) */
    img_info_t  tz_img_info;
    uint64_t    tz_entry_point;
    uint64_t    tz_dev_tree;

    /* Normal world image info and parameters (device tree) */
    img_info_t  nw_img_info;
    uint64_t    nw_entry_point;
    uint64_t    nw_dev_tree;

    /* Secure and normal world mailboxes info */
    mbox_info_t tz_mbox_info;
    mbox_info_t nw_mbox_info;

} mon_params_t;

#endif /* _MON_PARAMS_H_ */
