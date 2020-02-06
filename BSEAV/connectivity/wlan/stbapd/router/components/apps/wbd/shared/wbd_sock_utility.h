/*
 * WBD socket utility for both client and server (Linux)
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_sock_utility.h 690491 2017-03-16 11:22:30Z mateen $
 */

#ifndef _WBD_SOCK_UTILITY_H_
#define _WBD_SOCK_UTILITY_H_

#include <net/if.h>
#include <arpa/inet.h>
#include <linux/rtnetlink.h>

#define INVALID_SOCKET		-1
#define MAX_READ_BUFFER		1448
#define WBD_BUFFSIZE_4K		4096

#define NL_SOCK_BUFSIZE		8192

extern void wbd_close_socket(int *sockfd);

extern int wbd_connect_to_server(char *straddrs, unsigned int nport);

extern int wbd_socket_send_data(int sockfd, char *data, unsigned int len);

extern int wbd_socket_recv_data(int sockfd, char **data);

extern int wbd_socket_recv_bindata(int sockfd, char *data, unsigned int dlen);

extern int wbd_open_eventfd(int portno);

extern int wbd_open_server_fd(int portno);

extern int wbd_accept_connection(int server_fd);

extern int wbd_read_nl_sock(int sockFd, char *bufPtr, int seqNum, int pId);

/* Get the IP address from socket FD */
extern int wbd_sock_get_ip_from_sockfd(int sockfd, char *buf, int buflen);

#endif /* _WBD_SOCK_UTILITY_H_ */
