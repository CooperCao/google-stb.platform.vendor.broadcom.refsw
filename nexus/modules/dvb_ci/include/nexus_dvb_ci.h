/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
***************************************************************************/

#ifndef NEXUS_DVB_CI_H__
#define NEXUS_DVB_CI_H__

#include "nexus_types.h"
#include "nexus_gpio.h"
#include "nexus_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_DvbCi *NEXUS_DvbCiHandle;

/***************************************************************************
Summary:
    Transport interface to the DVB-CI module
 
Description: 
    Select which transport interface will be used on the chip to communicate
    data with the CAM.  If only one interface is available on a particular
    chip, this setting is ignored.
***************************************************************************/
typedef enum NEXUS_DvbCiTransportMode
{
    NEXUS_DvbCiTransportMode_eMpod,     /* Route TS using the MPOD block if available (TS signals are MPOD or CHIP2CI/CI2CHIP) */
    NEXUS_DvbCiTransportMode_ePpkt,     /* Route TS using PPKT interface (TS signals are PPKT_I PPKT_O) */
    NEXUS_DvbCiTransportMode_eMax
} NEXUS_DvbCiTransportMode;

/***************************************************************************
Summary:
    Settings to open a DVB-CI device
***************************************************************************/
typedef struct NEXUS_DvbCiOpenSettings
{
    uint32_t deviceOffset;              /* The physical base address for card accesses.  This offset must have been 
                                           registered via a call to NEXUS_P_AddMap() or open will fail. */
    unsigned ebiChipSelect;             /* EBI Chip Select Number.  Typically this ranges from 0..5 */
    bool littleEndian;                  /* If true, the bits 0..7 will be used for data on the EBI bus.  If false, 8..15 will be used. */
    NEXUS_DvbCiTransportMode tsMode;    /* Transport stream mode.  Default is NEXUS_DvbCiTransportMode_eMpod. */

    NEXUS_GpioHandle readyPin;          /* GPIO for the IREQ#/READY pin - If NULL, value for resetInterrupt will be used instead */
    unsigned readyInterrupt;            /* EXT IRQ number for RESET interrupt.  Only checked if readyPin is NULL.  For example, pass 0 for ext_irqb0. */

    NEXUS_GpioHandle cd1Pin;            /* GPIO for the CD1# signal - If NULL, value for cd1Interrupt will be used instead */
    unsigned cd1Interrupt;              /* EXT IRQ number for CD1# interrupt.  Only checked if cd1Pin is NULL. For example, pass 0 for ext_irqb0. */
    NEXUS_GpioHandle cd2Pin;            /* GPIO for the CD2# signal - If NULL, value for cd2Interrupt will be checked */
    unsigned cd2Interrupt;              /* EXT IRQ number for CD2# interrupt.  Only checked if cd2Pin is NULL.  
                                           Can be omitted for systems with a single CD# pin. For example, pass 0 for ext_irqb0. */
    NEXUS_GpioHandle resetPin;          /* GPIO for the PCMCA RESET signal */

    NEXUS_GpioHandle vccEnablePin0;    /* Pin 0 of vcc control circuit - pass NULL if not used */
    NEXUS_GpioHandle vccEnablePin1;    /* Pin 1 of vcc contrl circuit - pass NULL if not used */
    uint8_t vcctristateValue;          /* value to set vcc line to hi-z (2 bits - ignored if only one vpp pin is provided) */
    uint8_t vcc3vValue;                /* value to set vcc line to 3.3v */
    uint8_t vcc5vValue;                /* Value to set vcc line to 5v */

    NEXUS_GpioHandle vpp0EnablePin0;    /* Pin 0 of vpp0 control circuit - pass NULL if vpp is always active */
    NEXUS_GpioHandle vpp0EnablePin1;    /* Pin 1 of vpp0 contrl circuit - pass NULL if only one bit is used */
    uint8_t vpp0tristateValue;          /* value to set vpp line to hi-z (2 bits - ignored if only one vpp pin is provided) */
    uint8_t vpp03vValue;                /* value to set vpp line to 3.3v */
    uint8_t vpp05vValue;                /* Value to set vpp line to 5v */

    NEXUS_GpioHandle vpp1EnablePin0;    /* Pin 0 of vpp1 control circuit - pass NULL if only one vpp enable is used */
    NEXUS_GpioHandle vpp1EnablePin1;    /* Pin 1 of vpp1 contrl circuit - pass NULL if only one bit is used */
    uint8_t vpp1tristateValue;          /* value to set vpp line to hi-z (2 bits - ignored if only one vpp pin is provided) */
    uint8_t vpp13vValue;                /* value to set vpp line to 3.3v */
    uint8_t vpp15vValue;                /* Value to set vpp line to 5v */

    NEXUS_GpioHandle ebiEnablePin;      /* Handle for EBI bus driver - pass NULL if not used */
    uint8_t ebiEnableValue;             /* Value to enable EBI bus driver for PCMCIA mode accesses */

    NEXUS_GpioHandle vs1Pin;            /* Handle for the VS1# signal - pass NULL for 5V only slots */
    NEXUS_GpioHandle vs2Pin;            /* Handle for the VS2# signal - pass NULL for 5V only slots */

    NEXUS_I2cHandle fpgaI2c;            /* I2C bus for DVB-CI CPLD/FPGA - pass NULL if not used */
    uint16_t fpgaI2cAddr;               /* I2C bus address for DVB-CI CPLD/FPGA */
    uint32_t deviceOffsetIO;            /* The physical base address for IO accesses. This offset must have been 
                                           registered via a call to NEXUS_P_AddMap() or open will fail. */
} NEXUS_DvbCiOpenSettings;

/***************************************************************************
Summary:
    Get default settings to open a DVB-CI device

Description:
    This function allows for future expansion of the structure
    
***************************************************************************/
void NEXUS_DvbCi_GetDefaultOpenSettings(
    NEXUS_DvbCiOpenSettings *pSettings      /* [out] */
    );

/***************************************************************************
Summary:
    Open a DVB-CI device
***************************************************************************/
NEXUS_DvbCiHandle NEXUS_DvbCi_Open( /* attr{destructor=NEXUS_DvbCi_Close} */
    const NEXUS_DvbCiOpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Close a DVB-CI device
***************************************************************************/
void NEXUS_DvbCi_Close(
    NEXUS_DvbCiHandle handle
    );

/***************************************************************************
Summary:
    Settings for a DVB-CI device
***************************************************************************/
typedef struct NEXUS_DvbCiSettings
{
    NEXUS_CallbackDesc cardDetectCallback;  /* This will be called on insertion after
                                               the card has been reset and the CIS has been read.
                                               On removal, this will be called immediately. */

    NEXUS_CallbackDesc errorCallback;       /* This will be called if an error occurs while
                                               communicating with the card.  The error
                                               status can be found with NEXUS_DvbCi_GetStatus */

    NEXUS_CallbackDesc interruptCallback;   /* This will be called whenever the DA or FR interrupts
                                               are raised.  DA/FR interrupts will be disabled 
                                               until the app calls NEXUS_DvbCi_EnableInterrupts with
                                               the daEnabled and/or frEnabled flags set to true. */
} NEXUS_DvbCiSettings;

/***************************************************************************
Summary:
    Get the settings for a DVB-CI device
***************************************************************************/
void NEXUS_DvbCi_GetSettings(
    NEXUS_DvbCiHandle handle,
    NEXUS_DvbCiSettings *pSettings      /* [out] */
    );

/***************************************************************************
Summary:
    Set the settings for a DVB-CI device
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_SetSettings(
    NEXUS_DvbCiHandle handle,
    const NEXUS_DvbCiSettings *pSettings
    );

/***************************************************************************
Summary:
    Enable/Disable DA and FR interrupts for the module
 
Description: 
    If either interrupt fires, the callback specified in
    NEXUS_DvbCiSettings.interruptCallback will be called.  Removing or
    resetting a module (PCMCIA or via the ChangePersonality routine)
    will cause the interrupts to be disabled. They must be re-enabled by the
    application if those events occur.  Also, the interrupts are edge-triggered
    and must be re-enabled any time an interrupt has fired.
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_EnableInterrupts(
    NEXUS_DvbCiHandle handle,
    bool daEnabled,
    bool frEnabled);

/***************************************************************************
Summary:
    Enumeration of errors reported from a DVB-CI module

Description:
    Some of the errors below may occur during operation with a DVB-CI
    module.  The application may decide to either reset the card or 
    report the error to the user as appropriate.  Once an error has been
    reported, no further communication with the card will be permitted.

See Also:
    NEXUS_DvbCiStatus
    NEXUS_DvbCiSettings
***************************************************************************/
typedef enum NEXUS_DvbCiError
{
    NEXUS_DvbCiError_eNone,                     /* No error has occurred */
    NEXUS_DvbCiError_ePcmciaResetTimeout,       /* The READY signal was not asserted after a reset */
    NEXUS_DvbCiError_eInvalidCis,               /* The CIS is invalid */
    NEXUS_DvbCiError_ePersonalityChangeTimeout, /* The READY signal was not asserted after a personality change */
    NEXUS_DvbCiError_eRsTimeout,                /* The module did not assert FR after an RS reset */
    NEXUS_DvbCiError_eBufferNegotiationFailed,  /* The buffer negotiation failed */
    NEXUS_DvbCiError_eWriteError,               /* The WE bit was asserted */
    NEXUS_DvbCiError_eReadError,                /* The RE bit was asserted */
    NEXUS_DvbCiError_eMax
} NEXUS_DvbCiError;

/***************************************************************************
Summary:
    Status of a DVB-CI device
***************************************************************************/
typedef struct NEXUS_DvbCiStatus
{
    bool cardPresent;   /* If true, a card is inserted in the slot */
    bool validCis;      /* If true, a valid CA Module has been detected */
    bool pcmciaMode;    /* If true, the card is in PCMCIA mode.  
                           If false, the personality change has put it into DVB mode */
    bool daAsserted;    /* If true, the DA bit is asserted (valid if pcmciaMode = false) */
    bool frAsserted;    /* If true, the FR bit is asserted (valid if pcmciaMode = false) */
    bool iirAsserted;    /* If true, the IIR bit is asserted (valid if pcmciaMode = false) */
    bool daInterruptEnabled;    /* If true, the DA interrupt is enabled */
    bool frInterruptEnabled;    /* If true, the FR interrupt is enabled */

    NEXUS_DvbCiError lastError;     /* The current error state of the card */

    uint16_t negotiatedBufferSize;  /* The negotiated buffer size between the host and card 
                                       (valid if pcmciaMode = false) */
} NEXUS_DvbCiStatus;

/***************************************************************************
Summary:
    Get the status of a DVB-CI device
***************************************************************************/
void NEXUS_DvbCi_GetStatus(
    NEXUS_DvbCiHandle handle,
    NEXUS_DvbCiStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
    Apply a PCMCIA reset to a DVB-CI device
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_Reset(
    NEXUS_DvbCiHandle handle
   );

/***************************************************************************
Summary:
    Execute the personality change into DVB mode

Description:
    This routine should be called after the card has been detected and
    the cardPresent and validCis flags of the NEXUS_DvbCiStatus 
    structure are both set to true.  If this routine returns successfully,
    NEXUS_DvbCi_NegotiateBuffers should be called.

See Also:
    NEXUS_DvbCi_NegotiateBuffers
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_ChangePersonality(
    NEXUS_DvbCiHandle handle
    );

/***************************************************************************
Summary:
    Negotiate the data channel buffer size

Description:
    This routine should be called after NEXUS_DvbCi_ChangePersonality.  
    It will negotiate the data channel buffer size with the module.  The
    negotiated size can be read from NEXUS_DvbCi_GetStatus.  The negotiated
    size wll be the minimum of the buffer size provided to this call and the
    buffer provided by the module.

See Also:
    NEXUS_DvbCi_ChangePersonality
    NEXUS_DvbCi_GetStatus
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_NegotiateBuffers(
    NEXUS_DvbCiHandle handle,
    uint16_t maxBufferSize,         /* Pass 0 for an unlimited buffer size */
    uint16_t *pNegotiatedSize       /* [out] */
    );

/***************************************************************************
Summary:
    Write a buffer of data to a DVB-CI module

Description:
    This routine will write a buffer of data to a DVB-CI module.  If the module
    has asserted the DA bit indicating data is available, this will return an
    error and the application should read data instead.  If the FR bit is not set,
    this routine will also return an error and the host should retry after the
    NEXUS_DvbCiStatus.frAsserted is set to true.  The buffer provided should be
    <= the negotiated buffer size.

See Also:
    NEXUS_DvbCi_GetStatus
    NEXUS_DvbCi_Read
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_Write(
    NEXUS_DvbCiHandle handle,
    const void *pBuffer,        /* attr{nelem=bufferSize;reserved=16} Pointer to data */
    uint16_t bufferSize,        /* size of buffer in bytes - this should be >= the negotiated size */
    uint16_t *pSizeWritten      /* [out] number of bytes written */
    );

/***************************************************************************
Summary:
    Read a buffer of data from a DVB-CI module

Description:
    This routine will read a buffer of data from a DVB-CI module.  If the module
    has not asserted the DA bit, this will return an error and the application should
    retry after the NEXUS_DvbCiStatus.daAsserted is set to true.  The buffer provided
    should be >= the negotiated buffer size.

See Also:
    NEXUS_DvbCi_GetStatus
    NEXUS_DvbCi_Read
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_Read(
    NEXUS_DvbCiHandle handle,
    void *pBuffer,              /* [out] attr{nelem=bufferSize;reserved=16} Pointer to data */
    uint16_t bufferSize,        /* size of buffer in bytes - this should be <= the negotiated size */
    uint16_t *pSizeRead         /* [out] number of bytes read */
    );

/***************************************************************************
Summary:
    CIS information from the current card
***************************************************************************/
#define NEXUS_DVB_CI_MANUFACTURER_NAME_MAX 128
#define NEXUS_DVB_CI_PRODUCT_NAME_MAX 128
#define NEXUS_DVB_CI_ADDITIONAL_INFO_MAX 128
typedef struct NEXUS_DvbCiCisData
{    
    /* Module identification data extracted from the PCMCIA CIS */
    char manufacturerName[NEXUS_DVB_CI_MANUFACTURER_NAME_MAX];
    char productName[NEXUS_DVB_CI_PRODUCT_NAME_MAX];
    char additionalInfo[NEXUS_DVB_CI_ADDITIONAL_INFO_MAX];

    uint32_t corAddr;               /* The offset to the configuration option register (COR) */
    uint8_t corValue;               /* The value written to the COR to change personality */
    bool vpp5v;                     /* If true, VPP is 5V for this device, else 3.3V */
    bool interruptsSupported;       /* If true, this module supports DA/FR interrupts */
} NEXUS_DvbCiCisData;

/***************************************************************************
Summary:
    Get CIS information from the current card

Description:
    This routine will return data extracted from the CIS of the card
    currently inserted in the slot.

See Also:
    NEXUS_DvbCi_GetStatus
***************************************************************************/
NEXUS_Error NEXUS_DvbCi_GetCisData(
    NEXUS_DvbCiHandle handle,
    NEXUS_DvbCiCisData *pCisData    /* [out] */
    );

#ifdef __cplusplus
}
#endif


#endif /* #ifndef NEXUS_DVB_CI_H__ */


