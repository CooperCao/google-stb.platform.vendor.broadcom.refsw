############################################################
#     Copyright (c) 2007 Broadcom Corporation
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
#   FLV media library module descriptor
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

BFILE_INCLUDES := $(BSEAV)/lib/bfile

BFILE_SOURCES :=  \
	${BSEAV}/lib/bfile/bfile_io.c 

BFILE_MEDIA_SOURCES := \
	${BSEAV}/lib/bfile/bfile_cache.c \
	${BSEAV}/lib/bfile/bfile_buffer.c \
	${BSEAV}/lib/bfile/bfile_buffered.c \
	${BSEAV}/lib/bfile/bfile_util.c  

BFILE_MEDIA_INCLUDES := ${BFILE_INCLUDES}

BFILE_STDIO_SOURCES := \
	${BSEAV}/lib/bfile/bfile_stdio.c 
BFILE_STDIO_INCLUDES := ${BFILE_INCLUDES}

BFILE_ASYNC_SOURCES := \
	${BSEAV}/lib/bfile/bfile_async.c
BFILE_ASYNC_INCLUDES := ${BFILE_INCLUDES}


