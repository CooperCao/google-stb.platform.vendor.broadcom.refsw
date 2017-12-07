/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef __SARM_COMMAND_IDS_H__
#define __SARM_COMMAND_IDS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* #if SAGE_VERSION <= SAGE_VERSION_CALC(3,2)
 * #define SARM_VER_ID 0x00010000
 * #endif */

/* container out positions */
#define SARM_CONTAINER_ASYNC(container) (container->basicIn[0])
#define SARM_CONTAINER_CMD_RC(container) (container->basicIn[1])
#define SARM_CONTAINER_SARM_ID(container) (container->basicIn[2])
#define SARM_CONTAINER_STREAM_CNT(container) (container->basicIn[3])

#define SARM_CONTAINER_STATUS_BLOCK(container) container->blocks[0]
#define SARM_CONTAINER_START_PARAMS_BLOCK(container) container->blocks[0]

/*
 * List of supported SAGE commands
 * */

typedef enum sarm_CommandId_e {
    sarm_CommandId_eNone = 0,
    sarm_CommandId_eAudioOpen,
    sarm_CommandId_eAudioStart,
    sarm_CommandId_eAudioStop,
    sarm_CommandId_eAudioClose,
    sarm_CommandId_eMax
} sarm_CommandId_e;

typedef struct
{
    NEXUS_AudioCodec   codec;   /* Type of audio codec */
    bool               routingOnly; /* Bypass Monitoring */
    BAVC_XptContextMap inContext; /* RAVE writes/SAGE reads */
    BAVC_XptContextMap outContext; /* SAGE writes/Audio Decoder reads */
} _P_NEXUS_SageAudioStartSettings;

#ifdef __cplusplus
}
#endif

#endif /*__SARM_COMMAND_IDS_H__*/
