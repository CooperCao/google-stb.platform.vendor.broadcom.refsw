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
******************************************************************************/

#ifndef _ZIGBEE_FILE_SERVER_API_H_
#define _ZIGBEE_FILE_SERVER_API_H_

/* 7.1 */
typedef enum
{
    /* Completed successfully. */
    BROADBEE_NO_ERROR,
    /* Address is out of range. */
    BROADBEE_ADDRESS_OUT_OF_RANGE,
    /* Access has been denied. */
    BROADBEE_ACCESS_DENIED,
    /* Access has taken too long time. */
    BROADBEE_ACCESS_TIME_OUT,
    /* Data is not available. */
    BROADBEE_DATA_NOT_AVAILABLE,
} BroadBee_HostAccessResult_t;

/* 7.3 */

typedef struct _BroadBee_DataInHost_t
{
    /* Filename Index */
    uint8_t FilenameIndex;
    /* 32-bit address offset from the beginning of the file in Host */
    uint32_t address;
    /* Length to read in byte unit */
    uint32_t length;
    /* Pointer to actual data read from or to be written to Host */
    uint8_t *hostData;
} BroadBee_DataInHost_t;

typedef struct _BroadBee_ReadFileFromHostInd_t
{
    BroadBee_DataInHost_t dataFromHost;
    void (*BroadBee_ReadFromHostResp)( struct _BroadBee_ReadFileFromHostInd_t *readFileFromHostIndParam, BroadBee_DataInHost_t *returnData);
} BroadBee_ReadFileFromHostInd_t;

typedef struct _BroadBee_WriteFileToHostInd_t
{
    BroadBee_DataInHost_t dataToHost;
    void (*BroadBee_WriteToHostResp)( struct _BroadBee_WriteFileToHostInd_t *writeFileToHostIndParam, BroadBee_HostAccessResult_t *result);
} BroadBee_WriteFileToHostInd_t;

void BroadBee_ReadFileFromHostInd( BroadBee_ReadFileFromHostInd_t *readFileFromHostParam);
void BroadBee_WriteFileToHostInd( BroadBee_WriteFileToHostInd_t *writeFileToHostParam);

#endif

/* eof zigbee_file_server_api.h */