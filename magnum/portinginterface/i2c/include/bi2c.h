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
    BINT_Handle         hInt;

    // Do other initialization, i.e. for BREG, BCHP, etc

    BI2C_Open( &hI2c, hChip, hReg, hInt, (BI2C_Settings *)NULL );

    chanNo = 0; // example for channel 0
    BI2C_GetChannelDefaultSettings( hI2c, chanNo, &defChnSettings );

    // Make any changes required from the default values
    defChnSettings.clkRate      = BI2C_Clk_eClk400Khz;

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

#include "bi2c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Error Codes specific to BI2C

Description:

See Also:39178

****************************************************************************/
#define BI2C_ERR_NOTAVAIL_CHN_NO            BERR_MAKE_CODE(BERR_I2C_ID, 0)
#define BI2C_ERR_NO_ACK                     BERR_MAKE_CODE(BERR_I2C_ID, 1)
#define BI2C_ERR_SINGLE_MSTR_CREATE         BERR_MAKE_CODE(BERR_I2C_ID, 2)

/* To open a soft I2C channel, along with correct address and shift values of the gpio pins used for clock and data,
    BI2C_SOFT_I2C_CHANNEL_NUMBER should be passed to BI2C_OpenChannel().
    This is to verify that the user is opening a soft I2C channel rather than a hardware I2C hannel.
*/
#define BI2C_SOFT_I2C_CHANNEL_NUMBER 0x8642

/***************************************************************************
Summary:
    The handles for i2c module.

Description:
    Since BI2C is a device channel, it has main device handle as well
    as a device channel handle.

See Also:
    BI2C_Open(), BI2C_OpenChannel()

****************************************************************************/
typedef struct BI2C_P_Handle                *BI2C_Handle;
typedef struct BI2C_P_ChannelHandle         *BI2C_ChannelHandle;

/***************************************************************************
Summary:
    Enumeration for i2c clock rate

Description:
    This enumeration defines the clock rate for the I2C master

See Also:
    None.

****************************************************************************/
typedef unsigned int BI2C_Clk;
#define BI2C_Clk_eClk47Khz  47000
#define BI2C_Clk_eClk50Khz  50000
#define BI2C_Clk_eClk93Khz  93000
#define BI2C_Clk_eClk100Khz 100000
#define BI2C_Clk_eClk187Khz 187000
#define BI2C_Clk_eClk200Khz 200000
#define BI2C_Clk_eClk375Khz 375000
#define BI2C_Clk_eClk400Khz 400000

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
    BI2C_eSdaDelay370ns = 370,
    BI2C_eSdaDelay482ns = 482,
    BI2C_eSdaDelay593ns = 593,
    BI2C_eSdaDelay704ns = 704,
    BI2C_eSdaDelay815ns = 815,
    BI2C_eSdaDelay926ns = 926,
    BI2C_eSdaDelay1037ns = 1037
}  BI2C_SdaDelay;

/***************************************************************************
Summary:
    Enumeration for i2c timeout field

Description:
    Deprecated

See Also:
    None.

****************************************************************************/
typedef enum
{
   BI2C_TimeoutBasedOnClkSpeed  = 0
}  BI2C_Timeout;

typedef struct BI2C_gpio_params {
    unsigned address;
    unsigned shift;
} BI2C_gpio_params;

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
    BI2C_Clk            clkRate;          /* I2C clock speed */
    bool                intMode;          /* use interrupt flag */
    unsigned int        timeoutMs;        /* Timeout value in milliseconds.
                                                                              Set to zero if this I2C channel is not expected to fail during normal operation. A very large timeout will be used instead.
                                                                              Note that a proper timeout is usually not based on the time for I2C transaction to complete,
                                                                              but on the worst case latency of the OS for servicing interrupts. */
    bool                burstMode;        /* enable/disable burst mode read */
    struct {
        bool            enabled;           /* If false,  BSC/soft I2c will be used. If true, auto i2c will be used. */
    }autoI2c;
    bool                softI2c;          /* i2c gpio mode */
    struct {
        BI2C_gpio_params scl;
        BI2C_gpio_params sda;
    }gpio;
    bool                fourByteXferMode; /* use four byte transfer mode */
    bool                inputSwitching5V; /* use 5V for switching value */
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
    BI2C_Handle *pI2C,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    const BI2C_Settings *pDefSettings   /* [in] Default settings */
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
    BI2C_Handle hDev                    /* [in] Device handle */
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
    BI2C_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
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
    BI2C_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
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
BERR_Code BI2C_GetBscIndexDefaultSettings(
    BI2C_Handle hDev,                     /* Device handle */
    unsigned int bscIndex,                /* Channel number to default setting for */
    bool isSoftI2c,                       /* If true, get the defaults for soft i2c channel. */
    BI2C_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
    );

BERR_Code BI2C_GetChannelDefaultSettings(
    BI2C_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BI2C_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

BERR_Code BI2C_OpenBSCChannel(
    BI2C_Handle hDev,                   /* [in] Device handle */
    BI2C_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
    const BI2C_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function opens I2C module channel.

Description:
    This function is responsible for opening BI2C module channel. When a
    BI2C channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

    To open a softI2c channel, set softI2c to true in BI2C_ChanneSettings.
    Also, set gpio.scl and gpio.sda in BI2C_ChanneSettings as follows:

    This example shows the use case for 7346 B2 chip.
    Here, Registers with names ending in "_LO" control pins GPIO[31:0].
    Registers with names ending in "_HI" control pins GPIO[63:32].
    Registers with names ending in "_EXT" control pins GPIO[71:64], AON SGPIO[1:0], and SGPIO[5:0]
    with register bits [5:0] corresponding to pins SGPIO[5:0], register bits [7:6] corresponding to
    pins AON SGPIO[1:0], and register bits [15:8] corresponding to pins GPIO[71:64].

    Hence, set address as

    gpio.scl.address = BCHP_GIO_ODEN_LO or
    gpio.scl.address = BCHP_GIO_ODEN_HI or
    gpio.scl.address = BCHP_GIO_ODEN_EXT

    depending of which gpio is being used for clock. Similarly set gpio.sda.address.

    Also, set shift as

    gpio.scl.shift = 0, for sgpio_00, or
    gpio.scl.shift = 7, for aon_sgpio_01, or
    gpio.scl.shift = 9, for gpio_65.

    NOTE: The corresponding pinmuxes should be set to gpio in  SUN_TOP_CTRL or AON_PIN_CTRL register block.

Returns:
    TODO:

See Also:
    BI2C_CloseChannel(), BI2C_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BI2C_OpenChannel(
    BI2C_Handle hDev,                   /* [in] Device handle */
    BI2C_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
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
    BI2C_ChannelHandle hChn             /* [in] Device channel handle */
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
    BI2C_ChannelHandle hChn,            /* [in] Device channel handle */
    BI2C_Handle *pI2C                   /* [out] Returns Device handle */
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
    BI2C_ChannelHandle hChn,            /* [in] Device channel handle */
    BREG_I2C_Handle *pI2cRegHandle      /* [out] register handle */
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
    BREG_I2C_Handle     hI2cReg             /* [in] I2C register handle */
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
    BI2C_ChannelHandle  hChn,           /* [in] Device channel handle */
    BI2C_Clk            clk             /* [in] clock rate setting */
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
    BI2C_ChannelHandle  hChn            /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function sets the SDA delay for an I2C channel

Description:
    This function allows the user to change the SDS delay.

Returns:
    TODO:

See Also:

****************************************************************************/
void BI2C_SetSdaDelay(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    BI2C_SdaDelay       sdaDelay        /* Sda delay value */
    );

/***************************************************************************
Summary:
    This function enable/disable I2C burst mode operations.

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_SetBurstMode(
    BI2C_ChannelHandle hChn,    /* [in] I2C channel handle */
    bool bBurstMode             /* [out] Enable or Disable burst mode */
    );

/***************************************************************************
Summary:
    This function will verify if I2C burst mode has been enabled or
    disabled on an I2C channel.

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_GetBurstMode(
    BI2C_ChannelHandle hChn,    /* [in] I2C channel handle */
    bool *pbBurstMode           /* [out] current burst mode? */
    );

/***************************************************************************
Summary:
    This function will set the DATA_REG_SIZE field.

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
void BI2C_Set4ByteXfrMode(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    bool b4ByteMode                     /* Enable or Disable 4 byte xfr mode */
    );

/***************************************************************************
Summary:
    This function will check if the DATA_REG_SIZE field is set.

Description:

Returns:
    TODO:

See Also:

****************************************************************************/
bool BI2C_Is4ByteXfrMode(
    BI2C_ChannelHandle  hChn           /* Device channel handle */
    );

/***************************************************************************
Summary:
    This function will perform a software reset.

Description:

After an interruption in protocol, power loss or system reset, any 2-wire
part can be protocol reset by following these steps: (a) Create a start bit
condition, (b) clock 9 cycles, (c) create another start bit followed by stop
bit condition. The device is ready for next communication after above steps
have been completed.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BI2C_SwReset(
    BI2C_ChannelHandle  hChn           /* [in] Device channel handle */
    );

#ifdef __cplusplus
}
#endif

#endif



