/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

/***************************************************************************************
Interface description.

The NEXUS Keyslot API enables the allocation and configuration of a keyslot resource.

A keyslot is composed of three "blocks". Each of these blocks has three "key entries",
each of which can hold a key and a configuration relevant to that key.


         CPD block          CA  block          CPS block
        __________         __________         __________
       |          |       |          |       |          |
       |  ODD     |       |  ODD     |       |  ODD     |
       |__________|       |__________|       |__________|
       |          |       |          |       |          |
   ->  |  EVEN    |  ->   |  EVEN    |  ->   |  EVEN    |  ->
       |__________|       |__________|       |__________|
       |          |       |          |       |          |
       |  CLEAR   |       |  CLEAR   |       |  CLEAR   |
       |__________|       |__________|       |__________|


The BLOCKS are:
 CPD: Content Protection Descrambling Block. Used to decrypt TS streams and memory blocks.
      Does not support DVB algorithms.
 CA:  Conditional Access Block. Used to decrypt TS stream. Supports DVB algorithms.
 CPS: Content Protection Scrambling Block. Used to encrypt TS streams and memory blocks.
      Does not support DVB algorithms.

The KEY ENTRIES are:
 ODD:   Applied to TS packets with ODD (b11) scrambling contorl bits.
 EVEN:  Applied to TS packets with EVEN (b10) scrambling contorl bits.
 CLEAR: Applied to TS packets with CLEAR (b00) scrambling contorl bits.

Each Transport Stream (TS) packet of a PidChannel associated with a Keyslot can be
visualised as traversing the three blocks in sequence (CPD->CA->CPS). For each of the
blocks, if there is a key in the entry that corresponds with the TS packet's scrambling
conotrol bits, the BLOCK operates on the packet with the key entry.

Note: DMA'ed memory blocks are logically packetised by the hardware into an MPEG TS stream
      (with an assumed CLEAR TS polarity) and passed though CPD->CA->CPS on an internally
      managed PidChannel.

***************************************************************************************/
#ifndef NEXUS_KEYSLOT_H__
#define NEXUS_KEYSLOT_H__

#include "nexus_types.h"
#include "nexus_security_common.h"
#include "nexus_pid_channel.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NEXUS_KEYSLOT_MAX_KEY_SIZE (32)    /* Maximum key size is 256 bits. */
#define NEXUS_KEYSLOT_MAX_IV_SIZE  (16)    /* Equates to maximum algorithm block size. */


typedef enum NEXUS_KeySlotBlockType
{
    NEXUS_KeySlotBlockType_eCpd,      /* Content Protection Descrambling block. */
    NEXUS_KeySlotBlockType_eCa,       /* Conditional Access descrambling block. */
    NEXUS_KeySlotBlockType_eCps,      /* Content Protection Scrambling block. */
    NEXUS_KeySlotBlockType_eMax
} NEXUS_KeySlotBlockType;

typedef enum NEXUS_KeySlotPolarity
{
    NEXUS_KeySlotPolarity_eOdd,       /* ODD      (b11) Scrambling Control */
    NEXUS_KeySlotPolarity_eEven,      /* EVEN     (b10) Scrambling Control */
    NEXUS_KeySlotPolarity_eClear,     /* CLEAR    (b00) Scrambling Control */
    NEXUS_KeySlotPolarity_eMax
} NEXUS_KeySlotPolarity;


/* Specify how to handle residual data that is not a multiple of the algorithm block size */
typedef enum NEXUS_KeySlotTerminationMode
{
    NEXUS_KeySlotTerminationMode_eClear,
    NEXUS_KeySlotTerminationMode_eScte52,
    NEXUS_KeySlotTerminationMode_eCtsEcb,
    NEXUS_KeySlotTerminationMode_eCoronado,
    NEXUS_KeySlotTerminationMode_eCtsDvbCpcm,
    NEXUS_KeySlotTerminationMode_eFrontResidue,
    NEXUS_KeySlotTerminationMode_eMsc,
    NEXUS_KeySlotTerminationMode_eMscCtsDvbCpcm,
    NEXUS_KeySlotTerminationMode_eTsAndPacket,
    NEXUS_KeySlotTerminationMode_ePacket,
    NEXUS_KeySlotTerminationMode_eCbcMac,
    NEXUS_KeySlotTerminationMode_eCMac,

    NEXUS_KeySlotTerminationMode_eMax
} NEXUS_KeySlotTerminationMode;

/* Specify how to handle data that is shorter that algorithm block size */
typedef enum NEXUS_KeySlotTerminationSolitaryMode
{
    NEXUS_KeySlotTerminationSolitaryMode_eClear,
    NEXUS_KeySlotTerminationSolitaryMode_eReserved1,
    NEXUS_KeySlotTerminationSolitaryMode_eIv1,
    NEXUS_KeySlotTerminationSolitaryMode_eIv2,
    NEXUS_KeySlotTerminationSolitaryMode_eDvbCpcm,

    NEXUS_KeySlotTerminationSolitaryMode_eMax
} NEXUS_KeySlotTerminationSolitaryMode;


/*
    Keyslots of different types have different sizes and support a different number of IVs (initialisation vectors).
*/
typedef enum NEXUS_KeySlotType
{
    NEXUS_KeySlotType_eIvPerSlot,     /* SMALL keyslot. One IV for the whole keyslot. Suitable for most CA decryption. (128bit)*/
    NEXUS_KeySlotType_eIvPerBlock,    /* MEDIUM keyslot. One IV per Block (CPD/CA/CPS). Suitable for most block mode decryption (128bit). */
    NEXUS_KeySlotType_eIvPerEntry,    /* LARGE keyslot. One IV per Entry. Required to support a small number of scenarios (128bit). */
    NEXUS_KeySlotType_eIvPerBlock256, /* MEDIUM keyslot. One IV per Block (CPD/CA/CPS). Suitable for most block mode decryption. */
    NEXUS_KeySlotType_eIvPerEntry256, /* LARGE keyslot. One IV per Entry. Required to support a small number of scenarios. */
    NEXUS_KeySlotType_eMulti2,        /* Multi2 keyslot is specially handled by different BSP command. */
    NEXUS_KeySlotType_eOxford1,
    NEXUS_KeySlotType_eOxford2,
    NEXUS_KeySlotType_eOxford3,
    NEXUS_KeySlotType_eMax
} NEXUS_KeySlotType;

/*
    Select key slot mode.
*/
typedef enum NEXUS_KeySlotKeyMode
{
    NEXUS_KeySlotKeyMode_eRegular,         /* The default and most frequently used. */
    NEXUS_KeySlotKeyMode_eDes56,           /* Generate a 56 bit DES key */
    NEXUS_KeySlotKeyMode_eDvbConformance,  /* Generate a legacy DVB key. */
    NEXUS_KeySlotKeyMode_eMax
} NEXUS_KeySlotKeyMode;


typedef struct NEXUS_KeySlotAllocateSettings
{
    NEXUS_SecurityCpuContext owner;    /* Indicate from what context the allocated keyslot can be configured. */
    NEXUS_KeySlotType slotType;        /* The keyslot TYPE */
    bool useWithDma;                   /* true if keyslot is to be used with DMA. In this case, NEXUS will manage the
                                          pidChannel. */
} NEXUS_KeySlotAllocateSettings;

typedef struct NEXUS_KeySlotInformation
{
    NEXUS_KeySlotType slotType;        /* The keyslot type    */
    unsigned slotNumber;               /* The keyslot number. */
} NEXUS_KeySlotInformation;

typedef struct NEXUS_KeySlotKey
{
    unsigned size;                              /* the key size in bytes. */
    uint8_t key[NEXUS_KEYSLOT_MAX_KEY_SIZE];    /* the key data. */
} NEXUS_KeySlotKey;

typedef struct NEXUS_KeySlotIv
{
    unsigned size;                              /* the iv size in bytes. */
    uint8_t iv[NEXUS_KEYSLOT_MAX_IV_SIZE];      /* the iv data. */
} NEXUS_KeySlotIv;


/*
Uniquely identify an Entry within a keyslot.
*/
typedef enum NEXUS_KeySlotBlockEntry
{
    NEXUS_KeySlotBlockEntry_eCpdOdd,
    NEXUS_KeySlotBlockEntry_eCpdEven,
    NEXUS_KeySlotBlockEntry_eCpdClear,
    NEXUS_KeySlotBlockEntry_eCaOdd,
    NEXUS_KeySlotBlockEntry_eCaEven,
    NEXUS_KeySlotBlockEntry_eCaClear,
    NEXUS_KeySlotBlockEntry_eCpsOdd,
    NEXUS_KeySlotBlockEntry_eCpsEven,
    NEXUS_KeySlotBlockEntry_eCpsClear,
    NEXUS_KeySlotBlockEntry_eMax
} NEXUS_KeySlotBlockEntry;


/*
Description:
Configuration parameters that apply to the whole keyslot.
*/
typedef struct NEXUS_KeySlotSettings
{
    struct
    {
        bool source[NEXUS_SecurityRegion_eMax];           /* regions that data can be sourced from */
        bool destinationRPipe[NEXUS_SecurityRegion_eMax]; /* regions that data can moved to via R pipe */
        bool destinationGPipe[NEXUS_SecurityRegion_eMax]; /* regions that data can moved to via G pipe */
    }regions;                                             /* TODO, for discussion. Maybe it should be an array of regions??? */

    bool encryptBeforeRave;                               /* if true, CPS will encrypt packet *before* RAVE index parser. */

} NEXUS_KeySlotSettings;

/* The configuration parameters specific to a key Enrty. */
typedef struct NEXUS_KeySlotEntrySettings
{
    NEXUS_CryptographicAlgorithm algorithm;
    NEXUS_CryptographicAlgorithmMode algorithmMode;
    NEXUS_CounterMode counterMode;                       /* Valid only if algorithm supports counter mode. */
    unsigned counterSize;                                /* For algorithm modes predicated on a counter, this parameter spcifies
                                                            the size of the counter in bits. Supported values are 32, 64, 96 and 128 bits.*/
    NEXUS_KeySlotTerminationMode terminationMode;       /* Specify how to handle data that is not a multiple of
                                                          * algorithm block size */
    NEXUS_KeySlotTerminationSolitaryMode solitaryMode;   /* Specify how to handle data that is shorter that algorithm block size */
    NEXUS_KeySlotKeyMode keyMode;                        /* NEXUS_KeySlotKeyMode_eRegular is the default and most frequently used mode. */

    struct{                                              /* If either "external" options is set to true an external keyslot is reserved. [Default=false].*/
        bool key;                                        /* If true, allow a key to be set via BTP (broadcom Transport Packet) */
        bool iv;                                         /* If true, allow an IV to be set via BTP */
    }external;                                           /* See type BHSM_KeyslotExternalKeyData for more information on external keys. */

    bool pesMode;                                        /* only valid for BHSM_CryptographicAlgorithm == DVBCSA2 */

    struct
    {
        bool specify;                   /* If false, scrambled control will be determined by context:
                                         *    - encrypt operation will not modify scramble control bits.
                                         *    - decrypt operation will resolve to NEXUS_KeySlot_ScrambleControl_Clear
                                         * If true, scrambled control will be determined by following two parameters.  [default=false]*/
        NEXUS_KeySlotPolarity  rPipe;  /* "restricted" (usually decode) Pipe. Ignored if specify==false */
        NEXUS_KeySlotPolarity  gPipe;  /* "global" (usually record) Pipe. Ignored if specify==false. */
    }outputPolarity;                    /* Specify the scramble control bits of the processed packet. */

    bool rPipeEnable;       /* true if restricted Pipe (usually decode) is enabled. [default=true] */
    bool gPipeEnable;       /* true if global Pipe (usually record) is enabled. [default=true] */

    struct
    {
        unsigned roundCount;
        unsigned keySelect; /* 0 to 63 */
    }multi2;                /* Only requied for NEXUS_CryptographicAlgorithm_eMulti2.  */

}NEXUS_KeySlotEntrySettings;

/*
Description:
    External key data that must be retrieved for a keyslot entry that has been configured
    to have an external key/iv. This information is required to compile the BTP (Broadcom
    Transport Packet) within which the key/iv will be encoded and placed in the stream/DMA.
    Instrictions on how to compile a BTP is explained in a separately delivered document.
*/
typedef struct NEXUS_KeySlotExternalKeyData
{
    unsigned slotIndex;  /* Index into external key-slot table. */
    struct
    {
        bool valid;      /* Indicates that the associated offset is valid.  */
        unsigned offset; /* offset into external key slot (in 64bit steps). */
    } key, iv;
}NEXUS_KeySlotExternalKeyData;

/**
Description:
    Settings used for NEXUS_KeySlot_AddPidChannel
**/
typedef struct NEXUS_KeySlotAddPidChannelSettings
{
    bool secondary;         /* true if secondary pid. Default is primary. */
} NEXUS_KeySlotAddPidChannelSettings;

/**
Description:
    Settings used for NEXUS_KeySlot_SetMulti2Key
**/
typedef struct NEXUS_KeySlotSetMulti2Key
{
    unsigned keySelect; /* 0 to 63 */
    uint8_t  key[32];   /* Multi2 system Key. */

} NEXUS_KeySlotSetMulti2Key;




void NEXUS_KeySlot_GetDefaultAllocateSettings(
    NEXUS_KeySlotAllocateSettings *pSettings
    );

/*
Description:
    Allocate a Keyslot. NULL is returned if no resource is available.
*/
NEXUS_KeySlotHandle NEXUS_KeySlot_Allocate(
    const NEXUS_KeySlotAllocateSettings *pSettings
    );

/*
Description:
    Free a Keyslot
*/
void NEXUS_KeySlot_Free(
    NEXUS_KeySlotHandle handle
    );
/*
Description:
    Get keyslot configuration.
*/
void  NEXUS_KeySlot_GetSettings(
    NEXUS_KeySlotHandle handle,
    NEXUS_KeySlotSettings *pSettings  /*[out]*/
    );

/*
Description:
    Set keyslot configuration. These configuration parameters apply the all keyslot entries..
*/
NEXUS_Error NEXUS_KeySlot_SetSettings(
    NEXUS_KeySlotHandle handle,
    const NEXUS_KeySlotSettings *pSettings
    );

/*
Description:
    Get the configuration of one keyslot entry.
*/
void NEXUS_KeySlot_GetEntrySettings(
    NEXUS_KeySlotHandle handle,
    NEXUS_KeySlotBlockEntry entry,           /* block (cps/ca/cpd) and entry (odd/even/clear) */
    NEXUS_KeySlotEntrySettings *pSettings /*[out]*/
    );

/*
Description:
    Configure one entry of a keyslot.
*/
NEXUS_Error NEXUS_KeySlot_SetEntrySettings( NEXUS_KeySlotHandle handle,
                                            NEXUS_KeySlotBlockEntry entry,           /* block (cps/ca/cpd) and entry (odd/even/clear) */
                                            const NEXUS_KeySlotEntrySettings *pSettings
                                          );

/*
Description:
    Can be called if a keyslot entry had been configured to have an "external" keyslot location.
    The Key and Iv can thus be loaded with a BTP (Broadcom Transport Packet) embedded in the stream.
    The location of the BTP in the stream identifies where/when the key is loaded and activated.
*/
NEXUS_Error NEXUS_KeySlot_GetEntryExternalKeySettings(
    NEXUS_KeySlotHandle handle,
    NEXUS_KeySlotBlockEntry entry,         /* block (cps/ca/cpd) and entry (odd/even/clear) */
    NEXUS_KeySlotExternalKeyData *pData    /*[out] Data to be included in the BTP */
    );


/*
Description:
    Write a key into a keyslot entry
*/
NEXUS_Error NEXUS_KeySlot_SetEntryKey (
    NEXUS_KeySlotHandle handle,
    NEXUS_KeySlotBlockEntry entry,  /* block (cps/ca/cpd) and entry (odd/even/clear) */
    const NEXUS_KeySlotKey *pKey
    );

/*
Description:
    Write an IV into a keyslot entry.
    Note:
       pEntry->polarity will be ignored for keylosts of type NEXUS_KeySlotType_eIvPerBlock
       pEntry->block and pEntry->polarity will be ignored for keylosts of type NEXUS_KeySlotType_eIvPerSlot
*/
NEXUS_Error NEXUS_KeySlot_SetEntryIv(
    NEXUS_KeySlotHandle handle,
    NEXUS_KeySlotBlockEntry entry,   /* block (cps/ca/cpd) and entry (odd/even/clear) */
    const NEXUS_KeySlotIv *pIv,      /* attr{null_allowed=y} regular IV, Set to NULL to update pIv2 independently */
    const NEXUS_KeySlotIv *pIv2      /* attr{null_allowed=y} secondary IV, required by some algorithm configurations for
                                        processing residual data. Set to NULL if not required. */
    );

/*
Description:
    Invalidate all entries of a keyslot.
*/
NEXUS_Error NEXUS_KeySlot_Invalidate( NEXUS_KeySlotHandle handle );

/*
Description:
    Invalidate an individual keyslot entry.
*/
NEXUS_Error NEXUS_KeySlot_InvalidateEntry(
    NEXUS_KeySlotHandle handle,
    NEXUS_KeySlotBlockEntry entry    /* block and entry */
    );

/**
Description:
Return the defaults for settings for adding a pid channel to a keyslot.
**/
void NEXUS_KeySlot_GetDefaultAddPidChannelSettings(
    NEXUS_KeySlotAddPidChannelSettings *pSettings /* [out] */
    );

/*
Description:
    Associate a keyslot with a Pid channel.
*/
NEXUS_Error NEXUS_KeySlot_AddPidChannel(
    NEXUS_KeySlotHandle handle,
    NEXUS_PidChannelHandle pidChannel
    );

/*
Description:
    Associate a keyslot with a Pid channel.
*/
NEXUS_Error NEXUS_KeySlot_AddPidChannelWithSettings(
    NEXUS_KeySlotHandle handle,
    NEXUS_PidChannelHandle pidChannel,
    const NEXUS_KeySlotAddPidChannelSettings *pSettings /* attr{null_allowed=y} NULL is allowed for default settings. */
    );


NEXUS_Error NEXUS_KeySlot_RemovePidChannel(
    NEXUS_KeySlotHandle handle,
    NEXUS_PidChannelHandle pidChannel
    );

/*
Description:
    Returns information on keyslot
*/
NEXUS_Error NEXUS_KeySlot_GetInformation(
    NEXUS_KeySlotHandle handle,
    NEXUS_KeySlotInformation *pInfo    /*[out]*/
    );


/*
Description:
    Returns default configuration for NEXUS_KeySlotSetMulti2Key structure
*/
void NEXUS_KeySlot_GetDefaultMulti2Key(
    NEXUS_KeySlotSetMulti2Key *pKeyData    /*[out]*/
    );


/*
Description:
    Call to set 1 of 64 Multi2 systems keys.
    Only required for use with NEXUS_CryptographicAlgorithm_eMulti2.
*/
NEXUS_Error NEXUS_KeySlot_SetMulti2Key(
    const NEXUS_KeySlotSetMulti2Key *pKeyData
    );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
