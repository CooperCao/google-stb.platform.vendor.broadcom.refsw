/*
 * Application version of a high driver
 * This app takes place of a high level driver for a BMAC low driver
 * and uses the BMAC RPC API to control the device
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <osl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <bcmendian.h>
#include <bcm_rpc.h>
#include <bcm_rpc_tp.h>
#include <bcm_rpc_char.h>

int wl_msg_level = 1;
static struct rpc_info *rpc = NULL;
static rpc_tp_info_t *rpcb = NULL;
static chardev_bus_t *cbus = NULL;
static osl_t *osh = NULL;

int test_call();

int
main(int argc, char **argv)
{
	int err;

	cbus = chardev_attach(osh);
	if (cbus == NULL) {
		printf("Failed chardev_attach\n");
		goto fail;
	}

	rpcb = bcm_rpc_tp_attach(osh, cbus);
	if (rpcb == NULL) {
		printf("Failed bcm_rpc_tp_attach\n");
		goto fail;
	}

	rpc = bcm_rpc_attach(NULL, NULL, rpcb);
	if (rpc == NULL) {
		printf("Failed bcm_rpc_attach\n");
		goto fail;
	}

	/* success */

	while (1)
		;

	return err;

fail:
	if (cbus)
		chardev_detach(cbus);
	if (rpcb)
		bcm_rpc_tp_detach(rpcb);

	return 1;
}

int
test_call()
{
	int len = 10;
	rpc_buf_t *b;
	unsigned char* p;
	int err;
	int i;

	b = bcm_rpc_tp_buf_alloc(rpcb, len);
	if (b == NULL)
		return 1;

	p = bcm_rpc_buf_data(rpcb, b);
	for (i = 0; i < len; i++) {
		*p++ = 'a' + i;
	}

	err = bcm_rpc_tp_buf_send(rpcb, b);
	if (err)
		bcm_rpc_tp_buf_free(rpcb, b);

	sleep(10);

	return 0;
}


struct chardev_bus {
	osl_t		*osh;
	pthread_t	reader_thread;
	int		fd;
	chardev_rx_fn_t	rx_fn;
	void		*rx_context;
};

static int chardev_reader(chardev_bus_t *cbus);
static int chardev_read(chardev_bus_t *cbus, void* buf, int len);

chardev_bus_t*
chardev_attach(osl_t *osh)
{
	const char *chardev_name = "/dev/wlrpc";
	chardev_bus_t* cbus;
	int fd;
	int err;

	cbus = (chardev_bus_t*)MALLOC(osh, sizeof(chardev_bus_t));
	if (cbus == NULL) {
		printf("%s: chardev_bus_t malloc failed\n", __FUNCTION__);
		goto fail;
	}

	memset(cbus, 0, sizeof(chardev_bus_t));

	cbus->osh = osh;

	fd = open(chardev_name, O_RDWR);
	if (fd == -1) {
		printf("%s: Failed to open \"%s\"\n", __FUNCTION__, chardev_name);
		goto fail;
	}

	cbus->fd = fd;

	err = pthread_create(&cbus->reader_thread, NULL, (void*)chardev_reader, cbus);
	if (err) {
		perror("pthread_create");
		goto fail;
	}

	return cbus;

fail:
	if (cbus)
		chardev_detach(cbus);

	return NULL;
}

static char static_buf[1024];
const static int static_buf_len = 1024;

static int
chardev_reader(chardev_bus_t *cbus)
{
	int len;
	int8 len_buf[4];
	int err;
	void *buf;

	while (1) {
		err = chardev_read(cbus, len_buf, sizeof(uint32));
		if (err) {
			printf("%s: error %d from chardev_read\n", __FUNCTION__, err);
			return err;
		}
		len = (int)ltoh32_ua(len_buf);
		printf("%s: read len val %d\n", __FUNCTION__, len);

		if (len <= static_buf_len)
			buf = static_buf;
		else {
			buf = malloc(len);
			if (buf == NULL) {
				printf("%s: buffer malloc failed, %d bytes\n", __FUNCTION__, len);
				return -1;
			}
		}

		err = chardev_read(cbus, buf, len);
		if (err) {
			printf("%s: error %d from chardev_read\n", __FUNCTION__, err);
			if (buf != static_buf)
				free(buf);
			return err;
		}
		printf("%s: read %d bytes data\n", __FUNCTION__, len);
		if (cbus->rx_fn)
			(cbus->rx_fn)(cbus->rx_context, buf, len);
		else
			printf("%s: dropping data, no rx fn\n", __FUNCTION__);

		if (buf != static_buf)
			free(buf);
	}

	return 0;
}

static int
chardev_read(chardev_bus_t *cbus, void* buf, int len)
{
	int n;
	int offset;

	offset = 0;

	while (offset < len) {
		n = read(cbus->fd, buf + offset, len - offset);
		if (n > 0)
			offset += n;
		else if (n == 0)
			return -1;	/* EOF */
		else if (n == -1) {
			perror("chardev_read");
			return errno;
		}
	}

	return 0;
}

void
chardev_detach(chardev_bus_t* cbus)
{
	ASSERT(cbus);

	if (cbus->fd)
		close(cbus->fd);

	MFREE(cbus->osh, cbus, sizeof(chardev_bus_t));

	return;
}

void
chardev_register_callback(chardev_bus_t* cbus,
                          chardev_rx_fn_t rx_data, void *rx_context)
{
	cbus->rx_fn = rx_data;
	cbus->rx_context = rx_context;
}

int
chardev_send(chardev_bus_t* cbus, void* data, uint len)
{
	ssize_t ret;
	uint8 *p;

	while (len != 0) {
		ret = write(cbus->fd, data, len);

		if (ret == -1) {
			if (errno == EINTR)
				continue;
			perror("chardev_send");
			return ret;
		}

		len -= ret;

		p = data;
		p += ret;
		data = p;
	}

	return 0;
}
