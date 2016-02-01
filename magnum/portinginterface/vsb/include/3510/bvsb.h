/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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
The VSB Downstream PI controls the BCM3510 VSB/QAM demodulator IC.  The BCM3510
supports 8/16 VSB, 16/32/64/128/256 QAM Annex A, and 64/256 QAM Annex B 
modulation.  The BCM3510 accepts an analog input signal at standard IF 
frequencies.  The final error-corrected output is delivered in MPEG-2 transport
format.

The host communications interface to the BCM3510 is I2C, where the host 
processor executing the VSB PI is the I2C master.  The BCM3510 contains an 
Acquisition Processor (AP) that services commands written to the internal dual-
access Host Access Buffer (HAB).  After the AP processes HAB commands written 
by the host, it returns any response data back onto the HAB.  The protocol for 
writing and reading the HAB is implemented by the VSB PI.  The VSB PI provides 
a high level API for QAM/VSB channel acquisition and status reporting.

Interrupt Requirements
This module will need to process interrupts from a single BCM3510 interrupt 
pin.  Because the routing of the BCM3510 interrupt is board-specific, i.e. 
could be an L1 external EBI IRQ or a GPIO-based IRQ, the application will be 
required to map that interrupt and call the public BVSB isr function 
BVSB_HandleInterrupt_isr() when the interrupt is asserted.

BCM3510 interrupts are decoded using i2c.  However, the magnum architecture
prohibits i2c transactions at interrupt level.  Therefore, a background task will
need to wait for interrupt events and decode/clear the BCM3510 interrupts at task
level.  In the ISR, BVSB_HandleInterrupt_isr() needs to be called.  This isr 
function signals the interrupt event and disables the L1 interrupt.  The 
background task will need to call BVSB_ProcessInterruptEvent() when the interrupt
event is signalled.  This function decodes/clears the BCM3510 interrupts and re-
enables the L1 interrupt.  

Sample Code
//
// NOTE: The following sample code does not do any error checking
//
static BVSB_Handle      hVSB;
static BCHP_Handle      hCHP;
static BREG_I2C_Handle  hReg;

// This is a pointer to the BCM3510 AP microcode.  The microcode will vary 
// depending on board and/or customer, so it is not part of the VSB 
// portinginterface.
extern const uint8_t *pBcm3510ApImage;

void main(void)
{
   BVSB_Settings   settings;
   BVSB_AcqParams  acqParams;

   // do initialization 
   // (get BCHP_Handle, BREG_I2C_Handle, etc)
   ...

   // BVSB initialization
   BVSB_GetDefaultSettings(&settings);
   settings.chipAddr = 0x10;  // set the i2c address
   settings.interruptEnableFunc = vsb_enable_irq;
   settings.interruptEnableFuncParam = NULL;
   
   BVSB_Open(&hVSB, hChip, hReg, &settings);
   
   // enable the BCM3510 interrupt and map that interrupt to a function
   // that will call the BVSB PI function BVSB_HandleInterrupt_isr()
   ...

   // start a thread that runs bcm3510_background()
   ...

   // download the BCM3510 microcode and run the AP
   BVSB_InitAp(hVSB, pBcm3510ApImage);

   // tune the tuner
   ...

   // acquire 8VSB
   acqParams.mode = BVSB_ModulationType_eAnnexDVsb8;
   acqParams.symbolRate = 5381119;
   acqParams.ifFreq = 44000000;
   acqParams.symbolRateOffset = 0;
   acqParams.ifFreqOffset = 0;
   acqParams.ntscOffset = 0;
   BVSB_Acquire(hVSB, &acqParams);
 
   ...

   close_vsb:
   // disable VSB L1 interrupt
   // terminate bcm3510_background task
   ...
   BVSB_Close(hVSB);

   program_done:
   ... 
}


// The application must call BVSB_ProcessInterruptEvent() every time the VSB 
// interrupt event is set.
void bcm3510_background()
{
   // get handle to the BCM3510 interrupt event
   BVSB_GetInterruptEventHandle(hVSB, &event);

   while (1)
   {
      BKNI_WaitForEvent(event, BKNI_INFINTE);
      BVSB_ProcessInterruptEvent(hVSB, event);
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
******************************************************************************/

#ifndef BVSB_H__
#define BVSB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "breg_i2c.h"
#include "berr_ids.h"


/* VSB error codes */
#define BVSB_ERR_AP_NOT_INIT      BERR_MAKE_CODE(BERR_VSB_ID, 0)
#define BVSB_ERR_AP_NOT_RUNNING   BERR_MAKE_CODE(BERR_VSB_ID, 1)
#define BVSB_ERR_AP_NOT_STOPPED   BERR_MAKE_CODE(BERR_VSB_ID, 2)
#define BVSB_ERR_HAB_NOT_AVAIL    BERR_MAKE_CODE(BERR_VSB_ID, 3)
#define BVSB_ERR_CMD_NOT_SERVICED BERR_MAKE_CODE(BERR_VSB_ID, 4)
#define BVSB_ERR_CMD_FAILED       BERR_MAKE_CODE(BERR_VSB_ID, 5)


/******************************************************************************
Summary:
   This is an opaque handle that is used to access the VSB Downstream API.
******************************************************************************/
typedef struct BVSB_P_Handle   *BVSB_Handle;


/******************************************************************************
Summary:
   This enum specifies the modulation for the downstream channel.
******************************************************************************/
typedef enum BVSB_ModulationType 
{
   BVSB_ModulationType_eAnnexBQam256, /* Annex B 256-QAM */
   BVSB_ModulationType_eAnnexBQam64,  /* Annex B 64-QAM */
   BVSB_ModulationType_eAnnexAQam256, /* Annex A 256-QAM */ 
   BVSB_ModulationType_eAnnexAQam128, /* Annex A 128-QAM */
   BVSB_ModulationType_eAnnexAQam64,  /* Annex A 64-QAM */
   BVSB_ModulationType_eAnnexAQam32,  /* Annex A 32-QAM */
   BVSB_ModulationType_eAnnexAQam16,  /* Annex A 16-QAM */
   BVSB_ModulationType_eAnnexDVsb8,   /* Annex D 8-VSB */
   BVSB_ModulationType_eAnnexDVsb16   /* Annex D 16-VSB */
} BVSB_ModulationType;


/******************************************************************************
Summary:
   This structure contains the settings for the VSB Downstream PI.
******************************************************************************/
typedef struct BVSB_Settings
{
   uint16_t  chipAddr;   /* BCM3510 i2c chip address */
   void      (*interruptEnableFunc)(bool, void*); /* callback for enabling/disable L1 interrupt */
   void      *interruptEnableFuncParam; /* parameter passed to interruptEnableFunc */
} BVSB_Settings;


/******************************************************************************
Summary:
   This structure contains input parameters required to acquire the downstream
   channel.
******************************************************************************/
typedef struct BVSB_AcqParams 
{
   BVSB_ModulationType mode;   /* modulation */
   uint32_t  symbolRate;       /* standard symbol rate in Baud. This get auto selected when using VSB or QAM Annex B */
   uint32_t  ifFreq;           /* standard IF Frequency in Hz */
   int32_t  symbolRateOffset;  /* symbol rate offset in Baud */
   int32_t  ifFreqOffset;      /* IF Frequency offst in Hz */
   int32_t  ntscOffset;        /* NTSC offset in Hz */
} BVSB_AcqParams;


/******************************************************************************
Summary:
   This structure contains channel status information.
******************************************************************************/
typedef struct BVSB_Status 
{
   BVSB_ModulationType mode;   /* modulation */
   bool      bRcvrLocked;      /* true if the receiver is locked */
   bool      bFecLocked;       /* true if the FEC is locked */
   bool      bOPllLocked;      /* true if the output PLL is locked */

   uint32_t  symbolRate;       /* standard symbol rate in Baud */
   int32_t   symbolRateOffset; /* symbol rate offset in Baud */
   int32_t   symbolRateError;  /* symbol rate error in Baud */

   uint32_t  ifFreq;           /* standard IF frequency in Hz */
   int32_t   ifFreqOffset;     /* IF frequency offset in Hz */
   int32_t   ifFreqError;      /* IF frequency error in Hz */

   int32_t   ntscFilter;       /* location of NTSC filter notch 1 in Hz */
   int32_t   ntscOffset;       /* NTSC offset in Hz from host */
   int32_t   ntscError;        /* NTSC error in Hz */
   
   uint32_t  intAgcLevel;      /* IF AGC level in units of 1/10 percent */ 
   uint32_t  extAgcLevel;      /* tuner AGC level in units of 1/10 percent */
   uint32_t  snrEstimate;      /* SNR estimate in units of 1/256 dB */

   uint32_t  corrErrCount;    /* accumulated correctable bit errors by FEC */
   uint32_t  ucorrBlockCount; /* accumulated uncorrectable RS blocks */
   uint32_t  berCount;         /* internal bit error counter */

} BVSB_Status;


/******************************************************************************
Summary:
   This typedef contains AP status information.
******************************************************************************/
typedef uint32_t BVSB_ApStatus;   

/* bit definitions for BVSB_ApStatus */
#define BVSB_APSTATUS_RESET      0x00000001
#define BVSB_APSTATUS_IDLE       0x00000002
#define BVSB_APSTATUS_STOP       0x00000004
#define BVSB_APSTATUS_RUN        0x00000008
#define BVSB_APSTATUS_HABAV      0x00000010
#define BVSB_APSTATUS_MEMAV      0x00000020
#define BVSB_APSTATUS_HABERR     0x00000100
#define BVSB_APSTATUS_SCRERR     0x00000200
#define BVSB_APSTATUS_NOT_LOCK   0x00001000
#define BVSB_APSTATUS_LOCK       0x00002000
#define BVSB_APSTATUS_INIT_DONE  0x00008000
#define BVSB_APSTATUS_HABR       0x00010000
#define BVSB_APSTATUS_HMSK       0x00080000
#define BVSB_APSTATUS_SEND_MASK  0x0001803F
#define BVSB_APSTATUS_OK_TO_SEND 0x00008008


/******************************************************************************
Summary:
   This function initializes the VSB Downstream PI.
Description:
   The function must be called first to get a handle to the VSB PI.  This 
   handle is used by all other VSB PI function calls.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_Open(
   BVSB_Handle *hVSB,                  /* [out] VSB device handle */
   BCHP_Handle hChip,                  /* [in] chip handle */
   BREG_I2C_Handle hReg,               /* [in] register handle */
   const BVSB_Settings *pDefSettings   /* [in] default settings */
);


/******************************************************************************
Summary:
   This function closes the VSB Downstream PI.
Description:
   Releases all the resources allocated by VSB PI and disables VSB interrupts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_Close(
   BVSB_Handle hVSB   /* [in] VSB device handle */
);


/******************************************************************************
Summary:
   Gets the default settings for the VSB PI.
Description:
   The default VSB PI configuration settings are returned.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetDefaultSettings(
   BVSB_Settings *pDefSettings   /* [out] default settings */
);


/******************************************************************************
Summary:
   Downloads the microcode and runs the AP.
Description:
   The AP is placed in reset state while the microcode is downloaded to the 
   BCM3510 RAM.  Afterwards, the AP is released from its reset state and it 
   executes the microcode.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_InitAp(
   BVSB_Handle hVSB,       /* [in] VSB device handle */
   const uint8_t *pImage   /* [in] pointer to BCM3510 microcode image */
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
BERR_Code BVSB_GetApVersion(
   BVSB_Handle hVSB,   /* [in] VSB device handle */
   uint8_t *chipId,    /* [out] chip revision ID */
   uint8_t *apId,      /* [out] AP microcode revision ID */
   uint8_t *acqId,     /* [out] acquisition script version ID */
   uint8_t *cfgId      /* [out] configuration version ID */
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
   BVSB_Handle hVSB,        /* [in] VSB device handle */
   BVSB_ApStatus *pStatus   /* [out] AP status */
);


/******************************************************************************
Summary:
   Acquire the channel based on specified acquisition parameters.
Description:
   Initiates channel acquisition based on the specified settings.  This 
   function will return before the acquisition has completed.
   If mode selected in VSB or QAM Annex B, than symbolrate will be chosen
   automatically based on standards
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_Acquire(
   BVSB_Handle hVSB,                /* [in] VSB device handle */
   const BVSB_AcqParams *pAcqParams /* [in] acquisition parameters */
);


/******************************************************************************
Summary:
   Returns the acquisition parameters used by the most recent acquisition. 
Description:
   This function returns the acquisition parameters that were passed to the most
   recent call to BVSB_Acquire().  If BVSB_Acquire() was never called, then
   BVSB_GetAcqParams() returns the default acquisition parameters.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetAcqParams(
   BVSB_Handle hVSB,          /* [in] VSB device handle */
   BVSB_AcqParams *pAcqParams /* [out] acquisition parameters */
);


/******************************************************************************
Summary:
   Returns status on the current channel.
Description:
   Returns channel status information in the BVSB_Status structure.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetStatus(
   BVSB_Handle hVSB,      /* [in] VSB device handle */
   BVSB_Status *pStatus   /* [out] channel status */
);


/******************************************************************************
Summary:
   Returns lock status.
Description:
   This function is used to get a "quick" channel lock status with minimal I2C 
   overhead.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetLockStatus(
   BVSB_Handle hVSB,  /* [in] VSB device handle */
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
BERR_Code BVSB_GetSoftDecision(
   BVSB_Handle hVSB,    /* [in] VSB device handle */
   int8_t *iVal,        /* [out] I value */
   int8_t *qVal         /* [out] Q value */
);

/******************************************************************************
Summary:
   Get the maximum number of soft decisions (31) that can fit in the HAB
Description:
   Returns array of I and Q soft decisions.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetSoftDecisionBuf(
   BVSB_Handle hVSB,    /* [in] VSB device handle */
   int8_t *iVal,        /* [out] set of 31 I values */
   int8_t *qVal         /* [out] set of 31 Q values */
);

/******************************************************************************
Summary:
   Resets FEC counters 
Description:
   Clears the accumulated FEC counters
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_ResetInbandStatus(
   BVSB_Handle hVSB    /* [in] VSB device handle */
    );


/******************************************************************************
Summary:
   Generate an I2C stream to the RF_CLK/RF_DATA output pins on BCM3510.
Description:
   This function is typically used to control an I2C-based tuner connected to
   the RF_CLK/RF_DATA pins on the BCM3510.  Since RF_DATA is output only, I2C
   write transactions will only be supported and the I2C slave acknowledge bit 
   will be ignored.  This function will first generate an I2C start condition, 
   then clock out n bytes, then generate an I2C stop condition.  The I2C clock 
   is approximately 57 KHz.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_WriteI2c(
   BVSB_Handle hVSB,   /* [in] VSB device handle */
   const uint8_t *buf, /* [in] bytes to send */
   uint8_t n           /* [in] number of bytes to send */
);


/******************************************************************************
Summary:
   This function gets the Lock State Change event handle.
Description:
   If the application wants to know when the downstream goes in lock or out of 
   lock, it should use this function to get a handle to the Lock event.  
   This event is set whenever the channel lock status changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetLockStateChangeEventHandle(
   BVSB_Handle hVSB,            /* [in] VSB device handle */
   BKNI_EventHandle *phEvent    /* [out] event handle */
);


/******************************************************************************
Summary:
   This function returns the BCM3510 interrupt event handle.
Description:
   The application will need to call this function to get the interrupt event 
   handle for BCM3510.  The BCM3510 interrupt service routine, 
   BVSB_HandleInterrupt_isr(), signals the interrupt event.  The application 
   waits on this interrupt event, and calls BVSB_ProcessInterruptEvent() when
   the event is signalled.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_GetInterruptEventHandle(
   BVSB_Handle hVSB,           /* [in] VSB device handle */
   BKNI_EventHandle *phEvent   /* [out] event handle */
);


/******************************************************************************
Summary:
   This function processes BCM3510 interrupt events. 
Description:
   This function must be called by the application when a BCM3510 interrupt 
   event has been signalled.  The function will decode the interrupt status on 
   the BCM3510 and set approriate events.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_ProcessInterruptEvent(
   BVSB_Handle hVSB,           /* [in] VSB device handle */
   BKNI_EventHandle *phEvent   /* [in] event handle */
);


/******************************************************************************
Summary:
   This function is the interrupt handler for BCM3510 interrupts.
Description:
   The way the BCM3510 interrupt is routed is board specific.  For example, it
   can be an L1 EBI external IRQ, or may it be a GPIO-based interrupt.
   Therefore, the Interrupt Interface module cannot be used.  The application
   is required to enable the BCM3510 interrupt and call this routine at
   interrupt level.
Returns:
   BERR_Code
******************************************************************************/
void BVSB_HandleInterrupt_isr(
   BVSB_Handle hVSB          /* [in] VSB device handle */
);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVSB_H__ */
/* End of File */
