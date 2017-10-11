# sources: windows build file for wl_tool.exe
# 
# Copyright (C) 2005 Broadcom Corporation
# All Rights Reserved.
#
# $Id: sources.tool,v 1.6 2004-12-29 23:06:06 $
#
NIC_WL_EXE=wl_tool
TARGETPATH=windows/winxp/obj/tool


C_DEFINES = -DWL_TOOL
include sources.common
