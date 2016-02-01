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
#include "b_cdl_lib.h"
#include "cdl_image_brcm.h"
#include "cdl_storage_file.h"

BDBG_MODULE(cdl_storage_file);

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

struct cfe_flash_partition {
	char name[32];
	unsigned int offset;
	unsigned int size;
};
#define MAX_NUM_PARTITIONS 20
static struct cfe_flash_partition cfe_flash_partitions[MAX_NUM_PARTITIONS];
static int num_partitions = 0;
static char cfe_map_fn[64] = "./flash_partition.txt";

static int simulate_erase_flash_partition(char * devname) {
	int ret = -1;
	int fd = -1;
	char * buf = NULL;
	int bytes = 0;
	int n;
	unsigned int flash_size = 0;

	if ((fd = open(devname, O_CREAT|O_WRONLY)) < 0) {
		BDBG_ERR(("Cannot open %s\n", devname));
		goto out;
	}
	if ( (buf = (char  *) malloc(BUF_SIZE)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		goto out;
	}

	/* fill the image with 0xff */
	memset(buf, 0xff, BUF_SIZE);

	/* guess the flash size, choices are 32M, 64M, and 128M */
	flash_size = cfe_flash_partitions[num_partitions-1].offset;
	BDBG_ERR(("max offset 0x%x\n", flash_size));
	if (flash_size > 0x4000000) {
		flash_size = 0x8000000; /* 128M*/
	} else if (flash_size > 0x2000000) {
		flash_size = 0x4000000; /* 64M*/
	} else {
		flash_size = 0x2000000; /* 32M*/		
	}
	BDBG_ERR(("Guessed flash size 0x%x\n", flash_size));
	while (bytes < flash_size) {
		if ( (n = write(fd, buf, BUF_SIZE)) < 0) {
			BDBG_ERR(("write to %s failed\n", devname));
			goto out;
		}
		bytes += n;
	}
	ret = 0;
	
 out:
	if (buf) free(buf);
	if (fd >= 0) close(fd);
	return ret;
}

void * cdl_storage_file_open(const char *pathname, int flags) {
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
int cdl_storage_file_close(void * handle) {
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
int cdl_storage_file_read(void * handle, void * buf, size_t count) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return read(h->fd, buf, count);
}
/* return >=0 if succeed, -1 if fail */
int cdl_storage_file_write(void * handle, const void * buf, size_t count) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return write(h->fd, buf, count);
}
int cdl_storage_file_lseek(void * handle, int offset, int whence) {
	cdl_storage_t h = (cdl_storage_t) handle;

	/* BDBG_MSG(("Enter %s, %d\n", __FUNCTION__, count)); */
	return lseek(h->fd, offset, whence);
}

static int get_cfe_partitions (char * fn) {
	int ret = -1;
	FILE * fp = NULL;
	int bytes = 0;
	int n = 0;
	char * buf = NULL;

	/* parse the cfe flash partition */
	memset(cfe_flash_partitions, 0, sizeof(cfe_flash_partitions));

	if ((fp = fopen(fn, "rt")) < 0) {
		BDBG_ERR(("Cannot open %s\n", fn));
		goto out;
	}
	
	if ( (buf = (char  *) malloc(4096)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		goto out;
	}
	
	while (fgets(buf, 4096, fp) && n < MAX_NUM_PARTITIONS) {
		sscanf(buf, "%s %x\n", cfe_flash_partitions[n].name, &cfe_flash_partitions[n].offset);
		BDBG_ERR(("parsed cfe partition %s; 0x%08x\n", cfe_flash_partitions[n].name, 
			  cfe_flash_partitions[n].offset));
		n++;
	}

	num_partitions = n;
	ret = 0;
	
 out:
	if (buf) free(buf);
	if (fp) fclose(fp);
	return ret;
}

static int get_cfe_offset(char *flashdev) {
	int ret = -1;
	int i = 0;
	for ( i = 0; i < num_partitions; i++) {
		BDBG_MSG(("flashdev %s cfe partition %s, offset %x ret %x\n", flashdev, 
			  cfe_flash_partitions[i].name, cfe_flash_partitions[i].offset, ret ));
		if ( memcmp(flashdev, cfe_flash_partitions[i].name, strlen(flashdev)) == 0) {
			ret = cfe_flash_partitions[i].offset;
			break;
		}
	}
	return ret;
}

int cdl_storage_file_expand(void * handle, char * fn, int sec_header_len) {
	cdl_storage_t h = (cdl_storage_t) handle;
	int n, left;
	int fd = -1;
	char devname[64];
	struct cdl_image_header  * hdr = NULL;
	char * buf = NULL;
	int ret = -1;
	int len = sizeof(struct cdl_image_header);
	int total_len = 0;
	int padding_length, i; /* For the Nand Flash */

	BDBG_MSG(("Enter %s\n", __FUNCTION__));

	if (get_cfe_partitions(cfe_map_fn) < 0) {
		ret = -1;
		goto out;
	}
	

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


	if ( (total_len = cdl_storage_file_lseek(h, 0, SEEK_END)) < 0 ) {
		BDBG_ERR(("lseek error, got  %d\n", total_len));
		goto out;
	}

	if ( (ret = cdl_storage_file_lseek(h, sec_header_len, SEEK_SET)) != sec_header_len) {
		BDBG_ERR(("lseek error, got  %d, expect %d bytes\n", ret, sec_header_len));
	}
	
	if ( (ret = cdl_storage_file_read(h, hdr, len)) < len) {
		BDBG_ERR(("read error, read %d, expect %d bytes\n", ret, len));
	}

	/* print_image_header((struct cdl_image_header *)hdr); */

	/* erase flash partition */
	strncpy(devname, fn, 64);
	if (simulate_erase_flash_partition(devname) < 0) {
		BDBG_ERR(("Cannot erase %s\n", devname));
		goto out;			
	}

	if ((fd = open(devname, O_WRONLY)) < 0) {
		BDBG_ERR(("cannot open %s\n", devname));
		goto out;
	}

	/* expand idividual images */
	for ( i = 0; i < hdr->num; i++) {
		int image_size = 0;
		char * flashdev;
		int dst_offset;
		image_size = hdr->v.v1[i].size;
		flashdev = (char *)&hdr->v.v1[i].devname;
		if ((dst_offset = get_cfe_offset(flashdev)) < 0) {
			BDBG_ERR(("Cannot find partition %s", flashdev));
			goto out;
		}
		
		if (lseek(fd, dst_offset, SEEK_SET) < 0) {
			BDBG_ERR(("Cannot seek to 0x%08x", dst_offset));
			goto out;
		}
		/* write to file */
		left = image_size;
		
		do {
			n =  (left < BUF_SIZE) ? left : BUF_SIZE;
			if ( (ret = cdl_storage_file_read(h, buf, n)) < n) {
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
		BDBG_ERR(("Written %d bytes to %s offset 0x%x\n", image_size, flashdev, dst_offset));
	}

	ret = 0;
 out:
	if (buf) free(buf);
	if (hdr) free(hdr);
	if (fd != -1) close(fd);
	return ret;
}
