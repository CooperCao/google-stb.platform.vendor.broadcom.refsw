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

/* SARM Version verification */
#define SARM_MAJOR_VERSION 0x0001 /* SARM Major version must be same */
#define SARM_MINOR_VERSION 0x0000 /* SARM Minor version can mismatch up to compatibility minor version */
#define SARM_VERSION ((SARM_MAJOR_VERSION << 16) | (SARM_MINOR_VERSION))

#define SARM_MIN_VERSION 0x0000 /* SARM Compatibility Minor version */
#define SARM_MINOR_VERSION_MASK 0x0000FFFF

/* container OUT positions */
#define SARM_CONTAINER_CMD_RC(container) (container->basicOut[0])       /* Command Return Code */
#define SARM_CONTAINER_SARM_VER(container) (container->basicOut[1])     /* SARM Current version */
#define SARM_CONTAINER_SARM_MIN_VER(container) (container->basicOut[2]) /* SARM SAGE minimum compatible Minor version */

/* container IN positions */
#define SARM_CONTAINER_ASYNC(container) (container->basicIn[0])
#define SARM_CONTAINER_SARM_ID(container) (container->basicIn[1])      /* SARM ID for all commands */
#define SARM_CONTAINER_STREAM_CNT(container) (container->basicIn[2])   /* Total stream count */

#define SARM_CONTAINER_SARM_MAJOR_VER(container) (SARM_CONTAINER_SARM_VER(container) >> 16)
#define SARM_CONTAINER_SARM_MINOR_VER(container) (SARM_CONTAINER_SARM_VER(container) & SARM_MINOR_VERSION_MASK)

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

/* Following structures are shared between Host & SAGE, use aligned members */
typedef struct
{
    uint32_t CDB_Read;      /* Address of the coded data buffer READ register */
    uint32_t CDB_Base;      /* Address of the coded data buffer BASE register */
    uint32_t CDB_Wrap;      /* Address of the coded data buffer WRAPAROUND register */
    uint32_t CDB_Valid;     /* Address of the coded data buffer VALID register */
    uint32_t CDB_End;       /* Address of the coded data buffer END register */

    uint32_t ITB_Read;      /* Address of the index table buffer READ register */
    uint32_t ITB_Base;      /* Address of the index table buffer BASE register */
    uint32_t ITB_Wrap;      /* Address of the index table buffer WRAPAROUND register */
    uint32_t ITB_Valid;     /* Address of the index table buffer VALID register */
    uint32_t ITB_End;       /* Address of the index table buffer END register */
} SARM_XptContextMap;

typedef struct
{
    uint32_t codec;         /* Type of audio codec */
    uint32_t noMonitoring;         /* Request bypass Monitoring, straight copy from GLR to GLR only */
    SARM_XptContextMap inContext;  /* RAVE writes/SAGE reads */
    SARM_XptContextMap outContext; /* SAGE writes/Audio Decoder reads */
} SARM_P_SageAudioStartSettings;

#endif /*__SARM_COMMAND_IDS_H__*/
