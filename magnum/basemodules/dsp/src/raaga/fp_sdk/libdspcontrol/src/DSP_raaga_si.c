/****************************************************************************
 *                Copyright (c) 2001 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <inttypes.h>
#  include <stdbool.h>

#  include "framework.h"    /* it includes required Magnum modules in the right order */
#else
#  include "bstd.h"
#  include "bkni.h"

/* Workaround for the missing inttypes.h */
#define PRIu16      "u"
#define PRIu32      "u"
#endif


#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "DSP_raaga_internal.h"



#if !IS_HOST(SILICON)
#  error "This module is for silicon targets only"
#endif
#if !defined(RAAGA)
#  error "This module is only for Raaga"
#endif
#if FEATURE_IS(ENDIANESS, BIG)
#  error "Big endian platforms not properly supported at the moment"
#endif



/* ----------------------------------------
 *  Internal functions - memory management
 * ---------------------------------------- */
uint32_t RAAGA_malloc_physical(DSP *dsp, size_t size)
{
    void *soft_p;
    uint32_t phys_p=0;

    soft_p = BMEM_AllocAligned(dsp->hMem, size, 2, 0);
    if(soft_p == NULL)
        FATAL_ERROR("RAAGA: cannot allocate %d bytes", size);

    if((BMEM_ConvertAddressToOffset(dsp->hMem, soft_p, &phys_p)) != BERR_SUCCESS)
    {
        BMEM_Heap_Free(dsp->hMem, soft_p);
        FATAL_ERROR("RAAGA: cannot convert soft %p to physical", soft_p);
    }
    else
    {
        DSPLOG_JUNK("RAAGA: allocated %d bytes soft = %p, physical = %#x", size, soft_p, phys_p);
    }

    return phys_p;
}


void RAAGA_free_physical(DSP *dsp, uint32_t phys_p)
{
    void *soft_p;

    BMEM_ConvertOffsetToAddress(dsp->hMem, phys_p, (void **) &soft_p);
    DSPLOG_JUNK("RAAGA: freeing memory block at soft = %p, physical = %#x", soft_p, phys_p);

    if(BMEM_Heap_Free(dsp->hMem, soft_p) != BERR_SUCCESS)
        FATAL_ERROR("RAAGA: cannot free memory block at soft = %p, physical = %#x", soft_p, phys_p);
}


/* ------------------------------------
 *  Internal functions - DMA transfers
 * ------------------------------------ */
/**
 * @return 1 if Q0 is busy, 0 if Q0 is free
 */
static int RAAGA_dma_busy_Q0(DSP *dsp)
{
    return BCHP_GET_FIELD_DATA(DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_DMA_STATUS),
                               RAAGA_DSP_DMA_STATUS,
                               Q0_BUSY);
}


/**
 * @param timeout_us number of microseconds after which to give up
 * @return 0 if the queue is now free, 1 on timeout
 */
static int RAAGA_wait_dma_completion_Q0(DSP *dsp, uint32_t timeout_us)
{
    const uint32_t delay_us = 10;
    uint32_t max_count = timeout_us / delay_us;
    uint32_t count = 0;

    while(RAAGA_dma_busy_Q0(dsp))
    {
        count++;
        if(count > max_count)
            return 1;

        BKNI_Delay(delay_us);   /* waits in us */
    }

    return 0;
}


static void RAAGA_issue_dma_transfer_Q0(DSP *dsp, uint32_t src, uint32_t dest, uint8_t data_type, uint8_t swap_type, uint16_t size)
{
    uint32_t reg_val = 0x0;

    DSPLOG_DEBUG("RAAGA: issuing a DMA transfer src=%#010x, dest=%#010x, dataType=%d, swapType=%d, size=%d (%#x) bytes",
                 src, dest, data_type, swap_type, size, size);

    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_DMA_SRC_ADDR_Q0, src);
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_DMA_DEST_ADDR_Q0, dest);

    reg_val = (reg_val & ~(BCHP_MASK(RAAGA_DSP_DMA_TRANSFER_Q0, SWAP_TYPE)))
             | (BCHP_FIELD_DATA(RAAGA_DSP_DMA_TRANSFER_Q0, SWAP_TYPE, swap_type));
    reg_val = (reg_val & ~(BCHP_MASK(RAAGA_DSP_DMA_TRANSFER_Q0, DATA_TYPE)))
             | (BCHP_FIELD_DATA(RAAGA_DSP_DMA_TRANSFER_Q0, DATA_TYPE, data_type));
    reg_val = (reg_val & ~(BCHP_MASK(RAAGA_DSP_DMA_TRANSFER_Q0, TRANSFER_TYPE)))
             | (BCHP_FIELD_ENUM(RAAGA_DSP_DMA_TRANSFER_Q0, TRANSFER_TYPE, DMA_READ));
    reg_val = (reg_val & ~(BCHP_MASK(RAAGA_DSP_DMA_TRANSFER_Q0, TRANSFER_SIZE)))
             | (BCHP_FIELD_DATA(RAAGA_DSP_DMA_TRANSFER_Q0, TRANSFER_SIZE, size));

    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_DMA_TRANSFER_Q0, reg_val);
}


/* ------------------
 *  Public interface
 * ------------------ */
DSP_RET DSP_init(DSP *dsp, DSP_PARAMETERS *parameters)
{
    /* Keep a copy of provided handles */
    dsp->hReg = parameters->hReg;
    dsp->hMem = parameters->hMem;
#if defined(__FP2012__)
    dsp->debug_console_initialised = false;
#endif

    return DSP_SUCCESS;
}


void DSP_finish(DSP *dsp __unused)
{
}


DSP_RET DSP_reset(DSP *dsp)
{
    uint32_t reg_val;

    DSPLOG_INFO("DSP: resetting DSP");

    reg_val = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    reg_val = (reg_val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B)))
             | (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B, 0));
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_MISC_SOFT_INIT, reg_val);

    return DSP_SUCCESS;
}


DSP_RET DSP_enable(DSP *dsp, DSP_CORE core)
{
    uint32_t reg_val;

    if(core >= DSP_numCores(dsp))
    {
        DSPLOG_ERROR("DSP: trying to enable a non-existent (#%d) core", core);
        return DSP_WRONG_CORE;
    }

    DSPLOG_INFO("DSP: enabling DSP core %d", core);

    reg_val = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    reg_val = (reg_val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B)))
             | (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B, 1));
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_MISC_SOFT_INIT, reg_val);

    return DSP_SUCCESS;
}


/*
 * Overlays are not currently supported on Raaga.
 */
const DSP_DRAM_MEMORY_AREA *
DSP_getOverlayDRAM(DSP *dsp    __unused,
                   size_t size __unused)
{
    FATAL_ERROR ("No overlay support on Raaga SI");
}


DSP_RET DSP_writeSystemData(DSP *dsp, uint32_t phys_dest, const void *source, size_t n)
{
    void *soft_dest;                /* software data address */
    BMEM_ConvertOffsetToAddress(dsp->hMem, phys_dest, (void **) &soft_dest);

    DSPLOG_JUNK("DSP: writeSystemData from = %p, to_phys = %#x, to_soft = %p, size = %"PRIu32,
                source, phys_dest, soft_dest, n);

    BKNI_Memcpy(soft_dest, source, n);

#if FEATURE_IS(ENDIANESS, BIG)
/*    for(uint32_t ui32cnt = 0; ui32cnt < sizeInDWords; ui32cnt++)
 *    {
 *        *soft_dest = htofp32(*source);  // copy data
 *        soft_dest++;                    // move write pointer
 *        source++;                       // move read pointer
 *    } */
#endif

    return DSP_SUCCESS;
}


DSP_RET DSP_readSystemData(DSP *dsp, void *dest, uint32_t phys_source, size_t n)
{
    void *soft_source;              /* software data address */
    BMEM_ConvertOffsetToAddress(dsp->hMem, phys_source, (void **) &soft_source);

    DSPLOG_JUNK("DSP: readSystemData from_phys = %#x, from_soft = %p, to = %p, size = %"PRIu32,
                phys_source, soft_source, dest, n);

    BKNI_Memcpy(dest, soft_source, n);

#if FEATURE_IS(ENDIANESS, BIG)
/*     for(uint32_t ui32cnt = 0; ui32cnt < sizeInDWords; ui32cnt++)
 *     {
 *         *dest = fptoh32(*soft_source);  // copy data
 *         soft_source++;                  // move read pointer
 *         dest++;                         // move write pointer
 *     } */
#endif

    return DSP_SUCCESS;
}


void DSP_writeSharedRegister(DSP *dsp, uint32_t reg_addr, uint32_t val)
{
    DSPLOG_JUNK("DSP: writeSharedRegister address = %#x, data = %#x", reg_addr, val);
    BREG_Write32(dsp->hReg, reg_addr, val);
}


uint32_t DSP_readSharedRegister(DSP *dsp, uint32_t reg_addr)
{
    uint32_t val = BREG_Read32(dsp->hReg, reg_addr);
    DSPLOG_JUNK("DSP: readSharedRegister address = %#x, data = %#x", reg_addr, val);

    return val;
}


DSP_RET DSP_readSharedData(DSP *dsp, void * local_addr, uint32_t src, size_t len)
{
    if(src & 3)
    {
        DSPLOG_ERROR("DSP_readSharedData: address must be 32 bit-aligned");
        return DSP_BAD_ADDRESS_ALIGNMENT;
    }

    if(len & 3)
    {
        DSPLOG_ERROR("DSP_readSharedData: length must be a whole number of 32 bit words");
        return DSP_BAD_SIZE;
    }

    DSPLOG_JUNK("DSP: readSharedData address = %#x, length = %lu", src, len);
    {
        unsigned i;
        for(i = 0; i < len; i += 4)
            *((uint32_t *) ((char *) local_addr + i)) = BREG_Read32(dsp->hReg, src + i);
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_writeSharedData(DSP *dsp, uint32_t dest, void * local_addr, size_t len)
{
    if(dest & 3)
    {
        DSPLOG_ERROR("DSP_writeSharedData: address must be 32 bit-aligned");
        return DSP_BAD_ADDRESS_ALIGNMENT;
    }

    if(len & 3)
    {
        DSPLOG_ERROR("DSP_writeSharedData: length must be a whole number of 32 bit words");
        return DSP_BAD_SIZE;
    }

    DSPLOG_JUNK("DSP: writeSharedData address = %#x, length = %lu", dest, len);
    {
        unsigned i;
        for(i = 0; i < len; i += 4)
            BREG_Write32(dsp->hReg, dest + i, *((uint32_t *) ((char *) local_addr + i)));
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_writeDspData(DSP *dsp, DSP_ADDR dest, const void *src, size_t n)
{
    const uint32_t dma_transfer_wait_us = 5000; /* 5000us comes from the previous code
                                                 * for(int count=500; count > 0; count--)
                                                 *     BKNI_Delay(10); */

    if(n == 0)
        return DSP_SUCCESS;

    /* check if the DMA queue is already busy (it should never be, but better check) */
    if(RAAGA_dma_busy_Q0(dsp))
    {
        DSPLOG_ERROR("DSP_writeDspData: trying to issue a DMA transfer while the channel Q0 is busy - waiting");
        if(RAAGA_wait_dma_completion_Q0(dsp, dma_transfer_wait_us))
        {
            DSPLOG_ERROR("DSP_writeDspData: DMA timeout");
            return DSP_FAILURE;
        }
    }

    /* DMA queue 0 is now available for sure */
    {
        uint32_t dram;
        uint8_t data_type = 2;
        uint8_t swap_type = 0;
        uint32_t cur_src;
        uint32_t cur_dest = dest;

        DSPLOG_DEBUG("DSP_writeDspData: copying data to DRAM");
        dram = RAAGA_malloc_physical(dsp, n);
        DSP_writeSystemData(dsp, dram, src, n);

        DSPLOG_DEBUG("DSP_writeDspData: moving data to the DSP through DMA Q0");
        cur_src = dram;
        while(n > 0)
        {
            uint16_t cur_transfer_size = n > UINT16_MAX ? UINT16_MAX : n;

            DSPLOG_DEBUG("DSP_writeDspData: issuing a DMA transfer on Q0 for %"PRIu16" bytes", cur_transfer_size);
            RAAGA_issue_dma_transfer_Q0(dsp, cur_src, cur_dest, data_type, swap_type, cur_transfer_size);

            DSPLOG_DEBUG("DSP_writeDspData: waiting for the DMA transfer to complete");
            if(RAAGA_wait_dma_completion_Q0(dsp, dma_transfer_wait_us))
            {
                DSPLOG_ERROR("DSP_writeDspData: DMA timeout");
                RAAGA_free_physical(dsp, dram);
                return DSP_FAILURE;
            }

            cur_src += cur_transfer_size;
            cur_dest += cur_transfer_size;
            n -= cur_transfer_size;
        }

        RAAGA_free_physical(dsp, dram);
    }

    return DSP_SUCCESS;
}

/**
 * Read a 32 bit word from IMEM/DMEM at the specified address.
 * The word_addr address should be word-aligned, no checks are performed.
 */
static uint32_t
DSP_readDspWord(DSP *dsp, uint32_t word_addr, uint32_t command)
{
    /* Clear any pending host interrupts */
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_INTH_HOST_CLEAR, 1 << BCHP_RAAGA_DSP_INTH_HOST_CLEAR_MEM_PEEK_POKE_SHIFT);
    /* Push in the address */
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_ADDR, word_addr);
    /* Push in the command */
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_CMD, command);
    /* Wait for the interrupt */
    while((DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_INTH_HOST_STATUS) &
           (1 << BCHP_RAAGA_DSP_INTH_HOST_STATUS_MEM_PEEK_POKE_SHIFT)) == 0)
        BKNI_Delay(10);

    return DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_ID2R_RDATA);
}

/* Examine IMEM/DMEM using the peek/poke mechanism */
DSP_RET DSP_readDspData(DSP *dsp, void *dest, DSP_ADDR src, size_t n)
{
    const uint32_t command = ((uint32_t) src >= RAAGA_DMEM_START) << BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_CMD_MEMSEL_SHIFT
                             | 1 << BCHP_RAAGA_DSP_MEM_SUBSYSTEM_R2ID_CMD_RD_WR_CMD_SHIFT;
    uint8_t *char_dest;
    uint32_t offset;
    uint32_t *word_dest;

    /* Handle a misaligned start source address */
    char_dest = dest;
    offset = src & 3;
    if(offset != 0)
    {
        uint32_t mis_size = MIN(n, 4 - offset);
        uint32_t word = DSP_readDspWord(dsp, src & ~3, command);
        BKNI_Memcpy(dest, ((char *) &word) + offset, mis_size);   /* overkill but avoids a lot of shifts & masks */

        /* Move our starting point */
        src       += mis_size;
        char_dest += mis_size;
        n         -= mis_size;
    }

    /* Handle the aligned and word-multiple part of the data transfer
     * (src can still be misaligned, but in that case n == 0) */
    word_dest = (uint32_t *) char_dest;
    while(n >= 4)
    {
        *word_dest = DSP_readDspWord(dsp, src, command);

        word_dest++;
        src += 4;
        n -= 4;
    }

    /* Handle the non-word-multiple last part of the transfer */
    if(n > 0)
    {
        uint32_t word = DSP_readDspWord(dsp, src, command);
        BKNI_Memcpy(word_dest, &word, n);    /* again overkill but still clearer than shifting & masking */
    }

    return DSP_SUCCESS;
}


DSP_RET DSP_writeDataAsDsp(DSP *dsp, DSP_ADDR dest, void *src, size_t len)
{
    unsigned long ulbase_addr = (unsigned long)dsp->hReg->BaseAddr;
    uint32_t base_addr = (uint32_t)ulbase_addr;

    if(isSmem(dest, len) || isDmem(dest, len))
        /* DSP memories */
        return DSP_writeDspData(dsp, dest, src, len);
    else if((dest & 0xF0000000) == 0x80000000)
    {
        /* Registers, mapped as they would be seen from the Firepath core; translate
         * dest into a register index (similar to the constants from the RDB) before
         * passing it to DSP_writeSharedData. The conversion is a bit hacky as apparently
         * offsets in the 0x80000000 memory window don't match offsets in the system-wide
         * memory window. E.g.
         * BCHP_RAAGA_DSP_INTH_HOST_STATUS = 0x00c22200 in Raaga si BCM97425 RDB
         * BCHP_RAAGA_DSP_INTH_HOST_STATUS = 0x80022200 in FPSDK memmap-raaga.h
         *
         * The first register, BCHP_RAAGA_DSP_RGR_REVISION (= 0x00c00000 on BCM97425),
         * will give us an hint of how to build the target register index.
         */
        uint32_t reg_idx = (dest & 0x000FFFFF) + BCHP_RAAGA_DSP_RGR_REVISION;
        return DSP_writeSharedData(dsp, reg_idx, src, len);
    }
    else if(dest >= base_addr &&
            dest <= base_addr + dsp->hReg->MaxRegOffset)
    {
        /* Register, mapped as they would be seen from the host processor. */
        /* Extract the register index and pass it to DSP_writeSharedData. */
        uint32_t reg_idx = dest - base_addr;
        return DSP_writeSharedData(dsp, reg_idx, src, len);
    }
    else
    {
        DSPLOG_ERROR("DSP_writeDataAsDsp: requested address range %#010x, length %#x "
                     "matches neither DSP memories nor a Raaga register", dest, len);
        return DSP_BAD_ADDRESS_RANGE;
    }
}


DSP_RET DSP_readDataAsDsp(DSP *dsp, void *dest, DSP_ADDR src, size_t len)
{
    unsigned long ulbase_addr = (unsigned long)dsp->hReg->BaseAddr;
    uint32_t base_addr = (uint32_t)ulbase_addr;


    if(isSmem(src, len) || isDmem(src, len))
        /* DSP memories */
        return DSP_readDspData(dsp, dest, src, len);
    else if((src & 0xF0000000) == 0x80000000)
    {
        /* see above (DSP_writeDataAsDsp) for an explanation of the src conversion */
        uint32_t reg_idx = (src & 0x000FFFFF) + BCHP_RAAGA_DSP_RGR_REVISION;
        return DSP_readSharedData(dsp, dest, reg_idx, len);
    }
    else if(src >= base_addr &&
            src <= base_addr + dsp->hReg->MaxRegOffset)
    {
        /* see above (DSP_writeDataAsDsp) for an explanation of the src conversion */
        uint32_t reg_idx = src - base_addr;
        return DSP_readSharedData(dsp, dest, reg_idx, len);
    }
    else
    {
        DSPLOG_ERROR("DSP_readDataAsDsp: requested address range %#010x, length %#x "
                     "doesn't match DSP memories nor a Raaga register", src, len);
        return DSP_BAD_ADDRESS_RANGE;
    }
}
