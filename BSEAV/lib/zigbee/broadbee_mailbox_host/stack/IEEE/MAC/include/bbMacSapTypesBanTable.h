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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesBanTable.h $
*
* DESCRIPTION:
*   MAC-SAP ban table definitions. The ban table functionality used by a test scripts
*       to create networks with a complex link setup.
*
* $Revision: 3368 $
* $Date: 2014-08-21 16:02:35Z $
*
*****************************************************************************************/
#ifndef _BB_MAC_SAP_TYPES_BAN_TABLE_H
#define _BB_MAC_SAP_TYPES_BAN_TABLE_H

/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"
#include "bbMacSapAddress.h"

/************************* DEFINITIONS **************************************************/
#if (0 < _MAC_BAN_TABLE_SIZE_)
# ifdef _MAC_BAN_TABLE_
#  error Do not define _MAC_BAN_TABLE_ by yourself. It can break the logic.
# else
#  define _MAC_BAN_TABLE_
# endif

/**//**
 * \brief The special value of the link cost field to be banned entry.
 */
#define MAC_BANNED_LINK_COST_VALUE 0xFF

/**//**
 * \brief Link cost type. For more information see R20 chapter 3.6.3.1 p.388
 */
typedef uint8_t linkCost_t;


/**//**
 * \brief Confirmation type for all commands to the ban table.
 */
typedef struct _MAC_BanTableConfParams_t
{
    MAC_Status_t status;
} MAC_BanTableConfParams_t;


/**//**
 * \brief Block of type definitions appropriate to the MAC_BanTableSetDefaultAction command
 */
typedef enum _MAC_BanTableDefaultAction_t
{
    PASS_IF_NOT_EXIST = 0,
    BAN_IF_NOT_EXIST = 1,
} MAC_BanTableDefaultAction_t;
typedef struct _MAC_BanTableSetDefaultActionReqDescr_t  MAC_BanTableSetDefaultActionReqDescr_t;
typedef void MAC_BanTableSetDefaultActionCallback_t(MAC_BanTableSetDefaultActionReqDescr_t *const reqDescr, MAC_BanTableConfParams_t *const confParams);
typedef struct _MAC_BanTableSetDefaultActionReqParams_t
{
    MAC_BanTableDefaultAction_t action;
} MAC_BanTableSetDefaultActionReqParams_t;
struct _MAC_BanTableSetDefaultActionReqDescr_t
{
    MAC_BanTableSetDefaultActionReqParams_t params;
    MAC_BanTableSetDefaultActionCallback_t *callback;
};


/**//**
 * \brief Block of type definitions appropriate to the MAC_BanAllLink command
 */
typedef struct _MAC_BanTableAddLinkReqDescr_t  MAC_BanTableAddLinkReqDescr_t;
typedef void MAC_BanTableAddLinkCallback_t(MAC_BanTableAddLinkReqDescr_t *const reqDescr, MAC_BanTableConfParams_t *const confParams);
typedef struct _MAC_BanTableAddLinkReqParams_t
{
    /* Link extended address value */
    MAC_Addr64bit_t extAddr;
    /* Link short address value */
    MAC_Addr16bit_t shortAddr;
    /* Link cost value for all the frames received from the node. */
    linkCost_t      linkCost;
} MAC_BanTableAddLinkReqParams_t;
struct _MAC_BanTableAddLinkReqDescr_t
{
    MAC_BanTableAddLinkReqParams_t params;
    MAC_BanTableAddLinkCallback_t *callback;
};

/************************************************************************************//**
  \brief Sets an action for all addresses what not contained into the ban table.
  \param[in] reqDescr - the request pointer.
****************************************************************************************/
MAC_PUBLIC void MAC_BanTableSetDefaultAction(MAC_BanTableSetDefaultActionReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief Adds or updates ban table entry.
  \param[in] reqDescr - the request pointer.
****************************************************************************************/
MAC_PUBLIC void MAC_BanTableAddLink(MAC_BanTableAddLinkReqDescr_t *const reqDescr);

#endif /* _MAC_BAN_TABLE_SIZE_ */
#endif /* _BB_MAC_SAP_TYPES_BAN_TABLE_H */