/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BVCE_TELEM_H_
#define BVCE_TELEM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BVCE_Telem_P_Context *BVCE_Telem_Handle;

typedef struct BVCE_Telem_Settings
{
   unsigned uiUnused;
} BVCE_Telem_Settings;

void
BVCE_Telem_GetDefaultCreateSettings(
         BVCE_Telem_Settings *pstTelemSettings
         );

BERR_Code
BVCE_Telem_Create(
         BVCE_Telem_Handle *phVCETelem,
         const BVCE_Telem_Settings *pstTelemSettings
         );

BERR_Code
BVCE_Telem_Destroy(
         BVCE_Telem_Handle hVCETelem
         );

BERR_Code
BVCE_Telem_Parse(
   BVCE_Telem_Handle hVCETelem,
   const uint8_t *pInputBuffer0, /* [in] */
   unsigned uiInputLength0, /* [in] */
   const uint8_t *pInputBuffer1, /* [in] */
   unsigned uiInputLength1, /* [in] */
   unsigned *puiInputLengthRead, /* out */
   char *szOutputBuffer,   /* [out] pointer to buffer where log is copied to */
   unsigned uiOutputLength,  /* [in] maximum number of bytes to copy to buffer */
   unsigned *puiOutputLengthWritten  /* [out] number of bytes copied from debug log */
   );

#ifdef __cplusplus
}
#endif

#endif /* BVCE_TELEM_H_ */
/* End of File */

