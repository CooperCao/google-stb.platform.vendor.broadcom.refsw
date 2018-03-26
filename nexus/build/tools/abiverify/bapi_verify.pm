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
    'unsigned char',
    'unsigned int',
    'unsigned short',
) {
    $plain_types{$_}=1;
}

my %special_types = (
'NEXUS_CallbackDesc'=> { 'FORMAT' => '%p' , 'FORMAT_CONVERT' => ''},
'NEXUS_P_MemoryUserAddr' => { 'FORMAT' => '%p', 'FORMAT_CONVERT' => ''},
'NEXUS_KeySlotTag' => {'FORMAT' => '%p', 'FORMAT_CONVERT' => ''},
'size_t'    => {'FORMAT' => '%u', 'FORMAT_CONVERT' => '(unsigned)'},
'unsigned long' => {'FORMAT' => '%lu', 'FORMAT_CONVERT' => ''}
);

my $tab = '    ';


sub filter_array {
    my $field = shift;
    $field =~ s/\[[^\]]+\]/[0]/g;
    return $field;
}

sub get_compat_type
{
    my ($type,$kind) = @_;
    $type =~ s/ /_/g if $kind eq 'SPECIAL';
    $type;
}

sub get_compat_type_decl
{
    my ($type, $kind) = @_;
    my $compat_type = get_compat_type($type, $kind);
    if($kind eq 'STRUCT') {
        $compat_type = "struct B_NEXUS_COMPAT_TYPE($type)";
    } elsif($kind eq 'SPECIAL') {
        $type = "B_NEXUS_COMPAT_TYPE_MISC($type,$compat_type)";
    } elsif($kind ne 'PLAIN' and $kind ne 'GENERIC') {
        $compat_type = "B_NEXUS_COMPAT_TYPE_$kind($type)";
    }
    $compat_type;
}

my %enum_handles = map {$_ => 1} qw ( NEXUS_ParserBand NEXUS_Timebase NEXUS_InputBand );

sub is_class_handle {
    my $type = shift;
    my $class_handles = shift;
    foreach (@$class_handles) {
        return 1 if $type eq $_;
    }
    return 0;
}

sub is_handle {
    my $type = shift;
    my $class_handles = shift;
    if(exists $enum_handles{$type}) {
        return 1;
    } elsif(is_class_handle($type, $class_handles)) {
        return 1;
    } elsif( $type =~ /Handle$/) {
        return 1;
    }
    return 0;
}

sub is_special_handle {
    my $type = shift;
    exists $enum_handles{$type};
}

sub get_type_kind
{
    my ($type, $attr, $structs, $class_handles) = @_;
    my $kind;
    if((exists $plain_types{$type})) {
        $kind = 'PLAIN';
    } elsif(exists $special_types{$type}) {
        $kind = 'SPECIAL';
    } elsif(exists $structs->{$type}) {
        $kind = 'STRUCT';
    } elsif(exists $structs->{$type}) {
        $kind = 'STRUCT';
    } elsif(exists $attr->{memory}) {
        $kind = 'MEMORY';
    } elsif(exists $attr->{kind} && $attr->{kind} eq 'null_ptr') {
        $kind = 'NULL_POINTER';
    } elsif(is_special_handle($type)) {
        $kind = 'ENUM_HANDLE';
    } elsif( is_class_handle($type, $class_handles) or ($type eq 'NEXUS_AnyObject')) {
        $kind = 'HANDLE';
    } elsif( $type =~ /Handle$/) {
        $kind = 'FAKE_HANDLE';
    } else {
        $kind = 'GENERIC';
    }
    $kind;
}

sub get_field_kind
{
    my ($field, $structs, $class_handles) = @_;
    my $kind;
    if(exists $field->{COMPAT}{POINTER}) {
        $kind = 'POINTER';
    } else {
        $kind = get_type_kind($field->{TYPE}, $field->{ATTR}, $structs, $class_handles);
    }
    $kind;
}

sub verify_one_struct {
    my ($fout, $class_handles, $structs, $name, $path, $fields) = @_;
    my $field;
    for $field (@$fields) {
        my $type = $field->{TYPE};
        my $field_name = filter_array($field->{NAME});
        if(exists $structs->{$type}) {
            print $fout  "    /* $field->{NAME} $type */\n";
            verify_one_struct ($class_handles, $structs, $name, $path . " $field->{NAME} .", $structs->{$type});
        }
        print $fout "    VERIFY_FIELD($name,  $path $field_name, $type )\n";
        next if(exists $plain_types{$type});
        next if(exists $special_types{$type});
        next if(is_special_handle($type) || is_class_handle($type, $class_handles));
        if( (exists $field->{ATTR}{memory} && $field->{ATTR}{memory} eq 'cached') ||
             (exists $field->{ATTR}->{kind} && $field->{ATTR}->{kind} eq 'null_ptr')) {
            print $fout "    VERIFY_POINTER($name, $path $field_name, $type)\n";
        } else {
            my $kind = get_field_kind($field, $structs, $class_handles);
            $kind = 'FIELD_STRUCT' if($kind eq 'STRUCT');
            print $fout "    VERIFY_${kind}($name, $path $field_name, $type)\n";
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
    print $fout "\nVERIFY_BEGIN($_name)\n\n";
    for my $name (sort keys %$struct) {
        my $fields = $struct->{$name};
        verify_one_struct ($fout, $class_handles, $struct, $name, '', $fields);
        print $fout "    VERIFY_STRUCT($name)\n";
        print $fout "\n\n";
    }
    print $fout "VERIFY_END($_name)\n";
}

sub visit_struct {
    my ($structs,$visitor,@context) = @_;
    my %dependency;
    my %visited;
    my $round=0;

    # Structures have to be visited in the order of their dependencies
    while ( my ($struct_name, $fields) = each %$structs) {
        for my $field (@$fields) {
            if(exists $structs->{$field->{TYPE}}) {
                $dependency{$struct_name}{$field->{TYPE}} = 1;
            }
        }
    }

    while(1) {
        my $continue=0;
        for (sort keys %$structs) {
            my $struct_name = $_;
            my $fields = $structs->{$_};
            next if exists $visited{$struct_name};
            if(exists $dependency{$struct_name}) {
                if (grep  {not exists $visited{$_}} (keys %{$dependency{$struct_name}})) {
                    #print "[$round] postpone $struct_name\n";
                    $continue = 1;
                    next;
                }
            }
            $visited{$struct_name}=1;
            $visitor->($struct_name,$fields, @context);
        }
        last unless $continue;
        $round++;
    }
}

sub print_one_struct
{
    my ($struct_name, $fields, $fout,$structs,$class_handles,$translate) = @_;
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
            my $kind = get_field_kind($field, $structs, $class_handles);
            if($kind eq 'STRUCT') {
                $type = "struct B_NEXUS_COMPAT_TYPE($type)";
            } elsif($kind eq 'SPECIAL') {
                my $compat = get_compat_type($type, $kind);
                $type = "B_NEXUS_COMPAT_TYPE_MISC($type,$compat)";
            } elsif($kind ne 'PLAIN' and $kind ne 'GENERIC') {
                $type = "B_NEXUS_COMPAT_TYPE_$kind($type)";
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


sub print_struct
{
    my ($fout,$structs,$class_handles,$translate) = @_;
    visit_struct($structs, \&print_one_struct, $fout, $structs, $class_handles, $translate);
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

# set referenced found in a type
sub get_referenced_structs
{
    my ($struct, $structs, $opts) = @_;
    my %references;

    for my $field (@$struct) {
        if(exists $structs->{$field->{TYPE}}) {
            if( exists $opts->{WITH_UNION} or not grep {$_ eq 'union'} @{$field->{KIND}}) {
                $references{$field->{TYPE}}=$structs->{$field->{TYPE}};
            }
        }
    }
    # Do it recursively
    while(1) {
        my %new_references;
        while (my ($name, $fields) = each %references) {
            for my $field (@$fields) {
                if(exists $structs->{$field->{TYPE}}) {
                    if( exists $opts->{WITH_UNION} or not grep {$_ eq 'union'} @{$field->{KIND}}) {
                        if(not exists $references{$field->{TYPE}}) {
                            $new_references{$field->{TYPE}}=$structs->{$field->{TYPE}};
                        }
                    }
                }
            }
        }
        my $done=1;
        while (my ($name, $fields) = each %new_references) {
            $references{$name} = $fields;
            $done=0;
        }
        last if $done;
    }
    [keys %references];
}



sub process_functions {
    my ($fout, $module, $functions, $class_handles, $structs, $original_structs) = @_;
    my @fields_func_in;
    my @fields_func_out;
    my @ioctls;
    my %api_structs;
    my @api_ids;
    my %compat_info;
    my %compat_types;
    my @varargs_data_in;
    my @varargs_data_out;
    for my $func (@$functions) {
        next if(exists $func->{ATTR}{'local'});

        my $funcname = $func->{FUNCNAME};
        my $params = $func->{PARAMS};
        my @fields_in;
        my @fields_out;
        my @fields_varargs_in;
        my @fields_varargs_out;
        my @fields_varargs_data_in;
        my @fields_varargs_data_out;
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
                    my $reserved = 1;
                    if(exists $param->{ATTR}{reserved}) {
                        $reserved = $param->{ATTR}{reserved};
                    }
                    my $type = $param->{BASETYPE};
                    $type = 'uint8_t' if $type eq 'void';
                    my $data = {KIND => ['struct'], NAME => $param->{NAME} . "[$reserved]" , TYPE => $type};
                    $field->{TYPE} = 'int';
                    $ioctl_field->{COMPAT}{POINTER}=1;
                    push @ioctl_pointers, $ioctl_field;
                    if ($param->{INPARAM}) {
                        push @fields_pointer_in_null, $null; # Used in generic pointer validation
                        push @fields_varargs_in, $field;
                        push @fields_varargs_data_in, $data;
                    } else {
                        push @fields_pointer_out_null, $null; # Used in generic pointer validation
                        push @fields_varargs_out, $field;
                        push @fields_varargs_data_out, $data;
                    }
                    if(exists $structs->{$param->{BASETYPE}}) {
                        foreach (@{$structs->{$param->{BASETYPE}}}) {
                            if(exists $_->{ATTR}{memory} && $_->{ATTR}{memory} eq 'cached') {
                                my $memory = {KIND => ['struct'], NAME => (flat_name ($param->{NAME} , $_->{NAME})), TYPE => 'int', COMMENT => 'array of NEXUS_Addr'};
                                my $ioctl_memory = {KIND => ['struct'], NAME => (flat_name ($param->{NAME} , $_->{NAME})), TYPE => 'NEXUS_Addr *', COMMENT => 'array of NEXUS_Addr', COMPAT => {POINTER => 1}};
                                my $data_memory = {KIND => ['struct'], NAME => (flat_name ($param->{NAME} , $_->{NAME})) . "[$reserved]" , TYPE => 'NEXUS_Addr'};
                                my $reserved;
                                push @ioctl_pointers, $ioctl_memory;
                                if ($param->{INPARAM}) {
                                    push @fields_varargs_in, $memory;
                                    push @fields_varargs_data_in, $data_memory;
                                } else {
                                    push @fields_varargs_out, $memory;
                                    push @fields_varargs_data_out, $data_memory;
                                }
                            }
                        }
                    }
                    next;
                }
                if(exists $param->{ATTR}{memory} && $param->{ATTR}{memory} eq 'cached') {
                    my $null = {KIND => ['struct'], NAME => $param->{NAME}, TYPE => 'bool'};
                    $field->{TYPE} = 'NEXUS_Addr';
                    $ioctl_field->{COMPAT}{POINTER}=1;
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
                $ioctl_field->{COMPAT}{POINTER}=1;
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
            $api_structs{$name} =  \@fields_in;
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
        add_substruct(\@varargs_data_in, "_${funcname}", 'union', \@fields_varargs_data_in);
        add_substruct(\@ioctl_in, 'pointer', 'struct', \@ioctl_pointers);
        add_substruct(\@ioctl_in, 'memory', 'struct', \@fields_memory_in);

        my @out;
        if(scalar @fields_out) {
            my $name = "\U$module\E_${funcname}_out_args";
            $api_structs{$name} =  \@fields_out;
        }
        add_substruct(\@out, 'ret', 'struct', \@fields_out);
        add_substruct(\@out, 'pointer', 'struct', \@fields_pointer_out);
        add_substruct(\@out, 'vararg', 'struct', \@fields_varargs_out);
        add_substruct(\@out, 'memory', 'struct', \@fields_memory_out);
        if(scalar @out == 0) {
            my $unused = {KIND => ['struct'], NAME => '__unused' , TYPE => 'unsigned'};
            push @out, $unused;
        }
        add_substruct(\@varargs_data_out, "_${funcname}", 'union', \@fields_varargs_data_out);

        my @ioctl_out;
        add_substruct(\@ioctl_out, 'ret', 'struct', \@fields_out);
        add_substruct(\@ioctl_out, 'memory', 'struct', \@fields_memory_out);

        $api_structs{"B_${funcname}_ipc_in"} = \@in;
        $api_structs{"B_${funcname}_ipc_out"} = \@out;

        push @fields_func_in, {KIND => ['union'], NAME => "_${funcname}", TYPE => "B_${funcname}_ipc_in"};
        push @fields_func_out, {KIND => ['union'], NAME => "_${funcname}", TYPE => "B_${funcname}_ipc_out"};

        add_substruct(\@ioctl_func, 'in', 'union', \@ioctl_in);
        add_substruct(\@ioctl_func, 'out', 'union', \@ioctl_out);
        if(scalar @ioctl_func == 0) {
            my $unused = {KIND => ['struct'], NAME => '__unused' , TYPE => 'unsigned'};
            push @ioctl_func, $unused;
        }
        my $ioctl_name = "\U$module\E_${funcname}_ioctl_data";
        push @ioctls, { $ioctl_name => \@ioctl_func };
        $api_structs{$ioctl_name} = \@ioctl_func;

        push @api_ids, [$funcname,"NEXUS_P_API_${module}_${funcname}_id"];
    }
    my $api_in = "b_${module}_module_ipc_in";
    my $api_out = "b_${module}_module_ipc_out";
    $api_structs{$api_in} = \@fields_func_in;
    $api_structs{$api_out} = \@fields_func_out;
    if(scalar @varargs_data_in == 0) {
        my $unused = {KIND => ['struct'], NAME => '__unused' , TYPE => 'unsigned'};
        push @varargs_data_in , $unused;
    }
    if(scalar @varargs_data_out == 0) {
        my $unused = {KIND => ['struct'], NAME => '__unused' , TYPE => 'unsigned'};
        push @varargs_data_out , $unused;
    }

    $api_structs{"b_${module}_vararg_data_in"} = \@varargs_data_in;
    $api_structs{"b_${module}_vararg_data_out"} = \@varargs_data_out;


    $compat_info{API}{IN} = $api_in;
    $compat_info{API}{OUT} = $api_in;

    my $struct = [
        {KIND => ['union'], NAME => 'in' , TYPE => $api_in},
        {KIND => ['union'], NAME => 'out' , TYPE => $api_out}
    ];
    # find all types that are referenced in the API
    my %references;
    while ( my ($name, $fields) = each %api_structs) {
        my $struct_reference = get_referenced_structs($fields, $original_structs, {'WITH_UNION'=>1});
        $references{$name} = $fields;
        for (@$struct_reference) {
            $references{$_}=$original_structs->{$_};
        }
    }
    my $varargs_info = {'IN' => \@varargs_data_in, 'OUT' => \@varargs_data_out};

    return [\@ioctls, \%api_structs, \%references, \@api_ids, \%compat_info, $varargs_info];
}

sub get_arg_callback_id {
    my ($functions, $func, $arg_no) = @_;
    my $id = sprintf("0x%05x", ((bapi_util::func_id $functions, $func)*256 + $arg_no + 0x10000));
    $id;
}

sub get_field_callback_id {
    my ($structs, $param, $field_no) = @_;
    my $id = sprintf("0x%04x", ((bapi_util::struct_id $structs, $param->{BASETYPE})*256 + $field_no));
    $id;
}

sub get_callback_kind {
    my ($func,$class_handles,$param) = @_;

    my $func_callback_kind='_UNKNOWN';
    if(scalar @{$func->{PARAMS}} >= 1) {
        if (is_handle($func->{RETTYPE}, $class_handles)) {
            $func_callback_kind='_CONSTRUCTOR';
        } elsif (is_handle($func->{PARAMS}[0]{TYPE}, $class_handles)) {
            $func_callback_kind= '';
        } elsif(is_special_handle($func->{PARAMS}[0]{TYPE})) {
            $func_callback_kind = '_ENUM';
        } elsif ($func->{PARAMS}[0]{ISREF} && (scalar @{$func->{PARAMS}} == 1)) {
            $func_callback_kind = '_INIT';
        }
    }
    my $callback_kind = $func_callback_kind;
    if(exists $param->{ATTR}{pragma} && $param->{ATTR}{pragma} eq 'ClearCallbacks') {
        $callback_kind='_CLEAR';
    }
    $callback_kind;
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
        my $callback_kind=get_callback_kind($func, $class_handles, $param);
        $arg_no++;
        push @trace_names,$param->{NAME};
        if($param->{ISREF} || is_handle($param->{TYPE}, $class_handles) || $param->{TYPE} eq 'NEXUS_AnyObject' ) {
            push @trace_format, "%p";
            push @trace_args, "(void *)$param->{NAME}";
        } elsif (exists $special_types{$param->{TYPE}}) {
            push @trace_format, $special_types{$param->{TYPE}}->{FORMAT};
            push @trace_args,$special_types{$param->{TYPE}}->{FORMAT_CONVERT} . $param->{NAME};
        } else {
            push @trace_format, "%u";
            push @trace_args, "(unsigned)$param->{NAME}";
        }
        if ($param->{INPARAM}) {
            if(is_handle($param->{TYPE}, $class_handles)) {
                if( not (exists $enum_handles{$param->{TYPE}} or is_class_handle($param->{TYPE}, $class_handles))) {
                    if(exists $param->{attr}->{'null_allowed'} && $param->{attr}->{'null_allowed'} eq "true") {
                        push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_FAKE_HANDLE($api, $param->{NAME}, $param->{TYPE})";
                    } else {
                        push @{$code->{SERVER}{RECV}}, "B_IPC_SERVER_FAKE_HANDLE_NULL($api, $param->{NAME}, $param->{TYPE})";
                    }
                }
            }
        }
        if(exists $param->{BASETYPE} && $param->{BASETYPE} eq 'NEXUS_CallbackDesc') {
            my $id = get_arg_callback_id($functions, $func, $arg_no);
            if ($param->{INPARAM}) {
                push @{$code->{SERVER}{CALLBACK_PRE}}, "B_IPC_CALLBACK${callback_kind}_IN_PREPARE($api, $param->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
                push @{$code->{SERVER}{CALLBACK_POST}}, "B_IPC_CALLBACK${callback_kind}_IN_FINALIZE($api, $param->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
            } else {
                push @{$code->{SERVER}{CALLBACK_POST}}, "B_IPC_CALLBACK${callback_kind}_OUT($api, $param->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
            }
        }
        if(exists $param->{ATTR}{nelem}) {
            my $type = $param->{BASETYPE};
            $type = 'uint8_t' if $type eq 'void';
            my $kind = get_type_kind($type, $param->{ATTR}, $structs, $class_handles);
            my $compat_type = get_compat_type_decl($type, $kind);
            if ($param->{INPARAM}) {

                push @{$code->{COMPAT}{VARARG_IN_DECLARE}}, "NEXUS_P_COMPAT_VARARG_IN_DECLARE($api, $param->{NAME}, $type,$compat_type)";
                if(exists $param->{ATTR}{nelem_convert}) {
                    my $convert = $param->{ATTR}{nelem_convert};
                    push @{$code->{CLIENT}{SEND_VARARG}}, "B_IPC_CLIENT_SEND_VARARG_NELEM_CONVERT($api, $param->{NAME}, $type, $convert, $param->{ATTR}{nelem})";
                    push @{$code->{SERVER}{RECV_VARARG}}, "B_IPC_SERVER_RECV_VARARG_NELEM_CONVERT($api, $param->{NAME}, $type, $convert, $param->{ATTR}{nelem})";
                    push @{$code->{DRIVER}{RECV_VARARG}}, "B_IPC_DRIVER_RECV_VARARG_NELEM_CONVERT($api, $param->{NAME}, $type, $convert, $param->{ATTR}{nelem})";
                    push @{$code->{COMPAT}{VARARG_IN_PLACE}}, "NEXUS_P_COMPAT_VARARG_IN_PLACE_WITH_CONVERT($api, $param->{NAME}, $type, $compat_type, $convert, $param->{ATTR}{nelem})";
                    push @{$code->{COMPAT}{VARARG_IN_CONVERT}}, "NEXUS_P_COMPAT_VARARG_IN_CONVERT_WITH_CONVERT($api, $param->{NAME}, $type, $convert, $param->{ATTR}{nelem})";
                } else {
                    push @{$code->{CLIENT}{SEND_VARARG}}, "B_IPC_CLIENT_SEND_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                    push @{$code->{SERVER}{RECV_VARARG}}, "B_IPC_SERVER_RECV_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                    push @{$code->{DRIVER}{RECV_VARARG}}, "B_IPC_DRIVER_RECV_VARARG($api, $param->{NAME}, B_IPC_COMPAT_SELECT($type, $compat_type), $param->{ATTR}{nelem})";
                    push @{$code->{COMPAT}{VARARG_IN_PLACE}}, "NEXUS_P_COMPAT_VARARG_IN_PLACE($api, $param->{NAME}, $type, $compat_type, $param->{ATTR}{nelem})";
                    push @{$code->{COMPAT}{VARARG_IN_CONVERT}}, "NEXUS_P_COMPAT_VARARG_IN_CONVERT($api, $param->{NAME}, $kind, $type, $param->{ATTR}{nelem})";
                }
            }
            if($param->{BASETYPE} eq 'NEXUS_CallbackDesc') {
                push @{$code->{SERVER}{CALLBACK_PRE}}, 'B_IPC_NOT_SUPPORTED("NEXUS_CallbackDesc as varags")';
            }
            if(exists $structs->{$param->{BASETYPE}}) {
                if(exists $param->{ATTR}{nelem_convert} || exists $special_types{$param->{BASETYPE}}) {
                    push @{$code->{SERVER}{RECV_VARARG}}, 'B_IPC_NOT_SUPPORTED("nelem_convert in complex varags")';
                }
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
                            push @{$code->{COMPAT}{VARARG_IN_CONVERT}}, "NEXUS_P_COMPAT_VARARG_IN_ADDR_CONVERT($api, $param->{NAME}, $param->{TYPE}, $name, $param->{ATTR}{nelem})";
                        } else {
                            push @{$code->{SERVER}{PROCESS}}, "BDBG_CASSERT(0); /* memory pointers in out varargs are not supported */";
                        }
                    }
                }
            }
            if (not $param->{INPARAM}) {
                if(exists $param->{ATTR}{nelem_convert}) {
                    push @{$code->{SERVER}{OUT_VARARGS}{SIZE}}, 'B_IPC_NOT_SUPPORTED("nelem_convert in out varags")';
                }
                push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_OUT_PTR($api, $param->{NAME})";
                if(exists $param->{ATTR}{nelem_out}) {
                    push @{$code->{CLIENT}{RECV_VARARG}}, "B_IPC_CLIENT_RECV_VARARG($api, $param->{NAME}, $type, * $param->{ATTR}{nelem_out})";
                } else {
                    push @{$code->{CLIENT}{RECV_VARARG}}, "B_IPC_CLIENT_RECV_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                }
                push @{$code->{SERVER}{OUT_VARARGS}{PLACE}}, "B_IPC_SERVER_PLACE_OUT_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                push @{$code->{SERVER}{OUT_VARARGS}{SIZE}}, "B_IPC_SERVER_SIZE_OUT_VARARG($api, $param->{NAME}, $type, $param->{ATTR}{nelem})";
                push @{$code->{DRIVER}{DECLARE}}, "B_IPC_DRIVER_DATA( B_IPC_COMPAT_SELECT($param->{TYPE}, B_NEXUS_COMPAT_TYPE_POINTER($param->{BASETYPE})), $param->{NAME})";
                push @{$code->{DRIVER}{ASSIGN}}, "B_IPC_DRIVER_ASSIGN_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{RECV}}, "B_IPC_DRIVER_SET_OUT_PTR($api, $param->{NAME})";
                if(exists $param->{ATTR}{nelem_out}) {
                    push @{$code->{DRIVER}{SEND_VARARG}}, "B_IPC_DRIVER_SEND_VARARG_NELEM_OUT($api, $param->{NAME}, B_IPC_COMPAT_SELECT($type, $compat_type), $param->{ATTR}{nelem_out})";
                } else {
                    push @{$code->{DRIVER}{SEND_VARARG}}, "B_IPC_DRIVER_SEND_VARARG_NELEM($api, $param->{NAME}, B_IPC_COMPAT_SELECT($type, $compat_type), $param->{ATTR}{nelem})";
                }
                push @{$code->{COMPAT}{VARARG_OUT_DECLARE}}, "NEXUS_P_COMPAT_VARARG_OUT_DECLARE($api, $param->{NAME}, $type,$compat_type)";
                push @{$code->{COMPAT}{VARARG_OUT_DECLARE}}, "NEXUS_P_COMPAT_VARARG_OUT_DECLARE_NELEM($api, $param->{NAME}, $param->{ATTR}{nelem})";
                push @{$code->{COMPAT}{VARARG_OUT_SIZE}}, "NEXUS_P_COMPAT_VARARG_OUT_SIZE($api, $param->{NAME}, $type, $compat_type, $param->{ATTR}{nelem})";
                push @{$code->{COMPAT}{VARARG_OUT_PLACE}}, "NEXUS_P_COMPAT_VARARG_OUT_PLACE($api, $param->{NAME}, $type, $compat_type, $param->{ATTR}{nelem})";
                if(exists $param->{ATTR}{nelem_out}) {
                    push @{$code->{COMPAT}{VARARG_OUT_PROCESS}}, "NEXUS_P_COMPAT_VARARG_OUT_UPDATE_NELEM($api, $param->{NAME}, $param->{ATTR}{nelem}, $param->{ATTR}{nelem_out})";
                }
                push @{$code->{COMPAT}{VARARG_OUT_CONVERT}}, "NEXUS_P_COMPAT_VARARG_OUT_CONVERT($api, $param->{NAME}, $kind, $type, $param->{ATTR}{nelem})";

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
                my $type = $param->{BASETYPE};
                my $kind = get_type_kind($type, $param->{ATTR}, $structs, $class_handles);
                my $compat_type = get_compat_type_decl($type, $kind);
                push @{$code->{CLIENT}{SEND}}, "B_IPC_CLIENT_SEND_OUT_PTR($api, $param->{NAME})";
                push @{$code->{CLIENT}{RECV}}, "B_IPC_CLIENT_RECV_OUT_PTR($api, $param->{NAME})";
                push @{$code->{SERVER}{SET_OUT_PTR}}, "B_IPC_SERVER_SET_OUT_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{DECLARE}}, "B_IPC_DRIVER_DATA( B_IPC_COMPAT_SELECT($param->{TYPE}, B_NEXUS_COMPAT_TYPE_POINTER($param->{BASETYPE})), $param->{NAME})";
                push @{$code->{DRIVER}{ASSIGN}}, "B_IPC_DRIVER_ASSIGN_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{RECV}}, "B_IPC_DRIVER_SET_OUT_PTR($api, $param->{NAME})";
                push @{$code->{DRIVER}{SEND}}, "B_IPC_DRIVER_COPY_OUT_PTR($api, $param->{NAME})";
            }
            if(exists $structs->{$param->{BASETYPE}}) {
                my $field_no = 0;
                foreach (@{$structs->{$param->{BASETYPE}}}) {
                    $field_no++;
                    if(index($_->{NAME},'[') != -1)  {
                        if( $_->{TYPE} eq 'NEXUS_CallbackDesc') {
                            my $cmnt_in='';
                            my $cmnt_out='';
                            if(exists $param->{ATTR}{pragma} && $param->{ATTR}{pragma} eq 'IgnoreArrayCallbacks') {
                                $cmnt_in='/*';
                                $cmnt_out='*/';
                            }
                            push @{$code->{SERVER}{CALLBACK_PRE}}, "$cmnt_in B_IPC_NOT_SUPPORTED(\"NEXUS_CallbackDesc in $funcname $param->{NAME} $_->{NAME} in array is not supported\") $cmnt_out";
                            next;
                        }
                        if(exists $_->{ATTR}{memory} && $_->{ATTR}{memory} eq 'cached') {
                            push @{$code->{SERVER}{CALLBACK_PRE}}, "B_IPC_NOT_SUPPORTED(\"device address in $funcname $param->{NAME} $_->{NAME} in array is not supported\")";
                            next;
                        }
                    }
                    if( $_->{TYPE} eq 'NEXUS_CallbackDesc') {
                        my $id = get_field_callback_id($structs, $param, $field_no);
                        if ($param->{INPARAM}) {
                            push @{$code->{SERVER}{CALLBACK_PRE}}, "B_IPC_CALLBACK${callback_kind}_FIELD_IN_PREPARE($api, $param->{NAME}, $_->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
                            push @{$code->{SERVER}{CALLBACK_POST}}, "B_IPC_CALLBACK${callback_kind}_FIELD_IN_FINALIZE($api, $param->{NAME}, $_->{NAME}, $func->{PARAMS}[0]{NAME}, $id )";
                            $code->{SERVER}{CALLBACK_VERIFY}{$param->{BASETYPE}} = "B_IPC_CALLBACK${callback_kind}_STRUCT_IN_VERIFY($api, $param->{BASETYPE}, $func->{PARAMS}[0]{NAME})";
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
        if(is_handle($func->{RETTYPE}, $class_handles)) {
            $result_format=' result:%p';
            $result_args=',(void *)__result';
        } else {
            $result_format=' result:%u';
            $result_args=',__result';
        }
        push @{$code->{DRIVER}{RESULT}}, "B_IPC_DRIVER_RESULT($api)";
        if($func->{RETTYPE} eq 'NEXUS_Error') {
            push @{$code->{DRIVER}{RESULT}}, "B_IPC_DRIVER_CHECK_RETURN_CODE($api)";
        }
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
#include \"nexus_${module}_module.h\"
#include \"client/nexus_client_prologue.h\"
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
    my ($fout, $fout_compat, $module, $functions, $class_handles, $structs, $ioctls) = @_;
    my $id = 101;
    print $fout $autogenerated;
    print $fout_compat $autogenerated;
    print $fout "#include \"nexus_${module}_api.h\"\n";
    print $fout_compat "#include \"nexus_${module}_api_compat.h\"\n";
    print $fout "#define NEXUS_IOCTL_\U${module}\E_ID  PROXY_NEXUS_ModuleIoctl_e${module}\n";

    print $fout "#define IOCTL_\U${module}\E_NEXUS_INIT NEXUS_IOCTL($id, NEXUS_IOCTL_\U${module}\E_ID*NEXUS_IOCTL_PER_MODULE+0, PROXY_NEXUS_ModuleInit)\n";
    for my $func (@$functions) {
        next if(exists $func->{ATTR}{'local'});
        my $funcname = $func->{FUNCNAME};
        print $fout "#define IOCTL_\U${module}\E_${funcname} NEXUS_IOCTL($id, NEXUS_IOCTL_\U${module}\E_ID*NEXUS_IOCTL_PER_MODULE+NEXUS_P_API_${module}_${funcname}_id+1, \U${module}\E_${funcname}_ioctl_data)\n";
        print $fout_compat "#define IOCTL_\U${module}\E_${funcname}_compat NEXUS_IOCTL($id, NEXUS_IOCTL_\U${module}\E_ID*NEXUS_IOCTL_PER_MODULE+NEXUS_P_API_${module}_${funcname}_id+1, B_NEXUS_COMPAT_TYPE(\U${module}\E_${funcname}_ioctl_data))\n";
    }
}

sub generate_driver_process_ioctls {
    my ($fout, $module, $functions, $class_handles, $structs, $varargs_info) = @_;
    print $fout $autogenerated;
    for my $func (@$functions) {
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
        print $fout "    /* disable verification of arguments, they will get verified by nexus_p_api_call_verify */\n";
        print $fout "    /* coverity[ tainted_data : FALSE ] */\n";
        print $fout "    /* coverity[ tainted_data_transitive : FALSE ] */\n";
        print $fout "    /* coverity[ FORWARD_NULL : FALSE ] */\n";

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
}

sub generate_driver {
    my ($fout, $module, $functions, $class_handles, $structs, $varargs_info) = @_;
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

#define NEXUS_IOCTL_MODULE_INIT      IOCTL_\U${module}\E_NEXUS_INIT
#define NEXUS_IOCTL_MODULE_VERSION   NEXUS_${module}_MODULE_VERSION

#include \"nexus_${module}_abiverify_ioctl.h\"
#if NEXUS_COMPAT_32ABI
#include \"nexus_base_compat.h\"
#include \"nexus_${module}_abiverify_ioctl_compat.h\"
typedef B_NEXUS_COMPAT_TYPE(b_${module}_module_ipc_in) b_${module}_module_ipc_in_compat;
typedef B_NEXUS_COMPAT_TYPE(b_${module}_module_ipc_out) b_${module}_module_ipc_out_compat;
#endif
typedef b_${module}_module_ipc_in b_${module}_module_ipc_in_native;
typedef b_${module}_module_ipc_out b_${module}_module_ipc_out_native;

struct nexus_${module}_driver_module_data {
    union {
        struct {
            b_${module}_module_ipc_in ipc;
            b_${module}_vararg_data_in varargs;
        } native;
#if NEXUS_COMPAT_32ABI
        struct {
            B_NEXUS_COMPAT_TYPE(b_${module}_module_ipc_in) ipc;
            B_NEXUS_COMPAT_TYPE(b_${module}_vararg_data_in) varargs;
        } compat;
#endif
    } in;
    union {
        struct {
            b_${module}_module_ipc_out ipc;
            b_${module}_vararg_data_out varargs;
        } native;
#if NEXUS_COMPAT_32ABI
        struct {
            B_NEXUS_COMPAT_TYPE(b_${module}_module_ipc_out) ipc;
            B_NEXUS_COMPAT_TYPE(b_${module}_vararg_data_out) varargs;
        } compat;
#endif
    } out;
};

#define nexus_driver_module_data struct nexus_${module}_driver_module_data data;


#include \"driver/nexus_driver_prologue.h\"
#include \"driver/nexus_driver_units.h\"

#define NEXUS_DRIVER_MODULE_CLASS_TABLE    nexus_server_${module}_class_table
extern  const struct b_objdb_class  NEXUS_DRIVER_MODULE_CLASS_TABLE[];
B_IPC_DRIVER_CALL_HELPER(, ${module})
#if NEXUS_COMPAT_32ABI
B_IPC_DRIVER_CALL_HELPER(_compat, ${module})
#endif
";


    for my $mode ('native','compat') {
        if($mode eq 'compat') {
            print $fout "#if NEXUS_COMPAT_32ABI\n";
        }
        print $fout "union nexus_driver_${module}_args_${mode} {\n";
        print $fout "${tab}PROXY_NEXUS_ModuleInit init;\n";

        for my $func (@$functions) {
            next if(exists $func->{ATTR}{'local'});
            my $funcname = $func->{FUNCNAME};
            print $fout "${tab}struct {\n";
            if($mode eq 'compat') {
                print $fout "$tab${tab}B_NEXUS_COMPAT_TYPE(\U${module}\E_${funcname}_ioctl_data) ioctl;\n";
            } else {
                print $fout "$tab$tab\U${module}\E_${funcname}_ioctl_data ioctl;\n";
            }
            print $fout "$tab} _${funcname};\n";
        }
        print $fout "};\n";
        if($mode eq 'compat') {
            print $fout "#endif\n";
        }
    }

    print $fout "
union nexus_driver_${module}_args {
    union nexus_driver_${module}_args_native native;
#if NEXUS_COMPAT_32ABI
    union nexus_driver_${module}_args_compat compat;
#endif
};
#include \"driver/nexus_driver_body.h\"
#if NEXUS_COMPAT_32ABI
    if(compat) {
        B_IPC_DRIVER_MODULE_BEGIN($module, \U$module\E, compat)
#define B_IPC_COMPAT_SELECT(native, compat) compat
#define B_IPC_COMPAT_POINTER(arg) (void *)(unsigned long)(arg)
#define B_IPC_COMPAT_NAME(x) x##_compat
#include \"nexus_${module}_abiverify_process_ioctl.h\"
#undef B_IPC_COMPAT_SELECT
#undef B_IPC_COMPAT_POINTER
#undef B_IPC_COMPAT_NAME
        B_IPC_DRIVER_MODULE_END($module, \U$module\E)
    } else
#endif
    {
        B_IPC_DRIVER_MODULE_BEGIN($module, \U$module\E, native)
#define B_IPC_COMPAT_SELECT(native, compat) native
#define B_IPC_COMPAT_POINTER(arg) arg
#define B_IPC_COMPAT_NAME(x) x
#include \"nexus_${module}_abiverify_process_ioctl.h\"
#undef B_IPC_COMPAT_SELECT
#undef B_IPC_COMPAT_POINTER
#undef B_IPC_COMPAT_NAME
        B_IPC_DRIVER_MODULE_END($module, \U$module\E)
    }
#include \"driver/nexus_driver_epilogue.h\"\n
";
}

sub generate_client {
    my ($fout, $module, $functions, $class_handles, $structs, $destructors, $classes) = @_;
    print $fout $autogenerated;
    print $fout "B_IPC_CLIENT_MODULE_BEGIN($module, \U$module\E)\n";
    for my $func (@$functions) {
        next if(exists $func->{ATTR}{'local'});
        my $is_destructor = 0;
        for my $d (values %$destructors) {
            print Dumper($d) if 0;
            if( (exists  $d->{destructor}{FUNCNAME}) and $d->{destructor}{FUNCNAME} eq $func->{FUNCNAME}) {
                $is_destructor = 1; last;
            }
            if( (exists  $d->{release}{FUNCNAME}) and $d->{release}{FUNCNAME} eq $func->{FUNCNAME}) {
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
            my $has_callbacks =  bapi_classes::get_stopcallbacks_handle($func, $classes);
            $has_callbacks = defined $has_callbacks ? 'true' : 'false';
            print $fout "B_IPC_CLIENT_BEGIN_DESTRUCTOR($module, \U$module\E, $api, $funcname, ($args), $func->{PARAMS}[0]{NAME}, $has_callbacks)\n";
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
            if($func->{RETTYPE} eq 'NEXUS_Error') {
                print $fout "    B_IPC_CLIENT_CHECK_RETURN_CODE($api)\n";
            }
        }

        print $fout "    B_IPC_CLIENT_TRACE((\"<%s(%d):$code->{TRACE}{RESULT}{FORMAT} $code->{TRACE}{FORMAT}\", \"$funcname\", __rc $code->{TRACE}{RESULT}{ARG} $code->{TRACE}{ARG}))\n";
        bapi_util::print_code($fout, $code->{CLIENT}{RECV}, $tab);
        bapi_util::print_code($fout, $code->{CLIENT}{RECV_VARARG}, $tab);

        if($is_destructor) {
            print $fout "B_IPC_CLIENT_END_DESTRUCTOR($api, $func->{PARAMS}[0]{NAME})\n";
        } elsif($func->{RETTYPE} eq 'void') {
            print $fout "B_IPC_CLIENT_END_VOID($api)\n";
        } elsif (is_handle($func->{RETTYPE}, $class_handles)) {
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
    my %class_dict = map {$_ => 1} @$class_handles;
    my %all_callback_ids;

    for my $func (@$funcs) {
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
        my @array_objects;
        my @vararg_objects;
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
                my @pointer;
                if (!exists $param->{ATTR}->{'nelem'}) {
                    push @pointer, "BDBG_STRING(\"$param->{NAME}\") ,";
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
                } else {
                    push @pointer, "BDBG_STRING(\"$param->{NAME}\") ,";
                    push @pointer, "NEXUS_OFFSETOF($arg_type, $args . vararg. $param->{NAME}), /* offset */";
                    if($param->{INPARAM}) {
                        push @pointer, "NEXUS_OFFSETOF($arg_type, $args . pointer . is_null . $param->{NAME}), /* null_offset */";
                    } else {
                        push @pointer, "NEXUS_OFFSETOF($in_arg_type, $in_args . pointer . out_is_null . $param->{NAME}), /* null_offset */";
                    }
                    push @pointer, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                    push @pointer, ($param->{ATTR}->{'null_allowed'} ? "true":"false") . '/* null_allowed */';
                }
                push @pointers, \@pointer;
            }

            # find arguments that are pointers to structures and scan them
            if ($param->{ISREF} && exists $structs->{$param->{BASETYPE}}) {
                my $vararg = 0;
                if (exists $param->{ATTR}{'nelem'}) {
                    $vararg = 1;
                }
                # check if param is a struct which has a class handle field
                my $struct_field;
                my $field_no = 0;
                my @callback_ids;
                for $struct_field (@{$structs->{$param->{BASETYPE}}}) {
                    $field_no++;
                    my $field_name = $struct_field->{NAME};
                    my @array_size;
                    my @array_offset;
                    my $current_pos=0;
                    while(1) {
                        my $offset = index($field_name, '[', $current_pos);
                        last if($offset==-1);
                        $offset++;
                        my $array_end = index($field_name, ']', $offset);
                        last if($array_end==-1);
                        push @array_size, substr($field_name, $offset, $array_end-$offset, '');
                        push @array_offset, $offset;
                        $current_pos = $offset;
                    }
                    if (exists $class_dict{$struct_field->{TYPE}}) {
                        my @object;
                        my $handletype = $struct_field->{TYPE};
                        if($vararg) {
                            if($param->{INPARAM}) {
                                my @object;
                                push @object, "'NOT SUPPORTED input parameter $param->{TYPE} $param->{NAME} field $struct_field->{NAME} $struct_field->{TYPE}'";
                            } else {
                                if(is_special_handle($struct_field->{TYPE})) {
                                    push @object, "'NOT SUPPORTED output parameter $param->{TYPE} $param->{NAME} field $struct_field->{NAME} $struct_field->{TYPE}'";
                                } else {
                                    print $file "/* Ignored output parameter $param->{TYPE} $param->{NAME} field $struct_field->{NAME} $struct_field->{TYPE} */\n";
                                }
                            }
                        } else {
                            if ($struct_field->{TYPE} eq $handletype) {
                                my $class_name = bapi_classes::get_class_name($handletype);
                                my $depth;
                                push @object, 'BDBG_STRING("' . $param->{NAME} . '->' . $field_name . '"),';
                                push @object, "NEXUS_OFFSETOF($arg_type, $args . pointer . $param->{NAME}.$field_name), /* offset */";
                                if($param->{INPARAM}) {
                                    push @object, "NEXUS_OFFSETOF($arg_type, $args . pointer . is_null . $param->{NAME}), /* null_offset */";
                                } else {
                                    if(is_special_handle($struct_field->{TYPE})) {
                                        push @object, "NEXUS_OFFSETOF($in_arg_type, $in_args . pointer . out_is_null . $param->{NAME}), /* null_offset */";
                                    } else {
                                        print $file "/* Ignored output parameter $param->{TYPE} $param->{NAME} field $struct_field->{NAME} $struct_field->{TYPE} */\n";
                                        next;
                                    }
                                }
                                push @object, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                                push @object, "true, /* null allowed */";
                                $referenced_objects{$class_name} = 1;
                                push @object, "&NEXUS_OBJECT_DESCRIPTOR($class_name) /* descriptor */";
                                if(scalar @array_size) {
                                    my @field_array_objects;
                                    my $depth = scalar @array_size - 1;
                                    my $max_length = 48;
                                    my @indicies;
                                    for(0..$depth) {
                                        $indicies[$_]=0;
                                    }
                                    while(1) {
                                        my $field_name_array = $field_name;
                                        for(reverse 0..$depth) {
                                            substr($field_name_array, $array_offset[$_], 0, $indicies[$_]);
                                        }
                                        $object[1] = "NEXUS_OFFSETOF($arg_type, $args . pointer . $param->{NAME}.$field_name_array), /* offset */";
                                        my @test;
                                        for(0..$depth) {
                                            push @test, "$array_size[$_] > $indicies[$_]";
                                        }
                                        push @field_array_objects, {DATA=>[@object],TEST=>join(" && ", @test)};
                                        my $last_index=0;
                                        for(reverse 0..$depth) {
                                            $indicies[$_]++;
                                            last if($indicies[$_]<$max_length);
                                            $indicies[$_]=0;
                                            $last_index = $_-1;
                                        }
                                        last if($last_index<0);
                                    }
                                    for(@array_size) {
                                        push @field_array_objects,{DATA=>["'not supported'"],TEST=>"(!defined($_)) || $_==0"};
                                        push @field_array_objects,{DATA=>["'not supported'"],TEST=>"$_>$max_length"};
                                    }
                                    push @array_objects, {SIZE=>join('*', map {"($_)"} @array_size),DATA=>\@field_array_objects};
                                    @object = ();
                                }
                            }
                        }
                        push @objects, \@object if scalar @object;
                    }
                    if( $struct_field->{TYPE} eq 'NEXUS_CallbackDesc') {
                        if ($param->{INPARAM}) {
                            my $callback_kind=get_callback_kind($func, $class_handles,$param);
                            my $id = get_field_callback_id($structs, $param, $field_no);
                            push @callback_ids, [$struct_field->{NAME},$id];
                        }
                    }
                }
                if(scalar @callback_ids) {
                    my $callback_kind=get_callback_kind($func, $class_handles,$param);
                    if($callback_kind ne '_CONSTRUCTOR' && $callback_kind ne '_CLEAR') {
                        $all_callback_ids{$param->{BASETYPE}} = \@callback_ids;
                    }
                }
            }

            # find arguments that are objects
            if(exists $class_dict{$param->{TYPE}}) {
                my $handletype = $param->{TYPE};

                if ($param->{TYPE} eq $handletype) {
                    my @object;
                    my $class_name = bapi_classes::get_class_name($handletype);
                    push @object, "BDBG_STRING(\"$param->{NAME}\") ,";
                    push @object, "NEXUS_OFFSETOF($arg_type, $args . args . $param->{NAME} ),  /*  offset */";
                    push @object, "-1 , /* null_offset */";
                    push @object, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                    push @object, ($param->{ATTR}->{'null_allowed'} ? "true":"false") . ", /* null_allowed */";
                    push @object, "&NEXUS_OBJECT_DESCRIPTOR($class_name) /* descriptor */";
                    if($param->{INPARAM} || is_special_handle($param->{TYPE})) {
                        push @objects, \@object;
                        $referenced_objects{$class_name} = 1;
                    } else {
                        print $file "/* Ignored output parameter $param->{TYPE} $param->{NAME} */\n";
                    }
                }
            }
            if(exists $param->{BASETYPE} and exists $class_dict{$param->{BASETYPE}}) {
                my $handletype = $param->{BASETYPE};
                my $class_name = bapi_classes::get_class_name($handletype);
                my @object;
                if($param->{INPARAM}) {
                    if(exists $param->{ATTR}{handle_verify} && $param->{ATTR}{handle_verify} eq 'no') {
                        print $file "/* don't verify input parameter '$param->{TYPE} $param->{NAME}' */\n";
                    } else {
                        push @object, "'NOT SUPPORTED input parameter $param->{TYPE} $param->{NAME}'";
                    }
                } else {
                    if(is_special_handle($param->{BASETYPE})) {
                        push @object, "'NOT SUPPORTED output parameter $param->{TYPE} $param->{NAME}'";
                    } else {
                        print $file "/* Ignored output parameter '$param->{TYPE} $param->{NAME}' */\n";
                    }
                }
                push @objects, \@object if(scalar @object);
            }
        }
        for (sort keys %referenced_objects) {
            if(not exists $declared_objects{$_}) {
                print $file "extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR($_);\n";
                $declared_objects{$_}=1;
            }
        }
        my $nobjects = scalar @objects;
        if (scalar @objects or scalar @array_objects) {
            my $n=0;
            print $file "static const struct api_object_descriptor _objects_$func->{FUNCNAME} [] = {\n";
            for (@objects) {
                print $file ",\n" if $n;
                $n++;
                print $file " {\n";
                bapi_util::print_code $file, $_, "   ";
                print $file " }";
            }
            for (@array_objects) {
                $nobjects = $nobjects . "+ $_->{SIZE}";
                my $data = $_->{DATA};
                for(@$data) {
                    print $file "\n#if $_->{TEST}\n";
                    print $file ",\n" if $n;
                    $n++;
                    print $file " {\n";
                    bapi_util::print_code $file, $_->{DATA}, "   ";
                    print $file " }";
                    print $file "\n#endif\n";
                }
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

        push @function, "BDBG_STRING(\"$func->{FUNCNAME}\"),";
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
        push @function, $nobjects eq '0' ? "NULL,": "_objects_$func->{FUNCNAME},";
        push @function, $npointers==0 ? "NULL,": "_pointers_$func->{FUNCNAME},";
        print $file "static const struct api_function_descriptor _api_$func->{FUNCNAME} = {\n";
        bapi_util::print_code $file, \@function, "   ";
        print $file "};\n";
    }

    for my $name (sort keys %all_callback_ids) {
        print $file "B_IPC_SERVER_CALLBACK_LIST_BEGIN($name)\n";
        my @ids = map { "B_IPC_SERVER_CALLBACK_LIST_ID($_->[0],$_->[1])"} @{$all_callback_ids{$name}};
        bapi_util::print_code $file, \@ids, "   ";
        print $file "B_IPC_SERVER_CALLBACK_LIST_END($name)\n";
        print $file "\n";
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
    print $fout "#pragma GCC diagnostic ignored \"-Wunused-function\"";
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
    for my $func (@$functions) {
        next if(exists $func->{ATTR}{'local'});
        my @postprocess;
        my $funcname = $func->{FUNCNAME};
        my $api = "_${funcname}";
        my $bypass_call = 0;
        print_function_prototype($fout, $func);
        print $fout "B_IPC_SERVER_BEGIN($module, $api)\n";
        for my $param (@{$func->{PARAMS}} ) {
            print $fout "    B_IPC_SERVER_PARAM($param->{TYPE}, $param->{NAME})\n";
        }
        my $code = process_arguments($functions, $func, $class_handles, $structs);
        bapi_util::print_code($fout, $code->{SERVER}{DECLARE}, $tab);
        if($func->{RETTYPE} ne 'void') {
            print $fout "    B_IPC_SERVER_RESULT($func->{RETTYPE}, __result)\n";
            print $fout "    BSTD_UNUSED(__result);\n";
        }

        print $fout "    B_IPC_SERVER_VERIFY($module, _${funcname})\n";
        bapi_util::print_code($fout, $code->{SERVER}{RECV}, $tab);
        if(exists $code->{SERVER}{CALLBACK_VERIFY}) {
            for my $name (sort keys %{$code->{SERVER}{CALLBACK_VERIFY}}) {
                bapi_util::print_code($fout, [$code->{SERVER}{CALLBACK_VERIFY}{$name}], $tab);
            }
        }
        bapi_util::print_code($fout, $code->{SERVER}{RECV_VARARG}, $tab);
        if(exists $code->{SERVER}{OUT_VARARGS}) {
            bapi_util::print_code($fout, ["B_IPC_SERVER_BEGIN_OUT_VARARG($api)"],$tab);
            bapi_util::print_code($fout, $code->{SERVER}{OUT_VARARGS}{SIZE}, "$tab$tab");
            bapi_util::print_code($fout, ["B_IPC_SERVER_ALLOCATE_OUT_VARARG($api)"],"$tab$tab");
            bapi_util::print_code($fout, $code->{SERVER}{OUT_VARARGS}{PLACE}, "$tab$tab");
            bapi_util::print_code($fout, ["B_IPC_SERVER_END_OUT_VARARG($api)"],$tab);
        }
        bapi_util::print_code($fout, $code->{SERVER}{SET_OUT_PTR}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{PREPARE}, $tab);
        bapi_util::print_code($fout, $code->{SERVER}{CALLBACK_PRE}, $tab);

        print $fout "    B_IPC_SERVER_TRACE((\">%s: $code->{TRACE}{FORMAT}\", \"$funcname\"  $code->{TRACE}{ARG}))\n";

        if(exists $func->{PARAMS}[0]) {
            my $handle = $func->{PARAMS}[0]{NAME};
            for my $c (@$classes) {
                if($c->{DESTRUCTOR}{FUNCNAME} eq $funcname) {

                    if(is_special_handle($func->{PARAMS}[0]{TYPE})) {
                        print $fout "    B_IPC_SERVER_DESTROY_ENUM($module, $api, $c->{CLASS_NAME}, $handle)\n";
                    } else {
                        print $fout "    B_IPC_SERVER_DESTROY($module, $api, $c->{CLASS_NAME}, $handle)\n";
                    }
                    if(exists $c->{SHUTDOWN}) {
                        push @postprocess, "B_IPC_SERVER_SHUTDOWN($api, $c->{CLASS_TYPE}, $handle)";
                        $bypass_call = 1;
                    }
                    last;
                }
                if(exists $c->{RELEASE}{FUNCNAME} and $c->{RELEASE}{FUNCNAME} eq $funcname) {
                    print $fout "    B_IPC_SERVER_RELEASE($module, $api, $c->{CLASS_NAME}, $handle)\n";
                }
            }
        }


        if(!$bypass_call) {
            my $args = join(',', map  { $_->{NAME} } @{$func->{PARAMS}});
            if($func->{RETTYPE} eq 'void') {
                print $fout "    B_IPC_SERVER_CALL_VOID($api, $funcname, ($args))\n";
            } elsif(is_handle($func->{RETTYPE}, $class_handles)) {
                my $class_constructor;
                my $class_acquire;
                print $fout "    B_IPC_SERVER_CALL_HANDLE($api, $funcname, ($args))\n";
                for my $c (@$classes) {
                    if($c->{DESTRUCTOR_TYPE} eq 'destructor') {
                        if( scalar( grep {$funcname eq $_->{FUNCNAME}} @{$c->{CONSTRUCTORS}} )) {
                            if(is_special_handle($func->{RETTYPE})) {
                                print $fout "    B_IPC_SERVER_CONSTRUCTOR_ENUM($module, $api, $c->{CLASS_NAME}, $funcname)\n";
                            } else {
                                print $fout "    B_IPC_SERVER_CONSTRUCTOR($module, $api, $c->{CLASS_NAME}, $funcname)\n";
                            }
                            last;
                        }
                    }
                    if(exists $c->{ACQUIRE} and $c->{ACQUIRE}{FUNCNAME} eq $funcname) {
                        print $fout "    B_IPC_SERVER_ACQUIRE($module, $api, $c->{CLASS_NAME}, $funcname)\n";
                        last;
                    }
                }
            } else {
                print $fout "    B_IPC_SERVER_CALL($api, $funcname, ($args))\n";
            }
        }

        print $fout "    B_IPC_SERVER_TRACE((\"<%s(%d):$code->{TRACE}{RESULT}{FORMAT} $code->{TRACE}{FORMAT}\", \"$funcname\", __rc $code->{TRACE}{RESULT}{ARG} $code->{TRACE}{ARG}))\n";
        bapi_util::print_code($fout, $code->{SERVER}{CALLBACK_POST}, $tab);
        print $fout "    B_IPC_SERVER_CHECK_RETURN_CODE($api)\n";
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

sub parse_array_field {
    my ($field)=@_;
    my @elements;
    my @parts = split(/\[|\]/, $field);
    print STDERR Dumper($field,scalar @parts, \@parts) if 0;
    for(my $i=0; $i < scalar @parts; ) {
        my $part = $parts[$i];
        if($i>0) {
            $part = substr $part, 1; # drop leading '.'
        }
        if($i+1 == scalar @parts) {
            push @elements, {FIELD => $part};
            last;
        } else {
            my @size;
            for($i++;$i< scalar @parts;) {
                push @size, $parts[$i];
                $i++;
                if($i< scalar @parts) {
                    last unless $parts[$i] eq '';
                    $i++;
                }
            }
            push @elements, {FIELD => $part, SIZE => \@size};
        }
    }
    return \@elements;
}

if(0) {
    print STDERR Dumper(parse_array_field("fileName[64]"));
    print STDERR Dumper(parse_array_field("fileName[64].x[32]"));
    print STDERR Dumper(parse_array_field("fileName"));
    print STDERR Dumper(parse_array_field("_NEXUS_Platform_GetDefaultSettings_tagged.pointer.pSettings.displayModuleSettings.videoWindowHeapIndex[NEXUS_MAX_DISPLAYS][NEXUS_MAX_VIDEO_WINDOWS]"));
    print STDERR Dumper(parse_array_field("pointer.pSettings.customSettings.preambleA[4].value"));
    print STDERR Dumper(parse_array_field("scan.mode[NEXUS_FrontendQamAnnex_eMax][NEXUS_FrontendQamMode_eMax]"));
    exit(1);
}

# Recursively (!) visit fields in a structure
sub visit_field
{
    my ($fields, $structs, $visitor, @context) = @_;
    for my $field (@$fields) {
        my $type = $field->{TYPE};
        if(exists $structs->{$type}) {
            my $struct_fields = $structs->{$type};
            visit_field($struct_fields, $structs,
                sub {
                    my ($v_field,$path,$p_name, @p_context) = @_;
                    $visitor->($v_field, [$p_name, @$path], @p_context);
                }, $field->{NAME}, @context);
        } else {
            $visitor->($field, [], @context);
        }
    }
    1;
}

sub parse_union_field {
    my ($field)=@_;
    my @parts = split(/\./, $field->{NAME});
    my @union;
    for (0..$#parts) {
        push @union, {'NAME' => $parts[$_], 'KIND' => $field->{KIND}[$_]};
    }
    return \@union;
}

sub convert_one_struct {
    my ($struct_name, $fields, $fout, $module, $structs, $class_handles, $struct_kind, $structs_attr) = @_;
    my @from;
    my @to;
    my $cur_array=[];
    my $tab='    ';
    my $prev_union_fields=[];
    my $prev_union_pos;


    if($struct_name eq "b_${module}_module_ipc_in"  or $struct_name eq "b_${module}_module_ipc_out") {
        return;
    }
    if( exists $structs_attr->{$struct_name} && exists $structs_attr->{$struct_name}{'local'} && $structs_attr->{$struct_name}{'local'} eq 'true') {
        print $fout "NEXUS_P_COMPAT_LOCAL_STRUCT($struct_name)\n\n";
    }
    for my $field (@$fields) {
        my $type = $field->{TYPE};
        my $array = parse_array_field($field->{NAME});
        print STDERR Dumper($field->{NAME}, $array) if 0;
        my $n;
        my $match;
        for $n (0...$#$cur_array-1) {
            if(exists $array->[$n] and $cur_array->[$n]{FIELD} eq $array->[$n]{FIELD}) {
                $match = $n;
            } else {
                last;
            }
        }
        print STDERR Dumper($field->{NAME}, $match,$#$cur_array, $cur_array,$array) if 0;
        if(defined $match) {
            $match++;
        } else {
            $match = 0;
        }
        for (reverse $match..($#$cur_array-1)) {
            next unless exists $cur_array->[$_]{SIZE};
            my $size =$cur_array->[$_]{SIZE};
            my $dim = scalar @$size;
            my $sizes = join(',', @$size);
            my $indent = $tab x $_;
            push @from, "${indent}NEXUS_P_COMPAT_END_LOOP_$dim($_,$sizes)";
            push @to, "${indent}NEXUS_P_COMPAT_END_LOOP_$dim($_,$sizes)";
        }
        for ($match..($#$array-1)) {
            next unless exists $array->[$_]{SIZE};
            my $size =$array->[$_]{SIZE};
            my $dim = scalar @$size;
            my $sizes = join(',', @$size);
            my $indent = $tab x $_;
            push @from, "${indent}NEXUS_P_COMPAT_BEGIN_LOOP_$dim($_,$sizes) /* $field->{NAME} */";
            push @to, "${indent}NEXUS_P_COMPAT_BEGIN_LOOP_$dim($_,$sizes) /* $field->{NAME} */";
        }

        my $top = $#$array;
        my @names = map {
            my $n = $_;
            if(exists $array->[$n]{SIZE}) {
                $array->[$n]{FIELD} . (join('',(map {"[_n_${n}_${_}]"} (0..$#{$array->[$n]{SIZE}}))))
            } else {
                $array->[$n]{FIELD};
            }
        }  (0..($top-1));
        my $name = join('.', (@names,$array->[$top]{FIELD}));
        my $kind;
        my $args='';
        my $indent = $tab x $top;
        my $union_pos;
        my $union_fields = parse_union_field($field);
        for (0..$#$union_fields) {
            if($union_fields->[$_]{KIND} eq 'union') {
                $union_pos = $_-1;
                last;
            }
        }
        print STDERR Dumper($prev_union_pos,$union_pos, $union_fields) if 0;
        if(defined $union_pos) {
            if(not defined $prev_union_pos or $union_pos != $prev_union_pos or $union_fields->[$union_pos]{NAME} ne $prev_union_fields->[$prev_union_pos]{NAME}) {
                my $union = join('.', @{$field->{KIND}});
                my $union_field = join('.', map {$_->{NAME}} @$union_fields[0..$union_pos]);
                push @from, "${indent}NEXUS_P_COMPAT_COPY_UNION($union_field); /* UNION $field->{NAME} $union */";
                push @to, "${indent}NEXUS_P_COMPAT_COPY_UNION($union_field); /* UNION $field->{NAME} $union */";
                if(index($union_field,'[')!=-1) {
                    push @to, "${indent}NEXUS_P_COMPAT_ARRAY_UNION($union_field); /* not supported */";
                    push @from, "${indent}NEXUS_P_COMPAT_ARRAY_UNION($union_field); /* not supported */";
                }
            }
        } else {
            $kind = get_field_kind($field, $structs, $class_handles);
            $type = get_compat_type($type, $kind);
            print STDERR Dumper($field->{NAME},$type,$kind) if 0;
            if(not exists $array->[$top]{SIZE}) {
                push @from, "${indent}NEXUS_P_COMPAT_FIELD(IN, __rc, __src, __dst, $type, $name$args, $kind)";
                push @to, "${indent}NEXUS_P_COMPAT_FIELD(OUT, __rc, __src, __dst, $type, $name$args, $kind)";
            } else {
                my $size =$array->[$top]{SIZE};
                my $dim = scalar @$size;
                my $sizes = join(',', @$size);
                push @from, "${indent}NEXUS_P_COMPAT_FIELD_ARRAY_$dim(IN, __rc, __src, __dst, $type, $name$args, $sizes, $kind)";
                push @to, "${indent}NEXUS_P_COMPAT_FIELD_ARRAY_$dim(OUT, __rc, __src, __dst, $type, $name$args, $sizes, $kind)";
            }
        }
        $cur_array = $array;
        $prev_union_pos = $union_pos;
        $prev_union_fields = $union_fields;
    }
    for (reverse 0..($#$cur_array-1)) {
        next unless exists $cur_array->[$_]{SIZE};
        my $size =$cur_array->[$_]{SIZE};
        my $dim = scalar @$size;
        my $sizes = join(',', @$size);
        my $indent = $tab x $_;
        push @from, "${indent}NEXUS_P_COMPAT_END_LOOP_$dim($_,$sizes)";
        push @to, "${indent}NEXUS_P_COMPAT_END_LOOP_$dim($_,$sizes)";
    }

    if($struct_kind eq 'out') {
        print $fout "NEXUS_P_COMPAT_OUT_BEGIN($struct_name)\n";
        bapi_util::print_code $fout, \@to, "   ";
        print $fout "NEXUS_P_COMPAT_OUT_END($struct_name)\n";
        print $fout "\n\n";
    } elsif($struct_kind eq 'in') {
        print $fout "NEXUS_P_COMPAT_IN_BEGIN($struct_name)\n";
        bapi_util::print_code $fout, \@from, "   ";
        print $fout "NEXUS_P_COMPAT_IN_END($struct_name)\n";
        print $fout "\n\n";
    }
}

sub verify_one_union
{
    my ($fout, $class_handles, $structs, $name, $fields) = @_;
    my $field;
    for $field (@$fields) {
        my $type = $field->{TYPE};
        my $field_name = $field->{NAME};
        my $union_pos;
        my $union_fields = parse_union_field($field);
        for (0..$#$union_fields) {
            if($union_fields->[$_]{KIND} eq 'union') {
                $union_pos = $_-1;
                last;
            }
        }
        if(defined $union_pos) {
            my @base_parts;
            my @union_parts;
            for(0..$#$union_fields) {
                if($_>$union_pos) {
                    push @union_parts, $union_fields->[$_]{NAME};
                } else {
                    push @base_parts, $union_fields->[$_]{NAME};
                }
            }
            my $base = join('.', @base_parts);
            my $union = filter_array(join('.', @union_parts));
            my $kind = get_type_kind($field->{TYPE}, $field->{ATTR}, $structs, $class_handles);
            print $fout "    NEXUS_P_COMPAT_UNION_VERIFY_$kind($name, $base, $union, $type)\n";
        }
    }
}

sub generate_compat {
    my ($fout, $module, $functions, $referenced_structs, $structs, $class_handles, $api_ids, $varargs_info, $structs_attr) = @_;
    print $fout $autogenerated;
    print $fout "
#include \"nexus_${module}_module.h\"
BDBG_MODULE(nexus_${module}_compat);
#include \"nexus_${module}_api.h\"
#include \"abiverify/nexus_abiverify_compat.h\"
#include \"nexus_${module}_api_compat.h\"

";

    print $fout "NEXUS_P_COMPAT_UNION_BEGIN($module)\n\n";
    for my $name (sort keys %$structs) {
        my $fields = $structs->{$name};
        if(exists $structs_attr->{$name} && exists $structs_attr->{$name}{'local'} && $structs_attr->{$name}{'local'} eq 'true') {
            print $fout "    NEXUS_P_COMPAT_UNION_STRUCT_LOCAL($name)\n";
        } else {
            print $fout "    NEXUS_P_COMPAT_UNION_STRUCT($name)\n";
            verify_one_union ($fout, $class_handles, $structs, $name, $fields);
        }
        print $fout "\n\n";
    }
    print $fout "NEXUS_P_COMPAT_UNION_END($module)\n\n\n";

    for my $kind ('in','out') {
        my %references;
        my @api_structs = @{$referenced_structs->{"b_${module}_module_ipc_$kind"}};
        if(exists $referenced_structs->{"b_${module}_vararg_data_$kind"}) {
            push @api_structs, (grep {exists $structs->{$_->{TYPE}}} @{$referenced_structs->{"b_${module}_vararg_data_$kind"}});
        }
        for (map {$_->{TYPE}} @api_structs ) {
            $references{$_} = $referenced_structs->{$_};
            my $struct_reference = get_referenced_structs($referenced_structs->{$_}, $referenced_structs);
            for (@$struct_reference) {
                $references{$_}=$referenced_structs->{$_};
            }
        }
        visit_struct(\%references, \&convert_one_struct, $fout, $module, $referenced_structs, $class_handles, $kind, $structs_attr);
    }
    print $fout "NEXUS_P_COMPAT_DECLARE($module)\n";
    print $fout "NEXUS_P_COMPAT_PROLOGUE($module)\n";
    print $fout "NEXUS_P_COMPAT_DISPATCH($module)\n";
    my %func_map = map {$_->{FUNCNAME}, $_} @$functions;
    for (@$api_ids) {
        my $funcname=$_->[0];
        my $code = process_arguments($functions, $func_map{$funcname}, $class_handles, $structs);
        my @declare;
        my @vararg_convert_in;
        my $api = "_${funcname}";
        print $fout "NEXUS_P_COMPAT_BEGIN($module,$api)\n";
        bapi_util::print_code($fout, $code->{COMPAT}{VARARG_IN_DECLARE}, $tab);
        print $fout "    NEXUS_P_COMPAT_CONVERT_IN($module,$api)\n";
        bapi_util::print_code($fout, $code->{COMPAT}{VARARG_IN_PLACE}, $tab);
        bapi_util::print_code($fout, $code->{COMPAT}{VARARG_IN_CONVERT}, $tab);
        print $fout "    NEXUS_P_COMPAT_PROCESS($module,$api)\n";
        if($func_map{$funcname}->{RETTYPE} eq 'NEXUS_Error') {
            print $fout "    NEXUS_P_COMPAT_CHECK_RETURN_CODE($module, $api)\n";
        }

        print $fout "    NEXUS_P_COMPAT_CONVERT_OUT($module,$api)\n";
        if(exists $code->{COMPAT}{VARARG_OUT_DECLARE}) {
            bapi_util::print_code($fout, ["NEXUS_P_COMPAT_VARARG_OUT_BEGIN($api)"],$tab);
            bapi_util::print_code($fout, $code->{COMPAT}{VARARG_OUT_DECLARE}, "$tab$tab");
            bapi_util::print_code($fout, $code->{COMPAT}{VARARG_OUT_PROCESS}, "$tab$tab");
            bapi_util::print_code($fout, $code->{COMPAT}{VARARG_OUT_SIZE}, "$tab$tab");
            bapi_util::print_code($fout, ["NEXUS_P_COMPAT_VARARG_OUT_ALLOCATE($api)"],"$tab$tab");
            bapi_util::print_code($fout, $code->{COMPAT}{VARARG_OUT_PLACE}, "$tab$tab");
            bapi_util::print_code($fout, $code->{COMPAT}{VARARG_OUT_CONVERT}, "$tab$tab");
            bapi_util::print_code($fout, ["NEXUS_P_COMPAT_VARARG_OUT_END($api)"],$tab);
        }
        print $fout "NEXUS_P_COMPAT_END($module,$api)\n";
        print $fout "\n\n";
    }
    print $fout "NEXUS_P_COMPAT_EPILOGUE($module)\n";
};

sub generate_compat_api {
    my ($fout, $referenced_structs, $structs, $class_handles) = @_;
    print $fout $autogenerated;
    my %extra_structs;

    print_struct($fout, $referenced_structs, $class_handles, 1);
    for(keys %$structs) {
        $extra_structs{$_} = $structs->{$_} unless exists $referenced_structs->{$_};
    }
    print_struct($fout, \%extra_structs, $class_handles, 1);
}


sub generate {
    my ($destdir, $module, $functions, $class_handles, $structs, $version, $ioctls, $referenced_structs, $api_ids, $compat_info, $varargs_info, $structs_attr) = @_;
    my $fout;
    my $file;
    my $fout_compat;
    my $file_compat;
    my $destructors = bapi_classes::get_destructors $functions;
    my $classes = bapi_classes::get_classes $functions, $destructors, $structs;

    $file = "$destdir/nexus_${module}_abiverify_api.h",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_ipc_api($fout, $module, $functions, $class_handles, $structs, $version);
    close $fout;

    $file = "$destdir/nexus_${module}_abiverify_client.h",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_client($fout, $module, $functions, $class_handles, $structs, $destructors, $classes);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_client.c",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_ipc_client($fout, $module, $functions, $class_handles, $structs);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_ioctl.h",
    open ($fout, '>', $file) or die "Can't open '$file'";
    $file_compat = "$destdir/nexus_${module}_abiverify_ioctl_compat.h",
    open ($fout_compat, '>', $file_compat) or die "Can't open '$file_compat'";
    generate_ioctl($fout, $fout_compat, $module, $functions, $class_handles, $structs, $ioctls);
    close ($fout);
    close ($fout_compat);

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

    $file = "$destdir/nexus_${module}_abiverify_process_ioctl.h",
    open ($fout, '>', $file) or die "can't open '$file'";
    generate_driver_process_ioctls($fout, $module, $functions, $class_handles, $structs, $varargs_info);
    close ($fout);

    $file = "$destdir/nexus_${module}_abiverify_driver.c",
    open ($fout, '>', $file) or die "can't open '$file'";
    generate_driver($fout, $module, $functions, $class_handles, $structs, $varargs_info);
    close ($fout);

    $file = "$destdir/nexus_${module}_api_compat.h",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_compat_api($fout, $referenced_structs, $structs, $class_handles);
    close ($fout);

    $file = "$destdir/nexus_${module}_api_compat.c",
    open ($fout, '>', $file) or die "Can't open '$file'";
    generate_compat($fout, $module, $functions, $referenced_structs, $structs, $class_handles, $api_ids, $varargs_info, $structs_attr);
    close ($fout);

}

1;
