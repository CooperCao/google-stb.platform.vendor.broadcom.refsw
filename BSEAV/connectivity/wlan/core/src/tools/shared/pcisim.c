/*
 * Simple tcp socket based PCI i/o simulation.
 *
 * <epipci.h> defines the msg interface.
 *
 * Copyright 2001, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id$
 */

#include <stdlib.h>
#include <typedefs.h>
#include <rts/debug_mem.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <utils.h>
#include <sys_xx.h>
#include <pcisim.h>
#include <bcmendian.h>
#include <sim_dbg.h>

#ifdef BCMSIM
extern void process_intr(void); /* From misc.c */
extern void bcmsim_pci_transaction(pci_transaction_t *t);  /* From 47xx.c */
#endif

int pciverbose = 0;

#ifndef TRUE
#define	TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define	OPSZ	sizeof (pci_transaction_t)

/*
 * The client-side and server-side are different beasts.
 *
 * The client-side supports multiple user threads calling
 * pci_read() and pci_write() plus a dedicated thread processing
 * responses and interrupt msgs arriving on the client-side socket,
 * plus a dedicated interrupt thread.
 *
 * The server-side supports a single-threaded pci_recv()/pci_reply() loop
 * plus the occational pci_sendinterrupt().
 */

/*
 * Each client pci transaction is done in the context of a pciop.
 */
struct pciop {
	long event;
	pci_transaction_t t;
};

/*
 * There's a single client-side lock.
 */
static void* pci_lock = NULL;

/*
 * There's a table of pending client-side pci operations.
 */
#define	MAXPCIQ	16
static struct pciop *pciq[MAXPCIQ];

/*
 * For interrupts,
 * there's a boolean "interrupt is asserted" variable,
 * an event that the dedicated interrupt thread waits on,
 * and a DDK ISR callback function pointer.
 */
uint	pci_intr_asserted = 0;
long	pci_intr_event = 0;


/*
 * Both the client-side and server-side need a socket, sequence space,
 * and sockaddr.
 */
static int pci_ssocket = -1;
static int pci_csocket = -1;
static uint pci_sseq = 0;
static uint pci_cseq = 0;
static struct sockaddr_in server_sin;

/* local prototypes */
static void pci_htol(pci_transaction_t *t);
static void pci_ltoh(pci_transaction_t *t);
static void pci_loop(void);
#ifndef BCMSIM
static void pcienq(struct pciop *op);
#endif
static struct pciop *pcideq(uint seq);
static char* pci_word_format(uint be, char* in_buf);


int
pci_client_init(char *host, int port)
{
	struct hostent *hp;
	struct sockaddr_in local_sin;
	int i;

	/* init the client-side pci subsystem lock */
	pci_lock = alloclock();
	ASSERT(pci_lock);

	if (*host == '\0')
		gethostname (host, 256);

	if ((hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "gethostbyname error: %s\n", host);
		return (-1);
	}

	bzero(&server_sin, sizeof (struct sockaddr_in));
	server_sin.sin_addr.s_addr = *(int *)(hp->h_addr);
	server_sin.sin_family = AF_INET;
#ifdef notdef
	server_sin.sin_len = sizeof (server_sin);
#endif
	server_sin.sin_port = htons((ushort)port);

	if ((pci_csocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		syserr("socket call failed");

	bzero(&local_sin, sizeof (struct sockaddr_in));

	local_sin.sin_family = AF_INET;
	local_sin.sin_addr.s_addr = htonl(INADDR_ANY);
	local_sin.sin_port = 0;

	if (bind(pci_csocket, (struct sockaddr *) &local_sin, sizeof (struct sockaddr)) < 0)
		syserr ("bind call failed");

	if (connect(pci_csocket, (struct sockaddr *) &server_sin, sizeof (struct sockaddr)) < 0)
		syserr ("connect");

	/* clients start at sequence zero */
	pci_cseq = 0;

	/* init pci op queue table */
	for (i = 0; i < MAXPCIQ; i++)
		pciq[i] = NULL;

	/* init pci_intr_event */
	pci_intr_event = allocevent(FALSE, FALSE);
	ASSERT(pci_intr_event);

	/* start a thread that processes client-side received msgs */
	createthread((void*)pci_loop, (void*) 0, 31);
	msleep(100);

	return (pci_csocket);
}

int
pci_server_init(int *port)
{
	struct sockaddr_in sin;
	unsigned int addrlen;
	int listen_socket;
	int rc;
	int fd;
	int val;

	if ((listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return (-1);

	bzero(&sin, sizeof (struct sockaddr_in));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	/* bind to a port */
	if (*port == 0)
		*port = PCI_SOCKET_ADDR_BASE;

	sin.sin_port = htons((short) *port);
	rc = bind(listen_socket, (struct sockaddr *) &sin, sizeof (struct sockaddr));
	if (rc < 0)
		syserr("pci_server_init() bind");

	val = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) < 0)
		syserr("setsockopt: SO_REUSEADDR");

	if (listen(listen_socket, 1) < 0)
		syserr("pci_server_init() listen");

	addrlen = sizeof (sin);
	if ((fd = accept(listen_socket, (struct sockaddr*) &sin, &addrlen)) < 0)
		syserr("pci_server_init() accept");

	close(listen_socket);
	pci_ssocket = fd;

	val = 1;
	if (setsockopt(pci_ssocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof (val)) < 0)
		syserr("setsockopt: SO_REUSEADDR");

	/* servers start at higher sequence */
	pci_sseq = 20000;

	return (pci_ssocket);
}

/*
 * The byte enables are returned as 3 4-bit fields: the byte enables
 * for the first cycle, the byte enables for the middle cycles (always
 * all set) and the byte enables for the last cycle.
 */
static uint
compute_byte_enables(uint addr, uint size)
{
	uint offset, first, last, idx;

	offset = addr & 3;
	first = 0;
	last = (offset + size > 3) ? 4 : offset + size;
	for (idx = offset; idx < last; idx++)
		first |= (1 << idx);
	first = (~first) & 0xf;

	offset = (addr + size - 1) & 3;	/* bytes valid on last cycle */
	last = 0;
	for (idx = 0; idx <= offset; idx++)
			last |= (1 << idx);
	last = (~last) & 0xf;
	
	return (first << 8) | last;
}

#ifndef BCMSIM
int
pci_read(uint pci_addr, uchar *dst, uint nbytes, uint cfgread)
{
	struct pciop *op;
	int cmd;
	int seq;
	uint be;
	uint i;
	int result;

	SIM_PCI(("pci_read: addr 0x%x nbytes %d: ", 
			      pci_addr, nbytes));

	ASSERT(nbytes > 0);
	ASSERT(nbytes <= (PCI_MAX_BURST_SIZE - (pci_addr & 3)));

	lock(pci_lock);
	seq = pci_cseq++;
	unlock(pci_lock);

	cmd = cfgread? pci_cmd_conf_rd: pci_cmd_read;
	// If we are reading from simulated pci config space, 
	// we need to high nibble to be 'f'
	pci_addr |= cfgread ? PCI_CONFIG_BASE : 0;

	/*
	 * Stack addresses aren't accessible by other threads..
	 */
	op = malloc(sizeof (struct pciop));

	bzero(op, sizeof (struct pciop));
	op->t.sequence = seq;
	op->t.dest = PCI_DEST_MASTER;
	op->t.command = cmd;
	op->t.pci_addr = pci_addr & ~3;

	/*
	 * The 'size' field is the number of data phases needed
	 * for the pci transaction == the number of words spanned
	 * by the data.
	 */
	op->t.size = ((pci_addr & 3) + nbytes + 3) >> 2;

	/*
	 * The 'byte enable' fields are the compliment of the
	 * positions within the word of the valid bytes.
	 */
	be = compute_byte_enables(pci_addr, nbytes);
	op->t.first_byte_en = (be >> 8) & 0xf;
	op->t.last_byte_en = be & 0xf;

	pci_htol(&op->t);

	/*
	 * Yes I know allocating and destroying an Event handle
	 * for every pci transaction is pretty silly but it's late
	 * and I'm too lazy to add a cache of event handles right now..
	 */
	op->event = allocevent(FALSE, FALSE);

	pcienq(op);

	if (write(pci_csocket, (char*)&op->t, OPSZ) != OPSZ)
		return (-1);

	waitforevent(op->event);

	freeevent(op->event);

	if (op->t.result != PCI_RESULT_OK) {
		/* When the PCI bus indicates Master Abort and we are reading
		   config space, we must turn the results into 0xFF bytes as
		   per the PCI spec.  */
		if (cfgread && op->t.result == PCI_RESULT_MABT) {
			memset(dst, 0xFF, nbytes);
			result = 0;
		} else {
			result = -1;
		}
	} else {
		/* copy returned data into user buffer */
		bcopy((uchar*)(((uintptr)op->t.data) + (pci_addr & 3)), dst, nbytes);
		result = 0;
	}

	if (pciverbose) {
		switch (nbytes) {
		case 2:
			printf("%04x\n", *(ushort*)dst);
			break;
		case 4:
			printf("%08x\n", *(uint*)dst);
			break;
		default:
			for (i = 0; i < nbytes; i++)
				printf("%02x ", dst[i]);
			printf("\n");
		}
	}

	free((char*) op);

	return (result);
}

int
pci_write(uint pci_addr, uchar *src, uint nbytes, uint cfgwrite)
{
	struct pciop *op;
	int seq;
	int cmd;
	uint i;
	uint be;

	SIM_PCI(("pci_write: addr 0x%x nbytes %d: ", 
			      pci_addr, nbytes));

	ASSERT(nbytes > 0);
	ASSERT(nbytes <= (PCI_MAX_BURST_SIZE - (pci_addr & 3)));

	lock(pci_lock);
	seq = pci_cseq++;
	unlock(pci_lock);

	cmd = cfgwrite? pci_cmd_conf_wr: pci_cmd_write;
	// If we are writing into simulated pci config space, 
	// we need to high nibble to be 'f'
	pci_addr |= cfgwrite ? PCI_CONFIG_BASE : 0;

	/*
	 * Stack addresses aren't accessible by other threads..
	 */
	op = malloc(sizeof (struct pciop));

	bzero(op, sizeof (struct pciop));
	op->t.sequence = seq;
	op->t.dest = PCI_DEST_MASTER;
	op->t.command = cmd;
	op->t.pci_addr = pci_addr & ~3;

	/*
	 * The 'size' field is the number of data phases needed
	 * for the pci transaction == number of words spanned
	 * by the data.
	 */
	op->t.size = ((pci_addr & 3) + nbytes + 3) >> 2;

	/*
	 * The 'byte enable' fields are the compliment of the
	 * positions within the word of the valid bytes.
	 */
	be = compute_byte_enables(pci_addr, nbytes);
	op->t.first_byte_en = (be >> 8) & 0xf;
	op->t.last_byte_en = be & 0xf;


	if (pciverbose) {
		switch (nbytes) {
		case 2:
			printf("%04x", *(ushort*)src);
			break;
		case 4:
			printf("%08x", *(uint*)src);
			break;
		default:
			for (i = 0; i < nbytes; i++)
				printf("%02x ", src[i]);
		}
	}

	/* copy user data into proper byte lanes */
	bcopy(src, (uchar*)(((uintptr)op->t.data) + (pci_addr & 3)), nbytes);

	pci_htol(&op->t);

	op->event = allocevent(FALSE, FALSE);

	pcienq(op);

	if (write(pci_csocket, (char*) &op->t, OPSZ) != OPSZ)
		return (-1);

	waitforevent(op->event);

	freeevent(op->event);

	if (op->t.result != PCI_RESULT_OK) {
		printf("pci_write: result %d\n", op->t.result);
		return (-1);
	}

	free((char*) op);

	SIM_PCI(("\n"));

	return (0);
}

#else /* ifndef BCMSIM */

int
pci_read(uint pci_addr, uchar *dst, uint nbytes, uint cfgread)
{
	struct pciop op;
	int cmd;
	uint be;
	uint i;
	int result;

	SIM_PCI(("pci_read: addr 0x%x nbytes %d: ", 
			      pci_addr, nbytes));

	ASSERT(nbytes > 0);
	ASSERT(nbytes <= (PCI_MAX_BURST_SIZE - (pci_addr & 3)));

	cmd = cfgread? pci_cmd_conf_rd: pci_cmd_read;
	/* If we are reading from simulated pci config space, 
		 we need to high nibble to be 'f' */
	pci_addr |= cfgread ? PCI_CONFIG_BASE : 0;

	bzero(&op, sizeof (struct pciop));
	op.t.dest = PCI_DEST_MASTER;
	op.t.command = cmd;
	op.t.pci_addr = pci_addr & ~3;

	/*
	 * The 'size' field is the number of data phases needed
	 * for the pci transaction == the number of words spanned
	 * by the data.
	 */
	op.t.size = ((pci_addr & 3) + nbytes + 3) >> 2;

	/*
	 * The 'byte enable' fields are the compliment of the
	 * positions within the word of the valid bytes.
	 */
	be = compute_byte_enables(pci_addr, nbytes);
	op.t.first_byte_en = (be >> 8) & 0xf;
	op.t.last_byte_en = be & 0xf;

	bcmsim_pci_transaction(&op.t);

	if (op.t.result != PCI_RESULT_OK) {
		/* When the PCI bus indicates Master Abort and we are reading
		   config space, we must turn the results into 0xFF bytes as
		   per the PCI spec.  */
		if (cfgread && op.t.result == PCI_RESULT_MABT) {
			memset(dst, 0xFF, nbytes);
			result = 0;
		} else {
			result = -1;
		}
	} else {
		/* copy returned data into user buffer */
		bcopy((uchar*)(((uint)op.t.data) + (pci_addr & 3)), dst, nbytes);
		result = 0;
	}

	if (pciverbose) {
		switch (nbytes) {
		case 2:
			printf("%04x\n", *(ushort*)dst);
			break;
		case 4:
			printf("%08x\n", *(uint*)dst);
			break;
		default:
			for (i = 0; i < nbytes; i++)
				printf("%02x ", dst[i]);
			printf("\n");
		}
	}

	return (result);
}

int
pci_write(uint pci_addr, uchar *src, uint nbytes, uint cfgwrite)
{
	struct pciop op;
	int cmd;
	uint i;
	uint be;

	SIM_PCI(("pci_write: addr 0x%x nbytes %d: ", 
			      pci_addr, nbytes));

	ASSERT(nbytes > 0);
	ASSERT(nbytes <= (PCI_MAX_BURST_SIZE - (pci_addr & 3)));

	cmd = cfgwrite? pci_cmd_conf_wr: pci_cmd_write;
	// If we are writing into simulated pci config space, 
	// we need to high nibble to be 'f'
	pci_addr |= cfgwrite ? PCI_CONFIG_BASE : 0;

	bzero(&op, sizeof (struct pciop));
	op.t.dest = PCI_DEST_MASTER;
	op.t.command = cmd;
	op.t.pci_addr = pci_addr & ~3;

	/*
	 * The 'size' field is the number of data phases needed
	 * for the pci transaction == number of words spanned
	 * by the data.
	 */
	op.t.size = ((pci_addr & 3) + nbytes + 3) >> 2;

	/*
	 * The 'byte enable' fields are the compliment of the
	 * positions within the word of the valid bytes.
	 */
	be = compute_byte_enables(pci_addr, nbytes);
	op.t.first_byte_en = (be >> 8) & 0xf;
	op.t.last_byte_en = be & 0xf;


	if (pciverbose) {
		switch (nbytes) {
		case 2:
			printf("%04x", *(ushort*)src);
			break;
		case 4:
			printf("%08x", *(uint*)src);
			break;
		default:
			for (i = 0; i < nbytes; i++)
				printf("%02x ", src[i]);
		}
	}

	/* copy user data into proper byte lanes */
	bcopy(src, (uchar*)(((uint)op.t.data) + (pci_addr & 3)), nbytes);

	/* pci_htol(&op->t); */

	bcmsim_pci_transaction(&op.t);

	if (op.t.result != PCI_RESULT_OK) {
		printf("pci_write: result %d\n", op.t.result);
		return (-1);
	}

	SIM_PCI(("\n"));

	return (0);
}
#endif /* ifndef BCMSIM */

/* copy from host memory to pci memory */
void
pci_copyin(uchar *src, uint32 dst, uint n)
{
	int size;
	uint resid;
	int rc;

	resid = n;

	while (resid > 0) {
		size = MIN(resid, (PCI_MAX_BURST_SIZE - (dst & 3)));
		rc = pci_write(dst, src, size, 0);
		ASSERT(rc == 0);

		src += size;
		dst += size;
		resid -= size;
	}
}

/* copy from pci memory to host memory */
void
pci_copyout(uint32 src, uchar *dst, uint n)
{
	int size;
	uint resid;
	int rc;

	resid = n;

	while (resid > 0) {
		size = MIN(resid, (PCI_MAX_BURST_SIZE - (src & 3)));
		rc = pci_read(src, dst, size, 0);
		ASSERT(rc == 0);

		src += size;
		dst += size;
		resid -= size;
	}
}

/* copy from host memory to pci memory */
void
pci_cfg_copyin(uchar *src, uint32 dst, uint n)
{
	int size;
	uint resid;
	int rc;

	resid = n;

	while (resid > 0) {
		size = MIN(resid, (PCI_MAX_BURST_SIZE - (dst & 3)));
		rc = pci_write(dst, src, size, TRUE);
		ASSERT(rc == 0);

		src += size;
		dst += size;
		resid -= size;
	}
}

/* copy from pci configuration memory to host memory */
void
pci_cfg_copyout(uint32 src, uchar *dst, uint n)
{
	int size;
	uint resid;
	int rc;

	resid = n;

	while (resid > 0) {
		size = MIN(resid, (sizeof(uint) - (src & 3)));
		rc = pci_read(src, dst, size, TRUE);
		ASSERT(rc == 0);

		src += size;
		dst += size;
		resid -= size;
	}
}

int
pci_reply(pci_transaction_t *t)
{
	SIM_PCI(("pci_reply: "));
	if (pciverbose)
		pci_print(t, 0);
	pci_htol(t);

	if (write(pci_ssocket, (char*)t, OPSZ) != OPSZ)
		return (-1);

	return (0);
}

#ifdef BCMSIM
void
pci_unified_intr(pci_transaction_t *t)
{
	if (t->command == pci_interrupt) {
		/* process the interrupt */
		if (!t->data[0]) {
			SIM_PCI(("INTERRUPT %d\n", !t->data[0]));
			process_intr();
		}
	} else {
		SIM_PCI(("t->command != pci_interrupt\n"));
	}
}
#endif

/* dedicated client-side socket receive processing thread */
static void
pci_loop()
{
	pci_transaction_t t;
	struct pciop *op;

	/*
 	 * Loop processing msgs arriving on the client-side socket.
	 */
	while (1) {
		(void) pci_recv(&t, pci_csocket);

		if (t.command == pci_interrupt) {
			pci_intr_asserted = !t.data[0];
			SIM_PCI(("INTERRUPT %d\n", 
					      pci_intr_asserted));

			/* wakeup ioisr() thread */
			if (pci_intr_asserted)
				setevent(pci_intr_event);

			continue;
		}

		/* find an op in the queue that matches */
		op = pcideq(t.sequence);

		if (op) {
			/* convert it back to host representation */
			pci_ltoh(&op->t);
			ASSERT(op->t.command == t.command);
			ASSERT(op->t.size == t.size);

			/* copy returned data to user */
			op->t = t;

			/* wakeup sleeping thread */
			setevent(op->event);
		}
		else
			ASSERT(0);
	}
}

/*
 * Read from the pci socket
 * do whatever transaction arrives.
 */
int
pci_recv(pci_transaction_t *t, int s)
{
	int bytes;

	bytes = read(s, (char *)t, OPSZ);

	if (bytes < 0)
		syserr("pci_recv: read");
	if (bytes == 0) {
		printf("EOF\n");
		exit(0);
	}

	ASSERT(bytes == OPSZ);

	pci_ltoh(t);

	/* sanity check */
	ASSERT((t->command == pci_cmd_read) || (t->command == pci_cmd_write)
		|| (t->command == pci_interrupt)
		|| (t->command == pci_cmd_conf_rd)
		|| (t->command == pci_cmd_conf_wr)
		|| (t->command == pci_cmd_kill));

	return (bytes);
}

int
pci_sendinterrupt(uint value)
{
	pci_transaction_t t;
	int seq;
	
	SIM_PCI(("pci_sendinterrupt\n"));

	seq = pci_sseq++;
	
	bzero(&t, sizeof (pci_transaction_t));
	t.sequence = seq;
	t.dest = PCI_DEST_SLAVE;
	t.command = pci_interrupt;
	t.size = 1;
	t.data[0] = value;

#ifdef BCMSIM
	pci_unified_intr(&t);
#else
	pci_htol(&t);

	if (write(pci_ssocket, (char*) &t, OPSZ) != OPSZ)
		return (-1);
#endif

	return (0);
}


/*
 * Kill the simulator.
 */
int
pci_kill(void)
{

	struct pciop *op;

	/*
	 * Stack addresses aren't accessible by other threads..
	 */
	op = malloc(sizeof (struct pciop));

	bzero(op, sizeof (struct pciop));
	op->t.sequence = pci_cseq++;
	op->t.dest = PCI_DEST_MASTER;
	op->t.command = htol32 (pci_cmd_kill);

	if (write(pci_csocket, (char*) &op->t, OPSZ) != OPSZ)
		return (-1);
	return (0);
}

/*
 * Asynchronously request to client to kill server
 */
void pci_killme(void)
{
	pci_transaction_t t;

	if (pci_ssocket == -1)
		return;
	
	t.sequence = pci_sseq++;
	t.dest = PCI_DEST_MASTER;
	t.command = htol32(pci_cmd_kill);
	
	write(pci_ssocket, (char*) &t, sizeof(pci_transaction_t));
	if (read(pci_ssocket, (char*) &t, sizeof(pci_transaction_t)) == 0) {
		printf("EOF\n");
		exit(0);
	}
}

static char*
cmdname(uint cmd)
{
	static char buf[80];

	switch (cmd) {
	case pci_cmd_conf_rd:
		return ("pci_cmd_conf_rd");

	case pci_cmd_conf_wr:
		return ("pci_cmd_conf_wr");

	case pci_cmd_read:
		return ("pci_cmd_read");

	case pci_cmd_write:
		return ("pci_cmd_write");

	default:

		sprintf(buf, "cmd %d", cmd);
		return (buf);
	}
}

static char*
pci_word_format(uint be, char* in_buf)
{
	static char sbuf[32];
	char* buf;
	int last_en;
	int en;
	int i;

	if (in_buf == NULL)
		buf = sbuf;
	else
		buf = in_buf;

	strcpy(buf, "0x");
	
	last_en = 1;
	for (i = 3; i >= 0; i--) {
		en = !(be & (1<<i));
		if (!en && last_en) {
			last_en = 0;
			strcat(buf, "(");
		} else if (en && !last_en) {
			last_en = 1;
			strcat(buf, ")");
		}
		strcat(buf, "%02x");
	}

	if (!last_en)
		strcat(buf, ") ");
	else
		strcat(buf, " ");

	return buf;
}

void
pci_print(pci_transaction_t *t, uint request)
{
	int i;
	
	printf ("sequence %d %s pci_addr 0x%x size %d result %d", 
		t->sequence,
		cmdname(t->command),
		t->pci_addr, t->size,
		t->result);

	if ((request && (t->command == pci_cmd_conf_rd || t->command == pci_cmd_read)) ||
	    (!request && (t->command == pci_cmd_conf_wr || t->command == pci_cmd_write))) {
		printf("\n");
		return;
	}

	printf(" ");
	printf(pci_word_format(t->first_byte_en, NULL),
	       (t->data[0]>>24) & 0xff, (t->data[0]>>16) & 0xff,
	       (t->data[0]>>8) & 0xff, t->data[0] & 0xff);

	for (i = 1; i+1 < t->size; i++)
		printf("0x%08x ", (uint)t->data[i]);

	if (t->size > 1)
		printf(pci_word_format(t->last_byte_en, NULL),
		       (t->data[i]>>24) & 0xff, (t->data[i]>>16) & 0xff,
		       (t->data[i]>>8) & 0xff, t->data[i] & 0xff);

	printf("\n");
}

static void
pci_htol(pci_transaction_t *t)
{
	int nwords;
	int i;

	nwords = t->size;

	ASSERT(nwords <= (PCI_CACHE_LINE_SIZE/sizeof(int)));

	t->sequence = htol32(t->sequence);
	t->dest = htol32(t->dest);
	t->command = htol32(t->command);
	t->pci_addr = htol32(t->pci_addr);
	t->pci_addr_hi = htol32(t->pci_addr_hi);
	t->size = htol32(t->size);
	t->result = htol32(t->result);

	t->data[0] = htol32(t->data[0]);

	for (i = 1; i < nwords; i++)
		t->data[i] = htol32(t->data[i]);
}
	
static void
pci_ltoh(pci_transaction_t *t)
{
	int nwords;
	int i;

	t->sequence = ltoh32(t->sequence);
	t->dest = ltoh32(t->dest);
	t->command = ltoh32(t->command);
	t->pci_addr = ltoh32(t->pci_addr);
	t->pci_addr_hi = ltoh32(t->pci_addr_hi);
	t->size = ltoh32(t->size);
	t->first_byte_en = ltoh32(t->first_byte_en);
	t->last_byte_en = ltoh32(t->last_byte_en);
	t->result = ltoh32(t->result);

	nwords = t->size;

	ASSERT(nwords <= (PCI_CACHE_LINE_SIZE/sizeof(int)));

	t->data[0] = ltoh32(t->data[0]);

	for (i = 1; i < nwords; i++)
		t->data[i] = ltoh32(t->data[i]);
}

#ifndef BCMSIM
/*
 * Stuff the op into an unused entry.
 */
static void
pcienq(struct pciop *op)
{
	int i;

	lock(pci_lock);

	for (i = 0; i < MAXPCIQ; i++)
		if (pciq[i] == NULL) {
			pciq[i] = op;
			break;
		}

	unlock(pci_lock);

	ASSERT(i < MAXPCIQ);
}
#endif

/*
 * Find an op matching the given sequence number,
 * remove it from the table, and return it.
 */
static struct pciop*
pcideq(uint seq)
{
	int i;
	struct pciop *op;

	op = NULL;

	lock(pci_lock);

	for (i = 0; i < MAXPCIQ; i++)
		if (pciq[i] && (ltoh32(pciq[i]->t.sequence) == seq)) {
			op = pciq[i];
			pciq[i] = NULL;
			break;
		}

	unlock(pci_lock);

	return (op);
}
