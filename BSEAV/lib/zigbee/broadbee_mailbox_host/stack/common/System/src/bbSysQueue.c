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
 *      Queues engine implementation.
 *
*******************************************************************************/

/************************* INCLUDES *****************************************************/
#include "bbSysQueue.h"

/************************* IMPLEMENTATION ***********************************************/
/************************************************************************************//**
  \brief Gets queue head element.

  \param[in] queue - pointer to queue.
  \return pointer to queue head element.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueGetQueueHead(const SYS_QueueDescriptor_t *const queue)
{
    SYS_DbgAssert(queue, SYSQUEUE_SYSQUEUEGETQUEUEHEAD_0);
    return SYS_QueueGetNextQueueElement(queue);
}

/************************************************************************************//**
  \brief Gets parent element or NULL if parent is absent.

  \param[in] queue - a pointer to queue.
  \return a pointer to the parent element.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueFindParentElement(const SYS_QueueDescriptor_t *const queue,
        const SYS_QueueElement_t *element)
{
    SYS_QueueElement_t *parent = (SYS_QueueElement_t *)queue;
    SYS_QueueElement_t *iterator = SYS_QueueGetQueueHead(queue);

    while (iterator || !element)
    {
        if (element == iterator)
            return parent;
        parent = iterator;
        iterator = SYS_QueueGetNextQueueElement(iterator);
    }
    return NULL;
}

/************************************************************************************//**
  \brief Puts queue element after given queue element.

  \param[in] after   - new queue element is put after this one.
  \param[in] element - element to put.
****************************************************************************************/
void SYS_QueueInsertElement(SYS_QueueElement_t *after, SYS_QueueElement_t *element)
{
    SYS_DbgAssert(after, SYSQUEUE_SYSQUEUEINSERTELEMENT_0);
    SYS_DbgAssert(element, SYSQUEUE_SYSQUEUEINSERTELEMENT_1);

    element->nextElement = after->nextElement;
    after->nextElement = element;
}

/************************************************************************************//**
  \brief Puts element to queue. The element is added to the queue head.

  \param[in] queue   - pointer to queue.
  \param[in] element - element to put to queue.
****************************************************************************************/
void SYS_QueuePutQueueElementToHead(SYS_QueueDescriptor_t *queue,
        SYS_QueueElement_t *element)
{
    SYS_DbgAssert(element, SYSQUEUE_SYSQUEUEPUTQUEUEELEMENTTOHEAD_0);
    SYS_DbgAssertComplex(!SYS_QueueFindParentElement(queue, element), SYSQUEUE_SYSQUEUEPUTQUEUEELEMENTTOHEAD_1);

    SYS_QueueInsertElement(queue, element);
}

/************************************************************************************//**
  \brief Puts element to queue. The element is added to the queue tail.

  \param[in] queue   - pointer to queue.
  \param[in] element - element to put to queue.
****************************************************************************************/
void SYS_QueuePutQueueElementToTail(SYS_QueueDescriptor_t *queue,
        SYS_QueueElement_t *element)
{
    SYS_DbgAssertComplex(!SYS_QueueFindParentElement(queue, element), SYSQUEUE_SYSQUEUEPUTQUEUEELEMENTTOTAIL_0);

    SYS_QueueInsertElement(SYS_QueueGetQueueTail(queue), element);
}

/************************************************************************************//**
  \brief Removed queue element after given queue element.

  \param[in] after   - new queue element is put after this one.
  \param[in] element - element to put.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueRemoveNextElement(SYS_QueueElement_t *parent)
{
    SYS_DbgAssert(parent,               SYSQUEUE_SYSQUEUEREMOVENEXTELEMENT_0);
    SYS_DbgAssert(parent->nextElement,  SYSQUEUE_SYSQUEUEREMOVENEXTELEMENT_1);

    /* NOTE: 'Requet' memory allocated by mailbox was used after the execution of a callback function.
        Solution -> remove this element before callback call. */
    SYS_DbgAssertComplex((void *)0xFDFDFDFD != parent->nextElement->nextElement, SYSQUEUE_DELETED_MEMORY_WAS_USED);

    SYS_QueueElement_t *next = parent->nextElement;
    parent->nextElement = parent->nextElement->nextElement;
    return next;
}

/************************************************************************************//**
  \brief Removes head queue element.

  \param[in] queue   - pointer to queue.
  \returns pointer to a removed element if element is removed successfully, NULL otherwise.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueRemoveHeadElement(SYS_QueueDescriptor_t *queue)
{
    return SYS_QueueIsEmpty(queue) ? NULL : SYS_QueueRemoveNextElement(queue);
}

/************************************************************************************//**
  \brief Removes queue element.

  \param[in] queue   - pointer to queue.
  \param[in] element - element to delete.
  \returns pointer to a removed element if element is removed successfully, NULL otherwise.
****************************************************************************************/
SYS_QueueElement_t *SYS_QueueRemoveQueueElement(SYS_QueueDescriptor_t *queue,
        SYS_QueueElement_t *element)
{
    SYS_QueueElement_t *parent = SYS_QueueFindParentElement(queue, element);
    return (parent) ? SYS_QueueRemoveNextElement(parent) : NULL;
}

/* eof bbSysQueue.c */