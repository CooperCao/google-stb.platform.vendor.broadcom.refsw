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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      Queues engine interface.
 *
*******************************************************************************/

#ifndef _BB_SYS_QUEUE_H
#define _BB_SYS_QUEUE_H


/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */


/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Queue initialization value.
 */
#define SYS_QUEUE_INITIALIZER   ((const SYS_QueueDescriptor_t){ .nextElement = NULL })

/**//**
 * \brief Queue iteration cycle template.
 * \param[in] queue - pointer to queue.
 * \param[in] iteratorName - a name of iterator to be used.
 */
#define SYS_QUEUE_ITERATION(queue, iteratorName)                            \
    for (SYS_QueueElement_t *iteratorName = SYS_QueueGetQueueHead(queue);   \
            iteratorName != NULL;                                           \
            iteratorName = SYS_QueueGetNextQueueElement(iteratorName))

/**//**
 * \brief Removes safely iterator from the queue from iteration cycle.
 * \param[in] queue - pointer to queue.
 * \param[in] iterator - a name of iterator to be used.
 */
#define SYS_QUEUE_REMOVE_ITERATOR(queue, iterator)  do { \
        SYS_QueueElement_t *iteratorToBeRemoved = iterator; \
        iterator = SYS_QueueGetNextQueueElement(iterator); \
        SYS_QueueRemoveQueueElement(queue, iteratorToBeRemoved); \
    } while (0)


/************************* TYPES *******************************************************/
/**//**
 * \brief Queue element data type.
 * \details This type is to be used as a part of other structures if they should be
 *  linked into the queue.
 */
typedef struct _SysQueueElement_t
{
    struct _SysQueueElement_t *nextElement;     /*!< Pointer to the next queue element. */
} SYS_QueueElement_t;


/**//**
 * \brief Queue descriptor data type.
 */
typedef SYS_QueueElement_t  SYS_QueueDescriptor_t;


/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
  \brief Resets queue.

  \param[in] queue - pointer to queue.
****************************************************************************************/
INLINE void SYS_QueueResetQueue(SYS_QueueDescriptor_t *queue)
{
    queue->nextElement = NULL;
}

/************************************************************************************//**
  \brief Gets queue state.
  \param[in] queue - pointer to queue.

  \return true If queue is empty and false otherwise.
****************************************************************************************/
INLINE bool SYS_QueueIsEmpty(const SYS_QueueDescriptor_t *const queue)
{
    return (NULL == queue->nextElement);
}

/************************************************************************************//**
  \brief Checks element position.
  \param[in] queueElement - pointer to queue element.
  \return true If element on the last position into given queue and false otherwise.
****************************************************************************************/
INLINE bool SYS_QueueIsLastElement(const SYS_QueueElement_t *const element)
{
    return (NULL == element->nextElement);
}

/************************************************************************************//**
  \brief Gets next queue element.

  \param[in] queueElement - pointer to queue element.
  \return pointer to next queue element.
****************************************************************************************/
INLINE SYS_QueueElement_t *SYS_QueueGetNextQueueElement(const SYS_QueueElement_t *const element)
{
    return element ? element->nextElement : NULL;
}

/************************************************************************************//**
  \brief Gets queue head element.

  \param[in] queue - pointer to queue.
  \return pointer to queue head element.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueGetQueueHead(const SYS_QueueDescriptor_t *const queue);

/************************************************************************************//**
  \brief Gets parent element or NULL if parent is absent.

  \param[in] queue - a pointer to queue.
  \return a pointer to the parent element.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueFindParentElement(const SYS_QueueDescriptor_t *const queue,
        const SYS_QueueElement_t *element);

/************************************************************************************//**
  \brief Gets queue last element.

  \param[in] queue - pointer to queue.
  \return pointer to queue last element.
****************************************************************************************/
INLINE SYS_QueueElement_t *SYS_QueueGetQueueTail(const SYS_QueueDescriptor_t *const queue)
{
    return SYS_QueueFindParentElement(queue, NULL);
}

/************************************************************************************//**
  \brief Puts queue element after given queue element.

  \param[in] after   - new queue element is put after this one.
  \param[in] element - element to put.
****************************************************************************************/
void SYS_QueueInsertElement(SYS_QueueElement_t *after, SYS_QueueElement_t *element);

/************************************************************************************//**
  \brief Puts element to queue. The element is added to the queue head.

  \param[in] queue   - pointer to queue.
  \param[in] element - element to put to queue.
****************************************************************************************/
void SYS_QueuePutQueueElementToHead(SYS_QueueDescriptor_t *queue,
        SYS_QueueElement_t *element);

/************************************************************************************//**
  \brief Puts element to queue. The element is added to the queue tail.

  \param[in] queue   - pointer to queue.
  \param[in] element - element to put to queue.
****************************************************************************************/
void SYS_QueuePutQueueElementToTail(SYS_QueueDescriptor_t *queue,
        SYS_QueueElement_t *element);

/************************************************************************************//**
  \brief Removed queue element after given queue element.

  \param[in] after   - new queue element is put after this one.
  \param[in] element - element to put.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueRemoveNextElement(SYS_QueueElement_t *parent);

/************************************************************************************//**
  \brief Removes head queue element.

  \param[in] queue   - pointer to queue.
  \returns pointer to a removed element if element is removed successfully, NULL otherwise.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueRemoveHeadElement(SYS_QueueDescriptor_t *queue);

/************************************************************************************//**
  \brief Removes queue element.

  \param[in] queue   - pointer to queue.
  \param[in] element - element to delete.
  \returns pointer to a removed element if element is removed successfully, NULL otherwise.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueRemoveQueueElement(SYS_QueueDescriptor_t *queue,
        SYS_QueueElement_t *element);

/*
 * Repeat pragma GCC optimize because function definitions (including inlined) turn these pragrmas off automatically
 * when compiled by G++ but not GCC.
 */
#if (defined(__arm__) || defined(__i386__)) && !defined(__clang__)
# pragma GCC optimize "short-enums"     /* Implement short enums. */
# pragma GCC diagnostic ignored "-Wattributes"
#endif

#endif /* _BB_SYS_QUEUE_H */

/* eof bbSysQueue.h */