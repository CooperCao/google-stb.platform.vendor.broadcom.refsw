/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
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
 * Module Description:
 *
 * [File Description:]
 *
 ***************************************************************************/

/*******************************************************************************
* regArgo_HI.h - HOST INTERFACE REGISTER DEFINITIONS
*
 * $Id: regArgo_HI.h 352 2010-02-09 19:51:14Z mgallagh $
 * $LastChangedDate: 2010-02-09 11:51:14 -0800 (Tue, 09 Feb 2010) $
 * $LastChangedBy: mgallagh $
 * $Change: 3156 $
 * $LastChangedRevision: 352 $
*******************************************************************************/

#ifndef REG_JUNO_HI_H
#define REG_JUNO_HI_H

#include "bchp_dfe_agcdec.h"
#include "bchp_dfe_bertdec.h"
#include "bchp_dfe_eqdec.h"
#include "bchp_dfe_fecdec.h"
#include "bchp_dfe_fedec.h"
#include "bchp_dfe_mfdec.h"
#include "bchp_dfe_miscdec.h"
#include "bchp_dfe_ntscdec.h"
#include "bchp_dfe_ofsdec.h"
#include "bchp_dfe_ucdec.h"

#include "bchp_dfe_agcdec_8051.h"
#include "bchp_dfe_bertdec_8051.h"
#include "bchp_dfe_eqdec_8051.h"
#include "bchp_dfe_fecdec_8051.h"
#include "bchp_dfe_fedec_8051.h"
#include "bchp_dfe_mfdec_8051.h"
#include "bchp_dfe_miscdec_8051.h"
#include "bchp_dfe_ntscdec_8051.h"
#include "bchp_dfe_ofsdec_8051.h"
#include "bchp_dfe_ucdec_8051.h"

#if (BCHP_CHIP == 35233)
#define BCHP_DFE_BASE_ADDR 0x100000
#else
#define BCHP_DFE_BASE_ADDR 0xe00000
#endif


/*******************************************************************************
* Download Version Number
*******************************************************************************/
#define ixHI_RAM_VER_VALUE_2	ixMISC_RAM_VER_VALUE_2
#define ixHI_RAM_VER_VALUE_1	ixMISC_RAM_VER_VALUE_1
#define ixHI_RAM_VER_VALUE_0	ixMISC_RAM_VER_VALUE_0

/*******************************************************************************
* I2C Speed Select
*******************************************************************************/
#define ixHI_I2C_SPEED_SELECT		ixUC_CONTROL

/*******************************************************************************
* Acquisition Pause Controls
*******************************************************************************/
#define ixHI_VIDEO_PAUSE			ixUC_OP_MODE
#define ixHI_FDC_PAUSE				ixUC_CMD
#define		PAUSE_1_BIT				7
#define		PAUSE_2_BIT				6
#define		PAUSE_3_BIT				5
#define		PAUSE_4_BIT				4

/*******************************************************************************
* Host Interrupts
*
* The host sets bits in the HI_IRQ_MASK to select which sources can cause an
* interrupt output from the microcontroller.
*
* The microcontroller sets bits in the HI_IRQ_SOURCE to indicate the reason for
* a host interrupt.  This register is clear-on-read from the host.
*
* Both registers use the same mask bits, which appear below.
*******************************************************************************/
#define	ixHI_IRQ_SERVICED			ixUC_CMD_STATUS
#define ixHI_IRQ_MASK				ixUC_IRQ_MASK
#define ixHI_IRQ_SOURCE				ixUC_IRQ_SOURCE
#define		HI_IRQ_VIDEO			0x80	/* Video lock/loss status change */
#define		HI_IRQ_FDC				0x20	/* FDC lock/loss status change */
#define		HI_IRQ_RESERVED_10		0x10
#define		HI_IRQ_RESERVED_08		0x08
#define		HI_IRQ_SERVICE_2		0x04	/* Service results byte 2 ready */
#define		HI_IRQ_SERVICE_1		0x02	/* Service results byte 1 ready */
#define		HI_IRQ_SERVICE_0		0x01	/* Service results byte 0 ready */

/*******************************************************************************
* Microcontroller Reset
*******************************************************************************/
#define	ixHI_MICRO_RESET			ixUC_AGC_PROGRAM_DOWNLOAD_CONTROL
#define		UC_RESET				0x80

/*******************************************************************************
* Video Acquisition Control/Status
*******************************************************************************/

#if BTFE_X230FW
typedef enum
{
    VIDEO_MODE_VSB = 1,
    VIDEO_MODE_64QAM,
    VIDEO_MODE_256QAM,
    VIDEO_MODE_MAX
} VideoModeSelect;

/*
 * bit7-bit3 are used as video mode. It is defined as numerical value for the mode instead of using
 * one bit for one mode.
 * bit 0 is used as video status lock detect
 * bit 1 is used as video status loss detect
 */

#define ixHI_VIDEO_MODE_SELECT		ixUC_GP_0
#define 	VIDEO_MODE_SHIFT		3

#define ixHI_VIDEO_STATUS			ixUC_GP_1
#define		VIDEO_STATUS_MODE_MASK	0xF8
#define		AGC_START_MASK				0x04
#define		VIDEO_STATUS_LOSS_DET		0x02
#define		VIDEO_STATUS_LOCK_DET		0x01

#else
#define ixHI_VIDEO_MODE_SELECT		ixUC_GP_0
#define		VIDEO_8VSB_BIT			7
#define		VIDEO_64QAM_BIT			6
#define		VIDEO_256QAM_BIT		5



#define ixHI_VIDEO_STATUS				ixUC_GP_1
#define		VIDEO_STATUS_MODE_MASK		0xF8
#define		VIDEO_STATUS_MODE_8VSB		0x80
#define		VIDEO_STATUS_MODE_64QAM		0x40
#define		VIDEO_STATUS_MODE_256QAM	0x20


#define		VIDEO_STATUS_LOSS_DET		0x02
#define		VIDEO_STATUS_LOCK_DET		0x01
#endif

#define ixHI_SCD_MOD_FORMAT			ixUC_GP_60
/*******************************************************************************
* Analog Acquisition Options
*******************************************************************************/
#define ixHI_ANALOG_IF_FREQUENCY 	ixUC_GP_3
/* Firmware sets this register based on analog video standard. */
typedef enum
{
	IF__44MHZ,
	IF__36MHZ,
	IF__4_0_MHZ,
	IF__4_5_MHZ,
	IF__5_0_MHZ,
    IF__10_7_MHZ
} IF_Frequency;


/*******************************************************************************
* Fast Acquisition Options
*******************************************************************************/
#define ixHI_ACQ_TUNE_SETUP			ixUC_GP_4
#define		FAT_ACQ_SPEEDUP			0x01
#define 	FAT_FW_TUNE				0x02

#define ixHI_ACQ_TUNE_STATUS			ixUC_GP_5
#define 	FAT_FW_TUNE_ERROR		0x02
#define 	FAT_INVALID_MOD_FT		0x80

/*******************************************************************************
* Delay for tuner to settle.  This is not being used by 8051
*  FW as of Nov, 2009, but it may be in the future.
*******************************************************************************/
#define ixHI_AFE_DELAY_MSEC 		ixUC_GP_61

/*******************************************************************************
* IFD Filtering options
*******************************************************************************/
#define ixHI_IFD_FILTER_SELECT  ixUC_GP_62

#define		IFD_FILTER_TYPE_MASK              0x03
#define 	IFD_FILTER_TYPE_SHARP       0x00
#define 	IFD_FILTER_TYPE_LOW_RINGING 0x01
#define 	IFD_FILTER_TYPE_RESERVED1   0x02
#define 	IFD_FILTER_TYPE_RESERVED2   0x03
#define		IFD_SYSTEM_D_EQUALIZER_TYPE_MASK  0x04
#define		IFD_SYSTEM_D_EQUALIZER_OFF  (0x0 << 2)
#define		IFD_SYSTEM_D_EQUALIZER_ON   (0x1 << 2)



/*******************************************************************************
* Submode: ixUC_GP_6, ixUC_GP_7
*******************************************************************************/
#define ixHI_VIDEO_SUBMODE_SELECT		ixUC_GP_6

#define		SUBMODE_MASK		0x0F
#define		SUBMODE_J83_A		0x01
#define		SUBMODE_J83_B		0x02
#define		SUBMODE_J83_C		0x03
#define		SUBMODE_DVBT		0x04
#define		SUBMODE_ISDBT		0x05

#define		J83_MODE_MASK		0xF0
#define		J83_QAM16			0x00
#define		J83_QAM32			0x10
#define		J83_QAM64			0x20
#define		J83_QAM128			0x30
#define		J83_QAM256			0x40

/*******************************************************************************
* Unused - Use for debug
*******************************************************************************/
#define ixHI_UNUSED_1			ixUC_GP_10

/*******************************************************************************
* Device Type
*******************************************************************************/
#define ixHI_DEVICE_TYPE			ixUC_GP_11
#define		TYPE_UNKNOWN	0
#define		TYPE_X255		1
#define		TYPE_X243		2
#define		TYPE_X143		3
#define		TYPE_T507		4
#define     TYPE_X230		5
#define     TYPE_X330		6
/* ADD NEW DEVICE TYPES AT THE END OF THIS LIST */

/*******************************************************************************
* Microcontroller Service Status
*******************************************************************************/
#define ixHI_SERVICE_STATUS_0		ixUC_GP_12
#define ixHI_SERVICE_STATUS_1		ixUC_GP_13
#define ixHI_SERVICE_STATUS_2		ixUC_GP_14
#define		SERVICE_COMPLETE		0x80
#define		BUSY_ERROR				0x40
#define		SERVICE_ID_MASK			0x3F

/*******************************************************************************
* Microcontroller Service Request
*******************************************************************************/
#define ixHI_SERVICE_REQUEST		ixUC_GP_15
#define		SERVICE_REQUEST			0x80

/*******************************************************************************
 * SERVICE ID CODES
 *******************************************************************************/
#define SVC_CSECT_WRITE			0
#define SVC_CSECT_READ			1
#define SVC_I2C_WRITE			2
#define SVC_I2C_READ			3
#define SVC_POWER_UP			4
#define SVC_POWER_SAVING		5
#define SVC_SET_BERT			6
#define SVC_SET_FDC_CONFIG		7
#define SVC_GET_ACQ_STATUS		8
#define SVC_GET_PWR_SPECT		9
#define SVC_GET_DNLD_STATUS		10
#define SVC_GET_VSB_TAPS		11
#define SVC_GET_RS_ERRORS		12
#define SVC_GET_CHANNEL_EST		13
#define SVC_GPIO_MONITOR		14
#define SVC_GET_EQDATA		       15
#define SVC_GET_COFDM_STATUS	16
#define SVC_GET_TIMING_OFFSET	17
#define SVC_SET_BW_FILTER		18
#define SVC_SCI_WRITE			19
#define SVC_GPIO_CONFIG			20
#define SVC_GPIO_READ			21
#define SVC_GPIO_WRITE			22
#define SVC_GET_QAM_TAPS		23
#define SVC_SET_NFRAMES			24
#define SVC_SET_AUDIO_CONFIG	25
#define SVC_SET_VSB_CONFIG		26/*left for backward compatibility, remove eventually */
#define SVC_DEFAULT_IF_CONFIG	26
#define SVC_ATRACKING_RUN		27
#define SVC_ATRACKING_CONFIG	28
#define SVC_GPIO_STATUS_CONFIG  29
#define SVC_DUMMY				60/*Not an invalid service, just a dummy one */
	/* ADD NEW SERVICES HERE AND UPDATE NUMBER BELOW */
#define NUMBER_OF_SERVICES		30

/*******************************************************************************
* Microcontroller Service Input Parameters
*******************************************************************************/
#define ixHI_SERVICE_PARAMS_0		ixUC_GP_16
#define ixHI_SERVICE_PARAMS_1		ixUC_GP_17
#define ixHI_SERVICE_PARAMS_2		ixUC_GP_18
#define ixHI_SERVICE_PARAMS_3		ixUC_GP_19
#define ixHI_SERVICE_PARAMS_4		ixUC_GP_20
#define ixHI_SERVICE_PARAMS_5		ixUC_GP_21
#define ixHI_SERVICE_PARAMS_6		ixUC_GP_22
#define ixHI_SERVICE_PARAMS_7		ixUC_GP_23
#define ixHI_SERVICE_PARAMS_8		ixUC_GP_24
#define ixHI_SERVICE_PARAMS_9		ixUC_GP_25
#define ixHI_SERVICE_PARAMS_10		ixUC_GP_26
#define ixHI_SERVICE_PARAMS_11		ixUC_GP_27
#define ixHI_SERVICE_PARAMS_12		ixUC_GP_28
#define ixHI_SERVICE_PARAMS_13		ixUC_GP_29
#define ixHI_SERVICE_PARAMS_14		ixUC_GP_30
#define ixHI_SERVICE_PARAMS_15		ixUC_GP_31
#define 	SERVICE_PARAMS_LENGTH	16

/*******************************************************************************
* UAGC Tuner specific variables - These are loaded via AGC scripts,
*  then read by the 8051 firmware, then they could be used again
*   ixHI_UAGC_SETPOINT_1    for writes from MIPS to 8051
*   ixHI_UAGC_IF_TOP_DBM_1  for writes from MIPS to 8051
*   ixHI_UAGC_VGA_GAIN_1    for writes from MIPS to 8051
*******************************************************************************/
#define ixHI_UAGC_SETPOINT_1		ixUC_GP_32
#define ixHI_UAGC_SETPOINT_0		ixUC_GP_33
#define ixHI_UAGC_IF_TOP_DBM_1		ixUC_GP_34
#define ixHI_UAGC_IF_TOP_DBM_0		ixUC_GP_35
#define ixHI_UAGC_VGA_GAIN_1		ixUC_GP_36
#define ixHI_UAGC_VGA_GAIN_0		ixUC_GP_37

#define ixHI_UAGC_VIF_SETPOINT_1	ixUC_GP_51
#define ixHI_UAGC_VIF_SETPOINT_0	ixUC_GP_52

#ifdef USE_DFE_ADDR_32BIT

#define ixHI_UAGC_VIF_SETPOINT_OFFSET4_1 (BCHP_DFE_BASE_ADDR + (0xb650<<2))
#define ixHI_UAGC_VIF_SETPOINT_OFFSET4_0 (BCHP_DFE_BASE_ADDR + (0xb651<<2))


/*******************************************************************************
* BE-AGC Board specific variables - These are loaded via AGC scripts,
*  then read by the 8051 firmware, then they could be used again
*   ixHI_UAGC_SETPOINT_1    for writes from MIPS to 8051
*******************************************************************************/
#define ixHI_BEAGC_GAIN_NOMINAL_1	ixUC_GP_57
#define ixHI_BEAGC_GAIN_NOMINAL_0	ixUC_GP_58


/*******************************************************************************
* NTSC tuner specific variables - These are loaded via AGC table,
*  then used by the 8051 firmware when needed
*******************************************************************************/
#define ixHI_ANALOG_FREQ_RESPONSE	ixUC_GP_49
#define ixHI_ANALOG_DAC_GAIN		ixUC_GP_50

/*******************************************************************************
* 4 channel by 16 bit trace function
*
*******************************************************************************/
#define ixHI_TRACE_SELECT_CH1		ixUC_GP_53
#define ixHI_TRACE_SELECT_CH2		ixUC_GP_54
#define ixHI_TRACE_SELECT_CH3		ixUC_GP_55
#define ixHI_TRACE_SELECT_CH4		ixUC_GP_56

/*******************************************************************************
* 16 bit port for host access to configure analog demod firmware.
*  This is being implemented for the following reasons:
*   1. We need the analog demodulator firmware to be configured in the
*       field while we are tuned to a signal
*   2. There are many 16 bit variables in the analog demod firmware.
*   3. This could be slow since the analog demod firmware only runs
*       once every 3 msecs, but if the configuration is done using
*       the SCD GUI serial 115.2kbps link, 3msec is fast enough
*   4. There are problems with 16 bit accesses of hardware registers
*       in Pyro (X244), so forcing the host to poll a ready bit
*       may be necessary anyway.  At any rate, if a separate pair
*       of GP 8-bit registers are used to load a firmware parameter,
*       there would be no guarantee, the firmware would get the
*       complete 16 bit value.
*        Some notes on register access problem.
*         e.g. Reads
*          A. Host begins reads 2 byte HW register - byte 1 goes in temp reg
*          B. 8051 reads any HW register in same register block - byte 1
*                above is overwritten.
*          C. Host completes read of 2 byte HW register - byte 1 and byte 2
*                are concatenated, but byte 1 is wrong
*        e.g. Writes - same type of thing, but generally host won't be
*                writing these registers, when 8051 is running
*
*  Host should poll ready bit before writing address and data
*   e.g.
*
*
*  Details of which address configures which variable will be in ntsc.c
*******************************************************************************/
#define ixHI_PORT16_ADDRESS			ixUC_GP_44
#define ixHI_PORT16_DATA_1			ixUC_GP_45
#define ixHI_PORT16_DATA_0			ixUC_GP_46
#define ixHI_PORT16_READY			ixUC_GP_47
#define	 HI_PORT16_READY__host2firmware__SIZE  1
#define	 HI_PORT16_READY__host2firmware__MASK  0x01
#define	 HI_PORT16_READY__host2firmware__SHIFT 0
#define	 HI_PORT16_READY__firmware2host__SIZE  1
#define	 HI_PORT16_READY__firmware2host__MASK  0x02
#define	 HI_PORT16_READY__firmware2host__SHIFT 1


/*******************************************************************************
* UAGC Status Registers
*
*******************************************************************************/
#define ixHI_UAGC_PVID_DBM_1		ixUC_GP_38
#define ixHI_UAGC_PVID_DBM_0		ixUC_GP_39
#define ixHI_UAGC_SNRVID_DB_1		ixUC_GP_40
#define ixHI_UAGC_SNRVID_DB_0		ixUC_GP_41
#define ixHI_UAGC_DUR_DB			ixUC_GP_59

/*******************************************************************************
* ASM Configuation data: write by SCD, and read by 8051 ASMs
* each config data hold for 32 bytes
*
* ixHI_ASM_CONFIG_AREA:
*  byte 0:     acquire_t acquireConfig;          0 - Directed Acquire, 1 - Full Acquire, 2-Search/Scan
*  byte 1:     bandwidth_t bandwidthConfig;  0 - 6 MHz, 1 - 7 MHz, 2 - 8 MHz
*  byte 2:     spectral_t spectralConfig;   0 - non-inverted, 1 - inverted
*  byte 3-6:  uint32 defaultIFNomRate;  Nominal Rate of default IF center frequency in upper 3 bytes and lower byte 0x00.
*
* ixHI_J83_CONFIG_AREA
*  byte 0:      j 83abcMode_t j83abcMode; 0 - J83A, 1 - J83B, 2 - J83C
*  byte 1:       qamModeDigital_t qamMode; 0 - 16QAM, 1-32QAM, 2-64QAM, 3-128QAM, 4-256QAM
*  byte 2-5:    uint32 symbolRate;
*  byte 6-11:  uchar timingNominalRate[6];
*
* ixHI_UNIFIED_COFDM_CONFIG_AREA
*  -byte 0:    ofdmStandard          -byte 1:  CCIEnable		 -byte 2:    ACIEnable	    -byte 3: mobileMode
*  -byte 4:    StreamPriority        -byte 5:  carrierRange	     -byte 6:    impulse 	    -byte 7: rsLayer
*  -byte 8:    modeGuardMode         -byte 9:  mode              -byte 10:   guard          -byte 11: tpsMode
*  -byte 12:   codeLP                -byte 13: codeHP            -byte 14:   hierarchy      -byte 15: moduation
*  -byte 16:   modulationLayerA      -byte 17:  modulationLayerB -byte 18: modulationLayerC -byte 19: codeRateLayerA
*  -byte 20:   codeRateLayerB		 -byte 21:  codeRateLayerC   -byte 22: segmentLayerA    -byte 23: segmentLayerB
*  -byte 24:   segmentLayerC         -byte 25:  timeInterleaveLayerA  -byte 26: timeInterleaveLayerB  -byte 27: timeInterleaveLayerC
*  -byte 28:   partialReception
*
* ixHI_J83_STATUS_AREA
*  -byte 0:    reacqCounter     -byte 1:  acquireConfig    -byte 2:    bandwidthConfig	    -byte 3: spectralConfig
*  -byte 4:    j83abcMode        -byte 5:  qamMode	     -byte 6-11: timingNominalRate[6]
*  -byte 12:   operationDone   -byte 13: signalDetected  -byte 14:   bandEdgePosPower    -byte 15: bandEdgeNegPower
*  -byte 16-19: IFNomRate     - byte 20-23: baudRateDetectedInHz   - byte 24-27: rateNomFinal
*
*******************************************************************************/
#define ixHI_ASM_CONFIG_AREA		(BCHP_DFE_BASE_ADDR + (0x0000 << 2))
#define ixHI_J83_CONFIG_AREA		(BCHP_DFE_BASE_ADDR + (0x0020 << 2))
#define ixHI_UNIFIED_COFDM_CONFIG_AREA		(BCHP_DFE_BASE_ADDR + (0x0040 << 2))
#define ixHI_J83_STATUS_AREA		(BCHP_DFE_BASE_ADDR + (0x0060 << 2))
#define ixHI_UNIFIED_COFDM_STATUS_AREA		(BCHP_DFE_BASE_ADDR + (0x0080 << 2))

#define ixHI_VSB_CONFIG_AREA		(BCHP_DFE_BASE_ADDR + (0x0100 << 2))
#define ixHI_VSB_STATUS_AREA		(BCHP_DFE_BASE_ADDR + (0x0120 << 2))

/*******************************************************************************
* Service Results Areas -  X230: xdata absolute memory addresses 0x00C0 - 0x00Df
*                                     absolute memory addresses A7E0 - A7FF for results 0
*******************************************************************************/
#if BTFE_X230FW
    #define ixHI_SERVICE_RESULTS_0	(BCHP_DFE_BASE_ADDR + (0x01C0 << 2))
    #define HI_SVC_RES_0_LENGTH	64
#else
    #define ixHI_SERVICE_RESULTS_0	(BCHP_DFE_BASE_ADDR + (0xA7E0 << 2))
    #define HI_SVC_RES_0_LENGTH	32
#endif


/*******************************************************************************
* Service Results Areas - absolute memory addresses A800 - ABFF for results 1 (PSD)
*******************************************************************************/
#define ixHI_SERVICE_RESULTS_1	(BCHP_DFE_BASE_ADDR + (0xA800<< 2))
#define		HI_SVC_RES_1_LENGTH	1024

/*******************************************************************************
* Service Results Areas - absolute memory addresses C000 - for results 2 (VSB taps)
*******************************************************************************/
#define ixHI_SERVICE_RESULTS_2	(BCHP_DFE_BASE_ADDR + (0xC000<< 2))
#define		HI_SVC_RES_2_LENGTH		/* length is variable */

/*******************************************************************************
* Active Tracking Sub Commands
*******************************************************************************/
#define ATS_RETURN_STATUS				(0)
#define ATS_AGC_CONFIG_FAT   			(1)
#define ATS_AGC_CONFIG_FDC 				(2)
#define ATS_AGC_THRES_DIG_FAT_IF 		(3)
#define ATS_AGC_THRES_DIG_FAT_RF 		(4)
#define ATS_AGC_THRES_ANA_FAT_IF		(5)
#define ATS_AGC_THRES_ANA_FAT_RF 		(6)
#define ATS_AGC_FDC						(7)
#define ATS_CONFIG_SLOW_AGC_BW			(8)
#define ATS_AGC_DATA_FAST_AND_SLOW_VSB	(9)
#define ATS_AGC_DATA_FAST_AND_SLOW_QAM	(10)
#define ATS_AGC_DATA_FAST_AND_SLOW_QAM256	(11)

#define MIN_AGC_CONFIG_REG_INDEX        (12)
#define ATS_AGC_DATA_8VSB 				(12)
#define ATS_AGC_DATA_8VSB_ADJ_PLUS		(13)
#define ATS_AGC_DATA_8VSB_ADJ_MINUS		(14)
#define ATS_AGC_DATA_64QAM				(15)
#define	ATS_AGC_DATA_256QAM				(16)
#define ATS_AGC_DATA_NTSC				(17) /* unused? */
#define ATS_AGC_DATA_NTSC_TRACK			(18) /* unused? */
#define ATS_AGC_DATA_NTSC_NO_LOCK_TRACK	(19) /* unused? */
#define ATS_AGC_DATA_NTSC_FAST			(20)
#define ATS_AGC_DATA_NTSC_SLOW			(21)
#define ATS_AGC_DATA_NTSC_STOP			(22) /* unused? */
#define ATS_AGC_DATA_NTSC_AM_HUM		(23) /* unused? */
#define ATS_AGC_DATA_NTSC_MULTIPATH		(24) /* unused? */
#define ATS_AGC_DATA_FDC_INITIAL		(25)
#define ATS_AGC_DATA_FDC_FINAL			(26)
#define ATS_AGC_DATA_PAL_FAST           (27)
#define ATS_AGC_DATA_SECAM_FAST         (28)
#define ATS_AGC_DATA_FAST_VSB			(29)
#define ATS_AGC_DATA_SLOW_VSB   		(30)
#define ATS_AGC_DATA_FAST_QAM64 		(31)
#define ATS_AGC_DATA_SLOW_QAM64  		(32)
#define ATS_AGC_DATA_FAST_QAM256		(33)
#define ATS_AGC_DATA_SLOW_QAM256		(34)
#define ATS_AGC_SLOW_WAIT_CONFIG_FAT	(35)
#define ATS_AGC_FAST_WAIT_CONFIG_FAT	(36)
#define MAX_AGC_CONFIG_REG_INDEX        (36)

/*******************************************************************************
* Active Tracking Sub Commands Config FAT and FDC
*******************************************************************************/
#define AT_AGC_STOP						(0)
#define AT_AGC_RUNONCE					(1)
#define AT_AGC_START					(2)

/*******************************************************************************
* Acquisition Status Service Sub Commands
*******************************************************************************/
#define ACQ_STATUS_INDX_VSB			0
#define ACQ_STATUS_INDX_64QAM		1
#define ACQ_STATUS_INDX_256QAM		2
#define ACQ_STATUS_INDX_J83			3
#define ACQ_STATUS_INDX_NTSC		4
#define ACQ_STATUS_INDX_COFDM		5
#define ACQ_STATUS_INDX_UCOFDM		6


/******************************************************************************
 * Smart Antenna commands HI_SERVICE_PARAMS_0
 ******************************************************************************/
#define NXTENNA_STATUS_READ		0
#define NXTENNA_OPERATION_WRITE	1
#define NXTENNA_CONFIG_WRITE	2
#define NXTENNA_SIG_STR_READ	0x10
#define NXTENNA_CHAN_QUAL_READ	0x20

/******************************************************************************
 * Smart Antenna Read-Write sub-commands HI_SERVICE_PARAMS_1
 *****************************************************************************/
#define NXTNA_DISABLE_DRIVER	0
#define NXTNA_ENABLE_DRIVER 	1
#define NXTNA_SET_SETTING		3
#define NXTNA_SEEK_OPTIMUM		4
#define NXTNA_SET_GPIO_PIN		5


/*****************************************************************************
 * Nxtenna Firmware state flags
 ****************************************************************************/
#define NXTENNA_DRIVER_ENABLED		0x01
#define NXTENNA_SEEKING_OPTIMUM		0x02
#define NXTENNA_INITIALIZING		0x08


/*****************************************************************************
 * Trace buffer  - using same locations as in the section below
 *   Only used for IF demod now.  It can be used for other modulation types,
 *   unless it is placed in the interleaver RAM.
 ****************************************************************************/
#define TRACE_BUFFER_LOCATION	(BCHP_DFE_BASE_ADDR + (0x0A00<< 2))
#define TRACE_BUFFER_SIZE	    (BCHP_DFE_BASE_ADDR + (0x0600<< 2))

/* mcg TRACE_BUFFER_STATUS_ADDRESS and ixHI_TRACE_BUFFER_DECIMATION_FACTOR must be odd numbers !!!!! */
#define TRACE_BUFFER_STATUS_ADDRESS				(BCHP_DFE_BASE_ADDR + (0x09FF<< 2))
#define ixHI_TRACE_BUFFER_DECIMATION_FACTOR		(BCHP_DFE_BASE_ADDR + (0x09FD<< 2))

/*****************************************************************************
 * UAGC Firmware Variables
 *   The UAGC configuration registers are loaded from tables which can be
 *    different for each analog front end.  The following are some additional
 *    registers to load variable which are processed by UAGC firmware.
 ****************************************************************************/

#define ixHI_UAGC_B_0_3_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb610<< 2))
#define ixHI_UAGC_B_0_2_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb611<< 2))
#define ixHI_UAGC_B_0_1_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb612<< 2))
#define ixHI_UAGC_B_0_0_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb613<< 2))
#define ixHI_UAGC_B_1_3_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb614<< 2))
#define ixHI_UAGC_B_1_2_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb615<< 2))
#define ixHI_UAGC_B_1_1_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb616<< 2))
#define ixHI_UAGC_B_1_0_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb617<< 2))
#define ixHI_UAGC_B_2_3_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb618<< 2))
#define ixHI_UAGC_B_2_2_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb619<< 2))
#define ixHI_UAGC_B_2_1_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb61a<< 2))
#define ixHI_UAGC_B_2_0_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb61b<< 2))
#define ixHI_UAGC_B_3_3_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb61c<< 2))
#define ixHI_UAGC_B_3_2_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb61d<< 2))
#define ixHI_UAGC_B_3_1_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb61e<< 2))
#define ixHI_UAGC_B_3_0_SPEED_A (BCHP_DFE_BASE_ADDR + (0xb61f<< 2))

#define ixHI_UAGC_B_0_3_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb620<< 2))
#define ixHI_UAGC_B_0_2_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb621<< 2))
#define ixHI_UAGC_B_0_1_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb622<< 2))
#define ixHI_UAGC_B_0_0_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb623<< 2))
#define ixHI_UAGC_B_1_3_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb624<< 2))
#define ixHI_UAGC_B_1_2_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb625<< 2))
#define ixHI_UAGC_B_1_1_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb626<< 2))
#define ixHI_UAGC_B_1_0_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb627<< 2))
#define ixHI_UAGC_B_2_3_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb628<< 2))
#define ixHI_UAGC_B_2_2_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb629<< 2))
#define ixHI_UAGC_B_2_1_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb62a<< 2))
#define ixHI_UAGC_B_2_0_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb62b<< 2))
#define ixHI_UAGC_B_3_3_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb62c<< 2))
#define ixHI_UAGC_B_3_2_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb62d<< 2))
#define ixHI_UAGC_B_3_1_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb62e<< 2))
#define ixHI_UAGC_B_3_0_SPEED_B (BCHP_DFE_BASE_ADDR + (0xb62f<< 2))

#define ixHI_UAGC_B_0_3_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb630<< 2))
#define ixHI_UAGC_B_0_2_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb631<< 2))
#define ixHI_UAGC_B_0_1_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb632<< 2))
#define ixHI_UAGC_B_0_0_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb633<< 2))
#define ixHI_UAGC_B_1_3_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb634<< 2))
#define ixHI_UAGC_B_1_2_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb635<< 2))
#define ixHI_UAGC_B_1_1_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb636<< 2))
#define ixHI_UAGC_B_1_0_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb637<< 2))
#define ixHI_UAGC_B_2_3_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb638<< 2))
#define ixHI_UAGC_B_2_2_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb639<< 2))
#define ixHI_UAGC_B_2_1_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb63a<< 2))
#define ixHI_UAGC_B_2_0_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb63b<< 2))
#define ixHI_UAGC_B_3_3_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb63c<< 2))
#define ixHI_UAGC_B_3_2_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb63d<< 2))
#define ixHI_UAGC_B_3_1_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb63e<< 2))
#define ixHI_UAGC_B_3_0_SPEED_C (BCHP_DFE_BASE_ADDR + (0xb63f<< 2))

#define ixHI_UAGC_B_0_3_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb640<< 2))
#define ixHI_UAGC_B_0_2_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb641<< 2))
#define ixHI_UAGC_B_0_1_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb642<< 2))
#define ixHI_UAGC_B_0_0_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb643<< 2))
#define ixHI_UAGC_B_1_3_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb644<< 2))
#define ixHI_UAGC_B_1_2_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb645<< 2))
#define ixHI_UAGC_B_1_1_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb646<< 2))
#define ixHI_UAGC_B_1_0_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb647<< 2))
#define ixHI_UAGC_B_2_3_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb648<< 2))
#define ixHI_UAGC_B_2_2_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb649<< 2))
#define ixHI_UAGC_B_2_1_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb64a<< 2))
#define ixHI_UAGC_B_2_0_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb64b<< 2))
#define ixHI_UAGC_B_3_3_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb64c<< 2))
#define ixHI_UAGC_B_3_2_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb64d<< 2))
#define ixHI_UAGC_B_3_1_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb64e<< 2))
#define ixHI_UAGC_B_3_0_SPEED_D (BCHP_DFE_BASE_ADDR + (0xb64f<< 2))

/* Tuner frequency supplied to IFD FW for channel scan  U[30:15] */
#define ixHI_TUNER_FREQUENCY_HZ_1 (BCHP_DFE_BASE_ADDR + (0xb650<< 2))
#define ixHI_TUNER_FREQUENCY_HZ_0 (BCHP_DFE_BASE_ADDR + (0xb651<< 2))

#else /*else if not USE_DFE_ADDR_32BIT*/

#define ixHI_UAGC_VIF_SETPOINT_OFFSET4_1 0xb650
#define ixHI_UAGC_VIF_SETPOINT_OFFSET4_0 0xb651


/*******************************************************************************
* BE-AGC Board specific variables - These are loaded via AGC scripts,
*  then read by the 8051 firmware, then they could be used again
*   ixHI_UAGC_SETPOINT_1    for writes from MIPS to 8051
*******************************************************************************/
#define ixHI_BEAGC_GAIN_NOMINAL_1	ixUC_GP_57
#define ixHI_BEAGC_GAIN_NOMINAL_0	ixUC_GP_58


/*******************************************************************************
* NTSC tuner specific variables - These are loaded via AGC table,
*  then used by the 8051 firmware when needed
*******************************************************************************/
#define ixHI_ANALOG_FREQ_RESPONSE	ixUC_GP_49
#define ixHI_ANALOG_DAC_GAIN		ixUC_GP_50

/*******************************************************************************
* 4 channel by 16 bit trace function
*
*******************************************************************************/
#define ixHI_TRACE_SELECT_CH1		ixUC_GP_53
#define ixHI_TRACE_SELECT_CH2		ixUC_GP_54
#define ixHI_TRACE_SELECT_CH3		ixUC_GP_55
#define ixHI_TRACE_SELECT_CH4		ixUC_GP_56

/*******************************************************************************
* 16 bit port for host access to configure analog demod firmware.
*  This is being implemented for the following reasons:
*   1. We need the analog demodulator firmware to be configured in the
*       field while we are tuned to a signal
*   2. There are many 16 bit variables in the analog demod firmware.
*   3. This could be slow since the analog demod firmware only runs
*       once every 3 msecs, but if the configuration is done using
*       the SCD GUI serial 115.2kbps link, 3msec is fast enough
*   4. There are problems with 16 bit accesses of hardware registers
*       in Pyro (X244), so forcing the host to poll a ready bit
*       may be necessary anyway.  At any rate, if a separate pair
*       of GP 8-bit registers are used to load a firmware parameter,
*       there would be no guarantee, the firmware would get the
*       complete 16 bit value.
*        Some notes on register access problem.
*         e.g. Reads
*          A. Host begins reads 2 byte HW register - byte 1 goes in temp reg
*          B. 8051 reads any HW register in same register block - byte 1
*                above is overwritten.
*          C. Host completes read of 2 byte HW register - byte 1 and byte 2
*                are concatenated, but byte 1 is wrong
*        e.g. Writes - same type of thing, but generally host won't be
*                writing these registers, when 8051 is running
*
*  Host should poll ready bit before writing address and data
*   e.g.
*
*
*  Details of which address configures which variable will be in ntsc.c
*******************************************************************************/
#define ixHI_PORT16_ADDRESS			ixUC_GP_44
#define ixHI_PORT16_DATA_1			ixUC_GP_45
#define ixHI_PORT16_DATA_0			ixUC_GP_46
#define ixHI_PORT16_READY			ixUC_GP_47
#define	 HI_PORT16_READY__host2firmware__SIZE  1
#define	 HI_PORT16_READY__host2firmware__MASK  0x01
#define	 HI_PORT16_READY__host2firmware__SHIFT 0
#define	 HI_PORT16_READY__firmware2host__SIZE  1
#define	 HI_PORT16_READY__firmware2host__MASK  0x02
#define	 HI_PORT16_READY__firmware2host__SHIFT 1


/*******************************************************************************
* UAGC Status Registers
*
*******************************************************************************/
#define ixHI_UAGC_PVID_DBM_1		ixUC_GP_38
#define ixHI_UAGC_PVID_DBM_0		ixUC_GP_39
#define ixHI_UAGC_SNRVID_DB_1		ixUC_GP_40
#define ixHI_UAGC_SNRVID_DB_0		ixUC_GP_41
#define ixHI_UAGC_DUR_DB			ixUC_GP_59













/*******************************************************************************
* ASM Configuation data: write by SCD, and read by 8051 ASMs
* each config data hold for 32 bytes
*
* ixHI_ASM_CONFIG_AREA:
*  byte 0:     acquire_t acquireConfig;          0 - Directed Acquire, 1 - Full Acquire, 2-Search/Scan
*  byte 1:     bandwidth_t bandwidthConfig;  0 - 6 MHz, 1 - 7 MHz, 2 - 8 MHz
*  byte 2:     spectral_t spectralConfig;   0 - non-inverted, 1 - inverted
*  byte 3-6:  uint32 defaultIFNomRate;  Nominal Rate of default IF center frequency in upper 3 bytes and lower byte 0x00.
*
* ixHI_J83_CONFIG_AREA
*  byte 0:      j 83abcMode_t j83abcMode; 0 - J83A, 1 - J83B, 2 - J83C
*  byte 1:       qamModeDigital_t qamMode; 0 - 16QAM, 1-32QAM, 2-64QAM, 3-128QAM, 4-256QAM
*  byte 2-5:    uint32 symbolRate;
*  byte 6-11:  uchar timingNominalRate[6];
*
* ixHI_UNIFIED_COFDM_CONFIG_AREA
*  -byte 0:    ofdmStandard          -byte 1:  CCIEnable		 -byte 2:    ACIEnable	    -byte 3: mobileMode
*  -byte 4:    StreamPriority        -byte 5:  carrierRange	     -byte 6:    impulse 	    -byte 7: rsLayer
*  -byte 8:    modeGuardMode         -byte 9:  mode              -byte 10:   guard          -byte 11: tpsMode
*  -byte 12:   codeLP                -byte 13: codeHP            -byte 14:   hierarchy      -byte 15: moduation
*  -byte 16:   modulationLayerA      -byte 17:  modulationLayerB -byte 18: modulationLayerC -byte 19: codeRateLayerA
*  -byte 20:   codeRateLayerB		 -byte 21:  codeRateLayerC   -byte 22: segmentLayerA    -byte 23: segmentLayerB
*  -byte 24:   segmentLayerC         -byte 25:  timeInterleaveLayerA  -byte 26: timeInterleaveLayerB  -byte 27: timeInterleaveLayerC
*  -byte 28:   partialReception
*
* ixHI_J83_STATUS_AREA
*  -byte 0:    reacqCounter     -byte 1:  acquireConfig    -byte 2:    bandwidthConfig	    -byte 3: spectralConfig
*  -byte 4:    j83abcMode        -byte 5:  qamMode	     -byte 6-11: timingNominalRate[6]
*  -byte 12:   operationDone   -byte 13: signalDetected  -byte 14:   bandEdgePosPower    -byte 15: bandEdgeNegPower
*  -byte 16-19: IFNomRate     - byte 20-23: baudRateDetectedInHz   - byte 24-27: rateNomFinal
*
*******************************************************************************/
#define ixHI_ASM_CONFIG_AREA		0x0000
#define ixHI_J83_CONFIG_AREA		0x0020
#define ixHI_UNIFIED_COFDM_CONFIG_AREA		0x0040
#define ixHI_J83_STATUS_AREA		0x0060
#define ixHI_UNIFIED_COFDM_STATUS_AREA		0x0080

#define ixHI_VSB_CONFIG_AREA		0x0100
#define ixHI_VSB_STATUS_AREA		0x0120

/*******************************************************************************
* Service Results Areas -  X230: xdata absolute memory addresses 0x00C0 - 0x00Df
*                                     absolute memory addresses A7E0 - A7FF for results 0
*******************************************************************************/
#if BTFE_X230FW
    #define ixHI_SERVICE_RESULTS_0	0x01C0
    #define HI_SVC_RES_0_LENGTH	64
#else
    #define ixHI_SERVICE_RESULTS_0	0xA7E0
    #define HI_SVC_RES_0_LENGTH	32
#endif


/*******************************************************************************
* Service Results Areas - absolute memory addresses A800 - ABFF for results 1 (PSD)
*******************************************************************************/
#define ixHI_SERVICE_RESULTS_1	0xA800
#define		HI_SVC_RES_1_LENGTH	1024

/*******************************************************************************
* Service Results Areas - absolute memory addresses C000 - for results 2 (VSB taps)
*******************************************************************************/
#define ixHI_SERVICE_RESULTS_2	0xC000
#define		HI_SVC_RES_2_LENGTH		/* length is variable */

/*******************************************************************************
* Active Tracking Sub Commands
*******************************************************************************/
#define ATS_RETURN_STATUS				(0)
#define ATS_AGC_CONFIG_FAT   			(1)
#define ATS_AGC_CONFIG_FDC 				(2)
#define ATS_AGC_THRES_DIG_FAT_IF 		(3)
#define ATS_AGC_THRES_DIG_FAT_RF 		(4)
#define ATS_AGC_THRES_ANA_FAT_IF		(5)
#define ATS_AGC_THRES_ANA_FAT_RF 		(6)
#define ATS_AGC_FDC						(7)
#define ATS_CONFIG_SLOW_AGC_BW			(8)
#define ATS_AGC_DATA_FAST_AND_SLOW_VSB	(9)
#define ATS_AGC_DATA_FAST_AND_SLOW_QAM	(10)
#define ATS_AGC_DATA_FAST_AND_SLOW_QAM256	(11)

#define MIN_AGC_CONFIG_REG_INDEX        (12)
#define ATS_AGC_DATA_8VSB 				(12)
#define ATS_AGC_DATA_8VSB_ADJ_PLUS		(13)
#define ATS_AGC_DATA_8VSB_ADJ_MINUS		(14)
#define ATS_AGC_DATA_64QAM				(15)
#define	ATS_AGC_DATA_256QAM				(16)
#define ATS_AGC_DATA_NTSC				(17) /* unused? */
#define ATS_AGC_DATA_NTSC_TRACK			(18) /* unused? */
#define ATS_AGC_DATA_NTSC_NO_LOCK_TRACK	(19) /* unused? */
#define ATS_AGC_DATA_NTSC_FAST			(20)
#define ATS_AGC_DATA_NTSC_SLOW			(21)
#define ATS_AGC_DATA_NTSC_STOP			(22) /* unused? */
#define ATS_AGC_DATA_NTSC_AM_HUM		(23) /* unused? */
#define ATS_AGC_DATA_NTSC_MULTIPATH		(24) /* unused? */
#define ATS_AGC_DATA_FDC_INITIAL		(25)
#define ATS_AGC_DATA_FDC_FINAL			(26)
#define ATS_AGC_DATA_PAL_FAST           (27)
#define ATS_AGC_DATA_SECAM_FAST         (28)
#define ATS_AGC_DATA_FAST_VSB			(29)
#define ATS_AGC_DATA_SLOW_VSB   		(30)
#define ATS_AGC_DATA_FAST_QAM64 		(31)
#define ATS_AGC_DATA_SLOW_QAM64  		(32)
#define ATS_AGC_DATA_FAST_QAM256		(33)
#define ATS_AGC_DATA_SLOW_QAM256		(34)
#define ATS_AGC_SLOW_WAIT_CONFIG_FAT	(35)
#define ATS_AGC_FAST_WAIT_CONFIG_FAT	(36)
#define MAX_AGC_CONFIG_REG_INDEX        (36)

/*******************************************************************************
* Active Tracking Sub Commands Config FAT and FDC
*******************************************************************************/
#define AT_AGC_STOP						(0)
#define AT_AGC_RUNONCE					(1)
#define AT_AGC_START					(2)

/*******************************************************************************
* Acquisition Status Service Sub Commands
*******************************************************************************/
#define ACQ_STATUS_INDX_VSB			0
#define ACQ_STATUS_INDX_64QAM		1
#define ACQ_STATUS_INDX_256QAM		2
#define ACQ_STATUS_INDX_J83			3
#define ACQ_STATUS_INDX_NTSC		4
#define ACQ_STATUS_INDX_COFDM		5
#define ACQ_STATUS_INDX_UCOFDM		6


/******************************************************************************
 * Smart Antenna commands HI_SERVICE_PARAMS_0
 ******************************************************************************/
#define NXTENNA_STATUS_READ		0
#define NXTENNA_OPERATION_WRITE	1
#define NXTENNA_CONFIG_WRITE	2
#define NXTENNA_SIG_STR_READ	0x10
#define NXTENNA_CHAN_QUAL_READ	0x20

/******************************************************************************
 * Smart Antenna Read-Write sub-commands HI_SERVICE_PARAMS_1
 *****************************************************************************/
#define NXTNA_DISABLE_DRIVER	0
#define NXTNA_ENABLE_DRIVER 	1
#define NXTNA_SET_SETTING		3
#define NXTNA_SEEK_OPTIMUM		4
#define NXTNA_SET_GPIO_PIN		5


/*****************************************************************************
 * Nxtenna Firmware state flags
 ****************************************************************************/
#define NXTENNA_DRIVER_ENABLED		0x01
#define NXTENNA_SEEKING_OPTIMUM		0x02
#define NXTENNA_INITIALIZING		0x08


/*****************************************************************************
 * Trace buffer  - using same locations as in the section below
 *   Only used for IF demod now.  It can be used for other modulation types,
 *   unless it is placed in the interleaver RAM.
 ****************************************************************************/
#define TRACE_BUFFER_LOCATION	0x0A00
#define TRACE_BUFFER_SIZE	    0x0600

/* mcg TRACE_BUFFER_STATUS_ADDRESS and ixHI_TRACE_BUFFER_DECIMATION_FACTOR must be odd numbers !!!!! */
#define TRACE_BUFFER_STATUS_ADDRESS		0x09FF
#define ixHI_TRACE_BUFFER_DECIMATION_FACTOR	0x09FD

/*****************************************************************************
 * UAGC Firmware Variables
 *   The UAGC configuration registers are loaded from tables which can be
 *    different for each analog front end.  The following are some additional
 *    registers to load variable which are processed by UAGC firmware.
 ****************************************************************************/

#define ixHI_UAGC_B_0_3_SPEED_A 0xb610
#define ixHI_UAGC_B_0_2_SPEED_A 0xb611
#define ixHI_UAGC_B_0_1_SPEED_A 0xb612
#define ixHI_UAGC_B_0_0_SPEED_A 0xb613
#define ixHI_UAGC_B_1_3_SPEED_A 0xb614
#define ixHI_UAGC_B_1_2_SPEED_A 0xb615
#define ixHI_UAGC_B_1_1_SPEED_A 0xb616
#define ixHI_UAGC_B_1_0_SPEED_A 0xb617
#define ixHI_UAGC_B_2_3_SPEED_A 0xb618
#define ixHI_UAGC_B_2_2_SPEED_A 0xb619
#define ixHI_UAGC_B_2_1_SPEED_A 0xb61a
#define ixHI_UAGC_B_2_0_SPEED_A 0xb61b
#define ixHI_UAGC_B_3_3_SPEED_A 0xb61c
#define ixHI_UAGC_B_3_2_SPEED_A 0xb61d
#define ixHI_UAGC_B_3_1_SPEED_A 0xb61e
#define ixHI_UAGC_B_3_0_SPEED_A 0xb61f

#define ixHI_UAGC_B_0_3_SPEED_B 0xb620
#define ixHI_UAGC_B_0_2_SPEED_B 0xb621
#define ixHI_UAGC_B_0_1_SPEED_B 0xb622
#define ixHI_UAGC_B_0_0_SPEED_B 0xb623
#define ixHI_UAGC_B_1_3_SPEED_B 0xb624
#define ixHI_UAGC_B_1_2_SPEED_B 0xb625
#define ixHI_UAGC_B_1_1_SPEED_B 0xb626
#define ixHI_UAGC_B_1_0_SPEED_B 0xb627
#define ixHI_UAGC_B_2_3_SPEED_B 0xb628
#define ixHI_UAGC_B_2_2_SPEED_B 0xb629
#define ixHI_UAGC_B_2_1_SPEED_B 0xb62a
#define ixHI_UAGC_B_2_0_SPEED_B 0xb62b
#define ixHI_UAGC_B_3_3_SPEED_B 0xb62c
#define ixHI_UAGC_B_3_2_SPEED_B 0xb62d
#define ixHI_UAGC_B_3_1_SPEED_B 0xb62e
#define ixHI_UAGC_B_3_0_SPEED_B 0xb62f

#define ixHI_UAGC_B_0_3_SPEED_C 0xb630
#define ixHI_UAGC_B_0_2_SPEED_C 0xb631
#define ixHI_UAGC_B_0_1_SPEED_C 0xb632
#define ixHI_UAGC_B_0_0_SPEED_C 0xb633
#define ixHI_UAGC_B_1_3_SPEED_C 0xb634
#define ixHI_UAGC_B_1_2_SPEED_C 0xb635
#define ixHI_UAGC_B_1_1_SPEED_C 0xb636
#define ixHI_UAGC_B_1_0_SPEED_C 0xb637
#define ixHI_UAGC_B_2_3_SPEED_C 0xb638
#define ixHI_UAGC_B_2_2_SPEED_C 0xb639
#define ixHI_UAGC_B_2_1_SPEED_C 0xb63a
#define ixHI_UAGC_B_2_0_SPEED_C 0xb63b
#define ixHI_UAGC_B_3_3_SPEED_C 0xb63c
#define ixHI_UAGC_B_3_2_SPEED_C 0xb63d
#define ixHI_UAGC_B_3_1_SPEED_C 0xb63e
#define ixHI_UAGC_B_3_0_SPEED_C 0xb63f

#define ixHI_UAGC_B_0_3_SPEED_D 0xb640
#define ixHI_UAGC_B_0_2_SPEED_D 0xb641
#define ixHI_UAGC_B_0_1_SPEED_D 0xb642
#define ixHI_UAGC_B_0_0_SPEED_D 0xb643
#define ixHI_UAGC_B_1_3_SPEED_D 0xb644
#define ixHI_UAGC_B_1_2_SPEED_D 0xb645
#define ixHI_UAGC_B_1_1_SPEED_D 0xb646
#define ixHI_UAGC_B_1_0_SPEED_D 0xb647
#define ixHI_UAGC_B_2_3_SPEED_D 0xb648
#define ixHI_UAGC_B_2_2_SPEED_D 0xb649
#define ixHI_UAGC_B_2_1_SPEED_D 0xb64a
#define ixHI_UAGC_B_2_0_SPEED_D 0xb64b
#define ixHI_UAGC_B_3_3_SPEED_D 0xb64c
#define ixHI_UAGC_B_3_2_SPEED_D 0xb64d
#define ixHI_UAGC_B_3_1_SPEED_D 0xb64e
#define ixHI_UAGC_B_3_0_SPEED_D 0xb64f

/* Tuner frequency supplied to IFD FW for channel scan  U[30:15] */
#define ixHI_TUNER_FREQUENCY_HZ_1 0xb650
#define ixHI_TUNER_FREQUENCY_HZ_0 0xb651

#endif /*end if USE_DFE_ADDR_32BIT*/

#endif
