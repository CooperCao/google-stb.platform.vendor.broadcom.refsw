/*
 * daemon for configuring NVRAM
 *
 * Copyright (C) 2017, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id$
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmtimer.h>

#define NVRAM_FILE          "/tmp/NVRAM.db"
#define NVRAM_FLASH         "flash"
#define SYSFS_DEV           "/dev"
#define MTD_NAME_PATT       "mtd"
#define MMCBLK_NAME_PATT    "mmcblk0"
#define MMCBLK_PATT_SIZE    "Partition size"
#define MMCBLK_PATT_NAME    "Partition name"
#define MTD_NAME            "name"
#define NVRAM_PARTITION     "wlan"

/*Macro Definitions*/
#define _MALLOC_(x)	calloc(x, sizeof(char))
#define _MFREE_(buf, size) free(buf)

/*the following definition is from wldef.h*/
#define WL_MID_SIZE_MAX  32
#define WL_SSID_SIZE_MAX 48
#define WL_WEP_KEY_SIZE_MAX WL_MID_SIZE_MAX
#define WL_WPA_PSK_SIZE_MAX  72  // max 64 hex or 63 char
#define WL_UUID_SIZE_MAX  40

#define WL_DEFAULT_VALUE_SIZE_MAX  160
#define WL_DEFAULT_NAME_SIZE_MAX  20
#define WL_WDS_SIZE_MAX  80

/*Differennt Nvram variable has different value length. To keep the Hash table static and sequence,
when one nvrma variable is inserted into hash table, the location will not dynamic change.
This structure is used to keep nvram name and value length*/
/* When new nvram variable is defined and max length is more than WL_DEFAULT_VALUE_SIZE_MAX,
the name and max length should be added into var_len_tab*/

struct   nvram_var_len_table {
    char *name;
    unsigned int  max_len;
};

/*nvram variable vs max length table*/
struct nvram_var_len_table var_len_tab[] =
{
#ifdef NVRAM_PREFIX_PARSE
        {"ssid",     WL_SSID_SIZE_MAX+1},
        {"uuid",    WL_UUID_SIZE_MAX+1},
#else
        {"wsc_ssid",     WL_SSID_SIZE_MAX+1},
        {"wsc_uuid",    WL_UUID_SIZE_MAX+1},
        {"wps_ssid",     WL_SSID_SIZE_MAX+1},
        {"wps_uuid",    WL_UUID_SIZE_MAX+1},
#endif
        {"radius_key",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"wpa_psk",    WL_WPA_PSK_SIZE_MAX+1},
        {"key1",          WL_MID_SIZE_MAX+1 },
        {"key2",          WL_MID_SIZE_MAX+1 },
        {"key3",          WL_MID_SIZE_MAX+1 },
        {"key4",          WL_MID_SIZE_MAX+1 },
        {"wds",            WL_WDS_SIZE_MAX },
#ifdef NVRAM_PREFIX_PARSE
        {"ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
#else
        {"lan_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"lan1_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"lan2_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"lan3_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"lan4_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"br0_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"br1_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"br2_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
        {"br3_ifnames",  WL_DEFAULT_VALUE_SIZE_MAX * 3},
#endif
/*This nvram variavle is used for debug*/
        {"wldbg",            1024 },
};

#define VAR_LEN_COUNT (sizeof(var_len_tab) /sizeof(struct nvram_var_len_table))

#ifdef NVRAM_PREFIX_PARSE
/* defines the nvram prefix structure */
struct   prefix_table {
    char *name;
};

/* Defines nvram variable needing to filter out following prefix.
   Such as wl1.0_ssid, will be filtered to be ssid.

   This is a simple Pattern/Syntax mapping case.
   More complicate mapping could be implemented if necessary
 */
struct prefix_table prefix_tab[] =
{
	{"wl"},
	{"br"},
	{"lan"},
	{"eth"},
	{"usb"},
	{"wsc"},
	{"wps"},
};

#define PREFIX_CNT (sizeof(prefix_tab) /sizeof(struct prefix_table))
#endif

#define  NVRAMD_DFLT_PORT           5152           /* default nvramd port number       */

#define NSECS_PER_MSEC	1000*1000

typedef struct nvram_info {
	int nvram_max_size;      /* Max size of envram space available              */
	struct nvram_header *nvh; /* points to the nvram header in envram	     */
} nvram_info_t;

typedef struct cons_dev {
	char *devname;
	int  cns_fd;
} cons_dev_t;

typedef struct nvramd_info {
	nvram_info_t nvinfo;		/* envram info at server side				*/
	int listen_fd;			/* server listens on this fd				*/
	int fdmax;
	fd_set rfds;
	cons_dev_t cons;
	char *buf;
	int flash_repo;
	char *ofile;
} nvramd_info_t;

nvramd_info_t nvramd_info = {
	nvinfo: {
		nvram_max_size: MAX_NVRAM_SPACE,
		nvh: NULL,
	},
	listen_fd: -1,
};

#define MMCBLK_SECTOR_SIZE	512

/* internal structure */
static struct nvram_header nv_header = { 0x48534C46, 0x14, 0x52565344, 0, 0xffffffff };
static struct nvram_tuple * nvram_hash[512] = { NULL };
static sem_t mutex;
static char commitfile[256];
static char backupfile[256];

/* Debug-related definition */
#define DBG_NVRAM_SET		0x00000001
#define DBG_NVRAM_GET		0x00000002
#define DBG_NVRAM_GETALL	0x00000004
#define DBG_NVRAM_UNSET		0x00000008
#define DBG_NVRAM_COMMIT	0x00000010
#define DBG_NVRAM_UPDATE	0x00000020
#define DBG_NVRAM_INFO		0x00000040
#define DBG_NVRAM_ERROR		0x00000080

#ifdef BCMDBG

static int debug_nvram_level = DBG_NVRAM_SET| DBG_NVRAM_GET| DBG_NVRAM_INFO | DBG_NVRAM_ERROR;

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

#define DBG_SET_BITFLAG(fmt, arg...) \
	do { if (debug_nvram_level & DBG_NVRAM_SET) \
		printf("%s@%d: "fmt , __FUNCTION__ , __LINE__, ##arg); } while(0)

#define DBG_GET_BITFLAG(fmt, arg...) \
	do { if (debug_nvram_level & DBG_NVRAM_GET) \
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
#define DBG_SET_BITFLAG(fmt, arg...)
#define DBG_GET_BITFLAG(fmt, arg...)
#define DBG_COMMIT(fmt, arg...)
#define DBG_UPDATE(fmt, arg...)
#define DBG_INFO(fmt, arg...)
#define DBG_ERROR(fmt, arg...)
#endif

static int nvramd_set(char *name, char *value);

static void usage(void)
{
	printf(" \nnvramd for accesing the nvram data\n"
		"Options:				       \n"
		"	-f				-run this server in forground\n"
		"	-v				-console output\n"
		"	-i ifile			-take initial text file\n"
		"	-o ofile			-the target nvram repository\n"
		"	-c commitfile			-the committed text file\n"
		"	-b backupfile			-the backup text file before committing\n"
		"	-m				-nvram repository use flash memory\n"
		"	-h				-usage\n"
		"\n"
		"usage: nvramd [-f] [-v] [-h] [-i ifile] [-o ofile] [-b backupfile]\n");
	exit(0);
}

/* nvram file memory mapping */
static void * _nvram_file_mmap(char *fname, int size)
{
	int fd;
	void *va = NULL;

	if((fd = open(fname, O_CREAT|O_SYNC|O_RDWR, 0644)) < 0) {
		DBG_ERROR(" nvram: file %s open error\n", fname);
		return 0;
	}
	if (ftruncate(fd, size) == -1)
		perror("ftruncate() error");

	va = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED, fd, 0);
	if(va == ((caddr_t) - 1)) {
		DBG_ERROR(" nvram: mmap errorr\n");
		return 0;
	}
	DBG_INFO("va addr=0x%p %d bytes\n", va, ((struct nvram_header*)va)->len);
	close(fd);

	return va;
}

/* Release nvrma file memory mapping */
static int _nvram_file_update(void *va, int size)
{
	int fd;

	if((fd = open(nvramd_info.ofile, O_SYNC|O_RDWR, 0644)) < 0) {
		DBG_ERROR("file %s open error\n", nvramd_info.ofile);
		return -1;
	}
	/*flush*/
	DBG_INFO("close file with %d bytes\n",((struct nvram_header*)va)->len);
	if (ftruncate(fd,((struct nvram_header*)va)->len) == -1)
		perror("ftruncate() error");
	msync((caddr_t)va, ((struct nvram_header*)va)->len, MS_SYNC);
	munmap((caddr_t)va, size);
	close(fd);
	if (nvramd_info.ofile) {
		_MFREE_(nvramd_info.ofile, strlen(nvramd_info.ofile));
		nvramd_info.ofile = NULL;
	}
	return 0;
}

/* Check nvram variable and return itsmax  value length */
static unsigned int check_var(char *name)
{
	int idx =0;
	char short_name[64];
#ifdef NVRAM_PREFIX_PARSE
	int cnt = 0;
#endif

	//DBG_INFO("Check_var name=[%s]\n", name );
	memset(short_name, 0, sizeof(short_name));

#ifdef NVRAM_PREFIX_PARSE
	/* Remove the Prefix defined in prefix_tab[]. such as, from wl0_ssid to ssid */
	//DBG_INFO ( "prefix_tab cnt=%d", PREFIX_CNT );

	strcpy(short_name, name );
	for (cnt=0; cnt<PREFIX_CNT; cnt++) {
		if (!strncmp(name, prefix_tab[cnt].name, strlen(prefix_tab[cnt].name))) {
			/* Skip the chars between prefix_tab[cnt] and "_" */
			for (idx=strlen(prefix_tab[cnt].name); name[idx] !='\0' && name[idx]!='_'; idx++)
				;
			if (name[idx] == '_')
			        strcpy(short_name, name+idx+1);
		}
	}
	//DBG_INFO("name=[%s] short_name=[%s]\n", name, short_name );
#else
	if ( !strncmp(name, "wl_", 3) ) {
		strcpy(short_name, name+3);
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl0_", 4) ) {
		strcpy( short_name, name+4 );
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl1_", 4) ) {
		strcpy( short_name, name+4 );
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl0.1_", 6) ) {
		strcpy(short_name, name+6);
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl0.2_", 6) ) {
		strcpy(short_name, name+6);
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl0.3_", 6) ) {
		strcpy(short_name, name+6);
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl1.1_", 6) ) {
		strcpy(short_name, name+6);
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl1.2_", 6) ) {
		strcpy(short_name, name+6);
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else if ( !strncmp(name, "wl1.3_", 6) ) {
		strcpy(short_name, name+6);
		DBG_INFO ( "name=[%s] short_name=[%s]\n", name, short_name );
	}
	else {
		strcpy(short_name, name );
	}
#endif

	for ( idx=0; idx < VAR_LEN_COUNT && var_len_tab[idx].name[0] !='\0'; idx++ ) {
		if ( !strcmp( var_len_tab[idx].name, short_name) ) {
			//DBG_INFO("[%s] Max Len [%d]\n", name, var_len_tab[idx].max_len );
			return var_len_tab[idx].max_len;
		}
	}
	//DBG_INFO("[%s] Default Max Len [%d]\n", name, WL_DEFAULT_VALUE_SIZE_MAX );
	return WL_DEFAULT_VALUE_SIZE_MAX;
}

/* hash Function */
static INLINE uint hash(const char *s)
{
	uint hash = 0;

	while (*s) {
		hash = 31 * hash + *s++;
	}
	return hash;
}

/* Tuple allocation */
static struct nvram_tuple * _nvram_alloc(struct nvram_tuple *t, const char *name, const char *value)
{
	int len;

	len = check_var( (char *)name );

	if (!(t = _MALLOC_(sizeof(struct nvram_tuple) + strlen(name) + 1 + len + 1))) {
		DBG_ERROR ( "malloc failed\n");
		return NULL;
	}
	memset( &t[1], 0, strlen(name)+1+len+1 );
	/* Copy name and value to tuple */
	t->name = (char *) &t[1];
	strcpy(t->name, name);
	t->value = t->name + strlen(name) + 1;

	/* Here: Check value size not larger than sizeof(value) */
	if ( value && strlen(value) > len ) {
		DBG_INFO("%s is too large than allocated size[%d]\n", value, len );
		strncpy(t->value, value, len);
	}
	else
		strcpy(t->value, value?:"");

	return t;
}

/* Tuple free */
static void _nvram_free(struct nvram_tuple *t)
{
	if (t) {
		_MFREE_(t, sizeof(struct nvram_tuple) + strlen(t->name) + 1 + check_var( (char *)name ) + 1);
	}
}

/* returns the CRC8 of the nvram */
uint8 nvram_calc_crc(struct nvram_header *nvh)
{
	struct nvram_header tmp;
	uint8 crc;

	/* Little-endian CRC8 over ver (bit [15:8]) of crc_ver_init */
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
				nvramd_set("sromrev", p);
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
							nvramd_set("sromrev", "13");
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
							nvramd_set("sromrev", "13");
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

static int mmcblk_open(char *name)
{
	int fd = -1;
	FILE *proc_fp;
	FILE *pipe_fp;
	char buf[256];
	int i;
	char *delim1 = " ";
	char *delim2 = "'";
	char *p;
	int token;
	int sector = 0;
	int ret;

	if ((proc_fp = fopen("/proc/partitions", "r"))) {
		while (fgets(buf, sizeof(buf), proc_fp)) {
			p = strstr(buf, MMCBLK_NAME_PATT);
			if (p == NULL)
				continue;
			ret = sscanf(p, MMCBLK_NAME_PATT"p""%d", &i);
			if (ret == 1) {
				sprintf(buf, "sgdisk -i=%d %s/%s", i, SYSFS_DEV, MMCBLK_NAME_PATT);
				pipe_fp = popen(buf, "r");
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
									nvramd_info.nvinfo.nvram_max_size = MMCBLK_SECTOR_SIZE * sector;
									printf("found %s partition on %s/%sp%d\n", name, SYSFS_DEV, MMCBLK_NAME_PATT, i);
									sprintf(buf, "%s/"MMCBLK_NAME_PATT"p""%d", SYSFS_DEV, i);
									fd = open(buf, O_RDWR);
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
				printf("found %s partition on %s\n", name, dev);
				fd = open(dev, O_RDWR);
				break;
			}
		}
		fclose(fp);
	}
	return fd;
}

static int _nvram_read(void *buf)
{
	uint32 *src, *dst;
	uint i;
	int ret=0;

	if (!nv_header.magic)
		return -19; /* -ENODEV */

	src = (uint32 *) &nv_header;
	dst = (uint32 *) buf;

	if (((struct nvram_header *)dst)->magic == NVRAM_MAGIC &&
		((struct nvram_header *)dst)->len != sizeof(struct nvram_header)) {
		/* nvram exists */
		DBG_INFO("nvram exist, len=%d\n", ((struct nvram_header *)dst)->len);
		nv_header.len = ((struct nvram_header *)dst)->len;
		if (nvram_calc_crc(buf) != (uint8)((struct nvram_header *)dst)->crc_ver_init) {
			DBG_ERROR("nvram crc error\n");
			ret = -1;
		}
	} else {
		/* nvram is empty */
		DBG_INFO("nvram empty\n");
		nv_header.len = sizeof(struct nvram_header);
		for (i = 0; i < sizeof(struct nvram_header); i += 4)
			*dst++ = *src++;
		if (add_default_setting() < 0) {
			DBG_ERROR("cannot add default setting\n");
		}
		ret = 0;
	}

	return ret;
}

/* Get the value of an NVRAM variable from harsh table */
char *
nvramd_get(const char *name)
{
	uint i;
	struct nvram_tuple *t;
	char *value= NULL;

	if (!name)
		return NULL;

	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);
	/* Find the associated tuple in the hash table */
	for (t = nvram_hash[i]; t && strcmp(t->name, name); t = t->next) {
		;
	}

	value = t ? t->value : NULL;

	return value;
}

/* Get all NVRAM variables from hash table */
int nvramd_getall(char *buf, int count)
{
	uint i;
	struct nvram_tuple *t;
	int len = 0;

	bzero(buf, count);
	/* Write name=value\0 ... \0\0 */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if ((count - len) > (strlen(t->name) + 1 + strlen(t->value) + 1))
				len += sprintf(buf + len, "%s=%s", t->name, t->value) + 1;
			else
				break;
		}
	}

	return len;
}

static int nvramd_commit()
{
	char *ptr, *end;
	int i, sret;
	struct nvram_tuple *t;
	struct nvram_header *header = nvramd_info.nvinfo.nvh;
	FILE *ptFile = NULL;
	char tempfile[256];
	int nvram_fd = -1;
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
	int ret = -1;
	int writesize = 0;
	int mtd_dev = 0;

	if (commitfile[0]) {
		snprintf(tempfile, sizeof(tempfile) - 1, "%s.tmp", commitfile);
		ptFile = fopen(tempfile, "w");
	}

	/* Regenerate header */
	header->magic = NVRAM_MAGIC;

	/* Clear data area */
	ptr = (char *) header + sizeof(struct nvram_header);
	bzero(ptr, nvramd_info.nvinfo.nvram_max_size - sizeof(struct nvram_header));

	/* Leave space for a double NUL at the end */
	end = (char *) header + nvramd_info.nvinfo.nvram_max_size - 2;

	sret = sem_trywait(&mutex);
	if (sret != 0) {
		DBG_ERROR("commit sem_trywait: failed!\n");
		if (ptFile)
			fclose(ptFile);
		return -1;
	}
	/* Write out all tuples */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = t->next) {
			if ((ptr + strlen(t->name) + 1 + strlen(t->value) + 1) > end)
				break;
			ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
			if (ptFile)
				fprintf(ptFile, "%s=%s\n", t->name, t->value);
			DBG_COMMIT("nvramd_commit writing %s=%s\n", t->name, t->value);
		}
	}

	/* End with a double NUL */
	ptr += 2;

	/* Set new length */
	header->len = ROUNDUP(ptr - (char *) header, 4);
	sem_post(&mutex);

	header->crc_ver_init = (NVRAM_VERSION << 8);
	header->crc_ver_init |= nvram_calc_crc(header);

	if (nvramd_info.flash_repo) {
		if ((nvram_fd = mtd_open(NVRAM_PARTITION)) < 0) {
			if ((nvram_fd = mmcblk_open(NVRAM_PARTITION)) < 0) {
				printf("%s nvram partition not found\n", NVRAM_PARTITION);
				goto err;
			}
			writesize = MMCBLK_SECTOR_SIZE;
		} else {
			if (ioctl(nvram_fd, MEMGETINFO, &mtd) < 0) {
				printf("MEMGETINFO ioctl request failed\n");
				goto err;
			} else {
				writesize = mtd.writesize;
				ebsize_aligned = mtd.erasesize;
				mtd_dev = 1;
			}
		}
		imglen = header->len;
		filebuf = (unsigned char *)header;
		if ((pagebuf = _MALLOC_(writesize)) == NULL) {
			DBG_ERROR("Failed malloc: %d/%s\n", errno, strerror(errno));
			goto err;
		}

		while (imglen > 0 && offset < nvramd_info.nvinfo.nvram_max_size) {
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
	}

	if (ptFile) {
		fclose(ptFile);

		/* Backup the file */
		if (backupfile[0]) {
			if (rename(commitfile, backupfile) < 0)
				DBG_COMMIT("Rename %s to %s failed %d/%s!\n", commitfile, backupfile, errno, strerror(errno));
		}

		if (rename(tempfile, commitfile) < 0)
			DBG_COMMIT("Rename %s to %s failed %d/%s!\n", tempfile, commitfile, errno, strerror(errno));
	}

	ret = 0;

	DBG_COMMIT("header->len=%d\n", header->len);
err:
	if (pagebuf) {
		_MFREE_(pagebuf, writesize);
		pagebuf = NULL;
	}

	if (nvram_fd > 0) {
		close(nvram_fd);
	}
	return ret;
}

/* nvram variable set */
static int
nvramd_set(char *name, char *value)
{
	uint i, sret;
	int len;
	struct nvram_tuple *t, *u, *curr/*, *prev = NULL*/;

	sret = sem_trywait(&mutex);
	if (sret != 0) {
		DBG_ERROR("set %s to %s: sem_trywait: failed!\n", name, value);
		return -1;
	}
	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);
	for (curr = nvram_hash[i]; curr && strcmp(curr->name, name);
		/*prev = curr,*/ curr = curr->next) {
	}
	if ( curr != NULL ) { /* found the tuple */
		DBG_SET("<== _nvramd_set Found [%s]\n", name );
		len = check_var( (char *)name );  /* return the max value size */

		/* Here: Check value size not larger than sizeof(value) */
		if ( value && strlen(value) > len ) {
			DBG_INFO("%s is too large than allocated size[%d]\n", value, len );
			strncpy(curr->value, value, len);
		}
		else
			strcpy(curr->value, value?:"" );
	}
	else { /* this is a new tuple */
		if (!(u = _nvram_alloc(t, name, value))) {
			DBG_ERROR("no memory\n");
			sem_post(&mutex);
			return -12; /* -ENOMEM */
		}
		//DBG_SET("after _nvram_alloc u=%x\n", (uint32)u);

		/* add new tuple to the hash table */
		u->next = nvram_hash[i];
		nvram_hash[i] = u;
	}
	sem_post(&mutex);

	return 0;
}

/* nvram variable unset */
static int
nvramd_unset(char *name)
{
	uint i, sret;
	struct nvram_tuple *curr, *prev = NULL;
	sret = sem_trywait(&mutex);
	if (sret != 0) {
		DBG_ERROR("unset sem_trywait: failed!\n");
		return -1;
	}
	/* Hash the name */
	i = hash(name) % ARRAYSIZE(nvram_hash);
	for (curr = nvram_hash[i]; curr && strcmp(curr->name, name);
		prev = curr, curr = curr->next) {
	}

	if ( curr != NULL ) { /* found the tuple */
		DBG_SET("<== _nvramd_unset Found [%s]\n", name );

		/* free the unset entity */
		if (prev == NULL) {
			nvram_hash[i] = curr->next;
		} else {
			prev->next = curr->next;
		}
		_nvram_free(curr);
	}
	sem_post(&mutex);

	return 0;
}

/* (Re)initialize the hash table */
static int nvram_rehash(struct nvram_header *header)
{
	char *name, *value, *end, *eq;

	/* Parse and set "name=value\0 ... \0\0" */
	name = (char *) &header[1];
	end = (char *) header + nvramd_info.nvinfo.nvram_max_size - 2;
	end[0] = end[1] = '\0';
	for (; *name; name = value + strlen(value) + 1) {
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value = eq + 1;
		nvramd_set(name, value);
		*eq = '=';
	}
	return 0;
}

static int
nvramd_load_file(char *ifile, char *ofile)
{
	nvram_info_t *nv_info = &nvramd_info.nvinfo;
	struct nvram_header **envh = &nv_info->nvh;
	int ret = -1;
	int nvram_fd = -1;
	char *nvram_buf = NULL;
	long start_addr = 0;
	long ofs, end_addr = 0;
	long blockstart = 1;
	int bs, badblock = 0;
	loff_t seek;
	int firstblock = 1;
	int eb;
	int offs;
	int eb_size;
	int rd;
	int buf_ofs = 0;
	struct nvram_header *header;
	mtd_info_t mtd;
	uint32 imglen = 0;
	int mtd_dev = 0;

	if (!(nvramd_info.ofile = (char *)_MALLOC_(strlen(ofile)))) {
		DBG_ERROR("Failed malloc: %d/%s\n", errno, strerror(errno));
		return -12; /* -ENOMEM */
	} else {
		strcpy(nvramd_info.ofile, ofile);
	}

	if (nvramd_info.flash_repo) {
		if ((nvram_fd = mtd_open(NVRAM_PARTITION)) < 0) {
			if ((nvram_fd = mmcblk_open(NVRAM_PARTITION)) < 0) {
				printf("%s nvram partition not found\n", NVRAM_PARTITION);
				goto err;
			}
		} else {
			if (ioctl(nvram_fd, MEMGETINFO, &mtd) < 0) {
				printf("MEMGETINFO ioctl request failed\n");
				goto err;
			} else {
				nvramd_info.nvinfo.nvram_max_size = mtd.size;
				mtd_dev = 1;
			}
		}
	}

	if (*ifile) {
		FILE *ptFile;

		if ((ptFile = fopen(ifile, "r")) == NULL)
			printf(" nvram: input text file %s open error %d/%s\n", ifile, errno, strerror(errno));
		else {
			char *pcEnd, *pcValue, readbuf[128];

			while (fgets(readbuf, sizeof(readbuf), ptFile) != NULL) {
				if (readbuf[0] == '#')
					continue;
				pcValue = strchr(readbuf, '=');
				if (pcValue == NULL)
					continue;
				pcEnd = strpbrk(readbuf, "\n\r");
				if (pcEnd)
					*pcEnd = '\0'; /* Remove '\n' or '\r' */
				*pcValue++ = '\0';
				nvramd_set(readbuf, pcValue);
			}
			fclose(ptFile);
		}
	}

	if (!(*envh = (struct nvram_header *) _nvram_file_mmap(ofile,
		nvramd_info.nvinfo.nvram_max_size))) {
		DBG_ERROR("find_envram: out of memory\n");
		return -12; /* -ENOMEM */
	}

	if (nvramd_info.flash_repo) {
		nvram_buf = (char *)*envh;
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
		/* clear data area if wlan partition contains garbage */
		if (((struct nvram_header *)*envh)->magic != NVRAM_MAGIC)
			bzero(nvram_buf, nvramd_info.nvinfo.nvram_max_size);
	}

	if ((ret = _nvram_read(*envh)) == 0 &&
			((struct nvram_header *)*envh)->magic == NVRAM_MAGIC) {
		/* empty */
		nvram_rehash(*envh);
	}
err:
	if (nvram_fd > 0) {
		close(nvram_fd);
	}
	return ret;
}

/* free all tuples */
static void nvram_free(void)
{
	uint i, sret;
	struct nvram_tuple *t, *next;

	DBG_INFO("free hashtable\n");
	sret = sem_trywait(&mutex);
	if (sret != 0) {
		DBG_ERROR("free sem_trywait: failed!\n");
		return;
	}
	/* free hash table */
	for (i = 0; i < ARRAYSIZE(nvram_hash); i++) {
		for (t = nvram_hash[i]; t; t = next) {
			next = t->next;
			_nvram_free(t);
		}
		nvram_hash[i] = NULL;
	}
	sem_post(&mutex);
}

/*
 * set nvramd running state in shared memory object
 */
static int nvramd_started(int state)
{
	int shm_fd;
	void *attach;
	int value = 0;

	mkdir("/dev/shm", 0777);
	if ((shm_fd = shm_open("nvram", O_CREAT|O_RDWR, 0644)) < 0) {
		DBG_ERROR("nvramd: shm file open error %d/%s\n", errno, strerror(errno));
		return -1;
	}
	if (ftruncate(shm_fd, sizeof(int)) == -1)
		perror("ftruncate() error");
	attach = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED, shm_fd, (off_t)0);
	if (state != -1) {
		*((int *)attach) = state;
		value = state;
	} else {
		if (*((int *)attach) == 1) {
			printf("Another instance of this program already running\n");
			value = -1;
		}
	}
	if (munmap(attach, sizeof(int)))
		DBG_ERROR("munmap()\n");
	close(shm_fd);
	return value;
}

static void nvramd_sigterm_handler(int signum)
{
	int i;

	DBG_INFO("free resource\n");
	for(i = nvramd_info.listen_fd; i <= nvramd_info.fdmax; i++) {
		if(FD_ISSET(i, &nvramd_info.rfds)) {
			DBG_INFO("close fd %d\n", i);
			close(i);
		}
	}
	FD_ZERO(&nvramd_info.rfds);
	nvramd_info.listen_fd = -1;
	if (nvramd_info.buf) {
		_MFREE_(nvramd_info.buf, nvramd_info.nvinfo.nvram_max_size);
		nvramd_info.buf = NULL;
	}
	nvram_free();
	if (nvramd_info.nvinfo.nvh) {
		if (!_nvram_file_update(nvramd_info.nvinfo.nvh, nvramd_info.nvinfo.nvram_max_size))
			nvramd_info.nvinfo.nvh = NULL;
	}
	nvramd_started(0);
	exit (0);
}

/* Run in background */
static void
daemonize(unsigned int console_log)
{
	int i;

	i = fork();
	if (i<0) {  /* fork error */
		DBG_ERROR("Fork failed: %d/%s\n", errno, strerror(errno));
		exit(1);
	}
	if (i > 0)
		exit(0); /* parent exits */
	/* child (daemon) continues */
	setsid(); /* obtain a new process group */
	if (!console_log) {
		for (i = getdtablesize(); i >= 0; --i)
			close(i);  /* close all descriptors */
		nvramd_info.cons.devname = "/dev/null";
		nvramd_info.cons.cns_fd = open(nvramd_info.cons.devname, O_RDWR);
		if (dup(nvramd_info.cons.cns_fd) == -1)
			perror("dup() error");
		if (dup(nvramd_info.cons.cns_fd) == -1)
			perror("dup() error");
	}

	umask(027); /* set newly created file permissions */
	signal(SIGCHLD, SIG_IGN); /* ignore child */
	signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
}

/*
 * Initialize the TCP socket for clients to connect
 */
static int
nvramd_sock_init(unsigned int port)
{
	struct sockaddr_in serveraddr;
	int yes = 1;

	nvramd_info.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (nvramd_info.listen_fd < 0) {
		DBG_ERROR("Socket open failed: %d/%s\n", errno, strerror(errno));
		return -1;
	}
	if(setsockopt(nvramd_info.listen_fd, SOL_SOCKET, SO_REUSEADDR,
		&yes, sizeof(int)) == -1) {
		DBG_ERROR("setsockopt() error!\n");
		return -1;
	}

	memset(&serveraddr, 0, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);
	if (bind(nvramd_info.listen_fd, (struct sockaddr *)&serveraddr,
		sizeof(serveraddr)) < 0) {
		DBG_ERROR("Socket bind failed: %d/%s\n", errno, strerror(errno));
		return -1;
	}

	if (listen(nvramd_info.listen_fd, 5) < 0) {
		DBG_ERROR("Socket listen failed: %d/%s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}

/*
 * Receivs and processes the commands from client
 * o Wait for connection from client
 * o Process the command and respond back to client
 */
static int
nvramd_proc_client_req()
{
	struct sockaddr_in clientaddr;
	int addrlen;
	fd_set read_fds;
	int listener;
	int newfd;
	int i;
	char *buf;
	int resp_size = 0, rcount = 0;
	char *c, *data, *resp_msg;
	int noresp = 0;
	int left;
	int offset;

	FD_ZERO(&nvramd_info.rfds);
	FD_ZERO(&read_fds);
	listener = nvramd_info.listen_fd;
	FD_SET(listener, &nvramd_info.rfds);
	nvramd_info.fdmax = listener;

	if (!(buf = (char *)_MALLOC_(nvramd_info.nvinfo.nvram_max_size))) {
		DBG_ERROR("Failed malloc: %d/%s\n", errno, strerror(errno));
		return -1;
	}

	nvramd_info.buf = buf;
	nvramd_started(1);

	offset = 0;
	while (1) {
		read_fds = nvramd_info.rfds;
		if(select(nvramd_info.fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			if (errno == EINTR) {
				DBG_INFO("ignoring select errno=%d\n", errno);
				continue;
			}
			DBG_ERROR("select() error errno=%d %s!\n", errno, strerror(errno));
			break;
		}

		/* run through the existing connections looking for data to be read */
		for(i = 0; i <= nvramd_info.fdmax; i++) {
			if(FD_ISSET(i, &read_fds)) {
				/* we got one... */
				if(i == listener) {
					 /* handle new connections */
					addrlen = sizeof(clientaddr);
					if((newfd = accept(listener, (struct sockaddr *)&clientaddr, (socklen_t *)&addrlen)) == -1) {
						DBG_ERROR("accept() error!\n");
					} else {
						FD_SET(newfd, &nvramd_info.rfds); /* add to nvramd_info.rfds set */
						if(newfd > nvramd_info.fdmax) {
							/* keep track of the maximum */
							nvramd_info.fdmax = newfd;
						}
						DBG_INFO("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
					}
					break;
				} else {
					/* handle data from a client */
					if((rcount = recv(i, &buf[offset], nvramd_info.nvinfo.nvram_max_size-offset, 0)) <= 0) {
						/* got error or connection closed by client */
						if(rcount == 0)
							/* connection closed */
							DBG_INFO("socket %d hung up\n", i);
						else
							DBG_ERROR("recv() error!\n");
						/* close it... */
						close(i);
						/* remove from nvramd_info.rfds set */
						FD_CLR(i, &nvramd_info.rfds);

					} else {
						if (offset) {
							rcount += offset;
							offset = 0;
						}
						while (rcount > offset) {
							noresp = 0;
							left = strlen(&buf[offset]);
							if (offset+left+1>rcount) {
								noresp = 1;
								memcpy(&buf[0], &buf[offset], left);
								offset = left;
								buf[offset] = '\0';
								break;
							}
							/* Check if we have command and data in the expected order */
							if ((c = strchr(&buf[offset], ':'))) {
								*c++ = '\0';
								data = c;

								if (!strcmp(&buf[offset], "unset")) {
									nvramd_unset(c);
									noresp = 1;
								} else if (!strcmp(buf, "set")) {
									c = strsep(&data, "=");
									if (nvramd_set(c, data)) {
										resp_size = sprintf(buf, "set failed");
									}
								} else if (!strcmp(&buf[offset], "get")) {
									if ((resp_msg = nvramd_get(data))) {
										resp_size = sprintf(buf, "%s", resp_msg);
									} else /* not found */
										resp_size = -1;
								} else if (!strcmp(&buf[offset], "show")) {
									if ((resp_size = nvramd_getall(buf, nvramd_info.nvinfo.nvram_max_size))) {
									} else /* Error */
										resp_size = -1;
								} else if (!strcmp(&buf[offset], "commit")) {
									if (nvramd_commit() < 0) {
										resp_size = sprintf(buf, "Error on commit nvram");
									}
								} else {
									resp_size = sprintf(buf, "Invalid nvram command:%s", buf);
								}
								offset += (left+1);
							} else {
								noresp = 1;
								memcpy(&buf[0], &buf[offset], left);
								offset = left;
								break;
							}
							if (!noresp) {
								if (resp_size < 0) {
									resp_size = sizeof(unsigned long) - 1;
									memset(buf, 0, resp_size);
								}
								buf[resp_size++] = '\0';
								resp_msg = buf;
								if (send(i, resp_msg, resp_size, 0) < 0)
									DBG_ERROR("Failed sending message: %d/%s\n", errno, strerror(errno));
							}
						}
						if (!noresp)
							offset = 0;
						else
							if (offset >= rcount)
								offset = 0;
					}
				}
			}
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	unsigned int background = 1;
	unsigned int console_log = 0;
	unsigned int port = NVRAMD_DFLT_PORT;
	nvram_info_t *nv_info = &nvramd_info.nvinfo;
	sigset_t block_mask;
	struct sigaction sig_action;
	char ifile[256], ofile[256];

	ifile[0] = ofile[0] = commitfile[0] = backupfile[0] = '\0';
	while (*++argv) {
		if (!strcmp(*argv, "-v")) {
			console_log = 1;
		} else if (!strcmp(*argv, "-f")) {
			background = 0;
		} else if (!strcmp(*argv, "-h")) {
			usage();
		} else if (!strcmp(*argv, "-i")) {
			strncpy(ifile, *++argv, sizeof(ifile) - 1);
		} else if (!strcmp(*argv, "-o")) {
			strncpy(ofile, *++argv, sizeof(ofile) - 1);
		} else if (!strcmp(*argv, "-c")) {
			strncpy(commitfile, *++argv, sizeof(commitfile) - 1);
		} else if (!strcmp(*argv, "-b")) {
			strncpy(backupfile, *++argv, sizeof(backupfile) - 1);
		} else if (!strcmp(*argv, "-m")) {
			nvramd_info.flash_repo = 1;
		}
	}

	if (nvramd_started(-1) < 0) {
		exit(0);
	}
	/* create, initialize semaphore */
	if (sem_init(&mutex, 1, 1) < 0)
	{
		DBG_ERROR("semaphore initilization\n");
		exit(0);
	}
	/* Establish the signal handler. */
	sigfillset(&block_mask);
	sig_action.sa_handler = nvramd_sigterm_handler;
	sig_action.sa_mask = block_mask;
	sig_action.sa_flags = 0;
	sigaction(SIGTERM, &sig_action, NULL);
	if (ofile[0] == '\0')
		strncpy(ofile, NVRAM_FILE, sizeof(ofile) - 1);
	/* Extract nvram data info from file */
	if (nvramd_load_file(ifile, ofile))
		return -1;

	printf("nvramd: listening on port: %d\n", port);
	printf("nvram size: 0x%x\n"
		"nvram magic: 0x%08x\n"
		"len: 0x%08x\n",
		nv_info->nvram_max_size,
		nv_info->nvh->magic,
		nv_info->nvh->len);

	/* Run in background by default */
	if (background)
		daemonize(console_log);

	/* Initialize the connection for clients to connect */
	if (nvramd_sock_init(port) < 0) return -1;

	/* Process commands from client and respond back */
	nvramd_proc_client_req();

	return 0;
}
