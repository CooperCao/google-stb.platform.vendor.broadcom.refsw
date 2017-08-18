/*
 * Copyright 1997 Epigram, Inc.
 *
 * $Id$
 *
*/

#ifndef _pcisim_h_
#define _pcisim_h_


/*
 * PCI memory starts at PCI_MEMBASE and continues for PCI_MEMSIZE bytes.
 */
#define	PCI_MEMBASE	0x0
#define	PCI_MEMSIZE	(1024 * 1024 * 4)
#define	PCI_RE10BUS	0
#define	PCI_RE10SLOT	0
#define PCI_CONFIG_BASE     0xf0000000
#define	PCI_RE10CFG	(PCI_CONFIG_BASE \
			 | PCI_CONFIG_ADDR(PCI_RE10BUS, PCI_RE10SLOT, 0, 0))
#define	PCI_CODECCFG	(PCI_CONFIG_BASE \
			 | PCI_CONFIG_ADDR(PCI_RE10BUS, PCI_RE10SLOT, 1, 0))

/* pci_sendinterrupt() */
#define	INTA_ASSERT	0
#define	INTA_DEASSERT	1

/*
 * The pci message binary format to talk to the episocket package for leapfrog.
 */
#include <epipci.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Exported pci interface prototypes.
 */
int pci_client_init(char *host, int port);
int pci_server_init(int *);
int pci_read(uint32 pci_addr, uchar *buf, uint nbytes, uint cfgread);
int pci_write(uint32 pci_addr, uchar *buf, uint nbytes, uint cfgwrite);
void pci_copyin(uchar *src, uint32 dst, uint nbytes);
void pci_cfg_copyin(uchar *src, uint32 dst, uint nbytes);
void pci_cfg_copyout(uint32 src, uchar *dst, uint nbytes);
void pci_copyout(uint32 src, uchar *dst, uint nbytes);
int pci_reply(pci_transaction_t *t);
int pci_recv(pci_transaction_t *t, int s);
int pci_sendinterrupt(uint value);
void pci_intr(void);
void pci_print(pci_transaction_t *t, uint request);
int pci_kill(void);
void pci_killme(void);

extern int socketstarted;

#ifdef __cplusplus
}
#endif

extern unsigned int pci_intr_asserted;
extern int pciverbose;

#endif /* _pcisim_h_ */
