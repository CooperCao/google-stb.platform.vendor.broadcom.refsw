#!/usr/bin/perl
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
package bapi_verify;

use strict;
use warnings FATAL => 'all';
use Data::Dumper;
use bapi_util;
use bapi_classes;

my %plain_types = map {$_ => 1} qw (
bool
char
int
int16_t
int32_t
int64_t
NEXUS_Addr
uint16_t
uint32_t
uint64_t
uint8_t
unsigned
);
foreach (
    'unsigned short',
    'unsigned char',
) {
    $plain_types{$_}=1;
}

my %special_types = (
'NEXUS_CallbackDesc'=> { 'FORMAT' => '%p' , 'CONVERT' => ''},
'NEXUS_P_MemoryUserAddr' => { 'FORMAT' => '%p', 'CONVERT' => ''},
'NEXUS_KeySlotTag' => {'FORMAT' => '%p', 'CONVERT' => ''},
'NEXUS_AnyObject' => {'FORMAT' => '%p', 'CONVERT' => ''},
'size_t'    => {'FORMAT' => '%u', 'CONVERT' => '(unsigned)'},
'unsigned long' => {'FORMAT' => '%lu', 'CONVERT' => ''}
);

sub filter_array {
    my $field = shift;
    $field =~ s/\[[^\]]+\]/[1]/g;
    return $field;
}

sub verify_one_struct {
    my ($fout, $class_handles, $structs, $name, $path, $fields) = @_;
    my $field;
    for $field (@$fields) {
        my $type = $field->{TYPE};
        my $field_name = filter_array($field->{NAME});
        if(exists $structs->{$type}) {
            print $fout, "    /* $field->{NAME} $type */\n";
            verify_one_struct ($class_handles, $structs, $name, $path . " $field->{NAME} .", $structs->{$type});
        }
        print $fout "    VERIFY_FIELD($name,  $path $field_name, $type )\n";
        next if(exists $plain_types{$type});
        next if(exists $special_types{$type});
        next if(bapi_util::is_special_handle($type) || bapi_util::is_class_handle($type, $class_handles));
        if( (exists $field->{ATTR}{memory} && $field->{ATTR}{memory} eq 'cached') ||
             (exists $field->{ATTR}->{kind} && $field->{ATTR}->{kind} eq 'null_ptr')) {
            print $fout "    VERIFY_POINTER($name, $path $field_name, $type)\n";
        } elsif(bapi_util::is_handle($type, $class_handles)) {
            print $fout "    VERIFY_FAKE_HANDLE($name, $path $field_name, $type)\n";
        } else {
            print $fout "    VERIFY_PLAIN_TYPE($name, $path $field_name, $type)\n";
        }
    }
}

sub print_includes {
    my ($fout,$includes) = @_;
    for(@$includes) {
        if(m!([^/]+)$!) {
            print $fout "#include \"$1\"\n";
        }
    }
}

sub verify_struct
{
    my ($fout, $struct,$class_handles,$_name) = @_;
    my $name;
    my $fields;
    print $fout "\nVERIFY_BEGIN($_name)\n\n";
    while (($name, $fields) = each %$struct) {
        verify_one_struct ($fout, $class_handles, $struct, $name, '', $fields);
        print $fout "    VERIFY_STRUCT($name)\n";
        print $fout "\n\n";
    }
    print $fout "VERIFY_END($_name)\n";
}

sub print_struct
{
    my ($fout, $structs,$class_handles,$translate) = @_;
    my $struct_name;
    my $fields;
    while (($struct_name, $fields) = each %$structs) {
        my $cur_substruct=[];
        print Dumper($struct_name,$fields) if 0;
        my $kind = $fields->[0]{KIND}[0];
        if($translate) {
            print $fout "\n\ntypedef $kind B_NEXUS_COMPAT_TYPE($struct_name) {\n";
        } else {
            print $fout "\n\ntypedef $kind $struct_name {\n";
        }
        for my $field (@$fields) {
            print Dumper($field) if 0;
            my $substruct = [split(/\./,$field->{NAME})];
            print Dumper($field->{NAME},$substruct) if 0;
            my $name = $substruct->[$#$substruct];
            my $n;
            my $match;
            for $n (0...($#$cur_substruct-1)) {
                if(exists $substruct->[$n] and $substruct->[$n] eq $cur_substruct->[$n]) {
                    $match = $n;
                } else {
                   last;
                }
            }
            print Dumper($match,$#$cur_substruct, $cur_substruct,$substruct) if 0;
            if(defined $match) {
                $match++;
            } else {
                $match = 0;
            }
            for (reverse $match..($#$cur_substruct-1)) {
              print $fout "  " x ($_+1);
              print $fout "} $cur_substruct->[$_];\n";
            }
            for ($match..($#$substruct-1)) {
                print $fout "  " x ($_+1); print  $fout "$field->{KIND}[$_+1] {\n";
            }
            my $type = $field->{TYPE};
            if($translate) {
                if(exists $structs->{$field->{TYPE}}) {
                    $type = "struct B_NEXUS_COMPAT_TYPE($type})";
                } elsif(exists $field->{ATTR}->{memory}) {
                    $type = "B_NEXUS_COMPAT_TYPE_MEMORY($type)";
                } elsif(exists $field->{ATTR}->{kind} && $field->{ATTR}->{kind} eq 'null_ptr') {
                    $type = "B_NEXUS_COMPAT_TYPE_POINTER($type)";
                } elsif(bapi_util::is_handle($field->{TYPE}, $class_handles)) {
                    $type = "B_NEXUS_COMPAT_TYPE_HANDLE($type)";
                } elsif(exists $special_types{$type}) {
                    my $compat = $type;
                    $compat =~ s/ /_/g;
                    $type = "B_NEXUS_COMPAT_TYPE_MISC($type,$compat)";
                }
            }
            my $comment = "/* $field->{NAME} */";
            if(exists $field->{COMMENT}) {
                $comment = "/* $field->{COMMENT} */ $comment";
            }
            print $fout "  " x ($#$substruct+1);
            print $fout "$type $name; $comment\n";

            $cur_substruct = $substruct;
        }
        for (reverse 0..($#$cur_substruct-1)) {
          print $fout "  " x ($_+1);
          print $fout "} $cur_substruct->[$_];\n";
        }
        if($translate) {
            print $fout "} B_NEXUS_COMPAT_TYPE($struct_name);\n";
        } else {
            print $fout "} $struct_name;\n";
        }
    }
}

sub add_substruct {
    my ($new_fields, $name, $_kind, $fields) = @_;
    for my $field (@$fields) {
        my %paramhash = %$field;
        $paramhash{NAME} = "$name.$field->{NAME}";
        my @kind = ($_kind);
        push @kind, @{$paramhash{KIND}};
        $paramhash{KIND} = \@kind;
        push @$new_fields, \%paramhash;
    }
}

sub flat_name {
    my ($field,$name) = @_;
    $name =~ s/\./_/g;
    "_${field}_$name";
}

sub process_functions {
    my ($fout, $module, $functions, $class_handles, $structs) = @_;
    my @fields_func_in;
    my @fields_func_out;
    my @ioctls;
    my @args;
    for my $func (sort {$a->{FUNCNAME} cmp $b->{FUNCNAME}}  @$functions) {
        next if(exists $func->{ATTR}{'local'});

        my $funcname = $func->{FUNCNAME};
        my $params = $func->{PARAMS};
        my @fields_in;
        my @fields_out;
        my @fields_varargs_in;
        my @fields_varargs_out;
        my @fields_pointer_in;
        my @fields_pointer_out;
        my @fields_pointer_in_null;
        my @fields_pointer_out_null;
        my @fields_memory_in;
        my @fields_memory_out;
        my @ioctl_pointers;
        my @ioctl_func;
        my $retval = {
            KIND => ['struct'],
            NAME => '__retval'
        };
        if ( $func->{RETTYPE} !~ /void\s*\*$/ && $func->{RETTYPE} =~ /\*$/) {
            my $rettype = $func->{RETTYPE};
            $rettype =~ s/const//;
            $rettype =~ s/\*//;
            $retval->{TYPE} = $rettype;
        } elsif ($func->{RETTYPE} ne "void") {
            $retval->{TYPE} =$func->{RETTYPE};
        }
        push @fields_out, $retval if exists $retval->{TYPE};
        for my $param (@$params) {
            my $field = {
                KIND => ['struct'],
                NAME => $param->{NAME},
            };
            my $ioctl_field = {
                KIND => ['struct'],
                NAME => $param->{NAME},
                TYPE => $param->{TYPE}
            };
            if(exists $param->{ATTR}) {
                if(exists $param->{ATTR}{nelem}) {
                    my $null = {KIND => ['struct'], NAME => $param->{NAME}, TYPE => 'bool'};
                    $field->{TYPE} = 'int';
                    push @ioctl_pointers, $ioctl_field;
                    if ($param->{INPARAM}) {
                        push @fields_pointer_in_null, $null;
                        push @fields_varargs_in, $field;
                    } else {
                        push @fields_pointer_out_null, $null;
                        push @fields_varargs_out, $field;
                    }
                    if(exists $structs->{$param->{BASETYPE}}) {
                        foreach (@{$structs->{$param->{BASETYPE}}}) {
                            if(exists $_->{ATTR}{memory} && $_->{ATTR}{memory} eq 'cached') {
                                my $memory = {KIND => ['struct'], NAME => (flat_name ($param->{NAME} , $_->{NAME})), TYPE => 'int', COMMENT => 'array of NEXUS_Addr'};
                                my $ioctl_memory = {KIND => ['struct'], NAME => (flat_name ($param->{NAME} , $_->{NAME})), TYPE => 'NEXUS_Addr *', COMMENT => 'array of NEXUS_Addr'};
                                push @ioctl_pointers, $ioctl_memory;
                                if ($param->{INPARAM}) {
                                    push @fields_varargs_in, $memory;
                                } else {
                                    push @fields_varargs_out, $memory;
                                }
                            }
                        }
                    }
                    next;
                }
                if(exists $param->{ATTR}{memory} && $param->{ATTR}{memory} eq 'cached') {
                    my $null = {KIND => ['struct'], NAME => $param->{NAME}, TYPE => 'bool'};
                    $field->{TYPE} = 'NEXUS_Addr';
                    push @ioctl_pointers, $ioctl_field;
                    if ($param->{INPARAM}) {
                        push @fields_pointer_in_null, $null;
                        push @fields_memory_in, $field;
                    } else {
                        push @fields_pointer_out_null, $null;
                        push @fields_memory_out, $field;
                    }
                    next;
                }
            }
            if($param->{ISREF} && $param->{BASETYPE} ne 'void') {
                my $null = {KIND => ['struct'], NAME => $param->{NAME}, TYPE => 'bool'};
                $field->{TYPE} = $param->{BASETYPE};
                if(exists $structs->{$param->{BASETYPE}}) {
                    foreach (@{$structs->{$field->{TYPE}}}) {
                        if(exists $_->{ATTR}{memory} && $_->{ATTR}{memory} eq 'cached') {
                            my $memory = {KIND => ['struct'], NAME => (flat_name ($param->{NAME} , $_->{NAME})), TYPE => 'NEXUS_Addr'};
                            if ($param->{INPARAM}) {
                                push @fields_memory_in, $memory;
                            } else {
                                push @fields_memory_out, $memory;
                            }
                        }
                    }
                }
                push @ioctl_pointers, $ioctl_field;
                if ($param->{INPARAM}) {
                    push @fields_pointer_in, $field;
                    push @fields_pointer_in_null, $null;
                } else {
                    push @fields_pointer_out, $field;
                    push @fields_pointer_out_null, $null;
                }
            } else {
                $field->{TYPE} = $param->{TYPE};
                push @fields_in, $field;
            }
        }
        my @in;
        my @ioctl_in;
        if(scalar @fields_in) {
            my $name = "\U$module\E_${funcname}_in_args";
            push @args, {$name => \@fields_in};
            push @in, {KIND => ['struct'], NAME => 'args' , TYPE => $name};
            push @ioctl_in, {KIND => ['struct'], NAME => 'args' , TYPE => $name};
        }
        add_substruct(\@in, 'vararg', 'struct', \@fields_varargs_in);
        add_substruct(\@fields_pointer_in, 'is_null', 'struct', \@fields_pointer_in_null);
        add_substruct(\@fields_pointer_in, 'out_is_null', 'struct', \@fields_pointer_out_null);
        add_substruct(\@in, 'pointer', 'struct', \@fields_pointer_in);
        add_substruct(\@in, 'memory', 'struct', \@fields_memory_in);
        if(scalar @in == 0) {
            my $unused = {KIND => ['struct'], NAME => '__unused' , TYPE => 'unsigned'};
            push @in, $unused;
        }
        add_substruct(\@ioctl_in, 'pointer', 'struct', \@ioctl_pointers);
        add_substruct(\@ioctl_in, 'memory', 'struct', \@fields_memory_in);

        my @out;
        if(scalar @fields_out) {
            my $name = "b_${funcname}_out_args";
            push @args, {$name => \@fields_out};
        }
        add_substruct(\@out, 'ret', 'struct', \@fields_out);
        add_substruct(\@out, 'pointer', 'struct', \@fields_pointer_out);
        add_substruct(\@out, 'vararg', 'struct', \@fields_varargs_out);
        add_substruct(\@out, 'memory', 'struct', \@fields_memory_out);
        if(scalar @out == 0) {
            my $unused = {KIND => ['struct'], NAME => '__unused' , TYPE => 'unsigned'};
            push @out, $unused;
        }
        my @ioctl_out;
        add_substruct(\@ioctl_out, 'ret', 'struct', \@fields_out);
        add_substruct(\@ioctl_out, 'memory', 'struct', \@fields_memory_out);

        add_substruct(\@fields_func_in, "_${funcname}", 'union', \@in);
        add_substruct(\@fields_func_out, "_${funcname}", 'union', \@out);

        add_substruct(\@ioctl_func, 'in', 'union', \@ioctl_in);
        add_substruct(\@ioctl_func, 'out', 'union', \@ioctl_out);
        if(scalar @ioctl_func == 0) {
            my $unused = {KIND => ['struct'], NAME => '__unused' , TYPE => 'unsigned'};
            push @ioctl_func, $unused;
        }
        push @ioctls, {"\U$module\E_${funcname}_data" => \@ioctl_func};
    }
    my $struct_in = { "b_${module}_module_ipc_in" => \@fields_func_in};
    my $struct_out = { "b_${module}_module_ipc_out" => \@fields_func_out};
    my $struct = [
        {KIND => ['union'], NAME => 'in' , TYPE => "b_${module}_module_ipc_in"},
        {KIND => ['union'], NAME => 'out' , TYPE => "b_${module}_module_ipc_out"}
    ];

    return [$struct_in, $struct_out, \@ioctls, \@args];
}

sub process_arguments {
    my ($functions, $func,$class_handles, $structs) = @_;
    my $code = {};
    my $funcname = $func->{FUNCNAME};
    my $api = "_${funcname}";
    my $arg_no = 0;
    my @trace_format;
    my @trace_names;
    my @trace_args;
    for my $param (@{$func->{PARAMS}} ) {
        $arg_no++;
        my $callback_kind='_UNKNOWN';
        push @trace_names,$param->{NAME};
        if($param->{ISREF} || bapi_util::is_handle($param->{TYPE}, $class_handles)) {
            push @trace_format, "%p";
            push @trace_args, "(void *)$param->{NAME}";
        } elsif (exists $special_types{$param->{TYPE}}) {
            push @trace_format, $special_types{$param->{TYPE}}->{FORMAT};
            push @trace_args,$special_types{$param->{TYPE}}->{CONVERT} . $param->{NAME};
        } else {
            push @trace_format, "%u";
            push @trace_args, "(unsigned)$param->{NAME}";
        }
        if (bapi_util::is_handle($func->{RETTYPE}, $class_handles)) {
            $callback_kind='_CONSTRUCTOR';
        } elsif (bapi_util::is_handle($func->{PARAMS}[0]{TYPE}, $class_handles)) {
            $callback_kind= '';
        } elsif ($func->{PARAMS}[0]{ISREF}) {
            if($arg_no == 1 && (scalar @{$func->{PARAMS}} == 1)) {
                $callback_kind = '_INIT';
            } else {
                $callback_kind = '_UNKNOWN';
            }
        } else {
            $callback_kind = '_ENUM';
        }
        if(exists $param->{BASETYPE} && $param->{BASETYPE} eq 'NEXUS_CallbackDesc') {
            my $id = sprintf("0x%05x", ((bapi_util::func_id $functions, $func)*256 + $arg_no + 0x10000));
            if ($param->{INPARAM}) {
                push @{$code->{SERVER}{CALLBACK_PRE}}, "B_IPC_CALLBACK${callback_kind}_IN_PREPARE($api, $param->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
                push @{$code->{SERVER}{CALLBACK_POST}}, "B_IPC_CALLBACK${callback_kind}_IN_FINALIZE($api, $param->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
            } else {
                push @{$code->{SERVER}{CALLBACK_POST}}, "B_IPC_CALLBACK${callback_kind}_OUT($api, $param->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
            }
        }
        if(exists $param->{ATTR}{nelem}) {
            if ($param->{INPARAM}) {
                my $type = $param->{BASETYPE};
                $type = 'uint8_t' if $type eq 'void';
                push @{$code->{CLIENT}{SEND_VARARG}}, "B_IPC_CLIENT_SEND_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                push @{$code->{SERVER}{RECV_VARARG}}, "B_IPC_SERVER_RECV_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                push @{$code->{DRIVER}{RECV_VARARG}}, "B_IPC_DRIVER_RECV_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
            }
            if($param->{BASETYPE} eq 'NEXUS_CallbackDesc') {
                push @{$code->{SERVER}{CALLBACK_PRE}}, 'B_IPC_NOT_SUPPORTED("NEXUS_CallbackDesc as varags")';
            }
            if(exists $structs->{$param->{BASETYPE}}) {
                foreach (@{$structs->{$param->{BASETYPE}}}) {
                    if($_->{TYPE} eq 'NEXUS_CallbackDesc') {
                        push @{$code->{SERVER}{CALLBACK_PRE}}, 'B_IPC_NOT_SUPPORTED("NEXUS_CallbackDesc in varags")';
                    }
                    if(exists $_->{ATTR}{memory} && $_->{ATTR}{memory} eq 'cached') {
                        my $name = flat_name ($param->{NAME} , $_->{NAME});
                        if ($param->{INPARAM}) {
                            push @{$code->{CLIENT}{SEND_VARARG}}, "B_IPC_CLIENT_SEND_VARARG_ADDR($api, $param->{NAME}, $param->{TYPE}, $_->{NAME}, $name, $param->{ATTR}{nelem})";
                            push @{$code->{SERVER}{RECV_VARARG}}, "B_IPC_SERVER_RECV_VARARG_ADDR($api, $param->{NAME}, $param->{TYPE}, $_->{NAME}, $name, $param->{ATTR}{nelem})";
                            push @{$code->{DRIVER}{RECV_VARARG}}, "B_IPC_DRIVER_RECV_VARARG_ADDR($api, $param->{NAME}, $param->{TYPE}, $_->{NAME}, $name, $param->{ATTR}{nelem})";
                        } else {
                            push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_OUT_PTR($api, $param->{NAME})";
                            push @{$code->{CLIENT}{RECV_VARARG}}, "B_IPC_CLIENT_RECV_VARARG_ADDR($api, $param->{NAME}, $param->{TYPE}, $_->{NAME}, $name, $param->{ATTR}{nelem})";
                            push @{$code->{SERVER}{PREPARE}}, "B_IPC_SERVER_PREPARE_VARARG_ADDR($api, $param->{NAME}, $param->{TYPE}, $_->{NAME}, $name, $param->{ATTR}{nelem})";
                            push @{$code->{SERVER}{PROCESS}}, "B_IPC_SERVER_PROCESS_VARARG_ADDR($api, $param->{NAME}, $param->{TYPE}, $_->{NAME}, $name, $param->{ATTR}{nelem})";
                            push @{$code->{DRIVER}{SEND_VARARG}}, "B_IPC_SERVER_SEND_VARARG_ADDR($api, $param->{NAME}, $param->{TYPE}, $_->{NAME}, $name, $param->{ATTR}{nelem})";
                        }
                    }
                }
            }
            if (not $param->{INPARAM}) {
                my $type = $param->{BASETYPE};
                $type = 'uint8_t' if $type eq 'void';
                my $nelem = exists $param->{ATTR}{nelem_out} ? "* $param->{ATTR}{nelem_out}" : $param->{ATTR}{nelem};
                push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_OUT_PTR($api, $param->{NAME})";
                push @{$code->{CLIENT}{RECV_VARARG}}, "B_IPC_CLIENT_RECV_VARARG($api, $param->{NAME}, $type, $nelem)";
                push @{$code->{SERVER}{PREPARE}}, "B_IPC_SERVER_PREPATE_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                push @{$code->{DRIVER}{DECLARE}}, "B_IPC_DRIVER_DATA($param->{TYPE}, $param->{NAME})";
                push @{$code->{DRIVER}{ASSIGN}}, "B_IPC_DRIVER_ASSIGN_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{RECV}}, "B_IPC_DRIVER_SET_OUT_PTR($api, $param->{NAME})";
                if(exists $param->{ATTR}{nelem_out}) {
                    push @{$code->{DRIVER}{SEND_VARARG}}, "B_IPC_DRIVER_SEND_VARARG_NELEM_OUT($api, $param->{NAME}, $type, $param->{ATTR}{nelem_out})";
                } else {
                    push @{$code->{DRIVER}{SEND_VARARG}}, "B_IPC_DRIVER_SEND_VARARG_NELEM($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                }
            }
        } elsif(exists $param->{ATTR}{memory} && $param->{ATTR}{memory} eq 'cached') {
            if ($param->{INPARAM}) {
                push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_ADDR($api, $param->{NAME})";
                push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_RECV_ADDR($api, $param->{NAME})";
                push @{$code->{DRIVER}{RECV}}, "B_IPC_DRIVER_RECV_ADDR($api, $param->{NAME})";
            } else {
                push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_OUT_PTR($api, $param->{NAME})";
                push @{$code->{CLIENT}{RECV}}, "B_IPC_CLIENT_RECV_ADDR($api, $param->{NAME})";
                push @{$code->{SERVER}{SEND}}, "B_IPC_SERVER_SEND_ADDR($api, $param->{NAME})";
                push @{$code->{SERVER}{DECLARE}}, "B_IPC_SERVER_DATA($param->{BASETYPE}, $param->{NAME})";
                push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_SET_ADDR($param->{NAME})";
                push @{$code->{DRIVER}{RECV}}, "B_IPC_DRIVER_SET_OUT_ADDR($api, $param->{NAME})";
                push @{$code->{DRIVER}{SEND}}, "B_IPC_DRIVER_COPY_ADDR($api, $param->{NAME})";
            }
        } elsif($param->{ISREF}) {
            if ($param->{INPARAM}) {
                push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_IN_PTR($api, $param->{NAME})";
                push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_RECV_IN_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{RECV_PTR}}, "B_IPC_DRIVER_RECV_IN_PTR($api, $param->{NAME})";
            } else {
                push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_OUT_PTR($api, $param->{NAME})";
                push @{$code->{CLIENT}{RECV}}, "B_IPC_CLIENT_RECV_OUT_PTR($api, $param->{NAME})";
                push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_SET_OUT_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{DECLARE}}, "B_IPC_DRIVER_DATA($param->{TYPE}, $param->{NAME})";
                push @{$code->{DRIVER}{ASSIGN}}, "B_IPC_DRIVER_ASSIGN_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{RECV}}, "B_IPC_DRIVER_SET_OUT_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{SEND}}, "B_IPC_DRIVER_COPY_OUT_PTR($api, $param->{NAME})";
            }
            if(exists $structs->{$param->{BASETYPE}}) {
                my $field_no = 0;
                foreach (@{$structs->{$param->{BASETYPE}}}) {
                    $field_no++;
                    if( $_->{TYPE} eq 'NEXUS_CallbackDesc') {
                        my $id = sprintf("0x%04x", ((bapi_util::struct_id $structs, $param->{BASETYPE})*256 + $field_no));
                        if ($param->{INPARAM}) {
                            push @{$code->{SERVER}{CALLBACK_PRE}}, "B_IPC_CALLBACK${callback_kind}_FIELD_IN_PREPARE($api, $param->{NAME}, $_->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
                            push @{$code->{SERVER}{CALLBACK_POST}}, "B_IPC_CALLBACK${callback_kind}_FIELD_IN_FINALIZE($api, $param->{NAME}, $_->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
                        } else {
                            push @{$code->{SERVER}{CALLBACK_POST}}, "B_IPC_CALLBACK${callback_kind}_FIELD_OUT($api,$param->{NAME}, $_->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
                        }
                    }
                    if(exists $_->{ATTR}{memory} && $_->{ATTR}{memory} eq 'cached') {
                        my $name = flat_name ($param->{NAME} , $_->{NAME});
                        if ($param->{INPARAM}) {
                            push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_FIELD_ADDR($api, $param->{NAME}, $_->{NAME}, $name)";
                            push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_RECV_FIELD_ADDR($api, $param->{NAME}, $_->{NAME}, $name)";
                            push @{$code->{DRIVER}{COPY_IN}}, "B_IPC_DRIVER_RECV_FIELD_ADDR($api, $param->{NAME}, $_->{NAME}, $name)";
                        } else {
                            push @{$code->{CLIENT}{RECV}}, "B_IPC_CLIENT_RECV_FIELD_ADDR($api, $param->{NAME}, $_->{NAME}, $name)";
                            push @{$code->{SERVER}{SEND}}, "B_IPC_SERVER_SEND_FIELD_ADDR($api, $param->{NAME}, $_->{NAME}, $name)";
                            push @{$code->{DRIVER}{COPY_OUT}}, "B_IPC_DRIVER_SEND_FIELD_ADDR($api, $param->{NAME}, $_->{NAME}, $name)";
                        }
                    }
                }
            }
        } else {
            push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND($api, $param->{NAME})";
            push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_RECV($api, $param->{NAME})";
            push @{$code->{DRIVER}{RECV}}, "B_IPC_DRIVER_RECV($api, $param->{NAME})";
        }
    }
    $code->{TRACE}{FORMAT} = join(' ', map {"$trace_names[$_]:$trace_format[$_]"} (0..$#trace_names));
    $code->{TRACE}{ARG} = ( scalar @trace_args ? ',' : '')  . join(',', @trace_args);

    my $result_format='';
    my $result_args='';
    if($func->{RETTYPE} ne 'void') {
        if(bapi_util::is_handle($func->{RETTYPE}, $class_handles)) {
            $result_format=' result:%p';
            $result_args=',(void *)__result';
        } else {
            $result_format=' result:%u';
            $result_args=',__result';
        }
        push @{$code->{DRIVER}{RESULT}}, "B_IPC_DRIVER_RESULT($api)";
    }
    $code->{TRACE}{RESULT}{FORMAT} = $result_format;
    $code->{TRACE}{RESULT}{ARG} = $result_args;

    $code;
}

my $autogenerated ="
/**************************************************************
 *                                                            *
 * This file is autogenerated by the Nexus Platform makefile. *
 *                                                            *
 **************************************************************/
";

sub generate_ipc_client {
    my ($fout, $module, $functions, $class_handles, $structs) = @_;
    print $fout $autogenerated;
    print $fout "
#define NEXUS_PROXY_THUNK_LAYER\n
#include \"client/nexus_client_prologue.h\"
#include \"nexus_${module}_module.h\"
#include \"nexus_core_utils.h\"
#include \"client/nexus_client_units.h\"
#include \"nexus_${module}_abiverify_api.h\"

#include \"nexus_${module}_abiverify_client.h\"
";
}

sub generate_proxy_client {
    my ($fout, $module, $functions, $class_handles, $structs) = @_;
    print $fout $autogenerated;
    print $fout "
#define NEXUS_PROXY_THUNK_LAYER\n
#include \"proxy/nexus_proxy_prologue.h\"
#include \"nexus_${module}_module.h\"
#include \"nexus_core_utils.h\"

#include \"nexus_${module}_abiverify_ioctl.h\"
#define nexus_proxy_module_init nexus_proxy_${module}_init
#define nexus_proxy_module_uninit nexus_proxy_${module}_uninit
#define nexus_proxy_module_state nexus_proxy_${module}_state
#define NEXUS_IOCTL_MODULE_INIT IOCTL_\U${module}\E_NEXUS_INIT
#define NEXUS_PROXY_MODULE_VERSION NEXUS_${module}_MODULE_VERSION
#define NEXUS_PROXY_MODULE_NAME \"\U${module}\E\"

BDBG_MODULE(nexus_${module}_proxy);

#include \"proxy/nexus_proxy_body.h\"

#include \"proxy/nexus_proxy_units.h\"
#include \"nexus_${module}_abiverify_client.h\"

";
}

sub generate_ioctl {
    my ($fout, $module, $functions, $class_handles, $structs, $module_number, $ioctls) = @_;
    my $id = 101;
    print $fout $autogenerated;
    print $fout "
#include \"nexus_${module}_api.h\"
/* ioctl dispatch is faster if ioctl number doesn't include size of the structure */
#define NEXUS_IOCTL_\U${module}\E_ID  $module_number
";

    for (@$ioctls) {
        print_struct($fout, $_, $class_handles, 0);
    }
    print $fout "#define IOCTL_\U${module}\E_NEXUS_INIT NEXUS_IOCTL($id, NEXUS_IOCTL_\U${module}\E_ID*NEXUS_IOCTL_PER_MODULE+0, PROXY_NEXUS_ModuleInit)\n";
    for my $func (sort {$a->{FUNCNAME} cmp $b->{FUNCNAME}}  @$functions) {
        next if(exists $func->{ATTR}{'local'});
        my $funcname = $func->{FUNCNAME};
        print $fout "#define IOCTL_\U${module}\E_${funcname} NEXUS_IOCTL($id, NEXUS_IOCTL_\U${module}\E_ID*NEXUS_IOCTL_PER_MODULE+NEXUS_P_API_${module}_${funcname}_id+1, b_${funcname}_data)\n";
    }
}

my $tab = '    ';

sub generate_driver {
    my ($fout, $module, $functions, $class_handles, $structs) = @_;
    print $fout $autogenerated;
    print $fout "
#define NEXUS_DRIVER_MODULE_NAME \"\U${module}\E\"
#include \"nexus_${module}_module.h\"
BDBG_MODULE(nexus_${module}_driver);
BDBG_FILE_MODULE(nexus_trace_${module});
#if NEXUS_P_TRACE
#define NEXUS_P_TRACE_MSG(x) BDBG_MODULE_MSG(nexus_trace_${module}, x)
#else
#define NEXUS_P_TRACE_MSG(x)
#endif
#include \"nexus_core_utils.h\"

/* defines to make all module symbols uniques */
#define nexus_driver_module_ioctl nexus_driver_${module}_ioctl
#define nexus_driver_module_open nexus_driver_${module}_open
#define nexus_driver_module_close nexus_driver_${module}_close
#define nexus_driver_module_state nexus_driver_${module}_state
#define nexus_driver_module_args nexus_driver_${module}_args
#define nexus_driver_module_data struct { b_${module}_module_ipc_in in; b_${module}_module_ipc_out out;} data;

#define NEXUS_IOCTL_MODULE_INIT      IOCTL_\U${module}\E_NEXUS_INIT
#define NEXUS_IOCTL_MODULE_VERSION   NEXUS_${module}_MODULE_VERSION


#include \"driver/nexus_driver_prologue.h\"
#include \"nexus_${module}_abiverify_ioctl.h\"
#include \"../common/ipc/nexus_ipc_server_api.h\"
#include \"driver/nexus_driver_units.h\"

#define NEXUS_DRIVER_MODULE_CLASS_TABLE    nexus_server_${module}_class_table
extern  const struct b_objdb_class  NEXUS_DRIVER_MODULE_CLASS_TABLE[];
B_IPC_SERVER_DECLARE(${module})

union nexus_driver_module_args {
  PROXY_NEXUS_ModuleInit init;
";

    for my $func (sort {$a->{FUNCNAME} cmp $b->{FUNCNAME}}  @$functions) {
        next if(exists $func->{ATTR}{'local'});
        my $funcname = $func->{FUNCNAME};
        print $fout "  struct {\n";
        print $fout "    \U${module}\E_${funcname}_data ioctl;\n";
        print $fout "  } _${funcname};\n";
    }
    print $fout "};\n";
    print $fout "#include \"driver/nexus_driver_body.h\"\n";

    print $fout "B_IPC_DRIVER_MODULE_BEGIN($module, \U$module\E)\n";
    for my $func (sort {$a->{FUNCNAME} cmp $b->{FUNCNAME}}  @$functions) {
        next if(exists $func->{ATTR}{'local'});
        my $funcname = $func->{FUNCNAME};
        my $api = "_${funcname}";

        print_function_prototype($fout, $func);
        print $fout "B_IPC_DRIVER_BEGIN($module, \U${module}\E, _${funcname})\n";
        my $code = process_arguments($functions, $func, $class_handles, $structs);


        bapi_util::print_code($fout, $code->{DRIVER}{DECLARE}, $tab);
        if(scalar @{$func->{PARAMS}}) {
            print $fout "    B_IPC_DRIVER_RECV_IOCTL($module,  _${funcname})\n";
        }
        bapi_util::print_code($fout, $code->{DRIVER}{ASSIGN}, $tab);
        bapi_util::print_code($fout, $code->{DRIVER}{RECV}, $tab);
        bapi_util::print_code($fout, $code->{DRIVER}{COPY_IN}, $tab);
        bapi_util::print_code($fout, $code->{DRIVER}{RECV_PTR}, $tab);
        bapi_util::print_code($fout, $code->{DRIVER}{RECV_VARARG}, $tab);
        print $fout "    B_IPC_DRIVER_CALL($module, _${funcname})\n";

        bapi_util::print_code($fout, $code->{DRIVER}{COPY_OUT}, $tab);
        bapi_util::print_code($fout, $code->{DRIVER}{RESULT}, $tab);
        bapi_util::print_code($fout, $code->{DRIVER}{SEND}, $tab);
        bapi_util::print_code($fout, $code->{DRIVER}{SEND_VARARG}, $tab);
        if(defined $code->{DRIVER}{COPY_OUT} or defined $code->{DRIVER}{RESULT}) {
            print $fout "    B_IPC_DRIVER_SEND_IOCTL($module,  _${funcname})\n";
        }
        print $fout "B_IPC_DRIVER_END($module, $funcname)\n";
        print $fout "\n";
    }
    print $fout "B_IPC_DRIVER_MODULE_END($module, \U$module\E)\n";

    print $fout "#include \"driver/nexus_driver_epilogue.h\"\n";
}

sub generate_client {
    my ($fout, $module, $functions, $class_handles, $structs, $destructors) = @_;
    print $fout $autogenerated;
    print $fout "B_IPC_CLIENT_MODULE_BEGIN($module, \U$module\E)\n";
    for my $func (sort {$a->{FUNCNAME} cmp $b->{FUNCNAME}}  @$functions) {
        next if(exists $func->{ATTR}{'local'});
        my $is_destructor = 0;
        for my $d (values %$destructors) {
            print Dumper($d) if 0;
            if( (exists  $d->{destructor}{FUNCNAME}) and $d->{destructor}{FUNCNAME} eq $func->{FUNCNAME}) {
                $is_destructor = 1; last;
            }
        }
        my %code;
        my $funcname = $func->{FUNCNAME};
        my $api = "_$func->{FUNCNAME}";
        my @args_list = map  { "$_->{TYPE} $_->{NAME}" } @{$func->{PARAMS}};
        @args_list = ('void') if scalar @args_list == 0;
        my $args = join(",", @args_list);

        my $code = process_arguments($functions, $func, $class_handles, $structs);
        if($is_destructor) {
            print $fout "B_IPC_CLIENT_BEGIN_DESTRUCTOR($module, \U$module\E, $api, $funcname, ($args), $func->{PARAMS}[0]{NAME})\n";
        } elsif($func->{RETTYPE} eq 'void') {
            print $fout "B_IPC_CLIENT_BEGIN_VOID($module, \U$module\E, $api, $funcname, ($args))\n";
        } else {
            print $fout "B_IPC_CLIENT_BEGIN($module, \U$module\E, $func->{RETTYPE}, $api, $funcname, ($args))\n";
        }
        print $fout "    B_IPC_CLIENT_TRACE((\">%s: $code->{TRACE}{FORMAT}\", \"$funcname\"  $code->{TRACE}{ARG}))\n";
        bapi_util::print_code($fout, $code->{CLIENT}{SEND}, $tab);
        bapi_util::print_code($fout, $code->{CLIENT}{SEND_VARARG}, $tab);
        print $fout "    B_IPC_CLIENT_CALL($module, \U$module\E, $api)\n";
        if($is_destructor || $func->{RETTYPE} eq 'void') {
        } else {
            print $fout "    B_IPC_CLIENT_SET_RESULT($func->{RETTYPE}, $api)\n";
        }

        print $fout "    B_IPC_CLIENT_TRACE((\"<%s(%d):$code->{TRACE}{RESULT}{FORMAT} $code->{TRACE}{FORMAT}\", \"$funcname\", __rc $code->{TRACE}{RESULT}{ARG} $code->{TRACE}{ARG}))\n";
        bapi_util::print_code($fout, $code->{CLIENT}{RECV}, $tab);
        bapi_util::print_code($fout, $code->{CLIENT}{RECV_VARARG}, $tab);

        if($is_destructor) {
            print $fout "B_IPC_CLIENT_END_DESTRUCTOR($api, $func->{PARAMS}[0]{NAME})\n";
        } elsif($func->{RETTYPE} eq 'void') {
            print $fout "B_IPC_CLIENT_END_VOID($api)\n";
        } elsif (bapi_util::is_handle($func->{RETTYPE}, $class_handles)) {
            print $fout "B_IPC_CLIENT_END_HANDLE($func->{RETTYPE}, $api)\n";
        } else {
            print $fout "B_IPC_CLIENT_END($api)\n";
        }
        print $fout "\n";
    }
    print $fout "B_IPC_CLIENT_MODULE_END($module)\n";
}

sub generate_ipc_api {
    my ($fout, $module, $functions, $class_handles, $structs, $version ) = @_;
    print $fout $autogenerated;
    print $fout "#include \"nexus_${module}_api.h\"\n";
    print $fout "\n";
    print $fout "typedef struct B_IPC_" . (uc $module) . "_data {\n";
    print $fout "   NEXUS_Ipc_Header header;\n";
    print $fout "   union {\n";
    print $fout "       b_${module}_module_ipc_in in;\n";
    print $fout "       b_${module}_module_ipc_out out;\n";
    print $fout "   } data;\n";
    print $fout "}  B_IPC_\U$module\E_data;\n";
}

sub generate_meta
{
    my ($file, $module, $structs, $classes, $funcs, $class_handles, $mode) = @_;
    my $func;
    my $name;
    my %declared_objects;
    my $in_arg_type = "b_${module}_module_ipc_in";
    my $out_arg_type = "b_${module}_module_ipc_out";

    for $func (@$funcs) {
        next if(exists $func->{ATTR}{'local'});
        my $in_args_count = 0;
        my $out_args_count = 0;
        my $params = $func->{PARAMS};
        my $param;
        my $api = "_$func->{FUNCNAME}";
        my $in_args = $api;
        my $out_args = $api;
        my $isnull_suffix;
        my $isnull_prefix;
        my @function;
        my @objects;
        my @pointers;
        my %referenced_objects;

        for (@{$func->{PARAMS}}) {
            my $param = $_;
            my $args = $param->{INPARAM} ? $in_args : $out_args;
            my $arg_type = $param->{INPARAM} ? $in_arg_type : $out_arg_type;
            if($param->{INPARAM}) {
                $in_args_count++;
            } else {
                $out_args_count++;
            }

            # find arguments that are pointers
            if ($param->{ISREF}) {
                if (!exists $param->{ATTR}->{'nelem'}) {
                    my @pointer;
                    push @pointer, "\"$param->{NAME}\" ,";
                    if(exists $param->{ATTR}{memory} && $param->{ATTR}{memory} eq 'cached') {
                        push @pointer, "NEXUS_OFFSETOF($arg_type, $args . memory . $param->{NAME}), /* offset */";
                    } else {
                        push @pointer, "NEXUS_OFFSETOF($arg_type, $args . pointer . $param->{NAME}), /* offset */";
                    }
                    if($param->{INPARAM}) {
                        push @pointer, "NEXUS_OFFSETOF($arg_type, $args . pointer . is_null . $param->{NAME}), /* null_offset */";
                    } else {
                        push @pointer, "NEXUS_OFFSETOF($in_arg_type, $in_args . pointer . out_is_null . $param->{NAME}), /* null_offset */";
                    }
                    push @pointer, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                    push @pointer, ($param->{ATTR}->{'null_allowed'} ? "true":"false") . '/* null_allowed */';
                    push @pointers, \@pointer;
                } else {
                    my @pointer;
                    push @pointer, "\"$param->{NAME}\" ,";
                    push @pointer, "NEXUS_OFFSETOF($arg_type, $args . vararg. $param->{NAME}), /* offset */";
                    if($param->{INPARAM}) {
                        push @pointer, "NEXUS_OFFSETOF($arg_type, $args . pointer . is_null . $param->{NAME}), /* null_offset */";
                    } else {
                        push @pointer, "NEXUS_OFFSETOF($in_arg_type, $in_args . pointer . out_is_null . $param->{NAME}), /* null_offset */";
                    }
                    push @pointer, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                    push @pointer, ($param->{ATTR}->{'null_allowed'} ? "true":"false") . '/* null_allowed */';
                    push @pointers, \@pointer;
                }
            }

            # find arguments that are pointers to structures and scan them
            if ($param->{ISREF} && exists $structs->{$param->{BASETYPE}}) {
                next if (exists $param->{ATTR}->{'nelem'}); # TODO Currently we can't process objects in variable size arrays
                foreach (@$class_handles) {
                    # first, check if param is a struct which has a class handle field
                    my $handletype = $_;
                    my $struct_field;
                    for $struct_field (@{$structs->{$param->{BASETYPE}}}) {
                        next if $struct_field->{NAME} =~ /\[/; # For now skip arrays, we don't verify data inside arrays
                        if ($struct_field->{TYPE} eq $handletype ) {
                            my $class_name = bapi_classes::get_class_name($handletype);
                            my @object;

                            push @object, '"' . $param->{NAME} . '->' . $struct_field->{NAME} . '",';
                            push @object, "NEXUS_OFFSETOF($arg_type, $args . pointer . $param->{NAME}.$struct_field->{NAME} ), /* offset */";
                            if($param->{INPARAM}) {
                                push @object, "NEXUS_OFFSETOF($arg_type, $args . pointer . is_null . $param->{NAME}), /* null_offset */";
                            } else {
                                push @object, "NEXUS_OFFSETOF($in_arg_type, $in_args . pointer . out_is_null . $param->{NAME}), /* null_offset */";
                            }
                            push @object, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                            push @object, "true, /* null allowed */";
                            $referenced_objects{$class_name} = 1;
                            push @object, "&NEXUS_OBJECT_DESCRIPTOR($class_name) /* descriptor */";
                            push @objects, \@object;
                        }
                    }
                }
            }

            # find arguments that are objects
            foreach (@$class_handles) {
                my $handletype = $_;

                if ($param->{TYPE} eq $handletype) {
                    my @object;
                    my $class_name = bapi_classes::get_class_name($handletype);
                    push @object, "\"$param->{NAME}\" ,";
                    push @object, "NEXUS_OFFSETOF($arg_type, $args . args . $param->{NAME} ),  /*  offset */";
                    push @object, "-1 , /* null_offset */";
                    push @object, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                    push @object, ($param->{ATTR}->{'null_allowed'} ? "true":"false") . ", /* null_allowed */";
                    push @object, "&NEXUS_OBJECT_DESCRIPTOR($class_name) /* descriptor */";
                    push @objects, \@object;
                    $referenced_objects{$class_name} = 1;
                }
            }
        }
        for (sort keys %referenced_objects) {
            if(not exists $declared_objects{$_}) {
                print $file "extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR($_);\n";
                $declared_objects{$_}=1;
            }
        }
        my $nobjects = scalar @objects;
        if ($nobjects) {
            my $n=0;
            print $file "static const struct api_object_descriptor _objects_$func->{FUNCNAME} [] = {\n";
            for (@objects) {
                print $file ",\n" if $n;
                $n++;
                print $file " {\n";
                bapi_util::print_code $file, $_, "   ";
                print $file " }";
            }
            print $file "\n};\n";
        }
        my $npointers = scalar @pointers;
        if($npointers) {
            my $n=0;
            print $file "static const struct api_pointer_descriptor _pointers_$func->{FUNCNAME} [] = {\n";
            for (@pointers) {
                print $file ",\n" if $n;
                $n++;
                print $file " {\n";
                bapi_util::print_code $file, $_, "   ";
                print $file " }";
            }
            print $file "\n};\n";
        }

        push @function, "\"$func->{FUNCNAME}\",";
        if($in_args_count) {
            push @function, "sizeof( (($in_arg_type *)NULL)->$in_args),"
        } else {
            push @function, '0,';
        }
        if($out_args_count) {
            push @function, "sizeof( (($out_arg_type *)NULL)->$out_args),";
        } else {
            push @function, '0,';
        }
        push @function, $func->{CALLABLE_BY_UNTRUSTED} ? "false," : "true,";
        push @function, "$nobjects,";
        push @function, "$npointers,"; # npointers
        push @function, $nobjects==0 ? "NULL,": "_objects_$func->{FUNCNAME},";
        push @function, $npointers==0 ? "NULL,": "_pointers_$func->{FUNCNAME},";
        print $file "static const struct api_function_descriptor _api_$func->{FUNCNAME} = {\n";
        bapi_util::print_code $file, \@function, "   ";
        print $file "};\n";
    }
}

sub print_function_prototype {
    my ($fout, $func) = @_;
    my $args = join(' ,', map  { "$_->{TYPE} $_->{NAME}" } @{$func->{PARAMS}});

    print $fout "/* $func->{RETTYPE} $func->{FUNCNAME}($args) */\n";
}

sub generate_ipc_server {
    my ($fout, $module, $functions, $class_handles, $structs, $destructors, $classes) = @_;
    print $fout $autogenerated;
    print $fout "
#define NEXUS_IPC_MODULE_NAME \"$module\"
#include \"nexus_${module}_module.h\"
BDBG_MODULE(nexus_${module}_ipc);
BDBG_FILE_MODULE(nexus_trace_${module});
#if NEXUS_P_TRACE
#define NEXUS_P_TRACE_MSG(x) BDBG_MODULE_MSG(nexus_trace_${module}, x)
#else
#define NEXUS_P_TRACE_MSG(x)
#endif
#include \"../common/ipc/nexus_ipc_server_api.h\"
#include \"../common/ipc/nexus_ipc_server_units.h\"
#include \"nexus_${module}_api.h\"
#define NEXUS_DRIVER_MODULE_CLASS_TABLE    nexus_server_${module}_class_table
";
    bapi_classes::generate_class_table($fout, $classes);

    generate_meta($fout, $module, $structs, $classes, $functions, $class_handles);
    print $fout "\n";

    print $fout "B_IPC_SERVER_PROLOGUE($module)\n";
    print $fout "B_IPC_SERVER_DISPATCH($module)\n";
    for my $func (sort {$a->{FUNCNAME} cmp $b->{FUNCNAME}}  @$functions) {
        next if(exists $func->{ATTR}{'local'});
        my @postprocess;
        my $funcname = $func->{FUNCNAME};
        my $api = "_${funcname}";
        my $bypass_call = 0;
        print_function_prototype($fout, $func);
        print $fout "B_IPC_SERVER_BEGIN($module, _${funcname})\n";
        for my $param (@{$func->{PARAMS}} ) {
            print $fout "    B_IPC_SERVER_PARAM($param->{TYPE}, $param->{NAME})\n";
        }
        my $code = process_arguments($functions, $func, $class_handles, $structs);
        bapi_util::print_code($fout, $code->{SERVER}{DECLARE}, $tab);
        if($func->{RETTYPE} ne 'void') {
            print $fout "    B_IPC_SERVER_RESULT($func->{RETTYPE}, __result)\n";
        }

        print $fout "    B_IPC_SERVER_VERIFY($module, _${funcname})\n";
        bapi_util::print_code($fout, $code->{SERVER}{RECV}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{RECV_VARARG}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{PREPARE}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{CALLBACK_PRE}, $tab);

        print $fout "    B_IPC_SERVER_TRACE((\">%s: $code->{TRACE}{FORMAT}\", \"$funcname\"  $code->{TRACE}{ARG}))\n";

        if(exists $func->{PARAMS}[0]) {
            my $handle = $func->{PARAMS}[0]{NAME};
            for my $c (@$classes) {
                if($c->{DESTRUCTOR}{FUNCNAME} eq $funcname) {

                    if(bapi_util::is_special_handle($func->{PARAMS}[0]{TYPE})) {
                        print $fout "    B_IPC_SERVER_DESTROY_ENUM($api, $c->{CLASS_NAME}, $handle)\n";
                    } else {
                        print $fout "    B_IPC_SERVER_DESTROY($api, $c->{CLASS_NAME}, $handle)\n";
                    }
                    if(exists $c->{SHUTDOWN}) {
                        push @postprocess, "B_IPC_SERVER_SHUTDOWN($api, $c->{CLASS_TYPE}, $handle)";
                        $bypass_call = 1;
                    }
                    last;
                }
                if(exists $c->{RELEASE}{FUNCNAME} and $c->{RELEASE}{FUNCNAME} eq $funcname) {
                    print $fout "    B_IPC_SERVER_RELEASE($api, $c->{CLASS_NAME}, $handle)\n";
                }
            }
        }


        if(!$bypass_call) {
            my $args = join(',', map  { $_->{NAME} } @{$func->{PARAMS}});
            if($func->{RETTYPE} eq 'void') {
                print $fout "    B_IPC_SERVER_CALL_VOID($api, $funcname, ($args))\n";
            } elsif(bapi_util::is_handle($func->{RETTYPE}, $class_handles)) {
                my $class_constructor;
                my $class_acquire;
                print $fout "    B_IPC_SERVER_CALL_HANDLE($api, $funcname, ($args))\n";
                for my $c (@$classes) {
                    if($c->{DESTRUCTOR_TYPE} eq 'destructor') {
                        if( scalar( grep {$funcname eq $_->{FUNCNAME}} @{$c->{CONSTRUCTORS}} )) {
                            if(bapi_util::is_special_handle($func->{RETTYPE})) {
                                print $fout "    B_IPC_SERVER_CONSTRUCTOR_ENUM($api, $c->{CLASS_NAME}, $funcname)\n";
                            } else {
                                print $fout "    B_IPC_SERVER_CONSTRUCTOR($api, $c->{CLASS_NAME}, $funcname)\n";
                            }
                            last;
                        }
                    }
                    if(exists $c->{ACQUIRE} and $c->{ACQUIRE}{FUNCNAME} eq $funcname) {
                        print $fout "    B_IPC_SERVER_ACQUIRE($api, $c->{CLASS_NAME}, $funcname)\n";
                        last;
                    }
                }
            } else {
                print $fout "    B_IPC_SERVER_CALL($api, $funcname, ($args))\n";
            }
        }

        print $fout "    B_IPC_SERVER_TRACE((\"<%s(%d):$code->{TRACE}{RESULT}{FORMAT} $code->{TRACE}{FORMAT}\", \"$funcname\", __rc $code->{TRACE}{RESULT}{ARG} $code->{TRACE}{ARG}))\n";
        bapi_util::print_code($fout, $code->{SERVER}{CALLBACK_POST}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{PROCESS}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{SEND}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{SEND_VARARG}, $tab);
        print $fout "    B_IPC_SERVER_COMPLETED($module, _${funcname})\n";
        bapi_util::print_code($fout, \@postprocess, $tab);
        print $fout "B_IPC_SERVER_END($module, _${funcname})\n";
        print $fout "\n";
    }
    print $fout "B_IPC_SERVER_EPILOGUE($module)\n";
}

sub generate_server {
    my ($fout, $module, $functions, $class_handles, $structs, $destructors, $classes) = @_;
    print $fout $autogenerated;
    print $fout "
#define NEXUS_SERVER_MODULE_NAME \"$module\"
#include \"nexus_${module}_module.h\"
BDBG_MODULE(nexus_${module}_ipc_server);
BDBG_FILE_MODULE(nexus_trace_${module});
#define NEXUS_P_ABIVERIFY_SERVER  1
#if NEXUS_P_TRACE
#define NEXUS_P_TRACE_MSG(x) BDBG_MODULE_MSG(nexus_trace_${module}, x)
#else
#define NEXUS_P_TRACE_MSG(x)
#endif
#include \"nexus_core_utils.h\"
/* defines to make all module symbols uniques */
#define nexus_server_module_open nexus_server_${module}_open
#define nexus_server_module_close nexus_server_${module}_close
#define nexus_server_process nexus_server_${module}_process
#define nexus_server_args B_IPC_\U${module}\E_data
#define nexus_server_data_size nexus_server_${module}_data_size

#include \"server/nexus_server_prologue.h\"
#include \"../common/ipc/nexus_ipc_server_api.h\"
#include \"server/nexus_server_units.h\"
#include \"nexus_${module}_abiverify_api.h\"

#define NEXUS_DRIVER_MODULE_CLASS_TABLE    nexus_server_${module}_class_table
#define NEXUS_IPC_HEADER_VERSION   NEXUS_${module}_MODULE_VERSION
extern  const struct b_objdb_class  NEXUS_DRIVER_MODULE_CLASS_TABLE[];
";

    print $fout "B_IPC_SERVER_DECLARE($module)\n";
    print $fout "#include \"server/nexus_server_body.h\"\n";
    print $fout "B_IPC_SERVER_BODY($module)\n";
    print $fout "#include \"server/nexus_server_epilogue.h\"\n";
};

sub generate {
    my ($destdir, $module, $functions, $class_handles, $structs, $version, $module_number, $ioctls) = @_;
    my $fout;
    my $file;
    my $destructors = bapi_classes::get_destructors $functions;
    my $classes = bapi_classes::get_classes $functions, $destructors;

    $file = "$destdir/nexus_${module}_abiverify_api.h",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_ipc_api($fout, $module, $functions, $class_handles, $structs, $version);
    close $fout;

    $file = "$destdir/nexus_${module}_abiverify_client.h",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_client($fout, $module, $functions, $class_handles, $structs, $destructors);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_client.c",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_ipc_client($fout, $module, $functions, $class_handles, $structs);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_ioctl.h",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_ioctl($fout, $module, $functions, $class_handles, $structs, $module_number, $ioctls);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_proxy.c",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_proxy_client($fout, $module, $functions, $class_handles, $structs);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_server.c",

    $file = "$destdir/nexus_${module}_abiverify_ipc.c",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_ipc_server($fout, $module, $functions, $class_handles, $structs, $destructors, $classes);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_server.c",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_server($fout, $module, $functions, $class_handles, $structs, $destructors, $classes);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_driver.c",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_driver($fout, $module, $functions, $class_handles, $structs, $destructors);
    close ($fout);
}

1;
