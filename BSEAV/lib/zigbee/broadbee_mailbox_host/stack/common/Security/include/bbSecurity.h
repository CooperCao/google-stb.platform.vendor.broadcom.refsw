/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: branches/ext_xhuajun/MailboxIntegration/stack/common/Security/include/bbSecurity.h $
*
* DESCRIPTION:
*   CCM* encryption/decryption.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
*****************************************************************************************/


#ifndef _BB_SECURITY_H
#define _BB_SECURITY_H


/************************* INCLUDES *****************************************************/
#include "bbSysPayload.h"           /* System Payloads API. */
#include "bbSysMemMan.h"            /* Memory manager. */
#include "bbSysTaskScheduler.h"     /* Task scheduler. */

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief CCM* L value.
 */
#define SECURITY_CCM_L                  2

/**//**
 * \brief CCM* nonce size.
 */
#define SECURITY_CCM_NONCE_SIZE         (15 - SECURITY_CCM_L)

typedef enum _Security_CCMStatus_t
{
    CCM_SUCCESS = 0,
    CCM_ERROR_AUTH
} Security_CCMStatus_t;


/************************* TYPES ********************************************************/
/**//**
 * \brief CCM* request prototype.
 */
typedef struct _Security_CCMReq_t Security_CCMReq_t;

/**//**
 * \brief CCM* request callback type.
 */
typedef void (*Security_Callback_t)(Security_CCMReq_t *req, Security_CCMStatus_t status);

/**//**
 * \brief CCM* request type.
 */
struct _Security_CCMReq_t
{
    /* Service fields */
    struct
    {
        SYS_QueueElement_t queueElement;
        bool isEncrypt;
    } service;
    /* authentication only */
    SYS_DataPointer_t a;
    union
    {
        /* text to be encrypted */
        SYS_DataPointer_t m;
        /* text to be decrypted */
        SYS_DataPointer_t c;
    };
    /* Nonce */
    SYS_DataPointer_t nonce;
    /* MIC */
    SYS_DataPointer_t mic;
    /* pointer to key */
    const uint8_t *key;
    /* pointer to the callback function */
    Security_Callback_t callback;
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initializes security task.

 \return Nothing.
 ****************************************************************************************/
void Security_Init(void);

/************************************************************************************//**
 \brief Starts encryption process.

 \param[in] req - the pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void Security_EncryptReq(Security_CCMReq_t *req);

/************************************************************************************//**
 \brief Starts decryption process.

 \param[in] req - the pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void Security_DecryptReq(Security_CCMReq_t *req);

#endif /* _BB_SECURITY_H */