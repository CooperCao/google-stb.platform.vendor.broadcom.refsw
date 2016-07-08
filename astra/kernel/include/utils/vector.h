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
#ifndef VECTOR_H_
#define VECTOR_H_

#include "arm/arm.h"
#include "arm/spinlock.h"

#include "pgtable.h"
#include "tzmemory.h"

#include "lib_printf.h"
#include "lib_string.h"

#define RESIZE_NUM_PAGES  1

namespace tzutils {

    template <typename T>
    class Vector {
    public:
        void init() {
            capacity = 0;
            size = 0;
            numPages = 0;

        }

        void clear() {
            size = 0;
            downsize();
        }

        void pushBack(const T& t) {
            if (size == capacity)
                resize();

            dynArray[size] = t;
            size++;
        }

        void popBack(T *t) {
            if (size == 0){
                return;
            }

            size--;
            if (t != nullptr) *t = dynArray[size];

            if ((capacity - size ) > PAGE_SIZE_4K_BYTES)
                downsize();
        }

        T& operator [] (size_t idx) {
            return dynArray[idx];
        }

        const T& operator [] (size_t idx) const {
            return dynArray[idx];
        }

        int numElements() const { return (int)size; }

        inline int copyFrom(const Vector<T>& rhs) {
            int n = rhs.size;
            for(int i=0; i<n; i++) {
                pushBack(rhs.dynArray[i]);
            }
            return n;
        }

    private:
        void resize() {
            int newCapacity = capacity + (RESIZE_NUM_PAGES * PAGE_SIZE_4K_BYTES);
            PageTable *kernPageTable = PageTable::kernelPageTable();

            // printf("resizing from %d to %d\n", capacity, newCapacity);

            TzMem::VirtAddr vaddr = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, newCapacity, PageTable::ScanForward);
            if (vaddr == nullptr) {
                err_msg("Exhausted kernel heap space !\n");
                kernelHalt("Virtual Address Space exhausted");
            }

            // printf("will remap to %p from %p\n", vaddr, &dynArray[0]);

            int newNumPages = 0;
            TzMem::VirtAddr curr = vaddr;
            for (int i=0; i<newCapacity; i+=PAGE_SIZE_4K_BYTES) {

                TzMem::PhysAddr paddr = TzMem::allocPage(KERNEL_PID);
                if (paddr == nullptr) {
                    err_msg("%s: Exhausted physical memory\n", __PRETTY_FUNCTION__);
                    kernelHalt("out of memory");
                }

                kernPageTable->mapPage(curr, paddr, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);
                curr = (uint8_t *)curr + PAGE_SIZE_4K_BYTES;

                newNumPages++;
            }

            T *newArr = (T *)vaddr;
            for (int i=0; i<size; i++)
                newArr[i] = dynArray[i];

            capacity = newCapacity;

            curr = dynArray;
            for (int i=0; i<numPages; i++) {
                TzMem::PhysAddr phys = kernPageTable->lookUp(curr);
                kernPageTable->unmapPage(curr);
                TzMem::freePage(phys);

                // printf("\tUnmap %p. free %p\n", curr, phys);
                curr = (uint8_t *)curr + PAGE_SIZE_4K_BYTES;
            }

            dynArray = (T *)vaddr;
            numPages = newNumPages;
        }

        void downsize() {
            int spareNumPages = (capacity - size)/PAGE_SIZE_4K_BYTES;
            if (spareNumPages == 0)
                return;

            // printf("%s: downsizing %d pages\n", __PRETTY_FUNCTION__, spareNumPages);

            PageTable *kernPageTable = PageTable::kernelPageTable();

            TzMem::VirtAddr curr = (uint8_t *)dynArray + capacity - PAGE_SIZE_4K_BYTES;
            for (int i=0; i<spareNumPages; i++) {
                TzMem::PhysAddr phys = kernPageTable->lookUp(curr);
                kernPageTable->unmapPage(phys);
                TzMem::freePage(phys);

                curr = (uint8_t *)curr - PAGE_SIZE_4K_BYTES;
                capacity -= PAGE_SIZE_4K_BYTES;
                numPages--;
            }

            if (capacity == 0)
                dynArray = nullptr;
        }

    private:
        T *dynArray;
        int numPages;
        int size;
        int capacity;

    public:
        Vector() = default;
        ~Vector() = default;

        Vector(const Vector<T>& ) = delete;
        Vector<T>& operator = (const Vector<T>& ) = delete;
    };
}



#endif /* VECTOR_H_ */
