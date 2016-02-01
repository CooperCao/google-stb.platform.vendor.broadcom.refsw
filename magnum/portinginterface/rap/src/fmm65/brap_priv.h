/***************************************************************************
*     Copyright (c) 2004-2012, Broadcom Corporation
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
*		This file contains all private macros, enums, structures and 
*		functions privately used inside the Audio Manager module of the 
*		Raptor PI.
*
* Revision History:
*
* $brcm_Log: $
* 
oftware_Devel/RAP_354856_PH_4_0/2   11/8/08 12:23a sgadara
* PR 42025: [3548,3556] Add volume control, with cascaded mixers.
* 
* Hydra_Software_Devel/RAP_354856_PH_4_0/1   10/31/08 6:08p sgadara
* PR 42025: [3548,3556] Add volume control, with cascaded mixers. --
* >Provide separate mixers for each output port.
* 
* Hydra_Software_Devel/76   10/29/08 3:28a speter
* PR 44771: [3548,3556] Adding interrupt for Ring Buffer Overflow and
* cleaning up base3 code to remove 3563 and 7440 specific code
* 
* Hydra_Software_Devel/75   10/18/08 8:25a speter
* PR 44768: [3548,3556,7405] Moving bIndOpVolCtrl from channel params to
* BRAP_Settings.
* 
* Hydra_Software_Devel/74   10/17/08 6:05a speter
* PR 47360: [3548,3556]: Changing the design of Spdif Rx. SPDIF Rx is now
* a device level interrupt and handling of digital input format change
* is moved from RAP PI to higher layer.
* 
* Hydra_Software_Devel/73   10/17/08 2:32a speter
* PR 47360: [3548,3556]: Changing the design of Spdif Rx. SPDIF Rx is now
* a device level interrupt and handling of digital input format change
* is moved from RAP PI to higher layer.
* 
* Hydra_Software_Devel/72   10/15/08 7:53p gautamk
* PR47934: [7405] Few seconds of noise on SPDIF/HDMI during PCM to
* Compressed .
* 
* Hydra_Software_Devel/71   10/13/08 5:55p gautamk
* PR43454: [7405] Fixing compilation warning.
* 
* Hydra_Software_Devel/70   10/13/08 5:47p gautamk
* PR42213: [7405] Programming SRC for the PB when O/P sample rate is
* different from input, During decode's sample rate interrupt.
* 
* Hydra_Software_Devel/69   10/10/08 10:25a speter
* PR 47764: [3548,3556] Merging the release branch 3.0 to main line of
* development
* 
* Hydra_Software_Devel/68   9/23/08 9:45p sgadara
* PR 47227: [3548,3556] Disabling the watermark workaround for Autotrap.
* 
* Hydra_Software_Devel/RAP_354856_PH_3_0/7   10/9/08 11:14a speter
* PR 45444: [3548,3556] Adding the equalizer code for Sampling rate
* 
* Hydra_Software_Devel/RAP_354856_PH_3_0/6   10/7/08 11:42a sgadara
* PR 47576: [3548,3556] Programming the application provided volume
* levels for destination at start time.
* 
* Hydra_Software_Devel/RAP_354856_PH_3_0/5   10/6/08 8:19p speter
* PR 45444: [3548,3556] Adding equalizer related code
* 
* Hydra_Software_Devel/RAP_354856_PH_3_0/4   10/6/08 7:40p speter
* PR 45444: [3548,3556]: Adding generic code for equalizer
* 
* Hydra_Software_Devel/RAP_354856_PH_3_0/3   10/3/08 2:42p speter
* PR 44835: [3548,3556] Changing the APIs to remove hRap as it can be
* derived internally
* 
* Hydra_Software_Devel/RAP_354856_PH_3_0/2   10/1/08 3:24p speter
* PR 46051: [3548,3556] Changing DDEnc to AC3Enc
* 
* Hydra_Software_Devel/RAP_354856_PH_3_0/1   9/23/08 4:58p speter
* PR 45444: [3548,3556]  Changing the implementation of hardware
* equalizer to reflect the latest changes in the equalizer algorithm
* delivered by Arch team
* 
* Hydra_Software_Devel/67   9/19/08 8:56p sgadara
* PR 44770: [3548,3556] Implementing the Destination based Interrupts for
* 'capture to ringbuffer' feature.
* 
* Hydra_Software_Devel/66   9/19/08 6:23p gautamk
* PR45739: [7405] Changing bLargeRbuf to bIndepDelayEnabled
* 
* Hydra_Software_Devel/65   9/16/08 3:45p speter
* PR 46942: [3548,3556] Merging back to main line of development to get
* the distinct output feature
* 
* Hydra_Software_Devel/64   9/16/08 2:44p sushmit
* PR 43454:[7405]Adding AC3 Encode Support.
* 
* Hydra_Software_Devel/63   9/15/08 3:48p gautamk
* PR43454: [7405] Do not create seperate path/branch for
* bDownmixedOpPort= true O/P, when outputmode is 2_0
* 
* Hydra_Software_Devel/62   9/12/08 4:19p gautamk
* PR43454: [7405] Commenting prototype
* 
* Hydra_Software_Devel/61   9/8/08 5:28p gautamk
* PR45739: [7405] Adding support for indep delay for o/p
* 
* Hydra_Software_Devel/RAP_354856_20080904_BRANCH/1   9/5/08 8:42p sgadara
* PR 44835: [3556] Implementing destination-based volume APIs for volume
* control on capture data.
* 
* Hydra_Software_Devel/60   9/4/08 11:04p speter
* PR 46565: [3548,3556] Adding SRS-HD support
* 
* Hydra_Software_Devel/59   9/4/08 11:42a speter
* PR 45444: [3548,3556] Making the equalizer functionality easily
* manageable by using a macro
* 
* Hydra_Software_Devel/58   8/28/08 12:22p speter
* PR 46047: [3548,3556] Adding support for Custom Bass and Custom
* Surround
* 
* Hydra_Software_Devel/57   8/21/08 4:38p speter
* PR 45444: [3548,3556] Adding the hardware equalizer support.
* Implementing the algorithm provided by the architecture team. This is
* the initial version and not complete
* 
* Hydra_Software_Devel/56   8/20/08 6:11p gautamk
* PR43454: [7405] Changes for Concurrent downmix.
* 
* Hydra_Software_Devel/55   8/14/08 12:08p sushmit
* PR 43454:[7405]Updating DSOLA Params.
* 
* Hydra_Software_Devel/54   8/12/08 11:02p sushmit
* PR 43454:[7405]Adding DSOLA.
* 
* Hydra_Software_Devel/53   8/10/08 6:55p sushmit
* PR 43454:[7405] DTSENC Updates.
* 
* Hydra_Software_Devel/52   8/7/08 9:49a speter
* PR 45444: [3548,3556] Fixing build break in 7405 family due to
* equalizer checkins
* 
* Hydra_Software_Devel/51   8/6/08 2:53p speter
* PR 45444: [3548,3556] Adding the hardware equalizer support for 5 band
* equalization and Tone control
* 
* Hydra_Software_Devel/50   7/29/08 3:11p gautamk
* PR45180: [7335] Fixing compiler error for vxworks
* 
* Hydra_Software_Devel/49   7/17/08 4:26p sgadara
* PR 44895: [3548] To bring up 'capture to ringbuffer'  feature on 3548.
* 
* Hydra_Software_Devel/48   7/10/08 6:23p gautamk
* PR41825: [7405] merging the code for multiple associatio from Branch to
* mainline.
* 
* Hydra_Software_Devel/47   7/10/08 5:03p speter
* PR 44701: [3548,3556] Adding support for post processing algorithms
* PL2, SRSXT and AVL
* 
* Hydra_Software_Devel/46   7/8/08 7:19p gautamk
* PR43454: [7405] written function to determine if the path outputs
* compress data.
* 
* Hydra_Software_Devel/45   7/4/08 4:32p gautamk
* PR43454: [7405] Implementation for Adaptive rate ctrl (2ms lipsync)
* 
* Hydra_Software_Devel/44   6/26/08 9:13p gautamk
* PR43454: [7405] Adding sanity check for the network for audio
* processing stages.
* 
* Hydra_Software_Devel/43   6/20/08 5:41a speter
* PR 38950: [3548,3556] Adding custom voice user config support
* 
* Hydra_Software_Devel/42   6/11/08 7:31a speter
* PR38950: [3548,3556] Changing generic use of BTSC as RF Audio
* 
* Hydra_Software_Devel/41   6/2/08 1:59p gautamk
* PR41726: [7405] removing unwanted functions and enums
* 
* Hydra_Software_Devel/40   5/30/08 8:47p speter
* PR 38950: [3548,3556] Adding hRap as parameter to AudioDesc Object
* 
* Hydra_Software_Devel/39   5/30/08 2:21p gautamk
* PR41726: [7405] changes for audio descriptor
* 
* Hydra_Software_Devel/38   5/30/08 4:04a speter
* PR 38950: [3548,3556] Adding the firmware interface address parameter.
* 
* Hydra_Software_Devel/37   5/29/08 3:29p gautamk
* PR41726: [7405] Adding function to insert/destroy stages internally in
* Task network.
* 
* Hydra_Software_Devel/36   5/29/08 4:05a speter
* PR 38950: [3548,3556] Adding the audio descriptor support
* 
* Hydra_Software_Devel/35   5/29/08 2:14a speter
* PR 38950: [3548,3556] Adding Audio Descriptor support
* 
* Hydra_Software_Devel/34   5/28/08 8:07p gautamk
* PR41726: [7405] Adding support for SRC postprocessing
* 
* Hydra_Software_Devel/33   5/21/08 7:20p gdata
* PR41726: [7405] Implementing review comments for Interrputs
* 
* Hydra_Software_Devel/32   5/20/08 7:06p gautamk
* PR42807: [7335] Fixing the error condition incase of watchdog recovery
* 
* PR42837:[7325]
* 
* Hydra_Software_Devel/31   5/20/08 4:19p speter
* PR 42110: [3548,3556] ADC code update
* 
* Hydra_Software_Devel/30   5/20/08 2:36p speter
* PR 38950: [3548,3556] Adding APIs for new features like Audio
* Descriptors and NICAM
* 
* Hydra_Software_Devel/29   5/14/08 4:29p speter
* PR 38950: [3548,3556] Adding code to support HIFIDAC1 and HIFIDAC2
* 
* Hydra_Software_Devel/28   5/9/08 4:56p speter
* PR38950: [3548,3556] Fixing the compilation errors
* 
* Hydra_Software_Devel/27   5/9/08 4:18p gautamk
* PR42302:[7405] If watchdogISR is trigerred in between when PI waiting a
* response/ack from FW DSP, Then PI shoul exit gracefully. And then
* ProcessWatchdogRecovery() function should be called
* 
* PR42504:[7405]
* 
* Hydra_Software_Devel/26   5/9/08 2:59p speter
* PR 38950: [3548,3556] Fixing the code for 3548 family
* 
* Hydra_Software_Devel/25   5/5/08 11:43a speter
* PR 38950: [3548] Making 3548_FAMILY macro and using it for both 3548
* and 3556 cases.
* 
* Hydra_Software_Devel/24   5/2/08 2:46p gautamk
* PR41726: [7405] Adding PI support for Fw Verification in mainline
* 
* Hydra_Software_Devel/23   4/30/08 6:59p gautamk
* PR41726: [7405] Adding code to program user config param for processing
* algos on the fly.
* 
* Hydra_Software_Devel/22   4/29/08 12:24p speter
* PR 38950: [3548] Fixing the PLL related registers using VCX0 RDB
* 
* Hydra_Software_Devel/21   4/28/08 4:30p speter
* PR 38950: [3548] Fixing compilation for 3548/3556.
* 
* Hydra_Software_Devel/20   4/22/08 1:59p gautamk
* PR41994: [7405] Using BCHP_VER_ Correctly...
* 
* Hydra_Software_Devel/19   4/21/08 6:25p gautamk
* PR41726: [7405] IMplementing review comments start/stop APis
* 
* Hydra_Software_Devel/18   4/11/08 12:51a gautamk
* PR34648: [7405] Adding PI support for Trick modes.
* 
* Hydra_Software_Devel/17   4/8/08 2:23p gautamk
* PR34648: [7405] Merging wdog code to mainline
* 
* Hydra_Software_Devel/RAP_NEWFWARCH_WDOG/3   4/8/08 2:03p gautamk
* PR34648: [7405] Merging wdog code to mainline
* 
* Hydra_Software_Devel/RAP_NEWFWARCH_WDOG/2   4/1/08 2:28p gautamk
* PR34648: [7405] Merging change to wdog branch
* 
* Hydra_Software_Devel/14   3/31/08 12:14p nitinb
* PR 34648: [7405] Implemented review comments: Removed unused handle
* hArc from memory read/write functions. Removed function
* BRAP_P_DummyMemRead32().
* 
* Hydra_Software_Devel/13   3/27/08 10:55p sushmit
* PR 34648: [7405] Adding XptContextMap to RapCh for saving over
* flush/watchdog
* 
* Hydra_Software_Devel/12   3/18/08 7:01p gautamk
* PR34648: [7405] Fixing Bugs for GenCItOutput and createProcessing stage
* 
* Hydra_Software_Devel/11   3/3/08 6:39p gautamk
* PR34648: [7405] Implementing review comments.
* 
* Hydra_Software_Devel/10   2/29/08 4:25p gautamk
* PR34648: [7405] Changing file names includes
* 
* Hydra_Software_Devel/9   2/22/08 4:19p gautamk
* PR34648: [7405] fixing coverity defects + implementing review comments
* 
* Hydra_Software_Devel/8   2/19/08 4:07p gautamk
* PR34648: [7405] Updating function names.
* 
* Hydra_Software_Devel/7   2/19/08 3:32p gautamk
* PR34648: [7405] updating file names and fuction names
* 
* Hydra_Software_Devel/6   2/15/08 5:26p gdata
* PR34648: [7405] Adding the changes to support the interrupt install
* before channel start
* 
* Hydra_Software_Devel/5   2/7/08 5:38p nitinb
* PR34648: [7405] Fixed compilation warnings.
* 
* Hydra_Software_Devel/4   1/31/08 5:24p gautamk
* PR34648: [7405] Removing BRAP_DSPCHN_DecodeMode_eSimulMode Enum
* 
* Hydra_Software_Devel/3   1/29/08 7:04p gautamk
* PR34648: [7405] programming CDB/ITB END_PTR
* 
* Hydra_Software_Devel/2   1/24/08 12:05a sushmit
* PR34648: [7405] Merging in updated code from Base2
* 
* Hydra_Software_Devel/120   1/7/08 1:26p speter
* PR 38033: [3563] Fixing the header file for D0 chip revision
* 
* Hydra_Software_Devel/119   12/24/07 3:47p gdata
* PR37996: [7440] Using cache write to speed up the memory write at FW
* download
* 
* Hydra_Software_Devel/118   12/22/07 10:45a sgadara
* PR 38424: [7405] Merging changes to get Autotrap up on 7405
* 
* Hydra_Software_Devel/117   12/20/07 3:15p sushmit
* PR 36084: [7405] Proper storage for scaling coeffs created
* 
* Hydra_Software_Devel/116   12/17/07 1:30p sushmit
* PR 36084: [7405] Making bIndOpVolCtrl start time parameter common for
* all platforms.
* 
* Hydra_Software_Devel/115   12/16/07 7:16p sushmit
* PR 36084: [7405] Adding support for adding separate mixer per output
* port.
* 
* Hydra_Software_Devel/114   12/7/07 11:15a gdata
* PR37996: [7440] Adding changes to make firmware ack as interrupt driven
* 
* Hydra_Software_Devel/113   12/4/07 8:51p gdata
* PR37880: [7440] Enabling Open time ring buffer allocation
* 
* Hydra_Software_Devel/112   12/4/07 8:26p gdata
* PR37880: [7440] Merging the Massproduction branch to main branch.
* 
* Hydra_Software_Devel/111   11/27/07 10:40a speter
* PR 37577: [3563] Merging branch Phase2.9 to main line
*
* Hydra_Software_Devel/110   11/20/07 8:44p sushmit
* PR 36877: [7335] Bringup PI
* 
* Hydra_Software_Devel/109   11/19/07 3:17p sushmit
* PR 36822: [7405] Making MAI take data straight from Mixer.
* 
* Hydra_Software_Devel/RAP_3563_PH_2_9/2   11/16/07 10:08a speter
* PR 37112: [3563] Custom PP Set Config workaround code.
* 
* Hydra_Software_Devel/RAP_3563_PH_2_9/1   11/5/07 5:34p speter
* PR 36083: [3563] Moving the Custom Mixer requirement code in a branch
* 
* Hydra_Software_Devel/108   10/18/07 5:25p speter
* PR 36083: [3563] Fixing the API SetGain and Amplification to use the
* Mixer level in BRAP_P_GetMixerForOpPort. This also requied isolating
* BRAP_P_GetMixerForOpPort for 3563.
* 
* Hydra_Software_Devel/107   10/18/07 2:14p speter
* PR 36083: [3563] Checking in the new Volume APIs related changes.
* 
* Hydra_Software_Devel/106   10/4/07 2:32p gautamk
* PR35688: [7405] Implementing FW authentication support (NDS support)
* for 7405 code base.
* 
* Hydra_Software_Devel/105   10/4/07 2:11p sushmit
* PR 35464: [7405] BRAP_DSP_P_Close memory leak fixed & some other
* changes for memory leak debugging
* 
* Hydra_Software_Devel/104   10/1/07 7:14p gdata
* PR 32768: [7440] Enabled Secondary Metadata flag
* 
* Hydra_Software_Devel/103   9/30/07 1:54p sushmit
* PR35140: [7405] Develop 7325 Raptor PI for Bringup
* 
* Hydra_Software_Devel/102   9/28/07 10:48a speter
* PR 32625: [3563] Move volume amplification states from output port
* handle to rap handle
* 
* Hydra_Software_Devel/101   9/18/07 5:03p speter
* PR 34996: [3563] Adding support for Custom Mode Equalizer
* 
* Hydra_Software_Devel/100   9/14/07 12:06p gdata
* PR 32768: [7440] Code modifications for Secondary Metadata
* 
* Hydra_Software_Devel/1   1/22/08 4:47p sushmit
* PR34648: [7405] Merging in updated code from Base2
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/16   1/16/08 4:49p nitinb
* PR34648: [7405] Added support for programming configuration parameters
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/15   1/16/08 11:58a sushmit
* PR34648: [7405] Downloading CIT to DRAM
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/14   1/10/08 6:46p gautamk
* PR34648: [7405] Files for Mapping Processing network to CIT input
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/13   1/10/08 3:19p sushmit
* PR 34648: [7405] Removing hDsp from hDspChn
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/12   1/7/08 4:20p gdata
* PR34648:  [7405]Adding the code for interrupt changes
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/11   1/4/08 1:52p bhanus
* PR34648: [7405] Adding changes for Common Audio Processing network for
* CIT module.
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/10   1/3/08 2:19p bhanus
* PR34648: [7405] Adding changes for Channel Audio Processing network for
* CIT module.
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/9   1/2/08 4:15p gdata
* PR34648: [7405] Adding changes for Interrupt
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/8   12/31/07 7:06p sushmit
* PR34648: [7405] Updating more code.
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/7   12/31/07 3:03p nitinb
* PR34648: [7405] Updated data structures to integrate firmware interface
* module with other PI modules.
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/6   12/28/07 9:52p sushmit
* PR 34648: [7405] Initial Checkins after DSP Bootup completion.
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/5   12/20/07 2:38p srajapur
* PR 34648: [7405] Added hRap handle in the Audioprocessing Handle.
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/4   12/14/07 2:04p gautamk
* PR34648: [7405] Adding updated files for download FW modules
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/3   12/13/07 2:21p srajapur
* PR 34648: [7405] Added #define and modified structures for
* adding/removing audio (pre and post) processing stages.
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/2   10/31/07 1:53p nitinb
* PR 34648: [7405] Implemented review comments of new API interface as
* mentioned in PR progress-log
* 
* Hydra_Software_Devel/Rap_7405_NewFwIf/1   10/3/07 4:50p gautamk
* PR34648: [7405] checkin updated files for new PI/FW arch
* 
* Hydra_Software_Devel/99   9/3/07 1:31p kagrawal
* PR 32768: [7440] Merging changes from 7440_Phase2.0_Fixes branch to the
* main line to fix the compilation break
* 
* Hydra_Software_Devel/98   8/24/07 7:03p bhanus
* PR 34340 : [7440] Added code to do downmix in DB path.
* 
* Hydra_Software_Devel/97   8/10/07 11:49a sushmit
* PR 30552: [3563] Fixing issues with Compressed->PCM->Compressed input
* swiching in SPDIF SIMUL mode.
* 
* Hydra_Software_Devel/96   8/9/07 3:07p sushmit
* PR 30552: [3563] Added Channel Link Create/Destroy APIs. Modified
* BRAP_ProcessDigitalInputFormatChange to take care of Simul mode for
* compressed input.
* 
* Hydra_Software_Devel/95   8/1/07 3:59p bmishra
* PR 23951: [7440] Disabling Unified SR and Seq Path for Autotrap
* 
* Hydra_Software_Devel/94   7/26/07 2:37p kagrawal
* PR 32986 : [7440] Enabling Unified SR and Sequential Path flags
* 
* Hydra_Software_Devel/93   7/25/07 12:42p kagrawal
* PR 33094: [7440]
* - Disabling Unified SR and Sequential Path macros
* - Compressed HBR - Increasing decode channel ring-buffer size from 48k
* to 96k
* 
* Hydra_Software_Devel/92   7/23/07 9:24p sundaram
* PR 32986 : [7440] Enabling Unified SR and Sequential Path flags
* 
* Hydra_Software_Devel/91   7/19/07 7:08p kagrawal
* PR 32986 : [7440] Re-enabling PCM Playback in interrupt mode
* 
* Hydra_Software_Devel/90   7/16/07 6:42p bhanus
* PR 32986 : [7440 B0] Enabling Timer mode for PCM Playback
* 
* Hydra_Software_Devel/89   7/15/07 2:06p bhanus
* PR 32986 : [7440 B0] Code modified for running two 5.1 capture in
* parallel.
* 
* Hydra_Software_Devel/88   7/12/07 6:48p gdata
* PR22490 : Merging the CRC code on main branch
* 
* Hydra_Software_Devel/87   7/9/07 4:21p bhanus
* PR 32828 : [7440 B] Code modified for Bitstream HBR bring up.
* - Added condition to throw error if App tries to add any compressed
* output port along with HBR compressed.
* 
* Hydra_Software_Devel/86   7/9/07 3:44p gautamk
* PR32789: [7440] Add Phase2B Feature - PCM Playback in Interrupt Mode
* 
* Hydra_Software_Devel/85   7/9/07 2:13p sushmit
* PR 31453: Watchdog support for 3563
* 
* Hydra_Software_Devel/84   7/5/07 4:13p sushmit
* PR 32735: Support on the fly change of SPDIF channel status bits from
* host through DSP.
* 
* Hydra_Software_Devel/83   7/3/07 7:04p bmishra
* PR 23951: Merging Autotrap related changes
* 
* Hydra_Software_Devel/82   6/28/07 4:16p sushmit
* PR 30171: Mixers allocated at same level
* 
* Hydra_Software_Devel/81   6/27/07 8:36p gautamk
* PR32504: Providing  SPDIF0/1 C-bit  params to DSP1 for XCD.
* 
* Hydra_Software_Devel/80   6/27/07 12:30p gautamk
* PR32504: Adding support for having SPDIF0 +SPDIF1 enabled together for
* Simul  mode.
* 
* Hydra_Software_Devel/79   6/21/07 12:20p speter
* PR 27646: [7405] Fixing the PP defines for 7405
* 
* Hydra_Software_Devel/78   6/15/07 5:06p kagrawal
* PR 30069: Fixing compilation breaks in 3563
* 
* Hydra_Software_Devel/77   6/15/07 2:26p kagrawal
* PR 30069: [7440] Merging Phase2.0 branch to Hydra_Software_Devel
* 
* Hydra_Software_Devel/RAP_7440_Phase2.0_Branch/4   6/13/07 7:30p gautamk
* PR30069: Adding Private functon BRAP_P_GetInterchannelVolume , which
* populates volume for each channel, which in turn is written to DDBM
* Config params.
* 
* Hydra_Software_Devel/RAP_7440_Phase2.0_Branch/3   6/11/07 7:05p gautamk
* PR30069: New Config Parameter structure for Speaker Management
* 
* Hydra_Software_Devel/RAP_7440_Phase2.0_Branch/2   6/9/07 5:29p kagrawal
* PR 30069: [7440] Added support for DTS-Neo PostProcessing
* 
* Hydra_Software_Devel/RAP_7440_Phase2.0_Branch/1   6/7/07 2:20p kagrawal
* PR 30069: Added support for branches and stages for DSP1 audio post
* processing
*
* Hydra_Software_Devel/76   6/14/07 2:06p sushmit
* PR 27646: 7405 has two PLLs
* 
* Hydra_Software_Devel/75   6/11/07 9:05p nitinb
* PR 29166: Moved volume states from output handle to RAP handle. This
* removes restriction of calling add destination API before calling
* volume APIs.
* 
* Hydra_Software_Devel/74   6/11/07 7:03p nitinb
* PR 29166: Moved output port mute states from output handle to RAP
* handle. This removes restriction of calling add destination API before
* calling mute API.
* 
* Hydra_Software_Devel/73   6/7/07 7:26p bhanus
* PR 30069 : [7440B0] Added code for Secondary Metadata Processing.
* 
* Hydra_Software_Devel/72   6/5/07 10:59a speter
* PR 31827: [3563] Merging from Phase 2.0 release branch to main
* 
* Hydra_Software_Devel/RAP_3563_PH_2_0/1   5/29/07 1:46p sushmit
* PR28719: Adding code for independent delay & fixing MULPP broken
* 
* Hydra_Software_Devel/71   5/25/07 6:22p kagrawal
* PR 30069: Added first-cut PB watchdog support
* 
* Hydra_Software_Devel/70   5/14/07 5:24p kagrawal
* PR 30069: Added following support:
* -
* 
* Hydra_Software_Devel/69   5/11/07 8:59p sushmit
* PR 27646: Getting 7405 PI compiled
* 
* Hydra_Software_Devel/68   5/11/07 4:17p nitinb
* PR 30180: C0: Raptor: IOP outputs stale sample after stop, restart
* 
* Hydra_Software_Devel/67   5/6/07 6:42p bhanus
* PR 30069 : [7440 B0] Added code for :
* - Unified Sampling Rate
* - Copy path for Sequential Path
* - Enabling PCM playback on Sequential Path
* 
* Hydra_Software_Devel/66   5/6/07 3:27p kagrawal
* PR 30069: Added support for reusing an already opened/started path
* 
* Hydra_Software_Devel/65   5/3/07 5:09p gautamk
* PR26996: Exposing RAP PI interface to access toSet/Get RAMP Step size
* for mixer vol control.
* 
* Hydra_Software_Devel/64   5/3/07 4:58p speter
* PR 28716: [3563] SpdifRx, Input Spdif from Comp1->Comp2 switching
* support added. Major code revamp done in SpdifRx Module.
* 
* Hydra_Software_Devel/63   5/3/07 4:11p speter
* PR 30400: [3563] Merged the changes done as part of Phase 1.6 Release
* to the main line. Merge happened from branch RAP_3563_PH_1_6 to
* Hydra_Software_Devel
* 
* Hydra_Software_Devel/62   5/2/07 12:15p kagrawal
* PR 30069: Adding initial support for Watchdog Recovery
* 
* Hydra_Software_Devel/61   4/28/07 6:22p sushmit
* PR 28561: Fixing Start-Stop-Start of XCD
* 
* Hydra_Software_Devel/60   4/27/07 6:20p bhanus
* PR 30069 : Fixed the Rbuf size and startWr point for PCM PB channel.
* 
* Hydra_Software_Devel/59   4/26/07 7:28p bhanus
* PR 30069 : Code Modified for sequential path bring up
* 
* Hydra_Software_Devel/58   4/19/07 11:42a kagrawal
* PR 28561: Added uiPthOpenCnt and uiPthStartCnt in BRAP_P_ObjectHandles
* a.k.a. pPath
* 
* Hydra_Software_Devel/57   4/18/07 5:14p bhanus
* PR 28561 : Modified MixerSRCLayout for making it generic to be used by
* all paths.
* - Added code for sequential path implementation.
* 
* Hydra_Software_Devel/56   4/16/07 9:46a kagrawal
* PR 28561: Reduced channel handle size by changing sPath[] to *pPath[]
* 
* Hydra_Software_Devel/55   4/10/07 12:00p nitinb
* PR 29381: Removed field sSimulPtModuleHandles from channel handle.
* Modified prototype of function BRAP_P_InitRapChannelHandle to return
* BERR_Code
* 
* Hydra_Software_Devel/54   4/6/07 9:48p bhanus
* PR 28561 : [7440 B0] Enabled BRAP_P_MAI_NEEDS_MUX_SELECTOR for 7440 B0.
* 
* Hydra_Software_Devel/53   4/5/07 4:36p speter
* PR 28931: [3563] Adding DTS Passthru support in SPDIF/HDMI In
* 
* Hydra_Software_Devel/52   4/3/07 1:48p sushmit
* PR 28714: Multiple PP branch support
* 
* Hydra_Software_Devel/51   3/30/07 6:12p kagrawal
* PR 28561: [7440] Enabled BRAP_SetAudioProcessingConfig() and
* BRAP_GetAudioProcessingConfig() APIs
* - Added hAssociation in BRAP_P_DstDetails structure
* 
* Hydra_Software_Devel/50   3/30/07 11:13a nitinb
* PR 28180: [3563] Undef and redefine BDBG_ASSERT as per customer
* requirement. Controlled by macro BRAP_P_REDEFINE_BDBG_ASSERT. This is
* not a proper solution. Proper solution is to redefine BDBG_ASSERT in
* DBG module.
* 
* Hydra_Software_Devel/49   3/27/07 2:57p kagrawal
* PR 28718: Fixing compilation break for 7440 due to the last checkin
* 
* Hydra_Software_Devel/48   3/27/07 12:00p speter
* PR 28718: Spdif Rx code organisation related changes
* 
* Hydra_Software_Devel/47   3/16/07 5:07p gautamk
* PR27912 : Implementaion of Open time memory allocation for Ring Buffers
* 
* Hydra_Software_Devel/46   3/15/07 2:55a bhanus
* PR 28040 : [7440] Fixed the compilation break introduced in previous
* check-in
* 
* Hydra_Software_Devel/45   3/15/07 2:23a bhanus
* PR 28040 : [7440 B0] Enabling the New MIT for 7440 B0
* 
* Hydra_Software_Devel/44   3/14/07 3:50p kagrawal
* PR 28561: Moved channel handle malloc from BRAP_XYZ_P_OpenChannel() to
* the common BRAP_OpenChannel()
* 
* Hydra_Software_Devel/43   3/14/07 1:48p sushmit
* PR 28561: [7440] Picking up the latest RDB registers for interrupt
* 
* Hydra_Software_Devel/42   3/14/07 1:40p sushmit
* PR 28561: [7440] Picking up the latest RDB registers for interrupt
* 
* Hydra_Software_Devel/41   3/8/07 6:10p nitinb
* PR 26864: Changes to support configuration parameters for post
* processing algorithms
* 
* Hydra_Software_Devel/40   3/7/07 6:51p kagrawal
* PR 27912: [7440A0] Enabling new MIT for 7440 A0
* 
* Hydra_Software_Devel/39   3/2/07 11:24a kagrawal
* PR 27912: PI support for Speaker Management. Added following pvt
* functions
* - BRAP_P_OpenSMPath
* - BRAP_P_CloseSMPath
* - BRAP_P_StartSMPath
* - BRAP_P_StopSMPath
* - BRAP_P_GetSRFromOpDetails
* 
* Hydra_Software_Devel/38   3/1/07 10:07a kagrawal
* PR 27912: New MIT changes for 7440
* 
* Hydra_Software_Devel/37   2/27/07 9:02p kagrawal
* PR 27912: Modifications to take Audio Processing config params
* 
* Hydra_Software_Devel/36   2/27/07 6:27p nitinb
* PR 26864: Support post processing algorithms like SRS, AVL and PL-II in
* 3563 RAP PI
* 
* Hydra_Software_Devel/35   2/22/07 12:23p bhanus
* PR 28040 : Modified code for 7440 B0 porting
* 
* Hydra_Software_Devel/34   2/16/07 12:37p bhanus
* PR 27912 : Modified for  Static Downmixing Support.
* 
* Hydra_Software_Devel/33   2/8/07 11:10a bhanus
* PR 25607 : Added code for programming user provided mixing
* coefficients.
* 
* Hydra_Software_Devel/32   2/7/07 10:46a kagrawal
* PR 21368: [7440A0] Enabling software work-around for 'DP can mis-align
* bug'
* 
* Hydra_Software_Devel/31   2/5/07 1:10p bhanus
* PR 25607: Added support for cloning of output ports
* - Added code for BRAP_P_CloseCapPath() and BRAP_P_StopCapPath()
* 
* Hydra_Software_Devel/30   2/1/07 12:32p kagrawal
* PR 21368: Temporarily disabled the software workaround for DP mis-align
* bug. It is breaking compressed data flow through the mixer.
* 
* Hydra_Software_Devel/29   1/31/07 4:13p kagrawal
* PR 21368: Added software workaround for DP can mis-align bug
* 
* Hydra_Software_Devel/28   1/12/07 8:29p sushmit
* PR 26864:
* New MIT structure modfications
* PP memory calculation modifications
* Passthru context RBUF allocation fix
* Also contains Dec Config Params
* 
* Hydra_Software_Devel/27   1/12/07 10:33a kagrawal
* PR 25607: Added BRAP_P_IsDstAlreadyPresent() to check if an output port
* is already added to an association
* 
* Hydra_Software_Devel/26   1/9/07 6:26p bselva
* PR 26864: Checking in the changes for new dec config buffer param
* interface
* 
* Hydra_Software_Devel/25   1/5/07 10:48a kagrawal
* PR 25607: Distributing the destinations to each of the sPaths - added
* BRAP_P_DistributeDest()
* 
* Hydra_Software_Devel/24   12/29/06 1:09a nitinb
* PR 26766: 1. Added BTSC callback handle to RAP handle
* 2. Included file bchp_btsc_esr.h
* 3. Included file brap_int_priv.h after brap_capport_priv.h to use
*   typedefs from later file in brap_int_priv.h
* 
* Hydra_Software_Devel/23   12/28/06 1:42p speter
* PR 25346: [3563] SPDIF/HDMI In Feature - DSP source related changes
* 
* Hydra_Software_Devel/22   12/28/06 12:36p kagrawal
* PR 25607: Added Flush API
* 
* Hydra_Software_Devel/21   12/21/06 10:31a kagrawal
* PR 25607: - Fixed segment faults due to pDstDetails
* - Added support for adding and removing destinations in any sequence
* - Added support for using any valid destination after a series of calls
* to add-remove destinations
* 
* Hydra_Software_Devel/20   12/19/06 5:51p bselva
* PR 25346: Checking in the changes for 3563 new features
* 
* Hydra_Software_Devel/19   12/15/06 2:21p kagrawal
* PR 25607: Added proper support for HDMI output port
* 
* Hydra_Software_Devel/18   12/13/06 5:06p speter
* PR 25346: [3563] SPDIF/HDMI In Feature
* 
* Hydra_Software_Devel/17   12/13/06 4:39p speter
* PR 25346: [3563] SPDIF/HDMI In Feature
* 
* Hydra_Software_Devel/16   12/11/06 2:17p kagrawal
* PR 25607: Added sDstDetails to the associated channel handle. Removed
* it from hRapCh.
* 
* Hydra_Software_Devel/15   12/7/06 1:28p sushmit
* PR 25346: Merging 3563 to Hydra
* 
* Hydra_Software_Devel/RAP_3563_Merge/3   12/4/06 12:19p sushmit
* PR 24037: Adding 3563 specific preprocessor directive.
* 
* Hydra_Software_Devel/RAP_3563_Merge/2   12/4/06 11:11a sushmit
* PR 24037: Merging BTSC Code
* 
* Hydra_Software_Devel/RAP_3563_API_STUBS/1   12/3/06 10:58p nitinb
* PR 25346: Added BTSC code
* 
* Hydra_Software_Devel/RAP_3563_Merge/1   12/3/06 11:21p sushmit
* PR 24037: Merging 7440 to 3563.
* 
* Hydra_Software_Devel/14   11/27/06 12:13p sushmit
* PR 25607: Added code for CAP, XCD & ChSubTypeNone
* 
* Hydra_Software_Devel/13   11/22/06 7:20p kagrawal
* PR 25607: Added eInputAudMode and bInputLfePresent in the start param
* for decode channel
* 
* Hydra_Software_Devel/12   11/16/06 12:23p sushmit
* PR 25607: Encode/Transcode Additions.
* 
* Hydra_Software_Devel/11   11/10/06 1:16p kagrawal
* PR 25607: Added support for multi channel output on I2s_multi
* 
* Hydra_Software_Devel/10   11/9/06 6:41p bhanus
* PR 24717 : Replaced BTMR_TimerHandle with BTMR_Handle to fix
* compilation warning.
* 
* Hydra_Software_Devel/9   11/9/06 12:14p bhanus
* PR 24717 : Added Support for PCM Playback.
* 
* Hydra_Software_Devel/8   11/2/06 6:22p kagrawal
* PR 24717: Added some error handlings
* 
* Hydra_Software_Devel/7   10/10/06 6:52p kagrawal
* PR 24717:  Post bring up code clean-up
* 
* Hydra_Software_Devel/6   10/6/06 5:46p kagrawal
* PR 24717: Merged 7440 devel branch to Hydra_Software_Devel
* 
* Hydra_Software_Devel/5   9/19/06 6:05p kagrawal
* PR 20654: Added channel change support
* Added following private functions:
* - BRAP_P_StopFmmModuleHandles()
* - BRAP_P_CloseFmmModuleHandles()
* - BRAP_P_UnlinkFmmModuleHandles()
* - BRAP_P_StopDecPath()
* - BRAP_P_CloseDecPath()
* - BRAP_P_UngetChannelPaths()
* 
* Hydra_Software_Devel/4   9/18/06 3:47p kagrawal
* PR 20654: Fixed compilation warnings
* 
* Hydra_Software_Devel/3   9/7/06 4:36p kagrawal
* PR 20654: Modifications for bring up
* 
* Hydra_Software_Devel/2   8/29/06 4:32p kagrawal
* PR 20654: Removed redundant comments
* 
* Hydra_Software_Devel/1   8/17/06 12:07p speter
* PR 20654: Added from BLR_RAP_7440A0_Bring_Up_Branch
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/22   8/10/06 3:59p kagrawal
* PR 20654: Included new RDB files & fixed linking error
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/21   8/7/06 1:38p kagrawal
* PR 20654: Added support for decode passthru and simul mode
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/20   8/4/06 7:11p bhanus
* PR 20654: Added BRAP_GetGain() and BRAP_GetPan()
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/19   8/3/06 6:47p kagrawal
* PR 20654: Fixed compilation error
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/18   8/2/06 9:15p kagrawal
* PR 20654: Fixed compilation error
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/17   8/1/06 3:02p kagrawal
* PR 20654: Added linkage code for output and mixer
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/16   8/1/06 11:35a bhanus
* PR 20654: Added Interface to convert absolute value of SR to Enum
* BRAP_P_ConvertSrToEnum()
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/15   7/31/06 6:34p kagrawal
* PR 20654: Added BRAP_P_FCI_BLK_ID_XXX and some comments
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/14   7/28/06 10:51p kagrawal
* PR 20654: Added and modifiled StartFmmModules()
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/13   7/28/06 10:47a kagrawal
* PR 20654: Added code for BRAP_P_OpenFmmModuleHandles() and
* BRAP_P_LinkFmmModuleHandles()
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/12   7/28/06 10:37a bhanus
* PR 20654 : Added code for BRAP_SetGain() and BRAP_SetPan()
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/11   7/26/06 10:06p kagrawal
* PR 20654: Added declarations for private helper routines
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/10   7/19/06 5:04a kagrawal
* PR 20654: Added BRAP_DEC_P_ChannelClose()
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/9   7/19/06 4:32a kagrawal
* PR 20654: Added BRAP_DEC_P_ChannelOpen() and
* BRAP_P_InitRapChannelHandle()
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/8   7/19/06 1:12a kagrawal
* PR 20654: Included RDB files for 7440
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/7   7/17/06 1:21p bmishra
* PR 20654 : Added code for Mixer and SRC resource request for levels
* other than level 0
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/6   7/13/06 1:00p bmishra
* PR 20654 : Added code for Mixer and SRC resource request for level 0
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/5   7/11/06 8:38p bhanus
* PR 20654: Added code for SRC SRAM memory allocation and deallocation.
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/4   6/15/06 11:04a bhanus
* PR 20654: Moved BRAP_DECODE_FCI_ID and BRAP_PREPARE_FCI_ID  from brap.h
* to brap_priv.h and rename it to BRAP_P_DECODE_FCI_ID and
* BRAP_P_PREPARE_FCI_ID.
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/2   5/29/06 2:52p sushmit
* PR18604: Updating Encoder PI for 7440.
* 
* Hydra_Software_Devel/BLR_RAP_7440A0_Bring_Up_Branch/1   5/19/06 11:43a kagrawal
* PR 20654: Initial checkin (after delinking from base)
* 
***************************************************************************/

#ifndef _BRAP_PRIV_H__ /*{{{*/
#define _BRAP_PRIV_H__

#include "brap_types.h"

/* Note: All chip specific macros should be defined only after inclusion of brap_types.h. */

#if ( BCHP_CHIP == 7405 )
/* 7405 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7405
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7405 defines end */
#elif ( BCHP_CHIP == 7325 )
/* 7325 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7325
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7325 defines end */
#elif ( BCHP_CHIP == 7335 ) || (BCHP_CHIP ==7336)
/* 7335 defines start */
#if (BCHP_CHIP == 7336)
#define BRAP_PRIV_PI_CHIP_ID		  	0x7336
#else
#define BRAP_PRIV_PI_CHIP_ID		  	0x7335
#endif
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7335 defines end */
#elif ( BCHP_CHIP == 3548 )
/* 3548 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x3548
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   1
/* 3548 defines end */
#elif ( BCHP_CHIP == 3556 )
/* 3556 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x3556
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   1
/* 3556 defines end */
#elif ( BCHP_CHIP == 7420 )
/* 7420 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7420
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7420 defines end */
#elif ( BCHP_CHIP == 7340 )
/* 7340 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7340
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7340 defines ends */
#elif ( BCHP_CHIP == 7342 )
/* 7342 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7342
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7342 defines start */
#elif ( BCHP_CHIP == 7125 )
/* 7125 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7125
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7125 defines end */
#elif ( BCHP_CHIP == 7550 )
/* 7550 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7550
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7550 defines end */
#elif ( BCHP_CHIP == 7468 )
/* 7468 defines start */
#define BRAP_PRIV_PI_CHIP_ID		  	0x7468
#define BRAP_P_MAI_NEEDS_MUX_SELECTOR   0
/* 7468 defines end */
#else
#error Unsupported BCHP_CHIP
#endif

#define BRAP_P_INVALID_DRAM_ADDRESS			0xFFFFFFFF

/* This should be defined before brap_dspchn_priv.h is included */
#define BARC_Handle	BREG_Handle

/* Ramp step size of DP and SRC */
#define BRAP_P_DEFAULT_RAMP_STEP_SIZE    0xA00 

#if (BRAP_7405_FAMILY == 1)
#define BRAP_P_MAX_PP_SUPPORTED		3	/* Maximum PP supported + 1 for decoder */
#define BRAP_P_MAX_BRANCH_SUPPORTED	1	/* Number of branch supported */
#else
#define BRAP_P_MAX_PP_SUPPORTED		7	/* Maximum PP supported + 1 for decoder */
#define BRAP_P_MAX_BRANCH_SUPPORTED	3	/* Number of branch supported */
#endif
#define BRAP_P_MAX_RBUF_PER_PBCH 2

#define BRAP_OPEN_TIME_PATH_ALLOCATION	0

#define BRAP_OPEN_TIME_RBUF_ALLOCATION	1
/* Disable Metadata for other chips */
 

#define BRAP_P_MAX_IN_PARALLEL_FMM_PATH     1
#define BRAP_P_MAX_OUT_PARALLEL_FMM_PATH    2



/* Macro to apply fix for PR 30180. This fix is required for all future Raptor chips ( 3563C0
 * onwards ). This macro should be set to 1 for all the future chips that requires this fix. 
 * Currently this macro takes care of output ports available on 3563. */
#if ( (BRAP_3548_FAMILY == 1))
#define BRAP_P_RESET_IOP_SM		1
#else
#define BRAP_P_RESET_IOP_SM		0
#endif


#define BRAP_P_CUSTOM_MIXER_REQUIREMENT             0


#if (BRAP_3548_FAMILY == 1)
#define BRAP_P_EQUALIZER                            1
#else 
#define BRAP_P_EQUALIZER                            0
#endif

#if BRAP_P_CUSTOM_MIXER_REQUIREMENT
#define BRAP_P_MAX_MIXERS_CONTROLS_IN_DATA_PATH		3
#define BRAP_P_MIXER_FOR_MASTER_VOLUME              1
#else
#define BRAP_P_MAX_MIXERS_CONTROLS_IN_DATA_PATH		1
#define BRAP_P_MIXER_FOR_MASTER_VOLUME              0
#endif

/* As of now the seamless(undisturbed) condition is only for SCART output(DAC0) */
#ifdef RAP_DYNAMIC_PORT_SWITCHING
#define BRAP_DYNAMIC_PORT_SWITCHING                 1
#else
#define BRAP_DYNAMIC_PORT_SWITCHING                 0
#endif

/* Support for fixed 48khz capture */
#ifdef RAP_48KHZ_RINGBUFFER_DESTINATION
#define BRAP_48KHZ_RINGBUFFER_DESTINATION                 1
#else
#define BRAP_48KHZ_RINGBUFFER_DESTINATION                 0
#endif

#define BRAP_NUM_BUFFER_FOR_COMPRESSED 1

#define BRAP_NUM_BUFFER_FOR_BTSC 2

#define BRAP_NUM_BUFFER_FOR_DDP_PASSTHRU BRAP_NUM_BUFFER_FOR_COMPRESSED

#define BRAP_NUM_HBR_BUFFERS (BRAP_NUM_BUFFER_FOR_BTSC + BRAP_NUM_BUFFER_FOR_DDP_PASSTHRU)

#define BRAP_MAX_DEVICE_INPUT_BUFFER    16
#define BRAP_MAX_DEVICE_OUTPUT_BUFFER   16 /*5.1 + 7.1 +2.0*/



#define BRAP_P_MAX_DST_PER_RAPCH            8   /* Maximum number of 
                                                   destinations to which a RAP 
                                                   Channel can be routed to */
                                                /* TODO: correct the value */
#define BRAP_P_MAX_RAPCH_PER_DST            8  /* Maximum number of 
                                                   destinations to which a RAP 
                                                   Channel can be routed to */
                                                /* TODO: correct the value */

#define BRAP_P_MAX_ASSOCIATIONS_PER_CH      1
#define BRAP_P_LFE_CHAN_INDEX 5
#define BRAP_P_MAX_MULTI_STREAM_DECODER_OBJECTS 2
#define BRAP_MAX_SEC_CHANNEL_FOR_MS_DECODER     2


/* Max number of associated channel groups */
#define BRAP_P_MAX_AUDIO_DESC_OBJECTS           4


/*Max total supported  processing stage*/
#define BRAP_MAX_PP_SUPPORTED  ( \
		(BRAP_MAX_PP_BRANCH_SUPPORTED * BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED ) * \
		(BRAP_RM_P_MAX_DEC_CHANNELS + BRAP_RM_P_MAX_PCM_CHANNELS + BRAP_RM_P_MAX_CAP_CHANNELS ))

/*Max Destinations per processing stage*/
#define BRAP_P_MAX_DEST_PER_PROCESSING_STAGE 7

#if BRAP_P_EQUALIZER
/* Max number of equalizers */
#define BRAP_P_MAX_EQUALIZER_OBJECTS            4

/* Max number of IIR filters in a SRC */
#define BRAP_P_MAX_IIR_FILTERS_IN_SRC           8
#endif

/***************************************************************************
Summary:
    This structure holds all the inputs that are required for creating channel
    Audio processing network.
    Stages can be broadly divided in to three category.
    -- Decoder Stage
        -bDecoderStage
        -hDestHandle
        
    -- Virtual Decoder  Stage (Cloned Stage)
        -bCloneStage
        -ui32MasterBranchId
        -ui32MasterStageId
        
    -- Processing Stage
        -bInternalProcessingStage
        -hAudioProcessing
        -ui32CitBranchId
        -ui32CitStageId
For a particular stage either of three will be valid.        
***************************************************************************/ 
typedef struct BRAP_P_ChannelAudioProcessingStage
{
/*    --Valid for DecoderStage*/
    bool                        bDecoderStage;  /* True , If Decoder Stage , Otherwise False */
    BRAP_DestinationHandle          hDestHandle[BRAP_P_MAX_DEST_PER_PROCESSING_STAGE]; /* All the destination handle of Decoder stage */

/*    --Valid for Virtual DecoderStage (Cloned Stage)*/    
    bool                        bCloneStage;    /* True , If the Stage is a clone of  some other stage */
    unsigned int                    ui32MasterBranchId; /* Valid if bCloneStage = true. Branch ID of Master stage */
    unsigned int                    ui32MasterStageId; /* Valid if bCloneStage = true. Stage ID of Master stage */
/*    --Valid for Processing Stage*/    
    bool                    bInternalProcessingStage; /*True if stage is added internally */
    BRAP_ProcessingStageHandle  hAudioProcessing;   /* For Processing Stage */
    unsigned int                ui32CitBranchId;     /*BranchId of current stage in the network provided to CIT */    
    unsigned int                ui32CitStageId;     /*StageId of current stage in the network provided to CIT */        
/*    --Valid each stage  */
    BRAP_DestinationHandle      hValidDestHandle; /* This destination will represent the valid destination
                                                     to which the branch will feed in last valid stage */
    
}BRAP_P_ChannelAudioProcessingStage;

typedef struct BRAP_P_AudioProcNetwork
{
    BRAP_P_ChannelAudioProcessingStage sAudProcessingStage[BRAP_P_MAX_DST_PER_RAPCH]
                                                           [BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED]; 
}BRAP_P_AudioProcNetwork;


#include "brap_types_priv.h"
#include "brap_rm_priv.h"
#include "bkni.h" 

#include "brap.h"
#ifdef RAP_GFX_SUPPORT
#include "brap_gfx.h"
#endif
#include "brap_dspchn_priv.h"
#include "brap_fmm_priv.h"
#include "brap_rbuf_priv.h"
#include "brap_srcch_priv.h"
#include "brap_mixer_priv.h"
#include "brap_spdiffm_priv.h"
#include "brap_op_priv.h"
#include "brap_hifidac_priv.h"
#ifdef RAP_REALVIDEO_SUPPORT
#include "brap_videodecode_priv.h"
#endif
#if (BRAP_7550_FAMILY != 1)
#include "brap_src_priv.h"
#include "brap_dstch_priv.h"
#include "brap_capport_priv.h"
#endif
#include "brap_int_priv.h"
#include "brap_fwdwnld_priv.h"
#include "brap_fwproc_priv.h"
#include "brap_fwif_priv.h"

#if ( (BRAP_3548_FAMILY == 1) )
#include "brap_spdifrx_priv.h"
#include "brap_equalizer_priv.h"
#include "bchp_btsc_ctrl.h"
#include "bchp_btsc_rm.h"
#include "bchp_btsc_esr.h"
#include "bchp_spdif_rcvr_ctrl.h"
#include "bchp_spdif_rcvr_esr.h"

#endif

#include "bchp_aio_misc.h"
#include "bchp_aio_inth.h"
#include "bchp_aio_intd0.h"

#include "bchp_common.h"
#include "bchp_hifidac_ctrl0.h"
#include "bchp_hifidac_esr0.h"
#include "bchp_hifidac_rm0.h"
#include "bchp_int_id_aud_dsp_inth0.h"
#include "bchp_int_id_aio_inth.h"

#include "bchp_aud_dsp_cfg0.h"

#include "bchp_aud_dsp_ctrl0.h"

#include "bchp_aud_dsp_esr_be0.h"

#include "bchp_aud_dsp_esr_pi0.h"

#include "bchp_aud_dsp_esr_si00.h"

#include "bchp_aud_dsp_esr_si10.h"

#include "bchp_aud_dsp_esr_si20.h"

#include "bchp_aud_dsp_esr_si30.h"

#include "bchp_aud_dsp_esr_si40.h"

#include "bchp_aud_dsp_esr_si50.h"
#include "bchp_aud_dsp_esr_so00.h"


#include "bchp_aud_dsp_esr_so10.h"

#include "bchp_aud_dsp_esr_so20.h"

#include "bchp_aud_dsp_esr_so30.h"

#include "bchp_aud_dsp_esr_so40.h"

#include "bchp_aud_dsp_esr_so50.h"

#include "bchp_aud_dsp_inth0.h"

#include "bchp_aud_dsp_misc.h"

#include "bchp_aud_dsp_rgr.h"

#include "bchp_aud_fmm_iop_ctrl.h"
#include "bchp_aud_fmm_iop_esr.h"
#include "bchp_aud_fmm_misc.h"
#include "bchp_aud_fmm_ms_ctrl.h"
#include "bchp_aud_fmm_ms_esr.h"
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_op_esr.h"

# if (BRAP_7550_FAMILY == 1)
#include "bchp_aud_fmm_op_spdif_nco.h"
#else
#include "bchp_aud_fmm_src_ctrl0.h"
#include "bchp_aud_fmm_src_esr0.h"
#endif

# if (BRAP_3548_FAMILY != 1)&&(BRAP_7550_FAMILY != 1)
#include "bchp_aud_fmm_pll0.h"
#endif

#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_aud_fmm_bf_esr0_d0.h"
#include "bchp_aud_fmm_bf_esr0_h.h"
#include "bchp_aud_fmm_bf_esr1_d0.h"
#include "bchp_aud_fmm_bf_esr1_h.h"
#include "bchp_aud_fmm_bf_esr2_d0.h"
#include "bchp_aud_fmm_bf_esr2_h.h"

#include "bchp_aud_fmm_dp_ctrl0.h"
#include "bchp_aud_fmm_dp_esr00.h"
#include "bchp_aud_fmm_dp_esr20.h"



#if (BRAP_3548_FAMILY == 1)
#include "bchp_vcxo_ctl_misc.h"
#include "bchp_aud_fmm_src_ctrl1.h"
#include "bchp_aud_fmm_src_esr1.h"
#include "bchp_hifidac_ctrl1.h"
#include "bchp_hifidac_ctrl2.h"
#include "bchp_hifidac_esr1.h"
#include "bchp_hifidac_esr2.h"
#include "bchp_hifidac_rm1.h"
#include "bchp_hifidac_rm2.h"
#include "bchp_adccic_ctrl.h"
#include "bchp_adccic_esr.h"
#endif

#if (BCHP_CHIP == 7420)
#include "bchp_hifidac_ctrl1.h"
#include "bchp_hifidac_esr1.h"
#include "bchp_hifidac_rm1.h"
#endif

#if (BRAP_7405_FAMILY == 1)
#if (BRAP_PLL1_SUPPORTED == 1)   
#include "bchp_aud_fmm_pll1.h"
#endif
#endif



#define BRAP_PRIV_PI_VERSION_MAJOR			0x1	
#define BRAP_PRIV_PI_VERSION_MINOR			0x0
#define BRAP_PRIV_PI_VERSION_DATE			0x0000419
#define BRAP_PRIV_PI_VERSION_YEAR			0x2006
#define BRAP_PRIV_PI_VERSION_CUSTOMER_ID	"REF"


#define BRAP_P_USE_BARC         0
#define BRAP_P_USE_BRAP_TRANS   0

#if (BRAP_7405_FAMILY == 1)
#define BRAP_P_HAS_META_BUFFER		0
#define BRAP_P_HAS_SPDIF_RECIEVER		0
#else
#define BRAP_P_HAS_META_BUFFER		0
#define BRAP_P_HAS_SPDIF_RECIEVER		1
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


/*{{{ Defines */

#define BRAP_P_ALIGN(x,alignbits)   (((x) + ((1 << (alignbits)) - 1)) & ~((1 << (alignbits)) - 1))
#define BRAP_P_I2S_MAX_BITS_PER_SAMPLE      32	/* Maximum bits per sample 
											       supported by I2S output */
#define BRAP_P_MIXER_MAX_BITS_PER_SAMPLE    24	/* Maximum bits per sample 
											       supported by Mixer */
#define BRAP_P_MIN_BITS_PER_SAMPLE          16	/* Minimum bits per sample 
											       supported by Raptor */
#define BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA  16	/* bits per sample for 
											               compressed data 
											               supported by Rap */											   


#define BRAP_PB_P_RBUF_DEFAULT_FREE_BYTE_MARK 	75
#define BRAP_PB_P_RBUF_DEFAULT_SIZE             (2048*3*4)

#define BRAP_DEC_P_RBUF_DEFAULT_SIZE            (8192*3*4)

/* The default size for Capture RBUF can be less */
/* As a work around for HidefDVD PR6731, stop Source FIFO getting data from 
   capture buffers (in DelayBalance path)before any other FMM resource. After 
   Stopping the SRC Fifo, put a delay of 20 mSec so that the DST fifo internal
   buffers are full and hence the mis-aligning of the DST Fifo pointers can
   be avoided. The size of Capture RBUF is made smaller so that 20 mSec is 
   good enough to fill the buffer. Hence changing the size to 8K */
#define BRAP_CAP_P_RBUF_DEFAULT_SIZE            (4096*2)
#define BRAP_P_WATERMARK_WORKAROUND 0
#define BRAP_P_EDGE_TRIG_INTRPT     1
#define BRAP_P_MAX_DOWN_MIX_COEFF   3
#define BRAP_P_UNITY_5_23_FORMAT (1<<23)

/***************************************************************************
Summary:
    RAP FCI ID preparation and decode defines.
****************************************************************************/
#define BRAP_P_FCI_BLK_ID_MASK          0x3C0
#define BRAP_P_FCI_BLK_ID_SHIFT         6
#define BRAP_P_FCI_RSRC_ID_MASK         0x3f
#define BRAP_P_DECODE_FCI_ID(FCI_ID, BLK_ID, RSRC_ID) \
    { \
      (RSRC_ID) = ((FCI_ID) & BRAP_P_FCI_RSRC_ID_MASK); \
      (BLK_ID) = (((FCI_ID) & BRAP_P_FCI_BLK_ID_MASK) >> BRAP_P_FCI_BLK_ID_SHIFT); \
    }
#define BRAP_P_PREPARE_FCI_ID(FCI_ID, BLK_ID, RSRC_ID) \
    { \
      (FCI_ID) = ((BLK_ID) << BRAP_P_FCI_BLK_ID_SHIFT) | RSRC_ID; \
    }

/***************************************************************************
Summary:
    This structure holds a FMM DATA Path's property.
***************************************************************************/ 
typedef struct BRAP_P_FmmPathProp
{
    unsigned int            uiPathIdx;  /* Input Or Output Path Index */
}BRAP_P_FmmPathProp;

/***************************************************************************
Summary:
    Marco to check if a Pointer is valid
****************************************************************************/
#define BRAP_P_IsPointerValid(ptr) \
    (!( (ptr == NULL)||\
        (ptr == (void *)0xDA7ADEAD)||\
        (ptr == (void *)0xDEADDA7A)||\
        (ptr == (void *)0xFFFFFFFF)||\
        (ptr == (void *)0) ))

/***************************************************************************
Summary:
    Usage path get/set macros
****************************************************************************/

#define BRAP_P_SET_USG_PATH(pPth,ePth,uiInPthIdx,uiSelfPthIdx,uiOutPthIdx,uiFmmPp) \
    { \
        (pPth)->eUsgPath = (ePth); \
        (pPth)->sInPathProp[0].uiPathIdx = (uiInPthIdx); \
        (pPth)->uiSelfPathArrIdx = (uiSelfPthIdx); \
        (pPth)->sOutPathProp[(uiFmmPp)].uiPathIdx = (uiOutPthIdx); \
    }

#define BRAP_P_GET_USG_PATH(pPth,ePth,uiInPthIdx,uiSelfPthIdx,uiOutPthIdx,uiFmmPp) \
    { \
        (ePth) = (pPth)->eUsgPath; \
        (uiInPthIdx) = (pPth)->sInPathProp[0].uiPathIdx; \
        (uiSelfPthIdx) = (pPth)->uiSelfPathArrIdx; \
        (uiOutPthIdx) = (pPth)->sOutPathProp[(uiFmmPp)].uiPathIdx; \
    }

/***************************************************************************
Summary:
    Link info set macro
****************************************************************************/
#define BRAP_P_SET_LINK_INFO(sLink, eRsrc, uiLvl, uiChPair, uiPp,uiOutput) \
    { \
        (sLink).eRsrcType = (eRsrc); \
        (sLink).uiLevel = (uiLvl); \
        (sLink).eChnPair = (uiChPair); \
        (sLink).uiPrlPth = (uiPp); \
        (sLink).uiMixerOutput = (uiOutput); \
    }
        
/***************************************************************************
Summary:
    Marco for network validity
****************************************************************************/
#define BRAP_P_NETWORK_VALID(sAudProcessingStage) \
       ((true == sAudProcessingStage[0][0].bCloneStage)||\
        (true == sAudProcessingStage[0][0].bDecoderStage)||\
        (NULL != sAudProcessingStage[0][0].hAudioProcessing))

/***************************************************************************
Summary:
    Marco for branch validity
****************************************************************************/
#define BRAP_P_BRANCH_VALID(sAudProcessingStage, uiBranchId) \
        ((true == sAudProcessingStage[uiBranchId][0].bCloneStage)||\
        (true == sAudProcessingStage[uiBranchId][0].bDecoderStage)||\
        (NULL != sAudProcessingStage[uiBranchId][0].hAudioProcessing))

/***************************************************************************
Summary:
    Marco for Stage validity
****************************************************************************/
#define BRAP_P_STAGE_VALID(sAudProcessingStage, uiBranchId, uiStageId) \
        ((true == sAudProcessingStage[uiBranchId][uiStageId].bCloneStage)||\
        (true == sAudProcessingStage[uiBranchId][uiStageId].bDecoderStage)||\
        (NULL != sAudProcessingStage[uiBranchId][uiStageId].hAudioProcessing))
                    
/***************************************************************************
Summary:
    Macro to check if the channel is Primary/Master of the Multistream Decoder
****************************************************************************/
#define BRAP_P_PRIMARY_CHANNEL(hRapCh, uiAssocId) \
        ((BRAP_P_IsPointerValid(hRapCh->hRap->sAssociatedCh[uiAssocId].hMultiStreamDecoder)) &&\
        (BRAP_P_IsPointerValid(hRapCh->hRap->sAssociatedCh[uiAssocId].hMultiStreamDecoder->\
                                sExtMultiStreamDecoderDetails.hPrimaryChannel)) &&\
        (hRapCh == hRapCh->hRap->sAssociatedCh[uiAssocId].hMultiStreamDecoder->\
                    sExtMultiStreamDecoderDetails.hPrimaryChannel))

/***************************************************************************
Summary:
    Macro to check if the channel is Secondary/Slave of the Multistream Decoder
    whose Output(s) may need Sample Rate Conversion
****************************************************************************/
#define BRAP_P_SECONDARY_CHANNEL_SRC(hRapCh, uiAssocId) \
        ((BRAP_P_IsPointerValid(hRapCh->hRap->sAssociatedCh[uiAssocId].hMultiStreamDecoder)) &&\
        (BRAP_P_IsPointerValid(hRapCh->hRap->sAssociatedCh[uiAssocId].hMultiStreamDecoder->\
                                sExtMultiStreamDecoderDetails.hPrimaryChannel)) &&\
        (BRAP_P_State_eStarted == hRapCh->hRap->sAssociatedCh[uiAssocId].hMultiStreamDecoder->\
                    sExtMultiStreamDecoderDetails.hPrimaryChannel->eState) &&\
        (hRapCh != hRapCh->hRap->sAssociatedCh[uiAssocId].hMultiStreamDecoder->\
                    sExtMultiStreamDecoderDetails.hPrimaryChannel))

/***************************************************************************
Summary:
    SRC SRAM memory defines. These macros define the SRAM Memory present for 
    every SRC Block and Memory requirements for each SRC operation.
****************************************************************************/
#if ((BRAP_7405_FAMILY == 1)||(BRAP_3548_FAMILY == 1))
#define BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0         960
#define BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1         1600
#else
#define BRAP_P_MAX_SRAM_MEMORY_SRCBLK_0         1088
#define BRAP_P_MAX_SRAM_MEMORY_SRCBLK_1         1984
#endif
#define BRAP_P_SRAM_MEMORY_REQUIRED_DOWN4       512
#define BRAP_P_SRAM_MEMORY_REQUIRED_DOWN2       256
#define BRAP_P_SRAM_MEMORY_REQUIRED_UPx         128
#define BRAP_P_SRAM_MEMORY_REQUIRED_LIN_INT     4

/***************************************************************************
Summary:
    FCI ID as per section "12.2	FMM Common Internal interface" from "7440A0 
    FMM Architecture Design (FMM_7440_arch.doc) Revision 1".    
    ID[5:0] is used for clients inside a block such as BF, SRC, DP or IOP. 
    ID[9:6] is to identify blocks in FMM
    0x0: BF playback 
    0x1: BF capture
    0x2: SRC 0
    0x3: SRC 1
    0x4: DP 0
    0x5: DP 1
    0x6: IOP capture
    0x7-0xF: not defined
****************************************************************************/
#define BRAP_P_FCI_BLK_ID_BF_PLAYBACK       0x0
#define BRAP_P_FCI_BLK_ID_BF_CAPTURE        0x1
#define BRAP_P_FCI_BLK_ID_SRC0              0x2
#define BRAP_P_FCI_BLK_ID_SRC1              0x3
#define BRAP_P_FCI_BLK_ID_DP0               0x4
#define BRAP_P_FCI_BLK_ID_DP1               0x5
#define BRAP_P_FCI_BLK_ID_IOP_CAPTURE       0x6
    
/***************************************************************************
Summary:
    Define to prepare the the coefficients for gain control
****************************************************************************/
#define BRAP_P_PREPARE_GAIN_COEFFICIENT(GAIN_CO,GAIN_MSB,GAIN_LSB)          \
                        {                                                   \
                            (GAIN_CO) = (((GAIN_MSB)<<23) & 0xF800000)||    \
                                         ((GAIN_LSB) & 0x7FFFFF);           \
                        }

#define BRAP_P_DECODE_GAIN_COEFFICIENT(GAIN_CO,GAIN_MSB,GAIN_LSB)           \
                        {                                                   \
                            (GAIN_MSB)=((GAIN_CO)>>23)&0x1F;                \
                            (GAIN_LSB)=(GAIN_CO) & 0x7FFFFF;                \
                        }
/*}}}*/

/*{{{ Typedefs */

/***************************************************************************
Summary:
    These marcos will create, reset, destroy, set, wait.
***************************************************************************/ 

#define BRAP_P_EventCreate(hDsp,uiTaskId)                        \
            BKNI_CreateEvent(&(hDsp->hEvent[uiTaskId]))

#define BRAP_P_EventDestroy(hDsp,uiTaskId)                       \
            BKNI_DestroyEvent(hDsp->hEvent[uiTaskId])

#define BRAP_P_EventWait(hDsp, uiTime,uiTaskId)                     \
    BKNI_WaitForEvent((hDsp)->hEvent[uiTaskId], uiTime);    

#define BRAP_P_EventSet(hDsp,uiTaskId)                           \
            BKNI_SetEvent((hDsp)->hEvent[uiTaskId])    

#define BRAP_P_EventReset(hDsp,uiTaskId)            \
            BKNI_ResetEvent((hDsp)->hEvent[uiTaskId])

            
/***************************************************************************
Summary:
    This structure holds Internal destination property.
***************************************************************************/ 
typedef struct BRAP_P_InternalDst
{
    BRAP_CapInputPort       eCapPort[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
    BAVC_AudioSamplingRate  eInputSR;       /* Input SR of these Cap Ports */
    BRAP_OutputMode         eInputAudMode;  /* Input Audio of these capports */
    bool                    bLfeOn;         /* LFE present or not */

}BRAP_P_InternalDst;

/***************************************************************************
Summary:
    This structure holds a mixer-SRC level's property.
***************************************************************************/ 
typedef struct BRAP_P_MixingLevelProp
{
    BAVC_AudioSamplingRate  eInputSR;       /* Input SR to this level */ 
    BAVC_AudioSamplingRate  eOutputSR;      /* Output SR from level */ 
    BRAP_OutputMode         eInputAudMode;  /* Input Audio mode to level */ 
    bool                    bInputLfeOn;    /* Input LFE On status to level */ 
    BRAP_OutputMode         eOutputAudMode; /* Output Audio mode from level */ 
    bool                    bOutputLfeOn;   /* Output LFE On status from level*/
    
    BRAP_DstDetails         *pDstDetails[BRAP_RM_P_MAX_PARALLEL_PATHS];
                                            /* Pointer to dst details. This 
                                               is valid only if a particular
                                               Mixing Level feeds to a 
                                               destination directly. It is NULL
                                               if a mixing level doesn't feed to
                                               some destination directly */
                                             
    BRAP_P_InternalDst      *pIntDstDetails[BRAP_RM_P_MAX_MIXER_OUTPUTS];
                                            /* Pointer to internal destination as
                                               Internal capture port */
}BRAP_P_MixingLevelProp;

/***************************************************************************
Summary:
    Structure for storing the property of output audio mode 
****************************************************************************/
typedef struct BRAP_P_OpAudModProp
{
    unsigned int uiNoChannels;	        /* Number of output possible channels */
    uint32_t      ui8ChanBits;           /* Each bit corresponding to one channel
                                           |L|R|Ls|Rs|C|Lfe|Lb|Rb| 
                                           |7|6| 5| 4|3| 2 | 1| 0| */    
    bool bChnExists[BRAP_RM_P_MAX_OP_CHANNELS];
                                        /* Element of the array correspond to
                                           audio output channels defined by enum
                                           BRAP_OutputChannel. Each element of 
                                           this array specifies the following.
        					   TRUE: If the corresponding audio 
        							 output channel exists, 
        					   FALSE: otherwise */
}BRAP_P_OpAudModProp;


/***************************************************************************
Summary:
    Registers that need to be saved and restored while enabling/disabling test
    tone 
****************************************************************************/    
/* TODO Add all registers required for 7440 */
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


/***************************************************************************
Summary:
    This enumeration lists the various resources that can be used in a 
    channel.
***************************************************************************/ 
typedef enum BRAP_P_Rsrc
{   
    BRAP_P_Rsrc_eRBuf,
    BRAP_P_Rsrc_eSrcCh,
    BRAP_P_Rsrc_eDstCh,
    BRAP_P_Rsrc_eSrc,
    BRAP_P_Rsrc_eMixer,
    BRAP_P_Rsrc_eSpdifFm,
    BRAP_P_Rsrc_eSrcEq,
    BRAP_P_Rsrc_eOpPort,
    BRAP_P_Rsrc_eCapPort,
    BRAP_P_Rsrc_eMax
}BRAP_P_Rsrc;

/***************************************************************************
Summary:
    This structure holds the linkage info required to identify a resource 
    in BRAP_P_ObjectHandles. 
***************************************************************************/ 
typedef struct BRAP_P_LinkInfo
{
    BRAP_P_Rsrc             eRsrcType;  /* Resource type */
    unsigned int            uiLevel;    /* Level to which this resource belongs */
    BRAP_OutputChannelPair  eChnPair;   /* Channel pair */
    unsigned int            uiPrlPth;   /* Parallel path */
    unsigned int            uiMixerOutput;
}BRAP_P_LinkInfo;

/***************************************************************************
Summary:
    This structure holds the RBuf handle along with its linkage ids. 
***************************************************************************/ 
typedef struct BRAP_P_RBufHandleInfo
{
    bool                    bValid;         /* TRUE: valid, FALSE: invalid */
    BRAP_P_LinkInfo         sRbufInLink;    /* Output resource link info */
    BRAP_P_LinkInfo         sRbufSelfLink;  /* Self link info */
    BRAP_RBUF_P_Handle      hRbuf;          /* Handles */
    BRAP_P_LinkInfo         sRbufOutLink;   /* Output resource link info */
}BRAP_P_RBufHandleInfo;

/***************************************************************************
Summary:
    This structure holds the SrcCh handle along with its linkage ids. 
***************************************************************************/ 
typedef struct BRAP_P_SrcChHandleInfo
{
    bool                    bValid;         /* TRUE: valid, FALSE: invalid */
    BRAP_P_LinkInfo         sSrcChInLink;   /* Output resource link info */
    BRAP_P_LinkInfo         sSrcChSelfLink;   /* Self link info */
    BRAP_SRCCH_P_Handle     hSrcCh;         /* Handles */
    BRAP_P_LinkInfo         sSrcChOutLink;  /* Output resource link info */
}BRAP_P_SrcChHandleInfo;

/***************************************************************************
Summary:
    This structure holds the DstCh handle along with its linkage ids. 
***************************************************************************/ 
typedef struct BRAP_P_DstChHandleInfo
{
    bool                    bValid;         /* TRUE: valid, FALSE: invalid */
    BRAP_P_LinkInfo         sDstChInLink;   /* Output resource link info */
    BRAP_P_LinkInfo         sDstChSelfLink; /* Self link info */
    BRAP_DSTCH_P_Handle     hDstCh;         /* Handles */
    BRAP_P_LinkInfo         sDstChOutLink;  /* Output resource link info */
}BRAP_P_DstChHandleInfo;

/***************************************************************************
Summary:
    This structure holds the SRC handles along with its linkage ids. 
***************************************************************************/ 
typedef struct BRAP_P_SrcHandleInfo
{
    bool                    bValid;         /* TRUE: valid else invalid */
    BRAP_P_LinkInfo         sSrcInLink;     /* Input resource link info */
    BRAP_P_LinkInfo         sSrcSelfLink;   /* Self link info */
    BRAP_SRC_P_Handle       hSrc[BRAP_RM_P_MAX_SRC_IN_CASCADE]; 
                                            /* SRC Handles.
                                               Note: Max SRC ratio achievable
                                               from a single SRC can be 1:4 
                                               or 4:1. As per the usage cases,
                                               at max 12 to 196 or 196 to 12kHz 
                                               conversion will require 2 SRC's 
                                               in cascade */
    BRAP_P_LinkInfo         sSrcOutLink[BRAP_RM_P_MAX_RSRCS_CONNECTED_TO_SAME_SRC];
                                            /* Output resource link info */
}BRAP_P_SrcHandleInfo;  

/***************************************************************************
Summary:
    This structure holds the SRC-EQ handles along with its linkage ids. 
***************************************************************************/ 
typedef struct BRAP_P_SrcEqHandleInfo
{
    bool                    bValid;         /* TRUE: valid else invalid */
    BRAP_P_LinkInfo         sSrcEqInLink;     /* Input resource link info */
    BRAP_P_LinkInfo         sSrcEqSelfLink;   /* Self link info */
    BRAP_SRC_P_Handle       hSrcEq[BRAP_RM_P_MAX_SRC_IN_CASCADE]; 
                                            /* SRC Handles. */
    BRAP_P_LinkInfo         sSrcEqOutLink[BRAP_RM_P_MAX_RSRCS_CONNECTED_TO_SAME_SRC];
                                            /* Output resource link info */
}BRAP_P_SrcEqHandleInfo; 

/***************************************************************************
Summary:
    This structure holds the MIXER handles along with its linkage ids. 
***************************************************************************/ 
typedef struct BRAP_P_MixerHandleInfo
{
    bool                    bValid;         /* TRUE: valid else invalid */
    BRAP_P_LinkInfo         sMixerInLink[BRAP_RM_P_MAX_MIXER_INPUTS];
                                            /* Input resource link info */
    BRAP_P_LinkInfo         sMixerSelfLink; /* Self link info */
    BRAP_MIXER_P_Handle     hMixer;         /* handles */
    BRAP_P_LinkInfo         sMixerOutLink[BRAP_RM_P_MAX_MIXER_OUTPUTS];
                                            /* Output resource link info */
}BRAP_P_MixerHandleInfo;

/***************************************************************************
Summary:
    This structure holds the OP and SpdifFm handles along with their linkage 
    ids. 
***************************************************************************/ 
typedef struct BRAP_P_OpHandleInfo
{
    bool                    bValid;         /* TRUE: valid else invalid */
    BRAP_P_LinkInfo         sOpInLink;      /* Input resource link info */
    BRAP_P_LinkInfo         sOpSelfLink;    /* Self link info */
    BRAP_SPDIFFM_P_Handle   hSpdifFm;       /* handles */
    BRAP_OP_P_Handle        hOp;            /* handles */
    BRAP_OP_P_Handle        hMaiOp;         /* Special handle for MAI output 
                                               used only when MAI is tapping
                                               data from the associated hOp */
}BRAP_P_OpHandleInfo; 

/***************************************************************************
Summary:
    This structure holds the Capure port handles along with their linkage 
    ids. 
***************************************************************************/ 
typedef struct BRAP_P_CapPortHandleInfo
{
    bool                    bValid;             /* TRUE: valid else invalid */
    BRAP_P_LinkInfo         sCapPortInLink;     /* Input resource link info */
    BRAP_P_LinkInfo         sCapPortSelfLink;   /* Self link info */
    BRAP_CAPPORT_P_Handle   hCapPort;           /* handles */
    BRAP_P_LinkInfo         sCapPortoutLink;    /* Out link info */
}BRAP_P_CapPortHandleInfo; 

/***************************************************************************
Summary:
    This structure holds the various resource handles and their linkage info.
***************************************************************************/ 
typedef struct BRAP_P_ObjectHandles
{
    /* Note the linkage information here should be available 
       during watchdog recovery operations */

    /* Path Information */
    BRAP_P_UsgPath          eUsgPath;   /* Usage path type for which below
                                           resources are used */
    BRAP_P_FmmPathProp      sInPathProp[BRAP_P_MAX_IN_PARALLEL_FMM_PATH];
                                        /* Properties of the previous (incoming)
                                           path. This has path index that can be
                                           used on hRapCh->pPath[idx] to access 
                                           previous path */                                        
    unsigned int            uiSelfPathArrIdx;
                                        /* Index in pPath[] to which this
                                           structure is saved */
    BRAP_P_FmmPathProp      sOutPathProp[BRAP_P_MAX_OUT_PARALLEL_FMM_PATH];
                                        /* Properties of the next (outgoing)
                                           path. This has path index that can be
                                           used on hRapCh->pPath[idx] to access 
                                           the next path */
    unsigned int            uiPthOpenCnt;
                                        /* This is incremented everytime a path 
                                           is opened and decremented when closed.
                                           0 means not opened. */
    unsigned int            uiAssocId;
                                        /* This will be valid only capture path or 
                                        the path valid on associations*/
                                           
    unsigned int            uiPthStartCnt;
                                        /* This is incremented everytime a path 
                                           is started and decremented when 
                                           stopped. 0 means not started. */
    
    /* Resource grant structure for this path */
	BRAP_RM_P_ResrcGrant    sRsrcGrnt;	/* Resource granted by the resource 
                                           manager */

    /* Resource handles and their linkage information */                                           
	BRAP_FMM_P_Handle	    hFmm;	/* FMM Device Handle */       
	BRAP_DSP_Handle		    hDsp;	/* DSP Device Handle */
	BRAP_DSPCHN_Handle	    hDspCh;	/* DSP Context Handle */
	BRAP_RBUF_P_Handle	    hRBuf[BRAP_RM_P_MAX_OP_CHANNELS];	
                                    /* Ring buffer handles for different 
                                       output channels. Refer to enum 
                                       BRAP_OutputChannelIndex for each 
                                       output channel definition. Invalid 
                                       entries will be marked by NULL. */
	BRAP_P_SrcChHandleInfo  sSrcCh[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]; 
                                    /* Source Channel handles for different 
                                       output channel pairs. Refer to enum 
                                       BRAP_OutputChannelPairIndex 
									   for each output channel pair 
									   definition. Invalid entries will be 
									   marked by NULL. */
	BRAP_P_DstChHandleInfo  sDstCh[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]; 
                                    /* Destination Channel handles for different 
                                       output channel pairs. Refer to enum 
                                       BRAP_OutputChannelPairIndex 
									   for each output channel pair 
									   definition. Invalid entries will be 
									   marked by NULL. */
	BRAP_P_SrcHandleInfo    sSrc[BRAP_RM_P_MAX_MIXING_LEVELS]
                                [BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                    /* Set of SRCs classified based on the 
                                       level, channel pair and parallel path */
    BRAP_P_MixerHandleInfo  sMixer[BRAP_RM_P_MAX_MIXING_LEVELS]
                                  [BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                  [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                    /* Set of mixers classified based on the 
                                       level, channel pair and parallel path */
    BRAP_P_SrcEqHandleInfo  sSrcEq[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                   [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                    /* Set of SpdifFm and Output Ports 
                                       classified based on the channel 
                                       pairs and parallel paths */
    BRAP_P_OpHandleInfo     sOp[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                               [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                    /* Set of SpdifFm and Output Ports 
                                       classified based on the channel 
                                       pairs and parallel paths */
    BRAP_P_CapPortHandleInfo sCapPort[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                     [BRAP_RM_P_MAX_PARALLEL_PATHS];
                                    /* Capture Port info */
    BRAP_P_MixingLevelProp  sMixingLevels[BRAP_RM_P_MAX_MIXING_LEVELS];
                                    /* Mixing level (SRC + Mixer) properties */
    unsigned int                uiDestcount;                                    
    BRAP_DstDetails         *pDstDetails[BRAP_P_MAX_DST_PER_RAPCH]; 
                                    /* Pointer to DstDetails fed by this path */
    bool                    bMixPath;                                    
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)                                     
    unsigned int        uiMasterpathId; /* If Independent Delay is on, then it tells the path containing Master Rbuf */
#endif    
}BRAP_P_ObjectHandles;

/***************************************************************************
Summary:
    This structure holds the trick mode state information.
***************************************************************************/ 
typedef struct BRAP_P_TrickModeState
{
	unsigned int    uiFrameAdvResidualTime;	/* Residual time of frame advance 
											   trick mode  in TIME_45KHZ_TICKS*/
	bool			bAudioPaused;			/* Current status of audio decoder
											   wrt pause trick mode */
} BRAP_P_TrickModeState;


#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)

/***************************************************************************
Summary:
	This structure holds RBuf info.
***************************************************************************/  
typedef struct BRAP_P_RBufMem {
    void *      pRBufPtr;   /* Valid if some memory is allocated, else NULL */
    size_t      uiSize;         /* Size of the DRAM if pLeftRBufBase and/or 
                                   pRightRBufBase is not NULL */
    bool        bUsed;          /* false indicates this memory is not yet used;
                                   true other wise */
}BRAP_P_RBufMem;


/***************************************************************************
Summary:
	This structure holds the information of various device Specific Rbufs.
***************************************************************************/  
typedef struct BRAP_P_DeviceRBufPool 
{
    BRAP_DeviceRBufPool         *pExtRBufPoolSettings;
    BRAP_P_RBufMem              sInputBuffer[BRAP_MAX_DEVICE_INPUT_BUFFER];
    BRAP_P_RBufMem              sOutputBuffer[BRAP_MAX_DEVICE_OUTPUT_BUFFER];    
    BRAP_P_RBufMem              sDestRBuf[BRAP_OutputChannelPair_eMax*2];
    BRAP_P_RBufMem              sHbrRBuf[BRAP_NUM_HBR_BUFFERS];
}BRAP_P_DeviceRBufPool;

/***************************************************************************
Summary:
	This structure holds the information of various channel Specific Rbufs.
***************************************************************************/ 
typedef struct BRAP_P_ChannelRBufPool 
{
    BRAP_ChannelRBufPool         sExtRBufPoolSettings;

	/* For a decode channel */
    BRAP_P_RBufMem              sDecOutRBuf[BRAP_MAX_FW_PROC_BRANCHES]
                                        [BRAP_MAX_OUT_CH_PER_FW_PROC_BRANCH];

    BRAP_P_RBufMem              sDecInRBuf[BRAP_MAX_DEC_CHAN_WITH_CAPPORT_SOURCE]
                                        [BRAP_MAX_IN_CH_PER_FW_PROC_BRANCH];

    /* For a playback channel */
    BRAP_P_RBufMem              sPbRBuf[BRAP_P_MAX_RBUF_PER_PBCH];
}BRAP_P_ChannelRBufPool;

/***************************************************************************
Summary:
	This structure holds the gain coefficients per channel per output port.
***************************************************************************/ 
typedef struct BRAP_P_ScalingInfo 
{
    BRAP_OutputPort eOp[BRAP_RM_P_MAX_MIXER_PER_DP_BLCK * BRAP_RM_P_MAX_DP_BLCK]
                                    [BRAP_P_MAX_DST_PER_RAPCH];
    BRAP_GainControl sScalingInfo[BRAP_RM_P_MAX_MIXER_PER_DP_BLCK * BRAP_RM_P_MAX_DP_BLCK]
                                    [BRAP_P_MAX_DST_PER_RAPCH];
}BRAP_P_ScalingInfo;
#endif
#ifdef 	RAP_GFX_SUPPORT
typedef struct BRAP_GFX_P_Settings
{
    BRAP_GFX_OpenSettings   sOpenGfxSettings;
    BRAP_GFX_Settings   sStartGfxSettings;    
}BRAP_GFX_P_Settings;
#endif

/***************************************************************************
Summary:
    This structure holds all the resources that comprise an audio channel.
***************************************************************************/ 
typedef struct BRAP_P_Channel
{
	BCHP_Handle		        hChip;	    /* Handle to chip object */	
	BREG_Handle		        hRegister;	/* Handle to register object */
	BMEM_Handle		        hHeap;      /* Handle to memory object */
	BINT_Handle		        hInt;	    /* Handle to interrupt object */
	BRAP_Handle		        hRap;		/* Parent Audio Device handle */
    BTMR_TimerHandle        hTimer;     /* Handle to timer object */
#ifdef AUTOTRAP
    BTMR_TimerHandle        hTimer1;    /* Handle to timer object for capture */
#endif /* AUTOTRAP */

	unsigned int	        uiXptChannelNo;	
                                        /* Transport channel number */
	BAVC_XptContextMap	        sXptContextMap;	 
                                        /* This structure holds the CDB and 
										   ITB register addresses for the 
										   corresponding Rave Context 
										   allocated by the Xpt PI.*/
	BRAP_ChannelType        eChannelType;
                                        /* Channel type: Decode, PCMPlayback, 
									       PCMCapture */
	unsigned int	        uiChannelNo;	
                                        /* Channel id of the the type of 
									       channel specified by eChannelType */
    BRAP_ChannelSubType     eChannelSubType;   
                                        /* Channel subtupe like Primary, 
                                           Secondary or Neither Primary nor 
                                           Secondary */
    BRAP_AudioSource        eAudioSource;   
                                        /* Source of audio data to the Raptor 
                                           channel */

	BRAP_CapInputPort	    eCapInputPort;	
            	                        /* Type of Capture Input Source.This 
            	                           field is considered if channel type
            	                           is Capture or if the audio source in the 
            	                           channel settings is external capture port */

#ifdef BRAP_OPEN_TIME_PATH_ALLOCATION                                           
    BRAP_P_ObjectHandles 	*pMemPath[BRAP_P_MAX_PATHS_IN_A_CHAN];
                                        /* Internal module handles used to allocate \
                                        memory at open channel and used in start channel*/
#endif
                                        
    BRAP_P_ObjectHandles 	*pPath[BRAP_P_MAX_PATHS_IN_A_CHAN];
                                        /* Internal module handles used and 
                                           their linkage info*/
    /* TODO: simul mode can be removed if separate channels for decode and 
       passthru is decided later */
	bool			        bSimulModeConfig;	
                                        /* TRUE: Simultaneous mode can be 
                                           used for this channel,
									       FALSE:Otherwise */

    BINT_CallbackHandle     hDSPCallback;  
                                        /* Handle for AUD_DSP_INTH0_R5F_STATUS 
                                           L2 RAPTOR interrupt callbacks */

    BINT_CallbackHandle     hDSPAsynCallback[BRAP_RM_P_MAX_DSPS];  
                                        
    BINT_CallbackHandle     hDSPSynCallback[BRAP_RM_P_MAX_DSPS];  
                                        
    BINT_CallbackHandle     hFMMCallback; 
                                        /* Handle for BCHP_INT_ID_FMM_BF2
    								       RAPTOR interrupt callbacks */
#ifdef CRC_ENABLE    								       
     BINT_CallbackHandle     hFMMOpCallback; 
                                        /* Handle for BCHP_INT_ID_FMM_OP
                                           RAPTOR interrupt callbacks */
#endif
   								       
    BRAP_P_AppIntCbInfo     sAppIntCbInfo[BRAP_Interrupt_eMaxInterrupts]; 
                                        /* Contains info about application 
                                           interrupt callbacks. Is filled 
                                           when user installs an application 
                                           level callback. */    
    uint32_t                ui32FmmIntMask; 
                                        /* Bits in AIO_INTH_R5F that are 
                                           unmasked */
    uint32_t                ui32FmmBFIntMask; 
                                        /* Bits in AUD_FMM_BF_ESR1 that are 
                                           unmasked */
    uint32_t                ui32DspIntMask; 
                                        /* Bits in AUD_DSP_INTH0_R5F that are 
                                           unmasked */
    uint32_t                ui32AsynIntMask;
                                        /*This will be used to store the asynchronous
                                        interrupts mask*/
	BRAP_P_TrickModeState   sTrickModeState;	
                                        /* Trick mode state of the channel */
	BRAP_P_State		    eState;		/* Channel start/stop state */	
    unsigned int            uiAssociationId[BRAP_MAX_ASSOCIATED_GROUPS];
                                        /* Association ID: This indicates 
                                           the associated group ID's to which 
                                           this channel belongs. The ID acts as 
                                           an index for sAssociatedChanGroup 
                                           available in the RAP device handle */
    BRAP_DstDetails         *pDstDetails[BRAP_P_MAX_DST_PER_RAPCH]; 
                                        /* Pointer to sDstDetails present in
                                           the associated channel handle */
    unsigned int              uiInputSamplingRate; 
                                        /* Input sampling rate of the channel. 
                                           Primarily used for PCM PB and PCM CAP channel*/
    BRAP_OutputMode         eInputAudMode;
                                        /* Input Audio Mode for this channel */   
    bool                    bInputLfeOn;/* LFE on or off for the input audio 
                                           mode for this channel */
 	bool			        bInternalCallFromRap;	
                                    /* Flag to distinguish whether an API
									   is getting called from external
									   application or from within RAP modules */
    BRAP_GainControl        sGainInfo;
                                    /* Gain Coefficients programmed by the 
                                       application */
    BRAP_GainControl        sPanInfo;
                                    /* Pan Coefficients programmed by the 
                                       application */
    BRAP_P_ScalingInfo      sScalingInfo; /* Gain coeff per ch per op port */
    BRAP_P_InternalDst      sIntDst[BRAP_RM_P_MAX_INTERNAL_DST];
    BRAP_MixingCoef         sMixingCoeff;
	BRAP_CaptureMode  eCapMode;     /* Capture mode: capture only, 
	                                   Bypass or full Duplex valid 
	                                   only for a capture channel */

#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)
    BRAP_P_ChannelRBufPool  sChanRBufPool;
                                    /* Pre-allocated channel rbuf pool */
#endif									
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    bool                    bIndepDelayEnabled;  /* TRUE: Output ports connected to this 
                                            channel require larger 
                                            rbufs in order to support independent delay.
                                            FALSE: Normal RBUF size will be used 
                                            => No output port connected to this 
                                            channel can have an independent delay */                                                                
#endif
    BRAP_P_DecoderSettings  sDecSettings; /* Decoder settings, valid for decode channel */

    BRAP_P_ChannelAudioProcessingStage sChAudProcessingStage[BRAP_P_MAX_DST_PER_RAPCH]
                                                           [BRAP_MAX_STAGE_PER_BRANCH_SUPPORTED];
    BKNI_EventHandle		hFWAckEvent; 
	bool					bDecLocked;     /* Decoder lock/unlock state for this channel */
	BAVC_AudioSamplingRate	eSamplingRate;  /* Sampling freq value */
    unsigned int            uiNumCapPath;   /* No. of capture path present in channel.
                                            In case of watchdog recovery, It will 
                                            be retieved from Rapch */


    BRAP_AudioDescriptorFadeHandle hAdFade; /* Audio Descriptor handle for this channel */
    unsigned int  uiPBRate;       /* Rate at which the decoded audio 
                                                should be played out.
                                                Range: BRAP_DSPCHN_PLAYBACKRATE_NORMAL/2 to BRAP_DSPCHN_PLAYBACKRATE_NORMAL*2. i.e.0.5x to 2x
                                                Default BRAP_DSPCHN_PLAYBACKRATE_NORMAL. Currently Granularity of this is 1*/                                              
    BRAP_OutputMode	                eChannelOutputMode;	                                                
    bool                    bOpenTimeWrToRbuf;/*True: If Playback channel can write in to Rbuf at OpenTime*/
    unsigned int        uiTotalErrorCount;/*PTS_Error + Crc_Error + Dec_unlock +TSM_Fail */
    bool                    bGateOpened;/*True= If FW opens FMM gate, It gets et to true in  RampEnableIsr*/
    bool		    bStopinvoked;		/* Channel Intermediate  state */	
    bool    bEnableFixedSampleRateOutput;/*True: Audio output will always be  at 48Khz, 
                                                                    HW SRC will be done based on stream sample rate.
                                                                    Only applicable for Decode Channel Outputting PCM
                                                                    data. Because HW SRC of Compressed data will be garbage*/       
    BRAP_MultiStreamDecoderHandle   hMultiStreamDecoder; /* Multi Stream Decoder handle for this channel */
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT    
    unsigned int            uiFWMixerIpIndex;   /* Input port number of Firmware Mixer */
#endif                                                                    

    unsigned    uiModeValue;/*Whenever ModeChange interrupt occurs, Value gets stored here. This is later used in GetStreamInformation.*/

#ifdef RAP_REALVIDEO_SUPPORT
    /* Video Specific */
    BXDM_PictureProvider_Handle hPictureProvider;
    BRAP_VideoParams		    sVideoParams; /*Real Video Params*/
#endif
#ifdef RAP_GFX_SUPPORT                                                                    
    BRAP_GFX_P_Settings            sGfxSettings; /*Gfx Settings*/
#endif
}BRAP_P_Channel;

/***************************************************************************
Summary:
    Private Structre for AudioProcessingStage handle
**************************************************************************/
typedef struct BRAP_P_AudioProcessingStage
{
BRAP_ProcessingStageSettings    sProcessingStageSettings;
unsigned int			uiStagePosition;
bool	bChSpecificStage;
union
{
    BRAP_ChannelHandle	hRapCh;
    BRAP_AssociatedChannelHandle    hAssociation;
} uHandle;
BRAP_Handle                 hRap;
BRAP_DestinationHandle    hDestHandle[BRAP_P_MAX_DEST_PER_PROCESSING_STAGE];
} BRAP_P_AudioProcessingStage;


/***************************************************************************
Summary:
    Private Structre for AudioDescriptor handle
**************************************************************************/
typedef struct BRAP_P_AudioDescriptor
{
    BRAP_AudioDescriptorFadeTableParams sExtAudioDescDetails; /* Exposed audio desc details */
    BRAP_Handle		                    hRap;		/* Parent Audio Device handle */
    uint32_t                            ui32PanFadeInterfaceAddr; /* Dram location for interface with fw */
    bool                                bAvailable;           /* Is this object free */  
}BRAP_P_AudioDescriptor;




#if BRAP_P_EQUALIZER
/***************************************************************************
Summary:
    Private Structre for Equalizer handle
**************************************************************************/
typedef struct BRAP_P_Equalizer
{
    BRAP_EqualizerSettings              sExtEqualizerDetails;   /* Exposed Equalizer details */
    BRAP_Handle		                    hRap;		            /* Parent Audio Device handle */
    bool                                bAvailable;             /* Is this object free */  
    uint32_t                            ui32SamplingRate;       /* Sampling rate of the equalizer */
    BRAP_SRC_P_Handle                   hSrcEq[BRAP_RM_P_MAX_SRC_IN_CASCADE]
                                              [BRAP_RM_P_MAX_OP_CHANNEL_PAIRS]
                                              [BRAP_RM_P_MAX_PARALLEL_PATHS]
                                              [BRAP_P_MAX_PATHS_IN_A_CHAN];
}BRAP_P_Equalizer;
#endif

/***************************************************************************
Summary:
    Private Structre for a processing stage handle
**************************************************************************/

typedef struct BRAP_P_ProcessingSettings
{
	BRAP_ChannelHandle	hRapCh;
	BRAP_ProcessingSettings	sExternalSettings;
} BRAP_P_ProcessingSettings;


/***************************************************************************
Summary:
    Private function for destination handle
**************************************************************************/
typedef struct BRAP_P_DstDetails
{
    BRAP_DstDetails                 sExtDstDetails; /* Exposed destination details */
	BRAP_AssociatedChannelHandle    hAssociation;   /* Associated channel handle
                                                       to which this destination 
                                                       handle belongs */
	BRAP_P_ProcessingSettings	    *psProcessingSettings[BRAP_P_MAX_RAPCH_PER_DST+ 1];  
													   
    BRAP_DestinationHandle          hParentDestHandle;
    unsigned int                    uiForkingStage;
	BRAP_ProcessingType	            eAudProcessing[BRAP_MAX_PP_PER_BRANCH_SUPPORTED];
    BRAP_ProcessingType             eAudioProcessing;
                                                    /* The main audio processing
                                                       for this dst Details.*/

    BINT_CallbackHandle             hFmmBf1Callback; 
                                        /* Handle for BCHP_INT_ID_FMM_BF1
    								       RAPTOR interrupt callback */
    BINT_CallbackHandle             hFmmBf2Callback; 
                                        /* Handle for BCHP_INT_ID_FMM_BF2
    								       RAPTOR interrupt callback */
    								       
    uint32_t                        ui32FmmIntMask; 
                                        /* Bits in AIO_INTH_R5F that are 
                                           unmasked */    								       
    BRAP_P_AppIntCbInfo             sAppIntCbInfo[BRAP_DestinationInterrupt_eMax]; 
                                        /* Contains info about application 
                                           interrupt callbacks. Is filled 
                                           when user installs an application 
                                           level callback. */

    /* Internal Parameters for Rbuf destination -- begin -- */
    unsigned int      uiLtVolume[BRAP_OutputChannelPair_eMax][BRAP_P_MAX_MIXERS_CONTROLS_IN_DATA_PATH]; /* Left channel volume Level */
    unsigned int      uiRtVolume[BRAP_OutputChannelPair_eMax][BRAP_P_MAX_MIXERS_CONTROLS_IN_DATA_PATH]; /* Right channel volume Level */    
    unsigned int      uiLtBalance[BRAP_OutputChannelPair_eMax][BRAP_P_MAX_MIXERS_CONTROLS_IN_DATA_PATH]; /* Left To Right channel Scaling  */
    unsigned int      uiRtBalance[BRAP_OutputChannelPair_eMax][BRAP_P_MAX_MIXERS_CONTROLS_IN_DATA_PATH]; /* Right To Left channel Scaling  */
    bool              bMute[BRAP_OutputChannelPair_eMax];          /* Mute setting */    
    /* Internal Parameters for Rbuf destination --  end  -- */

    /* Internal Parameter for Equalizer operation*/
    BRAP_EqualizerHandle        hEqualizer;        
    bool        bSampleRateChangeCallbackEnabled;
                                            
}BRAP_P_DstDetails;


/***************************************************************************
Summary:
    Structure defining a group of Raptor Audio Channel.
***************************************************************************/ 
typedef struct BRAP_P_AssociatedChanGrp
{
	BRAP_Handle		   hRap;		/* Parent Audio Device handle */
    BRAP_ChannelHandle hPriDecCh[BRAP_MAX_PRI_DEC_CHAN_IN_ASSOCIATED_GRP];
    BRAP_ChannelHandle hSecDecCh[BRAP_MAX_SEC_DEC_CHAN_IN_ASSOCIATED_GRP];
    BRAP_ChannelHandle hPBCh[BRAP_MAX_PB_CHAN_IN_ASSOCIATED_GRP];
    BRAP_ChannelHandle hCapCh[BRAP_MAX_CAP_CHAN_IN_ASSOCIATED_GRP];

    /* Destinations added to this associated channel handle */
    BRAP_P_DstDetails  sDstDetails[BRAP_P_MAX_DST_PER_RAPCH]; 
    unsigned int       uiNumDstGrp;     /* Number of destination groups added 
                                           to this association */
    BRAP_P_ChannelAudioProcessingStage sAudProcessingStage[BRAP_P_MAX_DST_PER_RAPCH]
                                                           [BRAP_MAX_PP_PER_BRANCH_SUPPORTED];

    BRAP_ChannelHandle hMasterChannel;
    BRAP_MultiStreamDecoderHandle   hMultiStreamDecoder;                                           
}BRAP_P_AssociatedChanGrp;

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
	uint32_t    currentStaticMemoryPointer;
	uint32_t	currentStaticMemoryBase;
	uint32_t	staticMemorySize;
}BRAP_P_StaticAllocInfo;


#if ((BRAP_3548_FAMILY == 1) )
/***************************************************************************
Summary:
   Structure storing linkage of multiple audio channels for simul mode.
***************************************************************************/     
typedef struct BRAP_P_ChannelLinkInfo
{
    bool        bUsedLinkage;   /* Specifies whether linkage is used or not */
    BRAP_ChannelHandle  hRapMasterCh;   /* Master Channel handle */
    BRAP_ChannelHandle  hRapSlaveCh;    /* Slave Channel handles */
    BRAP_ChannelParams  RapMasterParams; /* Master Channel Start Params */
    BRAP_ChannelParams  RapSlaveParams; /* Slave Channel Start Params */
}BRAP_P_ChannelLinkInfo;
#endif
/*********************************************************************
Summary: Structure to hold configuration of Multi Stream Decoder (MS10/11) Detail.

See Also:
**********************************************************************/ 

typedef struct BRAP_P_MultiStreamDecoderparams
{
    unsigned int uiNumSecondaryChannel;
    BRAP_ChannelHandle hPrimaryChannel;     /*  Primary Channel handle.
                                                Not programmable on-the-fly */
    BRAP_ChannelHandle hSecondaryChannel[BRAP_MAX_SEC_CHANNEL_FOR_MS_DECODER]; /* Secondary channel handle.
                                                Not programmable on-the-fly */
} BRAP_P_MultiStreamDecoderparams;


/***************************************************************************
Summary:
    Private Structre for  MS10/11 Decoder.
**************************************************************************/
typedef struct BRAP_P_MultiStreamDecoderDetail
{
    BRAP_P_MultiStreamDecoderparams     sExtMultiStreamDecoderDetails; 
                                                /* Exposed Multi Stream Decoder details */
    BRAP_Handle		                    hRap;		/* Parent Audio Device handle */
    unsigned int                        uiNumValidIOBuffer[BRAP_P_MAX_FW_STG_INPUTS];   /*Number of valid IO Buffers*/
    uint32_t                            ui32InterTaskIoBufferAddress[BRAP_P_MAX_FW_STG_INPUTS][BRAP_AF_P_MAX_CHANNELS]; 
                                                /* Dram memory for Common IO Buffer b/w Master(FW Mixer) and Slave Tasks */
    uint32_t                            ui32InterTaskIoGenericBufferAddress[BRAP_P_MAX_FW_STG_INPUTS]; 
                                                /* Dram memory for Common IO Generic Buffer b/w Master and Slave Tasks */    
    uint32_t                            ui32InterTaskIoBufferHandle[BRAP_P_MAX_FW_STG_INPUTS]; 
                                                /* Offset Address provided by CIT containing Intertask IO Buf details */
    uint32_t                            ui32InterTaskIoGenericBufferHandle[BRAP_P_MAX_FW_STG_INPUTS]; 
                                                /* Offset Address provided by CIT containing Intertask IOGeneric Buf details */
    uint32_t                            ui32FeedbackBufferAddress;
                                                /* Feedback buffer b/w Tasks(Master writes-Slaves read) */
    bool                                bDRAMBuffersUsed[BRAP_P_MAX_FW_STG_INPUTS];    
    uint32_t                            ui32ReconfiguredCITAddress;
    bool                                bAvailable;           /* Is this object free */  
}BRAP_P_MultiStreamDecoderDetail;

typedef struct BRAP_P_OpenTimeMallocs
{
    BRAP_RM_P_ResrcGrant *psResrcGrant;
    BRAP_RM_P_ResrcReq  *psResrcReq;
    BRAP_DSPCHN_P_AudioParams    *pDspChParams;
    BRAP_P_DstDetails                       *pPvtDstDetails;
    BRAP_CIT_P_FwBranchInfo    *psBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];
    bool                                    bStageStatusBufFree[2*BRAP_MAX_FW_TSK_SUPPORTED];
    BRAP_FWIF_P_StageStatus    *psStageStatus[2*BRAP_MAX_FW_TSK_SUPPORTED];
}BRAP_P_OpenTimeMallocs;

/***************************************************************************
Summary:
   Structure defining the state of a Raptor Audio Device.
***************************************************************************/     
typedef struct BRAP_P_Device
{
	BCHP_Handle		        hChip;			/* handle to chip object */
	BREG_Handle		        hRegister;		/* handle to register object */
	BMEM_Handle		        hHeap;			/* handle to memory object */
	BINT_Handle		        hInt;			/* handle to interrupt object */
    BTMR_Handle             hTmr;           /* handle to timer device */

	BRAP_ChannelHandle	    hRapDecCh[BRAP_RM_P_MAX_DEC_CHANNELS];	
                                            /* Decode channel handles */
	BRAP_ChannelHandle	    hRapPbCh[BRAP_RM_P_MAX_PCM_CHANNELS];	
                                            /* Playback channel handles */
	BRAP_ChannelHandle	    hRapCapCh[BRAP_RM_P_MAX_CAP_CHANNELS];	
                                            /* Capture channel handles */
#ifdef RAP_REALVIDEO_SUPPORT
	BRAP_ChannelHandle	    hRapVideoDecCh[BRAP_RM_P_MAX_VIDEO_DEC_CHANNELS];	
                                            /* Video Decode channel handles */                                            
#endif
#ifdef RAP_GFX_SUPPORT                                            
	BRAP_ChannelHandle	    hRapGfxCh[BRAP_RM_P_MAX_GFX_CHANNELS];	
                                            /* Gfx channel handles */                                            
#endif                                            
#ifdef RAP_SCM_SUPPORT
	BRAP_ChannelHandle	    hRapScmCh[BRAP_RM_P_MAX_SCM_CHANNELS];	
											/* SCM channel handles */											 
#endif
                                           
	
	BRAP_DSP_Handle		    hDsp[BRAP_RM_P_MAX_DSPS];		
                                            /* DSP Device handles */
	BRAP_FMM_P_Handle	    hFmm[BRAP_RM_P_MAX_FMMS];		
                                            /* FMM Device handles */
	BRAP_RM_Handle		    hRm;		    /* Resource Manager handle */
	BRAP_ProcessingStageHandle  hAudioProcessingStageHandle[BRAP_MAX_PP_SUPPORTED];
    BRAP_FWIF_P_FwTaskHandle    hFwTask[BRAP_RM_P_MAX_DSPS][BRAP_RM_P_MAX_FW_TASK_PER_DSP];    

    unsigned int    uiCommandCounter;
	
							/* Handle for the audio processing stage  */			
	BRAP_Settings	        sSettings;	    /* Audio Device settings provided 
									           by the caller */



	BRAP_DSP_DwnldMemInfo	sDwnldMemInfo;	/* Structure to store the the buffer
											information to download the Execs.*/	
	BRAP_Mit				sMit;			/* Structure which contains the the 
											information of downloaded executable*/
	BRAP_ImgIdMappingArrays		sImgIdMappingArrays;	/* This structure contains the
											mapping information between Exec Ids
											and its image id. Also, It have the
											size information of each image Id*/
    BRAP_Fwif_P_MemRequirement  sMemReq;
    BRAP_Fwif_MemInfo       sMemInfo; 

	/* Each element of following two arrays correspondings to one output 
	   port as defined by the enum BRAP_OutputPort. If an element of 
	   bOpSettingsValid array is TRUE, corresponding output settings is 
	   passed by the user and is stored at the corresponding element of 
	   sOutputSettings. FALSE indicates otherwise. 
	*/
	bool				    bOpSettingsValid[BRAP_RM_P_MAX_OUTPUTS];
	BRAP_OutputPortConfig   sOutputSettings[BRAP_RM_P_MAX_OUTPUTS];

    /* The Following two arrays are used to store any extrenal settings for the 
       Input ports (External Capture ports )*/
    bool                    bInPortSettingValid[BRAP_CapInputPort_eMax];
#if (BRAP_7550_FAMILY !=1)
	BRAP_InputPortConfig    sInputSettings[BRAP_CapInputPort_eMax];
#endif
    bool                            bOutputMute[BRAP_RM_P_MAX_OUTPUTS];      
								/* Mute status of output ports. Array is 
								    indexed by enum BRAP_OutputPort.
								    TRUE: this output port has been Muted
                                                   	    FALSE: This output port is active */
		
    unsigned int            uiOutputLVolume[BRAP_RM_P_MAX_OUTPUTS];          
								/* Output port Left volume Level.  Array is 
								    indexed by enum BRAP_OutputPort.*/
    unsigned int            uiOutputRVolume[BRAP_RM_P_MAX_OUTPUTS];          
								/* Output port Right volume Level. Array is 
								    indexed by enum BRAP_OutputPort.*/

    unsigned int            uiOutputLVolumeGain[BRAP_RM_P_MAX_OUTPUTS];          
								/* Output port Left volume Level in 5.23 Format.  Array is 
								    indexed by enum BRAP_OutputPort.*/
    unsigned int            uiOutputRVolumeGain[BRAP_RM_P_MAX_OUTPUTS];          
								/* Output port Right volume Level in 5.23 Format. Array is 
								    indexed by enum BRAP_OutputPort.*/                                
	
    									  
	BRAP_P_StaticAllocInfo	sMemAllocInfo;	/* Static memory allocation info */
	unsigned int			uiTotalMemUsed;	/* Current device memory usage of RAP PI */
	bool				    bWatchdogRecoveryOn;
                                            /* Watchdog recovery on flag */
	bool					bStandby; 		/* in standby mode */

    BRAP_P_TestToneRegs     sToneRegs;      /* Registers saved/restored when 
                                               enabling/disabling the testtone */
	
    BINT_CallbackHandle     hCallback;      /* Handle for top level L2 RAPTOR 
                                               device level interrupt callbacks */
#if ((BRAP_3548_FAMILY == 1) )
    BINT_CallbackHandle     hRfAudioCallback;   /* Handle for RF Audio interrupt callback */

    BINT_CallbackHandle     hSpdifRxCallback;   /* Handle for SPDIF Rx interrupt callback */ 
    uint32_t                ui32SpdifRxCompState;   /* For Spdif Rx Hanlding  */                                                                                   
    BRAP_CapInputPort       eCapInputPort;          /* For Spdif Rx Hanlding  */    
    BRAP_P_SPDIFRX_InputState   eSpdifRxState;      /* For Spdif Rx Hanlding  */                                           
#endif                                                
    BRAP_P_AppIntCbInfo     sAppIntCbInfo[BRAP_DeviceLevelInterrupt_eMax]; 
                                            /* Contains info about application 
                                               interrupt callbacks for device 
                                               level interrupts. It is filled when 
                                               user installs an application level 
                                               callback */
	unsigned int		    uiDecodeCount;	/* Current count of decode operations. 
	                                           Decode part of simul mode operations 
	                                           also included in the count */
	unsigned int		    uiPassThruCount;/* Current count of pass thru operations. 
	                                           Pass thru part of simul mode operations
	                                           also included in the count */

    BRAP_P_AssociatedChanGrp sAssociatedCh[BRAP_MAX_ASSOCIATED_GROUPS];
                                            /* Array of group of channels associated 
                                               together */

    BRAP_DownMixingCoef     sDnMixCoeff[BRAP_P_MAX_DOWN_MIX_COEFF];
                                            /* Array of Possible Downmix Coeffs */
#if ((BRAP_3548_FAMILY == 1) )
    BRAP_P_ChannelLinkInfo      sChLinkInfo[BRAP_RM_P_MAX_LINKAGE_SUPPORTED];   /* Channel Link information for linked channels */
#endif
#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)
    BRAP_P_DeviceRBufPool   sDeviceRBufPool;/* Pre-allocated device Rbuf pool */        
#endif
  unsigned int        uiMixerRampStepSize; /*Mixer volume ramp step size. Its range is 0  to 0x7FFFFFF */ 
  unsigned int        uiSrcRampStepSize; /*SRC Scale ramp step size. Its range is 0  to 0x8000*/ 
  unsigned int        uiScalingCoefRampStepSize; /*Mixer Scaling coefficients ramp step size. Its range is 0  to 0x7FFFFFF . Default is 0x00100000*/  

    bool    bWatchdogTriggered;

    BRAP_P_AudioDescriptor  sAudioDescObjs[BRAP_P_MAX_AUDIO_DESC_OBJECTS];  /* Audio Descriptor Objects */
#if BRAP_P_EQUALIZER
    BRAP_P_Equalizer        sEqualizerObjs[BRAP_P_MAX_EQUALIZER_OBJECTS];   /* Equalizer objects */
#endif
#if (BRAP_7405_FAMILY == 1)
/* This implementaion is to fix a case where Dts XCD to PCm transition on 
HDMI soesn't happen on Onkyo reciever when we don't diable MAI_CFG at MAI_STOP*/
    bool    bLastRunHadDtsTranscodeOnHdmi;
    bool    bLastRunHadDtsTranscodeOnSpdif;
#endif    
#ifdef BCHP_AUD_FMM_MISC_HIFIOUT_SEL 
#if (BRAP_MAX_RFMOD_OUT > 0)
    BRAP_OutputPort eRfModSource[BRAP_MAX_RFMOD_OUT];
#endif
#endif
bool bIsExecDownloaded[BRAP_AF_P_AlgoId_eMax];/*stores if executables are already downloaded or not*/

BRAP_P_MultiStreamDecoderDetail sMultiStreamDecoderObjs[BRAP_P_MAX_MULTI_STREAM_DECODER_OBJECTS];  /*Multi Stream decoder Object*/

BRAP_P_OpenTimeMallocs  sOpenTimeMallocs;

#if (BRAP_7550_FAMILY !=1)
	BRAP_OP_PllSettings		sPllSettings[BRAP_OP_Pll_eMax];
#endif

}BRAP_P_Device;

/***************************************************************************
Summary:
    BRAP_P_OpPortPrevMuteStatus: Structure that holds mute states of all the
    output ports associated with a decode channel. This structure is used in
    sample rate interrupt handling. Structure is initialized with previous 
    mute states (prior to sample rate interrupt occurrence) of output ports.
    All the output ports are muted as part of sample rate interrupt handling. 
    At the end of sample rate handler, previous mute states are read from 
    this structure and restored back.
***************************************************************************/     
typedef struct BRAP_P_OpPortPrevMuteStatus
{
	bool bDecMuteStatus[BRAP_MAX_ASSOCIATED_GROUPS][BRAP_P_MAX_DST_PER_RAPCH][BRAP_OutputChannelPair_eMax];	
                                            /* Array to hold mute states of output ports that have
							 their handles in sModuleHandle */
} BRAP_P_OpPortPrevMuteStatus;




/*}}}*/


/*{{{ Function prototypes */

BERR_Code BRAP_P_Open(
	BRAP_Handle 			hRap	/* [in] The Raptor Audio device handle*/
);


/* BRAP_P_MuteChannelOutputOnSr_isr: Mutes the output ports associated with
 * a decode channel at output port level on sample rate change. Returns the
 * previous mute status of output ports in psOpPortPrevMuteStatus.
 */
BERR_Code 
BRAP_P_MuteAndSetSRChannelOutputOnSr_isr
(
	BRAP_ChannelHandle hRapCh,		/* [in] The RAP decode channel handle */ 
	BRAP_P_OpPortPrevMuteStatus *psOpPortPrevMuteStatus, /* [out] Returns previous mute states of output ports */
       BAVC_AudioSamplingRate	eSamplingRate  /* Sampling freq value */	
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

BERR_Code
BRAP_P_ProgramSRCOfPbAndCapCh(
                BRAP_ChannelHandle hRapCh,
                unsigned int        ui32SamplingRate);
/* BRAP_P_GetWatchdogRecoveryFlag: Returns watchdog recovery flag */
bool BRAP_P_GetWatchdogRecoveryFlag(BRAP_Handle hRap);

/* BRAP_P_GetInternalCallFlag: Returns Internal call flag
 * Internal Call Flag = True => Function is getting called due to top level exposed API
 * getting internally called (e.g. BRAP_DEC_Start() getting called from BRAP_DEC_Flush() etc)
 * Internal Call Falg = False => Function is getting called due to top level exposed API
 * getting called externally (e.g. BRAP_DEC_Start() getting called from application )
 */
bool BRAP_P_GetInternalCallFlag(BRAP_ChannelHandle hRapCh);

/*#define BRAP_MEM_DBG*/
/* BMEM can give effective memory leak information location by tagging */
#ifndef BRAP_MEM_DBG
void *BRAP_P_AllocAligned(
		BRAP_Handle  hRap,        /* [in] The RAP device handle */
		size_t       ulSize,      /* [in] size in bytes of block to allocate */
		unsigned int uiAlignBits, /* [in] alignment for the block */
		unsigned int uiBoundary   /* [in] boundary restricting allocated value */
		);
#else
#define BRAP_P_AllocAligned(hRap,ulSize,uiAlignBits,uiBoundary) \
    { \
        BMEM_AllocAligned((hRap)->hHeap, (ulSize), (uiAlignBits), (uiBoundary)) \
    }
#endif

/* BRAP_P_Free : A wrapper around the BMEM module to  free device memory.
 * Device memory is actually freed only for non-7411 chips. 
 */
BERR_Code BRAP_P_Free(
		BRAP_Handle  hRap,        	/* [in] The RAP device handle */
		void *pMemPtr			/* [in] Pointer to device memory to be freed */
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

/***************************************************************************
Summary:
    isr version of BRAP_ConvertOffsetToAddress
****************************************************************************/
#define BRAP_ConvertOffsetToAddress_isr(hHeap, ulOffset, ppvAddress) \
	BRAP_ConvertOffsetToAddress(hHeap, ulOffset, ppvAddress)

/***************************************************************************
Summary:
    isr version of BRAP_P_ConvertAddressToOffset
****************************************************************************/
#define BRAP_P_ConvertAddressToOffset_isr(hHeap, pvAddress, pulOffset) \
	BRAP_P_ConvertAddressToOffset(hHeap, pvAddress, pulOffset)
	

BERR_Code BRAP_P_CopyDataToDram(BMEM_Handle hHeap, uint32_t *data, uint32_t memAdr, uint32_t size);


/* BRAP_P_GetSelectedAlgos: This function returns audio types selected by application
 * during BRAP_Open call */
BERR_Code BRAP_P_GetSelectedAlgos(
			BRAP_Handle	hRap,		/* [in] RAP Handle */
			bool *pbSelectedAlgos	/* [in] pointer to bSupportAlgos[BRAP_MAX_AUDIO_TYPES] array */
);


#ifndef EMULATION
void BRAP_MemWrite32_isr(
					uint32_t	offset, /* Memory offset to write */
					uint32_t	data   /* Data to write */
					);
void BRAP_MemWrite32(
					uint32_t	offset, /* Memory offset to write */
					uint32_t	data   /* Data to write */
					);
void BRAP_MemWrite8(
					uint32_t	offset, /* Memory offset to write */
					uint8_t		data   /* Data to write */
					);
uint32_t BRAP_MemRead32_isr(
						uint32_t	offset /* Memory offset from which read will happen */
					   );
uint32_t BRAP_MemRead32(
						uint32_t	offset /* Memory offset from which read will happen */
					   );
uint8_t BRAP_MemRead8(
						uint32_t	offset /* Memory offset from which read will happen */
					   );

void BRAP_DSP_Write32(BREG_Handle hReg,
					uint32_t		offset,
					uint32_t		data,
					bool			bIsrCxt);

uint32_t BRAP_DSP_Read32(BREG_Handle hReg,
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

#endif

#ifdef EMULATION
#define BRAP_P_DRAMWRITE(addr, data)	zmw(addr, data)
#define BRAP_P_DRAMWRITE8(addr, data)	tb_zmw8(addr, data)
#define BRAP_P_DRAMREAD(addr)	    	zmr(addr)
#define BRAP_P_DRAMREAD8(addr)    	tb_zmr8(addr)
#else
#define BRAP_P_DRAMWRITE(addr, data)	BRAP_MemWrite32(addr, data)
#define BRAP_P_DRAMWRITE8(addr, data)	BRAP_MemWrite8(addr, data)
#define BRAP_P_DRAMREAD(addr)			BRAP_MemRead32(addr)
#define BRAP_P_DRAMREAD8(addr)		BRAP_MemRead8(addr)
#endif
	
#ifdef EMULATION
#define BRAP_MemWrite32(addr, data)  BRAP_P_DRAMWRITE(addr, data)
#define BRAP_MemWrite32_isr(addr, data)  BRAP_P_DRAMWRITE(addr, data)
#define BRAP_MemRead32(addr)  BRAP_P_DRAMREAD(addr)
#define BRAP_MemRead32_isr(addr)  BRAP_P_DRAMREAD(addr)
#define BRAP_DSP_Write32(hReg, addr, data, bIsrCxt) BRAP_Write32(hReg, addr, data)
#define BRAP_DSP_Read32(hReg, addr, bIsrCxt) BRAP_Read32(hReg, addr)
#endif


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

#if (BRAP_7550_FAMILY!=1)
BERR_Code
BRAP_P_GetPllForOp(
	const BRAP_Handle 	hRap, /* [in] The RAP handle */
	const BRAP_OutputPort	eOutputPort,	/* [in] Output port */
       BRAP_OP_Pll  * pPll  /* [out] the Pll associated with this output port */
);
#else
BERR_Code
BRAP_P_GetPllForOp(
	const BRAP_Handle 	hRap, /* [in] The RAP handle */
	const BRAP_OutputPort	eOutputPort,	/* [in] Output port */
       BRAP_OP_MClkRate  * eMClkRate  /* [out] the Pll associated with this output port */
);
#endif

BERR_Code BRAP_P_ConvertDelayToStartWRPoint(
	unsigned int 			uiDelay /* micro sec */,
	BRAP_BufDataMode 		eBufDataMode,
	BAVC_AudioSamplingRate 	eSamplingRate,
	unsigned int			uiIpBitsPerSample,
    unsigned int	        *pStartWRPoint
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
    BRAP_Interrupt  eInterrupt,  /* [in] The interrupt that needs to be deactivated */
    BRAP_FWIF_P_ResponseType    eRsp
);

/*****************************************************************************
Summary:
    Used to unmask a particular interrupt.

Description:
    This PI should be used by the application to unmask an interrupt. This can 
    be done only for the interrupts that are handled by Raptor PI and listed in 
    BRAP_DestinationInterrupt. For all other interrupts, application has to use 
    the standard INT interface.

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_UnmaskDestinationInterrupt (
	BRAP_DestinationHandle        hDstHandle,	   /* [in] The RAP destination handle */
    BRAP_DestinationInterrupt     eInterrupt  /* [in] The interrupt that needs to be deactivated */
);

/*****************************************************************************
Summary:
    Used to unmask a particular interrupt - isr version.

Description:
    ISR version routine for BRAP_P_UnmaskDestinationInterrupt().
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_UnmaskDestinationInterrupt_isr (
	BRAP_DestinationHandle        hDstHandle,	   /* [in] The RAP destination handle */
    BRAP_DestinationInterrupt     eInterrupt  /* [in] The interrupt that needs to be deactivated */
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
    BRAP_Interrupt  eInterrupt,  /* [in] The interrupt that needs to be deactivated */
    BRAP_FWIF_P_ResponseType    eRsp    
);

/*****************************************************************************
Summary:
    Used to mask a particular interrupt.

Description:
    This PI should be used by the application to mask an interrupt. This can 
    be done only for the interrupts that are handled by Raptor PI and listed in 
    BRAP_DestinationInterrupt. For all other interrupts, application has to use 
    the standard INT interface.

Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_MaskDestinationInterrupt (
	BRAP_DestinationHandle        hDstHandle,	   /* [in] The RAP destination handle */
    BRAP_DestinationInterrupt     eInterrupt  /* [in] The interrupt that needs to be deactivated */
);

/*****************************************************************************
Summary:
    Used to unmask a particular interrupt - isr version.

Description:
    ISR version routine for BRAP_P_MaskDestinationInterrupt().
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
******************************************************************************/
BERR_Code BRAP_P_MaskDestinationInterrupt_isr (
	BRAP_DestinationHandle        hDstHandle,	   /* [in] The RAP destination handle */
    BRAP_DestinationInterrupt     eInterrupt  /* [in] The interrupt that needs to be deactivated */
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
    BRAP_ChannelType      eNewChannelType,/*[in] The new RAP Channel type */
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
BERR_Code BRAP_P_CheckIfCompressedMixedWithPCM (
	const BRAP_Handle hRap,			/*[in] The Rap Handle*/
	const BRAP_OutputPort eOutputPort, /*[in] The output port for the current channel*/
	bool bNewChannelCompressed,	/*[in] Current channel compressed or not*/
	bool *bIsCompressedMixedWithPCM	/*[out] TRUE: if compressed and PCM are being mixed 
										     FALSE: If not*/
    );
/*}}}*/

/***************************************************************************
Summary:
    Private function that initializes Raptor channel handle with invalid 
    values.  
**************************************************************************/
BERR_Code 
BRAP_P_InitRapChannelHandle (
    BRAP_ChannelHandle  hRapCh,  /* [in] Rap channel handle to be inited to 
                                   invalid values */
BRAP_Handle 			    hRap		    /* [in] The Raptor Audio handle*/
                                   
    );

/*****************************************************************************
Summary:
    Private function that initializes Destination Details with invalid values.  
******************************************************************************/
void 
BRAP_P_InitDestination(
    BRAP_P_DstDetails  *psDstDetails  /* [in] Raptor Destination to be 
                                         initialized to invalid values */
    );

/***************************************************************************
Summary:
	Private API used to open a decode channel.
Description:
	It is used to instantiate a decode channel. It allocates channel handle 
	and resource required for the channel if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_DEC_P_ChannelClose
****************************************************************************/
BERR_Code BRAP_DEC_P_ChannelOpen( 
    BRAP_Handle                     hRap,		    /* [in] Raptor Audio Device 
                                                       handle*/
    BRAP_ChannelHandle              hRapCh,		    /* [in] Raptor Decode 
                                                       Channel handle */
    const BRAP_ChannelSettings      *pChnSettings   /* [in] Channel settings*/ 
	);

/***************************************************************************
Summary:
	API used to close a decode channel.
Description:
	It closes the instance of a decode channel operation. It frees the 
	channel handle and resources associated with it if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_DEC_P_ChannelOpen
****************************************************************************/
BERR_Code BRAP_DEC_P_ChannelClose( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	);

/***************************************************************************
Summary:
	Private API used to open a Play Back channel.
Description:
	It is used to instantiate a Play Back channel. It allocates channel handle 
	and resource required for the channel if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_PB_P_ChannelClose
****************************************************************************/
BERR_Code BRAP_PB_P_ChannelOpen( 
    BRAP_Handle                     hRap,		    /* [in] Raptor Audio Device 
                                                       handle*/
    BRAP_ChannelHandle              phRapCh,		/* [in] Raptor Play Back 
                                                       Channel handle */
    const BRAP_ChannelSettings      *pChnSettings   /* [in] Channel settings*/ 
	);
/***************************************************************************
Summary:
	API used to close a Play Back channel.
Description:
	It closes the instance of a Play Back channel operation. It frees the 
	channel handle and resources associated with it if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_PB_P_ChannelOpen
****************************************************************************/
BERR_Code BRAP_PB_P_ChannelClose( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	);

/***************************************************************************
Summary:
	Private API used to open a capture channel.
Description:
	It is used to instantiate a capture channel. It allocates channel handle 
	and resource required for the channel if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_CAP_P_ChannelClose
****************************************************************************/
BERR_Code BRAP_CAP_P_ChannelOpen( 
    BRAP_Handle                     hRap,		    /* [in] Raptor Audio Device 
                                                       handle*/
    BRAP_ChannelHandle              phRapCh,		/* [in] Raptor Capture 
                                                       Channel handle */
    const BRAP_ChannelSettings      *pChnSettings   /* [in] Channel settings*/ 
	);

/***************************************************************************
Summary:
	API used to close a capture channel.
Description:
	It closes the instance of a capture channel operation. It frees the 
	channel handle and resources associated with it if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_CAP_P_ChannelOpen
****************************************************************************/
BERR_Code BRAP_CAP_P_ChannelClose( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	);

/***************************************************************************
Summary:
    Private function that programs the scaling and panning coefficients for 
    a mixer    
**************************************************************************/
BERR_Code BRAP_P_ProgramCoefficients(
    BRAP_ChannelHandle          hRapCh,         /* [in] Rap channel Handle */
    const BRAP_MixingCoef	    *pMixingCoeff   /* [in] Mixing Coeff passed 
	                                               during channel start time */
);

#if (BRAP_DYNAMIC_PORT_SWITCHING == 1)
/***************************************************************************
Summary:
    Private function that gets the mixer associated to a output port
    
**************************************************************************/
BERR_Code
BRAP_P_FindAndDisableMixerOutputForOpPort (
    BRAP_Handle         hRap,               /*[in] Rap Handle */
    BRAP_OutputPort     eOpType,            /*[in] Output Port */
    bool                *pbParentMixerRuns  /*[out]*/
);

/***************************************************************************
Summary:
    Private function that gets the mixer associated to a output port
    
**************************************************************************/
BERR_Code
BRAP_P_FindAndDisableMixerOutputForRbuf (
    BRAP_Handle         hRap,               /*[in] Rap Handle */
    unsigned int     	uiRBufId,           /*[in] Ring Buffer Id */
    bool                *pbParentMixerRuns  /*[out]*/
);

/***************************************************************************
Summary:
    Private function that gets the mixer associated to a output port
    
**************************************************************************/
BERR_Code
BRAP_P_FindAndEnableMixerOutputForOpPort (
    BRAP_Handle         hRap,               /*[in] Rap Handle */
    BRAP_OutputPort     eOpType,            /*[in] Output Port */
    bool                *pbParentMixerRuns  /*[out]*/
);

/***************************************************************************
Summary:
    Private function that gets the mixer associated to a output port
    
**************************************************************************/
BERR_Code
BRAP_P_FindAndEnableMixerOutputForRbuf (
    BRAP_Handle         hRap,               /*[in] Rap Handle */
    unsigned int     	uiRBufId,           /*[in] Ring Buffer Id */
    bool                *pbParentMixerRuns  /*[out]*/
);

#endif

/***************************************************************************
Summary:
    This private routine returns the Raptor Channel handle part of the 
    Association channel handle.
**************************************************************************/
BRAP_ChannelHandle 
BRAP_P_GetRapChHandle(
        BRAP_AssociatedChannelHandle    hAssociatedCh   /* [in] Association handle */
	);

/***************************************************************************
Summary:
    Private function that forms Post-Mixing paths for subsequent destinations of
    a Raptor Channel. This is called after forming a path for first destination.
    The sequence of destinations is as per the order they are added.
****************************************************************************/
BERR_Code 
BRAP_P_FormPostMixingPathsForSubsequentDestinations(
    unsigned int                uiDstnIndex,    /* [in] Destination index in RapCh */
    unsigned int                uiPth,          /* [in] Index to form the Path from */        
    unsigned int                uiAssocId,      /* [in]*/
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
                                                        during channel start time */        
    BRAP_ChannelHandle              hRapCh      /* [in] RAP Channel handle */
        );
        
/***************************************************************************
Summary:
    Private function that identifies and prepares the various usage paths 
    required for this channel.
****************************************************************************/
BERR_Code 
BRAP_P_GetChannelPaths(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    unsigned int                *uiNumCapPath   /* [out] True if capture path
                                                   present in channel*/
    );

/***************************************************************************
Summary:
    Private function that ungets/resets the various usage paths for a particular
    channel.
**************************************************************************/
BERR_Code 
BRAP_P_UngetChannelPaths(
	BRAP_ChannelHandle 			hRapCh		    /* [in] RAP Channel handle */
    );

/***************************************************************************
Summary:
    Private function that checks if an audio algorithm is supported or not.
****************************************************************************/
BERR_Code 
BRAP_P_IsAlgoTypeSupported(
    BRAP_Handle             hRap,
    BRAP_DSPCHN_AudioType   eAlgoType
    );

/***************************************************************************
Summary:
    Private function that checks if a stream type is supported or not.
****************************************************************************/
BERR_Code 
BRAP_P_IsStreamTypeSupported(
    BAVC_StreamType     eStreamType
    );

/* 
TODO: Write summary
*/
BERR_Code BRAP_DEC_P_SetConfig (
	BRAP_ChannelHandle		hRapCh,		        /* [in] RAP channel handle */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */
	);

/* 
TODO: Write summary
*/
BERR_Code BRAP_DEC_P_GetDefaultConfig (
	BRAP_ChannelHandle	    hRapCh,		        /* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	            /* [in] Audio type for which to 
	                                               get default configuration 
	                                               parameters */
	BRAP_ProcessingType	ePpAlgo,	/* [in] Post processing type for
										which to get default configuration
										parameter. */
	BRAP_DEC_ConfigParams	*psDecConfigParams  /* [out] Decoder configuration
												   parameters */
	);

/* 
TODO: Write summary
*/
BERR_Code BRAP_DEC_P_GetCurrentConfig (
	BRAP_ChannelHandle	    hRapCh,		        /* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	            /* [in] Audio type for which to 
	                                               get current configuration 
	                                               parameters */
	unsigned int				uiPpBranchId,	/* [in] Branch Id of the post
											processing stage for which to get
											current counfiguration parameters.
											If current configuration is not
											required for any post processing
											stage, this field should be set to
											BRAP_MAX_PP_BRANCH_SUPPORTED */
	unsigned int				uiPpStageId,		/* [in] Stage Id of the post
											processing stage for which to get
											current counfiguration parameters.
											If current configuration is not
											required for any post processing
											stage, this field should be set to
											BRAP_MAX_PP_PER_BRANCH_SUPPORTED */									
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	);

/**************************************************************************
Summary:
    Private function that starts FMM module handles for a particular path.
**************************************************************************/
BERR_Code BRAP_P_StartFmmModuleHandles(
    BRAP_ChannelHandle          hRapCh,         /* [in] Channel Handle */
    const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed
	                                               during channel start time */
    unsigned int                uiPth,          /* [in] Path id */
    bool                        bCompress       /* [in] TRUE: if these resources
                                                   carry compressed data
                                                   FALSE : if PCM data */
    );

/**************************************************************************
Summary:
    Private function that stops FMM module handles for a particular path.
**************************************************************************/
BERR_Code BRAP_P_StopFmmModuleHandles(
    BRAP_ChannelHandle          hRapCh,         /* [in] Channel Handle */
    unsigned int                uiPth           /* [in] Path id */
    );

#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)

/***************************************************************************
Summary:
    Private function that returns the RBuf from Ring Buffer Pool.
***************************************************************************/
void
BRAP_P_ReturnRBufToPool(
    BRAP_ChannelHandle hRapCh,
    BRAP_P_ObjectHandles 		*pPath,			/* [in] Path id */
	bool						bIsDestRbuf,	/* [in] 1= Destination RBuf Type
												   0 =  Other RBuf Type */
	unsigned int 				uiFwProcBranchId,/* [in] FW Processing Branch Id */
	unsigned int 				uiCapPortSrcId	 /* [in] Capture Port Source Id */
    );
/**************************************************************************
Summary:
    Private function that gets the RBuf from Ring Buffer Pool.
**************************************************************************/

BERR_Code BRAP_P_GetRbufFromPool(
	BRAP_ChannelHandle			hRapCh,  			/* [in] Channel Handle */
	const BRAP_ChannelParams	*pAudioParams, 		/* [in] Audio params passed 
	                                               	during channel start time */
	BRAP_P_ObjectHandles		*pPath,				/* [in] Path id */
	bool						bIsDestRbuf,		/* [in] 1= Destination RBuf Type
													 0= Other RBuf Type */
	unsigned int		 		uiFwProcBranchId,	/* [in] FW Processing Branch Id */
	unsigned int 				uiCapPortSrcId,		/* [in] Capture Port Source Id */
	BRAP_RBUF_P_Settings 		*pRBufSettings,	    /* [out] Ring Buffer Settings */
	unsigned int                uiChannelNo         /* [in] Input Channel from 
	                                                   the channel pair */
	);
#endif	
/**************************************************************************
Summary:
    Private function that opens FMM module handles for a particular path.
**************************************************************************/
BERR_Code BRAP_P_OpenFmmModuleHandles(
    BRAP_ChannelHandle          hRapCh,         /* [in] Channel Handle */
    const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    unsigned int                uiPth,          /* [in] Path id */
    BRAP_RM_P_ResrcGrant        *pRsrcGrnt      /* [in] Resources to be opened */
    );

/**************************************************************************
Summary:
    Private function that closes FMM module handles for a particular path.
    Note: Output and SpdifFm module handles are not closed here.
**************************************************************************/
BERR_Code BRAP_P_CloseFmmModuleHandles(
    BRAP_ChannelHandle      hRapCh,     /* [in] Channel Handle */
    unsigned int            uiPth       /* [in] Path id */
    );

/**************************************************************************
Summary:
    Private function that links various FMM module handles to each other 
    for a particular path.
**************************************************************************/    
BERR_Code BRAP_P_LinkFmmModuleHandles(
    BRAP_ChannelHandle      hRapCh,     /* [in] Channel Handle */
    unsigned int            uiPth      /* [in] Path id */
    );

/**************************************************************************
Summary:
    Private function that unlinks various FMM module handles for a particular
    path.
**************************************************************************/    
BERR_Code BRAP_P_UnlinkFmmModuleHandles(
    BRAP_ChannelHandle      hRapCh,     /* [in] Channel Handle */
    unsigned int            uiPth       /* [in] Path id */
    );
    
/***************************************************************************
Summary: 
    This function gives the enum corresponding to the absolute value of 
    BAVC_AudioSamplingRate 
**************************************************************************/    
BERR_Code BRAP_P_ConvertSrToEnum (
    unsigned int            uiSR,   /* [in] sampling rate unsigned value */
    BAVC_AudioSamplingRate  *eSR    /* [out] samping rate enum */
);

/***************************************************************************
Summary: 
    This function gives the absolute value corresponding to the enum 
    BAVC_AudioSamplingRate 
**************************************************************************/ 
BERR_Code BRAP_P_ConvertSR (
    BAVC_AudioSamplingRate   eSR,   /* [in] samping rate enum */
    unsigned int            *uiSR   /* [out] sampling rate unsigned value */
);


BERR_Code BRAP_P_GetAudOpModeProp(
    BRAP_OutputMode     eOpMode,        /* [in] Output mode */
    bool                bLfeOn,         /* [in] if LFE is on/off */
    BRAP_P_OpAudModProp *sOpModeProp    /* [out] OpMode property structure */    
    );

/**************************************************************************
Summary:
    Private function that forms the Output ports start param.
**************************************************************************/ 
BERR_Code BRAP_P_FormOpParams
(
	BRAP_ChannelHandle      hRapCh,		    /* [in] Rap Channel handle */ 
    BRAP_P_OpHandleInfo     *pOpInfo,       /* [in] Output handle info */
    unsigned int            uiPath,         /* [in] Path Id */
	const BRAP_OutputPort	eOutputPort,    /* [in] Output port */
	BAVC_Timebase           eTimebase,      /* [in] Timebase */
	unsigned int            uiOutputBitsPerSample, 
	                                        /* [in] Bits per sample */
	void                    **pParamAddr	    /* [out] pointer of type void * to 
                                               return a pointer to the output 
                                               parameter struct */	
    );

/**************************************************************************
Summary:
    Private function that returns the FCI Id.
**************************************************************************/ 
BERR_Code 
BRAP_P_PrepareFciId(
    BRAP_P_ObjectHandles    *psPath,        /* [in] Pointer to usage path */
    BRAP_P_LinkInfo         *psInLink,      /* [in] Pointer to the inlink */     
    uint32_t                *pui32FciId,    /* [out] FCI id to be returned */
    unsigned int            uiSrcCascade,   /* [in] Cascade index. Valid only 
                                               for SRC */
    bool                    bLoopBack
    );

/***************************************************************************
Summary:
    Private function gets the burst repetition period for an algo type.

    Recommended Burst Repetition Periods for various algo types are as follows:
    AC3 and DTS = PER_3
    MPEG-2 Layers 1, 2, and 3 < 32kHz = PER_64
    MPEG-1, MPEG-2/AAC, and any other MPEG-2 formats = PER_32
****************************************************************************/
BERR_Code BRAP_P_GetBurstRepetitionPeriodForAlgo(
    BAVC_StreamType                 eStreamType,
                                        /* [in] audio stream type*/
    BRAP_DSPCHN_AudioType           eAudioType, 
                                        /* [in] audio type for which 
                                           burst repetition period is 
                                           sought */
    BRAP_SPDIFFM_P_BurstRepPeriod   *pBurstRepPeriod
                                        /* [out] corresponding burst 
                                           repetition period for the 
                                           algo type passed */
    );

/**************************************************************************
Summary:
    Private function that opens a decode path.
**************************************************************************/
BERR_Code 
BRAP_P_OpenDecPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    const unsigned int          uiPth,      	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    unsigned int                uiNumCapPath    /* [in] If path is feeding to 
                                                   a capture path */
    );

/**************************************************************************
Summary:
    Private function that closes a decode path.
**************************************************************************/
BERR_Code 
BRAP_P_CloseDecPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    );

/**************************************************************************
Summary:
    Private function that starts a decode path.
**************************************************************************/
BERR_Code 
BRAP_P_StartDecPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    );

/**************************************************************************
Summary:
    Private function that stops a decode path.
**************************************************************************/
BERR_Code 
BRAP_P_StopDecPath(
	BRAP_ChannelHandle 			hRapCh,     /* [in] RAP Channel handle */
    const unsigned int          uiPth       /* [in] Path idx corresponding 
                                               to pPath[] array element */
    );
    

/***************************************************************************
Summary:
    Private function that gets the mixer associated to a output port
    
**************************************************************************/

BERR_Code
BRAP_P_GetMixerForOpPort (
    BRAP_Handle         hRap,       /*[in] Rap Handle */
    BRAP_OutputPort     eOpType,    /*[in] Output Port */
    BRAP_MIXER_P_Handle *phMixer,   /*[out] Mixer Handle */
    unsigned int        *puiMixerOp
);


/***************************************************************************
Summary:
    Private function that gets the mixer associated to an output port and which mixer of the 
    cascaded mixers is defined by the mixer level, an input parameter.
    
**************************************************************************/
BERR_Code
BRAP_P_GetCustomMixerForOpPort (
    BRAP_Handle         hRap,       /*[in] Rap Handle */
    BRAP_OutputPort     eOpType,    /*[in] Output Port */
    const unsigned int  uiMixerLevel,    
    BRAP_MIXER_P_Handle *phMixer,   /*[out] Mixer Handle */
    unsigned int        *puiMixerOp
);

/***************************************************************************
Summary:
    Private function that gets the mixer associated to a ring buffer
    
**************************************************************************/
#if ((BRAP_3548_FAMILY == 1) || (BRAP_7405_FAMILY == 1))
BERR_Code BRAP_P_GetMixerForRBuf(
    BRAP_Handle         hRap,       /*[in] Rap Handle */
    unsigned int     	uiRBufId,   /*[in] Ring Buffer Id */
    BRAP_MIXER_P_Handle *phMixer,    /*[out] Mixer Handle */
    unsigned int        *puiMixerOp    
);
#endif

/**************************************************************************
Summary:
    Private function that opens a Capture path.
**************************************************************************/
BERR_Code 
BRAP_P_OpenCapPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    );

/**************************************************************************
Summary:
    Private function that closes a capture path.
**************************************************************************/
BERR_Code 
BRAP_P_CloseCapPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    );

/**************************************************************************
Summary:
    Private function that starts a Capture path.
**************************************************************************/
BERR_Code 
BRAP_P_StartCapPath(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	const BRAP_ChannelParams	*pAudioParams,  /* [in] Audio params passed 
	                                               during channel start time */
    const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                   to pPath[] array element */
    );

/**************************************************************************
Summary:
    Private function that stops a capture path.
**************************************************************************/
BERR_Code 
BRAP_P_StopCapPath(
	BRAP_ChannelHandle 			hRapCh,     /* [in] RAP Channel handle */
    const unsigned int          uiPth       /* [in] Path idx corresponding 
                                               to pPath[] array element */
    );

/**************************************************************************
Summary:
    Private function that outputs true if (eAudioMode2, bLfeOn2) is greater
    than (eAudioMode1, bLfeOn1) else it outputs false in pMatched.
**************************************************************************/
BERR_Code
BRAP_P_IsAudModeGreater(
	BRAP_OutputMode 	eAudioMode1, 
	bool 			    bLfeOn1,
	BRAP_OutputMode 	eAudioMode2, 
	bool 			    bLfeOn2,
	bool                *pMatched
	);


/**************************************************************************
Summary:
    Private function that checks the compatibility between the HDMI and 
    its mux selector.
**************************************************************************/
BERR_Code BRAP_P_IsMaiAndMuxSelectorCompatible(
    BRAP_Handle                     hRap,
    BRAP_AssociatedChannelHandle    hAssociatedCh,     
    BRAP_DstDetails                 *pDstDetails    
    );

/**************************************************************************
Summary:
    Private function that checks whether the given path feeds to Mai port.
**************************************************************************/
bool BRAP_P_IsMaiFedByPath(BRAP_P_ObjectHandles *pPath);

bool BRAP_P_IsDacFedByPath(BRAP_P_ObjectHandles *pPath);


/**************************************************************************
Summary:
    Private function that gets the current audio params for a decode channel.
**************************************************************************/
BERR_Code
BRAP_DEC_P_GetCurrentAudioParams (
	BRAP_ChannelHandle	    hRapCh,         /* [in] Decode channel handle */
	BRAP_ChannelParams	    *pAudioParams  /* [out] Current channel params */
	);

/**************************************************************************
Summary:
    Private function that gets the current audio params for a capture channel.
**************************************************************************/
BERR_Code
BRAP_CAP_P_GetCurrentAudioParams (
	BRAP_ChannelHandle	    hRapCh,         /* [in] Capture channel handle */
	BRAP_ChannelParams	    *pAudioParams  /* [out] Current channel params */
	);

#ifdef __cplusplus
}
#endif

BERR_Code BRAP_P_IsVolumeControlSupported(
    BRAP_Handle     hRap,
    BRAP_OutputPort eOpType,
    BRAP_ProcessingType eProcessingType
    );


/**************************************************************************
Summary:
    Private function that checks if a destination or its sub-part is already
    present in the channel. Valid only for an output port destination.
**************************************************************************/
bool BRAP_P_IsDstAlreadyPresent(
    BRAP_Handle                     hRap,
    BRAP_AssociatedChannelHandle    hAssociatedCh,     
    BRAP_DstDetails                 *pDstDetails    
    );
/***************************************************************************
Summary:
    Private function that programs the downmixing coefficients for a mixer    
**************************************************************************/
BERR_Code BRAP_P_ProgramDownMixCoefficients(
    BRAP_ChannelHandle          hRapCh,         /* [in] Rap channel Handle */
    unsigned int                uiPath
);

/**************************************************************************
Summary:
    Private function that returns the output sampling rate for the output 
    port(s) carried in pOpDetails.
**************************************************************************/
BAVC_AudioSamplingRate 
BRAP_P_GetSRFromOpDetails(
    BRAP_ChannelHandle          hRapCh,
    BRAP_OutputPortDetails      *pOpDetails
    );
#if (BRAP_OPEN_TIME_RBUF_ALLOCATION==1)

/***************************************************************************
Summary:
   This Funtion allocates device specific Rbuf Memory as per the settings
   provided in the device handle. 
***************************************************************************/
BERR_Code
BRAP_P_AllocateDeviceRBufPool(
    BRAP_Handle hRap
    );

/***************************************************************************
Summary:
   This Funtion frees device specific Rbuf Memory.
***************************************************************************/
void
BRAP_P_FreeDeviceRBufPool(
    BRAP_Handle hRap
    );

/***************************************************************************
Summary:
   This Funtion allocates channel specific Rbuf Memory as per the settings 
   provided in the channel handle.
***************************************************************************/
BERR_Code
BRAP_P_AllocateChannelRBufPool(
    BRAP_ChannelHandle hRapCh
    );

/***************************************************************************
Summary:
   This Funtion frees channel specific Rbuf Memory.
***************************************************************************/
void
BRAP_P_FreeChannelRBufPool(
    BRAP_ChannelHandle hRapCh
    );
#endif

/**************************************************************************
Summary:
    Private function that mallocs and invalid inits the pPath data structure.
**************************************************************************/
#ifdef BRAP_OPEN_TIME_PATH_ALLOCATION
BERR_Code
BRAP_P_MallocPath(
    BRAP_ChannelHandle  hRapCh,         /* [in] */
    uint32_t uiPth,     /* [in] */
    BRAP_P_ObjectHandles **pPath /* [out] */    
    );
#else
BERR_Code
BRAP_P_MallocPath(
    BRAP_P_ObjectHandles **pPath /* [out] */
    );
#endif

/**************************************************************************
Summary:
    Private function that frees the pPath data structure. It returns the NULL
    pPath.
**************************************************************************/
void
BRAP_P_FreePath(
    BRAP_P_ObjectHandles **pPath /* [in] */
    );

/**************************************************************************
Summary:
    Private function that gets the current channel settings.
**************************************************************************/
BERR_Code
BRAP_P_GetCurrentChannelSettings (
	BRAP_ChannelHandle	    hRapCh,         /* [in] Decode channel handle */
	BRAP_ChannelSettings    *pChanSettings  /* [out] Current channel settings */
	);

/**************************************************************************
Summary:
    Private function that finds the matching reference path in a channel 
    handle and returns the started corresponding path index.
    If the path is not found, uiStartedPathId BRAP_INVALID_VALUE is returned.
**************************************************************************/
BERR_Code
BRAP_P_GetCorrespondingStartedPath(
    BRAP_ChannelHandle      hRefRapCh,          /* [in] */
    unsigned int            uiRefPth,           /* [in] */
    unsigned int            uiRefPathOutIdx,    /* [in] */
    BRAP_ChannelHandle      hStartedRapCh,      /* [in] */
    unsigned int            *uiStartedPathId    /* [out] */
    );

/**************************************************************************
Summary:
    Private function that returns the base sampling rate for a given sampling 
    rate.
    
    Following are the base sampling rates:
        - 32 for (32, 64, 128) 
        - 48 for (48, 96, 192)
        - 44.1 for (44.1, 88.2, 176.4)    
**************************************************************************/
BAVC_AudioSamplingRate 
BRAP_P_GetBaseSR(
    BAVC_AudioSamplingRate eSR /* [in] */
    );

/**************************************************************************
Summary:
    Private function that finds the PLL that can support a particular 
    sampling rate.
**************************************************************************/
BERR_Code
BRAP_P_GetPLLForSR(
    BRAP_ChannelHandle      hRapCh, /* [in] */
    BAVC_AudioSamplingRate  eSR,    /* [in] */
    BRAP_OP_Pll             *ePll   /* [out] */
    );

/**************************************************************************
Summary:
    Private function that returns true if eCapPort is an internal capPort.
**************************************************************************/
bool
BRAP_P_IsInternalCapPort(
    BRAP_CapInputPort       eCapPort
    );

/**************************************************************************
Summary:
    Private function that gets the current audio params for a PCM Playback
    channel.
**************************************************************************/
BERR_Code
BRAP_PB_P_GetCurrentAudioParams(
	BRAP_ChannelHandle	    hRapCh,        /* [in] PCM PB channel handle */
	BRAP_ChannelParams	    *pAudioParams  /* [out] Current channel params */
	);


BERR_Code
BRAP_P_GetPvtDstDetails(
    BRAP_ChannelHandle  hRapCh,         /* [in] */
    BRAP_DstDetails     *pDstDetails,   /* [in] */
    BRAP_P_DstDetails   *pPvtDstDetails, /* [out] */
    unsigned int        *ui32AssocId
    );    

#define BRAP_P_GetPvtDstDetails_isr(hRapCh,pDstDetails,pPvtDstDetails,ui32AssocId) \
    BRAP_P_GetPvtDstDetails(hRapCh,pDstDetails,pPvtDstDetails,ui32AssocId)

BERR_Code
BRAP_P_GetAssocId(
    BRAP_ChannelHandle  hRapCh,         /* [in] */
    BRAP_DstDetails     *pDstDetails,   /* [in] */
    unsigned int            *ui32AssocId    /* [out] */
    );    



/**************************************************************************
Summary:
    Private function that returns Spdif channel parametrs.
	If in the Path
		- Only SPDIF0 exist : return channel params of SPDIF0
		- Only SPDIF1 exist : return channel params of SPDIF1		
		- If both SPDIF0 and SPDIF1 exist : both should have same c-bit config 
			params. And any one of them is returned.
**************************************************************************/
BERR_Code BRAP_P_GetSpdifChanStatusParams(
	BRAP_ChannelHandle 				hRapCh,						/* [in] */
	const unsigned int 				uiPth,						/* [in] */
	BRAP_OP_SpdifChanStatusParams	*psSpdifChStatusParams,		/* [out] */
	bool							*pbUpdateChStatusParams		/* [out] */
);

BERR_Code BRAP_P_GetChannelForOpPort(
    const BRAP_Handle       hRap, 	        /* [in] The RAP Device handle */
    const BRAP_OutputPort   eOpType,        /* [in] Output port */    
    BRAP_ChannelHandle      *phChannel,     /* [out] Channel handle */
    BRAP_DSPCHN_Handle      *phDspCh        /* [out] DSP Channel handle */
    );
/**************************************************************************
Summary:
    Private function that Resets the CHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK
    bit for the SrcCh which is unmasked to recieve the interrupt.
    Resetting this bit is a part of hardware workaround. For resetting the 
    bit, First set it to 0, then set the bit to 1 again
**************************************************************************/
void BRAP_P_ReArmFreeMarkInterrupt(
	BRAP_ChannelHandle	hRapch
	);

/**************************************************************************
Summary:
	Isr version of BRAP_P_ReArmFreeMarkInterrupt.
**************************************************************************/
void BRAP_P_ReArmFreeMarkInterrupt_isr(
	BRAP_ChannelHandle	hRapch
	);

/**************************************************************************
Summary:
    Private function that Resets the CHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK
    bit for the Destination which is unmasked to recieve the interrupt.
    Resetting this bit is a part of hardware workaround. For resetting the 
    bit, First set it to 0, then set the bit to 1 again
**************************************************************************/
void BRAP_P_ReArmFullMarkInterrupt(
	BRAP_DestinationHandle        hDstHandle
	);

/**************************************************************************
Summary:
	Isr version of BRAP_P_ReArmFullMarkInterrupt.
**************************************************************************/
void BRAP_P_ReArmFullMarkInterrupt_isr(
	BRAP_DestinationHandle        hDstHandle
	);

BERR_Code BRAP_DSP_P_GetFWSize (
    BRAP_Handle hRap, 
    BRAP_Img_Id firmware_id, 
    uint32_t *size
    );

void BRAP_P_ShowHandleStage( BRAP_ChannelHandle hRapCh);

void BRAP_P_ShowCitSettings(BRAP_P_AudioProcNetwork *psAudProcNetwork, BRAP_CIT_P_InputInfo    *pCitInputStruct);


/**************************************************************************
Summary:
    Update Start time only internally derivable FW exposed Decode parameters    

Description:
	Some user configuration parameters of FW structures are not exposed by PI,
	instead they are internally derived in PI as the required information 
	virtually exists. This may also involve verifying the components of the 
	Audio Network which is formed during Start Channel, so the user config 
	update made before the Raptor Channel is started might miss this. 
	So the below funciton called internally after the Audio Network formation 
	overwrites the relevant FW parameters with the right values before finally 
	doing the Mem Copy to the DRAM that FW reads.

See Also:
    BRAP_SetChannelConfig() -> BRAP_DEC_P_SetConfig()
**************************************************************************/
BERR_Code BRAP_P_UpdateStartTimeDeducibleFWUserConfig(
    BRAP_ChannelHandle      hRapCh,     /* [in] Channel Handle */
    BRAP_DSPCHN_AudioType   eDecodeAlgo /* [in] Decode Audio Algorithm Type */    
);

BERR_Code BRAP_P_MapSrcUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapAdPanUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapAdFadeUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapCustomVoiceUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapCustomSurroundUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapCustomBassUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapPl2UserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapSrsxtUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapSrshdUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapSrsTruVolumeUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );


BERR_Code BRAP_P_MapDolbyVolUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );
    
BERR_Code BRAP_P_MapDV258UserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

    
BERR_Code BRAP_P_MapGenericPassthruUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapAvlUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapDTSENCUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapAc3ENCUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapMp3EncUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapSbcEncUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapDolbyTranscodeUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapBrcm3DSurroundUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapMonoDownmixUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapDsolaUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BERR_Code BRAP_P_MapFwMixerUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapDDREUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );
#endif

BERR_Code BRAP_P_MapAudysseyVolUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapAudysseyABXUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapBtscUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapSrsCsTdUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_MapSrsEqHlUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    );

BERR_Code BRAP_P_ProgramAdaptRateCtrl (
                BRAP_ChannelHandle		hRapCh,
                const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                                       to pPath[] array element */	
		);

BERR_Code BRAP_P_ResetAdaptRateCtrl (
                BRAP_ChannelHandle		hRapCh,
                const unsigned int          uiPth       	/* [in] Path idx corresponding 
                                                                       to pPath[] array element */	
		);
		
/***********************************************
Summary: This function tells if the output of the path is compress or linear.
Please note that before this calling this function, the destinations in the path needs to be populated
***********************************************/
BERR_Code
BRAP_P_IsPathOutputCompress(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	unsigned int                                uiPth,
	bool                                         *pbCompress
	);

 void 
BRAP_P_InitSrcChHandleInfo(
    BRAP_P_SrcChHandleInfo  *pSrcChInfo
    );
 BERR_Code BRAP_P_GetStartedChannel(
    BRAP_ChannelHandle      hRapCh,
    bool                    *pbPrimaryPresent,
    BRAP_ChannelHandle      *phStrtdAssocCh
);

 BERR_Code BRAP_P_GetOtherStartedChannel(
    BRAP_ChannelHandle      hRapCh,
    bool                    *pbPrimaryPresent,
    BRAP_ChannelHandle      *phStrtdAssocCh
);

 bool    BRAP_P_CheckIfPtrInvalid(void * ptr);

 BERR_Code BRAP_P_SetOutputVolumeGain( 
    BRAP_Handle         hRap,       /*[in] Rap Handle */
    BRAP_OutputPort     eOpType,    /*[in] Output Port */
    unsigned int            uiLVolume,      /* [in] Left channel volume in 0.23 format */
    unsigned int            uiRVolume       /* [in] Right channel volume in in 0.23 format*/
);

bool   BRAP_P_IsPassthruSupportedWithoutLicense(BRAP_DSPCHN_AudioType  eType);

BERR_Code BRAP_P_GetAlgorithmProfileId(
    uint8_t     *pui8AlgoProfileId  /* [out] Algorithm Profile Id */
);

/***************************************************************************
Summary:
    Creates Multi Stream Decoder object.
Description:
    This function creates Multi Stream Decoder object. F
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_P_SetMultiStreamDecoderObject
    BRAP_P_GetMultiStreamDecoderObject    
    BRAP_P_DestroyMultiStreamDecoderObject

**************************************************************************/
BERR_Code BRAP_P_CreateMultiStreamDecoderObject(
    BRAP_Handle hRap, 
    BRAP_MultiStreamDecoderHandle *phMultiStreamDecoder
    );

/***************************************************************************
Summary:
    Destroys Multi Stream Decoder object.
Description:
    This function destroys Multi Stream Decoder object. 
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_P_CreateMultiStreamDecoderObject
    BRAP_P_SetMultiStreamDecoderObject
    BRAP_P_GetMultiStreamDecoderObject    
**************************************************************************/
BERR_Code BRAP_P_DestroyMultiStreamDecoderObject(
    BRAP_MultiStreamDecoderHandle hMultiStreamDecoder
    );


/***************************************************************************
Summary:
    Sets the Multi Stream Decoder Object
Description:
    This function sets the Multi Stream Decoder Object. 
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_P_GetMultiStreamDecoderObject
    BRAP_P_CreateMultiStreamDecoderObject
    BRAP_P_DestroyMultiStreamDecoderObject
**************************************************************************/
BERR_Code BRAP_P_SetMultiStreamDecoderObject(
    BRAP_MultiStreamDecoderHandle hMultiStreamDecoder,     /* [in] Audio Desc handle */ 
    BRAP_P_MultiStreamDecoderparams *psParams /* [in] Audio Desc Params */
    );

/***************************************************************************
Summary:
    Gets the Multi Stream Decoder Object
Description:
    This function gets the Multi Stream Decoder Object. 
    
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
    BRAP_P_SetMultiStreamDecoderObject
    BRAP_P_CreateMultiStreamDecoderObject
    BRAP_P_DestroyMultiStreamDecoderObject

**************************************************************************/
BERR_Code BRAP_P_GetMultiStreamDecoderObject(
    BRAP_MultiStreamDecoderHandle hMultiStreamDecoder,     /* [in] Audio Desc handle */ 
    BRAP_P_MultiStreamDecoderparams *psParams  /* [out] Audio Desc Params */
    );
#if (BRAP_7550_FAMILY != 1)
BERR_Code
BRAP_P_ConfigureAssociatedPathsSrc_isr(
    BRAP_ChannelHandle  hRapCh,
    unsigned int        uiAssocId,    
    unsigned int        ui32OutSamplingRate);

BERR_Code
BRAP_P_ConfigureAssociatedChannelsSrc_isr(
                BRAP_ChannelHandle hRapCh,
                unsigned int        ui32SamplingRate);
#endif

bool BRAP_P_IsPPPresent(
    BRAP_ChannelHandle  hRapCh,
    bool                bAssociationNw,    
    BRAP_ProcessingType ePPType);

bool BRAP_P_IsPPPresentOnOutput(
    BRAP_ChannelHandle  hRapCh,
    BRAP_ProcessingType ePPType,
    BRAP_OutputPort    eOpType);

bool BRAP_P_IsMs11UsageMode(
    BRAP_ChannelHandle  hRapCh);

bool BRAP_P_IsMs10UsageMode(
    BRAP_ChannelHandle  hRapCh);

bool BRAP_P_IsMultiStreamDecodeUsageMode(
    BRAP_ChannelHandle  hRapCh);    

bool BRAP_P_IsFwMixingPostLoopbackEnabled(
    BRAP_ChannelHandle  hRapCh);


BERR_Code BRAP_P_EnableAdaptiveRate(
	BRAP_ChannelHandle 			hRapCh,		    /* [in] RAP Channel handle */
	bool                *bAdaptiveRateEnable,
	bool                bPPMCorrection
    );

/* Return whether Association Network(PP) is valid */
bool BRAP_P_IsAssociationNWValid(
    BRAP_P_DstDetails   *pPvtDstDetails);

BERR_Code BRAP_P_IsFMMPathAllocated(
	BRAP_ChannelHandle      hRapCh,             /* [in] RAP Channel handle */
	unsigned int            uiPth,
	bool                    *bFMMPathAllocated  /* [Out] FMM Path not required/allocated */
    );

/***************************************************************************
Summary:
    Check if the Output Clock programming by DSP is required
    
Description:
    This function checks if the Output Clock programming is required by DSP. 
    Example Use case where this returns false, Mixing of two decoders' outputs 
    that are part of same Association whose outputs need to run at Master decoder's 
    stream sample rate. If Slave's stream sample rate is not same as Master, 
    then Slave's output is sample rate converted to match that of Master for mixing 
    and so the clock programming by DSP is not required.
    
    If Master is started first then the output clocks need not be reprogrammed 
    while Slave start. When Slave starts first the output clocks are 
    programmed by DSP for Slave's stream sample rate, later when Master is 
    started the clocks are reprogrammed by DSP for Master's stream sample rate 
    and the Slave output is sample rate converted accordingly. 
    
Returns:
    TRUE on success, otherwise FALSE
See Also:

****************************************************************************/
bool BRAP_P_DspToConfigureOutputClock(
    BRAP_ChannelHandle  hRapCh,         /* [in] RAP Channel handle */
    BRAP_DstDetails     *pDstDetails    /* [in] Pointer to Destination details */
    );
 
#endif /*}}} #ifndef _BRAP_PRIV_H__ */

/* End of File */
