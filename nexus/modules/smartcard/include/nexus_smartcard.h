/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Smartcard
*    Specific APIs related to SMARTCARD Control.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_SMARTCARD_H__
#define NEXUS_SMARTCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for the Smartcard Interface.
***************************************************************************/
typedef struct NEXUS_Smartcard *NEXUS_SmartcardHandle;

/***************************************************************************
Summary:
Smartcard state enum.

Description:
This represents the current state of the given slot and card.
***************************************************************************/
typedef enum NEXUS_SmartcardState
{
    NEXUS_SmartcardState_eUnknown = 0, /* Unknown state (perhaps not yet initialized).  Persistent. */
    NEXUS_SmartcardState_eColdResetting, /* A cold reset has been requested but is not yet complete.  Transient. */
    NEXUS_SmartcardState_eWarmResetting, /* A warm reset has been requested but is not yet complete.  Transient. */
    NEXUS_SmartcardState_eResetDone, /* The slot/card reset has completed.  Persistent. */
    NEXUS_SmartcardState_eActivating, /* The slot/card is currently being activated, but activation is not yet complete.  Transient. */
    NEXUS_SmartcardState_eReceiveDecodeAtr, /* The ATR is being received or decoded.  Transient. */
    NEXUS_SmartcardState_eReady, /* The slot/card is initialized and is awaiting sends/receives.  Persistent. */
    NEXUS_SmartcardState_eTransmitting, /* The slot/card is currently transmitting. Transient. */
    NEXUS_SmartcardState_eTransmitted, /* The slot/card has completed its transmission. Persistent. */
    NEXUS_SmartcardState_eReceiving, /* The slot/card is currently receiving.  Transient. */
    NEXUS_SmartcardState_eIgnore, /* The slot/card is ignoring events/commands. Persistent. */
    NEXUS_SmartcardState_eInitialized, /* The slot/card has been initialized, but the ATR has not yet been received.  Persistent. */
    NEXUS_SmartcardState_eMax /* A value indicating the total number of possible states.  The state returned from NEXUS_SmartCard_GetStatus should never exceed this value. */
} NEXUS_SmartcardState;

/***************************************************************************
Summary:
Status information returned by NEXUS_Smartcard_GetStatus.
***************************************************************************/
typedef struct NEXUS_SmartcardStatus
{
    bool cardPresent;
    NEXUS_SmartcardState state;
    unsigned iccClockFrequency; /* units of Hz */
} NEXUS_SmartcardStatus;

/***************************************************************************
Summary:
Smartcard protocol enum definition.
***************************************************************************/
typedef enum NEXUS_SmartcardProtocol
{
    NEXUS_SmartcardProtocol_eUnknown,
    NEXUS_SmartcardProtocol_eT0,
    NEXUS_SmartcardProtocol_eT1,
    NEXUS_SmartcardProtocol_eT14,
    NEXUS_SmartcardProtocol_eMax
} NEXUS_SmartcardProtocol;

/***************************************************************************
Summary:
Smartcard standard enum definition.

Description:
Used in NEXUS_SmartcardSettings
***************************************************************************/
typedef enum NEXUS_SmartcardStandard
{
    NEXUS_SmartcardStandard_eUnknown,
    NEXUS_SmartcardStandard_eNds,
    NEXUS_SmartcardStandard_eIso,
    NEXUS_SmartcardStandard_eEmv1996,
    NEXUS_SmartcardStandard_eEmv2000,
    NEXUS_SmartcardStandard_eIrdeto,
    NEXUS_SmartcardStandard_eArib,
    NEXUS_SmartcardStandard_eMt,
    NEXUS_SmartcardStandard_eConax,
    NEXUS_SmartcardStandard_eEs,
    NEXUS_SmartcardStandard_eNdsNoFlowControl,
    NEXUS_SmartcardStandard_eNordig,
    NEXUS_SmartcardStandard_eMax
} NEXUS_SmartcardStandard;

/***************************************************************************
Summary:
This enum is to identify the unit of timer value.
****************************************************************************/
typedef enum NEXUS_TimerUnit {
    NEXUS_TimerUnit_eEtu,       /* in Elementary Time Units */
    NEXUS_TimerUnit_eClk,       /* in raw clock cycles that smart card receives */
    NEXUS_TimerUnit_eMilliSec,  /* in milliseconds */
    NEXUS_TimerUnit_eMax
} NEXUS_TimerUnit;

/***************************************************************************
Summary:
The timer value that application set to or get from the smartcard.
****************************************************************************/
typedef struct NEXUS_TimerValue
{
    uint32_t        value;    /* timer value */
    NEXUS_TimerUnit unit;   /* units */
} NEXUS_TimerValue;

/***************************************************************************
Summary:
This enum is to identify error detect code (EDC) encoding.

Description:
This enumeration defines the supported error detect code (EDC) encoding .
****************************************************************************/
typedef enum NEXUS_EdcEncode {
    NEXUS_EdcEncode_eLrd,
    NEXUS_EdcEncode_eCrc,
    NEXUS_EdcEncode_eMax
} NEXUS_EdcEncode;

/***************************************************************************
Summary:
The configuration of EDC setting for T=1 protocol only.
****************************************************************************/
typedef struct NEXUS_EdcSetting
{
    NEXUS_EdcEncode edcEncode;   /* EDC encoding */
    bool    isEnabled;
} NEXUS_EdcSetting;

/***************************************************************************
Summary:
This enum is to identify read or write a specific register.
****************************************************************************/
typedef enum NEXUS_ScPresMode {
    NEXUS_ScPresMode_eDebounce = 0,
    NEXUS_ScPresMode_eMask,
    NEXUS_ScPresMode_eMax
}  NEXUS_ScPresMode;

/***************************************************************************
Summary:
The configuration of Smartcard Pres Debounce.
****************************************************************************/
typedef struct NEXUS_ScPresDbInfo
{
    NEXUS_ScPresMode    scPresMode;
    bool                isEnabled;
    uint8_t             dbWidth;
} NEXUS_ScPresDbInfo;

/***************************************************************************
Summary:
This enum is to identify action for NEXUS_Smartcard_ResetCard function.

Description:
This enumeration defines the supported action for NEXUS_Smartcard_ResetCard function.

See Also:
NEXUS_Smartcard_ResetCard
****************************************************************************/
typedef enum NEXUS_ResetCardAction
{
    NEXUS_ResetCardAction_eNoAction = 0,
    NEXUS_ResetCardAction_eReceiveAndDecode,
    NEXUS_ResetCardAction_eMax
} NEXUS_ResetCardAction;

/***************************************************************************
Summary:
This enum is to identify action for NEXUS_Smartcard_PowerIcc function.
****************************************************************************/
typedef enum NEXUS_SmartcardPowerIcc {
    NEXUS_SmartcardPowerIcc_ePowerDown = 0,   /* power down the ICC and request deactivation of the contact */
    NEXUS_SmartcardPowerIcc_ePowerUp = 1,    /* power up the ICC and request activation of the contact */
    NEXUS_SmartcardPowerIcc_eMax
} NEXUS_SmartcardPowerIcc;

/***************************************************************************
Summary:
Voltage level for the smartcard interface
****************************************************************************/
typedef enum NEXUS_SmartcardVcc {
    NEXUS_SmartcardVcc_e5V = 0,   /* 5v is default value */
    NEXUS_SmartcardVcc_e3V = 1,   /* 3v */
    NEXUS_SmartcardVcc_eMax
} NEXUS_SmartcardVcc;

/***************************************************************************
Summary:
Connection types to the smartcard interface
****************************************************************************/
typedef enum NEXUS_SmartcardConnection {
    NEXUS_SmartcardConnection_eTda8024,  /* Smartcard connected to the coupling chip TDA8024, which inturn in connected to the backend chip. */
    NEXUS_SmartcardConnection_eTda8034,  /* Smartcard connected to the coupling chip TDA8034, which inturn in connected to the backend chip. */
    NEXUS_SmartcardConnection_eDirect,   /* Smartcard is directly connected to the backend chip.
                                                                            Custom circuit needs to be present on the board to protect the smartcard from power surges, etc. */
    NEXUS_SmartcardConnection_eInternal, /* Coupling chip functionality integrated into the backedn chip. */
    NEXUS_SmartcardConnection_eMax
} NEXUS_SmartcardConnection;

/***************************************************************************
Summary:
Smartcard settings structure.

Description:
Smartcard settings structure, used by NEXUS_Smartcard_Open.
This allows protocol (T=0, T=1, T=14) and standard (NDS, ISO, EMV2000, IRDETO) to be selected.
***************************************************************************/
typedef struct NEXUS_SmartcardSettings
{
    NEXUS_SmartcardProtocol protocol;
    NEXUS_SmartcardStandard standard;
    uint8_t                 fFactor;
    uint8_t                 dFactor;
    uint8_t                 extClockDivisor;
    uint8_t                 txRetries;
    uint8_t                 rxRetries;
    uint8_t                 baudDiv;
    NEXUS_TimerValue        workWaitTime;
    NEXUS_TimerValue        blockWaitTime;
    NEXUS_TimerValue        extraGuardTime;
    NEXUS_TimerValue        blockGuardTime;
    uint32_t                characterWaitTime;
    NEXUS_EdcSetting        edcSetting;
    NEXUS_TimerValue        timeOut;
    bool                    autoDeactiveReq;
    bool                    nullFilter;
    NEXUS_ScPresDbInfo      scPresDbInfo;
    NEXUS_ResetCardAction   resetCardAction;
    NEXUS_TimerValue        blockWaitTimeExt;
    bool                    isPresHigh;
    NEXUS_CallbackDesc      cardCallback;    /* Called when a card is either inserted or removed. This replaces insertCard and removeCard. */
    bool                    setPinmux;       /* Only applies during NEXUS_Smartcard_Open. */
    bool                    connectDirectly; /* DEPRECATED. Use NEXUS_SmartcardConnection instead.
                                                                                   If true, smartcard is directly connected to the backend chip.
                                                                                   If false (default), smartcard connected to the coupling chip TDA8024.
                                                                               */
    NEXUS_SmartcardConnection connection;
    uint32_t                currentIFSD;     /* This attribute indicates the current IFSD */
    unsigned                sourceClockFreq; /* Configures the Smartcard CLK freq for each smartcard channel (slot). In units of Hz. */
    NEXUS_TimerValue        atrReceiveTime;  /* Defines the time to wait for the first byte of ATR. */
    unsigned                resetCycles;     /* Defines the reset cycles for the coupling chips. */
    bool                    setVcc;          /* If true, Sets Vcc to vcc volts. This setting is only for TDA8024 chip. */
    NEXUS_SmartcardVcc      vcc;             /* This setting applie only for TDA8024 chip. */
    struct
    {
        bool                vccInverted;   /* If true, then the direct power supply logic is inverted. That is,sc_vcc=1 will set smartcard VCC pin to low. Default is false. */
        bool                resetInverted; /* If true, then the smartcard reset is inverted. That is,sc_rst=1 will set smartcard reset pin to low. Default is false. */
    } directPowerSupply;                   /* This applies only to power supply connected directly to the smartcard and NOT using TDA8024. */
} NEXUS_SmartcardSettings;

/***************************************************************************
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_Smartcard_Open
***************************************************************************/
void NEXUS_Smartcard_GetDefaultSettings(
    NEXUS_SmartcardSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open a Smartcard handle.

Description:

See Also:
NEXUS_Smartcard_Close
***************************************************************************/
NEXUS_SmartcardHandle NEXUS_Smartcard_Open( /* attr{destructor=NEXUS_Smartcard_Close}  */
    unsigned index,
    const NEXUS_SmartcardSettings *pSettings
    );

/***************************************************************************
Summary:
Close a Smartcard handle.

Description:

See Also:
NEXUS_Smartcard_Open
***************************************************************************/
void NEXUS_Smartcard_Close(
    NEXUS_SmartcardHandle handle
    );

/***************************************************************************
Summary:
Get the settings for a SMARTCARD
***************************************************************************/
void NEXUS_Smartcard_GetSettings(
    NEXUS_SmartcardHandle handle,
    NEXUS_SmartcardSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Set the settings for a SMARTCARD
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_SetSettings(
    NEXUS_SmartcardHandle handle,
    const NEXUS_SmartcardSettings *pSettings
    );

/***************************************************************************
Summary:
Read data from the smartcard.

Description:
If read fails, you should call NEXUS_Smartcard_GetStatus in order to determine
the state of the smartcard interface.

See Also:
NEXUS_Smartcard_Write
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_Read(
    NEXUS_SmartcardHandle smartcard,    /* handle returned by NEXUS_SmartCard_Open */
    void *pdata,            /* [out] attr{nelem=numBytes;nelem_out=pBytesRead;reserved=64} memory to read into */
    size_t numBytes,        /* maximum number of bytes to read */
    size_t *pBytesRead      /* [out] amount of data read into memory */
    );

/***************************************************************************
Summary:
Write data to a smartcard.

Description:
If write fails, you should call NEXUS_Smartcard_GetStatus in order to determine
the state of the smartcard interface.

See Also:
NEXUS_Smartcard_Read
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_Write(
    NEXUS_SmartcardHandle smartcard,
    const void *pData, /* attr{nelem=numBytes;reserved=64} */
    size_t numBytes,
    size_t *pBytesWritten /* [out] */
    );

/***************************************************************************
Summary:
Get the current status of a smartcard.
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_GetStatus(
    NEXUS_SmartcardHandle smartcard,
    NEXUS_SmartcardStatus *pStatus   /* [out] */
    );

/***************************************************************************
Summary:
Reset a smartcard.

Description:
Reset the smartcard itself.

See Also:
NEXUS_Smartcard_Read
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_ResetCard(
    NEXUS_SmartcardHandle smartcard, /* handle returned by NEXUS_SmartCard_Open */
    void *pdata,            /* [out] attr{nelem=numBytes;nelem_out=pBytesRead;reserved=64} pointer to memory that can be read into */
    size_t numBytes,        /* maximum number of bytes pointed to by data */
    size_t *pBytesRead      /* [out] length of data read into the data field. */
    );

/***************************************************************************
Summary:
Reset the smartcard interface.

Description:
Reprogram all the Broadcom smartcard interface, not the card.
If you want to reset the card, use NEXUS_Smartcard_ResetCard.

The interface must be reset whenever a card is inserted.

See Also:
NEXUS_Smartcard_Read
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_Reset(
    NEXUS_SmartcardHandle smartcard, /* handle returned by NEXUS_SmartCard_Open */
    bool warmReset /* true for a warm reset, false for a cold reset */
    );

/***************************************************************************
Summary:
Detect the card insertion.

Description:
The function will be blocked until the card is inserted.

See Also:
NEXUS_Smartcard_Read
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_DetectCard(
    NEXUS_SmartcardHandle smartcard
    );

/***************************************************************************
Summary:
Power ICC

Description:
If iccAction is NEXUS_SmartcardPowerIcc_ePowerUp, this function shall set SC_VCC
high.  System should call this function so that the next NEXUS_Smartcard_GetStatus
will show the correct presence of the card or next NEXUS_Smartcard_DetectCard
will response correctly after TDA8004 emergency deactivation.

If iccAction is NEXUS_SmartcardPowerIcc_ePowerDown, this function shall set SC_VCC
low.  The next NEXUS_Smartcard_GetStatus may not show the correct presence of the
card or next NEXUS_Smartcard_DetectCard may not response correctly after
TDA8004 emergency deactivation.
**************************************************************************/
NEXUS_Error NEXUS_Smartcard_PowerIcc(
    NEXUS_SmartcardHandle      smartcard,
    NEXUS_SmartcardPowerIcc    iccAction
    );

/***************************************************************************
Summary:
Deactivate the smartcard.

Description:
This function is used to deactivate the smartcard. After deactivation, the smartcard
needs to be reset in order to communicate with it again.

See Also:
NEXUS_Smartcard_Read
***************************************************************************/
NEXUS_Error NEXUS_Smartcard_Deactivate (
    NEXUS_SmartcardHandle smartcard
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SMARTCARD_H__ */

