/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef BHSM_KEYLADDER_H__
#define BHSM_KEYLADDER_H__


/* added to remove header file dependency */
#include "bhsm_priv.h"



#ifdef __cplusplus
extern "C" {
#endif




/* Module Specific Functions */


/*** add new enums in this section if appliccable to all chips/versiona && all customer modes START*******/

/* AES128 counter mode, counter length enum */
typedef enum BCMD_Aes128_CounterSize_e {
    BCMD_AesCounterSize_e32=0,   /* fixed to b'00, 01, 10, 11*/
    BCMD_AesCounterSize_e64=1,
    BCMD_AesCounterSize_e96=2,
    BCMD_AesCounterSize_e128=3,
    BCMD_AesCounterSize_eInvalid
		
}BCMD_Aes128_CounterSize_e;

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

	
	uint32_t			unKeySlotType0Num;	 /* In: how many key slots are required for type0   */
	uint32_t			unKeySlotType1Num;	/* In: how many key slots are required for type1   */
	uint32_t			unKeySlotType2Num;	/* In: how many key slots are required for type2   */
	uint32_t			unKeySlotType3Num; /* In: how many key slots are required for type3   */
	uint32_t			unKeySlotType4Num; /* In: how many key slots are required for type4   */
	uint32_t			unKeySlotType5Num; /* In: how many key slots are required for type5   */
	uint32_t			unKeySlotType6Num; /* In: how many key slots are required for type6   */


	
	uint32_t			unStatus;  			/* Out: 0 for success, otherwise failed */		

	
} BHSM_InitKeySlotIO_t;

/*****************************************************************************
Summary:

This function allocates specified number of CA key slots for each key slot type.

Description:

There is a piece of hardware storage allocated for CA key slots.  The key slot table has 1530
(Refer to BCMD_XPTSECKEYTABLE_TOP) entries, where each entry has 64 bits. The 
total size could be varied from chip to chip.

There are 7 types of key slot being defined.  Each key slot in different key slot type consists 
of different number of entries as follow:
	Key slot type 0 always has 10 entries.
	Key slot type 1 always has 12 entries.
	Key slot type 2 always has 14 entries.
	Key slot type 3 always has 16 entries.
	Key slot type 4 always has 16 entries.
	Key slot type 5 always has 20 entries.
	Key slot type 6 always has 26 entries.

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

in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_initKeySlotIO  - BHSM_InitKeySlotIO_t

			
Output:

inoutp_initKeySlotIO -BHSM_InitKeySlotIO_t, 


Returns:

BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_AllocateCAKeySlot
BHSM_FreeCAKeySlot


*****************************************************************************/
BERR_Code   BHSM_InitKeySlot (
		BHSM_Handle			in_handle,
		BHSM_InitKeySlotIO_t	*inoutp_initKeySlotIO
);


/* Special dummy pid channel number is used as the safe required channel number parameter 
    being passed to BHSM_ConfigPidKeyPointerTable():
	 (1) if CA keyslot is allocated, yet no further configuration for any pid channel to consume it, 
            later when such keyslot needs to be freed, or
       (2) if the same pid channel is re-configured  to a different keyslot_b from keyslot_a, then the application needs to 
            free keyslot_a.  
*/
#define  DUMMY_PID_CHANNEL_TOAKEYSLOT   (BCMD_TOTAL_PIDCHANNELS)

/**************************************************************************************************
Summary:

Description:
this structure defines which PID channel to assocaite with which allocated key slot

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_PidChannelType_e
**************************************************************************************************/ 
typedef struct BHSM_ConfigPidKeyPointerTableIO {
	/* In:  which PID channcel is to be associated with an allocated key slot, 0~127   */	
	unsigned int					unPidChannel;

	/* In:   slot type of the allocated key slot , 0~6 at max depending on keyslot  initializaiton*/
	BCMD_XptSecKeySlot_e			ucKeySlotType;

	/* In:   slot number of the allocated key slot , depending on keyslot initializaiton*/ 
	unsigned int					unKeySlotNum;

	/* In:  the channcel is in primary or secondary PID channel table, BHSM_PidChannelType_e */ 
	BHSM_PidChannelType_e		pidChannelType;

	/* In:  conditional, only needed if a 2nd key slot is needed for the PID channel, slot type value*/
	uint32_t						unKeySlotB;

	/* In:  conditional, only needed if a 2nd key slot is needed for the PID channel, slot number value*/
	uint32_t						unKeySlotNumberB;

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
	uint32_t						unKeyPointerSel;	

	/* Out: 0 for success, otherwise failed */
	uint32_t						unStatus;

	
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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle						in_handle,
		BHSM_ConfigPidKeyPointerTableIO_t	*inoutp_configPidKeyPointerTableIO
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
	/* In:  load into which BSP KeyRAM buffer, a subset of BCMD_KeyRamBuf_e can be used
	           BCMD_KeyRamBuf_eFirstRam~BCMD_KeyRamBuf_eThirdRam for a key up to 192 bits
	           BCMD_KeyRamBuf_eFirstRam2048~BCMD_KeyRamBuf_eThirdRam2048 for a key up t0 2048bits
	*/
	BCMD_KeyRamBuf_e			keySource;
	
	/* In: the key size, either 64/128/192 bits as in BCMD_KeySize_e or 192~2048bits  */
	BCMD_KeySize_u				keySize;

	/* In: the actual key to be loaded, big endian order, starting from the 1st byte, padding at bottom */
	unsigned char 				aucKeyData[BCMD_USERKEY_KEYRAM2048_SIZE]; 

	/* In: need to further roue this key from BSP KeyRAM to keyslot? true/yes, false/no(only in buf)*/
	bool							bIsRouteKeyRequired;

	/* In: which kind of key slot destination, CA/RMX/M2M/HDMI/etc, see BCMD_KeyDestBlockType_e*/
	BCMD_KeyDestBlockType_e		keyDestBlckType;

	/* In: which entry type of key destination, odd/even key entry or IV entry or else, see 
		BCMD_KeyDestEntryType_e
	*/
	BCMD_KeyDestEntryType_e		keyDestEntryType;

	/* In:  value of the allocated key slot type  */
	BCMD_XptSecKeySlot_e			caKeySlotType;

	/* In:  value of the allocated key slot number */
	unsigned int					unKeySlotNum;

	/* In:  which mode to use the key, BCMD_KeyMode_eRegular normally, see BCMD_KeyMode_e */
	BCMD_KeyMode_e				keyMode;
	
	/* Out: 0 for success, otherwise failed */
	uint32_t						unStatus;
	
} BHSM_LoadRouteUserKeyIO_t;


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

The loaded key will be kept valid in the key slot until either the key slot is invalidated by
BHSM_InvalidateKey function or the chip is reset. The loaded algorithm and control bits will be 
kept in the key slot until either the key slot is reset by BHSM_ResetKeySlotCtrlBits or the 
chip is reset.

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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle					in_handle,
		BHSM_LoadRouteUserKeyIO_t		*inoutp_loadRouteUserKeyIO
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
	BCMD_InvalidateKey_Flag_e		invalidKeyType;  
	
	/* In: valid only if to invalidate KeyRAM or Both, select from a subset of BCMD_KeyRamBuf_e, 0 to 11, 
		 i.e. BCMD_KeyRamBuf_eFirstRam ~ BCMD_KeyRamBuf_eThirdRam
	  		BCMD_KeyRamBuf_eKey3KeyLadder1 ~ BCMD_KeyRamBuf_eKey5KeyLadder3
	*/
	BCMD_KeyRamBuf_e			keySrc;		
	
	/* In: valid only if to invalidate DestKey or Both, which kind of key slot, BCMD_KeyDestBlockType_e */
	BCMD_KeyDestBlockType_e		keyDestBlckType;	

	/* In: valid only if to invalidate DestKey or Both, value of the allocated key slot entry,
	         BCMD_KeyDestEntryType_e */
	BCMD_KeyDestEntryType_e		keyDestEntryType;	
	
	/* In: valid only if to invalidate DestKey or Both, value of the allocated key slot type,
		BCMD_XptSecKeySlot_e */
	BCMD_XptSecKeySlot_e			caKeySlotType;	

	/* In: valid only if to invalidate DestKey or Both, value of the allocated key slot number*/
	unsigned int					unKeySlotNum;  	

	/* Out: 0 for success, otherwise failed */
	uint32_t						unStatus;
	
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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle				in_handle,
		BHSM_InvalidateKeyIO_t		*inoutp_invalidateKeyIO
);


typedef enum BHSM_ResidueMode {
	BHSM_ResidueMode_eUnscrambled = 0,  /* the last incomplete block is clear, pass-through*/
	BHSM_ResidueMode_eResidueBlock = 1,	/* the last incomplete block is processed using Residue mode */	
	BHSM_ResidueMode_eCipherTextStealing = 2,	/* the last incomplete block is processed using CipherTextStealing mode */
	BHSM_ResidueMode_eCipherStealingComcast = 3	/* the last incomplete block is processed using CipherTextStealing mode */
} BHSM_ResidueMode_e;

typedef enum BHSM_DVBScrambleLevel {
	BHSM_DVBScrambleLevel_eTS = 0,  /*  transport stream level scrambling*/
	BHSM_DVBScrambleLevel_ePes = 1   /* program element stream level scrambling */
} BHSM_DVBScrambleLevel_e;


typedef union BHSM_ResidueMode_u {
	/* In: select the termination mode among BHSM_ResidueMode_e for non-DVB algorithms*/
	BHSM_ResidueMode_e 		residueMode;    

	/* In: select scrambling level among  BHSM_DVBScrambleLevel_e algorithm*/
	BHSM_DVBScrambleLevel_e	dvbScrambleLevel; 
	
} BHSM_ResidueMode_u;



typedef enum BHSM_CaModeShift {
	BHSM_CaModeShift_eCryptoAlgorithmShift = 0,
	BHSM_CaModeShift_eResidueModeShift = 13,  /* 2 bits, refer to  BHSM_ResidueMode_e */
	BHSM_CaModeShift_eDVBScrambleLevelShift = 13,  /* 1 bit, refer to BHSM_DVBScrambleLevel_e */		
	/* BHSM_CaModeShift_eScModShift = 15, */
	BHSM_CaModeShift_eRestrEnShift = 16,
	BHSM_CaModeShift_eGlobalEnShift = 17,
	BHSM_CaModeShift_eRestrScModShift = 18,
	BHSM_CaModeShift_eGlobalScModShift = 19,	
	BHSM_CaModeShift_eMscSelShift = 20,	
	BHSM_CaModeShift_eScValueShift = 21,	/* 2 bit for this field*/
	BHSM_CaModeShift_eSynEnableShift = 23,
	BHSM_CaModeShift_eCAMask = 0xFFFFFF,		/* in total there are 23 bits defined now */

	
	BHSM_CaModeShift_eEncScPolarityShift = 0,
	BHSM_CaModeShift_eMulti2KeySelectShift = 1,
	BHSM_CaModeShift_eAtscScrambleCtrlShift = 3,
	BHSM_CaModeShift_eAtscScModShift = 4,	
	BHSM_CaModeShift_eGlobalDropPktCtrlShift = 5,
	BHSM_CaModeShift_eRestrictDropPktCtrlShift = 6,	
	BHSM_CaModeShift_eGlobalRegOverwriteShift = 7,
	BHSM_CaModeShift_eCPDDisableShift = 8,	
	BHSM_CaModeShift_eCPSDisableShift = 9
	
} BHSM_CaModeShift_e;

typedef enum BHSM_M2mModeShift {
	BHSM_M2mModeShift_eCryptoAlgorithmShift = 0,
	BHSM_M2mModeShift_eAESCounterKeyModeShift = 13,  
	BHSM_M2mModeShift_eAuthCtrlShift = 16,
	BHSM_M2mModeShift_eMscCtrlSel = 18,
	BHSM_M2mModeShift_eUseExtKey = 19,
	BHSM_M2mModeShift_eUseExtIV = 20,
	BHSM_M2mModeShift_eEnableTimeStamp = 25,

	BHSM_M2mModeShift_eMax	
} BHSM_M2mModeShift_e;



/**************************************************************************************************
Summary:

Description:
this structure is a sub member in BHSM_ConfigAlgorithmIO.  All the setting will be translated into word32 
format and send to BSP configuring the key slot header area controlling bits, such as

<verbatim>
	Bit[7:0]- Alg_Sel: CA/CPD/CPS/RMX algorithm supported
	Bit[12:8]- Key_Offset: 5 bits of Key offset
	Bit[14:13]- Residue_Mode_Ctrl: 0x0=Unscrambled, 0x1=Residue block,0x2=Cipher text stealing
	Bit[13]-DVB_Scramble_Level:  0 = pes level, 1 = transport
	Bit[15]- Revered
	Bit[16]-Restr_En
	Bit[17]-Global_En 
	Bit[18]-Restr_SC_Mod
	Bit[19]-Global_SC_Mod
	Bit[20]-MSC Mode Selection for algorithms that use MSC.  0 - Select DTV_MSC0/MPEG_MSC0  
	                                                                              1 - Select DTV_MSC1/MPEG_MSC1 
	Bit[22:21]- SC value, 2 bits to be set if Global_SC_Mod or Restrict_SC_Mod is set
	
</verbatim>
<verbatim>
	Global Ctrl Bits:
	Bit[0]- Enc_SC_Polarity: 
	When Global_SC_Mod or Restr_SC_Mod for CPS is set, this bit determines the SC polarity to set to:
	For MPEG packets, Global_SC_Mod or Restr_SC_Mod = 1 will set the SC bits to 11. SC bits will be
	set to 10 otherwise.
	For non-MPEG packets, Global_SC_Mod or Restr_SC_Mod=1 is copied to the SC flag.
	Bit [2:1]- Multi2_Syskey_Sel: Multi 2 System key Select 
	Bit[3]- Atsc_Scramble_Ctrl
	Bit[4]- Atsc_Mod
	Bit[5]- Global_Drop_Packet_Ctrl: set to 1 to drop
	Bit[6]- Restr_Drop_Packet_Ctrl
	Bit[7]- Global_Region_Overwrite: when set, XPT secure modules allows all packets(restricted and 
			global) to go to global region. 
	Bit[8] CPD_Disable, set to 1 to disable CPD
	Bit[9] CPS_Disable, set to 1 to disable CPS(or Remux)
	Bit[10:31]-Reserved
</verbatim>
<verbatim>
	Bit[7:0] AlgSel, MEM2MEM, algorithm supported
	Bit[12:8]  5 bits of Key offset
	Bit[14:13] Mem2mem AES counter mode  key mode select 
	Bit[15]: reserved
	Bit[16:17] Authctrl, (decryption = 0x02, encryption = 0x01; pass = 0)
</verbatim>

See Also:
BHSM_ConfigAlgorithmIO
**************************************************************************************************/ 	
typedef struct BHSM_CACryptoAlg {
	/* In: select the algorith among BCMD_XptSecCryptoAlg_e */
	BCMD_XptSecCryptoAlg_e 		caSecAlg;	

	/* In:  set termination mode for non-DVB algorithms or scrambling level for DVB, see BHSM_ResidueMode_u */
	BHSM_ResidueMode_u			residueMode; 

	bool							bRestrictEnable;  /* In: future feature enhancements */
	bool							bGlobalEnable;    /* In: future feature enhancements */  
	bool							bRestrictScMod;  /* In: future feature enhancements */
	bool							bGlobalScMod;    /* In: used to clear scrambling control bits */		
	
	bool							bEncScPolarity;  /* In: future feature enhancements */
	unsigned char					ucMulti2KeySelect;/* In: Multi2 system key destination */
	bool							bAtscScrambleCtrl;/* In: future feature enhancements */
	bool							bAtscMod;	/* In: future feature enhancements */
	bool							bGlobalDropPktCtrl;/* In: future feature enhancements */
	bool							bRestrictDropPktCtrl;	/* In: future feature enhancements */
	bool							bGlobalRegOverwrite; /* In: future feature enhancements */
	bool							bCPDDisable;	/* In: future feature enhancements */
	bool							bCPSDisable;	/* In: future feature enhancements */

#if BHSM_SC_VALUE
	/* In:	MSC Mode Selection for algorithms that use MSC.  
	         0 - Select DTV_MSC0/MPEG_MSC0   - Select DTV_MSC1/MPEG_MSC1 */
	bool							bMscSel;	

	/* In:	SC value to replace SC field in packet when Global_SC_MOD or RESTR_SC_MOD is set, LSB 2 bits are valid  */
	unsigned char					uScValue;	
#endif 
	bool 						bSynEnable;    /* enable Synamedia AES */
}BHSM_CACryptoAlg_t;

typedef enum BHSM_M2mAuthCtrl {
	BHSM_M2mAuthCtrl_ePassThrough = 0x00, /* no any M2M processing, just by pass*/
	BHSM_M2mAuthCtrl_eScramble= 0x01,     /* M2M scrambling/encryption*/
	BHSM_M2mAuthCtrl_eDescramble = 0x02 	/* M2M descrambling/decryption*/

	
} BHSM_M2mAuthCtrl_e;

typedef struct BHSM_M2MCryptoAlg {
	/* In: M2M algorithm selection among BCMD_M2MSecCryptoAlg_e*/
	BCMD_M2MSecCryptoAlg_e 		m2mSecAlg; 
	
	/* In: used in two ways. (1) if used for AES128Counter mode, this is used as counter length selection,
	    select the value among BCMD_Aes128_CounterSize_e; (2) if used for all other algorithms, this is used
	    as termination mode selection as in BHSM_ResidueMode_e.  */
	unsigned char					ucAESCounterKeyMode;  

	/* In: M2M processing seletion, scramble/descramble/passthrough, among BHSM_M2mAuthCtrl_e */
	BHSM_M2mAuthCtrl_e			ucAuthCtrl;  

	/* In:  Select the MSC length from RDB registers when In AES MSC Mode (alg_sel = 0x11 or 0x14),
                "0" - the msc length read from register XPT_MMSCRAM_xxx_MSC_CTRL, [7:0]  for  MPEG2 packet
                        [15:8] for reserved1 packet.
                "1", the msc length read from register XPT_ MMSCRAM_ xxx_MSC_CTRL[23:16]  for  MPEG2 packet
                       [31:24] for reserved1 packet.
	*/
	bool							bMscCtrlSel;

	/* In: conditional. 1 to select pre-appended key inside data stream; 0 to select key in regular secure key table
	          applicable to RC4, WMDRM-PD in STB, AES-G in DVD. Ignored by CBCMAC (always to use pre-appended key).
	          Ignored by all other DVD algorithms (always use external pre-appended data key and title key in key table)
	*/
	bool							bUseExtKey;		

	/* In: conditional. 1 to select pre-appended IV inside data stream; 0 to select/load IV in regular secure key table
		   applicable to DVD algorithms only
	*/
	bool							bUseExtIV;		

	bool							bEnableTimestamp;
		
}BHSM_M2MCryptoAlg_t;


typedef union BHSM_CryptoAlg_u {
	/*In: for CA/RMX key slot related settings, in BHSM_CACryptoAlg_t*/
	BHSM_CACryptoAlg_t		caCryptAlg;	

	/* In: for M2M key slot related settings, in BHSM_M2MCryptoAlg_t */
	BHSM_M2MCryptoAlg_t		m2mCryptAlg;   
}BHSM_CryptoAlg_u;


/**************************************************************************************************
Summary:

Description:
this structure defines what (de)scrambling algorithm, control bits, key source are needed to configure 
for a key slot entry.

See Also:
BHSM_ConfigAlgorithm
**************************************************************************************************/ 
typedef struct BHSM_ConfigAlgorithmIO {
	
	/* In: the key source selection among a subset of BCMD_KeyRamBuf_e valued between 0~11, i.e.
	    BCMD_KeyRamBuf_eFirstRam ~ BCMD_KeyRamBuf_eKey5KeyLadder3 */
	BCMD_KeyRamBuf_e			keySource;  			

	/* In: the key source is in which block of CA or M2M keytable,  see BCMD_KeyDestBlockType_e*/
	BCMD_KeyDestBlockType_e		keyDestBlckType;	

	/* In: the key source is in which key slot entry as defined in BCMD_KeyDestEntryType_e*/
	BCMD_KeyDestEntryType_e		keyDestEntryType;
	
	/* In: the key source is of what key slot type*/
	BCMD_XptSecKeySlot_e			caKeySlotType;		

	/* In: the key source is in which key slot (key slot number) */
	unsigned int					unKeySlotNum;		

	/* In: control bits in the key slot header area are configured as set here,BHSM_CryptoAlg_u  */
	BHSM_CryptoAlg_u				cryptoAlg;                 /* different for M2M or CA/RMX, more than 10 acutal members herein must be set correctly*/


	/* Out: 0 for success, otherwise failed */
	uint32_t						unStatus;

	
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
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_configAlgorithmIO  - BHSM_ConfigAlgorithmIO_t.
			
Output:
inoutp_configAlgorithmIO  - BHSM_ConfigAlgorithmIO_t.


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
		BHSM_Handle				in_handle,
		BHSM_ConfigAlgorithmIO_t	*inoutp_configAlgorithmIO
);

#define BHSM_MULTI2_SYS_KEY_SIZE	 32


/**************************************************************************************************
Summary:

Description:
this structure defines data members to configure Multi2 algorithm which is different from the 
configuration of all the other algorithms 

See Also:
BHSM_ConfigMulti2
**************************************************************************************************/ 
typedef struct BHSM_ConfigMulti2IO {

	/* In: Multi round counber value  */
	unsigned char				ucMulti2RndCnt;		
	
	/* In: MULTI2 system key seed, big endian array, 32 bytes*/
	unsigned char				aucMulti2SysKey[BHSM_MULTI2_SYS_KEY_SIZE]; 

	/* In: which system key to configure, HW specific, so far only 0~3 is legal */
	unsigned char				ucSysKeyDest; 			

	/* Out: 0 for success, otherwise failed */
	uint32_t					unStatus;

	
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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle				in_handle,
		BHSM_ConfigMulti2IO_t		*inoutp_configMulti2IO
);



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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle					in_handle,
		BCMD_XptSecKeySlot_e			in_keySlotType,
		unsigned int					*outp_unKeySlotNum
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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle					in_handle,
		unsigned int					in_unPidChannel,
		BHSM_PidChannelType_e		in_pidChannelType,		
		BCMD_XptSecKeySlot_e			in_keySlotType,
		unsigned int					in_unKeySlotNum
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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle					in_handle,
		unsigned int					in_unPidChannel,
		BHSM_PidChannelType_e		in_pidChannelType,
		BCMD_XptSecKeySlot_e			*outp_ucKeySlotType,
		unsigned int					*outp_unKeySlotNum
);


/**************************************************************************************************
Summary:

Description:
this structure defines which PID channel to assocaite with which allocated key slot

See Also:
BHSM_ConfigPidKeyPointerTable
BHSM_PidChannelType_e
**************************************************************************************************/ 
typedef struct BHSM_M2MKeySlotIO {
	/* In:  Which client requests for M2M key slot   */
	BHSM_ClientType_e               client;

    /* In:  Which key slot type to request for  -- For Zeus 4.X support */
	BCMD_XptSecKeySlot_e            keySlotType;

	/* Out: 0 for success, otherwise failed */
	uint32_t						unStatus;

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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle					in_handle,
		BHSM_M2MKeySlotIO_t			*inoutp_M2MKeySlotIO
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
in_handle  - BHSM_Handle, Host Secure module handle.
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
	BHSM_Handle					in_handle,
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
in_handle  - BHSM_Handle, Host Secure module handle.
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
		BHSM_Handle						in_handle,
		BHSM_ConfigPidKeyPointerTableIO_t	*inoutp_configPidKeyPointerTableIO
);


/*****************************************************************************
Summary:

This function can be used to clear all the control bits and algorithm of a CA/RMX/M2M 


Description:

This function is a wrapper of BHSM_ConfigAlgorithm to clear all the control bits and algorithm
of a CA/RMX or M2M key slot. The control bits and algorithm of the specified key slot are 
configured to be all '0' via this function call. However, the keys are still kept intact. 

So, if applications want to clear all CA/RMX in the slot, pls call this PI two times, e.g.
   		BHSM_ResetKeySlotCtrlBits(handle, BCMD_KeyDestBlockType_eCA, slottype, slotnum);
   		BHSM_ResetKeySlotCtrlBits(handle, BCMD_KeyDestBlockType_eRmx, slottype, slotnum);

Certain pid channels may be used to descramble audio/video stream.  After this function is called,
the same pid channels can be re-used to play clear audio/video stream.


This PI is to support new chip/version since 7401C0 or 2006.Oct

Calling Context:

It can be called anytime for a key slot which had normally been configured previously.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed. 

Input:
in_handle  - BHSM_Handle, Host Secure module handle.
in_kSlotDestBlockType - BCMD_KeyDestBlockType_e, the supported values are a subset of
					BCMD_KeyDestBlockType_e, i.e.
    						BCMD_KeyDestBlockType_eCA = 1,
    						BCMD_KeyDestBlockType_eRmx = 3,
    						BCMD_KeyDestBlockType_eMem2Mem = 4,
in_kSlotType - BCMD_XptSecKeySlot_e, to specify which type of key slot 
in_kSlotNum - unsigned int, to specify which key slot to reset
                  
			
Output:
none.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_ConfigAlgorithm
BHSM_FreeCAKeySlot
BHSM_FreeM2MKeySlot
*****************************************************************************/
BERR_Code  BHSM_ResetKeySlotCtrlBits (
	BHSM_Handle				in_handle,
	BCMD_KeyDestBlockType_e 	in_kSlotDestBlockType,
	BCMD_XptSecKeySlot_e		in_kSlotType, 
	unsigned int 				in_kSlotNum
);

/* End of Module Specific Functions */


#if   BHSM_SECURE_PI_SUPPORT     /*  to hide certain PIs that need special PGP keys */
#include "bhsm_keyladder_enc.h"
#endif 

#ifdef __cplusplus
}
#endif

#endif /* BHSM_KEYLADDER_H__ */
