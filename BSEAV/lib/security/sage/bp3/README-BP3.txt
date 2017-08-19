BP3 Bin File

The BP3 Bin file is created and processed during provisioning.
Subsequently, the BP3 Bin file is processed after SAGE image installation on every boot.


The default location and name of the BP3 Bin file is ./bp3.bin.
The name of the file is "bp3.bin" and the directory is the current directory.

Users can customize bp3.bin file name and path by defining NEXUS_SAGE_BP3_BIN_PATH at build time to the absolute
path of the override file.

  export NEXUS_SAGE_BP3_BIN_PATH=/your/absolute/path/nexus_sage_bp3_bin_path.c
  The override file nexus_sage_bp3_bin_path.c has the name and path.
     char bp3_bin_file_name[] ="bp3.bin";
     char bp3_bin_file_path[] =".";

  cd nexus/build
  make
  cd nexus/nxclient
  make
