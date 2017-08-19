/*
 * Firmware debugger LINUX routines
 *
 * Copyright (C) 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id$
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <linux/file.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>

struct listener_params {
	int (*main)(void *handle);
	int port;
};

static int sock_send(struct socket *sock, const char *buf, const size_t length, unsigned long flags) {
	struct iovec iov;
	struct msghdr msg;
	int size; 
	int written = 0;
	int remaining = length;
	mm_segment_t old_mm;

	memset(&msg, 0, sizeof(msg));

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = flags;

	old_mm = get_fs();
	set_fs(KERNEL_DS);

	for (;;) {
		msg.msg_iov->iov_len = remaining;
		msg.msg_iov->iov_base = (char *) buf + written;

		size = sock_sendmsg(sock, &msg, remaining);

		if (size == -ERESTARTSYS)
			continue;

		if (!(flags & MSG_DONTWAIT) && (size == -EAGAIN))
			continue;

		if (size > 0) {
			written += size;
			remaining -= size;
			if (remaining)
				continue;
		}

		break;
	}

	set_fs(old_mm);

	return written ? written : size;
}

static int sock_read(struct socket *sock, char *buf, int buflen ) {
	struct msghdr msg;
	struct iovec iov;
	int size;
	mm_segment_t old_mm;

	memset(&msg, 0, sizeof(msg));

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_iov->iov_base = buf;
	msg.msg_iov->iov_len = buflen;

	old_mm = get_fs();
	set_fs(KERNEL_DS);

	for (;;) {
		size = sock_recvmsg(sock, &msg, buflen, 0);
		if (size != -EAGAIN && size != -ERESTARTSYS) {
			break;
		}
	}

	set_fs(old_mm);

	return size;
}

/* server_socket_listener()
 *
 * Listen on a port and spawn new threads to handle incomming connections
 */

static int server_socket_listener(void *vp) {
	int rval = -1;
	struct socket *new_socket;
	struct sockaddr_in srvaddr;
	struct socket *servsock = NULL;
	struct listener_params *lp = (struct listener_params *)vp;

	rval = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &servsock);

	memset(&srvaddr,0, sizeof(srvaddr));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(lp->port);
	srvaddr.sin_addr.s_addr= htonl(INADDR_ANY);;

	rval = servsock->ops->bind(servsock, (struct sockaddr *)&srvaddr, sizeof (srvaddr));
	if (rval < 0)
		goto out;

	for (;;) {
		if ((rval = servsock->ops->listen(servsock, 10) < 0))
			goto out;

		rval = kernel_accept(servsock, &new_socket, 0);

		kthread_run(lp->main, (void *) new_socket, "debugger_ui");
	}

out:
	sock_release(servsock);

	do_exit(rval);

	return rval;

}

static void socket_fputc(char c, void *handle) {
	sock_send((struct socket *)handle, &c, 1, MSG_DONTWAIT);
}

static int socket_fgetc(void *handle) {
	int len;
	char c;

	len = sock_read((struct socket *)handle, &c, 1);

	if (len)
		return c;

	return 0;
}

extern int debugger_ui_main(void *handle);
extern int debugger_gdb_main(void *handle);

struct listener_params listeners[] = {
	{ debugger_ui_main, 24 },
	{ debugger_gdb_main, 2424 },
	{ NULL }
};

/* Public functions */

void *g_bus_handle = NULL;

void debugger_init(void *bus_handle) {
	struct listener_params *lp;

	g_bus_handle = bus_handle;

	for (lp = listeners; lp->main; lp++)
		kthread_run( server_socket_listener, (void *) lp, "debugger_listener");
}

void dbg_fputc(char c, void *handle) {
	sock_send((struct socket *)handle, &c, 1, MSG_DONTWAIT);
}

int dbg_fgetc(void *handle) {
	int len;
	char c;

	len = sock_read((struct socket *)handle, &c, 1);

	if (len)
		return c;

	return 0;
}

char *dbg_fgets(char *s, void *handle)
{
	int ch;
	char *p;

	p = s;
	while ((ch = socket_fgetc(handle)) != '\n')
		*p++ = ch;

	*p = 0;
	return s;
}

void dbg_fputs(char *s, void *handle) {
	while (*s)
		socket_fputc(*s++, handle);
}

void dbg_mdelay(int d) {
	mdelay(d);
}

void dbg_fclose(void *handle) {
	sock_release((struct socket *) handle);
}

/* dhd_sdio.c */
extern uint32_t dhd_sdio_reg_read(void *handle, uint32_t addr);
extern void dhd_sdio_reg_write(void *handle, uint32_t addr, uint32_t val);

void dbg_reg_write(uint32_t addr, uint32_t val) {
	if (!g_bus_handle)
		return;

	dhd_sdio_reg_write(g_bus_handle, addr, val);
}

uint32_t dbg_reg_read(uint32_t addr) {
	if (!g_bus_handle)
		return 0;

	return dhd_sdio_reg_read(g_bus_handle, addr);
}
