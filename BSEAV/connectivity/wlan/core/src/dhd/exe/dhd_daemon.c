/*
 * Code for DHD daemon to handle timeouts
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#include <stdio.h>
#include <malloc.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <stdlib.h>
#include <dhd_daemon.h>

//#define DHDD_DEBUG
#define TRAP_COMMAND "\a" /* Trap command */

#ifdef DHDD_DEBUG
#define dhdd_printf printf
#else
#define dhdd_printf(fmt...)
#endif

struct msghdr msg;
static int uart_fd = 0;
static int sock_fd = 0;

static void
print_help(void)
{
	printf("Usage: dhdd <UART>\n");
	printf("Eg. dhdd /dev/ttyUSB1\n");
}

static int
tx_data(int uart_fd, char *tx_buf, int size)
{
	int count = 0;

	if (uart_fd != -1) {
		count = write(uart_fd, tx_buf, size);
	}
	return count;
}

int
main(int argc, char **argv)
{
	char *uart;
	struct termios tty_opt;
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	bcm_to_info_t *to_info;
	bcm_to_info_t cmd;
	int ret_val = -1;

	if (argc < 2) {
		print_help();
		goto exit;
	}

	(void)*argv++;
	uart = *argv;

	uart_fd = open(uart, O_RDWR | O_NOCTTY | O_NDELAY);
	if (uart_fd == -1) {
		printf("Error:Unable to open %s\n", uart);
		print_help();
		goto exit;
	}

	/* Initialise UART with 3000000 8N1 */
	if ((tcgetattr(uart_fd, &tty_opt)) != 0) {
		printf("TTY getattr Failed\n");
		goto exit1;
	}

	tty_opt.c_cflag = B3000000 | CS8 | CLOCAL | CREAD;
	tty_opt.c_iflag = IGNPAR;
	tty_opt.c_oflag = 0;
	tty_opt.c_lflag = 0;
	if ((tcflush(uart_fd, TCIFLUSH)) != 0) {
		printf("TTY tcflush Failed\n");
		goto exit1;
	}
	if ((tcsetattr(uart_fd, TCSANOW, &tty_opt)) != 0) {
		printf("TTY setattr Failed\n");
		goto exit1;
	}

	sock_fd = socket(PF_NETLINK, SOCK_RAW, BCM_NL_USER);
	if (sock_fd < 0) {
		printf("Socket Creation Failed\n");
		goto exit1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	memset(&dest_addr, 0, sizeof(dest_addr));

	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */

	ret_val = bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
	if (ret_val != 0) {
		printf("Bind Failed\n");
		goto exit1;
	}

	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(bcm_to_info_t)));
	if (nlh == NULL) {
		printf("Header allocation failed\n");
		goto exit1;
	}
	memset(nlh, 0, NLMSG_SPACE(sizeof(bcm_to_info_t)));
	nlh->nlmsg_len = NLMSG_SPACE(sizeof(bcm_to_info_t));
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	cmd.magic = BCM_TO_MAGIC;
	cmd.reason = REASON_DAEMON_STARTED;
	cmd.trap = 0;

	memcpy(NLMSG_DATA(nlh), (void *)&cmd, sizeof(bcm_to_info_t));

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Inform Kernel that DHD Daemon started */
	dhdd_printf("Sending start event to kernel\n");
	ret_val = sendmsg(sock_fd, &msg, 0);
	if (ret_val == -1) {
		printf("Data send Failed\n");
		goto exit1;
	}
	dhdd_printf("Done\n");

	while (1) {
		/* Read message from kernel */
		ret_val = recvmsg(sock_fd, &msg, 0);
		if (ret_val == -1) {
			continue;
		}
		/* Process received message */
		to_info = (bcm_to_info_t *)NLMSG_DATA(nlh);
		if (to_info->magic != BCM_TO_MAGIC) {
			dhdd_printf("Bad Magic Number\n");
			continue;
		}
		printf("Message %d Received\n", to_info->reason);
		switch (to_info->reason)
		{
			case REASON_COMMAND_TO:
				dhdd_printf("DHDD:Command Time Out\n");
				if (to_info->trap == DO_TRAP) {
					dhdd_printf("Doing FW TRAP\n");
					ret_val = tx_data(uart_fd, TRAP_COMMAND,
						sizeof(TRAP_COMMAND));
					if (ret_val == -1) {
						printf("Tx data Failed\n");
						goto exit1;
					}
				}
				break;
			case REASON_OQS_TO:
				dhdd_printf("DHDD:OQS Time Out\n");
				if (to_info->trap == DO_TRAP) {
					dhdd_printf("Doing FW TRAP\n");
					ret_val = tx_data(uart_fd, TRAP_COMMAND,
						sizeof(TRAP_COMMAND));
					if (ret_val == -1) {
						printf("Tx data Failed\n");
						goto exit1;
					}
				}
				break;
			case REASON_SCAN_TO:
				dhdd_printf("DHDD:SCAN Time Out\n");
				if (to_info->trap == DO_TRAP) {
					dhdd_printf("Doing FW TRAP\n");
					ret_val = tx_data(uart_fd, TRAP_COMMAND,
						sizeof(TRAP_COMMAND));
					if (ret_val == -1) {
						printf("Tx data Failed\n");
						goto exit1;
					}
				}
				break;
			case REASON_JOIN_TO:
				dhdd_printf("DHDD:JOIN Time Out\n");
				if (to_info->trap == DO_TRAP) {
					dhdd_printf("Doing FW TRAP\n");
					ret_val = tx_data(uart_fd, TRAP_COMMAND,
						sizeof(TRAP_COMMAND));
					if (ret_val == -1) {
						printf("Tx data Failed\n");
						goto exit1;
					}
				}
				break;
			case REASON_DAEMON_STARTED:
				dhdd_printf("DHDD:Kernel Daemon ACKED\n");
				break;
			default:
				dhdd_printf("DHDD:Unknown Reason\n");
		}
	}
exit1:
	close(uart_fd);
	if (sock_fd) {
		close(sock_fd);
	}
	if (nlh) {
		free(nlh);
		nlh = NULL;
	}
exit:
	dhdd_printf("DHD Daemon exited\n");
	return 0;
}
