#ifndef B_ESTB_CFG_LIB_H
#define B_ESTB_CFG_LIB_H
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fields that needs customerization
 */
#define B_ESTB_CFG_VENDOR_MAGIC "BRCM"
#define B_ESTB_CFG_VENDOR_VER_STR "Broadcom"
#define B_ESTB_CFG_VENDOR_VER_MAJOR   (short)0
#define B_ESTB_CFG_VENDOR_VER_MINOR   (short)1
#define B_ESTB_CFG_DEFAULT_PERM_CFG "./perm.bin"
#define B_ESTB_CFG_DEFAULT_DYN_CFG "./dyn.bin"
#define B_ESTB_CFG_DEFAULT_SYS_CFG "./sys.bin"


/*
 * The b_estb_cfg uses a directory/file style to save the values.
 *
 * Externally it uses string like "/perm/estb/vendor_id" to retrieve the value.
 * Internally each value is represented uniquely by a label.
 *
 * label is a 32 bit unsigned int that structured as following
 * [24:31] : represents the config file id, mapped to the top dir, like /perm,
 *           each id corresponding to a config file.  The following is an sample mapping.
 *           | file id | path    | cfg file name |
 *           | 0x00    | "/perm" | permcfg.bin   |
 *           | 0x01    | "/dyn"  | dyncfg.bin    |
 * [0:23] : represents the individual nodes, like /perm/estb/vendor_id
 *
 * The reason we use label internally is for backward compatability. The path and name
 * of a specific variable can be changed, but the label remains the same. So
 * it's easy to import from old version  into new version.
 *
 * To building the b_estb_cfg library.
 *
 * First, developer generates a text file that describes the node-to-lable map, e.g.,
 *
 * /
 * /perm
 * /perm/estb
 * /perm/estb/vendor_id, uint32, 4, B_ESTB_CFG_LABEL_PERM_ESTB_VENDOR_ID, 0x1234
 * /dyn
 * /dyn/estb
 * /dyn/estb/dh_key, bin, 1024, B_ESTB_CFG_LABEL_DYN_ESTB_DH_KEY, "0x44 0x44 0x44 0x44"
 *
 * Second, run perl script build_b_estb_cfg.pl to generate the C data structure
 * in b_estb_cfg_values.c.
 *
 * Third, make the library.
 */

#define B_ESTB_CFG_MAKE_LABEL(a,b) ((a) << 24 | (b & 0x00ffffff))
#define B_ESTB_CFG_GET_FILE_ID(a) ((a) >>24 )


#define B_ESTB_CFG_LABEL_RSVD0 0xF0000000 /* reserved label */


/* file ID for top dir, each definition maps to a config file. */
#define B_ESTB_CFG_LABEL_PERM 0x00  /* /perm */
#define B_ESTB_CFG_LABEL_DYN  0x01  /* /dyn */
#define B_ESTB_CFG_LABEL_SYS  0x02  /* /sys */


/* individual nodes */
#define B_ESTB_CFG_LABEL_PERM_ESTB_VENDOR_ID      B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0001 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_MAC_ADDR   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0003 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_ROOT_CA_CERT   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0004 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_DEVICE_CA_CERT   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0005 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_DEVICE_CERT   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0006 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_CV_ROOT_CA_CERT   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0007 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_CV_CA_CERT   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0008 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_APP_CV_CA_CERT   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0009 )
#define B_ESTB_CFG_LABEL_PERM_ESTB_DEV_CERT_PUB_KEY   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x000a )
#define B_ESTB_CFG_LABEL_PERM_ESTB_DEV_CERT_PRIV_KEY   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x000b )
#define B_ESTB_CFG_LABEL_PERM_ESTB_DH_PRIME   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x000c )
#define B_ESTB_CFG_LABEL_PERM_ESTB_DH_BASE   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x000d )
#define B_ESTB_CFG_LABEL_PERM_ESTB_HW_VERSION_ID    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x000e )
#define B_ESTB_CFG_LABEL_PERM_ESTB_MODEL_NAME   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x000f )
#define B_ESTB_CFG_LABEL_PERM_ESTB_VENDOR_NAME    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_PERM, 0x0010 )

#define B_ESTB_CFG_LABEL_DYN_ESTB_MANU_CVC   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0001 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_COSIGNER_CVC   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0002 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_APP_MANU_CVC   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0003 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_APP_COSIGNER_CVC   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0004 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_DH_KEY   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0005 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_AUTH_KEY   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0006 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_CARD_ID   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0007 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_HOST_ID   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0008 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x000a )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x000b )
#define B_ESTB_CFG_LABEL_DYN_ESTB_GROUP_ID    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x000c )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME_1    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x000d )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME_2    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x000e )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME_3    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x000f )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME_4    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0010 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME_5    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0011 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME_6    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0012 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME_7    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0013 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME_1    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0014 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME_2    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0015 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME_3    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0016 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME_4    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0017 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME_5    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0018 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME_6    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x0019 )
#define B_ESTB_CFG_LABEL_DYN_ESTB_NEW_FW_NAME_7    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x001a )
#define B_ESTB_CFG_LABEL_DYN_ESTB_LANG_CODE		B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x001b )
#define B_ESTB_CFG_LABEL_DYN_ESTB_IMAGE_STATUS            B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x001c )
#define B_ESTB_CFG_LABEL_DYN_ESTB_DOWNLOAD_COUNT          B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x001d )
#define B_ESTB_CFG_LABEL_DYN_ESTB_DOWNLOAD_FAILED_STATUS  B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x001e )
#define B_ESTB_CFG_LABEL_DYN_ESTB_CODE_DOWNLOAD_STATUS    B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_DYN, 0x001f )

#define B_ESTB_CFG_LABEL_SYS_CFE_VER   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_SYS, 0x0001 )

/*
 * there are 2 banks of partitions in cfe, each bank contains a set of (kernel, rootfs, app, ecmboot, and docsis)
 * partition. CFE will load images from one bank. the bank number is determined by this variable
 */
#define B_ESTB_CFG_LABEL_SYS_CFE_CUR_BANK   B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_SYS, 0x0002 )

#define B_ESTB_CFG_LABEL_SYS_ESTB_REBOOT_TYPE B_ESTB_CFG_MAKE_LABEL( B_ESTB_CFG_LABEL_SYS, 0x0003 )


/* name length */
#define NAME_LEN 128

/*
 * property of variable
 */

#define NODE_FLAG_UNLIMITED_LEN 0x01  /* the length of the variable is unlimted, otherwise, field_len is the max lenght allowed */
#define NODE_TYPE_DIR 0 /* directory */
#define NODE_TYPE_UINT32 1 /* unsigned int 32 */
#define NODE_TYPE_TXT 2 /* text string */
#define NODE_TYPE_BIN 3 /* binary string */

struct b_estb_cfg_property {
	unsigned char flag;
	unsigned char type;
	unsigned int label;
	int field_len; /* max field length allowed, for fixed length variable */
	int len; /* actual length used */
	char name[NAME_LEN]; /* full path name for now */
};



/***************************************************************************
Summary:
Initialize the internal structure pointed to by dir.

Description:
This function generates initializes the internal data structure pointed to by dir.
It restores the default data value. The dir most be either "/" or one of the top
directories.

A top directory must be initialized before being opened.

Input:
	char * path - specifies the path to initialize. Must be either "/"
                      (for all data structure). Or one of the top directories
                      (for individual config file),
                      like "/perm", "/dyn".
                      .
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Init(char * dir);

/***************************************************************************
Summary:
Freeup resource.

Description:
This function frees up the resource used by b_estb_cfg module

Input:
        directory
Returns:
	>= 0 - if succeed
        < 0 - if fail

***************************************************************************/
int B_Estb_cfg_Uninit(char * dir);

/***************************************************************************
Summary:
Print all the values under a directory. (top directory only)


Description:
This function prints all the value under a directory. Currently it works for
top dirctory only.

Input:
	char * dir - the directory to list

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
void B_Estb_cfg_List(char * dir);

/***************************************************************************
Summary:
Open a config file and save it under a top directory

Description:
This function reads a config file and saves all the value under a top directory.

Input:
	char * dir - the directory to read to. Must be a top directory like "/perm"
	char * fn - config file name

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Open(char * dir, char * fn);

/***************************************************************************
Summary:
Close a config file.


Description:
This function restore the default value under a directory by closing the current
config file. Currently it works for top dirctory only.
Input:
	char * dir - the directory to read to. Must be a top directory like "/perm"

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Close(char * dir);

/***************************************************************************
Summary:
Print the value represented by name.

Description:
This function prints the value represented by name, regardless of the type
of the value.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Print(char * name);

/***************************************************************************
Summary:
Import a bin stream from a file

Description:
This function fills a bin type variable (e.g.  a certificate) with a bin file.

Input:
	char * field_name - the name of the bin variable (e.g. "/perm/estb/ca_cert")
	char * fn_name - the file that contains the raw bin data

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Import_bin_from_file(char * field_name, char * fn_name);

/***************************************************************************
Summary:
Get the uint32 variable represented by name.

Description:
This function retrieves uint32 data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/vendor_id"
        unsigned int * val - the pointer for the returned data.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Get_uint32(char * name, unsigned int * val);

/***************************************************************************
Summary:
Set the uint32 variable represented by name.

Description:
This function saves uint32 data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/vendor_id"
        unsigned int val - the new value to save.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Set_uint32(char * name, unsigned int val);

/***************************************************************************
Summary:
Get the txt variable represented by name.

Description:
This function retrieves text data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/fw_name"
        char * val - the pointer for the returned text.
        int * len - the pointer for the returned text length.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Get_txt(char * name, char * val, int * len);

/***************************************************************************
Summary:
Set the text variable represented by name.

Description:
This function saves text data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/fw_name"
        char * val - the pointer to the new text to save.
        int len  - the length of the new text to save.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Set_txt(char * name, char * val, int len);

/***************************************************************************
Summary:
Get the binary sting represented by name.

Description:
This function retrieves binary string represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
        unsigned char * val - the pointer for the returned binary.
        int * len - the pointer for the returned binary length.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Get_bin(char * name, unsigned char * val, int * len);

/***************************************************************************
Summary:
Set the binary sting represented by name.

Description:
This function saves binary string represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
        unsigned char * val - the pointer for the new binary.
        int len - the new binary string length.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Set_bin(char * name, unsigned char * val, int len);


/***************************************************************************
Summary:
Get the property of a variable.

Description:
This function retrieves variable property represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
        struct b_estb_cfg_property *prop - the property of the name

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Get_Property(char * name, struct b_estb_cfg_property * prop);

#ifdef __cplusplus
}
#endif

#endif /* B_ESTB_CFG_LIB_H */
