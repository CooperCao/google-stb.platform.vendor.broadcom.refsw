/******************************************************************************
* Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bstd.h"
#include "bchp_common.h"
#include "bkni.h"
#include "bchp_sun_top_ctrl.h"

#if !defined(__KERNEL__)
#include <sys/mman.h>
#include <fcntl.h>
#endif

#include "breg_mem_priv.h"
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

static uint32_t breg_Read32(BREG_Handle reg, uint32_t addr);

static BERR_Code REG_open(struct BREG_Impl *reg)
{
    uint64_t register_base = BCHP_PHYSICAL_OFFSET + ((BCHP_REGISTER_START / 4096) * 4096);
    size_t register_size = BCHP_REGISTER_END - ((BCHP_REGISTER_START / 4096) * 4096);
    int fd;

    BKNI_Memset(reg, 0, sizeof(*reg));
#if defined(__KERNEL__)
    reg->BaseAddr = ioremap_nocache(BCHP_PHYSICAL_OFFSET, BCHP_REGISTER_END);
    if(reg->BaseAddr==NULL) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#else
    fd = open("/dev/mem", O_RDWR|O_SYNC);
    BDBG_ASSERT(fd>=0);
    reg->BaseAddr = mmap(0, register_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, register_base);
    if (reg->BaseAddr == MAP_FAILED) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#endif
#if defined(BCHP_SUN_TOP_CTRL_PRODUCT_ID)
    {
        uint32_t data;

        BDBG_MSG(("Reading BCHP_SUN_TOP_CTRL_PRODUCT_ID"));
        data = breg_Read32(reg, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
        BDBG_MSG(("BCHP_SUN_TOP_CTRL_PRODUCT_ID=%#x", (unsigned)data));
    }
#endif
    reg->MaxRegOffset = fd;
    return BERR_SUCCESS;
}

static void REG_close(BREG_Impl *reg)
{
    int fd = reg->MaxRegOffset;
    size_t register_size = BCHP_REGISTER_END - ((BCHP_REGISTER_START / 4096) * 4096);
#if defined(__KERNEL__)
    iounmap(reg->BaseAddr);
#else
    munmap(reg->BaseAddr, register_size);
    BKNI_Memset(reg, 0, sizeof(*reg));
    close(fd);
#endif
    return;
}

static void breg_Write32(BREG_Handle reg, uint32_t addr, uint32_t data)
{
    BREG_P_Write32(reg, addr, data);
    return;
}

static void breg_Write64(BREG_Handle reg, uint32_t addr, uint64_t data)
{
    BREG_P_Write64(reg, addr, data);
    return;
}

static uint32_t breg_Read32(BREG_Handle reg, uint32_t addr)
{
    uint32_t data;
    data = BREG_P_Read32(reg, addr);
    return data;
}

static uint64_t breg_Read64(BREG_Handle reg, uint32_t addr)
{
    uint64_t data;
    data = BREG_P_Read64(reg, addr);
    return data;
}

#undef BREG_Write64
#undef BREG_Read64
#define BREG_Write64 breg_Write64
#define BREG_Write32 breg_Write32
#define BREG_Read64 breg_Read64
#define BREG_Read32 breg_Read32
