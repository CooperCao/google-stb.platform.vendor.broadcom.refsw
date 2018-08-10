##############################################################################
#  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
##############################################################################


#!/usr/bin/perl
use strict;
use warnings;


if (!$ENV{'OUT_DIR'}) {die("Cannot find output directory. Please set OUT_DIR.\n");}
if (!$ENV{'BCHP_ID'}) {die("Cannot find Chip ID. Please set BCHP_ID.\n");}
if (!$ENV{'BCHP_VER'}) {die("Cannot find Chip Revision. Please set BCHP_VER.\n");}
if (!$ENV{'REFSW_VERSION'}) {die("Cannot find Reference Software Version. Please set REFSW_VERSION.\n");}

#my $SoftwareVersion = float($ENV{"REFSW_VERSION"});
my $SoftwareVersion = $ENV{"REFSW_VERSION"};
print 'Reference Software Version = '.$SoftwareVersion."\n";

my $PrivateKey=$ENV{"PRV_KEY"};
my $PublicKey=$ENV{"PUB_KEY"};
my $SSPublicKey=$ENV{"SS_PUB_KEY"};
my $OutputDir = $ENV{"OUT_DIR"};
my $ChipId = $ENV{"BCHP_ID"};
my $ChipVer = $ENV{"BCHP_VER"};
my $NumChipId = int($ChipId);
my $MarketId = $ENV{"MARKET_ID"};
my $MarketIdMask = $ENV{"MARKET_ID_MASK"};
my $EpochSel = $ENV{"EPOCH_SEL"};
my $Epoch = $ENV{"EPOCH"};
my $EpochMask = $ENV{"EPOCH_MASK"};
my $SigType = $ENV{"SIG_TYPE"};
my $SigVer = $ENV{"SIG_VER"};

my $SidEpoch = GetEpoch("SID_EPOCH");
my $SidMarketId = GetMarketId("SID_MARKET_ID");
my $SidMarketIdMask = GetMarketIdMask("SID_MARKET_ID_MASK");
my $SidMarketIdSel = GetMarketIdSel("SID_MARKET_ID_SEL");
my $SidSigVer = GetSigVer("SID_SIG_VER");

my $RaveEpoch = GetEpoch("RAVE_EPOCH");
my $RaveFwEpoch = GetEpoch("RAVE_FW_EPOCH");
my $RaveMarketId = GetMarketId("RAVE_MARKET_ID");
my $RaveMarketIdMask = GetMarketIdMask("RAVE_MARKET_ID_MASK");
my $RaveMarketIdSel = GetMarketIdSel("RAVE_MARKET_ID_SEL");
my $RaveSigVer = GetSigVer("RAVE_SIG_VER");

my $RaagaEpoch = GetEpoch("RAAGA_EPOCH");
my $RaagaFwEpoch = GetEpoch("RAAGA_FW_EPOCH");
my $RaagaMarketId = GetMarketId("RAAGA_MARKET_ID");
my $RaagaMarketIdMask = GetMarketIdMask("RAAGA_MARKET_ID_MASK");
my $RaagaMarketIdSel = GetMarketIdSel("RAAGA_MARKET_ID_SEL");
my $RaagaSigVer = GetSigVer("RAAGA_SIG_VER");

my $AvdEpoch = GetEpoch("AVD_EPOCH");
my $AvdFwEpoch = GetEpoch("AVD_FW_EPOCH");
my $AvdMarketId = GetMarketId("AVD_MARKET_ID");
my $AvdMarketIdMask = GetMarketIdMask("AVD_MARKET_ID_MASK");
my $AvdMarketIdSel = GetMarketIdSel("AVD_MARKET_ID_SEL");
my $AvdSigVer = GetSigVer("AVD_SIG_VER");

my $VceEpoch = GetEpoch("VCE_EPOCH");
my $VceFwEpoch = GetEpoch("VCE_FW_EPOCH");
my $VceMarketId = GetMarketId("VCE_MARKET_ID");
my $VceMarketIdMask = GetMarketIdMask("VCE_MARKET_ID_MASK");
my $VceMarketIdSel = GetMarketIdSel("VCE_MARKET_ID_SEL");
my $VceSigVer = GetSigVer("VCE_SIG_VER");

my $SidBin = $ENV{"SID_BIN"};
my $RaagaBin = $ENV{"RAAGA_BIN"};
my $RaveBin = $ENV{"RAVE_BIN"};
my $VcePicBin = $ENV{"VCE_PIC_BIN"};
my $VceMbBin = $ENV{"VCE_MB_BIN"};
my $AvdOlBin = $ENV{"AVD_OL_BIN"};
my $AvdIlBin = $ENV{"AVD_IL_BIN"};
my $BoltBin = $ENV{"BOLT_BIN"};


my $SidPreBin = $OutputDir.'/'.$ChipId.'_region_0x11.bin';
my $SidSigBin = $OutputDir.'/'.$ChipId.'_region_0x11.bin.signature';
my $SidSigBeBin = $OutputDir.'/'.$ChipId.'_region_0x11.bin.signature.be';
my $SidSigParamBin = $OutputDir.'/'.$ChipId.'_region_0x11_signature_param.bin';

my $RaagaPreBin = $OutputDir.'/'.$ChipId.'_region_0x9.bin';
my $RaagaSigBin = $OutputDir.'/'.$ChipId.'_region_0x9.bin.signature';
my $RaagaSigBeBin = $OutputDir.'/'.$ChipId.'_region_0x9.bin.signature.be';
my $RaagaSigParamBin = $OutputDir.'/'.$ChipId.'_region_0x9_signature_param.bin';

my $RavePreBin = $OutputDir.'/'.$ChipId.'_region_0x8.bin';
my $RaveSigBin = $OutputDir.'/'.$ChipId.'_region_0x8.bin.signature';
my $RaveSigBeBin = $OutputDir.'/'.$ChipId.'_region_0x8.bin.signature.be';
my $RaveSigParamBin = $OutputDir.'/'.$ChipId.'_region_0x8_signature_param.bin';

my $VcePicPreBin = $OutputDir.'/'.$ChipId.'_region_0xF.bin';
my $VcePicSigBin = $OutputDir.'/'.$ChipId.'_region_0xF.bin.signature';
my $VcePicSigBeBin = $OutputDir.'/'.$ChipId.'_region_0xF.bin.signature.be';
my $VcePicSigParamBin = $OutputDir.'/'.$ChipId.'_region_0xF_signature_param.bin';

my $VceMbPreBin = $OutputDir.'/'.$ChipId.'_region_0x10.bin';
my $VceMbSigBin = $OutputDir.'/'.$ChipId.'_region_0x10.bin.signature';
my $VceMbSigBeBin = $OutputDir.'/'.$ChipId.'_region_0x10.bin.signature.be';
my $VceMbSigParamBin = $OutputDir.'/'.$ChipId.'_region_0x10_signature_param.bin';

my $AvdOlPreBin = $OutputDir.'/'.$ChipId.'_region_0xD.bin';
my $AvdOlSigBin = $OutputDir.'/'.$ChipId.'_region_0xD.bin.signature';
my $AvdOlSigBeBin = $OutputDir.'/'.$ChipId.'_region_0xD.bin.signature.be';
my $AvdOlSigParamBin = $OutputDir.'/'.$ChipId.'_region_0xD_signature_param.bin';

my $AvdIlPreBin = $OutputDir.'/'.$ChipId.'_region_0xC.bin';
my $AvdIlSigBin = $OutputDir.'/'.$ChipId.'_region_0xC.bin.signature';
my $AvdIlSigBeBin = $OutputDir.'/'.$ChipId.'_region_0xC.bin.signature.be';
my $AvdIlSigParamBin = $OutputDir.'/'.$ChipId.'_region_0xC_signature_param.bin';

if (scalar(@ARGV) < 1) {
    print "Insufficient Arguments to 'sign_command.py'\n";
    exit(1);
}

my $OpType = $ARGV[0];

if ($SoftwareVersion > 15.3) {
    if ($OpType eq "fw") {
        GenerateSignatures();
	} elsif ($OpType eq "key") {
        GenerateKeyParams($PublicKey, "KEY2", 0);
	} elsif ($OpType eq "ss_key") {
        GenerateKeyParams($SSPublicKey, "KEY2", 0);
	} elsif ($OpType eq "pre_process") {
        GeneratePreProcess();
	} elsif ($OpType eq "post_process") {
        GeneratePostProcess();
	} elsif ($OpType eq "extract_avs_memsys") {
        #ExtractAvsMemsys();
        #GenerateKeyParams($SSPublicKey, "AVS_KEY2", 1)
	} elsif ($OpType eq "assemble_avs_memsys") {
        #AssembleAvs();
        #AssembleMemsys();
	} else {
        print "Unsupported Argument : ".$OpType."\n";
        exit(1);
	}
} else {
    if ($OpType eq "fw") {
        GenerateLegacySignatures();
	} elsif ($OpType eq "key") {
        GenerateKeyParams($PublicKey, "KEY2", 0);
    } elsif ($OpType eq "ss_key") {
        GenerateKeyParams($SSPublicKey, "KEY2", 0);
    } elsif ($OpType eq "pre_process") {
        #GenerateLegacyPreProcess();
    } elsif ($OpType eq "post_process") {
        #GenerateLegacyPostProcess();
    } else {
        print "Unsupported Argument : "+$OpType+"\n";
        exit(1);
	}
}

sub GetSigningRight {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
	} else {
        return "0x0";
	}
}

sub GetKeyExp {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
	} else {
        return "0x1";
    }
}

sub GetEpoch {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
	} else {
        return $ENV{"EPOCH"};
	}
}

sub GetEpochMask {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
	} else {
        return $ENV{"EPOCH_MASK"};
	}
}

sub GetEpochSel {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
	} else {
        return $ENV{"EPOCH_SEL"};
	}
}

sub GetMarketIdSel {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
    } else {
        return $ENV{"MARKET_ID_SEL"};
	}
}

sub GetMarketId {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
    } else {
        return $ENV{"MARKET_ID"};
	}
}

sub GetMarketIdMask {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
    } else {
        return $ENV{"MARKET_ID_MASK"};
	}
}

sub GetSigType {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
    } else {
        return $ENV{"SIG_TYPE"};
	}
}

sub GetSigVer {
    my $Engine = $_[0];

    if ($ENV{$Engine}) {
        return $ENV{$Engine};
    } else {
        return $ENV{"SIG_VER"};
	}
}

sub GenerateKeyParams {
    my ($Key, $ParamPrefix, $SkipArray) = @_;
    my $Key2Cmd = $ENV{"KEY2_COMMAND_FILE"};
    my $Key2SigCmd = $ENV{"KEY2_SIG_COMMAND_FILE"};
    my $Key2Bin = $OutputDir.'/'.$ENV{"KEY2_BIN"};
    my $Key2Src = $OutputDir.'/'.$ENV{"KEY2_FILE_NAME"};
	my $KeyCommandFile;
    open($KeyCommandFile, ">", $Key2Cmd);

    my $Key2Right = GetSigningRight($ParamPrefix."_SIGNING_RIGHT");
    my $Key2Exp = GetKeyExp($ParamPrefix."_EXPONENT");
    my $Key2MarketId = GetMarketId($ParamPrefix."_MARKET_ID");
    my $Key2MarketIdMask = GetMarketIdMask($ParamPrefix."_MARKET_ID_MASK");
    my $Key2MarketIdSel = GetMarketIdSel($ParamPrefix."_MARKET_ID_SEL");
    my $Key2EpochSel = GetEpochSel($ParamPrefix."_EPOCH_SEL");
    my $Key2Epoch = GetEpoch($ParamPrefix."_EPOCH");
    my $Key2EpochMask = GetEpochMask($ParamPrefix."_EPOCH_MASK");
    my $Key2SigType = GetSigType($ParamPrefix."_SIG_TYPE");
    my $Key2SigVer = GetSigVer($ParamPrefix."_SIG_VER");

	my $Line;
    $Line="load_pub_key -in=".$Key."\n";
    print $KeyCommandFile $Line;

    $Line='add_param_key -right='.$Key2Right.' -exp='.$Key2Exp.' -mid='.$Key2MarketId.' -mid_mask='.$Key2MarketIdMask.' -mid_sel='.$Key2MarketIdSel.' -epo_sel='.$Key2EpochSel.' -epo='.$Key2Epoch.' -epo_mask='.$Key2EpochMask.' -sig_type='.$Key2SigType.' -sig_ver='.$Key2SigVer.' -out='.$Key2Bin."\n";
    print $KeyCommandFile $Line;

    close $KeyCommandFile;

    open($KeyCommandFile, ">", $Key2SigCmd);
    $Line='add_signature -in_key='.$Key2Bin.' -in_signature='.$Key2Bin.'.signature.be -out_endian=le -out='.$Key2Bin.".signed_le\n";
    print $KeyCommandFile $Line;

    if ($SkipArray == 0) {
        if ($SoftwareVersion > 15.3) {
            $Line='c_array -in='.$Key2Bin.'.signed_le -var=gRegionVerificationKey2 -out='.$Key2Src."\n"
        } else {
            $Line='c_array -in='.$Key2Bin.'.signed_le -var=gKey2 -out='.$Key2Src."\n"
		}
        print $KeyCommandFile $Line;
    }
    close $KeyCommandFile;
}

sub GenerateLegacySignatures {
    my $SignCommandFile;
	open($SignCommandFile, ">",$OutputDir."/".$ChipId."_Legacy.in");

    my $Line='load_prv_key -in='.$PrivateKey."\n";
    print $SignCommandFile $Line;

    $Line='load_pub_key -in='.$PublicKey."\n";
    print $SignCommandFile $Line;

#Processing of SID image
    if ($NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635) {
        $Line='add_param_bin -in='.$SidBin.' -in_endian=le -cpu_type=sid -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -epo_sel='.$EpochSel.' -epo='.$SidEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$SidPreBin."\n";
        print $SignCommandFile $Line;
        $Line='sign -in='.$SidPreBin.' -out_endian=le -out_type=binary -out='.$SidSigBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process_sid -in='.$SidSigBin.' -out='.$OutputDir."/nexus_sid_firmware_signature.c\n";
        print $SignCommandFile $Line;
    }

#Processing of AUDIO image
    $Line='add_param_bin -in='.$RaagaBin.' -in_endian=le -cpu_type=raaga -mid='.$MarketId.' -fw_epo='.$RaagaFwEpoch.' -mid_mask='.$MarketIdMask.' -epo_sel='.$EpochSel.' -epo='.$RaagaEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RaagaPreBin."\n";
    print $SignCommandFile $Line;
    $Line='sign -in='.$RaagaPreBin.' -out_endian=le -out_type=binary -out='.$RaagaSigBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process_audio -in='.$RaagaSigBin.' -out='.$OutputDir."/nexus_audio_firmware_signature.c\n";
    print $SignCommandFile $Line;

#Processing of RAVE image
    $Line='add_param_bin -in='.$RaveBin.' -in_endian=be -cpu_type=rave -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$RaveFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaveEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RavePreBin."\n";
    print $SignCommandFile $Line;
    $Line='sign -in='.$RavePreBin.' -out_endian=le -out_type=binary -out='.$RaveSigBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process_rave -in='.$RaveSigBin.' -out='.$OutputDir."/nexus_rave_firmware_signature.c\n";
    print $SignCommandFile $Line;

#Processing of VCE image
    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364) {
        $Line='add_param_bin -in='.$VcePicBin.' -in_endian=le -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VcePicPreBin."\n";
        print $SignCommandFile $Line;
        $Line='sign -in='.$VcePicPreBin.' -out_endian=le -out_type=binary -out='.$VcePicSigBin."\n";
        print $SignCommandFile $Line;

        $Line='add_param_bin -in='.$VceMbBin.' -in_endian=le -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VceMbPreBin."\n";
        print $SignCommandFile $Line;
        $Line='sign -append -in='.$VceMbPreBin.' -out_endian=le -out_type=binary -out='.$VcePicSigBin."\n";
        print $SignCommandFile $Line;

        if ($NumChipId == 7445) {
            $Line='sign -append -in='.$VcePicPreBin.' -out_endian=le -out_type=binary -out='.$VcePicSigBin."\n";
            print $SignCommandFile $Line;
            $Line='sign -append -in='.$VceMbPreBin.' -out_endian=le -out_type=binary -out='.$VcePicSigBin."\n";
            print $SignCommandFile $Line;
		}

        $Line='post_process_vce -in='.$VcePicSigBin.' -out='.$OutputDir."/nexus_video_encoder_region_signatures.c\n";
        print $SignCommandFile $Line;
	}

#Processing of AVD image
    my $CpuType = "hvd";
    if ($NumChipId == 7360) {
        $CpuType = "avd";
    }

    $Line='add_param_bin -in='.$AvdOlBin.' -in_endian=le -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdOlPreBin."\n";
    print $SignCommandFile $Line;
    $Line='sign -in='.$AvdOlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;

    $Line='add_param_bin -in='.$AvdIlBin.' -in_endian=le -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdIlPreBin."\n";
    print $SignCommandFile $Line;
    $Line='sign -append -in='.$AvdIlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;

    $Line='sign -append -in='.$AvdIlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;

    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364) {
        if (($NumChipId == 7445) || ($NumChipId == 7439)) {
            $Line='sign -append -in='.$AvdOlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
            $Line='sign -append -in='.$AvdIlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
	}

        if ($NumChipId == 7445) {
            $Line='sign -append -in='.$AvdOlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
            $Line='sign -append -in='.$AvdIlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
	}
    }

    $Line='post_process_avd -in='.$AvdOlSigBin.' -out='.$OutputDir."/nexus_video_decoder_region_signatures.c\n";
    print $SignCommandFile $Line;

    close $SignCommandFile;
}

sub GenerateSignatures {
    my $SignCommandFile;
    open($SignCommandFile, ">",$OutputDir."/".$ChipId.".in");

#    my $Line = 'File for BCM'.$ChipId."\n";
#    print $SignCommandFile $Line;

    my $Line='load_prv_key -in='.$PrivateKey."\n";
    print $SignCommandFile $Line;

    $Line='load_pub_key -in='.$PublicKey."\n";
    print $SignCommandFile $Line;

    my $Append="";
    my $ParamEndian="";
    if ($NumChipId == 7278) {
        $ParamEndian=" -param_endian=le";
    }

#Processing of SID image
    if ((($ChipVer eq "A0") && ($NumChipId == 7260)) || ($NumChipId != 7260 && $NumChipId != 7278 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7255)) {
        $Line='add_param_bin -in='.$SidBin.' -in_endian=be -cpu_type=sid -mid='.$SidMarketId.' -mid_mask='.$SidMarketIdMask.' -mid_sel='.$SidMarketIdSel.' -epo_sel='.$EpochSel.' -epo='.$SidEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SidSigVer.' -out='.$SidPreBin."\n";
        print $SignCommandFile $Line;
        $Line='sign -in='.$SidPreBin.' -out_endian=le -out_type=binary -out='.$SidSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$SidSigBin.' -in_endian=be -cpu_type=sid -mid='.$SidMarketId.' -mid_mask='.$SidMarketIdMask.' -mid_sel='.$SidMarketIdSel.' -epo_sel='.$EpochSel.' -epo='.$SidEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SidSigVer.' -out='.$SidSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$SidSigParamBin.' -in_endian=be -region=0x11 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;
        $Append=" -append";
    }

#Processing of AUDIO image
    if ($NumChipId != 7255) {
        $Line='add_param_bin -in='.$RaagaBin.' -in_endian=be -cpu_type=raaga -mid='.$RaagaMarketId.' -mid_mask='.$RaagaMarketIdMask.' -mid_sel='.$RaagaMarketIdSel.' -fw_epo='.$RaagaFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaagaEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$RaagaSigVer.' -out='.$RaagaPreBin."\n";
        print $SignCommandFile $Line;
        $Line='sign -in='.$RaagaPreBin.' -out_endian=le -out_type=binary -out='.$RaagaSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$RaagaSigBin.$ParamEndian.' -in_endian=be -cpu_type=raaga -mid='.$RaagaMarketId.' -mid_mask='.$RaagaMarketIdMask.' -mid_sel='.$RaagaMarketIdSel.' -fw_epo='.$RaagaFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaagaEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$RaagaSigVer.' -out='.$RaagaSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$RaagaSigParamBin.' -in_endian=be -region=0x9 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;
        $Append=" -append";
        if ($NumChipId == 7445) {
            $Line='post_process'.$Append.' -in='.$RaagaSigParamBin.' -in_endian=be -region=0x12 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
        }
    }

#Processing RAVE image
    $Line='add_param_bin -in='.$RaveBin.' -in_endian=be -cpu_type=rave -mid='.$RaveMarketId.' -mid_mask='.$RaveMarketIdMask.' -mid_sel='.$RaveMarketIdSel.' -fw_epo='.$RaveFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaveEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$RaveSigVer.' -out='.$RavePreBin."\n";
    print $SignCommandFile $Line;
    $Line='sign -in='.$RavePreBin.' -out_endian=le -out_type=binary -out='.$RaveSigBin."\n";
    print $SignCommandFile $Line;
    $Line='add_param_bin -in='.$RaveSigBin.$ParamEndian.' -in_endian=be -cpu_type=rave -mid='.$RaveMarketId.' -mid_mask='.$RaveMarketIdMask.' -mid_sel='.$RaveMarketIdSel.' -fw_epo='.$RaveFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaveEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$RaveSigVer.' -out='.$RaveSigParamBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process'.$Append.' -in='.$RaveSigParamBin.' -in_endian=be -region=0x8 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;
    if ($NumChipId == 7255) {
        $Append=" -append";
    }

#Processing VCE image
    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364 && $NumChipId != 7255) {
        $Line='add_param_bin -in='.$VcePicBin.' -in_endian=be -cpu_type=vice -mid='.$VceMarketId.' -mid_mask='.$VceMarketIdMask.' -mid_sel='.$VceMarketIdSel.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$VceSigVer.' -out='.$VcePicPreBin."\n";
        print $SignCommandFile $Line;
        $Line='sign -in='.$VcePicPreBin.' -out_endian=le -out_type=binary -out='.$VcePicSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$VcePicSigBin.' -in_endian=be -cpu_type=vice -mid='.$VceMarketId.' -mid_mask='.$VceMarketIdMask.' -mid_sel='.$VceMarketIdSel.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$VceSigVer.' -out='.$VcePicSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$VcePicSigParamBin.' -in_endian=be -region=0xF -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;


        $Line='add_param_bin -in='.$VceMbBin.' -in_endian=be -cpu_type=vice -mid='.$VceMarketId.' -mid_mask='.$VceMarketIdMask.' -mid_sel='.$VceMarketIdSel.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$VceSigVer.' -out='.$VceMbPreBin."\n";
        print $SignCommandFile $Line;
        $Line='sign -in='.$VceMbPreBin.' -out_endian=le -out_type=binary -out='.$VceMbSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$VceMbSigBin.' -in_endian=be -cpu_type=vice -mid='.$VceMarketId.' -mid_mask='.$VceMarketIdMask.' -mid_sel='.$VceMarketIdSel.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$VceSigVer.' -out='.$VceMbSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$VceMbSigParamBin.' -in_endian=be -region=0x10 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;

        if ($NumChipId == 7445) {
            $Line='post_process'.$Append.' -in='.$VcePicSigParamBin.' -in_endian=be -region=0x13 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
            $Line='post_process'.$Append.' -in='.$VceMbSigParamBin.' -in_endian=be -region=0x14 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
		}
	}

#Processing AVD image
    my $CpuType = "hvd";
    if ($NumChipId == 7360) {
        $CpuType = "avd";
    }

    $Line='add_param_bin -in='.$AvdOlBin.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$AvdMarketId.' -mid_mask='.$AvdMarketIdMask.' -mid_sel='.$AvdMarketIdSel.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$AvdSigVer.' -out='.$AvdOlPreBin."\n";
    print $SignCommandFile $Line;
    $Line='sign -in='.$AvdOlPreBin.' -out_endian=le -out_type=binary -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;
    $Line='add_param_bin -in='.$AvdOlSigBin.$ParamEndian.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$AvdMarketId.' -mid_mask='.$AvdMarketIdMask.' -mid_sel='.$AvdMarketIdSel.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$AvdSigVer.' -out='.$AvdOlSigParamBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process'.$Append.' -in='.$AvdOlSigParamBin.' -in_endian=be -region=0xD -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;

    $Line='add_param_bin -in='.$AvdIlBin.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$AvdMarketId.' -mid_mask='.$AvdMarketIdMask.' -mid_sel='.$AvdMarketIdSel.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$AvdSigVer.' -out='.$AvdIlPreBin."\n";
    print $SignCommandFile $Line;
    $Line='sign -in='.$AvdIlPreBin.' -out_endian=le -out_type=binary -out='.$AvdIlSigBin."\n";
    print $SignCommandFile $Line;
    $Line='add_param_bin -in='.$AvdIlSigBin.$ParamEndian.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$AvdMarketId.' -mid_mask='.$AvdMarketIdMask.' -mid_sel='.$AvdMarketIdSel.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$AvdSigVer.' -out='.$AvdIlSigParamBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0xC -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;

    $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0xB -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;

    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364 && $NumChipId != 7255) {
        if (($NumChipId == 7445) || ($NumChipId == 7439) || ($NumChipId == 7278)) {
            $Line='post_process'.$Append.' -in='.$AvdOlSigParamBin.' -in_endian=be -region=0x16 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
            $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0x15 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
		}

        if ($NumChipId == 7445) {
            $Line='post_process'.$Append.' -in='.$AvdOlSigParamBin.' -in_endian=be -region=0x1D -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
            $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0x1C -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
		}
    }
    close $SignCommandFile;
}

sub GeneratePostProcess {
    open (my $SignCommandFile, ">", $OutputDir."/".$ChipId."_signature.in");
    my $Append="";
    my $Line="";
    my $ParamEndian="";
    if ($NumChipId == 7278) {
        $ParamEndian=" -param_endian=le";
    }

#Processing of SID image
    if ((($ChipVer eq "A0") && ($NumChipId == 7260)) || ($NumChipId != 7260 && $NumChipId != 7278 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7255)) {
        $Line='swap_signature -in='.$SidSigBeBin.' -out='.$SidSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$SidSigBin.' -in_endian=be -cpu_type=sid -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -epo_sel='.$EpochSel.' -epo='.$SidEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$SidSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$SidSigParamBin.' -in_endian=be -region=0x11 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;
        $Append=" -append";
    }

#Processing of AUDIO image
    if ($NumChipId != 7255) {
        $Line='swap_signature -in='.$RaagaSigBeBin.' -out='.$RaagaSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$RaagaSigBin.$ParamEndian.' -in_endian=be -cpu_type=raaga -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$RaagaFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaagaEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RaagaSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$RaagaSigParamBin.' -in_endian=be -region=0x9 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;
        $Append=" -append";
#        if NumChipId == 7445:
        if (($NumChipId == 7445) || ($NumChipId == 7439)) {
            $Line='post_process'.$Append.' -in='.$RaagaSigParamBin.' -in_endian=be -region=0x12 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
        }
    }

#Processing RAVE image
    $Line='swap_signature -in='.$RaveSigBeBin.' -out='.$RaveSigBin."\n";
    print $SignCommandFile $Line;
    $Line='add_param_bin -in='.$RaveSigBin.$ParamEndian.' -in_endian=be -cpu_type=rave -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$RaveFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaveEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RaveSigParamBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process'.$Append.' -in='.$RaveSigParamBin.' -in_endian=be -region=0x8 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;
    if ($NumChipId == 7255) {
        $Append=" -append";
    }

#Processing VCE image
    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364 && $NumChipId != 7255) {
        $Line='swap_signature -in='.$VcePicSigBeBin.' -out='.$VcePicSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$VcePicSigBin.' -in_endian=be -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VcePicSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$VcePicSigParamBin.' -in_endian=be -region=0xF -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;

        $Line='swap_signature -in='.$VceMbSigBeBin.' -out='.$VceMbSigBin."\n";
        print $SignCommandFile $Line;
        $Line='add_param_bin -in='.$VceMbSigBin.' -in_endian=be -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VceMbSigParamBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process'.$Append.' -in='.$VceMbSigParamBin.' -in_endian=be -region=0x10 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
        print $SignCommandFile $Line;

        if ($NumChipId == 7445) {
            $Line='post_process'.$Append.' -in='.$VcePicSigParamBin.' -in_endian=be -region=0x13 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
            $Line='post_process'.$Append.' -in='.$VceMbSigParamBin.' -in_endian=be -region=0x14 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
		}
	}

#Processing AVD image
    my $CpuType = "hvd";
    if ($NumChipId == 7360) {
        $CpuType = "avd";
    }

    $Line='swap_signature -in='.$AvdOlSigBeBin.' -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;
    $Line='add_param_bin -in='.$AvdOlSigBin.$ParamEndian.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdOlSigParamBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process'.$Append.' -in='.$AvdOlSigParamBin.' -in_endian=be -region=0xD -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;

    $Line='swap_signature -in='.$AvdIlSigBeBin.' -out='.$AvdIlSigBin."\n";
    print $SignCommandFile $Line;
    $Line='add_param_bin -in='.$AvdIlSigBin.$ParamEndian.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdIlSigParamBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0xC -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;

    $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0xB -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
    print $SignCommandFile $Line;

    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364 && $NumChipId != 7255) {
        if (($NumChipId == 7445) || ($NumChipId == 7439) || ($NumChipId == 7278)) {
            $Line='post_process'.$Append.' -in='.$AvdOlSigParamBin.' -in_endian=be -region=0x16 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
            $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0x15 -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
	    }

        if ($NumChipId == 7445) {
            $Line='post_process'.$Append.' -in='.$AvdOlSigParamBin.' -in_endian=be -region=0x1D -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
            $Line='post_process'.$Append.' -in='.$AvdIlSigParamBin.' -in_endian=be -region=0x1C -out='.$OutputDir."/nexus_security_regver_signatures.h\n";
            print $SignCommandFile $Line;
	    }
    }
}

sub GenerateLegacyPostProcess {
    open (my $SignCommandFile, ">", $OutputDir."/".$ChipId."_Legacy_signature.in");

    my $Line="";
#Processing of SID image
    if ($NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635) {
        $Line='swap_signature -in='.$SidSigBeBin.' -out='.$SidSigBin."\n";
        print $SignCommandFile $Line;
        $Line='post_process_sid -in='.$SidSigBin.' -out='.$OutputDir."/nexus_sid_firmware_signature.c\n";
        print $SignCommandFile $Line;
    }

#Processing of AUDIO image
    $Line='swap_signature -in='.$RaagaSigBeBin.' -out='.$RaagaSigBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process_audio -in='.$RaagaSigBin.' -out='.$OutputDir."/nexus_audio_firmware_signature.c\n";
    print $SignCommandFile $Line;

#Processing of RAVE image
    $Line='swap_signature -in='.$RaveSigBeBin.' -out='.$RaveSigBin."\n";
    print $SignCommandFile $Line;
    $Line='post_process_rave -in='.$RaveSigBin.' -out='.$OutputDir."/nexus_rave_firmware_signature.c\n";
    print $SignCommandFile $Line;

#Processing of VCE image
    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364) {

        $Line='swap_signature -in='.$VcePicSigBeBin.' -out='.$VcePicSigBin."\n";
        print $SignCommandFile $Line;

        $Line='swap_signature -append -in='.$VceMbSigBeBin.' -out='.$VcePicSigBin."\n";
        print $SignCommandFile $Line;

        if ($NumChipId == 7445) {
            $Line='swap_signature -append -in='.$VcePicSigBeBin.' -out='.$VcePicSigBin."\n";
            print $SignCommandFile $Line;
            $Line='swap_signature -append -in='.$VceMbSigBeBin.' -out='.$VcePicSigBin."\n";
            print $SignCommandFile $Line;
		}

        $Line='post_process_vce -in='.$VcePicSigBin.' -out='.$OutputDir."/nexus_video_encoder_region_signatures.c\n";
        print $SignCommandFile $Line;
	}
#Processing of AVD image
    $Line='swap_signature -in='.$AvdOlSigBeBin.' -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;

    $Line='swap_signature -append -in='.$AvdIlSigBeBin.' -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;

    $Line='swap_signature -append -in='.$AvdIlSigBeBin.' -out='.$AvdOlSigBin."\n";
    print $SignCommandFile $Line;

    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364) {
        if (($NumChipId == 7445) || ($NumChipId == 7439)) {
            $Line='swap_signature -append -in='.$AvdOlSigBeBin.' -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
            $Line='swap_signature -append -in='.$AvdIlSigBeBin.' -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
		}
        if ($NumChipId == 7445) {
            $Line='swap_signature -append -in='.$AvdOlSigBeBin.' -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
            $Line='swap_signature -append -in='.$AvdIlSigBeBin.' -out='.$AvdOlSigBin."\n";
            print $SignCommandFile $Line;
		}
	}
    $Line='post_process_avd -in='.$AvdOlSigBin.' -out='.$OutputDir."/nexus_video_decoder_region_signatures.c\n";
    print $SignCommandFile $Line;

    close $SignCommandFile;
}


sub GeneratePreProcess {
    open (my $SignCommandFile, ">", $OutputDir."/".$ChipId.".in");
    my $Line="";
#Processing of SID image
    if ((($ChipVer eq "A0") && ($NumChipId == 7260)) || ($NumChipId != 7260 && $NumChipId != 7278 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7255)) {
        $Line='add_param_bin -in='.$SidBin.' -in_endian=be -cpu_type=sid -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -epo_sel='.$EpochSel.' -epo='.$SidEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$SidPreBin."\n";
        print $SignCommandFile $Line;
    }

#Processing of AUDIO image
    if ($NumChipId != 7255) {
        $Line='add_param_bin -in='.$RaagaBin.' -in_endian=be -cpu_type=raaga -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$RaagaFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaagaEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RaagaPreBin."\n";
        print $SignCommandFile $Line;
    }

#Processing RAVE image
    $Line='add_param_bin -in='.$RaveBin.' -in_endian=be -cpu_type=rave -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$RaveFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaveEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RavePreBin."\n";
    print $SignCommandFile $Line;

#Processing VCE image
    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364 && $NumChipId != 7255) {
        $Line='add_param_bin -in='.$VcePicBin.' -in_endian=be -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VcePicPreBin."\n";
        print $SignCommandFile $Line;

        $Line='add_param_bin -in='.$VceMbBin.' -in_endian=be -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VceMbPreBin."\n";
        print $SignCommandFile $Line;
	}

#Processing AVD image
    my $CpuType = "hvd";
    if ($NumChipId == 7360) {
        $CpuType = "avd";
    }

    $Line='add_param_bin -in='.$AvdOlBin.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdOlPreBin."\n";
    print $SignCommandFile $Line;

    $Line='add_param_bin -in='.$AvdIlBin.' -in_endian=be -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdIlPreBin."\n";
    print $SignCommandFile $Line;

    close $SignCommandFile;
}

sub GenerateLegacyPreProcess {
    open (my $SignCommandFile, ">",  $OutputDir."/".$ChipId."_Legacy.in");
    my $Line="";

#Processing of SID image
    if ($NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635) {
        $Line='add_param_bin -in='.$SidBin.' -in_endian=le -cpu_type=sid -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -epo_sel='.$EpochSel.' -epo='.$SidEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$SidPreBin."\n";
        print $SignCommandFile $Line;
    }

#Processing of AUDIO image
    $Line='add_param_bin -in='.$RaagaBin.' -in_endian=le -cpu_type=raaga -mid='.$MarketId.' -fw_epo='.$RaagaFwEpoch.' -mid_mask='.$MarketIdMask.' -epo_sel='.$EpochSel.' -epo='.$RaagaEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RaagaPreBin."\n";
    print $SignCommandFile $Line;

#Processing of RAVE image
    $Line='add_param_bin -in='.$RaveBin.' -in_endian=be -cpu_type=rave -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$RaveFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$RaveEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$RavePreBin."\n";
    print $SignCommandFile $Line;

#Processing of VCE image
    if ($NumChipId != 7260 && $NumChipId != 74371 && $NumChipId != 7271 && $NumChipId != 7268 && $NumChipId != 7250 && $NumChipId != 7360 && $NumChipId != 7425 && $NumChipId != 7563 && $NumChipId != 75635 && $NumChipId != 7364) {
        $Line='add_param_bin -in='.$VcePicBin.' -in_endian=le -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VcePicPreBin."\n";
        print $SignCommandFile $Line;

        $Line='add_param_bin -in='.$VceMbBin.' -in_endian=le -cpu_type=vice -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$VceFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$VceEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$VceMbPreBin."\n";
        print $SignCommandFile $Line;
	}
#Processing of AVD image
    my $CpuType = "hvd";
    if ($NumChipId == 7360) {
        $CpuType = "avd";
    }

    $Line='add_param_bin -in='.$AvdOlBin.' -in_endian=le -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdOlPreBin."\n";
    print $SignCommandFile $Line;

    $Line='add_param_bin -in='.$AvdIlBin.' -in_endian=le -cpu_type='.$CpuType.' -mid='.$MarketId.' -mid_mask='.$MarketIdMask.' -fw_epo='.$AvdFwEpoch.' -epo_sel='.$EpochSel.' -epo='.$AvdEpoch.' -epo_mask='.$EpochMask.' -sig_type='.$SigType.' -sig_ver='.$SigVer.' -out='.$AvdIlPreBin."\n";
    print $SignCommandFile $Line;

    close $SignCommandFile;
}
