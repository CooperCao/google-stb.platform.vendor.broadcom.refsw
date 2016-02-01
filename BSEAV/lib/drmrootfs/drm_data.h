/***************************************************************************
*     (c)2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
**************************************************************************/

#ifndef DRM_DATA_H
#define DRM_DATA_H

#include "bstd.h"

typedef uint16_t _DRM_SHORT_WCHAR_;

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
** FUNCTION:
**   bdrm_get_pmc_bin_file_dir
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the folder that
**   the PMC core module will use to process the pmc.bin file
**
** PARAMETERS: N/A
**
** RETURNS: string (char *)
**
******************************************************************************/
char * bdrm_get_pmc_bin_file_dir(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_pmc_bin_file_name
**
** DESCRIPTION:
**   This function is used to retrieve the string representing the filename
**   of the pmc.bin file
**
** PARAMETERS: N/A
**
** RETURNS: string (char *)
**
******************************************************************************/
char * bdrm_get_pmc_bin_file_name(void);


/******************************************************************************
** FUNCTION:
**   bdrm_get_drm_key_region_tmp_dirname
**
** DESCRIPTION:
**   This function applies to users/customers with NAND flash - ignore else.
**   Common DRM will use the folder string returned by this API when
**   processing the DRM bin file from NAND flash.
**
** PARAMETERS: N/A
**
** RETURNS: N/A
**
******************************************************************************/
char * bdrm_get_drm_key_region_tmp_dirname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_drm_bin_file_path
**
** DESCRIPTION:
**   This function is used to retrieve the string representing the filepath
**   of the DRM bin file in rootfs
**
** PARAMETERS: N/A
**
** RETURNS: N/A
**
******************************************************************************/
char * bdrm_get_drm_bin_file_path(void);



_DRM_SHORT_WCHAR_ * bdrm_get_revocation_fname(void);
uint32_t  bdrm_get_revocation_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_cert_dir
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the folder where
** the certificates will be saved in the file system.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the folder where the certificates will be saved in the
** file system.
**
** SEE ALSO:
**  bdrm_get_cert_dir_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_cert_dir(void);


/******************************************************************************
** FUNCTION:
**   bdrm_get_pr_bin_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the path of the
** Playready DRM bin file.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the path of the Playready DRM bin file in the file system.
**
** SEE ALSO:
**  bdrm_get_cert_dir_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_pr_bin_fname(void);


/******************************************************************************
** FUNCTION:
**   bdrm_get_key_history_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the path of the
** Key History file.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the path of the Key History file in the file system.
**
** SEE ALSO:
**  bdrm_get_cert_dir_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_key_history_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_key_file_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the path of the
** Key file.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the path of the Key file in the file system.
**
** SEE ALSO:
**  bdrm_get_key_file_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_key_file_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_key_file_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the string containing the
** key file filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the string in bytes.
**
** SEE ALSO:
**  bdrm_get_key_file_fname
**
******************************************************************************/
uint32_t  bdrm_get_key_file_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_model_info
**
** DESCRIPTION:
**   This function is used to retrieve the string the model information.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the model information.
**
** SEE ALSO:
**  bdrm_get_model_info_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_model_info(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_model_info_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the string containing the
** model information.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the string in bytes.
**
** SEE ALSO:
**  bdrm_get_model_info
**
******************************************************************************/
uint32_t  bdrm_get_model_info_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_cert_dir_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the string containing the
** folder where the certificates will be saved in the file system.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the string in bytes.
**
** SEE ALSO:
**  bdrm_get_cert_dir
**
******************************************************************************/
uint32_t  bdrm_get_cert_dir_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_cert_templates_dir
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the folder where
** the certificates templates will be saved in the file system.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the folder where the certificates templates will be
** saved in the file system.
**
** SEE ALSO:
**  bdrm_get_cert_templates_dir_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_cert_templates_dir(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_cert_templates_dir_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the string containing the
** folder where the certificates templates will be saved in the file system..
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the string in bytes.
**
** SEE ALSO:
**  bdrm_get_cert_templates_dir
**
******************************************************************************/
uint32_t  bdrm_get_cert_templates_dir_lgth(void);


/******************************************************************************
** FUNCTION:
**   bdrm_get_hds_dir
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the folder where the
** data store files will be saved in the file system.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the folder where the data store files will be saved in
** the file system.
**
** SEE ALSO:
**  bdrm_get_hds_dir_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_hds_dir(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_hds_dir_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the string containing the
** folder where the data store files will be saved in the file system.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the string in bytes.
**
** SEE ALSO:
**  bdrm_get_hds_dir
**
******************************************************************************/
uint32_t  bdrm_get_hds_dir_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_key_file_dir
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the folder where the
** key file will be saved in the file system.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the folder where the key file will be saved in
** the file system.
**
** SEE ALSO:
**  bdrm_get_key_file_dir_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_key_file_dir(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_key_file_dir_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the string containing the
** folder where the key file will be saved in the file system.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the string in bytes.
**
** SEE ALSO:
**  bdrm_get_key_file_dir_lgth
**
******************************************************************************/
uint32_t  bdrm_get_key_file_dir_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_hds_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the data store filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the data store filename.
**
** SEE ALSO:
**  bdrm_get_hds_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_hds_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_hds_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the data store filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_hds_fname
**
******************************************************************************/
uint32_t  bdrm_get_hds_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_hds_pwd_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the data store
** password filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the data store password filename.
**
** SEE ALSO:
**  bdrm_get_hds_pwd_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_hds_pwd_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_hds_pwd_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the data store password
** filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_hds_pwd_fname
**
******************************************************************************/
uint32_t  bdrm_get_hds_pwd_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_dev_cert_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the device certificate
** filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   A string containing the device certificate filename.
**
** SEE ALSO:
**  bdrm_get_dev_cert_fname
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_dev_cert_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_dev_cert_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the device certificate filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_dev_cert_fname
**
******************************************************************************/
uint32_t  bdrm_get_dev_cert_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_zdev_cert_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the binary format
** device certificate filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**  A string containing the binary format device certificate filename.
**
** SEE ALSO:
**  bdrm_get_zdev_cert_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_zdev_cert_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_zdev_cert_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the binary format device
** certificate filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_zdev_cert_fname
**
******************************************************************************/
uint32_t  bdrm_get_zdev_cert_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_ndr_cert_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the NRD certificate
** filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**  A string containing the NRD certificate filename.
**
** SEE ALSO:
**  bdrm_get_ndr_cert_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_ndr_cert_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_ndr_cert_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the NRD certificate filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_ndr_cert_fname
**
******************************************************************************/
uint32_t  bdrm_get_ndr_cert_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_ndr_cert_template_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the NRD certificate
** template filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**  A string containing the NRD certificate template filename.
**
** SEE ALSO:
**  bdrm_get_ndr_cert_template_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_ndr_cert_template_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_ndr_cert_template_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the NRD certificate template
** filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_ndr_cert_template_fname
**
******************************************************************************/
uint32_t  bdrm_get_ndr_cert_template_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_zgpriv_sig_key_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the device signing
** public key filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**  A string containing the device signing public key filename.
**
** SEE ALSO:
**  bdrm_get_zgpriv_sig_key_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_zgpriv_sig_key_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_zgpriv_sig_key_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the device signing public key
** filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_zgpriv_sig_key_fname
**
******************************************************************************/
uint32_t  bdrm_get_zgpriv_sig_key_fname_lgth(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_zgpriv_enc_key_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the device encryption
** key filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**  A string containing the device encryption key filename.
**
** SEE ALSO:
**  bdrm_get_zgpriv_enc_key_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_zgpriv_enc_key_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_zgpriv_enc_key_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the device encryption key
** filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_zgpriv_enc_key_fname
**
******************************************************************************/
uint32_t  bdrm_get_zgpriv_enc_key_fname_lgth(void);


/******************************************************************************
** FUNCTION:
**   bdrm_get_ndr_priv_key_fname
**
** DESCRIPTION:
**   This function is used to retrieve the string containing the NDR private key
** filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**  A string containing the NDR private key filename.
**
** SEE ALSO:
**  bdrm_get_ndr_priv_key_fname_lgth
**
******************************************************************************/
_DRM_SHORT_WCHAR_ * bdrm_get_ndr_priv_key_fname(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_ndr_priv_key_fname_lgth
**
** DESCRIPTION:
**   This function is used to retrieve the length of the NDR private key filename.
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The length of the filename in bytes.
**
** SEE ALSO:
**  bdrm_get_ndr_priv_key_fname
**
******************************************************************************/
uint32_t  bdrm_get_ndr_priv_key_fname_lgth(void);


/**************************************Adobe Axcess related function***********************************/
/******************************************************************************
** FUNCTION:
**   bdrm_get_adobe_key_bin_file_path
**
** DESCRIPTION:
**   This function is used to retrieve the path of the Adobe Axcess key bin file
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The filename .string
**
** SEE ALSO:
**
**
******************************************************************************/
char * bdrm_get_adobe_key_bin_file_path(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_has_locked_rsa_pkg_file_path
**
** DESCRIPTION:
**   This function is used to retrieve the path of the Adobe hash locked key bin file
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The filename string
**
** SEE ALSO:
**
**
******************************************************************************/
char * bdrm_get_hash_locked_rsa_pkg_file_path(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_hash_locked_rsa_pkg_load_flag
**
** DESCRIPTION:
**   This function is used to retrieve the flasg which indicates whther hashlocked has been loaded or not
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   true or false
**
** SEE ALSO:
**
**
******************************************************************************/
bool bdrm_get_hash_locked_rsa_pkg_load_flag(void);



/******************************************************************************
** FUNCTION:
**   bdrm_get_hash_storage_data_section
** DESCRIPTION:
**   This function is used to retrieve the flasg which indicates whther hashlocked has been loaded or not
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   true or false
**
** SEE ALSO:
**
**
******************************************************************************/
uint8_t bdrm_get_hash_storage_data_section(void);


/******************************************************************************
** FUNCTION:
**   bdrm_get_otpKeyDataSection
** DESCRIPTION:
**   This function is used to retrieve the flasg which indicates whther hashlocked has been loaded or not
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   true or false
**
** SEE ALSO:
**
**
******************************************************************************/
uint8_t bdrm_get_otp_key_data_section(void);


uint8_t bdrm_get_verification_pubkey_offset(void);
uint8_t bdrm_get_verification_pubkey_index(void);
uint8_t bdrm_get_hashlocked_pkg_privkey_offset(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_adobe_shared_domain_key_load_flag
**
** DESCRIPTION:
**   This function is used to retrieve the flasg which indicates whether shared domain key has to be loaded or not
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   true or false
**
** SEE ALSO:
**
**
******************************************************************************/
bool bdrm_get_adobe_shared_domain_key_load_flag(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_widevine_keybox_file_path
**
** DESCRIPTION:
**   This function is used to retrieve the path of the widevine keybox file
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The filename string
**
** SEE ALSO:
**
**
******************************************************************************/
char * bdrm_get_widevine_keybox_file_path(void);

/******************************************************************************
** FUNCTION:
**   bdrm_get_shared_domain_key_bin_file_path
**
** DESCRIPTION:
**   This function is used to retrieve the path of the Adobe shared domain key bin file
**
** PARAMETERS:
**   N/A
**
** RETURNS:
**   The filename .string
**
** SEE ALSO:
**
**
******************************************************************************/
char * bdrm_get_shared_domain_key_bin_file_path(void);


#ifdef __cplusplus
}
#endif


#endif /*DRM_DATA_H*/
