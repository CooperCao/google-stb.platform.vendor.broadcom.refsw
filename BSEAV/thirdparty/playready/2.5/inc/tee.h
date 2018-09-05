/*************************************************************************************
 *                                                                                   *
 * This file defines client-side TEE APIs directly invoked by PlayReady code.        *
 *                                                                                   *
 * The stub implementations of these APIs in tee.c directly invoke TEE-side APIs,    *
 * which are defined in teeimpl.h and implemented in teeimpl.c.                      *
 *                                                                                   *
 * Real client-side API  implementations in tee.c shall be rewritten to invoke       *
 * TEE-side functions using TEE-specific call mechanisms.                            *
 *                                                                                   *
 *************************************************************************************/
 /******************************************************************************
* (c) 2014 Broadcom Corporation - copyright pending confirmation from Netflix
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef __TEE_INCLUDE_H__
#define __TEE_INCLUDE_H__

/*
 * teecommon.h includes some constants required by PlayReady and by the reference TEE implementation.
 * It is included here to avoid multiple includs from the PlayReady code.
 */
#include <teecommon.h>
#include <drmblackboxtypes.h>
#include <oemhal.h>

/********************************************************************************************************
 *                                                                                                      *
 * Design Notes:                                                                                        *
 *                                                                                                      *
 * TEE is configured with AES-128 "TEE Master Key", presumably set at manufacturing time and unique per *
 * device. teeimpl.c used hardcoded TEE Master Key, which MUST be changed for production TEE. This key  *
 * protects TEE state that aplication stores outside TEE.                                               *
 *                                                                                                      *
 ********************************************************************************************************/

/***************************
 * TEE Lifecycle Functions *
 ***************************/

/*************************************************************************************
 * Is called from Drm_Initialize(). Functionality is specific to TEE implementation. *
 *
 * Input: TEE state data and its size in bytes
 */
int TEE_Initialize(Oem_HalHandle		  halHandle,
				   const DRM_BYTE * const inbuf,
                   const DRM_DWORD        inlen);

/*
 * Is called from Drm_TurnSecureStop(). Functionality is specific to TEE implementation.
 *
 * Input: TEE state data and its size in bytes
 */
int TEE_Update(Oem_HalHandle		  halHandle,
               const DRM_BYTE * const inbuf,
               const DRM_DWORD        inlen);

/*
 * Is called from Drm_Reinitialize(). Functionality is specific to TEE implementation.
 */
int TEE_Reinitialize(Oem_HalHandle halHandle);

/*
 * Is called from Drm_Unitialize(). Functionality is specific to TEE implementation.
 */
int TEE_Uninitialize(Oem_HalHandle halHandle);

/***********************************************************************************************
 * Enable/Disable Secure Stop Functionality (default hard-coded into PlayReady implementation) *
 * If Secure Stop is disabled, other secure stops APIs become no-op:                           *
 * they do not fail, just return no secure stop / state data                                   *
 ***********************************************************************************************/
int TEE_TurnSecureStop(Oem_HalHandle 	halHandle,
					   const DRM_BOOL	inOnOff);

/*************************************************
 * Check if Secure Stop Functionality is Enabled *
 *************************************************/
/* NOTE: Returns TEE_SUCCESS if function execution is a success, whether Secure
   Stop is on or off. */
int TEE_IsSecureStopOn(Oem_HalHandle 		halHandle,
					   DRM_BOOL * const 	isOn);

/***************************************************************************************
 * Create a new Session inside the TEE.                                                *
 * The Session ID is to be used as the NONCE in a License Challenge.                   *
 ***************************************************************************************/
int TEE_StartSession(Oem_HalHandle 		halHandle,
					 const DRM_BYTE * const inSessionId,
                     const DRM_BOOL         isLdl);

/******************************************************************************
 * Cancel a new Session inside the TEE. Only possible before License binding. *
 ******************************************************************************/
int TEE_CancelSession(Oem_HalHandle 			halHandle,
					  const DRM_BYTE * const 	inSessionId);

/***********************************
 * Get Upper Limit of LDL Sessions *
 ***********************************/
int TEE_GetLdlSessionsLimit(Oem_HalHandle 	    halHandle,
                            DRM_DWORD * const   outLdlSessionsLimit);

/*****************************************
 * Flush all outstanding LDL chaallenges *
 *****************************************/
int TEE_FlushLdlChallenges(Oem_HalHandle halHandle);

/***************************
 * Bind license to session *
 ***************************/
int TEE_BindLicense(Oem_HalHandle 			halHandle,
					const DRM_BYTE  * const inSessionId,
					const DRM_BYTE  * const inKID,
                    const DRM_DWORD         inLDLSeconds);

/*************************************************************
 * Check if an unbound active session exists by content key. *
 *************************************************************/
int TEE_HasSessionByKID(Oem_HalHandle 		    halHandle,
					    const DRM_BYTE  * const inKID);

/***********************************************************************************
 * Bind content decryption key to a given session, at the beginning of a playback. *
 ***********************************************************************************/
int TEE_BindSession(Oem_HalHandle 						halHandle,
                    const DRM_BYTE  * const             inSessionId,
                    const OEM_HAL_KEY_REGISTER_INDEX 	inKeyIndex);

/***************************************
 * Stop Session at the end of playback *
 ***************************************/
int TEE_StopSession(Oem_HalHandle 						halHandle,
					const OEM_HAL_KEY_REGISTER_INDEX 	inKeyIndex);

/**************************************************************************
 * Check if secure stop state needs to be updated for a given content key *
 **************************************************************************/
int TEE_DecryptUpdate(Oem_HalHandle 					halHandle,
					  const OEM_HAL_KEY_REGISTER_INDEX 	inKeyIndex,
                      DRM_UINT64 const                  inInitializationVector,
					  DRM_DWORD * const                 actions);

/*********************************************************************************************
 * Return IDs of outstanding secure stops.                                                   *
 *********************************************************************************************/
int TEE_GetSecureStopIds(Oem_HalHandle 			halHandle,
						 DRM_BYTE          	    outSessionIds[TEE_MAX_NUM_SECURE_STOPS][TEE_SESSION_ID_LEN],
						 DRM_DWORD * const 		outNumSessionIds);

/*********************************************************************************************
 * Return secure stop for a given session ID.                                                *
 * Return error if sessionID does not exist or corresponds to the playback still in-progress *
 *********************************************************************************************/
int TEE_GetSecureStop(Oem_HalHandle 					halHandle,
					  const DRM_BYTE * const 			inSessionId,
					  const OEM_HAL_KEY_REGISTER_INDEX 	inKeyIndex,
					  DRM_BYTE * const 					outSecureStop,
					  DRM_DWORD  * const 				outSecureStopLen,
					 TEE_SIGNATURE * const 				outSig);

/*********************************************************************************
 * Permanently remove outstanding secure stop for a given session ID.            *
 * Pass updated persistent state back to the caller.                             *
 * Return error if sessionID does not correspond to any outstanding secure stop. *
 *********************************************************************************/
int TEE_CommitSecureStop(Oem_HalHandle 				halHandle,
						 const DRM_BYTE  * const 	inSessionId,
						 DRM_BYTE  * const 			outState,
						 DRM_DWORD * const 			outStateLen);

/*************************************************************
 * Permanently remove all outstanding secure stops.          *
 * Pass updated (empty) persistent state back to the caller. *
 *************************************************************/
int TEE_ResetSecureStops(Oem_HalHandle 		halHandle,
						 DRM_BYTE  * const 	outState,
						 DRM_DWORD * const 	outStateLen,
						 DRM_DWORD * const 	outNumRemoved);

/*****************************************************************************
 * Get current state to persist. State contains version and all secure stops *
 *****************************************************************************/
int TEE_GetState(Oem_HalHandle 		halHandle,
				 DRM_BYTE  * const 	outState,
				 DRM_DWORD * const 	outStateLen);


#ifdef PLAYREADY_SAGE_IMPL
/*****************************************************************************
 * Broadcom added function SAGE uses to pass data TEE needs from the
 * playready.bin file.
 *****************************************************************************/
int BTEE_SetPlayReadyData(DRM_VOID * 		f_pAesKeyEncKey,
						  DRM_VOID * 		f_binFileMgrHle);

/*****************************************************************************
 * Broadcom added function to pass encrypted state version buffer from host  *
 * to SAGE to initialize the state version in the TEE
 *****************************************************************************/
int BTEE_InitStateVersion(Oem_HalHandle 			halHandle,
						  const DRM_BYTE  * const 	inStateVersionBuf,
						  const DRM_DWORD  			inStateVersionLen);

/*****************************************************************************
 * Broadcom added function for SAGE that allows clients to determine if the
 * Sate Version has recently changed requiring a an update be written to
 * non-volative storage.
 *****************************************************************************/
DRM_BOOL BTEE_IsStateVersionUpdateRequired(Oem_HalHandle halHandle);

/*****************************************************************************
 * Broadcom added function for SAGE that allows clients to get encrypted
 * state version information so that it can be written into non-volatile
 * storage.
 *****************************************************************************/
int BTEE_GetStateVersion(Oem_HalHandle 		halHandle,
						 DRM_BYTE  * const 	outStateVersionBuf,
						 DRM_DWORD * const 	outStateVersionLen);

/*****************************************************************************
 * Broadcom added function for SAGE that allows clients to notify the State
 * Version module that recently updated state version data has been encrypted
 * and written to non-volatile storage.
 *****************************************************************************/
int BTEE_StateVersionUpdateComplete(Oem_HalHandle halHandle);
#endif

#endif /* __TEE_INCLUDE_H__ */
