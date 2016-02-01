#!/usr/bin/perl
#     (c)2003-2013 Broadcom Corporation
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

package bapi_driver_ioctl;

sub generate_class_code_post
{
    my ($classes, $func, $args) = @_;
    CLASS: for (@$classes) {
        my $class = $_;
        my $id=$class->{CLASS_NAME};
        for (@{$class->{CONSTRUCTORS}}) {
            if($_ == $func) {
                if($class->{DESTRUCTOR_TYPE} eq 'destructor') {
                    return ["NEXUS_DRIVER_CREATE_OBJECT($id, $args.ioctl.__retval);"];
                }
                last CLASS;
            }
        }
        if($class->{ACQUIRE} == $func) {
            my $param =  @{$func->{PARAMS}}[0];
            return ["NEXUS_DRIVER_ACQUIRE_OBJECT($id, $args.ioctl.__retval);"];
        }
    }
    return undef;
}

sub generate_class_code_pre
{
    my ($classes, $func, $args) = @_;
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
                my $handle = "$args.ioctl.$$params[0]->{NAME}";
                $bypass = 1;
                $shutdown = "nexus_driver_shutdown_close_$classtype($handle);";
            }
            my $param =  @{$func->{PARAMS}}[0];
            return ($bypass,["NEXUS_DRIVER_DESTROY_OBJECT($id, $args.ioctl.$param->{NAME});"],[$shutdown]);
        }
        if($class->{RELEASE} == $func) {
            my $param =  @{$func->{PARAMS}}[0];
            return ($bypass,["NEXUS_DRIVER_RELEASE_OBJECT($id, $args.ioctl.$param->{NAME});"]);
        }
    }
    return (0,undef);
}

sub generate_union
{
    my ($file, $module, $funcs, $structs) = @_;
    my $func;
    my $module_lc = lc $module;


    print $file "/* union to store temporary copy of all in and out parameters */\n";
    print $file "union nexus_driver_module_args {\n";
    print $file "  PROXY_NEXUS_ModuleInit init;\n";
    for $func (@$funcs) {
        my $params = $func->{PARAMS};
        my $param;
        my @struct;
        my $attr = bapi_common::process_function_attributes $func, $structs, $funcs;
        if (!$func->{NOSTRUCT}) {
            my $struct = bapi_common::ioctl_struct $module, $func;
            print $file "  struct {\n";
            print $file "     $struct ioctl;\n";
            for $param (@$params) {
                if ($param->{ISREF} && $param->{BASETYPE} ne 'void' && not (exists $attr->{'driver_overwrite'}->{$param->{NAME}}) ) {
                    print $file "     $param->{BASETYPE} $param->{NAME};\n";
                }
            }
            bapi_util::print_code $file, $attr->{'driver_ioctl_data'}, "     ";
            print $file "  } $func->{FUNCNAME};  \n";
        }
    }
    print $file "};\n";
}

sub case_ioctl
{
    my $module = shift;
    my $ioctl = shift;
    return <<ENDCASE;
 case NEXUS_IOCTL_NUM($ioctl):
ENDCASE
}

sub generate_ioctl
{
    my ($file, $module, $version, $structs, $classes, $funcs, $class_handles) = @_;
    my $func;
    my $ioctl = bapi_common::version_ioctl $module;
    my $name;

    print $file <<SWITCH;
    /* body of ioctl dispatch, convert IOCTL's back to number, that shall help compile to convert switch into the jump table */
    if ((NEXUS_IOCTL_NUM(cmd)/NEXUS_IOCTL_PER_MODULE) != NEXUS_IOCTL_$module\_ID) { NEXUS_DRIVER_TRACE(-1); goto err_invalid_ioctl;}
    switch(NEXUS_IOCTL_NUM(cmd)) {
    default:
        if(arg==0) {
            NEXUS_DRIVER_TRACE(-1);
            goto err_fault; /* just prevents warning */
        }
       NEXUS_DRIVER_TRACE(-1);
       goto err_invalid_ioctl;
SWITCH


    for $func (@$funcs) {
        my $params = $func->{PARAMS};
        my $param;
        my $ioctl = bapi_common::ioctl_name $module, $func;
        my $args = "module->args.$func->{FUNCNAME}";
        my $retval = "$args.ioctl.__retval";
        my $attr = bapi_common::process_function_attributes $func, $structs, $funcs;
        my ($bypass, $class_code_pre, $shutdown) = generate_class_code_pre $classes, $func, $args;
        my $class_code_post = generate_class_code_post $classes, $func, $args;
        
        next if(exists $func->{ATTR}->{'local'});

        print $file case_ioctl($module,$ioctl);
        print $file "   NEXUS_IOCTL_ENTER($func->{FUNCNAME});\n";
        if (!$func->{NOSTRUCT}) {
            # copy IOCTL parameters
            print $file "   if (" . bapi_util::call('copy_from_user_small', "&$args.ioctl", '(void*)arg', "sizeof($args.ioctl)") . "!=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}\n";
            
            bapi_util::print_code $file, $attr->{driver_pre_call}, "   ";
            
            # TODO: must do class_verification before driver_pre_call to avoid leaks if not verified.
            # solution is to push generate_class_verification logic into driver_pre_call.
        }
        print $file "   if(nexus_p_api_call_verify(&client->client, NEXUS_MODULE_SELF, &_api_$func->{FUNCNAME}, " . ($func->{NOSTRUCT} ? 'NULL': "&$args") . ")!=NEXUS_SUCCESS) { goto err_fault;}\n";
        bapi_util::print_code $file, $class_code_pre, "     ";
        if (!$bypass) {
            # disable verification of arguments, they were verified by nexus_p_api_call_verify
            print $file "   /* coverity[ tainted_data : FALSE ] */\n";
            print $file "   /* coverity[ tainted_data_transitive : FALSE ] */\n";
            print $file "   /* coverity[ FORWARD_NULL : FALSE ] */\n";

            # make a function call
            if ($func->{RETTYPE} ne "void") {
                print $file "   $retval = \n"
            }

            # capture arguments
            my @args;
            for $param (@$params) {
                $name = $param->{NAME};

                if (exists $attr->{'driver_overwrite'}->{$name}) {
                    push @args, "\n      $args.$name";
                } elsif ($param->{ISREF} && $param->{BASETYPE} ne 'void') {
                    push @args, "\n     ($args.ioctl.$name ? &$args.$name : NULL )";
                } else {
                    push @args, "\n      $args.ioctl.$name";
                }
            }

            print $file "   " . bapi_util::call($func->{FUNCNAME}, @args) . ";\n\n";
        }

        my @post;

        if(defined $attr->{driver_post_success}) {
            @post = @{$attr->{driver_post_success}};
        }

        for $param (@$params) {
            $name = $param->{NAME};
            # copy OUT parameters
            next if (exists $attr->{'driver_overwrite'}->{$name});
            if ($param->{ISREF} && !$param->{INPARAM} && $param->{BASETYPE} ne 'void' ) {
                # outparams may be NULL
                push @post, "if ($args.ioctl.$name) {";
                push @post, "  if (" . bapi_util::call('copy_to_user_small', "$args.ioctl.$name", "&$args.$name", "sizeof($args.$name)") . "!=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}";
                push @post, "}";
            }
        }
        print $file "  nexus_p_api_call_completed(&client->client, NEXUS_MODULE_SELF, &_api_$func->{FUNCNAME}, " . ($func->{NOSTRUCT} ? 'NULL': "&$args") . ");\n";
        if (defined $shutdown) {
            bapi_util::print_code $file, $shutdown, "     ";
        }
        if( (scalar @post) || (defined $attr->{driver_post_error}) || (defined $class_code_post)) {
            if ($func->{RETTYPE} eq "NEXUS_Error") {
                print $file "  if ($retval == NEXUS_SUCCESS) {\n";
            } elsif ($func->{RETTYPE} ne "void") {
                print $file "  if ((void*)$retval != NULL) {\n";
            } else {
                print $file "  {\n";
            }
            # assume that all data was initialized and not tainted
            print $file "   /* coverity[ tainted_data : FALSE ] */\n";
            bapi_util::print_code $file, \@post, "     ";
            bapi_util::print_code $file, $class_code_post, "     ";
            if (($func->{RETTYPE} ne "void") && (defined $attr->{driver_post_error}) ) {
                print $file "  } else { \n";
                bapi_util::print_code $file, $attr->{driver_post_error}, "     ";
            }
            print $file "  }\n";
        }

        if ($func->{RETTYPE} ne "void") {
            # copy only retval other members are left intact
           print $file "  if (" . bapi_util::call('copy_to_user_small', ('&(( struct ' . ((bapi_common::ioctl_struct $module, $func) . '*)arg)->__retval')),   "&$retval", "sizeof($retval)") . "!=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}\n";
        }
        bapi_util::print_code  $file, $attr->{driver_post_always}, "  ";
        print $file "  NEXUS_IOCTL_LEAVE($func->{FUNCNAME});\n";
        print $file "  break;\n\n";
    }
    print $file " }; /* end of switch */ \n\n\n\n";

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
    print $file "#define NEXUS_DRIVER_MODULE_NAME \"$module\"\n";
    print $file "#include \"nexus_${module_lc}_module.h\"\n";
    print $file "BDBG_MODULE(nexus_${module_lc}_driver);\n";
    print $file "#include \"nexus_core_utils.h\"\n";
    print $file "\n\n";
    print $file "/* defines to make all module symbols uniques */\n";
    print $file "#define nexus_driver_module_ioctl nexus_driver_${module_lc}_ioctl\n";
    print $file "#define nexus_driver_module_open nexus_driver_${module_lc}_open\n";
    print $file "#define nexus_driver_module_close nexus_driver_${module_lc}_close\n";
    print $file "#define nexus_driver_module_state nexus_driver_${module_lc}_state\n";
    print $file "#define nexus_driver_module_args nexus_driver_${module_lc}_args\n";
    print $file "\n\n";
    if(scalar @$classes) {
        print $file "#define NEXUS_DRIVER_MODULE_CLASS_TABLE    nexus_driver_$module\_class_table\n";
    }
    print $file "\n\n";

    print $file "#define NEXUS_IOCTL_MODULE_INIT      " .  (bapi_common::version_ioctl $module) . "\n";
    print $file "#define NEXUS_IOCTL_MODULE_VERSION   " .  (bapi_common::version_define $module) . "\n";
    print $file "\n\n";

    print $file "#include \"driver/nexus_driver_prologue.h\"\n";

    print $file "#include \"" . (bapi_common::ioctl_header $module) . "\"\n";
    print $file "\n\n\n";
    generate_union $file, $module, $funcs, $structs;
    print $file "\n\n\n";
    bapi_classes::generate_class_table $file, $classes;
    print $file "\n\n\n";
    bapi_classes::generate_meta $file, $module, $version, $structs, $classes, $funcs, $class_handles, {KIND=>'driver'};
    print $file "\n\n\n";
    print $file "#include \"driver/nexus_driver_body.h\"\n";
    print $file "\n\n\n";
    generate_ioctl $file, $module, $version, $structs, $classes, $funcs, $class_handles;
    print $file "\n\n\n";
    print $file "#include \"driver/nexus_driver_epilogue.h\"\n";
    print $file "\n\n\n";
    close FILE;
}

1;

