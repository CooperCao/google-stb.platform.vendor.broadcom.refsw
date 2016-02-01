/*********************************************************************************
*     Copyright (c) 2013, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*   This program is the proprietary software of Broadcom Corporation and/or its licensors, 
*   and may only be used, duplicated, modified or distributed pursuant to the terms and 
*   conditions of a separate, written license agreement executed between you and Broadcom 
*   (an "Authorized License").  Except as set forth in an Authorized License, 
*   Broadcom grants no license (express or implied), right to use, or waiver of any kind 
*   with respect to the Software, and Broadcom expressly reserves all rights in and to the 
*   Software and all intellectual property rights therein.  
* 
*   IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, 
*   AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*   Except as expressly set forth in the Authorized License,
*   1.     This program, including its structure, sequence and organization, constitutes the 
*       valuable trade secrets of Broadcom, and you shall use all reasonable efforts to protect 
*       the confidentiality thereof, and to use this information only in connection with your use 
*       of Broadcom integrated circuit products.
*   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
*       FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, 
*       STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
*       ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*       LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO 
*       DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE 
*       FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT 
*       OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN 
*       ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID 
*       FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
*       ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* PIFF Creation library
* 
* Revision History:
*
* $brcm_Log: $
* 
*********************************************************************************/

#ifndef BPIFF_ENCODER_H
#define BPIFF_ENCODER_H

#include "bstd.h"
#include "nexus_audio_encoder.h" 
#include "nexus_audio_mux_output.h"
#include "nexus_video_encoder.h"
#include "bpiff_encoder_types.h"
#include "drm_prdy.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary: 
Initializes PIFF encoder default settings with the following:
- systemId = PlayReady SystemID
- algorithm =  PIFF_Encryption_Type_eAesCtr
- completionCallBack.callback and context = NULLs
- licPolicyDescriptor :
    . wSecurityLevel = 2000
    . fCannotPersist = FALSE
    . oExpirationAfterFirstPlay fValid = FALSE 
    . oSourceId fValid = FALSE 
    . fRestrictedSourceId FALSE 
    . wCompressedDigitalVideo 0 
    . wUncompressedDigitalVideo 0 
    . wAnalogVideo 0 
    . wCompressedDigitalAudio 0 
    . wUncompressedDigitalAudio 0 
    . cExplicitAnalogVideoOPs 0 
    . cExplicitDigitalAudioOPs 0 
    . cPlayEnablers 2 
    . rgoPlayEnablers[0] DRM_PLAYENABLER_UNKNOWN_OUTPUT
    . rgoPlayEnablers[1] s_DRM_PLAYENABLER_CONSTRAINED_RESOLUTION_UNKNOWN_OUTPUT

***************************************************************************/
void piff_GetDefaultSettings(
         PIFF_Encoder_Settings *pSettings    /* [out] default settings */
    );

/***************************************************************************
Summary: 
Create a PIFF encoder handle. A PIFF encoder handle must be created before 
calling any PIFF functions here. This function returns a valid handle 
upon success. 

Description:
This function will initialize the PIFF_encoder_handle structure which 
will be used for the PIFF operations. 

Note: 

***************************************************************************/
PIFF_encoder_handle piff_create_encoder_handle(
        PIFF_Encoder_Settings  *pSettings,
        DRM_Prdy_Handle_t       PrdyDrmCtx );  


/***************************************************************************
Summary: 
Delete a PIFF encoder handle that all the resources will be released.  

Note: This is important that application must call this function after 
the encoding is complete or solicited to stop.


***************************************************************************/
void piff_destroy_encoder_handle(PIFF_encoder_handle handle);

/***************************************************************************
Summary: 
Start PIFF encoding. If no error, a well-formed PIFF file with all the
fragments encrypted is produced.

Note: in case of error or any unexpected termination, it guarantees that no 
temporary unencrypted payload will be left in the file system. 

***************************************************************************/
BPIFF_Error piff_encode_start( NEXUS_AudioMuxOutputHandle  audioMuxOutputHandle,
                               NEXUS_VideoEncoderHandle    videoEncoderHandle,
                               PIFF_encoder_handle         piffEncoderHandle);

/***************************************************************************
Summary: 
Stop PIFF encoding on demand. 

Description:
This function will stop PIFF encoding operation. Any ongoing fragment will 
still be processed and the PIFF encoding will be stopped for the next fragment. 
Please note that although an encoding is stopped in the middle of the processing, 
a partial complete well-formed PIFF file with already processed fragments will 
still be produced.

***************************************************************************/
BPIFF_Error  piff_encode_stop( PIFF_encoder_handle piffEncoderHandle);

/* Other helper functions */
BPIFF_Error  piff_get_key_id( PIFF_encoder_handle piffEncoderHandle, uint8_t *key_id );

BPIFF_Error  piff_get_key_id_based64W( PIFF_encoder_handle piffEncoderHandle, 
                                       uint16_t *key_id_base64W,/* in|out given buffer */
                                       uint32_t *pkeyIdSize );  /* out - actual size of the key id */

#ifdef __cplusplus
}
#endif


#endif /*DRM_PIFF_H*/
