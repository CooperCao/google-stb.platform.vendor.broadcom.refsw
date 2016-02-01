/***********************************************************************/
/*                                                                     */
/*   MODULE:  if_net.h                                                 */
/*   DATE:    05/16/02                                                 */
/*   PURPOSE: network interface ioctl definition                       */
/*                                                                     */
/***********************************************************************/
#ifndef _IF_NET_H_
#define _IF_NET_H_

typedef struct NETFILTER
{
	uint32_t ip_addr;
	uint32_t port;
} NETFILTER;

/*
 * Ioctl definitions.
 */

#define SIOCGLINKSTATE      (SIOCDEVPRIVATE + 0)
#define SIOCSCLEARMIBCNTR   (SIOCDEVPRIVATE + 1)
#define SIOCSNETFILTER		(SIOCDEVPRIVATE + 2)
#define SIOCGETMBOX         (SIOCDEVPRIVATE + 3)
#define SIOCPUTWINDOW       (SIOCDEVPRIVATE + 4)
#define SIOCGETWINDOW       (SIOCDEVPRIVATE + 5)
#define SIOCDOCSISNONVOL	(SIOCDEVPRIVATE + 6)
#define SIOCBULKDMA (SIOCDEVPRIVATE + 7)
#define SIOCPREPAREFORRESET (SIOCDEVPRIVATE + 8)

#endif /* _IF_NET_H_ */
