/*
 *  Copyright (C) 2004 Lars Kristian Klauske
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* GDB remote server */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef JTAG
#include "jtag.h"
#include "ice.h"
#include "lpec.h"
#endif

/* network functions */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/poll.h>
#include <unistd.h>
#include <errno.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <assert.h>
#include "debug.h"
#include "hnd_armtrap.h"

/* forward decls */
int gdb_read_registers(int length, char *buffer);
int gdb_write_registers(char *buffer);
int gdb_remove_breakpoint(int length, char *buffer);
int gdb_insert_breakpoint(int length, char *buffer);

uint32 baseaddress = 0;
#ifndef TARGET_HOST

uint32 tmp_type_offset = 0;
regs_type arm_regs = {
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, }, 	/* r0 --> r11 */
	12,								/* ip, r12 */
	13,								/* sp, r13 */
	14,								/* lr, r14 */
	0x2ab44,								/* pc, r15 */
	{ {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, },
	0,								/* fps */
	0xaa55aa55,								/* cpsr */
};


#endif /* TARGET_HOST */

#ifndef TARGET_HOST
int symfd;
void memory_read(uint32 address, char *data, uint32 length)
{
	int rc;

	if (address > baseaddress) 
		address -= baseaddress;
	printf("%s: address 0x%x length 0x%x, base 0x%04x\n", __FUNCTION__, address, length, baseaddress);

	rc = lseek(symfd, address, SEEK_SET);
	if (rc == -1) {
		printf("%s: file lseek error: address 0x%x\n", __FUNCTION__, address);
		goto done;
	}

	rc = read(symfd, data, length);
	if (rc != length) {
		printf("%s: file io error: read returned 0x%x requested 0x%x\n",
			__FUNCTION__, rc, length);
		goto done;
	}


done:
	return;
}
#endif /* TARGET_HOST */

#define GDB_ANSWER_OK	"OK"
#define GDB_ANSWER_S05	"S05"	/* breakpoint happened */
#define GDB_ANSWER_W07	"W07"	/* breakpoint happened */
#define GDB_ANSWER_W02	"W02"	/* interrupt occured */
#define GDB_ANSWER_S03	"S03"	/* interrupt occured */
#define GDB_ANSWER_W00	"W00"	/* process stopped */
#define GDB_ANSWER_S17	"S17"	/* process stopped */
#define GDB_ANSWER_E01	"E01"	/* error */
#define GDB_ANSWER_E02	"E02"	/* error */

uint32 SW_BP_PATTERN = 0xe7ffdefe;

/* implemented GDB commands:
D				detach
g				read registers
G<XX...>			write registers
m<addr>,<length>		read memory
M<addr>,<length>:<XX...>	write memory
c<addr>				continue
s<addr>				step
*/

int client_socket;
uint32 buffersize = 10000;

int last_running = 1;

#define sw_bp_max 99
uint32 sw_bp_addr[sw_bp_max+1];
uint32 sw_bp_memdata[sw_bp_max+1];

void execute_command(uint32 length, char *cmd);

void gdb_write_memory(uint32 addr, uint32 length, char *buffer);
void gdb_step(void);

char HexByte_to_Value(char input) {
	/* is it a number? */
	if ((input >= 0x30) && (input <= 0x39))
	return (input-0x30);

	/* or is it a lowercase */
	if ((input >= 0x61) && (input <= 0x66))
	return (input-0x57);

	/* or is it an uppercase */
	if ((input >= 0x41) && (input <= 0x46))
	return (input-0x37);

	return 0;
}


int Calculate_Checksum(uint32 length, char *input) {
	int ret = 0;
	/* int i; */

	for (; length > 0; length--) {
	ret += input[length-1];
	ret = ret & 0xff;
	}

	return ret;
}

uint32 HexString_to_Long(char *input) {
#define longsize 8
	uint32 ret;
	ret = (uint32)strtoul(input, NULL, 16);
	/* printf("Converting: %s  - Result: %8.8x\n",input, ret); */
	return ret;
}

/* unsigned long HexString_to_ULong (char *input) {
#define longsize 8
	unsigned long ret;
	ret = strtoul(input,NULL,16);
	printf("Converting: %s  - Result: %8.8x\n",input, ret);
	return ret;
	}
*/


void answer_gdb(uint32 length, char *text) {
	int checksum = Calculate_Checksum(length, text);
	char cs_string[5];
	sprintf(cs_string, "%2.2x", checksum);

	if ((text[0] == '+') || (text[0] == '-')) {
	/* printf ("Sending: %c\n",text[0]); */
	write(client_socket, text, 1);
	} else {
	printf("Sending: $%s#%2.2x\n", text, checksum);
	write(client_socket, "$", 1);
	if (length) write(client_socket, text, length);
	write(client_socket, "#", 1);
	write(client_socket, cs_string, 2);
	}

	return;
}

unsigned char MemoryLongData[8192];
unsigned char MemoryData[16400];

#define DEFPORT 8888

void do_gdb(unsigned short eport)
{
	int socket_desc;
	struct sockaddr_in address;
	unsigned int addrlen;
	struct pollfd ufds = { 0, POLLIN+POLLPRI, 0 };
	char buffer[buffersize];
	uint32 bufferfill = 0;
	char byte;
	int state = 0;
	int checksum;
	unsigned short lport;

	for (checksum = 0; checksum <= sw_bp_max; checksum++) {
		sw_bp_memdata[checksum] = SW_BP_PATTERN;
		sw_bp_addr[checksum] = 0;
	}

	/* create the master socket and check it worked */
	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		/* if socket failed then display error and exit */
		perror("Create socket");
		exit(EXIT_FAILURE);
	}

	/* type of socket created */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;

	/* if no specific port requested, allow a range */
	if (eport) {
		lport = eport;
	} else {
		lport = DEFPORT;
		eport = lport + 20;
	}
	
	for (; lport <= eport; lport++) {
		address.sin_port = htons(lport);

		if (bind(socket_desc, (struct sockaddr *)&address, sizeof(address)) < 0)
		{
			if (errno == EADDRINUSE)
				continue;

			/* hard failure: display error message and exit */
			perror("bind");
			exit(EXIT_FAILURE);
		}

		break;
	}

	if (lport > eport) {
		printf("Local port unavailable\n");
		exit(EXIT_FAILURE);
	}

	/* accept connection, wait if no connection pending */
	addrlen = sizeof(address);
	listen(socket_desc, 1);
	printf("Listening for Connection on port %d...\n", lport);
	client_socket = accept(socket_desc, (struct sockaddr *)&address, &addrlen);

	/* inform user of socket number - used in send and receive commands */
	printf("Client socket is %d\n", client_socket);
	state = 0;
	ufds.fd = client_socket;

#ifdef TARGET_HOST
	/* stopping target for gdb */
	debug_stop();
#endif
#ifdef JTAG
	ice_set_software_breakpoint(0, SW_BP_PATTERN);
	lpec_init();
#endif
	last_running = 0;

	while (client_socket) {
		int retval;
		int stat;
		fd_set rfds;

		FD_ZERO(&rfds);
		FD_SET(client_socket, &rfds);
	retval = select((client_socket + 1), &rfds, NULL, NULL, NULL);
	if (retval == -1 || retval == 0) {
		printf("Connection terminated\n");
		break;
	}

	stat = read(client_socket, &byte, 1);
		if (stat <= 0) {
			printf("No data or read error\n");
			break;
		}

	printf("%c", byte);
	fflush(stdout);

	    switch (state) {
		case 0:
		    if (byte == '$') {
			bufferfill = 0;
			state = 1;
		    }
		    break;
		case 1:
		    if (byte != '#') {
			if (bufferfill < buffersize-1) {
			    buffer[bufferfill] = byte;

			    /*if (bufferfill == 0) {
			      switch (buffer[0]) {
			      case 'M':
			      state = 0;
			      execute_command(bufferfill, buffer);
			      break;
			      default:
			      break;
			      }
			      }
				 */
			    bufferfill++;

		} else {
			bufferfill = 0;
			printf("Error! Buffer overflow while reading from Socket.\n");
			answer_gdb(1, "-");
			state = 0;
		}
		} else {
		buffer[bufferfill] = '\0';
		state = 2;
		}
		break;
	case 2:
		checksum = HexByte_to_Value(byte);
		checksum *= 16;
		state = 3;
		break;
	case 3:
		checksum += HexByte_to_Value(byte);
		printf("\nCommand: %s", buffer);
		printf(" - Checksum IN:%2.2x, CA:%2.2x\n",
			checksum, Calculate_Checksum(bufferfill, buffer));
		if (checksum == Calculate_Checksum(bufferfill, buffer)) {
		answer_gdb(1, "+");
		/* printf("Answered: Checksum Ok\n"); */
		/* printf("Doing Work: %s\n", buffer); */
		execute_command(bufferfill, buffer);
		} else {
		answer_gdb(1, "-");
		printf("Answered: Checksum Mismatch\n");
		}
		state = 0;
		break;
	default:
		break;

	    }

	    fflush(stdout);

	}

	/* shutdown master socket properly */
	close(socket_desc);
	return;
}


void execute_command(uint32 length, char *cmd) {
	int retsize = 0;
	uint32 MemoryAddress;
	uint32 MemoryLength;
	uint32 tempL = 0;

	switch (cmd[0]) {
	case 'H':	/* set thread */
	    answer_gdb(2, "OK");
	    printf("(set thread)\n");
	    break;

	case 'g':	/* read registers */
	    retsize = gdb_read_registers(buffersize, cmd);
	    answer_gdb(retsize, cmd);
	    printf("(read registers)\n");
	    break;

	case 'G':	/* write registers */
	    /* answer_gdb(0, "\0"); */
	    retsize = gdb_write_registers(cmd);
	    answer_gdb(2, "OK");
	    printf("(write registers)\n");
	    break;

	case 'P':	/* write a particular register */
	    answer_gdb(0, "\0");
	    printf("(write a particular register)\n");
	    break;

	case 'm':	/* read memory */
		{
		uint32 MemoryTemp = 0;

	    for (tempL = 1; ((tempL <= 8) && (cmd[tempL] != ',')); tempL++);
	    cmd[tempL] = '\0';
	    MemoryAddress = HexString_to_Long(cmd + 1);
	    MemoryLength = HexString_to_Long(cmd + tempL + 1);

		assert(MemoryLength <= sizeof(MemoryLongData));

		printf("MemoryAddress 0x%x MemoryLength 0x%x\n", MemoryAddress, MemoryLength);
	    memory_read(MemoryAddress, MemoryLongData, MemoryLength);

	    for (tempL = 0; tempL < MemoryLength; tempL++) {
		MemoryTemp = tempL * 2;
		sprintf(MemoryData + MemoryTemp, "%2.2x", MemoryLongData[tempL]);
		}

	    answer_gdb(MemoryLength * 2, MemoryData);
	    printf("(read memory)\n");

		}
	    break;

	case 'M':	/* write memory */
		cmd ++;

	    /* read address */
	    for (tempL = 0; ((tempL < 8) && (cmd[tempL] != ',')); tempL++);
	    cmd[tempL] = '\0';
	    MemoryAddress = HexString_to_Long(cmd);
	    cmd += tempL + 1;

	    /* read length */
	    for (tempL = 0; ((tempL < 8) && (cmd[tempL] != ':')); tempL++);
	    cmd[tempL] = '\0';
	    MemoryLength = HexString_to_Long(cmd);
	    cmd += tempL+1;

	    gdb_write_memory(MemoryAddress, MemoryLength, cmd);

	    answer_gdb(2, "OK");
	    printf("(write memory)\n");
	    break;

	case 'X':	/* write memory, binary data */
	    answer_gdb(0, "\0");
	    printf("(write memory, binary data)\n");
	    break;

	case 'z':	/* delete breakpoint */
	    printf("(delete breakpoint)\n");
	    retsize = gdb_remove_breakpoint(buffersize, cmd+1);
	    if (retsize) {
		answer_gdb(0, "\0");
	    } else {
		answer_gdb(2, "OK");
	    }
	    break;

	case 'Z':	/* set breakpoint */
	    printf("(set breakpoint)\n");
	    retsize = gdb_insert_breakpoint(buffersize, cmd+1);
	    if (retsize) {
		answer_gdb(0, "\0");
	    } else {
		answer_gdb(2, "OK");
	    }
	    break;

	case 'c':	/* continue */
#ifdef TARGET_HOST
	    debug_run();
	    /* answer_gdb(3, "S05"); */
	    /* answer_gdb(2, "OK"); */
	    last_running = 1;
#endif
	    printf("(continue)\n");
	    break;

	case 'C':	/* continue with signal */
	    answer_gdb(0, "\0");
	    printf("(continue with signal)\n");
	    break;

	case 's':	/* step */
	    gdb_step();
	    answer_gdb(3, "S05");
	    printf("(step)\n");
	    break;

	case 'S':	/* step with signal */
	    answer_gdb(0, "\0");
	    printf("(step with signal)\n");
	    break;

	case '?':	/* query last signal */
	    answer_gdb(3, "S05");
	    printf("(query last signal)\n");
	    break;

	case 'D':	/* detach */
	    printf("(detach)\n");
	    printf("Exiting\n");
	    close(client_socket);
	    client_socket = 0;
#ifdef TARGET_HOST
	    debug_run();
#endif
	    break;

	case 'T':	/* query is thread alive */
	    answer_gdb(0, "\0");
	    printf("(query is thread alive)\n");
	    break;

	case 'R':	/* restart the remote server */
	    answer_gdb(0, "\0");
	    printf("(restart the remote server)\n");
	    break;

	case '!':	/* use extended ops */
	    answer_gdb(0, "\0");
	    printf("(use extended ops)\n");
	    break;

	case 'k':	/* kill target */
	    answer_gdb(0, "\0");
	    printf("(kill target)\n");
	    break;

	case 'd':	/* toggle debug */
	    answer_gdb(0, "\0");
	    printf("(toggle debug)\n");
	    break;

	case 'r':	/* reset */
	    answer_gdb(0, "\0");
	    printf("(reset)\n");
	    break;

	case 't':	/* search memory */
	    answer_gdb(0, "\0");
	    printf("(search memory)\n");
	    break;

	case 'q':	/* general query */
	    answer_gdb(0, "\0");
	    printf("(general query)\n");
	    break;

	case 'Q':	/* general set */
	    answer_gdb(0, "\0");
	    printf("(general set)\n");
	    break;

	default:
	    answer_gdb(0, "\0");
	    printf("(unknown command %c)\n", cmd[0]);

	}

	printf("\n");

}

int gdb_insert_breakpoint(int length, char *buffer)
{
	char type;
	uint32 addr, range, mask;
	int idx = 0;
	int ret = 0;

	switch (type) {

	case '0':	/* set software breakpoint */
		printf("Setze SW Breakpoint an %8.8x, Bereich %8.8x, Maske %8.8x\n",
		addr, range, mask);
	    idx = 0;
#ifdef TARGET_HOST
	    while (idx <= sw_bp_max && sw_bp_memdata[idx] != SW_BP_PATTERN) {
		if (sw_bp_addr[idx] == addr) {
		    printf("Breakpoint already set!\n");
		    return -1;
		}
		idx++;
	    }
	    if (idx < sw_bp_max) {
		memory_read(addr, &sw_bp_memdata[idx], 1);
		sw_bp_addr[idx] = addr;
		memory_write(addr, &SW_BP_PATTERN, 1);
	    } else {
	        printf("Too many SW BPs !!!\n");
	        ret = -1;
	    }
#endif /* TARGET_HOST */
	    break;

	case '1':	/* set hardware breakpoint */
		printf(
		"Setze HW Breakpoint an %8.8x , Bereich %8.8x , Maske %8.8x\n",
		addr, range, mask);
	    printf("Not tested yet!\n");
#ifdef JTAG
	    if (!ice_wp_enabled(1)) {
		ice_set_hardware_breakpoint(1, addr, mask);
	    } else {
		printf("Too many HW BPs !!!\n");
		ret = -1;
	    }
#endif
	    break;

	case '2':	/* set watchpoint */
		printf(
		"Setze Watchpoint an %8.8x, Bereich %8.8x , Maske %8.8x\n",
		addr, range, mask);
	    printf("Not tested yet!\n");
#ifdef JTAG
	    if (!ice_wp_enabled(1)) {
		ice_set_hardware_watchpoint(1, addr, mask);
	    } else {
		printf("Too many HW WPs !!!\n", addr);
		ret = -1;
	    }
#endif
	    break;

	default:

		printf(
		"Unbekanntes BP-Kommando: %c , Addresse: %8.8x , Bereich %8.8x , Maske %8.8x\n",
		type, addr, range, mask);
	    ret = -1;
	    break;
	}

	return ret;

}

int gdb_remove_breakpoint(int length, char *buffer)
{
	char type;
	uint32 addr, range, mask;
	int idx = 0;
	int ret = 0;

	/* read command from buffer */
	type = buffer[0];
	buffer += 2;

	while (idx < 8 && buffer[idx] != ',')
		idx++;
	buffer[idx] = '\0';
	addr = HexString_to_Long(buffer);

	buffer += idx + 1;
	range = HexString_to_Long(buffer);
	mask = range - 1;

	/* now do something with it */
	switch (type) {

	case '0':	/* remove software breakpoint */
	    printf("Entferne SW Breakpoint an %8.8x\n", addr);
	    idx = 0;
		while (idx <= sw_bp_max &&
		sw_bp_addr[idx] != addr &&
		sw_bp_memdata[idx] != SW_BP_PATTERN)
			idx++;
#ifdef TARGET_HOST
	    if (idx < sw_bp_max) {
	        memory_write(sw_bp_addr[idx], &sw_bp_memdata[idx], 1);
	        sw_bp_memdata[idx] = SW_BP_PATTERN;
	    } else {
		ret = -1;
		printf("SW BP not found !!!\n");
	    }
#else
		ret = -1;
#endif

	    break;

	case '1':	/* remove hardware breakpoint */
		printf(
		"Entferne HW Breakpoint an %8.8x, Bereich %8.8x, Maske %8.8x\n",
		addr, range, mask);
	    printf("Not tested yet!\n");
#ifdef JTAG
	    if (!ice_wp_enabled(1) && ice_get_breakpoint_addr(1) == addr) {
		ice_disable_wp(1);
	    } else {
		printf("HW BP not found !!!\n");
		ret = -1;
	    }
#endif
	    break;

	case '2':	/* remove watchpoint */
		printf("Entferne Watchpoint an %8.8x, Bereich %8.8x, Maske %8.8x\n",
		addr, range, mask);
	    printf("Not tested yet!\n");
#ifdef JTAG
	    if (ice_wp_enabled(1) && ice_get_watchpoint_addr(1) == addr) {
		ice_disable_wp(1);
	    } else {
		printf("WP not found !!!\n");
		ret = -1;
	    }
#endif
	    break;

	default:
		printf(
		"Unbekanntes BP-Kommando: %c, Addresse: %8.8x, Bereich %8.8x, Maske %8.8x\n",
		type, addr, range, mask);
	    ret = -1;
	    break;
	}

	return ret;
}


void gdb_step() {
#ifdef TARGET_HOST
	int idx = 0;

	/* before stepping restore all memory data and clear breakpoints there */
	/* after  stepping reinstall software breakpoints and enable breakpoints */

	/* software breakpoints */
	while (idx <= sw_bp_max &&
		sw_bp_memdata[idx] != SW_BP_PATTERN &&
		sw_bp_addr[idx] != arm_regs.pc)
		idx++;
	if (idx < sw_bp_max)
	memory_write(sw_bp_addr[idx], &sw_bp_memdata[idx], 1);

	/* step now */
	if (idx < sw_bp_max)
	printf("Stepping over software breakpoint %i\n", idx);
	debug_step();

	/* software breakpoints */
	if (idx < sw_bp_max)
	memory_write(sw_bp_addr[idx], &SW_BP_PATTERN, 1);
#endif /* TARGET_HOST */
}

/* grap register values saved in memory, copy to arm_regs structure */
void
update_registers(void)
{
	trap_t trap_regs;

	memset(&trap_regs, 0, sizeof(trap_t));

	/* copy:
	 * 0: trap type
	 * 4: EPC
	 * 8: cpsr
	 * 12: spsr
	 * 16 - 76: r0 --> r15
	 */
	/* seek to offset */
	memory_read(tmp_type_offset, (char *)&trap_regs, sizeof(trap_t));

	memcpy(&arm_regs.r[0], &trap_regs.r0, 16*4);
	arm_regs.cpsr = trap_regs.cpsr;

}
int gdb_read_registers(int length, char *buffer)
{
	int temp;
	int realsize = 0;
	uint32 padding = 0;

	update_registers();

	printf("Registers:\n");
	for (temp = 0; temp < 8; temp++)
	printf("  R%2.2d - 0x%8.8x   R%2.2d - 0x%8.8x\n",
	temp, arm_regs.r[temp],
	temp + 8, arm_regs.r[temp + 8]);

	printf("\n  PC  - 0x%8.8x\n", arm_regs.pc);

	/* r0..r15 (32 bits) */
	for (temp = 0; temp <= 15; temp++)
	realsize += sprintf(buffer+realsize, "%8.8x", hton32(arm_regs.r[temp]));
	/* f0..f7 (4*32 bits) */
	for (temp = 0; temp <= 7; temp++)
		realsize += sprintf(buffer + realsize, "%8.8x%8.8x%8.8x", padding,
			padding, padding);
	/* fps (32 bits) */
	realsize += sprintf(buffer+realsize, "%8.8X", padding);
	/* cpsr (32 bits) */
	realsize += sprintf(buffer+realsize, "%8.8X", hton32(arm_regs.cpsr));

	/* printf("Returning %d Bytes for Registers\n", realsize); */
	return realsize;
}

int gdb_write_registers(char *buffer) {
#ifdef TARGET_HOST
	uint32 realsize = 0;
	uint32 mytemp = 0;
	uint32 tempL = 0;
	char minibuffer[10];

	buffer++;
	for (tempL = 0; (tempL < buffersize-1) && (buffer[tempL] != '\0'); tempL++)
		realsize = tempL;

	printf("Found %d bytes for register-write.\n", realsize);

	for (mytemp = 0; mytemp <= realsize; mytemp++) {
	minibuffer[mytemp%8] = buffer[mytemp];
	if (mytemp%8 == 7) {
	    minibuffer[8] = '\0';
	    tempL = (mytemp - (mytemp%8))>>3;
	    arm_regs.r[tempL] = HexString_to_Long(minibuffer);
	    printf("Register %d written: %8.8x - %s\n", tempL, arm_regs.r[mytemp], minibuffer);
	}
	}
	/* always switch to usr mode */
	arm_regs.cpsr &= ~PSR_M_sys;
	arm_regs.cpsr |= PSR_M_usr;
#endif /* TARGET_HOST */
	return 0;
}


#define change_endian(l) (\
	((l & 0x000000ff) << 24) |	\
	((l & 0x0000ff00) <<  8) |	\
	((l & 0x00ff0000) >>  8) |	\
	((l & 0xff000000) >> 24));

void gdb_write_memory(uint32 addr, uint32 length, char *buffer) {
#ifdef TARGET_HOST
	uint32 writeaddr;	/* memory_write Adresse */
	uint32 *writedata;	/* memory_write Anfang */
	uint32 writelength;	/* memory_write Länge */
	uint32 *longptr;
	char *charptr;
	uint32 data;

	/* im Speicher alles, in der selben Reihenfolge,
	 * wie den Datenstrom schreiben
	 * ===> BIG-ENDIAN auf little-Endian Maschine
	 */

	writeaddr   = addr;
	writedata   = malloc(sizeof(uint32)*(length+2));
	writelength = length;
	charptr     = (char *) writedata;

	/* Verschnitt am Anfang */
	if (writeaddr % 4 != 0) {
	printf("Unaligned begin address: ");
	writeaddr &= ~(0x03);
	memory_read(writeaddr, &data, 1);
	printf("%8.8x\n", data);
	*writedata = change_endian(data);
	writelength += addr % 4;
	charptr = (char *)((uint32)charptr + addr % 4);
	}

	/* Verschnitt am Ende */
	if (writelength % 4 != 0) {
	printf("Unaligned end address  : ");
	writelength &= ~(0x03);
	/* printf("addr+length=%x+%x=%x\n", writeaddr, writelength, writeaddr+writelength); */
	memory_read(writeaddr+writelength, &data, 1);
	printf("%8.8x\n", data);
	longptr = (uint32 *) ((uint32) writedata + writelength);
	*longptr = change_endian(data);
	writelength += 4;
	}

	/* Bytes einlesen */
	while (length > 0) {
	*charptr = HexByte_to_Value(*buffer)*16 + HexByte_to_Value(*(buffer+1));
	buffer += 2;
	*charptr++;
	length--;
	}

	/* im Speicher stehen jetzt Big-Endians auf einer Little-Endian (Intel) Maschine */
	/*
	printf("%x, %x:", writeaddr, writelength);
	length = writelength;
	longptr = writedata;
	while (length > 0) {
	*longptr = change_endian(*longptr);
	printf("%8.8x", *longptr);
	length-=4;
	longptr++;
	}
	printf("\n");
	*/

	/* so... und alles schreiben */
	memory_write(writeaddr, writedata, writelength);

	free(writedata);
#endif /* TARGET_HOST */
}
int main(int argc, char **argv)
{
	int argn = 0;
	unsigned short port = 0;
	uchar *filename;

#ifndef TARGET_HOST
	if (argc < 2 || argc > 5) {
	usage:
		printf("Usage: %s <symfile> [<offset>] [-p <port>]\n", argv[0]);
		goto exiterr;
	}
	argc -= 1; argn++;
	filename = argv[argn];
	{
		uchar *filename_str, *rest;
		uchar *base_ptr; 

		filename_str = argv[argn];
		filename = strtok_r(filename_str, "@", &rest);
		printf ("filename is %s\n", filename);
		if (*rest != NULL) {
			base_ptr = strtok_r(rest, "@", &filename_str);
			baseaddress = strtoul(base_ptr, NULL, 16);
		}
	}

	symfd = open(filename, O_RDONLY);
	if (symfd == -1) {
		printf("%s: file open error: name %s\n", __FUNCTION__, argv[argn]);
		goto exiterr;
	}
	argc -= 1; argn++;

	if (argc && (strcmp(argv[argn], "-p") != 0)) {
		tmp_type_offset = strtoul(argv[argn], NULL, 16);
		if (tmp_type_offset == 0) {
			printf("%s: bad offset arg: %s\n", __FUNCTION__, argv[argn]);
			goto exiterr;
		}
		argc--; argn++;
	}

	if (argc) {
		if (strcmp(argv[argn], "-p") != 0)
			goto usage;
		argc--; argv++;
		if (!argc)
			goto usage;
		port = strtoul(argv[argn], NULL, 0);
		if (port == 0) {
			printf("%s: bad port arg: %s\n", __FUNCTION__, argv[argn]);
			goto exiterr;
		}
		argc--; argn++;
	}

	if (argc)
		goto usage;
	

#endif /* TARGET_HOST */


	printf("Broadcom arm gdb server\n");
#ifdef JTAG
	printf("gdb-jtag-arm - gdb server with JTAG interface to the ARM processor family\n");
	printf("by Tobias Lorenz and Lars Kristian Klauske (Jan 2004)\n");
	printf("based on jtag-arm9 by Simon Wood. July 2001\n\n");

	pp_init();
	tapsm_reset(1);
#endif

	    do_gdb(port);

#ifdef JTAG
	pp_done();
#endif
	close(symfd);
	exit(0);
#ifndef TARGET_HOST
exiterr:
	exit(1);
#endif
}
