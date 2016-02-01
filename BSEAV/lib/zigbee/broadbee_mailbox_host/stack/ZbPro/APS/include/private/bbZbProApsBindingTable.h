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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsBindingTable.h $
 *
 * DESCRIPTION:
 *   ZigBee PRO APS Binding Table interface.
 *
 * $Revision: 1209 $
 * $Date: 2014-01-24 19:46:27Z $
 *
 ****************************************************************************************/


#ifndef _ZBPRO_APS_BINDING_TABLE_H
#define _ZBPRO_APS_BINDING_TABLE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsKeywords.h"     /* ZigBee PRO APS macro keywords definition. */
#include "bbZbProApsCommon.h"       /* ZigBee PRO APS general types definitions. */


/************************* TYPES ********************************************************/
/**//**
 * \brief Binding Table Key address information data structure.
 * \note The field SrcAddr (64-bit) is omitted because this implementation of APS does not
 *  support Binding Tables caching and as a result this field is constantly equal to this
 *  device MAC Extended Address (see ZigBee Document 08-0006-05 ZigBee PRO/2007 Layer PICS
 *  and Stack Profiles, item AZD600).
 */
typedef struct _ZbProApsBindingTableKey_t
{
    /* 16-bit data. */
    ZBPRO_APS_ClusterId_t   clusterId;      /*!< The identifier of the cluster on the source device
                                                that is bound to the destination device. */
    /* 8-bit data. */
    ZBPRO_APS_EndpointId_t  endpoint;       /*!< The source endpoint of the binding entry. */

} ZbProApsBindingTableKey_t;


/**//**
 * \brief Binding Table Peer address information data structure.
 */
typedef struct _ZbProApsBindingTablePeer_t
{
    /* Structured / (32+64)-bit data. */
    ZBPRO_APS_Address_t     addr;           /*!< The destination address and address mode
                                                of the binding entry. */
    /* 8-bit data. */
    ZBPRO_APS_EndpointId_t  endpoint;       /*!< The destination endpoint of the binding entry. */

} ZbProApsBindingTablePeer_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief
    Adds new record to the Binding Table according to the key.
  \param    key
    Pointer to the binding key.
  \param    peer
    Pointer to the peer address.
  \return
    TRUE if the new record is successfully added to the Binding Table, or there is already
    a record for the specified key-peer; FALSE if there is no corresponding record and
    there is no empty place in the Binding Table to add a new record.
*****************************************************************************************/
APS_PRIVATE bool zbProApsBindingTableAdd(ZbProApsBindingTableKey_t  *key,
                                         ZbProApsBindingTablePeer_t *peer);


/*************************************************************************************//**
  \brief
    Deletes an existing record from the Binding Table according to the key.
  \param    key
    Pointer to the binding key.
  \param    peer
    Pointer to the peer address.
  \return
    TRUE if a record with the specified key is found and deleted from the Binding Table;
    FALSE if there is no corresponding record in the Binding Table.
*****************************************************************************************/
APS_PRIVATE bool zbProApsBindingTableDelete(ZbProApsBindingTableKey_t  *key,
                                            ZbProApsBindingTablePeer_t *peer);


/*************************************************************************************//**
  \brief
    Returns the i-th element of the search-by-the-key results set from the Binding Table
    where the key and the index are specified.
  \param    cnt
    Number of matched peers to skip from the start of the table. Or the index of the
    search-by-the-key results set element to be returned, starting with zero.
  \param    key
    Pointer to the key to match with.
  \param    peer
    Pointer to the peer address empty object to be filled with returned data.
  \return
    TRUE if a peer was found, FALSE otherwise.
*****************************************************************************************/
APS_PRIVATE bool zbProApsBindingTableGetPeer(uint8_t                     cnt,
                                             ZbProApsBindingTableKey_t  *key,
                                             ZbProApsBindingTablePeer_t *peer);


#endif /* _ZBPRO_APS_BINDING_TABLE_H */