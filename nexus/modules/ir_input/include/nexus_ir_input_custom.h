/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* API Description:
*   API name: Keypad
*    Specific APIs related to Keypad Input Control.
*
***************************************************************************/
#ifndef NEXUS_IR_INPUT_CUSTOM_H__
#define NEXUS_IR_INPUT_CUSTOM_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
IR Input Tolerance specification for custom settings. Currently unused.
***************************************************************************/
typedef enum NEXUS_IrInputTolerance
{
    NEXUS_IrInputTolerance_e12_5,   /* +/- 12.5% */
    NEXUS_IrInputTolerance_e25,     /* +/- 25% */
    NEXUS_IrInputTolerance_e50,     /* +/- 50% */
    NEXUS_IrInputTolerance_eFixed,  /* Use the value in fixedTolerance */
    NEXUS_IrInputTolerance_eMax
} NEXUS_IrInputTolerance;

/***************************************************************************
Summary:
IR Input Value/Tolerance specification for custom settings
***************************************************************************/
typedef struct NEXUS_IrInputValue
{
    unsigned value;    /* value (10 bits) */
    uint8_t tolerance; /* tolerance select code */
} NEXUS_IrInputValue;

/***************************************************************************
Summary:
Custom IR input settings if pre-defined modes are not sufficient
***************************************************************************/
typedef struct NEXUS_IrInputCustomSettings
{
    unsigned countDivisor;      /* Clock divisor for counters that measure the symbol pulses. 
                                   For a divisor of N, set this register to the value N-1. Each 
                                   count unit is equivalent to the time period of (input clock period * N) */
    NEXUS_IrInputValue preambleA[4]; /* Preamble A pulse sequence */
    NEXUS_IrInputValue preambleB[4]; /* Preamble B pulse sequence */
    uint8_t preambleACount;  /* number of entries in preambleA[] */
    uint8_t preambleBCount;  /* number of entries in preambleB[] */
    int measurePreamblePulse;   /* false => even counts specifies pulse off period */
                                /* true => even counts specifies cycle period */
    int preambleBRepeat;        /* if true, pb[] matches a repeat sequence */
    unsigned pulseTolerance;    /* Symbol pulse ON period fixed-value tolerance. 
                                   Used only when Value Tolerance is set to NEXUS_IrInputTolerance_ePulse. 
                                   Set this to the tolerance value to use when measuring and validating the ON 
                                   period in each symbol. Units are in count units (see countDivisor) */
    unsigned t0;        /* The spacing in the symbol for code 0. The spacing for code n is (T0 + n * dT). 
                           See delT. Units are in count units (see countDivisor). When decoding bi-phase transmission, 
                           this parameter is interpreted as the bit period. */
    unsigned delT;      /* (dT) The spacing increment between the symbol for code n and the symbol for code n+1. 
                           See t0. Units are in count units (see count_divisor). Not used for decoding bi-phase 
                           transmission.  */

    int fixSymPulseLast;   /* false => fix-width symbol pulse between edges 0 & 1 */
                           /* true => fix-width symbol pulse between edges 1 & 2 */

    int measureSymPulse;   /* false => measure spacing for complete cycle */
                           /* true => measure spacing between 2 consecutive edges */

    NEXUS_IrInputValue symPulseWidth;   /* Symbol pulse ON period. Set this to the ON period of the symbol pulse 
                                           in count units (see countDivisor). Not used when decoding bi-phase 
                                           transmission*/

    NEXUS_IrInputValue spacingTol;      /* Spacing tolerance (value member is ignored) */

    unsigned nSymA;  /* Number of symbols in frame with Preamble A. Used only when preambleACount is non-zero. 
                        Set this with the number of expected symbols minus 1 */
    unsigned nSymB;  /* Number of symbols in frame with Preamble B. Used only when preambleACount is non-zero. 
                        Set this with the number of expected symbols minus 1 */
    unsigned bitsPerSym;    /* Number of data bits per symbol minus one */
    int mostSignifSymRecvFirst;    /* true => most significant symbol received first */
                                   /* false => least significant symbol received first */
    int leftAdjustRecvData;    /* true => resulting received data is left adjusted using '0' to pad LSBs */
                               /* false => resulting received data is right adjusted using '0' to pad MSBs */
    int biphaseCoded;  /* true => the input signal is bi-phase coded */
                       /* false => the input signal is pulse spacing coded */
    int twoSymPerCycle;  /* two symbols per cycle, 1 or 0 */
    int checkStopSym;    /* check stop symbol, 1 or 0 */
    int variableLenData; /* variable length data, 1 or 0. */

    unsigned timeoutDivisor; /* Clock divisor for time-out/watch-dog timers. For a divisor of N, 
                                set this register to the value N-1. Each timeout unit is equivalent 
                                to the time period of (input clock period * (countDivisor + 1) * N).  */

    unsigned frameTimeout;  /* Frame/packet time-out. The contents of this register is loaded into 
                               the frame timer at the start of a frame. When this timer expires a 
                               fault condition is declared and the current frame is discarded. 
                               Set this to a value corresponding to a period that is slightly longer 
                               than the longest frame. Units are in timeout units (see timeoutDivisor) */

    unsigned edgeTimeout;    /* Pulse edge time-out. This parameter is loaded into the edge timer whenever 
                                a transition is detected in the IR waveform. When this timer expires a fault 
                                condition is declared and the current frame is discarded. Set this to a value 
                                corresponding to a value slightly larger than the longest pulse period (either ON or OFF). 
                                Units are in timeout units (see timeoutDivisor).  */

    unsigned minOffTime; /* Minimum OFF time before becoming ready to received the next frame/packet. This parameter is 
                            loaded into the minimum_off timer whenever the IR signal is ON. After successfully decoding 
                            a frame or after a fault condition, the decoder will become ready to receive the next frame 
                            only after the minumum_off timer expires. This lowers the likelihood that trailing pulses from 
                            the current frame are mistakenly seen as the start of a new frame. Units are in timeout units 
                            (see timeoutDivisor) */

    NEXUS_IrInputValue stop;    /* Stop symbol pulse or cycle period. When either the checkStopSym or variableLenData
                                   parameter is set to true, this parameter is used to validate the stop symbol. 
                                   The measureSymPulse and fixSymPulseLast parameters determine the measured interval. 
                                   Set this parameter to the pulse or cycle period in count units or, under certain 
                                   conditions, timeout units, selected by the stopParamUnit parameter */

    unsigned dataSymTimeout;    /* Data symbol time-out. When checkStopSym parameter is set to true to enable stop 
                                   symbol validation, this parameter is used for stop symbol termination in cases where 
                                   there is no terminating edge from the next frame. This parameter is loaded into the data 
                                   symbol timer at the start of each symbol. The decoder uses the expiration of this timer
                                   as an indication of a the end of the stop symbol. When this parameter, when non-zero, 
                                   overrides the edge and frame time-out when decoding data symbols and has no effect during 
                                   the preamble phase.  Set this parameter to 0 (the default state) when the data symbol 
                                   timer is not used. The stopParamUnit parameter selects the units which can be in count units or timeout units.  */

    unsigned repeatTimeout;     /* Repeated command time-out. This parameter is loaded into the repeat timer 
                                   whenever a valid command is received. The repeat timer can be used to discriminate 
                                   between packets from the same key press and those from a subsequent key press. 
                                   When this parameter is set to a non-zero value, a valid packet received while the 
                                   repeat timer has not expired will assert the repeat flag in the event: if preambleBRepeat
                                   is set to true, detecting preamble B will assert the repeat flag only if preamble B begins 
                                   before the repeat timeout has expired; if preambleBRepeat is false, a valid data packet 
                                   received before the repeat timer expires will assert the repeat flag in addition to the usual 
                                   decoding process. Set this parameter to 0 (the default state) when the repeat timer is not used 
                                   to gate repeat codes or to flag repeated data packets. Units are in timeout units (see timeoutDivisor). */

    int stopParamUnit;      /* stop parameter unit selector: */
                            /*  1: stop has count units */
                            /*  0: stop has timeout units */

    int dataSymClkTickUnit; /* data symbol timer clock tick and */
                            /*  dataSymTimeout units selector: */
                            /*  0: dataSymTimeout has count units */
                            /*  1: dataSymTimeout has timeout units */

    int ignoreDataSymTimerEdge1;   /* ignore data symbol timer expiration */
                                   /* while waiting for Edge 1; 0 or 1. */

    int dataSymTimerExpStatEn; /* enable data symbol time-out */
                               /*   expiration flag to lflag bit in */
                               /*   status register */
    int enHavePreambleAftStop; /* enable havePreambleAftStop parameter */
                               /* for non-biphase decoding, 0 or 1 */
    int havePreambleAftStop;   /* have preamble after stop symbol */
    int restrictiveDecode;     /* restrictive decoding enabled */
    int rc6;                   /* RC6 encoded. Requires biphaseCoded=1 */
    int dontValidateMode;      /* don't validate RC6 mode bits */
    unsigned modeBits;         /* RC6 mode bits (3 bits), typically 6 */
    int dontValidateTrailer;   /* Reserved: don't validate RC6 trailer */
    unsigned rc6Trailer;       /* Reserved: RC6 trailer: false for Mode 6A */
    int dontValidateCustCode;  /* If true, don't validate customer code bits */

    unsigned custCode;         /* RC6 customer code bits (16 bits) */
    unsigned nCustCodeBits;    /* number of RC6 customer code bits: */

    unsigned passModeCustCodePass;  /* RC6 mode bits and customer code pass-through control.
                                       0: Exclude mode bits and customer code from
                                          received data. The nccb field determines
                                          the size of the customer code.
                                       1: Exclude mode bits from the received data, but,
                                          include customer code.
                                       2: Not allowed.
                                       3: Include both mode bits and customer code in 
                                          the received data. */
} NEXUS_IrInputCustomSettings;

/***************************************************************************
Summary:
Get default custom IR input settings 
***************************************************************************/
void NEXUS_IrInput_GetDefaultCustomSettings(
    NEXUS_IrInputCustomSettings *pSettings /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif
