/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ******************************************************************************/

#ifndef SAGE_MANUFACTURING_API_H___
#define SAGE_MANUFACTURING_API_H___

#include "bstd.h"

/* define value visible by sage provisioning code in order to fill basicOut[1]*/
#define OPERATION_SUCCESSFULLY_OCCURRED (1)

/* define value visible by sage validation code in order to fill basicOut[2]*/
#define HDCP22_MASK     (0x01)
#define EDRM_MASK       (0x02)
#define ECC_MASK        (0x04)
#define DTCP_IP_MASK    (0x08)
#define MEDIAROOM_MASK  (0x10)

/*define value used to indicated type of DRM bin file used in tool*/
#define DRM_BIN_FILE_TYPE_3 (3)
#define DRM_BIN_FILE_TYPE_2 (2)
#define DRM_BIN_FILE_TYPE_1 (1)

typedef enum SAGE_Manufacturing_OTP_Index
{
    SAGE_Manufacturing_OTP_Index_A = 0,
    SAGE_Manufacturing_OTP_Index_B,
    SAGE_Manufacturing_OTP_Index_C,
    SAGE_Manufacturing_OTP_Index_D,
    SAGE_Manufacturing_OTP_Index_E,
    SAGE_Manufacturing_OTP_Index_F,
    SAGE_Manufacturing_OTP_Index_G,
    SAGE_Manufacturing_OTP_Index_H,
    SAGE_Manufacturing_OTP_Index_UNDEFINED = 10
} SAGE_Manufacturing_OTP_Index;

/**
Summary:

Perform SRAI container allocation, SRAI platform Open and
Module Init for Provisioning and Validataion Module

**/
BERR_Code SAGE_Manufacturing_Init(SAGE_Manufacturing_OTP_Index otp_index);

/**
Summary:

Un-init SAGE Platform and Modules

**/
void SAGE_Manufacturing_Uninit(void);

/**
Summary:

Allocate SAGE container and shared memory

**/
int SAGE_Manufacturing_AllocBinBuffer(size_t file_size, uint8_t **ppBinDataBuffer);

/**
Summary:

Free shared memory and SAGE container

**/
int SAGE_Manufacturing_DeallocBinBuffer(void);

/**
Summary:

Parse DRM bin file buffer and display DRM info in the bin buffer

**/
int SAGE_Manufacturing_BinFile_ParseAndDisplay(uint8_t* pBinData, uint32_t binFileLength, int *validationCommand);

/**
Summary:

Verify DRM bin file type for provisioning

**/
int SAGE_Manufacturing_VerifyDrmBinFileType(uint8_t* pBinData, int validationCommand);

/**
Summary:

Provision input DRM bin file through SAGE command

**/
BERR_Code SAGE_Manufacturing_Provision_BinData(int *pStatus);

/**
Summary:

Validate HDCP 2.2, EDRM, or ECC if input includes keys through SAGE command

**/
int SAGE_Manufacturing_ValidateDRM(int *pStatus, int validationCommand);


/**
Summary:

Get error string from error codes through SageLib

**/
int SAGE_Manufacturing_GetErrorInfo(int *error_code, char *p_error_string);

#endif /* SAGE_MANUFACTURING_API_H___ */
