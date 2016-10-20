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
use bapi_util;

package bapi_common;

sub version_value
{
    my $version = shift;
    sprintf " 0x%08Xul ", ($version & 0xFFFFFFFF);
}

sub version_ioctl
{
    my $module = shift;
    return "IOCTL_${module}_NEXUS_INIT";
}

sub version_define
{
    my $module = shift;
    return "NEXUS_${module}_MODULE_VERSION";
}

sub ioctl_name
{
    my $module = shift;
    my $func = shift;

    return "IOCTL_${module}_$func->{FUNCNAME}";
}

sub ioctl_struct
{
    my $module = shift;
    my $func = shift;
    return "${module}_$func->{FUNCNAME}_data";
}

sub ioctl_header
{
    my $module = shift;
    my $module_lc = lc $module;

    return "nexus_${module_lc}_ioctl.h";
}

sub process_function_attributes {
    my $func = shift;
    my $structs = shift;
    my $funcs = shift;
    my $params = $func->{PARAMS};
    my @proxy_post_success;
    my @proxy_pre_call;
    my @driver_pre_call;
    my @driver_post_success;
    my @driver_post_error;
    my @driver_post_always;
    my @proxy_vars;
    my @driver_ioctl_reserved;
    my @driver_ioctl_data;
    my %driver_overwrite;
    my ($attr,$value);
    my $param;
    my $field;
    my $arg_no=0;
    my $driver_arg = "module->args.$func->{FUNCNAME}";

    if(exists $func->{ATTR}) {
        while (($attr, $value) = each %{$func->{ATTR}} ) {
            next if($attr eq 'destructor');
            next if($attr eq 'release');
            next if($attr eq 'local');
            next if($attr eq 'thunk');
            next if($attr eq 'shutdown');
            print STDERR "ERROR: Unsupported attribute $attr = $value for $func->{FUNCNAME}\n";
        }
    }

    for $param (@$params) {
        my $name = $param->{NAME};
        my $name_copy = "$name\_copy";
        $arg_no ++;
        
        if (!exists $param->{ATTR}->{'nelem'})
        {
            # ordinary params
            if ($param->{ISREF} && $param->{BASETYPE} ne 'void') {
                if ($param->{INPARAM}) {
                    push @driver_pre_call, "if ($driver_arg.ioctl.$param->{NAME}) {";
                    push @driver_pre_call, "  if (copy_from_user_small(&$driver_arg.$name, $driver_arg.ioctl.$name, sizeof($driver_arg.$name))!=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}";
                    push @driver_pre_call, "}";
                    my $null_allowed = $param->{ATTR}->{'null_allowed'};
                    if (!defined $null_allowed) {
                        push @driver_pre_call, "else {";
                        push @driver_pre_call, "  BDBG_ERR((\"NULL not allowed for $name param in $func->{FUNCNAME}: %d\", __LINE__));";
                        push @driver_pre_call, "  goto err_invalid_ioctl;";
                        push @driver_pre_call, "}";
                    }
                } 
                else {
                    push @driver_pre_call, "NEXUS_IOCTL_CLEAR($driver_arg.ioctl.$name);";
                }
            }
        }
        
        if($param->{BASETYPE} eq 'NEXUS_CallbackDesc') {
            if ($param->{INPARAM}) {
                my $handle = "$driver_arg.ioctl.$$params[0]->{NAME}";
                my $id = sprintf("0x%05x", ((bapi_util::func_id $funcs, $func)*256 + $arg_no + 0x10000));
                my $notnull_test = "$driver_arg.ioctl.$name !=  NULL";
                push @driver_pre_call, "if($notnull_test) {";
                push @driver_pre_call, "   /* MAP callback from proxy space to the driver space */";
                push @driver_pre_call, "   NEXUS_DRIVER_CALLBACK_TO_DRIVER(&$driver_arg.$name, $handle, $id);";
                push @driver_pre_call, "}";
                push @driver_post_success, "if($notnull_test) {";
                push @driver_post_success, "    /* after function succeded, commit changes */";
                push @driver_post_success, "    NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(&$driver_arg.$name, $handle, $id);";
                push @driver_post_success, "}";
                push @driver_post_error, "if($notnull_test) {";
                push @driver_post_error, "    /* if error occurred, then cancel changes in the callback mapping */";
                push @driver_post_error, "    NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(&$driver_arg.$name, $handle, $id);";
                push @driver_post_error, "}";
            } else {
                print STDERR "ERROR: pointer to NEXUS_CallbackDesc could be only input parameter\n";
            }
        }

        if (exists $structs->{$param->{BASETYPE}}) {
            my $field_no = 0;
            my @pre_update_proxy;
            my @pre_update_driver;
            my @post_update_proxy;
            my @post_update_driver;
            my @post_error_driver;

#            bapi_parse_c::print_struct $structs->{$param->{BASETYPE}}
            for $field (@{$structs->{$param->{BASETYPE}}}) {
                my $field_driver = "$driver_arg.$name.$field->{NAME}";

                $field_no ++;
                if($field->{TYPE} eq 'NEXUS_CallbackDesc') {
                    my $id = sprintf("0x%04x", ((bapi_util::struct_id $structs, $param->{BASETYPE})*256 + $field_no));
                    my $handle;
                    if ($func->{RETTYPE_ISHANDLE}) {
                        $handle = 'NULL'; # update after success
                    }
                    elsif ($$params[0]->{ISHANDLE}) {
                        $handle = "$driver_arg.ioctl.$$params[0]->{NAME}";
                    }
                    elsif (!$$params[0]->{ISREF} && $param != $$params[0]) {
                        # enum instead of handle.
                        # use enum type + enum value as psuedo handle
                        # hardcoded for a max of 128 enum values per type
                        # value<<2+1 to use non-4 byte aligned value to ensure no conflict with actual handle
                        $handle = "(unsigned long)(($id<<9) + ($driver_arg.ioctl.$$params[0]->{NAME}<<2) + 1)";
                    }
                    if (defined $handle && $param->{ISREF}) {
                        if ($param->{INPARAM}) {
                            push @pre_update_driver, "/* MAP callback from proxy space to the driver space */";
                            push @pre_update_driver, "NEXUS_DRIVER_CALLBACK_TO_DRIVER(&$field_driver, $handle, $id);";
                            if($handle eq 'NULL') {
                                push @post_update_driver, "/* since callback wasn't avaliable before calling function, use the return result to update callback */";
                                push @post_update_driver, "NEXUS_DRIVER_CALLBACK_UPDATE(&$field_driver, $handle, $id, $driver_arg.ioctl.__retval);";
                            } else {
                                push @post_update_driver, "/* after function succeded, commit changes */";
                                push @post_update_driver, "NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(&$field_driver, $handle, $id);";
                            }
                            push @post_error_driver, "/* if error occurred, then cancel changes in the callback mapping */";
                            push @post_error_driver, "NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(&$field_driver, $handle, $id);";
                        } else {
                            push @post_update_driver, "/* MAP callback from the driver space to proxy space */";
                            push @post_update_driver, "NEXUS_DRIVER_CALLBACK_TO_USER(&$field_driver, $handle, $id);";
                        }
                    }
                }

                if (exists $field->{ATTR}) {
#                   print " $func->{FUNCNAME} :";
#                   print "  field: $field->{TYPE} $field->{NAME} -> ";
#                   bapi_parse_c::print_attr $field->{ATTR};
                    while (($attr, $value) = each %{$field->{ATTR}} ) {
                        my $field_proxy = "$name->$field->{NAME}";
                        my $field_copy = "$name_copy.$field->{NAME}";
                        if($attr eq 'kind' && $value eq 'null_ptr' ) {
                            next;
                        } elsif($attr eq 'memory') {
                            if($value eq 'cached') {
                                if ($param->{ISREF}) {
                                    if ($param->{INPARAM}) {
                                        next if (exists $param->{ATTR}->{'nelem'});
                                        push @pre_update_proxy, "/* convert address to the device offset */";
                                        push @pre_update_proxy, "if ($field_copy) {$field_copy = (void *)(unsigned long) NEXUS_AddrToOffset($field_copy); if (!$field_copy) rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);}";

                                        push @pre_update_driver, "/* convert offset (substituted by the proxy ) to the cached address */";
                                        push @pre_update_driver, "NEXUS_DRIVER_RECV_ADDR($field_driver,cached);";
                                        next;
                                    } else {
                                        push @post_update_proxy, "/* convert offset (substituted by the driver) to the cached address */";
                                        push @post_update_proxy, "$field_proxy = $field_proxy ? NEXUS_OffsetToCachedAddr((unsigned long)$field_proxy) : $field_proxy ;";

                                        push @post_update_driver, "/* convert offset (substituted by the proxy) to the cached address */";
                                        push @post_update_driver, "NEXUS_DRIVER_SEND_ADDR($field_driver,cached);";
                                        next;
                                    }
                                }
                            }
                        }
                        print STDERR "ERROR: Unsupported attribute $attr = $value for $field->{NAME} in $param->{BASETYPE}\n";
                    }
                }
            }
            if(scalar @pre_update_proxy) {
                push @proxy_vars, "$param->{BASETYPE} $name_copy; /* placeholder for modified parameters */ ";
                push @proxy_vars, "const $param->{BASETYPE} *$name\_old; /* copy of old pointer */ ";
                push @proxy_pre_call, "BSTD_UNUSED($name\_old);\n";
                push @proxy_pre_call, "if($name) {";
                push @proxy_pre_call, "     /* replace the original pointer with local copy */";
                push @proxy_pre_call, "     $name\_old=$name;$name\_copy=*$name;$name=&$name\_copy;";
                push @proxy_pre_call, "     /* update values ... */";
                bapi_util::append_code \@proxy_pre_call, \@pre_update_proxy, "     ";
                push @proxy_pre_call, "}";
            }
            bapi_util::append_if \@driver_pre_call, "$driver_arg.ioctl.$name", \@pre_update_driver;
            bapi_util::append_if \@driver_post_error, "$driver_arg.ioctl.$name", \@post_error_driver;
            bapi_util::append_if \@driver_post_success, "$driver_arg.ioctl.$name", \@post_update_driver;
            bapi_util::append_if \@proxy_post_success , "$name", \@post_update_proxy;
        }
        if(exists $param->{ATTR}) {
            while (($attr, $value) = each %{$param->{ATTR}} ) {
                if($attr eq 'nelem_convert' || $attr eq 'nelem_out') {
                    if( !exists $param->{ATTR}->{'nelem'}) {
                        print STDERR "ERROR: $attr requires nelem: $func->{FUNCNAME}\n";
                    }
                    next; # handled with nelem
                }
                next if($attr eq 'null_allowed'); # skip here

                if($attr eq 'reserved') {
                    if($param->{BASETYPE} ne 'void') {
                        push @driver_ioctl_reserved, "$param->{BASETYPE} $name\[$value];";
                    } else {
                        push @driver_ioctl_reserved, "uint8_t $name\[$value];";
                    }
                }
                elsif($attr eq 'nelem' && $param->{ISREF}) {
                    my $length = "$driver_arg.ioctl.$value";
                    my $scale = "";
                    if( exists $param->{ATTR}->{'nelem_convert'}) {
                        $length =  $param->{ATTR}->{'nelem_convert'} . "($length)";
                    }
                    if($param->{BASETYPE} ne 'void') {
                        push @driver_ioctl_data, "$param->{BASETYPE} * $name;";
                        $scale = "sizeof(*$driver_arg.ioctl.$name) *";
                    } else {
                        push @driver_ioctl_data, "void * $name;";
                    }
                    $length = "($scale $length)";
                    if (!exists $param->{ATTR}->{'null_allowed'}) {
                        push @driver_pre_call, "if($driver_arg.ioctl.$name==NULL) {NEXUS_DRIVER_TRACE(-1); goto err_invalid_ioctl;}";
                    }
                    push @driver_pre_call, "if($driver_arg.ioctl.$name!=NULL) {";
                    push @driver_pre_call, "    /* allocate space for the $name */";
                    push @driver_post_always, "/* free space allocated for $name */";
                    if( exists $param->{ATTR}->{'reserved'}) {
                        push @driver_pre_call, "    if( $length <= $scale $param->{ATTR}->{'reserved'} ) {";
                        push @driver_pre_call, "        $driver_arg.$name = $driver_arg.reserved.$name;";
                        push @driver_pre_call, "    } else {";
                        push @driver_pre_call, "        if ($length == 0) {NEXUS_DRIVER_TRACE(-1); goto err_alloc;}";
                        push @driver_pre_call, "        $driver_arg.$name = NEXUS_DRIVER_ALLOC($length);";
                        push @driver_pre_call, "        if($driver_arg.$name == NULL) {goto err_alloc;}";
                        push @driver_pre_call, "    }";

                        push @driver_post_always, "if($driver_arg.$name != $driver_arg.reserved.$name) {";
                        push @driver_post_always, "    NEXUS_DRIVER_FREE($driver_arg.$name);";
                        push @driver_post_always, "}";

                    } else {
                        push @driver_pre_call, "    if ($length == 0) {NEXUS_DRIVER_TRACE(-1); goto err_alloc;}";
                        push @driver_pre_call, "    $driver_arg.$name = NEXUS_DRIVER_ALLOC($length);";
                        push @driver_pre_call, "    if ($driver_arg.$name == NULL) {NEXUS_DRIVER_TRACE(-1); goto err_alloc;}";
                        push @driver_post_always, "NEXUS_DRIVER_FREE($driver_arg.$name);";
                    }
                    if($param->{INPARAM}) {
                        push @driver_pre_call, "    if (copy_from_user_small($driver_arg.$name, $driver_arg.ioctl.$name, $length) !=0) {NEXUS_IOCTL_FAULT($func->{FUNCNAME}, $name);goto err_fault;}";
                        # Handle address translation
                        my @driver_update_structure;
                        my @proxy_update_structure;
                        if (exists $structs->{$param->{BASETYPE}}) {
                            my $field;
                            for $field (@{$structs->{$param->{BASETYPE}}}) {
                                if (exists $field->{ATTR}) {
                                    my ($attr,$value);
                                    while (($attr, $value) = each %{$field->{ATTR}} ) {
                                        if($attr eq 'memory' && $value eq 'cached' && $param->{ISREF} && $param->{INPARAM}) {
                                            my $field_copy = "$name_copy->$field->{NAME}";

                                            push @proxy_update_structure, "/* convert address to the device offset */";
                                            push @proxy_update_structure, "if ($field_copy) {$field_copy = (void *)(unsigned long) NEXUS_AddrToOffset($field_copy); if (!$field_copy) rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);}";

                                            push @driver_update_structure, "/* convert offset (substituted by the proxy) to the cached address */";
                                            push @driver_update_structure, "NEXUS_DRIVER_RECV_ADDR($driver_arg.$name [i].$field->{NAME},cached);";
                                            next;
                                        }
                                        print STDERR "ERROR: Unsupported attribute $attr = $value for $field->{NAME} in $param->{BASETYPE}\n";
                                    }
                                }
                            }
                        }
                        if (scalar @driver_update_structure) {
                            push @driver_pre_call, "    {unsigned i;for(i=0;i<$driver_arg.ioctl.$value;i++) {";
                            bapi_util::append_code \@driver_pre_call, \@driver_update_structure, "        ";
                            push @driver_pre_call, "    }}";
                        }
                        if (scalar @proxy_update_structure) {
                            # Approximate handling
                            push @proxy_pre_call, "    unsigned i;";
                            push @proxy_pre_call, "    $param->{BASETYPE} * $name_copy = __builtin_alloca(sizeof($param->{BASETYPE}) * $value);";
                            push @proxy_pre_call, "    if($name!=NULL) {";
                            push @proxy_pre_call, "        void *_save_data = $name_copy;";
                            push @proxy_pre_call, "        BKNI_Memcpy($name_copy, $name, sizeof($param->{BASETYPE}) * $value);";
                            push @proxy_pre_call, "        for(i=0;i<$value;i++) {";
                            bapi_util::append_code \@proxy_pre_call, \@proxy_update_structure, "            ";
                            push @proxy_pre_call, "            $name_copy++;";
                            push @proxy_pre_call, "        }";
                            push @proxy_pre_call, "        $name = _save_data;";
                            push @proxy_pre_call, "    }";
                        }
                    }
                    else {
                        my $length_out = $length;

                        # nelem_out is an optimization for out params. only copy the data actually populated.
                        if( exists $param->{ATTR}->{'nelem_out'}) {
                            $length_out = "$driver_arg.$param->{ATTR}->{'nelem_out'}";
                            if( exists $param->{ATTR}->{'nelem_convert'}) {
                                $length_out =  $param->{ATTR}->{'nelem_convert'} . "($length_out)";
                            }
                            $length_out = "($length_out * sizeof(*$driver_arg.ioctl.$name))";
                        }
                        push @driver_post_success, "if ($driver_arg.ioctl.$name!=NULL && copy_to_user_small($driver_arg.ioctl.$name, $driver_arg.$name, $length_out ) !=0) {NEXUS_IOCTL_FAULT($func->{FUNCNAME}, $name);goto err_fault;}";
                    }
                    push @driver_pre_call, "} else {";
                    push @driver_pre_call, "    $driver_arg.$name = NULL;";
                    push @driver_pre_call, "}";
                    $driver_overwrite{$name} = 1;
                }
                elsif($attr eq 'memory' && $value eq 'cached') {
                    if ($param->{INPARAM}) {
                        push @proxy_pre_call, "/* convert offset (substituted by the driver) to the cached address */";
                        push @proxy_pre_call, "if ($name) {$name = (void *)(unsigned long) NEXUS_AddrToOffset($name); if (!$name) rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);}";
                        push @driver_pre_call, "/* convert address to the device offset */";
                        push @driver_pre_call, "NEXUS_DRIVER_RECV_ADDR($driver_arg.ioctl.$name, cached);";
                    }
                    else {
                        # handle return value. if client passes NULL outparam, only the client crashes
                        push @proxy_post_success, "/* convert offset (substituted by the driver) to the cached address */";
                        push @proxy_post_success, "*$name = *$name ? NEXUS_OffsetToCachedAddr((unsigned long)*$name) : *$name;";
                        push @driver_post_success, "/* convert address to the device offset */";
                        push @driver_post_success, "NEXUS_DRIVER_SEND_ADDR($driver_arg.$name, cached);";
                    }
                }
                else {
                    print STDERR "ERROR: Unsupported attribute $attr = $value for $param->{NAME} in $func->{FUNCNAME}\n";
                }
            }
        }
    }
    
    my %result;
    if (scalar @proxy_post_success) {
        $result{'proxy_post_success'} = \@proxy_post_success;
    }
    if (scalar @driver_post_success) {
        $result{'driver_post_success'} = \@driver_post_success;
    }
    if (scalar @proxy_pre_call) {
        $result{'proxy_pre_call'} = \@proxy_pre_call;
    }
    if (scalar @driver_pre_call) {
        $result{'driver_pre_call'} = \@driver_pre_call;
    }
    if (scalar @proxy_vars) {
        $result{'proxy_vars'} = \@proxy_vars;
    }
    if (scalar @driver_post_error) {
        $result{'driver_post_error'} = \@driver_post_error;
    }
    if (scalar @driver_post_always) {
        $result{'driver_post_always'} = \@driver_post_always;
    }

    if(scalar @driver_ioctl_reserved) {
        push @driver_ioctl_data, "struct {";
        bapi_util::append_code \@driver_ioctl_data, \@driver_ioctl_reserved, "    ";
        push @driver_ioctl_data, "} reserved;";
    }
    if(scalar @driver_ioctl_data) {
        $result{'driver_ioctl_data'} = \@driver_ioctl_data;
    }
    $result{'driver_overwrite'} = \%driver_overwrite;
    return \%result;
}


1;

