/*
 * NVRAM variable manipulation (Linux user mode half)
 *
 * Copyright (C) 2017, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dirent.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <mtd/mtd-user.h>

#define SYSFS_DEV           "/dev"
#define MTD_NAME_PATT       "mtd"
#define MMCBLK_NAME_PATT    "mmcblk0"
#define MMCBLK_PATT_SIZE    "Partition size"
#define MMCBLK_PATT_NAME    "Partition name"
#define MTD_NAME            "name"
#define NVRAM_PARTITION     "wlan"
#define NVRAM_FILE          "/etc/nvram.wlan"	/* STBAP configuration */
#define NVRAM_TMP_FILE      "/tmp/NVRAM.db"		/* working copy of configuration */
#define NVRAM_LOCK_FILE     "/tmp/nvram.lock"	/* access control */
#define NVRAM_MFG_TAG       "/tmp/nvram.mfg"	/* tag to use flash as storage, it create and del during MFG test */
#define NVRAM_IS_MFG        "/var/nvram.mfg"	/* tag indicate NVRAM_FILE contains flash storage setting */
#define MAX_LOCK_WAIT       10
#define MMCBLK_SECTOR_SIZE  512
#define CODE_BUFF           16
#define HEX_BASE            16

#define VALID_BIT(bit) ((bit) >=0 && (bit) <= 31)

/*Macro Definitions*/
#define _MALLOC_(x)			calloc(x, sizeof(char))
#define _MFREE_(buf)		free(buf)
#define NVRAM_HASH_TABLE_SIZE	32

/* Debug-related definition */
#define DBG_NVRAM_SET		0x00000001
#define DBG_NVRAM_GET		0x00000002
#define DBG_NVRAM_GETALL 	0x00000004
#define DBG_NVRAM_UNSET 	0x00000008
#define DBG_NVRAM_COMMIT 	0x00000010
#define DBG_NVRAM_UPDATE 	0x00000020
#define DBG_NVRAM_INFO	 	0x00000040
#define DBG_NVRAM_ERROR 	0x00000080

#ifdef BCMDBG
static int debug_nvram_level = DBG_NVRAM_ERROR;

#define DBG_SET(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_SET) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#define DBG_GET(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_GET) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__,##arg); } while(0)

#define DBG_GETALL(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_GETALL) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__,##arg); } while(0)

#define DBG_UNSET(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_UNSET) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__,##arg); } while(0)

#define DBG_COMMIT(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_COMMIT) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__,##arg); } while(0)

#define DBG_UPDATE(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_UPDATE) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#define DBG_INFO(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_INFO) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#define DBG_ERROR(fmt, arg...) \
        do { if (debug_nvram_level & DBG_NVRAM_ERROR) \
                printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#else
#define DBG_SET(fmt, arg...)
#define DBG_GET(fmt, arg...)
#define DBG_GETALL(fmt, arg...)
#define DBG_UNSET(fmt, arg...)
#define DBG_COMMIT(fmt, arg...)
#define DBG_UPDATE(fmt, arg...)
#define DBG_INFO(fmt, arg...)
#define DBG_ERROR(fmt, arg...)	printf(fmt , ##arg);
#endif

/* Globals */
/* internal structure */
struct nvram_header nv_header = { 0x48534C46, 0x14, 0x52565344, 0, 0xffffffff };
static struct nvram_tuple *nvram_hash[NVRAM_HASH_TABLE_SIZE] = { NULL };
static int nvram_inited = 0;
static pid_t mypid = 0;
static char *nvram_buf = NULL;
static char *nvram_file = NVRAM_TMP_FILE;
static int filesize = 64*1024;

static int _lock();
static int _unlock();
static int _nvram_lock();
static int _nvram_unlock();
static int _nvram_set(const char *name, const char *value);
static void swap(char *ptr1, char *ptr2);
static int add_default_setting();
static void * _nvram_mmap_alloc(int size);
static void * _nvram_mmap_free(void *va, int size);
static int mmcblk_open(char *name);
static int mtd_open(char *name);
static INLINE uint hash(const char *s);
static struct nvram_tuple * _nvram_realloc(struct nvram_tuple *t, const char *name, const char *value);
static int _nvram_set(const char *name, const char *value);
static int _nvram_getall(char *buf, int count);
static int _nvram_commit(struct nvram_header *header);
static int _nvram_update_file(struct nvram_header *header);
static int nvram_rehash(struct nvram_header *header);
static int _nvram_update(struct nvram_header *header);
static char * _nvram_get(const char *name);
static void _nvram_free(struct nvram_tuple *t);
static void nvram_free(void);
static void _nvram_exit(void);

static int _lock()
{
	int fd;
	fd = open(NVRAM_LOCK_FILE,O_WRONLY|O_CREAT|O_EXCL, 0644);
	if (fd < 0 && errno == EEXIST) {
		DBG_INFO("%s is already locked\n", NVRAM_LOCK_FILE);
		return 0;
	} else if (fd < 0){
		DBG_ERROR("unexpected error checking lock");
		return 0;
	}
	DBG_INFO(" nvram : %s created\n", NVRAM_LOCK_FILE);
	close(fd);
	return 1;
}

static int _unlock()
{
	if (unlink(NVRAM_LOCK_FILE) < 0) {
		DBG_ERROR("cannot remove lock file");
		return 0;
	}
	DBG_INFO(" nvram : %s deleted\n", NVRAM_LOCK_FILE);
	return 1;
}

static int _nvram_lock()
{
	int i=0;

	while (i++ < MAX_LOCK_WAIT) {
		if(_lock())
			return 1;
		else
			usleep(500000);
	}
	return 0;
}

static int _nvram_unlock()
{
	int i=0;

	while (i++ < MAX_LOCK_WAIT) {
		if(_unlock())
			return 1;
		else
			usleep(500000);
	}
	return 0;
}

void nvram_unlock()
{
	_nvram_unlock();
}

/* returns the CRC8 of the nvram */
uint8 BCMINITFN(nvram_calc_crc)(struct nvram_header *nvh)
{
	struct nvram_header tmp;
	uint8 crc;

	/* Little-endian CRC8 over the last 11 bytes of the header */
	tmp.crc_ver_init = htol32((nvh->crc_ver_init & NVRAM_CRC_VER_MASK));

	crc = hndcrc8((uint8 *) &tmp + NVRAM_CRC_START_POSITION,
			1, CRC8_INIT_VALUE);

	/* Continue CRC8 over data bytes */
	crc = hndcrc8((uint8 *) &nvh[1], nvh->len - sizeof(struct nvram_header), crc);
	return crc;
}

static void swap(char *ptr1, char *ptr2)
{
    char temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

static int add_default_setting()
{
	FILE *fp;
	char *p;
	char *delim = " ";
	char buf[32];
	int ret = -1;
	int val = 0;
	int i, hn, ln, len, token;

	fp = popen("wl sromrev", "r");
	if (fp) {
		memset(buf, 0, sizeof(buf));
		if (fgets(buf, sizeof(buf), fp) != NULL) {
			if ((p = strtok(buf, delim)) != NULL) {
				_nvram_set("sromrev", p);
				ret = 0;
			}
		}
		pclose(fp);
	}
	if (ret < 0) {
		fp = popen("od -x /proc/device-tree/bolt/product-id", "r");
		if (fp) {
			memset(buf, 0, sizeof(buf));
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				for (token = 0, p = strtok(buf, delim); p != NULL; p = strtok(NULL, delim), token++) {
					if (token == 1) {
						swap(p, p+2);
						swap(p+1, p+3);
						len = 4;
						for (i = 0; i < len; i+=2) {
							hn = p[i] > '9' ? (p[i]|32) - 'a' + 10 : p[i] - '0';
							ln = p[i+1] > '9' ? (p[i+1]|32) - 'a' + 10 : p[i+1] - '0';
							val = (val << 8) + ((hn << 4) | ln);
						}

						switch(val)
						{
						case 0x7271:
							_nvram_set("sromrev", "13");
							ret = 0;
							break;
						}
					}
					/* if chip id not found, check if chip id is 5 hex digits format */
					if (ret < 0 && token == 2) {
						i = 2;
						hn = p[i] > '9' ? (p[i]|32) - 'a' + 10 : p[i] - '0';
						ln = p[i+1] > '9' ? (p[i+1]|32) - 'a' + 10 : p[i+1] - '0';
						val = (val << 8) + ((hn << 4) | ln);
						switch(val)
						{
						case 0x72712:
							_nvram_set("sromrev", "13");
							ret = 0;
							break;
						}
					}
				}
			}
		}
		pclose(fp);
	}
	return ret;
}

/* nvram file memory mapping */
static void * _nvram_mmap_alloc(int size)
{
	int fd;
	void *va=NULL;

	if ((fd = open(nvram_file, O_CREAT|O_SYNC|O_RDWR, 0644)) < 0) {
		DBG_ERROR("nvram: file open error\n");
		return 0;
	}

	ftruncate(fd, size);

	va = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED, fd, 0);
	if(va == ((caddr_t) - 1)) {
		DBG_ERROR("nvram: mmap error\n");
		close(fd);
		return 0;
	}
	DBG_INFO("va obtained=0x%p\n", va);
	DBG_INFO("va %d bytes\n", ((struct nvram_header*)va)->len);
	close(fd);
	return va;
}

/* release nvram file memory mapping */
static void * _nvram_mmap_free(void *va, int size)
{
	int fd;

	if((fd = open(nvram_file, O_SYNC|O_RDWR, 0644)) < 0) {
		DBG_ERROR("nvram: _nvram_mmap_free file open error");
		return 0;
	}
	/* flush */
	DBG_INFO("close file with %d bytes\n",((struct nvram_header*)va)->len);
	ftruncate(fd,((struct nvram_header*)va)->len);
	msync((caddr_t)va, ((struct nvram_header*)va)->len, MS_SYNC);
	munmap((caddr_t)va, size);
	close(fd);
	return NULL;
}

static int mmcblk_open(char *name)
{
	int fd = -1;
	FILE *proc_fp;
	FILE *pipe_fp;
	char dev[256];
	char buf[256];
	int i;
	char *delim1 = " ";
	char *delim2 = "'";
	char *p;
	int token;
	int sector = 0;
	int ret;

	if ((proc_fp = fopen("/proc/partitions", "r"))) {
		while (fgets(dev, sizeof(dev), proc_fp)) {
			p = strstr(dev, MMCBLK_NAME_PATT);
			if (p == NULL)
				continue;
			ret = sscanf(p, MMCBLK_NAME_PATT"p""%d", &i);
			if (ret == 1) {
				sprintf(dev, "sgdisk -i=%d %s/%s", i, SYSFS_DEV, MMCBLK_NAME_PATT);
				pipe_fp = popen(dev, "r");
				if (pipe_fp) {
					while (fgets(buf, sizeof(buf), pipe_fp) != NULL) {
						if (memcmp(buf, MMCBLK_PATT_SIZE, strlen(MMCBLK_PATT_SIZE)) == 0) {
							for (token = 0, p = strtok(buf, delim1); p != NULL; p = strtok(NULL, delim1), token++) {
								if (token == 1) {
									p = strtok(NULL, delim1);
									sector = atoi(p);
									break;
								}
							}
						}
						if (memcmp(buf, MMCBLK_PATT_NAME, strlen(MMCBLK_PATT_NAME)) == 0) {
							p = strtok(buf, delim2);
							if (p) {
								p = strtok(NULL, delim2);
								if (strstr(p, name)) {
									filesize = MMCBLK_SECTOR_SIZE * sector;
									DBG_INFO("found %s partition on %s/%sp%d\n", name, SYSFS_DEV, MMCBLK_NAME_PATT, i);
									sprintf(dev, "%s/"MMCBLK_NAME_PATT"p""%d", SYSFS_DEV, i);
									fd = open(dev, O_RDWR);
									break;
								}
							}
						}
					}
					pclose(pipe_fp);
					if (fd > 0)
						break;
				}
			}
		}
		fclose(proc_fp);
	}
	return fd;
}

static int mtd_open(char *name)
{
	int fd = -1;
	FILE *fp;
	char dev[256];
	int i;

	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, name)) {
				sprintf(dev, "%s/%s%d", SYSFS_DEV, MTD_NAME_PATT, i);
				fd = open(dev, O_RDWR);
				break;
			}
		}
		fclose(fp);
	}
	return fd;
}

/* hash Function */
static INLINE uint hash(const char *s)
{
	uint hashval = 0;

	while (*s) {
		hashval = 31 * hashval + *s++;
	}
	return hashval;
}

/* Tuple allocation */
static struct nvram_tuple * _nvram_realloc(struct nvram_tuple *t, const char *name, const char *value)
{
	int len = 0;
	struct nvram_tuple *u = NULL;

	if (value)
		len = strlen(value);

	if (t) {
		/* reuse the same buf if new value len not over old value */
		len = strlen(t->value);
		if (strlen(value) <= len)
			u = t;
	}

	if (!u) {
		if (!(t = _MALLOC_(sizeof(struct nvram_tuple) + strlen(name) + 1 + len + 1))) {
			DBG_ERROR("no memory\n");
			return NULL;
		}

		/* Copy name */
		t->name = (char *) &t[1];
		strcpy(t->name, name);
		t->value = t->name + strlen(name) + 1;
	}
	strcpy(t->value, value);
	return t;
}

/* Set the value to harsh table */
static int _nvram_set(const char *name, const char *value)
{
	uint i;
	struct nvram_tuple *t, *u, **prev;

	DBG_SET("<== _nvram_set [%s]=[%s]\n", name, value?:"NULL");
	/* Hash the name */
	i = hash(name) % NVRAM_HASH_TABLE_SIZE;

	/* Find the associated tuple in the hash table */
	for (prev = &nvram_hash[i], t = *prev; t && strcmp(t->name, name);
		prev = &t->next, t = *prev);

	if (value && !strncmp(value, "*DEL*", 5 )) {
		if (t) {
			*prev = t->next;
			_MFREE_(t);
		}
		return 0;
	}
	/* (Re)allocate tuple */
	if (!(u = _nvram_realloc(t, name, value))) {
		return -12; /* -ENOMEM */
	}

	/* Value reallocated */
	if (t && t == u)
		return 0;

	/* free old nvram tuple */
	if (t) {
		*prev = t->next;
		_MFREE_(t);
	}

	DBG_SET("after _nvram_realloc u=0x%p\n", u);
	/* Add new tuple to the hash table */
	u->next = nvram_hash[i];
	nvram_hash[i] = u;
	return 0;
}

/* Get all NVRAM variables from harsh table. */
static int _nvram_getall(char *buf, int count)
{
	uint i;
	struct nvram_tuple *t;
	int len = 0;

	memset(buf, 0, count);

	/* Write name=value\0 ... \0\0 */
	for (i = 0; i < NVRAM_HASH_TABLE_SIZE; i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if ((count - len) > (strlen(t->name) + 1 + strlen(t->value) + 1))
				len += sprintf(buf + len, "%s=%s", t->name, t->value) + 1;
			else
				break;
		}
	}
	return 0;
}

/* Regenerate NVRAM */
static int _nvram_commit(struct nvram_header *header)
{
	int nvram_fd;
	mtd_info_t mtd;
	struct erase_info_user64 ei64;
	long long blockstart = -1;
	long long offs;
	long long imglen = 0;
	int badblock = 0;
	loff_t seek;
	int ebsize_aligned;
	int eb;
	unsigned char *filebuf = NULL;
	unsigned char *pagebuf = NULL;
	unsigned char *writebuf = NULL;
	long long offset = 0;
	int len;
	int ret = 0;
	int writesize = 0;
	int mtd_dev = 0;

	if (access(NVRAM_MFG_TAG, F_OK) == 0) {
		if ((nvram_fd = mtd_open(NVRAM_PARTITION)) < 0) {
			if ((nvram_fd = mmcblk_open(NVRAM_PARTITION)) < 0) {
				DBG_ERROR("%s nvram partition not found\n", NVRAM_PARTITION);
				goto err;
			}
			writesize = MMCBLK_SECTOR_SIZE;
		} else {
			if (ioctl(nvram_fd, MEMGETINFO, &mtd) < 0) {
				DBG_ERROR("MEMGETINFO ioctl request failed\n");
				goto err;
			} else {
				writesize = mtd.writesize;
				ebsize_aligned = mtd.erasesize;
				mtd_dev = 1;
			}
		}
	} else {
		if((nvram_fd = open(NVRAM_FILE, O_CREAT|O_SYNC|O_RDWR, 0644)) < 0) {
			DBG_ERROR("can't access %s\n", NVRAM_FILE);
			goto err;
		}
		writesize = header->len;
	}

	imglen = header->len;
	filebuf = (unsigned char *)header;
	if ((pagebuf = malloc(writesize)) == NULL) {
		goto err;
	}

	header->crc_ver_init = (NVRAM_VERSION << 8);
	header->crc_ver_init |= nvram_calc_crc(header);

	while (imglen > 0 && offset < filesize) {
		if (mtd_dev) {
			while (blockstart != (offset & (~ebsize_aligned + 1))) {
				blockstart = offset & (~ebsize_aligned + 1);
				offs = blockstart;
				if (mtd.type == MTD_NANDFLASH || mtd.type == MTD_MLCNANDFLASH) {
					badblock = 0;
					do {
						eb = offs / ebsize_aligned;
						seek = (loff_t)eb * mtd.erasesize;
						ret = ioctl(nvram_fd, MEMGETBADBLOCK, &seek);
						if (ret < 0) {
							DBG_ERROR("MTD get bad block failed\n");
							goto err;
						} else if (ret == 1) {
							badblock = 1;
							DBG_INFO("Bad block at %llx, "
									"from %llx will be skipped\n",
									offs, blockstart);
						}

						if (badblock) {
							offset = blockstart + ebsize_aligned;
						}

						offs +=  ebsize_aligned;
					} while (offs < blockstart + ebsize_aligned);
				}

				if (badblock) {
					goto err;
				} else {
					ioctl(nvram_fd, MEMUNLOCK, ei64);
					ei64.start = (__u64)blockstart;
					ei64.length = (__u64)mtd.erasesize;
					ret = ioctl(nvram_fd, MEMERASE64, &ei64);
					if (ret < 0) {
						DBG_INFO("MTD Erase failure\n");
						goto err;
					}
				}
			}
		}

		if (imglen >= writesize) {
			imglen -= writesize;
			writebuf = filebuf;
		} else {
			memcpy(pagebuf, filebuf, imglen);
			memset(pagebuf + imglen, 0, writesize - imglen);
			writebuf = pagebuf;
			imglen = 0;
		}
		/* Write out data */
		if (mtd_dev) {
			eb = offset / mtd.erasesize;
			offs = offset % mtd.erasesize;
			/* Seek to the beginning of the eraseblock */
			seek = (off_t)eb * mtd.erasesize + offs;
		} else {
			seek = offset;
		}
		len = writesize;
		if (lseek(nvram_fd, seek, SEEK_SET) != seek) {
			DBG_ERROR("cannot seek to offset 0x%jx\n", seek);
			goto err;
		}
		ret = write(nvram_fd, writebuf, len);
		if (ret != len) {
			DBG_ERROR("cannot write %d bytes to mtd\n", len);
			goto err;
		}
		offset += writesize;
		filebuf += writesize;
	}
	ret = 0;
	goto exit;

err:
	ret = -1;
exit:
	if (pagebuf) {
		free(pagebuf);
		pagebuf = NULL;
	}

	if (nvram_fd > 0) {
		close(nvram_fd);
		nvram_fd = -1;
	}
	return ret;
}

/* flush hash table to file */
static int _nvram_update_file(struct nvram_header *header)
{
	char *ptr, *end;
	int i;
	struct nvram_tuple *t;

	/* Regenerate header */
	header->magic = NVRAM_MAGIC;

	/* Clear data area */
	ptr = (char *) header + sizeof(struct nvram_header);
	memset(ptr, 0, filesize - sizeof(struct nvram_header));

	/* Leave space for a double NUL at the end */
	end = (char *) header + filesize - 2;

	/* Write out all tuples */
	for (i = 0; i < NVRAM_HASH_TABLE_SIZE; i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if ((ptr + strlen(t->name) + 1 + strlen(t->value) + 1) > end)
				break;
			ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
			DBG_COMMIT("_nvram_update_file writing %s=%s\n", t->name, t->value);
		}
	}

	/* End with a double NUL */
	ptr += 2;

	/* Set new length */
	header->len = ROUNDUP(ptr - (char *) header, 4);

	/* Set new revision, steal config_refresh for indication  */
	header->crc_ver_init = (NVRAM_VERSION << 8);
	header->crc_ver_init |= nvram_calc_crc(header);
	header->config_refresh= mypid;
	DBG_COMMIT("new revision=%d\n", mypid );
	DBG_COMMIT("header->len=%d\n", header->len);
	return 0;
}

/* (Re)initialize the hash table. */
static int nvram_rehash(struct nvram_header *header)
{
	char *name, *value, *end, *eq;

	/* Parse and set "name=value\0 ... \0\0" */
	name = (char *) &header[1];
	end = (char *) header + header->len - 2;
	end[0] = end[1] = '\0';
	for (; *name; name = value + strlen(value) + 1) {
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value = eq + 1;
		_nvram_set(name, value);
		*eq = '=';
	}
	return 0;
}

/* (Re) merge the hash table. */
/* For conflict items, use the one in hash table, not from nvram. */
static int _nvram_update(struct nvram_header *header)
{
	char *name, *value, *end, *eq;

	DBG_UPDATE("_nvram_update: revision=%d mypid=%d\n", header->config_refresh, mypid);

	if(  header->config_refresh == mypid ){	/* nothing changed */
		DBG_UPDATE("nothing to update revision=%d\n", mypid);
		return 0;
	}

	/* read nvram and Parse/Set "name=value\0 ... \0\0" */
	name = (char *) &header[1];
	end = (char *) header + header->len - 2;
	end[0] = end[1] = '\0';
	for (; *name; name = value + strlen(value) + 1) {
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value= eq + 1;

		/* Put tuple to hash table */
		_nvram_set(name, value);
		*eq = '=';
	}
	return 0;
}

/* Get the value of an NVRAM variable from harsh table. */
static char * _nvram_get(const char *name)
{
	uint i;
	struct nvram_tuple *t;
	char *value= NULL;

	if (!name)
		return NULL;

	/* Hash the name */
	i = hash(name) % NVRAM_HASH_TABLE_SIZE;

	/* Find the associated tuple in the hash table */
	for (t = nvram_hash[i]; t && strcmp(t->name, name); t = t->next) {
		;
	}

	value = t ? t->value : NULL;
	return value;
}

int nvram_size(void *unused)
{
	int nvram_fd;
	mtd_info_t mtd;
	int ret = -1;

	if (access(NVRAM_MFG_TAG, F_OK) == 0) {
		if ((nvram_fd = mtd_open(NVRAM_PARTITION)) < 0) {
			if ((nvram_fd = mmcblk_open(NVRAM_PARTITION)) < 0) {
				DBG_ERROR("%s nvram partition not found\n", NVRAM_PARTITION);
			}
		} else {
			if (ioctl(nvram_fd, MEMGETINFO, &mtd) < 0) {
				DBG_ERROR("MEMGETINFO ioctl request failed\n");
			} else {
				filesize = mtd.size;
			}
		}
		if (nvram_fd > 0) {
			close(nvram_fd);
			ret = filesize;
		}
	} else {
		ret = filesize;
	}
	return ret;
}

int _nvram_init(void *unused)
{
	int nvram_fd;
	long start_addr = 0;
	long ofs, end_addr = 0;
	long blockstart = 1;
	int i, bs, badblock = 0;
	loff_t seek;
	int firstblock = 1;
	int eb;
	int offs;
	int eb_size;
	int ret;
	int rd;
	int buf_ofs = 0;
	uint32 *src, *dst;
	struct nvram_header *header;
	int restore = 0;
	mtd_info_t mtd;
	int rm_file = 0;
	uint32 imglen = 0;
	int mtd_dev = 0;
	char command[64];

	if (nvram_inited)
		return 0;

	if (access(NVRAM_MFG_TAG, F_OK) == 0) {
		if ((nvram_fd = mtd_open(NVRAM_PARTITION)) < 0) {
			if ((nvram_fd = mmcblk_open(NVRAM_PARTITION)) < 0) {
				DBG_ERROR("%s nvram partition not found\n", NVRAM_PARTITION);
				goto err;
			}
		} else {
			if (ioctl(nvram_fd, MEMGETINFO, &mtd) < 0) {
				DBG_ERROR("MEMGETINFO ioctl request failed\n");
				goto err;
			} else {
				filesize = mtd.size;
				mtd_dev = 1;
			}
		}
		if (access(NVRAM_IS_MFG, F_OK) == -1) {
			/* tag file not found then nvram_file is created by ap test */
			sprintf(command, "touch %s", NVRAM_IS_MFG);
			system(command);
			unlink(nvram_file);
		}
	} else {
		if (access(NVRAM_IS_MFG, F_OK) == 0) {
			/* tag file found show last update by mfg test */
			unlink(NVRAM_IS_MFG);
			unlink(nvram_file);
		}
		if((nvram_fd = open(NVRAM_FILE, O_CREAT|O_SYNC|O_RDWR, 0644)) < 0) {
			DBG_ERROR("can't access %s\n", NVRAM_FILE);
			goto err;
		}
	}

	if (access(nvram_file, F_OK) == -1) {
		restore = 1;
	}

	if (!(nvram_buf = _nvram_mmap_alloc(filesize))) {
		DBG_ERROR("can't access %s\n", nvram_file);
		goto err;
	}

	if (restore) {
		if (mtd_dev) {
			start_addr = 0;
			end_addr = mtd.size;
			bs = mtd.writesize;
			eb_size = mtd.erasesize;
			for (ofs = start_addr; ofs < end_addr; ofs += bs) {
				eb = ofs / eb_size;
				offs = ofs % eb_size;
				if (mtd.type == MTD_NANDFLASH || mtd.type == MTD_MLCNANDFLASH) {
					if (blockstart != (ofs & (~eb_size + 1)) || firstblock) {
						blockstart = ofs & (~eb_size + 1);
						firstblock = 0;
						seek = (loff_t)eb * eb_size;
						if ((badblock = ioctl(nvram_fd, MEMGETBADBLOCK, &seek)) < 0) {
							DBG_ERROR("ioctl MEMGETBADBLOCK error\n");
							goto err;
						}
					}
				}
				if (badblock) {
					/* skip bad block, increase end_addr */
					end_addr += eb_size;
					ofs += eb_size - bs;
					if (end_addr > mtd.size)
						end_addr = mtd.size;
					DBG_INFO("skip bad block %d\n", eb);
					continue;
				} else {
					/* Seek to the beginning of the eraseblock */
					seek = (off_t)eb * eb_size + offs;
					if (lseek(nvram_fd, seek, SEEK_SET) != seek) {
						DBG_ERROR("cannot seek to offset 0x%jx\n", seek);
						goto err;
					}
					rd = 0;
					if (seek == 0) {
						ret = read(nvram_fd, nvram_buf+buf_ofs, bs);
						if (ret > 0) {
							header = (struct nvram_header *)nvram_buf;
							if (header->magic == NVRAM_MAGIC) {
								rd += ret;
								buf_ofs += ret;
							} else {
								break;
							}
						}
					}
					while (rd < bs) {
						ret = read(nvram_fd, nvram_buf+buf_ofs, bs);
						if (ret < 0) {
							DBG_ERROR("cannot read %d bytes (eraseblock %d, offset %d)\n",
									  bs, eb, offs);
							goto err;
						}
						rd += ret;
						buf_ofs += ret;
					}
				}
			}
		} else {
			seek = 0;
			bs = MMCBLK_SECTOR_SIZE;
			if (lseek(nvram_fd, seek, SEEK_SET) == seek) {
				ret = read(nvram_fd, nvram_buf+buf_ofs, bs);
				if (ret > 0) {
					header = (struct nvram_header *)nvram_buf;
					if (header->magic == NVRAM_MAGIC) {
						buf_ofs = ret;
						imglen = header->len;
						rd = ret;
						while (rd < imglen) {
							ret = read(nvram_fd, nvram_buf+buf_ofs, bs);
							if (ret < 0) {
								DBG_ERROR("cannot read %d bytes (offset %d)\n",
											bs, buf_ofs);
								goto err;
							}
							rd += ret;
							buf_ofs += ret;
						}
					}
				}
			}
		}
	}

	header = (struct nvram_header *)nvram_buf;
	if (header->magic != NVRAM_MAGIC) {
		/* nvram is empty */
		src = (uint32 *) &nv_header;
		dst = (uint32 *) header;
		for (i = 0; i < sizeof(struct nvram_header); i += 4)
			*dst++ = *src++;
		DBG_INFO("nvram is empty and create default header\n");
		*dst = 0;
		if (add_default_setting() < 0) {
			DBG_ERROR("cannot add default setting\n");
			rm_file = 1;
			goto err;
		}
		_nvram_update_file(header);
	} else {
		if (nvram_calc_crc(header) != (uint8)header->crc_ver_init) {
			DBG_ERROR("nvram crc error\n");
			goto err;
		}
	}

	nvram_rehash(header);
	nvram_inited = 1;
	mypid = getpid();
	ret = 0;
	goto exit;

err:
	ret = -1;
exit:
	if (nvram_buf) {
		_nvram_mmap_free(nvram_buf, filesize);
		nvram_buf = NULL;
		if (rm_file) {
			if (access(nvram_file, F_OK) != -1 &&
				unlink(nvram_file) < 0) {
				DBG_ERROR("*** Failed to delete file %s. Error: %s\n",
						nvram_file, strerror(errno));
			}
			rm_file = 0;
		}
	}
	if (nvram_fd > 0) {
		close(nvram_fd);
		nvram_fd = -1;
	}
	return ret;
}

/* Tuple free */
static void _nvram_free(struct nvram_tuple *t)
{
	if (t) {
		_MFREE_(t);
	}
}

static void nvram_free(void)
{
	uint i;
	struct nvram_tuple *t, *next;

	/* Free hash table */
	for (i = 0; i < NVRAM_HASH_TABLE_SIZE; i++) {
		for (t = nvram_hash[i]; t; t = next) {
			next = t->next;
			_nvram_free(t);
		}
		nvram_hash[i] = NULL;
	}
}

static void _nvram_exit(void)
{
	if (nvram_inited) {
		nvram_free();
		nvram_inited = 0;
	}
}

void nvram_uninit(void)
{
	if(!_nvram_lock())
		return;
	_nvram_exit();
	_nvram_unlock();
}

char *nvram_get(const char *name)
{
	char *ret = NULL;
	struct nvram_header *header;

	DBG_GET("==>nvram_get\n");
	if(!_nvram_lock())
		return ret;

	if (!nvram_inited) {
	    if (_nvram_init(NULL) < 0) {
			_nvram_unlock();
			return ret;
		}
	} else {
		if ((header = _nvram_mmap_alloc(filesize))) {
			_nvram_update(header);
			_nvram_mmap_free(header, filesize);
		}
	}

	ret = _nvram_get(name);

	_nvram_unlock();

	DBG_GET("%s=%s\n",name, ret);
	DBG_GET("<==nvram_get\n");

	return ret;
}

char *nvram_get_bitflag(const char *name, const int bit)
{
	char *ptr;
	unsigned long nvramvalue = 0;
	unsigned long bitflagvalue = 1;

	if (!VALID_BIT(bit))
		return NULL;
	ptr = nvram_get(name);
	if (ptr) {
		bitflagvalue = bitflagvalue << bit;
		nvramvalue = strtoul(ptr, NULL, HEX_BASE);
		if (nvramvalue) {
			nvramvalue = nvramvalue & bitflagvalue;
		}
	}
	return ptr ? (nvramvalue ? "1" : "0") : NULL;
}

int nvram_set_bitflag(const char *name, const int bit, const int value)
{
	char nvram_val[CODE_BUFF];
	char *ptr;
	unsigned long nvramvalue = 0;
	unsigned long bitflagvalue = 1;

	if (!VALID_BIT(bit))
		return 0;
	memset(nvram_val, 0, sizeof(nvram_val));

	ptr = nvram_get(name);
	if (ptr) {
		bitflagvalue = bitflagvalue << bit;
		nvramvalue = strtoul(ptr, NULL, HEX_BASE);
		if (value) {
			nvramvalue |= bitflagvalue;
		} else {
			nvramvalue &= (~bitflagvalue);
		}
	}
	snprintf(nvram_val, sizeof(nvram_val)-1, "%lx", nvramvalue);
	return nvram_set(name, nvram_val);
}

int nvram_getall(char *buf, int count)
{
	int ret;
	struct nvram_header *header;

	DBG_GETALL("==>nvram_getall\n");
	if(!_nvram_lock())
		goto fail_getall;

	if (!nvram_inited) {
		if ((ret = _nvram_init(NULL)) < 0) {
			_nvram_unlock();
			goto fail_getall;
		}
	} else {
		if (!(header = (struct nvram_header *) _nvram_mmap_alloc(filesize))) {
			DBG_ERROR("out of memory\n");
			_nvram_unlock();
			goto fail_getall;
		}
		_nvram_update(header);
		_nvram_mmap_free(header, filesize);
	}

	ret = _nvram_getall(buf, count);
	_nvram_unlock();
	DBG_GETALL("<==nvram_getall\n");
	return ret;

fail_getall:
	return -1;
}

int nvram_set(const char *name, const char *value)
{
	struct nvram_header *header;
	int ret = 0;
	int update = 0;

	if(!_nvram_lock()) {
		DBG_SET("lock failure");
		return -1;
	}

	if (!nvram_inited) {
		if (_nvram_init(NULL) < 0) {
			_nvram_unlock();
			return -1;
		}
	} else {
		update = 1;
	}

	if ((header = (struct nvram_header *) _nvram_mmap_alloc(filesize))) {
		if (update)
			_nvram_update(header);
		ret = _nvram_set(name, value?:"");
		_nvram_update_file(header);
		_nvram_mmap_free(header, filesize);
	} else
		ret = -1;

	_nvram_unlock();
	return ret;
}

int nvram_unset(const char *name)
{
	int ret;

	DBG_UNSET("==>nvram_unset\n");
	/* nvram_unset just set a delete tag (*DEL*). Not really deleted, to keep the hash table in sequence */
	ret = nvram_set( name, "*DEL*");
	DBG_UNSET("<==nvram_unset\n");
	return ret;
}

int nvram_commit(void)
{
	struct nvram_header *header;
	int ret = 0;

	if (!nvram_inited) {
		if ((ret = _nvram_init(NULL)) < 0)
			return ret;
	}
	if ((header = _nvram_mmap_alloc(filesize))) {
		ret = _nvram_commit(header);
		_nvram_mmap_free(header, filesize);
		header = NULL;
	}
	return ret;
}

int nvram_restore(void)
{
	int ret;

	DBG_INFO("Erasing nvram file ...\n");
	if (access(nvram_file, F_OK) != -1 &&
		unlink(nvram_file) < 0) {
		DBG_INFO("*** Failed to delete file %s. Error: %s\n",
				nvram_file, strerror(errno));
	}
	ret = nvram_commit();
	return ret;
}

int nvram_set_filepath(char *nvram_filename)
{
	int ret = 0;

	if (nvram_filename)
		nvram_file = nvram_filename;
	else
		ret = -1;
	return ret;
}
