#!/usr/bin/perl
#     (c)2004-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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

package bapi_ipc_server;

sub generate_class_code_post
{
    my ($classes, $func, $in_args, $out_args) = @_;
    CLASS: for (@$classes) {
        my $class = $_;
        my $id=$class->{CLASS_NAME};
        for (@{$class->{CONSTRUCTORS}}) {
            if($_ == $func) {
                if($class->{DESTRUCTOR_TYPE} eq 'destructor') {
                    return ["NEXUS_DRIVER_CREATE_OBJECT($id, (void *)$out_args.__retval);"];
                }
                last CLASS;
            }
        }
        if($class->{ACQUIRE} == $func) {
            my $param = @{$func->{PARAMS}}[0];
            return ["NEXUS_DRIVER_ACQUIRE_OBJECT($id, (void *)$out_args.__retval);"];
        }
    }
    return undef;
}

sub generate_class_code_pre
{
    my ($classes, $func, $in_args, $out_args) = @_;
    CLASS: for (@$classes) {
        my $class = $_;
        my $id=$class->{CLASS_NAME};
        my $bypass=0;
        my $shutdown = "";
    
        if($class->{DESTRUCTOR} == $func) {
            if(exists $class->{SHUTDOWN}) {
                # for destructors with shutdown attributes, we must call the auto-generated shutdown_close function
                # instead of directly calling the normal close function.
                my $classtype = $class->{CLASS_TYPE};
                my $params = $func->{PARAMS};
                my $handle = "$in_args.$$params[0]->{NAME}";
                $bypass = 1;
                $shutdown = "nexus_driver_shutdown_close_$classtype($handle);";

            }
            my $param =  @{$func->{PARAMS}}[0];
            return ($bypass,["NEXUS_DRIVER_DESTROY_OBJECT($id, (void *)$in_args.$param->{NAME});",$shutdown]);
        }
        if($class->{RELEASE} == $func) {
            my $param =  @{$func->{PARAMS}}[0];
            return ($bypass,["NEXUS_DRIVER_RELEASE_OBJECT($id, (void *)$in_args.$param->{NAME});"]);
        }
    }
    return (0,undef);
}

sub generate_ipc
{
    my ($file, $module, $version, $structs, $classes, $funcs, $class_handles) = @_;
    my $func;
    my $ipc = bapi_common::version_ipc $module;
    my $name;

    for $func (@$funcs) {
        my $params = $func->{PARAMS};
        my $param;
        my $ipc = bapi_common::ipc_name $module, $func;
        my $in_args = "in_data->data.$func->{FUNCNAME}.in";
        my $out_args = "out_data->data.$func->{FUNCNAME}.out";
        my $retval = "$out_args.__retval";
        my $attr = bapi_common::process_function_attributes $func, $structs, $funcs;
        my ($bypass,$class_code_pre) = generate_class_code_pre $classes, $func, $in_args, $out_args;
        my $class_code_post = generate_class_code_post $classes, $func, $in_args, $out_args;

        next if(exists $func->{ATTR}->{'local'});

        print $file "case $ipc:\n";
        print $file "   rc = nexus_p_api_call_verify(&client->client, NEXUS_MODULE_SELF,  &_api_$func->{FUNCNAME}, in_data, in_data_size, out_mem_size);\n";
        print $file "   if(rc!=NEXUS_SUCCESS) {goto err_fault;}\n";
        print $file "   __variable_out_offset = (uint8_t *)&$out_args.variable_params - (uint8_t *)out_data;\n";

        # must do class_verification before driver_pre_call to avoid leaks if not verified
        bapi_util::print_code $file, $attr->{driver_pre_call}, "   ";
        bapi_util::print_code $file, $class_code_pre, "   ";


        if (!$bypass) {
            # make a function call

            if ($func->{RETTYPE} ne "void") {
                print $file "   $retval = \n"
            }


            my @args;
            for $param (@$params) {
                my $info = bapi_common::process_function_param $func, $param;
                push @args, "\n        " . $info->{'driver_arg'};
            }

            print $file "   " . bapi_util::call($func->{FUNCNAME} , @args) . ";\n\n";
        }

        if (defined $attr->{driver_post_success} || defined $attr->{driver_post_error} || defined $class_code_post) {
            if ($func->{RETTYPE} eq "NEXUS_Error") {
                print $file "  if ($retval == NEXUS_SUCCESS) {\n";
            } elsif ($func->{RETTYPE} ne "void") {
                print $file "  if ((void *)$retval != NULL) {\n";
            } else {
                print $file "  {\n";
            }
            bapi_util::print_code $file, $attr->{driver_post_success}, "     ";
            bapi_util::print_code $file, $class_code_post, "     ";
            if (($func->{RETTYPE} ne "void") && (defined $attr->{driver_post_error}) ) {
                print $file "  } else { \n";
                bapi_util::print_code $file, $attr->{driver_post_error}, "     ";
            }
            print $file "  }\n";
        }
        bapi_util::print_code  $file, $attr->{driver_post_always}, "  ";
        print $file "  nexus_p_api_call_completed(&client->client,  NEXUS_MODULE_SELF, &_api_$func->{FUNCNAME}, in_data, out_data);\n";
        print $file "  break;\n\n";
    }

}



sub generate
{
    my ($filename, $module, $version, $structs, $funcs, $class_handles) = @_;
    my $module_lc = lc $module;
    open FILE, ">$filename";
    my $file = \*FILE;

    my $destructors = bapi_classes::get_destructors $funcs;
    my $classes = bapi_classes::get_classes $funcs, $destructors;
    print $file bapi_util::header $module;
    print $file "#define NEXUS_SERVER_MODULE_NAME \"$module\"\n";
    print $file "#include \"nexus_${module_lc}_module.h\"\n";
    print $file "BDBG_MODULE(nexus_${module_lc}_ipc_server);\n";
    print $file "#include \"nexus_core_utils.h\"\n";
    print $file "\n\n";
    print $file "/* defines to make all module symbols uniques */\n";
    print $file "#define nexus_server_module_open nexus_server_${module_lc}_open\n";
    print $file "#define nexus_server_module_close nexus_server_${module_lc}_close\n";
    print $file "#define nexus_server_process nexus_server_${module_lc}_process\n";
    print $file "#define nexus_server_args " . (bapi_common::ipc_block $module) ."\n";
    print $file "#define nexus_server_data_size nexus_server_${module_lc}_data_size\n";
    print $file "\n\n";
    print $file "#define NEXUS_DRIVER_MODULE_CLASS_TABLE    nexus_server_$module\_class_table\n";
    print $file "\n\n";

    print $file "#include \"server/nexus_server_prologue.h\"\n";

    print $file "#include \"" . (bapi_common::ipc_header $module) . "\"\n";
    print $file "#define NEXUS_IPC_HEADER_VERSION   " . (bapi_common::version_define $module) . "\n";
    print $file "\n\n\n";
    bapi_classes::generate_class_table $file, $classes;
    print $file "\n\n\n";
    bapi_classes::generate_meta $file, $module, $version, $structs, $classes, $funcs, $class_handles, {
        KIND=>'ipc',
        FUNCHEADER=>sub {
            my ($function, $module,$func)=@_;
            my $ipc = bapi_common::ipc_name $module, $func;
            push @$function, "NEXUS_OFFSETOF($ipc\_data, in.variable_params) + sizeof(NEXUS_Ipc_Header),";
            push @$function, "NEXUS_OFFSETOF($ipc\_data, out.variable_params) + sizeof(NEXUS_Ipc_Header),";
        }
    };
    print $file "\n\n\n";
    print $file "#include \"server/nexus_server_body.h\"\n";
    print $file "\n\n\n";
    generate_ipc $file, $module, $version, $structs, $classes, $funcs, $class_handles;
    print $file "\n\n\n";
    print $file "#include \"server/nexus_server_epilogue.h\"\n";
    print $file "\n\n\n";
    print $file "\n\n\n";
    close FILE;
}

1;
