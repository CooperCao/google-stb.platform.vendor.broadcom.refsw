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

#define SERVER_PORT	9000

char xml_header_template[] = 	"POST /ContentDirectory/Control HTTP/1.1\n"
								"Accept: */*\n"
								"Accept-Language: en-us\n"
								"Referer: http://192.168.1.106:9000/ContentDirectory/Events\n"
								"User-Agent: uMediaBrowser UPnP/1.0\n"
								"Content-Type: text/xml; charset=\"utf-8\"\n"
								"soapaction: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\n"
								"Accept-Encoding: gzip, deflate\n"
								"Host: 192.168.1.106:9000\n"
								"Content-Length: %d\n"
								"Connection: Keep-Alive\n"
								"Cache-Control: no-cache\r\n\r\n";

char media[] = "0\\Movie\\";

char xml_body_template[] = 		"<?xml version=\"1.0\"?>"
								"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
								"<s:Body>"
								"<u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
								"<ObjectID>%s</ObjectID>"
								"<BrowseFlag>BrowseDirectChildren</BrowseFlag>"
								"<Filter>*</Filter>"
								"<StartingIndex>0</StartingIndex>"
								"<RequestedCount>6</RequestedCount>"
								"<SortCriteria></SortCriteria>"
								"</u:Browse>"
								"</s:Body>"
								"</s:Envelope>";

typedef struct upnp_video_item_t {
	char objid[256];
	char parentid[256];
	char title[256];
	char server[256];
	char uri[256];
	unsigned long port;
	unsigned long content_length;
} upnp_video_item_t;


/* This function matches everything between begin and end */
char *match(char *input, char *begin, char *end, int *offset, int *size){

	char *p, *q, *r;
	*size = 0;
	*offset = 0;
	
	p = strstr(input,begin);
	if(!p) return NULL;

	q = strstr(p+strlen(begin),end);
	if(!q) return NULL;

	*size = q - p - strlen(begin);
	r = (char *)malloc(*size+1);
	if(!r) return NULL;
	
	strncpy(r,p+strlen(begin),*size);
	*offset = p+strlen(begin) - input;
	r[*size]=0;
	return r;

}

/* This function looks for string item, and /item and returns what is between */
char *get_item(char *input, char *item, int *offset, int *size){
	char begin[128],end[128];
	char *p, *q, *r;
	strcpy(begin,item);
	strcpy(end+1,item);
	end[0]='/';
	return match(input,begin,end,offset,size);
}


int parse_upnp_video_item( upnp_video_item_t *v, char *input){
	char *m;
	int offset,size, consumed_offset;
	if(!v)
		return -1;
	
	v->uri[0]=0;
	m=match(input,"id=\"","\"",&offset,&size);
	if(m){
		sscanf(m,"%s",v->objid);
		//printf("objid=<<%s>>\n",v->objid);
		free(m);
	}
	else{
		v->objid[0]=0;
	}

	m=match(input,"parentID=\"","\"",&offset,&size);
	if(m){
		sscanf(m,"%s",v->parentid);
		//printf("parentid=<<%s>>\n",v->parentid);
		free(m);
	}
	else{
		v->parentid[0]=0;
	}

	m=match(input,"dc:title&gt;","&lt;/dc:title",&offset,&size);
	if(m){
		sscanf(m,"%s",v->title);
		//printf("title=<<%s>>\n",m,v->title);
		free(m);
	}
	else{
		v->title[0]=0;
	}

	m=match(input,"size=\"","\" ",&offset,&size);
	if(m){

		sscanf(m,"%d",&v->content_length);
		//printf("length=<<%d>>\n",v->content_length);
		free(m);
	}
	else{
		v->content_length=0;
	}

    m=match(input,"http://",":",&offset,&size);
	if(m){
		sscanf(m,"%s",v->server);
		//printf("server=<<%s>>\n",v->server);
		free(m);
	}
	else{
		v->server[0]=0;
	}

	consumed_offset = offset + strlen(v->server);
	m=match(input+consumed_offset,":","/",&offset,&size);
	if(m){
		sscanf(m,"%d",&v->port);
		//printf("port=<<%d>>\n",v->port);
		free(m);
	}
	else{
		v->port=80;
	}

	consumed_offset+=offset;
	m=match(input+consumed_offset,"/","&lt",&offset,&size);
	if(m){
		sprintf(v->uri,"/%s",m);
		//printf("uri=<<%s>>\n\n",v->uri);
		free(m);
	}
	else{
		v->uri[0]=0;
	}

	return 0;
}

char *get_upnp_video_items( int sd, char *parent, upnp_video_item_t *v ){

	char xml_send_hdr[2048];
	char xml_send_body[2048];
	char xml_recv_hdr[8192];
	char xml_recv_body[8192];
	int  n,nbytes,content_length,offset,size;
	char *m,temp;
	char *data,*container;
	int consumed=0;

	sprintf(xml_send_body,xml_body_template,parent);
	sprintf(xml_send_hdr,xml_header_template,strlen(xml_send_body));
	
	write(sd,xml_send_hdr,strlen(xml_send_hdr));
	write(sd,xml_send_body,strlen(xml_send_body));
	usleep(10000);
	nbytes = read(sd,xml_recv_hdr,8192);
	m = match(xml_recv_hdr,"Content-Length: ","\n",&offset,&size);
	sscanf(m,"%d",&content_length);
	//printf("Content Length = %d\n",content_length);
	free(m);
	
	nbytes = read(sd,xml_recv_body,content_length);
	//printf("%d <<%s>>\n",nbytes,xml_recv_body);

	//printf("<<%s>>\n",xml_recv_body);

	data = xml_recv_body;
	consumed=0;
	while(1){
		m = get_item(data+consumed,"item",&offset,&size);
		if(m){
			temp=data[offset+size];
			data[consumed+offset+size]=0;
			n = parse_upnp_video_item( v, data+consumed+offset);
			data[consumed+offset+size]=temp;
			consumed+=offset+size+strlen("item")+1;
			v++;
			free(m);		
		}
		else
			break;
	}

	return NULL;

}

void recv_http_media(int sd, char *url){

	int nbytes,total,cnt;
	double dt;
	struct timeval t0,t;	
	char read_buf[2048],send_buf[1024];
	/* Now do HTTP/XML */
	sprintf(send_buf,"GET %s HTTP/1.1\nContent-Length: 0\r\n\r\n",url);
	printf("<<%s>>\n",send_buf);

	write(sd,send_buf,strlen(send_buf));
	usleep(10000);
	total=0;
	cnt=0;
	gettimeofday(&t0,NULL);
	
	while( 1){
		nbytes= read(sd,read_buf,2048);
		if(nbytes<=0)
			break;
		total+=nbytes;
		cnt++;
		if(cnt%3000==0){
			gettimeofday(&t,NULL);
			dt = 100.*((t.tv_sec - t0.tv_sec) + 
				(t.tv_usec - t0.tv_usec)/1000000.);
			printf("total=%9d time=%10.3lf rate=%10.5lf\n",total,dt,8.*total/dt);
		}
	}
}


char *get_upnp_container( int sd, char *parent){

	char xml_send_hdr[2048];
	char xml_send_body[2048];
	char xml_recv_hdr[8192];
	char xml_recv_body[8192];
	int  nbytes,content_length,offset,size;
	char *m;
	char *container;

	sprintf(xml_send_body,xml_body_template,parent);
	sprintf(xml_send_hdr,xml_header_template,strlen(xml_send_body));
	
	write(sd,xml_send_hdr,strlen(xml_send_hdr));
	write(sd,xml_send_body,strlen(xml_send_body));
	usleep(10000);
	nbytes = read(sd,xml_recv_hdr,8192);
	//printf("%d <<%s>>\n",nbytes,xml_recv_hdr);
			
	m = match(xml_recv_hdr,"Content-Length: ","\n",&offset,&size);
	sscanf(m,"%d",&content_length);
	//printf("Content Length = %d\n",content_length);
	free(m);
	
	nbytes = read(sd,xml_recv_body,content_length);
	//printf("%d <<%s>>\n",nbytes,xml_recv_body);

	container = match(xml_recv_body,"container id=\"","\"",&offset,&size);
	if(container){
		//printf("OBJ=[%s]\n",container);
		return container;
	}

	return NULL;
}

int main(int argc, char **argv){	
	int i=0,sd, fd, cliLen;
	struct sockaddr_in localAddr, servAddr;
	
	int nbytes,rc,read_len, send_len;
	struct hostent *h;
	int total,cnt;
   
	char url[256];
	char *container;
	upnp_video_item_t video[12];

	if(argc < 2){
		printf("Usage: tcpclient host_ip_addr\n");
		exit(0);
	}

	h = gethostbyname(argv[1]);
	if (h == NULL)
	{
		perror("Unknown Host");
		return;
	}

	servAddr.sin_family = h->h_addrtype;
	memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0],h->h_length);
	servAddr.sin_port = htons(SERVER_PORT);

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd<0)
	{
		/* Socket Create Error */
		perror("Socket Open Err");
		return;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(0);

	if (bind(sd,(struct sockaddr *) &localAddr, sizeof(localAddr)))
	{
		perror("Socket Bind Err");
		return;
	}

	/* Connect to server */
	rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if (rc<0)
	{
		perror("Connect Error");
		return;
	}

	printf("Connected to Server.......\n");

#if 0
	container = get_upnp_container( sd,  "0\\Movie\\");
	get_upnp_video_items( sd, container, video);

	for(i=0;video[i].uri[0];i++)
		printf("URI=%s\n",video[i].uri);
#endif
	strcpy(url,"/disk/video/O3$536870962$2013265922.mpg");
	printf("URL=%s\n",url);

	recv_http_media(sd, url);

	return 0;
}



