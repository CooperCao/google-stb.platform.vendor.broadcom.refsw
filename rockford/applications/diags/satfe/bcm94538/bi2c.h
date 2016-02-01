/******************************************************************************* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

/*= Module Overview *********************************************************
<verbatim>

Overview
The I2C PI module controls the 4 I2C master controllers within the BCM7038.
The I2C master controllers within the BCM7038 are independent of each other.
Each controller is capable of the following clock rates: 47KHz, 50KHz, 93KHz,
100KHz, 187KHz, 200KHz, 375KHz, and 400KHz.  The I2C controllers now support
unlimited data transfer sizes through the ability to suppress START and STOP
conditions.

Design
The design for BI2C PI API is broken into two parts.
  Part 1 (open/close/configuration):
    These APIs are used for opening and closing BI2C device/device channel.
    When a device/device channel is opened, the device/device channel can be
    configured for transfer speed.
  Part 2 (command):
    These APIs are sending read and write commands using the I2C master controller.

Usage
The usage of BI2C involves the following:

   * Configure/Open of BI2C
      * Configure BI2C device for the target system
      * Open BI2C device
      * Configure BI2C device channel for the target system
      * Open BI2C device channel
	  * Create BI2C register handle

   * Sending commands
      * Send read/write commands using BI2C register handle.

Sample Code
void main( void )
{
    BI2C_Handle       hI2c;
    BI2C_ChannelHandle   hI2cChan;
    BI2C_ChannelSettings defChnSettings;
    BREG_Handle       hReg;
    BCHP_Handle       hChip;
    int chanNo;
    unsigned char      data[10];
    BREG_I2C_Handle      hI2cReg;
	BINT_Handle			hInt;

    // Do other initialization, i.e. for BREG, BCHP, etc

    BI2C_Open( &hI2c, hChip, hReg, hInt, (BI2C_Settings *)NULL );

    chanNo = 0; // example for channel 0
    BI2C_GetChannelDefaultSettings( hI2c, chanNo, &defChnSettings );

    // Make any changes required from the default values
    defChnSettings.clkRate		= BI2C_Clk_eClk400Khz;

    BI2C_OpenChannel( hI2c, &hI2cChan, chanNo, &defChnSettings );

    // Get handle to I2C Reg
    BI2C_CreateI2cRegHandle (hI2cChan, &hI2cReg);

    //
    // Do a read of 2 bytes from register 0x1b of I2C device whose
    // chip ID is 0x9C.
    //
    BREG_I2C_Read (hI2cReg, 0x9c, 0x1b, &data, 2);

    //
    // Do a write of 3 bytes from register 0x1000 of I2C device whose
    // chip ID is 0x8E.
    //
    // Fill in data to send
    data[0]               = 0xb2;
    data[1]               = 0x77;
    data[2]               = 0x20;
    BREG_I2C_WriteA16 (hI2cReg, 0x8e, 0x1000, &data, 3);
}

</verbatim>
***************************************************************************/


#ifndef BI2C_H__
#define BI2C_H__

#include "bchp.h"
#include "breg_mem.h"
#include "breg_i2c.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Number of I2C channels

Description:

See Also:

****************************************************************************/
#define BI2C_MAX_I2C_CHANNELS				1
#define BI2C_MAX_I2C_MSTR_CHANNELS			1

/***************************************************************************
Summary:
	Error Codes specific to BI2C

Description:

See Also:

****************************************************************************/
#define BI2C_ERR_NOTAVAIL_CHN_NO			BERR_MAKE_CODE(BERR_I2C_ID, 0)
#define BI2C_ERR_NO_ACK						BERR_MAKE_CODE(BERR_I2C_ID, 1)
#define BI2C_ERR_SINGLE_MSTR_CREATE			BERR_MAKE_CODE(BERR_I2C_ID, 2)



/***************************************************************************
Summary:
	The handles for i2c module.

Description:
	Since BI2C is a device channel, it has main device handle as well
	as a device channel handle.

See Also:
	BI2C_Open(), BI2C_OpenChannel()

****************************************************************************/
typedef struct BI2C_P_Handle				*BI2C_Handle;
typedef struct BI2C_P_ChannelHandle			*BI2C_ChannelHandle;

/***************************************************************************
Summary:
	Enumeration for i2c clock rate

Description:
	This enumeration defines the clock rate for the I2C master

See Also:
	None.

****************************************************************************/
typedef enum
{
   BI2C_Clk_eClk47Khz  = 47,
   BI2C_Clk_eClk50Khz  = 50,
   BI2C_Clk_eClk93Khz  = 93,
   BI2C_Clk_eClk100Khz = 100,
   BI2C_Clk_eClk187Khz = 187,
   BI2C_Clk_eClk200Khz = 200,
   BI2C_Clk_eClk375Khz = 375,
   BI2C_Clk_eClk400Khz = 400
}  BI2C_Clk;

/***************************************************************************
Summary:
	Required default settings structure for I2C module.

Description:
	The default setting structure defines the default configure of
	I2C when the device is opened.  Since BI2C is a device
	channel, it also has default settings for a device channel.
	Currently there are no parameters for device setting.

See Also:
	BI2C_Open(), BI2C_OpenChannel()

****************************************************************************/
typedef void *BI2C_Settings;

typedef struct BI2C_ChannelSettings
{
	BI2C_Clk            clkRate;		/* I2C clock speed */
	bool				intMode;		/* use interrupt flag */
} BI2C_ChannelSettings;

/***************************************************************************
Summary:
	This function opens I2C module.

Description:
	This function is responsible for opening BI2C module. When BI2C is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BI2C_Close(), BI2C_OpenChannel(), BI2C_CloseChannel(),
	BI2C_GetDefaultSettings()

****************************************************************************/
BERR_Code BI2C_Open(
	BI2C_Handle *pI2C,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle */
	const BI2C_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes I2C module.

Description:
	This function is responsible for closing BI2C module. Closing BI2C
	will free main BI2C handle. It is required that all opened
	BI2C channels must be closed before calling this function. If this
	is not done, the results will be unpredicable.

Returns:
	TODO:

See Also:
	BI2C_Open(), BI2C_CloseChannel()

****************************************************************************/
BERR_Code BI2C_Close(
	BI2C_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the default settings for I2C module.

Description:
	This function is responsible for returns the default setting for
	BI2C module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BI2C_Open()

****************************************************************************/
BERR_Code BI2C_GetDefaultSettings(
	BI2C_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

/***************************************************************************
Summary:
	This function returns the total number of channels supported by
	I2C module.

Description:
	This function is responsible for getting total number of channels
	supported by BI2C module, since BI2C device is implemented as a
	device channel.

Returns:
	TODO:

See Also:
	BI2C_OpenChannel(), BI2C_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BI2C_GetTotalChannels(
	BI2C_Handle hDev,					/* [in] Device handle */
	unsigned int *totalChannels			/* [out] Returns total number downstream channels supported */
	);

/***************************************************************************
Summary:
	This function gets default setting for a I2C module channel.

Description:
	This function is responsible for returning the default setting for
	channel of BI2C. The return default setting is used when opening
	a channel.

Returns:
	TODO:

See Also:
	BI2C_OpenChannel()

****************************************************************************/
BERR_Code BI2C_GetChannelDefaultSettings(
	BI2C_Handle hDev,					/* [in] Device handle */
	unsigned int channelNo,				/* [in] Channel number to default setting for */
    BI2C_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
	This function opens I2C module channel.

Description:
	This function is responsible for opening BI2C module channel. When a
	BI2C channel is	opened, it will create a module channel handle and
	configure the module based on the channel default settings. Once a
	channel is opened, it must be closed before it can be opened again.

Returns:
	TODO:

See Also:
	BI2C_CloseChannel(), BI2C_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BI2C_OpenChannel(
	BI2C_Handle hDev,					/* [in] Device handle */
	BI2C_ChannelHandle *phChn,			/* [out] Returns channel handle */
	unsigned int channelNo,				/* [in] Channel number to open */
	const BI2C_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
	);

/***************************************************************************
Summary:
	This function closes I2C module channel.

Description:
	This function is responsible for closing BI2C module channel. Closing
	BI2C channel it will free BI2C channel handle. It is required that all
	opened BI2C channels must be closed before closing BI2C.

Returns:
	TODO:

See Also:
	BI2C_OpenChannel(), BI2C_CloseChannel()

****************************************************************************/
BERR_Code BI2C_CloseChannel(
	BI2C_ChannelHandle hChn				/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function gets I2C module device handle based on
	the device channel handle.

Description:
	This function is responsible returning BI2C module handle based on the
	BI2C module channel.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_GetDevice(
	BI2C_ChannelHandle hChn,			/* [in] Device channel handle */
	BI2C_Handle *pI2C					/* [out] Returns Device handle */
	);


/***************************************************************************
Summary:
	This function creates an I2C register handle.

Description:
	This function is responsible for creating an I2C register handle for
	master I2C controller.  It fills in the BREG_I2C_Handle structure with
	pointers to the I2C PI functions.  The application can then use this
	I2C register interface to perform read and write operations.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_CreateI2cRegHandle(
	BI2C_ChannelHandle hChn,			/* [in] Device channel handle */
	BREG_I2C_Handle *pI2cRegHandle		/* [out] register handle */
	);

/***************************************************************************
Summary:
	This function closes I2C register handle

Description:
	This function is responsible for closing the I2C register handle.
	This function frees BI2C register handle.

Returns:
	TODO:

See Also:
	BI2C_CreateI2cRegHandle()

****************************************************************************/
BERR_Code BI2C_CloseI2cRegHandle(
	BREG_I2C_Handle		hI2cReg				/* [in] I2C register handle */
	);

/***************************************************************************
Summary:
	This function sets the clock rate for an I2C channel

Description:
	This function allows the user to change the I2C clock rate.

Returns:
	TODO:

See Also:

****************************************************************************/
void BI2C_SetClk(
	BI2C_ChannelHandle 	hChn,			/* [in] Device channel handle */
	BI2C_Clk			clk				/* [in] clock rate setting */
	);

/***************************************************************************
Summary:
	This function gets the clock rate for an I2C channel

Description:
	This function returns the current clock rate setting for an I2C channel

Returns:
	TODO:

See Also:

****************************************************************************/
BI2C_Clk BI2C_GetClk(
	BI2C_ChannelHandle 	hChn			/* [in] Device channel handle */
	);

BERR_Code BI2C_GetFd(BI2C_ChannelHandle hChn, int *pFd);

#ifdef __cplusplus
}
#endif

#endif
