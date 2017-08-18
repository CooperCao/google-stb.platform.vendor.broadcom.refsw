/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

/**
 * @file
 * @ingroup libsyschip
 * @brief Enumeration of the services that can be transported in a Target Buffer data stream.
 *
 * Note: this file gets included both by Firepath and Host code, so avoid dependencies
 *       to SDK headers as much as possible and macro guard DSP-specific definitions.
 */

#ifndef _TBUF_SERVICES_H_
#define _TBUF_SERVICES_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "fp_sdk_config.h"



#ifdef __cplusplus
extern "C" {
#endif


/**
 * IDs of the services that can be transported in a Target Buffer data stream.
 * One of these values gets embedded in the \ref TB_header#prologue field of a #TB_header.
 */
typedef enum
{
    TB_SERVICE_RAW              = 0,
    TB_SERVICE_TARGET_PRINT     = 1,
    TB_SERVICE_STAT_PROF        = 2,
    TB_SERVICE_INSTRUMENTATION  = 3,
    TB_SERVICE_CORE_DUMP        = 4,
    TB_SERVICE_MAX              = TB_SERVICE_CORE_DUMP,
    TB_SERVICE_COUNT            = TB_SERVICE_MAX + 1
} TB_service_id;


/**
 * Macro to obtain a #TB_service_flag from the corresponding #TB_service_id.
 */
#define TB_SERVICE_FLAG_FROM_ID(service_id)     (1 << (service_id))


/**
 * Services IDs in flag form to allow or'ing.
 */
typedef enum
{
    TB_SERVICE_RAW_FLAG             = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_RAW),
    TB_SERVICE_TARGET_PRINT_FLAG    = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_TARGET_PRINT),
    TB_SERVICE_STAT_PROF_FLAG       = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_STAT_PROF),
    TB_SERVICE_INSTRUMENTATION_FLAG = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_INSTRUMENTATION),
    TB_SERVICE_CORE_DUMP_FLAG       = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_CORE_DUMP),
    TB_SERVICE_ALL                  = TB_SERVICE_FLAG_FROM_ID(TB_SERVICE_MAX + 1) - 1
} TB_service_flag;


#ifdef __cplusplus
}
#endif



#endif /* _TBUF_SERVICES_H_ */
