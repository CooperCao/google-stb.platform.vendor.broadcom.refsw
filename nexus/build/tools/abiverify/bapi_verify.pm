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

my %plain_types = map {$_ => 1} qw (
bool
char
uint8_t
uint16_t
int16_t
uint32_t
uint64_t
int
unsigned
NEXUS_Addr
);
foreach (
    'unsigned short',
    'unsigned char',
) {
    $plain_types{$_}=1;
}

my %special_types = map {$_ => 1} qw (
NEXUS_CallbackDesc
NEXUS_P_MemoryUserAddr
NEXUS_KeySlotTag
NEXUS_AnyObject
size_t
);
foreach (
    'unsigned long'
) {
    $special_types{$_}=1;
};

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
        } elsif(bapi_util::is_handle($type)) {
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
        if($translate) {
            print $fout "\n\ntypedef struct B_NEXUS_COMPAT_TYPE($struct_name) {\n";
        } else {
            print $fout "\n\ntypedef struct $struct_name {\n";
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
            print $fout "  " x ($#$substruct+1);
            if($translate) {
                if(exists $structs->{$field->{TYPE}}) {
                    print $fout "struct B_NEXUS_COMPAT_TYPE($field->{TYPE}) $name; /* $field->{NAME} */\n";
                } elsif(exists $field->{ATTR}->{memory}) {
                    print $fout "B_NEXUS_COMPAT_TYPE_MEMORY($field->{TYPE}) $name; /* $field->{NAME} */\n";
                } elsif(exists $field->{ATTR}->{kind} && $field->{ATTR}->{kind} eq 'null_ptr') {
                    print $fout "B_NEXUS_COMPAT_TYPE_POINTER($field->{TYPE}) $name; /* $field->{NAME} */\n";
                } elsif(bapi_util::is_handle($field->{TYPE}, $class_handles)) {
                    print $fout "B_NEXUS_COMPAT_TYPE_HANDLE($field->{TYPE}) $name; /* $field->{NAME} */\n";
                } elsif(exists $special_types{$field->{TYPE}}) {
                    my $compat = $field->{TYPE};
                    $compat =~ s/ /_/g;
                    print $fout "B_NEXUS_COMPAT_TYPE_MISC($field->{TYPE},$compat) $name; /* $field->{NAME} */\n";
                } else {
                    print $fout "$field->{TYPE} $name; /* $field->{NAME} */\n";
                }
            } else {
                print $fout "$field->{TYPE} $name; /* $field->{NAME} */\n";
            }
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

sub process_functions {
    my ($fout, $functions, $class_handles) = @_;
    my @fields_api;
    for my $func (@$functions) {
        my $funcname = $func->{FUNCNAME};

        next if(exists $func->{ATTR}->{'local'});
        my $params = $func->{PARAMS};
        my @fields_in;
        my @fields_out;
        my @fields_func;
        my @fields_varargs;
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
            if(exists $param->{ATTR}) {
                $field->{ATTR} = $param->{ATTR};
                if(exists $field->{ATTR}{nelem}) {
                    if($param->{BASETYPE} eq 'void') {
                        $field->{TYPE} = 'uint8_t';
                    } else {
                        $field->{TYPE} = $param->{BASETYPE};
                    }
                    push @fields_varargs, $field;
                    next;
                }
            }
            if($param->{ISREF} && $param->{BASETYPE} ne 'void') {
                $field->{TYPE} = $param->{BASETYPE};
                if ($param->{INPARAM}) {
                    push @fields_in, $field;
                } else {
                    push @fields_out, $field;
                }
            } else {
                $field->{TYPE} = $param->{TYPE};
                push @fields_in, $field;
            }
        }
        add_substruct(\@fields_func, 'in', 'union', \@fields_in);
        add_substruct(\@fields_func, 'out', 'union', \@fields_out);
        add_substruct(\@fields_func, 'varargs', 'union', \@fields_varargs);
        add_substruct(\@fields_api, $funcname, 'union', \@fields_func);
    }
    my @fields_all;
    add_substruct(\@fields_all, 'api', 'struct', \@fields_api);
    my $struct = { 'b_module_ipc' => \@fields_all};
    print Dumper(\@fields_api) if 0;

    return $struct;
}


1;
