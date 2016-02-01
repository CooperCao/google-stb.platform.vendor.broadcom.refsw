/***************************************************************************
 *     Copyright (c) 2008-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * 	Sample Code for sending HTTP Get Request and writing received data to disk.
 *  
 * 
 *************************************************************/ 
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#define SERVER_PORT	5000
#define BUF_MASK	0x1ff
#define BUF_SIZE 	(188*7*64)

void usage()
{
    printf("Usage: http_test_client: -d <srv-ip> -p <srv-port> -f <content-file> [-h -a -v]\n");
    printf("options are:\n");
    printf(" -d <srv-ip>   # IP address of HTTP Server (e.g. 192.168.1.110)\n");
    printf(" -p <srv-port> # Port of HTTP Server (e.g. 5000)\n");
    printf(" -f <file>     # url of stream; /data/videos prepended (e.g. portugal.mpg) \n");
    printf(" -v            # print periodic stats (default: no)\n");
    printf(" -h            # prints http_test_client usage\n");
    printf("\n");
}

double difftime1(struct timeval *start, struct timeval *stop)
{
	double dt = 1000000.*(stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec);
	return dt;
}

/* This function creates a TCP connection to server and returns the socket descriptor */
int TcpConnect(char *server, int port )
{
	int sd,rc;

	struct sockaddr_in localAddr;
	char portString[16];
	struct addrinfo hints;
	struct addrinfo *addrInfo = NULL;
	struct addrinfo *addrInfoNode = NULL;

 	printf("TCP - Connection to %s:%d ...\n",server, port);



	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* we dont know the family */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    memset(portString, 0, sizeof(portString));  /* getaddrinfo() requires port # in the string form */
    snprintf(portString, sizeof(portString)-1, "%d", port);

	if (getaddrinfo(server, portString, &hints, &addrInfo) != 0) {
        printf("\nERROR: getaddrinfo failed for server:port: %s:%d\n", server, port);
        perror("getaddrinfo");
        return -EINVAL;
    }
	
	for (addrInfoNode = addrInfo; addrInfoNode != NULL; addrInfoNode = addrInfo->ai_next) {
        if (addrInfoNode->ai_family == AF_INET)
            break;
    }
    if (!addrInfoNode) {
        perror("%s: ERROR: no IPv4 address available for this server, no support for IPv6 yet");
        return -EINVAL;
    }

	addrInfo = addrInfoNode;


	/* Open Socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd<0) {
		perror("Socket Open Err");
		return -EINVAL;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(0);

	if (bind(sd,(struct sockaddr *) &localAddr, sizeof(localAddr))) {
		perror("Socket Bind Err");
		return -EINVAL;
	}

	rc = connect(sd, addrInfo->ai_addr, addrInfo->ai_addrlen);
	
	if (rc<0) {
		perror("Connect Error: Server busy or unavailable:");
		return -EBUSY;
	}

	printf("TCP: Connection Success\n");
	return sd;
}

#define PKTS_PER_READ 64
#define MAX_BUFFER_SIZE (1316 * PKTS_PER_READ)
#define HTTP_POST_SIZE 2048
#define CMND_BUFF_SIZE 256
#define TEMP_BUF_SIZE 512

int main(int argc, char **argv)
{
	char *post;
	char *server = NULL;
	char *fname = NULL;
	int maxrate = 19;
	int c;
	int sd;
	FILE *fp = NULL;
	char *p;
	char *rbuf;
	char *chkSum1;
	char cmndBuf[CMND_BUFF_SIZE] = "md5sum ";
	char tempBuf[TEMP_BUF_SIZE];
	char *chkSum;
	int  port = 0;
	ssize_t n;
	unsigned char *buf, *xbuf;
	ssize_t bytes = 0;
	unsigned long total=0;
	unsigned long count=0;
	char tok;
	ssize_t len;
	ssize_t offset;
	double dt;
	struct timeval start, stop;
	int verbose = 0;

	int readFailCount = 0;
	FILE *ptr;


    while ((c = getopt(argc, argv, "d:p:f:amvh")) != -1) {
        switch (c) {
        case 'd':
            server = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            fname = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
        default:
            usage();
            return -1;
        }
    }

	if (port == 0 || server == NULL || fname == NULL) {
		printf("Missing Args...\n");
		usage();
		exit(1);
	}

	printf("Server %s, Port %d, File %s, Maxrate %f\n",
			server, port, fname, maxrate);

	xbuf = (unsigned char *) malloc(BUF_SIZE + BUF_MASK);
	rbuf = (char *) malloc(1024);
	post = (char *) malloc(HTTP_POST_SIZE);

	if (xbuf == NULL || rbuf == NULL || post == NULL) {
		perror("malloc failure\n");
		exit(-1);
	}

	buf = (unsigned char *) (((unsigned long)xbuf + BUF_MASK) & ~BUF_MASK);

	/* Build HTTP Get Request */
	n = snprintf(post, HTTP_POST_SIZE,
			"GET /%s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
			"Rate: %d\r\n"
			"PlaySpeed.dlna.org: speed=1\r\n"
			"User-Agent: Test App\r\n"
			"\r\n",
			fname, server, port, maxrate);
	printf("Sending HTTP Request:----->[\n%s]\n", post);

	/* open file for writing */
	
	fp = fopen(fname,"wb");
	if (!fp) {
		perror("File open error:\n");
		exit(-1);
	}
	
	
	/* Connect to Server */
	sd = TcpConnect(server, port);
	if (sd < 0) {
			printf("Connection to Server Failed, Exiting...\n");
			exit(-1);
	}
	/* Send HTTP Get Request */
	n = write(sd, post, strlen(post));
	if ((unsigned)n != strlen(post)) {
		printf("Failed to write HTTP Get Request: rc %d\n", n);
		perror("write(): ");
		exit(-1);
	}

	printf("Succesfully Send HTTP Get Request to to %s:%d\n", server, port);
	usleep(10000);

	/* Read HTTP Response */
	memset(rbuf, 0, 1024);
	if ( (n = read(sd, rbuf, 1024)) <= 0) {
		printf("Failed to read HTTP Response: rc = %d\n", n);
		perror("read(): ");
		exit(-1);
	}

    rbuf[n] = '\0'; 

	/* Scan for end of HTTP header */
	p = strstr(rbuf,"\r\n\r\n");
	if(!p) {
		printf("No HTTP Header\n");
		len = 0;
		p = rbuf;
		tok = 0;
	}
	else {
		p+=4;
		tok = *p;
		*p = 0;
		len = strlen(rbuf);
	}

	printf("Total response len %d, payload len %d\n", n, n-len);
	printf("HTTP Response: -----> [\n%s]\n", rbuf);
	*p = tok;

	 /**	Extracting Checksum from response header		**/
	offset = sizeof("Checksum:") - 1;
	chkSum1 = strstr(rbuf,"Checksum:");
	chkSum1 = strndup(chkSum1+ offset,32);
	printf("Received checksum value is %s\n", chkSum1);

	
	

	/* write any data that was read part of the initial read */
	if (n>len) {
		if ( (total = write(fileno(fp), p, n - len)) <= 0 ) {
			printf("Failed to write initial payload bytes (%lu)\n", total);
			perror("write():\n");
			exit(-1);
		}
	}

		
	sleep(1);

	gettimeofday(&start, NULL);
	/* read data from network & write to the file */
	while(1) {
		if ( (n = read(sd, buf, BUF_SIZE)) <= 0) {
			gettimeofday(&stop, NULL);
			printf("read failed: n = %d\n", n);
			perror("read(): ");
            usleep(10000);
            readFailCount++;
            if(readFailCount == 30)
            {
				goto	http_test_client_exit;
            }
            continue;
			break;
		}

		gettimeofday(&stop, NULL);
		
		if (((bytes = write(fileno(fp), buf, n)) <= 0) || bytes != n) {
			printf("Failed to write payload bytes (%d)\n", bytes);
			perror("write():\n");
			break;
		}

after_write:
		dt = difftime1(&start, &stop);
		total += bytes;
		count++;
		if (verbose && (count % 100) == 0) {
			printf("[%10lu] Received %10lu Bytes in dt = %10.2fusec at rate %2.1f\n", 
					count, total, dt, (total * 8. / dt));  
		}
	}


	
http_test_client_exit:	

		
	strcat(cmndBuf,fname);

	printf("\n COMPUTING CHECKSUM OF THE RECEIVED FILE ******** %s \n", fname);

	if((ptr = popen(cmndBuf,"r")) != NULL)
	{
		printf("\n Md5Sum output ====================================\n");
		while (fgets(tempBuf, TEMP_BUF_SIZE, ptr) != NULL)
					  (void) printf("%s\n", tempBuf);
		if(ptr != NULL)
			(void) pclose(ptr);	

		 chkSum = strtok(tempBuf," ");

		 printf("%s\n", chkSum);
	}
	else
	{
		printf("\nChecksum computation failed popen Failed --------------------\n");
		return -1;
		
	}

	if(strcmp(chkSum,chkSum1)==0)
	{
		printf("\n Yeeeeeeeeeee Check sum  matched -- Passed the test\n");

	}
	else
	{
		printf("\n Oh noooo Check sum didn't match -- Test Failed  \n");

	}
		

	dt = difftime1(&start, &stop);
	printf("Final stats: Received %10lu bytes to %s file in %10.1fusec at %2.1f rate\n", 
			total, fname, dt, (total * 8. / dt));
	fclose(fp);
	free(xbuf);
	free(rbuf);
    free(post);
	return 0;
}
