#!/usr/bin/perl
#############################################################################
#
#     Copyright (c) 2006-2014, Broadcom Corporation*
#     All Rights Reserved*
#     Confidential Property of Broadcom Corporation*
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################

use strict;
use warnings;

use Data::Dumper;
$Data::Dumper::Sortkeys  = 1;
$Data::Dumper::Useqq    = 1;
$Data::Dumper::Indent   = 1;
$Data::Dumper::Terse    = 1;

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
				print "$clk_tree->{$_}{_srcs} not found\n";
			}
		}
	}
}

sub get_name {
	my ($reg, $field) = @_;
	my $name = $reg."_".$field;


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
    $func =~ s/\bDVPHRLP\b/DVPHR/g;
    $func =~ s/\bDVPHR\b/DVPHR0/g;
    $func =~ s/\bXPTWAKEUP\b/XPT_WAKEUP/g;
    $func =~ s/\bAIFSAT\b/AIFSAT0/g;
    $func =~ s/OBSERVE//g;

    return $func;
}

sub parse_rdb_file {
	my ($file, $clk_tree) = @_;
	my ($reg, $field, $resource, $default);
    my $coreprefix;

    open(INFILE,"$file") or die "Can't open input file $file";

    if($file =~ /\/design\/(.*?)\/rdb\//) {
        $coreprefix = uc($1);
    }

    foreach (<INFILE>) {
        s/\r\n/\n/; # convert DOS to Unix
		s/^\s+//;   # Remove whitespace at beginning of line
		s/\s+$//;   # Remove whitespace at end of line
		s/"//g;

        if (/^coreprefix\s*(\w+)/) {
            $coreprefix = $1;
        } elsif (/^regtype32\s*(\w+)/) {
			$reg = $1;
			$reg =~ s/Type/$coreprefix/;
		} elsif (/^field\s*(\w+)/) {
			$field = $1;
			$resource = $reg."_".$field;
		} elsif (/^default\s*(\d+)/) {
            $default = $1;
		} elsif (/\/\/PMFunction:\s*(.*?)\s*$/) {
            my $func = rename_function($resource, $1);
			push(@{$clk_tree->{$resource}{_funcs}}, split(' ', $func));
			push(@{$clk_tree->{$resource}{_toplvl}}, split(' ', $func));
            $clk_tree->{$resource}{_reg} = $reg;
            $clk_tree->{$resource}{_field} = $field;
            $clk_tree->{$resource}{_default} = $default;
			$clk_tree->{$resource}{_name} = get_name($reg, $field);
            $clk_tree->{$resource}{_delay} = 0;
            $clk_tree->{$resource}{_pda} = get_pda($1);
            $clk_tree->{$resource}{_polarity} = "";
            $clk_tree->{$resource}{_div} = "";
		} elsif (/\/\/PMSource(\d*):\s*(.*?)\s*$/) {
            my $indx = $1;
			my $srcs = $2;
            if($indx =~ /\d+/) {
                $clk_tree->{$resource}{_mux}++;
            }
			$srcs =~ s/\//_/g;
			push(@{$clk_tree->{$resource}{_srcs}}, split(' ', $srcs));
		} elsif (/\/\/PMDelay:(\d+)\/(\d+)us/) {
            $clk_tree->{$resource}{_delay} = $2;
        } elsif (/\/\/PMPolarity:\s*(.*?)\s*$/) {
            $clk_tree->{$resource}{_polarity} = $1;
        } elsif (/\/\/PMLogic:\s*(.*?)\s*$/) {
            $clk_tree->{$resource}{_div} = $1;
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
    $function =~ s/^HVD(\d*)_PDA(R|W)$/AVD$1_PWR/g;
    $function =~ s/^VICE2(\d*)$/VICE$1_CLK/g;
    $function =~ s/^VICE2(\d*)_PDA(R|W)$/VICE$1_PWR/g;
    $function =~ s/^AIO(\d*)$/AIO$1_CLK/g;
    $function =~ s/^RAAGA(\d*)$/RAAGA$1_CLK/g;
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
            if($clk_tree->{$src}{_div}) {

                $hw_node = $node;
                $hw_node =~ s/^HW_//;
                $hw_node =~ s/DIS_//;
                $hw_node =~ s/CH_\d_//;
                $hw_node =~ s/CH_//;
                $hw_node = "DV_".$hw_node."_div";

                $nodes->{$node}{$hw_node}++;
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_mask}{""}{""} = "";
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_delay} = $clk_tree->{$src}{_delay};
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_pda} = $clk_tree->{$src}{_pda};
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_div}{$clk_tree->{$src}{_div}}{$src} = $clk_tree->{$src}{_default};

                # Combine divisor nodes
                foreach (@{$clk_tree->{$src}{_srcs}}) {
                    if($clk_tree->{$_}{_div}) {
                        $combine = 1;
                        last;
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

                $nodes->{$node}{$hw_node}++;
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_mask}{$clk_tree->{$src}{_polarity}}{$src} = $clk_tree->{$src}{_default};
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_delay} = $clk_tree->{$src}{_delay};
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_pda} = $clk_tree->{$src}{_pda};
                $hw_desc->{$hw_node}{$clk_tree->{$src}{_reg}}{_div}{""}{""} = "";

                # Combine consecutive sources with same register
                foreach (@{$clk_tree->{$src}{_srcs}}) {
                    if($_ ne "null" && !$clk_tree->{$_}{_div}) {
                        if($clk_tree->{$src}{_reg} eq $clk_tree->{$_}{_reg}) {
                            $combine = 1;
                            last;
                        }
                    }
                }
            }
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
            $nodes->{$1.$2.$3."_PHY"}{"HW_".$1.$2.$3."_PHY"}++;
            foreach my $hw_node (keys %{$nodes->{$node}}) { $nodes->{"HW_".$1.$2.$3."_PHY"}{$hw_node} = $nodes->{$node}{$hw_node} }

            $nodes->{"BINT_OPEN"}{$1.$2.$3."_CLK"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$1.$2.$3."_CLK"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$1.$2.$3."_PHY"}++;

            if ($2 eq "TX") {
                $nodes->{"VDC_".$1.$2."_PHY".$3}{$1.$2.$3."_PHY"}++;
                $nodes->{"MAGNUM_CONTROLLED"}{"VDC_".$1.$2."_PHY".$3}++;
            } elsif ($2 eq "RX") {
                $nodes->{"VDC_".$1.$2."_CLK".$3}{$1.$2.$3."_CLK"}++;
                $nodes->{"MAGNUM_CONTROLLED"}{"VDC_".$1.$2."_CLK".$3}++;
            }

        } elsif($node =~ /^(RFM)$/) {
            $nodes->{$1."_PHY"}{"HW_".$1."_PHY"}++;
            foreach my $hw_node (keys %{$nodes->{$node}}) { $nodes->{"HW_".$1."_PHY"}{$hw_node} = $nodes->{$node}{$hw_node} }

            $nodes->{"BINT_OPEN"}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$1."_PHY"}++;
        } elsif($node =~ /AIO_CLK/) {
            $nodes->{"AUD_AIO"}{"$node"}++;
            $nodes->{"AUD_DAC"}{"HW_AUD_DAC"}++;
            $nodes->{"AUD_PLL0"}{"HW_AUD_PLL0"}++;  # TODO : Remove when fixed in rdb
            $nodes->{"AUD_PLL1"}{"HW_AUD_PLL1"}++;  # TODO : Remove when fixed in rdb
            foreach my $hw_node (keys %{$nodes->{$node}}) {
                if($hw_node !~ /VCXO/) {
                    $nodes->{"HW_AUD_DAC"}{$hw_node} = $nodes->{$node}{$hw_node};
                    $nodes->{"HW_AUD_PLL0"}{$hw_node} = $nodes->{$node}{$hw_node}; # TODO : Remove when fixed in rdb
                    $nodes->{"HW_AUD_PLL1"}{$hw_node} = $nodes->{$node}{$hw_node}; # TODO : Remove when fixed in rdb
                }
            }

            $nodes->{"BINT_OPEN"}{"AUD_AIO"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{"AUD_AIO"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{"AUD_DAC"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{"AUD_PLL0"}++;  # TODO : Remove when fixed in rdb
            $nodes->{"MAGNUM_CONTROLLED"}{"AUD_PLL1"}++;  # TODO : Remove when fixed in rdb
        } elsif($node =~ /AIO_SRAM/) {
            $nodes->{"AUD_AIO"}{"$node"}++;
        } elsif($node =~ /^(AVD)(\d*)(.*?)$/) {
            $nodes->{$1.$2}{$node}++;
            $nodes->{$1}{$1.$2}++;

            $nodes->{"BINT_OPEN"}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$1}++;
        } elsif($node =~ /^(M2MC)(\d*)(.*?)$/) {
            $nodes->{$1.$3}{$node}++;

            $nodes->{"BINT_OPEN"}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$1}++;
        } elsif($node =~ /^(VICE)(\d*)(_CLK)$/) {
            $nodes->{"VDC_STG".($2*2)}{$node}++;
            $nodes->{"VDC_STG".($2*2+1)}{$node}++;

            $nodes->{"BINT_OPEN"}{$node}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$node}++;
            $nodes->{"MAGNUM_CONTROLLED"}{"VDC_STG".($2*2)}++;
            $nodes->{"MAGNUM_CONTROLLED"}{"VDC_STG".($2*2+1)}++;

        } elsif($node =~ /^(XPT)(_XMEMIF)$/) {
            $nodes->{$1}{$1."_PARSER"}++;
            $nodes->{$1}{$1."_PLAYBACK"}++;
            $nodes->{$1}{$1."_RAVE"}++;
            $nodes->{$1}{$1."_PACKETSUB"}++;
            $nodes->{$1."_PARSER"}{$node}++;
            $nodes->{$1."_PLAYBACK"}{$node}++;
            $nodes->{$1."_RAVE"}{$node}++;
            $nodes->{$1."_PACKETSUB"}{$node}++;
            $nodes->{"DMA"}{$node}++;
            $nodes->{"HSM"}{$node}++;

            $nodes->{"BINT_OPEN"}{$1}++;
            $nodes->{"MAGNUM_CONTROLLED"}{$1}++;
        } elsif($node =~ /^(XPT)(_REMUX|_WAKEUP)$/) {
            $nodes->{$1}{$1.$2}++;
        } elsif($node =~ /^(BVN|VDC)(.*?)(DAC|VEC|656)?(.*?)/) {
            $nodes->{"VDC"}{$node}++;

            $nodes->{"BINT_OPEN"}{"VDC"}++;
            $nodes->{"MAGNUM_CONTROLLED"}{"VDC"}++;
        } else {
            if($node !~ /^HW/ && $node !~ /^SECURE_ACCESS$/ && $node !~ /^DV_/) {
                $nodes->{"MAGNUM_CONTROLLED"}{$node}++;
                if($node !~ /SRAM/ && $node !~ /PWR/ && $node !~ /PHY/ && $node !~ /DAC/) {
                    $nodes->{"BINT_OPEN"}{$node}++;
                }
            }
        }
    }

}

sub generate_brcm_copyright_header
{
    my @lines;

    push @lines, "/***************************************************************************\n";
    push @lines, "*     Copyright (c) 2006-2014, Broadcom Corporation*\n";
    push @lines, "*     All Rights Reserved*\n";
    push @lines, "*     Confidential Property of Broadcom Corporation*\n";
    push @lines, "*\n";
    push @lines, "*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE\n";
    push @lines, "*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR\n";
    push @lines, "*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.\n";
    push @lines, "*\n";
    push @lines, "* \$brcm_Workfile: \$\n";
    push @lines, "* \$brcm_Revision: \$\n";
    push @lines, "* \$brcm_Date: \$\n";
    push @lines, "*\n";
    push @lines, "* Module Description:\n";
    push @lines, "*\n";
    push @lines, "* Revision History:\n";
    push @lines, "*\n";
    push @lines, "* \$brcm_Log: \$\n";
    push @lines, "*\n";
    push @lines, "***************************************************************************/\n";
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
        foreach my $hw_node (sort keys %{$nodes->{$node}}) {
            push(@resource_list, $hw_node);
        }

        print $fh "$node -> {".join(", ", @resource_list)."}\n";
    }

    close($fh);
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
    print $fh "\n";
    print $fh "BDBG_MODULE(BCHP_PWR_IMPL);\n";
    print $fh "\n";

    foreach my $hw_node (sort keys %$hw_desc) {
        if($hw_node =~ /^DV_.*_div/) {
            print $fh "static void BCHP_PWR_P_".$hw_node."_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)\n";
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
            my $pda_read = ($hw_desc->{$hw_node}{$reg}{_pda} eq "READ")?1:0;
            my $vcxo_reset = ($reg =~ /VCXO\d*_PLL_RESET/)?1:0;

            if($pda_read) {
                print $fh "    {\n";
                print $fh "        uint32_t val=0, cnt=50;\n";
                print $fh "        while(cnt--) {\n";
                print $fh "            BKNI_Delay(10);\n";
                print $fh "        ";
            }
            if($vcxo_reset) {
                print $fh "    BSTD_UNUSED(reg);\n\n";
            } else {
                print $fh "    reg = BREG_Read32(handle->regHandle, BCHP_".$reg.");\n";
            }

            if (exists $hw_desc->{$hw_node}{$reg}{_div}{POSTDIV} ||
                exists $hw_desc->{$hw_node}{$reg}{_div}{MULT} ||
                exists $hw_desc->{$hw_node}{$reg}{_div}{PREDIV}) {
                foreach my $type (keys %{$hw_desc->{$hw_node}{$reg}{_div}}) {
                    foreach my $mask (keys %{$hw_desc->{$hw_node}{$reg}{_div}{$type}}) {
                        my $field = $mask;
                        $field =~ s/$reg//;
                        $field =~ s/^_//;
                        if ($type eq "POSTDIV") {
                            print $fh "    if(set) {\n";
                            print $fh "        BCHP_SET_FIELD_DATA(reg, $reg, $field, *postdiv);\n";
                            print $fh "        BREG_Write32(handle->regHandle, BCHP_".$reg.", reg);\n";
                            print $fh "    } else {\n";
                            print $fh "        *postdiv = BCHP_GET_FIELD_DATA(reg, $reg, $field);\n";
                            print $fh "    }\n";
                        } else {
                            print $fh "    if(!set) {\n";
                            if ($type eq "MULT") {
                                print $fh "        *mult = BCHP_GET_FIELD_DATA(reg, $reg, $field);\n";
                            } elsif ($type eq "PREDIV") {
                                print $fh "        *prediv = BCHP_GET_FIELD_DATA(reg, $reg, $field);\n";
                            } else {
                                print "Unknown Div field\n";
                                next;
                            }
                            print $fh "    }\n";
                        }
                    }
                }
            } else {
                foreach my $polarity (keys %{$hw_desc->{$hw_node}{$reg}{_mask}}) {
                    my $count = keys %{$hw_desc->{$hw_node}{$reg}{_mask}{$polarity}};
                    my $idx = 0;
                    if($pda_read) {print $fh "        ";}

                    print $fh "    mask = ";
                    if($count > 1) {print $fh "(";}
                    foreach my $mask (keys %{$hw_desc->{$hw_node}{$reg}{_mask}{$polarity}}) {
                        if($idx > 0) {print $fh "            ";}
                        print $fh "BCHP_".$mask."_MASK";
                        $idx++;
                        if($idx < $count) {print $fh " |\n";}
                    }
                    if($count > 1) {print $fh ")";}
                    print $fh ";\n";

                    if($pda_read) {
                        print $fh "            reg &= mask;\n";
                        if($polarity eq "HighIsOn") {
                            print $fh "            val |= activate?mask:0;\n";
                        } elsif($polarity eq "HighIsOff") {
                            print $fh "            val |= activate?0:mask;\n";
                        }
                    } elsif($vcxo_reset) {
                        print $fh "    BREG_AtomicUpdate32(handle->regHandle, BCHP_".$reg.", mask, ";
                        if($polarity eq "HighIsOn") {
                            print $fh "activate?mask:0);\n";
                        } elsif($polarity eq "HighIsOff") {
                            print $fh "activate?0:mask);\n";
                        }
                    } else {
                        print $fh "    reg &= ~mask;\n";
                        if($polarity eq "HighIsOn") {
                            print $fh "    reg |= activate?mask:0;\n";
                        } elsif($polarity eq "HighIsOff") {
                            print $fh "    reg |= activate?0:mask;\n";
                        }
                    }
                }
            }
            if($pda_read) {
                print $fh "            if (val == reg)\n";
                print $fh "                break;\n";
                print $fh "        }\n";
                print $fh "        if(!cnt)\n";
                print $fh "            BDBG_ERR((\"".$hw_node." Timeout\"));\n";
                print $fh "    }\n";
            } elsif(!$vcxo_reset && $hw_desc->{$hw_node}{$reg}{_div} eq "") {
                print $fh "    BREG_Write32(handle->regHandle, BCHP_".$reg.", reg);\n";
            }
            print $fh "\n";

            if($hw_desc->{$hw_node}{$reg}{_delay} > 0) {
                print $fh "    BKNI_Delay(".$hw_desc->{$hw_node}{$reg}{_delay}.");\n";
            }
        }
        printf $fh "}\n";
        print $fh "\n";
    }

    close($fh);
}

sub generate_bchp_resources_priv_h_file {
    my ($nonleafs, $nonleafshw, $leafs, $nonleafsdv, $leafsdv) = @_;
    my @nonleafshw = keys %$nonleafshw;
    my @leafs = keys %$leafs;
    my @hw_all = (@nonleafshw, @leafs);
    my @nonleafsdv = keys %$nonleafsdv;
    my @leafsdv = keys %$leafsdv;
    my @DV_all = (@nonleafsdv, @leafsdv);

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
    foreach (@hw_all, @DV_all) {
        if (length > $maxlen) {
            $maxlen = length;
        }
    }
    $maxlen += length("BCHP_PWR_")+3;

    my $idx = 1;
    foreach (sort @hw_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }
    foreach (@DV_all) {
        printf $fh ("#define %-${maxlen}s 0x%08x\n", "BCHP_PWR_$_", 0xff000000+$idx++);
    }

    print $fh "\n";
    print $fh "/* This is the link between the public and private interface */\n";
    print $fh "void BCHP_PWR_P_HW_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool activate);\n";
    print $fh "void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate);\n";
    print $fh "void BCHP_PWR_P_DIV_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set);\n";
    print $fh "\n";

    print $fh "#define BCHP_PWR_P_NUM_NONLEAFS   ", scalar(keys %$nonleafs), "\n";
    print $fh "#define BCHP_PWR_P_NUM_NONLEAFSHW ", scalar(keys %$nonleafshw), "\n";
    print $fh "#define BCHP_PWR_P_NUM_LEAFS      ", scalar(keys %$leafs), "\n";
    print $fh "#define BCHP_PWR_P_NUM_DIVS       ", scalar(@DV_all), "\n";
    print $fh "#define BCHP_PWR_P_NUM_ALLNODES   ", scalar(keys %$nonleafs)+scalar(keys %$leafs)+scalar(keys %$nonleafshw)+scalar(@DV_all), "\n";
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
        push @lines, "static const ${prefix1}* $prefix2${_}[] = {\n";
        foreach my $dep (sort keys %{$graph->{$_}}) {
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

    push @lines, "\n";
    push @lines2, "const ${prefix3} ${prefix3}List[BCHP_PWR_P_NUM_DIVS] = {\n";

    foreach (sort @$nodes) {
        foreach my $reg (sort keys %{$hw_desc->{$_}}) {
            foreach my $type (keys %{$hw_desc->{$_}{$reg}{_div}}) {
                foreach my $mask (keys %{$hw_desc->{$_}{$reg}{_div}{$type}}) {
                    if ($type eq "POSTDIV") {
                        $postdiv = $hw_desc->{$_}{$reg}{_div}{$type}{$mask};
                    } elsif ($type eq "MULT") {
                        $mult = $hw_desc->{$_}{$reg}{_div}{$type}{$mask};
                    } elsif ($type eq "PREDIV") {
                        $prediv = $hw_desc->{$_}{$reg}{_div}{$type}{$mask};
                    }
                }
            }
        }
        push @lines1, "const ${prefix2} ${prefix2}_${_}[] = {{${mult},${prediv},${postdiv}},{${mult},${prediv},".($postdiv *= 16)."}};\n";
        push @lines2, "    {${prefix1}_${_}, ${prefix2}_${_}},\n";
    }
    push @lines2, "};\n";

    push @lines, @lines1;
    push @lines, "\n";
    push @lines, @lines2;

    return @lines;
}

sub generate_bchp_resources_c_file {
    my ($nonleafs, $nonleafshw, $leafs, $nonleafsdv, $leafsdv, $nodes, $hw_desc) = @_;
    my @nonleafs = sort keys %$nonleafs;
    my @nonleafshw = sort keys %$nonleafshw;
    my @leafs = sort keys %$leafs;
    my @hw_all = (@nonleafshw, @leafs);
    my @nonleafsdv = keys %$nonleafsdv;
    my @leafsdv = keys %$leafsdv;
    my @DV_all = (@nonleafsdv, @leafsdv);
    @hw_all = sort @hw_all;
    @DV_all = sort @DV_all;

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

    @lines = generate_resource_string("BCHP_PWR_P_Resource", "BCHP_PWR", "Div", \@DV_all);
    print $fh @lines;

    my @resources;
    push @resources, @nonleafs;
    push @resources, @hw_all;
    push @resources, @DV_all;
    print $fh "/* List of resources */\n";
    @lines = generate_array_string("BCHP_PWR_P_Resource*", "BCHP_PWR_P_ResourceList",
        "BCHP_PWR_P_NUM_ALLNODES", "BCHP_PWR_P_Resource_", \@resources);
    print $fh @lines;

    print $fh "/* Coded dependencies */\n";
    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafs);
    print $fh @lines;

    # NonLeafHw nodes are tacked-on at the end
    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafshw);
    print $fh @lines;

    @lines = generate_dependency_string("BCHP_PWR_P_Resource", "BCHP_PWR_P_Depend_", $nodes, \@nonleafsdv);
    print $fh @lines;

    foreach (@resources) {
        if (!exists $nodes->{$_}) {
            $_ = "NULL";
        }
    }

    print $fh "/* List of coded dependencies */\n";
    @lines = generate_array_string("BCHP_PWR_P_Resource**", "BCHP_PWR_P_DependList",
        "BCHP_PWR_P_NUM_ALLNODES", "BCHP_PWR_P_Depend_", \@resources);
    print $fh @lines;

    @lines = generate_hwcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@hw_all);
    print $fh @lines;

    @lines = generate_dvcontrol_function_string("BCHP_PWR", "BCHP_PWR_P", \@DV_all);
    print $fh @lines;

    @lines = generate_dv_table("BCHP_PWR", "BCHP_PWR_P_DivTable", "BCHP_PWR_P_FreqMap", \@DV_all, $hw_desc);
    print $fh @lines;

    close $fh;
}

sub generate_bchp_files {
    my ($nodes, $hw_desc, $chp) = @_;
    my (%nonleafs, %nonleafshw, %leafs, %nonleafsdv, %leafsdv);

    foreach my $node (keys %$nodes) {
        if($node !~ /^(HW|DV)_/) {
            $nonleafs{$node}++;
        } elsif ($node =~ /^HW_/) {
            $nonleafshw{$node}++;
        } elsif ($node =~ /^DV_/) {
            $nonleafsdv{$node}++;
        }
        foreach my $child_node (keys %{$nodes->{$node}}) {
            if($child_node !~ /^(HW|DV)_/) {
                $nonleafs{$child_node}++;
                if(!exists $nodes->{$child_node}) {
                    print "Non leaf node $child_node has no child nodes\n";
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
            }
        }
    }

    generate_bchp_resources_txt_file($nodes, $chp);
    generate_bchp_impl_c_file($hw_desc);
    generate_bchp_resources_priv_h_file(\%nonleafs, \%nonleafshw, \%leafs, \%nonleafsdv, \%leafsdv);
    generate_bchp_resources_h_file(\%nonleafs);
    generate_bchp_resources_c_file(\%nonleafs, \%nonleafshw, \%leafs, \%nonleafsdv, \%leafsdv, $nodes, $hw_desc);
}

sub main {
	my (%clk_tree, %functions, %nodes, %hw_desc);
    my ($chp, $ver, $dir, $perf, $dbg);

    foreach (@ARGV) {
        if(/^\d\d\d\d$/) {
            $chp = $_;
        }
        if(/^[a-zA-Z]\d$/) {
            $ver = $_;
        }
        if(/\/snapshot/) {
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

    my $clk_dir = $dir."/design/clkgen/rdb/*.rdb";
    my $pda_dir = $dir."/design/avs_top/rdb/*.rdb";
    my @files = glob("$clk_dir $pda_dir");

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
}

exit main();
