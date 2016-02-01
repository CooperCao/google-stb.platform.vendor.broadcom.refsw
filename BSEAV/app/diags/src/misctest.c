/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "prompt.h"
#include "bstd.h"
#include "bkni.h"

#if (BCM_BOARD==97342) || (BCM_BOARD==97420)
    #define MISC_TEST_WKTMR 1
#else
    #define MISC_TEST_WKTMR 0
#endif

#if (0) /*(BCM_BOARD==97346)*/
    #define MISC_TEST_REMUX 1
#else
    #define MISC_TEST_REMUX 0
#endif

#if MISC_TEST_WKTMR==1
    #if BCHP_CHIP==7342
        #include "bchp_intr_ctrl.h"
    #else
        #include "bchp_int_id_mcif_intr2.h"
    #endif
    #include "bchp_wktmr.h"
    #include "breg_mem.h"
    #include "sys_handles.h"
    #include "mipsclock.h"
#endif

#if MISC_TEST_REMUX==1
    #include "nexus_transport_module.h"
#endif

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/
extern uint32_t CpuCountGet(void);
extern uint32_t total_ticks;

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

void bcmMiscTest(void);

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Global variables
 ***********************************************************************/
uint32_t total_ticks_before;
uint32_t total_ticks_after;
uint32_t count_before;
uint32_t count_after;
#define WKTMR_TIMEOUT 1 /*20*/
#define PRESCALER 27000000
#define FACTOR 1 /*1000000*/

#if BCHP_CHIP==7342
#define BCHP_INT_ID_WKTMR_ALARM_INTR BCHP_INT_ID_CREATE(BCHP_INTR_CTRL_CPU_STATUS, BCHP_INTR_CTRL_CPU_STATUS_WKTMR_ALARM_INTR_SHIFT)
#endif

#if MISC_TEST_WKTMR==1
     /***********************************************************************
     *
     *  wktmr_cb()
     * 
     *  Callback function for the wakeup timer test.
     *
     ***********************************************************************/
    void wktmr_cb(void)
    {
        uint32_t elapsed, elapsed_count;

        count_after = CpuCountGet();
    /*	printf("count_in_isr = 0x%08x\n", count_in_isr); */
        printf("Cpu Count = 0x%08x\n", count_after);
    /*	printf("time_spent_in_isr = %f msec\n", ((float)count_after-count_in_isr)/202500); */

        if (count_after > count_before)
            elapsed_count = count_after-count_before;
        else
            elapsed_count = count_after+(0xffffffff-count_before);

        printf("elapsed_count = %ul, msec = %f\n", elapsed_count, (float)elapsed_count / (get_cpu_clock_rate_hz() / (1000 * INPUT_CLK_CYCLES_PER_COUNT_REG_INCR)));

        printf("wktmr_cb!\n");
        printf("BCHP_WKTMR_PRESCALER=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_PRESCALER));
        printf("BCHP_WKTMR_PRESCALER_VAL RO=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_PRESCALER_VAL));
        printf("BCHP_WKTMR_EVENT=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_EVENT));
        printf("BCHP_WKTMR_COUNTER=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_COUNTER));
        printf("BCHP_WKTMR_ALARM=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_ALARM));
        total_ticks_after = total_ticks;
        printf("total_ticks_before = %d\n", total_ticks_before);
        printf("total_ticks_after = %d\n", total_ticks_after);
        elapsed = total_ticks_after-total_ticks_before;
        printf("elapsed = %d\n", elapsed); 
        BREG_Write32(bcmGetRegHandle(), BCHP_WKTMR_PRESCALER, PRESCALER/FACTOR);
        BREG_Write32(bcmGetRegHandle(), BCHP_WKTMR_EVENT, 0x00000001);
        BREG_Write32(bcmGetRegHandle(), BCHP_WKTMR_ALARM, WKTMR_TIMEOUT*FACTOR);
        BREG_Write32(bcmGetRegHandle(), BCHP_WKTMR_COUNTER, 0x00000000);
        count_before = CpuCountGet();
        total_ticks_before = total_ticks;

        printf("BCHP_WKTMR_PRESCALER=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_PRESCALER));
        printf("BCHP_WKTMR_PRESCALER_VAL RO=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_PRESCALER_VAL));
        printf("BCHP_WKTMR_EVENT=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_EVENT));
        printf("BCHP_WKTMR_COUNTER=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_COUNTER));
        printf("BCHP_WKTMR_ALARM=0x%08x\n", BREG_Read32(bcmGetRegHandle(), BCHP_WKTMR_ALARM));
    }
#endif

#if MISC_TEST_REMUX==1
    static void ParserAllPassToRemux( 
        BXPT_Handle hXpt
        )
    {
        BXPT_Remux_ChannelSettings RemuxSettings;
        BXPT_Remux_Handle hRemux;
        int pktno=0;
        char				str[20];

    #if 0
        BXPT_RsBuf_SetBandDataRate(hXpt, 0, 81000000, 188); 
    #if (BCHP_CHIP == 7325)
        BXPT_XcBuf_SetBandDataRate( hXpt, BXPT_XcBuf_Id_RMX0_IBP2, YOUR_BITRATE_HERE, 188 );
    #else        
        BXPT_XcBuf_SetBandDataRate(hXpt, BXPT_XcBuf_Id_RMX0_A, 81000000, 188);
        BXPT_XcBuf_SetBandDataRate(hXpt, BXPT_XcBuf_Id_RMX0_B, 81000000, 188);
    #endif
    #endif

        printf("Packet interface mapping:\n");
        printf("    0=BCM4506(A) Channel 0\n");
        printf("    1=BCM4506(A) Channel 1\n");
        printf("    2=PKT2\n");
        printf("    3=BCM4506(B) Channel 1\n");
        printf("    4=PKT4\n"); 
        printf("    5=BCM4506(B) Channel 0\n");
        printf("    6=PKT6\n"); 
        printf("    7=PKT7\n"); 
        printf("    8=SDS0\n"); 
        printf("    9=SDS1\n\n"); 

        while (1)
        {
            printf("Enter packet interface number:  ");
            rdstr(str);
            sscanf (str, "%d", &pktno);
            printf("\n");
            if (pktno > 9)
            {
                printf("Invalid entry\n");
            }
            else
                break;
        }

        /* Pass all data on input band 2 through PID channel 0 */
        BXPT_SetParserDataSource( hXpt, 0, BXPT_DataSource_eInputBand, pktno/*9*/);
        BXPT_ParserAllPassMode(hXpt,0,true);
        BXPT_SetParserEnable(hXpt, 0, true);
    /*	BXPT_ConfigurePidChannel( hXpt, 0, 0, 0); */
        BXPT_EnablePidChannel(hXpt, 0);

        /* Remux takes data on PID channel 0 and clocks it out at 81 Mbps. */
        BXPT_Remux_GetChannelDefaultSettings( hXpt, 0, &RemuxSettings ); 
        RemuxSettings.OutputClock = BXPT_RemuxClock_e54Mhz; /*BXPT_RemuxClock_e81Mhz;*/
        BXPT_Remux_OpenChannel( hXpt, &hRemux, 0, &RemuxSettings );
        BXPT_Remux_AddPidChannelToRemux( hRemux, BXPT_RemuxInput_eA, 0);
        BXPT_Remux_DoRemux( hRemux, true );

    /*
        printf( "Press any key to quit\n" );
        getchar();
    */
    }
#endif

 /***********************************************************************
 *
 *  bcmMiscTest()
 * 
 *  Misc test function
 *
 ***********************************************************************/
void bcmMiscTest(void)
{
    uint32_t command_id;

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  MISC TEST MENU  \n");
        printf("================\n");
        printf("    0) Exit\n");
        printf("    1) Wakeup Timer Test\n");
        printf("    2) Remux Test\n");
        printf("    3) OS Timer Tick Test\n");

        command_id = Prompt();
        
        switch(command_id)
        {
            case 0:
                return;

            case 1:
                #if MISC_TEST_WKTMR==1
                {
                    BINT_CallbackHandle hChnCallback;
                    uint32_t count1, count2, elapsed;


                    count1=CpuCountGet();
                    /*DetCpuTime();*/
                    count2=CpuCountGet();
                    elapsed = count2 - count1;
                    printf("count1=0x%08x, count2=0x%08x, elapsed=%d, msec=%f\n", count1, count2, elapsed, (float)elapsed / (get_cpu_clock_rate_hz() / (1000 * INPUT_CLK_CYCLES_PER_COUNT_REG_INCR)));

                    BINT_CreateCallback(
                        &hChnCallback, bcmGetIntHandle(), BCHP_INT_ID_WKTMR_ALARM_INTR,
                        (BINT_CallbackFunc)wktmr_cb, (void *) 0, 0x00 );
                    BINT_EnableCallback( hChnCallback );

                    printf("waiting...\n");

                    BREG_Write32(bcmGetRegHandle(), BCHP_WKTMR_EVENT, 0x00000001);
                    BREG_Write32(bcmGetRegHandle(), BCHP_WKTMR_ALARM, WKTMR_TIMEOUT);
                    BREG_Write32(bcmGetRegHandle(), BCHP_WKTMR_COUNTER, 0x00000000);
                    count_before = CpuCountGet();
                    printf("Cpu Count = 0x%08x\n", count_before);
                    total_ticks_before = total_ticks;

                    while (1);
                }
                #else
                printf("Wake timer test not supported\n");
                #endif
                break;

            case 2:
                #if MISC_TEST_REMUX==1
                    ParserAllPassToRemux(pTransport->xpt);
                #else
                    printf("Remux test not supported\n");
                #endif
                break;

            case 3:
                {
                    int count=0;
                    while (1) {
                        BKNI_Delay(1000000);
                        printf("elapsed seconds:  %d\n", count++);
                    }
                }
                break;


            default:
                printf("Invalid selection4\n");
                break;
        }
    }
}
