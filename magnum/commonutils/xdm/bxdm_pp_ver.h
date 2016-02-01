/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef bxdm_pp_VER_H__
#define bxdm_pp_VER_H__

/* DM version is split into MAJOR.MINOR.SUBMINOR.
 *
 * MAJOR should be incremented when changes are made that could break
 * existing applications.  Major re-writes, changes to default
 * accepted behavior, etc would cause the MAJOR number fo increase
 *
 * MINOR should be incremented when features are added that are
 * backwards compatible with existing applications
 *
 * SUBMINOR should be incremented when bug fixes are made to existing
 * functionality
 *
 */
#define BXDM_PictureProvider_P_VERSION_MAJOR    3
#define BXDM_PictureProvider_P_VERSION_MINOR    0
#define BXDM_PictureProvider_P_VERSION_SUBMINOR 0

#endif /* #ifndef bxdm_pp_VER_H__ */
