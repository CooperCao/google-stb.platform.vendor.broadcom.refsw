/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: IrBlaster
*    Specific APIs related to IR Blaster Control.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#ifndef NEXUS_IR_BLASTER_H__
#define NEXUS_IR_BLASTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
IR Blaster Handle
***************************************************************************/
typedef struct NEXUS_IrBlaster *NEXUS_IrBlasterHandle;

/***************************************************************************
Summary:
IR Blaster Mode
***************************************************************************/
typedef enum NEXUS_IrBlasterMode
{
    NEXUS_IrBlasterMode_eSony,
    NEXUS_IrBlasterMode_eGI,
    NEXUS_IrBlasterMode_ePioneer,
    NEXUS_IrBlasterMode_ePioneerAAAA,
    NEXUS_IrBlasterMode_eCustom,              /* Customer specific type. See NEXUS_IrBlaster_SetCustomSettings. */
    NEXUS_IrBlasterMode_eXmp2,
    NEXUS_IrBlasterMode_eRC6,
    NEXUS_IrBlasterMode_eMax
} NEXUS_IrBlasterMode;

/***************************************************************************
Summary:
IR Blaster Settings
***************************************************************************/
typedef struct NEXUS_IrBlasterSettings
{
    NEXUS_CallbackDesc transmitComplete;   /* transmit complete callback */
    NEXUS_IrBlasterMode mode;        /* IR Mode */
} NEXUS_IrBlasterSettings;

/***************************************************************************
Summary:
Get default settings for an IR blaster.
***************************************************************************/
void NEXUS_IrBlaster_GetDefaultSettings(
    NEXUS_IrBlasterSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Open an IR blaster.
***************************************************************************/
NEXUS_IrBlasterHandle NEXUS_IrBlaster_Open( /* attr{destructor=NEXUS_IrBlaster_Close} */
    unsigned index,
    const NEXUS_IrBlasterSettings *pSettings  /* attr{null_allowed=y} May be passed as NULL for defaults */
    );

/***************************************************************************
Summary:
Close an IR blaster.
***************************************************************************/
void NEXUS_IrBlaster_Close(
    NEXUS_IrBlasterHandle handle
    );

/***************************************************************************
Summary:
Get settings
***************************************************************************/
void NEXUS_IrBlaster_GetSettings(
    NEXUS_IrBlasterHandle irBlaster,
    NEXUS_IrBlasterSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set settings
***************************************************************************/
NEXUS_Error NEXUS_IrBlaster_SetSettings(
    NEXUS_IrBlasterHandle irBlaster,
    const NEXUS_IrBlasterSettings *pSettings
    );

/***************************************************************************
Summary:
IR Blaster Status
***************************************************************************/
typedef struct NEXUS_IrBlasterStatus
{
    bool busy;      /* true if the device is currently transmitting */
} NEXUS_IrBlasterStatus;

/***************************************************************************
Summary:
This function is used to poll the status of IR blaster.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_GetStatus(
    NEXUS_IrBlasterHandle handle,
    NEXUS_IrBlasterStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
This function sends an IR blaster sequence.

Description:
This function is used to send an IR blast sequence.
The caller should either poll the channel status or wait for the
transmitComplete callback before calling this repeatedly.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_Send(
    NEXUS_IrBlasterHandle handle,
    const uint32_t       *pData,       /* attr{nelem=numBits;nelem_convert=NEXUS_P_IR_BLASTER_PACKET_SIZE;reserved=4} pointer to data to blast */
    uint8_t               numBits,     /* number of bits to blast */
    bool                  headerPulse  /* If true, the header pulse will be sent */
    );

/***************************************************************************
Summary:
This function sends an RC6 IR blaster sequence.

Description:
This function is used to send an RC6 IR blast sequence.
The caller should either poll the channel status or wait for the
transmitComplete callback before calling this repeatedly.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_SendRC6(
    NEXUS_IrBlasterHandle handle,
    const uint32_t *pData,              /* attr{nelem=numBits;nelem_convert=NEXUS_P_IR_BLASTER_PACKET_SIZE;reserved=4} pointer to data to blast */
    uint8_t numBits,                    /* number of bits to blast */
    uint8_t mode,                       /* mode bits (only least significant three bits are used) */
    uint8_t trailer                     /* trailer bit (only least significant bit is used) */
    );

/***************************************************************************
Summary:
This function sends multiple IR blaster packets.

Description:
This function is used to send an IR blast sequence.
The caller should either poll the channel status or wait for the
transmitComplete callback before calling this repeatedly.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_SendABBB(
    NEXUS_IrBlasterHandle handle,
    const uint32_t     *pDataA,             /* attr{nelem=numBitsA;nelem_convert=NEXUS_P_IR_BLASTER_PACKET_SIZE;reserved=4} pointer to data A to blast */
    uint8_t             numBitsA,           /* number of bits in A to blast */
    const uint32_t     *pDataB,             /* attr{nelem=numBitsB;nelem_convert=NEXUS_P_IR_BLASTER_PACKET_SIZE;reserved=4} pointer to data B to blast */
    uint8_t             numBitsB,           /* number of bits in B to blast */
    bool                headerPulseA,       /* flag to send header A pulse */
    bool                headerPulseB,       /* flag to send header B pulse */
    bool                fixedFrameLength    /* flag to indicate fixed frame length */
    );

/***************************************************************************
Summary:
This function sends multiple IR blaster packets.

Description:
This function is used to send an IR blast sequence.
The caller should either poll the channel status or wait for the
transmitComplete callback before calling this repeatedly.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_SendAAAA(
    NEXUS_IrBlasterHandle handle,
    const uint32_t       *pDataA,           /* attr{nelem=numBitsA;nelem_convert=NEXUS_P_IR_BLASTER_PACKET_SIZE;reserved=4} pointer to data A to blast */
    uint8_t               numBitsA,         /* number of bits in A to blast */
    bool                  headerPulse,      /* flag to send header pulse */
    bool                  fixedFrameLength  /* flag to indicate fixed frame length */
    );

/***************************************************************************
Summary:
This function aborts a pending IR sequence.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_Abort(
    NEXUS_IrBlasterHandle handle
    );

/***************************************************************************
Summary:
This function repeats the previous operation.

Description:
This function is used to re-send the previously configured sequence
without having to reconfigure it again.
The caller should either poll the channel status or wait for the
transmitComplete callback before calling this repeatedly.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_Resend(
    NEXUS_IrBlasterHandle handle
    );

/***************************************************************************
Summary:
Send an XMP ACK packet.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_SendXmp2Ack(
    NEXUS_IrBlasterHandle irBlaster
    );

/***************************************************************************
Summary:
Send an XMP NACK packet.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_SendXmp2Nack(
    NEXUS_IrBlasterHandle irBlaster
    );

/***************************************************************************
Summary:
Send an XMP packet.
****************************************************************************/
NEXUS_Error NEXUS_IrBlaster_SendXmp2Bytes(
    NEXUS_IrBlasterHandle irBlaster,
    const uint8_t *pData, /* attr{nelem=numBytes} */
    unsigned numBytes
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_IR_BLASTER_H__ */

