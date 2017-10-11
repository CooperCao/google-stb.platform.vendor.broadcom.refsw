#!/bin/sh -e

# Kickoff CD generator for Fedora 19
# 	$Id$	

# Modifies netinst iso to reference a network-based ks.cfg file.  Note
# that isocp is used to modify the iso in place, since we don't know
# how to build a bootable iso from scratch.

dist=/projects/hnd_cdroms/os/Linux/Fedora/Fedora19-sig
iso=$dist/Fedora-19-x86_64-netinst.iso

## Convert path into NFS components
#mountinfo=`df -Pk $dist|tail -1`
#server=`expr "$mountinfo" : '\(.*\):'`
#filesystem=`expr "$mountinfo" : '.*:\([^ ]*\)'`
#mountpoint=`expr "$mountinfo" : '.* \([^ ]*\)$'`
#ks=${dist/#$mountpoint/$filesystem}/ks.cfg

#echo "KS $server:$ks"


#ks=$(echo $ks | sed 's/\//\\\//g')

cp $iso /tmp/kickoff.iso

# ISOLINUX
#sed "s/<KS>/nfs:$server:$ks/" $dist/isolinux.isolinux.cfg.in > /tmp/isolinux.isolinux.cfg

# EFI
#sed "s/<KS>/nfs:$server:$ks/" $dist/EFI.BOOT.grub.cfg.in > /tmp/EFI.BOOT.grub.cfg

~/src/isocp-1.0.1/isocp /tmp/kickoff.iso \
    $dist/isolinux.isolinux.cfg isolinux/isolinux.cfg
~/src/isocp-1.0.1/isocp /tmp/kickoff.iso \
    $dist/EFI.BOOT.grub.cfg EFI/BOOT/grub.cfg

echo 
echo "Please write /tmp/kickoff.iso to a thumb drive or CD, eg:"
echo "dd if=/tmp/kickoff.iso of=/dev/sdb"
echo "or"
echo "brasero -i /tmp/kickoff.iso"

exit
