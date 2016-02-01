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
#  include <stdint.h>
#  include <inttypes.h>
#else
#  include "bstd_defs.h"

/* Workaround for the missing inttypes.h */
#define PRIx32      "x"
#define PRIu32      "u"
#endif

#include "fp_sdk_config.h"

#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "libsyschip/src/heartbeat_internal.h"

#include "DSP_raaga_internal.h"



#if IS_HOST(DSP_LESS)
#  error "This module is not suitable for DSP-less builds"
#endif
#if !defined(RAAGA)
#  error "This module is only for Raaga"
#endif


void DSP_pollInterrupts(DSP *dsp, DSP_INTERRUPTS *interrupts)
{
    interrupts->dsp_inth_host_status = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_INTH_HOST_STATUS);
#if !(defined(__FP2012_ONWARDS__) && IS_HOST(BM))  /* BM doesn't support in full the rev2000 RDB */
    interrupts->dsp_fw_inth_host_status = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_FW_INTH_HOST_STATUS);
#else
    interrupts->dsp_fw_inth_host_status = 0;
#endif
#if IS_HOST(SILICON)
    interrupts->dsp_mem_subsystem_memsub_error_status = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_STATUS);
#else
    interrupts->dsp_mem_subsystem_memsub_error_status = 0;
#endif
}


#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)      /* don't interfere with Magnum about interrupts handling */

void DSP_clearInterrupts(DSP *dsp, DSP_INTERRUPTS *interrupts)
{
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_INTH_HOST_CLEAR, interrupts->dsp_inth_host_status);
#if !(defined(__FP2012_ONWARDS__) && IS_HOST(BM))  /* BM doesn't support in full the rev2000 RDB */
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_FW_INTH_HOST_CLEAR, interrupts->dsp_fw_inth_host_status);
#endif
#if IS_HOST(SILICON)
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_CLEAR, interrupts->dsp_mem_subsystem_memsub_error_status);
#endif
}


void DSP_clearAllInterrupts(DSP *dsp)
{
    DSP_INTERRUPTS all_interrupts = { 0xffffffff, 0xffffffff, 0xffffffff };
    DSP_clearInterrupts(dsp, &all_interrupts);
}

#endif /* !FEATURE_IS(SW_HOST, RAAGA_MAGNUM) */


void DSP_pollHeartbeat(DSP *dsp, DSP_HEARTBEAT *heartbeat)
{
    /* Look for the ["Done", exit_code] combination and fake the heartbeat status */
    uint32_t exit_data[2];
    DSP_readSharedData(dsp, &exit_data, BCHP_RAAGA_DSP_MISC_SCRATCH_2, sizeof(exit_data));
    if(exit_data[0] == 0x656e6f44)  /* "Done" */
    {
        heartbeat->phase = HB_PHASE_THE_END;
        heartbeat->subphase = HB_SUBPHASE_TEST_EXIT_WAIT_FOR_THE_END;
        heartbeat->argument = exit_data[1];
    }
    else
    {
        /* DSP_enabledStatus is not available on Raaga Si,
         * so let's pretend we are running. */
        /* FIXME: implement DSP_enabledStatus */
        heartbeat->phase = HB_PHASE_MAIN;
        heartbeat->subphase = HB_SUBPHASE_ENTERING_MAIN;
        heartbeat->argument = 0;
    }
}


void DSP_clearHeartbeat(DSP *dsp)
{
    uint32_t exit_data[2] = {0, 0};
    DSP_writeSharedData(dsp, BCHP_RAAGA_DSP_MISC_SCRATCH_2, &exit_data, sizeof(exit_data));
}


/*
 * Memory layout.
 */
const DSP_MEMORY_LAYOUT *DSP_getMemoryLayout(DSP *dsp __unused)
{
    static const DSP_MEMORY_AREA areas[] =
    {
        {
            DSP_MEMORY_TYPE_FP_SMEM,                                /* .type */
            DSP_MEMORY_CAN_READ | DSP_MEMORY_CAN_READ_SLOW |
            DSP_MEMORY_CAN_WRITE | DSP_MEMORY_CAN_WRITE_INDIRECT,   /* .flags */
            {
                4,  /* read_alignment */
                4,  /* read_size */
                1,  /* write_alignment */
                1   /* write_size */
            },                              /* constraints */
            DSP_ADDRSPACE_DSP,              /* addressSpace */
            0x00000000,                     /* startAddress */
#ifdef __FP2008__
            128 * 1024,                     /* length */
#else
            192 * 1024,                     /* length */
#endif
            0x00000000                      /* dspAddress */
        },
        {
            DSP_MEMORY_TYPE_FP_DMEM,                                /* type */
            DSP_MEMORY_CAN_READ | DSP_MEMORY_CAN_READ_SLOW |
            DSP_MEMORY_CAN_WRITE | DSP_MEMORY_CAN_WRITE_INDIRECT,   /* flags */
            {
                4,  /* read_alignment */
                4,  /* read_size */
                1,  /* write_alignment */
                1   /* write_size */
            },                              /* constraints */
            DSP_ADDRSPACE_DSP,              /* addressSpace */
            0x40000000,                     /* startAddress */
#ifdef __FP2008__
            96 * 1024,                      /* length */
#else
            128 * 1024,                     /* length */
#endif
            0x40000000                      /* dspAddress */
        },
        {
            DSP_MEMORY_TYPE_SHARED,                        /* type */
            DSP_MEMORY_CAN_READ | DSP_MEMORY_CAN_WRITE,    /* flags */
            {
                4,  /* read_alignment */
                4,  /* read_size */
                4,  /* write_alignment */
                4   /* write_size */
            },                              /* constraints */
            DSP_ADDRSPACE_SHARED,           /* addressSpace */
            0x00000000,                     /* startAddress */
            0x00FFFD98,                     /* length */
            DSP_MEMORY_AREA_NO_DSP_ACCESS   /* dspAddress */
        }
    };

    static const DSP_MEMORY_LAYOUT layout =
    {
        sizeof (areas) / sizeof (DSP_MEMORY_AREA),
        areas
    };

    return &layout;
}


/*
 * Debug console interface.
 */
#ifdef __FP2012__
static bool DSP_checkBuffersInitialised(DSP *dsp)
{
    if(!dsp->debug_console_initialised)
    {
        uint32_t write_index_rx, write_index_tx;

        /* Spin until the buffers appear to be set up correctly. */
        write_index_rx = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_WRITE_INDEX);
        write_index_tx = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_WRITE_INDEX);

        DSPLOG_JUNK("DBG(raaga): Write index (TX) = %d, Write index (RX) = %d\n",
                    write_index_tx, write_index_rx);

        if(write_index_rx == 1 && write_index_tx != 0)
        {
            dsp->debug_console_initialised = true;
            return true;
        }

        return false;
    }

    return true;
}
#endif


#ifdef __FP2012__
/* Raise an IRQ on the FirePath to indicate that bytes have been send, or
   that bytes have been consumed.  */
static void DSP_raiseIRQ(DSP *dsp, int tx, int rx)
{
    /* We only have a single IRQ that we can raise, so just raise it now,
     regardless of what type was requested.  To raise the IRQ write a 1
     into bit-0 (regardless of current value of the bit).  */
    uint32_t irq_value = (uint32_t) -1;
    DSP_writeSharedRegister (dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_IRQ, irq_value);
    DSPLOG_JUNK("DBG(raaga): Raise a debug console IRQ.\n");

    /* The following two lines are simply to
     silence compiler warnings about unused variables.  */
    (void) tx;
    (void) rx;
}
#else
  /* Interrupts are raised automatically on older platforms thanks to the
     debug console hardware. */
#endif


bool DSP_debugConsoleDataAvailable(DSP *dsp, DSP_CORE core __unused)
{
#ifdef __FP2012__
    uint32_t read_index, write_index;

    if (!DSP_checkBuffersInitialised(dsp))
      return false;

    /* Is there data available from the FirePath?  */
    read_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_READ_INDEX);
    write_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_WRITE_INDEX);

    read_index++;
    if (read_index == RAAGA_DSP_FP2012_DBG_CONSOLE_TX_BUFF_SIZE)
      read_index = 0;

    return (read_index != write_index);
#else
    /* All pre-fp2012 raaga systems. */
    uint32_t reg = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_STATUS);
    return (reg & BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_STATUS_RD_DATA_RDY_MASK) != 0;
#endif
}


bool DSP_debugConsoleSpace(DSP *dsp, DSP_CORE core __unused)
{
#ifdef __FP2012__
    uint32_t read_index, write_index;

    if (!DSP_checkBuffersInitialised (dsp))
      return false;

    /* Is there space to send data to the FirePath?  */
    read_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_READ_INDEX);
    write_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_WRITE_INDEX);

    return (read_index != write_index);
#else
    /* All pre-fp2012 raaga systems. */
    uint32_t reg = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_STATUS);
    return (reg & BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_STATUS_WR_DATA_ACCEPT_MASK) != 0;
#endif /* __FP2012__ */
}


DSP_RET DSP_debugConsoleRead(DSP *dsp, DSP_CORE core __unused, uint8_t *dst)
{
#ifdef __FP2012__
    uint32_t read_index, write_index;

    if (!DSP_checkBuffersInitialised (dsp))
    {
        DSPLOG_ERROR("DSP: CheckBuffersInitialised returns FALSE");
        return DSP_DEBUG_CONSOLE_NOT_INITIALISED;
    }

    /* Is there data available from the FirePath?  */
    read_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_READ_INDEX);
    write_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_WRITE_INDEX);

    read_index++;
    if (read_index == RAAGA_DSP_FP2012_DBG_CONSOLE_TX_BUFF_SIZE)
      read_index = 0;

    if (read_index != write_index)
    {
        uint32_t data;
        data = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_BUFF + (read_index * 4));
        DSPLOG_JUNK("DSP: debug console read data %"PRIx32
                    " at index %"PRIu32, data, read_index);
        *dst = (uint8_t) data;

        /* Update read index.  */
        DSP_writeSharedRegister (dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_READ_INDEX, read_index);
    }
    else
    {
        DSPLOG_JUNK("DSP: no data found in debug console while trying to read");
        return DSP_DEBUG_CONSOLE_NO_DATA;
    }

    DSPLOG_JUNK("DSP: debug console: read index = %d, write index = %d, buffer addr = %p, buffer size = %d\n",
                read_index, write_index, RAAGA_DSP_FP2012_DBG_CONSOLE_TX_BUFF,
                RAAGA_DSP_FP2012_DBG_CONSOLE_TX_BUFF_SIZE);

    DSP_raiseIRQ (dsp, 0, 1);

    return DSP_SUCCESS;
#else
    /* All pre-fp2012 raaga systems. */
    if(DSP_debugConsoleDataAvailable(dsp, core))
    {
        uint32_t reg = DSP_readSharedRegister(dsp, BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_RD_DATA);
        DSP_writeSharedRegister(dsp,
                                BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_CTRL,
                                BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_CTRL_RD_DATA_DONE_MASK);
        *dst = (uint8_t) reg;
        return DSP_SUCCESS;
    }
    else
        return DSP_DEBUG_CONSOLE_NO_DATA;
#endif /* __FP2012__ */
}


DSP_RET DSP_debugConsoleWrite(DSP *dsp, DSP_CORE core __unused, uint8_t src)
{
#ifdef __FP2012__
    uint32_t read_index, write_index;

    if (!DSP_checkBuffersInitialised (dsp))
    {
        DSPLOG_ERROR("DSP: CheckBuffersInitialised returns FALSE");
        return DSP_DEBUG_CONSOLE_NOT_INITIALISED;
    }

    /* Is there data available from the FirePath?  */
    read_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_READ_INDEX);
    write_index = DSP_readSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_WRITE_INDEX);

    if (read_index != write_index)
      {
        uint32_t data;

        data = (uint32_t) src;
        DSPLOG_JUNK("DSP: debug console sent data %"PRIx32, data);
        DSP_writeSharedRegister(dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_BUFF + (write_index * 4), data);

        /* Update write index. */
        write_index++;
        if (write_index == RAAGA_DSP_FP2012_DBG_CONSOLE_RX_BUFF_SIZE)
          write_index = 0;
        DSP_writeSharedRegister (dsp, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_WRITE_INDEX, write_index);
      }
    else
    {
        DSPLOG_JUNK("DSP: no space in debug console while trying to write");
        return DSP_DEBUG_CONSOLE_NO_SPACE;
    }

    DSPLOG_JUNK("DSP: debug console: read index = %d, write index = %d, buffer addr = %p, buffer size = %d",
                read_index, write_index, RAAGA_DSP_FP2012_DBG_CONSOLE_RX_BUFF,
                RAAGA_DSP_FP2012_DBG_CONSOLE_RX_BUFF_SIZE);

    DSP_raiseIRQ (dsp, 1, 0);

    return 0;
#else
    /* All pre-fp2012 raaga systems.  */
    DSP_writeSharedRegister(dsp, BCHP_RAAGA_DSP_PERI_DBG_CTRL_DEBUG_CONSOLE_WR_DATA, src);

    return 0;
#endif /* __FP2012__ */
}
