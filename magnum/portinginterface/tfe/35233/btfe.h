/***************************************************************************
 *     Copyright (c) 2004-2011, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/*================== Module Overview =====================================
<verbatim>
The BTFE (Broadcom Theater Front End) portinginterface (PI) controls the
DFE core on any ASIC based Broadcom chip, such as BCM35233 and future ones. 
The Magnum TFE is built on top of lower layer SCD SW stack, which controls the
DFE core and an integrated acquisition processor.

The magnum/portinginterface/tfe/btfe.h header file defines a standard TFE 
front-end API for all TFE based Broadcom chips. The implementation of 
the API for each chip is in the magnum/portinginterface/tfe/<chip> directory.  

BTFE is low-medium level interface and is HW dependant on Theater Frontend.

The BTFE PI device is accessed by the handle BTFE_Handle.  There is one 
BTFE_Handle per Broadcom chip.

The BTFE_Handle shall be provided in the following API functions to control
the acquisition state machine, get front end status, and configure the GPIOs
and master I2C controller: 

   BTFE_Open()
   BTFE_Close()
   BTFE_GetDefaultSettings()
   BTFE_GetTotalChannels()
   BTFE_GetChannelDefaultSettings()
   BTFE_OpenChannel()
   BTFE_CloseChannel()
   BTFE_Initialize()
   BTFE_GetVersion()
   BTFE_Acquire()
   BTFE_AbortAcq()
   BTFE_GetStatus()
   BTFE_SetConfig()
   BTFE_GetConfig()
   BTFE_SetGpio()
   BTFE_GetGpio()
   BTFE_WriteMi2c()
   BTFE_ReadMi2c()
   BTFE_GetAudioMaxThresholdEventHandle()
   BTFE_SetAudioMagShift()
   BTFE_GetLockStateChangeEventHandle()

Sample Code
//
// NOTE: This is sample code for a system that contains TFE core.  
//       This code does not do any error checking (for simplicity sake).
//
#include "btfe.h"
#include "btfe_priv.h"

static BTFE_Handle  hTFE;  // handle for TFE core
static BTFE_ChannelHandle *hTFEChan = NULL;
static BCHP_Handle  hCHP;
static BREG_Handle  hReg;

void main(void)
{
   BTFE_Settings     settings;
   BTFE_ModulationFormat format; 
   BTFE_ConfigSetTunerIF tunerIF;

   // do initialization
   // (get BCHP_Handle, BREG_Handle, etc)
   ...

   // BTFE initialization
   BTFE_GetDefaultSettings(&settings);
   // tuner-specific initialization
   settings.AGCScript = <AGC data array for the specific tuner being used>
   BTFE_Open(&hTFE, hChip, hReg, &settings);

   // allocate handles for the BTFE channels
   BTFE_GetTotalChannels(hTFE, &numChannels);
   hTFEChan = (BTFE_ChannelHandle *)BKNI_Malloc(numChannels * sizeof(BTFE_ChannelHandle));

   // open each 35233 channel
   for (i = 0; i < numChannels; i++)
   {
      BTFE_GetChannelDefaultSettings(hTFE, i, &chnSettings);
      BTFE_OpenChannel(hTFE, &hTFEChan[i], i, &chnSettings); 
   }
   
   BTFE_Initialize(hTFE, NULL);
   
   // tune the tuner
   // If tuner is connected to I2C bus controlled by the TFE PI, use BTFE_WriteMi2c() to tune the tuner.
   
   // configure tuner-specific parameters
   tunerIF.bOverrideDefault = true;
   tunerIF.center = ... 
   tunerIF.shift = ...
   BTFE_SetConfig(hTFE, BTFE_ConfigItem_eTunerIF, (void*)&tunerIF);

   // acquire 8TFE
   format = BTFE_ModulationFormat_eVSB;
   BTFE_Acquire(hTFEChan[i], format);
   
   ...

   close_tfe:
    for (i = 0; i < numChannels; i++)
      BTFE_CloseChannel(hTFEChan[i]);
   BTFE_Close(hTFE);
}
</verbatim>
========================================================================*/

#ifndef BTFE_H__
#define BTFE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "bint.h"
#include "berr_ids.h"


/***************************************************************************
Summary:
   BTFE error codes

Description:

See Also:

****************************************************************************/
   
#define BTFE_ERR_SCD_NOT_INITIALIZED          BERR_MAKE_CODE(BERR_TFE_ID, 0) /* This is a BERR_Code to indicate that scd in not initialized. */
#define BTFE_ERR_ARG_OUT_OF_RANGE             BERR_MAKE_CODE(BERR_TFE_ID, 1) /* This is a BERR_Code to indicate that the argument is out of range.  */
#define BTFE_ERR_CHIP_FEATURE_NOT_IMPLEMENTED BERR_MAKE_CODE(BERR_TFE_ID, 2) /* This is a BERR_Code to indicate that the chip feature is not implemented. */
#define BTFE_ERR_CHIP_NOT_AVAILABLE           BERR_MAKE_CODE(BERR_TFE_ID, 3) /* This is a BERR_Code to indicate that the chip is not available. */
#define BTFE_ERR_HANDLE_NOT_OPEN              BERR_MAKE_CODE(BERR_TFE_ID, 4) /* This is a BERR_Code to indicate that the handle is not open. */
#define BTFE_ERR_CHIP_NOT_OPEN                BERR_MAKE_CODE(BERR_TFE_ID, 5) /* This is a BERR_Code to indicate that the chip is not open. */
#define BTFE_ERR_CHIP_ERROR                   BERR_MAKE_CODE(BERR_TFE_ID, 6) /* This is a BERR_Code to indicate chip error. */
#define BTFE_ERR_BAD_FIRMWARE                 BERR_MAKE_CODE(BERR_TFE_ID, 7) /* This is a BERR_Code to indicate bad firmware. */
#define BTFE_ERR_COMMAND_IN_PROGRESS          BERR_MAKE_CODE(BERR_TFE_ID, 8) /* This is a BERR_Code to indicate that the command is still in progress. */
#define BTFE_ERR_SCD_ERROR                    BERR_MAKE_CODE(BERR_TFE_ID, 9) /* This is a BERR_Code to indicate scd function returned an error. */
#define BTFE_ERR_INVALID_PARAM_COMBO          BERR_MAKE_CODE(BERR_TFE_ID, 10) /* This is a BERR_Code to indicate parameter set is invalid */

   
/*================================
 * #defines
 *==============================*/

/* Number of elements in the Power Spectrum Arrary result */
#define TFE_PSDARRAYSIZE        (512)
/* Number of elements in the tap result */
#define TFE_TAPARRAYSIZE        (1280)


/*================================ 
 * enum's
 *==============================*/


/***************************************************************************
Summary:
  This enumeration lists all TFE supported modulation formats.

Description:
  This enumeration lists all TFE supported modulation formats.
  All analog and digital, ATSC and DVB are listed as well as 
  "internal state" related ones, like auto and last.
  Note: Unknown is used with mode auto detection, where it indicates 
  failed demodulation detection on a present signal.

See Also:
  BTFE_Acquire(), BTFE_StatusFAT
***************************************************************************/
typedef enum BTFE_ModulationFormat
{
   BTFE_ModulationFormat_eUnknown  = 0,
   BTFE_ModulationFormat_eLast, /* use last value passed to BTFE_Acquire() */
   BTFE_ModulationFormat_eVSB, 
   BTFE_ModulationFormat_eJ83ABC,
   BTFE_ModulationFormat_eAuto, /* Auto VSB/QAM Mode detection */
   BTFE_ModulationFormat_eNTSC_M,
   BTFE_ModulationFormat_eNTSC_N,
   BTFE_ModulationFormat_eNTSC_J,
   BTFE_ModulationFormat_eNTSC_443,
   BTFE_ModulationFormat_ePAL_I,
   BTFE_ModulationFormat_ePAL_B,
   BTFE_ModulationFormat_ePAL_B1,
   BTFE_ModulationFormat_ePAL_G,
   BTFE_ModulationFormat_ePAL_H,
   BTFE_ModulationFormat_ePAL_D,
   BTFE_ModulationFormat_ePAL_K,
   BTFE_ModulationFormat_ePAL_60,
   BTFE_ModulationFormat_ePAL_M,
   BTFE_ModulationFormat_ePAL_N,
   BTFE_ModulationFormat_ePAL_NC,
   BTFE_ModulationFormat_eSECAM_B,
   BTFE_ModulationFormat_eSECAM_D,
   BTFE_ModulationFormat_eSECAM_G,
   BTFE_ModulationFormat_eSECAM_H,
   BTFE_ModulationFormat_eSECAM_K,
   BTFE_ModulationFormat_eSECAM_K1,
   BTFE_ModulationFormat_eSECAM_L,
   BTFE_ModulationFormat_eSECAM_L1,
   BTFE_ModulationFormat_eUNIFIED_COFDM,
   BTFE_ModulationFormat_eQAM64,
   BTFE_ModulationFormat_eQAM256
} BTFE_ModulationFormat;

#define BTFE_IS_ANALOG_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == BTFE_ModulationFormat_eNTSC_M  ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eNTSC_N  ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eNTSC_J  ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eNTSC_443) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_I   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_B   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_B1  ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_G   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_H   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_D   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_K   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_60  ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_M   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_N   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_NC  ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_B ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_D ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_G ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_H ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_K ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_K1) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_L ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_L1)))

#define BTFE_IS_DIGITAL_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == BTFE_ModulationFormat_eVSB    ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eJ83ABC  ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eAUTO   ) ||   \
     ((mod_format) == BTFE_ModulationFormat_eUNIFIED_COFDM)))

#define BTFE_IS_DIGITAL_CABLE_FAT_MOD_FORMAT(mod_format)  \
     ((mod_format) == BTFE_ModulationFormat_eJ83ABC )

#define BTFE_IS_VSB_FAT_MOD_FORMAT(mod_format)  \
   (((mod_format) == BTFE_ModulationFormat_eVSB))

#define BTFE_IS_QAM_FAT_MOD_FORMAT(mod_format)  \
     ((mod_format) == BTFE_ModulationFormat_eJ83ABC )

#define BTFE_IS_COFDM_FAT_MOD_FORMAT(mod_format)  \
     ((mod_format) == BTFE_ModulationFormat_eUNIFIED_COFDM)))

#define BTFE_IS_PAL_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == BTFE_ModulationFormat_ePAL_I ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_B ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_B1) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_G ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_H ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_D ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_K ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_60) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_M ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_N ) ||   \
     ((mod_format) == BTFE_ModulationFormat_ePAL_NC)))

#define BTFE_IS_SECAM_FAT_MOD_FORMAT(mod_format)  \
   ((((mod_format) == BTFE_ModulationFormat_eSECAM_B) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_D) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_G) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_H) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_K) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_K1) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_L) ||   \
     ((mod_format) == BTFE_ModulationFormat_eSECAM_L1)))
     

/***************************************************************************
Summary:
  This enumeration lists BERT inputs.

Description:
  This enumeration lists BERT inputs.
  Note: BERT is engaged only per request (not running always) and it
  requires BERT equipment (signal generator,..)

See Also:
  BTFE_ConfigBERT
***************************************************************************/
typedef enum BTFE_BERTInput
{
   BTFE_BERTInput_eFAT, /* BERT data is the FAT after all error correction has been completed */
   BTFE_BERTInput_eDeinterleaver, /* FAT Data before any error correction */
   BTFE_BERTInput_eTrellis, /* FAT data after the trellis decoder */
   BTFE_BERTInput_eNone /* No BERT operations */
} BTFE_BERTInput;


/***************************************************************************
Summary:
  This enumeration lists two signal polarities.

Description:
  Selection of output signal polarity.
  Used for clock, data, sync, error and valid,..

See Also:
  BTFE_ConfigFATData, BTFE_ConfigFATAGC, BTFE_StatusFAT
***************************************************************************/
typedef enum BTFE_SignalPolarity
{
   BTFE_SignalPolarity_eInvert, /* signal is inverted */
   BTFE_SignalPolarity_eNoInvert /* signal is normal */
} BTFE_SignalPolarity;


/***************************************************************************
Summary:
  This enumeration lists TFE related signal strengths for the pads.

Description:
  This enumeration lists TFE related signal strengths for the pads.

Note: 
  It is recommended to use Default, so it is not recommended to be changed. 
  (For internal use.)

See Also:
  BTFE_ConfigFATData
***************************************************************************/
typedef enum BTFE_SignalStrength
{
   BTFE_SignalStrength_eDefault,
   BTFE_SignalStrength_e1,
   BTFE_SignalStrength_e2,
   BTFE_SignalStrength_e3,
   BTFE_SignalStrength_e4,
   BTFE_SignalStrength_e5,
   BTFE_SignalStrength_e6,
   BTFE_SignalStrength_e7,
   BTFE_SignalStrength_e8,
   BTFE_SignalStrength_e9,
   BTFE_SignalStrength_e10,
   BTFE_SignalStrength_e11,
   BTFE_SignalStrength_e12,
   BTFE_SignalStrength_e13,
   BTFE_SignalStrength_e14,
   BTFE_SignalStrength_e15
} BTFE_SignalStrength;


/***************************************************************************
Summary:
  This enumeration lists lock statuses.

Description:
  This enumeration lists lock statuses.
  
Note:
  NotReady - means that TFE is "in transition" trying to lock - check later.
  
See Also:
  BTFE_StatusBERT, BTFE_StatusFAT
***************************************************************************/
typedef enum BTFE_LockStatus
{
   BTFE_LockStatus_eUnlocked,
   BTFE_LockStatus_eLocked,
   BTFE_LockStatus_eNotReady
} BTFE_LockStatus;


/***************************************************************************
Summary:
  This enumeration lists burst modes MPEG Smoother configuration

Description:

See Also:
  BTFE_ConfigFATData
***************************************************************************/
typedef enum BTFE_BurstMode
{
   BTFE_BurstMode_eBurstOff, /* Smoother on (only outputs if it receives something) */
   BTFE_BurstMode_eBurstOn, /* Smoother off */
   BTFE_BurstMode_eConstantPacket /* Smoother on, but outputs even on error condition */
} BTFE_BurstMode;


/***************************************************************************
Summary:
  This enumeration lists TFE COFDM-ISDBT time-interleaving length

Description:
  

See Also:
  BTFE_StatusFAT
***************************************************************************/
typedef enum BTFE_CofdmTDI
{
   BTFE_CofdmTDI_eTDI0,
   BTFE_CofdmTDI_eTDI4,
   BTFE_CofdmTDI_eTDI8,
   BTFE_CofdmTDI_eTDI16
} BTFE_CofdmTDI;


/***************************************************************************
Summary:
  This enumeration specifies the COFDM standard used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_Standard
{
   BTFE_Cofdm_Standard_eDvbT, /* DVB-T */
   BTFE_Cofdm_Standard_eIsdbT /* ISDB-T */
} BTFE_Cofdm_Standard;

/***************************************************************************
Summary:
  This enumeration specifies the modeGuard used in COFDM acquisition.  Applies only 
  when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_ModeGuard
{
   BTFE_Cofdm_ModeGuard_eAuto,
   BTFE_Cofdm_ModeGuard_eManual
} BTFE_Cofdm_ModeGuard;

/***************************************************************************
Summary:
  This enumeration specifies the mode used in COFDM acquisition.  Applies only 
  when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_Mode
{
   BTFE_Cofdm_Mode_e2K,
   BTFE_Cofdm_Mode_e4K,
   BTFE_Cofdm_Mode_e8K
} BTFE_Cofdm_Mode;


/***************************************************************************
Summary:
  This enumeration specifies the guard used in COFDM acquisition.  Applies 
  only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_Guard
{
   BTFE_Cofdm_Guard_e1_32,
   BTFE_Cofdm_Guard_e1_16,
   BTFE_Cofdm_Guard_e1_8,
   BTFE_Cofdm_Guard_e1_4
} BTFE_Cofdm_Guard;


/***************************************************************************
Summary:
  This enumeration specifies the CCI mode used in COFDM acquisition.  Applies 
  only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_CciMode
{
   BTFE_Cofdm_CciMode_eAuto,
   BTFE_Cofdm_CciMode_eEnable,
   BTFE_Cofdm_CciMode_eNone
} BTFE_Cofdm_CciMode;


/***************************************************************************
Summary:
  This enumeration specifies the ACI mode used in COFDM acquisition.  Applies 
  only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_AciMode
{
   BTFE_Cofdm_AciMode_eAuto,
   BTFE_Cofdm_AciMode_eEnable,
   BTFE_Cofdm_AciMode_eNone
} BTFE_Cofdm_AciMode;


/***************************************************************************
Summary:
  This enumeration specifies the mobile mode used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_MobileMode
{
   BTFE_Cofdm_MobileMode_eAuto,
   BTFE_Cofdm_MobileMode_eFixed,
   BTFE_Cofdm_MobileMode_Pedestrian,
   BTFE_Cofdm_MobileMode_Mobile
} BTFE_Cofdm_MobileMode;

/***************************************************************************
Summary:
  This enumeration specifies the priority used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_Priority
{
   BTFE_Cofdm_Priority_eLow,
   BTFE_Cofdm_Priority_eHigh
} BTFE_Cofdm_Priority;


/***************************************************************************
Summary:
  This enumeration specifies the carrier range used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_CarrierRange
{
   BTFE_Cofdm_CarrierRange_eNarrow,
   BTFE_Cofdm_CarrierRange_eWide
} BTFE_Cofdm_CarrierRange;


/***************************************************************************
Summary:
  This enumeration specifies the impulse settingg used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_Impulse
{
   BTFE_Cofdm_Impulse_eAuto,
   BTFE_Cofdm_Impulse_eEnable,
   BTFE_Cofdm_Impulse_eNone
} BTFE_Cofdm_Impulse;


/***************************************************************************
Summary:
  This enumeration specifies the TPS mode used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_TpsMode
{
   BTFE_Cofdm_TpsMode_eAuto,
   BTFE_Cofdm_TpsMode_eManual
} BTFE_Cofdm_TpsMode;


/***************************************************************************
Summary:
  This enumeration specifies the code rate used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_CodeRate
{
   BTFE_Cofdm_CodeRate_e1_2,
   BTFE_Cofdm_CodeRate_e2_3,
   BTFE_Cofdm_CodeRate_e3_4,
   BTFE_Cofdm_CodeRate_e5_6,
   BTFE_Cofdm_CodeRate_e7_8
} BTFE_Cofdm_CodeRate;


/***************************************************************************
Summary:
  This enumeration specifies the hierarchy setting used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_Hierarchy
{
   BTFE_Cofdm_Hierarchy_eNone,
   BTFE_Cofdm_Hierarchy_e1,
   BTFE_Cofdm_Hierarchy_e2,
   BTFE_Cofdm_Hierarchy_e4
} BTFE_Cofdm_Hierarchy;


/***************************************************************************
Summary:
  This enumeration specifies the constellation used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_Constellation
{
   BTFE_Cofdm_Constellation_eDqpsk,
   BTFE_Cofdm_Constellation_eQpsk,
   BTFE_Cofdm_Constellation_e16Qam,
   BTFE_Cofdm_Constellation_e64Qam
} BTFE_Cofdm_Constellation;


/***************************************************************************
Summary:
  This enumeration specifies the RS Layer used in COFDM acquisition.  
  Applies only when modulation format is BTFE_ModulationFormat_eUNIFIED_COFDM.

Description:
  

See Also:
  BTFE_ConfigCofdm
***************************************************************************/
typedef enum BTFE_Cofdm_RsLayer
{
   BTFE_Cofdm_RsLayer_eNormal,
   BTFE_Cofdm_RsLayer_eA,  /* Layer A */
   BTFE_Cofdm_RsLayer_eB,  /* Layer B */
   BTFE_Cofdm_RsLayer_eC   /* Layer C */
} BTFE_Cofdm_RsLayer;


/***************************************************************************
Summary:
  This enumeration specifies the type of acquisition to be performed.

Description:  
  This enumeration specifies the type of acquisition to be performed.
  
See Also:
  BTFE_ConfigAcquisition
***************************************************************************/
typedef enum BTFE_AcquireConfig
{
    BTFE_AcquireConfig_eDirectedAcquire, /* directed acquire */
    BTFE_AcquireConfig_eFullAcquire,     /* full acquisition */
    BTFE_AcquireConfig_eSearchScan,       /* search/scan */
    BTFE_AcquireConfig_eSymbolRateVerify,
    BTFE_AcquireConfig_eSymbolRateScan,
    BTFE_AcquireConfig_eConstellationSearch
} BTFE_AcquireConfig;


/***************************************************************************
Summary:
  This enumeration specifies the channel bandwidth.

Description:  
  This enumeration specifies the channel bandwidth.
  
See Also:
  BTFE_ConfigAcquisition
***************************************************************************/
typedef enum BTFE_Bandwidth
{
   BTFE_Bandwidth_eUndefined,
   BTFE_Bandwidth_e1MHZ, /* 1MHz */
   BTFE_Bandwidth_e2MHZ,  /* 2 MHz */
   BTFE_Bandwidth_e3MHZ, /* 3 MHz */
   BTFE_Bandwidth_e4MHZ, /* 4 MHz */
   BTFE_Bandwidth_e5MHZ,  /* 5 MHz */
   BTFE_Bandwidth_e6MHZ, /* 6 MHz */
   BTFE_Bandwidth_e7MHZ, /* 7 MHz */
   BTFE_Bandwidth_e8MHZ, /* 8 MHz */
   BTFE_Bandwidth_e9MHZ, /* 9 MHz */
   BTFE_Bandwidth_e10MHZ  /* 10 MHz */
} BTFE_Bandwidth;


/***************************************************************************
Summary:
  This enumeration specifies the Annex A, B, or C for J83 mode.

Description:  
  This enumeration specifies the Annex A, B, or C for J83 mode.
  
See Also:
  BTFE_ConfigJ83abc
***************************************************************************/
typedef enum BTFE_J83_Mode
{
    BTFE_J83_Mode_eA, /* Annex A */
    BTFE_J83_Mode_eB, /* Annex B */
    BTFE_J83_Mode_eC  /* Annex C */
} BTFE_J83_Mode;


/***************************************************************************
Summary:
  This enumeration specifies the QAM constellation in J83 mode.

Description:  
  This enumeration specifies the QAM constellation in J83 mode.
  
See Also:
  BTFE_ConfigJ83abc
***************************************************************************/
typedef enum BTFE_J83_Constellation
{
   BTFE_J83_Constellation_e16Qam, /* 16-QAM */         
   BTFE_J83_Constellation_e32Qam, /* 32-QAM */
   BTFE_J83_Constellation_e64Qam, /* 64-QAM */
   BTFE_J83_Constellation_e128Qam, /* 128-QAM */
   BTFE_J83_Constellation_e256Qam, /* 256-QAM */
   BTFE_J83_Constellation_eUndefined
} BTFE_J83_Constellation;


/***************************************************************************
Summary:
    enum

Description:
   This enum is used in BTFE_SetConfig()/BTFE_GetConfig() for selecting which 
   TFE configuration data to access

See Also:
   BTFE_SetConfig(), BTFE_GetConfig()
****************************************************************************/
typedef enum BTFE_ConfigItem
{
   BTFE_ConfigItem_eBERT,           /* BERT */
   BTFE_ConfigItem_eGPIO,           /* GPIO settings */
   BTFE_ConfigItem_eFATData,        /* FATData */
   BTFE_ConfigItem_eFATAGC,         /* FATAGC */
   BTFE_ConfigItem_eAGCScript,      /* tuner AGCScript */
   BTFE_ConfigItem_eAcquisition,    /* common acquisition settings */
   BTFE_ConfigItem_eSetTunerIF,     /* SetTunerIF */
   BTFE_ConfigItem_eJ83ABC,         /* J83ABC settings */
   BTFE_ConfigItem_eUNIFIED_COFDM,  /* unified COFDM settings */
   BTFE_ConfigItem_eIsdbtBuffer,    /* ISDB-T memory buffer settings */
   BTFE_ConfigItem_ePad,            /* pad control settings */
   BTFE_ConfigItem_ePowerLevel, /* tuner power level */
   BTFE_ConfigItem_eRfOffset,
   BTFE_ConfigItem_ePowerSaving,
   BTFE_ConfigItem_eMax /* n/a */
} BTFE_ConfigItem;


/***************************************************************************
Summary:
   Specifies the status item to query.

Description:
   This enum is passed into BTFE_GetStatus() to select the type of status to
   query.

See Also:
   BTFE_GetStatus()
****************************************************************************/
typedef enum BTFE_StatusItem
{
   BTFE_StatusItem_eAGCIndicator, /* AGCIndicator */
   BTFE_StatusItem_eTunerAGC, /* TunerAGC */
   BTFE_StatusItem_eBERT, /* BERT */
   BTFE_StatusItem_ePSD, /* PSD */
   BTFE_StatusItem_eEQTaps, /* EQTaps */
   BTFE_StatusItem_eConstellationData, /* ConstellationData */
   BTFE_StatusItem_eFAT, /* FAT */
   BTFE_StatusItem_eMemoryRead, /* MemoryRead */
   BTFE_StatusItem_eJ83ABC, /* J83ABC channel scan status */
   BTFE_StatusItem_eVSB, /* VSB channel scan status */
   BTFE_StatusItem_eDVBT /* DVBT channel scan status */  
   /* extend.. */
} BTFE_StatusItem;


/*================================ 
 * structures 
 *==============================*/

/***********************
 * CONFIGURATION types 
 ***********************/

/***************************************************************************
Summary:
  This structure defines BERT configuration

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig()
  when config item is BTFE_ConfigItem_eBERT.

See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigBERT
{
   uint32_t       headerRemoval; /* if header should be suppressed or should be tested too */
   BTFE_BERTInput inputSelect; /* on which input to run BERT on, if BTFE_BERTInput_eNone then BERT is off */
   bool           bPNInversion; /* PN inversion TRUE is inverted and FALSE is non-inverted */
   bool           bPNSelection; /* TRUE means PN selected, FALSE means PN not selected */
   bool           bONFlag; /* TRUE is on, FALSE is off */
   uint32_t       syncErrorThreshold; /* number of sync errors to be allowed */
   uint32_t       syncAcquireCounter; /* number of retries for synchronization attempt (automatic) */
   uint32_t       syncLossCounter; /* number of sync loss-es */
   uint32_t       windowSize; /* windows size (how many samples in one window) */
} BTFE_ConfigBERT;


/***************************************************************************
Summary:
  This structure defines GPIO configuration

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig()
  when config item is BTFE_ConfigItem_eGPIO.
    
See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigGPIO
{
   uint32_t ownershipMask; /* bit packed mask: 0 = Maintains its assigned function and controlled by the firmware, 1 = Under application control */ 
   uint32_t inputMask; /* bit packed mask: 0 = User controlled output, 1 = User controlled input */  
   uint32_t outputType; /* bit packed mask: 0 = open drain, 1 = drive active high and low */ 
   uint8_t  i2cSpeedSelect;
} BTFE_ConfigGPIO;


/***************************************************************************
Summary:
  This structure defines GPIO Data setup for set/get of GPIO's

Description:
  Sets user controllable GPIO pins on/off.

See Also:
  BTFE_ConfigGPIO
***************************************************************************/
typedef struct BTFE_DataGPIO
{
   uint32_t  gpioData;
} BTFE_DataGPIO;


/***************************************************************************
Summary:
  This structure defines FAT Data configuration

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() when config item is BTFE_ConfigItem_eFATData. 

See Also:
  BTFE_SetConfig(), BTFE_GetConfig() 
***************************************************************************/
typedef struct BTFE_ConfigFATData
{
   BTFE_SignalPolarity dataPolarity; /* Polarity of the data signal */
   BTFE_SignalPolarity errorPolarity; /* Polarity of the Mpeg Packet error indicating signal */
   BTFE_SignalPolarity clockPolarity; /* Polarity of the clock signal */
   BTFE_SignalPolarity syncPolarity; /* Polarity of the MPEG packet sync present signal */
   BTFE_SignalPolarity validPolarity; /* Polarity of the valid data present signal */
   BTFE_BurstMode      burstMode; /* The burst mode */
   bool                bGatedClockEnable;      /* The Clock is gated, and only active during valid MPEG data */
   bool                bParallelOutputEnable;  /* Parallel Mode is selected, otherwise serial */
   bool                bHeaderEnable;          /* BERT mode, enables FAT channel MPEG packet first byte 0x47 */
   bool                bCableCardBypassEnable; /* if Enabled, puts the X210VC in CableCard bypass mode on the Xport A1 */
   bool                bFlipOrder;       /* True - get serial data from bit 0 of MPEG reg, False - get from bit 7 */
   bool                bMpegOutputEnable;/* True - MPEG out enabled, False - disabled */
   BTFE_SignalStrength dataStrength;    /* data  pad line strength */
   BTFE_SignalStrength errorStrength;   /* error pad line strength */
   BTFE_SignalStrength clockStrength;   /* clock pad line strength */
   BTFE_SignalStrength syncStrength;    /* sync  pad line strength */
   BTFE_SignalStrength validStrength;   /* valid pad line strength */
} BTFE_ConfigFATData;


/***************************************************************************
Summary:
  This structure defines board related TFE FAT AGC setup

Description:
  Board/platform related TFE FAT AGC settings. A pointer to this structure is 
  passed into BTFE_SetConfig()/BTFE_GetConfig() when config item is 
  BTFE_ConfigItem_eFATAGC.

See Also:
  Application note for configuring Board/platform related TFE FAT AGC settings, 
  BTFE_SetConfig() and BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigFATAGC
{
   BTFE_SignalPolarity  agcSdm1;          
   BTFE_SignalPolarity  agcSdm2;
   BTFE_SignalPolarity  agcSdmX;
   BTFE_SignalPolarity  agcSdmA;
} BTFE_ConfigFATAGC;


/***************************************************************************
Summary:
  This structure defines custom AGC settings script

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_eAGCScript.  The AGC settings script is
  tuner-specific and must be set up prior to acquisition.

Note:
  DANGER !!!
  - There is no error checking and mistakes can burn the chip !

See Also:
  Application note shall be provided for AGC script
***************************************************************************/
typedef struct BTFE_ConfigAGCScript
{
   int32_t *pData;
} BTFE_ConfigAGCScript;


/***************************************************************************
Summary:
  This structure defines tuner IF parameters.

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_eSetTunerIF.

See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigSetTunerIF
{
   bool       bOverrideDefault; /* if TRUE, then center and shift values will apply */
   uint32_t   center; /* tuner IF frequency */
   int32_t    shift;  /* tuner IF frequency shift */
   bool       bSpectInvertMode; /* tuner default spectrum */
} BTFE_ConfigSetTunerIF;


/***************************************************************************
Summary:
  This structure defines acquisition rules

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_eAcquisition.
  
See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigAcquisition
{
   BTFE_AcquireConfig acqConfig;
   BTFE_Bandwidth bandwidthConfig;
   bool         bSpectrumInversion; /* inverted spectrum related to the turner default (true - yes, false - no)
                                 Notes:
                               . most of the Tuners invert spectrum
                               . broadcaster can invert or not the the spectrum */
   bool         bSpectrumAutoDetect; /* auto spectrum detection */
   uint32_t   agcDelay;
   bool         bCoChannelRejection; /* not used (tuners are responsible now for filtering out..) */
   bool         bAdjChannelRejection; /* not used (tuners are responsible now for filtering out..) */
   bool         bMobileMode; /* not used (for future use) */
   bool         bEnhancedMode; /* not used (for future use) */
   bool         bLowPriority; /* not used (for future use) */
   uint32_t     uIfFrequency;/* frequency intended for demod to tune to (not practically used) */ 
} BTFE_ConfigAcquisition;


/***************************************************************************
Summary:
  This structure defines J83ABC acquisition settings.

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_eJ83ABC.
  
See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigJ83abc
{
    BTFE_J83_Mode            mode;
    BTFE_J83_Constellation   constellation;
    uint32_t                 symbolRate;   
} BTFE_ConfigJ83abc;


/***************************************************************************
Summary:
  This structure defines unified COFDM acquisition settings.

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_eUNIFIED_COFDM.
  
See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigCofdm
{
   /* 0 - 7 */
   BTFE_Cofdm_Standard      ofdmStandard;
   BTFE_Cofdm_CciMode       cci;
   BTFE_Cofdm_AciMode       aci;
   BTFE_Cofdm_MobileMode    mobile;
   BTFE_Cofdm_Priority      priority;
   BTFE_Cofdm_CarrierRange  carrierRange;
   BTFE_Cofdm_Impulse       impulse;
   BTFE_Cofdm_RsLayer       rsLayer;
   /* 8 - 15 */
   BTFE_Cofdm_ModeGuard	modeGuard; /* item 9-10 are applied when modeGuard is manual */
   BTFE_Cofdm_Mode          mode;
   BTFE_Cofdm_Guard         guard;
   BTFE_Cofdm_TpsMode	tps; /* item 12-27 are applied when tpsMode is manual */
   BTFE_Cofdm_CodeRate      codeLP; /* TPS */
   BTFE_Cofdm_CodeRate      codeHP;
   BTFE_Cofdm_Hierarchy     hierarchy;
   BTFE_Cofdm_Constellation modulation; /* QPSK, Qam16, Qam64 */
   /* 16-23 */
   BTFE_Cofdm_Constellation modulationLayerA;
   BTFE_Cofdm_Constellation modulationLayerB;
   BTFE_Cofdm_Constellation modulationLayerC;
   BTFE_Cofdm_CodeRate	codeRateLayerA;
   BTFE_Cofdm_CodeRate	codeRateLayerB;
   BTFE_Cofdm_CodeRate	codeRateLayerC;
   uint8_t			segmentLayerA; /* TMCC, value 0-13 and A+B+C=13 */
   uint8_t			segmentLayerB;
   /* 24 - 28 */
   uint8_t			segmentLayerC;
   uint8_t			timeInterleaveLayerA; /* 0-3*/
   uint8_t			timeInterleaveLayerB;
   uint8_t			timeInterleaveLayerC;
   bool			partialReception; /* true or false */
} BTFE_ConfigCofdm;


/***************************************************************************
Summary:
  This structure defines ISDB-T memory buffer settings.

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_eIsdbtBuffer.
  
See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigIsdbtBuffer
{
    uint32_t      address;  /* physical address */
} BTFE_ConfigIsdbtBuffer;

/***************************************************************************
Summary:
  This structure defines RF offset for DS channel scan

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_eRfOffset
  
See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigRfOffset
{
    int32_t      freqOffset; /* freq offset */
    uint32_t    symbolRate; /* symbol rate */
} BTFE_ConfigRfOffset;


/***************************************************************************
Summary:
  This structure defines pad control settings.

Description:
  A pointer to this structure is passed into BTFE_SetConfig()/BTFE_GetConfig() 
  when config item is BTFE_ConfigItem_ePad.
  
See Also:
  BTFE_SetConfig(), BTFE_GetConfig()
***************************************************************************/
typedef struct BTFE_ConfigPad
{
    bool bAgcEnable;  /* enable AGC pads: true=enabled, false=disabled */
} BTFE_ConfigPad;

/****************** 
 * STATUS types 
 ******************/

/***************************************************************************
Summary:
  AGC Status flags indicators

Description:
  AGC Status flags indicators
  Flags are indicating what status you'd like to get

See Also:
  BTFE_StatusAGCIndicator
***************************************************************************/
#define BTFE_AGCStatus_Sdm1         (1L<<0)
#define BTFE_AGCStatus_Sdm2         (1L<<1)
#define BTFE_AGCStatus_Sdmx         (1L<<2)
#define BTFE_AGCStatus_InternalAGC  (1L<<3)
#define BTFE_AGCStatus_AdcMin       (1L<<4)
#define BTFE_AGCStatus_AdcMax       (1L<<5)
#define BTFE_AGCStatus_AdcPower     (1L<<6)
#define BTFE_AGCStatus_PdetPower    (1L<<7)
#define BTFE_AGCStatus_AnalogPvid   (1L<<29)


/***************************************************************************
Summary:
   This structure contains AGC status information.

Description:
   The user sets bits in flags corresponding to the AGC status items to be 
   queried, then calls BTFE_GetStatus() passing the address to this structure 
   and setting status item to BTFE_StatusItem_eAGCIndicator.  When 
   BTFE_GetStatus() returns, only the status items specified in flags will be
   valid.

See Also:
  BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusAGCIndicator
{
   uint32_t  flags; /* bit packed flags BTFE_AGCStatus_* bits */
   uint32_t  sdm1; /* FAT only */
   uint32_t  sdm2; /* FAT only */
   uint32_t  sdmx; /* FAT only */
   int32_t   adcMin; /* FAT only (legacy only) */
   int32_t   adcMax; /* FAT only (legacy only) */
   uint32_t  adcPower; /* analog modes only */
   uint32_t  pdetPower; /* FAT only */
   int32_t   vidPower; /* FAT modes that use UAGC and analog only */
   uint16_t  vdcLevel; /* For calculation DVGA gain */
} BTFE_StatusAGCIndicator;


/***************************************************************************
Summary:
   This structure specifies the AGC script data.

Description:
   This structure defines AGC script data specific to the tuner being used. 
   The Data pointer passed into BTFE_GetStatus() will point to this data 
   structure when status item is BTFE_StatusItem_eTunerAGC.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusTunerAGC
{
   int32_t*  piAGCData;
} BTFE_StatusTunerAGC;


/***************************************************************************
Summary:
   This structure defines query structure to get BERT status.

Description:
   The Data pointer passed into BTFE_GetStatus() will point to this data 
   structure when status item is BTFE_StatusItem_eBERT.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusBERT
{
   BTFE_LockStatus lockStatus; /* BERT locked or not */
   uint32_t        errorCount; /* delta count since last reading (every read operation resets it) */
} BTFE_StatusBERT;


/***************************************************************************
Summary:
   This structure contains power spectrum data.

Description:
   Data represents spectrum view of the demod input.  The Data pointer passed 
   into BTFE_GetStatus() will point to this data structure when status item is
   BTFE_StatusItem_ePSD.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusPSD
{
   int32_t  powerSpectrumData[TFE_PSDARRAYSIZE];
} BTFE_StatusPSD;


/***************************************************************************
Summary:
   This structure contains data needed for Equalizer taps graph.

Description:
   This structure contains data needed for Equalizer taps graphs.  The Data 
   pointer passed into BTFE_GetStatus() will point to this data structure when
   status item is BTFE_StatusItem_eEQTaps.  This is only for internal use and 
   for graphical representation of equalizer taps.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusEQTaps
{
   int32_t  taps[TFE_TAPARRAYSIZE];
   int32_t  adjustment[TFE_TAPARRAYSIZE];
   int32_t  avgnorm[TFE_TAPARRAYSIZE];
} BTFE_StatusEQTaps;


/***************************************************************************
Summary:
   This structure contains constellation data.

Description:
   This structure defines data for graphical representation of constellation 
   data in 2D.  For VSB8 is 8 vertical lines, for QAM64 is 64 dots where 16 
   dots in each quadrant, etc.  The Data pointer passed into BTFE_GetStatus() 
   will point to this data structure when status item is 
   BTFE_StatusItem_eConstellationData.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusConstellationData
{
   int32_t    constX;
   int32_t    constY;
} BTFE_StatusConstellationData;


/***************************************************************************
Summary:
   FAT status bits for querying FAT status

Description:
   FAT status bits for querying FAT status
   Limitations:
   - do not query all flags set, but instead in groups or individually
   - when FAT is in digital do not query analog and vice versa

See Also:
  BTFE_StatusFAT
***************************************************************************/
#define BTFE_FATStatus_LockStatus          (1L<<0)
#define BTFE_FATStatus_DemodFormat         (1L<<1)
#define BTFE_FATStatus_SpectrumPolarity    (1L<<2)
#define BTFE_FATStatus_EqualizerSNR        (1L<<3)
#define BTFE_FATStatus_TimingOffset        (1L<<4)
#define BTFE_FATStatus_PilotOffset         (1L<<5)
#define BTFE_FATStatus_Errors              (1L<<6)
#define BTFE_FATStatus_CoarseOffset        (1L<<7)
#define BTFE_FATStatus_IagcGain            (1L<<8)
#define BTFE_FATStatus_Dur                 (1L<<9)
#define BTFE_FATStatus_PilotAmplitude      (1L<<10)
#define BTFE_FATStatus_EQCursor            (1L<<11)
#define BTFE_FATStatus_PilotEstimate       (1L<<12)
#define BTFE_FATStatus_AtsmState           (1L<<13)
#define BTFE_FATStatus_Dfs                 (1L<<14)
#define BTFE_FATStatus_QAMInterleaverMode  (1L<<15)
#define BTFE_FATStatus_Acb                 (1L<<16)
#define BTFE_FATStatus_CarrierOffset       (1L<<17)
#define BTFE_FATStatus_AGCSettleTime       (1L<<18)
#define BTFE_FATStatus_COFDMModFormat      (1L<<19)
#define BTFE_FATStatus_COFDMHierarchy      (1L<<20)
#define BTFE_FATStatus_COFDMMode           (1L<<21)
#define BTFE_FATStatus_COFDMGuardInt       (1L<<22)
#define BTFE_FATStatus_COFDM_codeRate      (1L<<23)
#define BTFE_FATStatus_Reserved1           (1L<<24)
#define BTFE_FATStatus_SampleFrequency     (1L<<25)
#define BTFE_FATStatus_TargetIfFrequency   (1L<<26)
#define BTFE_FATStatus_SymbolRate          (1L<<27)
#define BTFE_FATStatus_NormalizedIf        (1L<<28)
#define BTFE_FATStatus_CofdmBer            (1L<<29)
#define BTFE_FATStatus_Sqi                 (1L<<30)
#define BTFE_FATStatus_IfdLockStatus                 (1L<<31)


/***************************************************************************
Summary:
   This structure contains FAT status information.

Description:
   The user sets bits in flags corresponding to the FAT status items to be 
   queried, then calls BTFE_GetStatus() passing the address to this structure 
   and setting status item to BTFE_StatusItem_eFAT.  When BTFE_GetStatus() 
   returns, only the status items specified in flags will be valid.
   Limitations:
      - do not query all flags set, but instead in groups or individually
      - do not query items for which demodulator is not currently in that mode
        for example if demodulator is demodulating VSB, do not query COFDM or
        any item that is not related to VSB in this case.
      - when FAT is in digital do not query analog and vice versa

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusFAT
{
   uint32_t                   flags; /* bit packed BTFE_FATStatus_* bits */
   bool                       bStarted;
   BTFE_LockStatus            lockStatus;
   BTFE_ModulationFormat      demodulationFormat;
   uint32_t                   recommendedTimeoutValue; /* lock wait time */
   BTFE_SignalPolarity        spectrumPolarity;
   uint32_t                   equalizerSNR; /* VSB/QAM/COFDM */
   int32_t                    timingOffset;
   int32_t                    pilotOffset;
   uint32_t                   rSUncorrectableErrorsA; /* VSB/QAM/COFDM */
   uint32_t                   rSUncorrectableErrorsB;
   uint32_t                   rSUncorrectableErrorsC;
   uint32_t                   rSCorrectableErrorsA; /* VSB/QAM */
   uint32_t                   rSCorrectableErrorsB;
   uint32_t                   rSCorrectableErrorsC;
   int32_t                    coarseOffset;
   int32_t                    iAGCGain; /* VSB/QAM */
   int32_t                    dUR; /* VSB/QAM */
   int32_t                    pilotAmplitude; /* VSB/QAM */
   int32_t                    eqCursor; /* VSB/QAM */
   int32_t                    pilotEstimate; /* VSB/QAM */
   int32_t                    aTSMstate; /* VSB/QAM */
   int32_t                    dFSstate; /* VSB/QAM */
   int32_t                    dFSpolarity; /* VSB/QAM */
   int32_t                    qAMinterleaverMode; /* QAM */
   uint32_t                   acbState;
   uint32_t                   acbStatus;
   uint32_t                   acbTimer;
   uint32_t                   acbAcqTime;
   uint32_t                   acbNumReacqs;
   int32_t                    carrierOffset;
   uint32_t                   agcSettleTime; /* VSB/QAM */
   uint32_t                   sampleFrequency;
   uint32_t                   targetIfFrequency;        
   uint32_t                   symbolRate;
   BTFE_Cofdm_Constellation cofdmModFormat; /* COFDM */
   BTFE_Cofdm_Hierarchy        cofdmHierarchy; /* COFDM */
   BTFE_Cofdm_Mode             cofdmMode; /* COFDM */
   BTFE_Cofdm_Guard            cofdmGuardInt; /* COFDM */
   BTFE_Cofdm_CodeRate       cofdmCodeRate; /* COFDM */
   
   BTFE_Cofdm_Constellation cofdmModFormatA; /* ISDBT */
   BTFE_Cofdm_Constellation cofdmModFormatB; /* ISDBT */
   BTFE_Cofdm_Constellation cofdmModFormatC; /* ISDBT */

   BTFE_Cofdm_CodeRate     cofdmCodeRateA;  /* ISDBT */
   BTFE_Cofdm_CodeRate     cofdmCodeRateB;  /* ISDBT */
   BTFE_Cofdm_CodeRate     cofdmCodeRateC;  /* ISDBT */

   BTFE_CofdmTDI              cofdmTdiA;  /* ISDBT */
   BTFE_CofdmTDI              cofdmTdiB;  /* ISDBT */
   BTFE_CofdmTDI              cofdmTdiC;  /* ISDBT */
  
   uint32_t                   cofdmSegA;  /* ISDBT */
   uint32_t                   cofdmSegB;  /* ISDBT */
   uint32_t                   cofdmSegC;  /* ISDBT */

   int32_t                    normalizedIF;
   int32_t                    numRSpacketsA; /* VSB/QAM/COFDM */
   int32_t                    numRSpacketsB;
   int32_t                    numRSpacketsC;   
   uint32_t                   cofdmBerErrCnt; /* COFDM */
   uint32_t                   cofdmBerPktCnt; /* COFDM */
 
   bool                       ews;
   bool                       partialReception;
   uint32_t                  cellId;
   bool                       demodSpectrum;
   uint8_t			    ifdLockStatus; /* IFD  */
} BTFE_StatusFAT;


/***************************************************************************
Summary:
   This structure contains J83ABC status information.

Description:
   The Data pointer passed into BTFE_GetStatus() will point to this data 
   structure when status item is BTFE_StatusItem_eJ83ABC.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusJ83ABC
{
   uint8_t                reacqCounter;
   BTFE_AcquireConfig     acqConfig;
   BTFE_Bandwidth         bandwidthConfig;
   bool                   bSpectralInversion; /* true if inverted */
   BTFE_J83_Mode          mode;
   BTFE_J83_Constellation constellation;
   bool                   bOperationDone;  /* applies in BTFE_AcquireConfig_eSymbolRateVerify and BTFE_AcquireConfig_eSymbolRateScan */
   bool                   bSignalDetected;
   int8_t                 bandEdgePosPower;
   int8_t                 bandEdgeNegPower;
   uint32_t               IFNomRate;
   uint32_t               baudRateDetected; /* detected symbol rate in Hz when acquire config is BTFE_AcquireConfig_eSymbolRateScan */
   uint32_t               rateNomFinal;
} BTFE_StatusJ83ABC;

/***************************************************************************
Summary:
   This structure contains VSB status information.

Description:
   The Data pointer passed into BTFE_GetStatus() will point to this data 
   structure when status item is BTFE_StatusItem_eVSB.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusVSB
{
   uint8_t                operationDone;
   uint8_t                confirmVSB;  
} BTFE_StatusVSB;

/***************************************************************************
Summary:
   This structure contains DVBT status information.

Description:
   The Data pointer passed into BTFE_GetStatus() will point to this data 
   structure when status item is BTFE_StatusItem_eDVBT.

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusDVBT
{
   uint8_t               operationDone;
   uint8_t               scanResult;
   bool			spectraInverted;
   int32_t		carrierOffset;
   int16_t		timingOffset;
} BTFE_StatusDVBT;

/***************************************************************************
Summary:
   This structure defines query for internal demod memory. (debug only)

Description:
   This structure defines query for internal demod memory. (debug only)

See Also:
   BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusMemoryRead
{
   uint32_t  offset;
   uint32_t  size;
   uint8_t*  uiValues;
} BTFE_StatusMemoryRead;


/******************************************************************************
Summary:
   Handle for the tfe device
Description:
   This is an opaque handle for the BTFE device.
See Also:
   BTFE_Open()
******************************************************************************/
typedef struct BTFE_P_Handle *BTFE_Handle;


/******************************************************************************
Summary:
   Handle for an BTFE device channel
Description:
   This is an opaque handle for the BTFE device channel.
See Also:
   BTFE_OpenChannel()
******************************************************************************/
typedef struct BTFE_P_ChannelHandle *BTFE_ChannelHandle;


/***************************************************************************
Summary:
   Structure for TFE settings

Description:
   This structure contains the settings for the BTFE PI.

See Also:
   each BTFE_ConfigItem 
****************************************************************************/
typedef struct BTFE_Settings
{
   BTFE_ConfigGPIO                  gpio; /* _eGPIO */
} BTFE_Settings;


/******************************************************************************
Summary:
   Structure containing BTFE device channel settings.
Description:
   This structure contains BTFE device channel settings.
See Also:
   BTFE_OpenChannel(), BTFE_GetChannelDefaultSettings()
******************************************************************************/
typedef struct BTFE_ChannelSettings
{
   BTFE_ConfigBERT           bert; /* _eBERT */
   BTFE_ConfigFATData        fatData; /* _eFATData */
   BTFE_ConfigFATAGC         fatAGC; /* _eFATAGC */
   BTFE_ConfigAGCScript      agcScript; /* _eAGCScript */
   BTFE_ConfigSetTunerIF     setTunerIF; /* _eSetTunerIF */
   BTFE_ConfigAcquisition    acquisition; /* _eAcquisition */
   BTFE_ConfigJ83abc         j83abc; /* _eAcquisition */
   BTFE_ConfigCofdm		     unifiedCofdm; 
   BTFE_ConfigIsdbtBuffer    isdbtBuffer;
   BTFE_ConfigRfOffset	rfOffset;
} BTFE_ChannelSettings;


/* data types used in external tuner AGC script */
typedef struct
{ 
   char    name[12];   /* name in the spec for this entry */
   uint8_t topCTRL1;   /* value for reg_A (flags) */
   uint8_t topCTRL2;   /* value for reg_B (flags) */
   uint8_t reduction;  /* dB backoff from the MAX for this entry */
   uint8_t reserved;   /* some tuners may need additional switch */
} BTFE_RfTopData;

/* AGC Setup Script Token Values (negate these in scripts) */
#define BTFE_ATS_AGC_MAX                      (99) /* maximum ATS token value */
#define BTFE_NXT_AGC_SETUP_DONE              (100) /* end of agc setup data */
#define BTFE_UAGC_SETUP_DONE               (100) /* end of agc setup data */
#define BTFE_NXT_AGC_SETUP_VSB               (200) /* start of VSB setup */
#define BTFE_NXT_AGC_SETUP_64QAM             (300) /* start of 64Qam setup */
#define BTFE_NXT_AGC_SETUP_256QAM            (400) /* start of 256Qam setup */
#define BTFE_NXT_AGC_SETUP_NTSC              (500) /* start of NTSC setup */
#define BTFE_NXT_AGC_SETUP_NTSC_ADJ          (501) /* Start of NTSC adjacent setup */
#define BTFE_NXT_AGC_SETUP_NTSC_SLOW         (502) /* Start of NTSC slow mode */
#define BTFE_NXT_AGC_SETUP_NTSC_FAST         (503) /* start of ntsc fast mode */
#define BTFE_NXT_AGC_SETUP_PAL               (600) /* start of PAL setup */
#define BTFE_NXT_AGC_SETUP_PAL_ADJ           (601) /* Start of PAL adjacent setup */
#define BTFE_NXT_AGC_SETUP_PAL_SLOW          (602) /* Start of PAL slow mode */
#define BTFE_NXT_AGC_SETUP_PAL_FAST          (603) /* start of PAL fast mode */
#define BTFE_NXT_AGC_SETUP_SECAM             (700) /* start of SECAM setup */
#define BTFE_NXT_AGC_SETUP_SECAM_ADJ         (701) /* Start of SECAM adjacent setup */
#define BTFE_NXT_AGC_SETUP_SECAM_SLOW        (702) /* Start of SECAM slow mode */
#define BTFE_NXT_AGC_SETUP_SECAM_FAST        (703) /* start of SECAM fast mode */
#define BTFE_NXT_AGC_SETUP_COFDM             (800) /* start of COFDM setup */
#define BTFE_NXT_AGC_SETUP_FDC               (900) /* start of fdc channel */
#define BTFE_IF_DEMOD                        (910) /* start of AFE dependent IF Demod inits */
#define BTFE_UAGC_IF_VGA                     (920) /* start of AFE dependent UAGC inits */
#define BTFE_UAGC_DIGITAL_MODULATION         (930) /* start of AFE dependent UAGC inits */
#define BTFE_UAGC_ANALOG_NEGATIVE_MODULATION (940) /* start of AFE dependent UAGC inits */
#define BTFE_UAGC_ANALOG_POSITIVE_MODULATION (950) /* start of AFE dependent UAGC inits */

#define BTFE_DEFAULT_ANALOG_SPECTR_6_MHZ_SHIFT 0 /* 0KHz */
#define BTFE_DEFAULT_ANALOG_SPECTR_8_MHZ_SHIFT 0 /* 0KHz */  


/*================================ 
 * FUNCTIONS
 *==============================*/

/******************************************************************************
Summary:
  This function opens and initializes the TFE block.

Description:
  This function must be called first to get a BTFE_Handle.  This handle is 
  used by all other function calls in the BTFE API.
  This handle eventually be closed by calling BTFE_Close.

Input:
  hChip - The chip handle that application created earlier during chip
         initialization sequence. 
         This handle is used to query chip information, chip revision, 
         and miscellaneous chip info.

  hReg - The register handle that application created earlier during
         chip initialization sequence. 
         This handle is used to access chip registers (TFE registers).

  hInterrupt - The level2 interrupt handle that application created
         earlier chip initialization sequence.  
         This handle is use to install level 2 interrupt callback.

  pDefSettings - The default setting for BTFE to be set in. 
                 This parameter can be NULL. In this case BTFE's default
                 structure will be used. This default structure could be 
                 queried prior to BTFE_Open with BTFE_GetDefaultSettings, 
                 modified and passed to BTFE_Open.

Output:
  hTFE - a reference to a TFE handle. Upon successful open this will
          reference to a fresh new TFE handle (context).  
          If error occurs in BTFE_Open, then *h shall be NULL.

Returns:
   BERR_Code:
      BERR_INVALID_PARAMETER - Invalid function parameters.
      BERR_SUCCESS - Successfully opened TFE.

See Also:
   BTFE_Close, BTFE_GetDefaultSettings.
******************************************************************************/
BERR_Code BTFE_Open(
   BTFE_Handle *hTFE, /* [out] BTFE handle */
   BCHP_Handle hChip, /* [in] chip handle */
   BREG_Handle hReg, /* [in] register handle */
   BINT_Handle hInterrupt, /* [in] interrupt handle */
   const BTFE_Settings *pDefSettings /* [in] default settings */
);


/******************************************************************************
Summary:
   Closes the BTFE API.

Description:
   This function releases all the resources allocated by BTFE API and disables 
   DFE interrupts.

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_Close(
   BTFE_Handle hTFE /* [in] BTFE handle */
);


/******************************************************************************
Summary:
   Gets default BTFE settings.
Description:
   This function gets the default BTFE settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetDefaultSettings(
   BTFE_Settings *pChnDefSettings /* [out] default channel settings */
);


/******************************************************************************
Summary:
   Gets the total number of logical sub-devices for the BTFE device.
Description:
   This function gets the total number of channels supported by the BTFE 
   device, e.g. for BCM4501 devices, totalChannels returns the value 2.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetTotalChannels(
   BTFE_Handle  hTFE,             /* [in] BTFE handle */
   uint32_t     *totalChannels /* [out] number of channels supported */
);


/******************************************************************************
Summary:
   Gets default channel settings.
Description:
   This function gets the default settings for an BTFE device channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetChannelDefaultSettings(
   BTFE_Handle   hTFE,                      /* [in] BTFE handle */
   uint32_t      chnNo,                  /* [in] channel number */
   BTFE_ChannelSettings *pChnDefSettings /* [out] default channel settings */
);


/******************************************************************************
Summary:
   Initializes a BTFE device channel.
Description:
   This function initializes a BTFE device channel and returns a 
   BTFE_ChannelHandle.
Returns:
   BERR_Code : BERR_SUCCESS = the returned BTFE_ChannelHandle is valid
******************************************************************************/
BERR_Code BTFE_OpenChannel(
   BTFE_Handle                 hTFE,              /* [in] BTFE handle */
   BTFE_ChannelHandle         *pChannelHandle, /* [out] BTFE channel handle */
   uint32_t                   chnNo,           /* [in] channel number */
   const BTFE_ChannelSettings *pSettings       /* [in] channel settings */
);


/******************************************************************************
Summary:
   Closes the BTFE device channel.
Description:
   This function frees the channel handle and any resources contained in the
   channel handle.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_CloseChannel(
   BTFE_ChannelHandle hTFEChan  /* [in] BTFE channel handle */
);


/******************************************************************************
Summary:
   Downloads the firmware, runs the integrated microcontroller, and initializes 
   DFE memory/registers.
Description:
   This function downloads and runs the firmware, and initializes DFE memory 
   and registers.  If pImage is NULL, then the default firmware will be used.

Returns:
   BERR_Code : BERR_SUCCESS = The integrated microcontroller is running and
                              successfully initialized
******************************************************************************/
BERR_Code BTFE_Initialize(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   void *pImage /* [in] pointer to AP microcode image */
);


/******************************************************************************
Summary:
   Returns TFE PI version information.
Description:
   This function returns the major version, minor version, firmware CRC, and 
   customer version.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetVersion(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t *majorTFE, /* [out] major version */
   uint32_t *minorTFE, /* [out] minor version */
   uint32_t *majorFW, /* [out] major version */
   uint32_t *minorFW, /* [out] minor version */
   uint32_t *fwCRC, /* [out] firmware CRC */
   uint32_t *customer /* [out] customer version/name */
);


/******************************************************************************
Summary:
   Start FAT channel acquisition. 
Description:
   Tells the BCM35233 to start channel acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_Acquire(
   BTFE_ChannelHandle hTFEChan, /* [in] BVSB handle */
   BTFE_ModulationFormat format /* [in] acquisition parameters */
);


/******************************************************************************
Summary:
   Aborts the current acquisition.
Description:
   This function stops the demodulation process started by the most recent call
   to BTFE_Acquire().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_AbortAcq(
   BTFE_ChannelHandle hTFEChan /* [in] BTFE Handle */
);


/***************************************************************************
Summary:
   Get status information.

Description:
   The user specifies the status item to query and provides a pointer to the 
   data structure specific to the status item.  If BTFE_GetStatus() returns
   BERR_SUCCESS, the data structure is valid and can be used by the 
   application.

Returns:
   BERR_Code
****************************************************************************/
BERR_Code BTFE_GetStatus(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_StatusItem item, /* [in] status item to read */
   void *pData /* [out] status to read */
);


/***************************************************************************
Summary:
   Set TFE configuration.

Description:
   The user specifies the configuration item to change and provides a pointer
   to the configuration data structure specific to the configuration item. 

Returns:
   BERR_Code
****************************************************************************/
BERR_Code BTFE_SetConfig(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_ConfigItem item, /* [in] specifies which config item to set */
   void *pData /* [in] pointer to configuration data structure */
);


/***************************************************************************
Summary:
   Get TFE configuration.

Description:
   The user specifies the configuration item to change and provides a pointer
   to the configuration data structure specific to the configuration item.  If
   BTFE_GetConfig() returns BERR_SUCCESS, the data structure is valid and can be
   used by the application.

Returns:
   BERR_Code
****************************************************************************/
BERR_Code BTFE_GetConfig(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
   BTFE_ConfigItem item, /* [in] specifies which config item to get */
   void *pData /* [out] pointer to configuration data structure */
);


/******************************************************************************
Summary:
   Sets the state of GPIO pin(s).
Description:
   The GPIO pins selected by "mask" are set to the levels given by "state".
   Bit 0 corresponds to GPIO_0 pin, bit 1 corresponds to GPIO_1 pin, etc.  The
   selected GPIO pins should have already been configured as output pins by 
   BTFE_ConfigGPIO struct using BTFE_SetConfig().  If the frontend does not 
   control GPIO pins, then this function returns BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_SetGpio(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t mask, /* [in] selects which GPIO pins to set */
   BTFE_DataGPIO state /* [in] state of GPIO pins */
);


/******************************************************************************
Summary:
   Gets the state of GPIO pin(s).
Description:
   The GPIO pins selected by "mask" are read.  The state of the selected pins
   are returned in "state".  Bit 0 corresponds to GPIO_0 pin, bit 1 corresponds
   to GPIO_1 pin, etc.  The selected GPIO pins should have already been 
   configured as input pins by BTFE_ConfigGPIO struct using BTFE_SetConfig().  
   If the frontend does not control GPIO pins, then this function returns
   BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetGpio(
   BTFE_Handle hTFE, /* [in] BTFE handle */
   uint32_t mask, /* [in] selects which GPIO pins to read */
   BTFE_DataGPIO *pstate /* [out] state of selected GPIO pins */
);


/******************************************************************************
Summary:
   Initiates an I2C write transaction from the front end device's I2C 
   controller.
Description:
   This function programs the front end device's master i2c controller to 
   transmit the data given in buf[].  If there is no master i2c controller on 
   the front end device, this function will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_WriteMi2c(
   BTFE_Handle hTFE, /* [in] BTFE Handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *pbuf, /* [in] specifies the data to transmit */
   uint32_t n /* [in] number of bytes to transmit after the i2c slave address */
);


/******************************************************************************
Summary:
   Initiates an I2C read transaction from the front end device's I2C 
   controller.
Description:
   This function programs the front end device's master i2c controller to 
   transmit the data given in buf[].  If there is no master i2c controller on 
   the front end device, this function will return BERR_NOT_SUPPORTED.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_ReadMi2c(
   BTFE_Handle hTFE, /* [in] BTFE Handle */
   uint8_t slave_addr, /* [in] address of the i2c slave device */
   uint8_t *out_buf, /* [in] specifies the data to transmit before the i2c restart condition */
   uint8_t out_n, /* [in] number of bytes to transmit before the i2c restart condition not including the i2c slave address */
   uint8_t *in_buf, /* [out] holds the data read */
   uint8_t in_n /* [in] number of bytes to read after the i2c restart condition not including the i2c slave address */
);


/******************************************************************************
Summary:
   Returns the handle to the audio signal maximum threshold trigger event.
Description:
   This function returns the handle to the audio signal maximum threshold 
   event.  This event will be set by TFE PI when the upper threshold for the 
   audio signal has been exceeded.
Returns: 
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetAudioMaxThresholdEventHandle(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */   
   BKNI_EventHandle *hEvent /* [out] event handle */
);


/******************************************************************************
Summary:
   Set the audio MagShift Value
Description:
   TFE PI will only write mag_shift if it has changed. 
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_SetAudioMagShift(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFEHandle */
    uint8_t mag_shift /* [in] mag_shift value */
);


/******************************************************************************
Summary:
   Returns the Lock State Change event handle.
Description:
   If the application wants to know when the channel goes in lock or out of 
   lock, it should use this function to get a handle to the Lock State Change 
   event.  This event is set whenever the channel lock status changes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetLockStateChangeEventHandle(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BKNI_EventHandle *hEvent /* [out] lock event handle */
);


/******************************************************************************
Summary:
   Returns the channel scan event handle.
Description:
   This function returns the handle to the scan event.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BTFE_GetScanEventHandle(
   BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
   BKNI_EventHandle *hEvent /* [out] scan event handle */
);

#ifdef __cplusplus
}
#endif

#endif /* BTFE_H__ */

