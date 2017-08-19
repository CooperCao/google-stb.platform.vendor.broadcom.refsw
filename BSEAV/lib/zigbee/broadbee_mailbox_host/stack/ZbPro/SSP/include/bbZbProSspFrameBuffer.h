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

/******************************************************************************
 *
 * DESCRIPTION:
 *      Common Frame routine definitions.
 *
*******************************************************************************/

#ifndef _ZBPRO_SSP_FRAME_BUFFER_H
#define _ZBPRO_SSP_FRAME_BUFFER_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysPayload.h"
#include "bbSysQueue.h"
#include "bbMacSapAddress.h"

/*********************** DEFINITIONS ***************************************************/

/**//**
 * \brief Generalized frame buffer for Encryption/Decryption.
 * \ingroup ZBPRO_SSP_Types
 */
typedef struct _ZbProSspFrameBuffer_t
{
    SYS_DataPointer_t       header;         /*!< Common header */
    SYS_DataPointer_t       auxHeader;      /*!< Auxiliary header */
    SYS_DataPointer_t       mic;            /*!< Frame MIC */
    SYS_DataPointer_t       nonce;          /*!< Frame Nonce */

    SYS_DataPointer_t       payload;        /*!< Frame Payload */
} ZbProSspFrameBuffer_t;

/**//**
 * \brief Resets all pointers contained in the frame buffer.
 */
#define ZBPRO_SSP_RESET_FRAME_BUFFER(frameBuffer) \
    SYS_WRAPPED_BLOCK(memset(&frameBuffer, 0x00, sizeof(ZbProSspFrameBuffer_t)))

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Tries to allocate memory for all parts of an incoming frame.
    \param[in] frameBuffer - pointer to the frame buffer with already detached frame header.
    \return True is operation was successful, false otherwise.
****************************************************************************************/
bool zbProSspAllocRxFrameBuffer(ZbProSspFrameBuffer_t *frameBuffer);

/************************************************************************************//**
    \brief Tries to allocate memory for all parts of outcoming frame.
    \param[in] frameBuffer - pointer to the frame buffer with already detached frame header.
    \param[in] headerLength - common header length in bytes.
    \param[in] payloadLength - payload length in bytes.
    \return True is operation was successful, false otherwise.
****************************************************************************************/
bool zbProSspAllocTxFrameBuffer(ZbProSspFrameBuffer_t *frameBuffer,
                                uint8_t headerLength,
                                uint8_t payloadLength);

/************************************************************************************//**
    \brief Frees memory  allocated for frame.
    \param[in] frameBuffer - pointer to the frame buffer with already detached frame header.
    \param[in] freePayload - if true payload will be freed.
    \return Nothing.
****************************************************************************************/
void zbProSspFreeFrameBuffer(ZbProSspFrameBuffer_t *frameBuffer, bool freePayload);

/************************************************************************************//**
    \brief Detaches MIC from the frame payload.
    \param[in, out] frameBuffer - pointer to the frame buffer with already detached frame header.

    \note The caller MUST be sure that the function to work with frame buffer is
          called in appropriate order.
    \return True is operation was successful, false otherwise.
****************************************************************************************/
bool zbProSspDetachMic(ZbProSspFrameBuffer_t *frameBuffer);

/************************************************************************************//**
    \brief Splits the frame after transmission.
    \param[in, out] frameBuffer - pointer to the frame buffer with transmitted frame.
    \return Nothing.
****************************************************************************************/
void zbProSspSplitTxedFrame(ZbProSspFrameBuffer_t *const frameBuffer);

#endif /* _ZBPRO_SSP_FRAME_BUFFER_H */

/* eof bbZbProSspFrameBuffer.h */