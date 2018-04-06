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

#ifndef _BRCMSTB_SVC_H_
#define _BRCMSTB_SVC_H_

#include <smcc.h>
#include "service.h"

#define BRCMSTB_SVC_REVISION_MAJOR      0
#define BRCMSTB_SVC_REVISION_MINOR      1

/* Brcmstb service OEN */
#define BRCMSTB_SVC_OEN                 OEN_OEM_START

/* Brcmstb service FN for general queries */
#define BRCMSTB_SVC_CALL_COUNT          0xFF00
#define BRCMSTB_SVC_CALL_UID            0xFF01
#define BRCMSTB_SVC_REVISION            0xFF02

/* Brcmstb service FN consists of module bit[15:8] and func bit[7:0] */
#define BRCMSTB_SVC_MOD_SHIFT           8
#define BRCMSTB_SVC_MOD_MASK            0xFF

#define BRCMSTB_SVC_FUNC_SHIFT          0
#define BRCMSTB_SVC_FUNC_MASK           0xFF

#define BRCMSTB_SVC_FN(mod, func)       (((mod)  << BRCMSTB_SVC_MOD_SHIFT) | \
                                         ((func) << BRCMSTB_SVC_FUNC_SHIFT))

#define BRCMSTB_SVC_FID(mod, func, cc)  (((cc) << FUNCID_CC_SHIFT) | \
                                         (BRCMSTB_SVC_OEN << FUNCID_OEN_SHIFT) | \
                                         (BRCMSTB_SVC_FN(mod, func)))

#define BRCMSTB_SVC_MOD(fx)             (((fx) >> BRCMSTB_SVC_MOD_SHIFT)  & BRCMSTB_SVC_MOD_MASK)
#define BRCMSTB_SVC_FUNC(fx)            (((fx) << BRCMSTB_SVC_FUNC_SHIFT) & BRCMSTB_SVC_FUNC_MASK)

/* Brcmstb service modules */
/* Note: general queries with (mod == 0xFF) are handled separately */
#define BRCMSTB_SVC_MOD_PSCI            0x00
#define BRCMSTB_SVC_MOD_DVFS            0x01
#define BRCMSTB_SVC_MOD_ASTRA           0x02
#define BRCMSTB_SVC_MOD_LINUX           0x03
#define BRCMSTB_SVC_MOD_SCMI            0x04
#define BRCMSTB_SVC_MOD_MAX             0x05

#endif /* _BRCMSTB_SVC_H_ */
