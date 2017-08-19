/*
 * $Id$
 * Includes for wire functions
 */

extern void WireSetup (void);
extern char * WireClientRx (int *len);
extern int WireClientTx (char *pkt, int len);
extern int Wiredrop;
extern int Wiredropcnt;
