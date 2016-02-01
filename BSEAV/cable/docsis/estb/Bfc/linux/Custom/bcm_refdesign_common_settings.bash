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
#  Filename:       bcm_refdesign_common_settings.bash
#  Author:         David Pullen
#  Creation Date:  Jan 7, 2004
#
#****************************************************************************

# Common setup script for all BRCM reference designs.

echo "Setting common build options/environment from Bfc/ecos/Custom/bcm_refdesign_common_settings.bash"

# All of our reference designs should use this company logo file in the version
# banner.
CompanyLogoFile=${BfcCustomDir}/Broadcom/BrcmRefDesignCompanyLogo.h


