/***************************************************************************
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
 ***************************************************************************/
#ifndef VECTOR_H_
#define VECTOR_H_

#include "arm/arm.h"
#include "arm/spinlock.h"
#include "plat_config.h"

#include "pgtable.h"
#include "tzmemory.h"

#include "lib_printf.h"
#include "lib_string.h"

namespace tzutils {

    template <typename T>
    class Vector {
    public:
        void init() {
            if (sizeof(T) > PAGE_SIZE_4K_BYTES) {
                err_msg("%s: Size of element exceeds physical page size!\n", __PRETTY_FUNCTION__);
                kernelHalt("Size of element too large");
            }

            size = 0;
            numPages = 0;
            dynArray = nullptr;
        }

        void clear() {
            size = 0;
            downsize(true);
        }

        void pushBack(const T& t) {
            // resize if the new size exceeds capacity
            if ((size + 1) * sizeof(T) > numPages * PAGE_SIZE_4K_BYTES)
                resize();

            dynArray[size] = t;
            size++;
        }

        void popBack(T *t) {
            if (size == 0)
                return;

            size--;
            if (t != nullptr)
                *t = dynArray[size];

            // downsize if capacity still allows 1 more element after downsizing
            if ((size + 1) * sizeof(T) <= (numPages - 1) * PAGE_SIZE_4K_BYTES)
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
            int newNumPages = numPages + 1;

            // printf("%s: resizing from %d to %d pages\n", __PRETTY_FUNCTION__, numPages, newNumPages);

            PageTable *kernPageTable = PageTable::kernelPageTable();

            // reserve a new continuous virtual address range
            TzMem::VirtAddr vaddr = kernPageTable->reserveAddrRange(
                (void *)KERNEL_HEAP_START,
                newNumPages * PAGE_SIZE_4K_BYTES,
                PageTable::ScanForward);

            if (vaddr == nullptr) {
                err_msg("%s: Exhausted kernel heap space!\n", __PRETTY_FUNCTION__);
                kernelHalt("Virtual Address Space exhausted");
            }

            // remap existing physical pages
            TzMem::VirtAddr vaddr_old = dynArray;
            TzMem::VirtAddr vaddr_new = vaddr;
            for (int i = 0; i < numPages; i++) {
                TzMem::PhysAddr paddr = kernPageTable->lookUp(vaddr_old);

                kernPageTable->unmapPage(vaddr_old);
                kernPageTable->mapPage(vaddr_new, paddr, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);

                vaddr_old = (uint8_t *)vaddr_old + PAGE_SIZE_4K_BYTES;
                vaddr_new = (uint8_t *)vaddr_new + PAGE_SIZE_4K_BYTES;
            }

            // alloc and map new physical pages
            for (int i = numPages; i < newNumPages; i++) {
                TzMem::PhysAddr paddr = TzMem::allocPage(KERNEL_PID);
                if (paddr == nullptr) {
                    err_msg("%s: Exhausted physical memory!\n", __PRETTY_FUNCTION__);
                    kernelHalt("Out of memory");
                }

                kernPageTable->mapPage(vaddr_new, paddr, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);

                vaddr_new = (uint8_t *)vaddr_new + PAGE_SIZE_4K_BYTES;
            }

            numPages = newNumPages;
            dynArray = (T *)vaddr;
        }

        void downsize(bool clear = false) {
            int newNumPages = (clear) ? 0 : ((size + 1) * sizeof(T) - 1) / PAGE_SIZE_4K_BYTES + 1;

            if (newNumPages == numPages)
                return;

            // printf("%s: downsizing from %d to %d pages\n", __PRETTY_FUNCTION__, numPages, newNumPages);

            PageTable *kernPageTable = PageTable::kernelPageTable();

            TzMem::VirtAddr vaddr = (uint8_t *)dynArray + newNumPages * PAGE_SIZE_4K_BYTES;
            for (int i = newNumPages; i < numPages; i++) {
                TzMem::PhysAddr paddr = kernPageTable->lookUp(vaddr);

                kernPageTable->unmapPage(vaddr);
                TzMem::freePage(paddr);

                vaddr = (uint8_t *)vaddr + PAGE_SIZE_4K_BYTES;
            }

            numPages = newNumPages;
            if (numPages == 0)
                dynArray = nullptr;
        }

    private:
        T *dynArray;
        int numPages;
        int size;

    public:
        Vector() {init();}
        ~Vector() {clear();}

        Vector(const Vector<T>& ) = delete;
        Vector<T>& operator = (const Vector<T>& ) = delete;
    };
}



#endif /* VECTOR_H_ */
