# build steps for secure log application

# set SAGE support (if not already set by default in platform settings)
export SAGE_SUPPORT=y

#build nexus
cd nexus/build
make

#build the secure log library or thin layer
cd BSEAV/lib/security/sage/secure_log
make && make install

#build the secure log app
cd BSEAV/lib/security/sage/secure_log/examples/
make && make install

# Testing the secure_log_app which installs and starts Logging Utility TA and Sage Secure Framework
# once valid certificate binary is provided.
# It also creates and encrypted logs into sagelogTAtest.bin
# Example -
./nexus secure_log_app sage_ta_utility_dev.bin certificate.bin


# This sample application can be enhanced in order to enable logging for any other TAs
# Please refer Thin library for Secure Log which explains all the APIs

# Usage Guide
#-------------
#
# In order to use Secure Log for logging Sage Framework (SSF) or any TA
# Following are the requirements and API calling sequence.
# (a) First make sure to have a valid certificate
# (b) Also make sure that Valid Secure Log Type/Enable flag is in SSF header
#     or TA header for Logging to get started.i
# (c) TA itself is installed. If TA is not installed it can not be logged
#
# API Call sequence -
#  (a) First call "Secure_Log_TlConfigureBuffer" API and provide valid
#      Certificate binary file and size along with desired Logging Buffer Size
#      Currently Buffer Size is limited to Max 1MB.
#  (b) Call "Secure_Log_TlAttach" API with desired TA/Platform ID or SSF
#      (Platform Id = 0) for which logging needs to be started.
#      Note: Two conditions needs to be true for logging to really get started
#      (i) TA or SSF header should have correct Secure Log Type Flag
#      (ii) The Log Buffer is properly configured for SDL ID to which this TA ID
#           belongs.
#  (c) Call "Secure_Log_TlGetBuffer" API by providing
#       (i) "Secure_Log_TlBufferContext" which is  GLR memory to hold the Log Header
#       (ii) "pGLRBufferAddr" which points to GLR memory to hold the Encrypted Log Data
#       (iii) "GLRBufferSize" This size needs to be same as while configuring Log Buffer
#       (iv) "TA_Id" This is the Platform ID for which Data is needed.
#        Note: Encrypted Data will be for all the TA's which belongs to that SDL range
#              so even though TA ID is passed the encrypted data will contain all
#              the logs of TA's which are logging and belongs to same SDL ID.
#  (d) "Secure_Log_TlDetach" API provides flexibility to User to stop logging
#      any TA or SSF during run time.
#
#
# Simple Example:
#
# Secure_Log_TlConfigureBuffer(CertBinforSDL81, CertBinSizeSDL81, 0x1000);
# Secure_Log_TlAttach(0);
# Secure_Log_TlGetBuffer(pSecureLogBuffCtx,pGLRBufferAddr,0x1000,0);
