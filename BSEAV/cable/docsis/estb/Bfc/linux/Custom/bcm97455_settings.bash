#****************************************************************************
#
#  Copyright (c) 2004  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          16215 Alton Parkway
#          Irvine, California 92618
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
#****************************************************************************
#  $Id$
#
#  Filename:       bcm93368_settings.bash
#  Author:         Mike Sieweke
#  Creation Date:  Feb 15, 2005
#
#****************************************************************************

# Setup script specific to the bcm7455 reference design, but independent of
# any particular application.

# Load the common reference design setup script, which is also in the BFC
# OS-specific Custom directory.
source ${BfcOsCustomDir}/bcm_refdesign_common_settings.bash

echo "Setting build options from Bfc/linux/Custom/bcm97455_settings.bash"

#TODO:  ADD EJTAG SUPPORT - FOR NOW DISABLED ON LINUX BUILD
# Now add target-specific options.
app_options="${app_options} 3255 fpm nofpm_descr_only noejtag"


