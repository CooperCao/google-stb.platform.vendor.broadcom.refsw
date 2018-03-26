/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef BMUXLIB_ASP_SOURCE_H_
#define BMUXLIB_ASP_SOURCE_H_

#include "bmuxlib_asp_types.h"

typedef enum BMUXlib_ASP_Source_Type_e
{
    BMUXLIB_ASP_SOURCE_TYPE_INDEXED, /* A source containing both ITB/CDB */
    BMUXLIB_ASP_SOURCE_TYPE_SYSTEMDATA, /* An source generated internally by the mux manager (e.g. PCR generation) */
    BMUXLIB_ASP_SOURCE_TYPE_USERDATA, /* A source containing only CDB data of TS packets (e.g. teletext/subtitles) */

    /* Add more enums ABOVE this line */
    BMUXLIB_ASP_SOURCE_TYPE_UNKNOWN,
    BMUXLIB_ASP_SOURCE_TYPE_MAX
} BMUXlib_ASP_Source_Type_e;

typedef enum BMUXlib_ASP_Source_AV_Type_e
{
    BMUXLIB_ASP_SOURCE_AV_TYPE_VICE = 0, /* VICE 1.x/2.x+ */
    BMUXLIB_ASP_SOURCE_AV_TYPE_RAAGA, /* Audio ITB and Audio CDB Data */

    /* Add more enums ABOVE this line */
    BMUXLIB_ASP_SOURCE_AV_TYPE_UNKNOWN,
    BMUXLIB_ASP_SOURCE_AV_TYPE_MAX
} BMUXlib_ASP_Source_AV_Type_e;

typedef struct BMUXlib_ASP_Source_ContextMap_t
{
    uint32_t uiRead; /* Address of the buffer READ register */
    uint32_t uiBase; /* Address of the buffer BASE register */
    uint32_t uiValid; /* Address of the buffer VALID register */
    uint32_t uiEnd; /* Address of the buffer END register */
} BMUXlib_ASP_Source_ContextMap_t;

typedef struct BMUXlib_ASP_Source_ContextValues_t
{
    uint64_t uiRead; /* Value of the buffer READ register */
    uint64_t uiBase; /* Value of the buffer BASE register */
    uint64_t uiValid; /* Value of the buffer VALID register */
    uint64_t uiEnd; /* Value of the buffer END register */
} BMUXlib_ASP_Source_ContextValues_t;

typedef struct BMUXlib_ASP_Source_Interface_t
{
    BMUXlib_ASP_Source_Type_e eType; /* The source type */
    uint16_t uiPIDChannelIndex; /* The PID channel index associated with this source
                                 * See:
                                 *   BMUXlib_ASP_TS_Output_Descriptor_t.uiPIDChannelIndex
                                 */
    BMUXlib_ASP_Source_ContextMap_t stData; /* Specifies the register offsets containing the base/end/read/valid offsets for the data */
} BMUXlib_ASP_Source_Interface_t;

#endif /* BMUXLIB_ASP_SOURCE_H_ */
