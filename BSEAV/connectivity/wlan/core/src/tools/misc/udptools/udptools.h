#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT_DEFAULT		6000
#define TOS_DEFAULT		0x00
#define COUNT_DEFAULT		1
#define PKTSIZE_MAX		65536
#define PKTSIZE_DEFAULT		64
#define SEQ_NO_DEFAULT		1
