/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
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

/*============================= Module Overview ===============================
<verbatim>
The DSQ portinginterface controls the Broadcom satellite DiSEqC core 
starting with 28nm and later generations.  For earlier Broadcom satellite 
frontends, use the AST portinginterface instead.
</verbatim>
=============================================================================*/

#ifndef _BDSQ_H_
#define _BDSQ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bint.h"
#include "bkni.h"
#include "bfec.h"
#include "berr_ids.h"


#define BDSQ_API_VERSION 3

#define BDSQ_MAX_TX_LEN 128
#define BDSQ_MAX_RX_LEN 72


/******************************************************************************
Summary:
   BDSQ error codes.
Desciption:
   This is a list of return error codes specific to the BDSQ PI.
See Also:
   None.
******************************************************************************/
#define BDSQ_ERR_POWERED_DOWN    BERR_MAKE_CODE(BERR_DSQ_ID, 1)   /* cannot access h/w block due to it being powered down */
#define BDSQ_ERR_RX_PARITY       BERR_MAKE_CODE(BERR_DSQ_ID, 2)   /* indicates parity error in rx bytes */
#define BDSQ_ERR_RX_INCOMPLETE   BERR_MAKE_CODE(BERR_DSQ_ID, 3)   /* indicates incomplete rx transaction */


/******************************************************************************
Summary:
   BDSQ channel configuration parameters.
Desciption:
   This is an enumeration of channel configuration parameters used by the BDSQ PI for all chips.
See Also:
   None.
******************************************************************************/
/* channel configuration parameters */
enum {
   BDSQ_CHAN_CONFIG_RRTO_US,              /* diseqc receive reply timeout in usecs */
   BDSQ_CHAN_CONFIG_BIT_THRESHOLD_US,     /* threshold for diseqc rx bit detection in usecs */
   BDSQ_CHAN_CONFIG_TONE_THRESHOLD,       /* threshold for diseqc energy tone detect in units of 0.16 counts per mV */
   BDSQ_CHAN_CONFIG_PRETX_DELAY_MS,       /* delay time in msecs before transmitting diseqc command */
   BDSQ_CHAN_CONFIG_VSENSE_THRESHOLD_HI,  /* voltage detect overvoltage threshold */
   BDSQ_CHAN_CONFIG_VSENSE_THRESHOLD_LO,  /* voltage detect undervoltage threshold */
   BDSQ_CHAN_CONFIG_PGA_GAIN,             /* receiver pga gain */
   BDSQ_CHAN_CONFIG_ENABLE_LNBPU,         /* 0=LNBPU not used , 1=LNBPU on TXEN pin */
   BDSQ_CHAN_CHAN_CONFIG_MAX
};


/******************************************************************************
Summary:
   Handle for the BDSQ device.
Desciption:
   This is an opaque handle for the BDSQ device.
See Also:
   BDSQ_Open()
******************************************************************************/
typedef struct BDSQ_P_Handle *BDSQ_Handle;


/******************************************************************************
Summary:
   Handle for the BDSQ channel device.
Desciption:
   This is an opaque handle for the BDSQ channel device.
See Also:
   BDSQ_OpenChannel()
******************************************************************************/
typedef struct BDSQ_P_ChannelHandle *BDSQ_ChannelHandle;


/******************************************************************************
Summary:
   Status of the previous DiSEqC transaction
Description:
   This enum is the error status of the previous DiSEqC transaction.
See Also:
   BDSQ_Write, BDSQ_GetStatus
******************************************************************************/
typedef enum BDSQ_SendStatus
{
   BDSQ_SendStatus_eSuccess = 0, /* no error */
   BDSQ_SendStatus_eRxOverflow,
   BDSQ_SendStatus_eRxReplyTimeout,
   BDSQ_SendStatus_eRxParityError,
   BDSQ_SendStatus_eAcwTimeout,
   BDSQ_SendStatus_eBusy,
   BDSQ_SendStatus_eRxIncomplete
} BDSQ_SendStatus;


/******************************************************************************
Summary:
   This struct holds the status from the most recent diseqc transaction. 
Description:
   This struct holds the status from the most recent diseqc transaction. 
See Also:
   BDSQ_Write, BDSQ_GetStatus
******************************************************************************/
typedef struct BDSQ_Status
{
   BDSQ_SendStatus status;       /* status of most recent transaction */
   bool            bRxExpected;  /* true=reply bytes expected */
   uint16_t        posParityErr; /* bit mask indicating byte position of parity error */
   uint8_t         nRxBytes;     /* number of valid reply bytes received */
   uint8_t         rssiLevel;    /* max rssi level of most recent packet */
} BDSQ_Status;


/******************************************************************************
Summary:
   This struct contains 22kHz activity information.
Description:
   This struct contains 22kHz activity information.
See Also:
   BDSQ_GetActivityStatus, BDSQ_ResetActivityStatus
******************************************************************************/
typedef struct BDSQ_ActivityStatus
{
   bool  bToneDetected; /* tone detected sticky status */
   bool  bToneActive;   /* tone status in real-time */
} BDSQ_ActivityStatus;


/******************************************************************************
Summary:
   Structure containing BDSQ device channel settings.
Description:
   This structure contains BDSQ channel settings that will be passed into 
   BDSQ_OpenChannel().
See Also:
   BDSQ_OpenChannel(), BDSQ_GetChannelDefaultSettings()
******************************************************************************/
typedef struct BDSQ_ChannelSettings
{
   bool     bEnvelope;           /* false=tone mode, true=envelope mode on TXOUT pin */
   bool     bToneAlign;          /* tone alignment: false=off, true=on */
   bool     bDisableRRTO;        /* false=RxReplyTimeOut enabled, true=RRTO disabled */
   bool     bEnableToneburst;    /* tone burst: false=disabled, true=enabled */
   bool     bToneburstB;         /* applies if tone burst is enabled: false=tone A, true=tone B */
   bool     bOverrideFraming;    /* false=first byte bit 1 of command indicates reply expected, true=override framing byte, reply expectation depends on bExpectReply */
   bool     bExpectReply;        /* applies if bOverrideFraming set: false=don't expect reply, true=always expect reply */
   bool     bEnableLNBPU;        /* TBD deprecated */
   bool     bAdaptiveRxSlice;    /* false=off, true=enable adaptive rx slicing */
} BDSQ_ChannelSettings;


struct BDSQ_Settings;
struct BDSQ_ChannelSettings;

/******************************************************************************
Summary:
   Structure representing API function table.
Desciption:
   This structure contains pointers to all public BDSQ functions.
See Also:
   BDSQ_Settings
******************************************************************************/
typedef struct BDSQ_ApiFunctTable
{
   BERR_Code (*Open)(BDSQ_Handle*, BCHP_Handle, void*, BINT_Handle, const struct BDSQ_Settings*);
   BERR_Code (*Close)(BDSQ_Handle);
   BERR_Code (*GetTotalChannels)(BDSQ_Handle, uint32_t*);
   BERR_Code (*GetChannelDefaultSettings)(BDSQ_Handle, uint32_t, struct BDSQ_ChannelSettings*);
   BERR_Code (*OpenChannel)(BDSQ_Handle, BDSQ_ChannelHandle*, uint32_t, const struct BDSQ_ChannelSettings*);
   BERR_Code (*CloseChannel)(BDSQ_ChannelHandle);
   BERR_Code (*GetDevice)(BDSQ_ChannelHandle, BDSQ_Handle*);
   BERR_Code (*GetVersionInfo)(BDSQ_Handle, BFEC_VersionInfo*);
   BERR_Code (*Reset)(BDSQ_Handle);
   BERR_Code (*ResetChannel)(BDSQ_ChannelHandle);
   BERR_Code (*PowerDownChannel)(BDSQ_ChannelHandle);
   BERR_Code (*PowerUpChannel)(BDSQ_ChannelHandle);
   BERR_Code (*IsChannelOn)(BDSQ_ChannelHandle, bool*);
   BERR_Code (*PowerDownVsense)(BDSQ_ChannelHandle);
   BERR_Code (*PowerUpVsense)(BDSQ_ChannelHandle);
   BERR_Code (*IsVsenseOn)(BDSQ_ChannelHandle, bool*);
   BERR_Code (*SetTone)(BDSQ_ChannelHandle, bool);
   BERR_Code (*GetTone)(BDSQ_ChannelHandle, bool*);
   BERR_Code (*SetVoltage)(BDSQ_ChannelHandle, bool);
   BERR_Code (*GetVoltage)(BDSQ_ChannelHandle, uint16_t*);
   BERR_Code (*EnableVsenseInterrupt)(BDSQ_ChannelHandle, bool);
   BERR_Code (*EnableActivityInterrupt)(BDSQ_ChannelHandle, bool);
   BERR_Code (*EnableIdleInterrupt)(BDSQ_ChannelHandle, bool);
   BERR_Code (*Write)(BDSQ_ChannelHandle, const uint8_t*, uint8_t);
   BERR_Code (*EnableRx)(BDSQ_ChannelHandle, bool);
   BERR_Code (*EnableLnb)(BDSQ_ChannelHandle, bool);
   BERR_Code (*Read)(BDSQ_ChannelHandle, uint8_t*, uint8_t, uint8_t*, uint8_t*);
   BERR_Code (*SendAcw)(BDSQ_ChannelHandle, uint8_t);
   BERR_Code (*GetStatus)(BDSQ_ChannelHandle, BDSQ_Status*);
   BERR_Code (*GetActivityStatus)(BDSQ_ChannelHandle, BDSQ_ActivityStatus*);
   BERR_Code (*ResetActivityStatus)(BDSQ_ChannelHandle);
   BERR_Code (*SetChannelSettings)(BDSQ_ChannelHandle, BDSQ_ChannelSettings*);
   BERR_Code (*GetChannelSettings)(BDSQ_ChannelHandle, BDSQ_ChannelSettings*);
   BERR_Code (*SetConfig)(BDSQ_Handle, uint32_t, uint32_t);
   BERR_Code (*GetConfig)(BDSQ_Handle, uint32_t, uint32_t*);
   BERR_Code (*SetChannelConfig)(BDSQ_ChannelHandle, uint32_t, uint32_t);
   BERR_Code (*GetChannelConfig)(BDSQ_ChannelHandle, uint32_t, uint32_t*);
   BERR_Code (*GetTxEventHandle)(BDSQ_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetRxEventHandle)(BDSQ_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetVsenseEventHandle)(BDSQ_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetActivityEventHandle)(BDSQ_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetIdleEventHandle)(BDSQ_ChannelHandle, BKNI_EventHandle*);
} BDSQ_ApiFunctTable;


/******************************************************************************
Summary:
   Structure containing BDSQ device settings.
Desciption:
   This structure contains BDSQ settings that will be passed into BDSQ_Open().
See Also:
   BDSQ_Open()
******************************************************************************/
typedef struct BDSQ_Settings
{
   BDSQ_ApiFunctTable api; /* API function table for per-chip implementation */
} BDSQ_Settings;


/******************************************************************************
Summary:
   Initialize the BDSQ PI.
Description:
   This function returns a BDSQ device handle.
Returns:
   BERR_Code : BERR_SUCCESS = the returned BDSQ_Handle is valid
******************************************************************************/
BERR_Code BDSQ_Open(
   BDSQ_Handle *h,                /* [out] BDSQ handle */
   BCHP_Handle hChip,             /* [in] chip handle */
   void *pReg,                    /* [in] register or hab handle */
   BINT_Handle hInterrupt,        /* [in] BINT handle */
   const BDSQ_Settings *pSettings /* [in] default settings */
);


/******************************************************************************
Summary:
   Close the BDSQ PI.
Description:
   This function releases all resources allocated by the BDSQ device.  The
   BDSQ_Handle may not be used after this call.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_Close(
   BDSQ_Handle h   /* [in] BDSQ handle */
);


/******************************************************************************
Summary:
   Get the total number of logical sub-devices for the BDSQ device.
Description:
   This function returns the number of satellite DiSEqC channels supported
   by this chip.
Returns:
   BERR_Code : BERR_SUCCESS = totalChannels is valid
******************************************************************************/
BERR_Code BDSQ_GetTotalChannels(
   BDSQ_Handle h,             /* [in] BDSQ handle */
   uint32_t    *totalChannels /* [out] number of satellite downstream channels supported by this chip */
);


/******************************************************************************
Summary:
   Get default channel settings.
Description:
   This function gets the default settings for the BDSQ channel device.  The
   returned settings will normally be modified (if necessary) and passed into
   BDSQ_OpenChannel().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetChannelDefaultSettings(
   BDSQ_Handle h,                   /* [in] BDSQ handle */
   uint32_t    chan,                /* [in] channel index (0 based) */
   BDSQ_ChannelSettings *pSettings  /* [out] default channel settings */
);


/******************************************************************************
Summary:
   Initialize a BDSQ channel device.
Description:
   This function initializes a BDSQ channel device and returns a 
   BDSQ_ChannelHandle.
Returns:
   BERR_Code : BERR_SUCCESS = the returned BDSQ_ChannelHandle is valid
******************************************************************************/
BERR_Code BDSQ_OpenChannel(
   BDSQ_Handle                h,                /* [in] BDSQ handle */
   BDSQ_ChannelHandle         *pChannelHandle,  /* [out] BDSQ channel handle */
   uint32_t                   chan,             /* [in] channel index (0 based) */
   const BDSQ_ChannelSettings *pSettings        /* [in] channel settings */
);


/******************************************************************************
Summary:
   Close the BDSQ channel device.
Description:
   This function frees the channel handle and any memory allocated by 
   BDSQ_OpenChannel().  The channel handle should not be used after this call.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_CloseChannel(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
);


/******************************************************************************
Summary:
   Returns the BDSQ_Handle associated with the BDSQ_ChannelHandle.
Description:
   This function returns the BDSQ device handle that is associated with the 
   given channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetDevice(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BDSQ_Handle        *pDev   /* [out] BDSQhandle */
);


/******************************************************************************
Summary:
   Returns DSQ PI version information.
Description:
   This function returns DSQ PI version.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetVersionInfo(
   BDSQ_Handle h,             /* [in] BDSQ handle */
   BFEC_VersionInfo *pVersion /* [out] version information */
);


/******************************************************************************
Summary:
   Initializes the BDSQ PI and DiSEqC blocks.
Description:
   This function resets all functional blocks.  This function should be called
   after all the BDSQ handles have been opened.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_Reset(
   BDSQ_Handle h    /* [in] BDSQ handle */
);


/******************************************************************************
Summary:
   Initializes the specified DiSEqC channel.
Description:
   This function resets the specified DiSEqC channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_ResetChannel(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
);


/******************************************************************************
Summary:
   Power down the satellite DiSEqC channel.
Description:
   This function turns off the DiSEqC cores associated with the specified channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_PowerDownChannel(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
);


/******************************************************************************
Summary:
   Power up the satellite DiSEqC channel.
Description:
   This function is used to power up the specified channel after it was
   previously powered down with BDSQ_PowerDownChannel().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_PowerUpChannel(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
);


/******************************************************************************
Summary:
   Return power up state of the channel.
Description:
   This function is used determine if the channel is powered up.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_IsChannelOn(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool               *pOn /* [out] true=channel is currently powered up */
);


/******************************************************************************
Summary:
   Power down the vsense block associated with channel.
Description:
   This function turns off the vsense hardware associated with the specified channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_PowerDownVsense(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
);


/******************************************************************************
Summary:
   Power up the vsense block associated with channel.
Description:
   This function is used to power up the vsense channel after it was
   previously powered down with BDSQ_PowerDownChannel().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_PowerUpVsense(
   BDSQ_ChannelHandle h  /* [in] BDSQ channel handle */
);


/******************************************************************************
Summary:
   Return power up state of the vsense block.
Description:
   This function is used determine if the vsense channel is powered up.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_IsVsenseOn(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool               *pOn /* [out] true=channel is currently powered up */
);


/******************************************************************************
Summary:
   Enables the DiSEqC 22kHz continuous tone.
Description:
   This function turns on/off the DiSEqC 22kHz continuous tone.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_SetTone(
   BDSQ_ChannelHandle h,  /* [in] BDSQ channel handle */
   bool bEnable           /* [in] true = tone on, false = tone off */
);


/******************************************************************************
Summary:
   Detects if 22kHz tone present.
Description:
   This function determines if a 22kHz tone is present. 
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetTone(
   BDSQ_ChannelHandle h,  /* [in] BDSQ channel handle */
   bool *pbTone           /* [out] true = tone is present, false = tone absent */
);


/******************************************************************************
Summary:
   Sets the DiSEqC voltage level.
Description:
   This function sets the DiSEqC voltage level to VBOT (13V) or VTOP (18V).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_SetVoltage(
   BDSQ_ChannelHandle h, /* [in] BDSQ channel handle */
   bool bVtop            /* [in] true = VTOP, false = VBOT */
);


/******************************************************************************
Summary:
   Gets the DiSEqC voltage level.
Description:
   This function determines the DiSEqC voltage level.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetVoltage(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   uint16_t *pVoltage      /* [out] voltage estimation */
);


/******************************************************************************
Summary:
   Enables or disables the Vsense interrupt.
Description:
   This function enables or disables the Vsense interrupt.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_EnableVsenseInterrupt(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
);


/******************************************************************************
Summary:
   Enables or disables the DiSEqC activity detected interrupt.
Description:
   This function enables or disables the activity detected interrupt.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_EnableActivityInterrupt(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
);


/******************************************************************************
Summary:
   Enables or disables the DiSEqC idle timeout interrupt.
Description:
   This function enables or disables the idle timeout interrupt.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_EnableIdleInterrupt(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] true = enable, false = disable */
);


/******************************************************************************
Summary:
   Initiates a DiSEqC transaction.  Must disable Rx before Tx.
Description:
   This function transmits sendBufLen bytes from a buffer pointed to by 
   pSendBuf.  The transaction status is stored in the PI and can be retrieved 
   by BDSQ_GetStatus() after the transaction has completed.  Any reply bytes
   are read using BDSQ_Read().  BDSQ_GetTransactionDoneEventHandle() is used to
   get the handle to the Diseqc event, which signals when the transaction has 
   completed.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_Write(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   const uint8_t *pBuf,    /* [in] contains data to transmit */
   uint8_t n               /* [in] number of bytes to transmit */
);


/******************************************************************************
Summary:
   Enables Rx to listen for an incoming DiSEqC message.  Must disable before Tx.
   Diseqc event is set if message received successfully.
Description:
   This function enables the receiver to listen for an incoming DiSEqC message.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_EnableRx(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   bool bEnable            /* [in] enable/disable Rx */
);


/******************************************************************************
Summary:
   Toggles power state of external DiSEqC LNB
Description:
   This function toggles a control signal which controls the power state of an external LNB.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_EnableLnb(
   BDSQ_ChannelHandle h,  /* [in] BDSQ channel handle */
   bool bEnable           /* [in] enable/disable LNB */
);


/******************************************************************************
Summary:
   Return DiSEqC message received.
Description:
   This function returns the bytes that have been received.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_Read(
   BDSQ_ChannelHandle h,  /* [in] BDSQ handle */
   uint8_t     *pBuf,     /* [out] message data */
   uint8_t     nBufMax,   /* [in] maximum number of bytes that can be written into pBuf */
   uint8_t     *n,        /* [out] number of bytes received */
   uint8_t     *nNotRead  /* [out] number of bytes remaining in the rx fifo */
);


/******************************************************************************
Summary:
   Sends an Auto Control Word.
Description:
   This function transmits an Auto Control Word.
Returns:
   BERR_Code
******************************************************************************/
#define BDSQ_SendACW BDSQ_SendAcw
BERR_Code BDSQ_SendAcw(
   BDSQ_ChannelHandle h,   /* [in] BDSQ channel handle */
   uint8_t            acw  /* [in] auto control word to send */
);


/******************************************************************************
Summary:
   Gets the status of the most recent Diseqc transaction initiated by BDSQ_Write().
Description:
   Any reply bytes are returned in pRcvBuf and the number of reply bytes are 
   indicated in replyLen.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetStatus(
   BDSQ_ChannelHandle h,         /* [in] BDSQ channel handle */
   BDSQ_Status        *pStatus   /* [out] status of most recent transaction */
);


/******************************************************************************
Summary:
   Returns whether DiSEqC activity was detected.
Description:
   Returns if 22kHz activity detected since last call to BDSQ_ResetActivityStatus().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetActivityStatus(
   BDSQ_ChannelHandle  h,                 /* [in] BDSQ channel handle */
   BDSQ_ActivityStatus *pActivityStatus   /* [out] status of bus activity */
);


/******************************************************************************
Summary:
   Resets 22kHz activity status.
Description:
   Resets 22kHz activity status.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_ResetActivityStatus(
   BDSQ_ChannelHandle h    /* [in] BDSQ channel handle */
);


/******************************************************************************
Summary:
   Configure the DiSEqC channel settings.
Description:
   Configure the DiSEqC channel settings.
Returns:
   BERR_Code
******************************************************************************/   
BERR_Code BDSQ_SetChannelSettings(
   BDSQ_ChannelHandle   h,          /* [in] BDSQ channel handle */
   BDSQ_ChannelSettings *pSettings  /* [in] DiSEqC channel settings */
);


/******************************************************************************
Summary:
   Retrieve the DiSEqC channel settings.
Description:
   Retrieve the DiSEqC channel settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetChannelSettings(
   BDSQ_ChannelHandle   h,          /* [in] BDSQ channel handle */
   BDSQ_ChannelSettings *pSettings  /* [in] DiSEqC channel settings */
);


/******************************************************************************
Summary:
   Change the value of a global device configuration parameter.
Description:
   This function is used to change the value of a chip-specific global device
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_SetConfig(
   BDSQ_Handle h,    /* [in] BDSQ device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    val   /* [in] configuration parameter value */
);


/******************************************************************************
Summary:
   Get the current value of a global device configuration parameter.
Description:
   This function is used to read the value of a chip-specific global device
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetConfig(
   BDSQ_Handle h,    /* [in] BDSQ device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    *pVal /* [out] value of the configuration parameter */
);


/******************************************************************************
Summary:
   Change the value of a channel configuration parameter.
Description:
   This function is used to change the value of a chip-specific channel
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_SetChannelConfig(
   BDSQ_ChannelHandle h,    /* [in] BDSQ channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           val   /* [in] configuration parameter value */
);


/******************************************************************************
Summary:
   Get the current value of a channel configuration parameter.
Description:
   This function is used to read the value of a chip-specific channel
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetChannelConfig(
   BDSQ_ChannelHandle h,    /* [in] BDSQ channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           *pVal /* [out] value of the configuration parameter */
);


/******************************************************************************
Summary:
   Returns the DiSEqC transmit done event handle.
Description:
   The application is notified by the DiSEqC transmit done event when the
   transaction initiated by BDSQ_Write() has completed.  This function returns
   the handle to this event.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetTxEventHandle(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BKNI_EventHandle   *hEvent /* [out] DiSEqC Tx event handle */
);


/******************************************************************************
Summary:
   Returns the DiSEqC receive done event handle.
Description:
   The application is notified by the DiSEqC receive done event when there is a
   message received as a reply to a transmission or from BDSQ_EnableRx().
   This function returns the handle to this event.  When the DiSEqC receive done
   event is set, the application should call BDSQ_GetStatus() to get the
   transaction status and/or BDSQ_Read() to retrieve any received messages.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetRxEventHandle(
   BDSQ_ChannelHandle h,      /* [in] BDSQ channel handle */
   BKNI_EventHandle   *hEvent /* [out] DiSEqC Rx event handle */
);


/******************************************************************************
Summary:
   Returns the DiSEqC vsense event handle.
Description:
   The application is notified by the DiSEqC vsense handle when an over-voltage
   or under-voltage condition occurs.  An over-voltage condition exists when 
   the voltage exceeds the upper voltage threshold, and similarly the under-
   voltage condition is when the voltage falls below the lower threshold.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetVsenseEventHandle(
   BDSQ_ChannelHandle h,            /* [in] BDSQ channel handle */
   BKNI_EventHandle *hVsenseEvent   /* [out] DiSEqC vsense event handle */
);



/******************************************************************************
Summary:
   Returns the DiSEqC activity event handle.
Description:
   The application is notified by the DiSEqC activity handle when any DiSEqC
   bus activity is detected.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetActivityEventHandle(
   BDSQ_ChannelHandle h,            /* [in] BDSQ channel handle */
   BKNI_EventHandle *hActivityEvent /* [out] DiSEqC activity event handle */
);


/******************************************************************************
Summary:
   Returns the DiSEqC idle event handle.
Description:
   The application is notified by the DiSEqC idle handle when a DiSEqC
   idle timeout has occurred.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BDSQ_GetIdleEventHandle(
   BDSQ_ChannelHandle h,         /* [in] BDSQ channel handle */
   BKNI_EventHandle *hIdleEvent  /* [out] DiSEqC idle event handle */
);


#ifdef __cplusplus
}
#endif

#endif /* _BDSQ_H_ */

