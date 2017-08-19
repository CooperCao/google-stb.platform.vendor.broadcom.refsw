# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id: 483d7394d7fa14c7047fc14e3b00a73c5ffbc1ba $
#

######
#
# Second UTF Vertical MIMO Coffin in 2nd floor UTF Lab
#

#set ::UTF::SummaryDir "/projects/hnd_software/utf/pb1b"

# VLANS: Ports 7, 8, 13, 14 on the internal switch are mapped
# to the same numbered ports on the external switch.  All
# other ports are on the lab net.

# External
# 10.19.84.23 sr1eth01 Netgear Switch
# 10.19.85.15 sr1end05 Endpoint - mapped to port 13
# 10.19.85.16 sr1end06 Endpoint - mapped to port 14
# 10.19.85.17 sr1end07 Endpoint - mapped to port 7
# 10.19.85.18 sr1end08 Endpoint - mapped to port 8

UTF::Linux SR1End07 -sta {uplan eth1}

# Internal
# 10.19.84.39 pb1beth1 Netgear Switch
# 10.19.84.52 pb1bips1 available
# 10.19.84.66 pb1bnpc1 available
# 10.19.84.88 pb1btst2 available
# 10.19.84.89 pb1btst3 available
# 10.19.84.90 pb1btst4 available
# 10.19.84.91 pb1btst5 available
# 10.19.84.92 pb1btst6 available

UTF::Power::WTI pb1bips1


