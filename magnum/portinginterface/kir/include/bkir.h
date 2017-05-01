/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/

/*= Module Overview *********************************************************
<verbatim>

Overview
The KIR PI module controls the keyboard/IR receiver core within the BCM7038.
There are two separate keyboard/IR receivers within the 7038.  Each KIR core
can support 2 ports, i.e. 2 separate input pins.  There is built-in
hardware support for several IR devices, such as Sejin, TWIRP, GI Remote A and B.
In addition, there is a consumer IR decoder that is programmable by the user.

Design
The design for BKIR PI API is broken into two parts.
  Part 1 (open/close/configuration):
    These APIs are used for opening and closing BKIR device/device channel.
  Part 2 (enable device):
    These APIs are used to enable a KIR device type.
  Part 3 (get receiver data):
    These APIs are used to check for data received, read it, and return it
    to the caller.

Usage
The usage of BKIR involves the following:
   * Configure/Open of BKIR
      * Configure BKIR device for the target system
      * Open BKIR device
      * Configure BKIR device channel for the target system
      * Open BKIR device channel
   * Enable device
      * Enable the IR device type.
   * Check to see if data is received.
   * Get the data received.

Sample Code
void main( void )
{
    BKIR_Handle         hKir;
    BKIR_ChannelHandle  hKirChan;
    BKIR_ChannelSettings defChnSettings;
    BREG_Handle         hReg;
    BCHP_Handle         hChip;
    BINT_Handle         hInt;
    bool                readyFlag = 0;
    uint32_t            data;
    int                 chanNo;
    BKIR_KirInterruptDevice interruptDevice;

    // Do other initialization, i.e. for BREG, BCHP, etc

    // Make any changes required from the default values
    BKIR_Open (&hKir, hChip, hReg, hInt, (BKIR_Settings *)NULL);

    chanNo = 0; //example for channel 0
    BKIR_GetChannelDefaultSettings( hKir, chanNo, &defChnSettings );

    defChnSettings.irPort = 2;      // using port 2

    BKIR_OpenChannel( hKir, &hKirChan, chanNo, &defChnSettings );

    // Enable TWIRP device
    BKIR_EnableIrDevice (hKirChan, BKIR_KirDevice_eTwirpKbd);

    interruptDevice = BKIR_KirInterruptDevice_eNone;
    do
    {
        // Using polling
        BKIR_IsDataReady (hKirChan, &readyFlag);
        if (readyFlag)
            BKIR_Read (hKirChan, &interruptDevice, &data);
    } while (interruptDevice != BKIR_KirInterruptDevice_eTwirpKbd)
}

</verbatim>
***************************************************************************/


#ifndef BKIR_H__
#define BKIR_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bchp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (BCHP_CHIP==7550)
    #define BKIR_N_CHANNELS 1 /* 7550 does not follow normal convention */
#else
    #if defined(BCHP_KBD3_REG_START)
        #define BKIR_N_CHANNELS 3
    #elif defined(BCHP_KBD2_REG_START)
        #define BKIR_N_CHANNELS 2
    #elif defined(BCHP_KBD1_REG_START)
        #define BKIR_N_CHANNELS 1
    #endif
#endif

/***************************************************************************
Summary:
    Error Codes specific to BKIR

Description:

See Also:

****************************************************************************/
#define BKIR_ERR_NOTAVAIL_CHN_NO            BERR_MAKE_CODE(BERR_KIR_ID, 0)

/***************************************************************************
Summary:
    The handles for kir module.

Description:
    Since BKIR is a device channel, it has main device handle as well
    as a device channel handle.

See Also:
    BKIR_Open(), BKIR_OpenChannel()

****************************************************************************/
typedef struct BKIR_P_Handle                *BKIR_Handle;
typedef struct BKIR_P_ChannelHandle         *BKIR_ChannelHandle;

/***************************************************************************
Summary:
    Enumeration for supported KIR devices

Description:
    This enumeration defines the devices that the KIR PI supports.

See Also:
    None.

****************************************************************************/
typedef enum
{
    BKIR_KirDevice_eTwirpKbd = 1,       /* TWIRP */
    BKIR_KirDevice_eSejin38KhzKbd,      /* Sejin IR keyboard (38.4KHz) */
    BKIR_KirDevice_eSejin56KhzKbd,      /* Sejin IR keyboard (56.0KHz) */
    BKIR_KirDevice_eRemoteA,            /* remote A */
    BKIR_KirDevice_eRemoteB,            /* remote B */
    BKIR_KirDevice_eCirGI,              /* Consumer GI */
    BKIR_KirDevice_eCirSaE2050,         /* Consumer SA E2050 */
    BKIR_KirDevice_eCirTwirp,           /* Consumer Twirp */
    BKIR_KirDevice_eCirSony,            /* Consumer Sony */
    BKIR_KirDevice_eCirRecs80,          /* Consumer Rec580 */
    BKIR_KirDevice_eCirRc5,             /* Consumer Rc5 */
    BKIR_KirDevice_eCirUei,             /* Consumer UEI */
    BKIR_KirDevice_eCirRfUei,           /* Consumer RF UEI */
    BKIR_KirDevice_eCirEchoStar,        /* Consumer EchoRemote */
    BKIR_KirDevice_eSonySejin,          /* Sony Sejin keyboard using UART D */
    BKIR_KirDevice_eCirNec,             /* Consumer NEC */
    BKIR_KirDevice_eCirRC6,             /* Consumer RC6 */
    BKIR_KirDevice_eCirGISat,           /* Consumer GI Satellite */
    BKIR_KirDevice_eCirCustom,          /* Customer specific type */
    BKIR_KirDevice_eCirDirectvUhfr,     /* DIRECTV uhfr */
    BKIR_KirDevice_eCirEchostarUhfr,    /* Echostar uhfr */
    BKIR_KirDevice_eCirRcmmRcu,         /* RCMM Remote Control Unit */
    BKIR_KirDevice_eCirRstep,           /* R-step Remote Control Unit */
    BKIR_KirDevice_eCirXmp2,            /* XMP-2 4 byte data*/
    BKIR_KirDevice_eCirXmp2Ack,         /* XMP-2 Ack/Nak*/
    BKIR_KirDevice_eCirRC6Mode0,        /* Consumer RC6 Mode 0 */
    BKIR_KirDevice_eCirRca,             /* Consumer RCA */
    BKIR_KirDevice_eCirToshibaTC9012,   /* Consumer Toshiba */
    BKIR_KirDevice_eCirXip,             /* Consumer Tech4home XIP protocol */
    BKIR_KirDevice_eNumKirDevice        /* number of keyboard devices */
} BKIR_KirDevice;

/***************************************************************************
Summary:
    Enumeration for IR device type that generated the interrupt

Description:
    This enumeration defines the device type that generated the interrupt

See Also:
    None.

****************************************************************************/
typedef enum
{
    BKIR_KirInterruptDevice_eNone = 0,
    BKIR_KirInterruptDevice_eTwirpKbd,      /* TWIRP */
    BKIR_KirInterruptDevice_eSejinKbd,      /* SEJIN */
    BKIR_KirInterruptDevice_eRemoteA,
    BKIR_KirInterruptDevice_eRemoteB,
    BKIR_KirInterruptDevice_eCir

} BKIR_KirInterruptDevice;

/***************************************************************************
Summary:
    Enumeration for IR port selection

Description:
    This enumeration defines the port selection for each IR channel.

See Also:
    None.

****************************************************************************/
typedef enum
{
    BKIR_KirPort1,
    BKIR_KirPort2,
    BKIR_KirPortAuto
} BKIR_KirPort;

/***************************************************************************
Summary:
    Enumeration for IR tolerance selection

Description:
    This enumeration defines the encoding for the tolerance value.

See Also:
    None.

****************************************************************************/
typedef enum
{
    BKIR_KirInputTolerance_e12_5,   /* +/- 12.5% */
    BKIR_KirInputTolerance_e25,     /* +/- 25% */
    BKIR_KirInputTolerance_e50,     /* +/- 50% */
    BKIR_KirInputTolerance_eFixed  /* Use the value as tolerance */
} BKIR_Tolerance;

/***************************************************************************
Summary:
    Typedef for CIR parameters

Description:
    This structure typedef is designed for CIR parameters

See Also:
    None.

****************************************************************************/
typedef struct tspec {
    unsigned val;   /* value */
    unsigned char tol;  /* tolerance select code (see BKIR_Tolerance) */
} tspec;

typedef struct CIR_Param { /* CIR decoder configuration */
    unsigned countDivisor;  /* count clock divisor */
    tspec pa[4],        /* preamble A pulse sequence */
          pb[4];        /* preamble B pulse sequence */
    unsigned char paCount;  /* number of entries in pa[] */
    unsigned char pbCount;  /* number of entries in pb[] */
    int measurePreamblePulse;/* false => even counts specifies pulse off period */
                /* true => even counts specifies cycle period */
    int pbRepeat;       /* if true, pb[] matches a repeat sequence */
    unsigned pulseTol;      /* pulse tolerance */
    unsigned t0;        /* T0 */
    unsigned delT;      /* delta T */
    int fixSymPulseLast;    /* false => fix-width symbol pulse between */
                    /*   edges 0 & 1 */
                /* true => fix-width symbol pulse between */
                /*   edges 1 & 2 */
    int measureSymPulse;    /* false => measure spacing for complete cycle */
            /* true => measure spacing between 2 consecutive edges */
    tspec symPulseWidth;    /* data symbol fix-width pulse period */
    tspec spacingTol;       /* spacing tolerance value and select code */
    unsigned nSymA,     /* no. of symbols for sequence with preamble A */
             nSymB;     /* no. of symbols for sequence with preamble B */
    unsigned bitsPerSym;    /* no. of data bits per symbol */
    int mostSignifSymRecvFirst;/* true => most significant symbol received */
                    /*   first */
                /* false => least significant symbol received */
                /*   first */
    int leftAdjustRecvData; /* true => resulting received data is left */
                    /* adjusted using '0' to pad LSBs */
                    /* false => resulting received data is right */
                    /* adjusted using '0' to pad MSBs */
    int biphaseCoded;       /* true => the input signal is bi-phase coded */
                /* false => the input signal is pulse spacing */
                /*   coded */
    int twoSymPerCy;        /* two symbols per cycle, 1 or 0 */
    int chkStopSym;         /* check stop symbol, 1 or 0 */
    int varLenData;         /* variable length data, 1 or 0. */
    unsigned timeoutDivisor;    /* time-out clock divisor */
    unsigned frameTimeout;  /* frame time-out */
    unsigned edgeTimeout;   /* edge time-out */
    unsigned faultDeadTime; /* mininmum dead-time after fault */
    tspec    stop;          /* stop */
    unsigned dataSymTimeout;    /* data symbol timeout */

    unsigned repeatTimeout;     /* repeat timer timeout */
    int stopParamUnit;      /* stop parameter unit selector: */
                    /*  0: stop has count units */
                    /*  1: stop has timout units */
    int dataSymClkTickUnit;     /* data symbol timer clock tick and */
                    /*  dataSymTimeout units selector: */
                    /*  0: dataSymTimeout has count units */
                    /*  1: dataSymTimeout has timout units */
    int ignoreDataSymTimerEdge1;    /* ignore data symbol timer expiration */
                    /* while waiting for Edge 1; 0 or 1. */
    int dataSymTimerExpStatEn;  /* enable data symbol time-out */
                    /*   expiration flag to lflag bit in */
                    /*   status register */
    int enHavePreambleAftStop;  /* enable havePreambleAftStop parameter */
                    /* for non-biphase decoding, 0 or 1 */
    int havePreambleAftStop;    /* have preamble after stop symbol */
    int restrictiveDecode;      /* restrictive decoding enabled */
    int rc6;            /* RC6 encoded. Requires biphaseCoded=1 */
    int dontValidateMode;       /* don't validate RC6 mode bits */
    unsigned modeBits;      /* RC6 mode bits (3 bits), typically 6 */
    int dontValidateTrailer;    /* Reserved: don't validate RC6 trailer: */
                    /*  0:false, 1:true */
    unsigned trailer;       /* Reserved: RC6 trailer (1 bit): */
                    /*  0 for Mode 6A */
    int dontValidateCustCode;   /* don't validate customer code bits */
    unsigned custCode;      /* RC6 customer code bits (16 bits) */
    unsigned nCustCodeBits;     /* number of RC6 customer code bits: */
    unsigned passModeCustCodePass;
            /* RC6 mode bits and customer code pass-through control. */
            /* 0: Exclude mode bits and customer code from */
            /*    received data. The nccb field determines */
            /*    the size of the customer code. */
            /* 1: Exclude mode bits from the received data, but, */
            /*    include customer code. */
            /* 2: Not allowed. */
            /* 3: Include both mode bits and customer code in */
            /*    the received data. */
} CIR_Param;


/***************************************************************************
Summary:
    KIR user callback function

Description:
    The is the user callback function.  It allows a user to register a
    callback function with the KIR PI.  When a KIR interrupt happens,
    this callback function gets called if it's registered.

See Also:
    BKIR_RegisterCallback(), BKIR_UnregisterCallback()

****************************************************************************/
typedef BERR_Code (*BKIR_Callback)(BKIR_ChannelHandle hChn, void *pData);

/***************************************************************************
Summary:
    Required default settings structure for KIR module.

Description:
    The default setting structure defines the default configure of
    KIR when the device is opened.  Since BKIR is a device
    channel, it also has default settings for a device channel.
    Currently there are no parameters for device setting.

See Also:
    BKIR_Open(), BKIR_OpenChannel()

****************************************************************************/
typedef void *BKIR_Settings;

typedef struct BKIR_ChannelSettings
{
    BKIR_KirPort        irPort;         /* IR port */
    bool                intMode;        /* interrupt enable flag */
    BKIR_KirDevice      customDevice;   /* device that this custom cir is used for */
} BKIR_ChannelSettings;

/***************************************************************************
Summary:
    This function opens KIR module.

Description:
    This function is responsible for opening BKIR module. When BKIR is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BKIR_Close(), BKIR_OpenChannel(), BKIR_CloseChannel(),
    BKIR_GetDefaultSettings()

****************************************************************************/
BERR_Code BKIR_Open(
    BKIR_Handle *pKIR,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    const BKIR_Settings *pDefSettings   /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes KIR module.

Description:
    This function is responsible for closing BKIR module. Closing BKIR
    will free main BKIR handle. It is required that all opened
    BKIR channels must be closed before calling this function. If this
    is not done, the results will be unpredicable.

Returns:
    TODO:

See Also:
    BKIR_Open(), BKIR_CloseChannel()

****************************************************************************/
BERR_Code BKIR_Close(
    BKIR_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the default settings for KIR module.

Description:
    This function is responsible for returns the default setting for
    BKIR module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BKIR_Open()

****************************************************************************/
BERR_Code BKIR_GetDefaultSettings(
    BKIR_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    );

/***************************************************************************
Summary:
    This function returns the total number of channels supported by
    KIR module.

Description:
    This function is responsible for getting total number of channels
    supported by BKIR module, since BKIR device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BKIR_OpenChannel(), BKIR_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BKIR_GetTotalChannels(
    BKIR_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    );

/***************************************************************************
Summary:
    This function gets default setting for a KIR module channel.

Description:
    This function is responsible for returning the default setting for
    channel of BKIR. The return default setting is used when opening
    a channel.

Returns:
    TODO:

See Also:
    BKIR_OpenChannel()

****************************************************************************/
BERR_Code BKIR_GetChannelDefaultSettings(
    BKIR_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BKIR_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function gets default CIR setting for a KIR module channel and specified device.

Description:
    This function is responsible for returning the default setting for the
    specified device. The return default setting is used when setting custom CIR.

Returns:
    TODO:

****************************************************************************/
BERR_Code BKIR_GetDefaultCirParam (
    BKIR_KirDevice      device,          /* device type to enable */
    CIR_Param           *pCustomCirParam /* [output] Returns default setting */
);

/***************************************************************************
Summary:
    This function gets current CIR setting for a KIR module channel and specified device.

Description:
    This function is responsible for returning the current setting for
    channel of BKIR and the specified device. The return setting is
    used when setting custom CIR.

Returns:
    TODO:

See Also:
    BKIR_SetCustomCir()

****************************************************************************/
BERR_Code BKIR_GetCurrentCirParam (
    BKIR_ChannelHandle  hChn,            /* Device channel handle */
    BKIR_KirDevice      device,          /* device type to enable */
    CIR_Param           *pCustomCirParam /* [output] Returns default setting */
);

/***************************************************************************
Summary:
    This function opens KIR module channel.

Description:
    This function is responsible for opening BKIR module channel. When a
    BKIR channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

Returns:
    TODO:

See Also:
    BKIR_CloseChannel(), BKIR_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BKIR_OpenChannel(
    BKIR_Handle hDev,                   /* [in] Device handle */
    BKIR_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
    const BKIR_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function closes KIR module channel.

Description:
    This function is responsible for closing BKIR module channel. Closing
    BKIR channel it will free BKIR channel handle. It is required that all
    opened BKIR channels must be closed before closing BKIR.

Returns:
    TODO:

See Also:
    BKIR_OpenChannel(), BKIR_CloseChannel()

****************************************************************************/
BERR_Code BKIR_CloseChannel(
    BKIR_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets KIR module device handle based on
    the device channel handle.

Description:
    This function is responsible returning BKIR module handle based on the
    BKIR module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_GetDevice(
    BKIR_ChannelHandle hChn,            /* [in] Device channel handle */
    BKIR_Handle *pKIR                   /* [out] Returns Device handle */
    );

/***************************************************************************
Summary:
    This function checks to see if data is received.

Description:
    This function is used to check to see if data is received by the KIR receiver.

Returns:
    TODO:

See Also:


****************************************************************************/
BERR_Code BKIR_IsDataReady (
    BKIR_ChannelHandle  hChn,           /* [in] Device channel handle */
    bool                *dataReady      /* [out] flag to indicate if data is ready */
    );

/***************************************************************************
Summary:
    This function reads the received KIR data

Description:
    This function is used to read the data received by the KIR receiver.
    WARNING: the memory for the data returned MUST be larger than 8 chars.

Returns:
    TODO:

See Also:


****************************************************************************/
BERR_Code BKIR_Read(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    BKIR_KirInterruptDevice *pDevice,       /* [out] pointer to IR device type that generated the key */
    unsigned char           *data           /* [out] pointer to data received */
    );

/***************************************************************************
Summary:
    This function reads the received KIR data.
    Intended to be called from ISR.

Description:
    This function is used to read the data received by the KIR receiver.
    WARNING: the memory for the data returned MUST be larger than 8 chars.

Returns:
    TODO:

See Also:


****************************************************************************/
BERR_Code BKIR_Read_isr(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    BKIR_KirInterruptDevice *pDevice,       /* [out] pointer to IR device type that generated the key */
    unsigned char           *data           /* [out] pointer to data received */
    );

/***************************************************************************
Summary:
    This function checks to see if remote A repeat condition occurs.

Description:
    This function is used to check if the remote A repeat condition occurs.

Returns:
    TODO:

See Also:


****************************************************************************/
#define BKIR_IsRepeated BKIR_IsRepeated_isrsafe
BERR_Code BKIR_IsRepeated_isrsafe(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    bool                    *repeatFlag     /* [out] flag to remote A repeat condition */
    );

/***************************************************************************
Summary:
    This function checks to see if preamble A is detected.

Description:
    This function is used to check if the preamble A is detected.

Returns:
    TODO:

See Also:


****************************************************************************/
#define BKIR_IsPreambleA BKIR_IsPreambleA_isrsafe
BERR_Code BKIR_IsPreambleA_isrsafe(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    bool                    *preambleFlag   /* [out] flag for preamble A */
    );

/***************************************************************************
Summary:
    This function checks to see if preamble B is detected.

Description:
    This function is used to check if the preamble B is detected.

Returns:
    TODO:

See Also:


****************************************************************************/
#define BKIR_IsPreambleB BKIR_IsPreambleB_isrsafe
BERR_Code BKIR_IsPreambleB_isrsafe(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    bool                    *preambleFlag   /* [out] flag for preamble B */
    );

/***************************************************************************
Summary:
    This function returns the last key pressed.

Description:
    This function is used to retreive the key that was pressed to wakeup
    after an S3 or S5 suspend.  This must be used immediatly after channel
    open.

Returns:
    TODO:

See Also:

****************************************************************************/
void BKIR_GetLastKey(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    uint32_t *code,           /* [out] lower 32-bits of returned code */
    uint32_t *codeHigh,       /* [out] upper 32-bits of returned code */
    bool *preambleA,          /* [out] flag for preamble A */
    bool *preambleB           /* [out] flag for preamble B */
    );

/***************************************************************************
Summary:
    This function enables a KIR device.

Description:
    This function enables a KIR device type.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_EnableIrDevice (
    BKIR_ChannelHandle  hChn,           /* [in] Device channel handle */
    BKIR_KirDevice      device          /* [in] device type to enable */
    );

/***************************************************************************
Summary:
    This function disables a KIR device.

Description:
    This function disables a KIR device type.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_DisableIrDevice (
    BKIR_ChannelHandle  hChn,           /* [in] Device channel handle */
    BKIR_KirDevice      device          /* [in] device type to disable */
    );

/***************************************************************************
Summary:
    This function enables a data filter.

Description:
    This function enables a data filter based on the pattern.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_EnableDataFilter (
    BKIR_ChannelHandle  hChn,       /* [in] Device channel handle */
    uint64_t            pat0,       /* [in] pattern0 to match (only least significant 48bits are used) */
    uint64_t            pat1,       /* [in] pattern1 to match (only least significant 48bits are used) */
    uint64_t            mask0,      /* [in] don't care bits in pattern0 (only least significant 48bits are used) */
    uint64_t            mask1       /* [in] don't care bits in pattern1 (only least significant 48bits are used) */
);

/***************************************************************************
Summary:
    This function disables a data filter.

Description:
    This function disables a data filter based on the pattern.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_DisableDataFilter (
    BKIR_ChannelHandle  hChn        /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function enables a filter1.

Description:
    This function enables a filter1 using the filter width.
	Any pulse smaller than (28*filter_width+2)/27) microseconds	will be rejected
Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_EnableFilter1 (
    BKIR_ChannelHandle  hChn,       	/* [in] Device channel handle */
    unsigned int        filter_width 	/* filter width if smaller than this to be rejected */
    );

/***************************************************************************
Summary:
    This function disables a data filter.

Description:
    This function disables a data filter based on the pattern.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_DisableFilter1 (
    BKIR_ChannelHandle  hChn        /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function disables all KIR devices.

Description:
    This function disables all KIR devices.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_DisableAllIrDevices (
    BKIR_ChannelHandle  hChn            /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets the event handle for BKIR module channel.

Description:
    This function is responsible for getting the event handle. The
    application code should use this function get BKIR's event handle,
    which the application should use to pend on.  The KIR ISR will
    set the event.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BKIR_GetEventHandle(
    BKIR_ChannelHandle hChn,            /* [in] Device channel handle */
    BKNI_EventHandle *phEvent           /* [out] Returns event handle */
    );

/***************************************************************************
Summary:
    This function sets the custom device type.

Description:
    This function is called to set the custom device type.
    This function should be called before calling BKIR_EnableIrDevice function
    since custom device type is used in BKIR_EnableIrDevice function

Returns:
    TODO:

See Also:

****************************************************************************/
void BKIR_SetCustomDeviceType (
    BKIR_ChannelHandle  hChn,           /* Device channel handle */
    BKIR_KirDevice      customDevice    /* device that this custom cir is used for */
);

/***************************************************************************
Summary:
    This function sets the custom CIR's parameters.

Description:
    This function is called to set the custom CIR parameters to the ones
    pointed to by pCirParam.  This allows the user to change the CIR
    parameters on the fly.

Returns:
    TODO:

See Also:

****************************************************************************/
void BKIR_SetCustomCir (
    BKIR_ChannelHandle  hChn,           /* [in] Device channel handle */
    CIR_Param           *pCirParam      /* [in] Pointer to custom CIR parameters */
);

/***************************************************************************
Summary:
    This function registers a callback function with KIR PI

Description:
    This function is used to register a callback function with KIR PI.
    When a KIR interrupt happens, if a callback function has been registered
    for this channel, it will call that function.

Returns:
    TODO:

See Also:

****************************************************************************/
void BKIR_RegisterCallback (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    BKIR_Callback       callback,   /* Callback function to register */
    void                *pData      /* Data passed to callback function */
);

/***************************************************************************
Summary:
    This function unregisters a callback function with KIR PI

Description:
    This function is used to unregister a callback function for a
    particular KIR channel.

Returns:
    TODO:

See Also:

****************************************************************************/
void BKIR_UnregisterCallback (
    BKIR_ChannelHandle  hChn        /* Device channel handle */
);

#ifdef __cplusplus
}
#endif

#endif



