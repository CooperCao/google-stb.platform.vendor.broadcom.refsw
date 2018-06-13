# build steps for secure rsa utility application

# set SAGE support (if not already set by default in platform settings)
export SAGE_SUPPORT=y

#build nexus
cd nexus/build
make

#build the secure rsa app
cd BSEAV/lib/security/sage/utility/examples/securersa
SECURE_RSA_TEST=1024 make
SECURE_RSA_TEST=2048 make
SECURE_RSA_TEST=3072 make
