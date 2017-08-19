
#include <linux/kernel.h>

#include <typedefs.h>

#include "debugger.h"
#include "debugger_arm.h"

/************************************************************************/
/* BUFMAX defines the maximum number of characters in inbound/outbound buffers*/
/* at least NUMREGBYTES*2 are needed for register packets */
#define BUFMAX 2048

static const char hexchars[]="0123456789abcdef";

#define NUMREGS 72

/* Number of bytes of registers.  */
#define NUMREGBYTES (NUMREGS * 4)

void *g_io_handle;

/* Convert ch from a hex digit to an int */

static int hex (unsigned char ch)
{
	if (ch >= 'a' && ch <= 'f')
		return ch-'a'+10;
	if (ch >= '0' && ch <= '9')
		return ch-'0';
	if (ch >= 'A' && ch <= 'F')
		return ch-'A'+10;
	return -1;
}

static char remcomInBuffer[BUFMAX];
static char remcomOutBuffer[BUFMAX];

/* scan for the sequence $<data>#<checksum>     */

unsigned char * getpacket(void) {
	unsigned char *buffer = &remcomInBuffer[0];
	unsigned char checksum;
	unsigned char xmitcsum;
	int count;
	char ch;

	while (1)
	{
		/* wait around for the start character, ignore all other characters */
		while ((ch = FGETC(g_io_handle)) != '$')
			;

retry:
		checksum = 0;
		xmitcsum = -1;
		count = 0;

		/* now, read until a # or end of buffer is found */
		while (count < BUFMAX)
		{
			ch = FGETC(g_io_handle);
			if (ch == '$')
				goto retry;
			if (ch == '#')
				break;
			checksum = checksum + ch;
			buffer[count] = ch;
			count = count + 1;
		}
		buffer[count] = 0;

		if (ch == '#')
		{
			ch = FGETC(g_io_handle);
			xmitcsum = hex (ch) << 4;
			ch = FGETC(g_io_handle);
			xmitcsum += hex (ch);

			if (checksum != xmitcsum)
			{
				FPUTC('-', g_io_handle);	/* failed checksum */
			}
			else
			{
				FPUTC('+', g_io_handle);	/* successful transfer */

				/* if a sequence char is present, reply the sequence ID */
				if (buffer[2] == ':')
				{
					FPUTC(buffer[0], g_io_handle);
					FPUTC(buffer[1], g_io_handle);

					return &buffer[3];
				}

				return &buffer[0];
			}
		}
	}
}
/* send the packet in buffer.  */

	static void
putpacket (unsigned char *buffer) {
	unsigned char checksum;
	int count;
	unsigned char ch;

	/*  $<packet info>#<checksum>. */
	do
	{
		FPUTC('$', g_io_handle);
		checksum = 0;
		count = 0;

		while ((ch = buffer[count]))
		{
			FPUTC(ch, g_io_handle);
			checksum += ch;
			count += 1;
		}

		FPUTC('#', g_io_handle);
		FPUTC(hexchars[checksum >> 4], g_io_handle);
		FPUTC(hexchars[checksum & 0xf], g_io_handle);

	}
	while (FGETC(g_io_handle) != '+');
}

/* Indicate to caller of mem2hex or hex2mem that there has been an
   error.  */
static volatile int mem_err = 0;

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null), in case of mem fault,
 * return 0.
 * If MAY_FAULT is non-zero, then we will handle memory faults by returning
 * a 0, else treat a fault like any other fault in the stub.
 */

extern int dhd_readbytes(int addr, unsigned char *buf, int size);

static unsigned char *
mem2hex (unsigned char *mem, unsigned char *buf, int count, int may_fault) {
	int i;
	uint32_t *ip;
	unsigned char ch;
	int addr = (int) mem;
	unsigned char b[1000];

	if (count != 2 && count != 4) {
		ip = (uint32_t *) &b;

		printk("*** mem2hex count = %d\n", count);
		
		for (i = 0; i < count; i += 4)
			*ip++ = dbg_read32(addr + i);
	} else {
		if (count == 2)
			*((uint16_t *) &b) = dbg_read16(addr);
		else
			*((uint32_t *) &b) = dbg_read32(addr);
	}

	for( i = 0; i < count; i++ ) {
		ch = b[i];
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}

	*buf = 0;

	return buf;
}


/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */

static int hexToInt(char **ptr, int *intValue)
{
	int numChars = 0;
	int hexValue;

	*intValue = 0;

	while (**ptr)
	{
		hexValue = hex(**ptr);
		if (hexValue < 0)
			break;

		*intValue = (*intValue << 4) | hexValue;
		numChars ++;

		(*ptr)++;
	}

	return (numChars);
}


int gdb_read_registers(int length, char *buffer)
{
	int temp;
	int realsize = 0;
	uint32_t padding = 0;
	regs_type arm_regs;

	dbg_read_arm_regs(&arm_regs);

	printk("Registers:\n");
	for (temp = 0; temp < 8; temp++)
		printk("  R%2.2d - 0x%8.8x   R%2.2d - 0x%8.8x\n",
				temp, arm_regs.r[temp],
				temp + 8, arm_regs.r[temp + 8]);

	printk("\n  PC  - 0x%8.8x\n", arm_regs.pc);

	/* r0..r15 (32 bits) */
	for (temp = 0; temp <= 15; temp++)
		realsize += sprintf(buffer+realsize, "%8.8x", htonl(arm_regs.r[temp]));
	/* f0..f7 (4*32 bits) */
	for (temp = 0; temp <= 7; temp++)
		realsize += sprintf(buffer + realsize, "%8.8x%8.8x%8.8x", padding,
				padding, padding);
	/* fps (32 bits) */
	realsize += sprintf(buffer+realsize, "%8.8X", padding);
	/* cpsr (32 bits) */
	realsize += sprintf(buffer+realsize, "%8.8X", htonl(arm_regs.cpsr));

	/* printk("Returning %d Bytes for Registers\n", realsize); */
	return realsize;
}


void debugger_gdb_main(void *handle) {
	int sigval = 0;
	char *ptr;
	int bpt;
	int addr;
	int length;

	g_io_handle = handle;

	while (1)
	{
		remcomOutBuffer[0] = 0;

		ptr = getpacket();
		printk("command %c\n", *ptr);

		switch (*ptr++)
		{
			case 'q': 
				/* Query - usually called at startup */
				/* add init hooks here */
				dbg_init();
				dbg_halt();

				break;

			case 'c':
			       	/* continue */
				dbg_run();

				/* spin until breakpoint */

				while (dbg_get_arm_status() == ARM_RUNNING)
					dbg_mdelay(100);

				strcpy(remcomOutBuffer, "S05");
				break;

			case 's':
				/* step */
				dbg_ss();
				strcpy(remcomOutBuffer, "S05");
				break;

			case 'z':
				/* clear breakpoint */
				sscanf(ptr, "%d,%d,%d", &bpt, &addr, &length);
				printk("Clear bpt %s %d %d %d\n", ptr, bpt, addr, length);

				dbg_cbrk(bpt);

				strcpy(remcomOutBuffer, "OK");
				break;

			case 'Z': 
				/* set breakpoint */
				sscanf(ptr, "%d,%x,%d", &bpt, &addr, &length);
				printk("Set bpt %s %d %x %d\n", ptr, bpt, addr, length);

				dbg_brk(bpt, addr, 1, 0);

				strcpy(remcomOutBuffer, "OK");
				break;

			case '?':
				remcomOutBuffer[0] = 'S';
				remcomOutBuffer[1] = hexchars[sigval >> 4];
				remcomOutBuffer[2] = hexchars[sigval & 0xf];
				remcomOutBuffer[3] = 0;
				break;

			case 'd':		/* toggle debug flag */
				break;

			case 'g':
				gdb_read_registers( sizeof(remcomOutBuffer), remcomOutBuffer);
				break;

			case 'm':
				/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

				if (hexToInt(&ptr, &addr)
						&& *ptr++ == ','
						&& hexToInt(&ptr, &length))
				{
					printk("\taddr 0x%x, len 0x%x\n", addr, length);
					if (mem2hex((char *)addr, remcomOutBuffer, length, 1))
						break;

					strcpy (remcomOutBuffer, "E03");
				}
				else
					strcpy(remcomOutBuffer,"E01");
				break;

			case 'k':
				return;
				break;


		}			/* switch */

		/* reply to the request */
		putpacket(remcomOutBuffer);
	}
	g_io_handle = NULL;
}
