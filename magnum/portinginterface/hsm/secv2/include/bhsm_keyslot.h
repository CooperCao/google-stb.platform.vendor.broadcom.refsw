/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/***************************************************************************************
Interface decription.

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
#ifndef BHSM_KEYSLOT_H__
#define BHSM_KEYSLOT_H__

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"
#include "bkni_multi.h"
#include "bint.h"
#include "bmem.h"
#include "berr_ids.h"

#include "bhsm_common.h"
#ifdef __cplusplus
extern "C"
{
#endif


#define BHSM_KEYSLOT_MAX_KEY_SIZE (32)    /* Maximum key size is 256 bits. */
#define BHSM_KEYSLOT_MAX_IV_SIZE (BHSM_KEYSLOT_MAX_KEY_SIZE)


typedef struct BHSM_P_Keyslot* BHSM_KeyslotHandle;

typedef enum
{
    BHSM_KeyslotBlockType_eCpd,      /* Content Protection Descrambling block. */
    BHSM_KeyslotBlockType_eCa,       /* Conditional Access descrambling block. */
    BHSM_KeyslotBlockType_eCps,      /* Content Protection Scrambling block. */
    BHSM_KeyslotBlockType_eMax
} BHSM_KeyslotBlockType;

typedef enum
{
    BHSM_KeyslotPolarity_eOdd,       /* ODD      (b11) Scrambling Control */
    BHSM_KeyslotPolarity_eEven,      /* EVEN     (b10) Scrambling Control */
    BHSM_KeyslotPolarity_eClear,     /* CLEAR    (b00) Scrambling Control */
    BHSM_KeyslotPolarity_eReserved,  /* Reserved (b01) Scrambling Control */
    BHSM_KeyslotPolarity_eMax
} BHSM_KeyslotPolarity;


/* Specify how to handle residual data that is not a multiple of the algorithm block size */
typedef enum
{
    BHSM_Keyslot_TerminationMode_eClear,
    BHSM_Keyslot_TerminationMode_eScte52,
    BHSM_Keyslot_TerminationMode_eCtsEcb,
    BHSM_Keyslot_TerminationMode_eCtsDvbCpcm,
    BHSM_Keyslot_TerminationMode_eFrontResidue,
    BHSM_Keyslot_TerminationMode_eMsc,
    BHSM_Keyslot_TerminationMode_eReserved0x06,
    BHSM_Keyslot_TerminationMode_eTsAndPacket,
    BHSM_Keyslot_TerminationMode_ePacket,
    BHSM_Keyslot_TerminationMode_eCbcMac,
    BHSM_Keyslot_TerminationMode_eCMac,

    BHSM_Keyslot_TerminationMode_eMax
} BHSM_Keyslot_TerminationMode;

/* specify how the IV is processed.*/
typedef enum
{
    BHSM_Keyslot_IvMode_eNoPreProc,          /* use IV as is. This is the most used option */
    BHSM_Keyslot_IvMode_eMdi,
    BHSM_Keyslot_IvMode_eMdd,
    BHSM_Keyslot_IvMode_eNoPreProcWriteBack, /* use IV as is, but write back the
                                                final IV back to the slot after packet processing. */
    BHSM_Keyslot_IvMode_eMax
}BHSM_Keyslot_IvMode;


/* Specify how to handle data that is shorter that algorithm block size */
typedef enum
{
    BHSM_KeyslotTerminationSolitaryMode_eClear,
    BHSM_KeyslotTerminationSolitaryMode_eReserved1,
    BHSM_KeyslotTerminationSolitaryMode_eIv1,
    BHSM_KeyslotTerminationSolitaryMode_eIv2,
    BHSM_KeyslotTerminationSolitaryMode_eDvbCpcm,

    BHSM_KeyslotTerminationSolitaryMode_eMax
} BHSM_KeyslotTerminationSolitaryMode;


typedef struct
{
    BHSM_SecurityCpuContext owner;    /* Indicate from what context the allocated keyslot can be configured. */
    BHSM_KeyslotType slotType;        /* The keyslot TYPE */
    unsigned keySlotNumber;           /* Only required on SAGE side. Ignored on HOST side. */
    bool useWithDma;                  /* true if keyslot is to be used with a DMA engine, false for a
                                         playback/frontend source. */
} BHSM_KeyslotAllocateSettings;


typedef struct
{
    unsigned size;                             /* the key size in bytes. */
    uint8_t key[BHSM_KEYSLOT_MAX_KEY_SIZE];    /* the key data. */
} BHSM_KeyslotKey;

typedef struct
{
    unsigned size;                              /* the iv size. */
    uint8_t iv[BHSM_KEYSLOT_MAX_IV_SIZE];      /* the iv data. */
} BHSM_KeyslotIv;

typedef struct
{
    unsigned keyLadderIndex;
    unsigned keyLadderLayer;
}BHSM_KeyslotRouteKey;

typedef struct
{
    unsigned keyLadderIndex;
    unsigned keyLadderLayer;
    bool     configIv2;           /*true if key is to be routed to IV2 */
}BHSM_KeyslotRouteIv;

typedef struct
{
    BHSM_KeyslotType type; /* The keyslot type */
    unsigned number;       /* The keyslot number */
} BHSM_KeyslotInfo;


/*
Uniquely identify an Entry within a keyslot.
*/
typedef enum
{
    BHSM_KeyslotBlockEntry_eCpdOdd,
    BHSM_KeyslotBlockEntry_eCpdEven,
    BHSM_KeyslotBlockEntry_eCpdClear,
    BHSM_KeyslotBlockEntry_eCaOdd,
    BHSM_KeyslotBlockEntry_eCaEven,
    BHSM_KeyslotBlockEntry_eCaClear,
    BHSM_KeyslotBlockEntry_eCpsOdd,
    BHSM_KeyslotBlockEntry_eCpsEven,
    BHSM_KeyslotBlockEntry_eCpsClear,
    BHSM_KeyslotBlockEntry_eMax
} BHSM_KeyslotBlockEntry;


/*
Description:
Configuration parameters that apply to the whole keyslot.
*/
typedef struct
{
    struct
    {
        bool source[BHSM_SecurityRegion_eMax];           /* regions that data can be sourced from */
        bool destinationRPipe[BHSM_SecurityRegion_eMax]; /* regions that data can moved to via R pipe */
        bool destinationGPipe[BHSM_SecurityRegion_eMax]; /* regions that data can moved to via G pipe */
    }regions;

    bool encryptBeforeRave;                              /* if true, CPS will encrypt packet *before* RAVE index parser. */

} BHSM_KeyslotSettings;

/* The configuration paramters specific to a key Enrty. */
typedef struct
{
    BHSM_CryptographicAlgorithm algorithm;
    BHSM_CryptographicAlgorithmMode algorithmMode;
    unsigned  counterSize;                              /* For algorithms modes predicated on a counter, this parameter spcifies
                                                           the number of bits that hold the counter. Supported vales are 32, 64, 96 and 128 bits */
    BHSM_Keyslot_TerminationMode terminationMode;       /* Specify how to handle data that is not a multiple of
                                                           algorithm block size */
    BHSM_KeyslotTerminationSolitaryMode solitaryMode;   /* Specify how to handle data that is shorter that
                                                           algorithm block size */
    BHSM_Keyslot_IvMode ivMode;                         /* Selects mode for IV pre-processing. Only relevant to algorithm modes that use IVs. */

    struct{                                             /* If either "external" options is set to true an external keyslot is reserved. [Default=false].*/
        bool key;                                       /* If true, allow a key to be set via BTP */
        bool iv;                                        /* If true, allow an IV to be set via BTP */
    }external;                                          /* See type BHSM_KeyslotExternalKeyData for more information on external keys. */

    bool pesMode;                                       /* only valid for BHSM_CryptographicAlgorithm == DVBCSA2 */
    BHSM_CounterMode counterMode;                       /* */
    struct
    {
        bool specify;                   /* If false, scrambled control will be determined by context:
                                         *    - encrypt operation will not modify scramble control bits.
                                         *    - decrypt operation will resolve to BHSM_Keyslot_ScrambleControl_Clear
                                         * If true, scrambled control will be determined by following two parameters.  [default=false]*/
        BHSM_KeyslotPolarity  rPipe;  /* "restricted" (usually decode) Pipe. Ignored if specify==false */
        BHSM_KeyslotPolarity  gPipe;  /* "global" (usually record) Pipe. Ignored if specify==false. */
    }outputPolarity;                    /* Specify the scramble control bits of the processed packet. */

    bool rPipeEnable;       /* true if restricted Pipe (usually decode) is enabled. [default=true] */
    bool gPipeEnable;       /* true if global Pipe (usually record) is enabled. [default=true] */

    struct
    {
        unsigned roundCount;
        unsigned keySelect; /* 0 to 63 */
    }multi2;  /* only required for BHSM_CryptographicAlgorithm_eMulti2 */

}BHSM_KeyslotEntrySettings;

/*
Description:
    External key data that can be retrieved for a keyslot entry that has been configured
    to have an external key. This information is required to compile the BTP (Broadcom
    Transport Packet) within which the key/iv will be encoded and placed in the stream.
    Instrictions on how to compile a BTP is explained in a separately delivered document.
*/
typedef struct
{
    unsigned slotIndex;  /* Index into external key-slot table. */
    struct
    {
        bool valid;      /* Indicates that the associated offset is valid.  */
        unsigned offset; /* offset into external key slot (in 64bit steps). */
    } key, iv;
}BHSM_KeyslotExternalKeyData;

/**
Description:
    Settings used for BHSM_Keyslot_AddPidChannel
**/
typedef struct
{
    bool secondary;         /* true if secondary pid. Default is primary. */
} BHSM_KeyslotAddPidChannelSettings;


typedef struct BHSM_KeySlotSetMulti2Key
{
    unsigned keySelect; /* 0 to 63 */
    uint8_t  key[32];   /* Multi2 system Key. */
} BHSM_KeySlotSetMulti2Key;


void BHSM_Keyslot_GetDefaultAllocateSettings(
    BHSM_KeyslotAllocateSettings *pSettings
    );

/*
Description:
    Allocate a Keyslot. NULL is returned if no resource is available.
    When called from SAGE, the calling code must specify pSettings->slotType and
    pSettings->slotNumber of a keyslot allocated on the HOST side.
*/
BHSM_KeyslotHandle BHSM_Keyslot_Allocate(
    BHSM_Handle hHsm,
    const BHSM_KeyslotAllocateSettings *pSettings
    );

/*
Description:
    Free a Keyslot
*/
void BHSM_Keyslot_Free(
    BHSM_KeyslotHandle handle
    );

/*
Description:
    Get keyslot configuration.
*/
void  BHSM_Keyslot_GetSettings(
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotSettings *pSettings
    );

/*
Description:
    Set keyslot configuration. These configuration paramters apply the all keyslot entries..
*/
BERR_Code BHSM_Keyslot_SetSettings(
    BHSM_KeyslotHandle handle,
    const BHSM_KeyslotSettings *pSettings
    );

/*
Description:
    Get the configuration of one keyslot entry.
*/
void BHSM_Keyslot_GetEntrySettings(
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotBlockEntry entry,           /* block (cps/ca/cpd) and entry (odd/even/clear) */
    BHSM_KeyslotEntrySettings *pSettings
    );

/*
Description:
    Configure one entry of a keyslot.
*/
BERR_Code BHSM_Keyslot_SetEntrySettings( BHSM_KeyslotHandle handle,
                                            BHSM_KeyslotBlockEntry entry,           /* block (cps/ca/cpd) and entry (odd/even/clear) */
                                            const BHSM_KeyslotEntrySettings *pSettings
                                          );

/*
Description:
    Can be called if a keyslot entry had been configured to have an "external" keyslot location.
    The Key and Iv can thus be loaded with a BTP (Broadcom Transport Packet) embedded in the stream.
    The location of the BTP in the stream identifies where/when the key is loaded/active.

*/
BERR_Code BHSM_Keyslot_GetEntryExternalKeySettings(
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotBlockEntry entry,           /* block (cps/ca/cpd) and entry (odd/even/clear) */
    BHSM_KeyslotExternalKeyData *pData
    );


/*
Description:
    Write a key into a keyslot entry
*/
BERR_Code BHSM_Keyslot_SetEntryKey (
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotBlockEntry entry,  /* block (cps/ca/cpd) and entry (odd/even/clear) */
    const BHSM_KeyslotKey *pKey
    );

/*
Description:
    Write an IV into a keyslot entry.
    Note:
       pEntry->polarity will be ignored for keylosts of type BHSM_KeyslotType_eIvPerBlock
       pEntry->block and pEntry->polarity will be ignored for keylosts of type BHSM_KeyslotType_eIvPerSlot
*/
BERR_Code BHSM_Keyslot_SetEntryIv(
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotBlockEntry entry,   /* block (cps/ca/cpd) and entry (odd/even/clear) */
    const BHSM_KeyslotIv *pIv,      /* regular IV, Set to NULL to update pIv2 independently */
    const BHSM_KeyslotIv *pIv2      /* secondary IV, required by some algorithm configurations for
                                        processing residual data. Set to NULL if not required. */
    );

/*
Description:
    Route a key to the the keyslot from a Keyladder
*/
BERR_Code BHSM_Keyslot_RouteEntryKey (
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotBlockEntry entry,  /* block (cps/ca/cpd) and entry (odd/even/clear) */
    const BHSM_KeyslotRouteKey *pKey
    );

/*
Description:
    Route an Iv to the the keyslot from a Keyladder
*/
BERR_Code BHSM_Keyslot_RouteEntryIv(
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotBlockEntry entry,   /* block (cps/ca/cpd) and entry (odd/even/clear) */
    const BHSM_KeyslotRouteIv *pIv
    );

/*
Description:
    Invalidate all entries of a keyslot.
*/
BERR_Code BHSM_Keyslot_Invalidate( BHSM_KeyslotHandle handle );

/*
Description:
    Invalidate an individual keyslot entry.
*/
BERR_Code BHSM_Keyslot_InvalidateEntry(
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotBlockEntry entry    /* block and entry */
    );

/**
Description:
Return the defaults for settings for adding a pid channel to a keyslot.
**/
void BHSM_Keyslot_GetDefaultAddPidChannelSettings(
    BHSM_KeyslotAddPidChannelSettings *pSettings /* [out] */
    );

/*
Description:
    Associate a keyslot with a Pid channel.
*/
BERR_Code BHSM_Keyslot_AddPidChannel_WithSettings(
    BHSM_KeyslotHandle handle,
    unsigned pidChannelIndex,
    const BHSM_KeyslotAddPidChannelSettings *pSettings /* attr{null_allowed=y} NULL is allowed for default settings. */
    );

BERR_Code BHSM_Keyslot_AddPidChannel(
    BHSM_KeyslotHandle handle,
    unsigned pidChannelIndex
    );

BERR_Code BHSM_Keyslot_RemovePidChannel(
    BHSM_KeyslotHandle handle,
    unsigned pidChannelIndex
    );

/*
Description:
    Return keyslot information.
*/
BERR_Code BHSM_GetKeySlotInfo(
    BHSM_KeyslotHandle handle,
    BHSM_KeyslotInfo *pInfo
    );


BERR_Code BHSM_KeySlot_SetMulti2Key(
    BHSM_Handle hHsm,
    const BHSM_KeySlotSetMulti2Key *pKeyData
    );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
