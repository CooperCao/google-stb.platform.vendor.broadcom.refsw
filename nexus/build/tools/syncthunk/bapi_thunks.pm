#!/usr/bin/perl
#  Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

package bapi_thunks;

use bapi_util;
use bapi_classes;

# for user-mode IPC, server-side does not pass through the IPC thunk. Therefore a minimal amount of
# hook is needed in the one thunk that is passed: the sync thunk.
sub generate_ipc_code
{
    my ($func, $destructors, $classes) = @_;
    my @server_pre_lock;
    my @server_post_lock;
    my @server_post_success;
    my $class;

    CLASS: for $class (@$classes) {
        if ($class->{DESTRUCTOR} == $func) {
            my $params = $func->{PARAMS};
            my $handle = $$params[0]->{NAME};
            for (@{$class->{SHUTDOWN}}) {
                my $shutdown_target = $_->{'shutdown_target'};
                my $shutdown_get_connector = $_->{'shutdown_get_connector'};
                push @server_pre_lock, "nexus_driver_shutdown_$shutdown_target($shutdown_get_connector($handle));";
            }
            push @server_post_lock, "#if NEXUS_SERVER_SUPPORT";
            push @server_post_lock, "$class->{CLASS_NAME}_BaseObject_P_RegisterUnregister((void*)$handle, NEXUS_Object_P_RegisterUnregister_eSyncThunkDestroy, NEXUS_MODULE_SELF);";
            push @server_post_lock, "#endif";
        }
        for (@{$class->{CONSTRUCTORS}}) {
            if($_ == $func) {
                push @server_post_success, "#if NEXUS_SERVER_SUPPORT";
                push @server_post_success, "$class->{CLASS_NAME}_BaseObject_P_RegisterUnregister((void*)result, NEXUS_Object_P_RegisterUnregister_eSyncThunkCreate, NEXUS_MODULE_SELF);";
                push @server_post_success, "#endif";
            }
        }
    }
    
    my %result;
    $result{'server_pre_lock'} = \@server_pre_lock;
    $result{'server_post_lock'} = \@server_post_lock;
    $result{'server_post_success'} = \@server_post_success;
    return \%result;
}

sub get_thunked_funcs
{
    my ($funcs)=@_;
    my $func;
    my @thunked_funcs;

    for $func (@{$funcs}) {
        next if(exists $func->{ATTR}->{'local'});
        if(exists $func->{ATTR}->{'thunk'}) {
            next if($func->{ATTR}->{'thunk'} eq 'false');
            print STDERR "Unsupported attribute 'thunk'='$func->{ATTR}->{'thunk'}' in $func->{FUNCNAME}\n";
        }
        push @thunked_funcs, $func;
    }
    return \@thunked_funcs;
}

sub build_thunks
{
    my ($module, $filename, $funcs, $structs) = @_;
    my $func;
    my $destructors = bapi_classes::get_destructors $funcs;
    my $classes = bapi_classes::get_classes $funcs, $destructors, $structs;

    open FILE, ">$filename" or die "Can't open $filename";
    print FILE "/*********************************\n";
    print FILE "*\n";
    print FILE "* This file is autogenerated by the Nexus Platform makefile.\n";
    print FILE "*\n";
    print FILE "* This file acquires the module lock for every public API call into that module. The actual implementation of each function is remapped to _impl.\n";
    print FILE "*\n";
    print FILE "*********************************/\n";
    print FILE "#define NEXUS_THUNK_LAYER\n";
    print FILE "#include \"nexus_${module}_module.h\"\n";
    print FILE "#include \"nexus_core_utils.h\"\n";
    print FILE "#include \"b_objdb.h\"\n";
    print FILE "#include \"nexus_base_statistics.h\"\n";
    print FILE "BDBG_MODULE(nexus_${module}_thunks);\n";
    print FILE "BDBG_FILE_MODULE(nexus_trace_${module});\n";
    print FILE "#if NEXUS_P_SYNC_TRACE\n";
    print FILE "#define NEXUS_P_TRACE_MSG(x) BDBG_MODULE_MSG(nexus_trace_${module}, x)\n";
    print FILE "#else\n";
    print FILE "#define NEXUS_P_TRACE_MSG(x) \n";
    print FILE "#endif\n";
    print FILE "#if BDBG_DEBUG_BUILD\n";
    print FILE "#define NEXUS_THUNKS_TRACE(x) BERR_TRACE(x)\n";
    print FILE "#else\n";
    print FILE "#define NEXUS_THUNKS_TRACE(x) (x)\n";
    print FILE "#endif\n";
    print FILE "/* use local static function in order to reduce size of compiled code, size would reduce since NEXUS_XXXModule expanded to function call with three arguments (where at least one is a global symbol) */\n";
    print FILE "static void module_lock(const char *func) { if (!NEXUS_TryLockModule()) {NEXUS_Module_SetPendingCaller(NEXUS_MODULE_SELF,func);NEXUS_LockModule();NEXUS_Module_ClearPendingCaller(NEXUS_MODULE_SELF,func);}}\n";
    print FILE "static void module_unlock(void) { NEXUS_UnlockModule();}\n";

    my $thunked_funcs = get_thunked_funcs($funcs);

    # generate prototypes for all the _impl functions
    for $func (@$thunked_funcs) {
        my $impl = $func->{PROTOTYPE};
        $impl =~ s/$func->{FUNCNAME}/$func->{FUNCNAME}_impl/;
        print FILE "$impl;\n";
    }

    # generate the actual thunk layer functions which call the impl functions
    for $func (@$thunked_funcs) {
        my $params = $func->{PARAMS};
        my $param;
        my $generated_code = generate_ipc_code $func, $destructors, $classes;

        print FILE "$func->{PROTOTYPE}\n\{\n";
        if ($func->{RETTYPE} eq "void") {
        }
        else {
            print FILE "  $func->{RETTYPE} result;\n";
        }
        print FILE "    NEXUS_P_API_STATS_STATE();\n";

        print FILE "    /* verify that nexus is initialized  */\n";
        print FILE "  if(!NEXUS_MODULE_SELF) {\n";
        if ($func->{RETTYPE} eq "void") {
            print FILE "        return;\n";
        } elsif ($func->{RETTYPE} eq "NEXUS_Error") {
            print FILE "        return NEXUS_THUNKS_TRACE(NEXUS_NOT_INITIALIZED);\n";
        } else {
            print FILE "        (void)NEXUS_THUNKS_TRACE(NEXUS_NOT_INITIALIZED);return ($func->{RETTYPE})0;\n";
        }
        print FILE "  }\n";

        print FILE "    NEXUS_P_API_STATS_START(); /* this statistics count all API call overhead (including synchronization overhead) */\n";
        # enter MSG
        print FILE "  NEXUS_P_TRACE_MSG((\">%s\(";
        for $param (@$params) {
            print FILE "%#lx";
            if ($param != $params->[-1]) { print FILE ", "; }
        }
        print FILE ")\" ";
        print FILE ", \"$func->{FUNCNAME}\"";
        for $param (@$params) {
            print FILE ", ";
            print FILE "(unsigned long)$param->{NAME}";
        }
        print FILE "));\n";

        if (scalar @{$generated_code->{'server_pre_lock'}}) {
            my $file = \*FILE;
            bapi_util::print_code $file, $generated_code->{'server_pre_lock'}, "  ";
        }
        my $stopcallbacks_handle = bapi_classes::get_stopcallbacks_handle $func, $classes;
        if (defined $stopcallbacks_handle) {
            print FILE "NEXUS_StopCallbacks((void*)$stopcallbacks_handle);\n";
        }

        # make call
        print FILE "  module_lock(__FUNCTION__);\n";

        if (scalar @{$generated_code->{'server_post_lock'}}) {
            my $file = \*FILE;
            bapi_util::print_code $file, $generated_code->{'server_post_lock'}, "  ";
        }

        if ($func->{RETTYPE} eq "void") {
            print FILE "  $func->{FUNCNAME}_impl(";
        }
        else {
            print FILE "  result = $func->{FUNCNAME}_impl(";
        }
        for $param (@$params) {
            print FILE "$param->{NAME}";
            if ($param != $params->[-1]) { print FILE ", "; }
        }
        print FILE ");\n";

        # leave MSG and return value
        if ($func->{RETTYPE} eq "void") {
            print FILE "  module_unlock();\n";
            if (defined $stopcallbacks_handle) {
                print FILE "NEXUS_StartCallbacks((void*)$stopcallbacks_handle);\n";
            }
            print FILE "  NEXUS_P_TRACE_MSG((\"<%s\", \"$func->{FUNCNAME}\"));\n";
            print FILE "  NEXUS_P_API_STATS_STOP(\"$func->{FUNCNAME}\",NEXUS_MODULE_SELF); /* this statistics count all API call overhead */\n";
            print FILE "  return;\n";
        }
        else {
            if (scalar @{$generated_code->{'server_post_success'}}) {
                if ($func->{RETTYPE} eq "NEXUS_Error") {
                    print FILE "  if (!result) {\n";
                }
                else {
                    print FILE "  if (result) {\n";
                }
                my $file = \*FILE;
                bapi_util::print_code $file, $generated_code->{'server_post_success'}, "    ";
                print FILE "  }\n";
            }
            print FILE "  module_unlock();\n";
            if (defined $stopcallbacks_handle) {
                print FILE "NEXUS_StartCallbacks((void*)$stopcallbacks_handle);\n";
            }
            print FILE "  NEXUS_P_TRACE_MSG((\"<%s\=%#lx\", \"$func->{FUNCNAME}\", (unsigned long)result));\n";
            print FILE "  NEXUS_P_API_STATS_STOP(\"$func->{FUNCNAME}\", NEXUS_MODULE_SELF); /* this statistics count all API call overhead */\n";
            print FILE "  return result;\n";
        }
        print FILE "}\n\n";
    }
    close FILE;
}

sub build_remapping
{
    my ($filename, $funcs) = @_;
    my $func;
    open FILE, ">$filename" or die;

    print FILE "/*********************************\n";
    print FILE "*\n";
    print FILE "* This file is autogenerated by the Nexus Platform makefile.\n";
    print FILE "*\n";
    print FILE "* This file remaps every public API function to _impl. This allows the Nexus modules to call their own public API without reacquiring the module lock.\n";
    print FILE "*\n";
    print FILE "*********************************/\n";
    print FILE "#ifndef NEXUS_THUNK_LAYER\n";

    my $thunked_funcs = get_thunked_funcs($funcs);
    for $func (@{$thunked_funcs}) {
        print FILE "#define $func->{FUNCNAME} $func->{FUNCNAME}_impl\n";
    }
    print FILE "#endif\n";
    print FILE "\n";
    close FILE;
}


1;
