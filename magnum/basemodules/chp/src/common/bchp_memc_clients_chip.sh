#!/bin/sh
# Script to update mapping for all listed chips
set -e
set -u
while read chip rev ; do
    test -f magnum/basemodules/chp/include/$chip/common/memc/bchp_memc_clients_chip$rev.txt && perl magnum/basemodules/chp/src/common/bchp_memc_clients_chip.pl magnum/basemodules/chp/include/$chip/common/memc/bchp_memc_clients_chip$rev.txt magnum/basemodules/chp/include/$chip/common/memc/bchp_memc_clients_chip$rev.h
    perl magnum/basemodules/chp/src/common/bchp_memc_clients_chip_map.pl magnum/basemodules/chp/src/common/bchp_memc_clients_chip_map_all.h magnum/basemodules/chp/include/$chip/common/memc/bchp_memc_clients_chip$rev.h magnum/basemodules/chp/include/$chip/common/memc/bchp_memc_clients_chip_map$rev.h
done << _END_  
7231
7346
73465
7358
7360
7366
7425
7429
7435 _A0 
7435 _B0
7445
7552
75525
7563
75635
7584
75845
_END_
# 'softlinked' 7543

