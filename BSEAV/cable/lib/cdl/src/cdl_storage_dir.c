/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
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
 * Module Description:  OpenCable Common Download storage interface. Functions
 *                      in this module provides image storage. Expand the mono
 *                      image as local files. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "cdl_common.h"
#include "b_cdl_lib.h"
#include "cdl_image_brcm.h"
#include "cdl_storage_dir.h"

BDBG_MODULE(cdl_storage_dir);

struct cdl_storage_status {
	int max_reboot_count;
	int cur_reboot_count;
};

struct cdl_storage {
	char pathname[256];
	int fd;
	int ref_count;
	struct cdl_storage_status status;
};

typedef struct cdl_storage * cdl_storage_t;

void * cdl_storage_dir_open(const char *pathname, int flags) {
	cdl_storage_t h;
	int fd = 0;

	BDBG_MSG(("Enter %s\n", __FUNCTION__));

	if ((h = malloc(sizeof(struct cdl_storage))) == NULL) {
		BDBG_ERR(("malloc failed"));
		return NULL;
	}
	memset(h, 0, sizeof (struct cdl_storage));
	if (h->ref_count) {
		BDBG_ERR(("storage in use\n"));
		return NULL;
	}
	if ((fd = open(pathname, flags)) < 0) {
		BDBG_ERR(("Cannot open %s\n", pathname));
		return NULL;
	}
	memset(h, 0, sizeof (struct cdl_storage));
	if (h->ref_count) {
		BDBG_ERR(("storage in use\n"));
		return NULL;
	}

	memset(h, 0, sizeof(struct cdl_storage));
	strncpy(h->pathname, pathname, 256);
	h->fd = fd;
	h->ref_count++;
	return (void *)h;
}
int cdl_storage_dir_close(void * handle) {
	cdl_storage_t h = (cdl_storage_t) handle;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	if (h->ref_count) h->ref_count--;
	if (h->ref_count == 0) {
		close(h->fd);
	}
	free(h);
	return 0;
}
/* return >=0 if succeed, -1 if fail */
int cdl_storage_dir_read(void * handle, void * buf, size_t count) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return read(h->fd, buf, count);
}
/* return >=0 if succeed, -1 if fail */
int cdl_storage_dir_write(void * handle, const void * buf, size_t count) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return write(h->fd, buf, count);
}
int cdl_storage_dir_lseek(void * handle, int offset, int whence) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return lseek(h->fd, offset, whence);
}


int cdl_storage_dir_expand(void * handle, char * fn, int sec_header_len) {
	cdl_storage_t h = (cdl_storage_t) handle;
	int n, left;
	int fd = -1;
	char devname[64];
	struct cdl_image_header  * hdr = NULL;
	char * buf = NULL;
	int ret = -1;
	int len = sizeof(struct cdl_image_header);
	int total_len = 0;
	int i; /* For the Nand Flash */
    struct stat stat;
    char filepath[2*MAX_DEV_STR_LEN];
    int image_size = 0;

	BDBG_MSG(("Enter %s\n", __FUNCTION__));


	if ( (hdr = (struct cdl_image_header  *) malloc(len)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		goto out;
	}
	if ( (buf = (char  *) malloc(BUF_SIZE)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		goto out;
	}


	if ( (total_len = cdl_storage_dir_lseek(h, 0, SEEK_END)) < 0 ) {
		BDBG_ERR(("lseek error, got  %d\n", total_len));
		goto out;
	}

	if ( (ret = cdl_storage_dir_lseek(h, sec_header_len, SEEK_SET)) != sec_header_len) {
		BDBG_ERR(("lseek error, got  %d, expect %d bytes\n", ret, sec_header_len));
	}
	
	if ( (ret = cdl_storage_dir_read(h, hdr, len)) < len) {
		BDBG_ERR(("read error, read %d, expect %d bytes\n", ret, len));
	}

	/* print_image_header((struct cdl_image_header *)hdr); */

	strncpy(devname, fn, 64);
    ret = lstat(devname, &stat);
    if ( ret < 0) {
        BDBG_ERR(("%s: failed to stat %s", __FUNCTION__, devname));
        goto out;
    }
    if (S_ISDIR(stat.st_mode) == 0) {
        BDBG_ERR(("%s: %s is not a directory ", __FUNCTION__, devname));
        goto out;        
    } 

    /* write image header */
    snprintf(filepath, sizeof(filepath), "%s/%s", devname, "image_header.bin");
    if ((fd = open(filepath, O_WRONLY | O_CREAT)) < 0) {
        BDBG_ERR(("cannot open %s\n", filepath));
        goto out;
    }
    left = sizeof(struct cdl_image_header);
		
    ret = write(fd, (unsigned char *)hdr, left);
    if (ret < left) {
        BDBG_ERR(("%s write error, write %d, expect %d bytes\n", filepath, ret, left));
        ret = -1;
        goto out;
    }
    close(fd);


	/* expand idividual images */
	for ( i = 0; i < hdr->num; i++) {

		image_size = hdr->v.v1[i].size;

        snprintf(filepath, sizeof(filepath), "%s/%s", devname, (char *)&hdr->v.v1[i].filename);

        if ((fd = open(filepath, O_WRONLY | O_CREAT)) < 0) {
            BDBG_ERR(("cannot open %s\n", filepath));
            goto out;
        }

		/* write to file */
		left = image_size;
		
		do {
			n =  (left < BUF_SIZE) ? left : BUF_SIZE;
			if ( (ret = cdl_storage_dir_read(h, buf, n)) < n) {
				BDBG_ERR(("Read error, read %d, expect %d bytes\n", ret, n));
				goto out;
			}
			ret = write(fd, buf, n);
			if (ret < n) {
				BDBG_ERR(("%s write error, write %d, expect %d bytes\n", devname, ret, n));
				ret = -1;
				goto out;
			}
			left -= n;
		} while ( left > 0);
		BDBG_ERR(("Written %d bytes to %s\n", image_size, filepath));
        close(fd);
	}



    left = cdl_storage_dir_lseek(h, 0, SEEK_CUR);
    /* skip padding */
    if (left % 4) {
        left = (left/4) * 4 + 4;
        ret = cdl_storage_dir_lseek(h, left, SEEK_SET);
        if (ret != left) {
            BDBG_ERR(("Cannot seek to 0x%lx", left));
            ret = -1;
            goto out;
        }
    }
    if ((left < total_len) && (left > 0)) {
        /* write image tail if any */
		image_size = total_len - left;
        snprintf(filepath, sizeof(filepath), "%s/%s", devname, "image_tail.bin");
        if ((fd = open(filepath, O_WRONLY | O_CREAT)) < 0) {
            BDBG_ERR(("cannot open %s\n", filepath));
            goto out;
        }
        left = image_size;
		do {
			n =  (left < BUF_SIZE) ? left : BUF_SIZE;
			if ( (ret = cdl_storage_dir_read(h, buf, n)) < n) {
				BDBG_ERR(("Read error, read %d, expect %d bytes\n", ret, n));
				goto out;
			}
			ret = write(fd, buf, n);
			if (ret < n) {
				BDBG_ERR(("%s write error, write %d, expect %d bytes\n", devname, ret, n));
				ret = -1;
				goto out;
			}
			left -= n;
		} while ( left > 0);
		BDBG_ERR(("Written %d bytes to %s\n", image_size, filepath));
        close(fd);
    }

	ret = 0;
 out:
	if (buf) free(buf);
	if (hdr) free(hdr);
	if (fd != -1) close(fd);
	return ret;
}
