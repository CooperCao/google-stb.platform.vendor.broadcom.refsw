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
 * Module Description:  OpenCable Common Download monolithic image functions
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cdl_common.h"
#include "cdl_image_brcm.h"

BDBG_MODULE(cdl_image_brcm);

#define CDL_FILE_TYPE_INVALID  0  /* invalid type */
#define CDL_FILE_TYPE_ECMBOOT  1  /* eCM bootloader */
#define CDL_FILE_TYPE_DOCSIS0  2  /* DOCSIS0 image */
#define CDL_FILE_TYPE_CFE      3  /* eSTB bootloader CFE */
#define CDL_FILE_TYPE_KERNEL   4  /* eSTB kernel */
#define CDL_FILE_TYPE_ROOTFS   5  /* eSTB root file system */


/*
 * build monolithic image from the file names in <param.txt>
 * Example of param.txt
 * # cfe partition,   file name
 * cfe.ecmboot        ./signedsecure3255.bin
 * cfe.docsis0        ./ecram_sto.bin
 */

struct file_info {
	struct file_info *next;
	char cfe_partition[64];
	char file_name[64];
};

#define BUF_SIZE 0x10000   /* read/write buffer size */
char buf[BUF_SIZE];
char ver_buf[BUF_SIZE];
struct file_info * fi = NULL;

/* return NULL if not found. *eof = 1 if EOF, otherwise 0*/
static struct file_info * get_next_entry (FILE * fp, int * eof) {
	int i = 0;
	int len;
	int skip = 0;
	char * ptrs, *ptrd;
	struct file_info * entry;
	int found;

	found = 0;
	*eof = 0;
	if ((entry = (struct file_info *)malloc(sizeof(struct file_info))) ==NULL) {
		BDBG_ERR(("out of memory\n"));
		return NULL;
	}
	memset(entry, 0, sizeof(struct file_info));

	do {
		memset(buf, 0, sizeof(buf));
		i = 0;	
		skip = 0;
		while ((buf[i] = getc(fp)) != EOF) {
			if (buf[i] == '\n' || buf[i] == '\a') break;		
			if (buf[i] == '#') {
				buf[i] = 0;
				skip = 1; 
			}
			if (skip) continue;
			if (buf[i] == ' ') continue; 
			i++;
		}
		if (buf[i] == EOF) { 
			*eof = 1;
		}
		buf[i] = 0;
		len = i;
		if ( i == 0) continue;
		/* BDBG_ERR(("before : %s(%d)\n", buf, strlen(buf))); */

		i = 0;
		ptrs = &buf[i];
		ptrd = entry->cfe_partition;
		while (i < len) {
			if ( ptrs[i] == ',' || ptrs[i] == 0) {
				*ptrd = 0;
				i++;
				break;
			}
			*ptrd++ = ptrs[i++];
		}
		
		
		ptrs = &ptrs[i];
		i = 0;
		ptrd = entry->file_name;
		while (i < len) {
			if ( ptrs[i] == ',' || ptrs[i] == 0) {
				i++;
				*ptrd = 0;
				break;
			}
			*ptrd++ = ptrs[i++];
		}
	
		/*
		 * check error
		 */
		if (strlen(entry->cfe_partition) != 0 &&
		    strlen(entry->file_name) != 0) {
			found = 1;
		}
	} while (*eof == 0 && found == 0);

	if (!found) {
		free(entry);
		return NULL;
	}
	return entry;
}



void cdl_image_brcm_print_header(char * mono_file, int sec_header_len) {
	int i, num, len;
	struct cdl_image_header * hdr;
	int fdr;

	len = sizeof(struct cdl_image_header);
	if ((fdr = open(mono_file, O_RDONLY)) < 0) {
		BDBG_ERR(("%s: cannot open %s\n",  __FUNCTION__, mono_file));
		return;
	}
	lseek(fdr, sec_header_len, SEEK_SET);
	if ( (hdr = (struct cdl_image_header *)malloc(len)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		return;
	}

	num = read(fdr, hdr, len);
	if (num != len) {
		BDBG_ERR(("file read error, read %d, expect %d\n", num, len));
		return;
	}

	BDBG_ERR(("********** Downloaded Image Header **********\n"));
	BDBG_ERR(("Magic: 0x%08lx\n", hdr->magic));
	BDBG_ERR(("Version: %d\n", hdr->ver));
	BDBG_ERR(("Number of files: %d\n", hdr->num));
	BDBG_ERR(("Total length: 0x%08x\n", hdr->tot_len));

	if (hdr->ver == 1) {
		num = (hdr->num <= CDL_MAX_FILES) ? hdr->num : CDL_MAX_FILES;
		BDBG_ERR(("Type   FileName   DeviceName  Start       Size\n"));
		for ( i = 0; i < num; i++) {
			BDBG_ERR(("%2d     %-8s   %-20s  0x%08lx      %ld\n", hdr->v.v1[i].type,
			       hdr->v.v1[i].filename, hdr->v.v1[i].devname, hdr->v.v1[i].start, 
			       hdr->v.v1[i].size));
			
		}
	}

	BDBG_ERR(("*********************************************\n"));
	free(hdr);
	close(fdr);
	return;
}

int cdl_image_brcm_generate(char * mono_file, char * param_file ) {
	int ret = -1;
	FILE * fp = NULL;
	struct file_info *entry = NULL, *next = NULL, *prev = NULL;
	int eof;
	int i, fd, n, len;
	char tmp[64];
	unsigned long start, size;
	struct cdl_image_header  * hdr = NULL;
	int fdw = -1;
	int num_files = 0;

	memset(buf, 0, sizeof(buf));
	num_files = 0;
	if ((fp = fopen(param_file, "r")) == NULL) {
		BDBG_ERR(("%s: cannot open %s\n", __FUNCTION__, param_file));
		ret = -1;
		goto out;
	}

	/* parse the param file */
	do {
		next = get_next_entry(fp, &eof);
		if (next == NULL) break;
		if (fi == NULL) {
			fi = next;
		} else {
			entry->next = next;
			next->next = NULL;
		}
		entry = next;
	} while (eof == 0);

	/* generate monolithic image */
	entry = fi;
	prev = NULL;
	while (entry) {
		struct stat st;
		if (stat(entry->file_name, &st) < 0) {
			/* file not exist */
			BDBG_ERR(("file %s not exist, remove from list\n", entry->file_name));
			if (prev) {
				prev->next = entry->next;
			} else {
				fi = entry->next;
			}
			next = entry->next;
			free(entry);
			entry = next;
		} else {
			BDBG_ERR(("%s, %s\n", entry->cfe_partition, entry->file_name)); 
			prev = entry;
			entry = entry->next;
			num_files++;
		}
	}

	len = sizeof(struct cdl_image_header);
	if ((fdw = open(mono_file, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
	        BDBG_ERR(("%s: cannot open %s\n",  __FUNCTION__, mono_file));
		ret = -1;
		goto out;
	}
	if ( (hdr = (struct cdl_image_header *)malloc(len)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		goto out;
	}

	/* generate image header */
	memset(hdr, 0, len);
	hdr->magic = CDL_IMAGE_HEADER_MAGIC;
	hdr->ver = 1;
	hdr->num = num_files;
	start = len; 
	hdr->tot_len = start;
	size = 0;
	len = start;
	BDBG_ERR(("start 0x%lx\n", start));
	entry = fi;
	for (i = 0; i < hdr->num; i++) {
		hdr->v.v1[i].type = 0 ; /* not used */
		strncpy(hdr->v.v1[i].filename, entry->file_name, 64);
		strncpy(hdr->v.v1[i].devname, entry->cfe_partition, 64);
		hdr->v.v1[i].start = start;
	
		sprintf(tmp, "./%s", entry->file_name);
		if ((fd = open(tmp, O_RDONLY)) < 0) {
			BDBG_ERR(("%s: cannot open %s",  __FUNCTION__, tmp));
			ret = -1;
			goto out;
		}
		size = lseek(fd, 0, SEEK_END);
		close(fd);

		hdr->v.v1[i].size  = size;
		start += size;

		BDBG_ERR(("start %lx\n", start));
		hdr->tot_len = start;
		entry = entry->next;
	}
	if ( (ret = write(fdw, hdr, len)) < len) {
		BDBG_ERR(("Write error, write %d, expect %d bytes\n", ret, len));
	}

	entry = fi;
	for (i = 0; i < hdr->num; i++) {
		sprintf(tmp, "./%s", entry->file_name);
		if ((fd = open(tmp, O_RDONLY)) < 0) {
			BDBG_ERR(("%s: cannot open %s\n",  __FUNCTION__, tmp));
			ret = -1;
			goto out;
		}
		lseek(fdw, hdr->v.v1[i].start, SEEK_SET);
		do {
			n = read(fd, buf, BUF_SIZE);
			if (n < 0) {
				BDBG_ERR(("file read error\n"));
				break;
			}
			if (n == 0) {
				BDBG_ERR(("EOF\n"));
				break;
			}
			if ( (ret = write(fdw, buf, n)) < n) {
				BDBG_ERR(("Write error, write %d, expect %d bytes\n", ret, n));
			}
			
		} while ( n > 0);
		close(fd);
		entry = entry->next;
	}


	ret = 0;
 out:
	if (fdw >= 0) close(fdw);
	if (hdr) free(hdr);
	if (fi) {
		entry = fi->next;
		while (entry) {
			next = entry->next;
			free(entry);
			entry = next;
		}
		free(fi);
	}
	if (fp) fclose(fp);
	return ret;
}
int cdl_image_brcm_verify(char * mono_file, char * param_file ) {
	int ret = -1;
	FILE * fp = NULL;
	struct file_info *entry = NULL, *next = NULL, *prev = NULL;
	int eof;
	int i, fd, n, len;
	char tmp[64];
	unsigned long start, size;
	struct cdl_image_header  * hdr = NULL;
	int fdr = -1;
	int num_files = 0;
	int error_found = 0;

	memset(buf, 0, sizeof(buf));
	num_files = 0;
	if ((fp = fopen(param_file, "r")) == NULL) {
		BDBG_ERR(("%s: cannot open %s\n",  __FUNCTION__, param_file));
		ret = -1;
		error_found++;
		goto out;
	}

	/* parse the param file */
	do {
		next = get_next_entry(fp, &eof);
		if (next == NULL)  break;
		if (fi == NULL) {
			fi = next;
		} else {
			entry->next = next;
			next->next = NULL;
		}
		entry = next;
	} while (eof == 0);

	/* generate monolithic image */
	entry = fi;
	prev = NULL;
	while (entry) {
		struct stat st;
		if (stat(entry->file_name, &st) < 0) {
			/* file not exist */
			BDBG_ERR(("file %s not exist, remove from list\n", entry->file_name));
			if (prev) {
				prev->next = entry->next;
			} else {
				fi = entry->next;
			}
			next = entry->next;
			free(entry);
			entry = next;
		} else {
			BDBG_ERR(("%s, %s\n", entry->cfe_partition, entry->file_name)); 
			prev = entry;
			entry = entry->next;
			num_files++;
		}
	}

	len = sizeof(struct cdl_image_header);
	if ((fdr = open(mono_file, O_RDONLY)) < 0) {
		BDBG_ERR(("%s: cannot open %s\n",  __FUNCTION__, mono_file));
		ret = -1;
		error_found++;
		goto out;
	}
	if ( (hdr = (struct cdl_image_header *)malloc(len)) == NULL) {
		BDBG_ERR(("malloc failed\n"));
		ret = -1;
		error_found++;
		goto out;
	}

	/* generate image header */
	memset(hdr, 0, len);
	hdr->magic = CDL_IMAGE_HEADER_MAGIC;
	hdr->ver = 1;
	hdr->num = num_files;
	start = len; 
	hdr->tot_len = start;
	size = 0;
	len = start;
	BDBG_ERR(("start 0x%lx\n", start));
	entry = fi;
	for (i = 0; i < hdr->num; i++) {
		hdr->v.v1[i].type = 0 ; /* not used */
		strncpy(hdr->v.v1[i].filename, entry->file_name, 64);
		strncpy(hdr->v.v1[i].devname, entry->cfe_partition, 64);
		hdr->v.v1[i].start = start;
	
		sprintf(tmp, "./%s", entry->file_name);
		if ((fd = open(tmp, O_RDONLY)) < 0) {
			BDBG_ERR(("%s: cannot open %s",  __FUNCTION__, tmp));
			ret = -1;
			error_found++;
			goto out;
		}
		size = lseek(fd, 0, SEEK_END);
		close(fd);

		hdr->v.v1[i].size  = size;
		start += size;

		BDBG_ERR(("start %lx\n", start));
		hdr->tot_len = start;
		entry = entry->next;
	}
	if ( (ret = read(fdr, ver_buf, len)) < len) {
		BDBG_ERR(("read error, read %d, expect %d bytes\n", ret, len));
		error_found++;
		goto out;
	}
	if (memcmp(hdr, ver_buf, len)) {
		BDBG_ERR(("program header differs\n"));
		error_found++;
		goto out;
	}

	entry = fi;
	for (i = 0; i < hdr->num; i++) {
		sprintf(tmp, "./%s", entry->file_name);
		if ((fd = open(tmp, O_RDONLY)) < 0) {
			BDBG_ERR(("%s: cannot open %s\n",  __FUNCTION__, tmp));
			ret = -1;
			error_found++;
			goto out;
		}
		lseek(fdr, hdr->v.v1[i].start, SEEK_SET);
		do {
			n = read(fd, buf, BUF_SIZE);
			if (n < 0) {
				BDBG_ERR(("file read error\n"));
				error_found++;
				break;
			}
			if (n == 0) {
				BDBG_ERR(("EOF\n"));
				break;
			}
			if ( (ret = read(fdr, ver_buf, n)) < n) {
				BDBG_ERR(("Read error, read %d, expect %d bytes\n", ret, n));
				error_found++;
			}
			if (n < 0) {
				BDBG_ERR(("file read error\n"));
				error_found++;
				break;
			}
			if (n == 0) {
				BDBG_ERR(("EOF\n"));
				break;
			}
			if (memcmp(buf, ver_buf, n)) {
				BDBG_ERR(("file %s was not correct\n", tmp));
				error_found++;
				break;
			}
			
		} while ( n > 0);
		close(fd);
		entry = entry->next;
	}


	ret = 0;
 out:
	if (fdr >= 0) close(fdr);
	if (hdr) free(hdr);
	if (fi) {
		entry = fi->next;
		while (entry) {
			next = entry->next;
			free(entry);
			entry = next;
		}
		free(fi);
	}
	if (fp) fclose(fp);
	if (error_found) {
		BDBG_ERR(("Found %d errors, verification failed\n", error_found));
	} else {
		BDBG_ERR(("The monolithic image %s has been verified\n", mono_file));
	}
	return ret;
}

int cdl_image_brcm_check_integrity(char * mono_file) {
	/* customer specific check */
	return 1;
}
