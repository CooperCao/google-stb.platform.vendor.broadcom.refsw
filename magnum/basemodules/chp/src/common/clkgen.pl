#!/usr/bin/perl
#############################################################################
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
#############################################################################

use strict;
use warnings;

use Data::Dumper;
$Data::Dumper::Sortkeys  = 1;
$Data::Dumper::Useqq    = 1;
$Data::Dumper::Indent   = 1;
$Data::Dumper::Terse    = 1;

my @profiles = ('Standard');

my @nexus_functions = (
        "AIO",
        "BVN",
        "DVPHR", "DVPHR0", "DVPHR1",
        "DVPHT", "DVPHT0", "DVPHT1",
        "HVD", "HVD0", "HVD1", "HVD2",
        "ITU656",
        "M2MC", "M2MC0", "M2MC1",
        "RAAGA", "RAAGA0", "RAAGA1",
        "RFM",
        "SC", "SC0", "SC1", "PAD",
        "SID", "SID0", "SID1",
        "V3D", "V3D0", "V3D1",
        "VDAC",
        "VEC",
        "VICE2", "VICE20", "VICE21",
        "XPT",
        "VIP",
        "SECBUS",
        "AFEC",
        "AIF",
        "CHAN",
        "DSEC",
        "FSK",
        "LEAP",
        "SDS"
    );

my @div_functions = (
        "HVD", "HVD0", "HVD1", "HVD2",
        "RAAGA", "RAAGA0", "RAAGA1",
        "M2MC", "M2MC0", "M2MC1",
        "V3D", "V3D0", "V3D1",
        "VICE2", "VICE20", "VICE21",
        "SID", "SID0", "SID1"
    );

sub is_nexus_function {
    my $function = shift;

    foreach (@nexus_functions) {
        if(index($function, $_) > -1) {return 1;}
    }

    return 0;
}

sub is_div_function {
    my $function = shift;

    foreach (@div_functions) {
        if(index($function, $_) > -1) {return 1;}
    }

    return 0;
}

sub verify_tree {
	my $clk_tree = shift;

	for (sort keys %$clk_tree) {
		foreach my $src (@{$clk_tree->{$_}{_srcs}}) {
            my $cnt = 0;
			if(exists $clk_tree->{$src}) {
				# Eliminate node's functions from parents functions.
				# Do this till we reach null. This will give a list of top level nodes
				foreach my $func (@{$clk_tree->{$_}{_funcs}}) {
                    my $index = 0;
                    foreach my $top (@{$clk_tree->{$src}{_toplvl}}) {
                        if($top eq $func) {
                            splice(@{$clk_tree->{$src}{_toplvl}}, $index, 1);
                        }
                        $index++;
                    }
					if (!grep(/$func/, @{$clk_tree->{$src}{_funcs}})) {
						print "$func exists in $_ but not in $src\n";
					}
				}
			} elsif($src ne "null") {
				print "$src does not exist in tree. Removing it from $_"."'s source\n";
                splice(@{$clk_tree->{$_}{_srcs}}, $cnt, 1);
			}
            $cnt++;
		}
	}
}

sub get_name {
    my $name = shift;


    $name =~ s/CLKGEN_//g;
    $name =~ s/ONOFF_//g;
    $name =~ s/PLL_//g;
    $name =~ s/CLOCK_//g;
    $name =~ s/_CLOCK$//;
    $name =~ s/CLK//g;
    $name =~ s/ENABLE\d*_//g;
    $name =~ s/_ENABLE\d*$//;
    $name =~ s/DISABLE\d*_//g;
    $name =~ s/_DISABLE\d*$//;
    $name =~ s/INST_//g;
    $name =~ s/DIVIDER/DIV/g;
    $name =~ s/CHANNEL/CH/g;
    $name =~ s/TOP_//g;

	return $name;
}


sub get_pda {
    my $func = shift;

    if ($1 =~ /PDAW/){
        return "WRITE";
    } elsif ($1 =~ /PDAR/) {
        return "READ";
    } else  {
        return "";
    }
}

sub rename_function {
    my ($clk, $func) = @_;

    if($func =~/\bXPT\b/) {
        if($clk =~ /DISABLE_XPT_(20P25|27|40P5|54|81)_CLOCK/) {
            $func =~ s/XPT/XPT_REMUX/g;
         } else {
            $func .= " XPT_REMUX"
         }
    }
    $func =~ s/\bHVD\b/HVD0/g;
    $func =~ s/\bHVD_PDA(R|W)\b/HVD0_PDA$1/g;
    $func =~ s/\bDVPHT\b/DVPHT0/g;
    $func =~ s/\bDVPHT_PDA(R|W)\b/DVPHT0_PDA$1/g;
    $func =~ s/\bDVPHRLP\b/DVPHR/g;
    $func =~ s/\bDVPHR\b/DVPHR0/g;
    $func =~ s/\bDVPHR_PDA(R|W)\b/DVPHR0_PDA$1/g;
    $func =~ s/\bBVN(.*?)_PDA(R|W)\b/BVN_PDA$2/g;
    $func =~ s/\bXPTWAKEUP\b/XPT_WAKEUP/g;
    $func =~ s/\bAIFSAT\b/AIFSAT0/g;
    $func =~ s/OBSERVE//g;

    return $func;
}

sub parse_rdb_file {
	my ($file, $clk_tree) = @_;
	my ($reg, $field, $resource, $default, $case);
    my $coreprefix;
    my %featuredef;
    my $disabled = 0;

    open(INFILE,"$file") or die "Can't open input file $file";

    if($file =~ /\/design\/(.*?)\/rdb\//) {
        $coreprefix = uc($1);
    }
    $coreprefix =~ s/SYS_CTRL/SUN_TOP_CTRL/;

    foreach (<INFILE>) {
        s/\r\n/\n/; # convert DOS to Unix
		s/^\s+//;   # Remove whitespace at beginning of line
		s/\s+$//;   # Remove whitespace at end of line
		s/"//g;

        if (/^endfeature/) {
            $disabled = 0;
            next;
        }
        if ($disabled == 1) {
            next;
        }

        if (/^featuredef\s*(.*?)\s*(\d+)/) {
            $featuredef{$1} = $2;
        } elsif (/^feature\s*(.*)/) {
            my $feature = $1;
            if(exists $featuredef{$1}) {
               if ($featuredef{$1} == 0) {
                   $disabled = 1;
               }
            }
        } elsif (/^coreprefix\s*(\w+)/) {
            $coreprefix = $1;
        } elsif (/^regtype32\s*(\w+)/) {
			$reg = $1;
			$reg =~ s/Type/$coreprefix/;
            $case = "";
        } elsif (/^case\s*(\w+)/) {
			$case = $1;
        } elsif (/\s*union\s*/) {
            $case = "";
		} elsif (/^field\s*(\w+)/) {
            $field = $1;
            $resource = $reg."_".$field;
            if($case ne "") {
                $field = $case."_".$field;
            }
		} elsif (/^default\s*(\d+)/) {
            $default = $1;
		} elsif (/^\/\/PMFunction:\s*(.*?)\s*$/) {
            my $func = rename_function($resource, $1);
			push(@{$clk_tree->{$resource}{_funcs}}, split(' ', $func));
			push(@{$clk_tree->{$resource}{_toplvl}}, split(' ', $func));
            $clk_tree->{$resource}{_reg} = $reg;
            $clk_tree->{$resource}{_field} = $field;
            $clk_tree->{$resource}{_default} = $default;
			$clk_tree->{$resource}{_name} = get_name($resource);
            $clk_tree->{$resource}{_delay} = 0;
            $clk_tree->{$resource}{_pda} = get_pda($1);
            $clk_tree->{$resource}{_polarity} = "";
            $clk_tree->{$resource}{_div} = "";
		} elsif (/^\/\/PMSource(\d*):\s*(.*?)\s*$/) {
            my $indx = $1;
			my $srcs = $2;
            if($indx =~ /\d+/) {
                $clk_tree->{$resource}{_mux}++;
            }
			$srcs =~ s/\//_/g;
			push(@{$clk_tree->{$resource}{_srcs}}, split(' ', $srcs));
		} elsif (/^\/\/PMDelay:(\d+)\/(\d+)us/) {
            $clk_tree->{$resource}{_delay} = $2;
        } elsif (/^\/\/PMPolarity:\s*(.*?)\s*$/) {
            $clk_tree->{$resource}{_polarity} = $1;
        } elsif (/^\/\/PMLogic:\s*(.*?)\s*$/) {
            $clk_tree->{$resource}{_div} = $1;
        } elsif (/^\/\/PMPState:\s*STB:(.*?)(\d+):(.*?)(\d+):(\d+)/) {
            $clk_tree->{$resource}{_pmap}{$2}{_pstate}{$4} = $5;
        } elsif (/^\/\/PMPowerProfile:\s*STB\s*:\s*(.*?)\s*:\s*PMap(\d+)/) {
            $profiles[$2] = $1;
        }
	}

    close(INFILE);
}

sub skip_node {
    my ($clk_tree, $src) = @_;

    if(($src eq "null") ||
       ($src =~ /PLL_(SYS|SCB).*_PLL/)){
        return 1;
    }

    if(!exists $clk_tree->{$src}) {
        print "$src does not exist in tree\n";
        return 1;
    }

    foreach my $func (@{$clk_tree->{$src}{_funcs}}){
        if(!is_nexus_function($func) && !$clk_tree->{$src}{_div}) {
            return 1;
        }
    }

    if($clk_tree->{$src}{_div}) {
        my $match = 0;
        foreach my $func (@{$clk_tree->{$src}{_funcs}}){
            if(is_div_function($func)) {
                $match = 1;
            }
        }
        if($match == 0) {
            return 1;
        }
    }

    return 0;
}

sub generate_top_level_nodes {
	my ($clk_tree, $functions) = @_;

	for (sort keys %$clk_tree) {
		if (@{$clk_tree->{$_}{_toplvl}}) {
			foreach my $func (@{$clk_tree->{$_}{_toplvl}}) {
                push(@{$functions->{$func}}, $_);
			}
		}
	}
}

sub print_clock_tree {
	my ($fh, $clk_tree, $sources, $level) = @_;

	foreach my $src (@$sources) {

        if($level > 0) {print $fh "|       ";}
        foreach (2..$level) {print $fh "        ";}
        print $fh "|_______$src\n";

        if(!grep(/null/, @{$clk_tree->{$src}{_srcs}})) {
            print_clock_tree($fh, $clk_tree, \@{$clk_tree->{$src}{_srcs}}, $level+1);
        }
	}
}

sub print_nexus_tree {
	my ($fh, $clk_tree, $sources, $level) = @_;

	foreach my $src (@$sources) {

        my $next_level = $level;
        if(!skip_node($clk_tree, $src)) {

            my $fncs = join(' ', @{$clk_tree->{$src}{_funcs}});

            # Print each sources
            if($level > 0) {print $fh "|       ";}
            foreach (2..$level) {print $fh "        ";}
            print $fh "|_______$src".($clk_tree->{$src}{_delay}?" ($clk_tree->{$src}{_delay})":"")."  [$fncs]\n";

            $next_level++;
        }
        print_nexus_tree($fh, $clk_tree, \@{$clk_tree->{$src}{_srcs}}, $next_level);
	}
}

sub print_tree {
	my ($clk_tree, $functions) = @_;

	open(my $nexus_tree_fh, ">nexus_tree.txt") or die "Can't open output file";
	open(my $clock_tree_fh, ">clock_tree.txt") or die "Can't open output file";


	foreach my $func (sort keys %$functions) {

        if(is_nexus_function($func)) {
            print $nexus_tree_fh "$func\n";
            print_nexus_tree($nexus_tree_fh, $clk_tree, \@{$functions->{$func}}, 0);
            print $nexus_tree_fh "\n\n";
        }

        print $clock_tree_fh "$func\n";
        print_clock_tree($clock_tree_fh, $clk_tree, \@{$functions->{$func}}, 0);
        print $clock_tree_fh "\n\n";
	}

    close($nexus_tree_fh);
	close($clock_tree_fh);
}

sub print_power_p_tree {
	my ($fh, $clk_tree, $sources, $level) = @_;

	foreach my $src (@$sources) {

        if($level > 0) {print $fh "|       ";}
        foreach (2..$level) {print $fh "        ";}
        print $fh "|_______$clk_tree->{$src}{_reg}:$clk_tree->{$src}{_field}";
        if($clk_tree->{$src}{_polarity}) {print $fh    ":$clk_tree->{$src}{_polarity}";}
        if($clk_tree->{$src}{_div}) {print $fh    ":$clk_tree->{$src}{_div}";}
        print $fh "\n";

        if(!grep(/null/, @{$clk_tree->{$src}{_srcs}})) {
            print_power_p_tree($fh, $clk_tree, \@{$clk_tree->{$src}{_srcs}}, $level+1);
        }
	}
}

sub print_power_tree {
	my ($clk_tree, $functions, $chip) = @_;
    my $file = "bpower_".$chip."_tree.txt";

	open(my $fh, ">$file") or die "Can't open output file";

	foreach my $func (sort keys %$functions) {
        if($func !~ /PDA/) {
            print $fh "$func\n";
            print_power_p_tree($fh, $clk_tree, \@{$functions->{$func}}, 0);
            print $fh "\n\n";
        }
	}

	close($fh);
    return 0;
}

sub rename_node {
    my $function = shift;

    $function =~ s/^HVD(\d*)$/AVD$1_CLK/g;
    $function =~ s/^HVD[a-zA-Z](\d*)$/AVD$1_CLK/g;
    $function =~ s/^HVD(\d*)_PDA(R|W)$/AVD$1_PWR/g;
    $function =~ s/^HVD[a-zA-Z](\d*)_PDA(R|W)$/AVD$1_PWR/g;
    $function =~ s/^HVD(\d*)_CPU$/AVD$1_CPU/g;
    $function =~ s/^HVD[a-zA-Z](\d*)_CPU$/AVD$1_CPU/g;
    $function =~ s/^VICE2(\d*)$/VICE$1_CLK/g;
    $function =~ s/^VICE2(\d*)_PDA(R|W)$/VICE$1_PWR/g;
    $function =~ s/^AIO(\d*)$/AIO$1_CLK/g;
    $function =~ s/^RAAGA(\d*)$/RAAGA$1_CLK/g;
    $function =~ s/^RAAGA(\d*)_CPU$/RAAGA$1_DSP/g;
    $function =~ s/^VDAC(\d*)$/VDC_DAC$1/g;
    $function =~ s/^VEC/VDC_VEC/g;
    $function =~ s/^ITU656(\d*)/VDC_656_OUT$1/g;
    $function =~ s/^XPT$/XPT_XMEMIF/g;
    $function =~ s/^DVPHT(\d*)/HDMI_TX$1/g;
    $function =~ s/^DVPHR(\d*)/HDMI_RX$1/g;
    $function =~ s/^HDMI_TX(\d*)$/HDMI_TX$1_CLK/g;
    $function =~ s/^HDMI_RX(\d*)$/HDMI_RX$1_CLK/g;
    $function =~ s/^V3D/GRAPHICS3D/g;
    $function =~ s/^SC(\d*)$/SMARTCARD$1/g;
    $function =~ s/^PAD$//g;
    $function =~ s/PDA(R|W)/SRAM/g;
    $function =~ s/^SECBUS$/SECURE_ACCESS/g;

    return $function;
}

sub generate_nodes {
    my ($clk_tree, $sources, $nodes, $hw_desc, $node, $hw_node, $recurse) = @_;
    my ($combine, $create_hw);

    $node = rename_node($node);
    if($node =~ /^\s*$/) {
        return;
    }

    if($hw_node eq "") {
        $create_hw = 1;
    } else {
        $create_hw = 0;
    }

    foreach my $src (@$sources) {
        $combine = 0;

        if(!skip_node($clk_tree, $src)) {
            my $reg = $clk_tree->{$src}{_reg};

            if($clk_tree->{$src}{_pda} ne "") {
                $reg = $clk_tree->{$src}{_reg}."_".$clk_tree->{$src}{_pda};
            }

            if($clk_tree->{$src}{_div}) {

                $hw_node = $node;
                $hw_node =~ s/^HW_//;
                $hw_node =~ s/DIS_//;
                $hw_node =~ s/CH_\d_//;
                $hw_node =~ s/CH_//;
                $hw_node =~ s/POST_//;
                $hw_node =~ s/DIV_//;
                $hw_node =~ s/HOLD_//;
                $hw_node = "DV_".$hw_node."_div";

                if(!exists $nodes->{$node}{_list}{$hw_node}) {
                    push(@{$nodes->{$node}{_order}}, $hw_node);
                }
                $nodes->{$node}{_list}{$hw_node}++;
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{$clk_tree->{$src}{_field}}) {
                    $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{$clk_tree->{$src}{_field}} = $clk_tree->{$src}{_default};
                }
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{_pmap}) {
                    if(exists $clk_tree->{$src}{_pmap}) {
                        $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_div}}{_pmap} = $clk_tree->{$src}{_pmap}
                    }
                }

                # Combine divisor nodes
                foreach (@{$clk_tree->{$src}{_srcs}}) {
                    if($clk_tree->{$_}{_div}) {
                        $combine = 1;
                        last;
                    }
                }
            } elsif($clk_tree->{$src}{_mux}) {
                $hw_node = $src;
                $hw_node =~ s/CLKGEN_//;
                $hw_node =~ s/STB_//;
                $hw_node =~ s/INST_//;
                $hw_node =~ s/CLOCK_//;
                $hw_node =~ s/TOP_//;
                $hw_node = "MX_".$hw_node;

                if(!exists $nodes->{$node}{_list}{$hw_node}) {
                    push(@{$nodes->{$node}{_order}}, $hw_node);
                }
                $nodes->{$node}{_list}{$hw_node}++;
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{_mux}{$clk_tree->{$src}{_field}}) {
                    $hw_desc->{$hw_node}{$reg}{_field}{_mux}{$clk_tree->{$src}{_field}} = $clk_tree->{$src}{_default};
                }
                if(!exists $hw_desc->{$hw_node}{$reg}{_field}{_mux}{_pmap}) {
                    if(exists $clk_tree->{$src}{_pmap}) {
                        $hw_desc->{$hw_node}{$reg}{_field}{_mux}{_pmap} = $clk_tree->{$src}{_pmap}
                    }
                }
            } else {
                if($create_hw) {
                    $hw_node = join(' ', @{$clk_tree->{$src}{_funcs}});
                    if(((!grep(/null/, @{$clk_tree->{$src}{_srcs}})) &&
                        (!grep(/PLL_(SYS|SCB).*_PLL/, @{$clk_tree->{$src}{_srcs}}))
                       ) ||
                       ($recurse > 0)
                      ) {
                            $hw_node = $clk_tree->{$src}{_name};
                    }
                    $hw_node =~ s/\s+$//g;
                    $hw_node =~ s/\s+/_/g;
                    $hw_node = "HW_".$hw_node;
                    $hw_node = rename_node($hw_node);
                }

                if(!exists $nodes->{$node}{_list}{$hw_node}) {
                    push(@{$nodes->{$node}{_order}}, $hw_node);
                }
                $nodes->{$node}{_list}{$hw_node}++;
                $hw_desc->{$hw_node}{$reg}{_field}{$clk_tree->{$src}{_polarity}}{$clk_tree->{$src}{_field}} = $clk_tree->{$src}{_default};

                # Combine consecutive sources with same register
                if(scalar(@{$clk_tree->{$src}{_srcs}}) == 1) {
                    foreach (@{$clk_tree->{$src}{_srcs}}) {
                        if($_ ne "null" && !$clk_tree->{$_}{_div} && !$clk_tree->{$_}{_mux}) {
                            if($reg eq $clk_tree->{$_}{_reg}) {
                                $combine = 1;
                                last;
                            }
                        }
                    }
                }
            }
            $hw_desc->{$hw_node}{$reg}{_delay} = $clk_tree->{$src}{_delay};
            $hw_desc->{$hw_node}{$reg}{_pda} = $clk_tree->{$src}{_pda};
        } else {
            $hw_node = $node;
        }
        if(!$combine) {
            generate_nodes($clk_tree, \@{$clk_tree->{$src}{_srcs}}, $nodes, $hw_desc, $hw_node, "", 1);
        } else {
            generate_nodes($clk_tree, \@{$clk_tree->{$src}{_srcs}}, $nodes, $hw_desc, $node, $hw_node, 1);
        }
	}
}

sub generate_nexus_nodes {
    my ($clk_tree, $functions, $nodes, $hw_desc) = @_;

    foreach my $func (sort keys %$functions) {
        if(is_nexus_function($func)) {
            generate_nodes($clk_tree, \@{$functions->{$func}}, $nodes, $hw_desc, $func, "", 0);
        }
    }
}

sub generate_user_defined_nodes {
    my $nodes = shift;

    foreach my $node (keys %$nodes) {
        if($node =~ /(HDMI_)(TX|RX)(\d*)_CLK/) {
            $nodes->{$1.$2.$3."_PHY"}{_list}{"HW_".$1.$2.$3."_PHY"}++;
            foreach my $hw_node (keys %{$nodes->{$node}{_list}}) { $nodes->{"HW_".$1.$2.$3."_PHY"}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node} }

            $nodes->{"BINT_OPEN"}{_list}{$1.$2.$3."_CLK"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1.$2.$3."_CLK"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1.$2.$3."_PHY"}++;

            if ($2 eq "TX") {
                $nodes->{"VDC_".$1.$2."_PHY".$3}{_list}{$1.$2.$3."_PHY"}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_".$1.$2."_PHY".$3}++;
            } elsif ($2 eq "RX") {
                $nodes->{"VDC_".$1.$2."_CLK".$3}{_list}{$1.$2.$3."_CLK"}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_".$1.$2."_CLK".$3}++;
            }
        } elsif($node =~ /^(RFM)$/) {
            $nodes->{$1."_PHY"}{_list}{"HW_".$1."_PHY"}++;
            foreach my $hw_node (keys %{$nodes->{$node}{_list}}) { $nodes->{"HW_".$1."_PHY"}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node} }

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1."_PHY"}++;
        } elsif($node =~ /AIO_CLK/) {
            $nodes->{"AUD_AIO"}{_list}{"$node"}++;
            $nodes->{"AUD_DAC"}{_list}{"HW_AUD_DAC"}++;
            $nodes->{"AUD_PLL0"}{_list}{"HW_AUD_PLL0"}++;  # TODO : Remove when fixed in rdb
            $nodes->{"AUD_PLL1"}{_list}{"HW_AUD_PLL1"}++;  # TODO : Remove when fixed in rdb
            foreach my $hw_node (keys %{$nodes->{$node}{_list}}) {
                if($hw_node !~ /VCXO/) {
                    $nodes->{"HW_AUD_DAC"}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node};
                    $nodes->{"HW_AUD_PLL0"}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node}; # TODO : Remove when fixed in rdb
                    $nodes->{"HW_AUD_PLL1"}{_list}{$hw_node} = $nodes->{$node}{_list}{$hw_node}; # TODO : Remove when fixed in rdb
                }
            }

            $nodes->{"BINT_OPEN"}{_list}{"AUD_AIO"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"AUD_AIO"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"AUD_DAC"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"AUD_PLL0"}++;  # TODO : Remove when fixed in rdb
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"AUD_PLL1"}++;  # TODO : Remove when fixed in rdb
        } elsif($node =~ /AIO_SRAM/) {
            $nodes->{"AUD_AIO"}{_list}{"$node"}++;
        } elsif($node =~ /^(AVD|VICE)(\d*)(.*?)$/) {
            if($3 eq "_CPU") {
                foreach my $clk_node (keys %{$nodes->{$1.$2."_CLK"}{_list}}) {
                    foreach my $cpu_node (keys %{$nodes->{$1.$2.$3}{_list}}) {
                        $nodes->{$cpu_node}{_list}{$clk_node}++;
                        push(@{$nodes->{$cpu_node}{_order}}, $clk_node);

                    }
                    delete($nodes->{$1.$2."_CLK"}{_list}{$clk_node});
                }
                undef(@{$nodes->{$1.$2."_CLK"}{_order}});

                foreach my $cpu_node (keys %{$nodes->{$1.$2.$3}{_list}}) {
                    $nodes->{$1.$2."_CLK"}{_list}{$cpu_node}++;
                    push(@{$nodes->{$1.$2."_CLK"}{_order}}, $cpu_node);
                }
                delete($nodes->{$1.$2.$3});
            } else {
                $nodes->{$1.$2}{_list}{$node}++;
                $nodes->{$1}{_list}{$1.$2}++;

                $nodes->{"BINT_OPEN"}{_list}{$1}++;
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
            }
        } elsif($node =~ /^(RAAGA)(\d*)(.*?)$/) {
            if($3 eq "_DSP") {
                foreach my $clk_node (keys %{$nodes->{$1.$2."_CLK"}{_list}}) {
                    foreach my $cpu_node (keys %{$nodes->{$1.$2.$3}{_list}}) {
                        $nodes->{$cpu_node}{_list}{$clk_node}++;
                        push(@{$nodes->{$cpu_node}{_order}}, $clk_node);

                    }
                }
            }
            $nodes->{$1}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
        } elsif($node =~ /^(SID|GRAPHICS3D)(_CPU)$/) {
            foreach my $clk_node (keys %{$nodes->{$1}{_list}}) {
                foreach my $cpu_node (keys %{$nodes->{$1.$2}{_list}}) {
                    $nodes->{$cpu_node}{_list}{$clk_node}++;
                    push(@{$nodes->{$cpu_node}{_order}}, $clk_node);

                }
                delete($nodes->{$1}{_list}{$clk_node});
            }
            undef(@{$nodes->{$1}{_order}});

            foreach my $cpu_node (keys %{$nodes->{$1.$2}{_list}}) {
                $nodes->{$1}{_list}{$cpu_node}++;
                push(@{$nodes->{$1}{_order}}, $cpu_node);
            }
            delete($nodes->{$1.$2});
        } elsif($node =~ /^(M2MC)(\d*)(.*?)$/) {
            $nodes->{$1.$3}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
        } elsif($node =~ /^(VICE)(\d*)(_CLK)$/) {
            $nodes->{"VDC_STG".($2*2)}{_list}{$node}++;
            $nodes->{"VDC_STG".($2*2+1)}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$node}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$node}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_STG".($2*2)}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC_STG".($2*2+1)}++;

        } elsif($node =~ /^(XPT)(_XMEMIF)$/) {
            $nodes->{$1}{_list}{$1."_PARSER"}++;
            $nodes->{$1}{_list}{$1."_PLAYBACK"}++;
            $nodes->{$1}{_list}{$1."_RAVE"}++;
            $nodes->{$1}{_list}{$1."_PACKETSUB"}++;
            $nodes->{$1."_PARSER"}{_list}{$node}++;
            $nodes->{$1."_PLAYBACK"}{_list}{$node}++;
            $nodes->{$1."_RAVE"}{_list}{$node}++;
            $nodes->{$1."_PACKETSUB"}{_list}{$node}++;
            $nodes->{"DMA"}{_list}{$node}++;
            $nodes->{"HSM"}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{$1}++;
        } elsif($node =~ /^(XPT)(_REMUX|_WAKEUP)$/) {
            $nodes->{$1}{_list}{$1.$2}++;
        } elsif($node =~ /^(BVN|VDC)(.*?)(DAC|VEC|656)?(.*?)/) {
            $nodes->{"VDC"}{_list}{$node}++;

            $nodes->{"BINT_OPEN"}{_list}{"VDC"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{_list}{"VDC"}++;
        } elsif($node =~ /^MX.*V3D/) {
            $nodes->{"GRAPHICS3D_PLL_CH"}{_list}{$node}++;
        } else {
            if($node !~ /^HW/ && $node !~ /^SECURE_ACCESS$/ && $node !~ /^DV_/ && $node !~ /^MX_/) {
                $nodes->{"MAGNUM_CONTROLLED"}{_list}{$node}++;
                if($node !~ /SRAM/ && $node !~ /PWR/ && $node !~ /PHY/ && $node !~ /DAC/) {
                    $nodes->{"BINT_OPEN"}{_list}{$node}++;
                }
            }
        }
    }

    #check for nodes with no order
    foreach my $node (keys %$nodes) {
        if(not defined @{$nodes->{$node}{_order}}) {
            foreach my $child_node (keys %{$nodes->{$node}{_list}}) {
                push (@{$nodes->{$node}{_order}}, $child_node);
            }
        }
    }
}

sub generate_brcm_copyright_header
{
    my @lines;

    push @lines, " /******************************************************************************\n";
    push @lines, " *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.\n";
    push @lines, " *\n";
    push @lines, " *  This program is the proprietary software of Broadcom and/or its licensors,\n";
    push @lines, " *  and may only be used, duplicated, modified or distributed pursuant to the terms and\n";
    push @lines, " *  conditions of a separate, written license agreement executed between you and Broadcom\n";
    push @lines, " *  (an ".'"Authorized License"'.").  Except as set forth in an Authorized License, Broadcom grants\n";
    push @lines, " *  no license (express or implied), right to use, or waiver of any kind with respect to the\n";
    push @lines, " *  Software, and Broadcom expressly reserves all rights in and to the Software and all\n";
    push @lines, " *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU\n";
    push @lines, " *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY\n";
    push @lines, " *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.\n";
    push @lines, " *\n";
    push @lines, " *  Except as expressly set forth in the Authorized License,\n";
    push @lines, " *\n";
    push @lines, " *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade\n";
    push @lines, " *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,\n";
    push @lines, " *  and to use this information only in connection with your use of Broadcom integrated circuit products.\n";
    push @lines, " *\n";
    push @lines, " *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED".' "AS IS"'."\n";
    push @lines, " *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR\n";
    push @lines, " *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO\n";
    push @lines, " *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES\n";
    push @lines, " *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,\n";
    push @lines, " *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION\n";
    push @lines, " *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF\n";
    push @lines, " *  USE OR PERFORMANCE OF THE SOFTWARE.\n";
    push @lines, " *\n";
    push @lines, " *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS\n";
    push @lines, " *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR\n";
    push @lines, " *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR\n";
    push @lines, " *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF\n";
    push @lines, " *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT\n";
    push @lines, " *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S.".' $1'.", WHICHEVER IS GREATER. THESE\n";
    push @lines, " *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF\n";
    push @lines, " *  ANY LIMITED REMEDY.\n";
    push @lines, "\n";
    push @lines, " ******************************************************************************/\n";
    push @lines, "\n";
    return @lines;
}

sub generate_bchp_resources_txt_file {
    my ($nodes, $chip) = @_;
    my @resource_list;
    my (@magnum_nodes, @bint_nodes);
    my $file = "bchp_pwr_resources_".$chip.".txt";


    open(my $fh, ">$file") or die "Can't open output file $file";

    foreach my $node (sort keys %$nodes) {
        undef @resource_list;
        foreach my $hw_node (@{$nodes->{$node}{_order}}) {
            push(@resource_list, $hw_node);
        }

        print $fh "$node -> {".join(", ", @resource_list)."}\n";
    }

    close($fh);
}

sub generate_bchp_impl_string {
    my ($hw_desc, $hw_node, $reg) = @_;
    my @lines;
    my $pdar = ($hw_desc->{$hw_node}{$reg}{_pda} eq "READ");
    my $vcxo_reset = ($reg =~ /VCXO\d*_PLL_RESET/)?1:0;
    my $div = ($hw_node =~ /^DV_.*_div/)?1:0;
    my $mux = ($hw_node =~ /^MX_/)?1:0;
    my $register = $reg;

    #Strip away PDA tag from register. This was done to seperate
    #write and read when both the bits are in the same register.
    #TODO : Find a better way to  seperate the wirte and read
    if ($hw_desc->{$hw_node}{$reg}{_pda} ne "") {
        $register =~ s/_$hw_desc->{$hw_node}{$reg}{_pda}//;
    }

    if($pdar) {
        push @lines, "    {\n";
        push @lines, "        uint32_t val=0, cnt=50;\n";
        push @lines, "        while(cnt--) {\n";
        push @lines, "            BKNI_Delay(10);\n";
        push @lines, "        ";
    }
    if($vcxo_reset) {
        push @lines, "    BSTD_UNUSED(reg);\n\n";
    } else {
        push @lines, "    reg = BREG_Read32(handle->regHandle, BCHP_".$register.");\n";
    }

    if ($div) {
        foreach my $divisor (keys %{$hw_desc->{$hw_node}{$reg}{_field}}) {
            foreach my $field (keys %{$hw_desc->{$hw_node}{$reg}{_field}{$divisor}}) {
                if($field eq "_pmap") {next;}
                if ($divisor eq "POSTDIV") {
                    push @lines, "    if(set) {\n";
                    push @lines, "        BCHP_SET_FIELD_DATA(reg, $register, $field, *postdiv);\n";
                    push @lines, "        BREG_Write32(handle->regHandle, BCHP_".$register.", reg);\n";
                    push @lines, "    } else {\n";
                    push @lines, "        *postdiv = BCHP_GET_FIELD_DATA(reg, $register, $field);\n";
                    push @lines, "    }\n";
                } else {
                    push @lines, "    if(!set) {\n";
                    if ($divisor eq "MULT") {
                        push @lines, "        *mult = BCHP_GET_FIELD_DATA(reg, $register, $field);\n";
                    } elsif ($divisor eq "PREDIV") {
                        push @lines, "        *prediv = BCHP_GET_FIELD_DATA(reg, $register, $field);\n";
                    } else {
                        print "Unknown Div field\n";
                        next;
                    }
                    push @lines, "    }\n";
                }
            }
        }
    } elsif ($mux) {
        foreach my $field (keys %{$hw_desc->{$hw_node}{$reg}{_field}{_mux}}) {
            if($field eq "_pmap") {next;}
            push @lines, "    if(set) {\n";
            push @lines, "        BCHP_SET_FIELD_DATA(reg, $register, $field, *mux);\n";
            push @lines, "        BREG_Write32(handle->regHandle, BCHP_".$register.", reg);\n";
            push @lines, "    } else {\n";
            push @lines, "        *mux = BCHP_GET_FIELD_DATA(reg, $register, $field);\n";
            push @lines, "    }\n";
        }
    } else {
        foreach my $polarity (keys %{$hw_desc->{$hw_node}{$reg}{_field}}) {
            my $count = keys %{$hw_desc->{$hw_node}{$reg}{_field}{$polarity}};
            my $idx = 0;
            if($pdar) {push @lines, "        ";}

            push @lines, "    mask = ";
            if($count > 1) {push @lines, "(";}
            foreach my $field (keys %{$hw_desc->{$hw_node}{$reg}{_field}{$polarity}}) {
                if($idx > 0) {push @lines, "            ";}
                push @lines, "BCHP_".$register."_".$field."_MASK";
                $idx++;
                if($idx < $count) {push @lines, " |\n";}
            }
            if($count > 1) {push @lines, ")";}
            push @lines, ";\n";

            if($pdar) {
                push @lines, "            reg &= mask;\n";
                if($polarity eq "HighIsOn") {
                    push @lines, "            val |= activate?mask:0;\n";
                } elsif($polarity eq "HighIsOff") {
                    push @lines, "            val |= activate?0:mask;\n";
                }
            } elsif($vcxo_reset) {
                push @lines, "    BREG_AtomicUpdate32(handle->regHandle, BCHP_".$register.", mask, ";
                if($polarity eq "HighIsOn") {
                    push @lines, "activate?mask:0);\n";
                } elsif($polarity eq "HighIsOff") {
                    push @lines, "activate?0:mask);\n";
                }
            } else {
                push @lines, "    reg &= ~mask;\n";
                if($polarity eq "HighIsOn") {
                    push @lines, "    reg |= activate?mask:0;\n";
                } elsif($polarity eq "HighIsOff") {
                    push @lines, "    reg |= activate?0:mask;\n";
                }
            }
        }
    }
    if($pdar) {
        push @lines, "            if (val == reg)\n";
        push @lines, "                break;\n";
        push @lines, "        }\n";
        push @lines, "        if(!cnt)\n";
        push @lines, "            BDBG_ERR((\"".$hw_node." Timeout\"));\n";
        push @lines, "    }\n";
    } elsif(!$vcxo_reset && !$div && !$mux) {
        push @lines, "    BREG_Write32(handle->regHandle, BCHP_".$register.", reg);\n";
    }
    if($hw_desc->{$hw_node}{$reg}{_delay} > 0) {
        push @lines, "    if(activate)\n";
        push @lines, "        BKNI_Delay(".$hw_desc->{$hw_node}{$reg}{_delay}.");\n";
    }

    return @lines;
}

sub generate_bchp_impl_c_file {
    my $hw_desc = shift;

    open(my $fh, ">bchp_pwr_impl.c") or die "Can't open output file bchp_pwr_impl.c";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file maps the power resource control to register writes.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#include \"bchp.h\"\n";
    print $fh "#include \"bchp_priv.h\"\n";
    print $fh "#include \"bdbg.h\"\n";
    print $fh "#include \"bkni.h\"\n";
    print $fh "#include \"bchp_clkgen.h\"\n";
    print $fh "#include \"bchp_avs_top_ctrl.h\"\n";
    print $fh "#include \"bchp_sun_top_ctrl.h\"\n";
    print $fh "\n";
    print $fh "BDBG_MODULE(BCHP_PWR_IMPL);\n";

    foreach my $hw_node (sort keys %$hw_desc) {
        my ($pdar_done,$pdaw_done) = (0,0);
        my $tmp_reg;
        print $fh "\n";
        if($hw_node =~ /^DV_.*_div/) {
            print $fh "static void BCHP_PWR_P_".$hw_node."_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)\n";
            print $fh "{\n";
            print $fh "    uint32_t reg;\n";
            print $fh "\n";
            print $fh "    BDBG_MSG((\"".$hw_node.": %s\", set?\"write\":\"read\"));\n";
            print $fh "\n";
        } elsif ($hw_node =~ /^MX_/) {
            print $fh "static void BCHP_PWR_P_".$hw_node."_Control(BCHP_Handle handle, unsigned *mux, bool set)\n";
            print $fh "{\n";
            print $fh "    uint32_t reg;\n";
            print $fh "\n";
            print $fh "    BDBG_MSG((\"".$hw_node.": %s\", set?\"write\":\"read\"));\n";
            print $fh "\n";
        } elsif ($hw_node =~ /^HW_/) {
            print $fh "static void BCHP_PWR_P_".$hw_node."_Control(BCHP_Handle handle, bool activate)\n";
            print $fh "{\n";
            print $fh "    uint32_t mask, reg;\n";
            print $fh "\n";
            print $fh "    BDBG_MSG((\"".$hw_node.": %s\", activate?\"on\":\"off\"));\n";
            print $fh "\n";
        } else {
            print "Unknown node $hw_node\n";
            next;
        }
        foreach my $reg (sort keys %{$hw_desc->{$hw_node}}) {
            if($hw_desc->{$hw_node}{$reg}{_pda} eq "READ") {
                if(!$pdaw_done) {
                    $tmp_reg = $reg;
                    next;
                } else {
                    $pdar_done=1;
                }
            }
            if($hw_desc->{$hw_node}{$reg}{_pda} eq "WRITE") {$pdaw_done=1;}

            @lines = generate_bchp_impl_string($hw_desc, $hw_node, $reg);
            print $fh @lines;

            if($pdaw_done && !$pdar_done) {
                $pdar_done=1;
                if($tmp_reg) {
                    @lines = generate_bchp_impl_string($hw_desc, $hw_node, $tmp_reg);
                    print $fh @lines;
                } else {
                    print "PDAR not found : $hw_node\n";
                }
                $pdaw_done=0;
                $pdar_done=0;
            }
        }
        printf $fh "}\n";
    }

    close($fh);
}

sub generate_bchp_resources_priv_h_file {
    my ($nonleafs, $nonleafshw, $leafs, $nonleafsdv, $leafsdv, $nonleafsmx, $leafsmx) = @_;
    my @nonleafshw = keys %$nonleafshw;
    my @leafs = keys %$leafs;
    my @hw_all = (@nonleafshw, @leafs);
    my @nonleafsdv = keys %$nonleafsdv;
    my @leafsdv = keys %$leafsdv;
    my @DV_all = (@nonleafsdv, @leafsdv);
    my @nonleafsmx = keys %$nonleafsmx;
    my @leafsmx = keys %$leafsmx;
    my @MX_all = (@nonleafsmx, @leafsmx);

    open(my $fh, ">bchp_pwr_resources_priv.h") or die "Can't open output file bchp_pwr_resources_priv.h";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file contains a list of private power resource IDs that\n";
    print $fh "* represent HW clocks, and function prototypes for controlling\n";
    print $fh "* them.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#ifndef BCHP_PWR_RESOURCES_PRIV_H__\n";
    print $fh "#define BCHP_PWR_RESOURCES_PRIV_H__\n";
    print $fh "\n";
    print $fh "#include \"bchp_pwr.h\"\n";
    print $fh "\n";
    print $fh "/* Private power resource IDs */\n";

    # get the length of the longest node string
    my $maxlen = 0;
    foreach (@hw_all, @MX_all, @DV_all) {
        if (length > $maxlen) {
            $maxlen = length;
        }
    }
    $maxlen += length("BCHP_PWR_")+3;

    my $idx = 1;
    foreach (sort @hw_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }
    foreach (sort @MX_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }
    foreach (sort @DV_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }

    print $fh "\n";
    print $fh "/* This is the link between the public and private interface */\n";
    print $fh "void BCHP_PWR_P_HW_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool activate);\n";
    print $fh "void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate);\n";
    print $fh "void BCHP_PWR_P_MUX_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mux, bool set);\n";
    print $fh "void BCHP_PWR_P_DIV_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set);\n";
    print $fh "\n";

    print $fh "#define BCHP_PWR_P_NUM_NONLEAFS   ", scalar(keys %$nonleafs), "\n";
    print $fh "#define BCHP_PWR_P_NUM_NONLEAFSHW ", scalar(keys %$nonleafshw), "\n";
    print $fh "#define BCHP_PWR_P_NUM_LEAFS      ", scalar(keys %$leafs), "\n";
    print $fh "#define BCHP_PWR_P_NUM_MUXES      ", scalar(@MX_all), "\n";
    print $fh "#define BCHP_PWR_P_NUM_DIVS       ", scalar(@DV_all), "\n";
    print $fh "#define BCHP_PWR_P_NUM_ALLNODES   ", scalar(keys %$nonleafs)+scalar(keys %$leafs)+scalar(keys %$nonleafshw)+scalar(@MX_all)+scalar(@DV_all), "\n";
    print $fh "#define BCHP_PWR_NUM_P_STATES     ", scalar(@profiles)+1, "\n";
    print $fh "\n";

    print $fh "#endif\n";

    close($fh);
}

sub generate_bchp_resources_h_file {
    my $nonleafs = shift;

    open(my $fh, ">bchp_pwr_resources.h") or die "Can't open output file bchp_pwr_resources.h";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file contains a list of power resource IDs that can be\n";
    print $fh "* acquired and released.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#ifndef BCHP_PWR_RESOURCES_H__\n";
    print $fh "#define BCHP_PWR_RESOURCES_H__\n";
    print $fh "\n";

    # get the length of the longest node string
    my $maxlen = 0;
    foreach (keys %$nonleafs) {
        if (length > $maxlen) {
            $maxlen = length;
        }
    }
    $maxlen += length("BCHP_PWR_RESOURCE_")+3;

    my $idx = 1;
    foreach (sort keys %$nonleafs) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_RESOURCE_$_", $idx++);
    }

    print $fh "\n";
    print $fh "#endif\n";

    close $fh;
}

sub generate_resource_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $type = shift;
    my $elements = shift;
    my @lines;

    foreach (@$elements) {
        push @lines, "const $prefix1 ${prefix1}_${_}[] = {{\n";
        push @lines, "    ${prefix1}Type_e$type,\n";
        push @lines, "    ${prefix2}_${_},\n";
        push @lines, "    BDBG_STRING(\"$_\")\n";
        push @lines, "}};\n\n";
    }

    return @lines;
}

sub generate_array_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $prefix3 = shift;
    my $prefix4 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "const $prefix1 ${prefix2}[${prefix3}] = {\n";
    foreach (@$nodes) {
        if ($_ eq "NULL") {
            push @lines, "    NULL,\n";
        } else {
            push @lines, "    ${prefix4}$_,\n";
        }
    }
    push @lines, "};\n\n";

    return @lines;
}

sub generate_dependency_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $graph = shift;
    my $nodes = shift;
    my @lines;

    foreach (@$nodes) {
        push @lines, "static const ${prefix1}* const $prefix2${_}[] = {\n";
        foreach my $dep (@{$graph->{$_}{_order}}) {
            push @lines, "    ${prefix1}_${dep},\n";
        }
        push @lines, "    NULL\n";
        push @lines, "};\n\n";
    }

    return @lines;
}

sub generate_hwcontrol_function_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "#include \"bchp_pwr_impl.c\"\n";
    push @lines, "#include \"bchp_pwr_common.c\"\n";
    push @lines, "\n";
    push @lines, "void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate)\n";
    push @lines, "{\n";
    push @lines, "    switch(id) {\n";
    foreach (@$nodes) {
        push @lines, "        case ${prefix1}_${_}:\n";
        push @lines, "            ${prefix2}_${_}_Control(handle, activate);\n";
        push @lines, "            break;\n";
    }
    push @lines, "        default:\n";
    push @lines, "            BDBG_ASSERT(0);\n";
    push @lines, "            break;\n";
    push @lines, "    }\n";
    push @lines, "}\n\n";
    push @lines, "void BCHP_PWR_P_HW_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool activate)\n";
    push @lines, "{\n";
    push @lines, "    BCHP_PWR_P_HW_ControlId(handle, resource->id, activate);\n";
    push @lines, "}\n";

    return @lines;
}

sub generate_mxcontrol_function_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "\n";
    push @lines, "void BCHP_PWR_P_MUX_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mux, bool set)\n";
    push @lines, "{\n";
    if(scalar(@$nodes) == 0) {
        push @lines, "    BSTD_UNUSED(handle);\n";
        push @lines, "    BSTD_UNUSED(mux);\n";
        push @lines, "    BSTD_UNUSED(set);\n";
    }
    push @lines, "\n";
    push @lines, "    switch(resource->id) {\n";
    foreach (@$nodes) {
        push @lines, "        case ${prefix1}_${_}:\n";
        push @lines, "            ${prefix2}_${_}_Control(handle, mux, set);\n";
        push @lines, "            break;\n";
    }
    push @lines, "        default:\n";
    push @lines, "            BDBG_ASSERT(0);\n";
    push @lines, "            break;\n";
    push @lines, "    }\n";
    push @lines, "}\n";

    return @lines;
}

sub generate_dvcontrol_function_string
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $nodes = shift;
    my @lines;

    push @lines, "\n";
    push @lines, "void BCHP_PWR_P_DIV_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)\n";
    push @lines, "{\n";
    if(scalar(@$nodes) == 0) {
        push @lines, "    BSTD_UNUSED(handle);\n";
        push @lines, "    BSTD_UNUSED(mult);\n";
        push @lines, "    BSTD_UNUSED(prediv);\n";
        push @lines, "    BSTD_UNUSED(postdiv);\n";
        push @lines, "    BSTD_UNUSED(set);\n";
    }
    push @lines, "\n";
    push @lines, "    switch(resource->id) {\n";
    foreach (@$nodes) {
        push @lines, "        case ${prefix1}_${_}:\n";
        push @lines, "            ${prefix2}_${_}_Control(handle, mult, prediv, postdiv, set);\n";
        push @lines, "            break;\n";
    }
    push @lines, "        default:\n";
    push @lines, "            BDBG_ASSERT(0);\n";
    push @lines, "            break;\n";
    push @lines, "    }\n";
    push @lines, "}\n";

    return @lines;
}

sub generate_dv_table
{
    my $prefix1 = shift;
    my $prefix2 = shift;
    my $prefix3 = shift;
    my $nodes = shift;
    my $hw_desc = shift;
    my (@lines1, @lines2, @lines);
    my ($mult, $prediv, $postdiv);
    my %div_table;
    my $num_profiles = scalar @profiles;
    push @lines, "\n";
    push @lines2, "const ${prefix3} ${prefix3}List[BCHP_PWR_P_NUM_DIVS] = {\n";

    foreach (sort @$nodes) {
        my $postdiv = 1;
        my $prediv = 1 ;
        my $mult = 1;
        foreach my $reg (sort keys %{$hw_desc->{$_}}) {
            foreach my $type (keys %{$hw_desc->{$_}{$reg}{_field}}) {
                foreach my $field (keys %{$hw_desc->{$_}{$reg}{_field}{$type}}) {
                    if($field eq "_pmap") {next;}
                    if(exists $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}) {
                        my $entries = scalar keys %{$hw_desc->{$_}{$reg}{_field}{$type}{_pmap}};
                        if($num_profiles < $entries) {
                            print "Mismatch between PMPowerProfile tag and number of profiles profiles found for $reg:$type\n";
                        }
                        foreach my $profile (0..$num_profiles) {
                            if(exists $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}{$profile}) {
                                push(@{$div_table{$_}{$type}}, $hw_desc->{$_}{$reg}{_field}{$type}{_pmap}{$profile}{_pstate}{0});
                            } else {
                                if($profile == $num_profiles) {
                                    push(@{$div_table{$_}{$type}}, $hw_desc->{$_}{$reg}{_field}{$type}{$field});
                                } else {
                                    push(@{$div_table{$_}{$type}}, 0);
                                }
                            }
                        }
                    } else {
                        #print "no PMAP found for $reg:$type\n";
                        if ($type eq "MULT") {
                            $mult = 0;
                        } elsif ($type eq "PREDIV") {
                            $prediv = 0;
                        } elsif ($type eq "POSTDIV") {
                            $postdiv = 0;
                        }
                        foreach my $profile (0..$num_profiles) {
                            push(@{$div_table{$_}{$type}}, $hw_desc->{$_}{$reg}{_field}{$type}{$field});
                        }
                    }
                }
            }
        }
        if(exists $div_table{$_}{MULT}) {
            my $len= scalar(@{$div_table{$_}{MULT}});
            if($num_profiles>$len+1) {
                printf "Missing entries for $:MULT\n";
                next;
            }
        } else {
            printf "$_:MULT does not exist\n";
            next;
        }
        if(exists $div_table{$_}{PREDIV}) {
            my $len= scalar(@{$div_table{$_}{PREDIV}});
            if($num_profiles>$len+1) {
                printf "Missing entries for $:PREDIV\n";
                next;
            }
        } else {
            printf "$_:PREDIV does not exist\n";
            next;
        }
        if(exists $div_table{$_}{POSTDIV}) {
            my $len= scalar(@{$div_table{$_}{POSTDIV}});
            if($num_profiles>$len+1) {
                printf "Missing entries for $_:POSTDIV\n";
                next;
            }
        } else {
            printf "$_:POSTDIV does not exist\n";
            next;
        }
        if ($prediv == 0 && $mult == 0 && $postdiv == 0) {
            print "No pmap for $_\n";
            delete $div_table{$_};
        }

        if (exists $div_table{$_}{MULT} && exists $div_table{$_}{PREDIV} && exists $div_table{$_}{POSTDIV}) {
            push @lines1, "const ${prefix2} ${prefix2}_${_}[] = {";
            foreach my $profile (0..$num_profiles) {
                push @lines1, "{$div_table{$_}{MULT}[$profile],$div_table{$_}{PREDIV}[$profile],$div_table{$_}{POSTDIV}[$profile]},";
            }
            push @lines1, "};\n";
            push @lines2, "    {${prefix1}_${_}, ${prefix2}_${_}},\n";
        }
    }
    push @lines2, "};\n";

    push @lines, @lines1;
    push @lines, "\n";
    push @lines, @lines2;

    #print Dumper (%div_table);

    return @lines;
}

sub generate_bchp_resources_c_file {
    my ($nonleafs, $nonleafshw, $leafs, $nonleafsdv, $leafsdv, $nonleafsmx, $leafsmx, $nodes, $hw_desc) = @_;
    my @nonleafs = sort keys %$nonleafs;
    my @nonleafshw = sort keys %$nonleafshw;
    my @leafs = sort keys %$leafs;
    my @hw_all = (@nonleafshw, @leafs);
    my @nonleafsdv = sort keys %$nonleafsdv;
    my @leafsdv = sort keys %$leafsdv;
    my @DV_all = (@nonleafsdv, @leafsdv);
    my @nonleafsmx = sort keys %$nonleafsmx;
    my @leafsmx = sort keys %$leafsmx;
    my @MX_all = (@nonleafsmx, @leafsmx);
    @hw_all = sort @hw_all;
    @DV_all = sort @DV_all;
    @MX_all = sort @MX_all;

    open(my $fh, ">bchp_pwr_resources.c") or die "Can't open output file bchp_pwr_resources.c";

    my @lines = generate_brcm_copyright_header();
    print $fh @lines;

    print $fh "/***************************************************************************\n";
    print $fh "*\n";
    print $fh "* This file is auto-generated\n";
    print $fh "*\n";
    print $fh "* This file contains a coded representation of power resource\n";
    print $fh "* nodes and their dependencies.\n";
    print $fh "*\n";
    print $fh "***************************************************************************/\n";
    print $fh "\n";
    print $fh "#include \"bchp_pwr.h\"\n";
    print $fh "#include \"bchp_pwr_resources.h\"\n";
    print $fh "#include \"bchp_pwr_resources_priv.h\"\n";
    print $fh "\n";

    print $fh "/* Resource definitions */\n";
    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR_RESOURCE", "NonLeaf", \@nonleafs);
    print $fh @lines;

    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "Leaf", \@leafs);
    print $fh @lines;

    # NonLeafHw nodes are tacked-on at the end
    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "NonLeafHw", \@nonleafshw);
    print $fh @lines;

    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "Mux", \@MX_all);
    print $fh @lines;

    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "Div", \@DV_all);
    print $fh @lines;

    my @resources;
    push @resources, @nonleafs;
    push @resources, @hw_all;
    push @resources, @MX_all;
    push @resources, @DV_all;
    print $fh "/* List of resources */\n";
    @lines = generate_array_string("BCHP_PWR_P_Resource* const", "BCHP_PWR_P_ResourceList",
        "BCHP_PWR_P_NUM_ALLNODES", "BCHP_PWR_P_Resource_", \@resources);
    print $fh @lines;

    print $fh "/* Coded dependencies */\n";
    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafs);
    print $fh @lines;

    # NonLeafHw nodes are tacked-on at the end
    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafshw);
    print $fh @lines;

    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafsmx);
    print $fh @lines;

    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafsdv);
    print $fh @lines;

    foreach (@resources) {
        if (!exists $nodes->{$_}) {
            $_ = "NULL";
        }
    }

    print $fh "/* List of coded dependencies */\n";
    @lines = generate_array_string("BCHP_PWR_P_Resource* const * const", "BCHP_PWR_P_DependList",
        "BCHP_PWR_P_NUM_ALLNODES", "BCHP_PWR_P_Depend_", \@resources);
    print $fh @lines;

    @lines = generate_hwcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@hw_all);
    print $fh @lines;

    @lines = generate_mxcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@MX_all);
    print $fh @lines;

    @lines = generate_dvcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@DV_all);
    print $fh @lines;

    @lines = generate_dv_table("BCHP_PWR", "BCHP_PWR_P_DivTable", "BCHP_PWR_P_FreqMap", \@DV_all, $hw_desc);
    print $fh @lines;

    close $fh;
}

sub generate_bchp_files {
    my ($nodes, $hw_desc, $chp) = @_;
    my (%nonleafs, %nonleafshw, %leafs, %nonleafsdv, %leafsdv, %nonleafsmx, %leafsmx);

    foreach my $node (keys %$nodes) {
        if($node !~ /^(HW|DV|MX)_/) {
            $nonleafs{$node}++;
        } elsif ($node =~ /^HW_/) {
            $nonleafshw{$node}++;
        } elsif ($node =~ /^DV_/) {
            $nonleafsdv{$node}++;
        } elsif ($node =~ /^MX_/) {
            $nonleafsmx{$node}++;
        }
        foreach my $child_node (@{$nodes->{$node}{_order}}) {
            if($child_node !~ /^(HW|DV|MX)_/) {
                $nonleafs{$child_node}++;
                if(!exists $nodes->{$child_node}) {
                    print "Error : Non leaf node $child_node has no child nodes\n";
                }
            } elsif ($child_node =~ /^HW_/) {
                if(exists $nodes->{$child_node}) {
                    $nonleafshw{$child_node}++;
                } else {
                    $leafs{$child_node}++;
                }
            } elsif ($child_node =~ /^DV_/) {
                if(exists $nodes->{$child_node}) {
                    $nonleafsdv{$child_node}++;
                } else {
                    $leafsdv{$child_node}++;
                }
            } elsif ($child_node =~ /^MX_/) {
                if(exists $nodes->{$child_node}) {
                    $nonleafsmx{$child_node}++;
                } else {
                    print "Error : Leaf mux node found $child_node\n";
                    $leafsmx{$child_node}++;
                }
            }
        }
    }

    generate_bchp_resources_txt_file($nodes, $chp);
    generate_bchp_impl_c_file($hw_desc);
    generate_bchp_resources_priv_h_file(\%nonleafs, \%nonleafshw, \%leafs, \%nonleafsdv, \%leafsdv, \%nonleafsmx, \%leafsmx);
    generate_bchp_resources_h_file(\%nonleafs);
    generate_bchp_resources_c_file(\%nonleafs, \%nonleafshw, \%leafs, \%nonleafsdv, \%leafsdv, \%nonleafsmx, \%leafsmx, $nodes, $hw_desc);
}

sub main {
	my (%clk_tree, %functions, %nodes, %hw_desc);
    my ($chp, $ver, $dir, $perf, $dbg);
    my @files;

    foreach (@ARGV) {
        if(/^\d\d\d\d$/) {
            $chp = $_;
        }
        if(/^[a-zA-Z]\d$/) {
            $ver = $_;
        }
        if(/\/projects\//) {
            $dir = $_;
        }
        if(/perf/) {
            $perf = 1;
        }
        if(/debug/) {
            $dbg = 1;
        }
    }

    if(not defined $chp) {
        print "Please specify chip number";
        return;
    }
    if(not defined $ver) {
        print "Please specify chip version";
        return;
    }
    if(not defined $dir) {
        $dir = "/projects/BCM".$chp."/".$ver."/snapshot";
    }

    print "\n\n";
    print "Processing clock tree\n";
    print "Chip    : $chp\n";
    print "Version : $ver\n";
    print "Using RDB $dir\n";
    if( defined $perf) {
        print "\nGenerating files for performance tools only\n"
    }
    print"\n\n";

    my $clkgen_dir = $dir."/design/clkgen/rdb/*.rdb" if -e $dir."/design/clkgen/rdb/clkgen.rdb";
    my $avstop_dir = $dir."/design/avs_top/rdb/avs_top_ctrl.rdb" if -e $dir."/design/avs_top/rdb/avs_top_ctrl.rdb";
    my $sysctrl_dir = $dir."/design/sys_ctrl/rdb/sram_pda.rdb" if -e $dir."/design/sys_ctrl/rdb/sram_pda.rdb";

    push(@files, glob("$clkgen_dir")) if ($clkgen_dir);
    push(@files, glob("$avstop_dir")) if ($avstop_dir);
    push(@files, glob("$sysctrl_dir")) if ($sysctrl_dir);

	parse_rdb_file($_, \%clk_tree)  foreach (@files);

	verify_tree(\%clk_tree);

	generate_top_level_nodes(\%clk_tree, \%functions);

    if(not defined $perf) {
        generate_nexus_nodes(\%clk_tree, \%functions, \%nodes, \%hw_desc);

        generate_user_defined_nodes(\%nodes);

        generate_bchp_files(\%nodes, \%hw_desc, $chp);

        if(defined $dbg) {
            print_tree(\%clk_tree, \%functions);
        }
    } else {
        print_power_tree(\%clk_tree, \%functions, $chp);
    }

    #print Dumper (%clk_tree);
    #print Dumper (%functions);
    #print Dumper (%nodes);
    #print Dumper (%hw_desc);
    #print Dumper (@profiles);
}

exit main();
