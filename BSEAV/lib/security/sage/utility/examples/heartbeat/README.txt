# build steps for heartbeat utility application

# set SAGE support (if not already set by default in platform settings)
export SAGE_SUPPORT=y

#build nexus
cd nexus/build
make

#build the heartbeat app
cd BSEAV/lib/security/sage/utility/examples/heartbeat
make
