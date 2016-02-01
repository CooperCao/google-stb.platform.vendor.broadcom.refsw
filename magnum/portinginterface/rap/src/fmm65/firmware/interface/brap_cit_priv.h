/***************************************************************************
*     Copyright (c) 2006-2011, Broadcom Corporation
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
* $brcm_Revision: $
* $brcm_Date: $
* $brcm_Revision: $
* $brcm_Date: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*	This file contains contains code related to CIT module.
*
* Revision History:
*
* $brcm_Log: $
* 
  10/13/10 3:47p tthomas
* SW7550-601 : [7550] Adding CIT support for BTSC encoder
* 
* Hydra_Software_Devel/113   9/15/10 5:09p tthomas
* SW7468-298 : [7468] Merging the CIT changes for Video to main latest
* 
* Hydra_Software_Devel/111   8/10/10 1:25p tthomas
* SW7420-767: [7420] Merging the MS11 feature to main/latest
* 
* Hydra_Software_Devel/RAP_MS11_DEV/4   8/3/10 4:34p gautamk
* SW7420-767: [7420] Modifying user cfg size for MS11 usage modes
* 
* Hydra_Software_Devel/RAP_MS11_DEV/3   8/3/10 3:38p tthomas
* SW7420-767: [7420] unifying the MS decode mode enum
* 
* Hydra_Software_Devel/RAP_MS11_DEV/2   6/28/10 5:25p tthomas
* SW7420-767: [7420] Modifying the CIT PI interface
* 
* Hydra_Software_Devel/RAP_MS11_DEV/1   5/18/10 1:24p tthomas
* SW7405-4052: [7405] Adding MS11 Interface changes to branch
* 
* Hydra_Software_Devel/110   5/17/10 2:45p tthomas
* SW3548-2923: [3548] Adding the SRS Studio Support with Stages
* 
* Hydra_Software_Devel/109   5/12/10 1:59p tthomas
* SW7405-4322 : [7405]Adding DDRE Support and DV258 Support
* 
* Hydra_Software_Devel/108   5/10/10 10:57a tthomas
* SW3548-2923: [3548] Adding the SRS Studio Support
* 
* Hydra_Software_Devel/107   4/29/10 6:50p tthomas
* SW7405-3894 : [7405] Conditional Algo Support.. Adding API to return
* Audio Type name
* 
* Hydra_Software_Devel/105   4/19/10 5:51p tthomas
* SW7405-3492:[7405] Adding MS10 DD Decoder and MS10 Convert
* 
* Hydra_Software_Devel/104   4/12/10 7:36p tthomas
* SW7405-3492:[7405] Adding MS10 DD Decoder and MS10 Convert
* 
* Hydra_Software_Devel/103   4/7/10 6:55p tthomas
* SW7405-3492: [7405] Adding MS10 Support in CIT
* 
* Hydra_Software_Devel/102   3/11/10 3:31p tthomas
* SW3548-2816 : [3548] Making  DD Xcode as Encode
* 
* Hydra_Software_Devel/101   3/10/10 7:10a tthomas
* SW3543-1179 : [3548] Adding the CIT support for Mono Downmix
* 
* Hydra_Software_Devel/100   3/9/10 5:04p tthomas
* SW3548-2816 : [3548] Adding Support for FW Mixer and DD Xcode
* 
* Hydra_Software_Devel/99   3/4/10 7:13p tthomas
* SW3548-2809  : [3548] Adding support for BRCM 3D support
* 
* Hydra_Software_Devel/98   3/4/10 3:27a tthomas
* SW7405-3993 : [7405] Adding Support for Audyssey
* 
* Hydra_Software_Devel/97   1/25/10 2:29p tthomas
* SW7335-662 : [3548] Adding the static memory allocation of
* BRAP_CIT_P_BranchInfo
* 
* Hydra_Software_Devel/96   11/19/09 7:15p tthomas
* SW3556-832 : [3548,3556] Adding the "dynamic port switching" feature
* support.
* --> Addint the EXT Clock cfg to SPS also
* 
* Hydra_Software_Devel/95   11/19/09 5:24p tthomas
* SW3556-832 : [3548]Adding clock programming for EXT Clock port
* 
* Hydra_Software_Devel/94   11/11/09 7:18p tthomas
* SW3556-832 : [3548] PI -SPS (Seamless port Switching)API is finalised.
* SPS-API coding is completed and testing is done in MSVC test bench...
* Files are checking in after successful output verification
* 
* Hydra_Software_Devel/93   11/10/09 8:12p tthomas
* SW3556-832 : [3548] Making CIT-PI interface level to add Capture Port.
* Cap port is disabled in the FW interface. New API functions are added
* 
* Hydra_Software_Devel/92   9/4/09 10:11a tthomas
* SW7405-2967 : [7405] Adding support for DRA Decoder
* 
* Hydra_Software_Devel/90   7/2/09 3:30p tthomas
* PR55884 PR55887 : [7405] Adding AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1 support
* for SPDIF1 ports...
* 
* Hydra_Software_Devel/89   6/24/09 12:06p tthomas
* PR55884 PR55887 : [7405]3548 family chips do not have
* AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0. Making changes to accomodate this
* support in 3548.
* 
* Hydra_Software_Devel/88   6/23/09 1:38p tthomas
* PR55884 PR55887 : [7405] Merging NCO support  and Cstat-Hold in HWCBITS
* to main/latest
* 
* Hydra_Software_Devel/87   6/9/09 12:55p tthomas
* PR47682:[7405] Making Modificatios to support HWCBITS and FWBITS
* separately based on the flag available from PI...
* I have removed the selection of HWCBITS only when DDP is present....
* 
* Hydra_Software_Devel/RAP_DTA_7550/1   6/11/09 1:44p tthomas
* PR55884 : [7550]Adding NCO support  CIT Generation module
* 
* Hydra_Software_Devel/86   3/17/09 3:48p tthomas
* PR 52729: [3548] Merging only the cit_priv.h file. But not as the part
* of merging support for HBR mode is SPDIF 0. This is to avoid magnum
* build breaks...
* 
* Hydra_Software_Devel/85   3/16/09 10:01a tthomas
* PR47682:[7405] Adding Dolby volume PP
* 
* Hydra_Software_Devel/NEXTGEN_SPDIFHBR_BRANCH/1   3/13/09 5:22p tthomas
* PR 52729: [3548] Adding support for HBR mode is SPDIF 0
* 
* Hydra_Software_Devel/84   3/10/09 9:57p tthomas
* PR47683: [7405] Merging 32Khz to 48Khz upsampling support for
* Ac3Encoder. sOpSamplingFrequencyMapLut is changed
* 
* Hydra_Software_Devel/83   3/10/09 2:21p tthomas
* PR47683: [7405] Updating the threshold to consider the DSOLA sample
* accumulation
* 
* Hydra_Software_Devel/82   2/10/09 10:04a tthomas
* PR47682: [3548]Modifying the SrsVolume Iq name to SrsTruVolume
* 
* Hydra_Software_Devel/81   2/4/09 4:22p tthomas
* PR48683: [7405]  Adding Master Slave configuration as output port
* feature to Main
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_MS_BRANCH/1   1/28/09 8:57p tthomas
* PR48683: [7405] Adding Master Slave configuration as output port
* 
* Hydra_Software_Devel/80   1/22/09 11:03a tthomas
* PR47683: [3548] Adding more comments
* 
* Hydra_Software_Devel/79   12/19/08 2:23p tthomas
* PR47683: [3548] Adding support for Dummy SRS VolIQ
* 
* Hydra_Software_Devel/78   12/16/08 2:50p tthomas
* PR47683:[7405] Merging support time base for Task
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_TIMEBASE/1   12/10/08 1:39p tthomas
* PR47683:[7405] Adding support time base for Task
* 
* Hydra_Software_Devel/77   12/10/08 10:31a tthomas
* PR47682:[3548] Adding support for dummy DTS decoder and Generic
* passthrough as a stage
* 
* Hydra_Software_Devel/76   12/1/08 12:28p tthomas
* PR43158: [7420] Adding complete FMM support for 7420
* 
* Hydra_Software_Devel/75   11/7/08 2:46p tthomas
* PR43158: [3548] Adding the initial code base to support the minimal
* delay... The interface between PI is finalysed and testedThere is no
* parameter change now
* 
* Hydra_Software_Devel/74   10/28/08 3:43p tthomas
* PR47683 : [7405] Adding appropriate comments
* 
* Hydra_Software_Devel/73   9/26/08 8:19p tthomas
* PR47327 :[7405] Adding support for Master-Slave PPM correction  and
* Threshold Value to consider DDP PT
* 
* Hydra_Software_Devel/72   9/20/08 10:04p tthomas
* PR 43454: [7405]Adding correction for IOBuffer memory allocation
* 
* Hydra_Software_Devel/71   9/17/08 8:12p sushmit
* PR 43454:[7405] Reducing Cit Input Structure Size. Need to move
* allocation runtime later based on no. of actual branches.
* 
* Hydra_Software_Devel/70   9/9/08 7:15p sushmit
* PR 43454: [7405]Updating RBUF Sizes.
* 
* Hydra_Software_Devel/69   9/8/08 5:24p tthomas
* PR46530: [7405] Adding the Independent Delay Support
* 
* Hydra_Software_Devel/NEXTGEN_INDEP_DELAY_BRANCH/2   9/8/08 5:12p tthomas
* PR46530: [7405] Adding the FMM fork sorting
* 
* Hydra_Software_Devel/NEXTGEN_INDEP_DELAY_BRANCH/1   9/8/08 11:21a tthomas
* PR46530: [7405] Adding the Independent Delay Support
* 
* Hydra_Software_Devel/68   9/8/08 9:47a tthomas
* PR46530: [7405] Merging the CIT memory optimization to the main thread
* 
* Hydra_Software_Devel/NEXTGEN_CIT_MEM_OPT/1   9/7/08 6:29p tthomas
* PR41726: [7405] Adding Memory Optimization for CIT
* 
* Hydra_Software_Devel/67   9/4/08 4:37p tthomas
* PR41726: [7405] Adding support for SRS-HD PP
* 
* Hydra_Software_Devel/66   8/22/08 3:56p tthomas
* PR41726: [7405] Modifying the distinct output
* (BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS) per stage to 5 from 2
* 
* Hydra_Software_Devel/65   8/22/08 2:30p tthomas
* PR41726: [7405] Modifying the distinct output
* (BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS) per stage to 3 from 2
* 
* Hydra_Software_Devel/63   8/11/08 11:29a tthomas
* PR 43454:[7405] Adding support for 1) DSOLA 2) DVDLPCM decode
* 3)  DVDLPCM IDS
* 
* Hydra_Software_Devel/62   7/23/08 3:33p tthomas
* PR41726: [7405] Implementing Branching concept in the Distinct port
* 
* Hydra_Software_Devel/61   7/21/08 3:29p tthomas
* PR41726: [7405] Adding suppport WMA pass through
* 
* Hydra_Software_Devel/60   7/18/08 3:00p tthomas
* PR41726: [7405] Adding suppport for PCM Router Stage
* 
* Hydra_Software_Devel/59   7/9/08 11:04a tthomas
* PR41726: [7405] Removing warning
* 
* Hydra_Software_Devel/58   7/4/08 4:35p tthomas
* PR41726: [7405] Adding support for PPM Adaptive Rate Block
* 
* Hydra_Software_Devel/57   6/30/08 2:50p tthomas
* PR41726: [7405] Adding support Fork based matrixing. Only CIT
* implementation and the PI interface is complete
* 
* Hydra_Software_Devel/56   6/19/08 3:50p tthomas
* PR41726: [7405] Adding support for PES Based PCM Passthrough
* 
* Hydra_Software_Devel/55   6/10/08 10:05a tthomas
* PR41726: [7405] Modifying the CBIT configuraton for 3548 Family chips
* 
* Hydra_Software_Devel/53   5/29/08 3:27p gautamk
* PR41726: [7405] Adding function to insert/destroy stages internally in
* Task network.
* 
* Hydra_Software_Devel/52   5/29/08 2:59p tthomas
* PR41726: [7405] Adding Calculate Threshold and Block time function
* 
* Hydra_Software_Devel/51   5/29/08 9:10a tthomas
* PR41726: [7405] Correcting spelling mistake in AD PAN stage ID
* 
* Hydra_Software_Devel/50   5/27/08 8:34p tthomas
* PR41726: [7405] Adding support for SRC and Audio Descriptor
* 
* Hydra_Software_Devel/49   5/26/08 7:03p tthomas
* PR41994: [7405] Adding support for MAI Port: Tested and working
* 
* Hydra_Software_Devel/47   5/8/08 3:02p tthomas
* PR41994: [7405] Enabling the CIT Private defines and removing the od
* port Configuration
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_CIT_IS_SUPPORT/3   5/8/08 12:18p tthomas
* PR41994: [7405] Enabling the CIT Private defines and removing the od
* port Configuration
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_CIT_IS_SUPPORT/2   5/7/08 5:52p tthomas
* PR41994: [7405] Adding new generic IS: Tested and Added input and
* output port connectivity log
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_CIT_IS_SUPPORT/1   5/5/08 3:22p tthomas
* PR41994: [7405] Adding new Generic IS Support and MUlti PLL
* 
* Hydra_Software_Devel/46   5/7/08 9:34a tthomas
* PR 38950: [7405] Adding BRAP_3548_FAMILY  defines and MutiPLL and
* Single PLL conditional support
* 
* Hydra_Software_Devel/45   5/5/08 3:02p tthomas
* PR41994: [7405] Adding new EncAudioType, eDTSBroadcast
* 
* Hydra_Software_Devel/44   4/29/08 1:55p tthomas
* PR41994: [7405] Adding Support for the 3548 and 3556 Chip RDB
* 
* Hydra_Software_Devel/43   4/24/08 5:45p tthomas
* PR41994: [7405] Support for PPM in CIT Gen
* 
* Hydra_Software_Devel/42   4/24/08 2:56p tthomas
* PR41994: [7405] Removing the warnings from CIT Generation module for
* VXworks
* 
* Hydra_Software_Devel/41   4/24/08 2:34p tthomas
* PR41994: [7405] Removing the warnings from CIT Generation module for
* VXworks
* 
* Hydra_Software_Devel/40   4/21/08 6:21p gautamk
* PR41726: [7405] IMplementing review comments start/stop APis
* 
* Hydra_Software_Devel/39   4/10/08 7:59p tthomas
* PR 34648: [7405] Adding support for the HBR Flag, HBR RDB for DDP
* Passthru and Muti Pll port support
* 
* Hydra_Software_Devel/37   4/8/08 10:11a nitinb
* PR 39989: [7405] Added support for DDP pass through
* 
* Hydra_Software_Devel/36   4/1/08 3:23p tthomas
* PR 34648: [7405] Adding support for the DD Convert
* 
* Hydra_Software_Devel/35   3/20/08 2:46p tthomas
* PR34648 : [7405] Adding BERR_ENTER and BERR_LEAVE for al functions
* 
* Hydra_Software_Devel/34   3/14/08 1:57p tthomas
* PR34648 : [7405] Including the FMM port initialization for Zero port
* cfg and Define changes w.r.t PI
* 
* Hydra_Software_Devel/33   3/6/08 11:02a tthomas
* PR34648: [7405] Introducing PI System Error codes
* 
* Hydra_Software_Devel/32   3/5/08 10:30a tthomas
* PR34648: [7405] Removing the convertion of Invalid addresses to Offset.
* 
* Hydra_Software_Devel/31   3/3/08 2:49p gautamk
* PR34648: [7405] Fixing coverity defects.
* 
* Hydra_Software_Devel/30   2/29/08 4:22p gautamk
* PR34648: [7405] Changing file names includes
* 
* Hydra_Software_Devel/29   2/29/08 3:53p gautamk
* PR34648: [7405] Updating array size
* 
* Hydra_Software_Devel/28   2/29/08 11:02a tthomas
* PR34648: [7405]:  Adding the CIT Review comments : Completed Phase1
* part of review comments and coverity tool results
* 
* Hydra_Software_Devel/NEXGEN_MAGNUM_CIT_REVIEW_BRANCH/3   2/28/08 6:38p tthomas
* PR34648: [7405]: Adding the CIT Review comments : Completed Phase1 part
* of review comments and coverity tool results
* 
* Hydra_Software_Devel/NEXGEN_MAGNUM_CIT_REVIEW_BRANCH/2   2/28/08 1:11p tthomas
* PR34648: [7405]: Adding the CIT Review comments : Naming Convention
* Changes
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_TASK_RDB_CFG_BRANCH/3   2/26/08 7:40p tthomas
* PR34648: [7405]: Adding the MultiPLL PortCfg Support
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_TASK_RDB_CFG_BRANCH/2   2/20/08 5:08p tthomas
* PR34648: [7405]: Adding the port Enable enums
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_TASK_RDB_CFG_BRANCH/1   2/20/08 11:25a tthomas
* PR34648: [7405]: Adding support for the RDB Offsets
* 
* Hydra_Software_Devel/25   2/19/08 4:16p tthomas
* PR34648: [7405]: Removing the warnings
* 
* Hydra_Software_Devel/24   2/18/08 4:28p tthomas
* PR34648: [7405]: Merging to the main branch to add support for feneric
* Port Configuration
* 
* Hydra_Software_Devel/23   2/18/08 4:18p sushmit
* PR 34648: [7405] Reverting programming of bPcmOnSpdif boolean
* 
* Hydra_Software_Devel/22   2/18/08 2:59p sushmit
* PR34648: [7405] PI Changes for OpPortConfig to CIT Input
* 
* Hydra_Software_Devel/21   2/18/08 12:43p tthomas
* 
* PR34648: [7405]: Merging to the main branch for adding the AlgoId
* Support
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_TASK_ALGOID_CFG_PASSTHRU_BRANCH/1   2/15/08 11:52p tthomas
* PR34648: [7405]:  Adding support for Algo ID for every node
* configuration for generic pass thru
* 
* Hydra_Software_Devel/20   2/15/08 8:31p tthomas
* PR34648: [7405]: Spelling Mistake
* BRAP_CITGEN_P_TASK_PORT_CONFIG_MEM_SIZE
* 
* Hydra_Software_Devel/19   2/15/08 8:20p tthomas
* PR34648: [7405]: Modification for CIT Input Port Confg
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_TASK_SWAP_SPDIF_UCFG_BRANCH/2   2/15/08 6:55p tthomas
* PR34648: [7405]: Modifying the CIT Input for the Port Enables
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_TASK_SWAP_SPDIF_UCFG_BRANCH/1   2/15/08 3:58p tthomas
* PR34648: [7405]: Adding TASK SWAP  and SPDIF User CFG Support
* * Hydra_Software_Devel/18   2/14/08 1:41p tthomas
* Hydra_Software_Devel/18   2/14/08 1:41p tthomas
* PR34648: [7405]: Adding 1) Zero Fill Configuration 2) Port
* Configuration 3) Gateopen feature to CIT Gen module
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_CIT_PORT_CONFIG/3   2/13/08 10:44p tthomas
* PR34648: [7405]: Adding Zero fills feature to CIT Gen module
* 
* Hydra_Software_Devel/NEXTGEN_MAGNUM_CIT_PORT_CONFIG/2   2/13/08 08:05p tthomas
* PR34648: [7405]: Adding the Gate open configuration
*
* Hydra_Software_Devel/NEXTGEN_MAGNUM_CIT_PORT_CONFIG/2   2/12/08 03:13p tthomas
* PR34648: [7405]: Adding port configuration
*
* Hydra_Software_Devel/15   1/31/08 5:22p gautamk
* PR34648: [7405] Removing BRAP_DSPCHN_DecodeMode_eSimulMode Enum
* 
* Hydra_Software_Devel/14   1/30/08 3:05p gdata
* PR34648: [7405] Adding support for status buffers
* 
* Hydra_Software_Devel/13   1/17/08 5:07p sushmit
* PR34648: [7405] AC3 Bringup Changes
* 
* Hydra_Software_Devel/12   1/15/08 11:59a sushmit
* PR34648: [7405] Checkin in updated code
* 
* Hydra_Software_Devel/11   1/11/08 6:39p gautamk
* PR34648: [7405] Files for Mapping Processing network to CIT input
* 
* Hydra_Software_Devel/8   1/3/08 2:20p bhanus
* PR34648: [7405] Adding changes for Channel Audio Processing network for
* CIT module.
* - Changing unsigned int variables to uint32_t
* 
* Hydra_Software_Devel/7   12/31/07 3:02p nitinb
* PR34648: [7405] Fixed compilation error
* 
* Hydra_Software_Devel/6   12/28/07 9:54p sushmit
* PR 34648: [7405] Initial Checkins after DSP Bootup completion.
* 
* Hydra_Software_Devel/5   12/14/07 1:57p gautamk
* PR34648:  [7405] Adding updated files for downloading FW module.
* 
* Hydra_Software_Devel/4   12/13/07 3:11p nitinb
* PR34648: [7405] Updated CIT module interface
* 
* Hydra_Software_Devel/3   11/21/07 4:55p nitinb
* PR34648: [7405] Updated CIT module interface
* 
* Hydra_Software_Devel/2   11/16/07 1:20p nitinb
* PR34648: [7405] Added initial CIT module interface
* 
* Hydra_Software_Devel/1   11/16/07 10:47a nitinb
* PR34648: [7405] Files for CIT module
* 
***************************************************************************/
#ifndef _BRAP_CIT_PRIV_H__
#define _BRAP_CIT_PRIV_H__

#include "brap_af_priv.h"
#include "berr.h"

/* Test Bench Emulation Defines*/
/*-----------------------------*/
#ifdef TESTBENCH_BUILD	

	typedef unsigned long  BRAP_CIT_P_Boolean;		/* MSVC Boolean is long */
	#define BRAP_CITGEN_P_False					((uint32_t)0)	/* MSVC False is 0 */
	#define BRAP_CITGEN_P_True					((uint32_t)1)	/* MSVC False is 1 */
	#define BRAP_MAX_PP_PER_BRANCH_SUPPORTED	((uint32_t)7)
	#define BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED (BRAP_MAX_PP_PER_BRANCH_SUPPORTED+1)
	#define BRAP_MAX_PP_BRANCH_SUPPORTED		((uint32_t)3)
	#define BDBG_ENTER(x)						
	#define BDBG_LEAVE(x)						
	#define BDBG_ASSERT(x)	
	#define BSTD_UNUSED(x)				

#else

	#include "brap_dspchn.h"
    #include "brap.h"
	typedef bool BRAP_CIT_P_Boolean;
	#define BRAP_CITGEN_P_False false
	#define BRAP_CITGEN_P_True	true

#endif

/***************************************/
/*	 CIT GENERATION CODE DEFINES	   */
/***************************************/
#define BRAP_CIT_P_ONLY_ONE_PLL_SUPPORT
#define BRAP_CIT_P_ENABLE_FORK_MATRIXING
#define ANALYZE_IO_CFG 	
#define ENABLE_LOW_DELAY_API				/*Enabling Low delay code*/
#define BRAP_CIT_P_NEWSTATIC_MEMORY_ALLOC

/***************************************/
/*	 CIT GENERATION DEBUG CODE DEFINES */
/***************************************/

#define BRAP_CIT_P_CHECK_INPUT_RANGE
#define BRAP_CIT_P_CHECK_OUTPUT_RANGE
#define BRAP_CIT_P_PRINT_PPM_CFG

/***************************************/
/*		 Definition of SIZEOF		   */
/***************************************/
#ifdef ZSIM_PLATFORM
	#define SIZEOF(x)	( sizeof(x) * 2 )
#else
	#define SIZEOF(x)	( sizeof(x) )
#endif

/***************************************/
/*		 Definition of MAX 		   */
/***************************************/
#define BRAP_CIT_P_MAX_VALUE(a,b)	(a>b) ? (a) : (b)


/***************************************/
/*	 FIRMWARE TASK CONFIGURATION	   */
/***************************************/

/*	This value should be same as that of BRAP_AF_P_MAX_IP_FORKS */
#define BRAP_P_MAX_FW_STG_INPUTS					((uint32_t)3)
#define BRAP_P_MAX_FW_STG_OUTPUTS					((uint32_t)4)
#define BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS			((uint32_t)5)	/*This is based on the maximum Port Type ever possible*/
													
#define BRAP_P_MAX_FW_STG_PER_FW_BRANCH				((uint32_t)(BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED)) /* Max No of PP Stage + 1 Decode Stage *//*		((uint32_t)7)*/
#define BRAP_P_MAX_FW_BRANCH_PER_FW_TASK			((uint32_t)BRAP_MAX_PP_BRANCH_SUPPORTED)
#define BRAP_P_MAX_IS_BUFFER_PER_FW_TASK			((uint32_t)BRAP_P_MAX_FW_BRANCH_PER_FW_TASK + 1)
#define BRAP_P_MAX_NODES_PER_FW_STG					((uint32_t)5)/*to be removed */
#define BRAP_P_MAX_NODES_IN_TASK					((uint32_t)20)/*to be removed */
#define BRAP_P_MAX_OUTPUTS							((uint32_t)8)
/*#define BRAP_P_MAX_INDEPENDENT_DELAY				((uint32_t)300)*/
#define BRAP_P_MAX_ALGOS_IN_TASK					((uint32_t)( BRAP_P_MAX_FW_BRANCH_PER_FW_TASK * \
																 BRAP_P_MAX_FW_STG_PER_FW_BRANCH ))


/* STATIC MEMORY ALLOCATION FOR A TASK */

#define BRAP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES		((uint32_t)(2048*4)) /* Task Swap Buffer size in bytes */
#define BRAP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE			((uint32_t)((((SIZEOF(BRAP_AF_P_sFMM_DEST_CFG)*BRAP_AF_P_MAX_NUM_PLLS + 20)+3)>>2)<<2))	/* Task's output port configuration memory size in bytes */
#define BRAP_CIT_P_TASK_SPDIF_USER_CFG_MEM_SIZE			((uint32_t)((((SIZEOF(BRAP_AF_P_sSPDIF_USER_CFG)*BRAP_AF_P_MAX_NUM_SPDIF_PORTS + 20) +3)>>2)<<2)) /* Task's SPDIF user configuration memory size in bytes for all ports*/
#define BRAP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG			((uint32_t)((((SIZEOF(BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG) + 20)+3)>>2)<<2)) /* FMM gate open configuration memory size in bytes*/
#define BRAP_CIT_P_TASK_HW_FW_CONFIG					((uint32_t)((((SIZEOF(BRAP_AF_P_sFW_HW_CFG) + 20)+3)>>2)<<2)) 
#define BRAP_CIT_P_TASK_FS_MAPPING_LUT_SIZE				((uint32_t)((((SIZEOF(BRAP_AF_P_sOpSamplingFreq))+3)>>2)<<2)) 


/*  This is the extra memory that the PI need to allocate 
	in the task memory location other than the memory 
	estimated from brap_af_priv.c file
*/
#define BRAP_CIT_P_TOTAL_TASK_CFG_MEMORY				((uint32_t)(BRAP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES + \
																	BRAP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE	 + \
																	BRAP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG	 + \
																	BRAP_CIT_P_TASK_HW_FW_CONFIG			 + \
																	BRAP_CIT_P_TASK_FS_MAPPING_LUT_SIZE	))
/* Threshold specific defines */
#define BRAP_CIT_P_MINIMUM_ALGO_THRESHOLD				((uint32_t)(6144))	/*This is based in the DDP HBR and AAC QSF followed by SRC case */
#define BRAP_CIT_P_MAXIMUM_RESIDUAL_COLLECTION			((uint32_t)(1024))	/*This value is based on the DSOLA*/

/*----------------------------*/
/* CIT Module Private Defines */
/*----------------------------*/
#define BRAP_CIT_P_INVALID							((uint32_t)(0x7fffffff))
#define BRAP_CIT_P_NODE0							((uint32_t)(0))
#define BRAP_CIT_P_SRC0								((uint32_t)(0))
#define BRAP_CIT_P_DST0								((uint32_t)(0))
#define BRAP_CIT_P_NUM_SECIAL_NODES					((uint32_t)(2))
#define BRAP_CIT_P_TSM_NODE_INDEX					((uint32_t)(1))
#define BRAP_CIT_P_NUM_ALGO_NW_PASSES				((uint32_t)(2))
#define BRAP_CIT_P_INVALID_NODE_IDX					((uint32_t)(0xFFFFFFFF))
#define BRAP_CIT_P_PI_INVALID						((uint32_t)(-1))


#define BRAP_CIT_P_PRESENT							((uint32_t)(1))				
#define BRAP_CIT_P_ABSENT							((uint32_t)(0))				

#define BRAP_CIT_P_ASSOCIATED						((uint32_t)(1))				
#define BRAP_CIT_P_NOT_ASSOCIATED					((uint32_t)(0))				


/* Error Defines*/
/*--------------*/
/*	Need to define error codes for each module and each scenario */
#define BRAP_CIT_P_ERROR							((uint32_t)0xFFFFFFFF)
#define BRAP_AF_P_DRAM_ADDR_INVALID					((uint32_t)0x80000000)

/*---------------------------*/
/* Chip specific RDB defines */
/*---------------------------*/

#if((BCHP_CHIP == 3548)||(BCHP_CHIP == 3556))
	#define BRAP_3548_FAMILY    1
#else
	#define BRAP_3548_FAMILY    0
#endif

#if(BRAP_3548_FAMILY == 1)
	
	#define BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0	BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO	
	#define BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0	BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI
	#define BRAP_CIT_AUD_FMM_PLL0_MACRO						BCHP_VCXO_CTL_MISC_AC0_MACRO
	#define BRAP_CIT_AUD_FMM_PLL1_MACRO						BCHP_VCXO_CTL_MISC_AC1_MACRO
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0		(uint32_t)NULL
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1		(uint32_t)NULL
	#define BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_HBR0			(uint32_t)NULL
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0			(uint32_t)NULL
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1			(uint32_t)NULL

#else

	#define BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0	BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0	
	#define BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0	BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0
	#define BRAP_CIT_AUD_FMM_PLL0_MACRO						BCHP_AUD_FMM_PLL0_MACRO
	#define BRAP_CIT_AUD_FMM_PLL1_MACRO						BCHP_AUD_FMM_PLL1_MACRO
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0		BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1		BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1															
	#define BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_HBR0			BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_HBR0
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0			BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0
	#define BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1			BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1
	
#endif
/*-----------------------------------------------------------------------*/


#ifdef TESTBENCH_BUILD

	/* These defines are kept for compilation is MSVC */
	#define BCHP_AUD_FMM_PLL0_MACRO							(uint32_t)NULL
	#define BCHP_AUD_FMM_PLL1_MACRO							(uint32_t)NULL
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0		(uint32_t)NULL
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0		(uint32_t)NULL
	#define BCHP_HIFIDAC_RM0_PHASE_INC						(uint32_t)NULL
	#define BCHP_HIFIDAC_RM0_RATE_RATIO						(uint32_t)NULL
	#define BCHP_HIFIDAC_RM0_SAMPLE_INC						(uint32_t)NULL
	#define BCHP_HIFIDAC_CTRL0_RATEMGRCFG					(uint32_t)NULL
	#define BCHP_HIFIDAC_CTRL1_RATEMGRCFG					(uint32_t)NULL
	#define BCHP_HIFIDAC_CTRL2_RATEMGRCFG					(uint32_t)NULL
	#define BCHP_HIFIDAC_RM1_PHASE_INC						(uint32_t)NULL
	#define BCHP_HIFIDAC_RM1_RATE_RATIO						(uint32_t)NULL
	#define BCHP_HIFIDAC_RM1_SAMPLE_INC						(uint32_t)NULL
	#define BCHP_HIFIDAC_RM2_PHASE_INC						(uint32_t)NULL
	#define BCHP_HIFIDAC_RM2_RATE_RATIO						(uint32_t)NULL
	#define BCHP_HIFIDAC_RM2_SAMPLE_INC						(uint32_t)NULL

	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0			(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0				(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_FW_CBITS0					(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_FW_CBITS12					(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG0_MASK	(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1				(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_FW_CBITS24					(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_FW_CBITS36					(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG1_MASK	(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_ESR_STATUS						(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_ESR_STATUS_CLEAR				(uint32_t)NULL
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_HBR0				(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1			(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0			(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0				(uint32_t)NULL
	#define BCHP_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1				(uint32_t)NULL

	/* 3548  and 3556 Defines */
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO		(uint32_t)NULL
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI			(uint32_t)NULL
	#define BCHP_VCXO_CTL_MISC_AC0_MACRO					(uint32_t)NULL
	#define BCHP_VCXO_CTL_MISC_AC1_MACRO					(uint32_t)NULL	


	/* 7420 RDBS*/
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI			(uint32_t)NULL	
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO1		(uint32_t)NULL	

	/*7550 RDBs*/
	#define BCHP_AUD_FMM_OP_SPDIF_NCO_PHASE_INC				(uint32_t)NULL	
	#define BCHP_AUD_FMM_OP_SPDIF_NCO_RATE_RATIO			(uint32_t)NULL	
	#define BCHP_AUD_FMM_OP_SPDIF_NCO_SAMPLE_INC			(uint32_t)NULL	

	/*Capture */
	#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_BASE	(uint32_t)NULL	

#else	

	#define BCHP_AUD_FMM_MS_CTRL_FW_CBITS0                (BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE)
	#define BCHP_AUD_FMM_MS_CTRL_FW_CBITS1                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS0  + 4)
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS2                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS1  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS3                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS2  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS4                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS3  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS5                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS4  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS6                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS5  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS7                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS6  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS8                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS7  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS9                (BCHP_AUD_FMM_MS_CTRL_FW_CBITS8  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS10               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS9  + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS11               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS10 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS12               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS11 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS13               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS12 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS14               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS13 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS15               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS14 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS16               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS15 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS17               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS16 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS18               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS17 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS19               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS18 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS20               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS19 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS21               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS20 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS22               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS21 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS23               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS22 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS24               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS23 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS25               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS24 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS26               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS25 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS27               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS26 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS28               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS27 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS29               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS28 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS30               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS29 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS31               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS30 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS32               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS31 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS33               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS32 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS34               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS33 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS35               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS34 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS36               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS35 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS37               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS36 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS38               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS37 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS39               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS38 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS40               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS39 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS41               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS40 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS42               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS41 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS43               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS42 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS44               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS43 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS45               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS44 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS46               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS45 + 4) 
    #define BCHP_AUD_FMM_MS_CTRL_FW_CBITS47               (BCHP_AUD_FMM_MS_CTRL_FW_CBITS46 + 4) 
#endif

typedef enum
{
	eFALSE = 0,
	eTRUE	
} BOOLEAN;


/***************************************************************************
Summary:
	This enumeration for Selecting Shorting input for Decode 1st node and
	Fs Node.

	For Audio, the input of FS and Decoder first node are the same. But this
	is not applicable for Video

***************************************************************************/
typedef enum BRAP_CIT_P_SameInputForFsAndDecode
{
	BRAP_CIT_P_SameInputForFsAndDecode_eDisable = 0x0,		/* For video */
	BRAP_CIT_P_SameInputForFsAndDecode_eEnable, 			/* For audio */
	BRAP_CIT_P_SameInputForFsAndDecode_eMax,
   	BRAP_CIT_P_SameInputForFsAndDecode_eInvalid	= 0x7FFFFFFF

}BRAP_CIT_P_SameInputForFsAndDecode;

/*********************************************************************
Summary:
    Enums that describes the FMM Destination type 

Description:
    This enum describes the various FMM Destination types :
			Output Port/Capture Port
	
See Also:
	BRAP_CIT_P_FmmDstSelect.
**********************************************************************/ 
typedef enum BRAP_CIT_P_FmmDstSelect
{

	BRAP_CIT_P_FmmDstSelect_eOpPort,			/*Output Port type*/
	BRAP_CIT_P_FmmDstSelect_eCapturePort,		/*Capture Port Type */	
	BRAP_CIT_P_FmmDstSelect_eMax,			    /* Max value */
   	BRAP_CIT_P_FmmDstSelect_eInvalid = 0x7FFFFFFF          

}BRAP_CIT_P_FmmDstSelect;


/*********************************************************************
Summary:
    Enums that describes the FMM port Type : Master or Slave

Description:
    This enum describes the various the FMM port Type: Master or Slave.
	Used for Master slave configuration of FMM port
	
See Also:
	BRAP_CIT_P_EncAudioType.
**********************************************************************/ 
typedef enum BRAP_CIT_P_MasterSlaveType
{
	BRAP_CIT_P_MasterSlaveType_eMasterPort,			/* Master Port*/
	BRAP_CIT_P_MasterSlaveType_eSlavePort,			/* Slave Port  */	
	BRAP_CIT_P_MasterSlaveType_eOthers,				/* Other Ports */	
 	BRAP_CIT_P_MasterSlaveType_eMax,			    /* Max value */
   	BRAP_CIT_P_MasterSlaveType_eInvalid	= 0x7FFFFFFF          
} BRAP_CIT_P_MasterSlaveType;

/*********************************************************************
Summary:
    Enums that describes the various Encode algorithms.

Description:
    This enum describes the various encode algorithms supported. The 
	encode stage for the spplication is just like a post process. But 
	for the DSP, it is a different processing algorithm all together.
	To ensure functional separation, the PI will translate the processing
	type it receives from the application to the Encode audio type &
	processing audio type algorithms.

See Also:
	BRAP_CIT_P_ProcessingType.
**********************************************************************/ 

typedef enum BRAP_CIT_P_EncAudioType
{
	BRAP_CIT_P_EncAudioType_eMpeg1Layer3,			/* MPEG1 Layer 3 */
	BRAP_CIT_P_EncAudioType_eMpeg1Layer2,			/* MPEG1 Layer 2 */
	BRAP_CIT_P_EncAudioType_eDTS,					/* DTS */
 	BRAP_CIT_P_EncAudioType_eAacLc,					/* AAC-LC */
 	BRAP_CIT_P_EncAudioType_eAacHe,					/* AAC-HE */
	BRAP_CIT_P_EncAudioType_eAc3,					/* AC3 */
	BRAP_CIT_P_EncAudioType_eDTSBroadcast,			/* DTS Broadcast*/
	BRAP_CIT_P_EncAudioType_eSbc,					/* SBC Encoder*/
 	BRAP_CIT_P_EncAudioType_eMax,			        /* Max value */
   	BRAP_CIT_P_EncAudioType_eInvalid	= 0x7FFFFFFF          
} BRAP_CIT_P_EncAudioType;

/*********************************************************************
Summary:
    Enums that describes the various pre and post processing algorithms.

Description:
    This enum describes the various pre and post processing audio
	algorithms supported. 

See Also:
	BRAP_CIT_P_EncAudioType.
**********************************************************************/ 

typedef enum BRAP_CIT_P_ProcessingType
{
	BRAP_CIT_P_ProcessingType_eDdbm,			/* Dolby Digital Bass Management. */
    BRAP_CIT_P_ProcessingType_eDtsNeo,			/* DTS-Neo */     
	BRAP_CIT_P_ProcessingType_eAVL,				/* Automated Volume Level control. */
	BRAP_CIT_P_ProcessingType_eDDConvert,		/* DD Converter. */
	BRAP_CIT_P_ProcessingType_ePLll,			/* Dolby Prologic-II. */
	BRAP_CIT_P_ProcessingType_eSrsXt,			/* TruSurroundXT. */
	BRAP_CIT_P_ProcessingType_eXen,				/* XEN 3D Sound Processing. */
	BRAP_CIT_P_ProcessingType_eBbe,				/* BBE */
	BRAP_CIT_P_ProcessingType_eSrc,				/* Sample Rate Change */
	BRAP_CIT_P_ProcessingType_eCustomSurround,	/* CUSTOM Surround Algorithm */
	BRAP_CIT_P_ProcessingType_eCustomBass,		/* CUSTOM Bass Algorithm */	
	BRAP_CIT_P_ProcessingType_eCustomVoice,		/* CUSTOM Voice Algorithm */		
	BRAP_CIT_P_ProcessingType_ePeq,				/* PEQ Algorithm */		
	BRAP_CIT_P_ProcessingType_eDownmix0,		/* Downmix algorithm for AAC */	
	BRAP_CIT_P_ProcessingType_eAudioDescriptorFade,		/* AudioDescriptorFade */
	BRAP_CIT_P_ProcessingType_eAudioDescriptorPan,		/* AudioDescriptorPan */
	BRAP_CIT_P_ProcessingType_ePCMRouter,				/* PCM router */
	BRAP_CIT_P_ProcessingType_eWMAPassThrough,			/* WMA Passthrough */
	BRAP_CIT_P_ProcessingType_eDsola,					/* DSOLA */
	BRAP_CIT_P_ProcessingType_eSrsHd,					/* TruSurroundHD. */
	BRAP_CIT_P_ProcessingType_eGenericPassThru,			/* Generic pass through*/
	BRAP_CIT_P_ProcessingType_eSrsTruVolume,			/* SRS Tru Volume */			
	BRAP_CIT_P_ProcessingType_eDolbyVolume,				/* Dolby Volume */	
	BRAP_CIT_P_ProcessingType_eAudysseyVolume,			/* Audyssey Volume */
	BRAP_CIT_P_ProcessingType_eBrcm3DSurround,			/* Brcm 3D Surround  */
	BRAP_CIT_P_ProcessingType_eFWMixerPostProc,			/* FW Mixer */
	BRAP_CIT_P_ProcessingType_eMonoDownmix,				/* Mono DownMix */	
	BRAP_CIT_P_ProcessingType_eAudysseyABX,				/* Audyssey ABX  */	
	BRAP_CIT_P_ProcessingType_eDdre,					/* DDRE post processing  */
	BRAP_CIT_P_ProcessingType_eDv258,					/* DV258 post processing  */
	BRAP_CIT_P_ProcessingType_eSrsCsdTd,				/* SRS CS decoder and TruDialog  */
	BRAP_CIT_P_ProcessingType_eSrsGeq,					/* SRS Graphic equalizer  */
	BRAP_CIT_P_ProcessingType_eBtsc,					/* BTSC Encoder as PP  */		

			
	BRAP_CIT_P_ProcessingType_eMax,						/* Max value */
	BRAP_CIT_P_ProcessingType_eInvalid = 0x7FFFFFFF
} BRAP_CIT_P_ProcessingType;

/***************************************************************************
Summary:
	This enumeration defines various processing modes.

***************************************************************************/
typedef enum BRAP_CIT_P_ProcessingMode
{
	BRAP_DSPCHN_ProcessingMode_ePP = 0x0,		/* Post proc */
	BRAP_DSPCHN_ProcessingMode_eMax, 			/* Max value */
   	BRAP_DSPCHN_ProcessingMode_eInvalid	= 0x7FFFFFFF
} BRAP_CIT_P_ProcessingMode;

/***************************************************************************
Summary:
	This enumeration defines various Video Decoder modes.

***************************************************************************/
typedef enum BRAP_CIT_P_VideoDecodeMode
{
	BRAP_DSPCHN_VideoDecodeMode_eDecode = 0x0,	/* Decode */
	BRAP_DSPCHN_VideoDecodeMode_eMax, 			/* Max value */
   	BRAP_DSPCHN_VideoDecodeMode_eInvalid	= 0x7FFFFFFF
} BRAP_CIT_P_VideoDecodeMode;

/***************************************************************************
Summary:
	This enumeration for defining Master Task Detection Status	

***************************************************************************/
typedef enum BRAP_CIT_P_MixerTaskDetection
{
	BRAP_CIT_P_MixerTaskDetection_eNotDetected	= 0x0,	/* Not Detected  */
	BRAP_CIT_P_MixerTaskDetection_eDetected, 			/* Detected */
	BRAP_CIT_P_MixerTaskDetection_eMax,
   	BRAP_CIT_P_MixerTaskDetection_eInvalid	= 0x7FFFFFFF

}BRAP_CIT_P_MixerTaskDetection;

/*********************************************************************
Summary:
    Union to describe the Audio algorihtm running in the current stage.

Description:
    This union describes the Audio algorithm that is currently running
	in a stage of the algo network.

	We have added Video also here..

See Also:
**********************************************************************/ 
typedef union 
{
    BRAP_DSPCHN_AudioType		eDecAudioAlgo;
    BRAP_CIT_P_EncAudioType		eEncAudioAlgo;
    BRAP_CIT_P_ProcessingType   eProcAudioAlgo;
	BRAP_DSPCHN_GfxType     	eGfxAlgo;    
	BRAP_DSPCHN_VideoType		eVideoAlgo; 
	BRAP_DSPCHN_ScmType			eScmAlgo;

	
}BRAP_CIT_P_uAudioAlgorithm;

/*********************************************************************
Summary:
    Union to describe the Audio mode of the Algo running in the 
	current stage.

Description:
    This union describes the mode of the Audio algorithm that is 
	currently running in a stage of the algo network. for the decoder
	the mode could be decode or pass through mode. For an encoder, it
	could real time or Non real time mode. There is no specail mode
	for Post Proc Algos.

See Also:
**********************************************************************/ 
typedef union 
{
    BRAP_DSPCHN_DecodeMode		eDecAudioMode;
	BRAP_DSPCHN_EncodeMode		eEncAudioMode;
	BRAP_CIT_P_ProcessingMode	eProcessingAudioMode;
}BRAP_CIT_P_uAudioMode;

/*********************************************************************
Summary:
    Enum describing the type of the algo running on this stage.

Description:
    This enum describes the type of the algo running on this stage. This
	information is required to differentitate between decode, encode and
	post proc stages in the CIT generation module..


	We have added Video also here..

See Also:
**********************************************************************/ 
typedef enum BRAP_CIT_P_AudioAlgoType
{
	BRAP_CIT_P_AudioAlgoType_eDecode,
	BRAP_CIT_P_AudioAlgoType_eEncode,
	BRAP_CIT_P_AudioAlgoType_ePostProc,

	BRAP_CIT_P_GfxAlgoType_eDecode,		/* For Gfx on ZSP */
	BRAP_CIT_P_VideoAlgoType_eDecode,	/* For Video on ZSP */
	BRAP_CIT_P_ScmAlgoType_eDecode,		/* For SCM on ZSP */

	BRAP_CIT_P_AudioAlgoType_eMax,
	BRAP_CIT_P_AudioAlgoType_eInvalid = 0x7FFFFFFF
} BRAP_CIT_P_AudioAlgoType;


/*********************************************************************
Summary:
    Enumeration to describe type of source or destination for a FW stage

Description:
    This enumeration describes types of source/destiantion from/to 
	which a fw stage can receive/feed data.

See Also:
**********************************************************************/ 

typedef enum BRAP_CIT_P_FwStgSrcDstType
{
	BRAP_CIT_P_FwStgSrcDstType_eFwStg,				/* Source or destination is another FW stage */
	BRAP_CIT_P_FwStgSrcDstType_eRaveBuf,			
	BRAP_CIT_P_FwStgSrcDstType_eFMMBuf,			
	BRAP_CIT_P_FwStgSrcDstType_eRDB,	
	BRAP_CIT_P_FwStgSrcDstType_eDRAMBuf,		
	BRAP_CIT_P_FwStgSrcDstType_eInterTaskDRAMBuf,	/* The Dram Buffer Shared Across Multiple Tasks */
	BRAP_CIT_P_FwStgSrcDstType_eMax,
	BRAP_CIT_P_FwStgSrcDstType_eInvalid = 0x7FFFFFFF
} BRAP_CIT_P_FwStgSrcDstType;

/*********************************************************************
Summary:
    Structure to describe the Ring Buffer properties for Src or Dst 
	of a Stage.

Description:

See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_sIO_BUFF_ID
{
    uint32_t				ui32NumBuffers;				/*	Defines the number of 
															channels in the input 
															or output */
    BRAP_AF_P_BufferType    eBufferType;				/*	Defines the the location 
															of the input or output buffer. 
															This can take values defined 
															by eBUFFER_TYPE */

    uint32_t				ui32BufferId[BRAP_AF_P_MAX_CHANNELS]; 
														/* Valid for buffer types
                                                           BRAP_AF_P_eBufferType_RDB and
                                                           BRAP_AF_P_eBufferType_FMM */

	uint32_t				ui32AdaptRateCtrlIds[BRAP_AF_P_MAX_CHANNEL_PAIR]; /*Ids of Adapt rate ctrl block*/

}BRAP_CIT_P_sIO_BUFF_ID;

/*********************************************************************
Summary:
    Union to describe the IO Buffer properties for Src or Dst of a Stage.
Description:
    

See Also:
**********************************************************************/ 
/* Io Buffer*/
typedef union uIoBuff
{
    BRAP_AF_P_sIO_BUFFER			sIoBuffer;
    BRAP_CIT_P_sIO_BUFF_ID			sIoBuffId;

}uIoBuff;

/* Io generic Buffer*/
typedef union uIoGenereicBuff
{
	BRAP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffer;
	BRAP_CIT_P_sIO_BUFF_ID			sIoGenericBuffId;

}uIoGenereicBuff;

/*Combined Structure*/
typedef struct BRAP_CIT_P_IoBuf
{
	uIoBuff					uIoBuffer;			/*	If source or destination is
													IO buffer, then details of it */

	uIoGenereicBuff			uIoGenericBuffer;
												/*	If source or destination is
													IO buffer, then details of it */

												/*	Independent delay if the
													buffer is output buffer */
	uint32_t				ui32IndepDelay ;
	

	BRAP_AF_P_FmmDstFsRate	eFmmDstFsRate;		/* 
												   FMM Destination's associated sinking rate.
												   Like ui32IndepDelay, This parameter is associated only with OUTPUT FMM buffers .
												   Keep the value as Invalid for other cases.

												   Unlike ui32IndepDelay, eFmmDstFsRate will not change across Output ports in the
												   same Distinct output port.
												*/


} BRAP_CIT_P_IoBuf;


/*
	This structure is used to communicate the intertask DRAM buffer cfg.
	Here the DRAM Buffer will be allocated and configured in a master task
	and the handle will be passed to the slave task..

	Handle is passed from CIT(Master Task) -> PI and CIT (Slave Task).

	The slave task just passed the info to the DSP as the addr conversions
	are done in the Master Task.

*/
typedef struct BRAP_CIT_P_InterTaskDramBuffCfg       
{

	uint32_t	ui32IoBuffCfgAddr;			/* Io Buffer Config Address */
	uint32_t	ui32IoGenericBuffCfgAddr;	/* Io Generic Buffer Config Address */
	
}BRAP_CIT_P_InterTaskDramBuffCfg;


typedef struct BRAP_CIT_P_FwStage       
{
	uint32_t	ui32BranchId;
	uint32_t	ui32StageId;

	uint32_t	ui32DistinctOpNum;
	uint32_t	ui32OpNum;				/*	Id of the source stage output
											that is feeding data. 
											Valid only if FW stg is a source */
 
} BRAP_CIT_P_FwStage;					/*	If source or destination is
											another FW stage, then its
											branch id and stage id
										*/                                                             


/*********************************************************************
Summary:
    Structure for describing details of source or destination of a FW stage
Description:
    This structure describes details of a source or destiantion of a FW stage in a 
    network of stages of a firmware task.
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_FwStgSrcDstDetails
{
	BRAP_CIT_P_FwStgSrcDstType      eType;      /* Type of source or destination */
	
	union
	{
		BRAP_CIT_P_FwStage				sFwStage;	/*Stage Connectivity*/
		BRAP_CIT_P_IoBuf				sIoBuf;		/*Buffer Connectivity*/
		BRAP_CIT_P_InterTaskDramBuffCfg	sInterTaskDramBuffCfg; /* Intertask DRAM buff cfg*/
                                        
	} uDetails;

}BRAP_CIT_P_FwStgSrcDstDetails;

/*********************************************************************
Summary:
    Structure for describing connectivity of a FW stage with other stages
Description:

    This structure describes connectivity of a FW stage with other stages
	in the FW task as well as with input/output buffers.

See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_FwStgConnectivity
{
    uint32_t						ui32NumSrc; /*  Number of sources feeding data
													to this stage */

    BRAP_CIT_P_FwStgSrcDstDetails	sSrcDetails[BRAP_P_MAX_FW_STG_INPUTS];
												/*	Details of sources feeding data
													to this stage. Only first uiNumSrc
													number of entries of this array 
													are valid */

	uint32_t						ui32NumDistinctOp;
    uint32_t						ui32NumDst[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS];   
												/*	Number of destinations for each distinct o/p */

	BRAP_AF_P_DistinctOpType        eDistinctOpType[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS];
                                                /*	Op type for each distinct o/p */        
	
    BRAP_CIT_P_FwStgSrcDstDetails   sDstDetails[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS][BRAP_P_MAX_FW_STG_OUTPUTS];
                                                /*	Details of destinations receiving data
													from distinct outputs of this stage. 
													Only uiNumDst number of entries 
													of this array are valid. The first
													dimension of this array represents
													distinct outputs and second dimension
													of array represents stages that receive
													data from the corresponding output in
													first dimension. 
												*/
} BRAP_CIT_P_FwStgConnectivity;

/*********************************************************************
Summary:
    Structure that contains firmware stage information
Description:
    This structure contains information about a firmware stage. It 
	contains information like position of firmware stage (branch id, 
	stage id), connectivity of the stage with other stages in the 
	firmware task and algorithm running in the stage.

  Contents can be classified as : 
		1) Stage Identifiers
		2) Algorithm Configuration
		3) Connectivity

See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_FwStageInfo
{
    uint32_t					ui32BranchId;       /* Id of FW branch on which
													this stage is residing */

    uint32_t					ui32StageId;        /* Id of FW stage on FW
													branch */
	BRAP_CIT_P_AudioAlgoType	eAudioAlgoType;
    BRAP_CIT_P_uAudioAlgorithm	uAlgorithm;			/* Audio algorithm running
													in this stage */
	BRAP_CIT_P_uAudioMode		uAudioMode;

    BRAP_CIT_P_FwStgConnectivity	sStgConnectivity;/* Information on connectivity
													with other stages */

} BRAP_CIT_P_FwStageInfo;

/*********************************************************************
Summary:
    This structure contains information of a firmware branch

Description:
    This structure contains information of a firmware branch. It 
	also contains information of all the stages in a branch.

See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_FwBranchInfo
{
	uint32_t					ui32NumStages;

    BRAP_CIT_P_FwStageInfo      sFwStgInfo[BRAP_P_MAX_FW_STG_PER_FW_BRANCH];

} BRAP_CIT_P_FwBranchInfo;

/*********************************************************************
Summary:
    Structure to describe one MIT Entry

Description:
    This structure describes one entry of Master Index Table.

See Also:
**********************************************************************/ 
typedef struct BRAP_Mit_Entry
{
    uint32_t			ui32AlgoCodePtr;
    uint32_t			ui32AlgoCodeSize;
    uint32_t			ui32AlgoDataPtr;
    uint32_t			ui32AlgoDataSize;
}BRAP_Mit_Entry;

/*********************************************************************
Summary:
    Structure to describe Master Index Table
Description:
    This structure describes Master Index Table.
	Structure contains Code and LUT memory address and size in bytes.
	Populated by PI while opening device and channel.
	Used by CIT to populate Code and LUT specifics of CIT data structure
 

See Also:
**********************************************************************/ 
typedef struct BRAP_Mit
{
    BRAP_Mit_Entry	sAlgoDwnldDetail[BRAP_AF_P_AlgoId_eMax];

}BRAP_Mit;

/*********************************************************************
Summary:
    Memory information for CIT module
Description:
    This structure contains the memory information required by the CIT module.
    CIT module splits these memory chunks into various buffers. 
	
	Shared memory chunk is split by CIT module into following buffers

    - Scratch Buffer
    - Interstage Buffers
		
    Task memory is split by CIT module into following buffers
    - Interframe Buffers
    - Configuration Parameter Buffers
		- Status Buffer
	
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_MemInfo
{
    uint32_t            ui32SharedMemPhysAdr;   /*	Physical address of memory chunk
													that is split by CIT module into various
													buffers as required by FW task. This
													chunk is shared with other FW tasks
													running on same processor. */
    uint32_t            ui32SharedMemSize;      /*	Size of memory chunk
													that is split by CIT module into various
													buffers as required by FW task This
													chunk is shared between FW tasks
													running on same processor. */
    uint32_t            ui32TaskMemPhysAdr;     /*	Physical address of memory chunk
													that is split by CIT module into various
													buffers as required by FW task. This
													chunk is specific to a task. */
    uint32_t            ui32TaskMemSize;        /*	Size of memory chunk
													that is split by CIT module into various
													buffers as required by FW task. This
													chunk is specific to a task. */

} BRAP_CIT_P_MemInfo;


/*********************************************************************
Summary:
	SPDIF Port Configuration

Description:

    This structure describes the SPDIF port Configuration
	  
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_SpdifCfg
{
    BRAP_CIT_P_Boolean    bPcmOnSpdif;    /* True  = Data is Compressed,
                                             False = Data is PCM */

    BRAP_CIT_P_Boolean    bHbr;           /* True = HBR mode 
                                             False= Non HBR mode  */                                                                     

	BRAP_CIT_P_Boolean    bHwCbits;      /*  True when HwCbits cfg is required */

} BRAP_CIT_P_SpdifCfg;


/*********************************************************************
Summary:
	MAI port Configuration

Description:

    This structure defines the Mux selector for the MAI port
	  
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_MaiCfg
{
	BRAP_OutputPort		eMaiMuxSelector;/* The Mai Input Selector */
	BRAP_CIT_P_SpdifCfg sMaiContent;	/* The Mai Payload Format Cfg
										   PCM - Compresssed - HBR 
										*/	
}BRAP_CIT_P_MaiCfg;


/*********************************************************************
Summary:

	BRAP_CIT_P_uFmmDstPortType

Description:

	This is a union that describes the Dest Type specifics in detail.
	a) Output Port type specifics
	b) Cap port specifics

See Also:
**********************************************************************/ 

typedef union	BRAP_CIT_P_uFmmDstPortType
{

       BRAP_OutputPort			 eOpType;		/* The FMM output Port type */
	   BRAP_CapInputPort		 eCapPortType;	/* The FMM Capture Port type */

}BRAP_CIT_P_uFmmDstPortType;

/*********************************************************************
Summary:
	BRAP_CIT_P_sFmmDstDetails

Description:

	This is a structure to describe the Destination port select and
	its type in detail.

	eFmmDstSelect	: Selects Output Port	/ Cap port
	uFmmDstPortType : Describes Output Port	/ Cap port

See Also:
**********************************************************************/ 

typedef struct	BRAP_CIT_P_sFmmDstDetails
{
	BRAP_CIT_P_FmmDstSelect      eFmmDstSelect;		/* FMM Destination select*/   
	BRAP_CIT_P_uFmmDstPortType	 uFmmDstPortType;	/* FMM port type */

}BRAP_CIT_P_sFmmDstDetails;

/*********************************************************************
Summary:
	Port Configuration

Description:

    This structure describes the port Configuration

	HifiDac cfg: 

		 Even if the DAC is not configured with PLL,
				 it is expected that, CIT -PI interface needs an association 
				 btw HIFIDAC and PLL. This is done for simplicity in design.

				 Default PLL index for HIFIDAC when no other ports are available
				 in 0.			
				 The PLL index of HifiDac should be the same as that of other 
				 ports in the other case.				

		 If PLL index is not possible to give as default and there are
		 no other ports present in the system, PI can tell
		 BRAP_AF_P_NoneSelect and CIT will take care the rest.

See Also:
**********************************************************************/ 
typedef struct	BRAP_CIT_P_PortCfg
{

	   BRAP_AF_P_EnableDisable		ePortEnableDisableFlag;	/* Port Enable/Disable: Used for SPS */

	   BRAP_CIT_P_sFmmDstDetails	sFmmDstDetails;			/* FMM Destination Details */	   

															/*	This field will show whether the port is really enabled or not.
																CIT generation module doenot need to honour this flag as it can
																assume the ports are enabled at the channel open time.
															*/
	
	   BRAP_AF_P_PllNcoSelect		ePllNcoSelect;			/*	Flag to indicate whether
																port is associated with PLL or NCO */

       uint32_t						ui32PllIndex;			/*	Pll-index of a PLL associated a port */


	   BRAP_AF_P_FmmDstFsRate		eFmmDstFsRate;			/* FMM Destination's associated sinking rate */

       union
       {
            BRAP_CIT_P_SpdifCfg		sSpdifCfg;
			BRAP_CIT_P_MaiCfg		sMaiCfg; 

       } uOpCfg;

}BRAP_CIT_P_PortCfg;

/*********************************************************************
Summary:
	RDB Access Configuration

Description:

    This structure describes RDB Access Configuration
	The RDB address space used by PI is virtual address.

	The Physical RDB Address = Virtual_Addr + ui32RdbBaseOffset 
											+ ui32RdbDspOffset


See Also:
**********************************************************************/ 
typedef struct  BRAP_CIT_P_RdbCfg
{
	uint32_t	ui32RdbBaseOffset;

	uint32_t	ui32RdbDspOffset;

}BRAP_CIT_P_RdbCfg;

/*********************************************************************
Summary:
    Input data structure to CIT Generation module 

Description:
    This structure is input to CIT module and contains all the 
	information required by CIT module to prepare CIT structure.
	It contains
    1. Branch and Stage Information of audio algorithms 
    2. Memory to be used (Shared and Task memory)
    3. Master Index Table 
    4. Destination port (FMM and Capture) information
    5. Independent Delay 
	6. Dolby Usage modes
	7. RDB configuration

See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_InputInfo
{
	uint32_t				ui32NumBranches;
    BRAP_CIT_P_FwBranchInfo	*pBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];
												/* Information of FW branches in a FW
												   task */
    BRAP_Mit				*psMit;				/* Master Index Table */
	BRAP_CIT_P_MemInfo		sMemInfo;           /* Memory information */

	uint32_t                ui32NumOutputs;		/* Number of output ports for this task.
                                                   Only these many first entries in sPortCfg[]
                                                   array are valid. 
												*/

												/* Port Configuration*/
	BRAP_CIT_P_PortCfg		sPortCfg[BRAP_P_MAX_OUTPUTS];

										

	BRAP_CIT_P_RdbCfg		sRdbCfg;			/* RDB Access Configuration*/

	BRAP_AF_P_EnableDisable	eIndepDelayEnableFlag;
												/*BRAP_AF_P_eEnable  =  Independent delay enabled
												  BRAP_AF_P_eDisable =  Independent delay is disabled
												*/

	BRAP_AF_P_TimeBaseType	eTimeBaseType;		/* Time base type for a task 45Khz or 27 Mhz (Direct TV)
												*/

	BRAP_AF_P_DolbyMsUsageMode eDolbyMsUsageMode;
												/* This flag is required to modify the Frequency Map table */
				
} BRAP_CIT_P_InputInfo;


/*********************************************************************
Summary:
    Structure to describe the details of an interframe buffer for 
	a FW node

Description:
    This structure describes the details of an interframe buffer
	created in CIT module for a FW node.

See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_FwBufInfo
{
    BRAP_AF_P_AlgoId	eFwExecId;
    uint32_t			ui32InterframeBufAdr;
    uint32_t			ui32InterframeBufSize;
						
	uint32_t			ui32UserParamBufAdr;
    uint32_t			ui32UserParamBufSize;


	uint32_t			ui32StatusBufAdr;
    uint32_t			ui32StatusBufSize;
    
} BRAP_CIT_P_FwBufInfo;

/*********************************************************************
Summary:
    Structure to describe stage level auxiliary information generated 
	by CIT module
Description:
    This structure describes auxiliary information generated by CIT 
	module at stage level.
See Also:
**********************************************************************/

/*	WARNING!!! changed the user config info to an array of structures. 
	Need to communicate this to the PI team */
typedef struct BRAP_CIT_P_OpStgInfo
{
    BRAP_CIT_P_uAudioAlgorithm	uAlgorithm;			/*	Audio algorithm running
														in this stage */
	BRAP_CIT_P_AudioAlgoType	eAudioAlgoType;

	BRAP_CIT_P_uAudioMode		uAudioMode;

    uint32_t					ui32TsmNodeIndex;
    uint32_t					ui32StartNodeIndex; 														
    uint32_t					ui32NumNodes;
    BRAP_CIT_P_FwBufInfo		sFwOpNodeInfo[BRAP_AF_P_MAX_NUM_NODES_IN_ALGO];
} BRAP_CIT_P_OpStgInfo;

/*********************************************************************
Summary:
    Structure to describe branch level auxiliary information generated 
	by CIT module
Description:
    This structure describes auxiliary information generated by CIT
	module at branch level.
See Also:
**********************************************************************/

typedef struct BRAP_CIT_P_OpBranchInfo
{
	uint32_t					ui32NumStages;
    BRAP_CIT_P_OpStgInfo		sCitStgInfo[BRAP_P_MAX_FW_STG_PER_FW_BRANCH];

} BRAP_CIT_P_OpBranchInfo;

/*********************************************************************
Summary:
    Structure to describe output of CIT module
Description:
    This structure describes output of CIT module. Output of CIT module contains
    following 
    1. Completely initialized CIT structure
    2. Auxiliary information required by other  RAP PI modules to initilalize
        buffers created in CIT module
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_Output
{
    BRAP_AF_P_sTASK_CONFIG		sCit;			/* Cit Structure */
	
	uint32_t					ui32NumBranches;

    BRAP_CIT_P_OpBranchInfo		sCitBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];

	BRAP_AF_P_sDRAM_BUFFER		sStackSwapBuff;	/* Stack Swap Buffer */
	
	BRAP_AF_P_sDRAM_BUFFER		sSpdifUserConfigAddr[BRAP_AF_P_MAX_NUM_SPDIF_PORTS];

} BRAP_CIT_P_Output;

/*********************************************************************
Private structures of the CIT generation module
*********************************************************************/

/*The define is for IO buffer configuration memory size and is for both input and output */
#define BRAP_CIT_P_SIZEOF_IOBUFFCFG	(	SIZEOF(BRAP_AF_P_sIO_BUFFER) * 2 +\
										SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) * 2 )


/*********************************************************************
Summary:
    Enums that describes Input availability

Description:    
	Input present or absent

See Also:
	
**********************************************************************/ 

typedef enum
{
	BRAP_CIT_P_IP_STATUS_eNotAvaialble	= 0x0,
	BRAP_CIT_P_IP_STATUS_eAvaialble		= 0x1,
	BRAP_CIT_P_IP_STATUS_eMax
} BRAP_CIT_P_IP_STATUS;

/*********************************************************************
Summary:
    Structure to describe RDB Configuration
Description:

    This structure describes RDB offset Configuration
    1. CIT input Rdb cfg
    2. Final RDB offset
        
See Also:
**********************************************************************/ 

typedef struct BRAP_CIT_P_RdbConfig
{
	uint32_t				ui32TaskRdbOffset;
	BRAP_CIT_P_RdbCfg		sRdbCfg;

}BRAP_CIT_P_RdbConfig;

/*********************************************************************
Summary:
    Structure to describes all atributes of a Node
Description:
    Provides all information for the node to execute
        
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_sNodeAttr
{
	BRAP_AF_P_AlgoId				eAlgoId;

	/*	This is required as the Decode task need to know for taking
		Residue samples in consideration 
	*/
	uint32_t						ui32BranchId;	

	uint32_t						ui32AudioAlgorithmType;

	uint32_t						ui32CodeDramAddress;
	uint32_t						ui32CodeSize;

	uint32_t						ui32RomTableDramAddress;
	uint32_t						ui32RomTableSize;

	uint32_t						ui32InterFrmBuffDramAddress;
	uint32_t						ui32InterFrmBuffSize;

	uint32_t						ui32FwStatusBuffDramAddress;
	uint32_t						ui32FwStatusBuffSize;

	uint32_t						ui32InterStgIoBuffSize;
	uint32_t						ui32InterStgGenericBuffSize;

	uint32_t						ui32ScratchBuffDramAddress;
	uint32_t						ui32ScratchBuffSize;

	uint32_t						ui32UsrCfgBuffDramAddress;
	uint32_t						ui32UsrCfgBuffSize;

	uint32_t						ui32MaxSizePerChan;
	uint32_t						ui32MaxNumChansSupported;
	
	BRAP_AF_P_InterFrameBuffType	eInterFrameBuffType;

	BRAP_AF_P_FwStatus				eFwStatusBuffType;

	/*The stage output configuration. Added to sort the Output ports */
	uint32_t						ui32NumDistinctOp;
    uint32_t						ui32NumPortsPerDistOp[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS];   
	
	uint32_t						ui32NumSrc;
	uint32_t						ui32NumDst;

	uint32_t						ui32ModifiedNumDst;/*After removing redundant Interstage DST outputs*/


	
	uint32_t						ui32NodeIpBuffCfgAddr[BRAP_P_MAX_FW_STG_INPUTS];
	uint32_t						ui32NodeIpGenericDataBuffCfgAddr[BRAP_P_MAX_FW_STG_INPUTS];

	/* Valid/Invalid Flag for the Input ports */
	BRAP_AF_P_ValidInvalid			eNodeIpValidFlag[BRAP_AF_P_MAX_IP_FORKS];						

										
	uint32_t						ui32NodeOpBuffCfgAddr[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS][BRAP_P_MAX_FW_STG_OUTPUTS];	
	uint32_t						ui32NodeOpGenericDataBuffCfgAddr[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS][BRAP_P_MAX_FW_STG_OUTPUTS];

	/* The data type of the output buffers of a node*/
	BRAP_AF_P_DistinctOpType		eNodeOpBuffDataType[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS][BRAP_P_MAX_FW_STG_OUTPUTS];	

	/*Stage Output connectivity*/
	BRAP_CIT_P_FwStgSrcDstType		eConnectivityType[BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS][BRAP_P_MAX_FW_STG_OUTPUTS];	

} BRAP_CIT_P_sNodeAttr;


/*********************************************************************
Summary:
    Structure to describes the Network Structure of all stages and 
	Branches in a task.

Description:
    Gives the complete Network details for a task
        
See Also:
**********************************************************************/ 

/* Stage information */
typedef struct BRAP_CIT_P_StageInfo
{
    uint32_t						ui32BranchId;   /*	Id of FW branch on which
														this stage is residing */

    uint32_t						ui32StageId;    /*	Id of FW stage on FW
														branch */
	BRAP_CIT_P_AudioAlgoType		eAudioAlgoType;
    BRAP_CIT_P_uAudioAlgorithm		uAlgorithm;     /*	Audio algorithm running
														in this stage */
	BRAP_CIT_P_uAudioMode			uAudioMode;
    BRAP_CIT_P_FwStgConnectivity	sStgConnectivity;
													/*	Information on connectivity
														with other stages */

	int32_t							bIpsFromFwStage; 
													/*	Indicates that the inputs are 
														from f/w stage only. In this case,
														special nodes are absent */


	uint32_t						ui32NumNodes;

	BRAP_CIT_P_sNodeAttr			sNodeAttr[BRAP_AF_P_MAX_NUM_NODES_IN_ALGO];

	uint32_t						ui32InterStageBuffIdx;


} BRAP_CIT_P_StageInfo;

/*--------------------*/
/* Branch information */
/*--------------------*/
typedef struct BRAP_CIT_P_BranchInfo
{
	uint32_t						ui32NumStages;

    BRAP_CIT_P_StageInfo			sStgInfo[BRAP_P_MAX_FW_STG_PER_FW_BRANCH];

} BRAP_CIT_P_BranchInfo;


/*--------------------------*/
/* Task network information */
/*--------------------------*/
typedef struct BRAP_CIT_P_AlgoNwInfo
{
	uint32_t						ui32NumBranches;   

	BRAP_CIT_P_BranchInfo			*psBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];

} BRAP_CIT_P_AlgoNwInfo;


/*********************************************************************
Summary:
    This datastructure is the basic structure to describe the stage 
	execution
Description:
	1) BranchId
	2) StageId
        
See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_sAlgoExecOrder
{
    uint32_t                ui32BranchId;
    uint32_t                ui32StageId;

} BRAP_CIT_P_sAlgoExecOrder;

/*********************************************************************
Summary:
    This datastructure gives the memory requirement details for a task
	execution
Description:
	Memory requirement for a task        
See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_sTaskBuffInfo
{
    uint32_t                ui32TaskInterFrmMemSize;
    uint32_t                ui32TaskUsrCfgMemSize;
    uint32_t                ui32TaskIoBuffCfgStructMemSize;
    uint32_t                ui32TaskScratchMemSize;

    uint32_t                ui32BranchInterStgIoMemSize;
    uint32_t                ui32BranchInterStgGenericMemSize;

    uint32_t                ui32TaskInterStgIoMemSize;
    uint32_t                ui32TaskInterStgGenericMemSize;
	uint32_t				ui32NumInterStgBuffs;

	uint32_t				ui32MaxSizePerChannel;
	uint32_t				ui32MaxNumChannelsSupported;
	
	uint32_t                ui32TaskFwStatusBuffMemSize;
	uint32_t                ui32TaskStackMemSize;
	
	uint32_t                ui32TaskPortConfigMemSize;
	uint32_t                ui32TaskSPDIFConfigMemSize;
	
	uint32_t                ui32TaskGateOpenConfigMemSize;

	uint32_t                ui32TaskHwFwCfgMemSize;

	uint32_t				ui32SamplingFrequencyMapLutSize;


} BRAP_CIT_P_sTaskBuffInfo;

/*********************************************************************
Summary:
    This datastructure describes Io Buffer and IO genreic buffer 
	configuration
Description:
	1) Io Buffer and
	2) Io generic Buffer
        
See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_sBuffCfg
{
	uint32_t						ui32IoBufferStructAddr;
	BRAP_AF_P_sIO_BUFFER			sIoBuffer;

	uint32_t						ui32GenericBufferStructAddr;
	BRAP_AF_P_sIO_GENERIC_BUFFER	sGenericBuffer;


	uint32_t						ui32PortAssociated;
	uint32_t						ui32StageAssociated;

} BRAP_CIT_P_sBuffCfg;


/*********************************************************************
Summary:
    This datastructure describes Interstage buffer configuration of a
	Task
Description:
	Task's Interstage Configuration	
        
See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_TaskIsBuffCfg
{
	/*	This array holds the I/O buff cfg structures inter-stage buffers */
	BRAP_CIT_P_sBuffCfg	sIsBuffCfg[BRAP_P_MAX_IS_BUFFER_PER_FW_TASK];

} BRAP_CIT_P_TaskIsBuffCfg;


typedef struct BRAP_CIT_P_sOpPortInfo
{
	uint32_t						ui32NumopPorts;
	BRAP_AF_P_sOP_PORT_CFG			sOpPortCfg[BRAP_AF_P_MAX_OP_PORTS];

} BRAP_CIT_P_sOpPortInfo;


/*********************************************************************
Summary:
    This datastructure describes the SPDIF configuration for a PLL
Description:
	1) Spdif Enable and
	2) Contents of SPDIF
        
See Also:
**********************************************************************/

typedef struct BRAP_CIT_P_sSpdifInputCfg
{
	uint32_t			  ui32SpdifEnabled;

	BRAP_CIT_P_Boolean    bPcmOnSpdif;	

	BRAP_CIT_P_Boolean    bHbr;                 
	
	BRAP_CIT_P_Boolean    bHwCbits;

}BRAP_CIT_P_sSpdifInputCfg;


/*********************************************************************
Summary:
    This datastructure describes the MAI configuration for a PLL
Description:
	1) Mai port Enable
	2) Mai Payload content and Mux Type	
        
See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_sMaiInputCfg
{
	uint32_t			ui32MaiEnabled;
	BRAP_CIT_P_MaiCfg	sMaiCfg; 

}BRAP_CIT_P_sMaiInputCfg;


/*********************************************************************
Summary:
    This datastructure describes the IOP port Control information.
	This datastructure is the top control structure for DAC/I2S/CAP
Description:
	1) Port Enable
	2) Port Frequency Rate	
        
See Also: BRAP_CIT_P_sSpdifPortInfo

**********************************************************************/
typedef struct BRAP_CIT_P_sPortControlInfo
{
	uint32_t				ui32PortEnabled;

	BRAP_AF_P_FmmDstFsRate	eFmmDstFsRate;	/* FMM Destination's associated
											   sinking rate
											*/
}BRAP_CIT_P_sPortControlInfo;

/*********************************************************************
Summary:
    This datastructure describes the IOP port Control information for
	SPDIF port
	.
	
Description:
	1) Port Enable
	2) Port Frequency Rate	
    3) SPDIF data configuration
    
See Also:BRAP_CIT_P_sMaiPortInfo
**********************************************************************/
typedef struct BRAP_CIT_P_sSpdifPortInfo
{
	BRAP_CIT_P_sPortControlInfo		sSpdifPortControlInfo;

	BRAP_CIT_P_sSpdifInputCfg		sSpdifInputCfg;

}BRAP_CIT_P_sSpdifPortInfo;

/*********************************************************************
Summary:
    This datastructure describes the IOP port Control information for
	MAI port
	.
	
Description:
	1) Port Enable
	2) Port Frequency Rate	
    3) MAI data configuration
    
See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_sMaiPortInfo
{
	BRAP_CIT_P_sPortControlInfo		sMaiPortControlInfo;

	BRAP_CIT_P_MaiCfg				sMaiCfg; 

}BRAP_CIT_P_sMaiPortInfo;

/*********************************************************************
Summary:
    This datastructure is the database for the ports w.r.t PLL/NCO
Description:
	Mentions the ports associated with single PLL or NCO
        
See Also:
**********************************************************************/
typedef struct BRAP_CIT_P_sPllPortAssociation
{		
	BRAP_AF_P_PllNcoSelect				ePllNcoSelect;

	uint32_t							ui32PllEnable;	
	uint32_t							ui32PllIndex;	
	
	/* New Data structures */
	/*---------------------------------------------------------------------------------------*/

	BRAP_CIT_P_sPortControlInfo			sI2SPortInfo[BRAP_AF_P_MAX_NUM_I2S_PORTS];
	BRAP_CIT_P_sPortControlInfo			sDacPortInfo[BRAP_AF_P_MAX_NUM_DAC_PORTS];
	BRAP_CIT_P_sPortControlInfo			sCapPortInfo[BRAP_AF_P_MAX_NUM_CAP_PORTS];

	BRAP_CIT_P_sSpdifPortInfo			sSpdifPortInfo[BRAP_AF_P_MAX_NUM_SPDIF_PORTS];		
	BRAP_CIT_P_sMaiPortInfo				sMaiPortInfo;
	/*---------------------------------------------------------------------------------------*/

#if 0
	uint32_t						ui32I2SEnabled[BRAP_AF_P_MAX_NUM_I2S_PORTS];
	uint32_t						ui32DacEnabled[BRAP_AF_P_MAX_NUM_DAC_PORTS];
	uint32_t						ui32CapPortEnabled[BRAP_AF_P_MAX_NUM_CAP_PORTS];

	BRAP_CIT_P_sSpdifInputCfg		sSpdifInputCfg[BRAP_AF_P_MAX_NUM_SPDIF_PORTS];
	BRAP_CIT_P_sMaiInputCfg			sMaiInputCfg;
#endif


} BRAP_CIT_P_sPllPortAssociation;

/*********************************************************************
Summary:
    The datastructure used for Port Configuration
Description:
	The datastructure used for Port Sorting and the port configuration
        
See Also:
**********************************************************************/

typedef struct
{
	BRAP_AF_P_sFMM_DEST_CFG				sFmmDestCfg[BRAP_AF_P_MAX_NUM_PLLS];	
	BRAP_CIT_P_sPllPortAssociation		sPllPortAssociation[BRAP_AF_P_MAX_NUM_PLLS];

}BRAP_CIT_P_sPortCfg;



/*********************************************************************
Summary:
    Structure for Algo present in the Node structure
	Optional fields for the CIT_Gen Structure in special case
Description:
	All the big datastructures of CIT Gen module is set here    
        
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_sAlgoModePresent
{
	uint32_t	ui32DDP_PassThruPresent;
	uint32_t	ui32DTS_EncoderPresent;
	uint32_t	ui32AC3_EncoderPresent;
	uint32_t	ui32BTSC_EncoderPresent;
	uint32_t	ui32DolbyPulsePresent;
	uint32_t	ui32DDREPresent;
	
}BRAP_CIT_P_sAlgoModePresent;

/*********************************************************************
Summary:
    Structure mentions special tasks in the system. Based on the special 
	task configuration, setting required to be modified from the conventional
	cases.

Description:	
        
See Also:
**********************************************************************/ 
typedef struct BRAP_CIT_P_sSpecialTaskPresent
{
	/* Mixer Task Detection*/
	BRAP_CIT_P_MixerTaskDetection			eMixerTaskDetectionFlag;

}BRAP_CIT_P_sSpecialTaskPresent;

/*********************************************************************/
typedef struct
{
	uint32_t	ui32IsRequired;
	uint32_t	ui32BranchAssociated;
	uint32_t	ui32InterstageBufferId;

}BRAP_CITGEN_P_sOutputIsCfg;


/*********************************************************************
Summary:
    Internal Datastructures for the function
							BRAP_CITGEN_P_AllocIoBuffCfg	

Description:	
        
See Also:
**********************************************************************/ 
typedef struct BRAP_CITGEN_P_sAllocIoBuffCfgStructures
{

	BRAP_AF_P_sIO_BUFFER			sIoBuffTemp;
	BRAP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffTemp;


}BRAP_CITGEN_P_sAllocIoBuffCfgStructures;

/*********************************************************************
Summary:
    The complete datastructure definition for CIT Module
Description:
	All the big datastructures of CIT Gen module is set here    
        
See Also:
**********************************************************************/ 

typedef struct BRAP_CIT_P_sCitDataStructure
{
	BRAP_CIT_P_RdbConfig					sRdbCfg;
	BRAP_CIT_P_AlgoNwInfo					sAlgoNwInfo;	
	BRAP_CIT_P_sOpPortInfo					sOpPortInfo;	
	BRAP_CIT_P_sTaskBuffInfo				sTaskBuffInfo;
	BRAP_CIT_P_sAlgoExecOrder				sAlgoExecOrder[BRAP_P_MAX_ALGOS_IN_TASK];
	BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG	sTaskFmmGateOpenConfig;
	BRAP_AF_P_sDRAM_BUFFER					sTaskSPDIFPortConfigAddr[BRAP_AF_P_MAX_NUM_SPDIF_PORTS];

	/* Special Algorithm Stage detection flags */
	BRAP_CIT_P_sAlgoModePresent				sAlgoModePresent;

	/* Special Task Detection flag */
	BRAP_CIT_P_sSpecialTaskPresent			sSpecialTaskPresent;	

	BRAP_CITGEN_P_sOutputIsCfg				sOutputIsCfg[BRAP_P_MAX_ALGOS_IN_TASK][BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS];
	BRAP_AF_P_sFW_HW_CFG					sFwHwCfg;
	BRAP_CITGEN_P_sAllocIoBuffCfgStructures sAllocIoBuffCfgStructures;
	

}BRAP_CIT_P_sCitDataStructure;


/*---------------------------------------------------*/
/* Prototype Definition for CIT Genreation functions */
/*---------------------------------------------------*/

uint32_t BRAP_P_GenCit(	BMEM_Handle					hHeap,	
						BREG_Handle					hRegister,
						BRAP_CIT_P_InputInfo		*psCitIp,
						BRAP_CIT_P_Output			*psCitOp
					  );


void BRAP_P_AnalyseCit(BMEM_Handle					hHeap, 
					   BRAP_AF_P_sTASK_CONFIG		*psCit
					  );



/*--------------------------------------------------------------------------*/
/* Prototype Definition  and Structures for Calc Threshold and Block Time Fn*/
/*--------------------------------------------------------------------------*/

/*********************************************************************
Summary:
    Enums that describes whether the low delay mode is enabled/disabled

Description:
    PI conveys this information to API to indicate Lowdelay is enabled
	or not

See Also:
	
**********************************************************************/ 

typedef enum BRAP_CIT_P_LowDelayEnableMode
{
	BRAP_CIT_P_LowDelayMode_eDisabled,
	BRAP_CIT_P_LowDelayMode_eEnabled	=1,
	BRAP_CIT_P_LowDelayMode_eMax,			        /* Max value */
   	BRAP_CIT_P_LowDelayMode_eInvalid	= 0x7FFFFFFF          

}BRAP_CIT_P_LowDelayEnableMode;


/*********************************************************************
Summary:
    This structure contain elements that is returned by 
	CalcThreshold_BlockTime_AudOffset API.

Description:

	ui32Threshold and ui32BlockTime goes to FW and 
	ui32AudOffset goes to Application and TSM user cfg
        
See Also:
**********************************************************************/ 
typedef struct BRAP_CTB_Output
{
	uint32_t ui32Threshold;					/* Interms of samples */
	uint32_t ui32BlockTime;					/* Interms of Time (msec)  */
	uint32_t ui32AudOffset;					/* AudOffset in Time (msec) */

}BRAP_CTB_Output;




void BRAP_CITGEN_P_CalcThresholdAndBlockTime(BRAP_CTB_Output *psCTB_OutputStructure
				);

void BRAP_CITGEN_P_CalcThresholdZeroFillTimeAudOffset(
					BRAP_CIT_P_LowDelayEnableMode	eLowDelayEnableMode,
					BRAP_CIT_P_InputInfo			*psCitIp,
					BRAP_CTB_Output					*psCTB_OutputStructure
				);
/*-----------------------------------------------------------------------------*/
/* Prototype Definition  and Structures for Seamless Input Port Switching API  */
/*-----------------------------------------------------------------------------*/

/*********************************************************************
Summary:		
		This enum is the command to SIPS API to perform Adding/removing 
		a port.

Description:		
        
See Also:
**********************************************************************/ 
typedef enum BRAP_SIPS_P_AddRemoveIpPort
{
	BRAP_SIPS_P_AddRemoveIpPort_eAddPort,
	BRAP_SIPS_P_AddRemoveIpPort_eRemovePort	=1,
	BRAP_SIPS_P_AddRemoveIpPort_eMax,			        /* Max value */
   	BRAP_SIPS_P_AddRemoveIpPort_eInvalid	= 0x7FFFFFFF          

}BRAP_SIPS_P_AddRemoveIpPort;

/*********************************************************************
Summary:		
		This enum describes whether a free Ip port found or not

Description:		
        
See Also:
**********************************************************************/ 
typedef enum BRAP_SIPS_P_FreeIpPortFound
{
	BRAP_SIPS_P_FreeIpPortFound_ePresent,
	BRAP_SIPS_P_FreeIpPortFound_eAbsent	=1,
	BRAP_SIPS_P_FreeIpPortFound_eMax,			        /* Max value */
   	BRAP_SIPS_P_FreeIpPortFound_eInvalid	= 0x7FFFFFFF          

}BRAP_SIPS_P_FreeIpPortFound;

/*********************************************************************
Summary:		
		This structure is the input to the SIPS-API(Seamless Input Port
															 Switching)	
Description:
		
		a) AddRemoveIpPort Command
		b) Input Port IndexToRemove
		c) Cit Data Structure Addr
		d) Working Cit Data Structure Addr
        
See Also:
**********************************************************************/ 
typedef struct BRAP_SIPS_InputInfo
{
	/*Rdb Configuration */
	BRAP_CIT_P_RdbCfg		sRdbCfg;

	/* Command to add or remove the port from CIT input of a Task */
	BRAP_SIPS_P_AddRemoveIpPort		eAddRemoveIpPort;

	/* Input Source port details to be added */
	BRAP_CIT_P_FwStgSrcDstDetails	sInputSrcDetails;

	/* Input Port index to remove from the system */
	uint32_t						ui32InputPortIndexToRemove;

	/* The DRAM base address of CIT Data structure of the Running task
	   which needed to be reconfigured.
	*/
	uint32_t						ui32CitDataStructureDramAddr;

	/*
		DRAM address of the Working buffer for CIT modification. 

		Size of the buffer will be of the size BRAP_AF_P_sTASK_CONFIG.

		API Copies the CIT datastruture from 'ui32CitDataStructureDramAddr'
		to 'ui32WorkingCitDataStructureDramAddr'and modifies the structure 
	    parameters.

	*/

	uint32_t						ui32WorkingCitDataStructureDramAddr;


}BRAP_SIPS_InputInfo;


/*********************************************************************
Summary:		
		This enum describes Status of SIPS API Execution

Description:		
        
See Also:
**********************************************************************/ 
typedef enum BRAP_SIPS_P_ExecutionStatus
{
	BRAP_SIPS_P_ExecutionStatus_eSuccess,
	BRAP_SIPS_P_ExecutionStatus_eFail	=1,
	BRAP_SIPS_P_ExecutionStatus_eMax,			        /* Max value */
   	BRAP_SIPS_P_ExecutionStatus_eInvalid	= 0x7FFFFFFF          

}BRAP_SIPS_P_ExecutionStatus;

/***********************************************************************
Summary:		
	This structure is the output from  the SIPS-API (Seamless Input Port 
															  Switching)
Description:
		
		a) AddRemoveIpPort Command
		b) Input Port IndexToRemove
		c) Cit Data Structure Addr
		d) Working Cit Data Structure Addr
        
See Also:
*************************************************************************/ 
typedef struct BRAP_SIPS_OutputInfo
{
	BRAP_SIPS_P_ExecutionStatus	eExecutionStatus;

	uint32_t					ui32InputPortIndexAdded;		

												/* 
													Valid only with while adding 
													an Input port.
												*/
}BRAP_SIPS_OutputInfo;

/************************************************************************/ 

/*		Function Prototype for Seamless Input Port Switching(SIPS) API  */
/*----------------------------------------------------------------------*/
uint32_t BRAP_SIPS_P_SeamlessIpPortSwitchAPI(	
				BMEM_Handle				hHeap,				
				BRAP_SIPS_InputInfo		*psSipsIp,
				BRAP_SIPS_OutputInfo	*psSipsOp						
			);

/*---------------------------------------------------------------------*/
/* Prototype Definition  and Structures for Seamless Port Switching API*/
/*---------------------------------------------------------------------*/

/***********************************************************************
Summary:		
		This structure is the input to the SPS-API(Seamless Port Switching)
		It gives the TaskID and the FMM cfg buffers.

Description:
		
		a) Task ID
		b) FMM cfg memory address of running task
		c) The temp :operating buffer for SPS-API
        
See Also:
************************************************************************/ 

typedef struct BRAP_SPS_InputInfo
{
	/*Task ID of the Task for which SPS API(Seamless Port Switching) is
	called. 	*/
	uint32_t	ui32TaskId;	

	/*The FMM cfg memory addr that CIT allocated and FW using */
	uint32_t	ui32CitTaskFmmDestCfgMemoryAddr;

	/*	This is the memory allocated by PI for every task and and 
		SPS API use this buffer as operating buffer for port
		reconfiguration
	*/
	BRAP_AF_P_sDRAM_BUFFER		sOperatingTaskFmmDestCfgBuff;


}BRAP_SPS_InputInfo;

/*********************************************************************
Summary:
		This structure gives the info on the destination port to be 
		enabled or disabled in SPS API(Seamless Port Switching)....		
		
Description:
		
		Port type of Op Port and Cap Port is metioned.
		Enable/Disable Flag is selects future status of port.
        
See Also:
**********************************************************************/ 

typedef struct BRAP_SPS_InputPortCfg
{
	uint32_t                ui32NumOutputs;	/*	Number of output ports for this task.
												Only these many first entries in sPortCfg[]
												array are valid. 
											*/

	BRAP_CIT_P_PortCfg		sPortCfg[BRAP_P_MAX_OUTPUTS];

}BRAP_SPS_InputPortCfg;


/*		Function Prototype for Seamless Port Switching(SPS) API			*/
/*----------------------------------------------------------------------*/
uint32_t BRAP_SPS_P_SeamlessPortSwitch(	
				BMEM_Handle				hHeap,
				BREG_Handle				hRegister,
				BRAP_SPS_InputInfo		*psSpsIp,
				BRAP_SPS_InputPortCfg	*psInputPortCfg						
			);
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/* Prototype Definition for function to display DEC/PP/ENC type		    */
/*----------------------------------------------------------------------*/

const char*  BRAP_P_PrintDecEncPpTypeName(
				BRAP_AF_P_DecodeEncPPAlgoType  eDecodeEncPPAlgoType									 
			);

/*----------------------------------------------------------------------*/

#if 0

void BRAP_CITGEN_P_GenerateFwRdbAddrTable(
							BRAP_CIT_P_RdbCfg	*psRdbCfg,
							uint32_t			ui32FwRdbTableDramAddr
						);
#endif



/* ------------------------------------------------------------------------*/
/*							GFX SUPPORT									   */
/* ------------------------------------------------------------------------*/


typedef struct BRAP_CIT_P_GfxCITOutput
{
    BRAP_GFX_P_sTASK_CONFIG		sGfxCit;			/* GFX Cit Structure */

	uint32_t					ui32NumBranches;

    BRAP_CIT_P_OpBranchInfo		sCitBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];

	BRAP_AF_P_sDRAM_BUFFER		sStackSwapBuff;	/* Stack Swap Buffer */
	

} BRAP_CIT_P_GfxCITOutput;


/*---------------------------------------------------------------------
				Top level GFX CIT Generation Function 
---------------------------------------------------------------------*/
uint32_t BRAP_P_GenCitGfx(
						BMEM_Handle					hHeap,	
						BREG_Handle					hRegister,						
						BRAP_CIT_P_InputInfo		*psCitIp,
						BRAP_GFX_P_sGfxDecodeBuffCfg	*psGfxDecodeBuffCfg,
						BRAP_CIT_P_GfxCITOutput	    *psGfxCitOp
					  );

void BRAP_P_AnalyseGfxCit(
						BMEM_Handle					hHeap,
						BRAP_GFX_P_sTASK_CONFIG		*psGfxCit
					  );



/****************************************************************************/
/****************************************************************************/
/************************* VIDEO TASK  **************************************/
/****************************************************************************/
/****************************************************************************/



/****************************************************************************
Summary:

    Structure to describe output of Video CIT module

Description:

    This structure describes output of Video CIT module. Output of Video CIT 
	module contains following 

    1. Completely initialized Video CIT structure

    2. Auxiliary information required by other  RAP PI modules to initilalize
        buffers created in Video CIT module

See Also:

******************************************************************************/ 

typedef struct BRAP_CIT_P_VideoCITOutput
{
    BRAP_VF_P_sTASK_CONFIG		sVideoCit;			/* Video Cit Structure */

	uint32_t					ui32NumBranches;

    BRAP_CIT_P_OpBranchInfo		sCitBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];

	BRAP_AF_P_sDRAM_BUFFER		sStackSwapBuff;	/* Stack Swap Buffer */
	
	BRAP_AF_P_sDRAM_BUFFER		sSpdifUserConfigAddr[BRAP_AF_P_MAX_NUM_SPDIF_PORTS];

} BRAP_CIT_P_VideoCITOutput;

/*---------------------------------------------------------*/
/* Prototype Definition for Video CIT Genreation functions */
/*---------------------------------------------------------*/

uint32_t BRAP_P_VideoGenCit(
						BMEM_Handle					hHeap,	
						BREG_Handle					hRegister,						
						BRAP_CIT_P_InputInfo		*psCitIp,
						BRAP_VF_P_sVDecodeBuffCfg	*psVDecodeBuffCfg,
						BRAP_CIT_P_VideoCITOutput	*psCitOp
					  );


void BRAP_P_AnalyseVideoCit(
						BMEM_Handle					hHeap,
						BRAP_VF_P_sTASK_CONFIG		*psCit
					  );




/****************************************************************************/
/****************************************************************************/
/*************************  SCM TASK  ***************************************/
/****************************************************************************/
/****************************************************************************/



/****************************************************************************
Summary:

    Structure to describe output of SCM CIT module

Description:

    This structure describes output of SCM CIT module. Output of SCM CIT 
	module contains following 

    1. Completely initialized SCM CIT structure

    2. Auxiliary information required by other  RAP PI modules to initilalize
        buffers created in SCM CIT module

See Also:

******************************************************************************/ 

typedef struct BRAP_CIT_P_ScmCITOutput
{
    BRAP_SCM_P_sTASK_CONFIG		sScmCit;		/* SCM Cit Structure */

	uint32_t					ui32NumBranches;

    BRAP_CIT_P_OpBranchInfo		sCitBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];

	BRAP_AF_P_sDRAM_BUFFER		sStackSwapBuff;	/* Stack Swap Buffer */
	

} BRAP_CIT_P_ScmCITOutput;


/*---------------------------------------------------------------------
				Top level SCM CIT Generation Function 
---------------------------------------------------------------------*/

uint32_t BRAP_P_GenCitScm(
						BMEM_Handle					hHeap,	
						BREG_Handle					hRegister,						
						BRAP_CIT_P_InputInfo		*psCitIp,						
						BRAP_CIT_P_ScmCITOutput	    *psScmCitOp
					  );

void BRAP_P_AnalyseScmCit(
						BMEM_Handle					hHeap,
						BRAP_SCM_P_sTASK_CONFIG		*psScmCit
					  );


#endif
