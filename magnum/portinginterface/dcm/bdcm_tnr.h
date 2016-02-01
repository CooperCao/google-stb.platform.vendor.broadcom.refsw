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


#ifndef BDCM_TNR_H__
#define BDCM_TNR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "brpc_docsis.h"

#define	BDCM_TNR_IFFREQ			(44000000)		/* 44.00 MHz */

/***************************************************************************
Summary:
    A handle for Tuner module
****************************************************************************/
typedef struct BDCM_Tnr                *BDCM_TnrHandle;


/***************************************************************************
Summary:
    Enumeration for Tuner mode
****************************************************************************/
typedef enum BDCM_TnrMode
{
    BDCM_TnrMode_eDigital,
    BDCM_TnrMode_eAnalog,
    BDCM_TnrMode_eDocsis,   
    BDCM_TnrMode_eLast
} BDCM_TnrMode;

/***************************************************************************
Summary:
    Enumeration for Tuner type
****************************************************************************/
typedef enum BDCM_TnrType
{
    BDCM_TnrType_eAds,
    BDCM_TnrType_eAob,
    BDCM_TnrType_eAus,   
    BDCM_TnrType_eLast
} BDCM_TnrType;

/***************************************************************************
Summary:
    Structure for Tuner Information
****************************************************************************/
typedef struct BDCM_TnrInfo
{
    unsigned int tunerMaker;
    unsigned int tunerId;
    unsigned int tunerMajorVer;
    unsigned int tunerMinorVer;
} BDCM_TnrInfo;
/***************************************************************************
Summary:
    Structure to set tuner into power saver mode.
****************************************************************************/
typedef struct BDCM_TnrPowerSaverSettings
{
    bool enable; /* 1 =  enable power saver, 0 = disable power saver. */
} BDCM_TnrPowerSaverSettings;

/***************************************************************************
Summary:
    Structure to set tuner settings.
****************************************************************************/
typedef struct BDCM_TnrSettings
{
  BDCM_TnrType type;
  int adsTunerNum; /* Applicable only for tuner type = BDCM_TnrType_eQam */
  unsigned long ifFreq;
  uint32_t    minVer; /* minor chip revision number */
} BDCM_TnrSettings;



/***************************************************************************
Summary:
	This function returns default settings of a tuner.
****************************************************************************/
BERR_Code BDCM_Tnr_GetDefaultSettings(
   BDCM_TnrSettings *pSettings);


/***************************************************************************
Summary:
	This function opens a tuner module.
****************************************************************************/
BDCM_TnrHandle BDCM_Tnr_Open(
   void *handle,
   BDCM_TnrSettings *pSettings);

/***************************************************************************
Summary:
	This function closes a tuner module.
****************************************************************************/
void BDCM_Tnr_Close(
   BDCM_TnrHandle hTnr /* [in] Tuner handle */
   );

/***************************************************************************
Summary:
    This function sets the Tuner to the requested frequency.
****************************************************************************/
BERR_Code BDCM_Tnr_SetRfFreq(
    BDCM_TnrHandle hTnr,  /* [in] Tuner handle */
    uint32_t freq,           /* [in] Requested tuner freq., in Hertz */
    BDCM_TnrMode tunerMode /* [in] Requested tuner mode */
    );

/***************************************************************************
Summary:
    This function gets the current Tuner tuned frequency.
****************************************************************************/
BERR_Code BDCM_Tnr_GetRfFreq(
    BDCM_TnrHandle hTnr,   /* [in] Tuner handle */
    uint32_t *freq,           /* [out] Returns tuner freq., in Hertz */
    BDCM_TnrMode *tunerMode /* [out] Returns tuner mode */
    );

/***************************************************************************
Summary:
    This function gets the Tuner Information.

****************************************************************************/
BERR_Code BDCM_Tnr_GetInfo(
    BDCM_TnrHandle hTnr,  /* [in] Tuner handle */
    BDCM_TnrInfo *tnrInfo /* [out] Tuner information */
    );

/***************************************************************************
Summary:
    This function gets the power-saver mode.
****************************************************************************/
BERR_Code BDCM_Tnr_GetPowerSaver(
    BDCM_TnrHandle hTnr,                    /* [in] Tuner handle */
    BDCM_TnrPowerSaverSettings *pwrSettings /* [out] Power saver settings. */
    );

/***************************************************************************
Summary:
    This function sets the power-saver mode.
****************************************************************************/
BERR_Code BDCM_Tnr_SetPowerSaver(
    BDCM_TnrHandle hTnr,              /* [in] Tuner handle */
    BDCM_TnrPowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    );

/***************************************************************************
Summary:
    This function gets the Tuner's AGC register value.
****************************************************************************/
BERR_Code BDCM_Tnr_GetTunerAgcVal(
    BDCM_TnrHandle hTnr, /* [in] Tuner handle */
    uint32_t *agcVal        /* [out] output value */
    );

#ifdef __cplusplus
}
#endif
 
#endif
