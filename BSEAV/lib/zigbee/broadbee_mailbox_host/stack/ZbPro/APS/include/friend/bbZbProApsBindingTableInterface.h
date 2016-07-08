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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/friend/bbZbProApsBindingTableInterface.h $
 *
 * DESCRIPTION:
 *   ZigBee PRO APS Binding Table interface.
 *
 * $Revision: 7361 $
 * $Date: 2015-07-08 17:17:17Z $
 *
 ****************************************************************************************/
#ifndef BBZBPROAPSBINDINGTABLEINTERFACE_H_
#define BBZBPROAPSBINDINGTABLEINTERFACE_H_


/*************************************************************************************//**
  \brief Structure, which represents a base BindingTable entity element type.
  See ZigBee Document 053474r20, subclause 2.2.4.3.1.1, table 2.6.
*****************************************************************************************/
typedef struct{
    /* 64+32-bit field. */
    ZBPRO_APS_Address_t      dstAddr;         /*!< The destination address mode and address
                                                   for the binding entry. */
    /* 64-bit field. */
    ZBPRO_APS_ExtAddr_t      srcAddr;         /*!< The source IEEE address for the binding entry. */
    /* 16-bit field. */
    ZBPRO_APS_ClusterId_t    clusterId;      /*!< The identifier of the cluster on the source device that
                                                  is to be (un)bound to/from the destination device. */
    /* 8-bit field. */
    ZBPRO_APS_EndpointId_t   srcEndpoint;    /*!< The source endpoint for the binding entry. */
    /* 8-bit field. */
    ZBPRO_APS_EndpointId_t   dstEndpoint;    /*!< The destination endpoint for the binding entry. */

} ZBPRO_APS_BindingTableEntity_t;


/*************************************************************************************//**
  \brief Returns the BindingTable size in elements.
*****************************************************************************************/
uint8_t ZBPRO_APS_BindingTableSize(void);


/*************************************************************************************//**
  \brief Returns the BindingTable LinkedToDevices entity count in elements.
*****************************************************************************************/
uint8_t ZBPRO_APS_BindingTableLinksToDevicesNumber(void);


/*************************************************************************************//**
  \brief Returns the BindingTable LinkedToGroups entity count in elements.
*****************************************************************************************/
uint8_t ZBPRO_APS_BindingTableLinksToGroupNumber(void);


/*************************************************************************************//**
  \brief Returns the BindingTable entity by index.
  \param[in] index        - entity index.
  \param[in/out] entity   - pointer to BindingTableEntity, in which data must be placed
  \return TRUE if success, FALSE if there no a BindingTable entity with such index.
*****************************************************************************************/
bool ZBPRO_APS_BindingTableEntity(uint8_t   index,
                                  ZBPRO_APS_BindingTableEntity_t *entity);

#endif /* BBZBPROAPSBINDINGTABLEINTERFACE_H_ */
