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
    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
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

#ifdef BCHP_MEMC_GEN_0_REG_START
#include "bchp_memc_gen_0.h"
#else
#define BCHP_MEMC_GEN_0_MSA_RD_DATA0 0
#define BCHP_MEMC_GEN_0_MSA_WR_DATA0 0
#endif


struct msa_jw {
    uint32_t data[8];
};

struct msa_data {
    BSTD_DeviceOffset base; /* address of MSA transaction */
    unsigned bytes; /* number of bytes in MSA transaction */
    uint8_t align_head; /* number of bytes from start of MSA transaction to the user requested address */
    uint8_t align_tail; /* number of bytes from end of user transaction to end of MSA transaction */
    struct msa_jw jw[2];
};

static void msa_P_WriteCommand(BREG_Handle reg, unsigned offset, unsigned cmd, BSTD_DeviceOffset addr)
{
#ifdef BCHP_MEMC_GEN_0_REG_START
    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_CMD_TYPE+offset, cmd);
#if defined(BCHP_MEMC_GEN_0_MSA_CMD_TRIGGER)
    BREG_Write64(reg, BCHP_MEMC_GEN_0_MSA_CMD_ADDR+offset, addr/8);
    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_CMD_TRIGGER+offset, BCHP_FIELD_DATA(MEMC_GEN_0_MSA_CMD_TRIGGER,REQUEST,1));
#else
    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_CMD_ADDR+offset, addr/8);
#endif
    for(;;) {
        uint32_t data = BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_STATUS + offset);
        if(BCHP_GET_FIELD_DATA(data,MEMC_GEN_0_MSA_STATUS,BUSY)==0) {
            break;
        }
    }
#else
    BSTD_UNUSED(msa);
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(addr);
#endif
    return;
}

static BERR_Code msa_GetOffset(unsigned memc, unsigned *offset)
{
    switch(memc) {
#ifdef BCHP_MEMC_GEN_0_REG_START
    case 0: *offset = 0; break;
#endif
#ifdef BCHP_MEMC_GEN_1_REG_START
    case 1: *offset = BCHP_MEMC_GEN_1_REG_START - BCHP_MEMC_GEN_0_REG_START; break;
#endif
#ifdef BCHP_MEMC_GEN_2_REG_START
    case 2: *offset = BCHP_MEMC_GEN_2_REG_START - BCHP_MEMC_GEN_0_REG_START; break;
#endif
    default:
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BERR_SUCCESS;
}

static void msa_P_Enable(BREG_Handle reg, unsigned offset)
{
    uint32_t data;
    BSTD_UNUSED(reg);
    BSTD_UNUSED(offset);
    BSTD_UNUSED(data);
#if defined(BCHP_MEMC_GEN_0_MSA_MODE_CLOCK_GATE_MASK)
    data = BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_MODE + offset);
    if( BCHP_GET_FIELD_DATA(data, MEMC_GEN_0_MSA_MODE, CLOCK_GATE) != 0) {
        BCHP_SET_FIELD_CONST_DATA(data, MEMC_GEN_0_MSA_MODE, CLOCK_GATE, 0);
        BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_MODE + offset, data);
    }
#endif
    return ;
}

static void msa_P_Release(BREG_Handle reg, unsigned offset)
{
    uint32_t data = BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_STATUS + offset);
    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_STATUS + offset, data);
    return;
}

static BERR_Code msa_P_Acquire(BREG_Handle reg, unsigned offset)
{
    unsigned i;
    for(i=0;i<100;i++) {
        uint32_t data = BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_STATUS + offset);
        if(BCHP_GET_FIELD_DATA(data,MEMC_GEN_0_MSA_STATUS,T_LOCK)==0) {
            return BERR_SUCCESS;
        }
        BKNI_Delay(100);
    }
    return BERR_TRACE(BERR_NOT_AVAILABLE);
}

static BERR_Code msa_Reset(BREG_Handle reg, unsigned memc)
{
    BERR_Code rc;
    unsigned offset;

    rc = msa_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }

    msa_P_Enable(reg, offset);

    rc = msa_P_Acquire(reg, offset);
    if(rc!=BERR_SUCCESS) {
        BDBG_ERR(("Can't acquire access to MSA, try force release"));
        msa_P_Release(reg, offset);
        rc = msa_P_Acquire(reg, offset);
        if(rc!=BERR_SUCCESS) {
            return BERR_TRACE(rc);
        }
    }
    msa_P_Release(reg, offset);
    return BERR_SUCCESS;
}

static BERR_Code msa_P_PrepareBuffer(BSTD_DeviceOffset addr, unsigned bytes, struct msa_data *msa_data)
{
    unsigned msa_align_bytes;
    BSTD_DeviceOffset msa_addr;

    msa_align_bytes = (addr % sizeof(msa_data->jw[0].data));
    msa_addr = addr - msa_align_bytes;
    if(bytes + msa_align_bytes > sizeof(msa_data->jw)) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    msa_data->base = msa_addr;
    msa_data->bytes = ((bytes + msa_align_bytes + sizeof(msa_data->jw[0]) - 1) / sizeof(msa_data->jw[0])) * sizeof(msa_data->jw[0]);
    msa_data->align_head = msa_align_bytes;
    msa_data->align_tail = msa_data->bytes - (msa_align_bytes + bytes);
    return BERR_SUCCESS;
}

#define MSA_CMD_LINEAR_1JW_READ  0x01
#define MSA_CMD_LINEAR_1JW_WRITE 0x21
static BERR_Code msa_ExecuteRead(BREG_Handle reg, unsigned memc, BSTD_DeviceOffset addr, unsigned bytes, struct msa_data *msa_data)
{
    unsigned offset;
    BERR_Code rc;
    unsigned i;
    BSTD_DeviceOffset msa_addr;

    rc = msa_P_PrepareBuffer(addr, bytes, msa_data);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }

    rc = msa_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }

    rc = msa_P_Acquire(reg, offset);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }

    msa_addr = msa_data->base;
    for(i=0;i<sizeof(msa_data->jw)/sizeof(msa_data->jw[0]);i++) {
        unsigned j;
        msa_P_WriteCommand(reg, offset, MSA_CMD_LINEAR_1JW_READ, msa_addr);
        for(j=0;j<sizeof(msa_data->jw[i].data)/sizeof(msa_data->jw[i].data[0]);j++) {
            msa_data->jw[i].data[j] = BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_RD_DATA0 + offset + j*sizeof(uint32_t));
        }
        msa_addr +=  sizeof(msa_data->jw[i]);
        if(msa_addr >= addr+bytes) {
            break;
        }
    }
    msa_P_Release(reg, offset);
    return BERR_SUCCESS;
}

static uint32_t msa_Fetch32(BSTD_DeviceOffset addr, const struct msa_data *msa_data)
{
    unsigned offset;
    unsigned bank;

    BDBG_ASSERT(addr%sizeof(uint32_t)==0);

    if(addr < msa_data->base || addr >= msa_data->base + msa_data->bytes) {
        BDBG_ASSERT(0);
    }
    offset = addr - msa_data->base;
    bank = offset / sizeof(msa_data->jw[0].data);
    offset = offset % sizeof(msa_data->jw[0].data);
    offset = offset / sizeof(uint32_t);
    offset = (sizeof(msa_data->jw[0].data)/sizeof(uint32_t) - 1) - offset;
    BDBG_MSG(("bank:%u offset:%u", bank, offset));
    return msa_data->jw[bank].data[offset];
}

static uint8_t msa_Fetch8(BSTD_DeviceOffset addr, const struct msa_data *msa_data)
{
    uint32_t data;
    unsigned align_bytes = addr % sizeof(uint32_t);
    unsigned byte_no = align_bytes;
    data = msa_Fetch32(addr - align_bytes, msa_data);
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    byte_no = 3 - align_bytes;
#endif
    return data >> (8 * byte_no);
}

static BERR_Code msa_PrepareWrite(BSTD_DeviceOffset addr, unsigned bytes, struct msa_data *msa_data)
{
    BERR_Code rc;

    rc = msa_P_PrepareBuffer(addr, bytes, msa_data);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return BERR_SUCCESS;
}

static void msa_P_Store32(BSTD_DeviceOffset addr, uint32_t data, struct msa_data *msa_data)
{
    unsigned offset;
    unsigned bank;

    BDBG_ASSERT(addr%sizeof(uint32_t)==0);

    offset = addr - msa_data->base;
    bank = offset / sizeof(msa_data->jw[0].data);
    offset = offset % sizeof(msa_data->jw[0].data);
    offset = offset / sizeof(uint32_t);
    offset = (sizeof(msa_data->jw[0].data)/sizeof(uint32_t) - 1) - offset;
    msa_data->jw[bank].data[offset] = data;
    return;
}

static void msa_Store32(BSTD_DeviceOffset addr, uint32_t data, struct msa_data *msa_data)
{
    if(addr < msa_data->base + msa_data->align_head || addr + (sizeof(uint32_t) -1 ) >= msa_data->base + (msa_data->bytes - msa_data->align_tail)) {
        BDBG_ASSERT(0);
    }
    msa_P_Store32(addr, data, msa_data);
    return;
}

static void msa_Store8(BSTD_DeviceOffset addr, uint8_t data, struct msa_data *msa_data)
{
    uint32_t word;
    uint32_t mask;
    unsigned shift;
    unsigned align_bytes = addr % sizeof(uint32_t);
    unsigned byte_no = align_bytes;

    if(addr < msa_data->base + msa_data->align_head || addr >= msa_data->base + (msa_data->bytes - msa_data->align_tail)) {
        BDBG_ASSERT(0);
    }
    word = msa_Fetch32(addr - align_bytes, msa_data);
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    byte_no = 3 - align_bytes;
#endif
    shift = 8 * byte_no;
    mask = 0xFF << shift;
    word = word & (~mask);
    word = word | (((uint32_t)data) << shift);
    msa_P_Store32(addr - align_bytes, word, msa_data);
    return;
}

static BERR_Code msa_ExecuteWrite(BREG_Handle reg, unsigned memc, const struct msa_data *msa_data)
{
    unsigned i;
    unsigned offset;
    BERR_Code rc;
    unsigned block;
    BSTD_DeviceOffset msa_addr;

    rc = msa_GetOffset(memc, &offset);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }

    rc = msa_P_Acquire(reg, offset);
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }
    msa_addr = msa_data->base;

    for(i=0;i<sizeof(msa_data->jw)/sizeof(msa_data->jw[0]);i++) {
        unsigned j;
        uint32_t mask = 0;

        for(j=0;j<sizeof(msa_data->jw[i].data);j++) {
            unsigned byte = i*sizeof(msa_data->jw[i].data) + j;
            if( byte < msa_data->align_head || byte >= msa_data->bytes - msa_data->align_tail) {
                unsigned word = j / 4;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
                unsigned sub_byte = j % 4;
#else
                unsigned sub_byte = 3 - (j % 4);
#endif
                unsigned pos = word * 4 + sub_byte;
                uint32_t bit = 1ul << (sizeof(msa_data->jw[i].data) - 1 - pos);
                mask |= bit;
            }
        }
        BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_DQM + offset, mask);

        for(j=0;j<sizeof(msa_data->jw[i].data)/sizeof(msa_data->jw[i].data[0]);j++) {
            BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA0 + offset + j*sizeof(uint32_t), msa_data->jw[i].data[j]);
        }
        msa_P_WriteCommand(reg, offset, MSA_CMD_LINEAR_1JW_WRITE, msa_addr);
        msa_addr +=  sizeof(msa_data->jw[i]);
        if(msa_addr >= msa_data->base+msa_data->bytes) {
            break;
        }
    }
    msa_P_Release(reg, offset);
    return BERR_SUCCESS;
}
