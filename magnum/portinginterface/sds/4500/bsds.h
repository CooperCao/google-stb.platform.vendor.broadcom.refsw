/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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

/*= Module Overview ***********************************************************
<verbatim>
The Satellite Downstream PI controls the BCM4500 and BCM3440 satellite 
tuner/receiver chip set.  The BCM4500 supports "Legacy" QPSK (DVB, DSS, and 
Digicipher II standards), and Turbo QPSK/8PSK modulation.  The BCM4500 accepts 
an analog input signal from the BCM3440 baseband tuner.  The signal is 
demodulated and the final error-corrected output is delivered in MPEG-2 or 
DIRECTV transport format.

The BCM4500 also contains an integrated DISEQC transmitter and receiver, and an 
Acquisition Processor (AP).

The host communications interface to the BCM4500 is I2C, where the host 
processor executing the SDS PI is the I2C master.  To control the 
BCM4500/BCM3440 chip set, the host writes high level commands to an internal 
Host Access Buffer (HAB) on the BCM4500.  The AP services commands on the HAB 
and returns any response data back onto the HAB.  For more information on the
HAB protocol and HAB command set, refer to the BCM4500 Software User Manual 
(document number MCS954500-SWUM20x).  The BSDS PI implements the HAB protocol 
to tune the BCM3440 tuner, initiate channel acquisition, get status, and 
control DISEQC.

Interrupt Requirements
This module will need to process interrupts from the single BCM4500 host 
interrupt pin.  Because the routing of the BCM4500 interrupt is board-specific,
i.e. could be an L1 external EBI IRQ or a GPIO-based IRQ, the application will
be required to map that interrupt and call the public BSDS isr function
BSDS_HandleInterrupt_isr() when the interrupt is asserted.  

BCM4500 interrupts are decoded using i2c.  However, the magnum architecture
prohibits i2c transactions at interrupt level.  Therefore, a background task will
need to wait for interrupt events and decode/clear the BCM4500 interrupts at task
level.  In the ISR, BSDS_HandleInterrupt_isr() needs to be called.  This isr 
function signals the interrupt event and disables the L1 interrupt.  The 
background task will need to call BSDS_ProcessInterruptEvent() when the interrupt
event is signalled.  This function decodes/clears the BCM4500 interrupts and re-
enables the L1 interrupt.  

Sample Code:
//
// NOTE: The following sample code does not do any error checking.  This code
//       also assumes that the BCM4500 interrupt is on the L1 external EBI IRQ 
//       as in the BCM97038 reference board.
//
static BSDS_Handle      hSDS;
static BCHP_Handle      hChip;
static BREG_I2C_Handle  hReg;

// This is a pointer to the BCM4500 AP microcode.  The microcode may vary 
// depending on board and/or customer, so it is not part of the SDS 
// portinginterface. 
extern const uint8_t *pBcm4500ApImage;

void main(void)
{
   BSDS_Settings        settings;
   BSDS_AcqParams       acqParams;

   // do initialization
   // (get BCHP_Handle, BREG_I2C_Handle, etc)


   // BSDS initialization
   BSDS_GetDefaultSettings(&settings);
   settings.chipAddr = 0x7E; // set the i2c address
   settings.interruptEnableFunc = sds_enable_irq;
   settings.interruptEnableFuncParam = NULL;
   
   BSDS_Open(&hSDS, hChip, hReg, &settings);
  
   // enable the BCM4500 interrupt and map that interrupt to a function
   // that will call the BSDS PI function BSDS_HandleInterrupt_isr()
   ...
   
   // start a thread that runs bcm4500_background()
   ...

   // download the BCM4500 microcode and run the AP
   BSDS_InitAp(hSDS, pBcm4500ApImage));

   // some DISEQC initialization (board-specific)
   BSDS_SetGpioMode(hSDS, false, false, false);
   BSDS_SetDiseqc22KhzMode(hSDS, BSDS_Diseqc22KhzMode_eEnvelope);
   BSDS_ConfigDiseqc(hSDS, BSDS_DiseqcTone_eAbsent, BSDS_DiseqcVoltage_eBot);
   ...

   // tune to the transponder at 1119 MHz
   BSDS_Tune(hSDS, 1119000000);

   // acquire DVB code rate 5/6, 20 MBaud
   acqParams.mode = BSDS_ModulationType_eDvb5_6;
   acqParams.symbolRate = 20000000;
   acqParams.acqControl = (BSDS_MPEG | BSDS_PARALLEL | BSDS_CLKINV | BSDS_TEI);
   acqParams.carrierOffset = 0;
   acqParams.symbolRateOffset = 0;
   BSDS_Acquire(hSDS, &acqParams);

   ...

   close_sds:
   // disable SDS L1 interrupt
   // terminate bcm4500_background thread
   ...
   BSDS_Close(hSDS);

   program_done:
   ...
}


// The application must call BSDS_ProcessInterruptEvent() every time the SDS
// interrupt event is set. 
void bcm4500_background()
{
   // get handle to the BCM4500 interrupt event
   BSDS_GetInterruptEventHandle(hSDS, &event);

   while (1)
   {
      BKNI_WaitForEvent(event, BKNI_INFINTE);
      BSDS_ProcessInterruptEvent(hSDS, event);
   }
}


// This is an example of a callback function for enabling/disabling the L1 
// interrupt.
void sds_enable_irq(bool b, void *p)
{
   if (b)
      CPUINT1_Enable(SDS_IRQ);   // enable the L1 interrupt
   else
      CPUINT1_Disable(SDS_IRQ);  // disable the L1 interrupt
}
</verbatim>
******************************************************************************/

#ifndef BSDS_H__
#define BSDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "breg_i2c.h"
#include "berr_ids.h"


/* SDS Error Codes */
#define BSDS_ERR_AP_NOT_INIT      BERR_MAKE_CODE(BERR_SDS_ID, 0)
#define BSDS_ERR_AP_NOT_RUNNING   BERR_MAKE_CODE(BERR_SDS_ID, 1)
#define BSDS_ERR_AP_NOT_STOPPED   BERR_MAKE_CODE(BERR_SDS_ID, 2)
#define BSDS_ERR_HAB_NOT_AVAIL    BERR_MAKE_CODE(BERR_SDS_ID, 3)
#define BSDS_ERR_CMD_NOT_SERVICED BERR_MAKE_CODE(BERR_SDS_ID, 4)
#define BSDS_ERR_CMD_FAILED       BERR_MAKE_CODE(BERR_SDS_ID, 5)
#define BSDS_ERR_DISEQC_RRTO      BERR_MAKE_CODE(BERR_SDS_ID, 6)
#define BSDS_ERR_DISEQC_DEMOD     BERR_MAKE_CODE(BERR_SDS_ID, 7)
#define BSDS_ERR_DISEQC_PARITY    BERR_MAKE_CODE(BERR_SDS_ID, 8)
#define BSDS_ERR_DISEQC_OVERFLOW  BERR_MAKE_CODE(BERR_SDS_ID, 9)
#define BSDS_ERR_DISEQC_BUSY      BERR_MAKE_CODE(BERR_SDS_ID, 10)


/******************************************************************************
Summary:
   This is an opaque handle that is used to access the Satellite 
   Downstream API.
******************************************************************************/
typedef struct BSDS_P_Handle   *BSDS_Handle;


/******************************************************************************
Summary:
   This enum specifies the modulation and code rate for the SDS channel.
******************************************************************************/
typedef enum BSDS_ModulationType
{
   BSDS_ModulationType_eDvbScan,           /* QPSK DVB code rate scan */ 
   BSDS_ModulationType_eDvb1_2,            /* QPSK DVB rate 1/2 */
   BSDS_ModulationType_eDvb2_3,            /* QPSK DVB rate 2/3 */
   BSDS_ModulationType_eDvb3_4,            /* QPSK DVB rate 3/4 */
   BSDS_ModulationType_eDvb5_6,            /* QPSK DVB rate 5/6 */
   BSDS_ModulationType_eDvb6_7,            /* QPSK DVB rate 6/7 */
   BSDS_ModulationType_eDvb7_8,            /* QPSK DVB rate 7/8 */
   BSDS_ModulationType_eDssScan,           /* QPSK DSS code rate scan */
   BSDS_ModulationType_eDss1_2,            /* QPSK DSS rate 1/2 */
   BSDS_ModulationType_eDss2_3,            /* QPSK DSS rate 2/3 */
   BSDS_ModulationType_eDss3_4,            /* QPSK DSS rate 3/4 */
   BSDS_ModulationType_eDss5_6,            /* QPSK DSS rate 5/6 */
   BSDS_ModulationType_eDss6_7,            /* QPSK DSS rate 6/7 */
   BSDS_ModulationType_eDss7_8,            /* QPSK DSS rate 7/8 */
   BSDS_ModulationType_eDCIIScan,          /* QPSK DCII code rate scan */
   BSDS_ModulationType_eDCII5_11,          /* QPSK DCII rate 5/11 */
   BSDS_ModulationType_eDCII1_2,           /* QPSK DCII rate 1/2 */
   BSDS_ModulationType_eDCII3_5,           /* QPSK DCII rate 3/5 */
   BSDS_ModulationType_eDCII2_3,           /* QPSK DCII rate 2/3 */
   BSDS_ModulationType_eDCII3_4,           /* QPSK DCII rate 3/4 */
   BSDS_ModulationType_eDCII4_5,           /* QPSK DCII rate 4/5 */
   BSDS_ModulationType_eDCII5_6,           /* QPSK DCII rate 5/6 */
   BSDS_ModulationType_eDCII7_8,           /* QPSK DCII rate 7/8 */
   BSDS_ModulationType_eTurboScan,         /* Turbo 8PSK/QPSK code rate scan */
   BSDS_ModulationType_eTurboQpsk1_2,      /* Turbo QPSK rate 1/2 */
   BSDS_ModulationType_eTurboQpsk2_3,      /* Turbo QPSK rate 2/3 */
   BSDS_ModulationType_eTurboQpsk3_4,      /* Turbo QPSK rate 3/4 */
   BSDS_ModulationType_eTurboQpsk5_6,      /* Turbo QPSK rate 5/6 */
   BSDS_ModulationType_eTurboQpsk7_8,      /* Turbo QPSK rate 7/8 */
   BSDS_ModulationType_eTurbo8psk2_3,      /* Turbo 8PSK rate 2/3 */
   BSDS_ModulationType_eTurbo8psk5_6,      /* Turbo 8PSK rate 5/6 */
   BSDS_ModulationType_eTurbo8psk8_9,      /* Turbo 8PSK rate 8/9 */
   BSDS_ModulationType_eTurbo8psk3_4_2_05, /* Turbo 8PSK rate 3/4 (2.05 bps) */
   BSDS_ModulationType_eTurbo8psk3_4_2_10, /* Turbo 8PSK rate 3/4 (2.10 bps) */
   BSDS_ModulationType_eTurbo8psk3_4_2_20  /* Turbo 8PSK rate 3/4 (2.20 bps) */
} BSDS_ModulationType;


/* bit definitions for BSDS_AcqParams.acqControl */
#define BSDS_MPEG        0x00000020 /* 0=PN, 1=MPEG */
#define BSDS_PARALLEL    0x00000040 /* 0=serial, 1=parallel data out */
#define BSDS_ERRINV      0x00000080 /* PS_ERR: 0=active high, 1=active low */
#define BSDS_SYNCINV     0x00000100 /* PS_SYNC: 0=active high, 1=active low */
#define BSDS_VLDINV      0x00000200 /* PS_VALID: 0=active high, 1=active low */
#define BSDS_CLKINV      0x00000400 /* PS_CLK: 0=active high, 1=active low */
#define BSDS_CLKSUP      0x00000800 /* PS_CLK: 0=runs continuously, 1=suppress when PS_VALID not active */
#define BSDS_DELH        0x00001000 /* 0=no effect, 1=delete MPEG header */
#define BSDS_HEAD4       0x00002000 /* duration of PS_VALID/PS_SYNC: 0=1 byte, 1=4 bytes */
#define BSDS_SYNC1       0x00004000 /* 0=no effect, 1=PS_SYNC valid for 1 bit in serial mode */
#define BSDS_DELAY       0x00008000 /* 0=normal, 1=delay PS_ERR, PS_VALID, PS_DATA relative to PS_CLK */
#define BSDS_TEI         0x00010000 /* transport error indicator: 0=off, 1=on */
#define BSDS_ENABLE_BERT 0x00020000 /* Internal BERT: 0=disabled, 1=enabled */
#define BSDS_XBERT       0x00040000 /* External BERT: 0=disabled, 1=enabled */


/******************************************************************************
Summary:
   This structure contains the default parameters for the Satellite 
   Downstream.
******************************************************************************/
typedef struct BSDS_Settings
{
   uint16_t  chipAddr;   /* BCM4500 i2c chip address */
   void      (*interruptEnableFunc)(bool, void*); /* callback function for enabling/disabling L1 interrupt */
   void      *interruptEnableFuncParam; /* parameter passed to interruptEnableFunc */
} BSDS_Settings;


/******************************************************************************
Summary:
   This structure contains input parameters required to acquire the downstream
   channel.
******************************************************************************/
typedef struct BSDS_AcqParams 
{
   BSDS_ModulationType mode;   /* modulation */
   uint32_t  acqControl;       /* output interface parameters */
   uint32_t  symbolRate;       /* symbol rate in Baud */
   int32_t   carrierOffset;    /* carrier frequency offset in Hz */
   int32_t   symbolRateOffset; /* symbol rate offset in Baud */
} BSDS_AcqParams;


/******************************************************************************
Summary:
   This structure contains QPSK channel status information.
******************************************************************************/
typedef struct BSDS_QpskStatus 
{
   BSDS_ModulationType mode;   /* modulation */
   bool      bRsLocked;        /* true if RS is locked */
   bool      bVitLocked;       /* true if Viterbi is locked */
   bool      bBertLocked;      /* true if BERT is locked */
   uint32_t  tunerFreq;        /* tuner frequency in Hz */
   uint32_t  symbolRate;       /* symbol rate in Baud */
   int32_t   symbolRateOffset; /* symbol rate offset in Baud */
   int32_t   symbolRateError;  /* symbol rate error in Baud */
   int32_t   carrierOffset;    /* carrier frequency offset in Hz */
   int32_t   carrierError;     /* carrier frequency error in Hz */
   uint32_t  rsCorrCount;      /* accumulated number of RS correctable errors */
   uint32_t  rsUncorrCount;    /* accumulated number of RS uncorrectable errors */
   uint32_t  snrEstimate;      /* SNR estimate in units of 1/256 dB */
   uint32_t  inputPower;       /* input power estimate in units of -1/256 dBm */
} BSDS_QpskStatus;


/******************************************************************************
Summary:
   This structure contains Turbo channel status information.
******************************************************************************/
typedef struct BSDS_TurboStatus
{
   BSDS_ModulationType mode;   /* modulation */
   bool      bRcvrLocked;      /* true if the receiver is locked */
   bool      bFecLocked;       /* true if FEC is locked */
   bool      bBertLocked;      /* true if BERT is locked */
   uint32_t  tunerFreq;        /* tuner frequency in Hz */
   uint32_t  symbolRate;       /* symbol rate in Baud */
   int32_t   symbolRateOffset; /* symbol rate offset in Baud */
   int32_t   symbolRateError;  /* symbol rate error in Baud */
   int32_t   carrierOffset;    /* carrier frequency offset in Hz */
   int32_t   carrierError;     /* carrier frequency error in Hz */
   uint32_t  badBlockCount;    /* accumulated number of bad blocks */
   uint32_t  cleanBlockCount;  /* accumulated number of clean blocks */
   uint32_t  corrBlockCount;   /* accumulated number of correctable blocks */
   uint32_t  corrSymbolCount;  /* accumulated number of correctable symbols */
   uint32_t  bitsPerSym;       /* bits per symbol scaled 2^28 */
   uint32_t  snrEstimate;      /* SNR estimate in units of 1/256 dB */
   uint32_t  inputPower;       /* input power estimate in units of -1/256 dBm */
} BSDS_TurboStatus;


/******************************************************************************
Summary:
   This typedef contains AP status information.
******************************************************************************/
typedef uint32_t BSDS_ApStatus; 

/* bit definitions for BSDS_ApStatus */
#define BSDS_APSTATUS_RESET       0x00000001
#define BSDS_APSTATUS_IDLE        0x00000002
#define BSDS_APSTATUS_STOP        0x00000004
#define BSDS_APSTATUS_RUN         0x00000008
#define BSDS_APSTATUS_HABAV       0x00000010
#define BSDS_APSTATUS_MEMAV       0x00000020
#define BSDS_APSTATUS_HABERR      0x00000100
#define BSDS_APSTATUS_SCRERR      0x00000200
#define BSDS_APSTATUS_FECNL       0x00001000
#define BSDS_APSTATUS_FECL        0x00002000
#define BSDS_APSTATUS_DISEQC_DONE 0x00004000
#define BSDS_APSTATUS_INIT_DONE   0x00008000
#define BSDS_APSTATUS_HABR        0x00010000
#define BSDS_APSTATUS_HMSK        0x00080000
#define BSDS_APSTATUS_SEND_MASK   0x0001803F
#define BSDS_APSTATUS_OK_TO_SEND  0x00008008


/******************************************************************************
Summary:
   This enum specifies the DISEQC tone.
******************************************************************************/
typedef enum BSDS_DiseqcTone
{
   BSDS_DiseqcTone_eAbsent,   /* 22KHz continuous tone absent */
   BSDS_DiseqcTone_ePresent   /* 22KHz continuous tone present */
} BSDS_DiseqcTone;


/******************************************************************************
Summary:
   This enum specifies the DISEQC LNB voltage level.
******************************************************************************/
typedef enum BSDS_DiseqcVoltage
{
   BSDS_DiseqcVoltage_eBot,   /* VBOT voltage level */
   BSDS_DiseqcVoltage_eTop    /* VTOP voltage level */
} BSDS_DiseqcVoltage;


/******************************************************************************
Summary:
   This enum specifies the function for the BCM4500 ADCLK output pin.
******************************************************************************/
typedef enum BSDS_Diseqc22KhzMode
{
   BSDS_Diseqc22KhzMode_eTone,     /* 22KHz waveform on ADCLK pin */
   BSDS_Diseqc22KhzMode_eEnvelope  /* 22KHz envelope on ADCLK pin */
} BSDS_Diseqc22KhzMode;


/******************************************************************************
Summary:
   This typedef contains Diseqc Send options.
******************************************************************************/
typedef uint8_t BSDS_DiseqcSendOptions;

/* bit definitions for BSDS_DiseqcSendOptions */
#define BSDS_DISEQCSENDOPTIONS_TB_ENABLE     0x40
#define BSDS_DISEQCSENDOPTIONS_TONE_A        0x00
#define BSDS_DISEQCSENDOPTIONS_TONE_B        0x80
#define BSDS_DISEQCSENDOPTIONS_TONE_ALIGN    0x20
#define BSDS_DISEQCSENDOPTIONS_RRTO_DISABLE  0x10
#define BSDS_DISEQCSENDOPTIONS_RRTO_ENABLE   0x00


/******************************************************************************
Summary:
   This function initializes the Satellite Downstream PI.
Description:
   The function must be called first to get a handle to SDS.  This handle is 
   used by all other function calls.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_Open(
   BSDS_Handle *hSDS,                /* [out] SDS device handle */
   BCHP_Handle hChip,                /* [in] chip handle */
   BREG_I2C_Handle hReg,             /* [in] register handle */
   const BSDS_Settings *pDefSettings /* [in] default settings */
);


/******************************************************************************
Summary:
   This function closes the Satellite Downstream PI.
Description:
   Releases all the resources allocated by BSDS PI and disables SDS 
   interrupts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_Close(
   BSDS_Handle hSDS   /* [in] SDS device handle */
);


/******************************************************************************
Summary:
   Gets the default settings of BSDS.
Description:
   The default SDS configuration settings are returned.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetDefaultSettings(
   BSDS_Settings *pDefSettings   /* [out] default settings */
);


/******************************************************************************
Summary:
   Downloads the microcode and runs the acquisition processor (AP).
Description:
   The AP is placed in reset state while the microcode is downloaded to the 
   BCM4500 RAM.  Afterwards, the AP is released from its reset state and it 
   executes the microcode.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_InitAp(
   BSDS_Handle   hSDS,      /* [in] SDS device handle */
   const uint8_t *pImage    /* [in] AP microcode image */
);


/******************************************************************************
Summary:
   Returns version information.
Description:
   This function returns the following version information: chip revision, 
   microcode revision, acquisition script version, and configuration version.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetApVersion(
   BSDS_Handle hSDS,  /* [in] SDS device handle */
   uint8_t *chipId,   /* [out] chip revision ID */
   uint8_t *apId,     /* [out] AP microcode revision ID */
   uint8_t *acqId,    /* [out] acquisition script version ID */
   uint8_t *cfgId     /* [out] configuration version ID */
);


/******************************************************************************
Summary:
   Returns status of the AP.
Description:
   The state of the AP and the HAB is returned in this function.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetApStatus(
   BSDS_Handle hSDS,        /* [in] SDS device handle */
   BSDS_ApStatus *pStatus   /* [out] AP status */
);


/******************************************************************************
Summary:
   Tune the tuner to the specified frequency.
Description:
   Programs the BCM3440 tuner connected to the BCM4500 to the specified 
   frequency.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_Tune(
   BSDS_Handle hSDS,       /* [in] SDS device handle */
   const uint32_t freqHz   /* [in] tuner frequency in Hz */
);


/******************************************************************************
Summary:
   Tune the tuner to the specified transponder.
Description:
   Programs the BCM3440 tuner connected to the BCM4500 to a frequency specified
   by the transponder number.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_TuneTransponder(
   BSDS_Handle hSDS,            /* [in] SDS device handle */
   const uint8_t transponder    /* [in] transponder number from 1 to 31 */
);


/******************************************************************************
Summary:
   Acquire the channel based on specified settings.
Description:
   Initiates channel acquisition based on the specified settings.  This 
   function will return before the acquisition has completed.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_Acquire(
   BSDS_Handle hSDS,                /* [in] SDS device handle */
   const BSDS_AcqParams *pAcqParams /* [in] acquisition parameters */
);


/******************************************************************************
Summary:
   Returns the acquisition parameters used by the most recent acquisition. 
Description:
   This function returns the acquisition parameters that were passed to the most
   recent call to BSDS_Acquire().  If BSDS_Acquire() was never called, then
   BSDS_GetAcqParams() returns the default acquisition parameters.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetAcqParams(
   BSDS_Handle hSDS,          /* [in] SDS device handle */
   BSDS_AcqParams *pAcqParams /* [out] acquisition parameters */
);


/******************************************************************************
Summary:
   Returns status on the current DVB/DSS/DCII QPSK channel.
Description:
   Returns channel status information in the BSDS_QpskStatus structure.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetQpskStatus(
   BSDS_Handle hSDS,          /* [in] SDS device handle */
   BSDS_QpskStatus *pStatus   /* [out] QPSK channel status */
);


/******************************************************************************
Summary:
   Returns status on the current turbo (i.e. turbo 8PSK or turbo QPSK) channel.
Description:
   Returns channel status information in the BSDS_TurboStatus structure.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetTurboStatus(
   BSDS_Handle hSDS,           /* [in] SDS device handle */
   BSDS_TurboStatus *pStatus   /* [out] Turbo channel status */
);


/******************************************************************************
Summary:
   Returns FEC lock status
Description:
   This function is used to get a "quick" channel lock status with minimal I2C 
   overhead.  This function is applicable in both Legacy QPSK and Turbo modes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetLockStatus(
   BSDS_Handle hSDS,  /* [in] SDS device handle */
   bool *pLocked      /* [out] true if FEC is locked */
);


/******************************************************************************
Summary:
   Get soft decision I and Q values.
Description:
   Returns I and Q soft decisions.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetSoftDecision(
   BSDS_Handle hSDS,    /* [in] SDS device handle */
   int8_t *iVal,        /* [out] I value */
   int8_t *qVal         /* [out] Q value */
);


/******************************************************************************
Summary:
   Get current output bit rate.
Description:
   Returns output bitrate in bps.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetOutputBitRate(
	BSDS_Handle hSDS,  /* SDS device handle */
	uint32_t *bitrate  /* [output] bitrate in bps */
);


/******************************************************************************
Summary:
   Get number of bit errors since last call to this routine.
Description:
   Returns number of bit errors since last call.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetBerErrors(
	BSDS_Handle hSDS,     /* SDS device handle */
	uint32_t *num_errors  /* [output] number of errors */
);

/******************************************************************************
Summary:
   Get the maximum number of soft decisions (25) that can fit in the HAB
Description:
   Returns array of I and Q soft decisions.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetSoftDecisionBuf(
   BSDS_Handle hSDS,    /* [in] SDS device handle */
   int8_t *iVal,        /* [out] set of 25 I values */
   int8_t *qVal         /* [out] set of 25 Q values */
);


/******************************************************************************
Summary:
   Resets the decoder statistics internally within the BCM4500.
Description:
   If the receiver is in Turbo mode, this function resets the clean block,
   correctable block, and bad block counts.  If the receiver is in Legacy QPSK 
   mode, this function resets the Reed Solomon correctable and uncorrectable 
   error counts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_ResetDecoderStats(
   BSDS_Handle hSDS       /* [in] SDS device handle */
);


/******************************************************************************
Summary:
   Sets the DISEQC tone and voltage level.
Description:
   Sets the DISEQC 22KHz continuous tone and LNB voltage level.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_ConfigDiseqc(
   BSDS_Handle hSDS,          /* [in] SDS device handle */
   BSDS_DiseqcTone tone,      /* [in] DISEQC tone */
   BSDS_DiseqcVoltage voltage /* [in] DISEQC voltage level */
);


/******************************************************************************
Summary:
   Sends the DISEQC command and returns any reply bytes from the DISEQC slave device.
Description:
   This function sends sendBufLen bytes from pSendBuf to a DISEQC slave device.
   Any reply bytes are returned in pRcvBuf and the number of reply bytes are 
   indicated in replyLen.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_SendDiseqcCommand(
   BSDS_Handle hSDS,               /* [in] SDS device handle */
   BSDS_DiseqcSendOptions options, /* [in] Diseqc send options */
   const uint8_t *pSendBuf,        /* [in] contains data to send */ 
   uint8_t sendBufLen,             /* [in] size of send buffer (0 to 8 bytes) */
   uint8_t *pRcvBuf,        /* [out] contains any reply bytes received */
   uint8_t *pReplyLen       /* [out] number of reply bytes received (0 to 8 bytes) */
);


/******************************************************************************
Summary:
   Sends a DISEQC control word.
Description:
   Sends a DISEQC control word.  The level of the last bit is the new voltage 
   level.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_SendDiseqcAcw(
   BSDS_Handle hSDS,   /* [in] SDS device handle */
   uint8_t acw         /* [in] auto control word */
);


/******************************************************************************
Summary:
   Sets the BCM4500 ADCLK pin to output 22KHz tone or envelope.
Description:
   Selects the mode of the ADCLK digital output pin as either DISEQC tone 
   (22KHz waveform) or envelope (22KHz enable).  In either case, the output
   reflects the DISEQC PWK modulation protocol.  In tone mode, the signal can
   be directly used with a BJT current injection circuit to modulate the LNB
   line.  In envelope mode, the signal would be used as an enable for a 
   separate 22KHz oscillator.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_SetDiseqc22KhzMode(
   BSDS_Handle hSDS,         /* [in] SDS device handle */
   BSDS_Diseqc22KhzMode mode /* [in] specifies tone or envelope mode for ADCLK pin */
);


/******************************************************************************
Summary:
   Sets the operating mode for BCM4500 GPIO output pins
Description:
   This function selects the operating mode of output pins GPIO0, TESTIO, and
   TESTIO1 as either manually controlled, or automatically controlled by an
   internal DISEQC logic function.  When manual control is selected, the pin
   output level can be set using BSDS_SetGpioState() function.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_SetGpioMode(
   BSDS_Handle hSDS,         /* [in] SDS device handle */
   bool bManualGPIO0Control, /* [in] true if GPIO0 is in manual control mode, otherwise GPIO0 is the DISEQC 13V/18V select as well as automatic 13V/18V control word switching function */
   bool bManualTESTIO,       /* [in] true if TESTIO is in manual control mode, otherwise TESTIO is DISEQC 13V/18V select */
   bool bManualTESTIO1       /* [in] true if TESTIO1 is in manual control mode, otherwise TESTIO1 is DISEQC TXEN impedance control function */
);


/******************************************************************************
Summary:
   Sets the state of the BCM4500 GPIO output pins
Description:
   This function sets the state of the GPIO0, TESTIO, and TESTIO1 output pins
   when manual control of any of these pins is selected via BSDS_SetGpioMode().
   Additionally, the state of the GPIO1-7 pins (always in manual control mode)
   can be set in this function.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_SetGpioState(
   BSDS_Handle hSDS,        /* [in] SDS device handle */
   uint8_t gpioClearMask,   /* [in] clears GPIO0-7 if corresponding mask bit is set (mask bit 0 controls GPIO0 and mask bit 7 controls GPIO 7) */
   uint8_t gpioSetMask,     /* [in] sets GPIO0-7 if corresponding mask bit is set (mask bit 0 controls GPIO0 and mask bit 7 controls GPIO 8) */
   uint8_t testioClearMask, /* [in] clears TESTIO0/1 if corresponding mask bit is set (mask bit 0 controls TESTIO and mask bit 1 controls TESTIO1 */
   uint8_t testioSetMask    /* [in] sets TESTIO0/1 if corresponding mask bit is set (mask bit 0 controls TESTIO and mask bit 1 controls TESTIO1 */
);


/******************************************************************************
Summary:
   Reads a register in the SDS core.
Description:
   This function sends an HAB command to read an SDS register on the BCM4500.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_ReadCoreRegister(
   BSDS_Handle hSDS,       /* [in] SDS device handle */
   uint8_t     address,    /* [in] address of the register to read */
   bool        bMultibyte, /* [in] true=multi-byte register, false=single-byte register */
   uint32_t    *value      /* [out] register value */
);


/******************************************************************************
Summary:
   This function gets the Lock State Change event handle for SDS.  
Description:
   If the application wants to know when the SDS goes in lock or out of lock, 
   it should use this function to get a handle to the Lock event.  This event 
   is set whenever the channel lock status changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetLockStateChangeEventHandle(
   BSDS_Handle hSDS,           /* [in] SDS device handle */
   BKNI_EventHandle *phEvent   /* [out] event handle */
);


/******************************************************************************
Summary:
   This function returns the BCM4500 interrupt event handle.
Description:
   The application will need to call this function to get the event handle
   for BCM4500 interrupts.  The BCM4500 interrupt service routine signals this 
   event.  The application will wait on this interrupt event, and when this 
   event is signalled, the application must call BSDS_ProcessInterruptEvent(). 
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_GetInterruptEventHandle(
   BSDS_Handle hSDS,           /* [in] SDS device handle */
   BKNI_EventHandle *phEvent   /* [out] event handle */
);


/******************************************************************************
Summary:
   This function processes BCM4500 interrupt events. 
Description:
   This function must be called by the application when a BCM4500 
   interrupt event has been signalled.  The function will decode the 
   interrupt status on the BCM4500 and set approriate events.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSDS_ProcessInterruptEvent(
   BSDS_Handle hSDS,           /* [in] SDS device handle */
   BKNI_EventHandle *phEvent   /* [in] event handle */
);


/******************************************************************************
Summary:
   This function is the interrupt handler for BCM4500 interrupts.
Description:
   The way the BCM4500 interrupt is routed is board specific.  For example, it
   can be an L1 EBI external IRQ, or may it be a GPIO-based interrupt.  
   Therefore, the Interrupt Interface module cannot be used.  The application 
   is required to enable the BCM4500 interrupt and call this routine at 
   interrupt level.
Returns:
   BERR_Code
******************************************************************************/
void BSDS_HandleInterrupt_isr(
   BSDS_Handle hSDS          /* [in] SDS device handle */
);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSDS_H__ */
/* End of File */
