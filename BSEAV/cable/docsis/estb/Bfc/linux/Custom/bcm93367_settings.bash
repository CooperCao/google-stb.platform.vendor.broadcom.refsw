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
#  Filename:       bcm93367_settings.bash
#  Author:         Mike Sieweke
#  Creation Date:  Feb 15, 2005
#
#****************************************************************************

# Setup script specific to the bcm93367 reference design, but independent of
# any particular application.

# Load the common reference design setup script, which is also in the BFC
# OS-specific Custom directory.
source ${BfcOsCustomDir}/bcm_refdesign_common_settings.bash

echo "Setting build options from Bfc/ecos/Custom/bcm93367_settings.bash"

# Now add target-specific options.
app_options="${app_options} 3368 fpm nofpm_descr_only spiflash ejtag"


