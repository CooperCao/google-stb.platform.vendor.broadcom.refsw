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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkNeighborTable.h $
*
* DESCRIPTION:
*   Contains definitions for interface for ZigBee PRO NWK Neighbor Table.
*
* $Revision: 3955 $
* $Date: 2014-10-08 12:45:05Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_NEIGHBOR_TABLE_H
#define _ZBPRO_NWK_NEIGHBOR_TABLE_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"
#include "bbZbProNwkNeighbor.h" // TODO: should be a private
#include "private/bbZbProNwkCommonPrivate.h"
#include "bbZbProNwkConfig.h"
#include "private/bbZbProNwkDiscovery.h"

/************************* DEFINITIONS *************************************************/

#define ZBPRO_NWK_NT_SET_AS_EMPTY(ntEntry) \
    ((ntEntry)->relationship = ZBPRO_NWK_RELATIONSHIP_EMPTY)

#define ZBPRO_NWK_NT_ENTRY_IS_EMPTY(ntEntry) \
    (ZBPRO_NWK_RELATIONSHIP_EMPTY == (ntEntry)->relationship)

#define ZBPRO_NWK_NT_ENTRY_IS_AUTHENTICATED_CHILD(ntEntry) \
    (ZBPRO_NWK_RELATIONSHIP_CHILD == (ntEntry)->relationship)

#define ZBPRO_NWK_NT_ENTRY_IS_UNAUTHENTICATED_CHILD(ntEntry) \
    (ZBPRO_NWK_RELATIONSHIP_UNAUTHENTICATED_CHILD == (ntEntry)->relationship)

#define ZBPRO_NWK_NT_ENTRY_IS_END_DEVICE(ntEntry) \
    (ZBPRO_DEVICE_TYPE_END_DEVICE == (ntEntry)->deviceType)

#define ZBPRO_NWK_NT_ENTRY_IS_ROUTER(ntEntry) \
    (ZBPRO_DEVICE_TYPE_ROUTER == (ntEntry)->deviceType)

#define ZBPRO_NWK_NT_ENTRY_IS_CHILD(ntEntry) \
    (ZBPRO_NWK_NT_ENTRY_IS_AUTHENTICATED_CHILD(ntEntry) \
     || ZBPRO_NWK_NT_ENTRY_IS_UNAUTHENTICATED_CHILD(ntEntry))

#define ZBPRO_NWK_NT_ENTRY_IS_PARENT(ntEntry) \
    (ZBPRO_NWK_RELATIONSHIP_PARENT == (ntEntry)->relationship)

#define ZBPRO_NWK_NT_ENTRY_IS_RX_ON_WHEN_IDLE(ntEntry) \
    ((ntEntry)->rxOnWhenIdle)

#define ZBPRO_NWK_NT_ENTRY_IS_SAME_PAN_ID(ntEntry) \
    (zbProNwkNibApiGetPanId() == (ntEntry)->panId)

#define ZBPRO_NWK_NT_GET_ADDR(ntEntry)  ((ntEntry)->networkAddr)

/**//**
 * \brief Neighbor table iteration cycle template.
 * \param[in] iterator - a name of iterator to be used.
 */
#define ZBPRO_NWK_NEIGHBOR_ITERATION(iterator)  for (ZBPRO_NWK_Neighbor_t *iterator = zbProNwkNeighborTableBegin(); \
                                                        iterator < zbProNwkNeighborTableEnd(); \
                                                        ++iterator)
/**//**
 * \brief Calculates current iteration number starting from 0.
 * \param[in] iterator - the same iterator which was used in ZBPRO_NWK_NEIGHBOR_ITERATION() macro.
 */
#define ZBPRO_NWK_NEIGHBOR_NUMBER(iterator)     (iterator - zbProNwkNeighborTableBegin())


/**//**
 * \brief Type of the neighbor table.
 */
typedef struct _ZbProNwkNeighborTableDescr_t
{
    /* The array of a neighbor entry. */
    ZBPRO_NWK_Neighbor_t table[ZBPRO_NWK_NEIGHBOR_TABLE_SIZE];
    /* Timer for internal use. */
    SYS_TimeoutTask_t ageTimeoutTask;
} ZbProNwkNeighborTableDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Neighbor table task handler.
****************************************************************************************/
NWK_PRIVATE void zbProNwkNeighborTableHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
    \brief Reset the neighbor table.
****************************************************************************************/
NWK_PRIVATE void zbProNwkNeighborTableReset(void);

/************************************************************************************//**
    \brief Free an entry in the neighbor table.

    \param[in] neighbor - pointer to a neighbor.
****************************************************************************************/
NWK_PRIVATE void zbProNwkFreeNeighbor(ZBPRO_NWK_Neighbor_t *const neighbor);

/************************************************************************************//**
    \brief Update of time of a life of the neighbor.
    \param[in] neighbor - pointer to a neighbor.
****************************************************************************************/
NWK_PRIVATE void zbProNwkSetLifeTimeOfNeighbor(ZBPRO_NWK_Neighbor_t *const neighbor);

/************************************************************************************//**
    \brief Checking of that the given device is destination node for the
    short address.
    \param[in] shortAddr - unicast or multicast short address.
    \return 'true' if this node is destination else 'false'.
****************************************************************************************/
NWK_PRIVATE bool zbProNwkIsRouteDestination(const ZBPRO_NWK_NwkAddr_t shortAddr);

/************************************************************************************//**
    \brief Recalculation LQI and costs for a neighbor with given short address.
    \param[in] addr - short address of a neighbor.
    \param[in] linkQuality - the link quality indicator of a received packet.
****************************************************************************************/
NWK_PRIVATE void zbProNwkUpdateLqiAndLifeTime(ZBPRO_NWK_Neighbor_t *const neighbor, const ZBPRO_NWK_Lqi_t linkQuality);

/************************************************************************************//**
    \brief Updates extended address if the address was unknown.
    \param[in] neighbor - pointer to a neighbor.
    \param[in] extAddr - extended address appropriate to the entry.
    \return true, if Update succeeds; false, if an address conflict has been detected.
****************************************************************************************/
INLINE bool zbProNwkNtUpdateExtAddr(ZBPRO_NWK_Neighbor_t *const neighbor, const ZBPRO_NWK_ExtAddr_t *const extAddr)
{
    /* If a neighbor table or address map entry is located in which the 64-bit address is
        the null IEEE address (0x00....00), the 64-bit address in the table can be updated. */
    if (MAC_RESERVED_EXTENDED_ADDRESS == neighbor->extAddr)
        neighbor->extAddr = *extAddr;
    /* if the 64-bit address is not the null IEEE address and does not correspond to the
        received 16-bit address, the device has detected a conflict elsewhere in the network. */
    else if (*extAddr != neighbor->extAddr)
    {
        if (ZBPRO_NWK_COORDINATOR_ADDR != neighbor->networkAddr) // NOTE: Ignoring address conflict for the coordinator short address
            neighbor->isConflicted = true;
        return false;
    }
    return true;
}

/************************************************************************************//**
    \brief Marks neighbor as not conflicted.
    \param[in] nwkAddr - neighbor short address.
****************************************************************************************/
INLINE bool zbProNwkNtConflictResolved(const ZBPRO_NWK_NwkAddr_t shortAddr)
{
    ZBPRO_NWK_Neighbor_t *const neighbor = ZBPRO_NWK_FindNeighborByShortAddr(shortAddr);
    if (NULL != neighbor)
        neighbor->isConflicted = false;
    return (NULL != neighbor);
}

/************************************************************************************//**
    \brief Changes short address of appropriate entry.
    \param[in] extAddr - a neighbor extended address.
    \param[in] shortAddr - a new neighbor short address
****************************************************************************************/
INLINE void zbProNwkNtChangeShortAddr(const ZBPRO_NWK_ExtAddr_t *const extAddr, const ZBPRO_NWK_NwkAddr_t shortAddr)
{
    ZBPRO_NWK_Neighbor_t *const neighbor = ZBPRO_NWK_FindNeighborByExtAddr(extAddr);
    if (NULL != neighbor)
    {
        neighbor->networkAddr = shortAddr;
        neighbor->isConflicted = false;
    }
}

/************************************************************************************//**
    \brief Gets extended address associated with the short address.
    \param[in] shortAddr - short address.
    \return MAC_RESERVED_EXTENDED_ADDRESS if associated address not exist and extended address otherwise.
****************************************************************************************/
INLINE ZBPRO_NWK_ExtAddr_t zbProNwkNtGetExtbyShort(const ZBPRO_NWK_NwkAddr_t shortAddr)
{
    ZBPRO_NWK_Neighbor_t *const neighbor = ZBPRO_NWK_FindNeighborByShortAddr(shortAddr);
    return (NULL != neighbor) ? neighbor->extAddr : MAC_RESERVED_EXTENDED_ADDRESS;
}

/************************************************************************************//**
    \brief Find a neighbor with address conflict.
    \return short address of conflict or ZBPRO_NWK_NOT_VALID_UNICAST_ADDR if address not found.
****************************************************************************************/
ZBPRO_NWK_NwkAddr_t zbProNwkNtFindConflictedAddr(void);

/************************************************************************************//**
    \brief Start the age timer of the neighbor table.
****************************************************************************************/
NWK_PRIVATE void zbProNwkStartAgeTimerOfNeighborTable(void);

/************************************************************************************//**
    \brief Stop the age timer of the neighbor table.
****************************************************************************************/
NWK_PRIVATE void zbProNwkStopAgeTimerOfNeighborTable(void);

/************************************************************************************//**
    \brief Search of the following child after given with a flag rxOnWhenIdle in 'false'
  and that can take indirect packet.
    \param[in] neighbor - the neighbor with which search should begin.
    \param[in] exceptShortAddr - the address of the child which should be passed.

    \return Pointer to sleeping child's entry in the neighbor table.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_Neighbor_t *zbProNwkNextSleepingChildForIndirectTx(ZBPRO_NWK_Neighbor_t *neighbor,
        const ZBPRO_NWK_NwkAddr_t exceptShortAddr);

/******************************************************************************
    \brief Searching a full function device in PAN with minimum value of network
    address which more given address.
    \param[in] minAddr - value of minimum network address.

    \return Pointer to entry in the neighbor table or NULL.
 ******************************************************************************/
ZBPRO_NWK_Neighbor_t *zbProNwkNextFFDWithMinimumAddress(const ZBPRO_NWK_NwkAddr_t minAddr);

/************************************************************************************//**
    \brief Gets the current link cost values.
    \param[in] neighbor - pointer to a neighbor

    \return values of incoming and outgoing costs.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_LinkCost_t zbProNwkGetLinkCost(const ZBPRO_NWK_Neighbor_t *const neighbor);

/************************************************************************************//**
    \brief Updates neighbor LQI, outgoing cost and time of life.

    \param[in] extAddr        - an extended (64-bit) address.
    \param[in] shortAddr      - a short network (16-bit) address.
    \param[in] linkQuality    - the link quality indicator.
    \param[in] outgoingCost   - outgoing link cost.
****************************************************************************************/
NWK_PRIVATE void zbProNwkUpdateNeighborByLinkStatusCmd(const ZBPRO_NWK_ExtAddr_t *const extAddr,
    const ZBPRO_NWK_NwkAddr_t shortAddr, const ZBPRO_NWK_Lqi_t linkQuality, ZBPRO_NWK_PathCost_t outgoingCost);

/************************************************************************************//**
    \brief Selects best suitable parent.
    \param[in] extPanId               - extended pan id.
    \param[in] checkPermitJoiningFlag - check or not permit joining flag.
    \return pointer to best suitable parent or NULL if parent is not exist.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_Neighbor_t *zbProNwkNtGetSuitableParent(const ZBPRO_NWK_ExtPanId_t *const extPanId,
        const bool checkPermitJoiningFlag);

/************************************************************************************//**
    \brief Cleans potential parent flag.
****************************************************************************************/
INLINE void zbProNwkNtCleanPotentialParentFlag(ZBPRO_NWK_Neighbor_t *const neighbor)
{
    neighbor->potentialParent = false;
}

/************************************************************************************//**
    \brief Gets parent entry.
    \return pointer to the parent or NULL if parent is not exist.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_Neighbor_t *zbProNwkNtGetParentEntry(void);

/************************************************************************************//**
      \brief Adds suitable parent to table.
****************************************************************************************/
INLINE void zbProNwkNtMarkAsParent(ZBPRO_NWK_Neighbor_t *const neighbor)
{
    neighbor->relationship = ZBPRO_NWK_RELATIONSHIP_PARENT;
}

/************************************************************************************//**
    \brief Checks capabilities and adds child to the neighbor table.
    \param[in] extAddr - extended address of the child device.
    \param[in] preferAddr - preferred short address.
    \param[in] capabilityInfo - child capability information bit-field.
    \param[in] joinMethod - method which device use to join.

    \return pointer to the child entry if child is successfully added and NULL otherwise
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_Neighbor_t *zbProNwkNTAddChildEntry(
        const ZBPRO_NWK_ExtAddr_t *const extAddr,
        const ZBPRO_NWK_NwkAddr_t preferAddr,
        const ZBPRO_NWK_Capability_t capabilityInfo,
        const ZBPRO_NWK_RejoinMethod_t joinMethod,
        bool isSecureJoin,
        const ZBPRO_NWK_Lqi_t linkQuality);

/************************************************************************************//**
    \brief Add a suitable parent entry instead of a worst table entry.
    \param[in] beaconNotify - pointer to the beacon notification structure.

    \return pointer to the new entry if it's successfully added and NULL otherwise.
****************************************************************************************/
NWK_PRIVATE ZBPRO_NWK_Neighbor_t *zbProNwkNTAddSuitableParent(
        ZbProNwkParsedBeaconNotify_t *const beaconNotify);

/************************************************************************************//**
  \brief Remove all discovery information from neighbor table.
****************************************************************************************/
void zbProNwkNTRemoveDiscoveryInformation(void);

/************************************************************************************//**
    \brief Returns Router capacity flag.
    \return True if device is capable to accept a Router as a child, false otherwise.
****************************************************************************************/
bool zbProNwkNTGetRouterCapacity(void);

/************************************************************************************//**
    \brief Returns End Device capacity flag.
    \return True if device is capable to accept an End Device as a child, false otherwise.
****************************************************************************************/
bool zbProNwkNTGetEdCapacity(void);

/************************************************************************************//**
    \brief Checks a panId on uniqueness.
    \return True if given panId isn't encountered and false otherwise.
****************************************************************************************/
bool zbProNwkNTCheckPanId(const ZBPRO_NWK_PanId_t panId);

/************************************************************************************//**
    \brief Update all childs and parent entry.
****************************************************************************************/
void zbProNwkNtMoveToTheNewPanId(void);

/************************************************************************************//**
    \brief Returns next neighbor.
****************************************************************************************/
ZBPRO_NWK_Neighbor_t *ZBPRO_NWK_GetNextNeighborRouter(ZBPRO_NWK_Neighbor_t *const prevNighbor,
    const ZBPRO_NWK_NwkAddr_t forbiddenAddr1, const ZBPRO_NWK_NwkAddr_t forbiddenAddr2);

/************************************************************************************//**
    \brief  Resets counters of transmission failures over the all neighbors.
****************************************************************************************/
void zbProNwkNTResetTransmissionFailures(void);

/************************************************************************************//**
    \brief  Increment counter of transmission failures for the addressed neighbor.
    \param[in]  neighbor    Pointer to neighbor record which transmission counter is to
        be updated.
****************************************************************************************/
void zbProNwkNTUpdateTransmissionFailures(ZBPRO_NWK_Neighbor_t *const  neighbor);

#endif /* _ZBPRO_NWK_NEIGHBOR_TABLE_H */
