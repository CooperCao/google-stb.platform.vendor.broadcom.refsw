############################################################################
#     Copyright (c) 2003, Broadcom Corporation
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
# build various inc files from the component inc files
#
# Revision History:
#
# $brcm_Log: $
# 
###########################################################################


MAGNUM_OBJS := $(addprefix ${ODIR}/, $(addsuffix .o, ${notdir ${OBJ}}))
MAGNUM_INC := $(addprefix -I${MAGNUM}/, $(sort ${INC}))

vpath %c $(addprefix ${MAGNUM}/, $(sort $(dir ${OBJ})))



