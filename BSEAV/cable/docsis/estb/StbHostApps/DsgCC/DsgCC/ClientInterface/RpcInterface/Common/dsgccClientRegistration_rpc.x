   const MAXCLIENTNAMESIZE = 32;
   const MACADDRSIZE = 6;

struct dsgClientRegInfo {
  int clientProgNum;
  int clientPort;
  int idType;
  int clientId;
  char clientName[MAXCLIENTNAMESIZE];
  
  int handle;
};

struct dsg_classifier {
	unsigned char source_ip_used;
	unsigned char source_ip_mask_used;
	unsigned char dest_ip_used;
	unsigned char port_used;
	unsigned int source_ip_adr;
	unsigned int source_ip_mask;
	unsigned int dest_ip_adr;
	unsigned short port_start;
	unsigned short port_end; 
};

struct dsgClientClassifierRegInfo {
	struct dsgClientRegInfo dsgClient_RegInfo; 
	struct dsg_classifier dsgClassifier;
};

program DSGREGISTRAR {
  version DSGREGISTRARVERS {
    int REGISTERDSGCLIENT(struct dsgClientRegInfo) = 1;
    int UNREGISTERDSGCLIENT(struct dsgClientRegInfo) = 2;
    int KEEPALIVEDSGCLIENT(struct dsgClientRegInfo) = 3;
    int REGISTERDSGCLIENTWITHCLASSIFIER(struct dsgClientClassifierRegInfo) = 4;    
  } = 1;
} = 0x12081950;
