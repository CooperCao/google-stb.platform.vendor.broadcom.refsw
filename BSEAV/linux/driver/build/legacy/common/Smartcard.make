############################################################
#     Copyright (c) 2003-2004, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 02/09/2001 by Marcus Kellerman
#
# $brcm_Log: $
# 
############################################################

#
# smartcard
#
ifeq ($(ICAM_DRIVER),y)
CFLAGS += -DXTV_MULTI_STREAM -DNDS_ICAM -DBCM_SOFTWARE_SECTION_FILTER -DBCM_SOFTWARE_ICAM_PACKET_COUNTER_ASSOCIATION
BCM_SRC += \
	bcmicamfilter.c \
	bcmicamtrans.c \
	bcmpfemm.c \
	bcmicamsc.c \
	bcmpfecm.c \
	bcmpfsc.c \
	icamfilterdrv.c \
	icamsmartcarddrv.c \
	bcmicamfiltersw.c
endif

ifeq ($(SMARTCARD_DRIVER),y)

ifeq ($(CONFIG_BRCM_SC_ECHOSTAR),y)
CFLAGS += -DECHOSTAR -DSMARTCARD_DRIVER -DSC_TEST -DES_TEST
BCM_SRC += bcmesutil.c
endif

ifeq ($(CONFIG_BRCM_SC_IRDETO),y)
CFLAGS += -DIRDETO -DSMARTCARD_DRIVER -DSC_TEST -DIRDETO_TEST
BCM_SRC += bcmirdetoutil.c
endif

ifeq ($(CONFIG_BRCM_SC_EMV),y)
CFLAGS += -DEMV -DSMARTCARD_DRIVER -DSC_TEST -DEMV_TEST
BCM_SRC += bcmemvatr.c bcmsct0.c bcmsct1.c bcmemvutil.c 
endif

ifeq ($(CONFIG_BRCM_SC_ISO),y)
CFLAGS += -DISO -DSMARTCARD_DRIVER -DSC_TEST -DISO_TEST
BCM_SRC += bcmisoutil.c
endif

ifeq ($(CONFIG_BRCM_SC_NDS),y)
CFLAGS += -DNDS -DSMARTCARD_DRIVER -DSC_TEST -DNDS_TEST
BCM_SRC += bcmndsutil.c
endif

BCM_SRC += \
	bcmsmartcard.c		\
	bcmscutil.c			\
	bcmscstates.c		\
	smartcarddrv.c
endif
