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
 * FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/private/bbZbProZdoMem.h $
 *
 * DESCRIPTION:
 *   Contains implementation of the interface for ZigBee PRO ZDO memory objects.
 *
 * $Revision: 2572 $
 * $Date: 2014-05-30 15:26:01Z $
 *
 ****************************************************************************************/


#ifndef _BB_ZBPRO_ZDO_MEM_H
#define _BB_ZBPRO_ZDO_MEM_H


/************************* INCLUDES *****************************************************/
#include "private/bbZbProZdoState.h"
#include "private/bbZbProZdoTask.h"
#include "private/bbZbProZdoIb.h"
#include "private/bbZbProZdoNetworkManager.h"
#include "private/bbZbProZdoNetworkManagerIntraPanPort.h"
#include "private/bbZbProZdoSecurityManager.h"
#include "private/bbZbProZdoPersistentManager.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for ZDO layer Static Memory.
 */
typedef struct _ZbProZdoMemDescr_t
{
    ZbProZdoStateDescr_t                        zdoState;
    ZbProZdoTaskDescr_t                         zdoTaskDescr;
    ZbProZdoIb_t                                zdoIbDescr;
    ZbProZdoSecurityManagerDescr_t              securityManager;
    ZbProZdoNetworkManagerDescr_t               networkManager;
    ZbProZdoNetworkManagerIntraPanPortDescr_t   networkManagerIntraPanPort;
    ZbProZdoPersistenDescr_t                    zdoPersistentDescr;
} ZbProZdoMemDescr_t;

/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   ZDO layer Static Memory.
 */
extern ZbProZdoMemDescr_t  zbProZdoMemDescr;


/************************* INLINES ******************************************************/
/**//**
 * \brief   Returns pointer to ZDO State descriptor.
 */
INLINE ZbProZdoStateDescr_t *zbProZdoState(void)
{
    return &zbProZdoMemDescr.zdoState;
}

/**//**
 * \brief   Returns pointer to ZDO Task descriptor.
 */
INLINE ZbProZdoTaskDescr_t *zbProZdoTaskDescr(void)
{
    return &zbProZdoMemDescr.zdoTaskDescr;
}

/**//**
 * \brief   Returns pointer to ZDO-IB descriptor.
 */
INLINE ZbProZdoIb_t *zbProZdoIb(void)
{
    return &zbProZdoMemDescr.zdoIbDescr;
}

/**//**
 * \brief   Returns pointer to ZDO Security Manager internal descriptor.
 */
INLINE ZbProZdoSecurityManagerDescr_t *zbProZdoSecurityManagerDescr(void)
{
    return &zbProZdoMemDescr.securityManager;
}

/**//**
 * \brief   Returns pointer to ZDO Network Manager internal descriptor.
 */
INLINE ZbProZdoNetworkManagerDescr_t *zbProZdoNetworkManagerDescr(void)
{
    return &zbProZdoMemDescr.networkManager;
}

/**//**
 * \brief   Returns pointer to ZDO Intra-PAN Portability Manager internal descriptor.
 */
INLINE ZbProZdoNetworkManagerIntraPanPortDescr_t *zbProZdoNetworkManagerIntraPanPortDescr(void)
{
    return &zbProZdoMemDescr.networkManagerIntraPanPort;
}

INLINE ZbProZdoPersistenDescr_t *zbProZdoPersistentDescr(void)
{
    return &zbProZdoMemDescr.zdoPersistentDescr;
}

#endif /* _BB_ZBPRO_ZDO_MEM_H */
