/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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
* Module Description:
*			
*		This file contains all private macros, enums, structures and 
*		functions privately used inside the Audio Manager module of the 
*		Raptor PI.
*
* Revision History:
*
* $brcm_Log: $
* 
_Software_Devel/107   9/5/06 10:49a rjain
* PR 24059: Watermark interrupt workaround has to be used for 7401 C0
* also.
* 
* Hydra_Software_Devel/106   8/31/06 12:29p rjain
* PR 22868: Moving the hSecureMem handle into BRAP_Settings()
* 
* Hydra_Software_Devel/105   8/23/06 4:31p rjain
* PR 22868: added code to pass a 2nd memory handle (for secure region).
* this 2nd handle is used for RBUF memory allocation. Note: application
* has to take care to pass correct secure memory handle
* 
* Hydra_Software_Devel/104   8/13/06 8:24p nitinb
* PR 18976: Shifting 7411 FW RDB header files to basemodules/chp folder
* 
* Hydra_Software_Devel/103   8/2/06 5:59p bselva
* PR 22486: Merged the changes for multichannel and concurrent stereo
* downmix support for 7400 to main branch
* 
* Hydra_Software_Devel/102   6/29/06 11:37a bselva
* PR 22288: Corected the define B0 to BCHP_VER_B0
* 
* Hydra_Software_Devel/101   6/27/06 2:00p bhanus
* PR 22288 : Checking in timer workaround for water mark interrupt bug
* 
* Hydra_Software_Devel/100   6/19/06 4:36p rjain
* PR 21942: 7118 doesn't have bchp_memc_0.h, it has bchp_memc.h
* 
* Hydra_Software_Devel/99   6/12/06 12:03p rjain
* PR 21786: fixed handling of Cloning when in Simul mode.
* 
* Hydra_Software_Devel/98   6/7/06 5:17p nitinb
* PR 19615: Replacing BCHP_REV macros with BCHP_VER macro
* 
* Hydra_Software_Devel/97   6/2/06 12:42a nitinb
* PR 19615: Enabling switching between overclocked multichannel mode and
* at-clock stereo mode at decode-start
* 
* Hydra_Software_Devel/96   5/25/06 4:50p nitinb
* PR 19615: For multichannel mode, codecs require non-zero size of
* interstage buffers
* 
* Hydra_Software_Devel/95   5/24/06 4:13p nitinb
* PR 19615: Added function to stop I2S2 and corresponding mixer
* 
* Hydra_Software_Devel/94   5/12/06 6:30p bhanus
* PR 21388 : Checking in PCM Playback related changes.
* 
* Hydra_Software_Devel/93   5/9/06 11:23a rjain
* PR 21481: adding support for 7118 to RAP PI
* 
* Hydra_Software_Devel/92   5/4/06 3:03p bhanus
* PR 21388 : Checking in Add/Remove Output Port related changes for 7400.
* 
* Hydra_Software_Devel/91   5/3/06 6:33p nitinb
* PR 19615: Support upto 8 channel audio using two I2S ports on 7411D0
* 
* Hydra_Software_Devel/90   4/24/06 11:49a kagrawal
* PR 21159: Replaced uiGetSamplingRate[] by BRAP_P_ConvertSR() to support
* all the BAVC_AudioSamplingRate enums
* 
* Hydra_Software_Devel/89   4/11/06 6:05p sushmit
* PR18604: Modifying
* #if (( BCHP_CHIP == 7401 )||( BCHP_CHIP == 7400 ))
* With code like
* #ifndef BCHP_7411_VER
* 
* Hydra_Software_Devel/88   4/6/06 5:56p rjain
* PR 20318: Implementing code review comments for AM
* 
* Hydra_Software_Devel/87   4/4/06 4:14p rjain
* PR 20318: Implementing code review comments for AM
* 
* Hydra_Software_Devel/86   4/3/06 6:06p rjain
* PR 20318: Implementing review comments for AM
* 
* Hydra_Software_Devel/85   3/30/06 7:01p kagrawal
* PR 20318: Implemented review comments for AM and Sampling Rate Change
* 
* Hydra_Software_Devel/84   3/30/06 4:41p bselva
* PR 19878: Checking in the changes required for 7400 phase 0.5 release
* 
* Hydra_Software_Devel/83   3/30/06 3:19p bhanus
* PR 19360 : Added a check to return error if PCM data is mixed with
* compressed data
* 
* Hydra_Software_Devel/82   3/28/06 12:14p rnayak
* PR20439: Restore Bangalore Raptor files after magnum vob corruption on
* 17th March 2006
* 
* Hydra_Software_Devel/82   3/21/06 1:14p kagrawal
* PR 20319: Handling only registered buffer block interrupts for a
* channel
* 
* Hydra_Software_Devel/81   3/16/06 6:56p nitinb
* PR 20117: Implement dolby digital bass management algorithm
* 
* Hydra_Software_Devel/80   3/16/06 5:25p nitinb
* PR 20117: Implement dolby digital bass management algorithm
* 
* Hydra_Software_Devel/79   3/2/06 8:30p kagrawal
* PR 19615: Merged changes for multi channel support to the main branch
* 
* Hydra_Software_Devel/78   3/2/06 7:58p rjain
* PR 18264: changing to use FW RDB from basemodules/chp/7401 instead of
* \portinginterface\rap\7401
* 
* Hydra_Software_Devel/77   3/2/06 7:38p nitinb
* PR 19231: Replaced function BRAP_P_MuteChannelOutput_isr by pair of
* functions BRAP_P_MuteChannelOutputOnSr_isr and
* BRAP_P_UnMuteChannelOutputOnSr_isr
* 
* Hydra_Software_Devel/76   3/1/06 6:22p rjain
* PR 19946 :
* - Fixed some issues related to handling of Mai
* - Fixed some issues related to sampling rate changes for Mai, for
* cloned ports and for decode channels
* 
* Hydra_Software_Devel/75   3/1/06 9:44a kagrawal
* PR 18264: Implemented review comments for PCM playback
* 
* Hydra_Software_Devel/74   2/28/06 11:13a sushmit
* PR18604: Fix for 7411 compilation issues.
* 
* Hydra_Software_Devel/73   2/27/06 4:16p sushmit
* PR18604: Fixing 7411 compilation issues.
* 
* Hydra_Software_Devel/72   2/17/06 10:07a sushmit
* PR18604: Merging Encoder to latest.
* 
* Hydra_Software_Devel/72   2/17/06 9:52a sushmit
* PR18604: Merging Encoder to latest.
* 
* Hydra_Software_Devel/RAP_AUD_ENC/4   2/16/06 4:44p sushmit
* PR18604: Updating Audio Encoder PI.
* 
* Hydra_Software_Devel/RAP_AUD_ENC/3   2/6/06 4:15p sushmit
* PR18604: Merging latest Decoder PI to Encoder PI branch
* 
* Hydra_Software_Devel/69   1/23/06 5:27p bselva
* PR 19168: For 7400 RDB files are included from chp directory.
* 
* Hydra_Software_Devel/68   1/19/06 7:24p bselva
* PR 19168: Checking in the changes required for 7400 bringup
* 
* Hydra_Software_Devel/67   1/19/06 3:51p nitinb
* PR 19114: PVR Trickmode: problem with Audio PID=0x00
* 
* Hydra_Software_Devel/66   1/19/06 2:09p kagrawal
* PR 18264: 1. Rearchitected SRC to accomodate capture channel
* 2. Implemented review comments
* 
* Hydra_Software_Devel/65   12/15/05 9:56a kagrawal
* PR 18312: Implement 7411 RAP PI phase-5 deliverables
* - Added BRAP_P_MaskInterrupt() and BRAP_P_UnmaskInterrupt()
* 
* Hydra_Software_Devel/64   12/14/05 7:09p nitinb
* PR 18061: Raptor memory allocation must be less than 0.5M for MPEG/AC-3
* only
* 
* Hydra_Software_Devel/63   12/13/05 4:40p nitinb
* PR 18163: RAP BMEM leak on system uninit; calls to BMEM_AllocAligned
* but no calls to BMEM_Free
* 
* Hydra_Software_Devel/62   12/13/05 1:59p kagrawal
* PR 18578: Added support for dither and burst mode programming for
* SPDIFFM
* 
* Hydra_Software_Devel/61   12/9/05 4:41p rjain
* PR 18264: Capture PI Development
* 
* Hydra_Software_Devel/60   12/6/05 6:32p kagrawal
* PR 18312: Renamed BRAP_P_GetChannelPairUsingOpPort() to
* BRAP_P_GetChannelPairUsingOpPort_isr()
* 
* Hydra_Software_Devel/59   12/5/05 10:37a nitinb
* PR 18061: For decode channels, added check on possible combinations of
* decode modes
* 
* Hydra_Software_Devel/58   12/4/05 8:15p kagrawal
* PR 18312: Added support to write PCM data in interrupt driven mode
* 
* Hydra_Software_Devel/57   12/2/05 3:33p nitinb
* PR 18061: Raptor memory allocation must be less than 0.5M for MPEG/AC-3
* only
* 
* Hydra_Software_Devel/56   11/30/05 4:18p kagrawal
* PR 18279: Rearchitected output sampling rate change for PCM channel
* 
* Hydra_Software_Devel/55   11/25/05 2:47p bselva
* PR 17590: Fix for BigEndian system
* 
* Hydra_Software_Devel/54   11/23/05 3:01p rjain
* PR 17801: correcting testtone PI to save and restore ALL registers that
* it modifies.
* 
* Hydra_Software_Devel/53   11/16/05 9:12p rjain
* PR 18129: Making the changes reqd for PI-Fw interface to add an output
* port on the fly for a DEC channel.
* 
* Hydra_Software_Devel/52   11/16/05 1:30p rjain
* PR 18129: Added code for 1.Watchdog recovery. 2. Adding a cloned port
* before a channel start
* 
* Hydra_Software_Devel/51   11/14/05 9:53a rjain
* PR 18129: Adding support for routing to multiple output ports
* 
* Hydra_Software_Devel/50   11/10/05 11:53a kagrawal
* PR 17590: Merging PCM Playback and Mixing changes to
* Hydra_Software_Devel
* 
* Hydra_Software_Devel/Kapil_PCM_Playback/2   11/1/05 9:44a kagrawal
* 1. Implemented review comments on PCM playback and mixing
* 2. Added global private defines - BRAP_P_I2S_MAX_BITS_PER_SAMPLE,
* RAP_P_MIXER_MAX_BITS_PER_SAMPLE and BRAP_P_MIN_BITS_PER_SAMPLE
* 
* Hydra_Software_Devel/Kapil_PCM_Playback/1   10/24/05 4:51p kagrawal
* PR 17590: Initial check in for PCM Playback and Mixing
*  
* Hydra_Software_Devel/49   11/2/05 3:27p bselva
* PR 17590: Added watchdog support for 7401
* 
* Hydra_Software_Devel/48   10/18/05 4:57p rjain
* PR 17651: Changing checks for BCHP_CHIP ==/!= 7401 to check for
* BCHP_7411_VER instead
* 
* Hydra_Software_Devel/47   10/17/05 2:16p rjain
* PR 17651: Making changes for 7411 D0
* 
* Hydra_Software_Devel/46   10/14/05 3:30p nitinb
* PR 17420: Added prototype of private function
* BRAP_P_GetInternalCallFlag
* 
* Hydra_Software_Devel/45   10/13/05 2:39p bselva
* PR 16148: Fixed the compilation warning for the changed in rdb wrt to
* MEMC registers.
* 
* Hydra_Software_Devel/44   10/5/05 7:30p bselva
* PR 16148: Added interrupt support for 7401
* 
* Hydra_Software_Devel/43   10/3/05 1:09p rjain
* PR 16783: modifying Testtone API to save reqd. registers so that
* Channel Close/ reopen is not required for running the test tone
* 
* Hydra_Software_Devel/42   9/26/05 11:25p nitinb
* PR 16889: Fixing compilation error on 7401
* 
* Hydra_Software_Devel/41   9/22/05 9:55p nitinb
* PR 16889: Adding callback info to device level interrupts in RAP handle
* 
* Hydra_Software_Devel/40   9/20/05 1:07p nitinb
* PR 16899: Making changes in the code for watchdog feature
* 
* Hydra_Software_Devel/39   9/15/05 8:30p nitinb
* PR 16899: Adding flag bWatchdogRecoveryOn to RAP handle
* 
* Hydra_Software_Devel/38   9/14/05 7:50p nitinb
* PR 17042: Raptor does not support sample rate change in the middle of a
* stream
* 
* Hydra_Software_Devel/37   9/13/05 7:41p bmishra
* PR 16148: Merged 7401 related changes
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/7   8/19/05 12:02p bmishra
* PR 16148: Merged latest changes from the main branch
* 
* Hydra_Software_Devel/36   8/3/05 9:16p bselva
* PR 15766: Support for BRAP_GetRevision API added
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/6   8/1/05 5:09p bmishra
* PR 16148: Implemented changes due to addition of HiFiDAC APIs
* 
* Hydra_Software_Devel/35   7/21/05 12:58p rjain
* PR 16354: FMM related changes for Pause/Resume
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/5   7/20/05 2:27p bmishra
* PR 16148: Included RDB files related to HiFiDAC for 7401
* 
* Hydra_Software_Devel/34   7/19/05 6:58p nitinb
* PR 16354: Develope PVR features for RAP PI
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/4   7/15/05 3:29p bmishra
* experimental changes
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/3   7/14/05 11:11a bmishra
* PR 16148: Removed references to ARC
* 
* Hydra_Software_Devel/32   7/13/05 7:41p nitinb
* PR 16093: Registers must be accessed using absolute addresses
* 
* Hydra_Software_Devel/BLR_RAP_7401_Bringup/1   7/12/05 8:15p bmishra
* PR 16148: Modified the file for 7401 compatibility
* 
* Hydra_Software_Devel/31   7/11/05 11:18a nitinb
* PR 16148: Adding prototypes for wrapper functions to convert device
* memory physical address to virtual and vice versa
* 
* Hydra_Software_Devel/30   7/8/05 5:14p rjain
* PR 16148: Modify BRAP_Read/Write to use BREG_Read/Write instead of
* BARC_XXX for 7401
* 
* Hydra_Software_Devel/29   7/8/05 2:04p rjain
* PR 16148: Modifiying for 7401
* 
* Hydra_Software_Devel/28   7/6/05 10:42a bselva
* PR15611: Checking in RDB that has absolute physical register addresses
* 
* Hydra_Software_Devel/27   6/24/05 3:33p bmishra
* PR 14143: Implemented code review comments
* 
* Hydra_Software_Devel/26   6/23/05 6:44p nitinb
* PR 15976: Added function BRAP_P_DownloadDspFwInMem that is required for
* new MITimplementation.
* 
* Hydra_Software_Devel/25   6/18/05 4:04p rjain
* PR 15737: after handling all interrupts, PI now unmasks appropriate
* bits in AIO_INTH_R5F and  AUD_DSP_INTH0_R5F.
* 
* Hydra_Software_Devel/24   6/18/05 3:31p rjain
* PR 15925: PI changes required for routing decoded data to I2S and SPDIF
* ports simultanesouly
* 
* Hydra_Software_Devel/23   5/31/05 10:24a nitinb
* PR 15272: Adding prototypes for isr/task context sensative register
* read/write functions
* 
* Hydra_Software_Devel/22   5/30/05 6:50p rjain
* PR 15272: Updating interrupt infrastructure
* 
* Hydra_Software_Devel/21   5/25/05 8:21p nitinb
* PR 15496: Memory writes to 7411 memory must be followed by a read to
* ensure the data is flushed properly
* 
* Hydra_Software_Devel/20   5/25/05 5:56p nitinb
* PR 15272: Adding workaround for incorrect register addresses (offset by
* 0x2000) in RDB
* 
* Hydra_Software_Devel/19   5/25/05 3:59p rjain
* PR 15272: Adding some code for interrupt handling framework.
* 
* Hydra_Software_Devel/18   5/25/05 3:45p nitinb
* PR 15272: Removing inclusion of file brap_common.h
* 
* Hydra_Software_Devel/17   4/15/05 2:56p bmishra
* PR 14507: Simultaneous mode related changes
* 
* Hydra_Software_Devel/16   3/10/05 8:05p nitinb
* PR 13082: Only DSP firmware RDB files are renamed from BCHP_XXX to
* BRAP_XXX. Remaining RDB files name should continue to be BCHP_XXX
* 
* Hydra_Software_Devel/15   3/10/05 3:48p rjain
* PR 13082: updating RDB include after name change from bchp_XXX to
* brap_XXX
* 
* Hydra_Software_Devel/14   2/16/05 11:52p nitinb
* PR 13082: Defined macro BRAP_P_ALIGN
* 
* Hydra_Software_Devel/13   2/16/05 8:29p bmishra
* PR 14143: Changing BRAP_OutputPortSettings to BRAP_OutputSettings
* 
* Hydra_Software_Devel/12   2/16/05 1:45p rjain
* PR 14143: Implementing some comments from Phase 1 Code review done till
* date
* 
* Hydra_Software_Devel/11   2/8/05 7:00p rjain
* PR13082: Some bug fixes and clean up
* 
* Hydra_Software_Devel/10   2/2/05 9:58p bmishra
* PR13082: Fixed compilation error
* 
* Hydra_Software_Devel/9   2/2/05 5:35p rjain
* PR13082: compilation errors
* 
* Hydra_Software_Devel/8   2/2/05 4:49p rjain
* PR13082: Modifying according to settings/bugs highlighted in emulation
* 
* Hydra_Software_Devel/7   1/28/05 12:22a nitinb
* PR 13082: Fixed compilation warnings
* 
* Hydra_Software_Devel/6   1/27/05 6:10p bmishra
* PR 13827: Implemented Memory read and write functions using BARC
* interface
* 
* Hydra_Software_Devel/5   1/25/05 9:53a rjain
* PR13082: Modifying according to settings/bugs highlighted in emulation
* so far.
* 
* Hydra_Software_Devel/4   1/13/05 3:46p bmishra
* PR 13082: Fixed compilation errors
* 
* Hydra_Software_Devel/3   1/12/05 5:03p rjain
* PR13082: Updating with changes after compilation of FMM, AM and RM
* together.
* 
* Hydra_Software_Devel/2   1/11/05 11:45a rjain
* PR13082: included RDB header files
* 
* Hydra_Software_Devel/1   1/10/05 8:24p nitinb
* PR 13082: Moved files to new directory structure
* 
***************************************************************************/

#ifndef _BRAP_PRIV_H__ /*{{{*/
#define _BRAP_PRIV_H__

/* This should be defined before brap_dsp_priv.h is included */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
#define BARC_Handle	BREG_Handle
#endif

#include "brap.h"

#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
#define BRAP_DSP_P_7401_NEWMIT 1 /* define this as 1 for new 7401 mit   */
#endif 

#if (BRAP_DSP_P_7401_NEWMIT==1)
#define BRAP_P_MAX_PP_SUPPORTED		3	/* Maximum PP supported  */
#define BRAP_P_MAX_BRANCH_SUPPORTED	1	/* Number of branch supported */
#define BRAP_MAX_PP_PER_BRANCH_SUPPORTED 7	/* Number of post processing stages
											per post processing branch*/
#endif


#include "brap_types.h"
#include "brap_types_priv.h"
#include "brap_rm_priv.h"
#include "bkni.h" /* for the OS dependant API's */


#include "brap_dsp_priv.h"
#include "brap_int_priv.h"
#include "brap_fmm_priv.h"
#include "brap_rbuf_priv.h"
#include "brap_srcch_priv.h"
#include "brap_mixer_priv.h"
#include "brap_spdiffm_priv.h"
#include "brap_op_priv.h"
#include "brap_pcm_priv.h"
#include "brap_dstch_priv.h"

#ifndef BCHP_7411_VER /* For chips other than 7411 */
#include "brap_enc_priv.h"
#include "brap_hifidac_priv.h"
#include "bchp_memc_0.h"
#endif



/****************************************************************************
 * Sub-block register definitions.
 ***************************************************************************/

#include "bchp_aud_dsp_misc.h"
#include "bchp_aud_dsp_ctrl0.h"
#include "bchp_aud_dsp_cfg0.h"
#include "bchp_aud_dsp_inth0.h"
#include "bchp_aud_dsp_esr_be0.h"
#include "bchp_aud_dsp_esr_pi0.h"
#include "bchp_aud_dsp_esr_si00.h"
#include "bchp_aud_dsp_esr_si10.h"
#include "bchp_aud_dsp_esr_si20.h"
#include "bchp_aud_dsp_esr_so00.h"
#include "bchp_aud_dsp_esr_so00.h"
#include "bchp_aud_dsp_esr_so10.h"
#include "bchp_aud_dsp_esr_so20.h"

#include "bchp_aio_misc.h"
#include "bchp_aio_inth.h"
#include "bchp_aio_intd0.h"
#include "bchp_aud_fmm_misc.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_aud_fmm_bf_esr0.h"
#include "bchp_aud_fmm_bf_esr1.h"
#include "bchp_aud_fmm_dp_ctrl0.h"
#include "bchp_aud_fmm_dp_esr0.h"
#include "bchp_aud_fmm_ms_ctrl.h"
#include "bchp_aud_fmm_ms_esr.h"
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_op_esr.h"
#include "bchp_aud_fmm_pll0.h"
#include "bchp_aud_fmm_pll1.h"

#ifdef BCHP_7411_VER /* For the 7411 */ 
#include "bchp_aud_fmm_dp_ctrl1.h"
#include "bchp_aud_fmm_dp_esr1.h"
#include "bchp_aud_fmm_pll2.h"
#include "bchp_aud_fmm_nco.h"
#include "brap_common.h"
#else
#include "bchp_common.h"
#include "bchp_hifidac_ctrl0.h"
#include "bchp_hifidac_esr0.h"
#include "bchp_hifidac_rm0.h"
#include "bchp_int_id_aud_dsp_inth0.h"
#include "bchp_int_id_aio_inth.h"
#if ( BCHP_CHIP == 7400 )
#include "bchp_aud_dsp_esr_si30.h"
#include "bchp_aud_dsp_esr_si40.h"
#include "bchp_aud_dsp_esr_si50.h"
#include "bchp_aud_dsp_esr_so30.h"
#include "bchp_aud_dsp_esr_so40.h"
#include "bchp_aud_dsp_esr_so50.h"
#include "bchp_aud_fmm_dp_ctrl1.h"
#include "bchp_aud_fmm_dp_esr1.h"
#include "bchp_aud_fmm_pll2.h"
#include "bchp_hifidac_ctrl1.h"
#include "bchp_hifidac_esr1.h"
#include "bchp_hifidac_rm1.h"
#endif /* ( BCHP_CHIP == 7400 ) */
#endif


#if (!defined BCHP_7411_VER) || (EMULATION)
/* For 7401 etc or for EMULATION: Do not use BARC interface. Do not use BRAP_TRANS ie use normal XPT */
 #define BRAP_P_USE_BARC 0
 #define BRAP_P_USE_BRAP_TRANS 0
#else

 #define BRAP_P_USE_BARC 1   /* Use the BARC interface for 7411 */
 #define BRAP_P_USE_BRAP_TRANS 1  /* Use BRAP_TRANS channel for transport not XPT */
#endif


#if BRAP_P_USE_BARC == 1
#include "barc.h"
#endif 

#if BRAP_P_USE_BRAP_TRANS == 1
#include "brap_transport_priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


#if (BCHP_CHIP == 7401)
#define BRAP_PRIV_PI_CHIP_ID		  	0x7401
#elif (BCHP_CHIP == 7403)
#define BRAP_PRIV_PI_CHIP_ID		  	0x7403
#elif (BCHP_CHIP == 7118)
#define BRAP_PRIV_PI_CHIP_ID		  	0x7118
#elif (BCHP_CHIP == 7400)
#define BRAP_PRIV_PI_CHIP_ID		  	0x7400
#else
#define BRAP_PRIV_PI_CHIP_ID		  	0x7411
#endif

#define BRAP_PRIV_PI_VERSION_MAJOR			0x2	
#define BRAP_PRIV_PI_VERSION_MINOR			0x0
#define BRAP_PRIV_PI_VERSION_DATE				0x0000727
#define BRAP_PRIV_PI_VERSION_YEAR				0x2005
#define BRAP_PRIV_PI_VERSION_CUSTOMER_ID		"REF"

#define BRAP_P_INVALID_DRAM_ADDRESS			0xFFFFFFFF

#if (BRAP_7401_FAMILY == 1) || (defined BCHP_7411_VER)
#define BRAP_P_EDGE_TRIG_INTRPT     0
#else
#define BRAP_P_EDGE_TRIG_INTRPT     1
#endif

/*{{{ Defines */
#define BRAP_P_ALIGN(x,alignbits)   ((x + ((1 << alignbits) - 1)) & ~((1 << alignbits) - 1))

#define BRAP_P_I2S_MAX_BITS_PER_SAMPLE	32	/* Maximum bits per sample 
											   supported by I2S output */
#define BRAP_P_MIXER_MAX_BITS_PER_SAMPLE 24	/* Maximum bits per sample 
											   supported by Mixer */
#define BRAP_P_MIN_BITS_PER_SAMPLE		 16	/* Minimum bits per sample 
											   supported by Raptor */
#define BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA		 16	/* bits per sample for 
											   compressed data supported by Raptor */

#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7401 && BCHP_VER !=BCHP_VER_A0) || (BCHP_CHIP == 7403)
#define BRAP_P_WATERMARK_WORKAROUND 1
#else
#define BRAP_P_WATERMARK_WORKAROUND 0
#endif


#define BRAP_P_ABSOLUTE_VALUE(x)    ((x) < 0 ? -(x) : (x)) /* Returns the absolute value */
#define BRAP_P_IS_NEGATIVE_VALUE(x)    ((x) < 0 ? 1 : 0) /* Returns 1 if the value is lesser than zero, 
                                                                      0 otherwise */      

/*}}}*/

/*{{{ Typedefs */

        
/* Structure for storing the property of output audio mode */
typedef struct BRAP_DEC_P_OpAudModProp
{
	unsigned int uiNoChannels;				/* Number of output possible channels */
	bool bChnExists[BRAP_RM_P_MAX_OP_CHANNELS];/* Element of the array correspond 
											   to audio output channels defined 
											   by enum BRAP_OutputChannel. 
											   Each element of this array 
											   specifies the following.
											   TRUE: If the corresponding audio 
													 output channel exists, 
											   FALSE: otherwise */
}BRAP_P_OpAudModProp;



/* Registers that need to be saved and restored while enabling/disabling test tone */
typedef struct BRAP_P_TestToneRegs
{
	uint32_t ui32SpdifCfg;  /* BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 */
	uint32_t ui32I2sCfg; /* BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE */
	uint32_t ui32OpEnableStatus ; /* BCHP_AUD_FMM_OP_CTRL_ENABLE_STATUS */    
	uint32_t ui32SerOutOe; /* BCHP_AUD_FMM_MISC_SEROUT_OE */  

	uint32_t ui32MsBypass; /* BCHP_AUD_FMM_MS_CTRL_USEQ_BYPASS */
	uint32_t ui32MsStreamCtrl0; /* BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0 */
	uint32_t ui32MsStreamEna ; /* BCHP_AUD_FMM_MS_CTRL_STRM_ENA */
        /* Dont need to save the CBIT array coz that is formed afresh by the Host/FW when the channel is restarted */
        
	uint32_t ui32Client0Mix1Cfg; /* BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG */    
	uint32_t ui32Client2Mix1Cfg; /* BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT2_MIX1_CONFIG */    
	uint32_t ui32Pb0ScaleSrcCfg; /* BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG */    
	uint32_t ui32Pb2ScaleSrcCfg; /* BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK2_SCALE_SRC_CONFIG */        
	uint32_t ui32DpStreamEna; /* BCHP_AUD_FMM_DP_CTRL0_STRM_ENA */        
    
	uint32_t ui32Rbuf0BaseAddr;/* BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR */
	uint32_t ui32Rbuf0EndAddr; /* BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR */

	uint32_t ui32SrcCh0Cfg; /* BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CFG */
	uint32_t ui32SrcCh0Ctrl; /* BCHP_AUD_FMM_BF_CTRL_SOURCECH_0_CTRL */            
}BRAP_P_TestToneRegs;


#ifdef BCHP_7411_VER  /* only for 7411 */      
/***************************************************************************
Summary:
    This enumeration lists the 'states' of cloning. 
    
Description:

See Also:

***************************************************************************/ 
typedef enum BRAP_P_CloneState
{

    BRAP_P_CloneState_eConfigured,  /* Cloning has been requested on channel 
                                      open and resources allocated for it */
    BRAP_P_CloneState_eStarted,     /* Cloned ports have been started */
    BRAP_P_CloneState_eInvalid      /* Cloning has not been configured at all */
}BRAP_P_CloneState;
#endif

#if (BRAP_AD_SUPPORTED == 1)
/***************************************************************************
Summary:
    Private Structre for BRAP_AudioDescriptorObject handle
**************************************************************************/
typedef struct BRAP_AudioDescriptorObject
{
    BRAP_AudioDescriptorFadeTableParams sExtAudioDescDetails; /* Exposed audio desc details */
    BRAP_Handle		                    hRap;		/* Parent Audio Device handle */
    uint32_t                            ui32PanFadeInterfaceAddr; /* Dram location for interface with fw */
    bool                                bAvailable;           /* Is this object free */  
}BRAP_AudioDescriptorObject;
#endif

/* Internal module handles associated with a channel */
typedef struct BRAP_P_ObjectHandles
{

	BRAP_DSP_Handle		hDsp;		/* DSP Device Handle */
	BRAP_FMM_P_Handle	hFmm;		/* FMM Device Handle */
	BRAP_DSPCHN_Handle	hDspCh;		/* DSP Context Handle */
	BRAP_DSTCH_P_Handle	hDstCh;     /* Destination Channel handle. 
	                                   Valid only for Capture channels.
	                                   Invalid entries will be marked by NULL. */
	BRAP_RBUF_P_Handle	hRBuf[BRAP_RM_P_MAX_OP_CHANNELS];	/* Ring buffer 
									   handles for different output channels. 
									   Refer to enum BRAP_OutputChannelIndex 
									   for each output channel definition. 
									   Invalid entries will be marked by NULL.
									 */
	BRAP_SRCCH_P_Handle	hSrcCh[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]; /* Source Channel 
									   handles for different output channel pairs. 
									   Refer to enum BRAP_OutputChannelPairIndex 
									   for each output channel pair definition. 
									   Invalid entries will be marked by NULL.
									 */
	BRAP_MIXER_P_Handle	hMixer[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]; /* Mixer handles 
									   for different output channel pairs. Refer 
									   to enum BRAP_OutputChannelPairIndex 
									   for each output channel pair definition. 
									   Invalid entries will be marked by NULL.
									 */
	unsigned int		uiMixerInputIndex[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]; /* Index  
									   of the corresponding mixer input for 
									   different output channel pairs. Refer 
									   to enum BRAP_OutputChannelPairIndex 
									   for each output channel pair definition. 
									   Invalid entries will be marked by 
									   BRAP_INVALID_INDEX.
									 */
	BRAP_SPDIFFM_P_Handle	hSpdifFm[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]; /* MS handles 
									   for different output channel pairs. Refer 
									   to enum BRAP_OutputChannelPairIndex 
									   for each output channel pair definition. 
									   Invalid entries will be marked by NULL.
									 */
	BRAP_OP_P_Handle		hOp[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];	/* Output handles 
									   for different output channel pairs. Refer 
									   to enum BRAP_OutputChannelPairIndex 
									   for each output channel pair definition. 
									   Invalid entries will be marked by NULL.
									 */
}BRAP_P_ObjectHandles;

#ifndef BCHP_7411_VER  /* not for 7411 */  
/* Handles of the FMM submodules associated with a 'cloned' output port for routing to multiple output ports */


typedef struct BRAP_P_OpPathHandles
{
    bool  bSimul; /*TRUE: this is a simul port. FALSE: this is a normal port */
    BRAP_RBUF_P_Handle  hRbuf[BRAP_RM_P_MAX_RBUFS_PER_SRCCH]; /* Ring Buffer Handle*/
	BRAP_SRCCH_P_Handle	hSrcCh; /* Source Channel */
	BRAP_MIXER_P_Handle	hMixer; /* Mixer handles  */
	unsigned int		uiMixerInputIndex; /* Index of the corresponding mixer input*/
	BRAP_SPDIFFM_P_Handle	hSpdifFm; /* MS handles */
	BRAP_OP_P_Handle		hOp;	/* Output handle.*/
}BRAP_P_OpPathHandles;
#endif
typedef struct BRAP_P_TrickModeState
{
	unsigned int	uiFrameAdvResidualTime;	/* Residual time of frame advance trick
												mode */
	bool			bAudioPaused;			/* Current status of audio decoder
											   wrt pause trick mode */
} BRAP_P_TrickModeState;

typedef struct BRAP_P_GetInterruptCount
{
	uint32_t    uiFmmDstRbufFullWaterMark;	
    uint32_t    uiPtsError;
    uint32_t    uiDecoderUnlock;    
    uint32_t    uiFmmCRC;
    uint32_t    uiCrcError;
	uint32_t	uiFramesInError;
	uint32_t	uiTotalFrameDecoded;
} BRAP_P_GetInterruptCount;

/* Structure defining the state of a Raptor Audio Channel */
typedef struct BRAP_P_Channel
{
	
	BCHP_Handle		hChip;			/* Handle to chip object */	
	BREG_Handle		hRegister;		/* Handle to register object */
	BMEM_Handle		hHeap;			/* Handle to memory object */
	BINT_Handle		hInt;			/* Handle to interrupt object */
	BRAP_Handle		hRap;			/* Parent Audio Device handle */
#if BRAP_P_USE_BRAP_TRANS ==0
	unsigned int	uiXptChannelNo;	/* Transport channel number */
#else
	BRAP_TRANS_ChannelHandle	hRapTransCh;/* Associated Raptor Audio 
									   transport channel handle */
#endif
#if BRAP_P_WATERMARK_WORKAROUND==1
	BTMR_TimerHandle		hTimer;		/* Handle to timer object */
#endif	

	BRAP_P_ChannelType	eChannelType;/*Channel type: Decode, PCM Playback, 
									   PCM Capture */
	BRAP_CaptureMode  eCapMode;  /* Capture mode: capture only, Bypass or full Duplex
                                                              valid only for a capture channel */
	BRAP_CapInputType	eCapInputType;	/* Type of PCM Capture Input Source.
                                           Valid only for a capture channel */
#ifdef BCHP_7411_VER  /* only for 7411 */  											   	
	BRAP_OutputMode	eMaxOpMode;		/* Audio output mode corresponding 
									   to the maximum number of audio 
									   channels supported by this channel */
	BRAP_OutputMode	eCurOpMode;		/* Present audio output mode 
									   supported by this channel */
#endif									   	
	BRAP_BufDataMode eMaxBufDataMode; /* Max Buffer Data Mode. This will be 
										provided during channel open time for 
										PCM Playback and PCM Capture. The 
										buffer data mode passed during channel 
										start time (eBufDataMode) should be 
										less then or equal to this. */									   	
	BRAP_BufDataMode eBufDataMode;  /*	Buffer Data Mode. This will decide the 
										number of ring buffers required to be 
										allocated. This will be provided during 
										channel start time for PCM Playback and 
										PCM Capture. It should be less then or 
										equal to eMaxBufDataMode.  */									   	
	unsigned int	uiChannelNo;	/* Channel id of the the type of 
									   channel specified by eChannelType */
	bool			bSimulModeConfig;	/* TRUE: Simultaneous mode can 
   										   be used for this channel,
										   FALSE:Otherwise */
	bool			bMuxChPairOnI2s;	/* TRUE: 8 channel support using two
	                                       I2S ports
	                                       FALSE: Otherwise */
#ifdef BCHP_7411_VER  /* only for 7411 */  										   
	BRAP_P_CloneState   eClone;         /* State of cloning: whether it has 
                                           been configured or started etc */
#endif    
	BRAP_P_ObjectHandles	sModuleHandles;/* Internal module handles used. 
									   This information is populated with the 
									   help of the Resource Manager. If a 
									   channel is not valid, corresponding 
									   handle will contain NULL inside this 
									   structure. */
	
	BRAP_P_ObjectHandles	sSimulPtModuleHandles; /* Additional module 
									   handles required for the Pass-thru 
									   context of Simultaneous mode/Clone. This 
									   field is used only if the channel 
									   is configured to be in simultaneous 
									   mode i.e. bConfigureForSimMode = TRUE */

	BRAP_RM_P_ResrcGrant	sRsrcGrnt;	/* Resource granted by the 
									   resource manager */
	BRAP_RM_P_ResrcGrant	sSimulPtRsrcGrnt;	/* Resource granted by the 
									   resource manager for the Pass-thru 
									   context of Simultaneous mode / clone in decode mode. This 
									   field is used only if the channel 
									   is configured to be in simultaneous 
									   mode i.e. bConfigureForSimMode = TRUE */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
    BRAP_P_OpPathHandles    sCloneOpPathHandles[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS][BRAP_RM_P_MAX_OUTPUTS]; /*Module handles for all the resources associated with a cloned OP port */     
    BRAP_RM_P_OpPathResrc   sCloneOpPathResc[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS][BRAP_RM_P_MAX_OUTPUTS]; /* Resource Ids for all the resources associated with a cloned OP port */     

    BINT_CallbackHandle hDSPCallback;  /* Handle for AUD_DSP_INTH0_R5F_STATUS L2 RAPTOR
    									interrupt callbacks */

    BINT_CallbackHandle hFMMCallback; /* Handle for AIO_INTH_R5F_STATUS L2
    									RAPTOR interrupt callbacks */
#if ((BCHP_CHIP == 7400 && BCHP_VER!=A0)||(BCHP_CHIP == 7403))
     BINT_CallbackHandle     hFMMOpCallback; /* Handle for Output RAPTOR interrupt callbacks */
#endif
											
#else
    BINT_CallbackHandle hCallback;  /* Handle for top level L2 RAPTOR interrupt callbacks */
#endif
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    bool                    bLargeRbuf;  /* TRUE: Output ports connected to this 
                                            channel require larger 
                                            rbufs in order to support independent delay.
                                            FALSE: Normal RBUF size will be used 
                                            => No output port connected to this 
                                            channel can have an independent delay */                                                                
#endif
    
    BRAP_P_AppIntCbInfo sAppIntCbInfo[BRAP_Interrupt_eMaxInterrupts]; /* Contains 
    info about application interrupt callbacks. Is filled when user installs an 
    application level callback. */    

    uint32_t        ui32FmmIntMask; /* Bits in AIO_INTH_R5F that are unmasked */
    uint32_t        ui32FmmBFIntMask; /* Bits in AUD_FMM_BF_ESR1 that are unmasked */
    uint32_t        ui32DspIntMask; /* Bits in AUD_DSP_INTH0_R5F that are unmasked */
	BRAP_P_TrickModeState	sTrickModeState;	/* Trick mode state of the channel */
	bool			bInternalCallFromRap;	/* Flag to distinguish whether an API
											   is getting called from external
											   application or from within RAP modules */
	bool 		bStarted;				/* Decode channel start/stop state */
	bool			bMuxChPairOnI2sStarted;		/* Multichannel start/stop state */	
    BRAP_GetInterruptCount hInterruptCount;	
#if (BRAP_AD_SUPPORTED == 1)
    BRAP_AudioDescriptorFadeHandle hAdFade; /* Audio Descriptor handle for this channel */
#endif 
#ifdef RAP_SRSTRUVOL_CERTIFICATION  
        BRAP_BufInfo  sPcmBufferInfo;  /*PCm Buffer address for SRS Truvolume Certification*/
#endif
}BRAP_P_Channel;

/***************************************************************************
Summary:
	Structure for statically allocated chunk of memory for Raptor PI usage 
Description:
	This structure stores the information needed for static allocation 
	of the block passed to the RAP PI.
See Also:
****************************************************************************/
typedef struct BRAP_P_StaticAllocInfo	
{
	uint32_t			currentStaticMemoryPointer;
	uint32_t			currentStaticMemoryBase;
	uint32_t			staticMemorySize;
}BRAP_P_StaticAllocInfo;

/* This structure contains information regarding memory requirement
 * for a decode channel for a given decode mode */
typedef struct BRAP_DEC_P_ChannelMemReq
{
	unsigned int	ui32FwCodeSize;		/* FW Code + Table size */
	unsigned int	ui32ScratchBufSize;	/* Scratch buf size */
	unsigned int	ui32InterframeBufSize;	/* Interframe Buf size */
	unsigned int	ui32DecConfigParamBufSize;	/* Decoder configuration parameter Buf size */
	unsigned int	ui32RingBufSize;		/* Ring Buf Size */
	unsigned int 	ui32FsSize;			/*Frame Sync Size*/
#if (BRAP_DSP_P_7401_NEWMIT==1)
	unsigned int	uiInterstageBufSize;
	unsigned int	uiInterstageInterfaceBufSize;
	unsigned int	ui32PpFwCodeSize;		        /* FW Code + Table size */
    unsigned int    ui32PpScratchBufSize;
    unsigned int    ui32PpInterframeBufSize;
    unsigned int    ui32PpConfigParamBufSize;
#endif    
} BRAP_DEC_P_ChannelMemReq;

/* This structure contains information regarding various buffer sizes required for an 
  * algorithm for a given decode mode */
#if (BRAP_DSP_P_7401_NEWMIT==1)	
typedef struct BRAP_P_DecodeModeMemRequirement
{
	uint32_t	ui32FirmwareExecutablesSize;
	uint32_t	ui32InterframeBufSize;
	uint32_t	ui32InterstageInputBufSize;
	uint32_t	ui32InterstageInputInterfaceBufSize;
	uint32_t	ui32InterstageOutputBufSize;
	uint32_t	ui32InterstageOutputInterfaceBufSize;
	uint32_t	ui32DecScratchBufSize;
	uint32_t	ui32DecoderConfigParamStructSize;
} BRAP_P_DecodeModeMemRequirement;
typedef struct BRAP_P_PpModeMemRequirement
{
	uint32_t	ui32PpFirmwareExecutablesSize;
	uint32_t	ui32PpInterframeBufSize;
	uint32_t	ui32PpInterstageInputBufSize;
	uint32_t	ui32PpInterstageInputInterfaceBufSize;
	uint32_t	ui32PpInterstageOutputBufSize;
	uint32_t	ui32PpInterstageOutputInterfaceBufSize;
	uint32_t	ui32PpScratchBufSize;
	uint32_t	ui32PpConfigParamStructSize;
} BRAP_P_PpModeMemRequirement;
#else  
typedef struct BRAP_P_DecodeModeMemRequirement
{
	uint32_t			ui32FirmwareExecutablesSize;
	uint32_t			ui32InterframeBufSize;
	uint32_t			ui32InterstageBufSize;
	uint32_t			ui32InterstageInterfaceBufSize;
	uint32_t			ui32DecScratchBufSize;
	uint32_t			ui32DecoderConfigParamStructSize;
} BRAP_P_DecodeModeMemRequirement;
#endif

/* This structure contains information regarding total memory required for an 
  * algorithm considering all decode modes. It doesn't include following
  * common buffers that are shared between algorithms/FW-contexts.
  * CDB/ITB
  * Common Post Processing buffers (For downloading FW image)
  * Firmware Resident code
  * Master Index Table structure */
#if (BRAP_DSP_P_7401_NEWMIT==1)	
typedef struct BRAP_P_Dec_AlgoMemRequirement
{
	BRAP_P_DecodeModeMemRequirement	sDecodeMemReq;
	uint32_t			ui32PassThruExecSize;
	uint32_t			ui32RingBufSize;
} BRAP_P_Dec_AlgoMemRequirement;
typedef struct BRAP_P_Pp_AlgoMemRequirement
{
    BRAP_P_PpModeMemRequirement sPpMemReq;
} BRAP_P_Pp_AlgoMemRequirement;
#else  
typedef struct BRAP_P_AlgoMemRequirement
{
	BRAP_P_DecodeModeMemRequirement	sDecodeMemReq;
	uint32_t			ui32PassThruExecSize;
	uint32_t			ui32RingBufSize;
	uint32_t 			ui32FrameSyncSize;
} BRAP_P_AlgoMemRequirement;
#endif

/* Structure defining the state of a Raptor Audio Device */
typedef struct BRAP_P_Device
{

	BCHP_Handle		hChip;			/* handle to chip object */
	BREG_Handle		hRegister;		/* handle to register object */
	BMEM_Handle		hHeap;			/* handle to memory object */
	BINT_Handle		hInt;			/* handle to interrupt object */
#if BRAP_P_WATERMARK_WORKAROUND == 1
	BTMR_Handle		hTmr;			/* handle to timer object */
#endif

	BRAP_ChannelHandle	hRapDecCh[BRAP_RM_P_MAX_DEC_CHANNELS];	/* Decode channel handles */
	BRAP_ChannelHandle	hRapPbCh[BRAP_RM_P_MAX_PCM_CHANNELS];	/* Playback channel handles */
	BRAP_ChannelHandle	hRapCapCh[BRAP_RM_P_MAX_CAP_CHANNELS];	/* Capture channel handles */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	BRAP_ENC_ChannelHandle	hRapEncCh[BRAP_RM_P_MAX_ENC_CHANNELS];	/* Encode channel handles */
#endif
		
	BRAP_DSP_Handle		hDsp[BRAP_RM_P_MAX_DSPS];		/* DSP Device handles */
	BRAP_FMM_P_Handle	hFmm[BRAP_RM_P_MAX_FMMS];		/* FMM Device handles */
	BRAP_RM_Handle		hRm;		/* Resource Manager handle */

	BRAP_Settings	sSettings;		/* Audio Device settings provided 
									   by the caller */

	/* Output settings provided by the user */
	/* Each element of following two arrays correspondings to one output 
	   port as defined by the enum BRAP_OutputPort. If an element of 
	   bOpSettingsValid array is TRUE, corresponding output settings is 
	   passed by the user and is stored at the corresponding element of 
	   sOutputSettings. FALSE indicates otherwise. 
	*/
	bool				bOpSettingsValid[BRAP_RM_P_MAX_OUTPUTS];
	BRAP_OutputSettings	sOutputSettings[BRAP_RM_P_MAX_OUTPUTS];
    unsigned int        uiRampStepSize; /* Scale/volume ramp step size in 1/100 dB */     
	
	BRAP_P_StaticAllocInfo	sMemAllocInfo;			/* Static memory allocation info */
	unsigned int			uiTotalMemUsed;		/* Current device memory usage of RAP PI */
	bool				bWatchdogRecoveryOn; /* Watchdog recovery on flag */
	bool 			bResetHwDone; 		/*Hardware Reset Done or not.
										If Dsp Hardware Reset Done then bResetHwDone=true,
										otherwise bResetHwDone=false.
										This variable is used when application call start and stop decode,
										when Dsp hardware is reset*/

    BRAP_P_TestToneRegs sToneRegs;      /* Registers saved/restored when enabling/
                                       disabling the testtone. */
	
    BINT_CallbackHandle hCallback;  /* Handle for top level L2 RAPTOR device level interrupt callbacks */

    BRAP_P_AppIntCbInfo sAppIntCbInfo[BRAP_Interrupt_eMaxDeviceLevelInterrupts]; /* Contains 
    								info about application interrupt callbacks for device level 
									interrupts. Is filled when user installs an 
    								application level callback. */

	unsigned int		uiDecodeCount;	/* Current count of decode operations. Decode part of
										simul mode operations also included in the count. */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	unsigned int		uiEncodeCount;	/* Current count of encode operations. */
#endif
	unsigned int		uiPassThruCount;	/* Current count of pass thru operations. Pass thru part of 
										simul mode operations also included in the count. */
#if (BRAP_AD_SUPPORTED == 1)
    BRAP_AudioDescriptorObject  sAudioDescObjs[BRAP_P_MAX_AUDIO_DESC_OBJECTS];  /* Audio Descriptor Objects */
#endif
                                          
}BRAP_P_Device;

/* BRAP_P_OpPortPrevMuteStatus: Structure that holds mute states of all the output ports
  * associated with a decode channel. This structure is used in sample rate interrupt handling.
  * Structure is initialized with previous mute states (prior to sample rate interrupt occurrence) 
  * of output ports. All the output ports are muted as part of sample rate interrupt handling. 
  * At the end of sample rate handler, previous mute states are read from this structure and
  * restored back.
  */
typedef struct BRAP_P_OpPortPrevMuteStatus
{
	bool bDecMuteStatus[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];	/* Array to hold mute states
															    of output ports that have
															    their handles in sModuleHandle */
	bool bSimulPtMuteStatus[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];	/* Array to hold mute states
																     of output ports that have
																     their handles in sSimulPtModuleHandles */
	bool bCloneMuteStates[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS][BRAP_RM_P_MAX_OUTPUTS];
																	 /* Array to hold mute states
																	      of output ports that have
																	      their handles in sCloneOpPathHandles */
} BRAP_P_OpPortPrevMuteStatus;

/*}}}*/


/*{{{ Function prototypes */

BERR_Code BRAP_P_Open(
	BRAP_Handle 			hRap	/* [in] The Raptor Audio device handle*/
);

BERR_Code BRAP_P_OpenOpPathFmmModules
(
	BRAP_ChannelHandle 		hRapCh,			/* [in] The RAP Channel handle */
	BRAP_RBUF_P_Settings	*pRbufSettings, /* [in] Pointer to an array of 2 
													ring buffer settings */
	BRAP_SRCCH_P_Settings	*pSrcChSettings,/* [in] Source channel settings */
	BRAP_MIXER_P_Settings	*pMixerSettings,/* [in] Mixer settings */
	BRAP_SPDIFFM_P_Settings	*pSpdfFmSettings,/* [in] Spdif Formater settings */
	void					*pOpSettings,	/* [in] Output settings */
	BRAP_RM_P_OpPathResrc	*pOpPathResrc,	/* [in] The FMM Module ids to be opened */
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [out] Internal Module Handles */
);

BERR_Code BRAP_P_CloseOpPathFmmModules
(
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [in] Internal Module Handles */
);

BERR_Code BRAP_P_StartOpPathFmmModules
(
	BRAP_ChannelHandle 		hRapCh,			/* [in] The RAP Decode Channel handle */
	BRAP_RBUF_P_Params		*pRBufParams,	/* [in] Ring buffer start params */
	BRAP_SRCCH_P_Params		*pSrcChParams,	/* [in] Source channel start params */
	BRAP_MIXER_P_Params		*pMixerParams,	/* [in] Mixer start params */
	BRAP_SPDIFFM_P_Params	*pSpdifFmParams,/* [in] Spdif Formater start params */
	void					*pOpParams,		/* [in] Output params */
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [in] Internal Module Handles */
);

BERR_Code BRAP_P_StopOpPathFmmModules
(
	BRAP_OutputChannelPair	eOpChannel,		/* [in] Output Audio channel type */
	BRAP_P_ObjectHandles	*pModuleHandles /* [in] Internal Module Handles */
);

/* BRAP_P_MuteChannelOutputOnSr_isr: Mutes the output ports associated with
 * a decode channel at output port level on sample rate change. Returns the
 * previous mute status of output ports in psOpPortPrevMuteStatus.
 */
BERR_Code 
BRAP_P_MuteChannelOutputOnSr_isr
(
	BRAP_ChannelHandle hRapCh,		/* [in] The RAP decode channel handle */ 
	BRAP_P_OpPortPrevMuteStatus *psOpPortPrevMuteStatus /* [out] Returns previous mute states of output ports */
);

/* BRAP_P_UnMuteChannelOutputOnSr_isr: Unmutes the output ports associated with
 * a decode channel at output port level on sample rate change. Unmutes only those
 * output ports for which previous mute status was unmute. Previous mute states of
 * output ports are passed in parameter psOpPortPrevMuteStatus.
 */
BERR_Code 
BRAP_P_UnMuteChannelOutputOnSr_isr
(
	BRAP_ChannelHandle hRapCh, 		/* [in] The RAP decode channel handle */ 
	BRAP_P_OpPortPrevMuteStatus *psOpPortPrevMuteStatus /* [in] Previous mute states of output ports */
);

/* BRAP_P_GetWatchdogRecoveryFlag: Returns watchdog recovery flag */
bool BRAP_P_GetWatchdogRecoveryFlag(BRAP_Handle hRap);

/* BRAP_P_GetInternalCallFlag: Returns Internal call flag
 * Internal Call Flag = True => Function is getting called due to top level exposed API
 * getting internally called (e.g. BRAP_DEC_Start() getting called from BRAP_DEC_Flush() etc)
 * Internal Call Falg = False => Function is getting called due to top level exposed API
 * getting called externally (e.g. BRAP_DEC_Start() getting called from application )
 */
bool BRAP_P_GetInternalCallFlag(BRAP_ChannelHandle hRapCh);

void *BRAP_P_AllocAligned(
		BRAP_Handle  hRap,        /* [in] The RAP device handle */
		size_t       ulSize,      /* [in] size in bytes of block to allocate */
		unsigned int uiAlignBits, /* [in] alignment for the block */
		unsigned int uiBoundary   /* [in] boundary restricting allocated value */
#if (BRAP_SECURE_HEAP==1)
		,bool bAllocSecureMem		/*[in ] Enabled if the memory is allocated from secure memory*/
#endif		
		);

/* BRAP_P_Free : A wrapper around the BMEM module to  free device memory.
 * Device memory is actually freed only for non-7411 chips. For 7411 chips, this
 * function just returns.
 */
BERR_Code BRAP_P_Free(
		BRAP_Handle  hRap,        	/* [in] The RAP device handle */
		void *pMemPtr			/* [in] Pointer to device memory to be freed */
#if (BRAP_SECURE_HEAP==1)
		,bool bAllocSecureMem		/*[in ] Enabled if the memory is allocated from secure memory*/
#endif
		);


/* BRAP_P_ConvertAddressToOffset: Wrapper on BMEM function BMEM_ConvertAddressToOffset.
 * Since for 7411 chips, RAP PI doesn't get virtual pointer to device memory and access
 * device memory using physical address only, this function returns device offset equal
 * to virtual pointer to device memory for these chips. For all other chips it calls 
 * BMEM function BMEM_ConvertAddressToOffset for this conversion.
 */

BERR_Code BRAP_P_ConvertAddressToOffset(
	BMEM_Handle  Heap,    /* Heap that contains the memory block. */
	void        *pvAddress, /* Address of the memory block */
	uint32_t    *pulOffset   /* [out] Returned device offset. */
);

/* BRAP_P_ConvertAddressToOffset_isr: Isr version of BRAP_P_ConvertAddressToOffset */

BERR_Code BRAP_P_ConvertAddressToOffset_isr(
	BMEM_Handle  Heap,    /* Heap that contains the memory block. */
	void        *pvAddress, /* Address of the memory block */
	uint32_t    *pulOffset   /* [out] Returned device offset. */
);

/* BRAP_P_ConvertOffsetToAddress: Wrapper on BMEM function BMEM_ConvertOffsetToAddress.
 * Since for 7411 chips, RAP PI doesn't get virtual pointer to device memory and access
 * device memory using physical address only, this function returns device offset equal
 * to virtual pointer to device memory for these chips. For all other chips it calls 
 * BMEM function BMEM_ConvertOffsetToAddress for this conversion.
 */
BERR_Code BRAP_ConvertOffsetToAddress
(
	BMEM_Handle   hHeap,    /* Heap that contains the memory block */
	uint32_t      ulOffset,  /* Device offset within the heap. */
	void        **ppvAddress  /* [out] Returned address. */
);

void BRAP_P_DownloadDspFwInMem(BARC_Handle hArc, uint32_t *data, uint32_t memAdr, uint32_t size);

/* BRAP_P_GetDecChannelBufferSizes : This function finds out sizes of various buffers
    required for combination of algorithms selected by application for a RAP decode
    channel.
    */
BERR_Code BRAP_P_GetDecChannelBufferSizes(
						bool *pbSupportAlgos,	/* [in] pointer to bSupportAlgos[BRAP_MAX_AUDIO_TYPES] array */
						BRAP_DSPCHN_DecodeMode eDecodeMode, /* [in] Decode mode */
#ifdef BCHP_7411_VER
						BRAP_AudioMemAllocation	eAudioMemAllocation, /* [in] Audio memory allocation type */
#endif
						BRAP_DEC_P_ChannelMemReq	*psChMemReq /* [out] Dec channel memory requirement */
#ifndef BCHP_7411_VER			
		, 
		bool bFwAuthEnable	 /*Firmware authentication Enable bit */
#endif		
);

/* BRAP_P_GetSelectedAlgos: This function returns audio types selected by application
 * during BRAP_Open call */
BERR_Code BRAP_P_GetSelectedAlgos(
			BRAP_Handle	hRap,		/* [in] RAP Handle */
			bool *pbSelectedAlgos	/* [in] pointer to bSupportAlgos[BRAP_MAX_AUDIO_TYPES] array */
);

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
/* BRAP_P_GetSelectedPpAlgos: This function returns post processing algorithms selected
  * by application during BRAP_Open call */
 BERR_Code BRAP_P_GetSelectedPpAlgos(
			BRAP_Handle	hRap,		/* [in] RAP Handle */
			bool *pbSelectedPpAlgos	/* [out] pointer to bSupportPpAlgos[BRAP_MAX_PP_ALGOS] array */
);
#endif

/* BRAP_P_GetAlgoMemRequirement: This function gets total memory required for an
 * algorithm. This memory requirement is not considering following buffers
 * CDB/ITB
 * Common Post Processing buffers (For downloading FW image)
 * Firmware Resident code
 * Master Index Table structure */
#if (BRAP_DSP_P_7401_NEWMIT==1)
BERR_Code BRAP_P_GetAlgoMemRequirement (
				BRAP_Handle Rap, 	/* [in] RAP Handle */
				BRAP_DSPCHN_AudioType eAudioType,	/* [in] Audio Type */
				BRAP_P_Dec_AlgoMemRequirement *psAlgoMemReq /* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given algorithm */
);
#else
BERR_Code BRAP_P_GetAlgoMemRequirement (
				BRAP_Handle Rap, 	/* [in] RAP Handle */
				BRAP_DSPCHN_AudioType eAudioType,	/* [in] Audio Type */
				BRAP_P_AlgoMemRequirement *psAlgoMemReq /* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given algorithm */
);
#endif

/* BRAP_P_GetPpAlgoMemRequirement: This function gets memory requirement for a
 * post processing algorithm. This function essentially gets the split-up of scratch buffer
 * as required by post processing algorithm.
 */
#if (BRAP_DSP_P_7401_NEWMIT==1)
BERR_Code BRAP_P_GetPpAlgoMemRequirement(
				BRAP_Handle hRap, 							/* [in] RAP Handle */
				BRAP_DSPCHN_PP_Algo	ePpAlgo,			/* [in] Post processing algorithm */
				BRAP_P_Pp_AlgoMemRequirement * psPpAlgoMemReq	/* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given post processing algorithm */
);
#else
BERR_Code BRAP_P_GetPpAlgoMemRequirement(
				BRAP_Handle hRap, 							/* [in] RAP Handle */
				BRAP_DSPCHN_PP_Algo	ePpAlgo,			/* [in] Post processing algorithm */
				BRAP_P_AlgoMemRequirement * psPpAlgoMemReq	/* [out] pointer to struct to be
														   initialized with memory requirement
														   for the given post processing algorithm */
);
#endif

#if (BCHP_7411_VER) || (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
/* BRAP_P_GetPpBufferSizes : This function finds out total memory
    required for combination of post processing algorithms selected by 
    application.
    */
void BRAP_P_GetPpBufferSizes(
						bool *pbSupportPpAlgos,	/* [in] pointer to bSupportPpAlgos[BRAP_MAX_PP_ALGOS] array */
						BRAP_DEC_P_ChannelMemReq	*psPpMemReq /* [out] Post processing memory requirement */
);
#endif

#ifndef EMULATION
void BRAP_MemWrite32_isr(BARC_Handle hArc,   /* ARC Handle */
					uint32_t	offset, /* Memory offset to write */
					uint32_t	data   /* Data to write */
					);
void BRAP_MemWrite32(BARC_Handle hArc,   /* ARC Handle */
					uint32_t	offset, /* Memory offset to write */
					uint32_t	data   /* Data to write */
					);
void BRAP_MemWrite8(BARC_Handle hArc,   /* ARC Handle */
					uint32_t	offset, /* Memory offset to write */
					uint8_t		data   /* Data to write */
					);
uint32_t BRAP_MemRead32_isr(BARC_Handle hArc,  /* ARC Handle */
						uint32_t	offset /* Memory offset from which read will happen */
					   );
uint32_t BRAP_MemRead32(BARC_Handle hArc,  /* ARC Handle */
						uint32_t	offset /* Memory offset from which read will happen */
					   );
uint8_t BRAP_MemRead8(	BARC_Handle hArc,  /* ARC Handle */
						uint32_t	offset /* Memory offset from which read will happen */
					   );

void BRAP_DSP_Write32(BARC_Handle	hArc,
					uint32_t		offset,
					uint32_t		data,
					bool			bIsrCxt);

uint32_t BRAP_DSP_Read32(BARC_Handle hArc,
					uint32_t		offset,
					bool			bIsrCxt);

#endif /* EMULATION */


#ifdef EMULATION
#define BRAP_Read32(hReg, addr)             BREG_Read32(hReg, addr)
#define BRAP_Write32(hReg, addr, data)      BREG_Write32(hReg, addr, data)
#define BRAP_Read32_isr(hReg, addr)         BRAP_Read32(hReg, addr)
#define BRAP_Write32_isr(hReg, addr, data)  BRAP_Write32(hReg, addr, data)
#elif BRAP_P_USE_BARC == 0 /* for chips other than 7411 */
#define BRAP_Read32(hReg, addr)             BREG_Read32(hReg, addr)
#define BRAP_Write32(hReg, addr, data)      BREG_Write32(hReg, addr, data)
#define BRAP_Read32_isr(hReg, addr)             BREG_Read32_isr(hReg, addr)
#define BRAP_Write32_isr(hReg, addr, data)      BREG_Write32_isr(hReg, addr, data)
#else /* For 7411 */
#define BRAP_Read32(hReg, addr)             BARC_Reg_Read32((BARC_Handle)(hReg), addr)
#define BRAP_Write32(hReg, addr, data)      BARC_Reg_Write32((BARC_Handle)(hReg), addr, data)
#define BRAP_Read32_isr(hReg, addr)             BARC_Reg_Read32_isr((BARC_Handle)(hReg), addr)
#define BRAP_Write32_isr(hReg, addr, data)      BARC_Reg_Write32_isr((BARC_Handle)(hReg), addr, data)
#endif

#ifdef EMULATION
#define BRAP_P_DRAMWRITE(hArc, addr, data)	zmw(addr, data)
#define BRAP_P_DRAMWRITE8(hArc, addr, data)	tb_zmw8(addr, data)
#define BRAP_P_DRAMREAD(hArc, addr)	    	zmr(addr)
#define BRAP_P_DRAMREAD8(hArc, addr)    	tb_zmr8(addr)
#else
#define BRAP_P_DRAMWRITE(hArc, addr, data)	BRAP_MemWrite32(hArc, addr, data)
#define BRAP_P_DRAMWRITE8(hArc, addr, data)	BRAP_MemWrite8(hArc, addr, data)
#define BRAP_P_DRAMREAD(hArc, addr)			BRAP_MemRead32(hArc, addr)
#define BRAP_P_DRAMREAD8(hArc, addr)		BRAP_MemRead8(hArc, addr)
#endif
	
#ifdef EMULATION
#define BRAP_MemWrite32(hArc, addr, data)  BRAP_P_DRAMWRITE(hArc, addr, data)
#define BRAP_MemWrite32_isr(hArc, addr, data)  BRAP_P_DRAMWRITE(hArc, addr, data)
#define BRAP_MemRead32(hArc, addr)  BRAP_P_DRAMREAD(hArc, addr)
#define BRAP_MemRead32_isr(hArc, addr)  BRAP_P_DRAMREAD(hArc, addr)
#define BRAP_DSP_Write32(hArc, addr, data, bIsrCxt) BRAP_Write32(hArc, addr, data)
#define BRAP_DSP_Read32(hArc, addr, bIsrCxt) BRAP_Read32(hArc, addr)
#endif


#ifndef BCHP_7411_VER /* For chips other than 7411 */
#define BRAP_P_DummyMemRead32(hArc, addr) {}
#else
/* In 7411, memory writes must be followed by a dummy memory read
   to ensure the data is flushed properly */
#define BRAP_P_DummyMemRead32(hArc, addr)			\
		{											\
			uint32_t dummyVar;						\
			dummyVar = BRAP_MemRead32(hArc, addr);	\
		}
#endif

#ifndef BCHP_7411_VER /* For chips other than 7411 */
/* Read Physical Memory through MSA Client.
   It reads 8 32-bit words at a time.
   So uiPhyAddr should be a OWORD address */
void BRAP_P_ReadPhyMem( BREG_Handle hReg,
						uint32_t	uiPhyAddr,
						uint32_t*	pData);
/* Write Physical Memory through MSA Client.
   It writess 8 32-bit words at a time.
   So uiPhyAddr should be a OWORD address */
void BRAP_P_WritePhyMem( BREG_Handle hReg,
						 uint32_t	uiPhyAddr,
						 uint32_t*	pData);


void BRAP_P_DownloadDspFwInMemSwapped(BARC_Handle hArc, 
	uint32_t *data,
	uint32_t memAdr,
	uint32_t size);
#endif

/* This routine checks if an output port is used in a RAP channel.
   This routine checks sModuleHandles, sSimulPtModuleHandles and
   sCloneOpPathHandles to decide if an output port is in use by 
   this channel or not. 
   If an output port is used by a channel, this routine also informs
   the caller if the output port is a cloned port or not. This is an
   optional output parameter */    
BERR_Code
BRAP_P_IsOpPortUsedbyChannel(
	const BRAP_ChannelHandle 	hRapCh,     /* [in] The RAP Channel handle */
	const BRAP_OutputPort	    eOutputPort,/* [in] Output port */
    bool                        *pFlag,     /* [out] TRUE: output port eOutputPort 
                                                    is used by the channel hRapCh 
                                                FALSE: is not used by this channel*/
    bool                        *pClone     /* [out] TRUE: if output port is cloned port
                                                     FALSE: otherwise.
                                               This is an optional output parameter */
);


/* Returns the channel pair along with used for whether clone/SimulPt info if 
   the output is used by the channel */
BERR_Code
BRAP_P_GetChannelPairUsingOpPort_isr(
	const BRAP_ChannelHandle hRapCh,        /* [in] The RAP Channel handle */
	const BRAP_OutputPort	 eOutputPort, 	/* [in] Output port */
    unsigned int             *pChnPair,     /* [out] channel pair if eOutputPort 
                                                is used by the channel hRapCh */ 
    bool                     *pClone,       /* [out]TRUE: chan pair corresponds to 
                                               sCloneHandles */
    bool                     *pSimulPt      /* [out]TRUE: chan pair corresponds to 
                                               sSimulPtHandles */
                                            /* Note: if valid chan pair and both 
                                               pClone and pSimulPt are false, 
                                               then this is from sModuleHandle */
);

BERR_Code
BRAP_P_GetPllForOp(
	const BRAP_Handle 	hRap, /* [in] The RAP handle */
	const BRAP_OutputPort	eOutputPort,	/* [in] Output port */
       BRAP_OP_Pll  * pPll  /* [out] the Pll associated with this output port */
);

BERR_Code BRAP_P_ConvertDelayToStartWRPoint(
	unsigned int 			uiDelay /* micro sec */,
	BRAP_BufDataMode 		eBufDataMode,
	BAVC_AudioSamplingRate 	eSamplingRate,
	unsigned int			uiIpBitsPerSample,
    unsigned int	        *pStartWRPoint
	);

BERR_Code BRAP_P_GetBurstRepetitionPeriodForAlgo(
        BAVC_StreamType                 eStreamType,
                                        /* [in] audio stream type*/
        BRAP_DSPCHN_AudioType eAudioType, /* [in] audio type for which 
                                             burst repetition period is 
                                             sought */
        BRAP_SPDIFFM_P_BurstRepPeriod *eBurstRepPeriod
                                          /* [out] corresponding burst 
                                             repetition period for the 
                                             algo type passed */
);

/* This function gives the absolute value corresponding to the enum 
   BAVC_AudioSamplingRate */
BERR_Code BRAP_P_ConvertSR (
    BAVC_AudioSamplingRate   eSR,   /* [in] samping rate enum */
    unsigned int            *uiSR   /* [out] sampling rate unsigned value */
);

BERR_Code BRAP_P_GetCurrentMemoryUsage(
	BRAP_Handle 			hRap,		/* [in] The RAP device handle */
	unsigned int			*puiTotalMemUsed	/* [out] Total device memory used by RAP PI */
);
/*****************************************************************************
Summary:
    Used to unmask a particular interrupt.

Description:
    This PI should be used by the application to unmask an interrupt. This can 
    be done only for the interrupts that are handled by Raptor PI and listed in 
    BRAP_Interrupt. For all other interrupts, application has to use the 
    standard INT interface.

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_UnmaskInterrupt (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
);

/*****************************************************************************
Summary:
    Used to unmask a particular interrupt - isr version.

Description:
    ISR version routine for BRAP_P_UnmaskInterrupt().
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_UnmaskInterrupt_isr (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
);

/*****************************************************************************
Summary:
    Used to mask a particular interrupt.

Description:
    This PI should be used by the application to mask an interrupt. This can 
    be done only for the interrupts that are handled by Raptor PI and listed in 
    BRAP_Interrupt. For all other interrupts, application has to use the 
    standard INT interface.

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_MaskInterrupt (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
);

/*****************************************************************************
Summary:
    Used to unmask a particular interrupt - isr version.

Description:
    ISR version routine for BRAP_P_UnmaskInterrupt().
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_MaskInterrupt_isr (
	BRAP_ChannelHandle hRapCh,	/* [in] The RAP channel handle */
    BRAP_Interrupt  eInterrupt  /* [in] The interrupt that needs to be deactivated */
);

/***************************************************************************
Summary:
    This private routine is used to check if a new channel is compatible with
    existing channels.
Description:
    This private routine is used to check if a new channel is compatible with
    existing channels. This routine should be called at the very beginning of a 
    channel open(DEC/PB/CAP).

    The non support combinations of channels are as follows:
    1) Two DEC channels on the same output port i.e. 2 direct DEC channels
    2) Two DEC channels on different output ports where one is captured and
       mixed to another i.e. one direct DEC channel and another indirect DEC
       channel associated to an output port through a CAP channel.
    3) A CAP channel getting captured again by another CAP channel i.e. 
       one direct CAP channel and another indirect CAP channel

Returns:
	BERR_SUCCESS if compatible else error
See Also:
    BRAP_OP_P_GetAssociatedChannels
****************************************************************************/
BERR_Code BRAP_P_IsNewChanCompatible(
	BRAP_Handle 			hRap,		/* [in] The RAP device handle */
    BRAP_P_ChannelType      eNewChannelType,/*[in] The new RAP Channel type */
    BRAP_OutputPort         eOutputPort     /*[in] output port for the new channel */  
);


/***************************************************************************
Summary:
	Checks if Compressed channel being Mixed with PCM channel
Description:
 	This functions returns true of false depending on if compressed 
 	data on a channel is being mixed with PCm dat on other channel.
Returns:
        BERR_SUCCESS on success, else error.
        bIsCompressedMixedWithPCM would be true or false.
See Also:
 	None
**************************************************************************/
BERR_Code BRAP_P_CheckIfCompressedMixedWithPCM
(
	const BRAP_Handle hRap,			/*[in] The Rap Handle*/
	const BRAP_OutputPort eOutputPort, /*[in] The output port for the current channel*/
	bool bNewChannelCompressed,	/*[in] Current channel compressed or not*/
	bool *bIsCompressedMixedWithPCM	/*[out] TRUE: if compressed and PCM are being mixed 
										     FALSE: If not*/
);


/***************************************************************************
Summary:
        Given the timebase, bits per sample etc, this PI uses them to formulate the Output
        Param structure.
Description:        
        This is required to be done in many places. Hence it was extracted out as a 
        separate PI. It'll also ensure that if the OP param structure for 
        various ports are changed, or a new port is added, we need to update 
        it only in one place.

Returns:
        Address of Output Param structure
        BERR_SUCCESS on success, else error.
See Also:
 	None
**************************************************************************/
BERR_Code BRAP_P_FormOpParams
(
	BRAP_ChannelHandle     hRapCh,		/* [in] The RAP decode channel handle */ 
	const BRAP_OutputPort	    eOutputPort,/* [in] Output port */
	BAVC_Timebase                   eTimebase, /* [in] Timebase */
	unsigned int uiOutputBitsPerSample, /* [in] Bits per sample */
	void * * 			pParamAddr		/* [out] pointer of type void * to 
                                            return a pointer to the output parameter struct */	
);


#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)
/***************************************************************************
Summary:
	If the channel is carrying any PCM data, this PI programs the Adaptive Rate 
	control block with the correct SrcFifo number and sets the Window Width.
Returns:
        BERR_SUCCESS on success, else error.
See Also:
 	None
**************************************************************************/
BERR_Code BRAP_P_ProgramAdaptRateCtrl (
		BRAP_ChannelHandle		hRapCh
		);
#endif


/***************************************************************************
Summary:
    This API should be called for setting the Channel status params on-the-fly for SPDIF and HDMI.
    It should be called only after atleast one channel using this output port 
    is already running since it assumes that the sample rate info /dsp channel handle is 
    already available
    
    If there is any decode channel channel using this port, this PI will write the 
    new values to DRAM and the Fw will then program the CBIT registers.
    If there is no currently running decode channel using tihs port, the PI will 
    directly write to the CBIT registers.

    Note: if a channel using this port is started/restarted AFTER calling this PI, then the Channel status 
    params passed in BRAP_DEC_Start() [or corresponding PB/CAP start] will take effect.

See Also:
	BRAP_SetOutputConfig, BRAP_GetOutputDefaultConfig, BRAP_GetOutputConfig
****************************************************************************/
BERR_Code BRAP_P_SetCbit( 
	BRAP_Handle 				hRap,				/* [in] The Raptor Audio device handle*/
	const BRAP_OutputSettings	*pOutputSettings	/* [in] Output port settings*/ 
	);

#if BCHP_7411_VER > BCHP_VER_C0
/* BRAP_P_I2sAlignmentWorkaround: For 7411 I2S alignment all the three I2S needs to
 * be enabled and pulling data. For 7411D 'multichannel on I2S'  mode, application configures
 * two I2S (I2S0, I2S1) ports. This function is used as a workaround for configuring and
 * starting I2S2.
 * TODO: Remove this workaround and code properly.
 */
BERR_Code BRAP_P_I2sAlignmentWorkaround (
		BRAP_ChannelHandle		hRapCh
);

/* BRAP_P_I2sAlignmentWorkaroundStop: This function is used to stop I2S2 and
 * corresponding mixer in 'multichannel on I2S' mode.
 */
BERR_Code BRAP_P_I2sAlignmentWorkaroundStop ( 
		BRAP_ChannelHandle		hRapCh
);

BRAP_AudioMemAllocation BRAP_P_GetMemoryAllocationType (
							BRAP_Handle			hRap		/* [in] RAP Handle */
);

#endif

#if ( BCHP_CHIP == 7400 )
/**************************************************************************
Summary:
    Private function that Resets the CHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK
    bit for the SrcCh which is unmasked to recieve the interrupt.
    Resetting this bit is a part of hardware workaround. For resetting the 
    bit, First set it to 0, then set the bit to 1 again
**************************************************************************/
void BRAP_P_ReArmSrcChInterrupt(
	BRAP_ChannelHandle	hRapch
	);

/**************************************************************************
Summary:
	Isr version of BRAP_P_ReArmSrcChInterrupt.
**************************************************************************/

void BRAP_P_ReArmSrcChInterrupt_isr(
	BRAP_ChannelHandle	hRapch
	);
#endif

/*}}}*/


#ifdef __cplusplus
}
#endif

#endif /*}}} #ifndef _BRAP_PRIV_H__ */

/* End of File */
