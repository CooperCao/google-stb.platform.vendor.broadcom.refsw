/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bsyslib.h"
#include "bsyslib_callback.h"

#ifndef BASTMLIB_H__
#define BASTMLIB_H__

/***************************************************************************
Module Overview:

***************************************************************************/

/*
Summary:
A handle to an ASTM library instance.  An instance can handle N elementary 
streams and will compute one clock coupling mode and one tsm mode based on the 
information gathered for those streams.
*/
typedef struct BASTMlib_Impl * BASTMlib_Handle;

/*
Summary:
A presenter handle
Description:
A presenter represents any module that normally performs TSM to determine
when to release content to a display / output.
*/
typedef struct BASTMlib_Presenter_Impl * BASTMlib_Presenter_Handle;

/*
Summary:
What controls the rate of releasing frames for presentation?
Description:
This enum represents a selection between TSM mode and VSYNC / non-TSM mode.
Basically, we are deciding if frames will be released for presentation based
on their timestamp, or based on the rate of the output clock.
*/
typedef enum BASTMlib_PresentationRateControl
{
	BASTMlib_PresentationRateControl_eTimeStamp, /* frames are released based 
		on their time stamp */
	BASTMlib_PresentationRateControl_eOutputClock, /* frames are released based
		on output clock rate (timestamps are ignored) */
	BASTMlib_PresentationRateControl_eMax /* enum terminator */
} BASTMlib_PresentationRateControl;

/*
Summary:
What is the source of the STC?
*/
/* TODO: still poorly named */
typedef enum BASTMlib_StcSource
{
	BASTMlib_StcSource_eClockReference, /* PCR, SCR, RTS, etc. */
	BASTMlib_StcSource_ePresenter, /* a presenter */
	BASTMlib_StcSource_eMax /* enum terminator */
} BASTMlib_StcSource;

/*
Summary:
Clock rate used for calculating thresholds and normalizing event information
between events with different rates
*/
typedef enum BASTMlib_ClockRate
{
	BASTMlib_ClockRate_e45Khz = 45000,
	BASTMlib_ClockRate_e27Mhz = 27000000,
	BASTMlib_ClockRate_eMax = 0x7fffffff /* enum terminator */
} BASTMlib_ClockRate;

/*
Summary:
What clock is used to drive the timebases in the system?
*/
typedef enum BASTMlib_ClockCoupling
{
	BASTMlib_ClockCoupling_eInputClock, /* PCR, VSYNC, HSYNC, etc. */
	BASTMlib_ClockCoupling_eInternalClock, /* internal XTAL */
	BASTMlib_ClockCoupling_eMax /* enum terminator */
} BASTMlib_ClockCoupling;

/*
Summary:
Encapsulates all create-time, unchangeable settings for ASTM
*/
typedef struct BASTMlib_Settings
{
	BSYSlib_TimerCallback cbTimer;
	struct
	{
		BSYSlib_Callback cbStateChange; /* when the presentation state changes,
			if a callback is registered here, notification will occur */
	} sPresentation;
	struct
	{
		BSYSlib_Callback cbStateChange;/* when the clock coupling state changes,
			if a callback is registered here, notification will occur */
	} sClockCoupling;
} BASTMlib_Settings;

/*
Summary:
Presenter create-time settings
*/
typedef struct BASTMlib_Presenter_Settings
{
	const char * pcName;
} BASTMlib_Presenter_Settings;

/*
Summary:
Presenter configuration
*/
typedef struct BASTMlib_Presenter_Config
{
	BASTMlib_ClockRate ePtsDomain; /* the clocking domain of timestamps for
		this presenter */
	/* TODO: can we make it so older events are the ones that are lost? 
	what happens when we have already started processing events, we prob. 
	don't want to touch older ones as we would introduce discos */
	unsigned int uiMinimumTimeBetweenEvents; /* minimum time that must elapse 
		between receiving events, in ms. used with presentation processing 
		frequency to determine size of event ring buffer. if events come more 
		often than this time, the event buffer may overflow, and newer events 
		will be lost */
	unsigned int uiPassEventCountThreshold; /* how many pass events must be 
		received before deeming this presenter as "passing", in events */
	unsigned int uiFailEventCountThreshold; /* how many fail events must be 
		received before deeming this presenter as "failing", in events */
} BASTMlib_Presenter_Config;

/*
Summary:
 */
typedef void (*BASTMlib_ClockCouplingDataPresenceDetector)(void * pvContext, bool * pbDataPresent);

/*
Summary:
Configuration for ASTMlib.
Description:
These are configuration parameters that can be changed at any time.
*/
typedef struct
{
	bool bEnabled; /* if this flag is set to false, ASTM does not take place */
	BASTMlib_ClockRate eStcRate; /* the rate of the STC managed by this 
		instance of ASTMlib */
	struct
	{
		BASTMlib_StcSource ePreferredStcSource; /* this is used as the highest 
			level stc source goal of ASTM's presentation task */
		BASTMlib_Presenter_Handle hPreferredPresenter; /* if this handle is set
			and we determine the STC source should be a presenter, this is the
			presenter that will be used as the STC master.  If not set, the
			master presenter will be chosen in a first-come, first served 
			(FCFS) fashion. */
		unsigned int uiInitialAcquisitionTime; /* time just after calling start
			before processing presenter event queues for the first time, 
			in ms */
		unsigned int uiProcessingFrequency; /* after initial acquisition 
			period,	how often presenter event queues will be processed, 
			in ms */
		unsigned int uiTsmDisabledWatchdogTimeout; /* after entering 
			TSM-disabled mode, how long before we attempt to re-enter 
			TSM-enabled mode unconditionally */
		unsigned int uiSettlingTime; /* after changing presentation	modes, how 
			long we will wait for presenters to settle in the new mode */
		BASTMlib_PresentationRateControl ePreferredPresentationRateControl; /*
			This is used as the highest level presentation rate control goal of 
			ASTM's presentation task */
	} sPresentation; /* presentation config */
	struct
	{
		/* TODO: see comment above on loss of old vs. new events */
		unsigned int uiMinimumTimeBetweenEvents; /* minimum time that must 
			elapse between receiving events, in ms. used with processing 
			frequency to determine size of event ring buffer. if events come 
			more often than this time, the event buffer may overflow, and newer
			events will be lost */
		BASTMlib_ClockRate eClockReferenceDomain; /* the clocking domain of 
			the clock reference time stamps */
		unsigned int uiDeviationThreshold; /* threshold above which abs value of 
			clock reference rate difference to STC will be considered deviant */
		unsigned int uiDeviantCountThreshold; /* how many deviant clock
			coupling measurements must be received before deeming the clock
			reference bad and decoupling the timebases from the input */
		unsigned int uiIdealCountThreshold;/* how many ideal clock
			coupling measurements must be received before deeming the clock
			reference good and recoupling the timebases to the input */
		unsigned int uiInitialAcquisitionTime; /* time just after calling start
			before processing clock reference event queue for the first time, 
			in ms */
		unsigned int uiProcessingFrequency; /* after initial acquisition 
			period,	how often clock reference event queue will be processed, 
			in ms */
		unsigned int uiIdealProcessingFrequency; /* after detecting deviant
			clock reference events, how often clock reference event queue will 
			be processed, in ms */
		unsigned int uiSettlingTime; /* after changing clock coupling modes, 
			how long we will wait for clock reference to settle in the new 
			mode */
		BASTMlib_ClockCoupling ePreferredClockCoupling; /* this is used as the
			highest level goal of ASTM's clock coupling task */
		BASTMlib_ClockCouplingDataPresenceDetector pfDetectDataPresent; /* used to
		    detect if data is present for the clock recovery monitor */
		void * pvDataPresentContext; /* context for the data present detector */
	} sClockCoupling; /* clock coupling config */
} BASTMlib_Config;

/*
Summary:
Status information for a given instance of the library
*/
typedef struct
{
	BASTMlib_PresentationRateControl ePresentationRateControl;
	BASTMlib_StcSource eStcSource;
	BASTMlib_Presenter_Handle hStcMaster; /* if the STC source is PTS, from 
		which presenter are the PTSs derived? NULL if STC source is not PTS. */
	BASTMlib_ClockCoupling eClockCoupling;
} BASTMlib_Status;

/*
Summary:
The information required by the ASTM algorithm from the system during a 
presenter event
*/
typedef struct
{
	bool bPass; /* presenter's notion of TSM pass/fail for a given event */
	unsigned int uiPts; /* in clock ticks in presenter_config.ePtsDomain */
	unsigned int uiStc; /* in clock ticks at astm_config.eStcRate rate */
} BASTMlib_Presenter_Event;

/*
Summary:
The information required by the ASTM algorithm from the system during a 
clock coupling event
*/
typedef struct
{
	unsigned int uiClockReference; /* in clock ticks 
		in astm_config.sClockCoupling.eClockReferenceDomain domain */
	unsigned int uiStc; /* in clock ticks at astm_config.eStcRate rate */
} BASTMlib_ClockReference_Event;

/*
Summary:
Returns the default ASTM lib module settings
*/
void BASTMlib_GetDefaultSettings(
	BASTMlib_Settings * psSettings /* [out] */
);

/*
Summary:
Creates an ASTM lib instance
*/
BERR_Code BASTMlib_Create(
	BASTMlib_Handle * phAstm, /* [out] */
	const BASTMlib_Settings * psSettings
);

/*
Summary:
Destroys an instance of the ASTM library
*/
void BASTMlib_Destroy(
	BASTMlib_Handle hAstm
);

/*
Summary:
Returns the default configuration of an ASTM instance 
*/
void BASTMlib_GetDefaultConfig(
	BASTMlib_Config * psConfig
);

/*
Summary:
Returns the current configuration of an ASTM instance 
*/
void BASTMlib_GetConfig(
	const BASTMlib_Handle hAstm, 
	BASTMlib_Config * psConfig
);

/*
Summary:
Sets the current configuration of an ASTM instance 
*/
void BASTMlib_SetConfig(
	BASTMlib_Handle hAstm,
	const BASTMlib_Config * psConfig
);

/*
Summary:
Starts an ASTM instance
*/
BERR_Code BASTMlib_Start(
	BASTMlib_Handle hAstm
);

/*
Summary:
Stops an ASTM instance
*/
BERR_Code BASTMlib_Stop(
	BASTMlib_Handle hAstm
);

/*
Summary:
Returns the current status of an ASTM instance.
*/
void BASTMlib_GetStatus(
	BASTMlib_Handle hAstm,
	BASTMlib_Status * psStatus /* [out] */
);

/*
Summary:
Adds a presenter to the ASTM instance
*/
BERR_Code BASTMlib_AddPresenter(
	BASTMlib_Handle hAstm,
	BASTMlib_Presenter_Handle hPresenter
);

/*
Summary:
Removes a presenter from the ASTM instance
*/
void BASTMlib_RemovePresenter(
	BASTMlib_Handle hAstm,
	BASTMlib_Presenter_Handle hPresenter
);

/*
Summary:
Updates the current state of the ASTM module with information about a 
clock reference event
*/
void BASTMlib_ClockReferenceEventHandler_isr(
	BASTMlib_Handle hAstm,
	const BASTMlib_ClockReference_Event * psEvent
);

/*
Summary:
Returns the default ASTM presenter settings
*/
void BASTMlib_Presenter_GetDefaultSettings(
	BASTMlib_Presenter_Settings * psSettings /* [out] */
);

/*
Summary:
Creates an ASTM presenter
*/
void BASTMlib_Presenter_Create(	
	BASTMlib_Handle hAstm,
	BASTMlib_Presenter_Handle * phPresenter, /* [out] */
	const BASTMlib_Presenter_Settings * psSettings
);

/*
Summary:
Destroys an ASTM presenter
*/
void BASTMlib_Presenter_Destroy(
	BASTMlib_Presenter_Handle hPresenter
);

/*
Summary:
Returns the current ASTM presenter settings
*/
void BASTMlib_Presenter_GetSettings(
	const BASTMlib_Presenter_Handle hPresenter,
	BASTMlib_Presenter_Settings * psSettings /* [out] */
);

/*
Summary:
Sets the ASTM presenter settings
*/
void BASTMlib_Presenter_SetSettings(
	BASTMlib_Presenter_Handle hPresenter,
	const BASTMlib_Presenter_Settings * psSettings
);

/*
Summary:
Gets the default config for the specified presenter.
*/
void BASTMlib_Presenter_GetDefaultConfig(
	BASTMlib_Presenter_Config * psConfig /* [out] */
);
/*
Summary:
Gets the current config for the specified presenter.  If no config has been
set since creation, returns the default.
*/
void BASTMlib_Presenter_GetConfig(
	const BASTMlib_Presenter_Handle hPresenter,
	BASTMlib_Presenter_Config * psConfig /* [out] */
);

/*
Summary:
Sets the current config for the specified presenter
*/
void BASTMlib_Presenter_SetConfig(
	BASTMlib_Presenter_Handle hPresenter,
	const BASTMlib_Presenter_Config * psConfig
);

/*
Summary:
Updates the current state of the ASTM module with information about a 
presenter event
*/
void BASTMlib_Presenter_EventHandler_isr(
	BASTMlib_Presenter_Handle hPresenter,
	const BASTMlib_Presenter_Event * psEvent
);

#endif /* BASTMLIB_H__ */

