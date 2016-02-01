//
// SAP Fixed Header Format
//
typedef struct _sap {
  unsigned char compressed:1;
  unsigned char encrypted:1;
  unsigned char messageType:1;
  unsigned char rsvd:1;
  unsigned char addressType:1;
  unsigned char version:3;
  unsigned char authLen;
  unsigned short msgidHash;
  unsigned int sourceAddr;
} SAPHeader;

