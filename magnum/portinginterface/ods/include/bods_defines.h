/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
#ifndef BODS_DEFINES_H__
#define BODS_DEFINES_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bmem.h"
#include "btmr.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Error Codes specific to BODS

Description:

See Also:

****************************************************************************/
#define BODS_ERR_NOTAVAIL_STANDARD              BERR_MAKE_CODE(BERR_ODS_ID, 0) /* This is a BERR_Code to indicate that the Standard is not supported by the ODS PI */
#define BODS_ERR_NOTAVAIL_CHN_NO                BERR_MAKE_CODE(BERR_ODS_ID, 1) /* This is a BERR_Code to indicate that the channel number provided is not valid */
#define BODS_ERR_POWER_DOWN                     BERR_MAKE_CODE(BERR_ODS_ID, 2) /* This is a BERR_Code to indicate an ODS api was called while power is not enabled */

/***************************************************************************
Summary:
    The handle for ODS module.

Description:
    An opaque handle for BODS device.

See Also:
    BODS_Open()

****************************************************************************/
typedef struct BODS_P_Handle                *BODS_Handle;

/***************************************************************************
Summary:
    The handle for ODS module.

Description:
    An opaque handle for BODS device channel.

See Also:
    BODS_OpenChannel()

****************************************************************************/
typedef struct BODS_P_ChannelHandle         *BODS_ChannelHandle;

/***************************************************************************
 * Enumerated Type Definitions (General)
 ***************************************************************************/
 
/***************************************************************************
Summary:
    Enumeration for Broadcast Standard

Description:
    This enumeration defines the valid standards suported by the ODS module.

See Also:
    BODS_Acquire(), BODS_GetSelevtiveAsyncStatus()
****************************************************************************/
typedef enum BODS_Standard
{
    BODS_Standard_eDvbt,    /* Dvbt */
    BODS_Standard_eIsdbt,   /* Isdbt */
    BODS_Standard_eDvbt2,   /* Dvbt2 */
    BODS_Standard_eDvbc2,   /* Dvbc2 */
    BODS_Standard_eLast
} BODS_Standard;

/***************************************************************************
Summary:
    Enumeration for Acquisition Mode

Description:
    This enumeration defines the valid acquisition modes. In Manual 
    acquisition mode, the receiver attempts to acquire only once, even if a
    lock isnt obtained.  In Auto-acquisition mode, the receiver 
    continuously reacquires until the receiver is locked.

See Also:
    BODS_Acquire(), BODS_GetSelevtiveAsyncStatus()
****************************************************************************/
typedef enum BODS_AcquisitionMode
{
    BODS_AcquisitionMode_eAuto,     /* Acquisition Mode Auto */
    BODS_AcquisitionMode_eManual,   /* Acquisition Mode Manual */
    BODS_AcquisitionMode_eScan,     /* Acquisition Mode Scan */
    BODS_AcquisitionMode_eLast
} BODS_AcquisitionMode;

/***************************************************************************
Summary:
    Enumeration for Tune Acquire

Description:
    This enumeration defines the different tune/acquire modes supported by
    the OFD module.

See Also:
    BODS_Acquire()
****************************************************************************/
typedef enum BODS_TuneAcquire
{
    BODS_TuneAcquire_eTune,             /* Tune only, FW does not start acquire after tune */
    BODS_TuneAcquire_eAcquireAfterTune, /* Auto Acquire after tune */
    BODS_TuneAcquire_eLast     
} BODS_TuneAcquire;

/***************************************************************************
Summary:
    Enumeration for Callback types

Description:
    This enumeration defines the different Callback types support by the OFD
    module.

See Also:

****************************************************************************/
typedef enum BODS_Callback
{
    BODS_Callback_eLockChange,              /* Callback to notify application of lock change */
    BODS_Callback_eUpdateGain,              /* Callback to notify application to update gain */ 
    BODS_Callback_eNoSignal,                /* Callback to notify application there is no signal */ 
    BODS_Callback_eAsyncStatusReady,        /* Callback to notify application that the status requested is ready */
    BODS_Callback_eEmergencyWarningSystem,  /* Emergency Warning System */
    BODS_Callback_eLast
} BODS_Callback;

/***************************************************************************
Summary:
    Enumeration for Lock Status

Description:
    This enumeration represents the lock status of an ODS channel.

See Also:

****************************************************************************/
typedef enum BODS_LockStatus
{           
    BODS_LockStatus_eUnlocked,  /* Unlocked */
    BODS_LockStatus_eLocked,    /* Locked */ 
    BODS_LockStatus_eNoSignal,  /* No Signal */
    BODS_LockStatus_eLast    
} BODS_LockStatus;

/***************************************************************************
Summary:
    Enumeration for selective status types

Description:
    This enumeration represents the status types for ODS selective status.

See Also:
****************************************************************************/
typedef enum BODS_SelectiveAsyncStatusType
{
    BODS_SelectiveAsyncStatusType_eDvbt,                        /* status type supported for Dvbt */
    BODS_SelectiveAsyncStatusType_eIsdbt,                       /* status type supported for Isdbt */
    BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Pre,     /* Dvbt2 L1 Pre FEC Statistics */
    BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsL1Post,    /* Dvbt2 L1 Post FEC Statistics */    
    BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpA,      /* Dvbt2 PLPA FEC Statistics */
    BODS_SelectiveAsyncStatusType_eDvbt2FecStatisticsPlpB,      /* Dvbt2 PLPB FEC Statistics */
    BODS_SelectiveAsyncStatusType_eDvbt2L1Pre,                  /* Dvbt2 L1 Pre Status */    
    BODS_SelectiveAsyncStatusType_eDvbt2L1PostConfigurable,     /* Dvbt2 L1 Post Configurable Status */   
    BODS_SelectiveAsyncStatusType_eDvbt2L1PostDynamic,          /* Dvbt2 L1 Post Dynamic Status */ 
    BODS_SelectiveAsyncStatusType_eDvbt2L1Plp,                  /* Dvbt2 L1 PLP Status */ 
    BODS_SelectiveAsyncStatusType_eDvbt2Short,                  /* Dvbt2 Short Status */    
    BODS_SelectiveAsyncStatusType_eDvbc2FecStatisticsL1Part2,   /* Dvbc2 L1 Part2 FEC Statistics */     
    BODS_SelectiveAsyncStatusType_eDvbc2FecStatisticsPlpA,      /* Dvbc2 PLPA FEC Statistics */  
    BODS_SelectiveAsyncStatusType_eDvbc2FecStatisticsPlpB,      /* Dvbc2 PLPB FEC Statistics */   
    BODS_SelectiveAsyncStatusType_eDvbc2L1Part2,                /* Dvbc2 L1 Part2 Status */
    BODS_SelectiveAsyncStatusType_eDvbc2L1Dslice,               /* Dvbc2 L1 Dslice Status*/
    BODS_SelectiveAsyncStatusType_eDvbc2L1Notch,                /* Dvbc2 L1 Notch Status*/
    BODS_SelectiveAsyncStatusType_eDvbc2L1Plp,                  /* Dvbc2 L1 PLP Status */
    BODS_SelectiveAsyncStatusType_eDvbc2Short,                  /* Dvbc2 Short Status */     
    BODS_SelectiveAsyncStatusType_eLast
} BODS_SelectiveAsyncStatusType;

/***************************************************************************
Summary:
    Structure for Frontend Capabilities.

Description:
    This structure represents the Frondend Capabilities supported by the ODS
    module.

See Also:

****************************************************************************/
typedef struct BODS_FrontendCapabilities
{
    bool    dvbt;   /* If true, dvbt is supported */
    bool    isdbt;  /* If true, isdbt is supported */
    bool    dvbt2;  /* If true, dvbt2 is supported */
    bool    dvbc2;  /* If true, dvbc2 is supported */
} BODS_FrontendCapabilities;

/***************************************************************************
Summary:
    This structure represents ODS version.

Description:
    This structure represents ODS version information.
See Also:
    BODS_GetVersion()

****************************************************************************/
typedef struct BODS_Version
{
    uint32_t    familyId;  /* Family ID */
    uint32_t    chipId;    /* chip ID */
    uint8_t     majVer;    /* major chip revision number */
    uint8_t     minVer;    /* minor chip revision number */
    uint8_t     majFwVer;  /* FW major version */
    uint8_t     minFwVer;  /* FW minor version */    
} BODS_Version;

/***************************************************************************
Summary:
    ODS Power Saver Settings

Description:
    This structure represents the Power Saver Settings supported by the ODS
    module

See Also:

****************************************************************************/
typedef struct BODS_PowerSaverSettings
{
    uint8_t     reserved;   /* TBD */
} BODS_PowerSaverSettings;

/***************************************************************************
Summary:
    This structure represents the status ready types for selective status.

Description:
    This structure is returned when BODS_GetSelectiveAsyncStatusReadyType() 
    is called. This structure returns the available status types.

See Also:

****************************************************************************/
typedef struct BODS_SelectiveAsyncStatusReadyType
{
    uint32_t    dvbt:1;                         /* status ready type for DVB-T Status */                     
    uint32_t    isdbt:1;                        /* status ready type for ISDB-T Status */
    uint32_t    dvbt2FecStatisticsPlpA:1;       /* status ready type for DVB-T2 PLP A FEC Statistics */
    uint32_t    dvbt2FecStatisticsPlpB:1;       /* status ready type for DVB-T2 PLP B FEC Statistics */
    uint32_t    dvbt2FecStatisticsL1Pre:1;      /* status ready type for DVB-T2 L1 Pre FEC Statistics */
    uint32_t    dvbt2FecStatisticsL1Post:1;     /* status ready type for DVB-T2 L1 Post FEC Statistics */
    uint32_t    dvbt2L1Pre:1;                   /* status ready type for DVB-T2 L1 Pre Status */
    uint32_t    dvbt2L1PostConfigurable:1;      /* status ready type for DVB-T2 L1 Post Configurable Status */
    uint32_t    dvbt2L1PostDynamic:1;           /* status ready type for DVB-T2 L1 Post Dynamic Status */
    uint32_t    dvbt2L1Plp:1;                   /* status ready type for DVB-T2 L1 PLP Status */
    uint32_t    dvbt2ShortStatus:1;             /* status ready type for DVB-T2 Short Status */
    uint32_t    dvbc2FecStatisticsPlpA:1;       /* status ready type for DVB-C2 PLP A FEC Statistics */
    uint32_t    dvbc2FecStatisticsPlpB:1;       /* status ready type for DVB-C2 PLP B FEC Statistics */
    uint32_t    dvbc2FecStatisticsL1Part2:1;    /* status ready type for DVB-C2 L1 Part2 FEC Statistics */
    uint32_t    dvbc2L1Part2:1;                 /* status ready type for DVB-C2 L1 Part2 Status */
    uint32_t    dvbc2L1Dslice:1;                /* status ready type for DVB-C2 L1 Dslice Status */
    uint32_t    dvbc2L1Notch:1;                 /* status ready type for DVB-C2 L1 Notch Status */
    uint32_t    dvbc2L1Plp:1;                   /* status ready type for DVB-C2 L1 PLP Status */
    uint32_t    dvbc2ShortStatus:1;             /* status ready type for DVB-C2 Short Status */
}BODS_SelectiveAsyncStatusReadyType;

#ifdef __cplusplus
}
#endif

#endif

