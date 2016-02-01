/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/*================== Module Overview =====================================
<verbatim>
The BTHD (Broadcom Terrestrial Handheld Demodulator) portinginterface (PI)
controls the RF front-end block on any Broadcom chip that has one or more
DVB T/H core(s), such as BCM2940. The BTHD PI is responsible for controlling
the terrestrial tuner, and theterrestrial downstream receiver. 

The BTHD PI device is accessed by the handle BTHD_Handle. There is 1 
BTHD_Handle per Broadcom chip. A BTHD_Handle will have 1 or more channels, 
depending on the number of THD cores present in the Broadcom chip. 

The BTHD_Handle shall be proveded the following API
    BTHD_Open
    BTHD_Close
    BTHD_Init
    BTHD_TuneAcquire
    BTHD_GetVersion
    BTHD_GetThdStatus
    BTHD_ResetInbandStatus
    BTHD_GetInterruptEventHandler
    BTHD_HandleInterrupt_isr

The magnum/portinginterface/THD/bthd.h header file defines a standard THDT/H 
front-end API for all Broadcom THD chips.  The implementation of the API for
each chip is in the magnum/portinginterface/thd/<chip> directory.  

Interrupt Requirements
The application needs to call BTHD_HandleInterrupt_isr() when it receives
the THDT L1 interrupt.

Sample Code
//
// NOTE: This is sample code for a system that contains single-channel THDT/H
//       using BCM2940.  This code does not do any error checking
//
#include "bTHD.h"
#include "bTHD_3520.h"
#include "bTHD_3560.h"

static BTHD_Handle      hTHD;  // handle for BCM3560 THD core
static BCHP_Handle      hCHP;
static BREG_I2C_Handle  hReg;

// These are pointers to AP microcode used in BCM3560/BCM3520.  
// The microcode will vary depending on chip/board/customer, so it is not 
// part of the BTHD PI.
extern const uint8_t *pBcm3520ApImage;
extern const uint8_t *pBcm3560ApImage;

void main(void)
{
   BTHD_Settings     settings1, settings2;
   BTHD_InbandParams inbandParams;

   // do initialization
   // (get BCHP_Handle, BREG_I2C_Handle, etc)
   ...

   // BTHD initialization for BCM2940
   BTHD_GetDefaultSettings(&settings2);
   settings2.i2c.chipAddr = BCM2940_I2C_ADDR;
   settings2.i2c.interruptEnableFunc = bcm2940_enable_irq;
   settings2.i2c.interruptEnableFuncParam = NULL;

   // enable the BCM2940 host interrupt and map that interrupt to a function
   // that will call the BTHD PI function BTHD_HandleInterrupt_isr()
   ...

   // start a thread that runs bcm2940_background()
   ...

   // download the default Bcm2940 microcode and run the AP
   BTHD_Init(hTHD, NULL);

   // tune the tuner
   ...

   
   // acquire a DVB-T Channel
   inbandParams.tunerFreq = 500000000;
   inbandParams.bandwidth = BTHD_Bandwidth_8Mhz;
   inbandParams.cciMode = BTHD_CCI_None;
   BTHD_TuneAcquire(hTHD, &inbandParams);

   ...

   close_THD:
   // disable THD L1 interrupts
   // terminate bcm3520_background task
   ...
   BTHD_Close(hTHD);

   program_done:
   ...
}

// The application must call BTHD_ProcessInterruptEvent() every time the THD
// interrupt event is set.
void bcm2940_background()
{
   // get handle to the BCM3520 interrupt event
   BTHD_GetInterruptEventHandle(hTHD, &event);

   while (1)
   {
      BKNI_WaitForEvent(event, BKNI_INFINTE);
      BTHD_ProcessInterruptEvent(hTHD);
   }
}


// This is an example of a callback function for enabling/disabling the L1
// interrupt.
void THD_enable_irq(bool b, void *p)
{
   if (b)
      CPUINT1_Enable(THD_IRQ);   // enable the L1 interrupt
   else
      CPUINT1_Disable(THD_IRQ);  // disable the L1 interrupt
}
</verbatim>
========================================================================*/

#ifndef BTHD_H__
#define BTHD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "bint.h"
#include "btmr.h"
#include "bmem.h"
#include "berr_ids.h"
#include "bfec.h"


/***************************************************************************
Summary:
    BTHD error codes

Description:

See Also:

****************************************************************************/
#define BTHD_ERR_MI2C_NO_ACK     BERR_MAKE_CODE(BERR_THD_ID, 0x0) /* This is a BERR_Code to indicate no ack from i2c slave device (e.g. tuner). */
#define BTHD_ERR_DMA             BERR_MAKE_CODE(BERR_THD_ID, 0x1) /* This is a BERR_Code to indicate DMA transfer has failed */
#define BTHD_ERR_RFIFO_UNDERRUN  BERR_MAKE_CODE(BERR_THD_ID, 0x2) /* This is a BERR_Code to indicate DMA transfer has failed due to read fifo underflow */
#define BTHD_ERR_WFIFO_OVERWRITE BERR_MAKE_CODE(BERR_THD_ID, 0x3) /* This is a BERR_Code to indicate DMA transfer has failed due to write fifo overflow */
#define BTHD_ERR_COMMAND_TIMEOUT BERR_MAKE_CODE(BERR_THD_ID, 0x4) /* This is a BERR_Code to indicate a command was sent to firmware but was not acknowledged */
#define BTHD_ERR_INIT_TIMEOUT    BERR_MAKE_CODE(BERR_THD_ID, 0x5) /* This is a BERR_Code to indicate timeout occured while downloading the microcode */
#define BTHD_ERR_HAB_NO_ACK      BERR_MAKE_CODE(BERR_THD_ID, 0x6) /* This is a BERR_Code to indicate that the HAB command was not processed by AP. */
#define BTHD_ERR_HAB_TIMEOUT     BERR_MAKE_CODE(BERR_THD_ID, 0x7) /* This is a BERR_Code to indicate an HAB timeout. */
#define BTHD_ERR_AP_NOT_INIT     BERR_MAKE_CODE(BERR_THD_ID, 0x8) /* This is a BERR_Code to indicate that the AP has failed to initialize. */
#define BTHD_ERR_AP_FAIL         BERR_MAKE_CODE(BERR_THD_ID, 0x9) /* This is a BERR_Code to indicate that the AP is no longer responding to command */
#define BTHD_ERR_HABAV           BERR_MAKE_CODE(BERR_THD_ID, 0xa) /* This is a BERR_Code to indicate that the AP did not properly respond to HAB command */
#define BTHD_ERR_MEMAV           BERR_MAKE_CODE(BERR_THD_ID, 0xb) /* This is a BERR_Code to indicate that the AP failed while accessing memory */
#define BTHD_ERR_HAB_ERR         BERR_MAKE_CODE(BERR_THD_ID, 0xd) /* This is a BERR_Code to indicate an HAB command format error. */
#define BTHD_ERR_AP_COPY         BERR_MAKE_CODE(BERR_THD_ID, 0xe) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_AP_IRQ          BERR_MAKE_CODE(BERR_THD_ID, 0xf) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_AP_SCR          BERR_MAKE_CODE(BERR_THD_ID, 0x10) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_AP_ISB          BERR_MAKE_CODE(BERR_THD_ID, 0x11) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_AP_WD           BERR_MAKE_CODE(BERR_THD_ID, 0x12) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_AP_STACK        BERR_MAKE_CODE(BERR_THD_ID, 0x13) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_AP_BSC          BERR_MAKE_CODE(BERR_THD_ID, 0x14) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_AP_PLL          BERR_MAKE_CODE(BERR_THD_ID, 0x15)  /* This is a BERR_Code to indicate AP was unable to boot because PLL did not lock*/
#define BTHD_ERR_AP_HABAV        BERR_MAKE_CODE(BERR_THD_ID, 11) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_HOST_XFER       BERR_MAKE_CODE(BERR_THD_ID, 0x16)  /* This is a BERR_Code to indicate a host transfer error (underrun/overrun). */
#define BTHD_ERR_AP_UNKNOWN      BERR_MAKE_CODE(BERR_THD_ID, 0x17) /* This is a BERR_Code to indicate an AP-internal error. */
#define BTHD_ERR_INVALID_ANALOG_MODES   BERR_MAKE_CODE(BERR_THD_ID, 0x18) /* This is a BERR_Code to indicate the video and audio modes selected is not valid */
#define BTHD_ERR_POWER_DOWN     BERR_MAKE_CODE(BERR_THD_ID, 0x19) /* This is a BERR_Code to indicate a THD api was called while power is not enabled */

/***************************************************************************
Summary:
    BTHD Powerup/Powerdown code

Description: This is used by PowerEnable and PowerDisable function to allow
             the user to select which cores they would like the dynamicly 
             control the power.

See Also:

****************************************************************************/
#define BTHD_POWER_AP 0x1 /* This will only power on/off the internal acquistion processor */

/***************************************************************************
Summary:
    The handle for thd module.

Description:
    This is an opaque handle that is used to access the BTHD API.

See Also:
    BTHD_Open()

****************************************************************************/
typedef struct BTHD_P_Handle *BTHD_Handle;

/***************************************************************************
Summary:
    Enumeration for analog video modes

Description:
    This enum specifies analog video mode options for the IFD decoder. This
    only applies to IFD core in cases where this PI controls both ofdm and
    analog video receiver. 

See Also:
    None.

****************************************************************************/
typedef enum BTHD_AnalogVideoMode {
    BTHD_AnalogVideoMode_eNtsc_M,   /* NTSC-M mode. can be combined with BTSC or A2 audio */
    BTHD_AnalogVideoMode_eNtsc_N,   /* NTSC-N mode, can be combined with only BTSC audio */
    BTHD_AnalogVideoMode_ePal_M,    /* PAL-M mode, can be combined with only BTSC audio */
    BTHD_AnalogVideoMode_ePal_N,    /* PAL-N mode, can be combined with only BTSC audio */
    BTHD_AnalogVideoMode_ePal_I,    /* PAL-I mode, can be combined with only NICAM audio */
    BTHD_AnalogVideoMode_ePal_BGH,  /* PAL-B/G/H mode, can be combined with A2 or NICAM audio */
    BTHD_AnalogVideoMode_ePal_DK,   /* PAL-D/K mode, can be combined with NICAM audio only */
    BTHD_AnalogVideoMode_ePal_DK1,  /* PAL-D/K1 mode, can be combined with A2 audio only */
    BTHD_AnalogVideoMode_ePal_DK2,  /* PAL-D/K2 mode, can be combined with A2 audio only */
    BTHD_AnalogVideoMode_ePal_DK3,  /* PAL-D/K3 mode, can be combined with A2 audio only */
    BTHD_AnalogVideoMode_eSecam_BGH,/* SECAM-B/G/H mode, can be combined with A2 or NICAM audio */
    BTHD_AnalogVideoMode_eSecam_DK, /* SECAM-D/K mode, can be combined with only NICAM audio */
    BTHD_AnalogVideoMode_eSecam_L   /* SECAM-L mode, can be combined with onl y NICAM audio */
} BTHD_AnalogVideoMode;

/***************************************************************************
Summary:
    Enumeration for analog audio modes

Description:
    This enum specifies second audio carrier mode options for the IFD decoder. 
    This only applies to IFD core in cases where this PI controls both ofdm 
    an analog video receiver. Not all video/audio combo are valid, refer to 
    each video mode for more options

See Also:
    BTHD_AnalogVideoMode

****************************************************************************/
typedef enum BTHD_AnalogAudioMode {
    BTHD_AnalogAudioMode_eBtsc,     /* Selects BTSC audio */
    BTHD_AnalogAudioMode_eA2,       /* Selects A2 audio */
    BTHD_AnalogAudioMode_eNicam     /* Selects NICAM audio */
} BTHD_AnalogAudioMode;

/***************************************************************************
Summary:
    Enumeration for analog input

Description:
    This enum specifies analog audio input options for the IFD decoder. This
    only applies to IFD core in cases where this PI controls both ofdm and
    analog video receiver. 

See Also:
    None.

****************************************************************************/
typedef enum BTHD_AnalogInput {
    BTHD_AnalogInput_eIf = 0,       /* Audio input with IF signal */
    BTHD_AnalogInput_eSif_1 = 0x80  /* Audio input from SIF Channel 1 */
} BTHD_AnalogInput;

/***************************************************************************
Summary:
    Enum for IFD deviation modes

Description:
    This enum specifies deviation mode for the IFD core

See Also:
    None.

****************************************************************************/
typedef enum BTHD_AnalogAudioDeviation
{
    BTHD_AnalogAudioDeviation_eNormal,
    BTHD_AnalogAudioDeviation_eMedium,
    BTHD_AnalogAudioDeviation_eHigh
} BTHD_AnalogAudioDeviation;

/***************************************************************************
Summary:
    Enum for IFD acquisition mode

Description:
    This enum specifies whether should speed acquistion but acquire less offsets

See Also:
    None.

****************************************************************************/
typedef enum BTHD_IfdAcquisitionMode
{
    BTHD_IfdAcquisitionMode_eSlow,
    BTHD_IfdAcquisitionMode_eFast
} BTHD_IfdAcquisitionMode;


/***************************************************************************
Summary:
    Enum for THD acquisition mode

Description:
    This enum specifies whether we are in Scan, Manual or Auto mode

See Also:
    None.

****************************************************************************/
typedef enum BTHD_ThdAcquisitionMode
{
    BTHD_ThdAcquisitionMode_eAuto,
    BTHD_ThdAcquisitionMode_eManual,
    BTHD_ThdAcquisitionMode_eScan
} BTHD_ThdAcquisitionMode;

/***************************************************************************
Summary:
    Enumeration for pga gain.

Description:
    This enum specifies the pga gain.

See Also:
    None.

****************************************************************************/
typedef enum BTHD_PgaGain
{
   BTHD_PgaGain_e0dB,     
   BTHD_PgaGain_e1dB,   
   BTHD_PgaGain_e2dB,  
   BTHD_PgaGain_e3dB, 
   BTHD_PgaGain_e4dB, 
   BTHD_PgaGain_e5dB,
   BTHD_PgaGain_e6dB
} BTHD_PgaGain;

/***************************************************************************
Summary:
    Enumeration for Agc Pin Mode

Description:
    This enum specifies the voltage swing of the AGC pin

See Also:
    None.

****************************************************************************/
typedef enum BTHD_AgcPinMode
{
    BTHD_AgcPinMode_e33V, /* indicates AGC pin goes from 0 to 3.3V */
    BTHD_AgcPinMode_e25V  /* indicates AGC pin goes from 0 to 2.5V */
} BTHD_AgcPinMode;

/***************************************************************************
Summary:
    Enum for IFD pullin range

Description:
    This enum specifies how much frequency offset the receiver will acquire to

See Also:
    None.

****************************************************************************/
typedef enum BTHD_IfdPullInRange
{
    BTHD_IfdPullInRange_e32kHz=0,
    BTHD_IfdPullInRange_e250kHz,
    BTHD_IfdPullInRange_e500kHz,
    BTHD_IfdPullInRange_e750kHz,
    BTHD_IfdPullInRange_e1000kHz,
    BTHD_IfdPullInRange_e1100kHz
} BTHD_IfdPullInRange;

/***************************************************************************
Summary:
    Enumeration for Callback types

Description:
    This enumeration defines Callback types.

See Also:

****************************************************************************/
typedef enum BTHD_Callback
{
    BTHD_Callback_eLockChange,          /* Callback to notify application of lock change */
    BTHD_Callback_eUpdateGain,          /* Callback to notify application to update gain */ 
    BTHD_Callback_eNoSignal,            /* Callback to notify application there is no signal */ 
    BTHD_Callback_eAsyncStatusReady,    /* Callback to notify application there is no signal */    
    BTHD_Callback_eTuner,               /* Callback to tuner regarding a new setting */ 
    BTHD_Callback_eLast                 /* More may be required */
} BTHD_Callback;

/***************************************************************************
Summary:
    Structure containing IFD agc parameters

Description:
    This structure contains the setting for AGC block

See Also:
    None.

****************************************************************************/
typedef struct BTHD_IfdAgcSettings
{
    uint16_t top;           /* tuner TOP point */
    BTHD_PgaGain pgaGain;   /* pga gain in dB */ 
    uint8_t delay;          /* Delay in ms that the acquisition script will wait for the AGC hardware to settle */
    uint8_t rfBandwidth;    /* RF AGC band width */
    uint8_t ifBandwidth;    /* IF AGC band width */ 
} BTHD_IfdAgcSettings;

/***************************************************************************
Summary:
    Structure containing input parameters to IFD acquire command

Description:
    This structure contains the setting for IFD acquire command

See Also:
    None.

****************************************************************************/
typedef struct BTHD_IfdParams
{
   BTHD_AnalogVideoMode eVideoMode;     /* Selects analog video standards */
   BTHD_AnalogAudioMode eAudioMode;     /* Selects analog audio standards */
   BTHD_AnalogInput     eAnalogInput;   /* Select analog input options */
   uint32_t             ifFreqOffset;   /* IF Frequency offset */
   BTHD_AnalogAudioDeviation eAnalogAudioDeviation;      /* Selects deviation mode for analog audio */
   bool                 bSpectrumInvert; /* Invert input spectrum, only used in SECAM-L mode to get SECAM-L' */
   BTHD_IfdPullInRange  ePullInRange;
   BTHD_IfdAcquisitionMode eAcquisitionMode; /* Faster acquisition */
   BTHD_IfdAgcSettings  agcSettings;

} BTHD_IfdParams;

/*****************************************************************************
Summary:
   Structure containing host I2C settings
Description:
   This structure contains the settings for host I2C control.  For memory-
   mapped BTHD devices, this structure will not be used.
See Also:
   BTHD_Settings
******************************************************************************/
typedef struct BTHD_I2cSettings
{
   uint16_t chipAddr;  /* Bcm2940 i2c chip address, not needed for 3556 */
   void     (*interruptEnableFunc)(bool, void*); /* callback function for enabling/disabling L1 interrupt */
   void     *interruptEnableFuncParam; /* parameter passed to interruptEnableFunc */
} BTHD_I2cSettings;

/***************************************************************************
Summary:
    Enumeration for receiver deocde method

Description:
    This enum specifies the decode options

See Also:
    None.

****************************************************************************/
typedef enum BTHD_DecodeMode
{
   BTHD_Decode_Lp,      /* Low priority decode */
   BTHD_Decode_Hp     /* High priority decode */
} BTHD_DecodeMode;

/***************************************************************************
Summary:
    Enumeration for receiver CCI Mode

Description:
    This enum specifies CoChannel Interferece filter options

See Also:
    None.

****************************************************************************/
typedef enum BTHD_CCIMode
{
   BTHD_CCI_None,           /* No CCI selected */
   BTHD_CCI_Auto            /* Auto Selection */
} BTHD_CCIMode;

/***************************************************************************
Summary:
    Enumeration for receiver bandwidth

Description:
    This enum specifies the bandwidth the channel received

See Also:
    None.

****************************************************************************/
typedef enum BTHD_Bandwidth
{
   BTHD_Bandwidth_5Mhz=5,   /* Selects 5 MHz */
   BTHD_Bandwidth_6Mhz,   /* Selects 5 MHz */
   BTHD_Bandwidth_7Mhz,   /* Selects 5 MHz */
   BTHD_Bandwidth_8Mhz    /* Selects 5 MHz */
} BTHD_Bandwidth;


/***************************************************************************
Summary:
    Enumeration for receiver modulaton type

Description:
    This enum specifies the modulation type of the channel received

See Also:
    None.

****************************************************************************/
typedef enum BTHD_Modulation
{
    BTHD_Modulation_eDqpsk=0,      /* DQPSK Modulation */
    BTHD_Modulation_eQpsk=4,      /* QPSK Modulation */
    BTHD_Modulation_e16Qam=16,    /* 16-QAM Modulation */
    BTHD_Modulation_e64Qam=64     /* 64-QAM Modulation */
} BTHD_Modulation;


/***************************************************************************
Summary:
    Enumeration for THD pull-in range.

Description:
    This enum specifies the pull-in range of THD core

See Also:
    None.

****************************************************************************/
typedef enum BTHD_PullInRange
{
   BTHD_PullInRange_eNarrow,   /* carrier acquisition range is narrow +/-200kHz */
   BTHD_PullInRange_eWide     /* carrier acquisition range is wide +/-600kHz */
} BTHD_PullInRange;


/***************************************************************************
Summary:
    Enum for inband mode

Description:
    This enum specifies the mode for the inband receiver

See Also:
    None.

****************************************************************************/
typedef enum BTHD_InbandMode
{
  BTHD_InbandMode_eDvbt,           /* DVB-T */
  BTHD_InbandMode_eIsdbt          /* ISDB-T */
} BTHD_InbandMode;


/***************************************************************************
Summary:
    Enumeration for receiver transmission mode

Description:
    This enum specifies the transmission mode of the channel received

See Also:
    None.

****************************************************************************/
typedef enum BTHD_TransmissionMode
{
    BTHD_TransmissionMode_e2K=2, /* 2K Mode */
    BTHD_TransmissionMode_e4K=4, /* 4K Mode */
    BTHD_TransmissionMode_e8K=8  /* 8K Mode */
} BTHD_TransmissionMode;

/***************************************************************************
Summary:
    Enumeration for receiver guard interval

Description:
    This enum specifies the guard interval of the channel received

See Also:
    None.

****************************************************************************/
typedef enum BTHD_GuardInterval
{
    BTHD_GuardInterval_e1_32=32, /* Guard 1/32 */
    BTHD_GuardInterval_e1_16=16, /* Guard 1/16 */
    BTHD_GuardInterval_e1_8=8,   /* Guard 1/8 */
    BTHD_GuardInterval_e1_4=4    /* Guard 1/4 */
} BTHD_GuardInterval;

/***************************************************************************
Summary:
    Enumeration for receiver code rate

Description:
    This enum specifies the code rate of the channel received

See Also:
    None.

****************************************************************************/
typedef enum BTHD_CodeRate
{
    BTHD_CodeRate_e1_2=2, /* Rate 1/2 */
    BTHD_CodeRate_e2_3=3, /* Rate 2/3 */
    BTHD_CodeRate_e3_4=4, /* Rate 3/4 */
    BTHD_CodeRate_e5_6=6, /* Rate 5/6 */
    BTHD_CodeRate_e7_8=8  /* Rate 7/8 */ 
} BTHD_CodeRate;

/***************************************************************************
Summary:
    Enumeration for DVB-T hierarchical mode

Description:
    This enum specifies the hierarchy of the channel received

See Also:
    None.

****************************************************************************/
typedef enum BTHD_Hierarchy
{
    BTHD_Hierarchy_0=0, /* select hierarchy = 0 */
    BTHD_Hierarchy_1=1, /* select hierarchy = 1 */
    BTHD_Hierarchy_2=2, /* select hierarchy = 2 */
    BTHD_Hierarchy_4=4  /* select hierarchy = 4 */
} BTHD_Hierarchy;

/***************************************************************************
Summary:
    Enumeration for ISDB-T time interleaving

Description:
    This enum specifies the ISDB-T time interleaving mode

See Also:
    None.

****************************************************************************/
typedef enum BTHD_IsdbtTimeInterleaving
{
  BTHD_IsdbtTimeInterleaving_0X,
  BTHD_IsdbtTimeInterleaving_1X,
  BTHD_IsdbtTimeInterleaving_2X,
  BTHD_IsdbtTimeInterleaving_3X
}BTHD_IsdbtTimeInterleaving;

/***************************************************************************
Summary:
    Enumeration for NICAM audio recevier current encoder mode

Description:
    This enum specifies the NICAM encoder modes

See Also:
    None.

****************************************************************************/
typedef enum BTHD_NicamEncoderMode
{
    BTHD_Stereo_BackupFm=1,
    BTHD_DualMono_BackupFm=5,
    BTHD_Stereo_NoBackup=0,
    BTHD_DualMono_NoBackup=4,
    BTHD_MonoData_NoBackup=8,
    BTHD_MonoData_BackupFm=9,
    BTHD_Data_NoBackup=12,
    BTHD_Data_BackupFm=13
} BTHD_NicamEncoderMode;

/***************************************************************************
Summary:
    Enumeration for receiver mode

Description:
    This enum specifies the receiver  modes

See Also:
    None.

****************************************************************************/
typedef enum BTHD_ThdMode
{
    BTHD_Mode_eAuto = 0,
    BTHD_Mode_eFixed=1,
    BTHD_Mode_ePederstrian=2,
    BTHD_Mode_eMobile=3
} BTHD_ThdMode;


/***************************************************************************
Summary:
    Enumeration for receiver mode

Description:
    This enum specifies the receiver  modes

See Also:
    None.

****************************************************************************/
typedef enum BTHD_ModeGuard
{
    BTHD_ModeGuard_eManual = 0,
    BTHD_ModeGuard_eAuto=1,
    BTHD_ModeGuard_eAutoDvbt=2,
    BTHD_ModeGuard_eAutoIsdbt=3
} BTHD_ModeGuard;

/***************************************************************************
Summary:
    Enumeration for Lock Status

Description:
    This enumeration represents the lock status of of a THD channel.

See Also:

****************************************************************************/
typedef enum BTHD_LockStatus
{           
    BTHD_LockStatus_eUnlocked,
    BTHD_LockStatus_eLocked, 
    BTHD_LockStatus_eNoSignal,      
    BTHD_LockStatus_eLast    
} BTHD_LockStatus;

/***************************************************************************
Summary:
    Enumeration for spectrum Mode.
****************************************************************************/
typedef enum BTHD_SpectrumMode
{
    BTHD_SpectrumMode_eAuto,
    BTHD_SpectrumMode_eManual,
    BTHD_SpectrumMode_eLast
} BTHD_SpectrumMode;

/***************************************************************************
Summary:
    Enumeration for Inverted or non-nverted spectrum setting.
****************************************************************************/
typedef enum BTHD_InvertSpectrum
{
    BTHD_InvertSpectrum_eNormal,
    BTHD_InvertSpectrum_eInverted,
    BTHD_InvertSpectrum_eLast
} BTHD_InvertSpectrum;

/***************************************************************************
Summary:
    Structure for inband acquisition

Description:
    This structure contains parameters used for an inband channel acquisition.

See Also:
    BTHD_Bandwidth, BTHD_CCIMode, BTHD_DecodeMode, BTHD_InbandMode

****************************************************************************/
typedef struct BTHD_InbandParams
{
    /* Common Parameters */
  BTHD_InbandMode            mode;                           /* Selects inband mode (DVB-T or ISDB-T)*/
  BTHD_Bandwidth             bandwidth;                      /* Selects bandwidth of input signal */
  uint32_t                   tunerFreq;                      /* Set the current tuner frequency in Hz */
  uint32_t                   ifFreq;                         /* Selects IF freqeuncy in Hz, only used in 3556 */
  BTHD_CCIMode               cciMode;                        /* Selects co-channel filter options */
  BTHD_PullInRange           ePullinRange;                   /* Set wide or narrow carrier acquisition range */
  BTHD_ModeGuard             eModeGuardAcquire;              /* If Manual, caller will provide eTransmissionMode and eGuardInterval, else if true, receiver automatically discovers this info */
  BTHD_TransmissionMode      eTransmissionMode;              /* Manually set the transmission mode. Only used if bModeGuardAcquire=false */
  BTHD_GuardInterval         eGuardInterval;                 /* Manually set the guard interval. Only used if bModeGuardAcquire=false */
  bool                       bTuneAcquire;                   /* This bit directs the Downstream Demodulator to initiate an acquire immediately after its associated WFE or Tuner is tuned.  
                                                                If the bit is set, then the Demodulator will initiate an acquire immediately after the tuner has completed a tune  regardless of autoacquire.   
                                                                Note: Unless otherwise indicated, this flag only applies to the Acquisition Processors Internal WFE or Internal Tuner. */
  BTHD_SpectrumMode          spectrumMode;                   /* Defines manual or auto spectrum mode. */
  BTHD_InvertSpectrum        invertSpectrum;                 /* Defines normal or inverted spectrum selection for acquisition. */                                               

  /* DVB-T Specific Parameters */
  BTHD_DecodeMode            decodeMode;                     /* Selects the DVB-T hierarchical stream to decode (High Priority or Low Priority) */
  bool                       bTpsAcquire;                    /* If false, caller will provide the {modulation,high priority code rate, low priority code rate, hierarchy } TPS parameters. Only used if mode-DVB-T */
  BTHD_Modulation            eModulation;                    /* Manually set DVB-T modulation type. Only used if mode=DVB-T and bTpsAcquire=false */
  BTHD_CodeRate              eCodeRateHP;                    /* Manually set DVB-T high priority code rate. Only used if mode=DVB-T and bTpsAcquire=false */
  BTHD_CodeRate              eCodeRateLP;                    /* Manually set DVB-T low priority code rate. Only used if mode=DVB-T and bTpsAcquire=false */
  BTHD_Hierarchy             eHierarchy;                     /* Manually set DVB-T hierarchy. Only used if mode=DVB-T and bTpsAcquire=false */

  /* ISDB-T Specific Parameters */
  bool                       bTmccAcquire;                   /* If false, caller will provide the {partial reception, modulation, code rate, time interleaving, number of segments} TMCC parameters. Only used if mode=ISDB-T */
  bool                       bIsdbtPartialReception;         /* Manually set ISDB-T partial reception. Only used if mode=ISDB-T and bTmccAcquire=false */
  BTHD_Modulation            eIsdbtAModulation;              /* Manually set ISDB-T Layer A modulation type. Only usedd if mode=ISDB-T and bTmccAcquire=false */
  BTHD_CodeRate              eIsdbtACodeRate;                /* Manually set ISDB-T Layer A code rate. Only used if mode=ISDB-T and bTmccAcquire=false */
  BTHD_IsdbtTimeInterleaving eIsdbtATimeInterleaving;        /* Manually set ISDB-T Layer A time interleaving. Only used if mode=ISDB-T and bTmccAcquire=false */
  uint32_t                   eIsdbtASegments;                /* Manually set ISDB-T Layer A number of segments. Only used if mode=ISDB-T and bTmccAcquire=false */
  BTHD_Modulation            eIsdbtBModulation;              /* Manually set ISDB-T Layer B modulation type. Only usedd if mode=ISDB-T and bTmccAcquire=false */
  BTHD_CodeRate              eIsdbtBCodeRate;                /* Manually set ISDB-T Layer B code rate. Only used if mode=ISDB-T and bTmccAcquire=false */
  BTHD_IsdbtTimeInterleaving eIsdbtBTimeInterleaving;        /* Manually set ISDB-T Layer B time interleaving. Only used if mode=ISDB-T and bTmccAcquire=false */
  uint32_t                   eIsdbtBSegments;                /* Manually set ISDB-T Layer B number of segments. Only used if mode=ISDB-T and bTmccAcquire=false */
  BTHD_Modulation            eIsdbtCModulation;              /* Manually set ISDB-T Layer C modulation type. Only usedd if mode=ISDB-T and bTmccAcquire=false */
  BTHD_CodeRate              eIsdbtCCodeRate;                /* Manually set ISDB-T Layer C code rate. Only used if mode=ISDB-T and bTmccAcquire=false */
  BTHD_IsdbtTimeInterleaving eIsdbtCTimeInterleaving;        /* Manually set ISDB-T Layer C time interleaving. Only used if mode=ISDB-T and bTmccAcquire=false */
  uint32_t                   eIsdbtCSegments;                /* Manually set ISDB-T Layer C number of segments. Only used if mode=ISDB-T and bTmccAcquire=false */
  BTHD_ThdAcquisitionMode    eAcquisitionMode;               /* Acquisition Mode */  
} BTHD_InbandParams;


/***************************************************************************
Summary:
    Structure for THD status

Description:
    This structure contains THD status

See Also:
    None.

****************************************************************************/
typedef struct BTHD_THDStatus
{
  /* Common Fields */
  BTHD_InbandParams          acqParams;                      /* Input parameters from most recent acquisition */
  bool                       bReceiverLock;                  /* Receiver lock indicator */
  bool                       bFecLock;                       /* FEC lock indicator */
  bool                       bNoSignalDetected;              /* OFDM signal detected indicator. Should only be used by channel scan as a flag to move on to the next channel */
  BTHD_TransmissionMode      eTransmissionMode;              /* Transmission mode */
  BTHD_GuardInterval         eGuardInterval;                 /* Guard interval */
  uint32_t                   ulRFAgc;                        /* RF Agc value in 1/10 percent */
  uint32_t                   ulIFAgc;                        /* IF Agc value in 1/10 percent */
  int16_t                    nSignalStrength;                /* Signal strength in 1/100th of a dBm. Not available on BCM3556 */
  int32_t                    lCarrierOffset;                 /* Carrier offset in Hz */
  int32_t                    lTimingOffset;                  /* Timing offset in Hz */
  int32_t                    lgainOffset;                    /* internal AGC gain offset in units of 1/256th of a dB */  
  int16_t                    nSnr;                           /* SNR in 1/256th dB */
  bool                       bSpectrumInverted;              /* Spectral inversion indicator */
  uint32_t                   ulReacqCount;                   /* Number of reacquisitions performed */
  bool                       bValid;                         /* Indicates status structure returned is valid. When this is false, the application should ignore all status returned */

  /* DVB-T Specific Fields */
  BTHD_DecodeMode            eDecodeMode;                    /* DVB-T hierarchical decode mode (priority) that was set in the acquire parameters */
  BTHD_Modulation            eModulation;                    /* DVB-T modulation type */
  BTHD_CodeRate              eCodeRate;                      /* DVB-T code rate */
  BTHD_Hierarchy             eHierarchy;                     /* DVB-T hierarchy */
  uint16_t                   nCellId;                        /* DVB-T cell Id */

  uint32_t                   ulRsCorrectedBlocks;            /* DVB-T accumulated corrected RS blocks */
  uint32_t                   ulRsUncorrectedBlocks;          /* DVB-T accumlated uncorrected RS blocks */
  uint32_t                   ulRsCleanBlocks;                /* DVB-T accumlated clean RS blocks */
  uint32_t                   ulRsTotalBlocks;                /* DVB-T accumalted total RS blocks */
  uint32_t                   ulViterbiUncorrectedBits;       /* DVB-T accumlated error bits after Viterbi */
  uint32_t                   ulViterbiTotalbits;             /* DVB-T accumlated total bits after Viterbi */
  uint32_t                   ulViterbiBer;                   /* DVB-T Viterbi bit error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */
  uint32_t                   ulPreViterbiBer;                /* DVB-T pre-Viterbi bit error rate in 1/2147483648 th unit. To calculate the actual rate, take the value and divide by 2147483648 */
  uint8_t                    signalLevelPercent;             /* Signal Level Percent represents the Signal Strength of the received signal in percent */
  uint8_t                    signalQualityPercent;           /* Signal Quality Percent represents the Signal Quality of the received signal in percent */
  bool                       inDepthSymbolInterleave;        /* If true, in depth symbol interleaving is enabled.  Applies only for DVB-H. */
  bool                       timeSlicing;                    /* If true, time slicing of the services is used for receiver power savings. Applies only for DVB-H. */
  bool                       mpeFec;                         /* If true, MultiProtocol Encapsulation (MPE) Forward Error Correction (FEC) is used. Applies only for DVB-H*/

  /* ISDB-T Specific Fields */
  bool                       bIsdbtEWS;                      /* ISDB-T Emergency Warning System EWS indicator */
  bool                       bIsdbtPartialReception;         /* ISDB-T partial reception */

  BTHD_Modulation            eIsdbtAModulation;              /* ISDB-T Layer A modulation type */
  BTHD_CodeRate              eIsdbtACodeRate;                /* ISDB-T Layer A code rate */
  BTHD_IsdbtTimeInterleaving eIsdbtATimeInterleaving;        /* ISDB-T Layer A time interleaving */
  uint32_t                   eIsdbtASegments;                /* ISDB-T Layer A number of segments */
  uint32_t                   ulIsdbtARsCorrectedBlocks;      /* ISDB-T Layer A accumulated corrected RS blocks */
  uint32_t                   ulIsdbtARsUncorrectedBlocks;    /* ISDB-T Layer A accumlated uncorrected RS blocks */
  uint32_t                   ulIsdbtARsCleanBlocks;          /* ISDB-T Layer A accumlated clean RS blocks */
  uint32_t                   ulIsdbtARsTotalBlocks;          /* ISDB-T Layer A accumalted total RS blocks */
  uint32_t                   ulIsdbtAViterbiUncorrectedBits; /* ISDB-T Layer A accumlated error bits after Viterbi */
  uint32_t                   ulIsdbtAViterbiTotalbits;       /* ISDB-T Layer A accumlated total bits after Viterbi */
  uint8_t                    signalLevelAPercent;            /* Signal Level Percent ISDB-T Layer A represents the Signal Strength of the received signal in percent */
  uint8_t                    signalQualityAPercent;          /* Signal Quality Percent ISDB-T Layer A represents the Signal Quality of the received signal in percent */

  BTHD_Modulation            eIsdbtBModulation;              /* ISDB-T Layer B modulation type */
  BTHD_CodeRate              eIsdbtBCodeRate;                /* ISDB-T Layer B code rate */
  BTHD_IsdbtTimeInterleaving eIsdbtBTimeInterleaving;        /* ISDB-T Layer B time interleaving */
  uint32_t                   eIsdbtBSegments;                /* ISDB-T Layer B number of segments */
  uint32_t                   ulIsdbtBRsCorrectedBlocks;      /* ISDB-T Layer B accumulated corrected RS blocks */
  uint32_t                   ulIsdbtBRsUncorrectedBlocks;    /* ISDB-T Layer B accumlated uncorrected RS blocks */
  uint32_t                   ulIsdbtBRsCleanBlocks;          /* ISDB-T Layer B accumlated clean RS blocks */
  uint32_t                   ulIsdbtBRsTotalBlocks;          /* ISDB-T Layer B accumalted total RS blocks */
  uint32_t                   ulIsdbtBViterbiUncorrectedBits; /* ISDB-T Layer B accumlated error bits after Viterbi */
  uint32_t                   ulIsdbtBViterbiTotalbits;       /* ISDB-T Layer B accumlated total bits after Viterbi */
  uint8_t                    signalLevelBPercent;            /* Signal Level Percent ISDB-T Layer B represents the Signal Strength of the received signal in percent */
  uint8_t                    signalQualityBPercent;          /* Signal Quality Percent ISDB-T Layer B represents the Signal Quality of the received signal in percent */

  BTHD_Modulation            eIsdbtCModulation;              /* ISDB-T Layer C modulation type */
  BTHD_CodeRate              eIsdbtCCodeRate;                /* ISDB-T Layer C code rate */
  BTHD_IsdbtTimeInterleaving eIsdbtCTimeInterleaving;        /* ISDB-T Layer C time interleaving */
  uint32_t                   eIsdbtCSegments;                /* ISDB-T Layer C number of segments */
  uint32_t                   ulIsdbtCRsCorrectedBlocks;      /* ISDB-T Layer C accumulated corrected RS blocks */
  uint32_t                   ulIsdbtCRsUncorrectedBlocks;    /* ISDB-T Layer C accumlated uncorrected RS blocks */
  uint32_t                   ulIsdbtCRsCleanBlocks;          /* ISDB-T Layer C accumlated clean RS blocks */
  uint32_t                   ulIsdbtCRsTotalBlocks;          /* ISDB-T Layer C accumalted total RS blocks */
  uint32_t                   ulIsdbtCViterbiUncorrectedBits; /* ISDB-T Layer C accumlated error bits after Viterbi */
  uint32_t                   ulIsdbtCViterbiTotalbits;       /* ISDB-T Layer C accumlated total bits after Viterbi */
  uint8_t                    signalLevelCPercent;            /* Signal Level Percent ISDB-T Layer C represents the Signal Strength of the received signal in percent */
  uint8_t                    signalQualityCPercent;          /* Signal Quality Percent ISDB-T Layer C represents the Signal Quality of the received signal in percent */

} BTHD_THDStatus;

/***************************************************************************
Summary:
    Structure for IFD status

Description:
    This structure contains IFD status

See Also:
    None.

****************************************************************************/
typedef struct BTHD_IfdStatus
{   
   BTHD_IfdParams      acqParams;               /* input parameters from most recent acquisition */
   bool         bHsyncLocked;                   /* true if HSYNC is locked    */
   uint32_t     pixCarrFreq;                    /* pix carrier freq in Hz */   
   int32_t      pixCarrFreqError;               /* pix carrier error in Hz */
   uint32_t     rf_agc;                         /* AGT_DELTA_SIGMA level in units of 1/10 percent */
   uint32_t     if_agc;                         /* AGI_DELTA_SIGMA level in units of 1/10 percent */
   bool         bNicamLock;                     /* True if NICAM is locked */
   uint32_t     ulNicamSnr;                     /* NICAM slicer SNR */
   uint32_t     ulNicamParityError;             /* Number of parity errors in the current frame */
   uint32_t     ulNicamErrorCount;              /* Number of bit errors in the last 128 frames */
   BTHD_NicamEncoderMode eNicamEncoderMode;     /* Current NICAM encoder mode */
   uint32_t     ulAVRatio1Hi;                   /* AV Ratio in linear units (upper 32 bits) */
   uint32_t     ulAVRatio1Lo;                   /* AV Ratio in linear units (lower 32 bits) */
   uint32_t     ulAVRatio2Hi;                   /* AV Ratio for 2nd audio carrioer in linear units (upper 32 bits) */
   uint32_t     ulAVRatio2Lo;                   /* AV Ratio for 2nd audio carrioer in linear units (lower 32 bits) */
    uint32_t    ulAudioCarrier2Variance;        /*  Carrier Average Or Variance Read of 2nd audio carrier */
   
} BTHD_IfdStatus;

/***************************************************************************
Summary:
    Structure for IFD audio settings

Description:
    This structure is used to change the audio carrier frequency control 
    word. Since there is only a finite set of possible carrier frequency
    we use the video/audio mode as index to the look up table

See Also:
    None.

****************************************************************************/
typedef struct BTHD_IfdAudioSettings
{
    BTHD_AnalogVideoMode    eVideoMode;     /* Selects analog video standards */
    BTHD_AnalogAudioMode    eAudioMode;     /* Selects analog audio standards */
    uint32_t ulCarrier2DecimationRatio;  
} BTHD_IfdAudioSettings;

/***************************************************************************
Summary:
    Callback used for event notification.

Description:
    When this PI wants to notify an application, it will call this callback
    function the callback function is registered.

See Also:
    BTHD_InstallLockStateChangeCallback(), BTHD_InstallInterruptCallback()

****************************************************************************/
typedef BERR_Code (*BTHD_CallbackFunc)(void *pParam );

typedef struct BTHD_Settings BTHD_Settings;


/***************************************************************************
Summary:
    Structure for API function table

Description:
   This structure contains pointers to all public BTHD functions.

See Also:
    None.

****************************************************************************/
typedef struct BTHD_ApiFunctTable
{
   BERR_Code (*Open)(BTHD_Handle *, BCHP_Handle, void*, BINT_Handle, const BTHD_Settings *pDefSettings);
   BERR_Code (*Close)(BTHD_Handle);
   BERR_Code (*Init)(BTHD_Handle, const uint8_t *, uint32_t);
   BERR_Code (*ReadRegister)(BTHD_Handle, uint32_t, uint32_t*);
   BERR_Code (*WriteRegister)(BTHD_Handle, uint32_t, uint32_t*);
   BERR_Code (*Mi2cWrite)(BTHD_Handle, uint8_t, uint8_t*, uint8_t);
   BERR_Code (*Mi2cRead)(BTHD_Handle, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t);
   BERR_Code (*SetAcquireParams)(BTHD_Handle, const BTHD_InbandParams*);
   BERR_Code (*GetAcquireParams)(BTHD_Handle, BTHD_InbandParams*); 
   BERR_Code (*TuneAcquire)(BTHD_Handle, const BTHD_InbandParams*);
   BERR_Code (*GetThdStatus)(BTHD_Handle, BTHD_THDStatus *);
   BERR_Code (*ResetInbandStatus)(BTHD_Handle);
   BERR_Code (*GetChipRevision)(BTHD_Handle, uint8_t*);
   BERR_Code (*GetVersion)(BTHD_Handle, uint32_t*, uint32_t*);
   BERR_Code (*GetVersionInfo)(BTHD_Handle, BFEC_VersionInfo *);   
   BERR_Code (*GetInterruptEventHandle)(BTHD_Handle, BKNI_EventHandle*);
   BERR_Code (*HandleInterrupt_isr)(BTHD_Handle);
   BERR_Code (*ProcessInterruptEvent)(BTHD_Handle);
   BERR_Code (*GetLockStateChangeEventHandle)(BTHD_Handle, BKNI_EventHandle*);
   BERR_Code (*AcquireIfd)(BTHD_Handle, const BTHD_IfdParams*);
   BERR_Code (*GetIfdStatus)(BTHD_Handle, BTHD_IfdStatus*);
   BERR_Code (*GetSoftDecisionBuf)(BTHD_Handle, int16_t*, int16_t*);
   BERR_Code (*GetDefaultInbandParams)(BTHD_InbandParams*);
   BERR_Code (*GetDefaultIfdParams)(BTHD_IfdParams*);
   BERR_Code (*ResetIfdStatus)(BTHD_Handle);
   BERR_Code (*SetIfdAudioSettings)(BTHD_Handle, const BTHD_IfdAudioSettings*);
   BERR_Code (*GetIfdAudioSettings)(BTHD_Handle, BTHD_IfdAudioSettings*);
   BERR_Code (*PowerUp)(BTHD_Handle);
   BERR_Code (*PowerDown)(BTHD_Handle);
   BERR_Code (*GetEWSEventHandle)(BTHD_Handle, BKNI_EventHandle*);
   BERR_Code (*GetThdLockStatus)(BTHD_Handle, BTHD_LockStatus *);
   BERR_Code (*GetBBSInterruptEventHandle)(BTHD_Handle, BKNI_EventHandle*);
   BERR_Code (*ProcessBBSInterruptEvent)(BTHD_Handle);   
   BERR_Code (*RequestThdAsyncStatus)(BTHD_Handle);   
   BERR_Code (*GetThdAsyncStatus)(BTHD_Handle, BTHD_THDStatus *);   
   BERR_Code (*InstallCallback)(BTHD_Handle, BTHD_Callback, BTHD_CallbackFunc, void *);   
} BTHD_ApiFunctTable;

/***************************************************************************
Summary:
    Structure for THD settings

Description:
   This structure contains the settings for the BTHD PI.

See Also:
    None.


****************************************************************************/
struct BTHD_Settings
{
   BTHD_I2cSettings      i2c;      /* i2c settings */
   BTHD_ApiFunctTable    api;
   BTMR_Handle           hTmr;     /* Timer used for IFD acqusition, not need in 2940 */
   bool                  bRfDeltaSigmaInvert; /* Inverts RF delta sigma pin */
   bool                  bIfDeltaSigmaInvert; /* Inverts IF delta sigma pin */
   BTHD_AgcPinMode       IfAgcPinMode; /* specifis the voltage swing of RF delta sigma pin */
   BMEM_Heap_Handle      hHeap;
   bool                  bIfAgcZero;   /* Turns off IF AGC in the chip */
   void *hGeneric;         /* generic handle can be used for anything */   
   bool                  supportIsdbt;
};

/******************************************************************************
Summary:
   Initializes the BTHD API.
Description:
   This function must be called first to get a BTHD_Handle.  This handle is 
   used by all other function calls in the BTHD API.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_Open(
   BTHD_Handle *h,       /* [out] BTHD handle */
   BCHP_Handle hChip,    /* [in] chip handle */
   void        *pReg,    /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt, /* [in] interrupt handle */
   const BTHD_Settings *pDefSettings /* [in] default settings */
);

/******************************************************************************
Summary:
   Closes the BTHD API.
Description:
   This function releases all the resources allocated by BTHD API and disables 
   BCM3520 host interrupts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_Close(
   BTHD_Handle h   /* [in] BTHD handle */
);


/******************************************************************************
Summary:
   Downloads the microcode and runs the AP.
Description:
   This function downloads the microcode to the AP RAM and then starts running 
   the AP.
Returns:
   BERR_Code : BERR_SUCCESS = AP is running and successfully initialized
******************************************************************************/
BERR_Code BTHD_Init(
   BTHD_Handle   h,       /* [in] BTHD handle */
   const uint8_t *pImage, /* [in] pointer to microcode image. Set to NULL to use default image */
   uint32_t imageLength   /* [in] length of image. Set to 0 when using default image */
);

/******************************************************************************
Summary:
   Reads a register.
Description:
   This function reads a register.  This function is normally used for 
   non-memory mapped BTHD devices, such as BCM3520.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_ReadRegister(
   BTHD_Handle hTHD,  /* [in] BTHD handle */
   uint32_t    reg,   /* [in] address of register to read */
   uint32_t    *val   /* [in] contains data that was read */
);


/******************************************************************************
Summary:
   Writes a register.
Description:
   This function writes to a register.  This function is normally used for 
   non-memory mapped BTHD devices, such as BCM3520.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_WriteRegister(
   BTHD_Handle hTHD,  /* [in] BTHD handle */
   uint32_t    reg,   /* [in] address of register to write */
   uint32_t    *val   /* [in] contains data to write */
);


/******************************************************************************
Summary:
   Initiates an I2C write transaction from the front end device's I2C 
   controller.
Description:
   This function programs the front end device's master i2c controller to 
   transmit the data given in buf[].  If there is no master i2c controller on 
   the front end device, this function will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_Mi2cWrite(
   BTHD_Handle h,      /* [in] BTHD handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *buf,       /* [in] specifies the data to transmit */
   uint8_t n           /* [in] number of bytes to transmit after the i2c slave address */
);


/******************************************************************************
Summary:
   Initiates an I2C read transaction from the front end device's I2C 
   controller.
Description:
   This function programs the front end device's master i2c controller to 
   transmit the data given in buf[].  If there is no master i2c controller on 
   the front end device, this function will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_Mi2cRead(
   BTHD_Handle h,      /* [in] BTHD handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *out_buf,   /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,      /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,    /* [out] holds the data read */
   uint8_t in_n        /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
);

/***************************************************************************
Summary:
   Sets the Acquire parameters for the inband channel.
Description:
   Sets the Acquire parameters for the inband channel.
Returns:
   BERR_Code

****************************************************************************/ 
BERR_Code BTHD_SetAcquireParams(
   BTHD_Handle h,      /* [in] BTHD handle */
   const BTHD_InbandParams *pParams  /* [in] inband acquisition parameters */
);

/***************************************************************************
Summary:
   Gets the Acquire parameters for the inband channel.
Description:
   Gets the Acquire parameters for the inband channel.
Returns:
   BERR_Code

****************************************************************************/ 
BERR_Code BTHD_GetAcquireParams(
   BTHD_Handle h,      /* [in] BTHD handle */
   BTHD_InbandParams *pParams  /* [out] inband acquisition parameters */  
);
    
    
/******************************************************************************
Summary:
   Acquire the inband channel.
Description:
   Tells the Thd core to start DVB-T channel acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_TuneAcquire(
   BTHD_Handle h,                    /* [in] BTHD handle */
   const BTHD_InbandParams *pParams  /* [in] inband acquisition parameters */
);

/******************************************************************************
Summary:
   Acquire the inband ananlog channel
Description:
   Tells the IFD core to start channel acqusition
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_AcquireIfd(
    BTHD_Handle h,                  /* [in] BTHD handle */
    const BTHD_IfdParams *pParams   /* [in] IFD acquisition parameters */
    );

/******************************************************************************
Summary:
   Gets IFD status.
Description:
   This function returns IFD status information.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetIfdStatus(
    BTHD_Handle h,                  /* [in] BTHD handle */
    BTHD_IfdStatus *pIfdStatus      /* [out] IFD Status structure */
    );

/******************************************************************************
Summary:
    This function gets the status asynchronously of THD module channel.

Description:
    This function is responsible for asynchronously getting the complete status 
    for a THD module channel. BTHD_RequestThdAsyncStatus() is called to notify the 
    frontend to calculate the status and notify the backend asynchronously 
    that the status is ready then BTHD_GetAsyncThdStatus() needs to be called to 
    retrieve the status.

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_RequestThdAsyncStatus(
   BTHD_Handle h  /* [in] BTHD handle */
);

/******************************************************************************
Summary:
   Gets THD Async status.
Description:
   This function returns THD Asyncstatus information.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetThdAsyncStatus(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_THDStatus *pStatus  /* [out] THD status   */
);
    
/******************************************************************************
Summary:
   Gets THD status.
Description:
   This function returns THD status information.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetThdStatus(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_THDStatus *pStatus  /* [out] THD status   */
);


/******************************************************************************
Summary:
   Resets inband FEC bit error and block counters.
Description:
   This function clears the following counters in BTHD_THDStatus/BTHD_QamStatus:
   corrErrCount, corrBlockCount, ucorrBlockCount, cleanBlockCount, and berCount
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_ResetInbandStatus(
   BTHD_Handle h  /* [in] BTHD handle */
);


/******************************************************************************
Summary:
   Gets the currents settings for the BTHD PI.
Description:
   This function returns the current settings being used on the BTHD PI.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetSettings(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_Settings *pSettings /* [out] current BTHD settings */
);


/******************************************************************************
Summary:
   Modifies the currents settings for the BTHD PI.
Description:
   This function modifies the current settings for the BTHD PI.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_SetSettings(
   BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_Settings *pSettings /* [in] new BTHD settings */
);



/******************************************************************************
Summary:
   This function returns the BTHD interrupt event handle.
Description:
   This function is used for an i2c-controlled BTHD device only.  The 
   application will need to call this function to get the event handle
   for BTHD interrupts.  The application will wait on this interrupt event.
   When this event is signalled, the application must call 
   BTHD_ProcessInterruptEvent().  For memory-mapped BTHD devices, this function
   will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetInterruptEventHandle(
   BTHD_Handle h,            /* [in] BTHD handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
);



/******************************************************************************
Summary:
   This function returns the BTHD interrupt event handle.
Description:
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetBBSInterruptEventHandle(
   BTHD_Handle h,            /* [in] BTHD handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
);

/******************************************************************************
Summary:
   Handle BTHD interrupts.
Description:
   This function must be called when the host receives an interrupt from the 
   BTHD device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_HandleInterrupt_isr(
   BTHD_Handle h   /* [in] BTHD handle */
);


/******************************************************************************
Summary:
   This function processes BTHD interrupt events.
Description:
   This function is used for an i2c-controlled BTHD device only.  The 
   application calls this function when a BTHD interrupt event has been 
   signalled.  This function will decode the interrupt and set approriate 
   events.  For memory-mapped BTHD devices, this function will return
   BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_ProcessInterruptEvent(
   BTHD_Handle hTHD  /* [in] THD device handle */
);


/******************************************************************************
Summary:
   This function processes BTHD interrupt events.
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_ProcessBBSInterruptEvent(
   BTHD_Handle hTHD  /* [in] THD device handle */
);


/******************************************************************************
Summary:
   This function reads the chip id
Description:
   This function is used to figure out which chip revision is the software
   connected to. The revision is important information as it determines which
   hexfile to download
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetChipRevision(
   BTHD_Handle hTHD,  /* [in] THD device handle */
   uint8_t* revision /* [out] Chip revision, for 3461 revision is 16 bits */
);

/******************************************************************************
Summary:
   This function reads THD software version 
Description:
   This function is used to figure out what version of the THD software is 
   currently running. The revision is important information as it determines 
   which firmware was downloaded
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetVersion(
   BTHD_Handle hTHD,  /* [in] THD device handle */
   uint32_t* version, /* [out] firmware revision, for 3461 version is lower 16 bits(e.g. Ver. 1.4 is represented by 0x00000104) */
   uint32_t* checksum /* [out] firmware checksum */
);

/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the core driver version 
    information. It return the majorVersion and minorVersion of the core
    driver.
Returns:
    BERR_Code
****************************************************************************/
BERR_Code BTHD_GetVersionInfo(
    BTHD_Handle hTHD,              /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    );   
    
/******************************************************************************
Summary:
   Returns the Lock State Change event handle.
Description:
   If the application wants to know when the downstream channel goes in lock or
   out of lock, it should use this function to get a handle to the Lock State 
   Change event.  This event is set whenever the downstream channel lock status 
   changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetLockStateChangeEventHandle(
   BTHD_Handle h,     /* [in] BTHD channel handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);

/******************************************************************************
Summary:
   Gets I/Q soft decisions for THD
Description:
   This function is mainly used for inband QAM/VSB constellation display.  This
   function returns 30 pairs of I/Q values.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetSoftDecisionBuf(
   BTHD_Handle h,  /* [in] BTHD handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
);

/******************************************************************************
Summary:
   Gets default inband param settings used in DVB-T acquire
Description:
   This function is used to get a set of default parameters that can be passed
   to the BTHD_TuneAcquire function
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetDefaultInbandParams(
    BTHD_Handle h,  /* [in] BTHD handle */
    BTHD_InbandParams* pDefInbandParam /* [out] default param */
);

/******************************************************************************
Summary:
   Gets default inband param settings used in IFD acquire
Description:
   This function is used to get a set of default parameters that can be passed
   to the BTHD_AcquireIfd function
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetDefaultIfdParams(
    BTHD_Handle h,  /* [in] BTHD handle */
    BTHD_IfdParams* pDefIfdParam /* [out] default param */
);

/******************************************************************************
Summary:
   Reset internal counters for IFD status
Description:
   This function is used to get a set of default parameters that can be passed
   to the BTHD_AcquireIfd function
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_ResetIfdStatus(
    BTHD_Handle h  /* [in] BTHD handle */
);

/******************************************************************************
Summary:
   Retrieve the current IFD audio carrier settings
Description:
   This function is used to get current audio carrier settings
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetIfdAudioSettings(
    BTHD_Handle h,                          /* [in] BTHD handle */
    BTHD_IfdAudioSettings* audioSettings    /* [out] Audio settings structure */
);

/******************************************************************************
Summary:
   Sets the current IFD audio carrier settings
Description:
   This function is used to set current audio carrier settings after acqusition. 
   These settings will be overwritten on an IFDAcquire command.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_SetIfdAudioSettings(
    BTHD_Handle h,                         /* [in] BTHD handle */
    const BTHD_IfdAudioSettings* audioSettings   /* [in] Audio settings structure */
);

/******************************************************************************
Summary:
   Turn On clocks to all frontend related cores 
Description:
   This function is used by dynamic power management to essentially power up
   all frontend related cores.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_PowerUp(
    BTHD_Handle h                         /* [in] BTHD handle */
);

/******************************************************************************
Summary:
   Turn off clocks to all frontend related cores include AFE, THD, IFD and PLLs
Description:
   This function is used by dynamic power management to essentially shut off
   all frontend related cores. To bring core back the application needs to call
   either acquire function or the init function
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_PowerDown(
    BTHD_Handle h                         /* [in] BTHD handle */
);

/******************************************************************************
Summary:
   Get a handle for EWS
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetEWSEventHandle(   BTHD_Handle h,     /* [in] BTHD channel handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);

/******************************************************************************
Summary:
   Get the THD lock status without pulling all other status
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTHD_GetThdLockStatus(  BTHD_Handle h,           /* [in] BTHD handle */
   BTHD_LockStatus *pLockStatus /* [out] THD lock status   */
);

/***************************************************************************
Summary:
    This function is responsible for installing a callback function.

Description:
    This function installs a callback function.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTHD_InstallCallback(
    BTHD_Handle h,            /* [in] Device channel handle */
    BTHD_Callback callbackType,  /* [in] Type of callback */
    BTHD_CallbackFunc pCallback, /* [in] Function Ptr to callback */
    void *pParam                 /* [in] Generic parameter send on callback */
    );
#ifdef __cplusplus
}
#endif

#endif /* BTHD_H__ */

