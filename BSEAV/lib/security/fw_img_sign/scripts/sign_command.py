
import os
import sys

print os.environ['HOME']

try:
   os.environ["OUT_DIR"]
except KeyError:
   print "Cannot find output directory. Please set OUT_DIR"
   sys.exit(1)

try:
   os.environ["BCHP_ID"]
except KeyError:
   print "Cannot find Chip ID. Please set BCHP_ID"
   sys.exit(1)

try:
   os.environ["REFSW_VERSION"]
except KeyError:
   print "Cannot find Reference Software Version. Please set REFSW_VERSION"
   sys.exit(1)

def GetSigningRight(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return "0x0"

def GetKeyExp(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return "0x1"

def GetEpoch(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return os.environ["EPOCH"]

def GetEpochMask(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return os.environ["EPOCH_MASK"]

def GetEpochSel(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return os.environ["EPOCH_SEL"]

def GetMarketId(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return os.environ["MARKET_ID"]

def GetMarketIdMask(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return os.environ["MARKET_ID_MASK"]

def GetSigType(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return os.environ["SIG_TYPE"]

def GetSigVer(Engine):
    try:
        os.environ[Engine]
        return os.environ[Engine]
    except KeyError:
        return os.environ["SIG_VER"]

def GenerateKeyParams(Key, ParamPrefix, SkipArray):
    Key2Cmd = os.environ["KEY2_COMMAND_FILE"]
    Key2SigCmd = os.environ["KEY2_SIG_COMMAND_FILE"]
    Key2Bin = OutputDir+'/'+os.environ["KEY2_BIN"]
    Key2Src = OutputDir+'/'+os.environ["KEY2_FILE_NAME"]
    KeyCommandFile = open(Key2Cmd,'w')

    Key2Right = GetSigningRight(ParamPrefix+"_SIGNING_RIGHT")
    Key2Exp = GetKeyExp(ParamPrefix+"_EXPONENT")
    Key2MarketId = GetMarketId(ParamPrefix+"_MARKET_ID")
    Key2MarketIdMask = GetMarketIdMask(ParamPrefix+"_MARKET_ID_MASK")
    Key2EpochSel = GetEpochSel(ParamPrefix+"_EPOCH_SEL")
    Key2Epoch = GetEpoch(ParamPrefix+"_EPOCH")
    Key2EpochMask = GetEpochMask(ParamPrefix+"_EPOCH_MASK")
    Key2SigType = GetSigType(ParamPrefix+"_SIG_TYPE")
    Key2SigVer = GetSigVer(ParamPrefix+"_SIG_VER")

    Line='load_pub_key -in='+Key+'\n'
    KeyCommandFile.write(str(Line))

    Line='add_param_key -right='+Key2Right+' -exp='+Key2Exp+' -mid='+Key2MarketId+' -mid_mask='+Key2MarketIdMask+' -epo_sel='+Key2EpochSel+' -epo='+Key2Epoch+' -epo_mask='+Key2EpochMask+' -sig_type='+Key2SigType+' -sig_ver='+Key2SigVer+' -out='+Key2Bin+'\n'
    KeyCommandFile.write(str(Line))

    KeyCommandFile.close()

    KeyCommandFile = open(Key2SigCmd,'w')
    Line='add_signature -in_key='+Key2Bin+' -in_signature='+Key2Bin+'.signature.be -out_endian=le -out='+Key2Bin+'.signed_le\n'
    KeyCommandFile.write(str(Line))

    if SkipArray == 0:
        if SoftwareVersion > 15.3:
            Line='c_array -in='+Key2Bin+'.signed_le -var=gRegionVerificationKey2 -out='+Key2Src+'\n'
        else:
            Line='c_array -in='+Key2Bin+'.signed_le -var=gKey2 -out='+Key2Src+'\n'
        KeyCommandFile.write(str(Line))

    KeyCommandFile.close()


def GenerateLegacySignatures():
    SignCommandFile = open(OutputDir+"/"+ChipId+"_Legacy.in",'w')

    Line='load_prv_key -in='+PrivateKey+'\n'
    SignCommandFile.write(str(Line))

    Line='load_pub_key -in='+PublicKey+'\n'
    SignCommandFile.write(str(Line))

#Processing of SID image
    if (NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+SidBin+' -in_endian=le -cpu_type=sid -mid='+MarketId+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+SidEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+SidPreBin+'\n'
        SignCommandFile.write(str(Line))
        Line='sign -in='+SidPreBin+' -out_endian=le -out_type=binary -out='+SidSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process_sid -in='+SidSigBin+' -out='+OutputDir+'/nexus_sid_firmware_signature.c\n'
        SignCommandFile.write(str(Line))

#Processing of AUDIO image
    Line='add_param_bin -in='+RaagaBin+' -in_endian=le -cpu_type=raaga -mid='+MarketId+' -fw_epo='+RaagaFwEpoch+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+RaagaEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaagaPreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -in='+RaagaPreBin+' -out_endian=le -out_type=binary -out='+RaagaSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process_audio -in='+RaagaSigBin+' -out='+OutputDir+'/nexus_audio_firmware_signature.c\n'
    SignCommandFile.write(str(Line))

#Processing of RAVE image
    Line='add_param_bin -in='+RaveBin+' -in_endian=be -cpu_type=rave -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaveFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaveEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RavePreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -in='+RavePreBin+' -out_endian=le -out_type=binary -out='+RaveSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process_rave -in='+RaveSigBin+' -out='+OutputDir+'/nexus_rave_firmware_signature.c\n'
    SignCommandFile.write(str(Line))

#Processing of VCE image
    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+VcePicBin+' -in_endian=le -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VcePicPreBin+'\n'
        SignCommandFile.write(str(Line))
        Line='sign -in='+VcePicPreBin+' -out_endian=le -out_type=binary -out='+VcePicSigBin+'\n'
        SignCommandFile.write(str(Line))

        Line='add_param_bin -in='+VceMbBin+' -in_endian=le -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VceMbPreBin+'\n'
        SignCommandFile.write(str(Line))
        Line='sign -append -in='+VceMbPreBin+' -out_endian=le -out_type=binary -out='+VcePicSigBin+'\n'
        SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='sign -append -in='+VcePicPreBin+' -out_endian=le -out_type=binary -out='+VcePicSigBin+'\n'
            SignCommandFile.write(str(Line))
            Line='sign -append -in='+VceMbPreBin+' -out_endian=le -out_type=binary -out='+VcePicSigBin+'\n'
            SignCommandFile.write(str(Line))

        Line='post_process_vce -in='+VcePicSigBin+' -out='+OutputDir+'/nexus_video_encoder_region_signatures.c\n'
        SignCommandFile.write(str(Line))

#Processing of AVD image
    Line='add_param_bin -in='+AvdOlBin+' -in_endian=le -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdOlPreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -in='+AvdOlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))

    Line='add_param_bin -in='+AvdIlBin+' -in_endian=le -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdIlPreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -append -in='+AvdIlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))

    Line='sign -append -in='+AvdIlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))

    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        if (NumChipId == 7445) or (NumChipId == 7439):
            Line='sign -append -in='+AvdOlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))
            Line='sign -append -in='+AvdIlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='sign -append -in='+AvdOlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))
            Line='sign -append -in='+AvdIlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))

    Line='post_process_avd -in='+AvdOlSigBin+' -out='+OutputDir+'/nexus_video_decoder_region_signatures.c\n'
    SignCommandFile.write(str(Line))

    SignCommandFile.close()

def GenerateSignatures():
    SignCommandFile = open(OutputDir+"/"+ChipId+".in",'w')

    Line = 'File for BCM'+ChipId+'\n'
    SignCommandFile.write(str(Line))

    Line='load_prv_key -in='+PrivateKey+'\n'
    SignCommandFile.write(str(Line))

    Line='load_pub_key -in='+PublicKey+'\n'
    SignCommandFile.write(str(Line))

#Processing of SID image
    if (NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+SidBin+' -in_endian=be -cpu_type=sid -mid='+MarketId+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+SidEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+SidPreBin+'\n'
        SignCommandFile.write(str(Line))
        Line='sign -in='+SidPreBin+' -out_endian=le -out_type=binary -out='+SidSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='add_param_bin -in='+SidSigBin+' -in_endian=be -cpu_type=sid -mid='+MarketId+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+SidEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+SidSigParamBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process -in='+SidSigParamBin+' -in_endian=be -region=0x11 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))

#Processing of AUDIO image
    Line='add_param_bin -in='+RaagaBin+' -in_endian=be -cpu_type=raaga -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaagaFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaagaEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaagaPreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -in='+RaagaPreBin+' -out_endian=le -out_type=binary -out='+RaagaSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+RaagaSigBin+' -in_endian=be -cpu_type=raaga -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaagaFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaagaEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaagaSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+RaagaSigParamBin+' -in_endian=be -region=0x9 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))
    if NumChipId == 7445:
        Line='post_process -append -in='+RaagaSigParamBin+' -in_endian=be -region=0x12 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))

#Processing RAVE image
    Line='add_param_bin -in='+RaveBin+' -in_endian=be -cpu_type=rave -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaveFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaveEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RavePreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -in='+RavePreBin+' -out_endian=le -out_type=binary -out='+RaveSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+RaveSigBin+' -in_endian=be -cpu_type=rave -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaveFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaveEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaveSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+RaveSigParamBin+' -in_endian=be -region=0x8 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

#Processing VCE image
    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+VcePicBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VcePicPreBin+'\n'
        SignCommandFile.write(str(Line))
        Line='sign -in='+VcePicPreBin+' -out_endian=le -out_type=binary -out='+VcePicSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='add_param_bin -in='+VcePicSigBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VcePicSigParamBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process -append -in='+VcePicSigParamBin+' -in_endian=be -region=0xF -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))


        Line='add_param_bin -in='+VceMbBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VceMbPreBin+'\n'
        SignCommandFile.write(str(Line))
        Line='sign -in='+VceMbPreBin+' -out_endian=le -out_type=binary -out='+VceMbSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='add_param_bin -in='+VceMbSigBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VceMbSigParamBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process -append -in='+VceMbSigParamBin+' -in_endian=be -region=0x10 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='post_process -append -in='+VcePicSigParamBin+' -in_endian=be -region=0x13 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))
            Line='post_process -append -in='+VceMbSigParamBin+' -in_endian=be -region=0x14 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))

#Processing AVD image
    Line='add_param_bin -in='+AvdOlBin+' -in_endian=be -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdOlPreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -in='+AvdOlPreBin+' -out_endian=le -out_type=binary -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+AvdOlSigBin+' -in_endian=be -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdOlSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+AvdOlSigParamBin+' -in_endian=be -region=0xD -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

    Line='add_param_bin -in='+AvdIlBin+' -in_endian=be -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdIlPreBin+'\n'
    SignCommandFile.write(str(Line))
    Line='sign -in='+AvdIlPreBin+' -out_endian=le -out_type=binary -out='+AvdIlSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+AvdIlSigBin+' -in_endian=be -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdIlSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0xC -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

    Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0xB -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        if (NumChipId == 7445) or (NumChipId == 7439):
            Line='post_process -append -in='+AvdOlSigParamBin+' -in_endian=be -region=0x16 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))
            Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0x15 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='post_process -append -in='+AvdOlSigParamBin+' -in_endian=be -region=0x1D -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))
            Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0x1C -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))

    SignCommandFile.close()

def GeneratePostProcess():
    SignCommandFile = open(OutputDir+"/"+ChipId+"_signature.in",'w')

#Processing of SID image
    if (NumChipId != 7360 and NumChipId != 7425):
        Line='swap_signature -in='+SidSigBeBin+' -out='+SidSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='add_param_bin -in='+SidSigBin+' -in_endian=be -cpu_type=sid -mid='+MarketId+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+SidEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+SidSigParamBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process -in='+SidSigParamBin+' -in_endian=be -region=0x11 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))

#Processing of AUDIO image
    Line='swap_signature -in='+RaagaSigBeBin+' -out='+RaagaSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+RaagaSigBin+' -in_endian=be -cpu_type=raaga -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaagaFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaagaEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaagaSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+RaagaSigParamBin+' -in_endian=be -region=0x9 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))
#    if NumChipId == 7445:
    if (NumChipId == 7445) or (NumChipId == 7439):
        Line='post_process -append -in='+RaagaSigParamBin+' -in_endian=be -region=0x12 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))

#Processing RAVE image
    Line='swap_signature -in='+RaveSigBeBin+' -out='+RaveSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+RaveSigBin+' -in_endian=be -cpu_type=rave -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaveFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaveEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaveSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+RaveSigParamBin+' -in_endian=be -region=0x8 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

#Processing VCE image
    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        Line='swap_signature -in='+VcePicSigBeBin+' -out='+VcePicSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='add_param_bin -in='+VcePicSigBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VcePicSigParamBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process -append -in='+VcePicSigParamBin+' -in_endian=be -region=0xF -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))

        Line='swap_signature -in='+VceMbSigBeBin+' -out='+VceMbSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='add_param_bin -in='+VceMbSigBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VceMbSigParamBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process -append -in='+VceMbSigParamBin+' -in_endian=be -region=0x10 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
        SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='post_process -append -in='+VcePicSigParamBin+' -in_endian=be -region=0x13 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))
            Line='post_process -append -in='+VceMbSigParamBin+' -in_endian=be -region=0x14 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))

#Processing AVD image
    Line='swap_signature -in='+AvdOlSigBeBin+' -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+AvdOlSigBin+' -in_endian=be -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdOlSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+AvdOlSigParamBin+' -in_endian=be -region=0xD -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

    Line='swap_signature -in='+AvdIlSigBeBin+' -out='+AvdIlSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='add_param_bin -in='+AvdIlSigBin+' -in_endian=be -cpu_type=hvd -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdIlSigParamBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0xC -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

    Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0xB -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
    SignCommandFile.write(str(Line))

    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        if (NumChipId == 7445) or (NumChipId == 7439):
            Line='post_process -append -in='+AvdOlSigParamBin+' -in_endian=be -region=0x16 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))
            Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0x15 -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='post_process -append -in='+AvdOlSigParamBin+' -in_endian=be -region=0x1D -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))
            Line='post_process -append -in='+AvdIlSigParamBin+' -in_endian=be -region=0x1C -out='+OutputDir+'/nexus_security_regver_signatures.h\n'
            SignCommandFile.write(str(Line))


def GenerateLegacyPostProcess():
    SignCommandFile = open(OutputDir+"/"+ChipId+"_Legacy_signature.in",'w')

#Processing of SID image
    if (NumChipId != 7360 and NumChipId != 7425):
        Line='swap_signature -in='+SidSigBeBin+' -out='+SidSigBin+'\n'
        SignCommandFile.write(str(Line))
        Line='post_process_sid -in='+SidSigBin+' -out='+OutputDir+'/nexus_sid_firmware_signature.c\n'
        SignCommandFile.write(str(Line))

#Processing of AUDIO image
    Line='swap_signature -in='+RaagaSigBeBin+' -out='+RaagaSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process_audio -in='+RaagaSigBin+' -out='+OutputDir+'/nexus_audio_firmware_signature.c\n'
    SignCommandFile.write(str(Line))

#Processing of RAVE image
    Line='swap_signature -in='+RaveSigBeBin+' -out='+RaveSigBin+'\n'
    SignCommandFile.write(str(Line))
    Line='post_process_rave -in='+RaveSigBin+' -out='+OutputDir+'/nexus_rave_firmware_signature.c\n'
    SignCommandFile.write(str(Line))

#Processing of VCE image
    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):

        Line='swap_signature -in='+VcePicSigBeBin+' -out='+VcePicSigBin+'\n'
        SignCommandFile.write(str(Line))

        Line='swap_signature -append -in='+VceMbSigBeBin+' -out='+VcePicSigBin+'\n'
        SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='swap_signature -append -in='+VcePicSigBeBin+' -out='+VcePicSigBin+'\n'
            SignCommandFile.write(str(Line))
            Line='swap_signature -append -in='+VceMbSigBeBin+' -out='+VcePicSigBin+'\n'
            SignCommandFile.write(str(Line))

        Line='post_process_vce -in='+VcePicSigBin+' -out='+OutputDir+'/nexus_video_encoder_region_signatures.c\n'
        SignCommandFile.write(str(Line))

#Processing of AVD image
    Line='swap_signature -in='+AvdOlSigBeBin+' -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))

    Line='swap_signature -append -in='+AvdIlSigBeBin+' -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))

    Line='swap_signature -append -in='+AvdIlSigBeBin+' -out='+AvdOlSigBin+'\n'
    SignCommandFile.write(str(Line))

    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        if (NumChipId == 7445) or (NumChipId == 7439):
            Line='swap_signature -append -in='+AvdOlSigBeBin+' -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))
            Line='swap_signature -append -in='+AvdIlSigBeBin+' -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))

        if NumChipId == 7445:
            Line='swap_signature -append -in='+AvdOlSigBeBin+' -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))
            Line='swap_signature -append -in='+AvdIlSigBeBin+' -out='+AvdOlSigBin+'\n'
            SignCommandFile.write(str(Line))

    Line='post_process_avd -in='+AvdOlSigBin+' -out='+OutputDir+'/nexus_video_decoder_region_signatures.c\n'
    SignCommandFile.write(str(Line))

    SignCommandFile.close()


def GeneratePreProcess():
    SignCommandFile = open(OutputDir+"/"+ChipId+".in",'w')

#Processing of SID image
    if (NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+SidBin+' -in_endian=be -cpu_type=sid -mid='+MarketId+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+SidEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+SidPreBin+'\n'
        SignCommandFile.write(str(Line))

#Processing of AUDIO image
    Line='add_param_bin -in='+RaagaBin+' -in_endian=be -cpu_type=raaga -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaagaFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaagaEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaagaPreBin+'\n'
    SignCommandFile.write(str(Line))

#Processing RAVE image
    Line='add_param_bin -in='+RaveBin+' -in_endian=be -cpu_type=rave -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaveFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaveEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RavePreBin+'\n'
    SignCommandFile.write(str(Line))

#Processing VCE image
    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+VcePicBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VcePicPreBin+'\n'
        SignCommandFile.write(str(Line))

        Line='add_param_bin -in='+VceMbBin+' -in_endian=be -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VceMbPreBin+'\n'
        SignCommandFile.write(str(Line))

#Processing AVD image
    CpuType = "hvd"
    if (NumChipId == 7360):
        CpuType = "avd"
    Line='add_param_bin -in='+AvdOlBin+' -in_endian=be -cpu_type='+CpuType+' -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdOlPreBin+'\n'
    SignCommandFile.write(str(Line))

    Line='add_param_bin -in='+AvdIlBin+' -in_endian=be -cpu_type='+CpuType+' -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdIlPreBin+'\n'
    SignCommandFile.write(str(Line))

    SignCommandFile.close()

def GenerateLegacyPreProcess():
    SignCommandFile = open(OutputDir+"/"+ChipId+"_Legacy.in",'w')

#Processing of SID image
    if (NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+SidBin+' -in_endian=le -cpu_type=sid -mid='+MarketId+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+SidEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+SidPreBin+'\n'
        SignCommandFile.write(str(Line))

#Processing of AUDIO image
    Line='add_param_bin -in='+RaagaBin+' -in_endian=le -cpu_type=raaga -mid='+MarketId+' -fw_epo='+RaagaFwEpoch+' -mid_mask='+MarketIdMask+' -epo_sel='+EpochSel+' -epo='+RaagaEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RaagaPreBin+'\n'
    SignCommandFile.write(str(Line))

#Processing of RAVE image
    Line='add_param_bin -in='+RaveBin+' -in_endian=be -cpu_type=rave -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+RaveFwEpoch+' -epo_sel='+EpochSel+' -epo='+RaveEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+RavePreBin+'\n'
    SignCommandFile.write(str(Line))

#Processing of VCE image
    if (NumChipId != 74371 and NumChipId != 7271 and NumChipId != 7250 and NumChipId != 7360 and NumChipId != 7425):
        Line='add_param_bin -in='+VcePicBin+' -in_endian=le -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VcePicPreBin+'\n'
        SignCommandFile.write(str(Line))

        Line='add_param_bin -in='+VceMbBin+' -in_endian=le -cpu_type=vice -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+VceFwEpoch+' -epo_sel='+EpochSel+' -epo='+VceEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+VceMbPreBin+'\n'
        SignCommandFile.write(str(Line))

#Processing of AVD image
    CpuType = "hvd"
    if (NumChipId == 7360):
        CpuType = "avd"
    Line='add_param_bin -in='+AvdOlBin+' -in_endian=le -cpu_type='+CpuType+' -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdOlPreBin+'\n'
    SignCommandFile.write(str(Line))

    Line='add_param_bin -in='+AvdIlBin+' -in_endian=le -cpu_type='+CpuType+' -mid='+MarketId+' -mid_mask='+MarketIdMask+' -fw_epo='+AvdFwEpoch+' -epo_sel='+EpochSel+' -epo='+AvdEpoch+' -epo_mask='+EpochMask+' -sig_type='+SigType+' -sig_ver='+SigVer+' -out='+AvdIlPreBin+'\n'
    SignCommandFile.write(str(Line))

    SignCommandFile.close()

def ExtractAvsMemsys():
    ImageToolCfg = open(OutputDir+"/"+ChipId+"_extract_avs_memsys.cfg",'w')

    AvsMarketId = GetMarketId("AVS_MARKET_ID")
    AvsMarketIdMask = GetMarketIdMask("AVS_MARKET_ID_MASK")
    AvsEpochSel = GetEpochSel("AVS_EPOCH_SEL")
    AvsEpoch = GetEpoch("AVS_EPOCH")
    AvsEpochMask = GetEpochMask("AVS_EPOCH_MASK")
    AvsSigType = GetSigType("AVS_SIG_TYPE")
    AvsSigVer = GetSigVer("AVS_SIG_VER")

    MemsysMarketId = GetMarketId("MEMSYS_MARKET_ID")
    MemsysMarketIdMask = GetMarketIdMask("MEMSYS_MARKET_ID_MASK")
    MemsysEpochSel = GetEpochSel("MEMSYS_EPOCH_SEL")
    MemsysEpoch = GetEpoch("MEMSYS_EPOCH")
    MemsysEpochMask = GetEpochMask("MEMSYS_EPOCH_MASK")
    MemsysSigType = GetSigType("MEMSYS_SIG_TYPE")
    MemsysSigVer = GetSigVer("MEMSYS_SIG_VER")

    Line = '[global]\nbinaryfile='+BoltBin+'\n'
    ImageToolCfg.write(str(Line))

    Line='[AVS]\nmarketid='+AvsMarketId+'\nmarketidmask='+AvsMarketIdMask+'\nepoch='+AvsEpoch+'\nepochmask='+AvsEpochMask+'\nepochsel='+AvsEpochSel+'\n'
    ImageToolCfg.write(str(Line))
    Line='outfile='+OutputDir+'/'+ChipId+'_avs_to_sign.bin\n'
    ImageToolCfg.write(str(Line))

    Line='\n[memsys]\nmarketid='+MemsysMarketId+'\nmarketidmask='+MemsysMarketIdMask+'\nepoch='+MemsysEpoch+'\nepochmask='+MemsysEpochMask+'\nepochsel='+MemsysEpochSel+'\n'
    ImageToolCfg.write(str(Line))
    Line='outfile='+OutputDir+'/'+ChipId+'_memsys_to_sign.bin'
    ImageToolCfg.write(str(Line))


    ImageToolCfg.close()

def AssembleAvs():
    AvsBin = os.environ["AVS_BIN"]
    Key2Bin = OutputDir+'/'+os.environ["KEY2_BIN"]

    ImageToolCfg = open(OutputDir+"/"+ChipId+"_assemble_avs.cfg",'w')

    Line = '[global]\nbinaryfile='+AvsBin+'\n'
    ImageToolCfg.write(str(Line))

    Line='sigfile='+AvsBin+'.signature.be\n'
    ImageToolCfg.write(str(Line))

    Line='modfile='+Key2Bin+'.signed_le\n'
    ImageToolCfg.write(str(Line))

    Line='outfile='+AvsBin+'.signed\n'
    ImageToolCfg.write(str(Line))

    Line='firmware=avs\n'
    ImageToolCfg.write(str(Line))

    ImageToolCfg.close()

def AssembleMemsys():
    MemsysBin = os.environ["MEMSYS_BIN"]
    Key2Bin = OutputDir+'/'+os.environ["KEY2_BIN"]

    ImageToolCfg = open(OutputDir+"/"+ChipId+"_assemble_memsys.cfg",'w')

    Line = '[global]\nbinaryfile='+MemsysBin+'\n'
    ImageToolCfg.write(str(Line))

    Line='sigfile='+MemsysBin+'.signature.be\n'
    ImageToolCfg.write(str(Line))

    Line='modfile='+Key2Bin+'.signed_le\n'
    ImageToolCfg.write(str(Line))

    Line='outfile='+MemsysBin+'.signed\n'
    ImageToolCfg.write(str(Line))

    Line='firmware=memsys\n'
    ImageToolCfg.write(str(Line))

    ImageToolCfg.close()

SoftwareVersion = float(os.environ["REFSW_VERSION"])
print 'Reference Software Version = '+str(SoftwareVersion)

PrivateKey=os.environ["PRV_KEY"]
PublicKey=os.environ["PUB_KEY"]
SSPublicKey=os.environ["SS_PUB_KEY"]
OutputDir = os.environ["OUT_DIR"]
ChipId = os.environ["BCHP_ID"]
NumChipId = int(ChipId)
MarketId = os.environ["MARKET_ID"]
MarketIdMask = os.environ["MARKET_ID_MASK"]
EpochSel = os.environ["EPOCH_SEL"]
Epoch = os.environ["EPOCH"]
EpochMask = os.environ["EPOCH_MASK"]
SigType = os.environ["SIG_TYPE"]
SigVer = os.environ["SIG_VER"]

SidEpoch = GetEpoch("SID_EPOCH")

RaveEpoch = GetEpoch("RAVE_EPOCH")
RaveFwEpoch = GetEpoch("RAVE_FW_EPOCH")

RaagaEpoch = GetEpoch("RAAGA_EPOCH")
RaagaFwEpoch = GetEpoch("RAAGA_FW_EPOCH")

AvdEpoch = GetEpoch("AVD_EPOCH")
AvdFwEpoch = GetEpoch("AVD_FW_EPOCH")

VceEpoch = GetEpoch("VCE_EPOCH")
VceFwEpoch = GetEpoch("VCE_FW_EPOCH")

SidBin = os.environ["SID_BIN"]
RaagaBin = os.environ["RAAGA_BIN"]
RaveBin = os.environ["RAVE_BIN"]
VcePicBin = os.environ["VCE_PIC_BIN"]
VceMbBin = os.environ["VCE_MB_BIN"]
AvdOlBin = os.environ["AVD_OL_BIN"]
AvdIlBin = os.environ["AVD_IL_BIN"]
BoltBin = os.environ["BOLT_BIN"]


SidPreBin = OutputDir+'/'+ChipId+'_region_0x11.bin'
SidSigBin = OutputDir+'/'+ChipId+'_region_0x11.bin.signature'
SidSigBeBin = OutputDir+'/'+ChipId+'_region_0x11.bin.signature.be'
SidSigParamBin = OutputDir+'/'+ChipId+'_region_0x11_signature_param.bin'

RaagaPreBin = OutputDir+'/'+ChipId+'_region_0x9.bin'
RaagaSigBin = OutputDir+'/'+ChipId+'_region_0x9.bin.signature'
RaagaSigBeBin = OutputDir+'/'+ChipId+'_region_0x9.bin.signature.be'
RaagaSigParamBin = OutputDir+'/'+ChipId+'_region_0x9_signature_param.bin'

RavePreBin = OutputDir+'/'+ChipId+'_region_0x8.bin'
RaveSigBin = OutputDir+'/'+ChipId+'_region_0x8.bin.signature'
RaveSigBeBin = OutputDir+'/'+ChipId+'_region_0x8.bin.signature.be'
RaveSigParamBin = OutputDir+'/'+ChipId+'_region_0x8_signature_param.bin'

VcePicPreBin = OutputDir+'/'+ChipId+'_region_0xF.bin'
VcePicSigBin = OutputDir+'/'+ChipId+'_region_0xF.bin.signature'
VcePicSigBeBin = OutputDir+'/'+ChipId+'_region_0xF.bin.signature.be'
VcePicSigParamBin = OutputDir+'/'+ChipId+'_region_0xF_signature_param.bin'

VceMbPreBin = OutputDir+'/'+ChipId+'_region_0x10.bin'
VceMbSigBin = OutputDir+'/'+ChipId+'_region_0x10.bin.signature'
VceMbSigBeBin = OutputDir+'/'+ChipId+'_region_0x10.bin.signature.be'
VceMbSigParamBin = OutputDir+'/'+ChipId+'_region_0x10_signature_param.bin'

AvdOlPreBin = OutputDir+'/'+ChipId+'_region_0xD.bin'
AvdOlSigBin = OutputDir+'/'+ChipId+'_region_0xD.bin.signature'
AvdOlSigBeBin = OutputDir+'/'+ChipId+'_region_0xD.bin.signature.be'
AvdOlSigParamBin = OutputDir+'/'+ChipId+'_region_0xD_signature_param.bin'

AvdIlPreBin = OutputDir+'/'+ChipId+'_region_0xC.bin'
AvdIlSigBin = OutputDir+'/'+ChipId+'_region_0xC.bin.signature'
AvdIlSigBeBin = OutputDir+'/'+ChipId+'_region_0xC.bin.signature.be'
AvdIlSigParamBin = OutputDir+'/'+ChipId+'_region_0xC_signature_param.bin'

if len(sys.argv) < 2:
    print "Insufficient Arguments to 'sign_command.py'\n"
    sys.exit(1)

OpType = str(sys.argv[1])

if SoftwareVersion > 15.3:
    if (OpType == "fw"):
        GenerateSignatures()
    elif (OpType == "key"):
        GenerateKeyParams(PublicKey, "KEY2", 0)
    elif (OpType == "ss_key"):
        GenerateKeyParams(SSPublicKey, "KEY2", 0)
    elif (OpType == "pre_process"):
        GeneratePreProcess()
    elif (OpType == "post_process"):
        GeneratePostProcess()
    elif (OpType == "extract_avs_memsys"):
        ExtractAvsMemsys()
        GenerateKeyParams(SSPublicKey, "AVS_KEY2", 1)
    elif (OpType == "assemble_avs_memsys"):
        AssembleAvs()
        AssembleMemsys()
    else:
        print "Unsupported Argument : "+OpType+"\n"
        sys.exit(1)
else:
    if (OpType == "fw"):
        GenerateLegacySignatures()
    elif (OpType == "key"):
        GenerateKeyParams(PublicKey, "KEY2", 0)
    elif (OpType == "ss_key"):
        GenerateKeyParams(SSPublicKey, "KEY2", 0)
    elif (OpType == "pre_process"):
        GenerateLegacyPreProcess()
    elif (OpType == "post_process"):
        GenerateLegacyPostProcess()
    else:
        print "Unsupported Argument : "+OpType+"\n"
        sys.exit(1)
