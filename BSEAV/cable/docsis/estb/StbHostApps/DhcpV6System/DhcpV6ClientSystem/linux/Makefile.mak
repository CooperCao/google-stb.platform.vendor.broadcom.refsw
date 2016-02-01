#############################################################################
#
# Copyright (c) 2009 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
# AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
# SOFTWARE.  
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################
#  $Id$
#
#  Filename:       Makefile.mak
#  Author:         Mike Sieweke (based on VxWorks original by David Pullen)
#  Creation Date:  November 14, 2003
#
##############################################################################
#  Description:
#
#	Makefile for the GNU make utility, which sets things up for doing a
#	eCos build of this project.
#
##############################################################################


##############################################################################
#
# Set up relative paths as macros; this makes it easier to relocate things as
# needed, and also makes it easier to read the rest of the stuff.  The
# assumption is that the current directory will be the output directory (i.e.
# ecos/bcm93348, not the ecos directory).
#
# The following macros MUST be defined here; they are all used by
# BfcEcos.mak:
#
#    APP_LINUX_OUTPUT_DIR - the directory where intermediate and output files
#                          should be placed.  This should almost always be
#                          '.', which is the current directory.
#    APP_LINUX_DIR -    the directory where eCos-specific things for this
#                      application can be found.  This should almost always be
#                      "..", one level out from the output dir.
#    APP_PORTABLE_DIR - the directory where the portable files for the
#                       application can be found.  This should almost always be
#                       one level out from the ecos dir.
#    BFC_ROOT_DIR - the root directory for the project; the MKS project files
#                   are here.  All other directory paths will be made relative
#                   to this.  This will generally be one level out from the
#                   portable app dir.

APP_LINUX_OUTPUT_DIR = .
APP_LINUX_DIR = ..
APP_PORTABLE_DIR = ../..

BFC_ROOT_DIR = ../../../../..


##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

# Get the standard embedded target stuff from this makefile.
IncludeTargetSpecificMakefile = 0
TargetSpecificMakefile = BfcLnxEmbeddedTarget.mak

##############################################################################
#
# Include the main Linux makefile last.



include $(BFC_ROOT_DIR)/Bfc/make/BfcLinux.mak






