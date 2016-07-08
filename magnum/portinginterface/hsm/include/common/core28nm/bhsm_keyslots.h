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


#ifndef BHSM_KEYSLOTS_H__
#define BHSM_KEYSLOTS_H__


/* added to remove header file dependency */
#include "bhsm.h"
#include "bhsm_private.h"
#include "bsp_s_hw.h"


#ifdef __cplusplus
extern "C" {
#endif



#define BHSM_KEYLADDER_VKL_SHIFT    0x0
#define BHSM_KEYLADDER_KL_SHIFT     0x8

/**
Summary:
The following are generic region IDs. G indicates Global, R indicates Restricted.
**/
#define BHSM_GLOBAL_REGION        0x01
#define BHSM_RESTRICTED_REGION_0  0x02
#define BHSM_RESTRICTED_REGION_1  0x04
#define BHSM_RESTRICTED_REGION_2  0x08
#define BHSM_RESTRICTED_REGION_4  0x10
#define BHSM_RESTRICTED_REGION_5  0x20
#define BHSM_RESTRICTED_REGION_6  0x40
#define BHSM_RESTRICTED_REGION_7  0x80
/**
Summary:
The following are specified region IDs remapping from Generic.
**/
#define BHSM_REGION_GLR  BHSM_GLOBAL_REGION         /* GLR - Global Region */
#define BHSM_REGION_CRR  BHSM_RESTRICTED_REGION_0   /* CRR - Compresses Restricted Region, compressed referring to compressed media */


#define   BHSM_SLOT_NUM_INIT_VAL  (0xFFFF)


/* Module Specific Functions */


/*** add new enums in this section if appliccable to all chips/versiona && all customer modes START*******/

/**************************************************************************************************
BHSM_KeySlotOwner_e is used to identify keyslot "ownership".
 - _eFREE indicates that the keyslot has no owner and can be allocated to either HOST or SAGE
 - _eSAGE indicats that the keyslot is dedicated to sage and can't be configured from the HOST.
 - _eSHARED indicates that the keyslot can be configured from either HOST or SAGE.
**************************************************************************************************/

typedef enum BHSM_KeySlotOwner_e
{
    BHSM_KeySlotOwner_eFREE = 0,
    BHSM_KeySlotOwner_eSAGE,
    BHSM_KeySlotOwner_eSHARED
} BHSM_KeySlotOwner_e;


/*
Enumerator to identify a particular Bypass Keyslot.
*/
typedef enum BHSM_BypassKeySlot_e{
    BHSM_BypassKeySlot_eG2GR,      /* allows transfer of clear data from GLR to GLR or CRR */
    BHSM_BypassKeySlot_eGR2R,      /* allows transfer of clear data from GLR or CRR to CRR */
    BHSM_BypassKeySlot_eInvalid    /* Not a bypass bypass filter. */
}BHSM_BypassKeySlot_e;



/*** add new enums in this section if appliccable to all chips/versiona && all customer modes END********/


/**************************************************************************************************
Summary:

Description:
this structure defines how many key slots are required for each of key slot type 0, 1, 2, 3, 4, 5,6
respectively for CA key slot table initialization.

See Also:
BHSM_InitKeySlot
BCMD_XptSecKeySlot_e
**************************************************************************************************/
typedef struct BHSM_InitKeySlotIO {

    uint32_t            unKeySlotType0Num;   /* In: how many key slots are required for type0   */
    uint32_t            unKeySlotType1Num;   /* In: how many key slots are required for type1   */
    uint32_t            unKeySlotType2Num;   /* In: how many key slots are required for type2   */
    uint32_t            unKeySlotType3Num;   /* In: how many key slots are required for type3   */
    uint32_t            unKeySlotType4Num;   /* In: how many key slots are required for type4   */
    uint32_t            unKeySlotType5Num;   /* In: how many key slots are required for type5 - Zeus 4.0 */
    unsigned            numMulti2KeySlots;   /* In: Number of Multi2 Keyslot.  */

    uint32_t            unStatus;            /* Out: 0 for success, 0x20 for repeat command submission, otherwise failed */

    uint32_t            bMulti2SysKey;      /* DEPRECATED.  Use BHSM_GetCapabilities to retrieve this information */
    uint32_t            numKeySlotType0;    /* DEPRECATED.  Use BHSM_GetCapabilities to retrieve this information */
    uint32_t            numKeySlotType1;    /* DEPRECATED.  */
    uint32_t            numKeySlotType2;    /* DEPRECATED.  */
    uint32_t            numKeySlotType3;    /* DEPRECATED.  */
    uint32_t            numKeySlotType4;    /* DEPRECATED.  */
    uint32_t            numKeySlotType5;    /* DEPRECATED.  */

} BHSM_InitKeySlotIO_t;

/*****************************************************************************
Summary:

This function allocates specified number of CA key slots for each key slot type.

Description:

There is a piece of hardware storage allocated for CA key slots.  The key slot table has 1530
(Refer to BCMD_XPTSECKEYTABLE_TOP) entries, where each entry has 64 bits. The
total size could be varied from chip to chip.

There are 5 types of key slot being defined.  Each key slot in different key slot type consists
of different number of entries as follow:
    Key slot type 0 always has 10 entries.
    Key slot type 1 always has 12 entries.
    Key slot type 2 always has 14 entries.
    Key slot type 3 always has 16 entries.
    Key slot type 4 always has 16 entries.

This function requests BSP to initialize the specified number of key slots for each key slot type.
the number of key slots can be set to zero if the slot type is not used. As long as the combination of
all the key slots of for each key slot type does not exceed the maximum key slot table capacity,
BSP will return succesfully. No CA key slot can be accessed until this function is called and
returns successfully.

This function also keeps track of the allocation information, which can be used to check if a specific
key slot has been associated with certain pid channel.

Note that this function shall be called only once after system bootup. BSP shall return error if
calling this function more than once. However, the key slot allocation in BSP, resulted from the first
successful function call, remains intact until the chip is reset.  For debugging purpose, the
caller can call this function mutiple time, provided the same allocation is specified each time.
Calling this function multiple times with different allocation values shall have an undetermined
consequence.


Calling Context:

After each board reset, this function shall be called only once before CA key slot can be utilized.
In some system that does not requried CA descrambling, this function can be omitted.


Performance and Timing:

This is a synchronous/blocking function that will not return until it is done or failed.

Input:

hHsm  - BHSM_Handle, Host Secure module handle.
pInitKeySlotIO  - BHSM_InitKeySlotIO_t


Output:

inoutp_initKeySlotIO -BHSM_InitKeySlotIO_t,


Returns:

BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_AllocateCAKeySlot
BHSM_FreeCAKeySlot


*****************************************************************************/
BERR_Code   BHSM_InitKeySlot( BHSM_Handle hHsm,
                              BHSM_InitKeySlotIO_t  *pInitKeySlot );


/* Special dummy pid channel number is used as the safe required channel number parameter
    being passed to BHSM_ConfigPidKeyPointerTable():
     (1) if CA keyslot is allocated, yet no further configuration for any pid channel to consume it,
            later when such keyslot needs to be freed, or
       (2) if the same pid channel is re-configured  to a different keyslot_b from keyslot_a, then the application needs to
            free keyslot_a.
*/
#define  DUMMY_PID_CHANNEL_TOAKEYSLOT   (BCMD_TOTAL_PIDCHANNELS)

typedef enum BHSM_SPIDProg_e
{
    BHSM_SPIDProg_ePIDPointerA,
    BHSM_SPIDProg_ePIDPointerB

}  BHSM_SPIDProg_e;

/**************************************************************************************************
Summary:

Description:
this structure defines which PID channel to assocaite with which allocated key slot

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_PidChannelType_e
**************************************************************************************************/
typedef struct BHSM_ConfigPidKeyPointerTableIO {
    /* In:  which PID channel is to be associated with an allocated key slot, 0~127   */
    unsigned int                    unPidChannel;

    /* In:  Associate the keyslot with multiple PID channels, supported with BFW 4.0.0 and later. */
    bool                            setMultiplePidChannels;

    /* In:  The last PID channel to be associated with an allocated key slot, 0~1023, supported with BFW 4.0.0 and later. */
    unsigned int                    unPidChannelEnd;

    /* In:  Program SPID as PID pointer A or PID pointer B */
    BHSM_SPIDProg_e                    spidProgType;

    /* In: Whether to reset the PID to default entry  */
    bool                            bResetPIDToDefault;

    /* In:   slot type of the allocated key slot , 0~4  at max depending on keyslot  initializaiton*/
    BCMD_XptSecKeySlot_e            ucKeySlotType;

    /* In:   slot number of the allocated key slot , depending on keyslot initializaiton*/
    unsigned int                    unKeySlotNum;

    /* In:  the channcel is in primary or secondary PID channel table, BHSM_PidChannelType_e */
    BHSM_PidChannelType_e            pidChannelType;

    /* In:  conditional, only needed if a 2nd key slot is needed for the PID channel, slot type value*/
    BCMD_XptSecKeySlot_e            unKeySlotBType;

    /* In:  conditional, only needed if a 2nd key slot is needed for the PID channel, slot number value*/
    uint32_t                        unKeySlotNumberB;

    /* In:  conditional, only needed if a 2nd key slot is actually used for a selected destination
        which is chosend when a predefined bit is set to 1:
            bit[7] - MessageFilter  - via pipe R
            bit[6] - MessageFilter  - via pipe G
            bit[5] - RAVE     - via pipe R
            bit[4] - RAVE     - via pipe G
            bit[3] - REMUX 1  - via pipe R
            bit[2] - REMUX 1  - via pipe G
            bit[1] - REMUX 0  - via pipe R
            bit[0] - REMUX 0  - via pipe G
    */
    uint32_t                        unKeyPointerSel;

    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;


} BHSM_ConfigPidKeyPointerTableIO_t;

/*****************************************************************************
Summary:

This function associates a CA key slot with the specified pid channel.

Description:

This function is called to associate a CA key slot with the specified primary PID channel.
Optionally, this function can be used to associate a second key slot with the secondary PID
channel. Secondary pid channels can be used to route data to secure region and global
region of various hardware modules, like MessageFilter, RAVE, REMUX1, REMUX0.
However, this feature is currently designed for future enhanced features.

The total number of primary PID channels and secondary PID channels shall be
chip-specific.  However, the key slot should be allocated in advance.

The caller can call BHSM_ConfigPidChannelToDefaultKeySlot to terminate the association
of pid channel with a key slot.  This will re-associate the pid channel to the default key slot
that will bypass any scrambling/descrambling operation.



Calling Context:

This function shall be called after BHSM_InitKeySlot has allocated key slot successfully,
before any functions (for example BHSM_ConfigAlgorithm, BHSM_LoadRouteUserKey,
BHSM_GenerateRouteKey and etc) are called to configure algorithm or load key into the
CA key slot.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_configPidKeyPointerTableIO  - BHSM_ConfigPidKeyPointerTableIO_t


Output:
inoutp_configPidKeyPointerTableIO  - BHSM_ConfigPidKeyPointerTableIO_t


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_InitKeySlot
BHSM_ConfigAlgorithm
BHSM_LoadRouteUserKey
BHSM_GenerateRouteKey
BHSM_ConfigPidChannelToDefaultKeySlot

*****************************************************************************/
BERR_Code BHSM_ConfigPidKeyPointerTable (
        BHSM_Handle                        hHsm,
        BHSM_ConfigPidKeyPointerTableIO_t    *inoutp_configPidKeyPointerTableIO
);


/**************************************************************************************************
Summary:

Description:
this structure defines all the required setting and key data to load a control word into an allocated key slot,
and route it into a pre-allocated key slot if required.

See Also:
BHSM_LoadRouteUserKey
**************************************************************************************************/
typedef struct BHSM_LoadRouteUserKeyIO {

    /* In: the key size, either 64/128/192 bits as in BCMD_KeySize_e  */
    union{
        BCMD_KeySize_e eKeySize;
        unsigned short usKeySize; /* number of bits used only for 2048 bit user key*/
    }keySize;

    /* In: the actual key to be loaded, big endian order, starting from the 1st byte, padding at bottom */
    unsigned char                 aucKeyData[BCMD_KEYLADDER_KEYRAM_SIZE];

    /* In: need to further roue this key from BSP KeyRAM to keyslot? true/yes, false/no(only in buf)*/
    bool                            bIsRouteKeyRequired;

    /* In: which kind of key slot destination, CA/RMX/M2M/HDMI/etc, see BCMD_KeyDestBlockType_e*/
    BCMD_KeyDestBlockType_e        keyDestBlckType;

    /* In: which entry type of key destination, odd/even key entry or IV entry or else, see
        BCMD_KeyDestEntryType_e
    */
    BCMD_KeyDestEntryType_e        keyDestEntryType;

    /* In: IV type for this key */
    BCMD_KeyDestIVType_e        keyDestIVType;

    /* In:  value of the allocated key slot type  */
    BCMD_XptSecKeySlot_e            caKeySlotType;

    /* In:  value of the allocated key slot number */
    unsigned int                    unKeySlotNum;

    /* In:  which mode to use the key, BCMD_KeyMode_eRegular normally, see BCMD_KeyMode_e */
    BCMD_KeyMode_e                keyMode;

    /* In: The equivalent value of SC bits 0x01 for R-pipe (either odd (0x11) or clear (0x00))   */
    unsigned char               RpipeSC01Val;

    /* In: The equivalent value of SC bits 0x01 for G-pipe (either odd (0x11) or clear (0x00))   */
    unsigned char               GpipeSC01Val;

    /* In:    SC01 mode word mapping - of type BCMD_SC01ModeWordMapping_e for Zeus 3.0 */
    BHSM_SC01ModeWordMapping_e  SC01ModeMapping;


    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;

} BHSM_LoadRouteUserKeyIO_t;


/**************************************************************************************************
Summary:

Description:
Identifies a particular key location

See Also:
BHSM_LoadRouteUserKey
**************************************************************************************************/
typedef struct BHSM_KeyLocation_t {

    /* key slot number for type */
    unsigned int                unKeySlotNum;

    /* 1,2,3,4,5, ... */
    BCMD_XptSecKeySlot_e        caKeySlotType;

    /* CPD/CA/CPS/.../HDMI/...   */
    BCMD_KeyDestBlockType_e     keyDestBlckType;

    /* odd/even/clear/...    */
    BCMD_KeyDestEntryType_e     keyDestEntryType;

} BHSM_KeyLocation_t;


typedef struct BHSM_ExternalKeyIdentifier_t {
    unsigned slotIndex; /* Index into external key-slot table.  */

    struct {
        bool valid;      /* Indicates that the associated index is valid.  */
        unsigned offset;  /* Index into external key slot. */
    } key, iv;

} BHSM_ExternalKeyIdentifier_t;

BERR_Code BHSM_GetExternalKeyIdentifier(
        BHSM_Handle            hHsm,
        BHSM_KeyLocation_t   *pKeyLocation,
        BHSM_ExternalKeyIdentifier_t *pExtKey
);


/*****************************************************************************
Summary:

This function loads a control word into BSP. The key can either be routed to CA, M2M, HDMI, IV
and REMUX modules or be used for RSA, DSA, or DH operations.


Description:

This function can be used to load the key to the BSP Key RAM and use the key in RSA, DSA,
DH and other cryptographic operation. The function can also load the key to the BSP Key
RAM and then route the key to different CA and CP destination. However, depend on
the access control matrix and OTP progamming, loading control word to certain destination
may be blocked.

The length of selected BSP Key RAM buffer is either 192bits or 2048bits, each with three
allocated buffers.  192-bit buffer is typically used to store keys, with the size of 64/128/192
bits, and is normally used for CA, M2M, HDMI, IV and REMUX modules.  2048-bit buffer
is used to store keys with the size of 1024/2048 bits, and is normally used for
RSA, DSA, or DH operations.

Key slot of certain destination, for example CA, may contain a few keys, such as odd, even and IV
key.  This function can load only one key at a time. If the same key slot entry is
loaded multiple times, the last call may overwrite any previous loading.


The key order shall be in big endian. If the actual key bytes to load is shorter than
192 bits or 2048 bits, they will be loaded at at the most significant byte of the aucKeyData
variable. For example 64-bit DES key is occupying first 8 bytes of aucKeyData variable.

Refer to Load key documentation on how to load 3DES-ABA and 3DES-ABC keys.

Note that some systems may require to decrypt the encrypted key for CA and CP.
BHSM_GenerateRouteKey can be used for this purpose.


Calling Context:

This function shall be called after BHSM_ConfigAlgorithm configures the algorithm.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_loadRouteUserKeyIO  - BHSM_LoadRouteUserKeyIO_t.


Output:
inoutp_loadRouteUserKeyIO  - BHSM_LoadRouteUserKeyIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_FreeCAKeySlot
BHSM_AllocateCAKeySlot
BHSM_ConfigAlgorithm
BHSM_GenerateRouteKey

*****************************************************************************/
BERR_Code    BHSM_LoadRouteUserKey (
        BHSM_Handle                    hHsm,
        BHSM_LoadRouteUserKeyIO_t        *inoutp_loadRouteUserKeyIO
);



/**************************************************************************************************
Summary:

Description:
this structure defines all the data members to invalidate a BSP KeyRAM buf and/or a  key slot entry

See Also:
BHSM_InvalidateKey
**************************************************************************************************/
typedef struct BHSM_InvalidateKeyIO {

    /* In: how to invalidate,KeyRAM and/or key slotentry, as defined in BCMD_InvalidateKey_Flag_e */
    BCMD_InvalidateKey_Flag_e        invalidKeyType;

    /* In: If set to true, all entries will be invalidated, all entries for  CA/CPS/CPD, odd/even/clear, and IV. */
    bool                           bInvalidateAllEntries;

    /* In:  if bInvalidateAllEntries is true, specifies the default CRR->CRR, GLR->CRR, GLR->GLR configuration of the keyslot. */
    BCMD_ByPassKTS_e               bypassConfiguraion;

    /* In: key from which virtual key ladder to invalidate */
    BCMD_VKLID_e                virtualKeyLadderID;

    /* In: key from which key layer of the above virtual key ladder to invalidate */
    BCMD_KeyRamBuf_e            keyLayer;

    /* In: valid only if to invalidate DestKey or Both, which kind of key slot, BCMD_KeyDestBlockType_e */
    BCMD_KeyDestBlockType_e        keyDestBlckType;

    /* In: valid only if to invalidate DestKey or Both, value of the allocated key slot entry,
             BCMD_KeyDestEntryType_e */
    BCMD_KeyDestEntryType_e        keyDestEntryType;

    /* In:    SC01 mode word mapping - of type BCMD_SC01ModeWordMapping_e for Zeus 3.0 */
    BHSM_SC01ModeWordMapping_e      SC01ModeMapping;

    /* In: valid only if to invalidate DestKey or Both, value of the allocated key slot type,
        BCMD_XptSecKeySlot_e */
    BCMD_XptSecKeySlot_e            caKeySlotType;

    /* In: valid only if to invalidate DestKey or Both, value of the allocated key slot number*/
    unsigned int                    unKeySlotNum;

    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;

} BHSM_InvalidateKeyIO_t;


/*****************************************************************************
Summary:

This function is used to invalidate the intermediate key in BSP Key RAM and/or the key in key slot.

Description:

This function can be used to invalidate the intermedaite key in BSP Key RAM and/or the key in
the key slot.  The invalidated intermedaite key in BSP Key RAM cannot be used to generate the
next intermediate key.  The invalidated key in key slot cannot be used for any future (de)scrambling
operation.


Calling Context:

After key loading/generation successfully, if caller determines the key is no longer valid,
this function can be used to invalidate the key.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_invalidateKeyIO  - BHSM_InvalidateKeyIO_t


Output:
inoutp_invalidateKeyIO  - BHSM_InvalidateKeyIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_LoadRouteUserKey
BHSM_GenerateRouteKey

*****************************************************************************/
BERR_Code   BHSM_InvalidateKey (
        BHSM_Handle                hHsm,
        BHSM_InvalidateKeyIO_t        *inoutp_invalidateKeyIO
);


typedef enum BHSM_ResidueMode {
    BHSM_ResidueMode_eUnscrambled = 0,  /* the last incomplete block is clear, pass-through*/
    BHSM_ResidueMode_eResidueBlock = 1,    /* the last incomplete block is processed using Residue mode */
    BHSM_ResidueMode_eCipherTextStealing = 2,    /* the last incomplete block is processed using CipherTextStealing mode */
    BHSM_ResidueMode_eCipherStealingComcast = 3    /* the last incomplete block is processed using CipherTextStealing mode */
} BHSM_ResidueMode_e;

typedef enum BHSM_DVBScrambleLevel {
    BHSM_DVBScrambleLevel_eTS = 0,  /*  transport stream level scrambling*/
    BHSM_DVBScrambleLevel_ePes = 1   /* program element stream level scrambling */
} BHSM_DVBScrambleLevel_e;


typedef union BHSM_ResidueMode_u {
    /* In: select the termination mode among BHSM_ResidueMode_e for non-DVB algorithms*/
    BHSM_ResidueMode_e         residueMode;

    /* In: select scrambling level among  BHSM_DVBScrambleLevel_e algorithm*/
    BHSM_DVBScrambleLevel_e    dvbScrambleLevel;

} BHSM_ResidueMode_u;


typedef enum BHSM_CaModeShift {

    /* Module Key Mode Lo Shift */

    /* For Zeus 4.0+ */
    BHSM_CaModeShift_eCryptoAlgorithmShift     = 0,
    BHSM_CaModeShift_eCipherModeShift           = 5,  /* 3 bit, refer to BCMD_CipherModeSelect_e */
    BHSM_CaModeShift_eTerminationModeShift       = 8,  /* 3 bits, refer to  BHSM_ResidueMode_e/BCMD_TerminationMode_e */
    BHSM_CaModeShift_eCounterModeShift         = 8,
    BHSM_CaModeShift_eIVModeShift               = 12, /* 2 bits, refer to BCMD_IVSelect_e     */
    BHSM_CaModeShift_eCounterSizeShift         = 12,
    BHSM_CaModeShift_eSolitaryTermShift        = 14, /* 3 bits, refer to BCMD_SolitarySelect_e */
    BHSM_CaModeShift_eKeyOffsetShift           = 17,
    BHSM_CaModeShift_eIVOffsetShift            = 24,


    /* Module Key Mode Hi Shift */

    /* For Zeus 4.0+  */
    BHSM_CaModeShift_eAllowExtKeyShift          = 0,
    BHSM_CaModeShift_eAllowExtIVShift           = 1,
    BHSM_CaModeShift_eRpipeEnableShift            = 2,
    BHSM_CaModeShift_eGpipeEnableShift            = 3,
    BHSM_CaModeShift_eRpipeSCValShift           = 4,
    BHSM_CaModeShift_eGpipeSCValShift           = 6,

    /* For Zeus 4.0 */
    BHSM_CaModeShift_eRpipeFromRregionShift     = 8,
    BHSM_CaModeShift_eRpipeFromGregionShift     = 9,
    BHSM_CaModeShift_eRpipeToRregionShift       = 10,
    BHSM_CaModeShift_eRpipeToGregionShift        = 11,
    BHSM_CaModeShift_eGpipeFromRregionShift     = 12,
    BHSM_CaModeShift_eGpipeFromGregionShift     = 13,
    BHSM_CaModeShift_eGpipeToRregionShift        = 14,
    BHSM_CaModeShift_eGpipeToGregionShift        = 15,
    BHSM_CaModeShift_eMSCLengthSelectShift      = 16,
    BHSM_CaModeShift_eEncryptBeforeRaveShift    = 19,
    BHSM_CaModeShift_eCustomerNoShift           = 20,
         /* MAC group */
    BHSM_CaModeShift_eMACRegSelectShift         = 23,
         /* Multi2 group */
    BHSM_CaModeShift_eSysKeySelectShift         = 23,
    BHSM_CaModeShift_eRoundCountShift           = 29,
    BHSM_CaModeShift_eIrModEnableShift      = 23,
    BHSM_CaModeShift_eNDVBCSA3DvbCsaVarShift  = 23,
    BHSM_CaModeShift_eNDVBCSA3PermShift       = 28,
    BHSM_CaModeShift_eNDVBCSA3ModXRCShift     = 31,
    BHSM_CaModeShift_eNDVBCSA2KeyCtrlShift    = 23,
    BHSM_CaModeShift_eNDVBCSA2IVCtrlShift     = 26,
    BHSM_CaModeShift_eESDVBCSA2ModEnShift    = 23,

    /* The following Mode Shifts are for Zeus 4.1+  */
    BHSM_CaModeShift_eMSCLengthSelectShift41    = 8,
    BHSM_CaModeShift_eCustomerNoShift41         = 11,
         /* MAC group */
    BHSM_CaModeShift_eMACRegSelectShift41         = 14,
    BHSM_CaModeShift_eMACRegAllowNSRead         = 16,
         /* Multi2 group */
    BHSM_CaModeShift_eSysKeySelectShift41         = 14,
    BHSM_CaModeShift_eRoundCountShift41            = 20,
    BHSM_CaModeShift_eIrModEnableShift41    = 14,
    BHSM_CaModeShift_eNDVBCSA3DvbCsaVarShift41 = 14,
    BHSM_CaModeShift_eNDVBCSA3PermShift41         = 19,
    BHSM_CaModeShift_eNDVBCSA3ModXRCShift41       = 22,
    BHSM_CaModeShift_eNDVBCSA2KeyCtrlShift41    = 14,
    BHSM_CaModeShift_eNDVBCSA2IVCtrlShift41     = 17,
    BHSM_CaModeShift_eESDVBCSA2ModEnShift41    = 14

} BHSM_CaModeShift_e;

/* The following is for Zeus 4.1+ */

typedef enum BHSM_GlobalModeShift_e
{
    BHSM_GlobalModeShift_eInputRegion           = 0,
    BHSM_GlobalModeShift_eRpipeOutputRegion     = 8,
    BHSM_GlobalModeShift_eGPipeOutputRegion     = 16,
    BHSM_GlobalModeShift_eEncryptBeforeRave     = 24

} BHSM_GlobalModeShift_e;


typedef enum BHSM_M2mAuthCtrl {
    BHSM_M2mAuthCtrl_ePassThrough = 0x00, /* no any M2M processing, just by pass*/
    BHSM_M2mAuthCtrl_eScramble    = 0x01, /* M2M scrambling/encryption*/
    BHSM_M2mAuthCtrl_eDescramble  = 0x02  /* M2M descrambling/decryption*/


} BHSM_M2mAuthCtrl_e;

/*  Region bit map for G-pipe output, R-pipe output, input to G-pipe and R-pipe */
/*  To be used for Zeus 4.1+    */
#define BHSM_G_REGION           0x01
#define BHSM_R_REGION_0         0x02
#define BHSM_R_REGION_1         0x04
#define BHSM_R_REGION_2         0x08
#define BHSM_R_REGION_4         0x10
#define BHSM_R_REGION_5         0x20
#define BHSM_R_REGION_6         0x40
#define BHSM_R_REGION_7         0x80

#define BHSM_ALL_GLOBAL_REGION_MAP ( (BHSM_G_REGION << BHSM_GlobalModeShift_eInputRegion      )  | \
                                    ((BHSM_G_REGION | BHSM_R_REGION_0) << BHSM_GlobalModeShift_eRpipeOutputRegion)  | \
                                    ((BHSM_G_REGION | BHSM_R_REGION_0) << BHSM_GlobalModeShift_eGPipeOutputRegion)  )



/**************************************************************************************************
Summary:

Description:
this structure is a sub member in BHSM_ConfigAlgorithmIO.  All the setting will be translated into word32
format and send to BSP configuring the key slot header area controlling bits.



See Also:
BHSM_ConfigAlgorithmIO
**************************************************************************************************/
typedef struct BHSM_XPTCryptoAlgConfig
{
    /*  If M2M DMA, mode of operation -- To select either CPS or CPD */
    BHSM_M2mAuthCtrl_e              cryptoOp;

        /* Module Key Mode Lo Setting */
    /* In: select the algorithm among BCMD_XptM2MSecCryptoAlg_e */
    BCMD_XptM2MSecCryptoAlg_e         xptSecAlg;

    /* In:  set cipher mode for non-DVB algorithms or scrambling level for DVB-CSA2, see BCMD_CipherModeSelect_e */
    BCMD_CipherModeSelect_e            cipherDVBCSA2Mode;

    /* In:  Termination Mode or Counter Mode for selected algorithm */
    BCMD_TerminationMode_e            termCounterMode;

    /* In:  IV Mode or Counter Size for the selected algorithm */
    BCMD_IVSelect_e                    IVModeCounterSize;

    /* In: Mode for solitary block processing */
    BCMD_SolitarySelect_e            solitaryMode;

    /* In: offset of first word of key with respect to start of key slot, in 64 bit words */
    unsigned char                    keyOffset;


    /* In: offset of first word of IV with respect to start of key slot, in 64-bit words */
    unsigned char                    ivOffset;

          /* Module Key Mode Hi Setting */

    /* In: allow key to be from external source rather than key serializer */
    bool                            bUseExtKey;

    /* In: allow IV from external source rather than key serializer */
    bool                            bUseExtIV;


    /* In: 1 - Enable scrambling/descrambling of restricted packets (R-pipe) */
    bool                            bRpipeEnable;

    /* In: 1 - Enable scrambling/descrambling of global packets (G-pipe) */
    bool                            bGpipeEnable;

    /* In: SC value to replace SC field in restricted packets */
    unsigned char                    RpipeSCVal;

    /* In: SC value to replace SC field in global packets */
    unsigned char                    GpipeSCVal;

    /* The following 9 fields are not used for Zeus 4.1+ */

    /* In: 1 - using this keyslot enable packets from R-region into R-pipe */
    bool                            bRpipeFromRregion;

    /* In: 1 - using this keyslot enable packets from G-region into R-pipe */
    bool                            bRpipeFromGregion;

    /* In: 1 - using this keyslot enable packets in R-pipe into R-region */
    bool                            bRpipeToRregion;

    /* In: 1 - using this keyslot enable packets in R-pipe into G-region */
    bool                            bRpipeToGregion;

    /* In: 1 - using this keyslot enable packets from R-region into G-pipe */
    bool                            bGpipeFromRregion;

    /* In: 1 - using this keyslot enable packets from G-region into G-pipe */
    bool                            bGpipeFromGregion;

    /* In: 1 - using this keyslot enable packets in G-pipe into R-region */
    bool                            bGpipeToRregion;

    /* In: 1 - using this keyslot enable packets in G-pipe into G-region */
    bool                            bGpipeToGregion;


    /* In: 1 - encrypt packets destined for RAVE in CPS, before RAVE */
    bool                            bEncryptBeforeRave;

    /* In: Select length used in MSC algorithm, 0-3 for DTV packets; 0-7 for MPEG packets */
    unsigned char                    MSCLengthSelect;

    /* In: Customer number used for special processing  */
    BCMD_XptKeyTableCustomerMode_e    customerType;

        /* MAC group */
    /* In: CMAC/CBCMAC output register select (1 out of 4 128 bit registers */
    unsigned char                   MACRegSelect;
    bool                            MACNonSecureRegRead;      /* For Zeus 4.1+ */

        /* Multi2 group */
    /* In: Multi2 system key destination - Select 1 of 64 Multi2 system key*/
    unsigned char                    Multi2SysKeySelect;

    /* In: Multi2 round count */
    unsigned char                   Multi2RoundCount;

    /* In: modifier enabled */
    bool                            IdertoModEnable;

    unsigned char                    DVBCSA3dvbcsaVar;
    unsigned char                    DVBCSA3permutation;
    bool                            DVBCSA3modXRC;

    unsigned char                    DVBCSA2keyCtrl;
    unsigned char                    DVBCSA2ivCtrl;
    bool                            DVBCSA2modEnabled;

    /* In: modifier enabled */
    bool                            EsModEnable;


}BHSM_XPTCryptoAlgConfig_t;





/**************************************************************************************************
Summary:

Description:
this structure defines what (de)scrambling algorithm, control bits, key source are needed to configure
for a key slot entry.

See Also:
BHSM_ConfigAlgorithm
**************************************************************************************************/
typedef struct BHSM_ConfigAlgorithmIO
{
    /* In: the key source is in which block of CA or M2M keytable,  see BCMD_KeyDestBlockType_e*/
    BCMD_KeyDestBlockType_e            keyDestBlckType;

    /* In: the key source is in which key slot entry as defined in BCMD_KeyDestEntryType_e*/
    BCMD_KeyDestEntryType_e            keyDestEntryType;

    /* In: and Which IV type */
    BCMD_KeyDestIVType_e            keyDestIVType;

    /* In: Multi2 key Config */
    bool                            bMulti2KeyConfig;

    /* In: the key source is of what key slot type*/
    BCMD_XptSecKeySlot_e            caKeySlotType;

    /* In: the key source is in which key slot (key slot number) */
    unsigned int                    unKeySlotNum;

    /* In: control bits in the key slot header area are configured as set here  */
    BHSM_XPTCryptoAlgConfig_t        cryptoAlg;



    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;


} BHSM_ConfigAlgorithmIO_t;


/*****************************************************************************
Summary:

This function is used to configure (de)scrambling algorithms and various control bits
for a specified key slot.


Description:

This function is used to configure (de)scrambling algorithms and various control bits
for a specified key slot for CA, M2M, HDMI and REMUX modules.

Key slot of certain destination, for example CA, may contain two key entries (odd and even).
Each key entry may use different algorithms or control bits.  This function
can be used to configure algorithm and control bits for one key entry at a time.

Refer to <<Keyslot CA/CP Implementation documentation>> .


Calling Context:

This function shall be called before key loading/generation function, such as BHSM_LoadRouteUserKey and
BHSM_GenerateRouteKey. Key in the slot will be invalidated after calling this function.
This function is recommended to be called after BHSM_ConfigPidKeyPointerTable.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
pConfigAlgorithm  - BHSM_ConfigAlgorithmIO_t.

Output:
pConfigAlgorithm  - BHSM_ConfigAlgorithmIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_LoadRouteUserKey
BHSM_GenerateRouteKey
BHSM_AllocateCAKeySlot

*****************************************************************************/
BERR_Code BHSM_ConfigAlgorithm (
        BHSM_Handle                    hHsm,
        BHSM_ConfigAlgorithmIO_t    *pConfigAlgorithm
);


/**************************************************************************************************
Summary:

Description:
this structure describe the key slot entry whose configuration/algorithm settings are requested

See Also:
BHSM_KeySlotInfo
**************************************************************************************************/
typedef struct BHSM_KeySlotInfo
{
    /* In: the key source is in which block of CA or M2M keytable,  see BCMD_KeyDestBlockType_e*/
    BCMD_KeyDestBlockType_e            keyDestBlckType;

    /* In: the key source is in which key slot entry as defined in BCMD_KeyDestEntryType_e*/
    BCMD_KeyDestEntryType_e            keyDestEntryType;

    /* In: the key source is of what key slot type*/
    BCMD_XptSecKeySlot_e            caKeySlotType;

    /* In: the key source is in which key slot (key slot number) */
    unsigned int                    unKeySlotNum;

    /* out: control bits in the key slot header area are configured as set here,BHSM_CryptoAlg_u  */
    BHSM_XPTCryptoAlgConfig_t        cryptoAlg;

} BHSM_KeySlotInfo_t;




/*****************************************************************************
Summary:

This function is used to retrieve (de)scrambling algorithms and various control bits
for a specified key slot.


Description:

This function is used to retrieve (de)scrambling algorithms and various control bits
for a specified key slot for CA, M2M, HDMI and REMUX modules.

Key slot of certain destination, for example CA, may contain two key entries (odd and even).
Each key entry may use different algorithms or control bits.  This function
can be used to retrieve algorithm and control bits for one key entry at a time.


Calling Context:

This function shall be called after the specified key slot is configured with  BHSM_ConfigAlgorithm()


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_KeySlotInfoIO  - BHSM_KeySlotInfo_t.

Output:
inoutp_KeySlotInfoIO  - BHSM_KeySlotInfo_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_LoadRouteUserKey
BHSM_GenerateRouteKey
BHSM_AllocateCAKeySlot

*****************************************************************************/
BERR_Code BHSM_GetKeySlotConfigAlgorithm (
        BHSM_Handle                hHsm,
        BHSM_KeySlotInfo_t        *inoutp_KeySlotInfoIO
);




typedef struct BHSM_ConfigKeySlotIDDataIO
{
    /* In: the key source is in which block of CA or M2M keytable,  see BCMD_KeyDestBlockType_e*/
    BCMD_KeyDestBlockType_e        keyDestBlckType;

    /* In: the key source is in which key slot entry as defined in BCMD_KeyDestEntryType_e*/
    BCMD_KeyDestEntryType_e        keyDestEntryType;

    /* In: and Which IV */
    BCMD_KeyDestIVType_e        keyDestIVType;

    /* In: the key source is of what key slot type*/
    BCMD_XptSecKeySlot_e        caKeySlotType;

    /* In: the key source is in which key slot (key slot number) */
    unsigned int                unKeySlotNum;

    /* In: STB Owner ID Select */
    BCMD_STBOwnerID_e            STBOwnerIDSelect;

    /* In: CA Vendor ID */
    uint32_t                    CAVendorID;

    /* In: Module ID Select */
    BCMD_ModuleID_e                ModuleID;

    /* In: Select Key2 type */
    BCMD_ASKM_MaskKeySel_e      key2Select;

} BHSM_ConfigKeySlotIDDataIO_t;



/*****************************************************************************
Summary:

This function is used to gather ID data for a specified key slot.


Description:

Calling Context:

This function shall be called before key loading/generation function, such as BHSM_LoadRouteUserKey and
BHSM_GenerateRouteKey. Key in the slot will be invalidated after calling this function.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm                              - BHSM_Handle, Host Secure module handle.
inoutp_configKeySlotIDDataIO   - BHSM_ConfigKeySlotIDDataIO_t.

Output:


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_LoadRouteUserKey
BHSM_GenerateRouteKey
BHSM_AllocateCAKeySlot

*****************************************************************************/
BERR_Code BHSM_ConfigKeySlotIDData (
        BHSM_Handle                        hHsm,
        BHSM_ConfigKeySlotIDDataIO_t    *inoutp_configKeySlotIDDataIO
);


typedef struct BHSM_KeySlotGlobalCntrlWord
{
    /* In: the key source is of what key slot type*/
    BCMD_XptSecKeySlot_e        caKeySlotType;

    /* In: the key source is in which key slot (key slot number) */
    unsigned int                unKeySlotNum;

    /* In: Input region(s) assigned to this key slot */
    unsigned char               inputRegion;

    /* In: The region(s) connected to Rpipe output */
    unsigned char               RpipeOutput;

    /* In: The region(s) connected to Gpipe output */
    unsigned char               GpipeOutput;

    /* In: if packets need to be encrypted before RAVE */
    bool                        encryptBeforeRAVE;

} BHSM_KeySlotGlobalCntrlWord_t;



/*****************************************************************************
Summary:

This function is used to gather Global control word settings (Low) for a specified key slot.
These settings are applied to all key destinations (CA/CPS/CPD) and key slot types (Odd, Even, Clear, IV).


Description:

Calling Context:

This function shall be called before key loading/generation function, such as BHSM_LoadRouteUserKey and
BHSM_GenerateRouteKey.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm                                 - BHSM_Handle, Host Secure module handle.
inoutp_configKeySlotIDDataIO   - BHSM_ConfigKeySlotIDDataIO_t.

Output:


Returns:
BERR_SUCCESS           - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_LoadRouteUserKey
BHSM_GenerateRouteKey

*****************************************************************************/
BERR_Code BHSM_ConfigKeySlotGlobalCntrlWord (
        BHSM_Handle                        hHsm,
        BHSM_KeySlotGlobalCntrlWord_t    *inoutp_configKeySlotGlobalCntrlWord
);




#define BHSM_MULTI2_SYS_KEY_SIZE     32


/**************************************************************************************************
Summary:

Description:
this structure defines data members to configure Multi2 algorithm which is different from the
configuration of all the other algorithms

See Also:
BHSM_ConfigMulti2
**************************************************************************************************/
typedef struct BHSM_ConfigMulti2IO {

    /* In: Multi round counter value  */
    unsigned char                ucMulti2RndCnt;

    /* In: MULTI2 system key seed, big endian array, 32 bytes*/
    unsigned char                aucMulti2SysKey[BHSM_MULTI2_SYS_KEY_SIZE];

    /* In: which system key to configure, HW specific */
    unsigned char                ucSysKeyDest;

    /* Out: 0 for success, otherwise failed */
    uint32_t                    unStatus;


} BHSM_ConfigMulti2IO_t;



/*****************************************************************************
Summary:

This function is used to configure MULTI2 CA and load the system key.

Description:

This function can be used to configure the round count and system key destination for
the MULTI2 CA. This function also loads MULTI2 system key.

This function should be used together with a procedure of a regular CA key generation using CA
Keyladder, and the skeleton of function call sequence includes
      BHSM_AllocateCAKeySlot
      BHSM_ConfigPidKeyPointerTable
      BHSM_ConfigAlgorithm(odd)
      BHSM_ConfigAlgorithm(even)
      BHSM_GenerateRouteKey(odd)
      BHSM_GenerateRouteKey(even)
      BHSM_LoadRouteUserKey(IV for CBC mode)
      BHSM_ConfigMulti2


Calling Context:

It's called after a regular CA key generation, befored descrambling actual CA data stream.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_configMulti2IO  - BHSM_ConfigMulti2IO_t

Output:
inoutp_configMulti2IO  - BHSM_ConfigMulti2IO_t

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_ConfigAlgorithm
BHSM_GenerateRouteKey
BHSM_AllocateCAKeySlot

*****************************************************************************/
BERR_Code   BHSM_ConfigMulti2 (
        BHSM_Handle             hHsm,
        BHSM_ConfigMulti2IO_t  *pConfigMulti2
);



/**************************************************************************************************
Summary:

Description:

See Also:
**************************************************************************************************/
typedef struct  {
    /* In:  Which client requests for xpt key slot -- Zeus 4.0+   */
    BHSM_ClientType_e               client;

    /* In:  Which key slot type to request for   */
    BCMD_XptSecKeySlot_e            keySlotType;

    #if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
    /* In: PID channel associated with this key slot - For For BHSM_FreeCAKeySlot() */
    uint32_t                        pidChannel;
    #endif

    /* In: PID Channel type - For For BHSM_FreeCAKeySlot()   */
    BHSM_PidChannelType_e            pidChannelType;

    /* Out: Xpt key slot number allocated */
    /* In:   For BHSM_FreeCAKeySlot()    */
    uint32_t                        keySlotNum;

    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;


} BHSM_XptKeySlotIO_t, BHSM_KeySlotAllocate_t;


/*****************************************************************************
Summary:

This function is widely used (almost in each CA/RMX procedure) to allocate a key slot for
CA/RMX.

Description:

This function does not require any BSP operation. This function is used by host to keep track
of which CA/RMX key slot can be allocated for CA/RMX operation.


Calling Context:

This function should be called after BHSM_InitKeySlot and before calling BHSM_ConfigPidKeyPointerTable
to associate the pid channel with the CA/RMX key slot.


Performance and Timing:
This is an synchronous/blocking function, and shall return immediately since this does not
communicate with BSP.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
in_keySlotType  - BCMD_XptSecKeySlot_e,  Support BCMD_XptSecKeySlot_eType0/1/2/3/4/5/6.
        However, the specified key slot type should have at least 1  key slot allocation
        specified in BHSM_InitKeySlot function.

Output:
outp_unKeySlotNum -unsigned int *, the allocated key slot number

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_FreeCAKeySlot
BHSM_AllocateM2MKeySlot

*****************************************************************************/
BERR_Code   BHSM_AllocateCAKeySlot (
        BHSM_Handle             hHsm,
        BHSM_XptKeySlotIO_t    *pKeySlotConf
);

/*****************************************************************************
Summary:

This function is used to free a CA/RMX key slot.

Description:

This is a complementary operation of BHSM_AllocateCAKeySlot. This function is used to free a CA/RMX
key slot. This function is used by host to keep track of which key slot can be freed for CA/RMX
operation.


Calling Context:

This function should be called after BHSM_AllocateCAKeySlot.


Performance and Timing:
This is an synchronous/blocking function, and shall return immediately since this does not
communicate with BSP.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
in_unPidChannel - unsigned int,  the pid channel to be de-associated
in_pidChannelType - BHSM_PidChannelType_e, the pid channel is in primary PID channel
                    or secondary PID channel
in_keySlotType  - BCMD_XptSecKeySlot_e,  Support BCMD_XptSecKeySlot_eType0/1/2/3/4/5/6.
in_unKeySlotNum -unsigned int, which key slot to free

Output:
none

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_AllocateCAKeySlot
BHSM_FreeM2MKeySlot

*****************************************************************************/
BERR_Code   BHSM_FreeCAKeySlot (
    BHSM_Handle          pHsm,
    BHSM_XptKeySlotIO_t *pKeySlotConf

);


typedef struct BHSM_KeySlotOwnership_s{

    unsigned                keySlotNumber;   /* In: The keyslot number     */
    BCMD_XptSecKeySlot_e    keySlotType;     /* In: The keyslot type       */

    BHSM_KeySlotOwner_e     owner;           /* In/Out: The keyslot owner  */

}BHSM_KeySlotOwnership_t;


/*****************************************************************************
    BHSM_SetKeySlotOwnership sets the owner of the specified keyslot.
*****************************************************************************/
BERR_Code BHSM_SetKeySlotOwnership(
    BHSM_Handle              hHsm,
    BHSM_KeySlotOwnership_t *pConfig
    );

/*****************************************************************************
    BHSM_GetKeySlotOwnership gets the owner of the specified keyslot.
*****************************************************************************/
BERR_Code BHSM_GetKeySlotOwnership(
    BHSM_Handle              hHsm,
    BHSM_KeySlotOwnership_t *pConfig
    );


/*DEPRECATED ... use BERR_Code BHSM_SetKeySlotOwnership */
BERR_Code BHSM_SetKeySlotOwner(
    BHSM_Handle            hHsm,
    unsigned               keySlotNumber,
    BCMD_XptSecKeySlot_e   keySlotType,
    BHSM_KeySlotOwner_e    owner
    );
/*DEPRECATED ... use BERR_Code BHSM_GetKeySlotOwnership */
BERR_Code BHSM_GetKeySlotOwner (
    BHSM_Handle             hHsm,
    unsigned                keySlotNumber,
    BCMD_XptSecKeySlot_e    keySlotType,
    BHSM_KeySlotOwner_e     *pOwner
    );

/*****************************************************************************
Summary:

This function is used to retrieve the key slot type and key slot number of that key slot associated
with a specified PID channel.

Description:

Based on PID channel (e.g. 0~127) and  pid channel type(primary or secondary), this
function will determine the previously associated key slot type/number. No allocation of a
new key slot is required if a key slot has been associated with the specifed pid channel.

Calling Context:

This function may be called any time after BHSM_AllocateCAKeySlot is called.


Performance and Timing:
This is an synchronous/blocking function, and shall return immediately since this does not
communicate with BSP.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
in_unPidChannel - unsigned int,  the linked pid channel
in_pidChannelType - BHSM_PidChannelType_e, the pid channel is in primary PID table or secondary PID table

Output:
outp_ucKeySlotType  - BCMD_XptSecKeySlot_e *,  hold the returned key slot type
outp_unKeySlotNum -unsigned int *, hold the returned key slot number

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_AllocateCAKeySlot
BHSM_FreeCAKeySlot

*****************************************************************************/
BERR_Code  BHSM_LocateCAKeySlotAssigned (
        BHSM_Handle                    hHsm,
        unsigned int                    in_unPidChannel,
        BHSM_PidChannelType_e        in_pidChannelType,
        BCMD_XptSecKeySlot_e            *outp_ucKeySlotType,
        unsigned int                    *outp_unKeySlotNum
);


/**************************************************************************************************
Summary:

Description:

See Also:
**************************************************************************************************/
typedef struct BHSM_M2MKeySlotIO {
    /* In:  Which client requests for M2M key slot   */
    BHSM_ClientType_e               client;

    /* In:  Which key slot type to request for  -- For Zeus 4.X support */
    BCMD_XptSecKeySlot_e            keySlotType;

    /* Out: 0 for success, otherwise failed */
    uint32_t                        unStatus;

    /* Out: M2M key slot number allocated */
    uint32_t                        keySlotNum;


} BHSM_M2MKeySlotIO_t;



/*****************************************************************************
Summary:

This function is used to allocate a M2M key slot.

Description:


There is a piece of hardware storage allocated for M2M key slots.  The maximum key slot size is
96 (Refer to BCMD_MEM2MEMKEYTABLE_TOP) entries, where each entry is 64 bits. The total size could
be varied from chip to chip.  However, each M2M key slot should consume 6 entries. So we should
have a total of 16 M2M key slots.

Each M2M key slot can store up to 192-bit key plus 128-bit IV for CBC mode.  The key can
be loaded with control word or generated securely via key ladder.  Unlike CA key slot, there is no
odd/even key entry in the M2M key slot.

This function does not require any BSP operation. This function is used by host to keep track
of which M2M key slot can be allocated for M2M operation. M2M key table doesn't require
key slot initialization.

The list of algorithms implemeted in M2M module could be different from those of CA. Yet,
M2M key slot and CA/RMX key slot are using BHSM_ConfigAlgorithm to configure the algorithms.

M2M can support MPEG2 payload, Block mode for entire data region or customer specifc packet mode
scrambling/descrambling. A key for descrambling/scrambling possibly with an IV in M2M slot shall
be used for entire M2M operation.


Calling Context:

This function should be called before calling BHSM_ConfigAlgorithm to configure any M2M algorithm
and control bits.


Performance and Timing:
This is an synchronous/blocking function, and shall return immediately since this does not
communicate with BSP.


Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_M2MKeySlotIO -BHSM_M2MKeySlotIO_t, to hold the allocated key slot number

Output:
inoutp_M2MKeySlotIO -BHSM_M2MKeySlotIO_t, the allocated key slot number

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_FreeM2MKeySlot
BHSM_FreeCAKeySlot
BHSM_ConfigAlgorithm

*****************************************************************************/
BERR_Code    BHSM_AllocateM2MKeySlot (
        BHSM_Handle                    hHsm,
        BHSM_M2MKeySlotIO_t            *inoutp_M2MKeySlotIO
);



/*****************************************************************************

Summary:

This function is used to free a M2M key slot.


Description:

This is a complemetary operation of BHSM_AllocateM2MKeySlot. This function is used to
free a M2M key slot.  This function is used by host to keep track of which key slot can be
freed for M2M operation.

Calling Context:

This function should be called after BHSM_AllocateM2MKeySlot.


Performance and Timing:
This is an synchronous/blocking function, and shall return immediately since this does not
communicate with BSP.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
in_unKeySlotNum - unsigned int, which key slot to free

Output:
none

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_AllocateM2MKeySlot
BHSM_FreeCAKeySlot

*****************************************************************************/
BERR_Code  BHSM_FreeM2MKeySlot (
        BHSM_Handle                    hHsm,
    BHSM_M2MKeySlotIO_t       *inoutp_M2MKeySlotIO
);


/*****************************************************************************
Summary:

This function re-associate the selected PID channel to a predefined default key slot that has
default control bits and all keys are set to NULL.

Description:

This function is exactly the same as the BHSM_ConfigPidKeyPointerTable() function, except for linking
the selected PID channel to a predefined default key slot that has default control bits and all
keys are set to NULL.

Certain pid channels may be used to descramble audio/video stream.  After this function is called,
the same pid channels can be re-used to play clear audio/video stream.


Calling Context:

This function can be called after BHSM_ConfigPidKeyPointerTable.

Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.
inoutp_configPidKeyPointerTableIO  - BHSM_ConfigPidKeyPointerTableIO_t.


Output:
inoutp_configPidKeyPointerTableIO  - BHSM_ConfigPidKeyPointerTableIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ConfigPidKeyPointerTable

*****************************************************************************/
BERR_Code  BHSM_ConfigPidChannelToDefaultKeySlot (
        BHSM_Handle                        hHsm,
        BHSM_ConfigPidKeyPointerTableIO_t    *inoutp_configPidKeyPointerTableIO
);



BERR_Code BHSM_InvalidateTransportKeySlots(
    BHSM_Handle         hHsm,
    BHSM_ClientType_e   ownerShip
);

/*
    Configure/reconfigure  both the g2gr and gr2r bypass keyslots.
        - This function will be called internally by HSM moduele during initialisation.
          It needs to be re-called if the client performs an operation that would invalidate
          the bypass keyslots.
*/

BERR_Code BHSM_InitialiseBypassKeyslots( BHSM_Handle hHsm );



/**
Summary:

All pid channels must be associated with a keyslot. Pid channels handing encrtpted data will have a
regular client managed keyslot assocaiated with it.

For pid channels handeling clear data the system will default to a Bypass keyslot for capable of
transferring data from GLR to GLR.

If a pid channel is handling clear data and has a destination in CRR (with source CRR or GLR), this function
must be called to associated it with the BHSM_BypassKeySlot_eGR2R_e Bypass Keyslot.

**/
BERR_Code  BHSM_SetPidChannelBypassKeyslot(
    BHSM_Handle hHsm,
    unsigned pidChannelIndex,
    BHSM_BypassKeySlot_e bypassKeyslot
);

BERR_Code  BHSM_GetPidChannelBypassKeyslot(
    BHSM_Handle hHsm,
    unsigned pidChannelIndex,
    BHSM_BypassKeySlot_e *pBypassKeyslot
);

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
/**************************************************************************************************
Summary:

Description:
this structure defines the retrieved ownership states (FREE, SAGE, or SHARED) for all key slots.

See Also:
BHSM_QueryKeySlotsStatus
**************************************************************************************************/

/* Currently status query supports up to eType5, to be update to BCMD_XptSecKeySlot_eTypeMax. */
#define BHSM_KeySlotStatusQuery_eTypeMax (BCMD_XptSecKeySlot_eType5)

typedef struct{

       uint16_t numKeyslots; /*total number of slots*/
       struct {
              BCMD_XptSecKeySlot_e    type;
              uint16_t number;
              BHSM_KeySlotOwner_e owner;
       }slot[BHSM_MAX_KEYLSOTS];

} BHSM_KeySlotsStatus_t;


/*****************************************************************************
Summary:

This function retrieves the ownership states (FREE, SAGE, or SHARED) for all key slots in a single call.

Description:


Calling Context:

This function can be called by HOST and Sage.

Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.

Input:
hHsm  - BHSM_Handle, Host Secure module handle.


Output:
BHSM_KeySlotsStatus_t - the key slots' owership status.


Returns:
BERR_SUCCESS - success

See Also:
BHSM_KeySlotsStatus_t

*****************************************************************************/
BERR_Code BHSM_QueryKeySlotsStatus(
        BHSM_Handle           hHsm,
        BHSM_KeySlotsStatus_t  *pKeySlotsStatus    /*output, the types and keyslot numbers for each ownership. */
);

#endif /* #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2) */


/* End of Module Specific Functions */


#ifdef __cplusplus
}
#endif

#endif /* BHSM_KEYSLOTS_H__ */
