/******************************************************************************
 *	  (c)2010-2013 Broadcom Corporation
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
 * 1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/

#include <stdio.h>

typedef enum TftpDownloadTrigger
{
	kTftpDownloadTriggerEcmCfgFile		= 0,
	kTftpDownloadTriggerSnmp		= 1,
	kTftpDownloadTriggerCvt		        = 2
}TftpDownloadTrigger;


unsigned int EstbSwUpgrade_IsUpgradePermittedNow(void);
unsigned int EstbSwUpgrade_IsMonolithicImageHeaderValid(void);
unsigned int EstbSwUpgrade_IsMonolithicImageValid(void);
void EstbSwUpgrade_Notification(unsigned int eventType);
int EstbSwUpgrade_InProgress(TftpDownloadTrigger trigger);
void EstbSwUpgrade_InProgressSetCallback( int (*callback)(TftpDownloadTrigger));
void EstbSwUpgrade_IsUpgradePermittedNowSetCallback( int (*callback)(void));
void EstbSwUpgrade_IsMonolithicImageHeaderValidSetCallback(unsigned int (*callback) (void) );
void EstbSwUpgrade_IsMonolithicImageValidSetCallback(unsigned int (*callback) (void) );
/*	Please note that these values will be obsolete
	when these notifications are replaced with
	Common Download API notifications
*/
typedef enum SwUpgradeNotification
{
	kEcmDownloadFail		= 0,
	kEcmDownloadOk			= 1,

	kMonolithicDownloadFail = 2,
	kMonolithicDownloadOk	= 3
}SwUpgradeNotification;


/*  This is the name of the symbolic link that points to the
 * storage where NEWLY DOWNLOADED MONOLITH is stored.
 * This link MUST be setup prior to software download.
 */
#define kDefaultSymLinkToNewMonolith		"monolith_prog_storage"

/* This is the default storage where newly downloaded monolith is stored. */
/* The symbolic link "kDefaultSymLinkToNewMonolith" points to this location as default.
 *
 * Example: monolith_prog_storage -> mtd_block1
 */

/*#warning "VENDOR EXTENSION - kDefaultNewMonolithStoragePath must be customized!"*/
#define kDefaultNewMonolithStoragePath		"mtd_block1"   /* Ex.:"/dev/mtdocap5" */


/* This is the name of the symbolic link that points to the
 * storage where NEWLY DOWNLOADED eCM IMAGE is stored.
 * This link MUST be setup prior to software download.
 */
#define kDefaultSymLinkToNewEcmImage		"ecm_prog_storage"

/* This is the default storage where newly downloaded ecm image is stored. */
/* The symbolic link "kDefaultSymLinkToNewEcmImage" points to this location as default.
 *
 * Example: ecm_prog_storage -> mtd_block3
 */

/*#warning "VENDOR EXTENSION - kDefaultNewEcmImageStoragePath must be customized!"*/
#define kDefaultNewEcmImageStoragePath		"mtd_block3"  /* Ex.:"/dev/mtd_block3" */



/* This is expected to be used only when eCM is operating in DOCSIS-only mode*/
/* This is the name of the symbolic link that points to the
 * storage where OPERATIONAL eCM IMAGE is stored.
 * This link MUST be setup prior to software download.
 */
#define kDefaultSymLinkToOperationalEcmImage	"ecram_sto.bin.3255" /* Ex.:"ecram_sto.bin.7118" */

/* This is the storage where operational ecm images are stored.
 *
 * Example: ecram_sto.bin.3255 -> /dev/mtd_block4
 */

/*#warning "VENDOR EXTENSION - kDefaultOperationalEcmImageStoragePath must be customized!"*/
#define kDefaultOperationalEcmImageStoragePath	 "/dev/mtdocap3"

/* This is the default TFTP server path */
/* If defined, eSTB will find the new eCM image at this path
 * If not defined, eSTB will find the eCM image at the current working directory
 */
/* #define kDefaultValue_EstbTftpServerPath  "/tmp/share/eCM" */

/* This second operational image can be specified to implement dual boot */
/* IMPORTANT: Comment out this line to enable dual boot. Please note that
 * in order for dual boot to work flash partitions must be configured properly */
/* #define kDefaultOperationalEcmImageStoragePath2	 "/dev/mtdocap4" */

/* This function lets upper layer app to set the temporary file name for downloaded
 * monolithic image, before it's written to non-volatile device.
 */
void SetMonolithicImageTempFileName(char * fn) ;
/* This function returns the temporary file name for downloaded
 * monolithic image, before it's written to non-volatile device.
 */
char * GetMonolithicImageTempFileName(void) ;
