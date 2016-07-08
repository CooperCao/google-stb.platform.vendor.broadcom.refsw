/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ZCL Auto-Responses Manager private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_MANAGER_AUTO_RESPONSES_H
#define _BB_ZBPRO_ZCL_MANAGER_AUTO_RESPONSES_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Initializes Auto Responses Manager.
 */
void zbProManagerAutoResponsesInit(void);


/**//**
 * \brief   Allocates empty ZCL Local Request Descriptor from the Auto Responses Pull.
 * \param[in]   autoResponseType        Auto Response Type: either Specific (0x0) or
 *  Default (0x1).
 * \return  Pointer to allocated Request Descriptor Prototype; or NULL if there is no
 *  empty descriptor in the pull.
 */
ZbProZclLocalPrimitiveDescrPrototype_t* zbProZclManagerAutoResponsesAlloc(
                const ZBPRO_ZCL_ResponseType_t  autoResponseType);


/**//**
 * \brief   Frees used ZCL Local Request Descriptor and returns it to the Auto Responses
 *  Pull.
 * \param[in]   respDescr       Pointer to Auto Response ZCL Local Request Descriptor to
 *  be freed.
 */
void zbProZclManagerAutoResponsesFree(
                ZbProZclLocalPrimitiveDescrPrototype_t *const  respDescr);


#endif /* _BB_ZBPRO_ZCL_MANAGER_AUTO_RESPONSES_H */
