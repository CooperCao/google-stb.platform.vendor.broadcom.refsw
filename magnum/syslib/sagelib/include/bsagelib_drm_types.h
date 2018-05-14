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

#ifndef BSAGELIB_DRM_TYPES_H__
#define BSAGELIB_DRM_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary: This enum defines the DRM types supported.
**/
typedef enum BSAGElib_BinFileDrmTypes_e
{
    BSAGElib_BinFileDrmType_eNetflix = 0x00010001,
    BSAGElib_BinFileDrmType_eWidevine,
    BSAGElib_BinFileDrmType_eDtcpIp,
    BSAGElib_BinFileDrmType_ePlayready,
    BSAGElib_BinFileDrmType_eSecureRsa,
    BSAGElib_BinFileDrmType_eCustomPrivate,
    BSAGElib_BinFileDrmType_eAdobeAxcess,
    BSAGElib_BinFileDrmType_eHdcp22Rx,
    BSAGElib_BinFileDrmType_eHdcp22Tx,
    BSAGElib_BinFileDrmType_eSslCerts,
    BSAGElib_BinFileDrmType_eGeneric,
    BSAGElib_BinFileDrmType_eEdrm,
    BSAGElib_BinFileDrmType_eEcc,
    BSAGElib_BinFileDrmType_ePlayready30,
    BSAGElib_BinFileDrmType_eMediaroom,
    BSAGElib_BinFileDrmType_eBp3,
    BSAGElib_BinFileDrmType_eSecuremedia,
    BSAGElib_BinFileDrmType_eHdcp14Rx,
    BSAGElib_BinFileDrmType_eHdcp14Tx,
    BSAGElib_BinFileDrmType_eMarlin,
    BSAGElib_BinFileDrmType_eKeyMaster,
    BSAGElib_BinFileDrmType_eWVCert,
    BSAGElib_BinFileDrmType_eCSecureRsa,
    BSAGElib_BinFileDrmType_eNTKL,
    BSAGElib_BinFileDrmType_eBSecureRsa,
    BSAGElib_BinFileDrmType_eMax
} BSAGElib_BinFileDrmTypes_e;

#define MAX_NUMBER_BINFILE_DRM_TYPES (BSAGElib_BinFileDrmType_eMax - BSAGElib_BinFileDrmType_eNetflix)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSAGELIB_DRM_TYPES_H__ */
