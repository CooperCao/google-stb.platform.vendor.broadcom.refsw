
		BCM4710 BSP VxWorks Readme.


This file documents the directory structure and contents of the BSP
for the BCM4710.
The development environment is assumed to be a PC running Windows
with the Tornado 2.1 development environment already installed and
configured properly.

For more information on this release please consult the file
BCM47XX_Resource_Guide.pdf in the doc directory.

Contents of the distribution.

   bcm47xx/bsp/bridge

    This directory contains the VxWorks object files to instantiate
    the bridging functionality

   bcm47xx/bsp/il

	Sources for the iLine32 driver.

   bcm47xx/bsp/et

	Sources for the 10/100 Ethernet driver.

   bcm47xx/bsp/shared
   bcm47xx/bsp/include

	Common code and include files used by all of the above.

   bcm47xx/bootrom/pdiag

	Portable Power On Self Test for the BCM4710.

   bcm47xx/bootrom/pmon

	Source for the MIPS monitor "pmon". This code is based on version
	5.3.22 from carmel.com with extensive modification by HNBU to
	tailor it to the BCM4710 and integrate it with POST.

   bcm47xx/bin

	A binary image for the combined kernel and root file system
	processed with gzip and trx that can be burned directly into
	flash on a BCM94710D board.

   bcm47xx/doc

	The Resource Guide.

   bcm47xx/tools/visionICE

	Files needed to use WindRiver visionICE with the BCM4710.

   bcm47xx/tools

	Miscelaneous tools needed to build the ROM images.

Difference Files

The following diff output can be used to update the files provided by WindRiver so
that they will function correctly in the BCM4710 environment.  Use the "patch" 
utility to update the files:
    - Copy and paste the diff output below for a given file in to a new text
      file.  For instance, copy the text in the changes for fei82557End.c section
      below to a file called pci_c.diff
    - Be sure the difference file is in the same folder as the file to be updated
    - Make a backup of the original file
    - Enter the command "patch fei82557End.c pci_c.diff"
    - The fei82557End.c file will be updated and will now compile and function
      correctly

PCI
The following is output using the utility 'diff' on files that have changed in
order to support using an Intel 82559-based PCI Ethernet adapter in the
system.

            ******** Changes to original fei82557End.c ********
230a231,233
> /* mush include config.h before fei82557End.h */
> #include "config.h"
> 
241a245,250
> #ifdef INCLUDE_PCI_INTEL_END
> #include "vxbsp.h"
> #include "vx_osl.h"
> #include "sbpci.h"
> #endif
> 
655c664
< 
---
> #ifndef INCLUDE_PCI_INTEL_END
656a666
> #endif
712a723,731
> #ifdef INCLUDE_PCI_INTEL_END
> LOCAL STATUS sys557Init (DRV_CTRL *pDrvCtrl);
> LOCAL UINT32 runEprmCmd(DRV_CTRL *pDrvCtrl, UINT32 cmd, int cmd_len);
> LOCAL char *feiAlignPktTo16(char *pkt, int len);
> LOCAL int feiBoardIntEnable(int unit);
> LOCAL int feiBoardIntDisable(int unit);
> extern void unmask_pci_interrupt(UINT32 bit);
> extern void mask_pci_interrupt(UINT32 bit);
> #endif
822c841,843
< 
---
> #ifdef INCLUDE_PCI_INTEL_END
> 	pDrvCtrl->board.phyAddr  = 32;
> #else
823a845
> #endif
830c852,855
< 
---
> #ifdef INCLUDE_PCI_INTEL_END
> 	if (sys557Init (pDrvCtrl) == ERROR)
> 	goto errorExit;
> #else
837a863
> #endif
923d948
< 
962a988,990
> #ifdef INCLUDE_PCI_INTEL_END
> 	cacheDmaFree (pDrvCtrl->pMemBase + BCM4710_SDRAM_SWAPPED);
> #else
963a992
> #endif
987a1017,1025
> #ifdef INCLUDE_PCI_INTEL_END
> 	pDrvCtrl->unit = atoi (initString);
> 	pDrvCtrl->pMemBase = (char *)NONE;	/* no preallocated memory */
> 	pDrvCtrl->memSize = NONE;	/* no preallocated memory */
> 	pDrvCtrl->nCFDs = 0;	/* use default */
> 	pDrvCtrl->nRFDs = 0;	/* use default */
> 	pDrvCtrl->flags = 0;	/* flag not used */
> 	pDrvCtrl->offset = 2;	/* IP header alignment */
> #else
1027c1065
< 
---
> #endif
1133a1172,1174
> #ifdef INCLUDE_PCI_INTEL_END
> 		pDrvCtrl->pMemBase = (char *)cacheDmaMalloc (size) + BCM4710_SDRAM_SWAPPED;
> #else
1134a1176
> #endif
1461a1504
> #ifndef INCLUDE_PCI_INTEL_END
1472a1516
> #endif
1536a1581,1600
> #ifdef INCLUDE_PCI_INTEL_END
> 	/* clear up pending interrupts before enabling interrupt */
> 	CSR_WORD_RD (CSR_STAT_OFFSET, tempVar);
>     if ((tempVar & SCB_S_STATMASK) != 0) 
> 	    /* clear chip level interrupt pending, use byte access */
> 	    CSR_BYTE_WR (CSR_ACK_OFFSET, ((tempVar & SCB_S_STATMASK) >> 8));
> 
>     /* acknowledge interrupts */
> 
>     SYS_INT_ACK (pDrvCtrl);
> 
>     /* enable chip interrupts after fei82557Reset disabled them */
> 
>     I82557_INT_ENABLE;
> 
>     /* enable system interrupts after fei82557Reset disabled them */
> 
>     SYS_INT_ENABLE (pDrvCtrl);
> #endif
> 
1631a1696,1698
> #ifdef INCLUDE_PCI_INTEL_END
> 	len = netMblkToBufCopy (pMblk, (char *) pEnetHdr - BCM4710_SDRAM_SWAPPED, NULL);
> #else	
1632a1700
> #endif
1994c2062,2066
<     pData = (char *) (RFD_PKT_ADDR (pRFD));
---
> #ifdef INCLUDE_PCI_INTEL_END
>     pData = (char *) (RFD_PKT_ADDR (pRFD)) - BCM4710_SDRAM_SWAPPED;
> #else
> 	pData = (char *) (RFD_PKT_ADDR (pRFD));
> #endif
1997a2070,2073
> #ifdef INCLUDE_PCI_INTEL_END
> 	/* have to align the packet to 16-bit boundary for mips ip stack */
> 	pData = feiAlignPktTo16(pData, count & ~0xc000);
> #else
1999a2076
> #endif
2003c2080
<     pMblk->mBlkHdr.mData	= pData;
---
> 	pMblk->mBlkHdr.mData	= pData;
2606c2683,2688
< 
---
> #ifdef INCLUDE_PCI_INTEL_END
> 		for (ix = 0; ix < FEI_HADDR_LEN (&pDrvCtrl->endObj); ix++) {
> 			FEI_BYTE_WR((UINT32)((char *) CFD_IA_ADDR (pCFD) + ix), 
> 				*((char *) FEI_HADDR (&pDrvCtrl->endObj) + ix));
> 		}
> #else
2609a2692
> #endif
2718a2802,2806
> #ifdef INCLUDE_PCI_INTEL_END
> 	UINT32 ix;
> 	for (ix = 0; ix < FEI_ADDR_LEN; ix++)
> 		FEI_BYTE_WR(((UINT32)mCastAddr + ix), *((char *) mCastNode->addr + ix));
> #else
2719a2808
> #endif
4023a4113,4217
> 
> #ifdef INCLUDE_PCI_INTEL_END
> LOCAL STATUS 
> sys557Init (DRV_CTRL *pDrvCtrl)
> {
> 	UINT8 int_vect;
> 	UINT32 i, j;
> 	UINT32 read_cmd, ee_size;
> 	UINT16 value, sum, eeprom[0x100];
> 	FEI_BOARD_INFO *pBoard = &pDrvCtrl->board;
> 	#define EE_READ_CMD 6
> 
> 	/* disable board interrupt for pci device */
> 	feiBoardIntDisable(0);
> 
> 	/* get board base address */
> 	pciConfigInLong(1, 2, 0, 0x10, &pBoard->baseAddr);
> 	pBoard->baseAddr = KSEG1ADDR(pBoard->baseAddr & ~((UINT32)3));
> 	pDrvCtrl->pCSR = (CSR_ID) pBoard->baseAddr;
> 
> 	/* get interrupt vector (pci uses shared interrupt(int 0) */
> 	pciConfigInByte(0, 5, 0, 0x3c, &int_vect);
> 	pBoard->vector = (UINT32)int_vect;
> 
> 	/* interrupt func. */
> 	pBoard->intEnable = (FUNCPTR)feiBoardIntEnable;
> 	pBoard->intDisable = (FUNCPTR)feiBoardIntDisable;
> 
> 	if ((runEprmCmd(pDrvCtrl, EE_READ_CMD << 24, 27) & 0xffe0000) == 0xffe0000) {
> 		ee_size = 0x100;
> 		read_cmd = EE_READ_CMD << 24;
> 	} else {
> 		ee_size = 0x40;
> 		read_cmd = EE_READ_CMD << 22;
> 	}
> 
> 	for (j = 0, i = 0, sum = 0; i < ee_size; i++) {
> 		value = runEprmCmd(pDrvCtrl, read_cmd | (i << 16), 27);
> 		eeprom[i] = value;
> 		sum += value;
> 		if (i < 3) {
> 			pBoard->enetAddr[j++] = value;
> 			pBoard->enetAddr[j++] = value >> 8;
> 		}
> 	}
> 
> 	if (sum != 0xBABA)
> 		printf("Invalid EEPROM checksum.\n");
> 
> 	return OK;
> }
> 
> LOCAL UINT32 
> runEprmCmd(DRV_CTRL *pDrvCtrl, UINT32 cmd, int cmd_len)
> {
> 	UINT16 dataval;
> 	UINT32 retval = 0;
> 	#define EE_ENB (0x4800 | FEI_EECS)
> 	#define EE_WRITE_0	0x4802
> 	#define EE_WRITE_1	0x4806
> 
> 	CSR_WORD_WR (SCB_EEPROM, EE_ENB); DELAY(20);
> 	CSR_WORD_WR (SCB_EEPROM, EE_ENB | FEI_EESK); DELAY(20);
> 
> 	/* shift the command bits out. */
> 	do {
> 		dataval = (cmd & (1 << cmd_len)) ? EE_WRITE_1 : EE_WRITE_0;
> 		CSR_WORD_WR (SCB_EEPROM, dataval); DELAY(20);
> 		CSR_WORD_WR (SCB_EEPROM, dataval | FEI_EESK); DELAY(20);
> 		CSR_WORD_RD (SCB_EEPROM, dataval); DELAY(20);
> 		retval = (retval << 1) | ((dataval & FEI_EEDO) ? 1 : 0);
> 	} while (--cmd_len >= 0);
> 
> 	CSR_WORD_WR (SCB_EEPROM, EE_ENB); DELAY(20);
> 
> 	/* terminate the EEPROM access. */
> 	CSR_WORD_WR (SCB_EEPROM, EE_ENB & ~FEI_EECS); DELAY(20);
> 
> 	return retval;
> }
> 
> LOCAL char *
> feiAlignPktTo16(char *pkt, int len)
> {
> 	if (((UINT32)pkt & 3) == 2 || len == 0)
> 		return pkt;
> 
> 	bcopy(pkt, pkt + 2, (len + 3) & ~3);
> 	return (pkt + 2);
> }
> 
> LOCAL int
> feiBoardIntEnable(int unit)
> {
> 	unmask_pci_interrupt(PCI_INTA);
> 	return OK;
> }
> 
> LOCAL int
> feiBoardIntDisable(int unit)
> {
> 	mask_pci_interrupt(PCI_INTA);
> 	return OK;
> }
> #endif

            ******** Changes to original fei82557End.h ********
69a70,105
> #ifdef INCLUDE_PCI_INTEL_END
> 
> #ifndef FEI_LONG_WR
> #define FEI_LONG_WR(addr, value)                                        \
>     (* (UINT32 *) (addr) = ((UINT32) (value & 0xffffffff)))
> #endif /* FEI_LONG_WR */
>  
> #ifndef FEI_WORD_WR
> #define FEI_WORD_WR(addr, value)                                        \
>     (* (UINT16 *) ((UINT32)addr ^ 2) = ((UINT16) (value & 0x0000ffff)))
> #endif /* FEI_WORD_WR */
>  
> #ifndef FEI_BYTE_WR
> #define FEI_BYTE_WR(addr, value)                                        \
>     (* ((UINT8 *) ((UINT32)addr ^ 3)) = ((UINT8) ((value) & 0x000000ff)))
> #endif /* FEI_BYTE_WR */
>  
> #ifndef FEI_LONG_RD
> #define FEI_LONG_RD(addr, value)                                        \
>     (((UINT32) (value)) = (UINT32) ((*((UINT32 *)(addr)) & \
>      0xffffffff)))
> #endif /* FEI_LONG_RD */
>  
> #ifndef FEI_WORD_RD
> #define FEI_WORD_RD(addr, value)                                        \
>     (((UINT16) (value)) = (UINT16) ((*((UINT16 *)((UINT32)addr ^ 2)) & \
>      0x0000ffff)))
> #endif /* FEI_WORD_RD */
>  
> #ifndef FEI_BYTE_RD
> #define FEI_BYTE_RD(addr, value)                                        \
>     (((UINT8) (value)) = (UINT8) (((*((UINT8 *)((UINT32)addr ^ 3))) & 0x000000ff)))
> #endif /* FEI_BYTE_RD */
> 
> #else
> 
100a137,138
> 
> #endif


PCMCIA

            ******** Changes to original pccardLib.c ********
66,71c66
< #ifdef INCLUDE_INTERSIL_END
< #include "vxbsp.h"
< #include "icic.h"
< #endif
< 
< #if defined   INCLUDE_ELT || defined INCLUDE_INTERSIL_END
---
> #ifdef    INCLUDE_ELT
105,112d99
< #ifdef INCLUDE_INTERSIL_END
< IMPORT STATUS icicInit(int ioBase, int intVec,    int intLevel, FUNCPTR showRtn);
< IMPORT void islPcmciaAttach(int unit, char *arg1, int arg2, int arg3, int arg4,
<         int arg5, int arg6, int arg7, int arg8);
< LOCAL STATUS pccardIslEnabler(int sock,   ISL_RESOURCE *pIslResource, int numEnt, FUNCPTR showRtn);
< LOCAL STATUS  pccardIslCscIntr  (int sock, int csc);
< #endif    /* INCLUDE_INTERSIL_END */
< 
138,141d124
< #ifdef INCLUDE_INTERSIL_END
<   {0, 0, 0, 0, 0, 0}, /* CIS extraction */
<     {0, 0, 0, 0, 0, 0},   /* config registers */
< #else
144d126
< #endif
149,151d130
< #ifdef INCLUDE_INTERSIL_END
<   {PCMCIA_ICIC, ICIC_BASE_ADR, ICIC_INT_VEC, ICIC_INT_LVL, icicInit, NULL}
< #else
154d132
< #endif
157,161d134
< #ifdef INCLUDE_INTERSIL_END
< ISL_RESOURCE islResources[] = 
<   {{{0, 0, {0, 0}, {0, 0}, 0, 0, 0, 0, 0, 0}, ISL_INT_VEC}};
< #endif
< 
242,248d214
< #ifdef INCLUDE_INTERSIL_END
<   {
<      PCCARD_LAN_INTERSIL, (void *)islResources, NELEMENTS(islResources),
<      (FUNCPTR)pccardIslEnabler, NULL
<     },
< #endif    /* INCLUDE_INTERSIL_END */
< 
1135,1297d1100
< 
< #ifdef INCLUDE_INTERSIL_END
< /*******************************************************************************
< *
< * pccardIslEnabler - enable the PCMCIA INTERSIL based LNKSYS card
< *
< * This routine enables the PCMCIA INTERSIL based LNKSYS card.
< *
< * RETURNS:
< *
< * OK, ERROR_FIND if there is no ISL card, or ERROR if another error occurs.
< */
< STATUS pccardIslEnabler
<     (
<     int        sock,      /* socket no. */
<     ISL_RESOURCE *pIslResource, /* pointer to ELT resources */
<     int        numEnt,    /* number of ELT resource entries */
<     FUNCPTR    showRtn    /* show routine */
<     )
< 
<     {
<     PCMCIA_CTRL *pCtrl        = &pcmciaCtrl;
<     PCMCIA_CHIP *pChip        = &pCtrl->chip;
<     PCMCIA_CARD *pCard        = &pCtrl->card[sock];
<     DL_LIST       *pList      = &pCard->cisTupleList;
<     PCCARD_RESOURCE *pResource    = &pIslResource->resource;
<     short manufacturerID0 = 0;
<     short manufacturerID1 = 0;
<     int functionCode      = 0;
<     DL_NODE *pNode;
<     PCMCIA_IOWIN pcmciaIowin;
<     CIS_TUPLE *pTuple;
<     char *pChar;
<     int ctrl;
<     int sock0;
<   #define SWAP_WORD(x)        (MSB(x) | LSB(x) << 8)
<   extern void islPcmciaAttach(int unit, char *arg1, int arg2, \
<       int arg3, int arg4, int arg5, int arg6, int arg7, int arg8);
< 
<     if (!pChip->installed)
<   return (ERROR);
< 
<     for (pNode = DLL_FIRST (pList); pNode != NULL; pNode = DLL_NEXT(pNode))
<   {
<   pTuple  = (CIS_TUPLE *)((char *)pNode + sizeof (DL_NODE));
<   pChar   = (char *)pTuple + sizeof (CIS_TUPLE);
< 
<   switch (pTuple->code)
<       {
<       case CISTPL_FUNCID:
<       functionCode = *pChar;
<       break;
< 
<       case CISTPL_MANFID:
<       manufacturerID0 = SWAP_WORD(*(short *)pChar);
<       manufacturerID1 = SWAP_WORD(*(short *)(pChar+2));
<       break;
<       }
<   }
< 
<     /* return if we didn't recognize the card */
<     if ((functionCode != FUNC_LAN) ||
<         (manufacturerID0 != ISL_PCMCIA_ID0) || (manufacturerID1 != ISL_PCMCIA_ID1)) {
<       return (ERROR_FIND);
<   }
< 
<     /* get un-used resource */
< 
<     for (ctrl = 0; ctrl < numEnt; ctrl++)
<   {
<         pResource = &pIslResource->resource;
< 
<   for (sock0 = 0; sock0 < pChip->socks; sock0++)
<       {
<           PCMCIA_CARD *pCard0 = &pCtrl->card[sock0];
<       if (pCard0->pResource == pResource)
<       break;
<       }
<   if (sock0 == pChip->socks)
<       break;
< 
<   pIslResource++;
<   }
< 
<     if (ctrl == numEnt)
<   {
<   if (_func_logMsg != NULL)
<       (* _func_logMsg) ("pccardIslEnabler: sock=%d out of resource\n",
<                 sock, 0, 0, 0, 0, 0);
<   return (ERROR);
<   }
< 
<     /* configure the card with the resource */
< 
<     pCard->type       = PCCARD_LAN_INTERSIL;
<     pCard->sock       = sock;
<     pCard->ctrl       = ctrl;
<     pCard->detected   = TRUE;
<     pCard->pResource  = pResource;
<     pCard->cardStatus = (* pChip->status)(sock);
<     pCard->cscIntr    = (FUNCPTR)pccardIslCscIntr;
<     pCard->showRtn    = (FUNCPTR)showRtn;
< 
<     cisConfigregSet (sock, CONFIG_STATUS_REG, 0x00);
<     cisConfigregSet (sock, CONFIG_OPTION_REG, COR_LEVIREQ | 1);
< 
<     pcmciaIowin.window    = PCCARD_IOWIN0;
<     pcmciaIowin.flags = MAP_ACTIVE | MAP_16BIT;
<     pcmciaIowin.extraws   = pResource->ioExtraws;
<     pcmciaIowin.start = pResource->ioStart[0];
<     pcmciaIowin.stop  = pResource->ioStop[0];
< 
<     if (pCard->pNetIf != NULL)
<   free (pCard->pNetIf);
< 
<     if ((pCard->pNetIf = (NETIF *)malloc (sizeof(NETIF))) == NULL)
<   return (ERROR);
< 
<   if (pcmciaMemwin[CIS_MEM_TUPLE].start)
<       free(pcmciaMemwin[CIS_MEM_TUPLE].start);
< 
<     pCard->pNetIf->ifName = "isl";
<     pCard->pNetIf->attachRtn  = (void *)0;
<   pCard->pNetIf->arg1     = (char *)pResource->ioStart[0];
<     pCard->pNetIf->arg2       = pIslResource->intVector;
<     pCard->pNetIf->arg3       = (int)pCard->pNetIf->ifName;
<     pCard->pNetIf->arg4       = 0;
<     pCard->pNetIf->arg5       = 0;
<     pCard->pNetIf->arg6       = 0;
<     pCard->pNetIf->arg7       = 0;
<     pCard->pNetIf->arg8       = 0;
< 
<     pCard->installed      = TRUE;
< 
<   islPcmciaAttach(0, 
<                   pCard->pNetIf->arg1,
<                   pCard->pNetIf->arg2,
<                   pCard->pNetIf->arg3,
<                   pCard->pNetIf->arg4,
<                   pCard->pNetIf->arg5,
<                   pCard->pNetIf->arg6,
<                   pCard->pNetIf->arg7,
<                   pCard->pNetIf->arg8);
< 
<     return (OK);
<     }
< 
< /*******************************************************************************
< *
< * pccardIslCscIntr - ISL controller PCMCIA card status change interrupt handler.
< *
< * RETURNS: OK (always).
< */
< 
< LOCAL STATUS pccardIslCscIntr
<     (
<     int sock,         /* socket no. */
<     int csc           /* CSC bits */
<     )
<     {
<     return (OK);
<     }
< #endif    /* INCLUDE_INTERSIL_END */
