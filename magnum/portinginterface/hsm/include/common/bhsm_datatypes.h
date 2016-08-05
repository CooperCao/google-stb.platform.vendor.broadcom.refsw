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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef BHSM_DATATYPES_H__
#define BHSM_DATATYPES_H__

#include "bsp_s_keycommon.h"
#include  "bmem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BHSM_DEBUG_POLLING            (0)           /* to turn off                                */
#define BHSM_SECURE_MEMORY_SIZE       (4)           /* size of secure memory passed in by client. */

/*DEPRECATED ... scheduled for deletion. */
#define BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, errCodeValue, condition )            \
    if( (condition) ) \
    { \
        errCode = BERR_TRACE((errCodeValue));        \
        goto BHSM_P_DONE_LABEL;                            \
    }

/* HSM status codes. Range 0x0000 to 0xFFFF */
#define BHSM_STATUS_FAILED                           BERR_MAKE_CODE(BERR_ICM_ID, 1)  /* Return code for general failure. */
#define BHSM_STATUS_TIME_OUT                         BERR_MAKE_CODE(BERR_ICM_ID, 2)
#define BHSM_STATUS_PARM_LEN_ERR                     BERR_MAKE_CODE(BERR_ICM_ID, 3)
#define BHSM_STATUS_INPUT_PARM_ERR                   BERR_MAKE_CODE(BERR_ICM_ID, 4)
#define BHSM_STATUS_HW_BUSY_ERR                      BERR_MAKE_CODE(BERR_ICM_ID, 5)
#define BHSM_STATUS_VERSION_ERR                      BERR_MAKE_CODE(BERR_ICM_ID, 6)
#define BHSM_NOT_SUPPORTED_ERR                       BERR_MAKE_CODE(BERR_ICM_ID, 7)
#define BHSM_STATUS_OWNER_ID_ERR                     BERR_MAKE_CODE(BERR_ICM_ID, 8)
#define BHSM_STATUS_IRDY_ERR                         BERR_MAKE_CODE(BERR_ICM_ID, 9)      /* error is set after a loop of waiting is over, IRDY is still not ready*/
                                                                                                 /* for a multistep HSM command, like BHSM_ResetKeySlotCtrlBits(), if failed in the middle ( not an atomic processing)  */
#define BHSM_STATUS_FAILED_FIRST                     BERR_MAKE_CODE(BERR_ICM_ID, 10)     /* keyslot still ok to reuse*/
#define BHSM_STATUS_FAILED_REST                      BERR_MAKE_CODE(BERR_ICM_ID, 11)     /* keyslot partially modified, no reuse, suggest to free*/
#define BHSM_STATUS_TESTDATA_DIFF_BENCHDATA          BERR_MAKE_CODE(BERR_ICM_ID, 12)
#define BHSM_STATUS_MEMORY_HEAP_ERR                  BERR_MAKE_CODE(BERR_ICM_ID, 13)
#define BHSM_STATUS_CONTIGUOUS_MEMORY_ERR            BERR_MAKE_CODE(BERR_ICM_ID, 14)
#define BHSM_STATUS_MEMORY_PHYCOVERTING_ERR          BERR_MAKE_CODE(BERR_ICM_ID, 15)
#define BHSM_STATUS_VKL_ASSOCIATION_ERR              BERR_MAKE_CODE(BERR_ICM_ID, 16)
#define BHSM_STATUS_UNCONFIGURED_KEYSLOT_ERR         BERR_MAKE_CODE(BERR_ICM_ID, 17)
#define BHSM_STATUS_VKL_ALLOC_ERR                    BERR_MAKE_CODE(BERR_ICM_ID, 18)
#define BHSM_STATUS_INVALID_CMD                      BERR_MAKE_CODE(BERR_ICM_ID, 19)
#define BHSM_STATUS_BSP_ERROR                        BERR_MAKE_CODE(BERR_ICM_ID, 20)
#define BHSM_STATUS_RESOURCE_ALLOCATION_ERROR        BERR_MAKE_CODE(BERR_ICM_ID, 21)
#define BHSM_STATUS_OTP_PROGRAM_ERROR                BERR_MAKE_CODE(BERR_ICM_ID, 22)
#define BHSM_STATUS_PKE_IN_PROGRESS                  BERR_MAKE_CODE(BERR_ICM_ID, 0xA4)
#define BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED  BERR_MAKE_CODE(BERR_ICM_ID, 1000) /* regification not enabed in OTP.  */
#define BHSM_STATUS_REGION_VERIFICATION_FAILED       BERR_MAKE_CODE(BERR_ICM_ID, 1001) /* region signature verification failed. */
#define BHSM_STATUS_REGION_VERIFICATION_IN_PROGRESS  BERR_MAKE_CODE(BERR_ICM_ID, 1002) /* region verification is in progress. */
#define BHSM_STATUS_REGION_VERIFICATION_NOT_DEFINED  BERR_MAKE_CODE(BERR_ICM_ID, 1003) /* region verification is in progress. */
#define BHSM_STATUS_REGION_ALREADY_CONFIGURED        BERR_MAKE_CODE(BERR_ICM_ID, 1004) /* region is already configured.. */

/* To hide M2M changes from the upper layer Nexus Security */
#if HSM_IS_ASKM_28NM_ZEUS_4_2
#define   BCMD_KeyDestBlockType_eMem2Mem    BCMD_KeyDestBlockType_eReserved5
#elif HSM_IS_ASKM_28NM_ZEUS_4_0
#define   BCMD_KeyDestBlockType_eMem2Mem    BCMD_KeyDestBlockType_eReserved3
#endif

/* Used to enumerate the possible HSM Client contexts.  */
typedef enum BHSM_ClientType_e
{
    BHSM_ClientType_eHost = 0,   /* HSM is running on HOST */
    BHSM_ClientType_eSAGE = 1,   /* HSM is running on SAGE*/
    BHSM_ClientType_eNone,

    BHSM_ClientType_eMax = BHSM_ClientType_eNone
}BHSM_ClientType_e;


/* Used to specify which polarity the reserved (01) SC type should be mapped to. */
typedef enum BHSM_SC01ModeWordMapping_e
{
    BHSM_SC01ModeWordMapping_eClear,    /* manage SC01 as a clear SC */
    BHSM_SC01ModeWordMapping_eOdd,      /* manage SC01 as an odd SC */
    BHSM_SC01ModeWordMapping_eEven      /* manage SC01 as an even SC */

} BHSM_SC01ModeWordMapping_e;


/* AES128 counter mode, counter length enum */
typedef enum BCMD_Aes128_CounterSize_e {
    BCMD_AesCounterSize_e32=0,   /* fixed to b'00, 01, 10, 11*/
    BCMD_AesCounterSize_e64=1,
    BCMD_AesCounterSize_e96=2,
    BCMD_AesCounterSize_e128=3,
    BCMD_AesCounterSize_eInvalid

}BCMD_Aes128_CounterSize_e;


/* ASKM key2 configuration parameter. */
typedef enum BHSM_Key2Select
{
    BHSM_Key2Select_eRealMaskKey2  = 0x00,
    BHSM_Key2Select_eTestMaskKey2  = 0x01,
    BHSM_Key2Select_eFixedMaskKey2 = 0x02

} BHSM_Key2Select_e;


typedef struct
{
    unsigned numKeySlotTypes;
    uint8_t numKeySlot[BCMD_XptSecKeySlot_eTypeMax];
    unsigned numMulti2KeySlots;  /* Zeus3.0-,  1 enable (8 by default), 0 disable.
                                                              Zeus4.0+,  Number of Multi2 KeySlots  */
}BHSM_KeyslotTypes_t;


typedef struct BHSM_FirmwareVersion
{
    struct{
        unsigned major;
        unsigned minor;
        unsigned subMinor;
    } platform, bseck;              /* BSECK and platform versions. Platform should correlate with Zeus version. */

    struct {
        bool      valid;                 /* true if the parameter is available */
        unsigned  value;                 /* parameter value   */
    }bfwEpoch;                           /* The EPOCH of the BSECK Firmware */

    unsigned customerModes;
    bool verified;                  /* Firmware has been verified. */
}BHSM_FirmwareVersion;

typedef struct BHSM_Version
{
    BHSM_FirmwareVersion firmware;  /* Firmware version data*/
    unsigned zeus;                  /* Zeus version. Use BHSM_ZEUS_VERSION_* macros to resolve */
}BHSM_Version;

/* Structure to define capabiliites of HSM. */
typedef struct BHSM_Capabilities
{
    BHSM_Version version;               /* HSM version information */
    BHSM_KeyslotTypes_t keyslotTypes;   /* Keyslot configuration data */
    bool keyslotOwnershipSupported;     /* true if platform supports keyslot ownership */
}BHSM_Capabilities_t;



/***************************************************************************
Summary:
Required default settings structure for Host Secure module.
****************************************************************************/
typedef struct BHSM_Setting
{
    unsigned char     ucMaxChannels;        /* DEPRECATED  The concept of channels has been removed from HSM API */
    unsigned long     uSpecialControl;      /* DEPRECATED */
    unsigned long     ulTimeOutInMilSecs;   /* Timeout waiting for BSP responce. Default 2 seconds */
    BMEM_Heap_Handle  hHeap;                /* newly added for IPTV contiguous memeory support inside HSM*/
    BHSM_ClientType_e clientType;           /* specify client type to be Host or Sage */
    bool              sageEnabled;          /* SAGE is enabled in environment. Currenly only interpreted on HOST side */

    struct{
        unsigned size;          /* Size. Needs to be >= BHSM_SECURE_MEMORY_SIZE */
        void    *p;             /* Pointer to memory */
    }secureMemory;              /* Secure memory. Only required for sage cleint. */

}BHSM_Settings;

/*Enable selection between primary and secondary Pid Channels. */
typedef enum BHSM_PidChannelType {
    BHSM_PidChannelType_ePrimary,       /* The default. */
    BHSM_PidChannelType_eSecondary,     /* A Pid channel to be combined with the Primary. */
    BHSM_PidChannelType_eMax
} BHSM_PidChannelType_e;



/* DEPRECATED. The concept of channels have been deprecated from HSM API. */
typedef enum BHSM_HwModule {
   BHSM_HwModule_eCmdInterface1,
   BHSM_HwModule_eCmdInterface2,
   BHSM_HwModule_eMax
} BHSM_HwModule;


#ifdef __cplusplus
}
#endif

#endif /* BHSM_DATATYPES_H__ */
