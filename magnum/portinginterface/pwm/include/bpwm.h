/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


/*= Module Overview *********************************************************
<verbatim>

Overview
The PWM PI module controls the PWM core.
There are two independent PWM generators, each with programmable
control word (duty cycle).

Design
The design for BPWM PI API is broken into two parts.
  Part 1 (open/close/configuration):
	These APIs are used for opening and closing BPWM device/device channel.
  Part 2 (setting control word):
	These APIs are used to program the control word.
  Part 3 (start/stop PWM generation):
	These APIs are used to start and stop PWM signal generation.

Usage
The usage of BPWM involves the following:
   * Configure/Open of BPWM
	  * Configure BPWM device for the target system
	  * Open BPWM device
	  * Configure BPWM device channel for the target system
	  * Open BPWM device channel
   * Set the Frequency mode
	  * Program the desired frequency mode : Variable or constant mode
   * Set control word
	  * Program the desired duty cycle through control word
   * Set the On and Period interval in case of constant frequency mode
	  * Program the desired duty cycle with the on and period intervals
   * Controlling PWM generation
	  * Starting and stopping PWM signal generation.

sample code
void main( void )
{
	BPWM_Handle       hPwm;
	BPWM_ChannelHandle   hPwmChan;
	BREG_Handle       hReg;
	BCHP_Handle       hChip;
	BPWM_ChannelSettings defChnSettings;
	unsigned int ChanNo;

	// Do other initialization, i.e. for BREG, BCHP, etc.

	BPWM_Open( &hPwm, hChip, hReg, (BPWM_Settings *)NULL );

	ChanNo = 0; // example for channel 0
	BPWM_GetChannelDefaultSettings( hPwm, ChanNo, &defChnSettings );

	// Make any changes required from the default values
	defChnSettings.openDrainb = false;
	defChnSettings.BPWM_FreqModeType= Variable_Freq_Mode;

	BPWM_OpenChannel( hPwm, &hPwmChan, ChanNo, &defChnSettings );

	// in case you want to change the frequncy mode to contant mode to be able to control the duty cycle
	BPWM_SetFreqMode (hPwmChan, Constant_Freq_Mode);

	// 50% duty cycle
	BPWM_SetControlWord (hPwmChan, 0x8000);

   // set the On interval to 0x40 and the period interval to 0x80, that will keep the duty cycle at 50
   // you can change the duty cycle by chaaging the ratio of On and period intervals in case of contant frequency mode
   // in case of  variable frequency mode the On and period interval values will not have any impact.
   BPWM_SetOnAndPeriodInterval (hPwmChan, 0x40, 0x80);

	// Now start pwm generation
	BPWM_Start (hPwmChan);

	// Stop pwm generation
	BPWM_Stop (hPwmChan);

}
</verbatim>
***************************************************************************/


#ifndef BPWM_H__
#define BPWM_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bchp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Number of PWM channels supported
****************************************************************************/
/* For the following parts, the RDB says PWMB exists but it really doesn't or is minimally connected */
#if ((BCHP_CHIP==7346) || (BCHP_CHIP==7231) || (BCHP_CHIP==7358) || (BCHP_CHIP==73465))
    #define MAX_PWM_CHANNELS 2
#elif defined(BCHP_PWME_REG_START)
    #define MAX_PWM_CHANNELS 10
#elif defined(BCHP_PWMD_REG_START)
    #define MAX_PWM_CHANNELS 8
#elif defined(BCHP_PWMC_REG_START)
    #define MAX_PWM_CHANNELS 6
#elif defined(BCHP_PWMB_REG_START)
    #define MAX_PWM_CHANNELS 4
#else
    #define MAX_PWM_CHANNELS 2
#endif

/***************************************************************************
Summary:
	Error Codes specific to BPWM
****************************************************************************/
#define BPWM_ERR_NOTAVAIL_CHN_NO		BERR_MAKE_CODE(BERR_PWM_ID, 0)

/***************************************************************************
Summary:
	The handles for pwm module.

Description:
	Since BPWM is a device channel, it has main device handle as well
	as a device channel handle.

See Also:
	BPWM_Open(), BPWM_OpenChannel()

****************************************************************************/
typedef struct BPWM_P_Handle				*BPWM_Handle;
typedef struct BPWM_P_ChannelHandle			*BPWM_ChannelHandle;

/***************************************************************************
Summary:
	Required default settings structure for PWM module.

Description:
	The default setting structure defines the default configure of
	PWM when the device is opened.  Since BPWM is a device
	channel, it also has default settings for a device channel.
	Currently there are no parameters for device setting.

See Also:
	BPWM_Open(), BPWM_OpenChannel()

****************************************************************************/
typedef struct BPWM_Settings
{
	BINT_Handle hInterrupt; /* only required if using BPWM_RampOnInterval */
} BPWM_Settings;

typedef enum BPWM_FreqModeType
{
	Variable_Freq_Mode = 0,
	Constant_Freq_Mode = 1
} BPWM_FreqModeType;

/***************************************************************************
Summary:
	Enumeration for PWM open drain configuration
****************************************************************************/
typedef enum BPWM_OpenDrainOutput
{
	PwmOutOpenDrain = 0,
	PwmOutTotemPole
} BPWM_OpenDrainOutput;

typedef struct BPWM_ChannelSettings
{
	bool openDrainb;					/* open drain enable */
	BPWM_FreqModeType  FreqMode;                /* Variable Freq mode or Constant Freq mode */

	unsigned maxChangesPerInterrupt; /* maximum number of register writes per interrupt when using BPWM_RampOnInterval */
	BINT_Id interruptId; /* interrupt ID which BPWM_RampOnInterval will use to ramp */
	BINT_Id secondaryInterruptId; /* additional interrupt ID which BPWM_RampOnInterval will use to ramp */
} BPWM_ChannelSettings;

/***************************************************************************
Summary:
	This function opens PWM module.

Description:
	This function is responsible for opening BPWM module. When BPWM is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

See Also:
	BPWM_Close(), BPWM_OpenChannel(), BPWM_CloseChannel(),
	BPWM_GetDefaultSettings()

****************************************************************************/
BERR_Code BPWM_Open(
	BPWM_Handle *pPWM,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	const BPWM_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes PWM module.

Description:
	This function is responsible for closing BPWM module. Closing BPWM
	will free main BPWM handle. It is required that all opened
	BPWM channels must be closed before calling this function. If this
	is not done, the results will be unpredicable.

See Also:
	BPWM_Open(), BPWM_CloseChannel()

****************************************************************************/
BERR_Code BPWM_Close(
	BPWM_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the default settings for PWM module.

Description:
	This function is responsible for returns the default setting for
	BPWM module. The returning default setting should be when
	opening the device.

See Also:
	BPWM_Open()

****************************************************************************/
BERR_Code BPWM_GetDefaultSettings(
	BPWM_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

/***************************************************************************
Summary:
	This function returns the total number of channels supported by
	PWM module.

Description:
	This function is responsible for getting total number of channels
	supported by BPWM module, since BPWM device is implemented as a
	device channel.

See Also:
	BPWM_OpenChannel(), BPWM_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BPWM_GetTotalChannels(
	BPWM_Handle hDev,					/* [in] Device handle */
	unsigned int *totalChannels			/* [out] Returns total number downstream channels supported */
	);

/***************************************************************************
Summary:
	This function gets default setting for a PWM module channel.

Description:
	This function is responsible for returning the default setting for
	channel of BPWM. The return default setting is used when opening
	a channel.

See Also:
	BPWM_OpenChannel()

****************************************************************************/
BERR_Code BPWM_GetChannelDefaultSettings(
	BPWM_Handle hDev,					/* [in] Device handle */
	unsigned int channelNo,				/* [in] Channel number to default setting for */
	BPWM_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
	);

/***************************************************************************
Summary:
	This function opens PWM module channel.

Description:
	This function is responsible for opening BPWM module channel. When a
	BPWM channel is	opened, it will create a module channel handle and
	configure the module based on the channel default settings. Once a
	channel is opened, it must be closed before it can be opened again.

See Also:
	BPWM_CloseChannel(), BPWM_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BPWM_OpenChannel(
	BPWM_Handle hDev,					/* [in] Device handle */
	BPWM_ChannelHandle *phChn,			/* [out] Returns channel handle */
	unsigned int channelNo,				/* [in] Channel number to open */
	const BPWM_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
	);

/***************************************************************************
Summary:
	This function closes PWM module channel.

Description:
	This function is responsible for closing BPWM module channel. Closing
	BPWM channel it will free BPWM channel handle. It is required that all
	opened BPWM channels must be closed before closing BPWM.

See Also:
	BPWM_OpenChannel(), BPWM_CloseChannel()

****************************************************************************/
BERR_Code BPWM_CloseChannel(
	BPWM_ChannelHandle hChn				/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function gets PWM module device handle based on
	the device channel handle.

Description:
	This function is responsible returning BPWM module handle based on the
	BPWM module channel.
****************************************************************************/
BERR_Code BPWM_GetDevice(
	BPWM_ChannelHandle hChn,			/* [in] Device channel handle */
	BPWM_Handle *pPWM					/* [out] Returns Device handle */
	);

/***************************************************************************
Summary:
	This function sets the control word
****************************************************************************/
BERR_Code BPWM_SetControlWord(
	BPWM_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint16_t	 		cWord			/* [in] control word to program */
	);

/***************************************************************************
Summary:
	This function gets the control word
****************************************************************************/
BERR_Code BPWM_GetControlWord(
	BPWM_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint16_t	 		*cWord			/* [out] control word to program */
	);


/***************************************************************************
Summary:
	This function sets the Freq Model

Description:
	This function is used to program the desired Freq Mode before
	PWM generation.
***************************************************************************/
BERR_Code BPWM_SetFreqMode(
	BPWM_ChannelHandle	hChn,		/* Device channel handle */
	BPWM_FreqModeType	 Frequeny_Mode		/* Frequency mode  to program */
	);


/***************************************************************************
Summary:
	This function gets the Freq Mode
***************************************************************************/
BERR_Code BPWM_GetFreqMode(
	BPWM_ChannelHandle	hChn,			       /* Device channel handle */
	BPWM_FreqModeType	 		*Frequeny_Mode		/* Frequency mode  read */
	);

/***************************************************************************
Summary:
	This function sets the On Interval

Description:
	This function is used to program the desired On Interval before
	PWM generation.
****************************************************************************/
BERR_Code BPWM_SetOnInterval(
	BPWM_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint16_t	 		OnInterval			/* [out] control word to program */
	);

/***************************************************************************
Summary:
	This function sets the On Interval by making incremental changes (ie. a ramp function) at interrupt time

Description:
	This function is used to program the desired On Interval before
	PWM generation.

	You must set BPWM_Settings.hInterrupt and BPWM_ChannelSettings.interruptId to use this function.
	Set BPWM_ChannelSettings.maxChangesPerInterrupt to control the number of changes per interrupt.
****************************************************************************/
BERR_Code BPWM_RampOnInterval(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t OnInterval
	);

/***************************************************************************
Summary:
	isr variation of BPWM_RampOnInterval.
****************************************************************************/
BERR_Code BPWM_RampOnInterval_isr(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t OnInterval
	);

/***************************************************************************
Summary:
	This function gets the On Interval
****************************************************************************/
BERR_Code BPWM_GetOnInterval(
	BPWM_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint16_t	 		*OnInterval			/* [out] On interval to read */
	);

/***************************************************************************
Summary:
	This function sets the Period Interval

Description:
	This function is used to program the desired Period Interval before
	PWM generation.
****************************************************************************/
BERR_Code BPWM_SetPeriodInterval(
	BPWM_ChannelHandle	hChn,			/* Device channel handle */
	uint16_t	 		PeriodInterval			/* Period Interval  to program */
	);

/***************************************************************************
Summary:
	This function gets the Period Interval
****************************************************************************/
BERR_Code BPWM_GetPeriodInterval(
	BPWM_ChannelHandle	hChn,			/* [in] Device channel handle */
	uint16_t	 		*PeriodInterval		/* [out] Period interval  to read */
	);

/***************************************************************************
Summary:
	This function sets the On and Period Interval
****************************************************************************/
BERR_Code BPWM_SetOnAndPeriodInterval(
	BPWM_ChannelHandle	hChn,			        /* Device channel handle */
	uint16_t	 		     OnInterval,		        /* OnInterval  to program */
	uint16_t	 		     PeriodInterval		 /* Period Interval  to program */
	);

/***************************************************************************
Summary:
	This function gets the On and Period Interval
****************************************************************************/
BERR_Code BPWM_GetOnAndPeriodInterval(
	BPWM_ChannelHandle	hChn,			/* Device channel handle */
	uint16_t	 		*OnInterval,			/* OnInterval  to program */
	uint16_t	 		*PeriodInterval		/* Period Interval  to program */
	);

/***************************************************************************
Summary:
	This function starts PWM generation
****************************************************************************/
BERR_Code BPWM_Start(
	BPWM_ChannelHandle hChn			/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function stops PWM generation
****************************************************************************/
BERR_Code BPWM_Stop(
	BPWM_ChannelHandle hChn			/* [in] Device channel handle */
	);


/***************************************************************************
Summary:
	This function selects open drain vs. totem pole output
****************************************************************************/
BERR_Code BPWM_SetOpenDrainConfig(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_OpenDrainOutput	type			/* [in] open drain/totem pole select */
	);

/***************************************************************************
Summary:
	This function gets the open drain configuration
****************************************************************************/
BERR_Code BPWM_GetOpenDrainConfig(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_OpenDrainOutput	*type			/* [out] open drain/totem pole select */
	);


#if ((BCHP_CHIP==35230) || (BCHP_CHIP==35125) || (BCHP_CHIP==35233))
/***************************************************************************
Summary:
	Enumeration for PWM output polarity
****************************************************************************/
typedef enum BPWM_OutPolaritySel
{
	OutPolarityHigh = 0,
	OutPolarityLow = 1
} BPWM_OutPolaritySel;

/***************************************************************************
Summary:
	Enumeration for PWM Sync Edge Select
****************************************************************************/
typedef enum BPWM_SyncEdgeSel
{
	SyncEdgeNeg = 0,
	SyncEdgePos = 1
} BPWM_SyncEdgeSel;

/***************************************************************************
Summary:
	Enumeration for PWM double buffer mode control
****************************************************************************/
typedef enum BPWM_DblBufMode
{
	DblBufMode_Disabled = 0,
	DblBufMode_Reserved = 1,
	DblBufMode_Period   = 2,
	DblBufMode_Sync     = 3
} BPWM_DblBufMode;

/***************************************************************************
Summary:
	Enumeration for PWM two-input sync
****************************************************************************/
typedef enum BPWM_TwoInputSyncType
{
	TwoInputSync_Vsync = 0,
	TwoInputSync_Vsync2
} BPWM_TwoInputSyncType;

/***************************************************************************
Summary:
	This function sets the control word in register set 1

Description:
	This function is used to program the desired control word in register
	set 1 for two-input sync.
****************************************************************************/
BERR_Code BPWM_SetControlWordSet1
(
	BPWM_ChannelHandle  hChn,           /* [in] Device channel handle */
	uint16_t            cWord           /* [in] control word to program for register set 1 */
);

/***************************************************************************
Summary:
	This function gets the control word in register set 1

Description:
	This function is used to get the value of the control word in 
	register set 1 for two-input sync.
****************************************************************************/
BERR_Code BPWM_GetControlWordSet1
(
	BPWM_ChannelHandle  hChn,           /* [in] Device channel handle */
	uint16_t            *cWord          /* [out] control word to program for register set 1 */
);

/***************************************************************************
Summary:
	This function sets the On and Period values in register set 1

Description:
	This function is used to program the on interval and cycle period in 
	register set 1 for two-input sync.
****************************************************************************/
BERR_Code BPWM_SetOnAndPeriodIntervalSet1
(
	BPWM_ChannelHandle  hChn,           	/* [in] Device channel handle */
	uint16_t            OnInterval,         /* [in] OnInterval  to program */
	uint16_t            PeriodInterval      /* [in] Period Interval  to program */
);

/***************************************************************************
Summary:
	This function gets the On and Period values in register set 1

Description:
	This function is used to get the values of on interval and cycle period in 
	register set 1 for two-input sync.
****************************************************************************/
BERR_Code BPWM_GetOnAndPeriodIntervalSet1
(
	BPWM_ChannelHandle  hChn,           	/* [in] Device channel handle */
	uint16_t            *OnInterval,        /* [out] OnInterval  to program */
	uint16_t            *PeriodInterval     /* [out] Period Interval  to program */
);

/***************************************************************************
Summary:
	This function enables or disables sync mode
****************************************************************************/
BERR_Code BPWM_EnableSyncMode
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	bool				enable				/* [in] sync enable */
);

/***************************************************************************
Summary:
	This function sets double buffer mode
****************************************************************************/
BERR_Code BPWM_SetDblBufMode
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_DblBufMode		dblBufMode			/* [in] double buffer mode type */
);

/***************************************************************************
Summary:
	This function gets double buffer mode
****************************************************************************/
BERR_Code BPWM_GetDblBufMode
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_DblBufMode		*dblBufMode			/* [out] double buffer mode type */
);

/***************************************************************************
Summary:
	This function sets the sync edge type
****************************************************************************/
BERR_Code BPWM_SetSyncEdge
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_SyncEdgeSel	SyncEdgeSel			/* [in] edge type */
);

/***************************************************************************
Summary:
	This function gets the sync edge type
****************************************************************************/
BERR_Code BPWM_GetSyncEdge
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_SyncEdgeSel	*SyncEdgeSel		/* [out] edge type */
);

/***************************************************************************
Summary:
	This function sets the two-input sync type
****************************************************************************/
BERR_Code BPWM_SetTwoInputSync
(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_TwoInputSyncType	type			/* [in] two-input sync type */
);

/***************************************************************************
Summary:
	This function gets the two-input sync type
****************************************************************************/
BERR_Code BPWM_GetTwoInputSync
(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_TwoInputSyncType	*type			/* [out] two-input sync type */
);

/***************************************************************************
Summary:
	This function sets the output polarity
****************************************************************************/
BERR_Code BPWM_SetOutputPolarity
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_OutPolaritySel	OutPolSel			/* [in] output polarity */
);

/***************************************************************************
Summary:
	This function gets the output polarity
****************************************************************************/
BERR_Code BPWM_GetOutputPolarity
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_OutPolaritySel	*OutPolSel			/* [out] output polarity */
);

#endif

#ifdef __cplusplus
}
#endif

#endif
