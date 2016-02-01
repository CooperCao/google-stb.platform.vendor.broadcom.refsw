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
package bapi_classes;
use strict;
use warnings FATAL => 'all';

# generate hash of destructor functions mapped to their various destructors by attr
# the key is the constructor
# one destructor per attr per function
# the validation of destructors happens only here. every one else trusts this output.
sub get_destructors
{
    my ($funcs) = @_;
    my $func;
    my ($attr,$value);
    my %destructors;

    for $func (@$funcs) {
        if(exists $func->{ATTR}) {
            while (($attr, $value) = each %{$func->{ATTR}} ) {
                # one "if" clause for each prototype of destructor. valid prototypes are:
                # 1) void destructor(resource_handle)
                # 2) void destructor(main_handle, resource_handle)
                if($attr eq 'destructor' || $attr eq 'release') {
                    my $destructor;
                    for (@$funcs) {
                        if ($value eq $_->{FUNCNAME}) {
                            $destructor = $_;
                            last;
                        }
                    }
                   if (defined $destructor && $destructor->{RETTYPE} eq 'void' &&
                       (scalar @{$destructor->{PARAMS}}) == 1 &&
                       ${$destructor->{PARAMS}}[0]->{TYPE} eq $func->{RETTYPE} )
                    {
                        $destructors{$func}->{$attr} = $destructor;
                        next;
                    }
                    print STDERR "ERROR: invalid $attr $value in function $func->{FUNCNAME}\n";
                }
            }
        }
    }
    return \%destructors;
}

# check if a function is a destructor and which handle is being destroyed
sub get_stopcallbacks_handle
{
    my ($func, $classes) = @_;
    my $class;
    for $class (@$classes) {
        next if ($class->{DESTRUCTOR} != $func);
        if ($class->{HAS_CALLBACKS}) {
            return $func->{PARAMS}->[0]->{NAME};
        }
        last;
    }
    return undef;
}

# return a list of classes
sub get_classes
{
    my ($funcs, $destructors, $structs) = @_;
    my $func;
    my %classes;
    my $class;

    for $func (@$funcs) {
        my $attr;
        foreach $attr (keys %{$destructors->{$func}}) {
            my $destructor = $destructors->{$func}->{$attr};

            #print "$func->{FUNCNAME} $attr -> $destructor->{FUNCNAME}\n";
            if ($attr eq "release") {
                my $class_type = $func->{RETTYPE};
                $class = $classes{$class_type};

                if(defined $class && defined $class->{RELEASE}) {
                    print STDERR "ERROR: cannot have more than one acquire/release acquire=$func->{FUNCNAME}, release=$destructor->{FUNCNAME}\n";
                }
                else {
                    $class->{RELEASE} = $destructor;
                    $class->{ACQUIRE} = $func;
                    $classes{$class_type} = $class;
                }
            }
            else {
                my $class_type = $func->{RETTYPE};

                $class = $classes{$class_type};

                if(defined $class && $class->{DESTRUCTOR}) {
                    # you can have more than one constructor per class, but only one destructor per class
                    if($class->{DESTRUCTOR} == $destructor) {
                        push @{$class->{CONSTRUCTORS}}, $func;
                    } else {
                        print STDERR "ERROR: destructor mismatch $destructor->{FUNCNAME} for function $func->{FUNCNAME}:$func->{RETTYPE}\n";
                        print STDERR "  $class->{DESTRUCTOR} != $destructor\n";
                    }
                } else {
                    $class->{DESTRUCTOR} = $destructor;
                    $class->{CONSTRUCTORS} = [$func];
                    $class->{DESTRUCTOR_TYPE} = $attr;
                    $class->{CLASS_TYPE} = $class_type;
                    $class->{CLASS_NAME} = get_class_name($class_type);
                    $classes{$class_type} = $class;
                    #print "define class $class->{CLASS_NAME}, destructor $destructor->{FUNCNAME}\n";
                }
            }
        }
        if (exists $func->{ATTR}->{"shutdown"}) {
            my $class_type = $func->{PARAMS}[0]->{TYPE};
            $class = $classes{$class_type};
            my $shutdown;
            $shutdown->{'shutdown_target'} = $func->{ATTR}->{'shutdown'};
            $shutdown->{'shutdown_get_connector'} = $func->{FUNCNAME};
            push @{$class->{SHUTDOWN}}, $shutdown;
            $classes{$class_type} = $class;
        }
        
        # does the function have callbacks? if so, mark the class as having callbacks
        if ((scalar @{$func->{PARAMS}}) >= 1 && defined $structs) {
            my $class_type = $func->{PARAMS}[0]->{TYPE};
            $class = $classes{$class_type};
            if (!defined $class) {
                $class_type = $func->{RETTYPE};
                $class = $classes{$class_type};
            }
            if (defined $class && !$class->{HAS_CALLBACKS}) {
                my $has_callbacks;
                my $param;
                for $param (@{$func->{PARAMS}}) {
                    if ($param->{ISREF} && $param->{BASETYPE} eq 'NEXUS_CallbackDesc') {
                        $has_callbacks = 1;
                    }
                    elsif ($param->{ISREF} && exists $structs->{$param->{BASETYPE}}) {
                        my $field;
                        for $field (@{$structs->{$param->{BASETYPE}}}) {
                            if($field->{TYPE} eq 'NEXUS_CallbackDesc') {
                                $has_callbacks = 1;
                                last;
                            }
                        }
                    }
                    if ($has_callbacks) {
                        last;
                    }
                }
                if ($has_callbacks) {
                    #print "class $class->{CLASS_NAME} has callbacks, first found in $func->{FUNCNAME}\n";
                    $class->{HAS_CALLBACKS} = 1;
                }
            }
        }
    }

    my @c = values %classes;
    return \@c;
}

sub skip_thunk
{
    my $file = shift;
    return
        $file =~ /\*/ || # skip not expaneded wildcards
        $file =~ /_init.h$/ ||
        $file =~ /nexus_platform_client.h$/ ||
        $file =~ /nexus_base\w*.h$/;
}

sub get_class_name {
    my $type = shift;
    $type =~ s/Handle$//;
    return $type;
}


sub generate_class_table
{
    my ($file, $classes)=@_;
    if(scalar @$classes) {
        my $class;
                
        # create local close function so that pre-close functions can be called
        for $class (@$classes) {
            next if ($class->{DESTRUCTOR_TYPE} ne 'destructor');
            next if (!exists $class->{SHUTDOWN});
            
            my $handletype = $class->{CLASS_TYPE};
            my $func = $class->{DESTRUCTOR};
            
            # shutdown_close will perform a platform-level shutdown of the connector, then close.
            print $file "static void nexus_driver_shutdown_close_$handletype($handletype handle)\n";
            print $file "{\n";
            print $file "  NEXUS_UnlockModule();\n";
            for (@{$class->{SHUTDOWN}}) {
                my $shutdown_target = $_->{'shutdown_target'};
                my $shutdown_get_connector = $_->{'shutdown_get_connector'};
                my $params = $func->{PARAMS};
                print $file "  nexus_driver_shutdown_$shutdown_target($shutdown_get_connector(handle));\n";
            }
            print $file "  NEXUS_LockModule();\n";
            # actual close
            print $file "  $func->{FUNCNAME}(handle);\n";
            print $file "}\n\n";
            
        }
        
        print $file "B_OBJDB_TABLE_BEGIN(NEXUS_DRIVER_MODULE_CLASS_TABLE)\n";
        for $class (@$classes) {
            my $class_type = $class->{CLASS_TYPE};
            my $class_name = $class->{CLASS_NAME};
            my $destructorfunc;
            if ($class->{DESTRUCTOR_TYPE} eq 'destructor' && exists $class->{SHUTDOWN}) {
                $destructorfunc = "nexus_driver_shutdown_close_$class_type";
            }
            else {
                $destructorfunc = $class->{DESTRUCTOR}->{FUNCNAME};
            }
            my $releasefunc = $class->{RELEASE}->{FUNCNAME};
            if (!defined $destructorfunc) {
                $destructorfunc = "ERROR: missing destructor"; # required. this will cause the compilation to fail.
            }
            if (!defined $releasefunc) {
                $releasefunc = "NULL"; # not required
            }
            print $file "    B_OBJDB_TABLE_ENTRY($class_name,$releasefunc,$destructorfunc)\n";
        }
        print $file "B_OBJDB_TABLE_END\n";
    } else {
        print $file "B_OBJDB_TABLE_BEGIN(NEXUS_DRIVER_MODULE_CLASS_TABLE)\n";
        print $file "B_OBJDB_TABLE_END\n";
    }
}

sub generate_meta
{
    my ($file, $module, $version, $structs, $classes, $funcs, $class_handles, $mode) = @_;
    my $func;
    my $name;

    for $func (@$funcs) {
        my $params = $func->{PARAMS};
        my $param;
        my $in_args;
        my $out_args;
        my $arg_type;
        my $isnull_suffix;
        my $isnull_prefix;
        if($mode->{KIND} eq 'ipc') {
            $arg_type = 'nexus_server_args';
            $in_args = "data.$func->{FUNCNAME}.in";
            $out_args = "data.$func->{FUNCNAME}.out";
            $isnull_prefix = '';
            $isnull_suffix = '_isnull';
        } elsif($mode->{KIND} eq 'driver') {
            $arg_type = 'union nexus_driver_module_args';
            $in_args = "$func->{FUNCNAME}";
            $out_args = "$func->{FUNCNAME}";
            $isnull_prefix = 'ioctl.';
            $isnull_suffix = '';
        } else {
            die "$mode->{KIND} not supported";
        }
        my @function;
        my @objects;
        my @pointers;

        for (@{$func->{PARAMS}}) {
            my $param = $_;
            my $args = $param->{INPARAM} ? $in_args : $out_args;

            # find arguments that are pointers
            if ($param->{ISREF} && $param->{BASETYPE} ne 'void') {
                if (!exists $param->{ATTR}->{'nelem'}) {
                    my @pointer;
                    push @pointer, "\"$param->{NAME}\" ,";
                    push @pointer, "NEXUS_OFFSETOF($arg_type, $args . $param->{NAME}), /* offset */";
                    if($param->{INPARAM}) {
                        push @pointer, "NEXUS_OFFSETOF($arg_type, $args . $isnull_prefix$param->{NAME}$isnull_suffix ), /* null_offset */";
                    } else {
                        push @pointer, "-1, /* null_offset */";
                    }
                    push @pointer, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                    push @pointer, ($param->{ATTR}->{'null_allowed'} ? "true":"false") . '/* null_allowed */';
                    push @pointers, \@pointer;
                }
            }

            # find arguments that are pointers to structures and scan them
            if ($param->{ISREF} && exists $structs->{$param->{BASETYPE}}) {
                foreach (@$class_handles) {
                    # first, check if param is a struct which has a class handle field
                    my $handletype = $_;
                    my $struct_field;
                    for $struct_field (@{$structs->{$param->{BASETYPE}}}) {
                        if ($struct_field->{TYPE} eq $handletype ) {
                            my $class_name = bapi_classes::get_class_name($handletype);
                            my @object;

                            push @object, '"' . $param->{NAME} . '->' . $struct_field->{NAME} . '",';
                            push @object, "NEXUS_OFFSETOF($arg_type, $args . $param->{NAME}.$struct_field->{NAME} ), /* offset */";
                            if($param->{INPARAM}) {
                                push @object, "NEXUS_OFFSETOF($arg_type, $args . $isnull_prefix$param->{NAME}$isnull_suffix), /* null_offset */";
                            } else {
                                push @object, "-1, /* null_offset */";
                            }
                            push @object, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                            push @object, "true, /* null allowed */";
                            print $file "extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR($class_name);\n";
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
                    if($mode->{KIND} eq 'driver') {
                        push @object, "NEXUS_OFFSETOF($arg_type, $args .  ioctl . $param->{NAME} ),  /*  offset */";
                    } else {
                        push @object, "NEXUS_OFFSETOF($arg_type, $args . $param->{NAME} ),  /*  offset */";
                    }
                    push @object, "-1 , /* null_offset */";
                    push @object, ($param->{INPARAM} ? "true":"false") . ', /* inparam */';
                    push @object, ($param->{ATTR}->{'null_allowed'} ? "true":"false") . ", /* null_allowed */";
                    push @object, "&NEXUS_OBJECT_DESCRIPTOR($class_name) /* descriptor */";
                    push @objects, \@object;
                    print $file "extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR($class_name);\n";
                }
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
        if(exists $mode->{FUNCHEADER}) {
            $mode->{FUNCHEADER}->(\@function, $module,$func);
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


1;
