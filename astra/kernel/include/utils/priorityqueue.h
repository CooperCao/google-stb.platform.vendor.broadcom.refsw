/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/
#ifndef PRIORITYQUEUE_H_
#define PRIORITYQUEUE_H_

#include "vector.h"
namespace tzutils {

//#define TASK_LOG

template <typename T>
class PriorityQueue {
public:
    void init() {
        queue.init();
        queue.pushBack(nullptr);
    }

    void clear() {
        queue.clear();
    }

    void enqueue(T *element) {
        // printf("%s: \nadding %p handle 0x%x%x\n", __PRETTY_FUNCTION__, element,
        //      (unsigned int)(element->id() >> 32), (unsigned int)(element->id() & 0xffffffff));

        queue.pushBack(element);
        bubbleUp();
        // print();
    }

    T *head() {
        if (queue.numElements() > 1)
            return queue[1];
        return nullptr;
    }

    T *dequeue() {
        //print();
        if (queue.numElements() <= 1)
            return nullptr;

        T *rv = queue[1];
        queue[1] = queue[queue.numElements() - 1];
        queue.popBack(nullptr);

        //printf("after popback\n");
        //print();

        if (queue.numElements() > 1)
            bubbleDown(1);

        //printf("after bubbledown\n");
        //print();
        return rv;
    }

    T *remove(typename T::IDType elementId) {
        //printf("\n\nIn remove. Before any changes: \n");
        //print();

        int numElements = queue.numElements();
        for (int i=1; i<numElements; i++) {
            if (queue[i]->id() == elementId) {
                T *rv = queue[i];
                queue[i] = queue[numElements-1];

                //printf("before pop back:\n");
                //print();

                queue.popBack(nullptr);

                //printf("after pop back:\n");
                //print();

                //printf("removed %p. before bubble down\n", rv);
                bubbleDown(i);

                //printf("after bubbledown:\n");
                //print();

                return rv;
            }
        }

        //printf("Timer does not exist\n");
        //print();
        return nullptr;
    }



#ifdef TASK_LOG // To print the CPU %

    void printRunValue(const char* queueName, uint64_t worldRunTime) {
        int numElements = queue.numElements();
        printf("-%s - %d tasks-: \t", queueName, numElements-1);
        for (int i=1; i<numElements; i++) {
            //printf("%p %d: %p ", &queue[i], i, queue[i]);
            if (queue[i] != nullptr) {
                uint64_t handle = queue[i]->id();
                uint64_t cumRunTime = queue[i]->getRunValue();

                unsigned int global_percent  = (unsigned int)((cumRunTime *10000) / worldRunTime);
                printf(": Task %d %u.%u%%\t", (int)(handle & 0xffffffff), global_percent/100, global_percent%100);

            }
            else {
                //printf("\n");
            }
        }
        printf("\n");
    }
    void resetRunValue(uint64_t nowRunTime) {
        int numElements = queue.numElements();
        for (int i=1; i<numElements; i++) {
            if (queue[i] != nullptr) {
                queue[i]->setRunValue(nowRunTime);
            }
        }
    }


    void print() {
        int numElements = queue.numElements();
        printf("--%d--\n", numElements);
        for (int i=0; i<numElements; i++) {
            printf("%p %d: %p ", &queue[i], i, queue[i]);
            if (queue[i] != nullptr) {
                uint64_t handle = queue[i]->id();
                uint64_t totalRunTime = queue[i]->pqValue();
                uint64_t slot = queue[i]->slot();

                printf("Handle 0x%x%x totalRunTime 0x%x%x slot %d\n",
                        (unsigned int)(handle >> 32), (unsigned int)(handle & 0xffffffff),
                        (unsigned int)(totalRunTime >> 32), (unsigned int)(totalRunTime & 0xffffffff), (unsigned int)( slot & 0xffffffff));
            }
            else {
                printf("\n");
            }
        }
        printf("---\n");
    }


#endif
private:
    void bubbleUp() {

        int numElements = queue.numElements();
        int childIdx = numElements - 1;
        int parentIdx = childIdx/2;

        while (parentIdx >= 1) {
            T *child = queue[childIdx];
            T *parent = queue[parentIdx];
            if (parent->dominates(child))
                break;

            queue[parentIdx] = child;
            queue[childIdx] = parent;

            childIdx = parentIdx;
            parentIdx = childIdx/2;
        }
    }

    void bubbleDown(int idx) {

        int numElements = queue.numElements();
        int parentIdx = idx;

        while (parentIdx <= numElements/2) {
            if (parentIdx == (numElements-1))
                break;

            int childIdx1 = parentIdx * 2;
            if (childIdx1 >= numElements)
                break;

            int childIdx2 = childIdx1 + 1;

            T *parent = queue[parentIdx];
            T *child1 = queue[childIdx1];
            T *child2 = (childIdx2 < numElements) ? queue[childIdx2] : nullptr;

            if (parent->dominates(child1) && parent->dominates(child2))
                break;

            if (child1->dominates(child2) && child1->dominates(parent)) {
                queue[parentIdx] = child1;
                queue[childIdx1] = parent;

                parentIdx = childIdx1;
            }
            else {
                queue[parentIdx] = child2;
                queue[childIdx2] = parent;

                parentIdx = childIdx2;
            }
        }
    }



#if 0
    void print() {
        int numElements = queue.numElements();
        for (int i=0; i<numElements; i++) {
            printf("%p %d: %p ", &queue[i], i, queue[i]);
            if (queue[i] != nullptr) {
                uint64_t handle = queue[i]->id();
                uint64_t fireAt = queue[i]->pqValue();

                printf("Handle 0x%x%x fireAt 0x%x%x\n",
                            (unsigned int)(handle >> 32), (unsigned int)(handle & 0xffffffff),
                            (unsigned int)(fireAt >> 32), (unsigned int)(fireAt & 0xffffffff));
            }
            else {
                printf("\n");
            }
        }
    }
#endif

private:
    Vector<T *> queue;

public:
    PriorityQueue() = default;
    ~PriorityQueue() = default;

    PriorityQueue(const PriorityQueue<T>& ) = delete;
    PriorityQueue<T>& operator = (const PriorityQueue<T>& ) = delete;
};


}

#endif /* PRIORITYQUEUE_H_ */
