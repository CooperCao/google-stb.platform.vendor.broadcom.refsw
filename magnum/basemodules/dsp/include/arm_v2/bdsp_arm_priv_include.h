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

#ifndef BDSP_ARM_PRIV_INCLUDE_H_
#define BDSP_ARM_PRIV_INCLUDE_H_

#include "bchp.h"
#include "bint.h"
#include "bbox.h"
#include "breg_mem.h"
#include "bmma.h"
#include "btmr.h"
#include "bdbg.h"
#include "bimg.h"
#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#include "bchp_sun_top_ctrl.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_aud_misc.h"
#include "btee.h"
#include <fcntl.h>

/* BDSP top level includes */
#include "bdsp.h"
#include "bdsp_types.h"

/* BDSP common include*/
#include "bdsp_common_priv_include.h"

/* BDSP Arm includes */
#include "bdsp_arm.h"
#include "bdsp_arm_fw_algo.h"
#include "bdsp_arm_mm_priv.h"
#include "bdsp_common_cmdresp_priv.h"
#include "bdsp_arm_img.h"
#include "bdsp_arm_img_sizes.h"
#include "bdsp_arm_fwdownload_priv.h"
#include "bdsp_arm_fwinterface_priv.h"
#include "bdsp_arm_version.h"
#include "bdsp_arm_priv.h"
#include "bdsp_arm_command_priv.h"
#include "bdsp_arm_io_priv.h"
#include "bdsp_arm_int_priv.h"
#include "bdsp_arm_cit_priv.h"
#include "bdsp_arm_cit_log.h"
#include "bdsp_arm_capture_priv.h"
#endif /*BDSP_ARM_PRIV_INCLUDE_H_*/
