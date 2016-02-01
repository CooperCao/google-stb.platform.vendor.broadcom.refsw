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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkNeighbor.h $
*
* DESCRIPTION:
*   Contains definitions for interface for ZigBee PRO NWK Neighbor Table.
*
* $Revision: 3955 $
* $Date: 2014-10-08 12:45:05Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_NEIGHBOR_H
#define _ZBPRO_NWK_NEIGHBOR_H

/************************* INCLUDES ****************************************************/
#include "bbSysTimeoutTask.h"
#include "bbZbProNwkSapTypesJoin.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief The relationship between the neighbor and the current device.
 */
typedef enum _ZBPRO_NWK_Relationship_t
{
    ZBPRO_NWK_RELATIONSHIP_PARENT                = 0x00, /*!< neighbor is the parent */
    ZBPRO_NWK_RELATIONSHIP_CHILD                 = 0x01, /*!< neighbor is a child */
    ZBPRO_NWK_RELATIONSHIP_SIBLING               = 0x02, /*!< neighbor is a sibling */
    ZBPRO_NWK_RELATIONSHIP_NONE_OF_ABOVE         = 0x03, /*!< none of the above */
    ZBPRO_NWK_RELATIONSHIP_PREVIOUS_CHILD        = 0x04, /*!< previous child */
    ZBPRO_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD = 0x05, /*!< unauthenticated child */
    ZBPRO_NWK_RELATIONSHIP_EMPTY
} ZBPRO_NWK_Relationship_t;

/**//**
 * \brief Type of life time of neighbors in ticks.
 */
typedef uint16_t ZBPRO_NWK_LifeTime_t;

/**//**
 * \brief The neighbor table structure. See ZigBee r20 3.6.1.5 table 3.48 p.370
 */
typedef struct _ZBPRO_NWK_Neighbor_t
{
    /* The time of life of a neighbor entry. */
    ZBPRO_NWK_LifeTime_t lifeTime;
    /* The estimated link quality for RF transmissions from this device. */
    ZBPRO_NWK_Lqi_t lqi;
    /* Link cost in two directions. */
    ZBPRO_NWK_LinkCost_t linkCost;
    /* Relationship with neighbor device. */
    ZBPRO_NWK_Relationship_t relationship;
    /* The type of neighbor device. */
    ZBPRO_NWK_DeviceType_t deviceType;
    /* IEEE 802.15.4-2006 7.3.1.2 Capability Information field. */
    ZBPRO_NWK_Capability_t capability;
    /* The logical channel on which the network is operating. */
    ZBPRO_NWK_Channel_t logicalChannel;
    /* The 16-bit network address of the neighboring device. */
    ZBPRO_NWK_NwkAddr_t networkAddr;
    /* The 16-bit Pan Id number. */
    ZBPRO_NWK_PanId_t panId;
    /* 64-bit IEEE address that is unique to every device. */
    ZBPRO_NWK_ExtAddr_t extAddr;
    /* The 64-bit unique identifier of the network to which the device belongs.*/
    ZBPRO_NWK_ExtPanId_t extPanId;
    /* The tree depth of the neighbor device. */
    ZBPRO_NWK_Depth_t depth;
    /* The value identifying a snapshot of the network settings with which this node is operating with.*/
    ZBPRO_NWK_UpdateId_t updateId;
    /* A value indicating if previous transmissions to the device were successful or not.
     * Higher values indicate more failures. */
    ZBPRO_NWK_TransmitFailure_t transmitFailure;

    /* Indicates if neighbor's receiver is enabled during idle periods. */
    Bool8_t rxOnWhenIdle : 1;
    /* An indication of whether the device is accepting joining requests. */
    Bool8_t permitJoining : 1;
    /* An indication of whether the device has been ruled out as a potential parent. */
    Bool8_t potentialParent : 1;
    /* Network address of the neighbor is conflict with other address in network. */
    Bool8_t isConflicted : 1;
    /* Attributes which device use to join/leave. Used by zdo */
    ZBPRO_NWK_RejoinMethod_t joinMethod : 3;
    Bool8_t waitingAuth         : 1;
    Bool8_t isSecureJoin        : 1;
    Bool8_t isPendingLeave      : 1;
    Bool8_t rejoinAfterLeave    : 1;
    Bool8_t leaveRemovesChilds  : 1;
} ZBPRO_NWK_Neighbor_t;


typedef struct _ZBPRO_NWK_New_Child_Params
{
    /* 64-bit IEEE address that is unique to every device. */
    ZBPRO_NWK_ExtAddr_t extAddr;
} ZBPRO_NWK_New_Child_Params;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Searching a neighbor entry by extended address.

  \param[in] extAddr - extended IEEE address of neighbor.
  \return NULL if no records found, or entry with extAddr otherwise.
****************************************************************************************/
NWK_PUBLIC ZBPRO_NWK_Neighbor_t *ZBPRO_NWK_FindNeighborByExtAddr(const ZBPRO_NWK_ExtAddr_t *const extAddr);

/************************************************************************************//**
  \brief Searching a neighbor entry by short address.
  \param[in] shortAddr - network address of neighbor.
  \return NULL if no records found, or entry with shortAddr otherwise
****************************************************************************************/
NWK_PUBLIC ZBPRO_NWK_Neighbor_t *ZBPRO_NWK_FindNeighborByShortAddr(const ZBPRO_NWK_NwkAddr_t shortAddr);

// TODO: Implement the following function.
/************************************************************************************//**
  \brief Returns a valid neighbor entry with specified index.
  \param[in] index - index of neighbor to be read.
  \return NULL if record is not exist.
****************************************************************************************/
INLINE ZBPRO_NWK_Neighbor_t *ZBPRO_NWK_FindNeighborByIndx(const uint8_t index)
{
    (void)index;
    return NULL;
}

/************************************************************************************//**
  \brief Returns a first unauthenticated entry.
  \return NULL if record is not exist.
****************************************************************************************/
ZBPRO_NWK_Neighbor_t *ZBPRO_NWK_FindFirstUnauthenticatedChild(void);

/************************************************************************************//**
  \brief Is given neighbor is child.

  \param[in] neighbor - pointer to an entry of neighbor in the neighbor table.
  \return True, given neighbor is child.; otherwise - false.
****************************************************************************************/
INLINE bool ZBPRO_NWK_IsSleepingChild(ZBPRO_NWK_Neighbor_t const *const neighbor)
{
    return ((ZBPRO_NWK_RELATIONSHIP_CHILD == neighbor->relationship)
            || (ZBPRO_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD == neighbor->relationship))
           && ZBPRO_DEVICE_TYPE_END_DEVICE == neighbor->deviceType;
}

/*************************************************************************************//**
  \brief Marks device as target to leave procedure.
  \parem[in]
  \return NULL if record is not exist.
*****************************************************************************************/
INLINE void ZBPRO_NWK_MarkAsLeaveTarget(const ZBPRO_NWK_ExtAddr_t *const extAddr, const bool rejoinAfterLeave, const bool leaveRemovesChilds)
{
    ZBPRO_NWK_Neighbor_t *const neighbor = ZBPRO_NWK_FindNeighborByExtAddr(extAddr);
    if (neighbor && ZBPRO_NWK_IsSleepingChild(neighbor))
    {
        neighbor->isPendingLeave = true;
        neighbor->rejoinAfterLeave = rejoinAfterLeave;
        neighbor->leaveRemovesChilds = leaveRemovesChilds;
    }
}

/*************************************************************************************//**
  \brief Returns a first child to leave procedure.
  \return NULL if record is not exist.
*****************************************************************************************/
ZBPRO_NWK_Neighbor_t *ZBPRO_NWK_FindFirstChildToLeave(void);

// TODO: Implement the following function.
/************************************************************************************//**
  \brief Removes a neighbor entry form table.

  \param[in] neighbor - pointer to the neighbor.
****************************************************************************************/
INLINE void ZBPRO_NWK_RemoveNeighbor(ZBPRO_NWK_Neighbor_t *const neighbor)
{
    (void) neighbor;
}

/************************************************************************************//**
  \brief Authenticate a child node.

  \param[in] extAddr - pointer to the extended address of child.
  \return None.
****************************************************************************************/
NWK_PUBLIC bool ZBPRO_NWK_AuthenticateNeighbor(const ZBPRO_NWK_ExtAddr_t *const extAddr);

/************************************************************************************//**
  \brief Calculate an incoming cost by LQI.

  \param[in] lqi - link quality indicator.
  \return None.
****************************************************************************************/
NWK_PUBLIC ZBPRO_NWK_Cost_t ZBPRO_NWK_CostFromLqi(const ZBPRO_NWK_Lqi_t lqi);

/************************************************************************************//**
  \brief Returns total number of neighbors in the NeighborTable.
  \return number of items.
****************************************************************************************/
uint8_t ZBPRO_NWK_NeighborTableCount(void);

/************************************************************************************//**
  \brief Returns a pointer to neighbor from neighborTable with requested index.
  \param[in] elementIndex - index of requested neighbor.
  \return pointer to neighbor or NULL if the elementIndex is incorrect.
****************************************************************************************/
ZBPRO_NWK_Neighbor_t * ZBPRO_NWK_NeighborTableEntry(uint8_t elementIndex);

#endif /* _ZBPRO_NWK_NEIGHBOR_H */
