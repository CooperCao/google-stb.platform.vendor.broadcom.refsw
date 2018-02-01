/******************************************************************************
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

 ******************************************************************************/

#ifndef BHSM_PRIVATE_H__
#define BHSM_PRIVATE_H__

#include "bstd.h"
#include "bhsm.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bint.h"
#include "bmem.h"
#include "berr_ids.h"
#include "bhsm_datatypes.h"
#include "bsp_s_keycommon.h"
#include "bsp_s_commands.h"
#include "bsp_s_hw.h"

#define   BHSM_CONTIGUOUS_MEMORY_SIZE       (256+1024*64) /* 64 KBytes, can be adjusted later*/
#define   BHSM_SHA_BUFFER_SIZE              (64*1024)
#define   BHSM_EXTIVKEY_BUFFER_SIZE         (64)          /* 0 - 31 : IV    -  32 - 63 : Key    */
#define   BHSM_NUM_BYPASS_KEYLSOTS          (2)           /* one for G->G transfers, one for GR->R transfers*/

#ifdef __cplusplus
extern "C" {
#endif


/* Definitions */
#define BHSM_MAX_KEYLSOTS                         (186)    /* Absolute mixumum number of Keyslots that can be supported by BSP for Zeus 4.1*/

/* External Keys */
#define BHSM_EXTERNAL_KEYSLOT_KEY_SIZE            (2)      /* 2*64bits */
#define BHSM_EXTERNAL_KEYSLOT_SLOT_SIZE           (8)      /* number of 64bit locations per slot  */
#define BHSM_EXTERNAL_KEYSLOTS_MAX                (512/BHSM_EXTERNAL_KEYSLOT_SLOT_SIZE)
#define BHSM_EXTERNAL_KEYSLOTS_SAGE_MAX           (BHSM_EXTERNAL_KEYSLOTS_MAX/2) /* max Number of external slots that can be managed from SAGE */

#define BHSM_STASH_DWORDSIZE_SIGNATURE            (1)  /* 4 bytes signature */
#define BHSM_STASH_DWORDSIZE_M2M_SLOTS            ((BCMD_MAX_M2M_KEY_SLOT+31)/32)
#define BHSM_STASH_DWORDSIZE_NUM_KEYSLOT_TYPES    (1)
#define BHSM_STASH_DWORDSIZE_KEYSLOT_TYPES        (2)


#define BHSM_SRAM_SIGNATURE                    0xface1003   /* Signature for SRAM block in SAGE global SRAM */

#define BHSM_MAX_VKL                           (BCMD_VKL_KeyRam_eMax)

#define BHSM_HAMCSHA1_CONTEXTSWITCH_NUMBER     3


typedef struct BHSM_P_CAKeySlotTypeInfo {
    unsigned     int        unKeySlotNum;
    unsigned     char       ucKeySlotStartOffset;

} BHSM_P_CAKeySlotTypeInfo_t;

typedef struct BHSM_P_XPTKeySlotAlgorithm
{
    bool            configured;                     /* if this key slot entry was configured */
    uint32_t        ulCAControlWordHi;
    uint32_t        ulCAControlWordLo;
    uint32_t        ulCPDControlWordHi;
    uint32_t        ulCPDControlWordLo;
    uint32_t        ulCPSControlWordHi;
    uint32_t        ulCPSControlWordLo;

    /* index into BHSM_P_Handle::externalKeySlotTable  */
    struct{
        bool valid;
        unsigned slotNum;
    }externalKeySlot;

} BHSM_P_XPTKeySlotAlgorithm_t;

 typedef struct BHSM_P_M2MKeySlotAlgorithm
 {
     bool             configured;                     /* if this key slot entry was configured */
     uint32_t         ulM2MControlWordHi;
     uint32_t         ulM2MControlWordLo;

 } BHSM_P_M2MKeySlotAlgorithm_t;


typedef struct BHSM_P_AskmData
{
    uint32_t        ulSTBOwnerIDSelect;
    uint32_t        ulCAVendorID;
    uint32_t        ulModuleID;
    uint32_t        maskKeySelect;

} BHSM_P_AskmData_t;

typedef struct BHSM_P_CAKeySlotInfo {
    BCMD_XptSecKeySlot_e          keySlotType;
    unsigned                      keySlotNum;
    bool                          bIsUsed;
    BHSM_ClientType_e             client;           /* Owner of this key slot, either hosr MIPS/ARM or SAGE   */
    bool                          bDescrambling;    /* operation -- to be used for Zeus 4.0+, M2M DMA                               */
    unsigned int                  PIDChannel;       /* M2M DMA PID channel -- to be used for Zeus 4.0+,  from 768 - 1023  */
    uint32_t                      ulGlobalControlWordHi;   /* For Zeus 4.1+ */
    uint32_t                      ulGlobalControlWordLo;   /* For Zeus 4.1+ */
    BHSM_P_XPTKeySlotAlgorithm_t  aKeySlotAlgorithm[BCMD_KeyDestEntryType_eMax];  /* Storage for Algorithm control bits for Odd, Even, Clear key */
    BHSM_P_AskmData_t             askmData[BCMD_KeyDestEntryType_eMax];

} BHSM_P_CAKeySlotInfo_t;


 typedef struct BHSM_P_PidChnlToCAKeySlotNum {
     BCMD_XptSecKeySlot_e        keySlotType;   /* ?? May not need this */
    unsigned int                 unKeySlotNum;

 } BHSM_P_PidChnlToCAKeySlotNum_t;


 typedef struct BHSM_P_M2MKeySlotInfo {

    bool                        bIsUsed;
    BHSM_ClientType_e           client;
    /* Storage for Algorithm control bits for Odd, Even, Clear key -- consistent with CA */
    BHSM_P_M2MKeySlotAlgorithm_t  aKeySlotAlgorithm[BCMD_KeyDestEntryType_eMax];
    BHSM_P_AskmData_t         askmData[BCMD_KeyDestEntryType_eMax];

 } BHSM_P_M2MKeySlotInfo_t;

struct BHSM_P_Handle;


/* Representation of BSP external key slot table */
typedef struct {

    bool allocated;             /* external slot is allocated to a ragular keyslot*/
    unsigned int slotPtr;       /* pointer into exteranl key table represented by this slot */

    struct {
        bool valid;
        unsigned offset;
    } key, iv;                  /* external keyslot elements */

}BHSM_ExternalKeySlot_t;



/***************************************************************************
Summary:
Structure that defines Host Secure module handle.

Description:
Structure that defines Host Secure module handle.

See Also:
BHSM_Open()

****************************************************************************/
typedef struct BHSM_P_Handle
{
    BDBG_OBJECT( BHSM_P_Handle )

    BHSM_Settings          currentSettings;   /* current settings */

    BREG_Handle            regHandle;    /* register handle */
    BCHP_Handle            chipHandle;  /* chip handle */
    BINT_Handle            interruptHandle;   /* interrupt handle */

    bool                   bIsOpen;    /* Is Module opened */

    BINT_CallbackHandle    IntCallback;  /* Interrupt Callback */

    /* If multi2 system key slot is allocated */
    unsigned              numMulti2KeySlots;

    unsigned int          numKeySlotTypes;   /* Number of keyslot types */

    /* Number of key slots reserved for each type, and their location within pKeySlot */
    BHSM_P_CAKeySlotTypeInfo_t keySlotTypes[BCMD_XptSecKeySlot_eTypeMax];

    /* number of available key slots. */
    unsigned int             unTotalCAKeySlotNum;

    /* keyslot configuration. */
    BHSM_P_CAKeySlotInfo_t  *pKeySlot[BHSM_MAX_KEYLSOTS];

    /*
        Given pid channel, return CA key slot number.
        Each Pid Channel can have primary pid and secondary pid. Therefore,
        we need 2 key slot per pid channel.
        We can have multiple pid channels associated with a key slot.
        We can also have a pid channel associated with 2 key slots.
    */
    BHSM_P_PidChnlToCAKeySlotNum_t         mapPidChannelToKeySlot[BCMD_TOTAL_PIDCHANNELS][BHSM_PidChannelType_eMax];


   #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
    /* Use this to find out the M2M vacant key slot. */
    BHSM_P_M2MKeySlotInfo_t aunM2MKeySlotInfo[BCMD_MAX_M2M_KEY_SLOT];
   #endif

    bool                    bIsBusyHmacSha1[BHSM_HAMCSHA1_CONTEXTSWITCH_NUMBER];    /*  DEPRECATED.  */
    bool                    bIsBusyPKE;         /* Is PKE engine busy ? */

    BMEM_Heap_Handle       hHeap;

#if  (BHSM_IPTV == 1)
    unsigned char          *pContiguousMem;
#endif

    bool                   keySlotsInitialised;
    bool                   hsmPiRunningFullRom;

    BHSM_FirmwareVersion   firmwareVersion;

    BHSM_ExternalKeySlot_t externalKeySlotTable[BHSM_EXTERNAL_KEYSLOTS_MAX];  /* Representation of BSP external key slot table */



     /* structure representing the available Virtual Key Ladders */
     struct
     {
         bool                    free;          /* the VKL is currenlty not allocated */
         bool                    neverUsed;     /* the VKL has never been used. */
         BCMD_CustomerSubMode_e  custSubMode;   /* the customer mode assocaited with the keyslot */
         BHSM_ClientType_e       client;        /* the cleint type. Persistent when keyslot is not allocated.  */
     } vkl[BHSM_MAX_VKL];

    void * pBspMessageModule;                   /* module data for the BSP Message interface. */

} BHSM_P_Handle;


bool isSecurityInRom( BHSM_P_Handle  *hHsm );

bool isKeySlotOwershipSupported( BHSM_P_Handle  *hHsm );

BERR_Code  loadBspVersion( BHSM_Handle hHsm );              /* load the BFW version from the BFW. */

bool isBfwVersion_GreaterOrEqual( BHSM_Handle hHsm, unsigned major, unsigned minor, unsigned subMinor ); /* Returns true if current BFW is greater than or equal to specified value */

/* Get the pointer to a keyslot for a given keySlotType and keysloto number. */
BERR_Code BHSM_P_GetKeySlot(
        BHSM_P_Handle  *hHsm,
        BHSM_P_CAKeySlotInfo_t **ppKeyslot,
        BCMD_XptSecKeySlot_e keySlotType,
        unsigned keySlotNum
    );


#ifdef __cplusplus
}
#endif

#endif /* BHSM_PRIVATE_H__ */
