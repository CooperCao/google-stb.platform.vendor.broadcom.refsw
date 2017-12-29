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

#ifndef _GICV2_PRIV_H_
#define _GICV2_PRIV_H_

/* Word offsets of GIC distributor registers */
#define GICD_CTLR               (0x0000 /4)
#define GICD_TYPER              (0x0004 /4)
#define GICD_IIDR               (0x0008 /4)
#define GICD_IGROUPRn           (0x0080 /4)
#define GICD_ISENABLERn         (0x0100 /4)
#define GICD_ICENABLERn         (0x0180 /4)
#define GICD_ISPENDRn           (0x0200 /4)
#define GICD_ICPENDRn           (0x0280 /4)
#define GICD_ISACTIVERn         (0x0300 /4)
#define GICD_ICACTIVERn         (0x0380 /4)
#define GICD_IPRIORITYRn        (0x0400 /4)
#define GICD_ITARGETSRn         (0x0800 /4)
#define GICD_ICFGRn             (0x0C00 /4)
#define GICD_NSACRn             (0x0E00 /4)
#define GICD_SGIR               (0x0F00 /4)
#define GICD_CPENDSGIRn         (0x0F10 /4)
#define GICD_SPENDSGIRn         (0x0F20 /4)
/* ID registers with ARM implementation */
#define GICD_ICPIDR4            (0x0FD0 /4)
#define GICD_ICPIDR5            (0x0FD4 /4)
#define GICD_ICPIDR6            (0x0FD8 /4)
#define GICD_ICPIDR7            (0x0FDC /4)
#define GICD_ICPIDR0            (0x0FE0 /4)
#define GICD_ICPIDR1            (0x0FE4 /4)
#define GICD_ICPIDR2            (0x0FE8 /4)
#define GICD_ICPIDR3            (0x0FEC /4)
#define GICD_ICCIDR0            (0x0FF0 /4)
#define GICD_ICCIDR1            (0x0FF4 /4)
#define GICD_ICCIDR2            (0x0FF8 /4)
#define GICD_ICCIDR3            (0x0FFC /4)

/* Word offsets of GIC CPU interface registers */
#define GICC_CTLR               (0x0000 /4)
#define GICC_PMR                (0x0004 /4)
#define GICC_BPR                (0x0008 /4)
#define GICC_IAR                (0x000C /4)
#define GICC_EOIR               (0x0010 /4)
#define GICC_RPR                (0x0014 /4)
#define GICC_HPPIR              (0x0018 /4)
#define GICC_ABPR               (0x001C /4)
#define GICC_AIAR               (0x0020 /4)
#define GICC_AEOIR              (0x0024 /4)
#define GICC_AHPPIR             (0x0028 /4)
#define GICC_APRn               (0x00D0 /4)
#define GICC_NSAPRn             (0x00E0 /4)
#define GICC_IIDR               (0x00FC /4)
#define GICC_DIR                (0x1000 /4)

/* Word offsets of GIC virtual interface control registers */
#define GICH_HCR                (0x0000 /4)
#define GICH_VTR                (0x0004 /4)
#define GICH_VMCR               (0x0008 /4)
#define GICH_MISR               (0x0010 /4)
#define GICH_EISR0              (0x0020 /4)
#define GICH_EISR1              (0x0024 /4)
#define GICH_ELSR0              (0x0030 /4)
#define GICH_ELSR1              (0x0034 /4)
#define GICH_APR                (0x00F0 /4)
#define GICH_LRn                (0x0100 /4)

#endif /* _GICV2_PRIV_H_ */
