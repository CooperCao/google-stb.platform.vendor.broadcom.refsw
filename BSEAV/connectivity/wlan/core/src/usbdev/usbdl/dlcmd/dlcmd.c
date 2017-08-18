/*
 * Broadcom dlcmd bootloader reg read/write utility
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
#include <string.h>
#include <usb.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <zlib.h>
#include <trxhdr.h>
#include "usbrdl.h"
#include "../../dongle/usb.h"
#include <bcmdevs.h>
#include "../usb_osl.h"

#include <hnd_debug.h>
#include <hnd_armtrap.h>

/* #define BCMQT */

#ifdef BCMQT
#undef RDL_CHUNK
#define RDL_CHUNK 250
#undef TIMEOUT
#define TIMEOUT     1000000	/* Timeout for usb commands */
#endif

#define MEMBLOCK	512

#define BCMDBG

/* Posible locations of dump info ptr ptr */
#define DUMP_INFO_PTR_PTR_END 0xffffffff
const uint32 dump_info_ptr_ptr[] = {0xf8, 0x878, DUMP_INFO_PTR_PTR_END};

enum bldr_cmd_e
{
	BLDR_VERSION,
	BLDR_BOOT,
	BLDR_READ,
	BLDR_WRITE,
	BLDR_WRITE_BLK,
	BLDR_EXEC,
	BLDR_DUMP,
	BLDR_INVALID
};
typedef enum bldr_cmd_e bldr_cmd;

static struct bcm_device_id bcm_device_ids[] = {
	{"brcm RDL (alpha)", BCM_DNGL_VID, 0xcafe },
	{"brcm RDL", BCM_DNGL_VID, 0xbd11},
	{"brcm RDL", BCM_DNGL_VID, BCM_DNGL_BL_PID_4328},
	{"brcm RDL", BCM_DNGL_VID, BCM_DNGL_BL_PID_4322},
	{"brcm RDL", BCM_DNGL_VID, 0xbd14},
	{"brcm RDL", BCM_DNGL_VID, 0xbd15},
	{"brcm RDL", BCM_DNGL_VID, 0xbd16},
	{"brcm RDL", BCM_DNGL_VID, BCM_DNGL_BL_PID_43236},
	{"brcm RDL", BCM_DNGL_VID, BCM_DNGL_BL_PID_43239},
	{"brcm RDL", BCM_DNGL_VID, BCM_DNGL_BL_PID_4324},
	{"brcm RDL", BCM_DNGL_VID, 0x4319},
	{"brcm RDL", 0x0720, BCM_DNGL_BL_PID_4328},
	{"brcm RDL", BCM_DNGL_VID, BCM_DNGL_BL_PID_4335},
	{"brcm RDL", BCM_DNGL_VID, BCM_DNGL_BL_PID_4345},
#ifdef BCMQT
	{"brcm RDL-QT", BCM_DNGL_VID, BCM_DNGL_BL_PID_4328},
	{"brcm RDL-QT", BCM_DNGL_VID, BCM_DNGL_BL_PID_4322},
	{"brcm RDL-QT", 0x0720, BCM_DNGL_BL_PID_4328},
	{"brcm RDL-QT", 0x0720, BCM_DNGL_BL_PID_4322},
#endif
	{NULL, 0xffff, 0xffff}
};

static char *progname;
static struct usbinfo *info = NULL;
static bool cpuless = FALSE;

/* Read a buffer (multiple of 4 bytes read if cpuless) */
int
readbuf(uint32 addr, int len, uchar *buf)
{
	int status;

	memset(buf, 0, len);

	if (cpuless) {
		status = usbdev_control_read(info, 0x00,
					     (addr & 0xffff),
					     (addr >> 16),
					     buf, len,
					     FALSE, TIMEOUT);
		if (status < 0) {
			fprintf(stderr, "Readbuf (cpuless) failed with parameters %x %x\n", addr, len);
			return -1;
		}

	} else {
		hwacc_t hwacc;
		int i;

		/* Copy all of the 4-byte chunks */
		for (i = 0; i < (len & ~4); i+=4, addr+=4) {
			status = usbdev_control_read(info, DL_RDHW32,
						     (addr & 0xffff),
						     (addr >> 16),
						     (char *)&hwacc,
						     sizeof(hwacc_t),
						     TRUE,
						     TIMEOUT);

			if (status < 0 || hwacc.cmd != DL_RDHW ||
			    hwacc.addr != addr || hwacc.len != len) {
				fprintf(stderr, "Readbuf failed with parameters %x %x\n", addr, len);
				return -1;
			}

			*((uint32 *) &buf[i]) = hwacc.data;
		}

		/* Finish up with the 1-byte chunks */
		for (i = (len & ~4); i < len; i++,addr++) {
			status = usbdev_control_read(info, DL_RDHW8,
						     (addr & 0xffff),
						     (addr >> 16),
						     (char *)&hwacc,
						     sizeof(hwacc_t),
						     TRUE,
						     TIMEOUT);

			if (status < 0 || hwacc.cmd != DL_RDHW ||
			    (hwacc.addr != addr) || (hwacc.len != len)) {
				fprintf(stderr, "Readbuf failed with parameters %x %x\n", addr, len);
				return -1;
			}

			buf[i] = (uint8) hwacc.data;
		}
	}

	return (0);
}


static void
usage(void)
{
	printf("Usage:\n"
	       "\t%s r <addr> <datalen>\n"
	       "\t%s w <addr> <datalen> <data>\n"
	       "\t%s e <addr>\n"
	       "\t%s d [filename]\n",
	       progname, progname, progname, progname);
	if (info) {
		usbdev_deinit(info);
	}

	exit(1);
}

/* pretty hex print a contiguous buffer */
void
myprhex(uchar *buf, uint nbytes, uint base)
{
	char line[128], *p;
	int len = sizeof(line);
	int nchar;
	uint i;

	p = line;
	for (i = 0; i < nbytes; i++) {
		if (i % 16 == 0) {
			nchar = snprintf(p, len, "  %08d: ", i + base);    /* line prefix */
			p += nchar;
			len -= nchar;
		}
		if (len > 0) {
			nchar = snprintf(p, len, "%02x ", buf[i]);
			p += nchar;
			len -= nchar;
		}

		if (i % 16 == 15) {
			printf("%s\n", line);       /* flush line */
			p = line;
			len = sizeof(line);
		}
	}

	/* flush last partial line */
	if (p != line)
		printf("%s\n", line);
}

int
main(int argc, char **argv)
{
	struct bcm_device_id *bcmdev;
	int status = 0;
	hwacc_t hwacc;
	hwacc_blk_t *blk_hwacc = NULL;
	bldr_cmd cmd = BLDR_INVALID;
	uint32 addr = 0, datalen = 0, data = 0;

	char *filename = NULL;
	uint8 *datap = NULL;

	progname = argv[0];

	/* example: 'dlcmd r 0x18000044 4' to read 4 bytes from addr 0x18000044 */
	/* example: 'dlcmd w 0x18000052 1 0x1f' to write 1 byte to addr 0x18000052 */
	/* example: 'dlcmd v  To get the chip revision */
	/* example: 'dlcmd b To reboot the dongle */
	/* example: 'dlcmd wb 0x60001044 8 0x12345678 0x90987654 write 8bytes at location 0x1044 */
	/* example: 'dlcmd e 0x9fc09ea0 to execute a function at addr 0x9fc09ea0 */

	if (argc < 2)
		usage();

	if (tolower(argv[1][0]) != 'r' && tolower(argv[1][0]) != 'w' &&
		tolower(argv[1][0]) != 'e' && tolower(argv[1][0]) != 'b' &&
		tolower(argv[1][0]) != 'v')
		usage();

	switch (tolower(argv[1][0]))
	{
		case 'w':
		if (argc < 5)
			usage();

			addr = strtoul(argv[2], NULL, 0);
			datalen = strtoul(argv[3], NULL, 0);

			/* If its a block write request */
			if ('b' == tolower(argv[1][1])) {
				uint* todata;
				int vals = 4;
				cmd = BLDR_WRITE_BLK;
				blk_hwacc = (hwacc_blk_t*)malloc(sizeof(hwacc_blk_t) + datalen - 1);
				blk_hwacc->cmd = DL_WRHW_BLK;
				blk_hwacc->len = datalen;
				blk_hwacc->addr = addr;

				todata = (uint*)&blk_hwacc->data[0];

				while (vals < argc) {
					*todata = strtoul(argv[vals], NULL, 0);
					printf("0x%08x", *todata);
					todata++;
					vals++;
				}
				printf("\n");
			} else {
		data = strtoul(argv[4], NULL, 0);
				if (datalen != 1 && datalen != 2 && datalen != 4) {
					printf("Invalid datalen: must be one of 1, 2 or 4\n");
					usage();
	}
				cmd = BLDR_WRITE;
			}
		break;

		case 'r':
			if (argc < 4)
			usage();

			cmd = BLDR_READ;
			addr = strtoul(argv[2], NULL, 0);
		datalen = strtoul(argv[3], NULL, 0);
		if (datalen != 1 && datalen != 2 && datalen != 4) {
			printf("Invalid datalen: must be one of 1, 2 or 4\n");
			usage();
		}
		break;

		case 'e':
			cmd = BLDR_EXEC;
	addr = strtoul(argv[2], NULL, 0);
		break;

		case 'b':
			cmd = BLDR_BOOT;
		break;

		case 'v':
			cmd = BLDR_VERSION;
		break;
		
 		case 'd':
			cmd = BLDR_DUMP;
			if (argc > 2) {
				filename = argv[2];
			}
			if (tolower(argv[1][1]) == 'c') {
				cpuless = TRUE;
			}
		break;

	}

	info = usbdev_init(bcm_device_ids, &bcmdev);
	if ((info == NULL) || (bcmdev == NULL)) {
		printf("No devices found\n");
		return -1;
	}

	fprintf(stderr, "Found Device %s", bcmdev->name);
	if (strstr(bcmdev->name, "CPULess")) {
		cpuless = TRUE;
		fprintf(stderr, ", operating in CPULess mode");
	}
	fprintf(stderr, "\n");

#ifdef BCMDBG
	printf("Found Device %s\n", bcmdev->name);
	usb_set_debug(0xff);
#endif

	switch (cmd) {
		case BLDR_VERSION:
		{
			bootrom_id_t id;
			/* get version */
			status = usbdev_control_read(info, DL_GETVER, 1, 0, (char*)&id,
			                             sizeof(id), TRUE, TIMEOUT);

			if (status < 0)
		goto err;

			if (status == sizeof(id)) {
				/* Latest bootloder returns ramsize, chiprev & board rev
				 * as part of it.
				 */
				printf("Get CHIP revision: %x, rev=%d, ramsize=0x%X remapbase=0x%X "
				"boardtype=%d boardrev=%d\n", id.chip, id.chiprev, id.ramsize,
				id.remapbase, id.boardtype, id.boardrev);
			} else {
				printf("Get CHIP revision: %x, rev=%d\n", id.chip, id.chiprev);
			}
	}
		break;

		case BLDR_BOOT:
		{
			rdl_state_t rdl;
			/* Hardware access */
			/* reboot device */
			status = usbdev_control_read(info, DL_REBOOT, 1, 0, (char*)&rdl,
			                             sizeof(rdl_state_t), TRUE, TIMEOUT);
			if (status < 0)
		goto err;
			printf("Dongle reboot success\n");
		}
		break;

		case BLDR_EXEC:
		{
			uint32 null_cmd = 0;
			/* Hardware access */
			status = usbdev_control_write(info, DL_EXEC, (addr & 0xffff),
						      (addr >> 16), (char *)&null_cmd,
						      sizeof(null_cmd), TRUE, TIMEOUT);
			if (status < 0)
				goto err;
		}
		break;

		case BLDR_READ:
		{
			if (cpuless) {
				if (datalen <= 4) {
					status = usbdev_control_read(info, 0x00,
					                             (addr & 0xffff),
					                             (addr >> 16),
					                             (char*)&hwacc.data,
					                             datalen, FALSE,
					                             TIMEOUT);
				} else {
					uchar *buf;
					int len = 0;
					int totlen = datalen;
					FILE *fp = NULL;
					if (filename) {
						if ((fp = fopen(filename, "wb")) == NULL) {
							fprintf(stderr,
								"%s: Could not open %s: %s\n",
								__FUNCTION__, filename,
								strerror(errno));
						}
					}

					buf = malloc(MEMBLOCK);
					if (buf == NULL) {
						printf("%s: malloc failure: %s\n", argv[0],
						       strerror(errno));
						goto err;
					}
					while (totlen > 0) {
						len = totlen > (MEMBLOCK - 16) ? (MEMBLOCK - 16) :
							totlen;
						memset(buf, '\0', MEMBLOCK);
						status = usbdev_control_read(info, 0x00,
						                             (addr & 0xffff),
						                             (addr >> 16),
						                             buf, len,
						                             FALSE, TIMEOUT);
						if (status >= 0) {
							if (fp) {
								if (fwrite(buf, sizeof(unsigned char), len, fp) != len) {
									fprintf(stderr, "%s: error writing to file %s\n", __FUNCTION__, filename);
								}
							} else {
								myprhex(buf, len, addr);
							}
						}
						else
							break;
						totlen -= (MEMBLOCK - 16);
						addr += (MEMBLOCK - 16);
					}
					if (fp) {
						fclose(fp);
					}
					free(buf);
					hwacc.data = addr;
				}
				if (status < 0)
					goto err;
			} else {
				uint32 cmd;
				if (datalen != 1 && datalen != 2 && datalen != 4) {
					printf("Invalid datalen: must be one of 1, 2 or 4\n");
					usage();
				}
				hwacc.addr = addr;
				if (datalen == 1)
					cmd = DL_RDHW8;
				else if (datalen == 2)
					cmd = DL_RDHW16;
				else
					cmd = DL_RDHW32;

				/* Hardware access */
				status = usbdev_control_read(info, cmd, (addr & 0xffff),
				                             (addr >> 16), (char *)&hwacc,
				                             sizeof(hwacc_t), TRUE,
				                             TIMEOUT);

				if (status < 0 || hwacc.cmd != DL_RDHW|| hwacc.addr != addr ||
				    hwacc.len != datalen)
					goto err;
			}
			if (datalen <= 4)
				printf("0x%x\n", hwacc.data);
		}
		break;

		case BLDR_WRITE:
			if (cpuless) {
				if (datalen <= 4) {
					status = usbdev_control_write(info, 0x00,
					                              (addr & 0xffff),
					                              (addr >> 16),
					                              (char*)&data,
					                              4, FALSE, TIMEOUT);
				} else {
					status = usbdev_control_write(info, 0x00,
					                              (addr & 0xffff),
					                              (addr >> 16),
					                              (char*)datap,
					                              datalen, FALSE, TIMEOUT);
					free(datap);
				}
				if (status < 0)
					goto err;
			} else {
				hwacc.cmd = DL_WRHW;
				hwacc.addr = addr;
				hwacc.data = data;
				hwacc.len = datalen;
				status = usbdev_control_write(info, DL_WRHW, 1,
				                              0, (char *)&hwacc,
				                              sizeof(hwacc_t),
				                              TRUE, TIMEOUT);
				if (status < 0)
					goto err;
			}
			break;

		case BLDR_WRITE_BLK:
			status = usbdev_control_write(info, DL_WRHW_BLK, 1, 0,
			                              (char *)blk_hwacc,
			                              (sizeof(hwacc_blk_t) + datalen - 1),
			                              TRUE, TIMEOUT);

			if (status < 0)
				goto err;

			break;

	        case BLDR_DUMP:
		{
			/* Dump the RAM contents */

			/* Find the dump info area using magic pointer */
			uint32 dumpInfoPtrPtr;
			uint32 dumpInfoMagic;
			uint32 dumpInfoPtr;
			hnd_debug_t debugInfo;
			trap_t armtrap;
			int i;

			FILE *fp = NULL;

			/*
			 * Different chips have different fixed
			 * dump_info_ptr_ptrs becuase of different ROM
			 * locations/uses.  Try them all looking for
			 * the magic number.
			 */
			for (i = 0;; i++) {
				if (dump_info_ptr_ptr[i] == DUMP_INFO_PTR_PTR_END) {
					goto err;
				}
		
				if (readbuf(dump_info_ptr_ptr[i], 4, (uchar *)
				            &dumpInfoMagic) < 0) {
					goto err;
				}

				if (dumpInfoMagic == HND_DEBUG_PTR_PTR_MAGIC) {
					break;
				}
			}

			if (readbuf(dump_info_ptr_ptr[i] + 4, 4, (uchar *)
			            &dumpInfoPtrPtr) < 0) {
				goto err;
			}

			if (dumpInfoPtrPtr == 0) {
				fprintf(stderr, "Error: Dump info ptr ptr is zero\n");
				goto errret;
			}

			if (readbuf(dumpInfoPtrPtr, 4, (uchar *) &dumpInfoPtr) < 0) {
				goto err;
			}

			if (dumpInfoPtr == 0) {
				fprintf(stderr, "Error: Dump info ptr is zero\n");
				goto errret;
			}

			/* Read the area the debuginfoptr points at */
			if (readbuf(dumpInfoPtr, sizeof(hnd_debug_t),
			            (uchar *) &debugInfo) < 0) {
				goto err;
			}

			/* Sanity check the area */
			if ((debugInfo.magic != HND_DEBUG_MAGIC) ||
			    (debugInfo.version != HND_DEBUG_VERSION)) {
				fprintf(stderr, "Error: Invalid debug info area\n");
				goto errret;
			}


			/* Get the base and size to dump */
			int ram_addr = debugInfo.ram_base;
			int ram_size = debugInfo.ram_size;

			/* Get the arm trap area */
			prstatus_t prstatus;
			bzero(&prstatus, sizeof(prstatus));
			if (debugInfo.trap_ptr != 0) {
				int i;
				if (readbuf(debugInfo.trap_ptr, sizeof(trap_t),
				            (uchar *) &armtrap) < 0) {
					goto err;
				}

				/* Populate the prstatus */
				prstatus.si_signo = armtrap.type;
				uint32 *reg = &armtrap.r0;
				for (i = 0; i < 15; i++, reg++) {
					prstatus.uregs[i] = *reg;
				}
				prstatus.uregs[15] = armtrap.epc;
			}

			if (filename) {
				if ((fp = fopen(filename, "wb")) == NULL) {
					fprintf(stderr,
						"%s: Could not open %s: %s\n",
						__FUNCTION__, filename,
						strerror(errno));
				}
			} else {
				fp = stdout;
			}


			fprintf(fp, "Dump starts for version %s FWID 01-%x\n",
				debugInfo.epivers, debugInfo.fwid);
			fprintf(fp, "XXXXXXXXXXXXXXXXXXXX");
			fprintf(fp, "%8.8X", (unsigned int) sizeof(debugInfo));

			if (fwrite(&debugInfo, sizeof(unsigned char), sizeof(debugInfo), fp) !=
			    sizeof(debugInfo)) {
				fprintf(stderr, "%s: error writing to file %s\n",
					__FUNCTION__, filename);
			}

			if (fwrite(&prstatus, sizeof(unsigned char), sizeof(prstatus), fp) !=
			    sizeof(prstatus)) {
				fprintf(stderr, "%s: error writing to file %s\n",
					__FUNCTION__, filename);
			}

			/* Write the ram size as another sanity check */
			fprintf(fp, "%8.8X", ram_size);

			/* Get a buffer to use for writing the RAM */
			uchar *buf = malloc(MEMBLOCK);
			if (buf == NULL) {
				fprintf(stderr, "malloc failure: %s\n", strerror(errno));
				goto err;
			}

			/* Read and write RAM in MEMBLOCK sized chunks */
			int totlen = ram_size;
			while (totlen > 0) {
				int len = totlen > (MEMBLOCK - 16) ? (MEMBLOCK - 16) : totlen;

				if (readbuf(ram_addr, len, buf) < 0) {
					goto err;
				}

				if (fwrite(buf, sizeof(unsigned char), len, fp) != len) {
					fprintf(stderr, "%s: error writing to file %s\n",
						__FUNCTION__, filename);
				}

				totlen -= len;
				ram_addr += len;
			}

			/* Cleanup */
			if (fp) {
				fclose(fp);
			}
			free(buf);
		}
		break;

		default:
			usage();
	}

	return 0;

err:
	printf("Error\n");

errret:
	if (info) {
		usbdev_deinit(info);
	}

return -1;
}
