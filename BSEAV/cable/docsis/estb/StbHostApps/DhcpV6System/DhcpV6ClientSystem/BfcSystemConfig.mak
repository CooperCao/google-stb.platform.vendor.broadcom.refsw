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
#
#  Filename:       BfcSystemConfig.mak
#  Author:         David Pullen
#  Creation Date:  November 12, 2003
#
##############################################################################
#  Description:
#
# 	This file contains all of the BFC system configuration options that will
#	be used in the build process.
#
##############
#
# NOTE:  This file is geared toward the GNU Make utility, and may use any
#        syntax that is valid for recent-vintage GNU Make.
#
##############################################################################


# We pretty much always need to configure these items for message logging output
# support.
BFC_CONFIG_MESSAGE_LOGGING_OSTREAM_DEFAULT_NULL	= 0
BFC_CONFIG_MESSAGE_LOGGING_OSTREAM_DEFAULT 	= cout


BFC_INCLUDE_DHCPV6_SUPPORT = 1

BFC_INCLUDE_NETWORKING_SUPPORT = 0
BFC_INCLUDE_NETWORKING_CONSOLE_SUPPORT=0
BFC_INCLUDE_DHCPV6_DIB_SUPPORT=1
BFC_INCLUDE_ESTB_IPNETWORKING_SUPPORT=1
BFC_INCLUDE_IPHELPERS_SUPPORT=1
BFC_INCLUDE_NETWORKING_STUB_SUPPORT=1

BCM_ESTB_STANDALONE_APP_SUPPORT=1
BCM_STANDALONE_DHCP_CLIENT = 1
BFC_INCLUDE_ESTB_DHCPV6_SUPPORT = 1

BCM_EVD_INTERFACE_INCLUDED=0
BCM_EVD_RPCCL_INTERFACE_INCLUDED=0

BCM_RPC_INTERFACE_INCLUDED=0
BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED=0

#############################
# DHCPv4 support
#############################
BFC_INCLUDE_DHCP_SUPPORT=1
BFC_INCLUDE_DHCPV4_SUPPORT=1
BCM_V2_DHCP_SUPPORT = 1
