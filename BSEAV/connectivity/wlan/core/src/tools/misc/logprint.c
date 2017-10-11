/*
 * File PRINTLOG utility - Print the event log
 *
 * Copyright (C) 2012 Broadcom Corporation
 *
 * $Id: printlog.c 241182 2011-02-17 21:50:03Z $
 */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <bcmtlv.h>

#include <typedefs.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include "logdump_decode.h"

#define DUMP_INFO		/* Used to select options in debug.h */
#include "hnd_debug.h"

/* This define turns pointers into uint32 for compatability between
 * the target processor and the processor running this utility
 */
#define EVENT_LOG_COMPILE
#define EVENT_LOG_DUMPER
#include "event_log.h"

#include "proto/event_log_payload.h"

#define MAX_BLOCKS 100		/* No more than 100 blocks per set */
#define MAX_SETS 25		/* No more than 25 sets */

static char *progname;

static unsigned char *shadow_ram;

static int reverse = 0;

static uint32 ram_addr;
static uint32 ram_size;
static FILE *dump;

static char **fmts = NULL;		/* Format string area */
static int num_fmts = -1;

static uint32 prev_ts = 0;

static int logstrs_size = 0;

static event_log_top_t top;

static int setmask = 0;
static int merge = 0;
static int raw = 0;
static int noinfo = 0;
static int piped = 0;
static int decode = 0;
static int logtrace = 0;
static int set;

static FILE *fmtfile = NULL;
static FILE *debugfile = NULL;

const uint32 dump_info_ptr_ptr[] = {DUMP_INFO_PTR_PTR_LIST};

#define MAGIC_NUMBER 0xeae47c06

typedef struct host_packet_header {
	int magic_num;
	int buf_size;
	int seq_num;
} host_packet_header_t;

eventdump_header_t eventdump_header[EVENT_LOG_TAG_MAX+1];

eventdump_info_t eventdump_info[EVENT_LOG_TAG_MAX+1];

#define ARMCYCLE_PER_MSECOND_4345 215713
#define ARMCYCLE_PER_MSECOND_4350 361125
#define ARMCYCLE_PER_MSECOND_4334 145500
#define ARMCYCLE_PER_MSECOND_4324 215713
#define ARMCYCLE_PER_MSECOND_4355 320000


int armcycle_per_msec = ARMCYCLE_PER_MSECOND_4350;

static uint32 setp[MAX_SETS];

typedef void (*logbuf_format_fn_t)(bcm_xtlv_t *xtlv);

void format_logbuf(uint32 tag, uint32 fmt, uint32 count, uint32 *args);
void format_raw(uint32 tag, uint32 fmt_num, uint32 len, uint8* data);
void format_string(bcm_xtlv_t *xtlv);
void format_txq_summary(bcm_xtlv_t *xtlv);
void format_scb_subq_summary(bcm_xtlv_t *xtlv);
void format_scb_ampdu_summary(bcm_xtlv_t *xtlv);
void format_bsscfg_q_summary(bcm_xtlv_t *xtlv);
void format_txstatus_log(bcm_xtlv_t *xtlv);

logbuf_format_fn_t fmt_fns[] = {
	format_string,
	format_txq_summary,
	format_scb_subq_summary,
	format_scb_ampdu_summary,
	format_bsscfg_q_summary,
	format_txstatus_log,
};

#define LAST_FMTFN	ARRAYSIZE(fmt_fns)

static void
usage(void)
{
	fprintf(stderr, "Usage: %s [-d <dumpfile>] [-fmts <fmtfile>] "
		"[-merge] [-set n] [-raw] [-reverse] [-logtrace] [-piped] [-decode]\n"
		"\t<dumpfile> is producing using \"dlcmd d\"\n"
		"\t           defaults to stdin for piping from live systems\n"
		"\t<fmtfile> is a file contining the format strings associated with this dumpfile\n"
		"\t          If no fmtfile is specified then the log entries are\n"
		"\t          printed as integers\n"
		"\t-merge prints all sets using a timestamp-based merge\n"
		"\t       The default is to print each set seperately\n"
		"\t-set n specifies that only set N should be printed\n"
		"\t       The set may be repeated to print more than one set\n"
		"\t       The default is to print all sets\n"
		"\t-reverse prints the log entries in reverse order (newest first)\n"
		"\t-raw disables the prescan of the input for the start of the dump data\n"
		"\t-logtrace prints logtrace output from stdin (usually piped)\n"
		"\t-piped uart output of printed logs entries (TAG_FLAG_PRINT)\n",

		progname);
	exit(-1);
}

void arm_cycle_per_sec(char *chip)
{
	if (!strncmp(chip, "4345", 4))
		armcycle_per_msec = ARMCYCLE_PER_MSECOND_4345;

	if (!strncmp(chip, "4350", 4))
		armcycle_per_msec = ARMCYCLE_PER_MSECOND_4350;

	if (!strncmp(chip, "4334", 4))
		armcycle_per_msec = ARMCYCLE_PER_MSECOND_4334;

	if (!strncmp(chip, "4324", 4))
		armcycle_per_msec = ARMCYCLE_PER_MSECOND_4324;

	if (!strncmp(chip, "4355", 4))
		armcycle_per_msec = ARMCYCLE_PER_MSECOND_4355;

	fprintf(debugfile, "armcycle_per_sec: =%d\n", armcycle_per_msec);
}

int
printf_percent_s_to_p(char *fmt)
{
	int s_to_d_done = FALSE;

	while (*fmt != '\0')
	{
		/* Skip characters will we see a % */
		if (*fmt++ != '%')
		{
			continue;
		}

		/*
		 * Skip any flags, field width and precision:
		 *Flags: Followed by %
		 * #, 0, -, ' ', +
		 */
		if (*fmt == '#')
			fmt++;

		if (*fmt == '0' || *fmt == '-' || *fmt == '+')
			fmt++;

		/*
		 * Field width:
		 * An optional decimal digit string (with non-zero first digit)
		 * specifying a minimum field width
		 */
		while (*fmt && isdigit(*fmt))
			fmt++;

		/*
		 * Precision:
		 * An optional precision, in the form of a period ('.')  followed by an
		 * optional decimal digit string.
		 */
		if (*fmt == '.')
		{
			fmt++;
			while (*fmt && isdigit(*fmt)) fmt++;
		}

		/* If %s is seen, change it to %p */
		if (*fmt == 's')
		{
			*fmt = 'p';
			s_to_d_done = TRUE;
		}
		if (*fmt)
			fmt++;
	}

	return s_to_d_done;
}

/* process one eventlog message */
void process_event_log_data(uint32 *header, uint32 *data)
{
int fmt_num;
FILE *log_fp;
event_log_hdr_t *log_header;
uint32 arm_time_cycle;

	/* get eventlog msg header */
	log_header = (event_log_hdr_t*)(header+1);
	arm_time_cycle = *(header);

	fmt_num = log_header->fmt_num >> 2;
	log_fp = NULL;

	fprintf(debugfile, "log_header size: %d tag:%d format: %x type: %x\n",
		log_header->count, log_header->tag,
		log_header->fmt_num >> 2, log_header->t);

	if (log_header->tag > EVENT_LOG_TAG_MAX) {
		fprintf(debugfile, "getting invalid log header tag:%d \n", log_header->tag);
		return;
	}

	/* if a dump message and a callback is registred process the data in the callback */
	if ((fmt_num == 0x3fff) || (log_header->tag == EVENT_LOG_TAG_TRACE_CHANSW)) {
		if ((eventdump_header[log_header->tag].dump_callback != NULL) &&
			(eventdump_info[log_header->tag].statistic_file != NULL)) {
				eventdump_header[log_header->tag].dump_callback(
					&eventdump_info[log_header->tag],
					data,
					arm_time_cycle,
					log_header->count);
				fprintf(debugfile, "\n");
		}
		else {
			fprintf(debugfile, "no log file defined for tag: %d dumping data\n",
				log_header->tag);
		}
	}

	/* log logstring eventlog messages */
	log_fp = eventdump_info[log_header->tag].log_file;

	if ((fmt_num != 0x3fff) && (fmt_num != 0))
	{
		/* Just pass 16 args to cover all cases */
		char *fmt = fmts[fmt_num];

		if (log_fp) {
			fprintf(log_fp, "log_header size: %d tag:%d format: %x type: %x\n",
				log_header->count, log_header->tag,
					log_header->fmt_num >> 2,
					log_header->t);

			fprintf(log_fp, fmt,
				data[0], data[1],
				data[2], data[3],
				data[4], data[5],
				data[6], data[7],
				data[8], data[9],
				data[10], data[11],
				data[12], data[13],
				data[14], data[15]);

			if (fmt[strlen(fmt) - 1] != '\n') {
				fprintf(log_fp, "\n");	/* Add newline if missing */
			}
		}

		printf(fmt,
			data[0], data[1],
			data[2], data[3],
			data[4], data[5],
			data[6], data[7],
			data[8], data[9],
			data[10], data[11],
			data[12], data[13],
			data[14], data[15]);

		if (fmt[strlen(fmt) - 1] != '\n') {
			printf("\n");	/* Add newline if missing */
		}
	}

	if (fmt_num == 0x3fff) {
		format_logbuf(set, log_header->fmt_num, (log_header->count - 1), data);
		printf("\n");
	}

}

double base_time = 0;

/* Print a single entry */
void
log_print(uint32 *log_ptr, uint32 timestamp, uint32 base_cycles, int set)
{
	event_log_hdr_t hdr;

	hdr.t = *log_ptr++;

	if (hdr.tag == EVENT_LOG_TAG_NULL) {
		return;
	}

	/* Print the prologue */
	if (prev_ts != timestamp) {
		base_time = ((double)(timestamp)/MSEC_IN_SECOND);
		printf("%.6f %2d ", base_time, set);
	} else {
		printf("         %2d ", set);
	}

	if (hdr.tag == EVENT_LOG_TAG_TS) {
		/* Time sync */
		printf("%.6f: Timesync %.6f:\n",
				base_time+((double)TIME_IN_SECONDS((*log_ptr - base_cycles))),
				((double)(*(log_ptr + 1)))/MSEC_IN_SECOND);
		prev_ts = timestamp;
	} else {
		/* Normal log data entry */
		int fmt_num = hdr.fmt_num >> 2;	/* Entry is *4 */

		printf("%.6f: tag: %2.2d ",
			base_time+((double)TIME_IN_SECONDS((*log_ptr - base_cycles))),
		    hdr.tag);

		if (fmt_num > num_fmts) {
			/* should really just do this for 'tag' values
			 * that don't have a defined binary type
			 */
			format_logbuf(set, hdr.fmt_num, (hdr.count - 1), log_ptr);
		} else {
			/* Just pass 16 args to cover all cases */
			char *fmt = fmts[fmt_num];
			printf(fmt,
			       *log_ptr, *(log_ptr + 1), *(log_ptr + 2),
			       *(log_ptr + 3), *(log_ptr + 4), *(log_ptr + 5),
			       *(log_ptr + 6), *(log_ptr + 7), *(log_ptr + 8),
			       *(log_ptr + 9), *(log_ptr + 10),
			       *(log_ptr + 11), *(log_ptr + 12),
			       *(log_ptr + 13), *(log_ptr + 14),
			       *(log_ptr + 15));
			if (fmt[strlen(fmt) - 1] != '\n') {
				printf("\n");	/* Add newline if missing */
			}
		}

	}

	prev_ts = timestamp;

}

/* Print a single entry  in reverse */
void
reverse_print(uint32 *log_ptr, uint32 timestamp, uint32 base_cycles, int set)
{
	int i;
	uint32 data_save[257];
	event_log_hdr_t hdr;

	hdr.t = *log_ptr;
	data_save[0] = *log_ptr;
	log_ptr -= hdr.count;

	/* Save the data copying the timestamp first */
	data_save[1] = log_ptr[hdr.count - 1];
	for (i = 0; i < (hdr.count - 1); i++) {
		data_save[i + 2] = log_ptr[i];
	}

	log_print(&data_save[0], timestamp, base_cycles, set);
}

/* Perform seek/read with checks */
void
dump_get(void *dest, int size, int offset)
{
	if ((offset < ram_addr) || ((offset + size) > (ram_addr + ram_size))) {
		fprintf(stderr, "%s Internal Error: attempt to seek to offset "
				"ram - base %x size %d"
				"outside ram - offset %x size %d\n",
				progname, ram_addr, ram_size, offset, size);
		exit(-1);
	}
	bcopy(&shadow_ram[offset-ram_addr], dest, size);
}

uint32 *
next_entry(uint32 *ptr, uint32 *limit)
{
	event_log_hdr_t hdr;
	hdr.t = *ptr;
	/* Skip null entries */
	while (1) {
		if (reverse) {
			if (ptr <= limit) {
				return 0;
			}
		} else {
			if (ptr >= limit) {
				return 0;
			}
		}
		hdr.t = *ptr;
		if (hdr.tag != EVENT_LOG_TAG_NULL) {
			break;
		}
		if (reverse) {
			ptr--;
		} else {
			ptr++;
		}
	}

	return ptr;
}

void
read_fmtfile(void)
{
	logstr_header_t *hdr;
	uint32 *lognums;
	char *logstrs;
	int ram_index;

	if (fmtfile != NULL) {
		/* Read in the format strings */
		int i;
		char *raw_fmts = malloc(logstrs_size);
		if (raw_fmts == NULL) {
			fprintf(stderr, "%s Internal Error: Malloc failure at line %d\n",
				progname, __LINE__);
			exit(-1);
		}

		if (fread(raw_fmts, sizeof(char), logstrs_size, fmtfile) !=
		    logstrs_size) {
			fprintf(stderr, "%s Error: Log strings file read failed\n", progname);
			exit(-1);
		}

		/* Remember header from the logstrs.bin file */
			hdr = (logstr_header_t *) (raw_fmts + logstrs_size -
				sizeof(logstr_header_t));

		if (hdr->log_magic == LOGSTRS_MAGIC) {
			/*
			* logstrs.bin start with header.
			*/
			num_fmts =  hdr->rom_logstrs_offset / sizeof(uint32);
			ram_index = (hdr->ram_lognums_offset -
				hdr->rom_lognums_offset) / sizeof(uint32);
			lognums = (uint32 *) &raw_fmts[hdr->rom_lognums_offset];
			logstrs = (char *)   &raw_fmts[hdr->rom_logstrs_offset];
		} else {

			/*
			 * Legacy logstrs.bin format without header.
			 */

			num_fmts = *((uint32 *) (raw_fmts)) / sizeof(uint32);
			if (num_fmts == 0) {

				/* Legacy ROM/RAM logstrs.bin format:
				  *	 - ROM 'lognums' section
				  *	  - RAM 'lognums' section
				  *	  - ROM 'logstrs' section.
				  *	  - RAM 'logstrs' section.
				  *
				  * 'lognums' is an array of indexes for the strings in the
				  * 'logstrs' section. The first uint32 is 0 (index of first
				  * string in ROM 'logstrs' section).
				  *
				  * The 4324b5 is the only ROM that uses this legacy format. Use the
				  * fixed number of ROM fmtnums to find the start of the RAM
				  * 'lognums' section. Use the fixed first ROM string ("Con\n") to
				  * find the ROM 'logstrs' section.
				  */

				#define NUM_4324B5_ROM_FMTS		186
				#define FIRST_4324B5_ROM_LOGSTR	"Con\n"
				ram_index = NUM_4324B5_ROM_FMTS;
				lognums = (uint32 *) raw_fmts;

				num_fmts =  ram_index;
				logstrs = (char *) &raw_fmts[num_fmts << 2];
				while (strncmp(FIRST_4324B5_ROM_LOGSTR, logstrs, 4)) {
					num_fmts++;
					logstrs = (char *) &raw_fmts[num_fmts << 2];
				}
			} else {
					/* Legacy RAM-only logstrs.bin format:
					 *	  - RAM 'lognums' section
					 *	  - RAM 'logstrs' section.
					 *
					 * 'lognums' is an array of indexes for the strings in the
					 * 'logstrs' section. The first uint32 is an index to the
					 * start of 'logstrs'. Therefore, if this index is divided
					 * by 'sizeof(uint32)' it provides the number of logstr
					 *  entries.
					 */
					ram_index = 0;
					lognums = (uint32 *) raw_fmts;
					logstrs = (char *)	 &raw_fmts[num_fmts << 2];
				}
		}

		fmts = calloc(num_fmts, sizeof(char *));
		if (fmts == NULL) {
			fprintf(stderr, "%s Internal Error: Malloc failure (fmts)\n", progname);
			exit(-1);
		}

		for (i = 0; i < num_fmts; i++) {
			/* ROM lognums index into logstrs using 'rom_logstrs_offset' as a base
			* (they are 0-indexed relative to 'rom_logstrs_offset').
			*
			* RAM lognums are already indexed to point to the correct RAM logstrs (they
			* are 0-indexed relative to the start of the logstrs.bin file).
			*/
			if (i == ram_index) {
				logstrs = raw_fmts;
			}
			fmts[i] = &logstrs[lognums[i]];

			/* Check if %s is present and change it to %p in place. */
			printf_percent_s_to_p(fmts[i]);
		}
	}
}

void
decode_fmtfile(void)
{
	int fmt_num;

	read_fmtfile();

	for (fmt_num = 1; fmt_num < num_fmts; ++fmt_num) {
		/* Just pass 16 args to cover all cases */
		char *fmt = fmts[fmt_num];
		printf("%3x: %s", fmt_num << 2, fmt);
		if (fmt[strlen(fmt) - 1] != '\n')
			printf("\n");	/* Add newline if missing */
	}
}

void
process_piped(void)
{
	char *cp;
	char buff[256];
	uint32 ts_sec, ts_ms, tag, fmt, args[16];
	int match, fmt_num;

	read_fmtfile();
	while (1) {

		if (fgets(buff, 256, stdin) == NULL) {
			exit(0);	/* EOF */
		}
		if (errno != 0) {
			exit(errno);	/* Read error */
		}
		match = 0;
		if ((cp = strstr(buff, " EL: "))) {
			--cp;
			while (cp > buff && !isspace(*cp))
				--cp;
			match = sscanf(cp, "%d.%d EL: %x %x"
			               " %x %x %x %x %x"
			               " %x %x %x %x %x"
			               " %x %x %x %x %x %x",
			               &ts_sec, &ts_ms, &tag, &fmt,
			               &args[0], &args[1], &args[2], &args[3], &args[4],
			               &args[5], &args[6], &args[7], &args[8], &args[9],
			               &args[10], &args[11], &args[12], &args[13],
			               &args[14], &args[15]);
			if (errno == ERANGE) {
				printf("***Failed to interpret the next line (%s errno=%d)\n",
					strerror(errno), errno);
				errno = 0;
			}
		}
		if (match < 3) {
			/* it's not encoded so just print it */
			printf(buff);
			continue;
		}

		fmt_num = fmt >> 2;		/* Entry is *4 */

		/* print the timestamp in a compatible format */
		printf("[%06d.%03d] ", ts_sec, ts_ms);

		if (fmt_num > num_fmts) {
			/* should really just do this for 'tag' values
			 * that don't have a defined binary type
			 */
			format_logbuf(tag, fmt, match - 4, args);
		} else {
			/* Just pass 16 args to cover all cases */
			char *fmt = fmts[fmt_num];
			if (cp && cp > buff) {
				*cp = '\0';
				printf("%s ", buff);
			}
			printf(fmt,
			       args[0], args[1], args[2], args[3], args[4],
			       args[5], args[6], args[7], args[8], args[9],
			       args[10], args[11], args[12], args[13],
			       args[14], args[15]);
			if (fmt[strlen(fmt) - 1] != '\n') {
				printf("\n");	/* Add newline if missing */
			}
		}
	}	 /* End piped while */
}

void
format_logbuf(uint32 tag, uint32 fmt, uint32 count, uint32 *args)
{
	uint8 data[4*256] = {0};
	bcm_xtlv_t *xtlvp;
	uint16 xtlv_type;
	uint16 xtlv_len;
	uint len;
	uint i;

	count = MIN(256, count);
	/* get the length in bytes */
	len = count * 4;

	if (count == 0) {
		printf("fmt: %X len %d:", fmt, len);
		format_raw(tag, fmt, len, data);
		return;
	}

	/* move the uint32 values from arg into a byte array
	 * treating the uint32s as LE
	 */
	for (i = 0; i < count; i++) {
		uint32 v = args[i];

		data[i * 4 + 0] = (uint8) v;
		data[i * 4 + 1] = (uint8)(v >> 8);
		data[i * 4 + 2] = (uint8)(v >> 16);
		data[i * 4 + 3] = (uint8)(v >> 24);
	}

	xtlvp = (bcm_xtlv_t*)data;

	/* if we are not looking at a valid XTLV, just print raw bytes */
	if (!bcm_valid_xtlv(xtlvp, len, BCM_XTLV_OPTION_NONE)) {
		/* one more chance for the bad length txstatus structs,
		 * adjust the len down by 4 bytes
		 */
		if (len > BCM_XTLV_HDR_SIZE &&
		    xtlvp->id == EVENT_LOG_XTLV_ID_UCTXSTATUS &&
		    xtlvp->len > BCM_XTLV_HDR_SIZE) {

			xtlvp->len -= BCM_XTLV_HDR_SIZE; /* adjust for fw bug */

			if (!bcm_valid_xtlv(xtlvp, len, BCM_XTLV_OPTION_NONE)) {
				/* still no good */
				xtlvp->len += BCM_XTLV_HDR_SIZE; /* restore orig */
				printf("fmt: %X len %d:", fmt, len);
				format_raw(tag, fmt, len, data);
				return;
			}
		} else {
			printf("fmt: %X len %d:", fmt, len);
			format_raw(tag, fmt, len, data);
			return;
		}
	}

	xtlv_type = BCM_XTLV_ID(xtlvp);
	xtlv_len  = BCM_XTLV_LEN(xtlvp);

	if (xtlv_type < LAST_FMTFN) {
		printf("fmt: %X len %d: XTLV ID %u Len %u:", fmt, len, xtlv_type, xtlv_len);

		/* call the xtlv_type specific format fn */
		(fmt_fns[xtlv_type])(xtlvp);
	} else {
		printf("fmt: %X len %d: XTLV ID %u Len %u", fmt, len, xtlv_type, xtlv_len);

		/* no registered fn, just print raw */
		format_raw(tag, fmt, len - BCM_XTLV_HDR_SIZE, data + BCM_XTLV_HDR_SIZE);
	}
}

void
format_raw(uint32 tag, uint32 fmt, uint32 len, uint8* data)
{
	int i;

	printf("\n");

	for (i = 0; i < len; i++) {
		printf(" %02X", data[i]);
		/* newline after every 4 words, extra space after every word */
		if (i % 16 == 15) {
			printf("\n");
		} else if (i % 4 == 3) {
			printf(" ");
		}
	}

	/* if we did not end at a new line, print one */
	if (i % 16 != 0) {
		printf("\n");
	}
}

void
format_string(bcm_xtlv_t *xtlv)
{
	int i;
	char *s = (char*)xtlv->data;

	printf(" \"");

	for (i = 0; i < xtlv->len; i++) {
		char c = s[i];

		if (isprint(c)) {
			printf("%c", c);
		} else {
			/* if not printable, print as "\xAB" style escape */
			printf("\\x%02X", (uint)c);
		}

		/* wrap at 80 chars */
		if (i % 80 == 79) {
			printf("\n");
		}
	}

	printf("\"\n");
}

void
format_txq_summary(bcm_xtlv_t *xtlv)
{
	int i;
	txq_summary_t sum;
	uint32 bmap;
	int first;
	uint fixed_payload = (TXQ_SUMMARY_LEN - BCM_XTLV_HDR_SIZE);
	uint rem;
	uint16 *p;

	if (xtlv->len < fixed_payload) {
		printf("TxQ Summary: ERR: xtlv too short, %d bytes, expected %d\n",
		       xtlv->len, fixed_payload);
		return;
	}

	memcpy(&sum, xtlv, TXQ_SUMMARY_LEN);

	printf("\nTxQ Summary: BSSCFGS [");

	/* print the bsscfg idx numbers associated with the queue */
	bmap = ltoh32(sum.bsscfg_map);
	first = TRUE;
	for (i = 0; i < 32; i++) {
		if ((bmap & (1<<i)) != 0) {
			if (first) {
				printf("%d", i);
				first = FALSE;
			} else {
				printf(" %d", i);
			}
		}
	}
	printf("] ");

	printf("flowctl 0x%0X\n", ltoh32(sum.stopped));

	/* Calc how much space remains after the fixed portion of the txq_summary struct.
	 * xtlv->len is the actual payload space in the xtlv. fixed_payload is the
	 * fixed part of txq_summary_t before the variable plen[].
	 */
	rem = xtlv->len - fixed_payload;
	if (rem < sum.prec_count * sizeof(sum.plen[0])) {
		printf("*ERR* too short for plen array, %d bytes, expected %d\n",
		       rem, (int)(sum.prec_count * sizeof(sum.plen[0])));
		return;
	}

	printf("qlen (%d queues): ", sum.prec_count);

	p = (uint16*)&xtlv->data[fixed_payload];

	for (i = 0; i < sum.prec_count; i++, p++) {
		uint16 plen = ltoh_ua(p);
		printf(" %u", plen);
	}
	printf("\n");
}

void
format_scb_subq_summary(bcm_xtlv_t *xtlv)
{
	int i;
	scb_subq_summary_t sum;
	uint fixed_payload = (SCB_SUBQ_SUMMARY_LEN - BCM_XTLV_HDR_SIZE);
	uint rem;
	uint16 *p;

	if (xtlv->len < fixed_payload) {
		printf("SCBQ Summary: ERR: xtlv too short, %d bytes, expected %d\n",
		       xtlv->len, fixed_payload);
		return;
	}

	memcpy(&sum, xtlv, SCB_SUBQ_SUMMARY_LEN);

	printf("\nSCBQ Summary: Cubby ID/SubID %d/%d flags 0x%08X\n", sum.cubby_id, sum.sub_id, sum.flags);

	/* Calc how much space remains after the fixed portion of the txq_summary struct.
	 * xtlv->len is the actual payload space in the xtlv. fixed_payload is the
	 * fixed part of txq_summary_t before the variable plen[].
	 */
	rem = xtlv->len - fixed_payload;
	if (rem < sum.prec_count * sizeof(sum.plen[0])) {
		printf("*ERR* too short for plen array, %d bytes, expected %d\n",
		       rem, (int)(sum.prec_count * sizeof(sum.plen[0])));
		return;
	}

	printf("qlen (%d queues): ", ltoh16(sum.prec_count));
	p = (uint16*)&xtlv->data[fixed_payload];

	for (i = 0; i < sum.prec_count; i++, p++) {
		uint16 plen = ltoh_ua(p);
		printf(" %u", plen);
	}
	printf("\n");
}

void
format_scb_ampdu_summary(bcm_xtlv_t *xtlv)
{
	scb_ampdu_tx_summary_t sum;
	uint32 flags;
	bool bar_ackpend;

	if (xtlv->len < sizeof(scb_ampdu_tx_summary_t) - BCM_XTLV_HDR_SIZE) {
		printf("SCB AMPDU TID Summary: ERR: xtlv too short, %d bytes, expected %d\n",
		       xtlv->len, (int)(sizeof(scb_ampdu_tx_summary_t)));
		return;
	}

	memcpy(&sum, xtlv, sizeof(scb_ampdu_tx_summary_t));
	flags = ltoh32(sum.flags);
	bar_ackpend = (flags & SCBDATA_AMPDU_TX_F_BAR_ACKPEND) != 0;

	printf("\nSCB AMPDU TID Summary: TID %d flags 0x%08X (%s) ba_state %d "
	       "start_seq 0x%04X max_seq 0x%04X\n",
	       sum.tid, flags, (bar_ackpend ? "BAR_ACKPEND" : ""),
	       sum.ba_state, ltoh16(sum.start_seq), ltoh16(sum.max_seq));
	printf("rel_bytes_inflight %d rel_bytes_target %d "
	       "bar_cnt %d barpending_seq 0x%04X, bar_ackpending_seq 0x%04X\n",
	       ltoh32(sum.released_bytes_inflight), ltoh32(sum.released_bytes_target),
	       sum.bar_cnt, ltoh16(sum.barpending_seq), ltoh16(sum.bar_ackpending_seq));
}

void
format_bsscfg_q_summary(bcm_xtlv_t *xtlv)
{
	int i;
	bsscfg_q_summary_t sum;
	uint fixed_payload = (BSSCFG_Q_SUMMARY_LEN - BCM_XTLV_HDR_SIZE);
	uint rem;
	uint16 *p;

	if (xtlv->len < fixed_payload) {
		printf("BSSCFG Q Summary: ERR: xtlv too short, %d bytes, expected %d\n",
		       xtlv->len, fixed_payload);
		return;
	}

	memcpy(&sum, xtlv, BSSCFG_Q_SUMMARY_LEN);

	printf("\nBSSCFG Q Summary: IDX %d %02X:%02X:%02X:%02X:%02X:%02X type/subtype %d/%d ",
	       sum.bsscfg_idx,
	       sum.BSSID.octet[0], sum.BSSID.octet[1], sum.BSSID.octet[2],
	       sum.BSSID.octet[3], sum.BSSID.octet[4], sum.BSSID.octet[5],
	       sum.type, sum.subtype);

	/* prec_count will be zero if the queues are empty */
	if (sum.prec_count == 0) {
		printf("(empty queue)\n");
		return;
	}

	/* Calc how much space remains after the fixed portion of the txq_summary struct.
	 * xtlv->len is the actual payload space in the xtlv. fixed_payload is the
	 * fixed part of txq_summary_t before the variable plen[].
	 */
	rem = xtlv->len - fixed_payload;
	if (rem < sum.prec_count * sizeof(sum.plen[0])) {
		printf("*ERR* too short for plen array, %d bytes, expected %d\n",
		       rem, (int)(sum.prec_count * sizeof(sum.plen[0])));
		return;
	}

	printf("qlen (%d queues): ", ltoh16(sum.prec_count));

	p = (uint16*)&xtlv->data[fixed_payload];

	for (i = 0; i < sum.prec_count; i++, p++) {
		uint16 plen = ltoh_ua(p);
		printf(" %u", plen);
	}
	printf("\n");
}

void
format_txstatus_log(bcm_xtlv_t *xtlv)
{
	int i;
	xtlv_uc_txs_t *log;
	uint fixed_payload = (XTLV_UCTXSTATUS_LEN - BCM_XTLV_HDR_SIZE);
	uint rem;
	uint wc;

	if (xtlv->len < fixed_payload) {
		printf("TxStatus History: ERR: xtlv too short, %d bytes, expected %d\n",
		       xtlv->len, fixed_payload);
		return;
	}

	log = (xtlv_uc_txs_t *)xtlv;

	/* Calc how much space remains after the fixed portion of the struct.
	 * xtlv->len is the actual payload space in the xtlv. fixed_payload is the
	 * fixed part of txq_summary_t before the variable w[].
	 */
	rem = xtlv->len - fixed_payload;
	/* total word count */
	wc = rem / 4;

	printf("\nTxStatus History: %u words per pkg, %u pkgs\n",
	       log->entry_size, wc / log->entry_size);

	if (wc == 0) {
		printf("<empty history>\n");
		return;
	}

	for (i = 0; i < wc; i++) {
		/* line header */
		if (i % log->entry_size == 0) {
			printf("%2u:", i / log->entry_size);
		}

		printf(" %08X", log->w[i]);

		/* newline after every pkg */
		if (i % log->entry_size == log->entry_size - 1) {
			printf("\n");
		}
	}

	/* if we did not end at a new line, print one */
	if (i % log->entry_size != 0) {
		printf("\n");
	}
}

void
process_logtrace(void)
{
	read_fmtfile();
	while (1) {
		uint32 ts, ord, w;
		uint32 fmt, args[128];
		int fmt_num, offset, i;
		char buff[2048];
		char *pos;

		if (fgets(buff, 2048, stdin) == NULL) {
			exit(0);	/* EOF */
		}
		if (errno != 0) {
			exit(errno);	/* Read error */
		}

		if ((pos = strstr(buff, "Logtrace ")) == 0) {
			continue;		/* Not one of ours */
		}

		if (sscanf(pos, "Logtrace %x timestamp %x %x %x%n",
		           &ord, &ts, &w, &fmt, &offset)) {
		     pos += offset;

		     w = ntohl(w);

			 /* Read all of the words in the buffer */
		     for (i = 0; i < w; i++) {
			     if (sscanf(pos, "%x%n", &args[i], &offset) == 0) {
				     printf("Truncated console output\n");
				     w = i - 1;
				     break;
			     }

				 pos += offset;
		     }

		     for (i = 0; i < w;) {
			     fmt_num = args[i] >> 2;	/* Entry is *4 */

			     if (fmt_num > num_fmts) {
				     printf("Invalid format num: 0x%x\n", args[i]);
				     break;
			     }

				 /* Just pass 16 args to cover all cases */
			     char *fmt = fmts[fmt_num];
			     printf(fmt,
			            args[i + 0], args[i + 1],
			            args[i + 2], args[i + 3],
			            args[i + 4], args[i + 5],
			            args[i + 6], args[i + 7],
			            args[i + 8], args[i + 9],
			            args[i + 10], args[i + 11],
			            args[i + 12], args[i + 13],
			            args[i + 14], args[i + 15]);
		     }
		}
	}	 /* End piped while */
}

void
read_dump(void)
{
	hnd_debug_t debugInfo;

	if (raw == 0) {
		int totCount = 0;
		/* The dlcmd dump command puts 20 Xs in the output to mark the
		 * start of the actual dump area.  We print out anything up to
		 * that since this could be run as a filter.
		 */
		int scanCount = 0;
		while (1) {
			char c;
			if ((totCount++ + 20) >= 1024) {
				fprintf(stderr, "Failed to find start of dump"
					" in first %i characters - resetting"
					" to RAW mode\n", 1024);
				raw = 1;
				break;
			}
			c = fgetc(dump);
			if (c == 'X') {
				scanCount++;
			} else {
				scanCount = 0;
			}

			if (scanCount == 20) {
				break;
			}
		}
	}

	if (raw == 0) {
		/* Found 20 X - now get the debug area info size */
		int debugSize = 0;
		int i, fRamSize;
		prstatus_t status;

		for (i = 0; i < 8; i++) {
			debugSize = (debugSize << 4) + (fgetc(dump) & 0x0f);
		}

		if (debugSize != sizeof(hnd_debug_t)) {
			fprintf(stderr, "%s: Debug size mismatch\n", progname);
			exit(-1);
		}

		/* Now read the actual debugInfo */
		fread(&debugInfo, sizeof(char), sizeof(debugInfo), dump);

		/* Read the prstatus area */
		fread(&status, sizeof(char), sizeof(status), dump);

		/* Parse the ramsize for sanity */
		fRamSize = 0;
		for (i = 0; i < 8; i++) {
			fRamSize = (fRamSize << 4) + (fgetc(dump) & 0x0f);
		}

	} else {
		/* Raw mode - have to find the debug area by hand */
		hnd_debug_ptr_t debugPtr;
		int32 dumpsize;
		uint i;

		fseek(dump, 0, SEEK_END);
		dumpsize = ftell(dump);
		fseek(dump, 0, SEEK_SET);
		for (i = 0; ; i++) {
			if (dump_info_ptr_ptr[i] == DUMP_INFO_PTR_PTR_END) {
				if (noinfo == 0) {
					fprintf(stderr, "%s: Debug area pointer "
					        "not found - assuming noinfo\n", progname);
				}
				bzero(&debugInfo, sizeof(debugInfo));
				noinfo = 1;
				break;
			}
			fseek(dump, dump_info_ptr_ptr[i], SEEK_SET);
			fread(&debugPtr, sizeof(char), sizeof(debugPtr), dump);
			if (debugPtr.magic == HND_DEBUG_PTR_PTR_MAGIC) {
				int32 seekoffset[2];
				int  i;

				/* for new firmware there is a value in: "debugPtr.ram_base_addr" */
				/* for old firmware there is no value there */
				seekoffset[0] = debugPtr.hnd_debug_addr - debugPtr.ram_base_addr;
				seekoffset[1] = debugPtr.hnd_debug_addr;
				for (i = 0; i < 2; i++) {
					if ((seekoffset[i] >= 0) && (seekoffset[i] < dumpsize)) {
						fseek(dump, seekoffset[i], SEEK_SET);
						fread(&debugInfo, sizeof(char), sizeof(debugInfo),
							dump);
						if ((debugInfo.magic == HND_DEBUG_MAGIC) &&
							(debugInfo.version == HND_DEBUG_VERSION))
						  break;
					}
				}
				break;
			}
		}
	}

	/* Sanity check the area */
	if ((noinfo == 0) && ((debugInfo.magic != HND_DEBUG_MAGIC) ||
	                      (debugInfo.version != HND_DEBUG_VERSION))) {
		fprintf(stderr, "%s: Error: Invalid debug info area\n", progname);
		noinfo = 1;
	}

	/* Get the memory bounds */
	ram_addr = debugInfo.ram_base;
	ram_size = debugInfo.ram_size;

	if (noinfo) {
		/* No size - use the dump file size */
		fseek(dump, 0, SEEK_END);
		ram_size = ftell(dump);
		rewind(dump);
	}

	fseek(dump, 0, SEEK_SET);

	shadow_ram = malloc(ram_size);
	if (shadow_ram == NULL) {
		fprintf(stderr, "%s Error: Shadow ram malloc failed\n", progname);
		exit(-1);
	}

	if (fread(shadow_ram, sizeof(char), ram_size, dump) != ram_size) {
		fprintf(stderr, "%s Error: Shadow ram read failed\n", progname);
		exit(-1);
	}

	/* Read in the event log top area */
	if (debugInfo.event_log_top == 0) {
		fprintf(stderr, "%s Error: Event log info not initialized in dump\n", progname);
		exit(-1);
	}

	dump_get(&top, sizeof(event_log_top_t), debugInfo.event_log_top);

	if ((top.magic != EVENT_LOG_TOP_MAGIC) || (top.version != EVENT_LOG_VERSION)) {
		fprintf(stderr, "%s Error: Event log top info invalid in dump\n", progname);
		exit(-1);
	}

	if ((top.num_sets == 0) || (top.sets == 0)) {
		printf("No event log sets active in this dump\n");
		exit(-1);
	}

	/* Get the pointers to the sets */
	dump_get(&setp, sizeof(uint32) * top.num_sets, top.sets);

	logstrs_size = top.logstrs_size;
}

/* register callback funciton for a specific event log Tag */
/* to process dump messages */
int register_eventlog_dump_processing_callback(char *output_file_name,
	uint16 tag,
	void (*callback_function)(eventdump_info_t*,
	uint32*, uint32, uint32))
{
char file_name[MAX_FILE_NAME_SIZE];

	if (callback_function == NULL) {
		fprintf(debugfile, "\ninvalid callback_function\n");
		return -1;
	}

	if ((tag == EVENT_LOG_TAG_RATE_CNT) || (tag == EVENT_LOG_TAG_CTL_MGT_CNT)) {
		eventdump_header[tag].dump_callback = callback_function;
		return 0;
	}

	/* sanity check */
	if (sizeof(output_file_name) > (MAX_FILE_NAME_SIZE - sizeof("statistic_.txt"))) {
		fprintf(debugfile, "\noutput_file_name too long size got: %d max size: %d\n",
			(int)(sizeof(output_file_name)),
			MAX_FILE_NAME_SIZE);
		return -1;
	}
	if (tag > EVENT_LOG_TAG_MAX) {
		fprintf(debugfile, "\ninvalid tag got: %d max tag: %d\n", tag, EVENT_LOG_TAG_MAX);
		return -1;
	}

	memcpy(eventdump_header[tag].dump_file_name,
		output_file_name,
		(int)(sizeof(output_file_name)));
	eventdump_header[tag].dump_callback = callback_function;

	/* open files for tag */
	sprintf(file_name, "%s%s%s", "statistic_", output_file_name, ".txt");
	fprintf(debugfile, "%s\n", file_name);
	eventdump_info[tag].statistic_file = fopen(file_name, "w+");
	if (eventdump_info[tag].statistic_file == NULL) {
		printf("can not create: %s\n", file_name);
		return -1;
	}
	sprintf(file_name, "%s%s%s", "log_", output_file_name, ".txt");
	fprintf(debugfile, "%s\n", file_name);
	eventdump_info[tag].log_file = fopen(file_name, "w+");
	if (eventdump_info[tag].log_file == NULL) {
		fprintf(debugfile, "can not create: %s\n", file_name);
		return -1;
	}

	if (tag == EVENT_LOG_TAG_TRACE_CHANSW) {
		memcpy(eventdump_header[EVENT_LOG_TAG_RATE_CNT].dump_file_name,
			output_file_name,
			(int)(sizeof(output_file_name)));
		memcpy(eventdump_header[EVENT_LOG_TAG_CTL_MGT_CNT].dump_file_name,
			output_file_name,
			(int)(sizeof(output_file_name)));
		eventdump_info[EVENT_LOG_TAG_RATE_CNT].statistic_file =
			eventdump_info[tag].statistic_file;
		eventdump_info[EVENT_LOG_TAG_CTL_MGT_CNT].statistic_file =
			eventdump_info[tag].statistic_file;
		eventdump_info[EVENT_LOG_TAG_RATE_CNT].log_file =
			eventdump_info[tag].log_file;
		eventdump_info[EVENT_LOG_TAG_CTL_MGT_CNT].log_file =
			eventdump_info[tag].log_file;
	}

	return 0;

}

int valid_event_log_header(event_log_hdr_t	*log_header)
{
int fmt_num;

	if (log_header->count == 0)
		return 0;

	if ((log_header->tag > EVENT_LOG_TAG_MAX) || log_header->tag == 0)
	{
		fprintf(stderr, "getting invalid event log tag: %d\n", log_header->tag);
		return 0;
	}

	fmt_num = log_header->fmt_num >> 2;

	if ((fmt_num > num_fmts) && (fmt_num != 0x3fff))
	{
		fprintf(stderr, "getting invalid event log fmt_num: %x\n", fmt_num);
		return 0;
	}

	return 1;
}

/* process host input file */
/* the input file consist of host packets */
/* each host packet is of the following format: */
/* host_packet_header_t follow by data in the size */
/* of host_packet_header->buf_size */
void process_input_file(FILE *fp)
{
int    file_size, input_data_size, index, input_data_index;
uint32 *input_data = NULL;
uint32 *reverse_buffer;
int i, k, offset;
host_packet_header_t *host_packet_header = NULL;
event_log_hdr_t	   *log_header;
uint32 *one_host_buffer;
int copy_index;


	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* allocate buffers */
	input_data = (uint32*)malloc(file_size);
	if (input_data == NULL) {
		fprintf(stderr, "Internal error: malloc failure for processing input file");
		return;
	}
	reverse_buffer = (uint32*)malloc(file_size);
	if (reverse_buffer == NULL) {
		fprintf(stderr, "Internal error: malloc failure for processing input file");
		return;
	}
	one_host_buffer = (uint32*)malloc(file_size);
	if (one_host_buffer == NULL) {
		fprintf(stderr, "Internal error: malloc failure for processing input file");
		return;
	}

	/* open input file */
	input_data_size = fread(input_data, 1, file_size, fp);

	input_data_index = 0;
	bool skip;

	/* process all the data in the input file */
	while (input_data_index < input_data_size / 4)
	{
		skip = TRUE;

		/* skip invalid data */
		/* every packet start with a magic number skip the anything */
		/* before the magic number calculate the distance to the next */
		/* magic number and skip host packets with wrong size */
		while (skip && (input_data_index < input_data_size / 4)) {
			while (input_data[input_data_index] != MAGIC_NUMBER)
				input_data_index++;
			host_packet_header = (host_packet_header_t*)(&input_data[input_data_index]);
			index = input_data_index + 1;
			while ((input_data[index] != MAGIC_NUMBER) && (index < input_data_size / 4))
				index++;
			fprintf(debugfile,
				"dhd buffer size: %d header size: %d index in input file:"
				"%d input file size: %d\n",
				index - input_data_index - 3,
				host_packet_header->buf_size / 4,
				input_data_index,
				input_data_size / 4);
			if ((index - input_data_index - 3) != (host_packet_header->buf_size / 4)) {
				fprintf(debugfile,
					"skipping dhd packet invalid size got: %d expected: %d\n",
					index - input_data_index - 3,
					host_packet_header->buf_size / 4);
				input_data_index = index;
			}
			else {
				skip = FALSE;
			}
		}

		if (input_data_index == input_data_size / 4)
			return;

		/* reverse one host packet content */
		index = input_data_index + host_packet_header->buf_size / 4;
		for (i = input_data_index, k = 0; i < index; i ++, k ++) {
			reverse_buffer[k] = input_data[index + 2 - k];
		}

		index = 0;

		log_header = (event_log_hdr_t*)(&reverse_buffer[index]);

		/* ignore zero's in the end of the buffer */
		while ((!valid_event_log_header(log_header)) &&
			(index <= (host_packet_header->buf_size - 1))) {
				index ++;
				log_header = (event_log_hdr_t*)(&reverse_buffer[index]);
		}

		copy_index = 0;

		/* the eventlog messages start from the end of the host buffer */
		/* arrange them in "one_host_buffer" */
		while (index < (host_packet_header->buf_size / 4 - 2))
		{
			log_header = (event_log_hdr_t*)(&reverse_buffer[index]);
			if (log_header->count > 0) {
				offset = input_data_index + host_packet_header->buf_size / 4 + 3 -
					index - log_header->count - 1;
				memcpy((uint8*)(&one_host_buffer[copy_index]),
					(uint8*)(&input_data[offset]),
					(log_header->count - 1) * 4);
				copy_index += (log_header->count - 1);
				memcpy((uint8*)(&one_host_buffer[copy_index]),
					(uint8*)(&reverse_buffer[index]), 8);
				copy_index += 2;
			}
			index += (log_header->count + 1);
		}

		/* kepp reverse copy of "one_host_buffer" in reverse_buffer */
		for (k = 1; k < copy_index; k ++) {
			reverse_buffer[k] = one_host_buffer[copy_index-k];
		}

		index = 0;

		/* process the each eventlog message in the host buffer */
		while (index < copy_index)
		{
			if (reverse_buffer[index] == 0) {
				index++;
				continue;
			}
			log_header = (event_log_hdr_t*)(&reverse_buffer[index+1]);

			offset = copy_index - index - log_header->count;

			if ((log_header->count-1) < (copy_index-index)) {
				process_event_log_data(&reverse_buffer[index],
						&one_host_buffer[offset]);
				index += (log_header->count+1);
			}
			else {
				fprintf(debugfile, "skipping invalid eventlog packet size got:"
					"%d size left in dhd buffer: %d \n",
					log_header->count, copy_index - index);
				index = copy_index;
			}
		}
		input_data_index += (host_packet_header->buf_size / 4 + 3);
	}

	return;
}

int
main(int argc, char *argv[])
{
	uint input_file = 0;
	FILE *input_file_handle = NULL;
	event_log_set_t sets[MAX_SETS];
	uint32 *logs[MAX_SETS];
	uint32 *last_ptr[MAX_SETS];
	int i;

	dump = stdin;

	/* Process args */
	progname = argv[0];

	argv++;

	debugfile = fopen("logprint_debugfile.txt", "w+");

	if (debugfile == NULL)
	{
		fprintf(stderr, "%s: Cannot create debug file - debugfile.txt\n",
				progname);
		exit(0);
	}

	while (*argv) {
		if (strcmp(*argv, "-help") == 0) {
			usage();

		} else if (strcmp(*argv, "-d") == 0) {
			argv++;
			dump = fopen(*argv, "r");
			if (dump == NULL) {
				fprintf(stderr, "%s: Cannot open dump file - %s\n",
					progname, *argv);
				usage();
			}
		} else if (strcmp(*argv, "-f") == 0) {
			input_file = 1;
			argv++;
			input_file_handle = fopen(*argv, "r");
			if (input_file_handle == NULL) {
				fprintf(stderr, "%s: Cannot open input file - %s\n",
					progname, *argv);
				usage();
			}
		} else if (strcmp(*argv, "-fmts") == 0) {
			struct stat fmtstat;
			argv++;

			fmtfile = fopen(*argv, "r");
			if (fmtfile == NULL) {
				fprintf(stderr, "%s: Cannot open format file - %s\n",
					progname, *argv);
				usage();
			}

			if (stat(*argv, &fmtstat) == 0) {
				logstrs_size = fmtstat.st_size;
			}

		} else if (strcmp(*argv, "-merge") == 0) {
			merge = 1;

		} else if (strcmp(*argv, "-set") == 0) {
			int this_set = strtoul(*++argv, NULL, 0);
			if (this_set > 32) {
				usage();
			}
			setmask |= 1 << this_set;

		} else if (strcmp(*argv, "-raw") == 0) {
			/* Raw mode assume input starts with coredump
			 * and not marker of 20 "X"
			*/
			raw = 1;

		} else if (strcmp(*argv, "-noinfo") == 0) {
			noinfo = 1;
		} else if (strcmp(*argv, "-reverse") == 0) {
			reverse = 1;

		} else if (strcmp(*argv, "-piped") == 0) {
			piped = 1;

		} else if (strcmp(*argv, "-logtrace") == 0) {
			logtrace = 1;

		} else if (strcmp(*argv, "-decode") == 0) {
			decode = 1;
		} else {
			usage();
		}
		argv++;
	}

	if (dump == NULL) {
		usage();
	}

	if (setmask == 0) {		/* Check for no sets specified */
		setmask = 0xffffffff;	/* Default is all sets */
	}
	if (decode) {
		decode_fmtfile();
		exit(0);
	}
	if (piped) {
		process_piped();
		exit(0);
	}

	if (logtrace) {
		process_logtrace();
		exit(0);
	}

	read_fmtfile();

	for (i = 0; i <= EVENT_LOG_TAG_MAX; i ++) {
		eventdump_info[i].first_dump_info = TRUE;
	}

	/* register callback functions for a specific eventlog tag */
	register_dump_processing_callbacks();

	if (input_file) {
		process_input_file(input_file_handle);
		fclose(input_file_handle);
		goto clean;
	}

	/* Dump file processing */
	read_dump();

	/* Read in all of the blocks and data for each set requested */
	for (set = 0; set < top.num_sets; set++) {
		/* Process each requested set that has blocks */
		int totsize = 0;

		uint32 log_offset[MAX_BLOCKS];
		int log_size[MAX_BLOCKS];

		int num_blocks, j;

		event_log_block_t block;
		uint32 block_ptr;
		char *bufptr;

		uint32 last_ts, last_cyclecount;

		if (setp[set] != 0) {
			dump_get(&sets[set], sizeof(event_log_set_t), setp[set]);
		}

		/* Remember the timestamp and cycle count at the end */
		last_cyclecount = sets[set].cyclecount;
		last_ts = sets[set].timestamp;

		logs[set] = NULL;	/* Safety */

		/* Make sure this set was requested */
		if ((setmask & (1 << set)) == 0) {
			continue;
		}

		if ((setp[set] == 0) || (sets[set].first_block == 0)) {
			setmask &= ~(1 << set);
			continue;	/* Empty set */
		}

		/* Compute the total size of all blocks */
		block_ptr = sets[set].cur_block;
		for (j = 0; j < MAX_BLOCKS; j++) {
			/* Read the block */
			dump_get(&block, sizeof(event_log_block_t), block_ptr);
			log_offset[j] = block_ptr + offsetof(event_log_block_t, event_logs);
			log_size[j] = block.end_ptr - log_offset[j];

			totsize += log_size[j];

			block_ptr = block.next_block;

			if (block_ptr == sets[set].cur_block) {
				break;
			}
		}

		if (j >= MAX_BLOCKS) {
			fprintf(stderr, "%s: Internal error: Invalid block "
				"chain in dump for set %d\n", progname, set);
			exit(-1);
		}

		num_blocks = j + 1;

		/* Allocate a contiguous chunk */
		logs[set] = malloc(totsize);
		if (logs[set] == NULL) {
			fprintf(stderr, "%s: Internal error: malloc failure "
				"for logs for set %d\n", progname, set);
		}

		bufptr = (char *) logs[set];

		/* The buffer is filled in order so we start
		 * at the current pointer in the current block
		 * and wrap back
		 */
		dump_get(bufptr,
			(log_offset[0] + log_size[0]) - sets[set].cur_ptr,
			sets[set].cur_ptr);
		bufptr += (log_offset[0] + log_size[0]) - sets[set].cur_ptr;

		for (j = 1; j < num_blocks; j++) {
			dump_get(bufptr, log_size[j], log_offset[j]);
			bufptr += log_size[j];
		}

		/* Now pick up the last piece of curblock  if any */
		if (sets[set].cur_ptr != log_offset[0]) {
			dump_get(bufptr, sets[set].cur_ptr - log_offset[0],
				log_offset[0]);
			bufptr += sets[set].cur_ptr - log_offset[0];
		}

		last_ptr[set] = ((uint32 *) bufptr);

		/* Now reverse the pointers if necessary */
		uint32 *log_ptr = last_ptr[set];

		while (--log_ptr >= logs[set]) {
			int i;
			event_log_hdr_t hdr;
			uint32 data_save[256];
			uint32 cycles;

			/* Get the header out of the way and copy */
			hdr.t = *log_ptr;

			/* Check for partially overriten entries */
			if ((log_ptr - logs[set]) < (hdr.count)) {
				/* Clear the rest */
				do {
					*log_ptr = 0;
				} while (log_ptr-- != logs[set]);
				break;
			}

			if (hdr.tag == EVENT_LOG_TAG_NULL) {
				continue;
			}

			cycles = *(log_ptr - 1);
			log_ptr -= hdr.count;

			/* Put the header and timestamp first if we
			 * are not reversing
			 */
			if (reverse == 0) {
				/* Save the data coying the timestamp first */
				for (i = 0; i < (hdr.count - 1); i++) {
					data_save[i] = log_ptr[i];
				}

				/* Now place the header at the front
				 * and copy back.
				 */
				log_ptr[0] = hdr.t;
				log_ptr[1] = cycles;
				for (i = 0; i < (hdr.count - 1); i++) {
					log_ptr[i+2] = data_save[i];
				}

				if (hdr.tag == EVENT_LOG_TAG_TS) {
					/*  Percolate the timestamps down the
					 *  stack
					 */
					uint32 next_ts = log_ptr[2];
					uint32 next_cycles = log_ptr[3];
					log_ptr[3]  = last_cyclecount;
					last_cyclecount = next_cycles;
					log_ptr[2] = last_ts;
					last_ts = next_ts;
				}
			}
		}

		if (reverse == 0) {
			/* Percolate the TS to the top */
			sets[set].cyclecount = last_cyclecount;
			sets[set].timestamp = last_ts;
		}
	}

	/* Ready to print */
	if (merge == 0) {
		int set;

		for (set = 0; set < top.num_sets; set++) {
			uint32 cur_timestamp, cur_cycles;

			if ((setmask & (1 << set)) == 0) {
				continue;
			}

			prev_ts = 0;		/* Init for each set */
			cur_timestamp = sets[set].timestamp;
			cur_cycles = sets[set].cyclecount;

			printf("\n\nTime(ms) Set Cycles    Log entries for set %d\n\n", set);

			/* Walk through log */
			if (reverse) {
				uint32 *log_ptr = last_ptr[set];
				while (--log_ptr > logs[set]) {
					event_log_hdr_t hdr;

					reverse_print(log_ptr, cur_timestamp,
						cur_cycles, set);

					hdr.t = *log_ptr;
					if (hdr.tag == EVENT_LOG_TAG_TS) {
						/* Update cur info */
						cur_cycles = log_ptr[-2];
						cur_timestamp = log_ptr[-3];
					}

					log_ptr -= hdr.count;

				}

			} else {
				/* Print in normal order */
				uint32 *log_ptr = logs[set];
				while (log_ptr != last_ptr[set]) {
					event_log_hdr_t hdr;
					hdr.t = *log_ptr;
					log_print(log_ptr, cur_timestamp,
						cur_cycles, set);

					log_ptr += hdr.count + 1;

					hdr.t = *log_ptr;
					if (hdr.tag == EVENT_LOG_TAG_TS) {
						/* Update cur info */
						cur_timestamp = log_ptr[2];
						cur_cycles = log_ptr[3];
					}
				}
			}

		}

	} else {
		/* Merge */
		int set;
		int active_mask = setmask;
		uint32 *log_ptr[MAX_SETS];
		uint32 ts[MAX_SETS];
		uint32 cycles[MAX_SETS];
		uint32 base_cycles[MAX_SETS];

		printf("\n\nTime(ms) Set  Cycles     Log entries for sets");
		/* Init the pointers and the headers */
		for (set = 0; set < top.num_sets; set++) {
			if (active_mask & (1 << set)) {
				printf(" %d", set);
				ts[set] = sets[set].timestamp;
				base_cycles[set] = sets[set].cyclecount;
				if (reverse) {
					log_ptr[set] = next_entry(last_ptr[set],
						logs[set]);
				} else {
					log_ptr[set] = next_entry(logs[set],
						last_ptr[set]);
				}

				if (log_ptr[set] == NULL) {
					active_mask &= ~(1 << set);
				} else if (reverse) {
					cycles[set] = log_ptr[set][-1];
				} else {
					cycles[set] = log_ptr[set][1];
				}
			}
		}

		printf("\n\n");

		while (1) {
			/* Find the next highest to print */
			int best_set =  -1;
			event_log_hdr_t hdr;

			for (set = 0; set < top.num_sets; set++) {
				if ((active_mask & (1 << set)) == 0) {
					continue;
				}

				/* See if this is best so far */
				if (best_set == -1) {
					best_set = set;
				} else if (reverse) {
					if ((ts[best_set] < ts[set]) ||
					    ((ts[best_set] == ts[set]) &&
					     cycles[best_set] < cycles[set])) {
						best_set = set;
					}

				} else if ((ts[best_set] > ts[set]) ||
					((ts[best_set] == ts[set]) &&
					cycles[best_set] > cycles[set])) {
						best_set = set;
				}
			}

			if (best_set == -1) {
				break;		/* All done */
			}

			/* Print and move on to the next entry */
			if (reverse) {
				reverse_print(log_ptr[best_set], ts[best_set],
					base_cycles[best_set], best_set);

				hdr.t = *log_ptr[best_set];
				if (hdr.tag == EVENT_LOG_TAG_TS) {
					/* Update the timestamp */
					ts[best_set] =
						log_ptr[best_set][-3];
					base_cycles[best_set] =
						log_ptr[best_set][-2];
				}

				log_ptr[best_set] -= hdr.count + 1;
				log_ptr[best_set] =
					next_entry(log_ptr[best_set],
					logs[best_set]);

			} else {
				log_print(log_ptr[best_set], ts[best_set],
					base_cycles[best_set], best_set);

				hdr.t = *log_ptr[best_set];
				log_ptr[best_set] += hdr.count + 1;
				log_ptr[best_set] =
					next_entry(log_ptr[best_set],
					last_ptr[best_set]);
			}

			if (log_ptr[best_set] == NULL) {
				/* Done with this set */
				active_mask &= ~(1 << best_set);
			} else if (reverse) {
				cycles[best_set] = log_ptr[best_set][-1];
			} else {
				cycles[best_set] = log_ptr[best_set][1];
				hdr.t = *log_ptr[best_set];
				if (hdr.tag == EVENT_LOG_TAG_TS) {
					/* Update the timestamp */
					ts[best_set] =
						log_ptr[best_set][2];
					base_cycles[best_set] =
						log_ptr[best_set][3];
				}
			}

		}
	}

clean:

	/* close all the output files we open */
	for (i = 0; i <= EVENT_LOG_TAG_MAX; i ++) {
		if ((i != EVENT_LOG_TAG_RATE_CNT) && (i != EVENT_LOG_TAG_CTL_MGT_CNT)) {
			if (eventdump_info[i].statistic_file != 0)
				fclose(eventdump_info[i].statistic_file);
			if (eventdump_info[i].log_file != 0)
				fclose(eventdump_info[i].log_file);
		}
	}

	/* Cleanup */
	for (set = 0; set < top.num_sets; set++) {
		if (logs[set]) {
			free(logs[set]);
		}
	}

	if (fmts) {
		free(fmts);
	}

	free(shadow_ram);

	return 0;
}
