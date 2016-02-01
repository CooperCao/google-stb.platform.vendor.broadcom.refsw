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
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysPriorityQueue.h $
*
* DESCRIPTION:
*   Priority queue engine interface and implementation.
*
* $Revision: 1195 $
* $Date: 2014-01-23 13:03:59Z $
*
****************************************************************************************/


#ifndef _BB_SYS_PRIORITYQUEUE_H
#define _BB_SYS_PRIORITYQUEUE_H


/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */
#include "bbSysQueue.h"             /* Queues engine interface.      */


/************************* TYPES *******************************************************/
/**//**
 * \brief Priority queue key data type (i.e., the priority value of an element).
 */
typedef uint32_t  SYS_ElementPriority_t;


/**//**
 * \brief Priority queue element data type.
 * \details This type is to be used as a part of other structures if they should be
 *  linked into the priority queue.
 */
typedef struct _SysPriorityQueueElement_t
{
    SYS_QueueElement_t      queueElement;   /*!< Service structure for queues support. */
    SYS_ElementPriority_t   priority;       /*!< Key - the priority value. The lower the
                                                number, the higher the priority. */
} SYS_PriorityQueueElement_t;


/**//**
 * \brief Priority queue descriptor data type.
 */
typedef SYS_QueueDescriptor_t  SYS_PriorityQueueDescriptor_t;


/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Resets queue.
  \param[in] priorityQueue - pointer to priority queue.
****************************************************************************************/
INLINE void SYS_PriorityQueueReset(SYS_PriorityQueueDescriptor_t *priorityQueue)
{
    SYS_QueueResetQueue(priorityQueue);
}

/************************************************************************************//**
  \brief Convert queue element to priority queue element.
  \param[in] element - a pointer to a queue element.

  \return a pointer to a priority queue element.
****************************************************************************************/
INLINE SYS_PriorityQueueElement_t *SYS_PriorityQueueConvertToElement(SYS_QueueElement_t *element)
{
    return element ? GET_PARENT_BY_FIELD(SYS_PriorityQueueElement_t, queueElement, element) : NULL;
}

/************************************************************************************//**
  \brief Checks whether element belongs to priority queue.
  \param[in] priorityQueue  - pointer to priority queue.
  \param[in] element - pointer to element.

  \returns true if element belongs to queue, false otherwise.
****************************************************************************************/
INLINE SYS_PriorityQueueElement_t *SYS_PriorityQueueFindParentElement(
    const SYS_PriorityQueueDescriptor_t *const priorityQueue,
    const SYS_PriorityQueueElement_t *const element)
{
    SYS_DbgAssert(element, SYSPRIORITYQUEUE_SYSPRIORITYQUEUEFINDPARENTELEMENT_0);
    return SYS_PriorityQueueConvertToElement(SYS_QueueFindParentElement(priorityQueue, &element->queueElement));
}

/************************************************************************************//**
  \brief Puts element to priority queue. The element is added to ordered list.
  \param[in] priorityQueue   - pointer to priority queue.
  \param[in] element  - element to put to queue.
  \param[in] priority - element priority.

  \return true if element was put successfully, false otherwise.
****************************************************************************************/
INLINE void SYS_PriorityQueuePutElement(SYS_PriorityQueueDescriptor_t *priorityQueue,
                                        SYS_PriorityQueueElement_t *element, SYS_ElementPriority_t priority)
{
    SYS_DbgAssert(priorityQueue, SYSPRIORITYQUEUE_SYSPRIORITYQUEUEPUTELEMENT_0);
    SYS_DbgAssert(element, SYSPRIORITYQUEUE_SYSPRIORITYQUEUEPUTELEMENT_1);

    ATOMIC_SECTION_ENTER(PRIORITYQUEUE_PUT)
    SYS_PriorityQueueElement_t *iterator = SYS_PriorityQueueConvertToElement(SYS_QueueGetQueueHead(priorityQueue));
    SYS_PriorityQueueElement_t *prev = SYS_PriorityQueueConvertToElement(priorityQueue);

    element->priority = priority;
    /* Find the first element with priority lower that the given one. */
    while (iterator && iterator->priority <= priority)
    {
        prev = iterator;
        iterator = SYS_PriorityQueueConvertToElement(SYS_QueueGetNextQueueElement(&iterator->queueElement));
    }

    SYS_DbgAssertComplex(!SYS_QueueFindParentElement(priorityQueue, &element->queueElement), SYSPRIORITYQUEUE_SYSPRIORITYQUEUEPUTELEMENT_2);
    SYS_QueueInsertElement(&prev->queueElement, &element->queueElement);
    ATOMIC_SECTION_LEAVE(PRIORITYQUEUE_PUT)
}

/************************************************************************************//**
  \brief Remove priority queue element.

  \param[in] priorityQueue  - pointer to priority queue.
  \param[in] element - element to delete.
  \returns true if element is removed successfully, false otherwise.
****************************************************************************************/
INLINE SYS_PriorityQueueElement_t *SYS_PriorityQueueRemoveElement(SYS_PriorityQueueDescriptor_t *priorityQueue,
        SYS_PriorityQueueElement_t *element)
{
    SYS_PriorityQueueElement_t *result = NULL;

    SYS_DbgAssert(element, SYSPRIORITYQUEUE_SYSPRIORITYQUEUEREMOVEELEMENT_0);

    ATOMIC_SECTION_ENTER(PRIORITYQUEUE_REMOVE)
    result = SYS_PriorityQueueConvertToElement(
                SYS_QueueRemoveQueueElement(priorityQueue, &element->queueElement));
    ATOMIC_SECTION_LEAVE(PRIORITYQUEUE_REMOVE)
    return result;
}

/************************************************************************************//**
  \brief Gets queue element with highest priority.
  \param[in] priorityQueue - pointer to priority queue.

  \return pointer to queue element or NULL if queue is empty.
****************************************************************************************/
INLINE SYS_PriorityQueueElement_t *SYS_PriorityQueueExtractElement(SYS_PriorityQueueDescriptor_t *priorityQueue)
{
    SYS_PriorityQueueElement_t *result = NULL;

    ATOMIC_SECTION_ENTER(PRIORITYQUEUE_EXTRACT)
    SYS_QueueElement_t *head = SYS_QueueGetQueueHead(priorityQueue);
    result = head ? SYS_PriorityQueueRemoveElement(priorityQueue,
                    SYS_PriorityQueueConvertToElement(head)) :
                    NULL;
    ATOMIC_SECTION_LEAVE(PRIORITYQUEUE_EXTRACT)
    return result;
}

#endif /* _BB_SYS_PRIORITYQUEUE_H */