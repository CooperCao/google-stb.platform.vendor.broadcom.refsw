/****************************************************************************
*  
*  Copyright (c) 2010-2011 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or
*  its licensors, and may only be used, duplicated, modified or distributed
*  pursuant to the terms and conditions of a separate, written license
*  agreement executed between you and Broadcom (an "Authorized License").
*  Except as set forth in an Authorized License, Broadcom grants no license
*  (express or implied), right to use, or waiver of any kind with respect to
*  the Software, and Broadcom expressly reserves all rights in and to the
*  Software and all intellectual property rights therein.  IF YOU HAVE NO
*  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
*  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
*  SOFTWARE.  
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************
*
*  Filename: bcmvenet.h 
*
****************************************************************************
* Description: This is the network interface driver for the virtual Ethernet 
*              interface between 7125 BNM and HOST.
*               
* Updates    : 08-28-2009  wfeng.  Created.
*              12-20-2010  jcyr. Added support for second ethernet interface
*                          to ECM Unimac.
*              01-04-2011  jcyr. Added support for host application initiated
*                          ECM boot.
* 
****************************************************************************/

#pragma once

// Parameter list for the SIOC_HOSTBOOT IOCTL call

struct venet_ioctl_parm {
    unsigned int physicalMemorySize;    // Size of the platform physical memory in bytes
    unsigned int bootSegSize;           // Boot segment size in bytes
    unsigned int docsisSegSize;         // DOCSIS segment size in bytes
    void *bootCodePtr;                  // User space address of the boot code image
    unsigned int bootCodeSize;          // Boot code image size in bytes
    void *docsisCodePtr;                // User space address of the DOCSIS code image
    unsigned int docsisCodeSize;        // DOCSIS code image size in bytes
    void *dynNonVolPtr;                 // User space address of the dynamic nonvol data
    unsigned int dynNonVolSize;         // Dynamic nonvol data size in bytes
    void *permNonVolPtr;                // User space address of the permanent nonvol data
    unsigned int permNonVolSize;        // Permanent nonvol data size in bytes
};

// SIOC_HOSTBOOT IOCTL number
#define SIOC_HOSTBOOT (SIOCDEVPRIVATE + 10)
// Return 0 if successful, -EINVAL on failure

// SIOC_GET_AND_RESET_DMA_READY_INDICATION number
#define SIOC_GET_AND_RESET_DMA_READY_INDICATION (SIOCDEVPRIVATE + 11)
// Returns 0 if DMA_READY has not occurred since last IOCTL call
//         1 if DAM_READY has occurred since last IOCTL call

#define NONVOL_MAX_SIZE (32 * 1024)
#define DOCSIS_MAX_SIZE (2 * 1024 * 1024)
