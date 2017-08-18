"""Make Python treat the directory as module package."""

import os
import sys

# Figure out library import path.
facility_dir = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.realpath(__file__))))
infra_dir = os.path.dirname(facility_dir)
lib_dir = os.path.join(infra_dir, 'library')
sys.path.extend([os.getcwd(), lib_dir])
