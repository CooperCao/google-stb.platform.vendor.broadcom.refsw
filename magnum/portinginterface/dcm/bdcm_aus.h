/***************************************************************************
 *     Copyright (c) 2013-2013, Broadcom Corporation
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

#ifndef BDCM_AUS_H__
#define BDCM_AUS_H__

#include "brpc.h"
#include "brpc_docsis.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
    A handle for DOCSIS upstream channel.
****************************************************************************/
typedef struct BDCM_AusChannel    *BDCM_AusChannelHandle;


/***************************************************************************
Summary:
    Enumeration for operation mode of Qam Upstream.
****************************************************************************/
typedef enum BDCM_AusOperationMode
{
    BDCM_AusOperationMode_eAnnexA = 0,
    BDCM_AusOperationMode_eDvs178,
    BDCM_AusOperationMode_eDocsis,
    BDCM_AusOperationMode_ePod,
    BDCM_AusOperationMode_eTestCw,
    BDCM_AusOperationMode_eTestPn23,
    BDCM_AusOperationMode_ePodAnnexA,
    BDCM_AusOperationMode_ePodDvs178,
    BDCM_AusOperationMode_eLast
} BDCM_AusOperationMode;

/***************************************************************************
Summary:
    The settings for Out-of-Band Upstream module.
****************************************************************************/
typedef struct BDCM_AusSettings
{
    BRPC_DevId devId;		/* generic device ID */
    unsigned long xtalFreq; 				/* Crystal Freqency in Hertz */
} BDCM_AusSettings;


/***************************************************************************
Summary:
    Enumeration for burst bank number of Qam Upstream.
****************************************************************************/
typedef enum BDCM_AusBurstBank
{
    BDCM_AusBurstBank_eNbr0 = 0,
    BDCM_AusBurstBank_eNbr1,
    BDCM_AusBurstBank_eNbr2,
    BDCM_AusBurstBank_eNbr3,
    BDCM_AusBurstBank_eNbr4,
    BDCM_AusBurstBank_eNbr5,
    BDCM_AusBurstBank_eLast
} BDCM_AusBurstBank;

/***************************************************************************
Summary:
    Structure for burst bank of Qam Upstream.
    Please see Docsis 1.1 Radio Frequency Interface Specification
    (SP-RFIv1.1-103-991105) documentation under 'Upstream
    Physical-Layer Burst Attributes' for detail information of burst profile.
****************************************************************************/
typedef struct BDCM_AusBurstProfile
{
    uint8_t modulationType;         /* 1=QPSK, 2=16Qam */
    uint8_t diffEncodingOnOff;      /* 1=On, 2=Off */
    uint16_t preambleLength;        /* 9:0 bits */
    uint16_t preambleValOffset;     /* 8:0 bits */
    uint8_t fecBytes;               /* 0-10 */
    uint8_t fecCodewordInfoBytes;   /* 0, 16-253 */
    uint16_t scramblerSeed;         /* 14:0 bits */
    uint8_t maxBurstSize;           /* TODO: */
    uint8_t guardTimeSize;          /* TODO: */
    uint8_t lastCodewordLength;     /* 1=fixed, 2=shortened */
    uint8_t scramblerOnOff;         /* 1=on, 2=off */
    uint8_t nbrPreambleDiffEncoding;/* Number of Preamble different encoding */
} BDCM_AusBurstProfile;

/***************************************************************************
Summary:
    This structure represents the AUS Status for a Qam Upstream.
****************************************************************************/
typedef struct BDCM_AusStatus
{
    bool isPowerSaverEnabled;                /* Eanble=1, Disable=0 */
    BDCM_AusOperationMode operationMode;	 /* Current Operation mode */
    uint32_t sysXtalFreq;                    /* in Hertz, Sys. Xtal freq */
    unsigned int powerLevel;                 /* Current Power level, in hundredth of dBmV */
    unsigned long rfFreq;                    /* Current RF frequency, in Hertz */
    unsigned long symbolRate;                /* Current symbol rate, in baud */
} BDCM_AusStatus;

/***************************************************************************
Summary:
    Required default settings structure for Qam Upstream module.
****************************************************************************/
#define	BDCM_AUS_XTALFREQ					(27000000)	/* 27.00 MHz */
#define	BDCM_AUS_OPERATIONMODE				(BDCM_AusOperationMode_eDvs178)

/***************************************************************************
Summary:
    This function opens a Qam Upstream module.
****************************************************************************/
BDCM_AusChannelHandle BDCM_Aus_OpenChannel(
    void *handle,
    const BDCM_AusSettings *pDefSettings /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_CloseChannel(
    BDCM_AusChannelHandle hChannel /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the default settings for Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_GetChannelDefaultSettings(
    BDCM_AusSettings *pDefSettings /* [out] Returns default setting */
    );

/***************************************************************************
Summary:
    This function sets operation mode of Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_SetChannelOperationMode(
    BDCM_AusChannelHandle hChannel,     /* [in] Device handle */
    BDCM_AusOperationMode operationMode /* [in] Requested operation mode */
    );

/***************************************************************************
Summary:
    This function gets the current operation mode of Qam Upstream module.
***************************************************************************/
BERR_Code BDCM_Aus_GetChannelOperationMode(
    BDCM_AusChannelHandle hChannel,      /* [in] Device handle */
    BDCM_AusOperationMode *operationMode	/* [out] Current operation mode */
    );

/***************************************************************************
Summary:
    This function sets the symbol rate of Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_SetChannelSymbolRate(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    unsigned long symbolRate          /* [in] Requested symbol rate, in baud */
    );

/***************************************************************************
Summary:
    This function gets the current symbol rate of Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_GetChannelSymbolRate(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    unsigned long *symbolRate          /* [out] Current symbol rate, in baud */
    );

/***************************************************************************
Summary:
    This function sets the Rf frequency of Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_SetChannelRfFreq(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    unsigned long rfFreq               /* [in] Requested RF frequency, in Hertz */
    );

/***************************************************************************
Summary:
    This function gets the current Rf frequency of Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_GetChannelRfFreq(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    unsigned long *rfFreq              /* [out] Current RF frequency, in Hertz */
    );

/***************************************************************************
Summary:
    This function sets power level for Qam upstream to H/W.
****************************************************************************/
BERR_Code BDCM_Aus_SetChannelPowerLevel(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    unsigned int powerLevel            /* [in] Requested power level, in hundredth of dBmV */
    );

/***************************************************************************
Summary:
    This function gets the current power level of Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_GetChannelPowerLevel(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    unsigned int *powerLevel           /* [out] Current power level, in hundredth of dBmV */
    );

/***************************************************************************
Summary:
    This function gets the status of Qam Upstream module.
****************************************************************************/
BERR_Code BDCM_Aus_GetChannelStatus(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    BDCM_AusStatus *pStatus         /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function transmits a single 54 byte StarVue mode data packet.
    The contents of the packet are (header+payload) are set by the calling
****************************************************************************/
BERR_Code BDCM_Aus_TransmitChannelStarvuePkt(
    BDCM_AusChannelHandle hChannel, /* [in] Device handle */
    uint8_t *ucXmitBuffer,             /* [in] Buffer to transmit */
    unsigned int size                  /* [in] Size of Buffer (in bytes) */
    );

/***************************************************************************
Summary:
    This function enables transmitter.
****************************************************************************/
BERR_Code BDCM_Aus_EnableChannelTransmitter(
    BDCM_AusChannelHandle hChannel /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function disables transmitter.
****************************************************************************/
BERR_Code BDCM_Aus_DisableChannelTransmitter(
    BDCM_AusChannelHandle hChannel /* [in] Device handle */
    );

#ifdef __cplusplus
}
#endif

#endif



