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
#ifndef BHSM_PRIV_H__
#define BHSM_PRIV_H__

#include "berr_ids.h"
/*#include "bhsm.h"   ly*/
#include "bhsm_datatypes.h"
#include "bsp_s_keycommon.h"
#include "bsp_s_commands.h"
#include "bsp_s_otp.h"
#if (BCHP_CHIP==7401) &&  (BCHP_VER == BCHP_VER_A0)
#include "bsp_s_hw_7401.h"
#else
#include "bsp_s_hw.h"
#endif
#include "bkni.h"
#include "bkni_multi.h"
#include "bint.h"			/* ly 12/8, used in BHSM_P_Handle */

#if  (BHSM_IPTV ==1)	
#include  "bmem.h"
#define  BHSM_CONTIGUOUS_MEMORY_SIZE 		(256+1024*64)    /* 64 KBytes, can be adjusted later*/
#endif


/*#include "bhsm_keyladder.h"   ly  L258 to use this, moved to bsp_s_hw.h */

/* 2/20/2007, this is only a patch to help hw, may have performance impact on key loading
	7401 B0, C0, C1 only +  M2M only  +   for all customer modes
*/
#if (BCHP_CHIP==7401) &&  (BCHP_VER >= BCHP_VER_B0)&& (BCHP_VER <= BCHP_VER_C1)
#include  "bchp_memc_0.h"				/* other STB chips, may use bchp_memc_1.h etc.  */

/* !!!!!!!!! at most, only one can be set to true. never both to true. both can be false to turn off completely !!!!!*/
#define    BHSM_PATCH_MEMC_DMA		1		/* workaround in a special submit command only */
#define    BHSM_PATCH_MEMC_DMA_PI	0		/* workaround in the individual PI only */
/* !!!!!!!!! at most, only one can be set to true. never both to true. both can be false to turn off completely !!!!!*/


#define    BHSM_PATCH_MEMC_DMA_WAITING    (500)        /* in units of 2us, please adjust this in the experiment*/
#define    BHSM_PATCH_MEMC_DMA_REG  	  (BCHP_MEMC_0_CLIENT_INFO_44)  /* if other reg, please adjust this also*/
#define    BHSM_PATCH_MEMC_DMA_BOMASK    (0x001FFF3F)      /* block out value mask, change according to RDB*/


#if (BCHP_CHIP==7401) &&  (BCHP_VER == BCHP_VER_C1)   /* for HPC20 project only, 4/5/07 */
#define BKNI_ACQUIREMUTEX_LOCK_TRUST    (1)      /*  1 to use BKNI_Sleep()/non-cpu-blocking, 0 to use BKNI_Delay()/cpu-blocking */
#endif

#else
#define     BHSM_PATCH_MEMC_DMA		0
#define     BHSM_PATCH_MEMC_DMA_PI	0
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* Definitions */

/* Host Secure Module magic number used to check if opaque handle is corrupt */
#define BHSM_P_HANDLE_MAGIC_NUMBER           0xfacedead

/* Host Secure Channel magic number used to check if opaque handle is corrupt */
#define BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER   0xbacaface



/* End of Definitions */ 

/* Enum Types */

typedef enum BHSM_ClientType_e
{
	BHSM_ClientType_eHost,
	BHSM_ClientType_eSAGE,

	BHSM_ClientType_eMax

}  BHSM_ClientType_e;

/* End of Enum Types */


/* Host Secure Private Data Structures */

/***************************************************************************
Summary:
Structure that defines Host Secure module handle.

Description:
Structure that defines Host Secure module handle.

See Also:
BHSM_Open()

****************************************************************************/  
 typedef struct BHSM_P_CommandData
{
		BCMD_cmdType_e		cmdId;  /* Command id == tag id */
		uint32_t				unContMode;
/*10/05/05,Allen.C add to support lock mutex in the command function instead of BHSM_P_CommonSubmitCommand ()*/		
		bool					bLockWaive;
		
		/* This length (in bytes) does not include header . Max is 364 bytes. Padded with zero to make it word aligned*/
		uint32_t				unInputParamLen;   
		uint32_t				unInputParamsBuf[BCMD_BUFFER_BYTE_SIZE/4];

		/* The following is for automated test */
		/* This length (in bytes) does not include header . Max is 364 bytes. Padded with zero to make it word aligned*/		
		uint32_t				unOutputParamLen;   
		uint32_t				unOutputParamsBuf[BCMD_BUFFER_BYTE_SIZE/4];

} BHSM_P_CommandData_t;


#if 0
typedef struct BHSM_P_InterruptCallback
{
	BHSM_IsrCallbackFunc	callBack;	 /*  interrupt Callback Function. */
	void 					*parm1;		/* First callback arg. */
	void					*parm2;	/* Second callback arg. */
} BHSM_P_InterruptCallback;
#endif


 typedef struct BHSM_P_CAKeySlotTypeInfo {
 	unsigned 	int		unKeySlotNum;   
	unsigned 	char		ucKeySlotStartOffset;
	
 } BHSM_P_CAKeySlotTypeInfo_t;

  typedef struct BHSM_P_KeySlotAlgorithm {
 	uint32_t		ulGlobalControlBits; 	
 	uint32_t		ulCAControlBits;
 	uint32_t		ulCPDControlBits;
 	uint32_t		ulCPSControlBits;   /* Remux */

 } BHSM_P_KeySlotAlgorithm_t;

 typedef struct BHSM_P_CAKeySlotInfo {
 	BCMD_XptSecKeySlot_e		keySlotType;   /* ?? May not need this */
	bool						bIsUsed;
	/* Storage for Algorithm control bits for Odd, Even, Clear key */		
	BHSM_P_KeySlotAlgorithm_t	aKeySlotAlgorithm[BCMD_KeyDestEntryType_eMaxNumber + 1] ;	
	
 } BHSM_P_CAKeySlotInfo_t;


 typedef struct BHSM_P_PidChnlToCAKeySlotNum {
 	BCMD_XptSecKeySlot_e		keySlotType;   /* ?? May not need this */
	unsigned int 				unKeySlotNum;
	
 } BHSM_P_PidChnlToCAKeySlotNum_t;


 typedef struct BHSM_P_M2MKeySlotInfo {

	bool						bIsUsed;
	
 } BHSM_P_M2MKeySlotInfo_t;


struct BHSM_P_Handle;             /*Leoh */

/***************************************************************************
Summary:
Structure that defines Host Secure channel handle.

Description:
Structure that defines Host Secure channel handle.

See Also:
BHSM_OpenChannel()

****************************************************************************/  
typedef struct BHSM_P_ChannelHandle
{
	uint32_t       			ulMagicNumber; /* Must be  BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER */

	struct BHSM_P_Handle *   		moduleHandle;   /* Module handle */
	
	BHSM_ChannelSettings	currentChannelSettings;   /* current channel settings */

	uint8_t        			ucChannelNumber;     /* channel number */

	bool          			bIsOpen;    /* Is channel opened */

	uint32_t	  			ulSequenceNum; /* sequence number, only use 8 bits now. Watch out for big/little endian */
	
	BKNI_MutexHandle  	mutexLock;  /* to synchronize the command */

	unsigned long			ulInCmdBufAddr;                   /* Start Address of input cmd buffer */
	unsigned long			ulOutCmdBufAddr;                   /* Start Address of output cmd buffer */

	unsigned long			ulILoadRegAddr;
	unsigned long			ulILoadVal;

	unsigned long			ulIReadyRegAddr;
	unsigned long			ulIReadyVal;	
 
	BKNI_EventHandle 		oLoadWait;
	
	uint8_t				oLoadSet;			/* for CTRL-C issue */

#if BHSM_DEBUG_POLLING  		/* a safe place, not in bhsm.c */
#define BHSM_DEBUG_POLLINGBUF_MAX (256)
	uint16_t          	pollingIntervalBuf[BHSM_DEBUG_POLLINGBUF_MAX];    /* max interval = 65535 us = 65 ms, long enough*/
	uint16_t          	pollingBufIndex;
#endif

	
} BHSM_P_ChannelHandle;

/*typedef struct BHSM_P_ChannelHandle     *BHSM_ChannelHandle;  Leoh*/


#define BHSM_HAMCSHA1_CONTEXTSWITCH_NUMBER 	3
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
	uint32_t  				ulMagicNumber; /* Must be  BHSM_P_HANDLE_MAGIC_NUMBER */

	struct BHSM_P_ChannelHandle *	channelHandles[BHSM_MAX_SUPPOTED_CHANNELS];	

	BHSM_Settings 		currentSettings;   /* current settings */

	BREG_Handle			regHandle;    /* register handle */
	BCHP_Handle			chipHandle;  /* chip handle */
	BINT_Handle			interruptHandle;   /* interrupt handle */


	bool					bIsOpen;    /* Is Module opened */

	BINT_CallbackHandle	IntCallback;  /* Interrupt Callback */
	BINT_CallbackHandle	IntCallback2;  /* Interrupt Callback */	

#if 0 /* Too risky */
	BHSM_P_InterruptCallback BHSM_P_IntrCallbacks[ BHSM_IntrType_eMax ];		/* HSM interrupt handlers. */
#endif

	/* Mutex to be acquired before changing aunCAKeySlotInfo[] */
  	BKNI_MutexHandle					 caKeySlotMutexLock;

	/* Number of Key slots per key slot type */
	BHSM_P_CAKeySlotTypeInfo_t 				aunCAKeySlotTypeInfo[BCMD_XptSecKeySlot_eTypeMax];  

	/* Total  Number of CA Key slots */
	unsigned int				unTotalCAKeySlotNum;

	/* The layout of all the key slots. Use this to find out the CA key slot type and vacant key slot. */
	BHSM_P_CAKeySlotInfo_t 				aunCAKeySlotInfo[BCMD_TOTAL_PIDCHANNELS*2];  
	
	/* 
		Given pid channel, return CA key slot number. 
		Each Pid Channel can have primary pid and secondary pid. Therefore,
		we need 2 key slot per pid channel.
		We can have multiple pid channels associated with a key slot.
		We can also have a pid channel associated with 2 key slots.
	*/
	BHSM_P_PidChnlToCAKeySlotNum_t 		aPidChannelToCAKeySlotNum[BCMD_TOTAL_PIDCHANNELS][BHSM_PidChannelType_eMax];


	/* Mutex to be acquired before changing aunM2MKeySlotInfo[] */
  	BKNI_MutexHandle					 m2mKeySlotMutexLock;

	/* Use this to find out the M2M vacant key slot. */
	BHSM_P_M2MKeySlotInfo_t 				aunM2MKeySlotInfo[BCMD_MAX_M2M_KEY_SLOT];  	
	
	bool          			bIsBusyHmacSha1[BHSM_HAMCSHA1_CONTEXTSWITCH_NUMBER];    /* Is context switch of HMAC SHA1 busy ? */
	bool					bIsBusyPKE;		 /* Is PKE engine busy ? */
	
	uint16_t				chipId;
	uint16_t				chipRev;	

#if  (BHSM_IPTV ==1)	
	BMEM_Heap_Handle	         hHeap;		/* 10/30/07 added for IPTV contiguous memeory support inside HSM*/
	unsigned char			*pContiguousMem;
#endif

} BHSM_P_Handle;


/*typedef struct BHSM_P_Handle            *BHSM_Handle;   Leoh*/



/* End of Host Secure Private Data Structures */



/* Private Function */

void BHSM_P_IntHandler_isr(
	void *inp_param1,		
	int in_param2			
);

/* Use this function to submit cmds.  This function takes care of continual modes, owner id, version number.  
ucInCmdData includes parameter length and on.  After checking output cmd, return to the caller.
Need a mutex to protect the increment seq number.  
Continual mode for diff cmds could be tricky: TBD.
Version number is from a shared aegis file ???
return error=> other than status, everything is undetermined.
 */

BERR_Code BHSM_P_SubmitCommand (
		struct BHSM_P_ChannelHandle     *	in_channelHandle	
);


BERR_Code BHSM_P_CommonSubmitCommand (
		struct BHSM_P_ChannelHandle     *	in_channelHandle,	
		BHSM_P_CommandData_t	*inoutp_commandData
);

BERR_Code BHSM_P_SubmitCommand_DMA (
		struct BHSM_P_ChannelHandle     *	in_channelHandle	
);


BERR_Code BHSM_P_CommonSubmitCommand_DMA (
		struct BHSM_P_ChannelHandle     *	in_channelHandle,	
		BHSM_P_CommandData_t	*inoutp_commandData
);


void BHSM_P_ExceptionInterruptCB_isr( 
	      void		*inp_handle,
	      void       	*inp_data 
) ;


#ifdef BHSM_AUTO_TEST
BERR_Code BHSM_P_CommonSubmitRawCommand (
		struct BHSM_P_ChannelHandle     *	in_channelHandle,	
		BHSM_P_CommandData_t	*inoutp_commandData
);
#endif

/* End of Private Function */

#ifdef __cplusplus
}
#endif

#endif /* BHSM_PRIV_H__ */
