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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      Non-volatile memory manager definitions.
 *
*******************************************************************************/

#ifndef _SYS_NVM_MANAGER_H
#define _SYS_NVM_MANAGER_H

#include "bbSysTypes.h"
#include "bbSysQueue.h"
#include "bbSysPayload.h"

/**//**
 * \brief The result of access to/from Host file system. Refer to BroadBeeAppSwRefManual_v0p3a.pdf.
 */
typedef enum _NVM_HostAccessResult_t
{
    /* Completed successfully. */
    NVM_NO_ERROR,
    /* Address is out of range. */
    NVM_ADDRESS_OUT_OF_RANGE,
    /* Access has been denied. */
    NVM_ACCESS_DENIED,
    /* Access has taken too long time. */
    NVM_ACCESS_TIME_OUT,
    /* Data is not available. */
    NVM_DATA_NOT_AVAILABLE
} NVM_HostAccessResult_t;

/**//**
 * \brief Read File indication type declaration.
 */
typedef struct _NVM_ReadFileIndDescr_t NVM_ReadFileIndDescr_t;

/**//**
 * \brief Structure contains Read File indication parameters.
 */
typedef struct _NVM_ReadFileIndParams_t
{
    uint32_t            fileIndex;
    uint32_t            address;
    uint32_t            length;
} NVM_ReadFileIndParams_t;

/**//**
 * \brief Structure contains Read File response parameters.
 */
typedef struct _NVM_ReadFileRespParams_t
{
    NVM_HostAccessResult_t  status;
    SYS_DataPointer_t       payload;
} NVM_ReadFileRespParams_t;

/**//**
 * \brief Read File response type.
 */
typedef void (*NVM_ReadFileResp_t)(NVM_ReadFileIndDescr_t *orgInd, NVM_ReadFileRespParams_t *resp);

/**//**
 * \brief Read File indication descriptor.
 */
typedef struct _NVM_ReadFileIndDescr_t
{
#ifdef MAILBOX_STACK_SIDE
    struct
    {
        SYS_QueueElement_t          next;
    } service;
#endif

    NVM_ReadFileIndParams_t params;
    NVM_ReadFileResp_t      callback;
} NVM_ReadFileIndDescr_t;

/**//**
 * \brief Open File indication type declaration.
 */
typedef struct _NVM_OpenFileIndDescr_t NVM_OpenFileIndDescr_t;

/**//**
 * \brief Structure contains Open File indication parameters.
 */
typedef struct _NVM_OpenFileIndParams_t
{
    uint32_t             fileIndex;
} NVM_OpenFileIndParams_t;

/**//**
 * \brief Structure contains Open File response parameters.
 */
typedef struct _NVM_OpenFileRespParams_t
{
    NVM_HostAccessResult_t  status;
} NVM_OpenFileRespParams_t;

/**//**
 * \brief Start Writing file to  host response type.
 */
typedef void (*NVM_OpenFileResp_t)(NVM_OpenFileIndDescr_t *orgInd, NVM_OpenFileRespParams_t *resp);

/**//**
 * \brief Open File indication descriptor.
 */
typedef struct _NVM_OpenFileIndDescr_t
{
#ifdef MAILBOX_STACK_SIDE
    struct
    {
        SYS_QueueElement_t          next;
    } service;
#endif

    NVM_OpenFileIndParams_t  params;
    NVM_OpenFileResp_t       callback;
} NVM_OpenFileIndDescr_t;

/**//**
 * \brief Write File indication type declaration.
 */
typedef struct _NVM_WriteFileIndDescr_t NVM_WriteFileIndDescr_t;

/**//**
 * \brief Structure contains Write File indication parameters.
 */
typedef struct _NVM_WriteFileIndParams_t
{
    uint32_t            fileIndex;
    uint32_t            address;
    SYS_DataPointer_t   payload;
} NVM_WriteFileIndParams_t;

/**//**
 * \brief Structure contains Write File response parameters.
 */
typedef struct _NVM_WriteFileRespParams_t
{
    NVM_HostAccessResult_t  status;
} NVM_WriteFileRespParams_t;

/**//**
 * \brief Write File response type.
 */
typedef void (*NVM_WriteFileResp_t)(NVM_WriteFileIndDescr_t *orgInd, NVM_WriteFileRespParams_t *resp);

/**//**
 * \brief Write File indication descriptor.
 */
typedef struct _NVM_WriteFileIndDescr_t
{
#ifdef MAILBOX_STACK_SIDE
    struct
    {
        SYS_QueueElement_t          next;
    } service;
#endif

    NVM_WriteFileIndParams_t  params;
    NVM_WriteFileResp_t       callback;
} NVM_WriteFileIndDescr_t;

/**//**
 * \brief Close File indication type declaration.
 */
typedef struct _NVM_CloseFileIndDescr_t NVM_CloseFileIndDescr_t;

/**//**
 * \brief Structure contains Close File indication parameters.
 */
typedef struct _NVM_CloseFileIndParams_t
{
    uint32_t             fileIndex;
} NVM_CloseFileIndParams_t;

/**//**
 * \brief Structure contains Close File response parameters.
 */
typedef struct _NVM_CloseFileRespParams_t
{
    NVM_HostAccessResult_t  status;
} NVM_CloseFileRespParams_t;

/**//**
 * \brief Finish Writing file to  host response type.
 */
typedef void (*NVM_CloseFileResp_t)(NVM_CloseFileIndDescr_t *orgInd, NVM_CloseFileRespParams_t *resp);

/**//**
 * \brief Close File indication descriptor.
 */
typedef struct _NVM_CloseFileIndDescr_t
{
#ifdef MAILBOX_STACK_SIDE
    struct
    {
        SYS_QueueElement_t          next;
    } service;
#endif

    NVM_CloseFileIndParams_t  params;
    NVM_CloseFileResp_t       callback;
} NVM_CloseFileIndDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
 \brief Read File indication. Function is to read data from files
        placed in Host Non-volatile memory.
 \param[in] indDescr - pointer to the indication parameters.
 ***************************************************************************************/
void NVM_ReadFileInd(NVM_ReadFileIndDescr_t *indDescr);

/************************************************************************************//**
    \brief Open File indication. Function is intended to open file placed in Host
           Non-volatile memory for writing.
    \note Host application should create a temporary copy of the file and all following
          Write File indications (addressed to the same file index) will modify only this
          temporary copy.
    \param[in] indDescr - pointer to the indication parameters.
 ***************************************************************************************/
void NVM_OpenFileInd(NVM_OpenFileIndDescr_t *indDescr);

/************************************************************************************//**
    \brief Write File indication. Function is intended to write data to files
           placed in Host Non-volatile memory.
    \note Separate Write file indication will cause atomic updating of the file but
          if it follows the Open File indication Host application should modify only
          temporary copy of the file.
 \param[in] indDescr - pointer to the indication parameters.
 ***************************************************************************************/
void NVM_WriteFileInd(NVM_WriteFileIndDescr_t *indDescr);

/************************************************************************************//**
    \brief Close File indication. Function is intended to close writing session.
    \note Host application should replace the file with the updated temporary copy
          which was made during Open File operation.
    \param[in] indDescr - pointer to the indication parameters.
 ***************************************************************************************/
void NVM_CloseFileInd(NVM_CloseFileIndDescr_t *indDescr);

#ifdef _HOST_
void Zigbee_NVM_Init(
    int(*open_func_user)(const char *path, int oflag, ... ),
    int (*close_func_user)(int fildes),
    ssize_t (*write_func_user)(int fildes, const void *buf, size_t nbyte),
    ssize_t (*read_func_user)(int fildes, void *buf, size_t nbyte),
    off_t (*lseek_func_user)(int fd, off_t offset, int whence)
    );
void Zigbee_NVM_Uninit(void);
#endif

#endif /* _SYS_NVM_MANAGER_H */

/* eof bbSysNvmManager.h */
