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
############################################################################## 
#
#  Filename:       bcm97455vendor_settings.bash
#  Author:         Pinar Taskiran
#  Creation Date:  March 10, 2008
#
#****************************************************************************

# Setup script specific to the bcm74558 reference design for the
# CmDocsisSystem.  This is where build options specific to this app go.
#
# This sets up the build for the generic Broadcom CM Vendor Extensions.

# Base this on the basic bcm97455 reference design.
source ${SystemCustomDir}/bcm97455_settings.bash

echo "Setting build options from DhcpV6System/linux/Custom/bcm97455vendor_settings.bash"

# Now add vendor support.
app_options="${app_options} cmvendor"

# Set the name of the makefile that sets up the vendor-specific stuff for the
# build.  The makefile must exist in Bfc/make, and should include both the
# company name and product name.
#
# Here, we use Broadcom as the vendor name and bcm93368 as the product.
CmVendorExtensionMakefile=BroadcomBcm93368Cm.mak

# We also need to specify the directory where the product-specific files are.
# This directory is relative to the Cm/CmExtensionApi directory.  Again, this
# needs to include the company name and product name.
CmVendorExtensionDir=Broadcom/bcm97455

# Finally, point out the name of the product-specific nonvol defaults file so
# that it will override or add to anything already specified.  It is assumed
# that this file is somewhere in the include path, usually in the directory
# specified for CmVendorExtensionDir.
#CmVendorExtensionNonVolDefaults=bcm93368_NonVolDefaults.h

