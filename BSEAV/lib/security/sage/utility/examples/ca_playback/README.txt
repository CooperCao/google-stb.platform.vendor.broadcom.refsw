# build steps for ca_playback utility application

# set SAGE support (if not already set by default in platform settings)
export SAGE_SUPPORT=y

#build nexus
cd nexus/build
make

#build the ca_playback app
cd BSEAV/lib/security/sage/utility/examples/ca_playback
make
