/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_TYPES_H_
#define BAPE_TYPES_H_

/***************************************************************************
Summary:
Device Handle
***************************************************************************/
typedef struct BAPE_Device *BAPE_Handle;

/***************************************************************************
Summary:
Mixer Input Representation
***************************************************************************/
typedef struct BAPE_MixerInputObject *BAPE_MixerInput;

/***************************************************************************
Summary:
Output Port Representation
***************************************************************************/
typedef struct BAPE_MixerOutputObject *BAPE_MixerOutput;

/***************************************************************************
Summary:
Input Port Representation
***************************************************************************/
typedef struct BAPE_InputPortObject *BAPE_InputPort;

/***************************************************************************
Summary:
Channel Modes
***************************************************************************/
typedef enum BAPE_ChannelMode
{
    BAPE_ChannelMode_e1_0,
    BAPE_ChannelMode_e1_1,
    BAPE_ChannelMode_e2_0,
    BAPE_ChannelMode_e3_0,
    BAPE_ChannelMode_e2_1,
    BAPE_ChannelMode_e3_1,
    BAPE_ChannelMode_e2_2,
    BAPE_ChannelMode_e3_2,
    BAPE_ChannelMode_e3_4,
    BAPE_ChannelMode_eMax
} BAPE_ChannelMode;

/***************************************************************************
Summary:
Channels
***************************************************************************/
typedef enum BAPE_Channel
{
    BAPE_Channel_eLeft,
    BAPE_Channel_eRight,
    BAPE_Channel_eLeftSurround,
    BAPE_Channel_eRightSurround,
    BAPE_Channel_eCenter,
    BAPE_Channel_eLfe,
    BAPE_Channel_eLeftRear,
    BAPE_Channel_eRightRear,
    BAPE_Channel_eMax
} BAPE_Channel;

/***************************************************************************
Summary:
Channel Pairs
***************************************************************************/
typedef enum BAPE_ChannelPair
{
    BAPE_ChannelPair_eLeftRight,
    BAPE_ChannelPair_eLeftRightSurround,
    BAPE_ChannelPair_eCenterLfe,
    BAPE_ChannelPair_eLeftRightRear,
    BAPE_ChannelPair_eMax
} BAPE_ChannelPair;

/***************************************************************************
Summary:
Stereo Mode
***************************************************************************/
typedef enum BAPE_StereoMode
{
    BAPE_StereoMode_eLeftRight,
    BAPE_StereoMode_eLeftLeft,
    BAPE_StereoMode_eRightRight,
    BAPE_StereoMode_eRightLeft
} BAPE_StereoMode;

/***************************************************************************
Summary:
Dual Mono Modes
***************************************************************************/
typedef enum BAPE_DualMonoMode
{
    BAPE_DualMonoMode_eStereo,
    BAPE_DualMonoMode_eLeft,
    BAPE_DualMonoMode_eRight,
    BAPE_DualMonoMode_eMix,
    BAPE_DualMonoMode_eMax
} BAPE_DualMonoMode;

/***************************************************************************
Summary:
SPDIF Burst Types
***************************************************************************/
typedef enum BAPE_SpdifBurstType
{
    BAPE_SpdifBurstType_eNull,
    BAPE_SpdifBurstType_ePause,
    BAPE_SpdifBurstType_eNone,
    BAPE_SpdifBurstType_eMax
} BAPE_SpdifBurstType;

/***************************************************************************
Summary:
SPDIF Output Formatter Settings
***************************************************************************/
typedef struct BAPE_SpdifChannelStatus
{
    bool separateLeftRight;     /* Use different channel number bits for left and right channels (Bits 20..21) */
    bool professional;          /* Bit 0 */
    bool copyright;             /* Bit 2 */
    uint8_t formatInformation;  /* Bits 3..5 */
    uint8_t categoryCode;       /* Bits 8..15 */
    uint8_t sourceNumber;       /* Bits 16..19 */
    uint8_t clockAccuracy;      /* bits 28..29 */
    uint8_t cgmsA;              /* bits 40..41 */
} BAPE_SpdifChannelStatus;

/***************************************************************************
Summary:
SCLK (Bit Clock) Rate
***************************************************************************/
typedef enum BAPE_SclkRate
{
    BAPE_SclkRate_e64Fs,
    BAPE_SclkRate_e128Fs,
    BAPE_SclkRate_eMax
} BAPE_SclkRate;

/***************************************************************************
Summary:
PLL Indexes
***************************************************************************/
typedef enum BAPE_Pll
{
    BAPE_Pll_e0,
    BAPE_Pll_e1,
    BAPE_Pll_e2,
    BAPE_Pll_eMax
} BAPE_Pll;

/***************************************************************************
Summary:
I2S Data Justification
***************************************************************************/
typedef enum BAPE_I2sJustification
{
    BAPE_I2sJustification_eLsbFirst,    /* LSB is at the LRCK transition */
    BAPE_I2sJustification_eMsbFirst,    /* MSB is at the LRCK transition */
    BAPE_I2sJustification_eMax
} BAPE_I2sJustification;

/***************************************************************************
Summary:
I2S Data Alignment
***************************************************************************/
typedef enum BAPE_I2sDataAlignment
{
    BAPE_I2sDataAlignment_eAligned,     /* Data is aligned to LRCK */
    BAPE_I2sDataAlignment_eDelayed,     /* Data is delayed one SCLK from LRCK */
    BAPE_I2sDataAlignment_eMax
} BAPE_I2sDataAlignment;

/***************************************************************************
Summary:
I2S LRCK Polarity
***************************************************************************/
typedef enum BAPE_I2sLRClockPolarity
{
    BAPE_I2sLRClockPolarity_eLeftHigh,  /* Left is high on LRCK */
    BAPE_I2sLRClockPolarity_eLeftLow,   /* Left is low on LRCK */
    BAPE_I2sLRClockPolarity_eMax
} BAPE_I2sLRClockPolarity;

/***************************************************************************
Summary:
I2S SCLK Polarity
***************************************************************************/
typedef enum BAPE_I2sSclkPolarity
{
    BAPE_I2sSclkPolarity_eRising,   /* Rising edge aligned with SDATA */
    BAPE_I2sSclkPolarity_eFalling,   /* Falling edge aligned with SDATA */
    BAPE_I2sSclkPolarity_eMax
} BAPE_I2sSclkPolarity;

/***************************************************************************
Summary:
Normal volume (0dB)
***************************************************************************/
#define BAPE_VOLUME_NORMAL (0x800000)

/***************************************************************************
Summary:
Minimum volume (mute)
***************************************************************************/
#define BAPE_VOLUME_MIN    (0)

#endif /* #ifndef BAPE_TYPES_H_ */

