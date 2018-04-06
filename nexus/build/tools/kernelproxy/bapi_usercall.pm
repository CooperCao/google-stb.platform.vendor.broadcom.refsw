#!/usr/bin/perl
#  Copyright (C) 2004-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# File Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################
use strict;
use bapi_common;
use bapi_classes;

package bapi_usercall;

sub generate
{
    my ($filename, $module, $structs, @funcs) = @_;
    my $func;
    my $destructors = bapi_classes::get_destructors \@funcs;
    my $classes = bapi_classes::get_classes \@funcs, $destructors, $structs;
    my $module_lc = lc $module;
    open FILE, ">$filename";

    print FILE bapi_util::header $module;

    print FILE "#define NEXUS_PROXY_THUNK_LAYER\n";
    print FILE "#include \"proxy/nexus_proxy_prologue.h\"\n";
    print FILE "#include \"nexus_${module_lc}_module.h\"\n";
    print FILE "#include \"nexus_core_utils.h\"\n";
    print FILE "BDBG_MODULE(nexus_${module_lc}_proxy);\n";
    print FILE "\n\n";
    print FILE "#define nexus_proxy_module_init nexus_proxy_${module_lc}_init\n";
    print FILE "#define nexus_proxy_module_uninit nexus_proxy_${module_lc}_uninit\n";
    print FILE "#define nexus_proxy_module_state nexus_proxy_${module_lc}_state\n";
    print FILE "#include \"" . (bapi_common::ioctl_header $module) . "\"\n";
    print FILE "#define NEXUS_IOCTL_MODULE_INIT " .  (bapi_common::version_ioctl $module) . "\n";
    print FILE "#define NEXUS_PROXY_MODULE_VERSION " .  (bapi_common::version_define $module) . "\n";
    print FILE "#define NEXUS_PROXY_MODULE_NAME \"$module\"\n";
    print FILE "#include \"proxy/nexus_proxy_body.h\"\n";

    for $func (@funcs) {
        my $params = $func->{PARAMS};
        my $param;
        my $ioctl = bapi_common::ioctl_name $module, $func;
        my $arg = "NULL";
        my $attr = bapi_common::process_function_attributes $func, $structs, \@funcs;

        next if(exists $func->{ATTR}->{'local'}); # local function compiled verbatim into the proxy code

        print FILE "$func->{PROTOTYPE}\n\{\n";
        print FILE "  int rc;\n";
        if ($func->{RETTYPE} eq "NEXUS_Error") {
            print FILE "  NEXUS_Error result=NEXUS_SUCCESS;\n";
        } elsif ($func->{RETTYPE} ne "void") {
            print FILE "  $func->{RETTYPE} result=($func->{RETTYPE})NULL;\n";
        }


        print FILE "  NEXUS_PROXY_ENTER($func->{FUNCNAME});\n";
        my $stopcallbacks_handle = bapi_classes::get_stopcallbacks_handle $func, $classes;

        print FILE "  if(nexus_proxy_module_state.fd >= 0) {\n";
        # If no params, no ioctl
        unless ($func->{NOSTRUCT}) {
            my $struct = bapi_common::ioctl_struct $module, $func;

            print FILE "    $struct st;\n";
            bapi_util::print_code \*FILE, $attr->{proxy_vars}, "    ";
            print FILE "\n";

            if (defined $attr->{proxy_pre_call}) {
                print FILE "    rc = 0;\n";
                print FILE "    {\n";
                bapi_util::print_code \*FILE, $attr->{proxy_pre_call}, "    ";
                if ($func->{RETTYPE} eq "NEXUS_Error") {
                    print FILE "    if (rc!=0) {result=BERR_TRACE(rc);goto done;}\n";
                } else {
                    print FILE "    if (rc!=0) {rc=BERR_TRACE(rc);goto done;}\n";
                }
                print FILE "    }\n";
            }

            for $param (@$params) {
                print FILE "    st.$param->{NAME} = $param->{NAME};\n";
            }
            $arg = "&st";
        }
        if (defined $stopcallbacks_handle) {
            print FILE "    NEXUS_StopCallbacks((void *)$stopcallbacks_handle);\n";
        }
        print FILE "    rc = ioctl( nexus_proxy_module_state.fd, $ioctl, $arg);\n";
        if ($func->{RETTYPE} eq "NEXUS_Error") {
            print FILE "    if (rc!=0) {result=BERR_TRACE(NEXUS_OS_ERROR);goto done;}\n";
        } else {
            print FILE "    if (rc!=0) {rc=BERR_TRACE(NEXUS_OS_ERROR);goto done;}\n";
        }

        if ($func->{RETTYPE} ne "void") {
            print FILE "    result = st.__retval;\n";
        }


        if(exists $attr->{proxy_post_success})  {
            # if postprocessing is required test error codee, and produce postprocessing
            if ($func->{RETTYPE} eq "NEXUS_Error") {
                print FILE "    if(result==NEXUS_SUCCESS) {\n";
            } elsif ($func->{RETTYPE} ne "void") {
                print FILE "    if(result!=NULL) {\n";
            } else {
                print FILE "    {\n";
            }
            bapi_util::print_code \*FILE, $attr->{proxy_post_success}, "    ";
            print FILE "    }\n";
        }

        print FILE "  } else {\n";
        if ($func->{RETTYPE} eq "NEXUS_Error") {
            print FILE "    result=BERR_TRACE(NEXUS_OS_ERROR);goto done;\n";
        } else {
            print FILE "    rc=BERR_TRACE(NEXUS_OS_ERROR);goto done;\n";
        }
        print FILE "  }\n";


        print FILE "\n\n done:\n";

        print FILE "  NEXUS_PROXY_LEAVE($func->{FUNCNAME});\n";
        if ($func->{RETTYPE} ne "void") {
            print FILE "  return result;\n"
        } else {
            print FILE "  return ;\n"
        }
        print FILE "}\n\n\n";
    }
    close FILE;
}

1;

