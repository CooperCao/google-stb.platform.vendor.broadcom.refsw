/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BHDM_PRIV_H__
#define BHDM_PRIV_H__

#include "bstd.h"

#include "blst_queue.h"

#include "../src/common/bhdm_config.h"

#include "bhdm_hdcp.h"
#include "bhdm_edid.h"
#include "bhdm_tmds_priv.h"
#include "bhdm_packet_acr_priv.h"
#if BHDM_CONFIG_BTMR_SUPPORT
#include "bhdm_monitor.h"
#endif


#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "breg_i2c.h"   /* I2C */


#include "bchp.h"       /* Chip Info */
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"

#include "bchp_hdmi.h"
#include "bchp_hdmi_rm.h"  /* HDMI Rate Manager */
#include "bchp_hdmi_ram.h" /* HDMI Packet RAM */

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if BHDM_CONFIG_40NM_SUPPORT \
||      BHDM_CONFIG_28NM_SUPPORT
#include "bchp_aon_hdmi_tx.h"
#include "bchp_hdmi_tx_intr2.h"
#include "bchp_int_id_hdmi_tx_intr2.h"
#ifdef BCHP_AON_PM_L2_REG_START
#include "bchp_int_id_aon_pm_l2.h"
#include "bchp_int_id_aon_l2.h"
#endif
#else
#include "bchp_hdmi_intr2.h"
#include "bchp_int_id_hdmi_intr2.h"
#endif

#if BHDM_CONFIG_65NM_SUPPORT || BHDM_CONFIG_40NM_SUPPORT || BHDM_CONFIG_28NM_SUPPORT
#include "bchp_hdmi_tx_phy.h"
#endif

#include "bchp_dvp_ht.h"

#if BHDM_HAS_MULTIPLE_PORTS
#include "bchp_dvp_ht_1.h"
#include "bchp_int_id_hdmi_tx_intr2_1.h"

#endif

#if BHDM_CONFIG_HAS_HDCP22
#include "bhdm_auto_i2c.h"
#include "bhdm_auto_i2c_priv.h"
#include "bhdm_scdc.h"

#ifdef BCHP_BSC_REG_START
#include "bchp_bsc.h"
#include "bchp_i2c_rdb_remap.h"
#endif

#ifdef BCHP_BSCA_REG_START
#include "bchp_bsca.h"
#endif
#include "bchp_hdmi_tx_auto_i2c.h"
#include "bchp_int_id_hdmi_tx_hae_intr2_0.h"
#include "bchp_int_id_hdmi_tx_scdc_intr2_0.h"
#endif

#if BHDM_OVERRIDE_DBG_MSG_LEVEL
#undef BDBG_MSG
#define BDBG_MSG(format) BDBG_P_PRINTMSG(BDBG_eMsg, format)
#undef BDBG_WRN
#define BDBG_WRN(format) BDBG_P_PRINTMSG(BDBG_eWrn, format)
#endif

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(HDMI);


#define BHDM_CHECK_RC( rc, func )                 \
do                                                \
{                                                                                         \
        if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
        {                                                                                     \
                goto done;                                                            \
        }                                                                                     \
} while(0)


/* Get the offset of two groups of register. */
#define BHDM_P_GET_REG_OFFSET(eCoreId, ulPriReg, ulSecReg) \
        ((BAVC_HDMI_CoreId_e0 == (eCoreId)) ? 0 : ((ulSecReg) - (ulPriReg)))

typedef enum BHDM_P_HdmCoreId
{
        BHDM_P_eHdmCoreId0 = 0,
        BHDM_P_eHdmCoreId1,
        BHDM_P_eHdmCoreIdMax
} BHDM_P_HdmCoreId ;


#define MAKE_INTR_ENUM(IntName) BHDM_INTR_e##IntName
#define MAKE_INTR_NAME(IntName) "BHDM_" #IntName
#define MAKE_HAE_INTR_ENUM(IntName) BHDM_HAE_INTR_e##IntName

/******************************************************************************
Summary:
Enumeration of BHDM_Interrupts
*******************************************************************************/
typedef enum
{
#if BHDM_CONFIG_DUAL_HPD_SUPPORT
        /* 00 */ MAKE_INTR_ENUM(HOTPLUG_REMOVED),
        /* 01 */ MAKE_INTR_ENUM(HOTPLUG_CONNECTED),
#else
        /* 00 */ MAKE_INTR_ENUM(HOTPLUG),
#endif
        /* 01 */ MAKE_INTR_ENUM(DF_FULL_MINUS),
        /* 02 */ MAKE_INTR_ENUM(DF_ALMOST_FULL),
        /* 03 */ MAKE_INTR_ENUM(DF_EMPTY_MINUS),
        /* 04 */ MAKE_INTR_ENUM(DF_ALMOST_EMPTY),

        /* 05 */ MAKE_INTR_ENUM(PKT_WRITE_ERR),

        /* 07 */ MAKE_INTR_ENUM(HDCP_REPEATER_ERR),
        /* 08 */ MAKE_INTR_ENUM(HDCP_V_MISMATCH),
        /* 09 */ MAKE_INTR_ENUM(HDCP_V_MATCH),
        /* 10 */ MAKE_INTR_ENUM(HDCP_RI),
        /* 11 */ MAKE_INTR_ENUM(HDCP_AN),
        /* 12 */ MAKE_INTR_ENUM(PKT_OVERFLOW),
        /* 13 */ MAKE_INTR_ENUM(HDCP_PJ),
        /* 14 */ MAKE_INTR_ENUM(MAI_FORMAT_UPDATE),

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
        /* 15 */ MAKE_INTR_ENUM(HDCP_PJ_MISMATCH),
        /* 16 */ MAKE_INTR_ENUM(HDCP_RI_A_MISMATCH),
        /* 17 */ MAKE_INTR_ENUM(HDCP_RI_B_MISMATCH),
#endif

#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT
        /* 18 */ MAKE_INTR_ENUM(RSEN),
#endif

        /* 19 */ MAKE_INTR_ENUM(LAST)
} BHDM_P_InterruptMask ;


#if BHDM_HAS_HDMI_20_SUPPORT
typedef enum
{
        /* 00 */ MAKE_HAE_INTR_ENUM(OK_TO_ENC_EN),
        /* 01 */ MAKE_HAE_INTR_ENUM(REAUTH_REQ),
        /* 02 */ MAKE_HAE_INTR_ENUM(LAST)
} BHDM_P_HAE_InterruptMask;
#endif


#if 0
#define BHDM_PixelClock_eUnused                 BHDM_PixelClock_eCount
#define BHDM_PixelClock_eDviClockRate   BHDM_PixelClock_e54
#endif

typedef struct BHDM_EDID_P_VideoDescriptor
{
        BLST_Q_ENTRY(BHDM_EDID_P_VideoDescriptor ) link ;
        BFMT_VideoFmt eVideoFmt  ; /* BCM Video Format */
        uint8_t VideoIdCode ;      /* CEA-861B Video Id Code */
        uint8_t NativeFormat ;     /* Native Format for Monitor */
        uint8_t nthDescriptor ; /* position of the descriptor in the EDID Video Data Block */
} BHDM_EDID_P_VideoDescriptor ;


/* declaration of the head type for Video Descriptor list */
typedef struct BHDM_EDID_VideoDescriptorHead BHDM_EDID_VideoDescriptorHead;
BLST_Q_HEAD(BHDM_EDID_VideoDescriptorHead, BHDM_EDID_P_VideoDescriptor );

typedef struct BHDM_EDID_P_DetailedTiming
{
	BLST_Q_ENTRY(BHDM_EDID_P_DetailedTiming) link ;
	BHDM_EDID_DetailTiming stDetailedTiming ;

	BFMT_VideoFmt eVideoFmt  ; /* BCM Video Format */
} BHDM_EDID_P_DetailedTiming ;

/* declaration of the head type for Video Descriptor list */
typedef struct BHDM_EDID_DetailedTimingHead BHDM_EDID_DetailedTimingHead ;
BLST_Q_HEAD(BHDM_EDID_DetailedTimingHead, BHDM_EDID_P_DetailedTiming);


/******************************************************************************
Summary:
struct containing Video Format information specified EDID
*******************************************************************************/
typedef struct BHDM_EDID_P_CEA_861B_VIDEO_FORMAT
{
        uint8_t            CeaVideoCode ;      /* Short Descriptor */
        uint16_t           HorizontalPixels ;  /* Horiz Active Pixels */
        uint16_t           VerticalPixels ;       /* Vertical Active Pixels */
        BAVC_ScanType      eScanType ;         /* Progressive, Interlaced */
        BAVC_FrameRateCode eFrameRateCode ;    /* Vertical Frequency */
        BFMT_AspectRatio   eAspectRatio ;      /* Horiz  to Vertical Ratio */
} BHDM_EDID_P_CEA_861B_VIDEO_FORMAT ;


/******************************************************************************
Summary:
Enumeration containing Audio Formats specified in CEA Short Audio Descriptors
*******************************************************************************/
typedef enum BHDM_EDID_P_AudioFormat
{
        BHDM_EDID_P_AudioFormat_eReserved,
        BHDM_EDID_P_AudioFormat_ePCM,
        BHDM_EDID_P_AudioFormat_eAC3,
        BHDM_EDID_P_AudioFormat_eMPEG1,
        BHDM_EDID_P_AudioFormat_eMP3,
        BHDM_EDID_P_AudioFormat_eMPEG2,
        BHDM_EDID_P_AudioFormat_eAAC,
        BHDM_EDID_P_AudioFormat_eDTS,
        BHDM_EDID_P_AudioFormat_eATRAC,
        BHDM_EDID_P_AudioFormat_eOneBit,
        BHDM_EDID_P_AudioFormat_eDDPlus,
        BHDM_EDID_P_AudioFormat_eDTSHD,
        BHDM_EDID_P_AudioFormat_eMATMLP,
        BHDM_EDID_P_AudioFormat_eDST,
        BHDM_EDID_P_AudioFormat_eWMAPro,
        BHDM_EDID_P_AudioFormat_eMaxCount
} BHDM_EDID_P_AudioFormat ;


/******************************************************************************
Summary:
Enumeration containing Sample Rates specified in CEA Short Audio Descriptors
*******************************************************************************/
typedef enum BHDM_EDID_P_AudioSampleRate
{
        BHDM_EDID_P_AudioSampleRate_e32KHz  =  1,
        BHDM_EDID_P_AudioSampleRate_e44KHz  =  2,
        BHDM_EDID_P_AudioSampleRate_e48KHz  =  4,
        BHDM_EDID_P_AudioSampleRate_e88KHz  =  8,
        BHDM_EDID_P_AudioSampleRate_e96KHz  = 16,
        BHDM_EDID_P_AudioSampleRate_e176KHz = 32,
        BHDM_EDID_P_AudioSampleRate_e192KHz = 64
} BHDM_EDID_P_AudioSampleRate;


typedef struct BHDM_EDID_P_AUDIO_FORMATS  {
        BHDM_EDID_P_AudioFormat EdidAudioFormat ;
        BAVC_AudioCompressionStd BcmAudioFormat ;
} BHDM_EDID_P_AUDIO_FORMATS ;

typedef struct BHDM_EDID_P_AUDIO_SAMPLE_RATES {
        BHDM_EDID_P_AudioSampleRate EdidAudioSampleRate ;
        BAVC_AudioSamplingRate BcmAudioSampleRate ;
} BHDM_EDID_P_AUDIO_SAMPLE_RATES ;

#define BHDM_EDID_P_MAX_NUMBER_OF_EDID_BLOCK 8

typedef struct _BHDM_EDID_DATA_
{
        uint8_t Block[BHDM_EDID_BLOCKSIZE] ;
        uint8_t lastBlockRead;

        /* Raw data of EDID block */
        uint8_t CachedBlockRawData[BHDM_EDID_P_MAX_NUMBER_OF_EDID_BLOCK][BHDM_EDID_BLOCKSIZE];

        /* flag to indicate whether the block has been catched - true: cached, false: not cached */
        bool bBlockCached[BHDM_EDID_P_MAX_NUMBER_OF_EDID_BLOCK] ;

        BHDM_EDID_BasicData        BasicData;
        bool BcmMonitorRangeParsed;
        BHDM_EDID_MonitorRange MonitorRange;
        uint8_t                MonitorName[BHDM_EDID_DESC_ASCII_STRING_LEN] ;
        BHDM_EDID_DetailedTimingHead SupportedDetailTimingList;
        uint8_t                SupportedDetailTimingsIn1stBlock ;
        uint8_t                RxHasHdmiSupport;
        BHDM_EDID_RxVendorSpecificDB RxVSDB;
        uint8_t uiRxVendorSpecificDBLength;

        BHDM_EDID_RxHfVsdb      RxHdmiForumVsdb ;

        BHDM_EDID_VideoDescriptorHead   VideoDescriptorList ;
        uint8_t NumBcmSupportedVideoDescriptors;
        uint8_t BcmSupportedVideoIdCodes[BHDM_EDID_MAX_CEA_VIDEO_ID_CODES] ;

        bool First16VideoDescriptorsChecked ;
        uint16_t First16VideoDescriptorsMask;

        uint8_t DescriptorHeader[BHDM_EDID_DESC_HEADER_LEN] ;

        /* keep track of Broadcom Audio/Video Formats supported by the EDID/monitor */
        bool BcmVideoFormatsChecked ;
        bool BcmSupportedVideoFormats[BFMT_VideoFmt_eMaxCount] ;
        bool UnsupportedVideoFormatReported[BFMT_VideoFmt_eMaxCount] ;

        /* keep track of Broadcom 4:2:0 Video Formats supported by the EDID/monitor */
        bool YCbCr420CapabilityMapDBFound ;
        bool YCbCr420CapabilityVideDBFound ;
        bool BcmSupported420VideoFormatsChecked ;
        bool BcmSupported420VideoFormats[BFMT_VideoFmt_eMaxCount] ;
        bool BcmSupported420VideoFormatsReported ;

        bool BcmFormatPreferenceBlockFound ;
        bool BcmSupporteedPreferredFormats[BFMT_VideoFmt_eMaxCount] ;

        BHDM_EDID_3D_Structure_ALL BcmSupported3DStructureAll;
        bool Bcm3DFormatsChecked ;
        BHDM_EDID_3D_Structure_ALL BcmSupported3DFormats[BFMT_VideoFmt_eMaxCount] ;

        bool BcmAudioFormatsChecked ;
        BHDM_EDID_AudioDescriptor BcmSupportedAudioFormats[BAVC_AudioCompressionStd_eMax] ;


        BHDM_EDID_ColorimetryDataBlock ColorimetryDB ;
        uint8_t uiColorimetryDBSupportByte ;
        uint8_t uiColorimetryDBMetaDataSupportByte ;

        BHDM_EDID_VideoCapabilityDataBlock VideoCapabilityDB ;
        BHDM_EDID_HDRStaticDB HdrDB ;

} BHDM_EDID_DATA ;

#if BHDM_CONFIG_BTMR_SUPPORT
/* time units for BTMR which returns microseconds */
#define BHDM_P_MILLISECOND 1000
#define BHDM_P_SECOND 1000000

typedef enum
{
        BHDM_P_TIMER_eHotPlug,  /* hotplug settle time */
        BHDM_P_TIMER_eHotPlugChange,   /* monitor hotplug changes */
        BHDM_P_TIMER_eFormatDetection,
        BHDM_P_TIMER_eMonitorStatus,
        BHDM_P_TIMER_eScdcStatus, /* timer to wait for SCDC status */
        BHDM_P_TIMER_eTxScramble, /* timer to wait before enabling Tx Scrambling */
        BHDM_P_TIMER_ePacketChangeDelay, /* timer to delay transmisson of HDMI Packets after a change */
        BHDM_P_TIMER_eForcedTxHotplug, /* timer act as pulse width when force Tx hotplug_in */
        BHDM_P_TIMER_eHdcp22TxStableLink    /* timer act as the minimum time without REAUTH_REQ to
                                                consider a stable link */
} BHDM_P_TIMER ;


BERR_Code BHDM_P_CreateTimer(const BHDM_Handle hHDM, BTMR_TimerHandle * timerHandle, uint8_t timerId) ;
BERR_Code BHDM_P_DestroyTimer(const BHDM_Handle hHDM, BTMR_TimerHandle *timerHandle, uint8_t timerId) ;
void BHDM_MONITOR_P_CreateTimers(BHDM_Handle hHDMI) ;
void BHDM_MONITOR_P_DestroyTimers(BHDM_Handle hHDMI) ;
void BHDM_P_StopTimers_isr(const BHDM_Handle hHDMI) ;

void BHDM_EDID_P_FreeVideoDescriptorList(BHDM_Handle hHDMI) ;
void BHDM_EDID_P_FreeDetailedTimingList(BHDM_Handle hHDMI) ;
uint8_t BHDM_EDID_P_ValidEdidCheckSum(uint8_t *pEDID) ;

#define BHDM_EDID_P_MAX_EXTENSIONS_SUPPORTED 4
BERR_Code BHDM_EDID_P_VerifyNumBlockExtensions(uint8_t *extensions) ;

void BHDM_EDID_DEBUG_P_PrintEdidBlock(uint8_t * pEDID) ;


#endif

typedef enum BHDM_P_TmdsMode
{
        BHDM_P_TmdsMode_eLow,
        BHDM_P_TmdsMode_eHigh
} BHDM_P_TmdsMode ;


void BHDM_P_Hotplug_isr(BHDM_Handle hHDMI) ;

#if BHDM_CONFIG_HAS_HDCP22
#define hdmiRegBuffSize (BCHP_HDMI_HDCP2TX_CTRL0 - BCHP_HDMI_REG_START)/4 + 1
#define autoI2cRegBuffSize (BCHP_HDMI_TX_AUTO_I2C_HDCP2TX_FSM_DEBUG - BCHP_HDMI_TX_AUTO_I2C_REG_START)/4 + 1
#endif


/* Maximum consecutive Ri/R0 failures before the HPD signal to the Tx is pulsed
NOTE: Attached Rx does not see this HPD signal */
#define BHDM_HDCP_MAX_RI_FAILURE_THRESHOLD 10

/* Pulse width for simulated Tx HPD signal */
#define BHDM_HDCP_FORCE_HPD_PULSE_WIDTH 100 /* in ms */

/* Minimum consecutive Ri matches in order to reset Ri/R0 mismatch counters */
#define BHDM_HDCP_MINIMUM_RI_STABLE_COUNT   5

/* Maximum consecutive REAUTH_REQ from attached Rx/Repeater before the HPD signal
to the Tx is pulsed. NOTE: Attached Rx does not see this HPD signal */
#define BHDM_HDCP_MAX_REAUTH_REQ_THRESHOLD 10

/* Minimum period of time (in milliseconds) without receiving a REAUTH_REQ
    to consider HDCP 2.2 stable link */
#define BHDM_HDCP_MINIMUM_TIME_FOR_HDCP22_STABLE_LINK 10

/*******************************************************************************
Private HDMI Handle Declaration
*******************************************************************************/
typedef struct BHDM_P_Handle
{
        BDBG_OBJECT(HDMI)

        BAVC_HDMI_CoreId eCoreId ;
        uint32_t             ulOffset ;

        BCHP_Handle   hChip ;
        BREG_Handle   hRegister ;
        BINT_Handle   hInterrupt ;
        BREG_I2C_Handle hI2cRegHandle ;
        BINT_CallbackHandle hCallback[MAKE_INTR_ENUM(LAST)] ;

        BKNI_EventHandle BHDM_EventHDCP ;
        BKNI_EventHandle BHDM_EventHDCPRiValue ;
        BKNI_EventHandle BHDM_EventHDCPPjValue ;
        BKNI_EventHandle BHDM_EventHDCPRepeater;
        BKNI_EventHandle BHDM_EventRxSense ;
        BKNI_EventHandle BHDM_EventAvRateChange ;
        BKNI_EventHandle BHDM_EventRAM ;   /* debugging events */
        BKNI_EventHandle BHDM_EventFIFO ;  /* debugging events */
        BHDM_Settings DeviceSettings ;
        bool bForcePacketUpdates ;   /* enables Packet updates when true */
        uint32_t uiPacketRestartMask ;

        BHDM_Status DeviceStatus ;

        /* moved from Device Settings */
        BHDM_P_TmdsClock    eTmdsClock ;
        uint64_t ulPixelClkRate64BitMask ;
        uint32_t ulPixelClockRate ;
        uint32_t ulVertRefreshRate ;

        /* selected output port DVO12/DVO24/HDMI ; set once */
        BHDM_OutputPort eOutputPort ;

        uint8_t RxDeviceAttached ;
        bool bCrcTestMode ; /* ignore HPD signal and power detection when testing crc */
        bool phyPowered ;

        bool AvMuteState ;
        bool AudioMuteState ;
        bool hotplugInterruptFired;
        bool rxSensePowerDetected ;

        uint8_t PacketBytes[BHDM_NUM_PACKET_BYTES] ;


        bool standby; /* true if in standby */
        bool enableWakeup; /* true if standby wakeup from CEC is enabled */


        /******************/
        /* HDCP variables */
        /******************/
        uint32_t HDCP_RiCount ;
        uint8_t HDCP_PjMismatchCount ;

        uint8_t HDCP_AutoRiMismatch_A;
        uint8_t HDCP_AutoRiMismatch_B;
        uint8_t HDCP_AutoPjMismatch;

        uint16_t
                HDCP_Ri2SecsAgo,
                HDCP_Ri4SecsAgo,
                HDCP_Ri6SecsAgo ;

        uint16_t
                HDCP_TxRi,
                HDCP_RxRi ;

        uint8_t
                HDCP_TxPj,
                HDCP_RxPj ;

        uint8_t HDCP_AuthenticatedLink ;

        BHDM_HDCP_Version
                HdcpVersion ;  /* HDCP Version to Use */

        uint8_t RxBCaps ;
        uint16_t RxStatus ;

        /* store copy of Attached KSV and Repeater KSV List */
        uint8_t HDCP_RxKsv[BHDM_HDCP_KSV_LENGTH] ;

        uint8_t HDCP_RepeaterDeviceCount ;
        uint8_t *HDCP_RepeaterKsvList ;

        BHDM_HDCP_OPTIONS HdcpOptions  ;
        bool bHdcpAnRequest ;
        bool bHdcpValidBksv ;
        bool bAutoRiPjCheckingEnabled  ;
        uint8_t AbortHdcpAuthRequest ;

        unsigned hdcpRxBcapsReadError ;


        /******************/
        /* EDID variables */
        /******************/
        BHDM_EDID_DATA AttachedEDID ;

#if BHDM_CONFIG_PLL_KICKSTART_WORKAROUND
        uint32_t uiPllKickStartCount ;
#endif

        BHDM_CallbackFunc pfHotplugChangeCallback ;
        void *pvHotplugChangeParm1 ;
        int iHotplugChangeParm2 ;

#if BHDM_CONFIG_BTMR_SUPPORT
        BTMR_Handle hTMR ;
        BTMR_TimerHandle TimerHotPlug ;  /* delay HP processing */
        BTMR_TimerHandle TimerHotPlugChange ; /* monitor for stable HP */
        BTMR_TimerHandle TimerFormatChange ;  /* monitor for format changes */
        BTMR_TimerHandle TimerPacketChangeDelay ;  /* delay the transmission of packets */
        bool TimerFormatInitialChangeReported ;  /* eliminate first message  */

        BTMR_TimerHandle TimerStatusMonitor ;
        BHDM_MONITOR_Status MonitorStatus ;
        BHDM_MONITOR_TxHwStatusExtra stMonitorTxHwStatusExtra ;

        bool HpdTimerEnabled ;
        BTMR_TimerHandle TimerForcedTxHotplug;  /* pulse width when override TX HOTPLUG_IN */
#endif

#if BCHP_PWR_SUPPORT
    uint8_t clkPwrResource[BHDM_P_eHdmCoreIdMax];
    uint8_t phyPwrResource[BHDM_P_eHdmCoreIdMax];
#endif

        BHDM_TxSupport TxSupport ;
        uint8_t ScrambleSetting ;
        BKNI_EventHandle BHDM_EventScramble ;

        BHDM_TmdsPreEmphasisRegisters TmdsPreEmphasisRegisters[BHDM_TMDS_RANGES];

        /* HDMI 2.0 Registers */
        bool TmdsDisabledForBitClockRatioChange ;

#if BHDM_HAS_HDMI_20_SUPPORT
        BTMR_TimerHandle TimerScdcStatus ;
        BTMR_TimerHandle TimerTxScramble ;
        BHDM_P_TmdsMode eTmdsMode ;
        bool TmdsBitClockRatioChange ;

#if BHDM_CONFIG_HAS_HDCP22
        bool bAutoI2cMode ;
        uint32_t AutoI2CBuffer[BHDM_AUTO_I2C_P_MAX_READ_BYTES] ;
        BINT_CallbackHandle hAutoI2cCallback[MAKE_AUTO_I2C_INTR_ENUM(LAST)] ;

        BHDM_ScrambleConfig ScrambleConfig ;

        /************/
        /*     SCDC      */
        /************/
        BHDM_AUTO_I2C_CHANNEL AutoI2CChannel_ScdcUpdate ;
        BKNI_EventHandle AutoI2CEvent_ScdcUpdate ;
        BHDM_SCDC_ManufacturerData manufacturerData;
        uint8_t ScdcBuffer[2] ;

        /************/
        /*    HDCP 22   */
        /************/
        BHDM_AUTO_I2C_CHANNEL AutoI2CChannel_Hdcp22RxStatus ;
        BKNI_EventHandle AutoI2CEvent_Hdcp22RxStatusUpdate ;
        BKNI_EventHandle BHDM_EventHdcp22EncEnUpdate;
        BKNI_EventHandle BHDM_EventHdcp22ReAuthRequest;
        bool bReAuthRequestPending;
        BINT_CallbackHandle hHAECallback[MAKE_HAE_INTR_ENUM(LAST)] ;
        uint8_t Hdcp22RxStatusBuffer[2] ;
        BTMR_TimerHandle TimerHdcp22StableLink;  /* link is considered stabled if no REAUTH_REQ is received
                                                during this time */

        /************/
        /*   AutoWrite  */
        /************/
        BHDM_AUTO_I2C_CHANNEL AutoI2CChannel_Read ;
        BKNI_EventHandle AutoI2CEvent_Write ;

        /************/
        /*   AutoRead  */
        /************/
        BHDM_AUTO_I2C_CHANNEL AutoI2CChannel_Write ;
        BKNI_EventHandle AutoI2CEvent_Read ;

        BHDM_AUTO_I2C_P_READ_DATA ePendingReadType ;

        BHDM_AUTO_I2C_TriggerConfiguration AutoI2CChannel_TriggerConfig[BHDM_AUTO_I2C_CHANNEL_eMax] ;

        BHDM_SCDC_StatusControlData stStatusControlData ;

        /* buffer to temporary hold the whole HDMI register block */
        uint32_t hdmiRegBuff[hdmiRegBuffSize];

        /* buffer to temporary hold the full HDMI_TX_AUTO_I2C register block */
        uint32_t autoI2cRegBuff[autoI2cRegBuffSize];

#endif
#endif
 } BHDM_P_Handle ;


/**********************************
 *      PRIVATE FUNCTIONS
 **********************************/

/******************************************************************************
Summary:
Handle interrupts from the HDMI core.

Description:
Interrupts received from the HDMI core must be handled.  The following
is a list of possible interrupts.

        o  HDCP_PJ_MISMATCH_INTR
        o  HDCP_RI_A_MISMATCH_INTR
        o  HDCP_RI_B_MISMATCH_INTR

        o  HDCP_PJ_INTR

        o  PKT_OVERFLOW_INTR

        o  HDCP_AN_READY_INTR
        o  HDCP_RI_INTR
        o  HDCP_V_MATCH_INTR
        o  HDCP_V_MISMATCH_INTR
        o  HDCP_REPEATER_ERR_INTR

        o  CEC_INTR

        o  ILLEGAL_WRITE_TO_ACTIVE_RAM_PACKET_INTR

        o  DRIFT_FIFO_ALMOST_EMPTY_INTR
        o  DRIFT_FIFO_EMPTY_MINUS_INTR
        o  DRIFT_FIFO_ALMOST_FULL_INTR
        o  DRIFT_FIFO_FULL_MINUS_INTR

        o  HOTPLUG_INTR

Input:
        pParameter - pointer to interrupt specific information BHDM_Open.

Output:
        <None>

Returns:
        <None>

See Also:

*******************************************************************************/
void BHDM_P_HandleInterrupt_isr
(
        void *pParam1,                                          /* Device channel handle */
        int parm2                                                       /* not used */
) ;

#if BHDM_HAS_HDMI_20_SUPPORT
void BHDM_P_HandleHAEInterrupt_isr
(
        void *pParam1,                                          /* Device channel handle */
        int parm2                                                       /* not used */
) ;
#endif

#if BHDM_CONFIG_DVO_SUPPORT
BERR_Code BHDM_DVO_P_EnableDvoPort(
        BHDM_Handle hHDMI,              /* [in] HDMI handle */
        BHDM_OutputFormat eOutputFormat /* [in] format to use on Output Port */
) ;
#endif

void BHDM_P_ConfigureInputAudioFmt(
        BHDM_Handle hHDMI,                                                      /* [in] HDMI handle */
        const BAVC_HDMI_AudioInfoFrame *stAudioInfoFrame        /* [in] audio Info Frame settings */
) ;

BERR_Code BHDM_P_WritePacket(
        BHDM_Handle hHDMI,
        BHDM_Packet PhysicalHdmiRamPacketId,
        uint8_t PacketType,
        uint8_t PacketVersion,
        uint8_t PacketLength,
        uint8_t *PacketBytes
) ;

BERR_Code BHDM_P_SetGamutMetadataPacket(
        BHDM_Handle hHDMI               /* [in] HDMI Handle */
) ;

BERR_Code BHDM_P_ConfigurePhy(
        BHDM_Handle hHDMI,  /* [in] HDMI handle */
        const BHDM_Settings *NewHdmiSettings    /* [in] New HDMI settings */
);

void BHDM_P_PowerOnPhy (BHDM_Handle hHDMI);
void BHDM_P_PowerOffPhy (BHDM_Handle hHDMI);

void BHDM_P_SetPreEmphasisMode (
        BHDM_Handle hHDMI,
        uint8_t uValue,
        uint8_t uDriverAmp
);

BERR_Code BHDM_P_GetPreEmphasisConfiguration (
        BHDM_Handle hHDMI,
        BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
);


BERR_Code BHDM_P_SetPreEmphasisConfiguration(
        BHDM_Handle hHDMI,
        BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
);

void BHDM_P_GetReceiverSense(
        BHDM_Handle hHDMI,
        uint8_t *ReceiverSense
) ;

void BHDM_P_ClearHotPlugInterrupt(
   BHDM_Handle hHDMI            /* [in] HDMI handle */
);

void BHDM_P_CheckHotPlugInterrupt(
        BHDM_Handle hHDMI,               /* [in] HDMI handle */
        uint8_t *bHotPlugInterrupt      /* [out] Interrupt asserted or not */
);

void BHDM_P_RxDeviceAttached_isr(
        BHDM_Handle hHDMI,               /* [in] HDMI handle */
        uint8_t *bDeviceAttached        /* [out] Device Attached Status  */
) ;



void BHDM_P_EnableInterrupts_isr(const BHDM_Handle hHDMI) ;
void BHDM_P_DisableInterrupts(const BHDM_Handle hHDMI) ;
void BHDM_P_DisableDisplay_isr(const BHDM_Handle hHDMI  /* [in] HDMI handle */) ;

#if BHDM_CONFIG_DEBUG_FIFO
BERR_Code BHDM_P_EnableFIFOInterrupts(
        BHDM_Handle hHDMI, bool on) ;
#endif

typedef struct _BHDM_P_FIFO_DATA_
{
        uint8_t uiRdAddr,   uiWrAddr ;
        uint8_t uiFullness ;
        bool bUnderFlow, bOverFlow ;
} BHDM_P_FIFO_DATA ;

void BHDM_P_CaptureFIFOData(BHDM_Handle hHDMI, BHDM_P_FIFO_DATA *FifoData) ;

void BHDM_P_EnableTmdsData_isr(
        BHDM_Handle hHDMI, bool bEnableTmdsOutput) ;

void BHDM_P_EnableTmdsClock_isr(
   BHDM_Handle hHDMI, bool bEnableTmdsClock) ;

#if BHDM_CONFIG_BTMR_SUPPORT
void BHDM_P_AllocateTimers(const BHDM_Handle hHDMI) ;
void BHDM_P_FreeTimers(const BHDM_Handle hHDMI) ;
#endif

#if BHDM_CONFIG_HAS_HDCP22
BERR_Code BHDM_P_AcquireHDCP22_Resources(
        BHDM_Handle hHDMI) ;

void BHDM_HDCP_P_ReadHdcp22RxStatus_isr(const BHDM_Handle hHDMI) ;

void BHDM_SCDC_P_ReadUpdate0Data_isr(const BHDM_Handle hHDMI) ;
void BHDM_SCDC_P_ProcessUpdate_isr(const BHDM_Handle hHDMI) ;
void BHDM_SCDC_P_ClearStatusUpdates_isr(BHDM_Handle hHDMI) ;

void BHDM_SCDC_P_GetScrambleParams_isrsafe(const BHDM_Handle hHDMI, BHDM_ScrambleConfig *ScrambleSettings) ;
void BHDM_SCDC_P_ConfigureScramblingTx_isr(BHDM_Handle hHDMI, BHDM_ScrambleConfig *pstSettings) ;
void BHDM_SCDC_DisableScrambleTx(BHDM_Handle hHDMI) ;
void BHDM_SCDC_P_ConfigureScramblingTx(
        BHDM_Handle hHDMI, BHDM_ScrambleConfig *pstScrambleConfig) ;

void BHDM_P_ResetHDCPI2C_isr(const BHDM_Handle hHDMI);

void BHDM_P_SetHdcpConstrainAvConfiguration(
	BHDM_Handle hHDMI, const BHDM_Settings *HdmiSettings) ;
#endif

const uint8_t * BHDM_EDID_P_GetDebugEdid(const BHDM_Handle hHDMI) ;

const char * BHDM_P_GetVersion(void) ;

const char * BHDM_EDID_DEBUG_CeaAudioSampleRateToStr(
    uint8_t uiCeaSampleRateId) ;

const char * BHDM_EDID_DEBUG_CeaAudioBitDepthToStr(
    uint8_t uiCeaBitDepth) ;


BERR_Code BHDM_P_BREG_I2C_Read(
        const BHDM_Handle hHDMI,
        uint16_t chipAddr,
        uint8_t subAddr,
        uint8_t *pData,
        size_t length
);


BERR_Code BHDM_P_BREG_I2C_ReadNoAddr(
        const BHDM_Handle hHDMI,
        uint16_t chipAddr,
        uint8_t *pData,
        size_t length
);

void BHDM_HDCP_P_ResetSettings_isr(const BHDM_Handle hHDMI) ;
void BHDM_P_EnablePacketTransmission_isr(
   const BHDM_Handle hHDMI,
   BHDM_Packet PacketId
) ;
#ifdef __cplusplus
}
#endif

#endif /* BHDM_PRIV_H__ */
/* end bhdm_priv.h */
