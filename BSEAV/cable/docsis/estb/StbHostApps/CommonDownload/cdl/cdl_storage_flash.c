/***************************************************************************
 *     Copyright (c) 2007-2014, Broadcom Corporation
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
 * Module Description:  OpenCable Common Download storage interface. Functions
 *                      in this module provides image storage
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
#include "cdl_image_brcm.h"
#include "cdl_storage_flash.h"

BDBG_MODULE(cdl_storage_flash);

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


static int erase_flash_partition(char * devname) {
#if 0
	char tmp[256];
	sprintf(tmp, "eraseall %s", devname);
	return system(tmp);
#else
	BDBG_ERR(("Erase is done in mtdocap while writing\n"));
	return 0;
#endif
}

void * cdl_storage_flash_open(const char *pathname, int flags) {
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
	/* erase the device first */
	/*
	if ((flags & O_WRONLY) || (flags & O_RDWR)) {
		char tmp[256];
		sprintf(tmp, "eraseall %s",pathname); 
		system(tmp);
	}
	*/
	if ((fd = open(pathname, flags)) < 0) {
		BDBG_ERR(("Cannot open %s\n", pathname));
		return NULL;
	}
	memset(h, 0, sizeof(struct cdl_storage));
	strncpy(h->pathname, pathname, 256);
	h->fd = fd;
	h->ref_count++;
	return (void *)h;
}
int cdl_storage_flash_close(void * handle) {
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
int cdl_storage_flash_read(void * handle, void * buf, size_t count) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return read(h->fd, buf, count);
}
/* return >=0 if succeed, -1 if fail */
int cdl_storage_flash_write(void * handle, const void * buf, size_t count) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return write(h->fd, buf, count);
}
int cdl_storage_flash_lseek(void * handle, int offset, int whence) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return lseek(h->fd, offset, whence);
}

int cdl_storage_flash_expand(void * handle, char * fn, int sec_header_len) {
	cdl_storage_t h = (cdl_storage_t) handle;
	int n, left;
	int fd = -1;
	char devname[64];
	struct cdl_image_header  * hdr = NULL;
	char * buf = NULL;
	int ret = -1;
	int len = sizeof(struct cdl_image_header);
	int total_len = 0;

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


	if ( (total_len = cdl_storage_flash_lseek(h, 0, SEEK_END)) < 0 ) {
		BDBG_ERR(("lseek error, got  %d\n", total_len));
		goto out;
	}

	if ( (ret = cdl_storage_flash_lseek(h, sec_header_len, SEEK_SET)) != sec_header_len) {
		BDBG_ERR(("lseek error, got  %d, expect %d bytes\n", ret, sec_header_len));
	}
	
	if ( (ret = cdl_storage_flash_read(h, hdr, len)) < len) {
		BDBG_ERR(("read error, read %d, expect %d bytes\n", ret, len));
	}
	/* print_image_header((struct cdl_image_header *)hdr);*/

	/* erase flash partition */
	strncpy(devname, fn, 64);
	if (erase_flash_partition(devname) < 0) {
		BDBG_ERR(("Cannot erase %s\n", devname));
		goto out;			
	}

	if ((fd = open(devname, O_WRONLY)) < 0) {
		BDBG_ERR(("cannot open %s\n", devname));
		goto out;
	}

	/* seek to the start of data */
	if ( (ret = cdl_storage_flash_lseek(h, sec_header_len, SEEK_SET)) != sec_header_len) {
		BDBG_ERR(("lseek error, got  %d, expect %d bytes\n", ret, sec_header_len));
	}
	
	/* write flash partition */
	left = total_len - sec_header_len;
	do {
		n =  (left < BUF_SIZE) ? left : BUF_SIZE;
		if ( (ret = cdl_storage_flash_read(h, buf, n)) < n) {
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

	ret = 0;
 out:
	if (buf) free(buf);
	if (hdr) free(hdr);
	if (fd != -1) close(fd);
	return ret;
}
