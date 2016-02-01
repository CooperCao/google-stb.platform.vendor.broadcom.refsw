/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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
 * Module Description: Sample Code for sending HTTP POST
 *  
 * Revision History:
 *    
 * $brcm_Log: $
 * 
 *************************************************************/ 
#define _FILE_OFFSET_BITS 64
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>

#define SERVER_PORT	6000

/* This function creates a TCP connection to server and returns the socket descriptor */
int TcpConnect(char *server, int port)
{

        int sd,rc;

        struct sockaddr_in localAddr, servAddr;
        struct hostent *h;

        printf("TCP - Connection to %s:%d ...\n",server,port);

        h = gethostbyname(server);
        if (h == NULL)
        {
                perror("Unknown Host");
                return;
        }

        servAddr.sin_family = h->h_addrtype;
        memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0],h->h_length);
        servAddr.sin_port = htons(port);

        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd<0)
        {
                /* Socket Create Error */
                perror("Socket Open Err");
                return -EINVAL;
        }

        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(0);

        if (bind(sd,(struct sockaddr *) &localAddr, sizeof(localAddr)))
        {
                perror("Socket Bind Err");
                return -EINVAL;
        }

        /* Connect to server */
        rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
        if (rc<0)
        {
                perror("Connect Error: Server busy or unavailable");
                return -EBUSY;
        }

        printf("TCP: Connection Success\n");
        return sd;
}

int main(int argc, char **argv){
	int  fd;
	char post[512];
	int  pcrpid = 0x31;
	int  client_id = 0;
	char server[128];
	char sync = 0x47;
	int  i,port,n;
	unsigned char *buf;
	char fname[64];
	unsigned long bytes=0;
	unsigned long count=0;
	buf = (unsigned char *)malloc(512*188);

	sprintf(post,"GET /%s HTTP/1.1\r\n\r\n",argv[3]);
	FILE *fp;

	if(argc < 2){
		printf("Usage: tcpclient server port url\n");
		printf("Usage: tcpclient 192.168.1.10 5000 /test.mpg\n");
		return 0;
	}

	//strcpy(fname,argv[3]+1);
	strcpy(fname,argv[3]);
	fp  = fopen(fname,"wb");
	
	port = atoi(argv[2]);
	strcpy(server,argv[1]);

	fd = TcpConnect(server,port);
	if(fd>0){
		printf("Connected\n");
		write(fd,post,strlen(post));
	}
	usleep(50000);

	n = read(fd,buf,1024);
	buf[n] = 0;
	printf("[%s]\n",buf);

	/* Read first data bytes */
	n = read(fd,buf,188*512);
	for(i=0;i<n;i++){
		if(buf[i]==0x47)
			break;
	}

	printf("offset=%d len=%d\n",i,n-i);
	if(i==n){
		printf("Sync Not found\n");
		write(fileno(fp),buf,n);
	}
	else{
		printf("Sync found at byte=%d\n",i);
		write(fileno(fp),buf+i,n-i);
	}
	
	while(1){
		n = read(fd,buf,188*512);
		bytes += write(fileno(fp),buf,n);
		count ++;
		if((count % 1000) == 0)
			printf("%u (%u) Packets (Bytes) written\n",count,bytes);  
	}

	free(buf);

}

