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
The IRB PI module controls the IR Blaster core within the BCM7038.  The IR 
blaster within the BCM7038 supports up to 160 pulse seuqences.


Design
The design for BIRB PI API is broken into three parts.
  Part 1 (open/close/configuration):
    These APIs are used for opening and closing BIRB device.
    
  Part 2 (sequence configuration):
    These APIs are used to configure IR blast sequence.

  Part 3 (send IR blast):
    These APIs are used to send the IR blast.

Usage
The usage of BIRB involves the following:
   * Configure/Open of BIRB
      * Configure BIRB device for the target system
      * Open BIRB device
   * Configure blast sequence
      * Configure blast sequence registers for target device.
   * Send blast
      * Start the IR blast sequence.

Sample Code
void main( void )
{
    BIRB_Handle       hIrb;
    BIRB_Settings     defSettings;
    BREG_Handle       hReg;
    BCHP_Handle       hChip;
	BINT_Handle		  hInterrupt;
	BKNI_EventHandle  hEvent;
	uint32_t          data;

    // Do other initialization, i.e. for BREG, BCHP, etc.

    BIRB_Open (&hIrb, hChip, hReg, hInterrupt, (BIRB_Settings *)NULL);
	BIRB_GetEventHandle (hIrb, &hEvent);

    // Blast to a Sony TV
    BIRB_Config (hIrb, BIRB_Device_eSony);

    // Sony POWER code is 0x95, 12 bits
	data = 0x95;
    BIRB_Send (hIrb, &data, 12);

	// Wait to IR blast to finish
	BKNI_WaitForEvent(hEvent, BKNI_INFINITE);
}

</verbatim>
***************************************************************************/


#ifndef BIRB_H__
#define BIRB_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Error Codes specific to BIRB

Description:

See Also:

****************************************************************************/
#define BIRB_ERR_TOO_MANY_SEQ			BERR_MAKE_CODE(BERR_IRB_ID, 0)

/***************************************************************************
Summary:
	The handle for irb module.

Description:

See Also:
	BIRB_Open()

****************************************************************************/
typedef struct BIRB_P_Handle				*BIRB_Handle;

/***************************************************************************
Summary:
	Enumeration for supported devices
									
Description:
	This enumeration lists the device supported by this IRB PI.
	New device types can be added as needed..

See Also:
	None.

****************************************************************************/
typedef enum
{
   BIRB_Device_eSony,
   BIRB_Device_eGI,
   BIRB_Device_ePioneer,
   BIRB_Device_ePioneerAAAA,
   BIRB_Device_eCustom,
   BIRB_Device_eXmp2,
   BIRB_Device_eRC6
} BIRB_Device;

/***************************************************************************
Summary:
	IR blaster configuration

Description:
	This structure type contains IR blast information for a particular IR
	protocol.

See Also:

****************************************************************************/
#define MASTER_DIVISOR 			(27)	/* masterDivisor (1..256) */
#define CARRIER_CLOCK_DIVISOR 	(1)		/* carrierClockDivisor (1..16) */
#define INDEX_CLOCK_DIVISOR 	(10)	/* indexClockDivisor (1..16) */
#define INDEX_PERIOD 			(10)	/* index clock period in us */
#define CARRIER_PERIOD 			(1)		/* carrier clock period in us */

#define FRAME_PERIOD 			(45000)	/* frame period in us */
#define FRAME_PERIOD_64_BIT 	(145000)/* frame period in us */

#define FRAME_PERIOD_PIONEER_ABBB_1	(108026)	/* frame period in us */
#define FRAME_PERIOD_PIONEER_ABBB_2	(108026)	/* frame period in us */
#define FRAME_PERIOD_PIONEER_ABBB_3	(46420)		/* frame period in us */
#define FRAME_PERIOD_PIONEER_ABBB_4	(59080)		/* frame period in us */

#define CASE2_DURATION_INDEX		8

#define FRAME_PERIOD_PIONEER_AAAA_4	(115000)	/* frame period in us */

#define irb_sequenceIndexMemLength 	161
#define irb_durationMemLength  		14

/* defins for XMP-2 configuration */
#define BLAST_INTRENABLE_ALTMODCNT (0x20)
#define DELTA1        1
#define DATA0         3
#define DATA2         4
#define DATA4         5
#define DATA6         6
#define DATA8         7
#define DATA10        8
#define DATA12        9
#define DATA14        10
#define DATA_PULSE    11
#define TRAILER_PULSE 12
#define ZERO          13
typedef struct irb_IndexPair 
{
	uint16_t on;		/* index of on duration */
	uint16_t off;		/* index of off duration */
} irb_IndexPair;

typedef struct irb_Configuration 
{
	char		*name;							/* name of configuration */
	uint8_t		masterClockDivisor;				/* pre-scaler for the primary IRB clock */
	uint8_t		carrierClockDivisor;			/* pre-scaler for carrier counter clock */
	uint8_t		indexClockDivisor;				/* pre-scaler for modulation counter clock */
	uint8_t		noCarrier;						/* 0=output w/ carrier, 1=output w/out carrier */
	uint8_t		carrierHighCount;				/* high time for the carrier frequency */
	uint8_t		carrierLowCount;				/* low time for the carrier frequency */
	uint8_t		numberSequences;				/* number of modulation on/off sequences */
	irb_IndexPair logic0IndexPair;				/* pointer to mod_reg_file with logic0 */
	irb_IndexPair logic1IndexPair;				/* pointer to mod_reg_file with logic1 */
	irb_IndexPair sequenceIndex[irb_sequenceIndexMemLength];	/* blast_seq_regfile */
	uint32_t	duration[irb_durationMemLength];				/* blast_mod_regfile */
	uint32_t	framePeriod;					/* IR packet frame period A */
	uint32_t	framePeriodB;					/* IR packet frame period B */
	uint16_t	lastSequenceOffIndex;			/* index to the last sequence off duration */
	uint8_t		repeatCount;					/* IR blaster repeat number */
	uint8_t		repeatStartIndex;				/* IR blaster repeat index */
	int 		altModCnt;
} SIrbConfiguration;

/***************************************************************************
Summary:
	Required default settings structure for IRB module.

Description:
	The default setting structure defines the default configure of
	IRB when the device is opened.  

See Also:
	BIRB_Open()

****************************************************************************/
typedef struct BIRB_Settings
{
	BIRB_Device		irbDev;				/* IR Blaster device type */
	bool			intMode;			/* interrupt enable flag */
} BIRB_Settings;


/***************************************************************************
Summary:
	This function opens IRB module.

Description:
	This function is responsible for opening BIRB module. When BIRB is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BIRB_Close()
	BIRB_GetDefaultSettings()

****************************************************************************/
BERR_Code BIRB_Open(
	BIRB_Handle *pIRB,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle */
	const BIRB_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes IRB module.

Description:
	This function is responsible for closing BIRB module. Closing BIRB 
	will free main BIRB handle. 

Returns:
	TODO:

See Also:
	BIRB_Open()

****************************************************************************/
BERR_Code BIRB_Close(
	BIRB_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the default settings for IRB module.

Description:
	This function is responsible for returns the default setting for 
	BIRB module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BIRB_Open()

****************************************************************************/
BERR_Code BIRB_GetDefaultSettings(
	BIRB_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

/***************************************************************************
Summary:
	This function gets the event handle for IRB module channel.

Description:
	This function is responsible for getting the event handle. The
	application code should use this function to get the IRB event handle, 
	and pend on the event.  The IRB ISR will set the event.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BIRB_GetEventHandle(
	BIRB_Handle		hDev,				/* [in] Device handle */
	BKNI_EventHandle *phEvent			/* [out] Returns event handle */
	);

/***************************************************************************
Summary:
	This function sets the custom IRB parameters

Description:
	This function sets the custom-type IRB parameters to what the user
	requests.  This allows the user to change IRB parameters on the fly.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
void BIRB_ConfigCustom (
	BIRB_Handle 		hIrb, 			/* [in] Device handle */
	SIrbConfiguration 	*pConfig		/* [in] Pointer to custom config */
);

/***************************************************************************
Summary:
	This function configures the IR blaster sequence type.

Description:
	This function is used to configure the IR blaster prior to blasting.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_Config ( 
	BIRB_Handle 		hIrb, 			/* [in] Device handle */
	BIRB_Device 		irbDev			/* [in] IR device type */
	);

/***************************************************************************
Summary:
	This function resets the IR blaster

Description:
	This function is used to reset the IR blaster or to abort a blast sequence.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
void BIRB_Reset(
	BIRB_Handle 	hDev				/* [in] Device handle */
);

/***************************************************************************
Summary:
	This function starts the blast without reconfiguring sequence.

Description:
	This function is used to re-send the previously configured sequence
	without having to reconfigure it again.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_Blast ( 
	BIRB_Handle 		hIrb			/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function checks to see if IR blast is finished

Description:
	This function is used to poll the status of IR blaster.  If a blast is
	in progress, it returns false.  If no blast is in progress, it returns true.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
bool BIRB_IsIrbFinished(
	BIRB_Handle 	hDev				/* [in] Device handle */
);

/***************************************************************************
Summary:
	This function sends the IR blaster sequence.

Description:
	This function is used to send an IR blast sequence.
	This function always sends the header pulse.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_Send ( 
	BIRB_Handle 		hDev,			/* [in] Device handle */
	uint32_t			*pData,			/* [in] pointer to data to blast */
	uint8_t				bits			/* [in] number of bits to blast */
	);

/***************************************************************************
Summary:
	This function sends the IR blaster sequence.

Description:
	This function is used to send an IR blast sequence with option to disable
	header pulse
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_SendWithHeaderOption ( 
	BIRB_Handle 		hDev,			/* [in] Device handle */
	uint32_t			*pData,			/* [in] pointer to data to blast */
	uint8_t				bits,			/* [in] number of bits to blast */
	bool				headerPulse		/* [in] header flag */
	);

/***************************************************************************
Summary:
	This function sends the IR blaster sequence for RC6.

Description:
	Only one bit is used for the parameter trailer.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_SendRC6 (
    BIRB_Handle         hDev,           /* Device handle */
    uint32_t            *pData,         /* pointer to data to blast */
    uint8_t             bits,           /* number of bits to blast */
    uint8_t             mode,           /* 3 bits */
    uint8_t             trailer         /* 0 or 1 */
);

/***************************************************************************
Summary:
	This function sends multiple IR blaster packets.

Description:
	This function is used to multiple blaster packets.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_SendABBB ( 
	BIRB_Handle 		hDev,			/* [in] Device handle */
	uint32_t			*pDataA,		/* [in] pointer to data A to blast */
	uint8_t				bitsA,			/* [in] number of bits in A to blast */
	uint32_t			*pDataB,		/* [in] pointer to data B to blast */
	uint8_t				bitsB, 			/* [in] number of bits in B to blast */
	bool				headerA, 		/* [in] flag to send header A pulse */
	bool				headerB, 		/* [in] flag to send header B pulse */
	bool				fixedFlag 		/* [in] flag to indicate fixed frame length */
);

/***************************************************************************
Summary:
	This function sends multiple IR blaster packets.

Description:
	This function is used to multiple blaster packets.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_SendAAAA ( 
	BIRB_Handle 		hDev,			/* [in] Device handle */
	uint32_t			*pDataA,		/* [in] pointer to data A to blast */
	uint8_t				bitsA,			/* [in] number of bits in A to blast */
	bool				headerPulse, 	/* [in] flag to send header pulse */
	bool				fixedFlag		/* [in] flag to indicate fixed frame length */
);


/***************************************************************************
Summary:
	This function configures IR blaster registers

Description:
	This function is used to write a pre-configured IR blast sequence to 
	hardware.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
void BIRB_ConfigRegisters(
	BIRB_Handle 	hDev				/* [in] Device handle */
);

/***************************************************************************
Summary:
	This function sends XMP2 ACK packet.

Description:
	This function is used to send XMP2 ACK packet.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_SendXmp2Ack ( 
    BIRB_Handle         hDev           /* Device handle */
);

/***************************************************************************
Summary:
	This function sends XMP2 NACK packet.

Description:
	This function is used to send XMP2 NACK packet.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_SendXmp2Nack ( 
    BIRB_Handle         hDev           /* Device handle */
);

/***************************************************************************
Summary:
	This function sends XMP2 packet.

Description:
	This function is used to send XMP2 packet.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BIRB_SendXmp2Bytes ( 
    BIRB_Handle         hDev,           /* Device handle */
    uint8_t             *pByte,         /* date to send */
    uint8_t             numByte         /* 1 or 4 bye */
);

#ifdef __cplusplus
}
#endif
 
#endif



