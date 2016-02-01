/***************************************************************************
 *     Copyright (c) 2004-2010, Broadcom Corporation
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
The BVSB portinginterface controls the front-end block on any Broadcom chip
that has a VSB core, such as BCM3520 and BCM3560.  BVSB PI may also decode
other signals such as QAM, NTSC, and/or OOB, depending on the capabilities 
of the VSB chip.  The VSB core contains an Acquisition Processor (AP) that 
is responsible for channel acquisition and status.  The BVSB PI controls
the AP by writing commands to an internal dual-access Host Access Buffer 
(HAB).  The AP services the HAB command and returns any response data back
onto the HAB. 

The magnum/portinginterface/vsb/bvsb.h header file defines a standard VSB 
front-end API for all Broadcom VSB chips.  The implementation of the API for
each chip is in the magnum/portinginterface/vsb/<chip> directory.  


Interrupt Requirements
The application needs to call BVSB_HandleInterrupt_isr() when it receives
the VSB L1 interrupt.


Sample Code
//
// NOTE: This is sample code for a system that contains dual-channel VSB
//       using BCM3560 and BCM3520.  This code does not do any error 
//       checking.
//
#include "bvsb.h"
#include "bvsb_3520.h"
#include "bvsb_3560.h"

static BVSB_Handle      hVSB1;  // handle for BCM3560 VSB core
static BVSB_Handle      hVSB2;  // handle for BCM3520 VSB core
static BCHP_Handle      hCHP;
static BREG_I2C_Handle  hReg;

// These are pointers to AP microcode used in BCM3560/BCM3520.  
// The microcode will vary depending on chip/board/customer, so it is not 
// part of the BVSB PI.
extern const uint8_t *pBcm3520ApImage;
extern const uint8_t *pBcm3560ApImage;

void main(void)
{
   BVSB_Settings     settings1, settings2;
   BVSB_InbandParams inbandParams;

   // do initialization
   // (get BCHP_Handle, BREG_I2C_Handle, etc)
   ...

   // BVSB initialization for BCM3560
   BVSB_3560_GetDefaultSettings(&settings1);
   BVSB_Open(&hVSB, hChip, hReg, &settings);

   // enable the BCM3560 VSB host interrupt and map that interrupt to a function
   // that will call the BVSB PI function BVSB_HandleInterrupt_isr()
   ...

   // BVSB initialization for BCM3520
   BVSB_3520_GetDefaultSettings(&settings2);
   settings2.i2c.chipAddr = BCM3520_I2C_ADDR;
   settings2.i2c.interruptEnableFunc = bcm3520_enable_irq;
   settings2.i2c.interruptEnableFuncParam = NULL;

   // enable the BCM3520 host interrupt and map that interrupt to a function
   // that will call the BVSB PI function BVSB_HandleInterrupt_isr()
   ...

   // start a thread that runs bcm3520_background()
   ...

   // download the BCM3520 microcode and run the AP
   BVSB_InitAp(hVSB1, pBcm3560ApImage);
   BVSB_InitAp(hVSB2, pBcm3520ApImage);

   // tune the tuner
   ...

   // acquire 8VSB on BCM3560
   inbandParams1.mode = BVSB_InbandMode_e8VSB;
   inbandParams1.symbolRateOffset = 0;
   inbandParams1.ifFreqOffset = 0;
   BVSB_AcquireInband(hVSB1, &inbandParams1);
   
   // acquire 64-QAM Annex B on BCM3520
   inbandParams1.mode = BVSB_InbandMode_e64QAM_B;
   inbandParams1.symbolRateOffset = 0;
   inbandParams1.ifFreqOffset = 0;   
   BVSB_AcquireInband(hVSB2, &inbandParams2);

   ...

   close_vsb:
   // disable VSB L1 interrupts
   // terminate bcm3520_background task
   ...
   BVSB_Close(hVSB1);
   BVSB_Close(hVSB2);

   program_done:
   ...
}

// The application must call BVSB_ProcessInterruptEvent() every time the VSB
// interrupt event is set.
void bcm3520_background()
{
   // get handle to the BCM3520 interrupt event
   BVSB_GetInterruptEventHandle(hVSB, &event);

   while (1)
   {
      BKNI_WaitForEvent(event, BKNI_INFINTE);
      BVSB_ProcessInterruptEvent(hVSB);
   }
}


// This is an example of a callback function for enabling/disabling the L1
// interrupt.
void vsb_enable_irq(bool b, void *p)
{
   if (b)
      CPUINT1_Enable(VSB_IRQ);   // enable the L1 interrupt
   else
      CPUINT1_Disable(VSB_IRQ);  // disable the L1 interrupt
}
</verbatim>
========================================================================*/

#ifndef BVSB_H__
#define BVSB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "bint.h"
#include "btmr.h"
#include "berr_ids.h"
#include "bfmt.h"


/***************************************************************************
Summary:
	BVSB error codes

Description:

See Also:

****************************************************************************/
#define BVSB_ERR_AP_FAIL       BERR_MAKE_CODE(BERR_VSB_ID, 0)  /* This is a BERR_Code to indicate that host was unable to reset/idle/run the AP. */
#define BVSB_ERR_HOST_XFER     BERR_MAKE_CODE(BERR_VSB_ID, 1)  /* This is a BERR_Code to indicate a host transfer error (underrun/overrun). */
#define BVSB_ERR_IOMB_XFER     BERR_MAKE_CODE(BERR_VSB_ID, 2)  /* This is a BERR_Code to indicate an IO Mailbox ISB transfer error. */
#define BVSB_ERR_MEMAV         BERR_MAKE_CODE(BERR_VSB_ID, 3)  /* This is a BERR_Code to indicate a memory access violation. */
#define BVSB_ERR_AP_EEPROM     BERR_MAKE_CODE(BERR_VSB_ID, 4)  /* This is a BERR_Code to indicate no i2c ack from EEPROM. */
#define BVSB_ERR_AP_NOT_INIT   BERR_MAKE_CODE(BERR_VSB_ID, 5)  /* This is a BERR_Code to indicate that the AP has failed to initialize. */
#define BVSB_ERR_HABAV         BERR_MAKE_CODE(BERR_VSB_ID, 6)  /* This is a BERR_Code to indicate an HAB access violation. */
#define BVSB_ERR_HAB_NOT_AVAIL BERR_MAKE_CODE(BERR_VSB_ID, 7)  /* This is a BERR_Code to indicate that the HAB is busy. */
#define BVSB_ERR_HAB_ERR       BERR_MAKE_CODE(BERR_VSB_ID, 8)  /* This is a BERR_Code to indicate an HAB command format error. */
#define BVSB_ERR_HAB_NO_ACK    BERR_MAKE_CODE(BERR_VSB_ID, 9)  /* This is a BERR_Code to indicate that the HAB command was not processed by AP. */
#define BVSB_ERR_HAB_FAIL      BERR_MAKE_CODE(BERR_VSB_ID, 10) /* This is a BERR_Code to indicate bad/unexpected HAB data read. */
#define BVSB_ERR_AP_HABAV      BERR_MAKE_CODE(BERR_VSB_ID, 11) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_AP_COPY       BERR_MAKE_CODE(BERR_VSB_ID, 12) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_AP_IRQ        BERR_MAKE_CODE(BERR_VSB_ID, 13) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_AP_SCR        BERR_MAKE_CODE(BERR_VSB_ID, 14) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_AP_ISB        BERR_MAKE_CODE(BERR_VSB_ID, 15) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_AP_WD         BERR_MAKE_CODE(BERR_VSB_ID, 16) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_AP_STACK      BERR_MAKE_CODE(BERR_VSB_ID, 17) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_AP_BSC        BERR_MAKE_CODE(BERR_VSB_ID, 18) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_IOMB_BUSY     BERR_MAKE_CODE(BERR_VSB_ID, 19) /* This is a BERR_Code to indicate the IO MBOX was busy. */
#define BVSB_ERR_HAB_TIMEOUT   BERR_MAKE_CODE(BERR_VSB_ID, 20) /* This is a BERR_Code to indicate an HAB timeout. */
#define BVSB_ERR_AP_UNKNOWN    BERR_MAKE_CODE(BERR_VSB_ID, 21) /* This is a BERR_Code to indicate an AP-internal error. */
#define BVSB_ERR_MI2C_NO_ACK   BERR_MAKE_CODE(BERR_VSB_ID, 22) /* This is a BERR_Code to indicate no ack from i2c slave device (e.g. tuner). */
#define BVSB_ERR_POWER_DOWN   BERR_MAKE_CODE(BERR_VSB_ID, 23) /* This is a BERR_Code to indicate that an API was called while cores are powered down */
#define BVSB_ERR_INVALID_ANALOG_MODES	BERR_MAKE_CODE(BERR_THD_ID, 0x18) /* This is a BERR_Code to indicate the video and audio modes selected is not valid */ 
#define BVSB_ERR_INVALID_STATUS_CALL	BERR_MAKE_CODE(BERR_THD_ID, 0x19) /* This is a BERR_Code to indicate the caller is requesting a status for a mode that is different the one the receiver is currently in */



/***************************************************************************
Summary:
	The handle for vsb module.

Description:
	This is an opaque handle that is used to access the BVSB API.

See Also:
	BVSB_Open()

****************************************************************************/
typedef struct BVSB_P_Handle *BVSB_Handle;

/***************************************************************************
Summary:
	Enumeration for phase loop bandwidth

Description:
	This enum specifies the phase tracking loop bandwidth.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_PhaseLoopBw
{
   BVSB_PhaseLoopBw_eLow,     /* lower loop bandwidth */
   BVSB_PhaseLoopBw_eMedium,  /* medium loop bandwidth */
   BVSB_PhaseLoopBw_eHigh     /* higher loop bandwidth */
} BVSB_PhaseLoopBw;


/***************************************************************************
Summary:
	Enumeration for QAM interleave depth.

Description:
	This enum specifies the interleaver depth in the QAM receiver.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_IDepth
{
   BVSB_IDepth_e204_1 = 0,  /* (204,1) */
   BVSB_IDepth_e102_2,      /* (102,2) */
   BVSB_IDepth_e68_3,       /* (68,3)  */
   BVSB_IDepth_e51_4,       /* (51,4)  */
   BVSB_IDepth_e34_6,       /* (34,6)  */
   BVSB_IDepth_e17_12,      /* (17,12) */
   BVSB_IDepth_e12_17,      /* (12,17) */
   BVSB_IDepth_e6_34,       /* (6,34)  */
   BVSB_IDepth_e4_51,       /* (4,51)  */
   BVSB_IDepth_e3_68,       /* (3,68)  */
   BVSB_IDepth_e2_102,      /* (2,102) */
   BVSB_IDepth_e1_204       /* (1,204) */
} BVSB_IDepth;

/***************************************************************************
Summary:
	Enumeration for QAM Nyquist filter rolloff.

Description:
	This enum specifies the QAM receiver's Nyquist filter rolloff.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_NyquistFilter
{
   BVSB_NyquistFilter_e12, /* 12% Nyquist filter */
   BVSB_NyquistFilter_e15, /* 15% Nyquist filter */
   BVSB_NyquistFilter_e18  /* 18% Nyquist filter */
} BVSB_NyquistFilter;
/***************************************************************************
Summary:
	Enumeration for NTSC pull-in range.

Description:
	This enum specifies the pull-in range of IFD.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_Ifd_PullInRange
{
   BVSB_PullInRange_eWide,     /* Normal wide pull-in range mode*/
   BVSB_PullInRange_e282KHz,   /* +/- 250 + 32 KHz mode*/
   BVSB_PullInRange_e32kHz,     /* +/- 32 KHz mode*/
   BVSB_PullInRange_e250kHz= BVSB_PullInRange_e282KHz ,
   BVSB_PullInRange_e500kHz,
   BVSB_PullInRange_e750kHz,
   BVSB_PullInRange_e1000kHz,
   BVSB_PullInRange_e1100kHz = BVSB_PullInRange_eWide
} BVSB_Ifd_PullInRange;

/***************************************************************************
Summary:
	Enumeration for analog mode.

Description:
	This enum specifies the analog mode: NTSC/PAL-D/PAL-I/PAL-BGH.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_AnalogMode
{
   BVSB_AnalogMode_eNTSC,
   BVSB_AnalogMode_ePalD,
   BVSB_AnalogMode_ePalI,
   BVSB_AnalogMode_ePalBGH
} BVSB_AnalogMode;



/***************************************************************************
Summary:
    Define different country code.
Description:
    This enum specifies the country code for BTSC.
See Also:
    BVSB_NtscSettings
****************************************************************************/
typedef enum BVSB_BtscCountry
{
    BVSB_BtscCountry_eUS,                       /* BTSC US */
    BVSB_BtscCountry_eUS_High_Deviation,        /* BTSC US high deviation */
    BVSB_BtscCountry_eKorea                     /* BTSC Korea */
} BVSB_BtscCountry;

/***************************************************************************
Summary:
	Enumeration for pga gain.

Description:
	This enum specifies the pga gain.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_PgaGain
{
   BVSB_PgaGain_e0dB,     
   BVSB_PgaGain_e1dB,   /* This option is not available on 3563,3520 */  
   BVSB_PgaGain_e2dB,   /* This option is not available on 3563,3520 */  
   BVSB_PgaGain_e3dB, 
   BVSB_PgaGain_e4dB,   /* This option is not available on 3563,3520 */     
   BVSB_PgaGain_e5dB,   /* This option is not available on 3563,3520 */ 
   BVSB_PgaGain_e6dB, 
   BVSB_PgaGain_e9dB,   /* This option is not available on 3549 */
   BVSB_PgaGain_e12dB,  /* This option is not available on 3549 */
   BVSB_PgaGain_e15dB   /* This is option not available on 3549 */
} BVSB_PgaGain;

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
typedef enum BVSB_AnalogVideoMode {
	BVSB_AnalogVideoMode_eNtsc_M,	/* NTSC-M mode. can be combined with BTSC or A2 audio */
	BVSB_AnalogVideoMode_eNtsc_N,	/* NTSC-N mode, can be combined with only BTSC audio */
	BVSB_AnalogVideoMode_ePal_M,    /* PAL-M mode, can be combined with only BTSC audio */
	BVSB_AnalogVideoMode_ePal_N,	/* PAL-N mode, can be combined with only BTSC audio */
	BVSB_AnalogVideoMode_ePal_I,	/* PAL-I mode, can be combined with only NICAM audio */
	BVSB_AnalogVideoMode_ePal_BGH,  /* PAL-B/G/H mode, can be combined with A2 or NICAM audio */
	BVSB_AnalogVideoMode_ePal_DK,   /* PAL-D/K mode, can be combined with NICAM audio only */
	BVSB_AnalogVideoMode_ePal_DK1,  /* PAL-D/K1 mode, can be combined with A2 audio only */
	BVSB_AnalogVideoMode_ePal_DK2,  /* PAL-D/K2 mode, can be combined with A2 audio only */
	BVSB_AnalogVideoMode_ePal_DK3,  /* PAL-D/K3 mode, can be combined with A2 audio only */
	BVSB_AnalogVideoMode_eSecam_BGH,/* SECAM-B/G/H mode, can be combined with A2 or NICAM audio */
	BVSB_AnalogVideoMode_eSecam_DK, /* SECAM-D/K mode, can be combined with only NICAM audio */
	BVSB_AnalogVideoMode_eSecam_L   /* SECAM-L mode, can be combined with onl y NICAM audio */
} BVSB_AnalogVideoMode;

/***************************************************************************
Summary:
	Enumeration for analog audio modes

Description:
	This enum specifies second audio carrier mode options for the IFD decoder. 
	This only applies to IFD core in cases where this PI controls both ofdm 
	an analog video receiver. Not all video/audio combo are valid, refer to 
	each video mode for more options

See Also:
	BVSB_AnalogVideoMode

****************************************************************************/
typedef enum BVSB_AnalogAudioMode {
	BVSB_AnalogAudioMode_eBtsc,		/* Selects BTSC audio */
	BVSB_AnalogAudioMode_eA2,		/* Selects A2 audio */
	BVSB_AnalogAudioMode_eNicam		/* Selects NICAM audio */
} BVSB_AnalogAudioMode;

/***************************************************************************
Summary:
	Enumeration for IFD acquire mode

Description:
	This enum specifies whether IFD acquistion will spend extra time to 
    improve acquisition robustness

See Also:
	BVSB_AnalogVideoMode

****************************************************************************/
typedef enum BVSB_IfdAcquisitionMode
{
    BVSB_IfdAcquisitionMode_eSlow,
    BVSB_IfdAcquisitionMode_eFast
} BVSB_IfdAcquisitionMode;


/***************************************************************************
Summary:
	Structure for VSB settings.

Description:
	This structure contains settings for inband VSB acquisitions.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_VsbSettings
{
   BVSB_PhaseLoopBw bw; /* phase tracking loop bandwidth setting           */  
   bool bAutoAcq;   /* auto reacquisition: false=disabled, true=enabled    */
   bool bFastAcq;   /* fast acquisition: false=disabled, true=enabled      */
   bool bTei;       /* xport error indicator: false=disabled, true=enabled */
   bool bTerr;      /* receiver mode: true=terrestrial, false=cable        */
   bool bNtscSweep; /* NTSC filter sweep: false=disabled, true=enabled     */
   bool bRfiSweep;  /* RFI filter sweep: false=disabled, true=enabled      */
   BVSB_PgaGain pgaGain;  /* pga gain in dB */
} BVSB_VsbSettings;

/***************************************************************************
Summary:
	Structure for QAM settings.

Description:
	This structure contains settings for inband QAM acquisitions.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_QamSettings
{
   BVSB_PhaseLoopBw   bw; /* phase tracking loop bandwidth setting */  
   BVSB_IDepth        idepth;   /* interleaver depth (Annex A only) */
   BVSB_NyquistFilter nyquist; /* specifies Nyquist filter rolloff */
   bool bAutoAcq; /* auto reacquisition: false=disabled, true=enabled */
   bool bFastAcq; /* fast acquisition: false=disabled, true=enabled */
   bool bTerr;    /* receiver mode: true=terrestrial, false=cable */
   bool bEq;      /* equalizer size: false=24-tap FFE and 24-tap DFE, true=192-tap FFE and 132-tap DFE */
   bool bCh;      /* false=use 6MHz channelization, true=use 8MHz channelization */
   bool bTei;     /* transport error indicator: false=disabled, true=enabled */
   bool bSpinv;   /* auto invert spectrum: false=disabled, true=enabled */ 
   bool bDavic;   /* true=select DAVIC unmapper, false=select DVB unmapper */
   BVSB_PgaGain pgaGain;  /* pga gain in dB */
} BVSB_QamSettings;


/***************************************************************************
Summary:
	Structure for NTSC settings.

Description:
	This structure contains settings for inband NTSC acquisitions.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_NtscSettings
{
   BVSB_PhaseLoopBw bw; /* phase tracking loop bandwidth setting         */  
   bool bFastAcq;  /* fast acquisition: false=disabled, true=enabled     */
   bool bTerr;     /* receiver mode: true=terrestrial, false=cable       */
   bool bAutoAcq;  /* auto reacquisition: false=disabled, true=enabled   */
   BVSB_Ifd_PullInRange ifd_pullin_range;  /* IFD pull in range settings: BVSB_PullInRange_eWide = +/- 1.15MHz, BVSB_PullInRange_e282KHz = +/- 282KHz, BVSB_PullInRange_e32KHz = +/- 32KHz*/
   unsigned ifd_picture_carrier;  /* IFD picture carrier frequency (Hz) */
   BVSB_AnalogMode analog_mode;
   BVSB_BtscCountry btsc_country; /* BTSC country code */
   BVSB_PgaGain pgaGain;  /* pga gain in dB */
#ifdef NOT_USED
   uint8_t delay;          /* Delay in ms that the acquisition script will wait for the AGC hardware to settle */
   uint8_t rfBandwidth;    /* RF AGC band width */
   uint8_t ifBandwidth;    /* IF AGC band width */	
   uint16_t top;           /* tuner TOP point */
   bool bSpectrumInvert; /* Invert input spectrum, only used in SECAM-L mode to get SECAM-L' */
#endif

   
} BVSB_NtscSettings;

/***************************************************************************
Summary:
	Structure for out of band settings.

Description:
	This structure contains settings for inband out of band acquisitions.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_OobSettings
{
   BVSB_PhaseLoopBw bw; /* phase tracking loop bandwidth setting          */  
   bool bAutoAcq;   /* auto reacquisition: false=disabled, true=enabled   */
   bool bSpinv;     /* auto invert spectrum: false=disabled, true=enabled */ 
   bool bBypassFEC; /* bypass FEC for POD applications: true=bypass the FEC, false=use DVS-167 or DVS-178 FEC */
} BVSB_OobSettings;


/***************************************************************************
Summary:
	Structure for inband output interface

Description:
	This structure contains settings for inband output interface.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_InbandOiSettings
{
   bool bHead4;   /* parallel: 0=sync is 1 byte wide, 1=sync is 4 bytes wide; serial: see bSync1 */
   bool bSync1;   /* serial only: 1=sync is 1 bit wide; 0=sync is 32 bits wide if bHead4 is true else sync is 8 bits wide */
   bool bXBERT;   /* false=DS_PS_CLK runs continuously, true=DS_PS_CLK suppressed when DS_PS_SYNC active */
   bool bErrinv;  /* false=DS_PS_ERR is normal, true=DS_PS_ERR is inverted */
   bool bSyncinv; /* false=DS_PS_SYNC is normal, true=DS_PS_SYNC is inverted */
   bool bVldinv;  /* false=DS_PS_VALID is normal, true=DS_PS_VALID is inverted */
   bool bClksup;  /* false=DS_PS_CLK runs continuously, true=DS_PS_CLK suppressed when PS_VALID not active */
   bool bClkinv;  /* false=DS_PS_CLK is normal, true=DS_PS_CLK is inverted */
   bool bSerial;  /* true=serial data out, false=parallel data out */
} BVSB_InbandOiSettings;

/***************************************************************************
Summary:
	Enum for BTSC output

Description:
	This enum specifies the BTSC output mode.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_BtscDecodeMode
{
   BVSB_BtscDecodeMode_eMono,      /* Mono     */
   BVSB_BtscDecodeMode_eStereo,    /* Stereo   */
   BVSB_BtscDecodeMode_eSAP,       /* SAP      */
   BVSB_BtscDecodeMode_eSAP_Mono   /* SAP/Mono */
} BVSB_BtscDecodeMode;

/***************************************************************************
Summary:
	Enum for audio sample rate

Description:
	This enum specifies the audio sample rate.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_BtscSampleRate
{
   BVSB_BtscSampleRate_e32KHz,      /* 32 KHz   */
   BVSB_BtscSampleRate_e44_1KHz,    /* 44.1 KHz */
   BVSB_BtscSampleRate_e48KHz       /* 48 KHz   */
} BVSB_BtscSampleRate;

/***************************************************************************
Summary:
	Enum for audio DAC pads

Description:
	This enum specifies the data source to the Audio DAC pads.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_BtscDacSelect
{
   BVSB_BtscDacSelect_eAudDac,   /* Audio DAC */
   BVSB_BtscDacSelect_eHiFiDac   /* HiFi DAC  */
} BVSB_BtscDacSelect;

/***************************************************************************
Summary:
	Structure for BTSC configuration

Description:
	This structure contains BTSC configuration parameters.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_BtscSettings
{
   BVSB_BtscDecodeMode decodeMode; /* decode mode for the BTSC receiver */
   BVSB_BtscSampleRate sampleRate; /* audio sample rate                 */
   bool                bI2sOut;    /* primary i2s output selection: true=pads used for i2s output, false=pads used for audio DAC */
} BVSB_BtscSettings;

/***************************************************************************
Summary:
	Structure for I2C configuration

Description:
	This structure contains the settings for I2C control.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_I2cSettings
{
   uint16_t chipAddr;  /* BCM3520 i2c chip address (i2c) */
   void     (*interruptEnableFunc)(bool, void*); /* callback function for enabling/disabling L1 interrupt */
   void     *interruptEnableFuncParam; /* parameter passed to interruptEnableFunc */
} BVSB_I2cSettings;

/***************************************************************************
Summary:
	Typedef for status information

Description:
	This typedef contains AP status information

See Also:
	None.

****************************************************************************/
typedef uint32_t BVSB_ApStatus;

/* bit definitions for BVSB_ApStatus */
#define BVSB_APSTATUS_RESET      0x00000001 /* AP is in reset state */
#define BVSB_APSTATUS_IDLE       0x00000002 /* AP is in idle state */
#define BVSB_APSTATUS_RUN        0x00000000 /* AP is running */
#define BVSB_APSTATUS_RUN_MASK   0x00000003 /* AP execution state mask */
#define BVSB_APSTATUS_HAB_DONE   0x00000400 /* HAB done */
#define BVSB_APSTATUS_HAB_ERR    0x00001000 /* HAB access violation */
#define BVSB_APSTATUS_MEM_ERR    0x00002000 /* memory access violation */
#define BVSB_APSTATUS_IOMB_ERR   0x00004000 /* io_mbox isb transfer error */
#define BVSB_APSTATUS_H_ERR      0x00008000 /* host transfer error */
#define BVSB_APSTATUS_INIT_DONE  0x00800000 /* AP init done */
#define BVSB_APSTATUS_LOCK       0x00400000 /* inband lock status */
#define BVSB_APSTATUS_HAB_MASK   0x00800403 /* HAB available condition mask */
#define BVSB_APSTATUS_HAB_READY  0x00800400 /* HAB available condition */
#define BVSB_APSTATUS_HABCMD_ERR 0x00080000 /* HAB command syntax error */
#define BVSB_APSTATUS_AP_ERR     0x00040000 /* internal AP error */

/***************************************************************************
Summary:
	Enum for signal detect type

Description:
	This enum specifies the type for signal detect

See Also:
	None.

****************************************************************************/
typedef enum BVSB_DetectType
{
   BVSB_DetectType_ePresent,              /* For only Signal/No Signal */
   BVSB_DetectType_eVSB,                    /* For Signal/No Signal + VSB */
   BVSB_DetectType_eNTSC,                  /* For Signal/No Signal + NTSC */
   BVSB_DetectType_eNTSC_VSB,           /* For Signal/No Signal + NTSC + VSB */
   BVSB_DetectType_eQAM,                   /* For Signal/No Signal + QAM */
   BVSB_DetectType_eNTSC_QAM,          /* For Signal/No Signal + NTSC + QAM */
   BVSB_DetectType_eNTSC_VSB_QAM   /* For Signal/No Signal + NTSC + VSB + QAM */
} BVSB_DetectType;

/***************************************************************************
Summary:
	Enum for signal detect mode

Description:
	This enum specifies the mode for signal detect

See Also:
	None.

****************************************************************************/
typedef enum BVSB_SignalMode
{
   BVSB_SignalMode_eVSB,		/*VSB */
   BVSB_SignalMode_eQAM,		/* QAM */
   BVSB_SignalMode_eQAM_64,     /* QAM 64*/
   BVSB_SignalMode_eQAM_256,    /* QAM 256*/
   BVSB_SignalMode_eNTSC,		/* NTSC */
   BVSB_SignalMode_eUnknown,
   BVSB_SignalMode_eNoSignal
} BVSB_SignalMode;

/***************************************************************************
Summary:
	Enumeration for NICAM audio recevier current encoder mode

Description:
	This enum specifies the NICAM encoder modes

See Also:
	None.

****************************************************************************/
typedef enum BVSB_NicamEncoderMode
{
	BVSB_Stereo_BackupFm=1,
	BVSB_DualMono_BackupFm=5,
	BVSB_Stereo_NoBackup=0,
	BVSB_DualMono_NoBackup=4,
	BVSB_MonoData_NoBackup=8,
	BVSB_MonoData_BackupFm=9,
	BVSB_Data_NoBackup=12,
	BVSB_Data_BackupFm=13
} BVSB_NicamEncoderMode;

/***************************************************************************
Summary:
	Enum for IFD deviation modes

Description:
	This enum specifies deviation mode for the IFD core

See Also:
	None.

****************************************************************************/
typedef enum BVSB_AnalogAudioDeviation
{
    BVSB_AnalogAudioDeviation_eNormal,
    BVSB_AnalogAudioDeviation_eMedium,
    BVSB_AnalogAudioDeviation_eHigh
} BVSB_AnalogAudioDeviation;

/***************************************************************************
Summary:
	Enum for inband mode

Description:
	This enum specifies the mode for the inband receiver

See Also:
	None.

****************************************************************************/
typedef enum BVSB_InbandMode
{
   BVSB_InbandMode_e8VSB,      /* 8-VSB */
   BVSB_InbandMode_e16VSB,     /* 16-VSB */
   BVSB_InbandMode_e1024QAM_B, /* 1024-QAM Annex B */
   BVSB_InbandMode_e512QAM_B, /* 512-QAM Annex B */
   BVSB_InbandMode_e256QAM_B,  /* 256-QAM Annex B */
   BVSB_InbandMode_e128QAM_B,  /* 128-QAM Annex B */
   BVSB_InbandMode_e64QAM_B,   /* 64-QAM Annex B  */
   BVSB_InbandMode_e64_256QAM_B, /* Automatic 64/256-QAM Annex B (This is not available on all chipsets) */
   BVSB_InbandMode_e256QAM_A,  /* 256-QAM Annex A */
   BVSB_InbandMode_e128QAM_A,  /* 128-QAM Annex A */
   BVSB_InbandMode_e64QAM_A,   /* 64-QAM Annex A  */
   BVSB_InbandMode_e32QAM_A,   /* 32-QAM Annex A  */
   BVSB_InbandMode_e16QAM_A,   /* 16-QAM Annex A  */
   BVSB_InbandMode_eAnalog,
   BVSB_InbandMode_eNTSC = BVSB_InbandMode_eAnalog,
   BVSB_InbandMode_eAnalog_SIF,
   BVSB_InbandMode_eNTSC_SIF = BVSB_InbandMode_eAnalog_SIF,
   BVSB_InbandMode_eUnknown
} BVSB_InbandMode;

/***************************************************************************
Summary:
	Structure for inband acquisition

Description:
	This structure contains parameters used for an inband channel acquisition.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_InbandParams
{
   BVSB_InbandMode mode; 
   int32_t         ifFreqOffset;     /* IF frequency offset in Hz; for IFD, this parameter is the pix carrier offset */
   int32_t         symbolRateOffset; /* symbol rate offset in sym/sec for VSB and QAM; not applicable in IFD */
   int32_t		   symbolRate;	/* symbol rate for QAM Annex A*/

   BVSB_AnalogVideoMode	eVideoMode;		/* Selects analog video standards. Only applies when analog mode is used */
   BVSB_AnalogAudioMode eAudioMode;     /* Selects analog audio standards. Only applies when analog mode is used */
   BVSB_AnalogAudioDeviation eAnalogAudioDeviation;      /* Selects deviation mode for analog audio */
   BVSB_IfdAcquisitionMode   eAcquisitionMode;           /* Select IFD acquisition mode */
   bool                      ifdAgcControl; /* Gives control of AGC pins to IFD only applies when using SIF mode */
   
} BVSB_InbandParams;

/***************************************************************************
Summary:
	Structure for VSB status

Description:
	This structure contains VSB status

See Also:
	None.

****************************************************************************/
typedef struct BVSB_VsbStatus
{
   BVSB_InbandParams acqParams;       /* input parameters from most recent acquisition */
   BVSB_VsbSettings  acqSettings;     /* VSB acquisition settings */
   bool              bFecLocked;      /* true if the FEC is locked */
   bool              bPllLocked;      /* true if the receiver output PLL is locked */
   bool              bNtscEngaged;    /* true if NTSC co-channel has been detected and the NTSC filter has been engaged */ 
   bool              bRfiEngaged;     /* true if RFI has been detected and the RFI filter has been engaged */
   uint32_t          ifFreq;          /* standard IF frequency in Hz */
   int32_t           ifFreqError;     /* IF frequency error in Hz */
   uint32_t          symbolRate;      /* standard symbol rate in sym/sec */
   int32_t           symbolRateError; /* symbol rate error in sym/sec */
   uint32_t          corrErrCount;    /* accumulated correctable bit errors by FEC */
   uint32_t          corrBlockCount;  /* accumulated correctable RS blocks */
   uint32_t          ucorrBlockCount; /* accumulated uncorrectable RS blocks */
   uint32_t          cleanBlockCount; /* accumulated clean RS blocks */
   uint32_t          berCount;        /* accumulated internal BERT error count */
   uint32_t          reacqCount;      /* number of reacquisitions performed by AP */
   uint32_t          rf_agc;          /* AGT_DELTA_SIGMA level in units of 1/10 percent */
   uint32_t          if_agc;          /* AGI_DELTA_SIGMA level in units of 1/10 percent */
   uint16_t          snr;             /* SNR in 1/256 dB */
   int16_t           agf;             /* int AGC gain in 1/256 dB */
   uint32_t			 PreRS;			  /* Pre RS vaule */
   uint32_t			 PostRS;		  /* Post RS vaule */
   uint32_t			 BER;			  /* BER value */
} BVSB_VsbStatus;

/***************************************************************************
Summary:
	Structure for QAM status

Description:
	This structure contains QAM status

See Also:
	None.

****************************************************************************/
typedef struct BVSB_QamStatus
{
   BVSB_InbandParams acqParams;       /* input parameters from most recent acquisition */
   BVSB_QamSettings  acqSettings;     /* QAM acquisition settings */
   bool              bFecLocked;      /* true if the FEC is locked */
   bool              bPllLocked;      /* true if the receiver output PLL is locked */
   bool              bSpinv;          /* true if spectrum inverted */
   uint32_t          ifFreq;          /* standard IF frequency in Hz */
   int32_t           ifFreqError;     /* IF frequency error in Hz */
   uint32_t          symbolRate;      /* standard symbol rate in sym/sec */
   int32_t           symbolRateError; /* symbol rate error in sym/sec */
   uint32_t          corrErrCount;    /* accumulated correctable bit errors by FEC */
   uint32_t          corrBlockCount;  /* accumulated correctable RS blocks */
   uint32_t          ucorrBlockCount; /* accumulated uncorrectable RS blocks */
   uint32_t          cleanBlockCount; /* accumulated clean RS blocks */
   uint32_t          berCount;        /* accumulated internal BERT error count */
   uint32_t          reacqCount;      /* number of reacquisitions performed by AP */   
   uint32_t          rf_agc;          /* AGT_DELTA_SIGMA level in units of 1/10 percent */
   uint32_t          if_agc;          /* AGI_DELTA_SIGMA level in units of 1/10 percent */
   uint16_t          snr;             /* SNR in 1/256 dB */
   int16_t           agf;             /* int AGC gain in 1/256 dB */
   uint32_t          BER;			  /* BER value */
} BVSB_QamStatus;

/***************************************************************************
Summary:
	Structure for NTSC status

Description:
	This structure contains NTSC status

See Also:
	None.

****************************************************************************/
typedef struct BVSB_NtscStatus
{   
   BVSB_InbandParams   acqParams;       /* input parameters from most recent acquisition */
   BVSB_NtscSettings   acqSettings;     /* NTSC acquisition settings */
   bool                bHsyncLocked;    /* true if HSYNC is locked    */
   uint32_t            pixCarrFreq;     /* pix carrier freq in Hz */   
   int32_t             pixCarrFreqError;/* pix carrier error in Hz */
   uint32_t            rf_agc;          /* AGT_DELTA_SIGMA level in units of 1/10 percent */
   uint32_t            if_agc;          /* AGI_DELTA_SIGMA level in units of 1/10 percent */

   bool			       bNicamLock;      /* True if NICAM is locked */
   uint32_t	           ulNicamSnr;		/* NICAM slicer SNR */
   uint32_t		       ulNicamParityError;  /* Number of parity errors in the current frame */
   uint32_t		       ulNicamErrorCount;   /* Number of bit errors in the last 128 frames */
   BVSB_NicamEncoderMode eNicamEncoderMode; /* Current NICAM encoder mode */
   uint32_t            ulAVRatio1Hi;    /* AV Ratio in linear units (upper 32 bits) */
   uint32_t            ulAVRatio1Lo;    /* AV Ratio in linear units (lower 32 bits) */
   uint32_t            ulAVRatio2Hi;    /* AV Ratio for 2nd audio carrioer in linear units (upper 32 bits) */
   uint32_t            ulAVRatio2Lo;    /* AV Ratio for 2nd audio carrioer in linear units (lower 32 bits) */
   uint32_t           ulAudioCarrier2Variance;  /*  Carrier Average Or Variance Read of 2nd audio carrier */
   
} BVSB_NtscStatus;

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
typedef struct BVSB_IfdAudioSettings
{
    BVSB_AnalogVideoMode	eVideoMode;		/* Selects analog video standards */
    BVSB_AnalogAudioMode    eAudioMode;     /* Selects analog audio standards */
    uint32_t ulCarrier2DecimationRatio;  
} BVSB_IfdAudioSettings;

/***************************************************************************
Summary:
	Structure for BTSC status

Description:
	This structure contains BTSC status

See Also:
	None.

****************************************************************************/
typedef struct BVSB_BtscStatus
{
   BVSB_BtscDecodeMode decodeMode;      /* decode mode                */
   BVSB_BtscSampleRate sampleRate;      /* audio sample rate          */
   bool                bI2sOut;         /* primary i2s output selection: true=pads used for i2s output, false=pads used for audio DAC */
   bool                bSapDetected;    /* true if SAP is detected    */
   bool                bStereoDetected; /* true if stereo is detected */
   bool                bBtscMute;       /* true if BTSC mute is on    */
   uint8_t             btscLeftVolume;  /* volume level for left channel  */
   uint8_t             btscRightVolume; /* volume level for right channel */
} BVSB_BtscStatus;


/***************************************************************************
Summary:
	Enum for out of band mode

Description:
	This enum is for out of band mode

See Also:
	None.

****************************************************************************/
typedef enum BVSB_OobMode
{
   BVSB_OobMode_eDs178,     /* DS-178 mode (1024000 sym/sec)          */
   BVSB_OobMode_eDs167_A,   /* DS-167 mode, Grade A (772000 sym/sec)  */
   BVSB_OobMode_eDs167_B,   /* DS-167 mode, Grade B (1544000 sym/sec) */
   BVSB_OobMode_eUnknown 
} BVSB_OobMode;

/***************************************************************************
Summary:
	Enum for out of band clock suppression

Description:
    This enum is for out of band clock suppression mode

See Also:
	None.

****************************************************************************/
typedef enum BVSB_OobClockSuppression
{
    BVSB_OobClockSuppression_eContinuous,
    BVSB_OobClockSuppression_eSuppressed
} BVSB_OobClockSuppression;

/***************************************************************************
Summary:
	Enum for out of band clock inversion

Description:
    This enum is to invert out of band FEC clock. 

See Also:
	None.

****************************************************************************/
typedef enum BVSB_OobClock
{
    BVSB_OobClock_eNormal,
    BVSB_OobClock_eInverted
} BVSB_OobClock;

/***************************************************************************
Summary:
	Enumeration for Agc Pin Mode

Description:
	This enum specifies the voltage swing of the AGC pin

See Also:
	None.

****************************************************************************/
typedef enum BVSB_AgcPinMode
{
    BVSB_AgcPinMode_e33V, /* indicates AGC pin goes from 0 to 3.3V */
    BVSB_AgcPinMode_e25V  /* indicates AGC pin goes from 0 to 2.5V */
} BVSB_AgcPinMode;

/***************************************************************************
Summary:
	Structure for out of band parameters

Description:
   This structure contains parameters used for an out of band channel 
   acquisition.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_OobParams
{   
   BVSB_OobMode mode;         /* oob mode */
} BVSB_OobParams;


/***************************************************************************
Summary:
	Structure for out of band status

Description:
   This structure contains status for out of band 

See Also:
	None.

****************************************************************************/
typedef struct BVSB_OobStatus
{
   BVSB_OobParams    acqParams;       /* input parameters from most recent acquisition */
   BVSB_OobSettings  acqSettings;     /* OOB acquisition settings */
   bool              bRcvrLocked;     /* true if the OOB receiver is locked */
   bool              bFecLocked;      /* true if the OOB FEC is locked */
   bool              bSpinv;          /* true if the OOB receiver has inverted the spectrum to acquire the signal */   
   uint32_t          ifFreq;          /* standard IF frequency in Hz */
   int32_t           ifFreqError;     /* IF frequency error in Hz */
   uint32_t          loFreq;          /* LO frequency */
   uint32_t          symbolRate;      /* standard symbol rate in sym/sec */
   int32_t           symbolRateError; /* symbol rate error in sym/sec */
   uint32_t          corrBlockCount;  /* accumulated correctable RS blocks */
   uint32_t          ucorrBlockCount; /* accumulated uncorrectable RS blocks */
   uint32_t          berCount;        /* accumulated internal BERT error count */   
   uint32_t          reacqCount;      /* number of reacquisitions performed by AP */   
   uint16_t          snr;             /* SNR in 1/256 dB */
   uint16_t          oob_agc;         /* OOB_DELTA_SIGMA level (0000=min level, FFFF=max level) */
   uint16_t          ext_agc;         /* external AGC level (0000=min level, FFFF=max level) */
   uint32_t          atm_cell_total_count; /* accumulated ATM cell count */
   uint32_t          atm_cell_loss_count;  /* accumulated ATM cell loss count */
} BVSB_OobStatus;

/***************************************************************************
Summary:
	Structure for Signal Detection Settings

Description:
   These settings will adjust the 

See Also:
	None.

****************************************************************************/
typedef struct BVSB_DetectionSettings
{
    uint32_t qamParam4;
    uint16_t agf;
    uint16_t agc;
} BVSB_DetectionSettings;

/**
Declare BVSB_Settings typedef because of circular reference 
of BVSB_ApiFunctTable.
**/

typedef struct BVSB_Settings BVSB_Settings;

/***************************************************************************
Summary:
	Structure for API function table

Description:
   This structure contains pointers to all public BVSB functions.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_ApiFunctTable
{
   BERR_Code (*Open)(BVSB_Handle *, BCHP_Handle, void*, BINT_Handle, const BVSB_Settings *pDefSettings);
   BERR_Code (*Close)(BVSB_Handle);
   BERR_Code (*InitAp)(BVSB_Handle, const uint8_t *);
   BERR_Code (*GetApStatus)(BVSB_Handle, BVSB_ApStatus *);
   BERR_Code (*GetApVersion)(BVSB_Handle, uint16_t*, uint16_t*, uint8_t*, uint8_t*, uint8_t*);
   BERR_Code (*ReadRegister)(BVSB_Handle, uint32_t, uint32_t*);
   BERR_Code (*WriteRegister)(BVSB_Handle, uint32_t, uint32_t*);
   BERR_Code (*Mi2cWrite)(BVSB_Handle, uint8_t, uint8_t*, uint8_t);
   BERR_Code (*Mi2cRead)(BVSB_Handle, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t);
   BERR_Code (*AcquireInband)(BVSB_Handle, const BVSB_InbandParams*);
   BERR_Code (*GetVsbStatus)(BVSB_Handle, BVSB_VsbStatus *);
   BERR_Code (*GetQamStatus)(BVSB_Handle, BVSB_QamStatus *);
   BERR_Code (*GetNtscStatus)(BVSB_Handle, BVSB_NtscStatus *);
   BERR_Code (*ResetInbandStatus)(BVSB_Handle);
   BERR_Code (*AcquireOob)(BVSB_Handle, const BVSB_OobParams*);
   BERR_Code (*GetOobStatus)(BVSB_Handle, BVSB_OobStatus *);
   BERR_Code (*ResetOobStatus)(BVSB_Handle);
   BERR_Code (*SetBtscConfig)(BVSB_Handle, BVSB_BtscSettings *);
   BERR_Code (*SetBtscVolume)(BVSB_Handle, uint8_t, uint8_t, bool);
   BERR_Code (*GetBtscStatus)(BVSB_Handle, BVSB_BtscStatus *);
   BERR_Code (*SetInbandOi)(BVSB_Handle, BVSB_InbandOiSettings *);
   BERR_Code (*GetSoftDecisionBuf)(BVSB_Handle, int16_t*, int16_t*);
   BERR_Code (*GetOobSoftDecisionBuf)(BVSB_Handle, int16_t*, int16_t*);
   BERR_Code (*SetTmConfig)(BVSB_Handle, void*);
   BERR_Code (*GetTmConfig)(BVSB_Handle, void*);
   BERR_Code (*WriteConfig)(BVSB_Handle, uint16_t, uint8_t*, uint8_t);
   BERR_Code (*ReadConfig)(BVSB_Handle, uint16_t, uint8_t*, uint8_t);
   BERR_Code (*GetLockStateChangeEventHandle)(BVSB_Handle, BKNI_EventHandle*);
   BERR_Code (*GetOobLockStateChangeEventHandle)(BVSB_Handle, BKNI_EventHandle*);
   BERR_Code (*GetAntennaEventHandle)(BVSB_Handle, BKNI_EventHandle*);
   BERR_Code (*GetInterruptEventHandle)(BVSB_Handle, BKNI_EventHandle*);
   BERR_Code (*HandleInterrupt_isr)(BVSB_Handle);
   BERR_Code (*ProcessInterruptEvent)(BVSB_Handle);
   BERR_Code (*GetChipRevision)(BVSB_Handle, uint8_t*);
   BERR_Code (*DetectChannelSignal)(BVSB_Handle, BVSB_DetectType, BVSB_SignalMode*);
   BERR_Code (*GetUnlockstatusEventHandle)(BVSB_Handle, BKNI_EventHandle* );
   BERR_Code (*GetLockstatusEventHandle)(BVSB_Handle, BKNI_EventHandle* );
   BERR_Code (*SetInbandIfFreq)(BVSB_Handle, uint32_t);
   BERR_Code (*SetIfdPullInRange)(BVSB_Handle, BVSB_Ifd_PullInRange);
   BERR_Code (*PowerDown)(BVSB_Handle);
   BERR_Code (*SetPgaGain)(BVSB_Handle, BVSB_InbandMode, BVSB_PgaGain);
   BERR_Code (*SetOobInterfaceControl)(BVSB_Handle, BVSB_OobClockSuppression, BVSB_OobClock);
} BVSB_ApiFunctTable;

/***************************************************************************
Summary:
	Structure for VSB settings

Description:
   This structure contains the settings for the BVSB PI.

See Also:
	None.

****************************************************************************/
struct BVSB_Settings
{
   BVSB_I2cSettings       i2c;      /* i2c settings */
   BVSB_VsbSettings       vsb;      /* VSB acquisition settings */
   BVSB_QamSettings       qam;      /* QAM acquisition settings */
   BVSB_NtscSettings      ntsc;     /* NTSC acquisition settings */
   BVSB_BtscSettings      btsc;     /* BTSC initial settings */
   BVSB_OobSettings       oob;      /* OOB acquisition settings */
   BVSB_InbandOiSettings  inbandOi; /* Inband xport output interface settings */
   BVSB_DetectionSettings detect;   /* DetectChannelSignal settings */
   BVSB_ApiFunctTable     api;
   BTMR_Handle            hTmr;     /* Timer used for IFD acqusition, not need in 3520 */
   bool                  bRfDeltaSigmaInvert; /* Inverts RF delta sigma pin */
   bool                  bIfDeltaSigmaInvert; /* Inverts IF delta sigma pin */
   BVSB_AgcPinMode       IfAgcPinMode; /* specifis the voltage swing of RF delta sigma pin */
   bool                  bIfAgcZero;   /* Turns off IF AGC in the chip */
   bool                  bRfAgcZero;   /* Turns off RF AGC in the chip */
   bool                  bRfDeltaSigmaOpenDrain; /* Set RF delta sigame to open drain mode */
   bool                  bIfDeltaSigmaOpenDrain; /* Set IF delta sigame to open drain mode */
   bool                  bOpenDrainPinsOnClose; /* Sets AGC pins to open drain mode when the module is closed */
};


/******************************************************************************
Summary:
   Initializes the BVSB API.
Description:
   This function must be called first to get a BVSB_Handle.  This handle is 
   used by all other function calls in the BVSB API.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_Open(
   BVSB_Handle *h,       /* [out] BVSB handle */
   BCHP_Handle hChip,    /* [in] chip handle */
   void        *pReg,    /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt, /* [in] interrupt handle */
   const BVSB_Settings *pDefSettings /* [in] default settings */
);


/******************************************************************************
Summary:
   Closes the BVSB API.
Description:
   This function releases all the resources allocated by BVSB API and disables 
   BCM3520 host interrupts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_Close(
   BVSB_Handle h   /* [in] BVSB handle */
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
BERR_Code BVSB_InitAp(
   BVSB_Handle   h,       /* [in] BVSB handle */
   const uint8_t *pImage  /* [in] pointer to AP microcode image */
);


/******************************************************************************
Summary:
   Returns status of the AP.
Description:
   The state of the AP and the HAB are returned in this function.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetApStatus(
   BVSB_Handle   hVSB,      /* [in] VSB device handle */
   BVSB_ApStatus *pStatus   /* [out] AP status */
);


/******************************************************************************
Summary:
   Returns AP version information.
Description:
   This function returns the chip revision, AP microcode revision, acquisition 
   script version, and configuration version.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetApVersion(
   BVSB_Handle h,             /* [in] BVSB handle */
   uint16_t       *pChipId,   /* [out] VSB chip id */
   uint16_t       *pChipVer,  /* [out] chip revision number */
   uint8_t        *pApVer,    /* [out] AP microcode version */
   uint8_t        *pScrVer,   /* [out] acquisition script version */
   uint8_t        *pCfgVer    /* [out] host configuration version */
);


/******************************************************************************
Summary:
   Reads a register.
Description:
   This function reads a register.  This function is normally used for 
   non-memory mapped BVSB devices, such as BCM3520.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_ReadRegister(
   BVSB_Handle hVSB,  /* [in] BVSB handle */
   uint32_t    reg,   /* [in] address of register to read */
   uint32_t    *val   /* [in] contains data that was read */
);


/******************************************************************************
Summary:
   Writes a register.
Description:
   This function writes to a register.  This function is normally used for 
   non-memory mapped BVSB devices, such as BCM3520.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_WriteRegister(
   BVSB_Handle hVSB,  /* [in] BVSB handle */
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
BERR_Code BVSB_Mi2cWrite(
   BVSB_Handle h,      /* [in] BVSB handle */
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
BERR_Code BVSB_Mi2cRead(
   BVSB_Handle h,      /* [in] BVSB handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *out_buf,   /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,      /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,    /* [out] holds the data read */
   uint8_t in_n        /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
);


/******************************************************************************
Summary:
   Acquire the inband channel.
Description:
   Tells the BCM3520 to start inband channel acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_AcquireInband(
   BVSB_Handle h,                    /* [in] BVSB handle */
   const BVSB_InbandParams *pParams  /* [in] inband acquisition parameters */
);



/******************************************************************************
Summary:
   Gets VSB status.
Description:
   This function returns VSB status information.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetVsbStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_VsbStatus *pStatus  /* [out] VSB status   */
);


/******************************************************************************
Summary:
   Gets QAM status.
Description:
   This function returns QAM status information.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetQamStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_QamStatus *pStatus  /* [out] QAM status   */
);


/******************************************************************************
Summary:
   Gets NTSC status.
Description:
   This function is used to get NTSC status information.  This function returns
   BERR_NOT_SUPPORTED if NTSC/BTSC is not available or not controlled by this 
   front end device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetNtscStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_NtscStatus *pStatus /* [out] NTSC/BTSC status   */
);


/******************************************************************************
Summary:
   Resets inband FEC bit error and block counters.
Description:
   This function clears the following counters in BVSB_VsbStatus/BVSB_QamStatus:
   corrErrCount, corrBlockCount, ucorrBlockCount, cleanBlockCount, and berCount
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_ResetInbandStatus(
   BVSB_Handle h  /* [in] BVSB handle */
);


/******************************************************************************
Summary:
   Acquire the out of band channel.
Description:
   Tells the BCM3520 to start OOB channel acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_AcquireOob(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_OobParams *pParams  /* [in] acquisition parameters */
);


/******************************************************************************
Summary:
   Gets OOB status.
Description:
   This function is used to get OOB status information.  This function returns
   BERR_NOT_SUPPORTED if OOB is not available or not controlled by this front 
   end device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetOobStatus(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_OobStatus *pStatus  /* [out] OOB status   */
);


/******************************************************************************
Summary:
   Resets block counters in BVSB_OobStatus.
Description:
   This function clears the corrBlockCount and ucorrBlockCount counters in 
   BVSB_OobStatus.  This function returns BERR_NOT_SUPPORTED if OOB is not 
   available or not controlled by this front end device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_ResetOobStatus(
   BVSB_Handle h   /* [in] BVSB handle */
);


/******************************************************************************
Summary:
   Configure BTSC.
Description:
   This function configures the sample rate, decode mode, and primary I2S
   output of the BTSC decoder.  This function returns BERR_NOT_SUPPORTED if 
   BTSC is not available or not controlled by this BVSB device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetBtscConfig(
   BVSB_Handle h,             /* [in] BVSB handle                */
   BVSB_BtscSettings *pParams /* [in] BTSC configuration parameters */
);


/******************************************************************************
Summary:
   Sets the volume level of the BTSC decoder.
Description:
   This function controls the BTSC volume level for left and right channels.
   The left_volume and right_volume parameters are attenuation in units of dB.  
   The volume level of 0 corresponds to no attenuation or full volume.  The 
   valid range for the volume levels are 0 to 95 dB.  This function returns
   BERR_NOT_SUPPORTED if BTSC is not available or not supported by this BVSB
   device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetBtscVolume(
   BVSB_Handle h,     /* [in] BVSB handle                              */
   uint8_t left_volume,  /* [in] left volume attenuation in dB (0 to 95 dB)  */
   uint8_t right_volume, /* [in] right volume attenuation in dB (0 to 95 dB) */
   bool    bMute         /* [in] BTSC mute control: 0=not muted, 1=muted     */
);


/******************************************************************************
Summary:
   Gets BTSC status.
Description:
   This function is used to get BTSC status information.  This function returns
   BERR_NOT_SUPPORTED if BTSC is not available or not controlled by this BVSB 
   device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetBtscStatus(
   BVSB_Handle h,            /* [in] BVSB handle */
   BVSB_BtscStatus *pStatus  /* [out] BTSC status   */
);


/******************************************************************************
Summary:
   Reconfigures the inband transport output interface.
Description:
   This function configures the inband transport output interface.  The 
   settings will not take effect until the next call to BVSB_AcquireInband().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetInbandOi(
   BVSB_Handle           h,         /* [in] BVSB handle */
   BVSB_InbandOiSettings *pInbandOi /* [in] inband transport output interface settings */
);


/******************************************************************************
Summary:
   Gets I/Q soft decisions for QAM/VSB.
Description:
   This function is mainly used for inband QAM/VSB constellation display.  This
   function returns 30 pairs of I/Q values.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
);


/******************************************************************************
Summary:
   Gets I/Q soft decisions for OOB
Description:
   This function is mainly used for OOB constellation display.  This function 
   returns 30 pairs of I/Q values.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetOobSoftDecisionBuf(
   BVSB_Handle h,  /* [in] BVSB handle */
   int16_t *pI,    /* [out] 30 I-values */
   int16_t *pQ     /* [out] 30 Q-values */
);


/******************************************************************************
Summary:
   Reconfigures the output pads.
Description:
   This function reconfigures the output pads.  This function returns
   BERR_NOT_SUPPORTED if pad configuration is not controlled by this BVSB 
   device (e.g. BCM3560).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetTmConfig(
   BVSB_Handle h,     /* [in] BVSB handle */
   void        *pCfg  /* [in] pad configuration parameters */
);


/******************************************************************************
Summary:
   Returns the output pads configuration settings.
Description:
   This function returns pad configuration settings.  This function returns
   BERR_NOT_SUPPORTED if pad configuration is not controlled by this BVSB 
   device (e.g. BCM3560).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetTmConfig(
   BVSB_Handle h,     /* [in] BVSB handle */
   void        *pCfg  /* [out] pad configuration parameters */
);


/******************************************************************************
Summary:
   Writes to the host configuration space.
Description:
   This function writes to the host configuration space on the front end device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_WriteConfig(
   BVSB_Handle h,     /* [in] BVSB handle */
   uint16_t offset,   /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,      /* [in] data to write */
   uint8_t len        /* [in] number of bytes to write */
);


/******************************************************************************
Summary:
   Reads from the host configuration space.
Description:
   This function reads the host configuration space on the front end device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_ReadConfig(
   BVSB_Handle h,  /* [in] BVSB handle */
   uint16_t offset,   /* [in] 16-bit offset within the host configuration space */
   uint8_t *buf,      /* [in] buffer to hold the read data */
   uint8_t len        /* [in] number of bytes to read */
);


/******************************************************************************
Summary:
   Gets the currents settings for the BVSB PI.
Description:
   This function returns the current settings being used on the BVSB PI.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetSettings(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_Settings *pSettings /* [out] current BVSB settings */
);


/******************************************************************************
Summary:
   Modifies the currents settings for the BVSB PI.
Description:
   This function modifies the current settings for the BVSB PI.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetSettings(
   BVSB_Handle h,           /* [in] BVSB handle */
   BVSB_Settings *pSettings /* [in] new BVSB settings */
);

/******************************************************************************
Summary:
   Returns the Lock State Change event handle.
Description:
   If the application wants to know when the inband channel goes in lock or out
   of lock, it should use this function to get a handle to the Lock State 
   Change event.  This event is set whenever the inband channel lock status 
   changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetLockStateChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);

/******************************************************************************
Summary:
   Returns the Out-of-Band Lock State Change event handle.
Description:
   If the application wants to know when the out of band channel goes in lock or out
   of lock, it should use this function to get a handle to the Lock State 
   Change event.  This event is set whenever the OOB channel lock status 
   changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetOobLockStateChangeEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);

/******************************************************************************
Summary:
   Returns the lock/unlock event handle.
Description:
   If the application wants to know when the inband channel goes in lock or out
   of lock, it should use this function to get a handle to the Lock State 
   Change event.  This event is set whenever the inband channel lock status 
   changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetUnlockEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);

/******************************************************************************
Summary:
   Returns the lock/unlock event handle.
Description:
   If the application wants to know when the inband channel goes in lock or out
   of lock, it should use this function to get a handle to the Lock State 
   Change event.  This event is set whenever the inband channel lock status 
   changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetLockEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);

/******************************************************************************
Summary:
   Returns the Antenna interrupt event handle.
Description:
   This event is set whenever an interrupt is received from the Antenna core.
   This function will return BERR_NOT_SUPPORTED if the Antenna core is not
   present in this BVSB device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetAntennaEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);


/******************************************************************************
Summary:
   This function returns the BVSB interrupt event handle.
Description:
   This function is used for an i2c-controlled BVSB device only.  The 
   application will need to call this function to get the event handle
   for BVSB interrupts.  The application will wait on this interrupt event.
   When this event is signalled, the application must call 
   BVSB_ProcessInterruptEvent().  For memory-mapped BVSB devices, this function
   will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetInterruptEventHandle(
   BVSB_Handle h,            /* [in] BVSB handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
);



/******************************************************************************
Summary:
   Handle BVSB interrupts.
Description:
   This function must be called when the host receives an interrupt from the 
   BVSB device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_HandleInterrupt_isr(
   BVSB_Handle h   /* [in] BVSB handle */
);


/******************************************************************************
Summary:
   This function processes BVSB interrupt events.
Description:
   This function is used for an i2c-controlled BVSB device only.  The 
   application calls this function when a BVSB interrupt event has been 
   signalled.  This function will decode the interrupt and set approriate 
   events.  For memory-mapped BVSB devices, this function will return
   BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_ProcessInterruptEvent(
   BVSB_Handle hVSB  /* [in] VSB device handle */
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
BERR_Code BVSB_GetChipRevision(
   BVSB_Handle hVSB,  /* [in] VSB device handle */
   uint8_t* revision /* [out] Chip revision */
);

/******************************************************************************
Summary:
   This function detect certain signal.
Description:
   This function is used for detect certain signal, based on parameters setup
   in BVSB_DetectionSettings.  If a signal is found, this routine will return
   BERR_SUCCSESS.  At that point, the signal is locked and more details of the 
   signal can be found by calling the appropriate BVSB_GetXxxStatus routine for 
   the signal type found.  For example, the user could distinguish what QAM 
   modulation was found, it can call BVSB_GetQamStatus() if BVSB_SignalMode_eQAM
   was returned as the signal mode.

   Note: This routine is not available on all chipsets.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_DetectChannelSignal(
	BVSB_Handle h,  /* [in] VSB device handle */
	BVSB_DetectType eDetectType,   /* [in] signal detect type */
	BVSB_SignalMode* peSignalMode /* [out] signal mode */
);


/******************************************************************************
Summary:
   Configure Inband IF frequency.
Description:
   This function configures the inband if frequency of demod
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetInbandIfFreq(
   BVSB_Handle h,             /* [in] BVSB handle                */
   uint32_t uIfFreq_hz        /* [in] IF freq in Hz configuration parameters */
);

/******************************************************************************
Summary:
   Set the Pull-in range when acquiring analog.
Description:
   This function set the pull-in range of NTSC/PAL acquiring
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetIfdPullInRange(
	BVSB_Handle h,      
	BVSB_Ifd_PullInRange if_pulling_range
);

/******************************************************************************
Summary:
   Power down all frontend related cores
Description:
   This function disables power to DS and IFD core. To bring back power, application can call either BVSB_InitAp function or any of the acquire functions
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_PowerDown(
    BVSB_Handle h           /* [in] BVSB handle */
);

/******************************************************************************
Summary:
   Change the way out of band clock works
Description:
   This function all caller to change the behaviour of the OOB FEC clock output.
   Only valid on 3520
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_SetOobInterfaceControl(
   BVSB_Handle  h,    /* [in] BVSB handle */
   BVSB_OobClockSuppression clockSuppression,
   BVSB_OobClock clock
);







#ifdef __cplusplus
}
#endif

#endif /* BVSB_H__ */

