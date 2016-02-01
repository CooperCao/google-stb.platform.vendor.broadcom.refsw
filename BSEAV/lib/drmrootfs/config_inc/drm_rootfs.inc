/* #############################################################################
#    (c)2013 Broadcom Corporation
# 
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
#  
# Except as expressly set forth in the Authorized License,
#  
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#  
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
# USE OR PERFORMANCE OF THE SOFTWARE.
# 
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
# ANY LIMITED REMEDY.
#
##############################################################################
*/
/*
// IMPORTANT:  This file should be included EXCLUSIVELY by drm_data.cpp.
*/

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
** VARIABLE:
**   drm_key_region_tmp
**
** DESCRIPTION:
**   This configuration variable applies to users/customers with NAND flash support.
**   It should be ignored otherwise.
**   'drm_key_region_tmp_dir' points to a directory at run time that the Common 
**   DRM library will use to process the DRM bin file from flash.
**
** NOTES:
**   This file must only be included in drm_data.h. 
**
**  Refer to drm_data.h for more details.
**
******************************************************************************/
char drm_key_region_tmp_dir[] ="/tmp/";

/******************************************************************************
** VARIABLE:
**   drm_bin_file_rootfs_path
**
** DESCRIPTION:
**   This configuration variable holds the file path where the DRM binary file
**   exists in rootfs.
**
** NOTES:
**   This file must only be included in drm_data.h. 
**
**  Refer to drm_data.h for more details.
**
******************************************************************************/
#ifdef ANDROID
char drm_bin_file_rootfs_path[] ="/mnt/hwcfg/drm.bin";
#else
char drm_bin_file_rootfs_path[] ="./drm.bin";
#endif


/******************************************************************************
** VARIABLE:
**   pmc_bin_file_dir
**
** DESCRIPTION:
**   This configuration variable holds the path to the directory (only) where
**   the PMC bin file will be located
**
** NOTES:
**   This file must only be included in drm_data.h. 
**
**  Refer to drm_data.h for more details.
**
******************************************************************************/
char pmc_bin_file_dir[] ="/mnt/";

/******************************************************************************
** VARIABLE:
**   pmc_bin_file_name
**
** DESCRIPTION:
**   This configuration variable holds the file name (only) of the PMC bin file
**
** NOTES:
**   This file must only be included in drm_data.h. 
**
**  Refer to drm_data.h for more details.
**
******************************************************************************/
char pmc_bin_file_name[] ="pmc.bin";

#ifdef __cplusplus
}
#endif
