source /projects/hnd_sig_ext20/zhuj/utfws/unittest/utfconf/iptv3.tcl

set xargs {
    -brand linux-2.6.36-arm-external-vista-router-dhdap-atlas-full-src
    -perfonly 1 -datarate 0 -docpu 1 -nosamba 0
}

#---------------------------------- r1 4366B1------------------------------

############################################
# BISON04T_BRANCH_7_14 internal (17.1)
###########################################
r1 clone atlasII/r1/0 -sta {atlasII/r1/0 eth1 atlasII/r1/0.%15 wl0.%} \
    -perfchans {3l}
r1 clone atlasII/r1/1 -sta {atlasII/r1/1 eth2 atlasII/r1/1.%15 wl1.%} \
    -perfchans {36/80} -channelsweep {-max 64}
r1 clone atlasII/r1/2 -sta {atlasII/r1/2 eth3 atlasII/r1/2.%15 wl2.%} \
    -perfchans {161/80} -channelsweep {-min 100}

r1 destroy

###########################################
# BISON04T_BRANCH_7_14 external (17.1)
##########################################

atlasII/r1/0 clone atlasII/r1x/0 {*}$xargs
atlasII/r1/1 clone atlasII/r1x/1 {*}$xargs
atlasII/r1/2 clone atlasII/r1x/2 {*}$xargs

atlasII/r1x/2 configure \
    -dualband {atlasII/r1x/1 -c2 36/80 -c1 161/80 -lan1 192int11 -lan2 192int12}

################################################
# BISON04T_TWIG_7_14_131 internal (golden2.x)
################################################
atlasII/r1/0 clone atlasII/r1b131/0 -tag BISON04T_TWIG_7_14_131
atlasII/r1/1 clone atlasII/r1b131/1 -tag BISON04T_TWIG_7_14_131
atlasII/r1/2 clone atlasII/r1b131/2 -tag BISON04T_TWIG_7_14_131

################################################
# BISON04T_TWIG_7_14_131 external (golden2.x)
################################################
atlasII/r1b131/0 clone atlasII/r1b131x/0 {*}$xargs
atlasII/r1b131/1 clone atlasII/r1b131x/1 {*}$xargs
atlasII/r1b131/2 clone atlasII/r1b131x/2 {*}$xargs

atlasII/r1b131x/0 configure \
    -dualband {atlasII/r1b131x/2 -c1 36/80 -c2 161/80 -lan1 192int11 -lan2 192int12}

#################################
# BISON04T_REL_7_14_131_2509 external
#################################
atlasII/r1b131/0 clone atlasII/r1b131_2509x/0 {*}$xargs
atlasII/r1b131/1 clone atlasII/r1b131_2509x/1 {*}$xargs
atlasII/r1b131/2 clone atlasII/r1b131_2509x/2 {*}$xargs

atlasII/r1b131_2509x/0 configure \
    -dualband {atlasII/r1b131_2509x/2 -c1 36/80 -c2 161/80 -lan1 192int11 -lan2 192int12}

################################
# private images
################################

#atlasII/r1b131/2 clone r1b1-jdbug34 -image "/projects/hnd_sw_routerdev_ext2/work/jihuac/test/vi-be/linux-131-0419-clean.trx"

#---------------------------------- r2 4366C0------------------------------
############################################
# BISON04T_BRANCH_7_14 internal (17.1)
###########################################
r2 clone atlasII/r2/0 -sta {atlasII/r2/0 eth1 atlasII/r2/0.%15 wl0.%} \
    -perfchans {3l}
r2 clone atlasII/r2/1 -sta {atlasII/r2/1 eth2 atlasII/r2/1.%15 wl1.%} \
    -perfchans {36/80} -channelsweep {-max 64}
r2 clone atlasII/r2/2 -sta {atlasII/r2/2 eth3 atlasII/r2/2.%15 wl2.%} \
    -perfchans {161/80} -channelsweep {-min 100}

r2 destroy
lappend xargs -nvram "[atlasII/r2/2 cget -nvram]
    wl1_country_code=Q1
    wl1_country_rev=137
    wl2_country_code=Q1
    wl2_country_rev=137
"

############################################
# BISON04T_BRANCH_7_14 external (17.1)
############################################
atlasII/r2/0 clone atlasII/r2x/0 {*}$xargs
atlasII/r2/1 clone atlasII/r2x/1 {*}$xargs -perfchans {36/80 36/160}
atlasII/r2/2 clone atlasII/r2x/2 {*}$xargs -perfchans {161/80 128/160}

atlasII/r2x/2 configure \
    -dualband {atlasII/r2x/1 -c2 36/80 -c1 161/80 -lan1 192int21 -lan2 192int22}

#############################################
# BISON04T_TWIG_7_14_164 internal (Golden3.x)
#############################################
atlasII/r2/0 clone atlasII/r2b164/0 -tag BISON04T_TWIG_7_14_164
atlasII/r2/1 clone atlasII/r2b164/1 -tag BISON04T_TWIG_7_14_164
atlasII/r2/2 clone atlasII/r2b164/2 -tag BISON04T_TWIG_7_14_164
#############################################
# BISON04T_TWIG_7_14_164 external (Golden3.x)
#############################################
atlasII/r2b164/0 clone atlasII/r2b164x/0 {*}$xargs
atlasII/r2b164/1 clone atlasII/r2b164x/1 {*}$xargs
atlasII/r2b164/2 clone atlasII/r2b164x/2 {*}$xargs

atlasII/r2b164x/0 configure \
    -dualband {atlasII/r2b164x/2 -c1 36/80 -c2 161/80 -lan1 192int21 -lan2 192int22}
#############################################
# BISON04T_TWIG_7_14_131 internal (Golden2.x)
#############################################
atlasII/r2/0 clone atlasII/r2b131/0 -tag BISON04T_TWIG_7_14_131
atlasII/r2/1 clone atlasII/r2b131/1 -tag BISON04T_TWIG_7_14_131
atlasII/r2/2 clone atlasII/r2b131/2 -tag BISON04T_TWIG_7_14_131

#############################################
# BISON04T_TWIG_7_14_131 external (Golden2.x)
#############################################
atlasII/r2b131/0 clone atlasII/r2b131x/0 {*}$xargs
atlasII/r2b131/1 clone atlasII/r2b131x/1 {*}$xargs
atlasII/r2b131/2 clone atlasII/r2b131x/2 {*}$xargs

atlasII/r2b131x/0 configure \
    -dualband {atlasII/r2b131x/2 -c1 36/80 -c2 161/80 -lan1 192int21 -lan2 192int22}


################################
# private images
################################

#atlasII/r2b131/2 clone r2c0-jdbug34 -image "/projects/hnd_sw_routerdev_ext2/work/jihuac/test/vi-be/linux-131-0419-clean.trx"

#---------------------------------------------------------------------------------

