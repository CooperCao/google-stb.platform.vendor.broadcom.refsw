/***************************************************************************
 *     (c)2005-2013 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
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
#ifndef BADS_DEF_H__
#define BADS_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *  BADS CORE Versions
 ****************************************************************************/
#define BADS_P_BCHP_DS_CORE_V(MAJOR,MINOR) ((MAJOR*10)+MINOR)

#define BADS_P_BCHP_DS_CORE_V_9_1 (BADS_P_BCHP_DS_CORE_V(9,1))
#define BADS_P_BCHP_DS_CORE_V_9_2 (BADS_P_BCHP_DS_CORE_V(9,2))
#define BADS_P_BCHP_DS_CORE_V_9_3 (BADS_P_BCHP_DS_CORE_V(9,3))
#define BADS_P_BCHP_DS_CORE_V_9_4 (BADS_P_BCHP_DS_CORE_V(9,4))
#define BADS_P_BCHP_DS_CORE_V_9_5 (BADS_P_BCHP_DS_CORE_V(9,5))
#define BADS_P_BCHP_DS_CORE_V_9_6 (BADS_P_BCHP_DS_CORE_V(9,6))

/***************************************************************************
 *  BADS CORE Defines
 ****************************************************************************/
#if (BCHP_VER == BCHP_VER_A0)
  #if ((BCHP_FAMILY==3462))
    #define BADS_P_BCHP_DS_CORE_VER (BADS_P_BCHP_DS_CORE_V_9_5)
	#define FFT_IRQ 1
  #elif ((BCHP_FAMILY==7584))
    #define BADS_P_BCHP_DS_CORE_VER (BADS_P_BCHP_DS_CORE_V_9_6)
	#define FFT_IRQ 0
  #endif
#elif (BCHP_VER == BCHP_VER_B0)
  #if ((BCHP_CHIP==7552) || (BCHP_FAMILY==3461) || (BCHP_FAMILY==3462))
    #define BADS_P_BCHP_DS_CORE_VER (BADS_P_BCHP_DS_CORE_V_9_2)
	#define FFT_IRQ 1
  #endif
#elif (BCHP_VER == BCHP_VER_C0)
  #if ((BCHP_FAMILY==3128))
    #define BADS_P_BCHP_DS_CORE_VER (BADS_P_BCHP_DS_CORE_V_9_3)
	#define FFT_IRQ 1
  #endif
#endif

#ifndef BADS_P_BCHP_DS_CORE_VER
  #error DS core NOT DEFINED in ADS PI
#endif

/***************************************************************************
 *  BADS define statements
 ****************************************************************************/
#define PRINT_DEBUG 0				  /*Print debug messages to the UART*/
#define ADS_INTERNAL_ERROR_CHECKING 3 /*0 no error checking, 1 check divide by 0, 2 ADD check ranges, 3 ADD check results*/

#define TIMING_SCAN_FFT_TIMEOUT 7    /*Timing Scan FFT Interrupt timeout in mS*/
#define CARRIER_SCAN_FFT_TIMEOUT 11  /*Carrier Scan FFT Interrupt timeout in mS*/
#define NUM_TIMING_FFTS 3			 /*number of timing FFT's to perform 1,2, or 3*/
#define NUM_CARRIER_FFTS 3			 /*number of carrier FFT's to perform 1,2, or 3*/

#define PRE_NYQUIST_FILTER_BW_1MHZ 60 /*Bandwidth for stepping carrier loop during baud/carrier search: This is in KHz and is referenced to 1 MBaud and must be >= 8*/
#define MAX_PHASE_ERROR 5000		  /*maximum phase loop offset to declare carrier lock*/
#define SNR_LEAKY_AVG 512		      /*leaky averager rate for average SNR computation*/
	 
/***************************************************************************************************************
*Acquisition parameters
 ***************************************************************************************************************/
/*Define number of times to retry if not locked and we are in Auto Acquire mode is enabled for different AcqTypes*/
#define NUM_RETRIES_IF_AUTOACQUIRE_AND_AUTO 2
#define NUM_RETRIES_IF_AUTOACQUIRE_AND_FAST 0
#define NUM_RETRIES_IF_AUTOACQUIRE_AND_SLOW 0
#define NUM_RETRIES_IF_AUTOACQUIRE_AND_SCAN 0

/*Number of fast acquires before transitioning to slow acquire if Auto Acquire mode is enabled and if acquisition type is AUTO*/
/*Number of slow acquires before transitioning to slow scan acquire if Auto Acquire mode is enabled and if acquisition type is AUTO*/
/*Slow scan acquire is the union of the acquire params and the scan params*/
#define NUM_FAST_ACQUIRES 1     
#define NUM_SLOW_ACQUIRES 1

/*Number of ms for AGCB to converge, this is baud based but we don't know the baud rate*/
#define AGCB_CONVERGENCE_TIME_MS 3       

 /*Number of baud samples for TL to converge, 5000 = 1 ms for Baud rate of 5 MBAUD*/
#define TL_TIME_ACQ_BAUD_SAMPLES  10000   
#define TL_TIME_TRK_BAUD_SAMPLES   5000  

/*Number of baud samples for different EQ modes to converge, 5000 = 1 ms for Baud rate of 5 MBAUD*/
#define CMA_TIME_BLIND1_BAUD_SAMPLES 30000 
#define CMA_TIME_BLIND2_BAUD_SAMPLES 20000  
#define CMA_TIME_LOCKED1_BAUD_SAMPLES 30000  
#define CMA_TIME_LOCKED2_BAUD_SAMPLES 20000  
#define CMA_TIME_FAST_TRIM_BAUD_SAMPLES 10000   
#define LMS_TIME_FAST_TRIM_BAUD_SAMPLES  10000  
#define CMA_TIME_SLOW_TRIM1_BAUD_SAMPLES 768000    
#define LMS_TIME_SLOW_TRIM1_BAUD_SAMPLES 256000    
#define CMA_TIME_SLOW_SLOW_TRIM1_BAUD_SAMPLES 768000*2    /*AnnexA Null packet support*/
#define LMS_TIME_SLOW_SLOW_TRIM1_BAUD_SAMPLES 256000*8    /*AnnexA Null packet support*/
#define CMA_TIME_SLOW_TRIM2_BAUD_SAMPLES 48000    
#define LMS_TIME_SLOW_TRIM2_BAUD_SAMPLES 16000 

/*Number of samples to wait when switching in the CPL integrator leak and phase accummulator leak*/
/*FEC sync timeout in ms for AnnexA and Annex B*/
#define ANNEXA_FEC_LOCK_TIMEOUT	20    
#define ANNEXB_FEC_LOCK_TIMEOUT 40    

/*****************************************************************************************************************
*Lock Detector Values
*The PI polls the BADS_P_Get_LockStatus function
*Lock is declared IF the number of clean blocks detected in since the previous call is >= NUM_CLEAN_BLOCKS_TO_LOCK 
*if this condition is not met then 
*Lock is declared if there have NOT been more then NUM_BAD_BLOCK_TO_UNLOCK bad blocks have accumulated 
*if this condition is not met then 
*Unlock is declared if there have been more then NUM_BAD_BLOCK_TO_UNLOCK bad blocks have accumulated
*****************************************************************************************************************/
#define NUM_CLEAN_BLOCKS_TO_LOCK 1
#define NUM_BAD_BLOCK_TO_UNLOCK 1000
#define STUCK_FEC_RESET_COUNT 10

/*Acquire Parameter Ranges*/
#define MAX_CARRIER_RANGE 1000000
#define MIN_BAUD_RATE 1000000
#define MAX_BAUD_RATE 7300000

/*Scan Parameter Ranges*/
#define MAX_CARRIER_SCAN 1000000
#define MIN_BAUD_SCAN 1000000
#define MAX_BAUD_SCAN 7300000

/*AnnexB Baud Ranges*/
#define Q64_ANNEXB_SYMBOL_RATE 5056941
#define MAX_Q64_ANNEXB_SYMBOL_RATE 5081941
#define MIN_Q64_ANNEXB_SYMBOL_RATE 5031941
#define Q256_Q1024_ANNEXB_SYMBOL_RATE 5360537
#define MAX_Q256_Q1024_ANNEXB_SYMBOL_RATE 5385537
#define MIN_Q256_Q1024_ANNEXB_SYMBOL_RATE 5335537

/*What CWC parameters to use if INIT_BBS_CWC==BADS_Internal_Params_eEnable*/
/*the CWC will use parameters from the UFE or the WFE to know what RF frequency the toned are at*/
/*See global_clk.h files and the BADS_P_Set_CWC_Auto() funtion for more information*/
#define CWC_MODE1	2	/*0 disable, 1 for Non-tracking mode (CWC_LFC1 - CWC_LFC4 will be set to 0), 2 for AFC mode, 3 for PLL mode*/
#define CWC_MODE2	3		        
#define CWC_MODE3	3		
#define CWC_MODE4	3
#define CWC_AFC_ACQ_BW 0x12800320
#define CWC_AFC_TRK_BW 0x03200160 
#define CWC_AFC_MU 2
#define CWC_AFC_LEAK 0
#define CWC_PLL_ACQ_BW 0x00040064
#define CWC_PLL_TRK_BW 0x00040064
#define CWC_PLL_MU 2
#define CWC_PLL_LEAK 0

/*CWC core was changed in core 9.6*/
#define CWC_FOFS_LIMIT_ACQ 7 /*7 means no limit on CWC during acquisition*/

/******************************************************************************************
*THE CWC HAS SOME STRANGE PROGRAMMING DO NOT CHANGE THE DEFINITIONS BELOW
*The chip has a bitwise definition for the AFC/PLL mode but no bitwise RDB representation
*******************************************************************************************/
#define CWC_ACQ_LFC1 ((CWC_MODE1 == 3) ? CWC_PLL_ACQ_BW : CWC_AFC_ACQ_BW) 
#define CWC_ACQ_LFC2 ((CWC_MODE2 == 3) ? CWC_PLL_ACQ_BW : CWC_AFC_ACQ_BW)
#define CWC_ACQ_LFC3 ((CWC_MODE3 == 3) ? CWC_PLL_ACQ_BW : CWC_AFC_ACQ_BW) 
#define CWC_ACQ_LFC4 ((CWC_MODE4 == 3) ? CWC_PLL_ACQ_BW : CWC_AFC_ACQ_BW) 
#define CWC_TRK_LFC1 ((CWC_MODE1 == 3) ? CWC_PLL_TRK_BW : CWC_AFC_TRK_BW) 
#define CWC_TRK_LFC2 ((CWC_MODE2 == 3) ? CWC_PLL_TRK_BW : CWC_AFC_TRK_BW)
#define CWC_TRK_LFC3 ((CWC_MODE3 == 3) ? CWC_PLL_TRK_BW : CWC_AFC_TRK_BW) 
#define CWC_TRK_LFC4 ((CWC_MODE4 == 3) ? CWC_PLL_TRK_BW : CWC_AFC_TRK_BW) 
#define CWC_MU1 ((CWC_MODE1 == 3) ? CWC_PLL_MU : CWC_AFC_MU) 
#define CWC_MU2 ((CWC_MODE2 == 3) ? CWC_PLL_MU : CWC_AFC_MU)
#define CWC_MU3 ((CWC_MODE3 == 3) ? CWC_PLL_MU : CWC_AFC_MU) 
#define CWC_MU4 ((CWC_MODE4 == 3) ? CWC_PLL_MU : CWC_AFC_MU)
#define CWC_LK1 ((CWC_MODE1 == 3) ? CWC_PLL_LEAK : CWC_AFC_LEAK) 
#define CWC_LK2 ((CWC_MODE2 == 3) ? CWC_PLL_LEAK : CWC_AFC_LEAK)
#define CWC_LK3 ((CWC_MODE3 == 3) ? CWC_PLL_LEAK : CWC_AFC_LEAK) 
#define CWC_LK4 ((CWC_MODE4 == 3) ? CWC_PLL_LEAK : CWC_AFC_LEAK)
#define CWC_PLL ((CWC_MODE1 == 3) ? 1 : 0) | ((CWC_MODE2 == 3) ? 1 : 0)<<1 | ((CWC_MODE3 == 3) ? 1 : 0)<<2 | ((CWC_MODE4 == 3) ? 1 : 0)<<3

/**************************************************************************************
*Initial Values for the BADS_Internal_Params structure
***************************************************************************************/
#define INIT_BIG_EQ					 BADS_Internal_Params_eEnable      /*Enable/Disable Big Equalizer: Big EQ is 36/36 taps w/18 overlap taps, Big EQ is 18/24 taps w/6 overlap taps */
#define INIT_CWC					 BADS_Internal_Params_eEnable      /*Enable/Disable CWC cancellers*/
#define INIT_CFL                     BADS_Internal_Params_eEnable      /*Enable/Disable front carrier frequency loop*/
#define INIT_DDAGC                   BADS_Internal_Params_eEnable      /*Enable/Disable DD_AGC*/
#define INIT_IMC                     BADS_Internal_Params_eDisable     /*Enable/Disable IMC circuit if chip has an IMC*/
#define INIT_ACQUISITION_TEST        BADS_Internal_Params_eDisable     /*Enable/Disable internal acquisition percentage test with SPARE register*/

#if (BCHP_FAMILY==3128) || (BCHP_FAMILY==7584)
	#define INIT_VIDEO_CANCELLATION_ANNEX_B            BADS_Internal_Params_eEnable  /*Enable/Disable CWC at RF Freq - 1.75 MHz, the Tak special for WFE*/
#else
	#define INIT_VIDEO_CANCELLATION_ANNEX_B            BADS_Internal_Params_eDisable  
#endif

#define INIT_DUAL_SCAN            BADS_Internal_Params_eEnable /*Enable/Disable Dual Scan, Disable mean one fast scan, Enable for one fast scan then one slow scan*/

#if (BCHP_FAMILY==7584)
	#define INIT_FOI_TIMING           BADS_Internal_Params_eDisable /*THE FOI LOOP IS BROKEN IN THE 7584 A0*/
#else	
	#define INIT_FOI_TIMING           BADS_Internal_Params_eEnable /*Enable/Disable FOI Timing Detector: Disable mean transition tracker, Enable for FOI*/
#endif

#define INIT_CALLBACK_ENABLE      BADS_Internal_Params_eEnable /*Enable/Disable front end callback function*/

#define INIT_TIMING_SCAN_THRESHOLD   0x3000			           /*FFT Timing Scan Threshold: only 17 bits in chip*/
#define INIT_CARRIER_SCAN_THRESHOLD  0x3000                    /*FFT Carrier Scan Threshold: only 17 bits in chip*/

#ifdef __cplusplus
}
#endif

#endif