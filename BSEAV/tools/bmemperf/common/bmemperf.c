/******************************************************************************
 *    (c)2013-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELYn
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "bmemperf.h"
#include "bmemperf_info.h"
#include "bmemperf_utils.h"

#if BCHP_MEMC_GEN_0_REG_START
#include "bchp_memc_gen_0.h"
#endif
#if BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_REG_START
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#endif

#define true  1
#define false 0

static unsigned long int unique_addr[10];
static unsigned long int unique_addr_count = 0;
int add_to_unique(
    unsigned long int *addr
    )
{
    unsigned int      idx;
    unsigned long int mask = (long unsigned int)addr&(long unsigned int)~4095UL;

    for (idx = 0; idx<unique_addr_count; idx++)
    {
        if (unique_addr[idx] == mask)
        {
            /*printf("%lx match at idx %u (%lx)\n", addr, idx, unique_addr[idx]);*/
            break;
        }
    }

    /* if a match was not found, add this to the end */
    if (idx == unique_addr_count)
    {
        #if 0
        printf( "%lx added to unique_addr at idx %u\n", mask, idx );
        #endif
        unique_addr[idx] = mask;
        unique_addr_count++;
    }

    return( 0 );
} /* add_to_unique */

/**TODO : Need to add in the init code ***/
/*volatile unsigned int g_client_id ;*/
extern bmemperf_info g_bmemperf_info;

extern unsigned int g_ddr_clock_frequency;

unsigned int        g_timer_Count_inDDR_cycles = 0;
extern unsigned int g_memc_max_min_mode;

extern unsigned int           g_interval;   /**g_interval is in msec unit*/
volatile unsigned int         g_clock_time; /**1msec in scb frequency , since time is in msec unit**/
int                           g_memFd;
static volatile unsigned int *g_pMem = NULL;

unsigned int g_memc_arb_reg_offset[BMEMPERF_NUM_MEMC];
unsigned int g_memc_ddr_stat_reg_offset[BMEMPERF_NUM_MEMC];
unsigned int g_memc_ddr_reg_offset[BMEMPERF_NUM_MEMC];
unsigned int g_memc_ddr_stat_cas_client_offset[BMEMPERF_NUM_MEMC];

volatile unsigned int g_refresh_delay[BMEMPERF_NUM_MEMC];

bmemperf_data bmemperfData[BMEMPERF_NUM_MEMC];

unsigned int bmemperf_read_ddr_stat_value(
    volatile unsigned int *g_pMem,
    unsigned int           reg,
    unsigned int           memc_index
    )
{
    volatile long unsigned int *pMemTemp;

    pMemTemp  = (unsigned long int *) g_pMem;
    pMemTemp += (( g_memc_ddr_stat_reg_offset[memc_index] + reg )>>2 );
    add_to_unique((long unsigned int *) pMemTemp );

    return( *pMemTemp );
}

void    bmemperf_write_ddr_stat_value(
    volatile unsigned int *g_pMem,
    unsigned int           reg,
    unsigned int           value,
    unsigned int           memc_index
    )
{
    volatile long unsigned int *pMemTemp;

    pMemTemp  = (unsigned long int *) g_pMem;
    pMemTemp += (( g_memc_ddr_stat_reg_offset[memc_index] + reg )>>2 );
    add_to_unique((long unsigned int *) pMemTemp );
    *pMemTemp = value;
}

unsigned int bmemperf_read_ddr_stat_client_cas_value(
    volatile unsigned int *g_pMem,
    unsigned int           reg,
    unsigned int           memc_index
    )
{
    volatile long unsigned int *pMemTemp;

    pMemTemp  = (unsigned long int *) g_pMem;
    pMemTemp += (( g_memc_ddr_stat_cas_client_offset[memc_index] + reg )>>2 );
    add_to_unique((long unsigned int *) pMemTemp );
    return( *pMemTemp );
}

void    bmemperf_write_ddr_stat_client_cas_value(
    volatile unsigned int *g_pMem,
    unsigned int           reg,
    unsigned int           value,
    unsigned int           memc_index
    )
{
    volatile long unsigned int *pMemTemp;

    pMemTemp  = (unsigned long int *) g_pMem;
    pMemTemp += (( g_memc_ddr_stat_cas_client_offset[memc_index] + reg )>>2 );
    add_to_unique((long unsigned int *) pMemTemp );
    *pMemTemp = value;
}

unsigned int bmemperf_read_arb_reg_value(
    volatile unsigned int *g_pMem,
    unsigned int           reg,
    unsigned int           memc_index
    )
{
    volatile long unsigned int *pMemTemp;

    pMemTemp  = (unsigned long int *) g_pMem;
    pMemTemp += (( g_memc_arb_reg_offset[memc_index] + reg )>>2 );
    add_to_unique((long unsigned int *) pMemTemp );
    /*printf("%s: g_memc_arb_reg_offset[%u]: 0x%x; addr 0x%x\n", __FUNCTION__, memc_index, g_memc_arb_reg_offset[memc_index], pMemTemp );*/
    return( *pMemTemp );
}

void    bmemperf_write_arb_reg_value(
    volatile unsigned int *g_pMem,
    unsigned int           reg,
    unsigned int           value,
    unsigned int           memc_index
    )
{
    volatile long unsigned int *pMemTemp;

    pMemTemp = (unsigned long int *) g_pMem;
    /*printf("%s: writing 0x%x to addr %p; ", __FUNCTION__, value, (void*) pMemTemp);*/
    pMemTemp += (( g_memc_arb_reg_offset[memc_index] + reg )>>2 );
    /*printf("+ offset 0x%x + reg 0x%x => %p\n", g_memc_arb_reg_offset[memc_index], reg, (void*) pMemTemp);*/
    add_to_unique((long unsigned int *) pMemTemp );
    *pMemTemp = value;
}

unsigned int bmemperf_read_ddr_reg_value(
    volatile unsigned int *g_pMem,
    unsigned int           reg,
    unsigned int           memc_index
    )
{
    volatile long unsigned int *pMemTemp;

    pMemTemp  = (unsigned long int *) g_pMem;
    pMemTemp += (( g_memc_ddr_reg_offset[memc_index] + reg )>>2 );
    add_to_unique((long unsigned int *) pMemTemp );
    return( *pMemTemp );
}

unsigned int bmemperf_get_ddr_stat_cas_client_data(
    unsigned int client_id,
    unsigned int memc_index
    )
{
    volatile unsigned int cas_count  = 0;
    volatile unsigned int reg_offset = 0;

    reg_offset = ( client_id * BMEMPERF_RDB_REG_SIZE );
    /**     bmemperf_read_ddr_stat_client_cas_value(g_pMem,BMEMPERF_MEMC_DDR_0_STAT_CAS_CLIENT_11,memc_index);**/
    cas_count = bmemperf_read_ddr_stat_client_cas_value( g_pMem, reg_offset, memc_index );

    return( cas_count );
}

void bmemperf_configure_stat_control_reg(
    unsigned int client_id,
    unsigned int memc_index
    )
{
    /*** configuring STAT_CONTROL ****/
    volatile unsigned int max_min_mode = 0; /**1 enable max_min_mode , 0 disable max_min_mode **/
    /** PER_CLIENT_MODE Controls the operating mode of the per-client counters:
        0: CAS command mode: Each counter counts the number of issued CAS commands for that client.
        1: Consumption mode: Each counter contains the sum of the cycles consumed by the client, either from transferring data, waiting for penalties between data bursts, or waiting for */

    volatile unsigned int value1;
    volatile unsigned int enable_flag = 1;

    /*** First stop the stats before programing control register  *****/
    value1 = 0;

    bmemperf_write_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CONTROL, value1, memc_index );

    /** configure time g_interval in terms of SCB clock frequency **/
    value1 = g_clock_time;
    bmemperf_write_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_TIMER, value1, memc_index );

    if (!g_memc_max_min_mode)
    {
        max_min_mode = 0;
    }
    else
    {
        max_min_mode = 1;
    }

    value1  = 0;
    value1  = (( client_id << BCHP_MEMC_DDR_0_STAT_CONTROL_CLIENT_ID_SHIFT )& BCHP_MEMC_DDR_0_STAT_CONTROL_CLIENT_ID_MASK );
    value1 |= (( enable_flag << BCHP_MEMC_DDR_0_STAT_CONTROL_STAT_ENABLE_SHIFT )& BCHP_MEMC_DDR_0_STAT_CONTROL_STAT_ENABLE_MASK );
    value1 |= (( max_min_mode << BCHP_MEMC_DDR_0_STAT_CONTROL_COUNTER_MODE_SHIFT )& BCHP_MEMC_DDR_0_STAT_CONTROL_COUNTER_MODE_MASK );

    /** Currently we only support to collect cas count per client so we don't need to do anytrhing.
     *  If we support transaction count then we need to set the
     *  bit as 1*/
#if 0
    per_client_mode = 1;
    value1         |= (( per_client_mode<< BCHP_MEMC_DDR_0_STAT_CONTROL_PER_CLIENT_MODE_SHIFT )& BCHP_MEMC_DDR_0_STAT_CONTROL_PER_CLIENT_MODE_MASK );
#endif

    /*  printf(" MEMC_DDR_0_STAT_CONTROL value is %u\n", value1);   **/
    /** writel_relaxed(value1, (memc0_base + BCHP_MEMC_DDR_0_STAT_CONTROL));    **/
    bmemperf_write_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CONTROL, value1, memc_index );
} /* bmemperf_configure_stat_control_reg */

/**
 *  Function: This function will return the width of the DDR bus: either 16 or 32 bits.
 **/
unsigned int bmemperf_bus_width(
    unsigned int memc_index
    )
{
    volatile unsigned int arb_reg_val  = 0;
    static unsigned int   bus_width[3] = {0, 0, 0};

    if (g_pMem)
    {
        /* if the bus width has not be determined yet, read the register now */
        if (( memc_index < BMEMPERF_NUM_MEMC ) && ( bus_width[memc_index] == 0 ))
        {
#ifdef BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG
            /* older 40-nm chips */
            volatile unsigned long int *pTempReg = (volatile unsigned long int *)((unsigned long int)g_pMem + BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG );

            /* read the current value of the register */
            arb_reg_val = *pTempReg;

            arb_reg_val = ( BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG_DRAM_WIDTH_MASK & arb_reg_val ) >> BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_CONFIG_DRAM_WIDTH_SHIFT;
#else
            /* newer 28-nm chips */
            volatile unsigned long int *pTempReg = (volatile unsigned long int *)((unsigned long int)g_pMem + BCHP_MEMC_DDR_0_CNTRLR_CONFIG );

            /* read the current value of the register */
            arb_reg_val = *pTempReg;

            arb_reg_val = ( BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_TOTAL_WIDTH_MASK & arb_reg_val ) >> BCHP_MEMC_DDR_0_CNTRLR_CONFIG_DRAM_TOTAL_WIDTH_SHIFT;
#endif

            if (arb_reg_val == 1)
            {
                bus_width[memc_index] = 16;
            }
            else
            {
                bus_width[memc_index] = 32;
            }
            printf( "%s: arb_reg_val %x; bus_width[%u] (%u)\n", __FUNCTION__, arb_reg_val, memc_index, bus_width[memc_index] );
        }
    }
    else
    {
        printf( "FATAL ERROR: g_pMem %p; g_memc_arb_reg_offset[%u] %p\n", (void *) g_pMem, memc_index, (void *) g_memc_arb_reg_offset[memc_index] );
        return( 32 );
    }

    return( bus_width[memc_index] );
} /* bmemperf_bus_width */

int bmemperf_init(
    void
    )
{
    int ret = 0, i;
    bmemperf_boxmode_source boxmodeSource;

    get_boxmode( &boxmodeSource );
    bmemperf_boxmode_init( boxmodeSource.boxmode );

    /* Open /dev/mem for memory mapping */
    g_memFd = open( "/dev/mem", O_RDWR|O_SYNC );  /*O_SYNC for uncached address */

    fcntl( g_memFd, F_SETFD, FD_CLOEXEC );

    #if 0
    printf( "%s: mmap64(NULL, mapped_size 0x%x, PROT_READ %u, MAP_SHARED %u, fd %u, addr %x)\n", __FUNCTION__,
        ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET  );
    #endif
    g_pMem = mmap64( 0, ( BCHP_REGISTER_SIZE<<2 ), PROT_READ|PROT_WRITE, MAP_SHARED, g_memFd, BCHP_PHYSICAL_OFFSET  );

    /*printf("%s: g_pMem %p\n", __FUNCTION__, (void*) g_pMem );*/
    if (!g_pMem)
    {
        printf( "Failed to mmap64 fd=%d, addr 0x%08x\n", g_memFd, BCHP_PHYSICAL_OFFSET );
        return( -1 );
    }

    /********** For memc_0  *****************/
    if (!g_ddr_clock_frequency)
    {
        /** this will make sure that we only take the ddr freq when it is not passed thru command line for bmemeperf_app **/
        g_ddr_clock_frequency = g_bmemperf_info.ddrFreqInMhz;
    }

    printf( "%s: num_memc %d; ddrFreqInMhz %d; scb %d\n", __FUNCTION__, g_bmemperf_info.num_memc, g_bmemperf_info.ddrFreqInMhz, g_bmemperf_info.scbFreqInMhz );
    g_clock_time = ( g_bmemperf_info.scbFreqInMhz*1000 ); /**1msec in scb frequency , since time is in msec unit**/

    g_clock_time *= g_interval;

    /** initialize the register offsets ***/
    for (i = 0; i<BMEMPERF_NUM_MEMC; i++)
    {
        g_memc_arb_reg_offset[i]             = 0;
        g_memc_ddr_stat_reg_offset[i]        = 0;
        g_memc_ddr_reg_offset[i]             = 0;
        g_memc_ddr_stat_cas_client_offset[i] = 0;
        g_refresh_delay[i] = 0;
    }

        #if 0
    printf( "%s: NEXUS_NUM_MEMC %u\n", __FUNCTION__, NEXUS_NUM_MEMC );
        #endif
    for (i = 0; i < BMEMPERF_NUM_MEMC; i++)
    {
        /** deriving the offsets for all possible memc's **/
        g_memc_ddr_reg_offset[i] = BMEMPERF_MEMC_BASE + ( i*BMEMPERF_MEMC_REGISTER_SIZE );
        #if 0
        printf( "%s: memc %u: base 0x%x; g_memc_ddr_reg_offset 0x%x\n", __FUNCTION__, i, BMEMPERF_MEMC_BASE, g_memc_ddr_reg_offset[i] );
        #endif

        g_memc_ddr_stat_reg_offset[i] = BMEMPERF_MEMC_BASE + BMEMPERF_MEMC_DDR_STATS_REGISTER_OFFSET + ( i *BMEMPERF_MEMC_REGISTER_SIZE );
        #if 0
        printf( "%s: memc %u: base 0x%x; g_memc_ddr_stat_reg_offset 0x%x\n", __FUNCTION__, i, BMEMPERF_MEMC_BASE, g_memc_ddr_stat_reg_offset[i] );
        #endif

        g_memc_arb_reg_offset[i] = BMEMPERF_MEMC_BASE + BMEMPERF_MEMC_ARB_CLIENT_INFO_OFFSET + ( i *BMEMPERF_MEMC_REGISTER_SIZE );
        #if 0
        printf( "%s: memc %u: base 0x%x; g_memc_arb_reg_offset 0x%x\n", __FUNCTION__, i, BMEMPERF_MEMC_BASE, g_memc_arb_reg_offset[i] );
        #endif

        g_memc_ddr_stat_cas_client_offset[i] = BMEMPERF_MEMC_BASE + BMEMPERF_MEMC_DDR_STAT_CAS_CLIENT_OFFSET + ( i *BMEMPERF_MEMC_REGISTER_SIZE );
        #if 0
        printf( "%s: memc %u: base 0x%x; cas_client_offset 0x%x\n", __FUNCTION__, i, BMEMPERF_MEMC_BASE, BMEMPERF_MEMC_DDR_STAT_CAS_CLIENT_OFFSET );
        #endif

        /*** First stop the stats before programing control register  *****/

        /*** get g_refresh_delay **/
        /** g_refresh_delay = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_DRAM_TIMING_4));  **/
        g_refresh_delay[i] = bmemperf_read_ddr_reg_value( g_pMem, BMEMPERF_MEMC_DDR_DRAM_TIMING_4, i );

        /** We use same MASK and SHIFT value for all the memc , since their bit defination are same **/
        g_refresh_delay[i]  &= BCHP_MEMC_DDR_0_DRAM_TIMING_4_REFRESH_DELAY_MASK;
        g_refresh_delay[i] >>= BCHP_MEMC_DDR_0_DRAM_TIMING_4_REFRESH_DELAY_SHIFT;
    }

    g_timer_Count_inDDR_cycles = ( g_interval * g_ddr_clock_frequency*BMEMPERF_DDR0_CLOCK_FREQ_UNIT );

    #if 0
    printf( "%s: done\n", __FUNCTION__ );
    #endif
    return( ret );
} /* bmemperf_init */

/** g_interval is = time*1000000 , this is for the sake of computaion and since we want result in Mbps **/
float bmemperf_computeBWinMbps(
    unsigned int cycles,
    unsigned int interval_in_MHZ,
    unsigned int memc_index
    )
{
    float ftemp;

    unsigned long long k;

    /** we do this since we cdon't have 64bit operation on arm v7 else this causes overflow and wrong result ,
    we will loose precision with this **/
    k     = (unsigned long long)( cycles );
    k     = (unsigned long long)( k* bmemperf_bus_width( memc_index ) * 2 );
    ftemp = k/interval_in_MHZ;

#ifdef  BMEMPERF_DEBUG
    printf( "\n cycles = %u, interval_in_MHZ = %u, k= %15lld, ftemp = %f \n", cycles, interval_in_MHZ, k, ftemp );
#endif

    return( ftemp );
} /* bmemperf_computeBWinMbps */

void bmemperf_normalmode_system_data(
    unsigned int memc_index
    )
{
    volatile unsigned int value1, value2;
    volatile unsigned int penalty_all_cycles      = 0;
    volatile unsigned int refresh_cycles          = 0;
    volatile unsigned int idle_nop_cycles         = 0;
    unsigned int          async_fifo_empty_cycles = 0;
    unsigned int          ddr_bus_idle_cycles     = 0;
    unsigned int          transaction_cycles      = 0;
    unsigned int          dataCycles              = 0;
    unsigned int          temp;
    float                 ftemp1;

    /*value1 = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_CAS_ALL));*/

    value1 = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CAS_ALL, memc_index );

    dataCycles = ( value1 *( BLOCK_LENGTH>>1 ));

#ifdef  BMEMPERF_DEBUG
    printf( "\n %s: Overall system data cycles %d \n", __FUNCTION__, dataCycles );
#endif

    /*  refresh_cycles = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_REFRESH));    **/
    refresh_cycles = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_REFRESH, memc_index );

    refresh_cycles *= g_refresh_delay[memc_index];

    /** value2 = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_PENALTY_ALL));    **/
    value2 = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_PENALTY_ALL, memc_index );

    penalty_all_cycles = value2 - ( value1*(( BLOCK_LENGTH>>1 ) - 1 )) - refresh_cycles;

    /** idle_nop_cycles = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_IDLE_NOP));  **/
    idle_nop_cycles = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_IDLE_NOP, memc_index );

#ifdef  BMEMPERF_DEBUG
    printf( "g_timer_Count_inDDR_cycles is %u\n", g_timer_Count_inDDR_cycles );
#endif

    async_fifo_empty_cycles = g_timer_Count_inDDR_cycles -( value1 + value2 + idle_nop_cycles );
    ddr_bus_idle_cycles     = async_fifo_empty_cycles + idle_nop_cycles;
    transaction_cycles      = dataCycles + refresh_cycles + penalty_all_cycles;

    temp = ( g_interval*BMEMPERF_DDR0_CLOCK_FREQ_UNIT );

    bmemperfData[memc_index].dataBW = bmemperf_computeBWinMbps( dataCycles, temp, memc_index );

#ifdef  BMEMPERF_DEBUG
    printf( "\n %s: bmemperfData[%d].dataBW %d \n", __FUNCTION__, memc_index, bmemperfData[memc_index].dataBW );
#endif

    bmemperfData[memc_index].refreshBW = bmemperf_computeBWinMbps( refresh_cycles, temp, memc_index );

    bmemperfData[memc_index].transactionBW =  bmemperf_computeBWinMbps( transaction_cycles, temp, memc_index );

    bmemperfData[memc_index].idleBW =  bmemperf_computeBWinMbps( ddr_bus_idle_cycles, temp, memc_index );

    ftemp1 = ( dataCycles );
    bmemperfData[memc_index].dataUtil  = ftemp1/g_timer_Count_inDDR_cycles;
    bmemperfData[memc_index].dataUtil *= 100; /**percentage **/
}                                             /* bmemperf_normalmode_system_data */

void bmemperf_set_arb_vals(
    unsigned int client_id,
    unsigned int memc_index,
    unsigned int bo_val,
    unsigned int rr_flag
    )
{
    volatile unsigned int tempReg;
    volatile unsigned int arb_reg_val = 0;

    volatile unsigned int current_bo_val  = 0;
    volatile unsigned int current_rr_flag = 0;

    /*** First collecting existing arb val  ***/
    tempReg     = ( client_id<<2 );
    arb_reg_val = bmemperf_read_arb_reg_value( g_pMem, tempReg, memc_index );

    current_rr_flag = (( arb_reg_val & BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_MASK )>> BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_SHIFT );
    current_bo_val  = (( arb_reg_val & BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_MASK )>> BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_SHIFT );
#ifdef BMEMPERF_DEBUG
    printf( "%s: current arb_reg_val = %u,current_rr_flag = %u, current_bo_val = %u, new bo_val = %u and new rr_flag = %u \n", __FUNCTION__, arb_reg_val, current_rr_flag, current_bo_val, bo_val, rr_flag );
#endif

    if (bo_val != BMEMPERF_INVALID)
    {
        /** first remove the existing value **/
        arb_reg_val ^= (( current_bo_val << BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_SHIFT )&BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_MASK );

        /**add new val **/
        arb_reg_val |= (( bo_val << BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_SHIFT )&BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_MASK );
    }

    if (rr_flag != BMEMPERF_INVALID)
    {
        /** first remove the existing value **/
        arb_reg_val ^= (( current_rr_flag << BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_SHIFT )&BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_MASK );

        /**add new val **/
        arb_reg_val |= (( rr_flag << BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_SHIFT )&BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_MASK );
    }

    bmemperf_write_arb_reg_value( g_pMem, tempReg, arb_reg_val, memc_index );
} /* bmemperf_set_arb_vals */

unsigned int  bmemperf_collect_arb_details(
    unsigned int client_id,
    unsigned int memc_index
    )
{
    volatile unsigned int tempReg;
    volatile unsigned int arb_reg_val = 0;

    /*** collecting all arb  details ***/
    tempReg     = ( client_id<<2 );
    arb_reg_val = bmemperf_read_arb_reg_value( g_pMem, tempReg, memc_index );

    /*  printf(" Value of BCHP_MEMC_ARB_0_CLIENT_INFO_10  = %x\n", *pMemTemp);  */
    bmemperfData[memc_index].rrEn  = (( arb_reg_val & BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_MASK )>> BCHP_MEMC_ARB_0_CLIENT_INFO_2_RR_EN_SHIFT );
    bmemperfData[memc_index].boVal = (( arb_reg_val & BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_MASK )>> BCHP_MEMC_ARB_0_CLIENT_INFO_2_BO_VAL_SHIFT );

    return( arb_reg_val );
}

/**
 *  Function: This function will return a true or false based on whether or not the specified client id on the
 *  specified memc has it RTS_ERR bit set. There are error bits for 32 clients in each register.
 **/
unsigned int  bmemperf_get_arb_err(
    unsigned int client_id,
    unsigned int memc_index
    )
{
    volatile unsigned int arb_reg_val  = 0;
    volatile unsigned int arb_reg_val2 = 0;
    unsigned long int     bitmask      = 0;

    /* there are 32 client values per register (1 bit each); each addr is 4 bytes long */
    volatile unsigned long int *pTempReg = (volatile unsigned long int *)((unsigned long int)g_pMem +  g_memc_arb_reg_offset[memc_index] +
                                                                          BCHP_MEMC_ARB_0_RTS_ERR_0 - BCHP_MEMC_ARB_0_CLIENT_INFO_0 + ( client_id/32*4 ));

    /* read the current value of the register (contains error bits for 32 clients) */
    arb_reg_val = arb_reg_val2 = *pTempReg;

    /* shift desired client id to the appropriate bit position */
    bitmask = 1<<( client_id%32 );

    /* clear all other bit values except the one we are interested in */
    arb_reg_val &= bitmask;

    return(( arb_reg_val ) ? 1 : 0 );
} /* bmemperf_get_arb_err */

/**
 *  Function: This function will cause all RTS_ERR bits to be cleared for the specified memc. This is
 *  accomplished by writing a 1 to the RTS_ERR_INFO_WRITE_CLEAR register for the specified memc.
 **/
unsigned int  bmemperf_clear_arb_err(
    unsigned int memc_index
    )
{
    volatile unsigned long int *pTempReg = (volatile unsigned long int *)((unsigned long int)g_pMem +  g_memc_arb_reg_offset[memc_index] +
                                                                          BCHP_MEMC_ARB_0_RTS_ERR_INFO_WRITE_CLEAR - BCHP_MEMC_ARB_0_CLIENT_INFO_0 );

    if (g_pMem)
    {
        *pTempReg = 1;
    }
    else
    {
        printf( "FATAL ERROR: g_pMem %p; g_memc_arb_reg_offset[%u] %p\n", (void *) g_pMem, memc_index, (void *) g_memc_arb_reg_offset[memc_index] );
    }

    return( 0 );
}

void bmemperf_normalmode_client_service_data(
    unsigned int memc_index
    )
{
    volatile unsigned int value1, value2;
    unsigned int          client_data_cycles         = 0;
    unsigned int          client_Intr_penalty_cycles = 0;
    volatile unsigned int client_post_penalty_cycles = 0;
    unsigned int          client_penalty_cycles      = 0;
    unsigned int          client_transaction_cycles  = 0;

    volatile unsigned int number_of_trans_read  = 0;
    volatile unsigned int number_of_trans_write = 0;
    unsigned int          temp;
    float                 ftemp1;

    /**  value1 = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_CAS)); **/
    value1 = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CLIENT_SERVICE_CAS, memc_index );

    client_data_cycles = value1*( BLOCK_LENGTH>>1 );

    /** value2 = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_INTR_PENALTY));    **/
    value2 = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CLIENT_SERVICE_INTR_PENALTY, memc_index );

    client_Intr_penalty_cycles = value2 - ( value1*(( BLOCK_LENGTH>>1 ) - 1 ));

    /**     client_post_penalty_cycles = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_POST_PENALTY));    **/
    client_post_penalty_cycles = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CLIENT_SERVICE_POST_PENALTY, memc_index );

    client_penalty_cycles     = client_Intr_penalty_cycles + client_post_penalty_cycles;
    client_transaction_cycles = client_data_cycles + client_penalty_cycles;

    /*printf("\n client_transaction_cycles = %u \n", client_transaction_cycles);*/

    /*  number_of_trans_read = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_TRANS_READ));    */
    number_of_trans_read = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CLIENT_SERVICE_TRANS_READ, memc_index );

    /** number_of_trans_write = readl_relaxed((memc0_base + BCHP_MEMC_DDR_0_STAT_CLIENT_SERVICE_TRANS_WRITE));  **/
    number_of_trans_write = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_CLIENT_SERVICE_TRANS_WRITE, memc_index );

    temp = ( g_interval*BMEMPERF_DDR0_CLOCK_FREQ_UNIT );

    bmemperfData[memc_index].clientDataBW = bmemperf_computeBWinMbps( client_data_cycles, temp, memc_index );

    bmemperfData[memc_index].clientTransBW = bmemperf_computeBWinMbps( client_transaction_cycles, temp, memc_index );

    ftemp1 = ( client_data_cycles );
    bmemperfData[memc_index].clientDataUtil  = ftemp1/g_timer_Count_inDDR_cycles;
    bmemperfData[memc_index].clientDataUtil *= 100; /**percentage **/

    /**since g_interval is in msec , so approximating the
     * TransactionCount for 1 sec interval  */
    bmemperfData[memc_index].clientRdTransInPerc = ( number_of_trans_read * 1000 )/( g_interval );
    bmemperfData[memc_index].clientWrTransInPerc = ( number_of_trans_write * 1000 )/( g_interval );

    /** computing avgClientDataSizeInBits **/
    if (( number_of_trans_read + number_of_trans_write ) != 0)
    {
        ftemp1  = client_data_cycles;
        ftemp1 /= ( number_of_trans_read + number_of_trans_write );
        ftemp1 *= bmemperf_bus_width( memc_index ) * 2;
        bmemperfData[memc_index].avgClientDataSizeInBits = ftemp1;
    }
    else
    {
        bmemperfData[memc_index].avgClientDataSizeInBits = 0;
    }

    /** computing avgClientTransSizeInBits **/
    if (( number_of_trans_read + number_of_trans_write ) != 0)
    {
        ftemp1  = client_transaction_cycles;
        ftemp1 /= ( number_of_trans_read + number_of_trans_write );
        ftemp1 *= bmemperf_bus_width( memc_index ) * 2;
        bmemperfData[memc_index].avgClientTransSizeInBits = ftemp1;
    }
    else
    {
        bmemperfData[memc_index].avgClientTransSizeInBits = 0;
    }

    /** computing avgTimeBwnClientTransInNsec **/
    if (( number_of_trans_read + number_of_trans_write ) != 0)
    {
        bmemperfData[memc_index].avgTimeBwnClientTransInNsec = ( 1000 *1000 )/( number_of_trans_read + number_of_trans_write * g_interval );
    }
    else
    {
        bmemperfData[memc_index].avgTimeBwnClientTransInNsec = 0;
    }

    /** approx total transaction count in a second,
     *  g_timeinterval is in msec unit  **/
    bmemperfData[memc_index].totalTrxnCount = (( number_of_trans_read + number_of_trans_write )*1000 )/( g_interval );
} /* bmemperf_normalmode_client_service_data */

void bmemperf_maxminmode_system_data(
    unsigned int memc_index
    )
{
    volatile unsigned int max_data_cycles         = 0;
    volatile unsigned int min_data_cycles         = 0;
    volatile unsigned int min_txn_cycles          = 0;
    volatile unsigned int max_txn_cycles          = 0;
    volatile unsigned int min_ddr_bus_idle_cycles = 0;
    volatile unsigned int max_ddr_bus_idle_cycles = 0;
    unsigned int          temp;

    float ftemp1;

    /*************** Overall system data ****************/
    max_data_cycles = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MAX_CAS_ALL, memc_index );

    max_data_cycles = max_data_cycles*( BLOCK_LENGTH>>1 );

    min_data_cycles = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MIN_CAS_ALL, memc_index );
    min_data_cycles = min_data_cycles*( BLOCK_LENGTH>>1 );

    max_txn_cycles = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MAX_TRANS_CYCLES_ALL, memc_index );

    min_txn_cycles = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MIN_TRANS_CYCLES_ALL, memc_index );

    max_ddr_bus_idle_cycles = g_timer_Count_inDDR_cycles - min_txn_cycles; /** max_idle so (time_IN_DDR -min_dat_Cycle ) **/
    min_ddr_bus_idle_cycles = g_timer_Count_inDDR_cycles - max_txn_cycles; /** min_idle so (time_IN_DDR - max_dat_Cycle ) **/

    /*  printf("\n max_data_cycles = %u, max_txn_cycles= %u \n", max_data_cycles , max_txn_cycles); */

    ftemp1 = ( max_data_cycles );
    bmemperfData[memc_index].max_data_util  = ftemp1/g_timer_Count_inDDR_cycles;
    bmemperfData[memc_index].max_data_util *= 100; /**percentage **/

    ftemp1 = ( min_data_cycles );
    bmemperfData[memc_index].min_data_util  = ftemp1/g_timer_Count_inDDR_cycles;
    bmemperfData[memc_index].min_data_util *= 100; /**percentage **/

    temp = ( g_interval*BMEMPERF_DDR0_CLOCK_FREQ_UNIT );

    bmemperfData[memc_index].max_data_bw = bmemperf_computeBWinMbps( max_data_cycles, temp, memc_index );

    bmemperfData[memc_index].min_data_bw = bmemperf_computeBWinMbps( min_data_cycles, temp, memc_index );

    bmemperfData[memc_index].max_txn_bw = bmemperf_computeBWinMbps( max_txn_cycles, temp, memc_index );

    bmemperfData[memc_index].min_txn_bw = bmemperf_computeBWinMbps( min_txn_cycles, temp, memc_index );

    bmemperfData[memc_index].min_idle_bw = bmemperf_computeBWinMbps( min_ddr_bus_idle_cycles, temp, memc_index );

    bmemperfData[memc_index].max_idle_bw = bmemperf_computeBWinMbps( max_ddr_bus_idle_cycles, temp, memc_index );
} /* bmemperf_maxminmode_system_data */

void bmemperf_maxminmode_client_service_data(
    unsigned int memc_index
    )
{
    volatile unsigned int client_max_data_cycle = 0;
    volatile int          client_min_data_cycle = 0;
    volatile unsigned int client_max_txn_cycle  = 0;
    volatile int          client_min_txn_cycle  = 0;
    volatile unsigned int client_max_srvc_txn   = 0;
    volatile unsigned int client_min_srvc_txn   = 0;
    unsigned int          temp;

    float ftemp1;

    client_max_data_cycle = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MAX_CLIENT_SERVICE_CAS, memc_index );
    client_max_data_cycle = client_max_data_cycle*( BLOCK_LENGTH>>1 );

    client_min_data_cycle = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MIN_CLIENT_SERVICE_CAS, memc_index );

#ifdef  BMEMPERF_DEBUG
    printf( "\n ----------------- Client Min data cycle = %x\n", client_min_data_cycle );
#endif
    if (client_min_data_cycle == -1) /** this is done since min numbers are initialized to -1 **/
    {
        client_min_data_cycle = 0;
    }
    client_min_data_cycle = client_min_data_cycle*( BLOCK_LENGTH>>1 );

    client_max_txn_cycle = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MAX_CLIENT_SERVICE_CYCLES, memc_index );

    client_min_txn_cycle = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MIN_CLIENT_SERVICE_CYCLES, memc_index );

    if (client_min_txn_cycle == -1) /** this is done since min numbers are initialized to -1 **/
    {
        client_min_txn_cycle = 0;
    }

    client_max_srvc_txn = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MAX_CLIENT_SERVICE_TRANS, memc_index );
    /**since g_interval is in msec , so approximating the
     * MaxTransactionCount for 1 sec interval */
    bmemperfData[memc_index].max_client_srvc_txn = ( client_max_srvc_txn * 1000 )/( g_interval );

    client_min_srvc_txn = bmemperf_read_ddr_stat_value( g_pMem, BMEMPERF_MEMC_DDR_STAT_MIN_CLIENT_SERVICE_TRANS, memc_index );

    bmemperfData[memc_index].min_client_srvc_txn = client_min_srvc_txn;

    ftemp1 = ( client_max_data_cycle );
    bmemperfData[memc_index].max_client_data_util  = ftemp1/g_timer_Count_inDDR_cycles;
    bmemperfData[memc_index].max_client_data_util *= 100; /**percentage **/

    ftemp1 = ( client_min_data_cycle );
    bmemperfData[memc_index].min_client_data_util  = ftemp1/g_timer_Count_inDDR_cycles;
    bmemperfData[memc_index].min_client_data_util *= 100; /**percentage **/

    temp = ( g_interval*BMEMPERF_DDR0_CLOCK_FREQ_UNIT );

    bmemperfData[memc_index].max_client_data_bw = bmemperf_computeBWinMbps( client_max_data_cycle, temp, memc_index );

    bmemperfData[memc_index].min_client_data_bw = bmemperf_computeBWinMbps( client_min_data_cycle, temp, memc_index );

    bmemperfData[memc_index].max_client_txn_bw = bmemperf_computeBWinMbps( client_max_txn_cycle, temp, memc_index );

    bmemperfData[memc_index].min_client_txn_bw = bmemperf_computeBWinMbps( client_min_txn_cycle, temp, memc_index );
} /* bmemperf_maxminmode_client_service_data */

int comapare_cas_data(
    const void *a,
    const void *b
    )
{
    bmemperf_client_casdata_Index *cas_a = (bmemperf_client_casdata_Index *)a;
    bmemperf_client_casdata_Index *cas_b = (bmemperf_client_casdata_Index *)b;

    if (cas_a->data < cas_b->data)
    {
        return( 1 );
    }
    else if (cas_a->data == cas_b->data)
    {
        return( 0 );
    }
    else
    {
        return( -1 );
    }
} /* comapare_cas_data */
