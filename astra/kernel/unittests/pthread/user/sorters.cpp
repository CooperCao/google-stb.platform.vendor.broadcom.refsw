/******************************************************************************
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
 *****************************************************************************/

/*
 * sorters.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: gambhire
 */

#include <iostream>
#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include <limits>

extern "C" void sortTest();

void printArray(const int *array, size_t arraySize) {
    for (int i=0; i<arraySize; i++)
        std::cout << array[i] << " ";
    std::cout << std::endl;
}

void printTimeDiff(const timespec *before, const timespec *after) {
    uint64_t ticksBefore = before->tv_sec*1000000 + before->tv_nsec/1000;
    uint64_t ticksAfter = after->tv_sec*1000000 + after->tv_nsec/1000;
    uint64_t delta = ticksAfter - ticksBefore;

    if (delta > 1000000) {
        std::cout << delta/1000000 << " seconds " << delta%1000000 << " microseconds " << std::endl;
    }
    else {
        std::cout << delta << " microseconds" << std::endl;
    }
    //std::cout << seconds << ":" << ns << std::endl;
}

void checkSortOrder(int *array, size_t arraySize) {
    for (int i=0; i<arraySize-1; i+=2) {
        assert(array[i] <= array[i+1]);
    }
}

void insertionSort(int *array, size_t arraySize) {
    for (int i=1; i<arraySize; i++) {
        int j=i-1;
        int key = array[i];
        while ((j >= 0) && (key < array[j])) {
            array[j+1] = array[j];
            j--;
        }
        array[j+1] = key;
    }
}

void selectionSort(int *array, size_t arraySize) {

    for (int i=0; i<arraySize-1; i++) {
        int minElement = std::numeric_limits<int>::max();
        int minIdx = -1;
        for (int j=i+1; j<arraySize; j++) {
            if (array[j] < minElement) {
                minElement = array[j];
                minIdx = j;
            }
        }

        if (minElement < array[i]) {
            int tmp = array[i];
            array[i] = minElement;
            array[minIdx] = tmp;
        }
    }
}

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int quickPartition(int *array, int lowerBound, int upperBound) {
    if (upperBound <= lowerBound)
        return -1;

    int partitionPos = lowerBound;
    int pivot = array[upperBound];
    for (int i=lowerBound; i<upperBound; i++) {
        if (array[i] < pivot) {
            swap(&array[i], &array[partitionPos]);
            partitionPos++;
        }
    }

    swap(&array[upperBound], &array[partitionPos]);
    return partitionPos;
}

void quickSort(int *array, int lowerBound, int upperBound) {
    if (upperBound <= lowerBound)
        return;

    int partitionPos = lowerBound;
    int pivot = array[upperBound];
    for (int i=lowerBound; i<upperBound; i++) {
        if (array[i] <= pivot) {
            swap(&array[i], &array[partitionPos]);
            partitionPos++;
        }
    }

    swap(&array[upperBound], &array[partitionPos]);

    //std::cout << std::endl << "lb: " << lowerBound << " ub: " << upperBound << " Array: ";
    //printArray(&array[lowerBound], upperBound-lowerBound+1);

    quickSort(array, lowerBound, partitionPos-1);
    quickSort(array, partitionPos+1, upperBound);
}

void mergeSort(int *array, int lowerBound, int upperBound, int *sortedArray) {
    if (upperBound <= lowerBound)
        return;

    int mid = (lowerBound + upperBound)/2;

    int leftSize = mid - lowerBound + 1;
    int *sortedLeft = new int[leftSize];
    mergeSort(array, lowerBound, mid, sortedLeft);

    int rightSize = upperBound - mid;
    int *sortedRight = new int[rightSize];
    mergeSort(array, mid+1, upperBound, sortedRight);

    int i=0, j=0, k = 0;
    while ((i < leftSize) && ( j < rightSize)) {
        if (sortedLeft[i] <= sortedRight[j]) {
            sortedArray[k] = sortedLeft[i];
            i++;
        }
        else {
            sortedArray[k] = sortedRight[j];
            j++;
        }
        k++;
    }

    while ( i < leftSize) {
        sortedArray[k] = sortedLeft[i];
        k++;
        i++;
    }

    while ( j < rightSize) {
        sortedArray[k] = sortedRight[j];
        k++;
        j++;
    }

    delete sortedLeft;
    delete sortedRight;

}

void sortTest() {

#if 0
    int *data = new int[1024];
    for (int i=0; i<1024; i++) {
        data[i] = rand();
    }
    quickSort(data, 0, 1023);
    checkSortOrder(data, 1024);
#endif

    int *data = new int[1024*1024];
    for (int i=0; i<1024; i++) {
        data[i] = rand();
    }

    std::cout << "sort tests: " << std::endl;

    for (int i=512; i<1024*1024; i+=512) {
        for (int j=0; j<i; j++)
            data[j] = rand();

        struct timespec before, after;
#if 0
        clock_gettime(CLOCK_REALTIME, &before);
        insertionSort(data, i);
        clock_gettime(CLOCK_REALTIME, &after);
        checkSortOrder(data, i);

        std::cout << i << " elements: Insertion sort time ";
        printTimeDiff(&before, &after);


        for (int j=0; j<i; j++)
            data[j] = rand();
        clock_gettime(CLOCK_REALTIME, &before);
        selectionSort(data, i);
        clock_gettime(CLOCK_REALTIME, &after);
        checkSortOrder(data, i);

        std::cout << i << " elements: Selection sort time ";
        printTimeDiff(&before, &after);
#endif

        for (int j=0; j<i; j++)
            data[j] = rand();
        clock_gettime(CLOCK_REALTIME, &before);
        quickSort(data, 0, i-1);
        clock_gettime(CLOCK_REALTIME, &after);
        checkSortOrder(data, i);

        std::cout << i << " elements: Quick sort time ";
        printTimeDiff(&before, &after);
    }

}
