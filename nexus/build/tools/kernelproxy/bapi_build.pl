#!/usr/bin/perl
#  Broadcom Proprietary and Confidential. (c)2003-2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
#############################################################################
use strict;
use warnings FATAL => 'all';

use lib "../common";
use bapi_ioctl_def;
use bapi_usercall;
use bapi_driver_ioctl;
use bapi_kernel_export;
use bapi_main;

# This uses and consumes ARGV
my $main = bapi_main::main ('kernelproxy');
my $destdir = $main->{DESTDIR};
my $module = $main->{MODULE};
my $funcrefs =$main->{FUNCREFS};


my @base_funcrefs;
if ($module eq "CORE") {
    # these functions from nexus_base_*.h aren't C parser friendly, but are still 
    # of value to export for kernel driver code
    my @export_funcs = ("NEXUS_AddrToOffset",
        "NEXUS_GetAddrType",
        "NEXUS_OffsetToCachedAddr",
        "NEXUS_OffsetToUncachedAddr",
        "NEXUS_Thread_Create",
        "NEXUS_Time_Diff_isrsafe",
        "NEXUS_Time_Get_isrsafe");
    for (@export_funcs) {
        push @base_funcrefs, ({FUNCNAME => $_});
    }
}

# Build thunk layer
my $module_lc = lc $module;
bapi_ioctl_def::generate "${destdir}/" . (bapi_common::ioctl_header $module) , $module, $main->{MODULE_NUMBER}, $main->{VERSION}, @$funcrefs;
bapi_usercall::generate "${destdir}/nexus_${module_lc}_proxy.c", $module, $main->{STRUCTS}, @$funcrefs;
bapi_kernel_export::generate "${destdir}/nexus_${module_lc}_kernel_export.c", $module_lc, (@$funcrefs, @base_funcrefs);
bapi_driver_ioctl::generate "${destdir}/nexus_${module_lc}_driver.c", $module, $main->{VERSION}, $main->{STRUCTS}, $funcrefs, $main->{CLASS_HANDLES};

