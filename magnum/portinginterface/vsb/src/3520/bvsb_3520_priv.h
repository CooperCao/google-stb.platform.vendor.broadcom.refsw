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
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef _BVSB_3520_PRIV_H__
#define _BVSB_3520_PRIV_H__

#include "bvsb_3520.h"


#define BVSB_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

#define BVSB_WriteHostRegister(h, offset, val) \
   BREG_I2C_Write(((BVSB_3520_P_Handle *)(h->pImpl))->hRegister, h->settings.i2c.chipAddr, offset, val, 1)

#define BVSB_ReadHostRegister(h, offset, val) \
   BREG_I2C_Read(((BVSB_3520_P_Handle *)(h->pImpl))->hRegister, h->settings.i2c.chipAddr, offset, val, 1)

/* window addresses passed to BVSB_P_SetApWindow() */
#define BVSB_WINDOW_IRAM      0x00000000
#define BVSB_WINDOW_IROM      0x00008000
#define BVSB_WINDOW_HAB       0x00010000
#define BVSB_WINDOW_IDATA     0x00010400

/* bit definitions used in audio HAB commands */
#define BVSB_AUDIO_CONFIG_DECODE_MODE_MASK     0x03
#define BVSB_AUDIO_CONFIG_DECODE_MODE_MONO     0x00
#define BVSB_AUDIO_CONFIG_DECODE_MODE_STEREO   0x01
#define BVSB_AUDIO_CONFIG_DECODE_MODE_SAP      0x02
#define BVSB_AUDIO_CONFIG_DECODE_MODE_SAP_MONO 0x03
#define BVSB_AUDIO_CONFIG_SR_MASK              0x0C
#define BVSB_AUDIO_CONFIG_SR_32KHZ             0x00
#define BVSB_AUDIO_CONFIG_SR_44_1KHZ           0x04
#define BVSB_AUDIO_CONFIG_SR_48KHZ             0x08
#define BVSB_AUDIO_CONFIG_DAC_SELECT_MASK      0x20
#define BVSB_AUDIO_CONFIG_DAC_SELECT_JDA       0x00
#define BVSB_AUDIO_CONFIG_DAC_SELECT_HIFIDAC   0x20
#define BVSB_AUDIO_CONFIG_SEL_MASK             0x40
#define BVSB_AUDIO_CONFIG_SEL_AUDIO_DAC        0x00
#define BVSB_AUDIO_CONFIG_SEL_I2S              0x40
#define BVSB_AUDIO_CONFIG_MUTE                 0x80
#define BVSB_AUDIO_SAP_DETECTED                0x01
#define BVSB_AUDIO_STEREO_DETECTED             0x02


/***************************************************************************
Summary:
	Structure for chip-specific VSB handle

Description:
	This is the chip-specific component of the BVSB_Handle.

See Also:
	None.

****************************************************************************/
typedef struct BVSB_3520_P_Handle
{
   BREG_I2C_Handle  hRegister;         /* handle to the master I2C device used to access registers */
   BKNI_EventHandle hInterruptEvent;   /* interrupt event handle */   
   BKNI_EventHandle hApiEvent;         /* API event handle */   
   BKNI_EventHandle hLockChangeEvent;  /* change of lock status event handle */
   BKNI_EventHandle hOobLockChangeEvent;  /* change of lock status event handle */
   BKNI_EventHandle hInitDoneEvent;    /* AP initialization done event handle */
   BKNI_EventHandle hHabDoneEvent;     /* HAB done event handle */
   BKNI_EventHandle hAntennaEvent;     /* Antenna event handle */
   BKNI_EventHandle hBscEvent;         /* BSC event handle */
   uint8_t          last_page_16_15;   /* most recent position of the 128 byte window into the AP address space */
   uint8_t          last_page_14_7;    /* most recent position of the 128 byte window into the AP address space */
   uint8_t          last_mbox_15_8;    /* most recent position of the IO MBOX window */
   uint8_t          last_bsc_slave_addr; /* last BSC slave addr */
} BVSB_3520_P_Handle;

/***************************************************************************
Summary:
	Enum for pad drive strength

Description:
	This enum specifies the pad drive strength.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_3520_PadDriveStrength
{
   BVSB_3520_PadDriveStrength_e6mA = 0,  /* 6 mA */
   BVSB_3520_PadDriveStrength_e4mA,      /* 4 mA */
   BVSB_3520_PadDriveStrength_e2mA,      /* 2 mA */
   BVSB_3520_PadDriveStrength_eTRISTATE  /* tristate */
} BVSB_3520_PadDriveStrength;

/***************************************************************************
Summary:
	Enum for audio pad drive strength

Description:
	This enum specifies the audio pad drive strength.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_3520_AudioPadDriveStrength
{
   BVSB_3520_AudioPadDriveStrength_e12mA = 0,  /* 12 mA */
   BVSB_3520_AudioPadDriveStrength_e8mA,       /* 8 mA */
   BVSB_3520_AudioPadDriveStrength_e4mA,       /* 4 mA */
   BVSB_3520_AudioPadDriveStrength_eTRISTATE   /* tristate */
} BVSB_3520_AudioPadDriveStrength;

/***************************************************************************
Summary:
	Enum for output format of the DS_CLK_F1B_ADC pin

Description:
	This enum specifies the output format of the DS_CLK_F1B_ADC pin.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_F1BADC_OutputFormat 
{
   BVSB_F1BADC_OutputFormat_eNothing = 0,  /* output nothing */
   BVSB_F1BADC_OutputFormat_eAD_clk = 2,   /* output A/D sample clock */
   BVSB_F1BADC_OutputFormat_eBaud_clk = 3  /* output pulse swallowed baud clock */
} BVSB_F1BADC_OutputFormat; 

/***************************************************************************
Summary:
	Enum for output format of the DS_CLK_ADC pin

Description:
	This enum specifies the output format of the DS_CLK_ADC pin.

See Also:
	None.

****************************************************************************/
typedef enum BVSB_ADCSEL_OutputFormat 
{
   BVSB_ADCSEL_OutputFormat_eNothing = 0,   /* output nothing */
   BVSB_ADCSEL_OutputFormat_eDelayedReset, /* output delayed reset signal */
   BVSB_ADCSEL_OutputFormat_eAD_clk         /* output A/D sample clock */
} BVSB_ADCSEL_OutputFormat;


/***************************************************************************
Summary:
	Structure for pad configuration

Description:
	This structure contains pad configuration parameters

See Also:
	None.

****************************************************************************/
typedef struct BVSB_PadConfig
{
   BVSB_3520_PadDriveStrength driveStrength_ps_data_7; /* DS_PS_DATA_7 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_data_6; /* DS_PS_DATA_6 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_data_5; /* DS_PS_DATA_5 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_data_4; /* DS_PS_DATA_4 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_data_3; /* DS_PS_DATA_3 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_data_2; /* DS_PS_DATA_2 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_data_1; /* DS_PS_DATA_1 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_data_0; /* DS_PS_DATA_0 pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_clk; /* DS_PS_CLK pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_err; /* DS_PS_ERR pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_valid; /* DS_PS_VALID pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_ps_sync; /* DS_PS_SYNC pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_oob_clk; /* OOB_CLK pin drive strength */
   BVSB_3520_PadDriveStrength driveStrength_oob_sdata; /* OOB_SDATA pin drive strength */
   BVSB_3520_AudioPadDriveStrength driveStrength_rn_mclk; /* AUD_Rn_MCLK pin drive strength */
   BVSB_3520_AudioPadDriveStrength driveStrength_rp_sclk; /* AUD_Rp_SCLK pin drive strength */
   BVSB_3520_AudioPadDriveStrength driveStrength_ln_lr; /* AUD_Ln_LR pin drive strength */
   BVSB_3520_AudioPadDriveStrength driveStrength_lp_data; /* AUD_Lp_DATA pin drive strength */
   bool bTristate_clk_adc_f1b; /* true=DS_CLK_ADC_F1B pin tristate */
   bool bTristate_clk_adc; /* true=DS_CLK_ADC pin tristate */
   bool bTristate_gpo_7; /* true=GPO_7 pin tristate */
   bool bTristate_gpo_6; /* true=GPO_6 pin tristate */
   bool bTristate_gpo_5; /* true=GPO_5 pin tristate */
   bool bTristate_gpo_4; /* true=GPO_4 pin tristate */
   bool bTristate_gpo_3; /* true=GPO_3 pin tristate */
   bool bTristate_gpo_2; /* true=GPO_2 pin tristate */
   bool bTristate_gpo_1; /* true=GPO_1 pin tristate */
   bool bTristate_gpo_0; /* true=GPO_0 pin tristate */
   BVSB_F1BADC_OutputFormat f1bsel;  /* DS_CLK_F1B_ADC pin output format */
   BVSB_ADCSEL_OutputFormat adcsel;  /* DS_CLK_ADC pin output format */

   bool bOpenDrain_hm; /* IF demod hsync output */
   bool bOpenDrain_gm; /* general purpose delta sigma output */
   bool bOpenDrain_am; /* DS_AGCT_CTL pin output */
   bool bOpenDrain_im; /* DS_AGCI_CTL pin output */
   bool bOpenDrain_om; /* OOB_AGC pin output */

   bool bErs;         /* PS_ERR function: false=TEI, true=lock indicator */
   bool bTristate_at; /* true = DS_AGCT_CTL pin is tristate */
   bool bTristate_it; /* true = DS_AGCI_CTL pin is tristate */
   bool bTristate_ot; /* true = OOB_AGCI pin is tristate */
   uint8_t gpio_01;   /* GPIO_0/GPIO_1 function selection (see 3520 user manual) */
   uint8_t oob_gpo;   /* OOB_GPO control word (see 3520 user manual) */
   uint8_t agctsel;   /* AGCTSEL control word (see 3520 user manual) */
   uint8_t agcisel;   /* AGCISEL control word (see 3520 user manual) */
   uint8_t agcgpsel;  /* AGCGPSEL control word (see 3520 user manual) */
} BVSB_3520_PadConfig;


/******************************************************************************
Summary:
   Reads a register in an internal core via the IO_MBOX interface.
Description:
   This function uses the IO_MBOX interface to access the RBUS.  This function 
   assumes that the accessed core has already been unlocked in the bus bridge,
   which is the default in BCM3520-A1 and later.  
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_ReadMbox(
   BVSB_Handle h,    /* [in] BVSB PI Handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [out] value read from register */
);


/******************************************************************************
Summary:
   Writes to a register in an internal core via the IO_MBOX interface.
Description:
   This function uses the IO_MBOX interface to access the RBUS.  This function
   assumes that the accessed core has already been unlocked in the bus bridge,
   which is the default in BCM3520-A1 and later.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_WriteMbox(
   BVSB_Handle h,    /* [in] BVSB PI Handle */
   uint16_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [in] value to write */
);


/******************************************************************************
Summary:
   Reads from the host access buffer.
Description:
   This function positions the 128 byte window in the AP address space to the
   host access buffer area.  (n) bytes are then read into (buf) starting from 
   the given HAB (offset) address.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_ReadHab(
   BVSB_Handle h,   /* [in] BVSB PI Handle */
   uint8_t offset,  /* [in] starting offset within the HAB to read */
   uint8_t *buf,    /* [out] holds the data read */
   uint8_t n        /* [in] number of bytes to read */ 
);


/******************************************************************************
Summary:
   Writes to the host access buffer.
Description:
   This function positions the 128 byte window in the AP address space to the
   host access buffer area.  (n) bytes in (buf) are then written starting from 
   the given HAB (offset) address.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_WriteHab(
   BVSB_Handle h,   /* [in] BVSB PI Handle */
   uint8_t offset,  /* [in] starting offset in the HAB to write */
   uint8_t *buf,    /* [in] specifies the data to write */
   uint8_t n        /* [in] number of bytes to write */
);


/******************************************************************************
Summary:
   Reads from the AP RAM/ROM space.
Description:
   This function is normally used for verifying the contents of the AP RAM 
   after microcode download.  This function assumes that the AP is already in 
   idle or reset state.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_ReadMemory(
   BVSB_Handle h,  /* [in] BVSB PI Handle */
   uint16_t addr,  /* [in] starting address */
   uint8_t *buf,   /* [out] holds the data read */
   uint16_t n      /* [in] number of bytes to read */
);


/******************************************************************************
Summary:
   Writes to the AP RAM space.
Description:
   This function is normally used for downloading the microcode.  This function
   assumes that the AP is already in idle or reset state.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_WriteMemory(
   BVSB_Handle h, /* [in] BVSB PI Handle */ 
   uint16_t addr, /* [in] starting address in AP RAM */
   uint8_t *buf,  /* [in] specifies the data to write */
   uint16_t n     /* [in] number of bytes to write */
);


/******************************************************************************
Summary:
   Positions the 128 byte window in the AP address space.
Description:
   The last 128-byte window position is stored in the BVSB_Handle struct,
   so I2C accesses will occur only if the window has moved.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_SetApWindow(
   BVSB_Handle h,    /* [in] BVSB PI Handle */
   uint32_t window   /* [in] base address of the 128-byte window */
);


/******************************************************************************
Summary:
   Resets the AP.
Description:
   This function puts the AP in reset state.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_ResetAp(
   BVSB_Handle h   /* [in] BVSB PI Handle */
);


/******************************************************************************
Summary:
   Runs the AP.
Description:
   This function takes the AP to out of reset/idle state and starts running.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_RunAp(
   BVSB_Handle h   /* [in] BVSB PI Handle */
);


/******************************************************************************
Summary:
   Disables all host interrupts.
Description:
   This function clears the host interrupt enable bits on the BCM3520.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_DisableInterrupts(
   BVSB_Handle h   /* [in] BVSB PI Handle */
);


/******************************************************************************
Summary:
   Returns the BERR_Code corresponding to the given BVSB_ApStatus.
Description:
   This function returns ths BERR_Code of any errors in the given BVSB_ApStatus.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_DecodeError(
   BVSB_Handle h,           /* [in] BVSB PI Handle */
   BVSB_ApStatus *pApStatus /* [in] AP status returned by BVSB_GetApStatus */
);


/******************************************************************************
Summary:
   This function sends an HAB command.
Description:
   This function writes a command to the HAB, asserts HABR, waits for the AP to 
   release the HAB, then optionally checks for a service acknowlegement from 
   the AP.  If bCheckForAck is true, then read_len must be greater than 0.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_SendHabCommand(
   BVSB_Handle h,      /* [in] BVSB PI Handle */
   uint8_t *write_buf, /* [in] specifies the HAB command to send */
   uint8_t write_len,  /* [in] number of bytes in the HAB command */ 
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB */
   bool bCheckForAck   /* [in] true = determine if the AP has serviced the command */
);


/******************************************************************************
Summary:
   This function sends the command already written in the HAB.
Description:
   This function asserts HABR, waits for the AP to release the HAB, then 
   optionally checks for a service acknowlegement from the AP.  If bCheckForAck
   is true, then read_len must be greater than 0.  This function is called by
   BVSB_3520_P_SendHabCommand().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_ServiceHab(
   BVSB_Handle h,   /* [in] BVSB PI Handle */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
   uint8_t read_len,   /* [in] number of bytes to read from the HAB */
   bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
   uint8_t ack_byte    /* [in] value of the ack byte to expect */
);


/******************************************************************************
Summary:
   This function checks if the host is currently permitted to send an HAB 
   command.
Description:
   The HAB is available to the host if all 3 conditions currently apply: 
   1) The AP is running.
   2) The AP has initialized.
   3) The AP is not currently servicing the HAB (i.e. HABR bit is 0)
Returns:
   BERR_Code - BERR_SUCCESS if HAB is available, otherwise BERR_Code is reason
   why HAB is not available
******************************************************************************/
BERR_Code BVSB_3520_P_CheckHab(
   BVSB_Handle h  /* [in] BVSB PI Handle */
);


/******************************************************************************
Summary:
   Enables/Disables the L1 host interrupt.
Description:
   Enables/Disables the L1 host interrupt by calling the application-supplied 
   callback routine the the BVSB settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_EnableHostInterrupt(
   BVSB_Handle h, /* [in] BVSB PI handle */
   bool bEnable   /* [in] true=enables the L1 interrupt on the host processor */
);


/******************************************************************************
Summary:
   Enables the AP initialization done interrupt.
Description:
   This function is called by the BVSB_InitAp().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_EnableInitDoneInterrupt(
   BVSB_Handle h  /* [in] BVSB PI Handle */
);


/******************************************************************************
Summary:
   Enables/Disables the inband channel lock interrupt.
Description:
   This function enables or disables an interrupt to be generated upon change 
   in inband channel lock status.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_EnableLockInterrupt(
   BVSB_Handle h, /* [in] BVSB PI Handle */
   bool bEnable   /* [in] true = enable lock interrupts, false = disables lock interrupts */
);

/******************************************************************************
Summary:
   Enables/Disables the out-of-band channel lock interrupt.
Description:
   This function enables or disables an interrupt to be generated upon change 
   in out-of-band channel lock status.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_EnableOobLockInterrupt(
   BVSB_Handle h, /* [in] BVSB PI Handle */
   bool bEnable   /* [in] true = enable lock interrupts, false = disables lock interrupts */
);


/******************************************************************************
Summary:
   Enables/Disables the BSC interrupt.
Description:
   This function enables or disables the BSC transaction done interrupt.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_EnableBscInterrupt(
   BVSB_Handle h, /* [in] BVSB PI Handle */
   bool bEnable      /* [in] true = enable the BSC interrupt */
);


/******************************************************************************
Summary:
   Sets certain BKNI_Events based on the source of the BCM3520 host interrupt.
Description:
   This function reads and clears the BCM3520 interrupt status registers, 
   clears the enable mask of the interrupts, and sets BKNI_Events based on the 
   interrupt sources.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_DecodeInterrupt(
   BVSB_Handle h /* [in] BVSB PI Handle */
);


/******************************************************************************
Summary:
   Initiates VSB acquisition on the BCM3520.
Description:
   This function sends the Acquire_VSB HAB command to the AP.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireVsb(
   BVSB_Handle h,                   /* [in] BVSB PI Handle */
   const BVSB_InbandParams *pParams /* [in] inband acquisition parameters */
);


/******************************************************************************
Summary:
   Initiates QAM acquisition on the BCM3520.
Description:
   This function sends the Acquire_QAM HAB command to the AP.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireQam(
   BVSB_Handle h,                   /* [in] BVSB PI Handle */
   const BVSB_InbandParams *pParams /* [in] inband acquisition parameters */
);


/******************************************************************************
Summary:
   Initiates NTSC acquisition on the BCM3520.
Description:
   This function sends the Acquire_NTSC HAB command to the AP.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_AcquireNtsc(
   BVSB_Handle h,                   /* [in] BVSB PI Handle */
   const BVSB_InbandParams *pParams /* [in] inband acquisition parameters */
);


/******************************************************************************
Summary:
   Waits for a BVSB event. 
Description:
   This function waits for an interrupt within the given timeout period.  Any
   interrupts are then decoded via I2C.  If the given event has been signaled,
   then this function returns BERR_SUCCESS.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BVSB_3520_P_WaitForEvent(
   BVSB_Handle h,             /* [in] BVSB PI Handle */
   BKNI_EventHandle hEvent,   /* [in] event to wait on */
   int timeoutMsec            /* [in] timeout in milliseconds */
);

/* BCM3520 implementation of API functions */
BERR_Code BVSB_3520_P_Open(BVSB_Handle *, BCHP_Handle, void*, BINT_Handle, const struct BVSB_Settings*);
BERR_Code BVSB_3520_P_Close(BVSB_Handle);
BERR_Code BVSB_3520_P_InitAp(BVSB_Handle, const uint8_t *);
BERR_Code BVSB_3520_P_GetApStatus(BVSB_Handle, BVSB_ApStatus*);
BERR_Code BVSB_3520_P_GetApVersion(BVSB_Handle, uint16_t*, uint16_t*, uint8_t*, uint8_t*, uint8_t*);
BERR_Code BVSB_3520_P_ReadRegister(BVSB_Handle, uint32_t, uint32_t*);
BERR_Code BVSB_3520_P_WriteRegister(BVSB_Handle, uint32_t, uint32_t*);
BERR_Code BVSB_3520_P_Mi2cWrite(BVSB_Handle, uint8_t, uint8_t*, uint8_t);
BERR_Code BVSB_3520_P_Mi2cRead(BVSB_Handle, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t);
BERR_Code BVSB_3520_P_AcquireInband(BVSB_Handle, const BVSB_InbandParams *);
BERR_Code BVSB_3520_P_GetVsbStatus(BVSB_Handle, BVSB_VsbStatus *);
BERR_Code BVSB_3520_P_GetQamStatus(BVSB_Handle, BVSB_QamStatus *);
BERR_Code BVSB_3520_P_GetNtscStatus(BVSB_Handle, BVSB_NtscStatus *);
BERR_Code BVSB_3520_P_ResetInbandStatus(BVSB_Handle);
BERR_Code BVSB_3520_P_AcquireOob(BVSB_Handle, const BVSB_OobParams *);
BERR_Code BVSB_3520_P_GetOobStatus(BVSB_Handle, BVSB_OobStatus *);
BERR_Code BVSB_3520_P_ResetOobStatus(BVSB_Handle);
BERR_Code BVSB_3520_P_ConfigBtsc(BVSB_Handle, BVSB_BtscSettings *);
BERR_Code BVSB_3520_P_SetBtscVolume(BVSB_Handle, uint8_t, uint8_t, bool);
BERR_Code BVSB_3520_P_GetBtscStatus(BVSB_Handle, BVSB_BtscStatus *);
BERR_Code BVSB_3520_P_SetInbandOi(BVSB_Handle, BVSB_InbandOiSettings *);
BERR_Code BVSB_3520_P_GetSoftDecisionBuf(BVSB_Handle, int16_t*, int16_t*);
BERR_Code BVSB_3520_P_GetOobSoftDecisionBuf(BVSB_Handle, int16_t*, int16_t*);
BERR_Code BVSB_3520_P_SetTmConfig(BVSB_Handle, void*);
BERR_Code BVSB_3520_P_GetTmConfig(BVSB_Handle, void*);
BERR_Code BVSB_3520_P_WriteConfig(BVSB_Handle, uint16_t, uint8_t*, uint8_t);
BERR_Code BVSB_3520_P_ReadConfig(BVSB_Handle, uint16_t, uint8_t*, uint8_t);
BERR_Code BVSB_3520_P_GetLockStateChangeEventHandle(BVSB_Handle, BKNI_EventHandle*); 
BERR_Code BVSB_3520_P_GetOobLockStateChangeEventHandle(BVSB_Handle, BKNI_EventHandle*);
BERR_Code BVSB_3520_P_GetAntennaEventHandle(BVSB_Handle, BKNI_EventHandle*);
BERR_Code BVSB_3520_P_GetInterruptEventHandle(BVSB_Handle, BKNI_EventHandle*);
BERR_Code BVSB_3520_P_HandleInterrupt_isr(BVSB_Handle);
BERR_Code BVSB_3520_P_ProcessInterruptEvent(BVSB_Handle);
BERR_Code BVSB_3520_P_GetChipRevision(BVSB_Handle, uint8_t*);
BERR_Code BVSB_3520_P_SetInbandIfFreq(BVSB_Handle, uint32_t);
BERR_Code BVSB_3520_P_SetOobInterfaceControl(BVSB_Handle, BVSB_OobClockSuppression, BVSB_OobClock);

#ifdef __cplusplus
}
#endif

#endif /* BVSB_3520_P_PRIV_H__ */

