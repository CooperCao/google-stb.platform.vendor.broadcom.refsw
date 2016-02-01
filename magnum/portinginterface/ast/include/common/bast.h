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
The BAST (Broadcom Advanced SatelliTe downstream) portinginterface (PI) 
controls the RF front-end on any Broadcom chip that has one or more Satellite 
Downstream (SDS) core(s), such as BCM4501.  The BAST PI is responsible for 
controlling the satellite tuner (internal tuner or external BCM3440), the 
satellite downstream receiver, DISEQC, and FTM functions.  The BAST PI controls
other resources that may also exist in the SDS core such as Master I2C 
controller and GPIOs.

The BAST PI device is accessed by the handle BAST_Handle.  There is 1 
BAST_Handle per Broadcom chip.  A BAST_Handle will have 1 or more channels,
depending on the number of SDS cores present in the Broadcom chip.  Each 
channel will be accessed by a handle of type BAST_ChannelHandle.  

The BAST_Handle shall be provided in the following API functions to 
control the AP, Top Mux (i.e. pin muxing), interrupts, and FTM:
   BAST_Open
   BAST_Close
   BAST_GetTotalChannels
   BAST_InitAp
   BAST_SoftReset
   BAST_GetApStatus
   BAST_GetApVersion
   BAST_GetInterruptEventHandle
   BAST_HandleInterrupt_isr
   BAST_ProcessInterruptEvent
   BAST_ResetFtm
   BAST_ReadFtm
   BAST_WriteFtm
   BAST_GetFtmEventHandle
   BAST_GetTmConfig
   BAST_SetTmConfig
   BAST_ConfigGpio
   BAST_SetGpio
   BAST_GetGpio
   BAST_PowerDownFtm
   BAST_PowerUpFtm
   BAST_ConfigLna
   BAST_ConfigAgc
   
The BAST_ChannelHandle shall be provided in the following API functions to 
control a specific downstream channel:
   BAST_OpenChannel
   BAST_CloseChannel
   BAST_GetChannelDefaultSettings
   BAST_GetDevice
   BAST_TuneAcquire
   BAST_GetChannelStatus
   BAST_GetLockStatus
   BAST_ResetStatus
   BAST_SetDiseqcTone
   BAST_GetDiseqcTone
   BAST_SetDiseqcVoltage
   BAST_GetDiseqcEventHandle
   BAST_SendDiseqcCommand
   BAST_GetDiseqcStatus
   BAST_ResetDiseqc
   BAST_SendACW
   BAST_WriteMi2c
   BAST_ReadMi2c
   BAST_GetSoftDecisionBuf
   BAST_ReadRegister
   BAST_WriteRegister
   BAST_ReadConfig
   BAST_WriteConfig
   BAST_GetLockStateChangeEventHandle 
   BAST_ReadAgc
   BAST_WriteAgc
   BAST_FreezeAgc
   BAST_FreezeEq
   BAST_PowerDown
   BAST_PowerUp
   BASt_AbortAcq
   
The magnum/portinginterface/sds/bast.h header file defines the standard BAST 
API for all Broadcom satellite downstream chips.  The implementation of the API
for each chip is in the magnum/portinginterface/sds/<chip> directory.  


Interrupt Requirements
The application needs to call BAST_HandleInterrupt_isr() when it receives 
the SDS L1 interrupt.


Sample Code
//
// NOTE: This is sample code for a BCM4501.
//       This code does not do any error checking.
//
#include "bast.h"
#include "bast_4501.h"

static BAST_Handle        h4501;      // BCM4501 device handle
static BAST_ChannelHandle *h4501Chan; // handle for each SDS channel
static BCHP_Handle        hCHP;
static BREG_I2C_Handle    hReg;

// pBcm4501ApImage is a pointer to the AP microcode to be downloaded to the 
// BCM4501.  Since the microcode may vary per chip/board/customer, it is not 
// part of the BAST PI.
extern const uint8_t *pBcm4501ApImage;

void main(void)
{
   BAST_Settings        settings;
   BAST_ChannelSettings chnSettings;
   BAST_AcqSettings     acqParams;
   int                  i;

   // do initialization
   // (get BCHP_Handle, BREG_I2C_Handle, etc)
   ...

   // BAST initialization for BCM4501
   BAST_4501_GetDefaultSettings(&settings);
   settings.i2c.chipAddr = BCM4501_I2C_ADDR;
   settings.i2c.interruptEnableFunc = bcm4501_enable_irq;
   settings.i2c.interruptEnableFuncParam = NULL;
   settings.bEnableInternalTuner = true; // use internal tuner 
   BAST_Open(&h4501, hChip, (void*)hReg, NULL, &settings);

   // allocate handles for the BAST channels
   BAST_GetTotalChannels(h4501, &numChannels);
   h4501Chan = (BAST_ChannelHandle *)BKNI_Malloc(numChannels * sizeof(BAST_ChannelHandle));

   // open each 4501 channel
   for (i = 0; i < numChannels; i++)
   {
      BAST_GetChannelDefaultSettings(h4501, i, &chnSettings);
      BAST_OpenChannel(h4501, &h4501Chan[i], i, &chnSettings); 
   }

   // enable the BCM4501 host interrupt and map that interrupt to a function
   // that will call the BAST PI function BAST_HandleInterrupt_isr()
   ...

   // start a thread that runs bcm4501_background() - see below
   ...

   // download the BCM4501 microcode and run the AP
   BAST_InitAp(h4501, pBcm4501ApImage);

   // enable FTM mode (instead of DISEQC)
   BAST_ResetFtm(h4501);

   //------------------------ channel 0 acquisition ---------------------------
   // tune tuner 0 to RF frequency 1170 MHz and acquire legacy mode DTV 6/7
   acqParams.mode = BAST_Mode_eDss_6_7;
   acqParams.carrierFreqOffset = 0;
   acqParams.symbolRate = 20000000;
   BAST_TuneAcquire(h4501Chan[0], 1170000000, &acqParams);
   
   ...

   close_sds:
   // disable SDS L1 interrupts
   // terminate bcm4501_background task
   ...
   for (i = 0; i < numChannels; i++)
      BAST_CloseChannel(h4501Chan[i]);
   BAST_Close(h4501);

   program_done:
   ...
}

// The application must call BAST_ProcessInterruptEvent() every time the
// BAST interrupt event is set.
void bcm4501_background()
{
   // get handle to the BCM4501 interrupt event
   BAST_GetInterruptEventHandle(h4501, &event);

   while (1)
   {
      BKNI_WaitForEvent(event, BKNI_INFINTE);
      BAST_ProcessInterruptEvent(h4501);
   }
}


// This is an example of a callback function for enabling/disabling the L1
// interrupt.
void bcm4501_enable_irq(bool b, void *p)
{
   if (b)
      CPUINT1_Enable(SDS_IRQ);   // enable the L1 interrupt
   else
      CPUINT1_Disable(SDS_IRQ);  // disable the L1 interrupt
}
</verbatim>
========================================================================*/

#ifndef BAST_H__
#define BAST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "bint.h"
#include "berr_ids.h"
#include "bfec.h"


#define BAST_API_VERSION 4


/******************************************************************************
Summary:
   BAST error codes
Description:
   These are API error codes specific to the BAST PI.
See Also:
   None.
******************************************************************************/
#define BAST_ERR_AP_FAIL         BERR_MAKE_CODE(BERR_AST_ID, 0)  /* This is a BERR_Code to indicate that host was unable to reset/idle/run the AP. */
#define BAST_ERR_HOST_XFER       BERR_MAKE_CODE(BERR_AST_ID, 1)  /* This is a BERR_Code to indicate a host transfer error (underrun/overrun). */
#define BAST_ERR_IOMB_XFER       BERR_MAKE_CODE(BERR_AST_ID, 2)  /* This is a BERR_Code to indicate an IO Mailbox ISB transfer error. */
#define BAST_ERR_MEMAV           BERR_MAKE_CODE(BERR_AST_ID, 3)  /* This is a BERR_Code to indicate a memory access violation. */
#define BAST_ERR_HAB_CHECKSUM    BERR_MAKE_CODE(BERR_AST_ID, 4)  /* This is a BERR_Code to indicate bad checksum read from the HAB (usually due to I2C bit errors) */
#define BAST_ERR_AP_NOT_INIT     BERR_MAKE_CODE(BERR_AST_ID, 5)  /* This is a BERR_Code to indicate that the AP has failed to initialize. */
#define BAST_ERR_HABAV           BERR_MAKE_CODE(BERR_AST_ID, 6)  /* This is a BERR_Code to indicate an HAB access violation. */
#define BAST_ERR_HAB_NOT_AVAIL   BERR_MAKE_CODE(BERR_AST_ID, 7)  /* This is a BERR_Code to indicate that the HAB is busy. */
#define BAST_ERR_HAB_ERR         BERR_MAKE_CODE(BERR_AST_ID, 8)  /* This is a BERR_Code to indicate an HAB command format error. */
#define BAST_ERR_HAB_NO_ACK      BERR_MAKE_CODE(BERR_AST_ID, 9)  /* This is a BERR_Code to indicate that the HAB command was not processed by AP. */
#define BAST_ERR_HAB_FAIL        BERR_MAKE_CODE(BERR_AST_ID, 10) /* This is a BERR_Code to indicate bad/unexpected HAB data read. */
#define BAST_ERR_AP_HABAV        BERR_MAKE_CODE(BERR_AST_ID, 11) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_AP_COPY         BERR_MAKE_CODE(BERR_AST_ID, 12) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_AP_IRQ          BERR_MAKE_CODE(BERR_AST_ID, 13) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_AP_SCR          BERR_MAKE_CODE(BERR_AST_ID, 14) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_AP_ISB          BERR_MAKE_CODE(BERR_AST_ID, 15) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_AP_WD           BERR_MAKE_CODE(BERR_AST_ID, 16) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_AP_STACK        BERR_MAKE_CODE(BERR_AST_ID, 17) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_HAB_BAD_CMD     BERR_MAKE_CODE(BERR_AST_ID, 18) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_IOMB_BUSY       BERR_MAKE_CODE(BERR_AST_ID, 19) /* This is a BERR_Code to indicate the IO MBOX was busy. */
#define BAST_ERR_HAB_TIMEOUT     BERR_MAKE_CODE(BERR_AST_ID, 20) /* This is a BERR_Code to indicate an HAB timeout. */
#define BAST_ERR_AP_UNKNOWN      BERR_MAKE_CODE(BERR_AST_ID, 21) /* This is a BERR_Code to indicate an AP-internal error. */
#define BAST_ERR_MI2C_NO_ACK     BERR_MAKE_CODE(BERR_AST_ID, 22) /* This is a BERR_Code to indicate no ack from i2c slave device (e.g. tuner). */
#define BAST_ERR_HAB_NOT_SUPP    BERR_MAKE_CODE(BERR_AST_ID, 23) /* This is a BERR_Code to indicate HAB command not supported. */
#define BAST_ERR_HAB_BAD_PARAM   BERR_MAKE_CODE(BERR_AST_ID, 24) /* This is a BERR_Code to indicate parameter out of range in the HAB command */
#define BAST_ERR_HAB_CMD_FAIL    BERR_MAKE_CODE(BERR_AST_ID, 25) /* This is a BERR_Code to indicate HAB command could not be serviced. */
#define BAST_ERR_DISEQC_BUSY     BERR_MAKE_CODE(BERR_AST_ID, 26) /* This is a BERR_Code to indicate DISEQC is busy. */
#define BAST_ERR_HAB_DISEQC_BUSY BAST_ERR_DISEQC_BUSY
#define BAST_ERR_TUNER_FAIL      BERR_MAKE_CODE(BERR_AST_ID, 27) /* This is a BERR_Code to indicate tuner unable to tune. */
#define BAST_ERR_FTM_TX_BUSY     BERR_MAKE_CODE(BERR_AST_ID, 28) /* This is a BERR_Code to indicate that that an FTM packet could not be transmitted because the transmitter is busy */
#define BAST_ERR_FTM_RX_FULL     BERR_MAKE_CODE(BERR_AST_ID, 29) /* This is a BERR_Code to indicate that a received FTM packet could not be stored because the Rx buffer is full */ 
#define BAST_ERR_FTM_MSG_DROPPED BERR_MAKE_CODE(BERR_AST_ID, 30) /* This is a BERR_Code to indicate that a local FTM message could not be sent to the host because host buffer is full */ 
#define BAST_ERR_POWERED_DOWN    BERR_MAKE_CODE(BERR_AST_ID, 31) /* This is a BERR_Code to indicate that a command was not serviced because a required core was powered down */
#define BAST_ERR_MI2C_BUSY       BERR_MAKE_CODE(BERR_AST_ID, 32) /* This is a BERR_Code to indicate that the MI2C operation could not be performed due to device busy */
#define BAST_ERR_SAMPLE_FREQ     BERR_MAKE_CODE(BERR_AST_ID, 33) /* This is a BERR_Code to indicate the sample freq could not be set. */


/******************************************************************************
Summary:
   Handle for the sds device
Description:
   This is an opaque handle for the BAST device.
See Also:
   BAST_Open()
******************************************************************************/
typedef struct BAST_P_Handle *BAST_Handle;


/******************************************************************************
Summary:
   Handle for an BAST device channel
Description:
   This is an opaque handle for the BAST device channel.
See Also:
   BAST_OpenChannel()
******************************************************************************/
typedef struct BAST_P_ChannelHandle *BAST_ChannelHandle;


/******************************************************************************
Summary:
   Specifies the modulation, FEC, and code rate.
Description:
   This enum specifies the modulation, FEC, and code rate.
See Also:
   BAST_AcqSettings, BAST_Acquire(), BAST_GetChannelStatus()
******************************************************************************/
typedef enum BAST_Mode
{
   BAST_Mode_eDvb_scan = 0x00,  /* Legacy QPSK DVB code rate scan */
   BAST_Mode_eDvb_1_2 = 0x01,   /* Legacy QPSK DVB code rate 1/2 */
   BAST_Mode_eDvb_2_3 = 0x02,   /* Legacy QPSK DVB code rate 2/3 */
   BAST_Mode_eDvb_3_4 = 0x03,   /* Legacy QPSK DVB code rate 3/4 */
   BAST_Mode_eDvb_5_6 = 0x04,   /* Legacy QPSK DVB code rate 5/6 */
   BAST_Mode_eDvb_7_8 = 0x05,   /* Legacy QPSK DVB code rate 7/8 */
   BAST_Mode_eDss_scan = 0x06,  /* Legacy QPSK DTV code rate scan */
   BAST_Mode_eDss_1_2 = 0x07,   /* Legacy QPSK DTV code rate 1/2 */
   BAST_Mode_eDss_2_3 = 0x08,   /* Legacy QPSK DTV code rate 2/3 */
   BAST_Mode_eDss_6_7 = 0x09,   /* Legacy QPSK DTV code rate 6/7 */
   BAST_Mode_eDcii_scan = 0x0A, /* Legacy QPSK DigicipherII code rate scan */
   BAST_Mode_eDcii_1_2 = 0x0B,  /* Legacy QPSK DigicipherII code rate 1/2 */
   BAST_Mode_eDcii_2_3 = 0x0C,  /* Legacy QPSK DigicipherII code rate 2/3 */
   BAST_Mode_eDcii_3_4 = 0x0D,  /* Legacy QPSK DigicipherII code rate 3/4 */
   BAST_Mode_eDcii_5_6 = 0x0E,  /* Legacy QPSK DigicipherII code rate 5/6 */
   BAST_Mode_eDcii_7_8 = 0x0F,  /* Legacy QPSK DigicipherII code rate 7/8 */
   BAST_Mode_eDcii_5_11 = 0x10, /* Legacy QPSK DigicipherII code rate 5/11 */
   BAST_Mode_eDcii_3_5 = 0x11,  /* Legacy QPSK DigicipherII code rate 3/5 */
   BAST_Mode_eDcii_4_5 = 0x12,  /* Legacy QPSK DigicipherII code rate 4/5 */
   BAST_Mode_eLdpc_Qpsk_1_2 = 0x20,  /* NBC QPSK LDPC+BCH code rate 1/2 */
   BAST_Mode_eLdpc_Qpsk_3_5 = 0x21,  /* NBC QPSK LDPC+BCH code rate 3/5 */
   BAST_Mode_eLdpc_Qpsk_2_3 = 0x22,  /* NBC QPSK LDPC+BCH code rate 2/3 */
   BAST_Mode_eLdpc_Qpsk_3_4 = 0x23,  /* NBC QPSK LDPC+BCH code rate 3/4 */
   BAST_Mode_eLdpc_Qpsk_4_5 = 0x24,  /* NBC QPSK LDPC+BCH code rate 4/5 */
   BAST_Mode_eLdpc_Qpsk_5_6 = 0x25,  /* NBC QPSK LDPC+BCH code rate 5/6 */
   BAST_Mode_eLdpc_Qpsk_8_9 = 0x26,  /* NBC QPSK LDPC+BCH code rate 8/9 */
   BAST_Mode_eLdpc_Qpsk_9_10 = 0x27, /* NBC QPSK LDPC+BCH code rate 9/10 */
   BAST_Mode_eLdpc_8psk_3_5 = 0x28,  /* NBC 8PSK LDPC+BCH code rate 3/5 */
   BAST_Mode_eLdpc_8psk_2_3 = 0x29,  /* NBC 8PSK LDPC+BCH code rate 2/3 */
   BAST_Mode_eLdpc_8psk_3_4 = 0x2A,  /* NBC 8PSK LDPC+BCH code rate 3/4 */
   BAST_Mode_eLdpc_8psk_5_6 = 0x2B,  /* NBC 8PSK LDPC+BCH code rate 5/6 */
   BAST_Mode_eLdpc_8psk_8_9 = 0x2C,  /* NBC 8PSK LDPC+BCH code rate 8/9 */
   BAST_Mode_eLdpc_8psk_9_10 = 0x2D, /* NBC 8PSK LDPC+BCH code rate 9/10 */
   BAST_Mode_eLdpc_scan = 0x2E,      /* LDPC code rate scan */
   BAST_Mode_eLdpc_ACM = 0x2F,       /* LDPC ACM */
   BAST_Mode_eTurbo_scan = 0x30,     /* Turbo code rate scan */
   BAST_Mode_eTurbo_Qpsk_1_2 = 0x31, /* Turbo QPSK 1/2 */
   BAST_Mode_eTurbo_Qpsk_2_3 = 0x32, /* Turbo QPSK 2/3 */
   BAST_Mode_eTurbo_Qpsk_3_4 = 0x33, /* Turbo QPSK 3/4 */
   BAST_Mode_eTurbo_Qpsk_5_6 = 0x34, /* Turbo QPSK 5/6 */
   BAST_Mode_eTurbo_Qpsk_7_8 = 0x35, /* Turbo QPSK 7/8 */
   BAST_Mode_eTurbo_8psk_2_3 = 0x36, /* Turbo 8PSK 2/3 */
   BAST_Mode_eTurbo_8psk_3_4 = 0x37, /* Turbo 8PSK 3/4 */
   BAST_Mode_eTurbo_8psk_4_5 = 0x38, /* Turbo 8PSK 4/5 */
   BAST_Mode_eTurbo_8psk_5_6 = 0x39, /* Turbo 8PSK 5/6 */
   BAST_Mode_eTurbo_8psk_8_9 = 0x3A, /* Turbo 8PSK 8/9 */
   BAST_Mode_eBlindScan = 0x3F,      /* Blind scan */
   BAST_Mode_eUnknown = 0xFF         /* invalid mode, do not use */
} BAST_Mode;

#define BAST_MODE_IS_DVB(x) (x <= BAST_Mode_eDvb_7_8)
#define BAST_MODE_IS_DTV(x) ((x >= BAST_Mode_eDss_scan) && (x <= BAST_Mode_eDss_6_7))
#define BAST_MODE_IS_DCII(x) ((x >= BAST_Mode_eDcii_scan) && (x <= BAST_Mode_eDcii_4_5))
#define BAST_MODE_IS_LEGACY_QPSK(x) (x <= BAST_Mode_eDcii_4_5)
#define BAST_MODE_IS_LDPC(x) ((x >= BAST_Mode_eLdpc_Qpsk_1_2) && (x <= BAST_Mode_eLdpc_ACM))
#define BAST_MODE_IS_LDPC_ACM(x) (x == BAST_Mode_eLdpc_ACM)
#define BAST_MODE_IS_LDPC_8PSK(x) ((x >= BAST_Mode_eLdpc_8psk_3_5) && (x <= BAST_Mode_eLdpc_8psk_9_10))
#define BAST_MODE_IS_LDPC_QPSK(x) ((x >= BAST_Mode_eLdpc_Qpsk_1_2) && (x <= BAST_Mode_eLdpc_Qpsk_9_10))
#define BAST_MODE_IS_TURBO(x) ((x >= BAST_Mode_eTurbo_scan) && (x <= BAST_Mode_eTurbo_8psk_8_9))
#define BAST_MODE_IS_BLIND_SCAN(x) (x == BAST_Mode_eBlindScan)
#define BAST_MODE_IS_TURBO_8PSK(x) ((x >= BAST_Mode_eTurbo_8psk_2_3) && (x <= BAST_Mode_eTurbo_8psk_8_9))

/*****************************************************************************
Summary:
   Structure containing parameters used to acquire the transponder
Description:
   This structure contains acquisition parameters used to acquire the 
   transponder.
See Also:
   BAST_Acquire()
8*****************************************************************************/
typedef struct BAST_AcqSettings
{
   BAST_Mode mode;              /* modulation and code rate */
   uint32_t  symbolRate;        /* symbol rate in sym/sec */
   int32_t   carrierFreqOffset; /* carrier frequency offset in Hz */
   uint32_t  acq_ctl;           /* see BAST_ACQSETTINGS_* macros for bit definitions */
} BAST_AcqSettings;

#define BAST_ACQSETTINGS_MPEG                0x00000000 /* MPEG data */
#define BAST_ACQSETTINGS_PN                  0x00000001 /* PN data */
#define BAST_ACQSETTINGS_PRBS23              0x00000000 /* PRBS-23 */
#define BAST_ACQSETTINGS_PRBS15              0x00000002 /* PRBS-15 */
#define BAST_ACQSETTINGS_NYQUIST_35          0x00000000 /* .35 nyquist filter */
#define BAST_ACQSETTINGS_NYQUIST_20          0x00000004 /* .20 nyquist filter */
#define BAST_ACQSETTINGS_BERT_ENABLE         0x00000008 /* enable internal BERT */
#define BAST_ACQSETTINGS_PN_INVERT           0x00000010 /* PN data inverted */
#define BAST_ACQSETTINGS_BERT_RESYNC_DISABLE 0x00000020 /* BERT resync disable */
#define BAST_ACQSETTINGS_REACQ_DISABLE       0x00000040 /* disable auto reacquisition */
#define BAST_ACQSETTINGS_DCII_COMBINED       0x00000000 /* DCII only: combined mode */
#define BAST_ACQSETTINGS_DCII_SPLIT          0x00000100 /* DCII only: split mode */
#define BAST_ACQSETTINGS_DCII_SPLIT_I        0x00000000 /* DCII split mode only: split I */
#define BAST_ACQSETTINGS_DCII_SPLIT_Q        0x00000200 /* DCII split mode only: split Q */
#define BAST_ACQSETTINGS_OQPSK               0x00000400 /* OQPSK */
#define BAST_ACQSETTINGS_SPINV_NORMAL        0x00000000 /* spectral inversion: normal I/Q */
#define BAST_ACQSETTINGS_LDPC_PILOT_SCAN     0x00001000 /* LDPC pilot scan: 0=disabled, 1=enabled */
#define BAST_ACQSETTINGS_SPINV_Q_INV         0x00002000 /* spectral inversion: Q invert */
#define BAST_ACQSETTINGS_SPINV_I_INV         0x00004000 /* spectral inversion: I invert */
#define BAST_ACQSETTINGS_SPINV_IQ_SCAN       0x00006000 /* spectral inversion: I/Q scan mode */
#define BAST_ACQSETTINGS_RS_DISABLE          0x00008000 /* disable Reed Solomon */
#define BAST_ACQSETTINGS_LDPC_NO_PILOT       0x00000000 /* LDPC mode only: no pilot */
#define BAST_ACQSETTINGS_LDPC_PILOT          0x00010000 /* LDPC mode only: pilot */
#define BAST_ACQSETTINGS_LDPC_PILOT_PLL      0x00020000 /* LPPC pilot PLL: 0=disabled, 1=enabled */
#define BAST_ACQSETTINGS_TUNER_TEST_MODE     0x00040000 /* Tuner Test Mode: 0=disable, 1=enable */
#define BAST_ACQSETTINGS_LDPC_SHORT_FRAME    0x00080000 /* LDPC mode only: 0=normal FEC frame, 1=short FEC frame */
#define BAST_ACQSETTINGS_TONE_SEARCH_MODE    0x00100000 /* 1=enable tone search */
#define BAST_ACQSETTINGS_SIGNAL_DETECT_MODE  0x00200000 /* signal detect mode: 0=disable, 1=enable */
#define BAST_ACQSETTINGS_BYPASS_TUNE         0x40000000 /* 0=normal, 1=dont tune */
#define BAST_ACQSETTINGS_BYPASS_ACQUIRE      0x80000000 /* 0=normal, 1=dont do acquisition */
#define BAST_ACQSETTINGS_DEFAULT             0x00036004 /* default settings */


/*****************************************************************************
Summary:
   Structure containing host I2C settings
Description:
   This structure contains the settings for host I2C control.  For memory-
   mapped BAST devices, this structure will not be used.
See Also:
   BAST_Settings
******************************************************************************/
typedef struct BAST_I2cSettings
{
   uint16_t chipAddr;  /* BCM4501 i2c chip address */
   void     (*interruptEnableFunc)(bool, void*); /* callback function for enabling/disabling L1 interrupt */
   void     *interruptEnableFuncParam; /* parameter passed to interruptEnableFunc */
} BAST_I2cSettings;


/******************************************************************************
Summary:
   AP status information
Description:
   This typedef contains AP status information.
See Also:
   BAST_GetApStatus()
******************************************************************************/
typedef uint32_t BAST_ApStatus;

/* bit definitions for BAST_ApStatus */
#define BAST_APSTATUS_RESET      0x00000001 /* AP is in reset state */
#define BAST_APSTATUS_IDLE       0x00000002 /* AP is in idle state */
#define BAST_APSTATUS_RUN        0x00000000 /* AP is running */
#define BAST_APSTATUS_RUN_MASK   0x00000003 /* AP execution state mask */
#define BAST_APSTATUS_HAB_DONE   0x00000004 /* HAB done */
#define BAST_APSTATUS_HAB_READY  0x00000020 /* HAB available condition */
#define BAST_APSTATUS_HAB_MASK   0x00000027 /* HAB available condition mask */

#define BAST_APSTATUS_HAB_ERR    0x00001000 /* HAB access violation */
#define BAST_APSTATUS_MEM_ERR    0x00002000 /* memory access violation */
#define BAST_APSTATUS_IOMB_ERR   0x00004000 /* io_mbox isb transfer error */
#define BAST_APSTATUS_H_ERR      0x00008000 /* host transfer error */

#define BAST_APSTATUS_AP_ERR     0x00010000 /* internal AP error */
#define BAST_APSTATUS_HABCMD_ERR 0x00020000 /* HAB command syntax error */
#define BAST_APSTATUS_FTM_ERR    0x00200000 /* FTM error */
#define BAST_APSTATUS_FTM        0x00400000 /* pending FTM message */
#define BAST_APSTATUS_INIT_DONE  0x00800000 /* AP init done */
#define BAST_APSTATUS_DS_DONE_0  0x01000000 /* channel 0 DISEQC transaction completed */
#define BAST_APSTATUS_LOCK_0     0x08000000 /* channel 0 locked */
#define BAST_APSTATUS_DS_DONE_1  0x10000000 /* channel 1 DISEQC transaction completed */
#define BAST_APSTATUS_LOCK_1     0x80000000 /* channel 1 locked */


/******************************************************************************
Summary:
   Phase rotation status
Description:
   This enum specifies the phase rotation status.
See Also:
   BAST_GetQpskStatus()
******************************************************************************/
typedef enum BAST_PhaseRotation
{
   BAST_PhaseRotation_e0 = 0,   /* 0 degrees */
   BAST_PhaseRotation_e90,      /* 90 degrees */
   BAST_PhaseRotation_e180,     /* 180 degrees */
   BAST_PhaseRotation_e270      /* 270 degrees */
} BAST_PhaseRotation;


/******************************************************************************
Summary:
   Spectral inversion mode
Description:
   This enum specifies the spectral inversion mode.
See Also:
   BAST_QpskStatus
******************************************************************************/
typedef enum BAST_Spinv
{
   BAST_Spinv_eNormal = 0, /* I and Q not inverted */
   BAST_Spinv_eIinv,       /* I-channel inverted */
   BAST_Spinv_eQinv,       /* Q-channel inverted */
   BAST_Spinv_eScan        /* spectral inversion scan mode */
} BAST_Spinv;


/******************************************************************************
Summary:
   Status of the last DISEQC transaction
Description:
   This enum is the error status of the last DISEQC transaction.
See Also:
   BAST_SendDiseqcCommand, BAST_GetDiseqcStatus
******************************************************************************/
typedef enum BAST_DiseqcSendStatus
{
   BAST_DiseqcSendStatus_eSuccess = 0, /* no error */
   BAST_DiseqcSendStatus_eRxOverflow,
   BAST_DiseqcSendStatus_eRxReplyTimeout,
   BAST_DiseqcSendStatus_eRxParityError,
   BAST_DiseqcSendStatus_eAcwTimeout,
   BAST_DiseqcSendStatus_eBusy
} BAST_DiseqcSendStatus;


/******************************************************************************
Summary:
   This struct holds the status and any reply bytes from the most recent diseqc
   transaction. 
Description:
   This struct holds the status and any reply bytes from the most recent diseqc
   transaction. 
See Also:
   BAST_SendDiseqcCommand, BAST_GetDiseqcStatus
******************************************************************************/
#define DISEQC_REPLY_BUF_SIZE 16
typedef struct BAST_DiseqcStatus
{
   BAST_DiseqcSendStatus status;   /* status of most recent transaction */
   bool                  bReplyExpected; /* true=reply bytes expected */
   uint8_t               nReplyBytes;    /* number of valid reply bytes in reply_buffer */
   uint8_t               replyBuffer[DISEQC_REPLY_BUF_SIZE]; /* holds the reply bytes received */
} BAST_DiseqcStatus;


/******************************************************************************
Summary:
   Structure containing Legacy QPSK status
Description:
   This structure contains legacy mode QPSK status.
See Also:
   BAST_GetChannelStatus()
******************************************************************************/
typedef struct BAST_QpskStatus
{
   uint32_t           rsCorrCount;   /* accumulated number of RS correctable errors */
   uint32_t           rsUncorrCount; /* accumulated number of RS uncorrectable errors */
   uint32_t           preVitErrCount; /* accumulated Viterbi re-encoder error count */
   BAST_Spinv         spinv;         /* spectral inversion state */
   BAST_PhaseRotation phase;         /* phase rotation status */
} BAST_QpskStatus;


/******************************************************************************
Summary:
   Structure containing LDPC status
Description:
   This structure contains LDPC status.
See Also:
   BAST_GetChannelStatus()
******************************************************************************/
typedef struct BAST_LdpcStatus
{
   uint32_t totalBlocks; /* accumulated number of BCH blocks */
   uint32_t corrBlocks;  /* accumulated number of BCH corrected blocks */
   uint32_t badBlocks;   /* accumulated number of BCH bad blocks */
   uint32_t hpstatus;    /* header processor status */
   uint32_t fec_clock;   /* FEC clock in Hz */
} BAST_LdpcStatus;


/******************************************************************************
Summary:
   Structure containing Turbo FEC status
Description:
   This structure contains Turbo FEC status.
See Also:
   BAST_GetChannelStatus()
******************************************************************************/
typedef struct BAST_TurboStatus
{
   uint32_t totalBlocks; /* accumulated number blocks */
   uint32_t corrBlocks;  /* accumulated number of corrected blocks */
   uint32_t badBlocks;   /* accumulated number of bad blocks */
   uint32_t hpstatus;    /* header processor status */
   uint32_t fec_clock;   /* FEC clock in Hz */
} BAST_TurboStatus;


/******************************************************************************
Summary:
   Structure containing channel status
Description:
   This structure contains channel status.
See Also:
   BAST_GetChannelStatus()
******************************************************************************/
typedef struct BAST_ChannelStatus
{
   BAST_Mode mode;            /* modulation and code rate */
   bool      bTunerLocked;    /* true = internal tuner locked */
   bool      bDemodLocked;    /* true = demod locked */
   bool      bBertLocked;     /* true = BERT is locked */
   uint32_t  symbolRate;      /* commanded symbol rate in Baud */
   int32_t   symbolRateError; /* symbol rate error in Baud */
   uint32_t  carrierOffset;   /* commanded carrier frequency offset in Hz */
   int32_t   carrierError;    /* carrier frequency error in Hz */
   uint32_t  outputBitrate;   /* output bit rate in bps */
   uint32_t  berErrors;       /* accumulated number of BER errors (valid if BERT enabled) */
   uint32_t  mpegErrors;      /* accumulated number of MPEG frame errors */
   uint32_t  mpegCount;       /* total MPEG frame count */
   uint32_t  reacqCount;      /* number of auto re-acquisitions */
   uint32_t  snrEstimate;     /* raw value for SNR estimate (requires conversion to dB) */
   uint32_t  IFagc;           /* IF AGC integrator value */
   uint32_t  RFagc;           /* RF AGC integrator value */
   int32_t   agf;             /* AGF integrator value */
   uint32_t  tunerFreq;       /* tuner frequency in Hz */
   uint32_t  sample_clock;    /* A/D sample freq in Hz */
   uint32_t  tunerFilter;     /* tuner LPF cutoff freq in Hz */
   union modeStatus 
   {
      BAST_QpskStatus  legacy; /* legacy QPSK specific status */
      BAST_LdpcStatus  ldpc;   /* NBC LDPC+BCH specific status */
      BAST_TurboStatus turbo;  /* Turbo FEC specific status */
   } modeStatus;
} BAST_ChannelStatus;


/******************************************************************************
Summary:
   AGC select
Description:
   This enum selects the AGC.
See Also:
   BAST_ReadAgc, BAST_WriteAgc, BAST_FreezeAgc
******************************************************************************/
typedef enum BAST_Agc
{
   BAST_Agc_eRF = 0, /* RF AGC */
   BAST_Agc_eIF
} BAST_Agc;
  

/******************************************************************************
Summary:
   BCM3445 LNA switch configuration.  
Description:
   This enum selects the internal switch configuration for the BCM3445 LNA.
   NOTE: Additional configuration options will no longer be supported for 
   BAST_ConfigLna().  Use BAST_ConfigBcm3445() instead, as BAST_ConfigLna()
   will be deprecated.
See Also:
   BAST_ConfigLna
******************************************************************************/
typedef enum BAST_LnaIntConfig
{
   BAST_LnaIntConfig_eIn1Out1_In1Out2 = 0, /* IN1(VGA) to OUT1, IN1(VGA) to (OUT2,DAISY) */
   BAST_LnaIntConfig_eIn1Out1_In2Out2,     /* IN1(VGA) to OUT1, IN2(VGA) to (OUT2,DAISY) */
   BAST_LnaIntConfig_eIn2Out1_In2Out2,     /* IN2(VGA) to OUT1, IN2(VGA) to (OUT2,DAISY) */
   BAST_LnaIntConfig_eIn1DbOut1_In2Out2,   /* IN1(DB) to OUT1, IN2(VGA) to (OUT2,DAISY) */
   BAST_LnaIntConfig_eOff,                 /* OUT1, OUT2, and DAISY are powered down */
   BAST_LnaIntConfig_eIn2Out1_In1Out2,     /* IN2(VGA) to OUT1, IN1(VGA) to (OUT2, DAISY) */
   BAST_LnaIntConfig_eIn2Out1_In2Out2_DaisyOff /* IN2(VGA) to OUT1, IN2(VGA) to OUT2, DAISY powered down */
} BAST_LnaIntConfig;


/******************************************************************************
Summary:
   BCM3445 LNA output configuration.
Description:
   This enum selects the mapping of the BCM3445 output to SDS input channel.
   NOTE: Additional configuration options will no longer be supporteed for 
   BAST_ConfigLna().  Use BAST_ConfigBcm3445() instead, as BAST_ConfigLna()
   will be deprecated.
See Also:
   BAST_ConfigLna
******************************************************************************/
typedef enum BAST_LnaExtConfig
{
   BAST_LnaExtConfig_eOut1Sds0_Out2Sds1 = 0, /* OUT1 to SDS0, OUT2 to SDS1 */
   BAST_LnaExtConfig_eOut1Sds0_DaisySds1,    /* OUT1 to SDS0, Daisy to SDS1 */
   BAST_LnaExtConfig_eOut1Sds1_Out2Sds0      /* OUT1 to SDS1, OUT2 to SDS0 */
} BAST_LnaExtConfig;


/***************************************************************************
Summary:
   Structure containing BCM3445 status.
Description:
   Structure containing BCM3445 status.
See Also:
   BAST_GetLnaStatus()
****************************************************************************/
typedef struct BAST_LnaStatus
{
   BAST_LnaIntConfig int_config; /* int_config parameter from most recent BAST_ConfigLna() */
   BAST_LnaExtConfig ext_config; /* ext_config parameter from most recent BAST_ConfigLna() */
   uint8_t  version;             /* BCM3445 chip version, 0 if not detected */
   uint8_t  agc0;                /* SDS0 AGC value */
   uint8_t  agc1;                /* SDS1 AGC value */
} BAST_LnaStatus;


/******************************************************************************
Summary:
   Mi2c channel enum
Description:
   This enum specifies which MI2C controls a particular BCM3445.
See Also:
   BAST_Bcm3445Settings, BAST_ConfigBcm3445
******************************************************************************/
typedef enum BAST_Mi2cChannel
{
   BAST_Mi2cChannel_e0 = 0, /* Mi2c from SDS 0 */
   BAST_Mi2cChannel_e1,     /* Mi2c from SDS 1 */
   BAST_Mi2cChannel_e2      /* Mi2c from SDS 2 */
} BAST_Mi2cChannel;


/******************************************************************************
Summary:
   FSK channel enum
Description:
   This enum specifies which FSK DAC is used for Tx/Rx.
See Also:
   BAST_Bcm3445Settings, BAST_ConfigBcm3445
******************************************************************************/
typedef enum BAST_FskChannel
{
   BAST_FskChannel_e0 = 0,
   BAST_FskChannel_e1
} BAST_FskChannel;


/******************************************************************************
Summary:
   Specifies the configuration of an output channel on BCM3445
Description:
   The enum specifies the configuration of an output channel on BCM3445.
See Also:
   BAST_Bcm3445Settings, BAST_ConfigBcm3445
******************************************************************************/
typedef enum BAST_Bcm3445OutputConfig
{
   BAST_Bcm3445OutputConfig_eOff = 0, /* output is powered off */
   BAST_Bcm3445OutputConfig_eDaisy,   /* output comes from daisy */
   BAST_Bcm3445OutputConfig_eIn1Vga,  /* output comes from IN1(VGA) */
   BAST_Bcm3445OutputConfig_eIn1Db,   /* output comes from IN1(DB) */
   BAST_Bcm3445OutputConfig_eIn2Vga   /* output comes from IN2(VGA) */
} BAST_Bcm3445OutputConfig;


/******************************************************************************
Summary:
   BCM3445 LNA output configuration.
Description:
   This structure specifies the configuration of the BCM3445.
See Also:
   BAST_ConfigBcm3445
******************************************************************************/
typedef struct BAST_Bcm3445Settings
{
   BAST_Mi2cChannel         mi2c;  /* specifies which MI2C is used to control this BCM3445 */
   BAST_Bcm3445OutputConfig out1;  /* OUT1 configuration */
   BAST_Bcm3445OutputConfig out2;  /* OUT2 configuration */
   BAST_Bcm3445OutputConfig daisy; /* DAISY configuration */
} BAST_Bcm3445Settings;


/******************************************************************************
Summary:
   Specifies a BCM3445 output channel.
Description:
   This enum type specifies a BCM3445 output channel.
See Also:
   BAST_MapBcm3445ToTuner
******************************************************************************/
typedef enum BAST_Bcm3445OutputChannel
{
   BAST_Bcm3445OutputChannel_eNone = 0,  /* no output selected */
   BAST_Bcm3445OutputChannel_eOut1,      /* BCM3445 OUT1 output */
   BAST_Bcm3445OutputChannel_eOut2,      /* BCM3445 OUT2 output */
   BAST_Bcm3445OutputChannel_eDaisy      /* BCM3445 DAISY output */
} BAST_Bcm3445OutputChannel;


/***************************************************************************
Summary:
   Structure containing BCM3445 status.
Description:
   Structure containing BCM3445 status.
See Also:
   BAST_GetBcm3445Status()
****************************************************************************/
typedef struct BAST_Bcm3445Status
{
   BAST_Mi2cChannel          mi2c;        /* specifies the BCM3445 */
   BAST_Bcm3445OutputChannel tuner_input; /* specifies the BCM3445 output channel feeding this tuner */
   BAST_Bcm3445OutputConfig  out_cfg;     /* specifies how the BCM3445 output channel is configured */
   uint8_t                   status;      /* see BAST_BCM3445_STATUS_* macros */
   uint8_t                   version;     /* BCM3445 version number */
   uint8_t                   agc;         /* AGC value read from BCM3445 */
} BAST_Bcm3445Status;


/******************************************************************************
Summary:
   Specifies the configuration of an output channel on the Tuner LNA
Description:
   The enum specifies the configuration of an output channel on internal Tuner LNA.
See Also:
   BAST_TunerLnaSettings, BAST_ConfigTunerLna
******************************************************************************/
typedef enum BAST_TunerLnaOutputConfig
{
   BAST_TunerLnaOutputConfig_eOff = 0, /* output is powered off */
   BAST_TunerLnaOutputConfig_eIn0,     /* output comes from input 0 */
   BAST_TunerLnaOutputConfig_eIn1,     /* output comes from input 1 */
   BAST_TunerLnaOutputConfig_eDaisy    /* output comes from input daisy */
} BAST_TunerLnaOutputConfig;


/******************************************************************************
Summary:
   Tuner LNA output configuration.
Description:
   This structure specifies the configuration of the Tuner LNA crossbar.
See Also:
   BAST_ConfigTunerLna
******************************************************************************/
typedef struct BAST_TunerLnaSettings
{
   BAST_TunerLnaOutputConfig out0;  /* OUT0 configuration */
   BAST_TunerLnaOutputConfig out1;  /* OUT1 configuration */
   BAST_TunerLnaOutputConfig daisy; /* DAISY configuration */
} BAST_TunerLnaSettings;


/******************************************************************************
Summary:
   Tuner LNA status.
Description:
   Structure containing Tuner LNA status.
See Also:
   BAST_GetTunerLnaStatus
******************************************************************************/
typedef struct BAST_TunerLnaStatus
{
   uint8_t status;
   BAST_TunerLnaOutputConfig outCfg;
   uint32_t lnaAgc;
   uint32_t bbAgc;
} BAST_TunerLnaStatus;


/***************************************************************************
Summary:
   Structure containing signal detect status.
Description:
   This structure contains the status on the most recent signal detect 
   operation initiated by BAST_TuneAcquire().
See Also:
   BAST_GetSignalDetectStatus()
****************************************************************************/
typedef struct BAST_SignalDetectStatus
{
   bool bEnabled;          /* true if signal detect mode was initiated by the last BAST_TuneAcquire() */
   bool bDone;             /* true if bEnabled AND signal detection operation has completed */
   bool bTimingLoopLocked; /* true if bDone AND timing loop is locked */
} BAST_SignalDetectStatus;


/******************************************************************************
Summary:
   BCM3445 status codes
Description:
   These are bit definition of BAST_Bcm3445Status.status
See Also:
   BAST_Bcm3445Status, BAST_GetBcm3445Status
******************************************************************************/
#define BAST_BCM3445_STATUS_INITIALIZED      0x01 /* BCM3445 successfully initialized */
#define BAST_BCM3445_STATUS_ERR_INVALID_CFG  0x10 /* invalid configuration specified */
#define BAST_BCM3445_STATUS_ERR_I2C          0x20 /* I2C error */
#define BAST_BCM3445_STATUS_ERR_NOT_DETECTED 0x40 /* No BCM3445 detected */
#define BAST_BCM3445_STATUS_ERROR            0x70


/******************************************************************************
Summary:
   BCM3445 status codes
Description:
   These are bit definition of BAST_Bcm3445Status.status
See Also:
   BAST_Bcm3445Status, BAST_GetBcm3445Status
******************************************************************************/
#define BAST_TUNER_LNA_STATUS_INITIALIZED       0x01
#define BAST_TUNER_LNA_STATUS_ERR_INVALID_CFG   0x10  /* invalid configuration specified */
#define BAST_TUNER_LNA_STATUS_ERROR             0x10


/***************************************************************************
Summary:
   Structure specifying notch filter settings for spur cancellation.
Description:
   Structure specifying notch filter settings for spur cancellation.
See Also:
   BAST_EnableSpurCanceller()
****************************************************************************/
typedef struct BAST_SpurCancellerConfig
{
   uint32_t freq;         /* spur frequency in Hz */
} BAST_SpurCancellerConfig;


/******************************************************************************
Summary:
   Specifies service provider 
Description:
   This enum specifies the service provider to enable specific settings for 
   acquisition, diseqc, and fsk.
See Also:
   BAST_ConfigLna
******************************************************************************/
typedef enum BAST_NetworkSpec
{
   BAST_NetworkSpec_eDefault = 0,
   BAST_NetworkSpec_eEcho,
   BAST_NetworkSpec_eEuro
} BAST_NetworkSpec;


/***************************************************************************
Summary:
   Structure containing peak scan status.
Description:
   Structure containing peak scan status.
See Also:
   BAST_GetPeakScanStatus()
****************************************************************************/
typedef struct BAST_PeakScanStatus
{
   uint32_t status;      /* 0=scan finished, 1=scan in progress, 2=scan error */
   uint32_t tunerFreq;   /* tuner frequency in Hz from last BAST_PeakScan() call */
   uint32_t out;         
   uint32_t peakPower;  
} BAST_PeakScanStatus;


/***************************************************************************
Summary:
   Enum to specify MPEG packetizer error mode for LDPC acquisitions.
Description:
   Enum to specify MPEG packetizer error mode for LDPC acquisitions.
See Also:
   BAST_OutputTransportSettings()
****************************************************************************/
typedef enum BAST_BchMpegErrorMode
{
   BAST_BchMpegErrorMode_eBchOrCrc8, /* (BCH flag) AND (CRC8 flag) */
   BAST_BchMpegErrorMode_eCrc8,      /* CRC8 flag */
   BAST_BchMpegErrorMode_eBch,       /* BCH flag */
   BAST_BchMpegErrorMode_eBchAndCrc8 /* (BCH flag) AND (CRC8 flag) */
} BAST_BchMpegErrorMode;


/***************************************************************************
Summary:
   Structure containing MPEG transport output settings.
Description:
   Structure containing MPEG transport output settings.
See Also:
   BAST_GetOutputTransportSettings(), BAST_SetOutputTransportSettings()
****************************************************************************/
typedef struct BAST_OutputTransportSettings
{
   bool bSerial;     /* false=parallel output, true=serial output */
   bool bClkInv;     /* false=normal CLK, true=inverted CLK */
   bool bClkSup;     /* false=CLK runs continuously, true=CLK is suppressed when VALID not active */
   bool bVldInv;     /* false=VALID is active high, true=VALID is active low */
   bool bSyncInv;    /* false=SYNC is active high, true=SYNC is active low */
   bool bErrInv;     /* false=ERR is active high, true=ERR is active low */
   bool bXbert;      /* external bert mode: false=disabled, true=enabled */
   bool bTei;        /* transport error indicator flag in MPEG2 transport: false=off, true=on */
   bool bDelay;      /* output delay: false=normal operation, true=delay ERR, VALID, and DATA relative to CLK */
   bool bSync1;      /* SYNC duration in serial mode: false=no effect (SYNC is either 1 byte (bHead4=false), or 4 bytes (bHead4=true); true=SYNC will be valid for 1 bit in serial mode.  Has no effect in parallel mode.  In parallel mode, SYNC duration is defined by bHead4. */
   bool bHead4;      /* mpeg packet header length, defines the duration of VALID and SYNC: true=header is 4 bytes, false=header is 1 byte */
   bool bDelHeader;  /* true=delete MPEG header */
   bool bOpllBypass; /* true=bypass OPLL, false=use normal OPLL setting */
   BAST_BchMpegErrorMode bchMpegErrorMode; /* MPEG error mode for LDPC BCH */
} BAST_OutputTransportSettings;


/***************************************************************************
Summary:
   Structure containing DiSEqC settings.
Description:
   Structure containing DiSEqC settings.
See Also:
   BAST_GetDiseqcSettings(), BAST_SetDiseqcSettings()
****************************************************************************/
typedef struct BAST_DiseqcSettings
{
   bool bEnvelope;         /* false=tone mode, true=envelope mode on TXOUT pin */
   bool bToneAlign;        /* tone alignment: false=off, true=on */
   bool bDisableRRTO;      /* false=RxReplyTimeOut enabled, true=RRTO disabled */
   bool bEnableToneburst;  /* tone burst: false=disabled, true=enabled */
   bool bToneburstB;       /* applies if tone burst is enabled: false=tone A, true=tone B */
   bool bOverrideFraming;  /* false=first byte bit 1 of command indicates reply expected, true=override framing byte, reply expectation depends on bExpectReply */
   bool bExpectReply;      /* applies if bOverrideFraming set: false=don't expect reply, true=always expect reply */
   bool bEnableLNBPU;      /* false=LNBPU not used , true=LNBPU on TXEN pin */
   bool bDisableRxOnly;    /* false=RX , true=disable RX only mode */
   uint32_t rrtoUsec;      /* diseqc receive reply timeout in usecs */
   uint16_t bitThreshold;  /* threshold for diseqc rx bit detection in usecs */
   uint8_t toneThreshold;  /* threshold for diseqc tone detect in units of 0.16 counts per mV */
   uint8_t preTxDelay;     /* delay time in msecs before transmitting diseqc command */
   uint8_t vsenseThresholdHi; /* voltage detect overvoltage threshold */
   uint8_t vsenseThresholdLo; /* voltage detect undervoltage threshold */
} BAST_DiseqcSettings;


struct BAST_Settings;
struct BAST_ChannelSettings;

/* bit definitions for options parameter in BAST_PowerDown()/BAST_PowerUp() */
#define BAST_CORE_SDS         0x00000001 /* SDS and FEC cores */
#define BAST_CORE_DISEQC      0x00000002 /* DISEQC core */
#define BAST_CORE_ALL         (BAST_CORE_SDS | BAST_CORE_DISEQC)


/******************************************************************************
Summary:
   Structure for API function table
Description:
   This structure contains pointers to all public BAST functions.
See Also:
   BAST_Settings
******************************************************************************/
typedef struct BAST_ApiFunctTable
{
   BERR_Code (*Open)(BAST_Handle *, BCHP_Handle, void*, BINT_Handle, const struct BAST_Settings *pDefSettings);
   BERR_Code (*Close)(BAST_Handle);
   BERR_Code (*GetTotalChannels)(BAST_Handle, uint32_t *);
   BERR_Code (*GetChannelDefaultSettings)(BAST_Handle, uint32_t chnNo, struct BAST_ChannelSettings *pChnDefSettings);
   BERR_Code (*OpenChannel)(BAST_Handle, BAST_ChannelHandle *, uint32_t chnNo, const struct BAST_ChannelSettings *pChnDefSettings);
   BERR_Code (*CloseChannel)(BAST_ChannelHandle);
   BERR_Code (*GetDevice)(BAST_ChannelHandle, BAST_Handle *);

   BERR_Code (*InitAp)(BAST_Handle, const uint8_t *);
   BERR_Code (*SoftReset)(BAST_Handle);
   BERR_Code (*GetApStatus)(BAST_Handle, BAST_ApStatus *);
   BERR_Code (*GetApVersion)(BAST_Handle, uint16_t*, uint8_t*, uint32_t*, uint8_t*, uint8_t*);
   BERR_Code (*SetTmConfig)(BAST_Handle, void*);
   BERR_Code (*GetTmConfig)(BAST_Handle, void*);
   BERR_Code (*ConfigGpio)(BAST_Handle, uint32_t, uint32_t);
   BERR_Code (*SetGpio)(BAST_Handle, uint32_t, uint32_t);
   BERR_Code (*GetGpio)(BAST_Handle, uint32_t, uint32_t*);

   BERR_Code (*TuneAcquire)(BAST_ChannelHandle, const uint32_t, const BAST_AcqSettings*);
   BERR_Code (*GetChannelStatus)(BAST_ChannelHandle, BAST_ChannelStatus *);
   BERR_Code (*GetLockStatus)(BAST_ChannelHandle, bool *);
   BERR_Code (*ResetStatus)(BAST_ChannelHandle);
   BERR_Code (*SetDiseqcTone)(BAST_ChannelHandle, bool);
   BERR_Code (*GetDiseqcTone)(BAST_ChannelHandle, bool*);
   BERR_Code (*SetDiseqcVoltage)(BAST_ChannelHandle, bool);
   BERR_Code (*SendDiseqcCommand)(BAST_ChannelHandle, const uint8_t *pSendBuf, uint8_t sendBufLen);
   BERR_Code (*GetDiseqcStatus)(BAST_ChannelHandle, BAST_DiseqcStatus *pStatus);
   BERR_Code (*ResetDiseqc)(BAST_ChannelHandle, uint8_t);
   BERR_Code (*ResetFtm)(BAST_Handle);
   BERR_Code (*ReadFtm)(BAST_Handle, uint8_t *pBuf, uint8_t *n);
   BERR_Code (*WriteFtm)(BAST_Handle, uint8_t *pBuf, uint8_t n);
   BERR_Code (*PowerDownFtm)(BAST_Handle);
   BERR_Code (*PowerUpFtm)(BAST_Handle);

   BERR_Code (*WriteMi2c)(BAST_ChannelHandle, uint8_t, uint8_t*, uint8_t);
   BERR_Code (*ReadMi2c)(BAST_ChannelHandle, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t);
   BERR_Code (*GetSoftDecisionBuf)(BAST_ChannelHandle, int16_t*, int16_t*);
   BERR_Code (*ReadAgc)(BAST_ChannelHandle, BAST_Agc, uint32_t*);
   BERR_Code (*WriteAgc)(BAST_ChannelHandle, BAST_Agc, uint32_t*);
   BERR_Code (*FreezeAgc)(BAST_ChannelHandle, BAST_Agc, bool);
   BERR_Code (*FreezeEq)(BAST_ChannelHandle, bool);
   BERR_Code (*PowerDown)(BAST_ChannelHandle, uint32_t options);
   BERR_Code (*PowerUp)(BAST_ChannelHandle, uint32_t options);
   BERR_Code (*ReadRegister)(BAST_ChannelHandle, uint32_t, uint32_t*);
   BERR_Code (*WriteRegister)(BAST_ChannelHandle, uint32_t, uint32_t*);
   BERR_Code (*ReadConfig)(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);
   BERR_Code (*WriteConfig)(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);

   BERR_Code (*GetInterruptEventHandle)(BAST_Handle, BKNI_EventHandle*);
   BERR_Code (*GetLockStateChangeEventHandle)(BAST_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetFtmEventHandle)(BAST_Handle, BKNI_EventHandle*);
   BERR_Code (*GetDiseqcEventHandle)(BAST_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*HandleInterrupt_isr)(BAST_Handle);
   BERR_Code (*ProcessInterruptEvent)(BAST_Handle);
   BERR_Code (*AbortAcq)(BAST_ChannelHandle);
   BERR_Code (*ConfigLna)(BAST_Handle, BAST_LnaIntConfig, BAST_LnaExtConfig);
   BERR_Code (*GetLnaStatus)(BAST_Handle, BAST_LnaStatus *pStatus);
   BERR_Code (*ConfigAgc)(BAST_Handle, uint32_t ctl);
   BERR_Code (*SendACW)(BAST_ChannelHandle, uint8_t);
   BERR_Code (*GetDiseqcVoltage)(BAST_ChannelHandle, uint8_t*);
   BERR_Code (*GetDiseqcVsenseEventHandles)(BAST_ChannelHandle, BKNI_EventHandle*, BKNI_EventHandle*);
   BERR_Code (*EnableVsenseInterrupts)(BAST_ChannelHandle, bool);
   BERR_Code (*PeakScan)(BAST_ChannelHandle, uint32_t);
   BERR_Code (*GetPeakScanStatus)(BAST_ChannelHandle, BAST_PeakScanStatus*);
   BERR_Code (*GetPeakScanEventHandle)(BAST_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*EnableStatusInterrupts)(BAST_ChannelHandle, bool);
   BERR_Code (*GetStatusEventHandle)(BAST_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*ConfigBcm3445)(BAST_Handle, BAST_Bcm3445Settings*);
   BERR_Code (*MapBcm3445ToTuner)(BAST_ChannelHandle, BAST_Mi2cChannel, BAST_Bcm3445OutputChannel);
   BERR_Code (*GetBcm3445Status)(BAST_ChannelHandle, BAST_Bcm3445Status*);
   BERR_Code (*EnableSpurCanceller)(BAST_ChannelHandle, uint8_t n, BAST_SpurCancellerConfig*);
   BERR_Code (*ResetChannel)(BAST_ChannelHandle, bool);
   BERR_Code (*EnableDiseqcLnb)(BAST_ChannelHandle, bool);
   BERR_Code (*SetSearchRange)(BAST_Handle, uint32_t);
   BERR_Code (*GetSearchRange)(BAST_Handle, uint32_t*);
   BERR_Code (*SetAmcScramblingSeq)(BAST_ChannelHandle, uint32_t, uint32_t, uint32_t, uint32_t);
   BERR_Code (*SetTunerFilter)(BAST_ChannelHandle, uint32_t);
   BERR_Code (*GetSignalDetectStatus)(BAST_ChannelHandle, BAST_SignalDetectStatus*);
   BERR_Code (*SetOutputTransportSettings)(BAST_ChannelHandle, BAST_OutputTransportSettings*);
   BERR_Code (*GetOutputTransportSettings)(BAST_ChannelHandle, BAST_OutputTransportSettings*);
   BERR_Code (*SetDiseqcSettings)(BAST_ChannelHandle, BAST_DiseqcSettings*);
   BERR_Code (*GetDiseqcSettings)(BAST_ChannelHandle, BAST_DiseqcSettings*);
   BERR_Code (*SetNetworkSpec)(BAST_Handle, BAST_NetworkSpec);
   BERR_Code (*GetNetworkSpec)(BAST_Handle, BAST_NetworkSpec*);
   BERR_Code (*ConfigTunerLna)(BAST_Handle, BAST_TunerLnaSettings*);
   BERR_Code (*GetTunerLnaStatus)(BAST_ChannelHandle, BAST_TunerLnaStatus*);
   BERR_Code (*SetFskChannel)(BAST_Handle h, BAST_FskChannel fskTxChannel, BAST_FskChannel fskRxChannel);
   BERR_Code (*GetFskChannel)(BAST_Handle h, BAST_FskChannel *fskTxChannel, BAST_FskChannel *fskRxChannel);
   BERR_Code (*SetPeakScanSymbolRateRange)(BAST_ChannelHandle, uint32_t, uint32_t);
   BERR_Code (*GetPeakScanSymbolRateRange)(BAST_ChannelHandle, uint32_t*, uint32_t*);   
   BERR_Code (*SetAdcSelect)(BAST_ChannelHandle, uint8_t);
   BERR_Code (*GetAdcSelect)(BAST_ChannelHandle, uint8_t*, uint8_t*);
   BERR_Code (*GetVersionInfo)(BAST_Handle, BFEC_VersionInfo*);
} BAST_ApiFunctTable;


/******************************************************************************
Summary:
   Structure containing global BAST PI settings.
Description:
   This structure contains global settings for the BAST PI device.
See Also:
   BAST_Open()
******************************************************************************/
typedef struct BAST_Settings
{
   BAST_I2cSettings    i2c; /* host i2c settings */
   BAST_ApiFunctTable  api; /* API function table for per-chip implementation */
   BAST_NetworkSpec    networkSpec;
} BAST_Settings;


/******************************************************************************
Summary:
   Structure containing BAST device channel settings.
Description:
   This structure contains BAST device channel settings.
See Also:
   BAST_OpenChannel(), BAST_GetChannelDefaultSettings()
******************************************************************************/
typedef struct BAST_ChannelSettings
{
   uint8_t      reserved; /* TBD */
} BAST_ChannelSettings;


/******************************************************************************
Summary:
   Initializes the BAST API.
Description:
   This function initializes a BAST_Handle.  
Returns:
   BERR_Code : BERR_SUCCESS = the returned BAST_Handle is valid
******************************************************************************/
BERR_Code BAST_Open(
   BAST_Handle *h,         /* [out] BAST handle */
   BCHP_Handle hChip,      /* [in] chip handle */
   void *pReg,             /* [in] pointer to register or i2c handle */
   BINT_Handle hInterrupt, /* [in] interrupt handle */
   const BAST_Settings *pDefSettings /* [in] default settings */
);


/******************************************************************************
Summary:
   Closes the BAST API.
Description:
   This function releases all the resources allocated by BAST API and disables 
   BCM4501 host interrupts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_Close(
   BAST_Handle h   /* [in] BAST handle */
);


/******************************************************************************
Summary:
   Gets the total number of logical sub-devices for the BAST device.
Description:
   This function gets the total number of channels supported by the BAST 
   device, e.g. for BCM4501 devices, totalChannels returns the value 2.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetTotalChannels(
   BAST_Handle  h,             /* [in] BAST handle */
   uint32_t     *totalChannels /* [out] number of channels supported */
);


/******************************************************************************
Summary:
   Gets default channel settings.
Description:
   This function gets the default settings for an BAST device channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetChannelDefaultSettings(
   BAST_Handle   h,                      /* [in] BAST handle */
   uint32_t      chnNo,                  /* [in] channel number */
   BAST_ChannelSettings *pChnDefSettings /* [out] default channel settings */
);


/******************************************************************************
Summary:
   Initializes a BAST device channel.
Description:
   This function initializes a BAST device channel and returns a 
   BAST_ChannelHandle.
Returns:
   BERR_Code : BERR_SUCCESS = the returned BAST_ChannelHandle is valid
******************************************************************************/
BERR_Code BAST_OpenChannel(
   BAST_Handle                 h,              /* [in] BAST handle */
   BAST_ChannelHandle         *pChannelHandle, /* [out] BAST channel handle */
   uint32_t                   chnNo,           /* [in] channel number */
   const BAST_ChannelSettings *pSettings       /* [in] channel settings */
);


/******************************************************************************
Summary:
   Closes the BAST device channel.
Description:
   This function frees the channel handle and any resources contained in the
   channel handle.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_CloseChannel(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
);


/******************************************************************************
Summary:
   Returns the BAST_Handle associated with the BAST_ChannelHandle.
Description:
   This function returns the BAST_Handle associated with the BAST_ChannelHandle.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetDevice(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   BAST_Handle *pDev      /* [out] BAST handle */
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
BERR_Code BAST_InitAp(
   BAST_Handle   h,       /* [in] BAST handle */
   const uint8_t *pImage  /* [in] pointer to AP microcode image */
);


/******************************************************************************
Summary:
   Soft reset the tuner and sds.
Description:
   This function resets the BAST PI to its initial state
Returns:
   BERR_Code : BERR_SUCCESS
******************************************************************************/
BERR_Code BAST_SoftReset(
   BAST_Handle   h       /* [in] BAST handle */
);


/******************************************************************************
Summary:
   Reset a specific tuner.
Description:
   This function resets the specified tuner to its initial state
Returns:
   BERR_Code : BERR_SUCCESS
******************************************************************************/
BERR_Code BAST_ResetChannel(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bForceReset       /* [in] true=force reset, false=abort when busy */
);


/******************************************************************************
Summary:
   Returns status of the AP.
Description:
   The state of the AP and the HAB are returned in this function.  If the BAST
   device does not have an AP, then BERR_NOT_SUPPORTED is returned.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetApStatus(
   BAST_Handle   h,         /* [in] BAST device handle */
   BAST_ApStatus *pStatus   /* [out] AP status */
);


/******************************************************************************
Summary:
   Returns AP version information.
Description:
   This function returns the chip revision, AP microcode revision, and 
   configuration version.  This function is deprecated in later chips.  Use
   BAST_GetVersionInfo() instead.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetApVersion(
   BAST_Handle    h,          /* [in] BAST handle */
   uint16_t       *pChipId,   /* [out] chip id */
   uint8_t        *pChipVer,  /* [out] chip revision number */
   uint32_t       *pBondId,   /* [out] chip bonding option */
   uint8_t        *pApVer,    /* [out] AP microcode version */
   uint8_t        *pCfgVer    /* [out] host configuration version */
);


/******************************************************************************
Summary:
   Configures the top mux block.
Description:
   This function reconfigures the top mux.  This function returns 
   BERR_NOT_SUPPORTED if the top mux is not controlled by this BAST device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SetTmConfig(
   BAST_Handle h,     /* [in] BAST handle */
   void        *pCfg  /* [in] TM configuration parameters */
);


/******************************************************************************
Summary:
   Returns the settings of the top mux block.
Description:
   This function returns chip-specific top mux settings.  This function returns
   BERR_NOT_SUPPORTED if the top mux is not controlled by this BAST device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetTmConfig(
   BAST_Handle h,     /* [in] BAST handle */
   void        *pCfg  /* [out] TM configuration parameters */
);


/******************************************************************************
Summary:
   Sets the direction of GPIO pin(s).
Description:
   The GPIO pins selected by "write_mask" are configured as output, and the 
   GPIO pins selected by "read_mask" are configured as input.  Bit 0 
   corresponds to GPIO_0 pin, bit 1 corresponds to GPIO_1 pin, etc.  If the 
   satellite downstream core does not have GPIO pins, then this function 
   returns BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ConfigGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    write_mask,  /* [in] selects which GPIO pins are output */
   uint32_t    read_mask    /* [in] selects which GPIO pins are input */
);


/******************************************************************************
Summary:
   Sets the state of GPIO pin(s).
Description:
   The GPIO pins selected by "mask" are set to the levels given by "state".
   Bit 0 corresponds to GPIO_0 pin, bit 1 corresponds to GPIO_1 pin, etc.  The
   selected GPIO pins should have already been configured as output pins by 
   BAST_ConfigGpio().  If the satellite downstream core does not have GPIO 
   pins, then this function returns BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SetGpio(
   BAST_Handle h,     /* [in] BAST handle */
   uint32_t    mask,  /* [in] selects which GPIO pins to set */
   uint32_t    state  /* [in] state of GPIO pins */
);


/******************************************************************************
Summary:
   Gets the state of GPIO pin(s).
Description:
   The GPIO pins selected by "mask" are read.  The state of the selected pins
   are returned in "state".  Bit 0 corresponds to GPIO_0 pin, bit 1 corresponds
   to GPIO_1 pin, etc.  The selected GPIO pins should have already been 
   configured as input pins by BAST_ConfigGpio().  If the satellite downstream 
   core does not have GPIO pins, then this function returns BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetGpio(
   BAST_Handle h,      /* [in] BAST handle */
   uint32_t    mask,   /* [in] selects which GPIO pins to read */
   uint32_t    *state  /* [out] state of selected GPIO pins */
);


/******************************************************************************
Summary:
   Tunes the tuner and acquires the satellite downstream channel.
Description:
   This function initiates channel acquisition.  This function will return
   before the acquisition has completed.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_TuneAcquire(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   const uint32_t     freq,         /* [in] RF tuner frequency in Hz */
   const BAST_AcqSettings *pParams  /* [in] acquisition parameters */
);


/******************************************************************************
Summary:
   Returns channel status. 
Description:
   This function returns downstream status information in the 
   BAST_ChannelStatus structure.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetChannelStatus(
   BAST_ChannelHandle h,        /* [in] BAST handle */
   BAST_ChannelStatus *pStatus  /* [out] pointer to the status structure */
);


/******************************************************************************
Summary:
   Returns channel lock status. 
Description:
   This function is used to get the demod lock status for a specific downstream
   channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetLockStatus(
   BAST_ChannelHandle h,      /* [in] BAST handle */
   bool *bLocked              /* [out] true if demod is locked */
);


/******************************************************************************
Summary:
   Resets FEC bit error and block counters.
Description:
   This function clears the following counters in BAST_ChannelStatus: 
   mpegErrors, mpegCount, reacqCount, berErrors, rsCorrCount, and rsUncorrCount
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ResetStatus(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
);


/******************************************************************************
Summary:
   Sets the DISEQC 22KHz continuous tone.
Description:
   This function turns on/off the DISEQC 22KHz continuous tone.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bTone             /* [in] true = tone on, false = tone off */
);


/******************************************************************************
Summary:
   Detects a 22KHz tone present.
Description:
   This function determines if a 22KHz tone is present. 
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetDiseqcTone(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool *pbTone           /* [out] true = tone is present, false = tone absent */
);


/******************************************************************************
Summary:
   Sets the DISEQC voltage level.
Description:
   This function sets the DISEQC voltage level to VBOT (13V) or VTOP (18V).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SetDiseqcVoltage(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
);


/******************************************************************************
Summary:
   Gets the DISEQC voltage level.
Description:
   This function determines the DISEQC voltage level.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetDiseqcVoltage(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint8_t *pVoltage      /* [out] voltage estimation */
);


/******************************************************************************
Summary:
   Toggles power state of external DISEQC LNB
Description:
   This function toggles a control signal which controls the power state of an external LNB.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_EnableDiseqcLnb(
   BAST_ChannelHandle h,  /* [in] BAST channel handle */
   bool bEnable           /* [in] true = LNB on, false = LNB off */
);


/******************************************************************************
Summary:
   Toggles the Vsense interrupts.
Description:
   This function enables or disables the over-voltage and under-voltage interrupts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_EnableVsenseInterrupts(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
);


/******************************************************************************
Summary:
   Initiates a DISEQC transaction.
   device.
Description:
   This function transmits sendBufLen bytes from a buffer pointed to by 
   pSendBuf.  The transaction status and any reply bytes are subsequently 
   stored in the PI and can be retrieved by BAST_GetDiseqcStatus() after the 
   transaction has completed.  BAST_GetDiseqcEventHandle() is used to get the
   handle to the Diseqc event, which indicates when the transaction has 
   completed.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SendDiseqcCommand(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   const uint8_t *pSendBuf, /* [in] contains data to send */
   uint8_t sendBufLen       /* [in] number of bytes to send */
);


/******************************************************************************
Summary:
   Gets the status of the most recent Diseqc transaction initiated by 
   BAST_SendDiseqcCommand().
   device.
Description:
   This function sends sendBufLen bytes from pSendBuf to a DISEQC slave device.
   Any reply bytes are returned in pRcvBuf and the number of reply bytes are 
   indicated in replyLen.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetDiseqcStatus(
   BAST_ChannelHandle h,      /* [in] BAST channel handle */
   BAST_DiseqcStatus *pStatus /* [out] status of most recent transaction */
);


/******************************************************************************
Summary:
   Resets the DISEQC block and prepares the BAST PI for DISEQC operation 
Description:
   This function resets the diseqc block to its known initial state.  This 
   function should be called before any other DISEQC function.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ResetDiseqc(
   BAST_ChannelHandle h,    /* [in] BAST channel handle */
   uint8_t options          /* [in] reset options */
);


/******************************************************************************
Summary:
   Sends an Auto Control Word.
Description:
   This function transmits an Auto Control Word.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SendACW(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t acw           /* [in] auto control word to send */
);


/******************************************************************************
Summary:
   Resets the FTM block and prepares the BAST PI for FTM operation
Description:
   This function should be called before any other FTM function.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ResetFtm(
   BAST_Handle    /* [in] BAST handle */
);
#define BAST_ResetFsk(h) BAST_ResetFtm(h)


/******************************************************************************
Summary:
   Read FTM message.
Description:
   This function gets the message or FTM packet.  This function should be 
   called when the FTM Event is set.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ReadFtm(
   BAST_Handle,    /* [in] BAST handle */
   uint8_t *pBuf,  /* [out] data read */
   uint8_t *n      /* [out] length of data read */
);
#define BAST_ReadFsk(h, pBuf, n) BAST_ReadFtm(h, pBuf, n)


/******************************************************************************
Summary:
   Write FTM message or packet.
Description:
   This function sends an FTM message. 
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_WriteFtm(
   BAST_Handle,    /* [in] BAST handle */
   uint8_t *pBuf,  /* [in] data to write */
   uint8_t n       /* [in] length of data to write */
);
#define BAST_WriteFsk(h, pBuf, n) BAST_WriteFtm(h, pBuf, n)


/******************************************************************************
Summary:
   Power down the FTM block.
Description:
   This function powers down the FTM block.  No other FTM function may be 
   called while FTM is powered down.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_PowerDownFtm(
   BAST_Handle   /* [in] BAST handle */
);
#define BAST_PowerDownFsk(h) BAST_PowerDownFtm(h)


/******************************************************************************
Summary:
   Power up the FTM block.
Description:
   This function should be called to power up the FTM block after it has been
   powered down by the BAST_PowerDownFtm() call.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_PowerUpFtm(
   BAST_Handle   /* [in] BAST handle */
);
#define BAST_PowerUpFsk(h) BAST_PowerUpFtm(h)


/******************************************************************************
Summary:
   Initiates an I2C write transaction from the SDS core's master I2C controller.
Description:
   This function programs the SDS master i2c controller to transmit the data 
   given in buf[].  If there is no SDS master i2c controller, this function 
   will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_WriteMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *buf,         /* [in] specifies the data to transmit */
   uint8_t n             /* [in] number of bytes to transmit after the i2c slave address */
);


/******************************************************************************
Summary:
   Initiates an I2C read transaction from the SDS master I2C controller.
Description:
   This function programs the SDS master I2C controller to transmit the data 
   given in buf[].  If there is no SDS master i2c controller, this function 
   will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ReadMi2c(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint8_t slave_addr,   /* [in] address of the i2c slave device */
   uint8_t *out_buf,     /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n,        /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf,      /* [out] holds the data read */
   uint8_t in_n          /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
);


/******************************************************************************
Summary:
   Gets I/Q soft decisions.
Description:
   This function is mainly used to provide data for a constellation display.  
   This function returns 15 pairs of I/Q values.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetSoftDecisionBuf(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   int16_t *pI,           /* [out] 15 I-values */
   int16_t *pQ            /* [out] 15 Q-values */
);


/******************************************************************************
Summary:
   Read RF/IF AGC
Description:
   This function gets the integrator value of the RF/IF AGC
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ReadAgc(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   BAST_Agc,              /* [in] AGC select */
   uint32_t *pAgc         /* [out] integrator value */
);


/******************************************************************************
Summary:
   Write RF/IF AGC
Description:
   This function sets the integrator value of the RF/IF AGC
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_WriteAgc(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   BAST_Agc,              /* [in] AGC select */
   uint32_t *pAgc         /* [in] integrator value */
);


/******************************************************************************
Summary:
   Freeze/unfreeze the RF/IF AGC
Description:
   This function freezes/unfreezes the RF/IF AGC
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_FreezeAgc(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   BAST_Agc,              /* [in] AGC select */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
);


/******************************************************************************
Summary:
   Freeze/unfreeze the Equalizer
Description:
   This function freezes/unfreezes the Equalizer
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_FreezeEq(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   bool bFreeze           /* [in] true = freeze, false = unfreeze */
);


/******************************************************************************
Summary:
   Power down the specified core(s) in the downstream channel.
Description:
   This function shuts off the power in the specified core(s).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_PowerDown(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
);


/******************************************************************************
Summary:
   Power up the Satellite Downstream Core.
Description:
   Call this function to power up the specified core(s) after it has been 
   powered down with BAST_PowerDown().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_PowerUp(
   BAST_ChannelHandle h,  /* [in] BAST handle */
   uint32_t options       /* [in] see BAST_CORE_* macros */
);


/******************************************************************************
Summary:
   Reads a register.
Description:
   This function reads a register.  This function is normally used for 
   non memory-mapped BAST devices, such as BCM4501.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ReadRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [in] contains data that was read */
);


/******************************************************************************
Summary:
   Writes a register.
Description:
   This function writes to a register.  This function is normally used for 
   non memory-mapped BAST devices, such as BCM4501.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_WriteRegister(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to write */
   uint32_t           *val   /* [in] contains data to write */
);


/******************************************************************************
Summary:
   Reads from the configuration space.
Description:
   This function reads from the configuration space.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ReadConfig(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint16_t id,              /* [in] configuration parameter ID */
   uint8_t *buf,             /* [out] bytes read */
   uint8_t n                 /* [in] length of configuration parameter */
);


/******************************************************************************
Summary:
   Writes to the configuration space.
Description:
   This function writes to the configuration space.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_WriteConfig(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint16_t id,              /* [in] configuration parameter ID */
   uint8_t *buf,             /* [in] bytes to write */
   uint8_t n                 /* [in] length of configuration parameter */
);


/******************************************************************************
Summary:
   This function returns the BAST interrupt event handle.
Description:
   This function is used for an i2c-controlled BAST device only.  The 
   application needs to call this function to get the event handle for BAST 
   interrupts.  The application will wait on this interrupt event.  When this 
   event is signalled, the application must call BAST_ProcessInterruptEvent().
   For memory-mapped BAST devices, this function will return 
   BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetInterruptEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
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
BERR_Code BAST_GetLockStateChangeEventHandle(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   BKNI_EventHandle *hEvent  /* [out] lock event handle */
);


/******************************************************************************
Summary:
   Returns the FTM event handle.
Description:
   The application is notified by the FTM event when there is an FTM message to
   be read.  This function returns the handle to the FTM event.  When the FTM 
   event is signalled, the application should call BAST_ReadFtm() to retrieve
   the message. 
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetFtmEventHandle(
   BAST_Handle h,            /* [in] BAST handle */
   BKNI_EventHandle *hEvent  /* [out] FTM event handle */
);
#define BAST_GetFskEventHandle(h, hEvent) BAST_GetFtmEventHandle(h, hEvent)


/******************************************************************************
Summary:
   Returns the Diseqc event handle.
Description:
   The application is notified by the Diseqc event when the diseqc transaction
   that has been initiated by BAST_SendDiseqcCommand() has completed.  This 
   function returns the handle to the Diseqc event.  When the Diseqc event is 
   set, the application should call BAST_GetDiseqcStatus() to get the 
   transaction status and any reply bytes received.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetDiseqcEventHandle(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BKNI_EventHandle   *hEvent  /* [out] Diseqc event handle */
);


/******************************************************************************
Summary:
   Returns the Diseqc over-voltage & under-voltage event handle.
Description:
   The application is notified by the Diseqc over-voltage event when the voltage
   exceeds the upper voltage threshold.  Similarly the Diseqc under-voltage event 
   notifies the application when the voltage falls below the lower threshold.
   This function returns the handles to both Diseqc events.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetDiseqcVsenseEventHandles(
   BAST_ChannelHandle h,                        /* [in] BAST channel handle */
   BKNI_EventHandle *hDiseqcOverVoltageEvent,   /* [out] Diseqc event handle */
   BKNI_EventHandle *hDiseqcUnderVoltageEvent   /* [out] Diseqc event handle */
);


/******************************************************************************
Summary:
   Handle BAST interrupts.
Description:
   This function must be called when the host receives an interrupt from the 
   BAST device.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_HandleInterrupt_isr(
   BAST_Handle h   /* [in] BAST handle */
);


/******************************************************************************
Summary:
   This function processes BAST interrupt events.
Description:
   This function is used for an i2c-controlled BAST device only.  The 
   application calls this function when the BAST interrupt event has been 
   signalled.  This function will decode the interrupt and set the approriate 
   events.  For memory-mapped BAST devices (e.g. not the BCM4501), this 
   function will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ProcessInterruptEvent(
   BAST_Handle h  /* [in] BAST device handle */
);


/******************************************************************************
Summary:
   Aborts the current acquisition.
Description:
   This function aborts the current acquisition started by the most recent 
   call to BAST_TuneAcquire().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_AbortAcq(
   BAST_ChannelHandle h  /* [in] BAST channel handle */
);


/******************************************************************************
Summary:
   Configure the LNA.
Description:
   This function configures the external BCM3445 LNA.
   NOTE: Additional configuration options will no longer be supporteed for 
   BAST_ConfigLna().  Use BAST_ConfigBcm3445() instead, as BAST_ConfigLna()
   will be deprecated.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ConfigLna(
   BAST_Handle        h,          /* [in] BAST device handle */
   BAST_LnaIntConfig  int_config, /* [in] BCM3445 internal switch configuration */
   BAST_LnaExtConfig  ext_config  /* [in] BCM3445 output mapping to SDS channels */
);


/******************************************************************************
Summary:
   Return LNA status.
Description:
   This function returns LNA status.  Use this function after the LNA was 
   configured with BAST_ConfigLna().
   NOTE: This function will be deprecated.  Use BAST_ConfigBcm3445(), 
   BAST_MapBcm3445ToTuner(), and BAST_GetBcm3445Status() for managing the BCM3445
   instead.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetLnaStatus(
   BAST_Handle h,           /* [in] BAST device handle */
   BAST_LnaStatus *pStatus  /* [out] lna status struct */
);


/******************************************************************************
Summary:
   Configure AGC pins.
Description:
   This function configures the AGC pins (chip/core specific).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ConfigAgc(
   BAST_Handle h,    /* [in] BAST device handle */
   uint32_t    ctl   /* [in] control word, see BAST_AGC_CONFIG_* bit definitions */
);


/******************************************************************************
Summary:
   Initiates an input power scan.
Description:
   Scans the symbol rate range given by BLIND_SCAN_SYM_RATE_MIN and 
   BLIND_SCAN_SYM_RATE_MAX configuration parameters at the specified frequency.
   If BLIND_SCAN_SYM_RATE_MIN or BLIND_SCAN_SYM_RATE_MAX is 0, this function 
   will scan for an unmodulated carrier tone.  When the scan completes, the 
   hPeakScanEvent will be set, indicating to the host that it should query the
   status using BAST_GetPeakScanStatus().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_PeakScan(
   BAST_ChannelHandle h,           /* [in] BAST channel handle */
   uint32_t           tunerFreq    /* [in] frequency in Hz */
);


/******************************************************************************
Summary:
   Get the status of the most recent scan initiated by BAST_PeakScan().
Description:
   This function returns the status of the most recent scan initiated by 
   BAST_PeakScan();
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetPeakScanStatus(
   BAST_ChannelHandle  h,          /* [in] BAST channel handle */
   BAST_PeakScanStatus *pStatus    /* [out] Diseqc event handle */
);


/******************************************************************************
Summary:
   Returns the peak scan done event handle.
Description:
   The peak scan done event is set when the peak scan initiated by 
   BAST_PeakScan() completes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetPeakScanEventHandle(
   BAST_ChannelHandle h,                /* [in] BAST channel handle */
   BKNI_EventHandle  *hPeakScanEvent    /* [out] Peak scan done event handle */
);


/******************************************************************************
Summary:
   Enables/disables status interrupts
Description:
   This function enables or disables the status interrupts.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_EnableStatusInterrupts(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
);


/******************************************************************************
Summary:
   Returns the status event handle.
Description:
   Returns the status event handle.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetStatusEventHandle(
   BAST_ChannelHandle h,        /* [in] BAST channel handle */
   BKNI_EventHandle  *hEvent    /* [out] status event handle */
);


/******************************************************************************
Summary:
   Configures an external BCM3445 LNA.
Description:
   Call this function for each BCM3445 to be managed by the BAST PI.  This 
   function configures the BCM3445 on the specified SDS MI2C bus.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ConfigBcm3445(
   BAST_Handle h,                  /* [in] BAST device handle */
   BAST_Bcm3445Settings *pSettings /* [in] BCM3445 configuration settings */
);


/******************************************************************************
Summary:
   Associates a BCM3445 output channel to this channel's tuner.
Description:
   Call this function after configuring each BCM3445 with BAST_ConfigBcm3445.
   BAST_MapBcm3445ToTuner tells the BAST PI which BCM3445 output port will go 
   to the tuner.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_MapBcm3445ToTuner(
   BAST_ChannelHandle        h,    /* [in] BAST channel handle */
   BAST_Mi2cChannel          mi2c, /* [in] specifies which BCM3445 */
   BAST_Bcm3445OutputChannel out   /* [in] BCM3445 output channel */
);


/******************************************************************************
Summary:
   Get BCM3445 status.
Description:
   This function gets the status of the BCM3445 that is connected to the
   channel.  The BCM3445 should be configured and mapped to the tuner's input
   prior to calling BAST_GetBcm3445Status().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetBcm3445Status(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   BAST_Bcm3445Status *pStatus /* [out] BCM3445 status struct */
);


/******************************************************************************
Summary:
   Configures internal Tuner LNA.
Description:
   This function configures the internal Tuner LNA.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_ConfigTunerLna(
   BAST_Handle h,                   /* [in] BAST device handle */
   BAST_TunerLnaSettings *pSettings /* [in] Tuner LNA configuration settings */
);


/******************************************************************************
Summary:
   Get internal Tuner LNA status.
Description:
   This function gets the status of the internal Tuner LNA.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetTunerLnaStatus(
   BAST_ChannelHandle h,         /* [in] BAST channel handle */
   BAST_TunerLnaStatus *pStatus  /* [out] Tuner LNA status struct */
);


/******************************************************************************
Summary:
   Enable/disable spur cancellers.
Description:
   This function configures up to 6 CW cancellers.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_EnableSpurCanceller(
   BAST_ChannelHandle h,             /* [in] BAST channel handle */
   uint8_t            n,             /* [in] number of spurs to cancel (i.e. size of pConfig array), maximum is 6 */
   BAST_SpurCancellerConfig *pConfig /* [in] array of spur settings */
);


/******************************************************************************
Summary:
   Set carrier search range.
Description:
   This function sets the search range to be used on all acquisitions.  The
   demod will search +/-searchRange relative to the tuning frequency specified
   in BAST_TuneAcquire().  Default search range is 5000000 (i.e. +/- 5MHz).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SetSearchRange(
   BAST_Handle h,         /* [in] BAST device handle */
   uint32_t searchRange   /* [in] search range in Hz */
);


/******************************************************************************
Summary:
   Get carrier search range.
Description:
   This function gets the search range used in all acquisitions.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetSearchRange(
   BAST_Handle h,          /* [in] BAST device handle */
   uint32_t *pSearchRange  /* [out] search range in Hz */
);


/******************************************************************************
Summary:
   Specifies the AMC scrambling sequence.
Description:
   This function specifies the AMC scrambling sequence used in LDPC
   acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SetAmcScramblingSeq(
   BAST_ChannelHandle h, /* [in] BAST channel handle */
   uint32_t xseed,       /* [in] physical layer scrambler seed */
   uint32_t plhdrscr1,   /* [in] PL Header Scrambling Sequence Bit[31:0] */
   uint32_t plhdrscr2,   /* [in] PL Header Scrambling Sequence Bit[63:32] */
   uint32_t plhdrscr3    /* [in] PL Header Scrambling Sequence Bit[89:64] */
);


/******************************************************************************
Summary:
   Set tuner filter cutoff.
Description:
   This function sets the cutoff frequency of the low pass filter in the 
   internal tuner.  If cutoffHz is set to 0, then the tuner filter will be 
   set automatically by the firmware.  This function is typically used for
   testing only.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_SetTunerFilter(
   BAST_ChannelHandle h,       /* [in] BAST channel handle */
   uint32_t           cutoffHz /* [in] filter cutoff in Hz */
);


/******************************************************************************
Summary:
   Get the status of the most recent signal detect operation.
Description:
   Signal detect mode is a quick acquisition to lock the timing loop and verify
   there is a signal at a specified frequency.  Signal detect mode is initiated
   when BAST_ACQSETTINGS_SIGNAL_DETECT_MODE is specified in BAST_TuneAcquire().
   BAST_GetSignalDetectStatus() returns the status of the acquisition in signal
   detect mode.  
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_GetSignalDetectStatus(
   BAST_ChannelHandle      h,       /* [in] BAST channel handle */
   BAST_SignalDetectStatus *pStatus /* [out] returned status */
);

   
/******************************************************************************
Summary:
   Configure the MPEG transport output settings.
Description:
   Configure the MPEG transport output settings.
Returns:
   BERR_Code
******************************************************************************/   
BERR_Code BAST_SetOutputTransportSettings(
   BAST_ChannelHandle h,                   /* [in] BAST channel handle */
   BAST_OutputTransportSettings *pSettings /* [in] transport settings */
);


/******************************************************************************
Summary:
   Get the MPEG transport output settings.
Description:
   Get the MPEG transport output settings.
Returns:
   BERR_Code
******************************************************************************/   
BERR_Code BAST_GetOutputTransportSettings(
   BAST_ChannelHandle h,                   /* [in] BAST channel handle */
   BAST_OutputTransportSettings *pSettings /* [out] transport settings */
);


/******************************************************************************
Summary:
   Configure the DiSEqC settings.
Description:
   Configure the DiSEqC settings.
Returns:
   BERR_Code
******************************************************************************/   
BERR_Code BAST_SetDiseqcSettings(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   BAST_DiseqcSettings *pSettings   /* [in] diseqc settings */
);


/******************************************************************************
Summary:
   Get the DiSEqC settings.
Description:
   Get the DiSEqC settings.
Returns:
   BERR_Code
******************************************************************************/   
BERR_Code BAST_GetDiseqcSettings(
   BAST_ChannelHandle h,            /* [in] BAST channel handle */
   BAST_DiseqcSettings *pSettings   /* [out] diseqc settings */
);


/******************************************************************************
Summary:
   Set the network spec.
Description:
   Set the network spec.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_SetNetworkSpec(
   BAST_Handle      h,            /* [in] BAST device handle */
   BAST_NetworkSpec networkSpec   /* [in] network spec */
);

   
/******************************************************************************
Summary:
   Get the network spec.
Description:
   Get the network spec.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_GetNetworkSpec(
   BAST_Handle      h,             /* [in] BAST device handle */
   BAST_NetworkSpec *pNetworkSpec  /* [out] network spec */
);


/******************************************************************************
Summary:
   Set the channels for FSK Tx/Rx.
Description:
   Set the channels for FSK Tx/Rx.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_SetFskChannel(
   BAST_Handle     h,               /* [in] BAST device handle */
   BAST_FskChannel fskTxChannel,    /* [in] channel for fsk transmit */
   BAST_FskChannel fskRxChannel     /* [in] channel for fsk receive */
);

   
/******************************************************************************
Summary:
   Get the channels for FSK Tx/Rx.
Description:
   Get the channels for FSK Tx/Rx.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_GetFskChannel(
   BAST_Handle     h,               /* [in] BAST device handle */
   BAST_FskChannel *fskTxChannel,   /* [out] channel for fsk transmit */
   BAST_FskChannel *fskRxChannel    /* [out] channel for fsk receive */
);


/******************************************************************************
Summary:
   Set the symbol rate range used in peak scan function.
Description:
   This function sets the minimum and maximum symbol rate that will be searched
   in the BAST_PeakScan() function.  For CW tone detection, set minSymbolRate
   and maxSymbolRate to 0 before calling BAST_PeakScan().
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_SetPeakScanSymbolRateRange(
   BAST_ChannelHandle h,             /* [in] BAST channel handle */
   uint32_t           minSymbolRate, /* [in] minimum symbol rate in sym/sec */
   uint32_t           maxSymbolRate  /* [in] maximum symbol rate in sym/sec */
);


/******************************************************************************
Summary:
   Get the symbol rate range used in peak scan function.
Description:
   This function returns the minimum and maximum symbol rates used by the
   BAST_PeakScan() function.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_GetPeakScanSymbolRateRange(
   BAST_ChannelHandle h,               /* [in] BAST channel handle */
   uint32_t           *pMinSymbolRate, /* [out] minimum symbol rate in sym/sec */
   uint32_t           *pMaxSymbolRate  /* [out] maximum symbol rate in sym/sec */
);  


/******************************************************************************
Summary:
   Selects the ADC for the channel.
Description:
   This function configures the crossbar to select an ADC for the channel.  This
   feature is applicable on chips that have multiple wideband tuners such as 
   BCM4538.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_SetAdcSelect(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint8_t adcSelect       /* [in] specifies the ADC input for the channel */
);


/******************************************************************************
Summary:
   Returns the ADC currently selected for the channel.
Description:
   This function returns the crossbar configuration.  This feature is applicable
   on chips that have multiple wideband tuners such as BCM4538.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_GetAdcSelect(
   BAST_ChannelHandle h,   /* [in] BAST channel handle */
   uint8_t *pAdcSelect,    /* [out] current ADC selection for the channel */
   uint8_t *pNumAdc        /* [out] number of wideband ADCs on the chip */
);


/******************************************************************************
Summary:
   Returns AST PI version information
Description:
   This function returns AST PI version information in the FEC_DeviceVersionInfo
   struct, which is driver version information common to all Magnum frontends.
Returns:
   BERR_Code
******************************************************************************/  
BERR_Code BAST_GetVersionInfo(
   BAST_Handle h,             /* [in] BAST device handle */
   BFEC_VersionInfo *pVersion /* [out] version information */
);


/******************************************************************************
Summary:
   Fixed point multiply (P = A * B).
Description:
   This function multiplies 2 unsigned 32-bit numbers and returns a 64-bit product (P_hi and P_lo).
Returns:
   BERR_Code
******************************************************************************/
void BAST_MultU32U32(
   uint32_t A,     /* [in] first operand */
   uint32_t B,     /* [in] second operand */
   uint32_t *P_hi, /* [out] bits 63:32 of product */
   uint32_t *P_lo  /* [out] bits 31:0 of product */
);


/******************************************************************************
Summary:
   Fixed point divide (Q = A / B).
Description:
   This function divides an unsigned 64-bit number by an unsigned 32-bit number.
Returns:
   BERR_Code
******************************************************************************/
void BAST_DivU64U32(
   uint32_t A_hi,   /* [in] bits 63:32 of dividend */
   uint32_t A_lo,   /* [in] bits 31:0 of dividend */
   uint32_t B,      /* [in] divisor */
   uint32_t *Q_hi,  /* [out] bits 63:32 of quotient */
   uint32_t *Q_lo   /* [out] bits 31:0 of quotient */
);


#ifdef __cplusplus
}
#endif

#endif /* BAST_H__ */

