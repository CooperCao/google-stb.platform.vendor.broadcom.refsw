#ifndef BTNR_DTT75409_H__
#define BTNR_DTT75409_H__

#include "bchp.h"
#include "breg_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	Required default settings structure for DTT75409 Tuner module.

Description:
	The default setting structure defines the default configuration of
	DTT75409 tuner when the device is opened.

See Also:
	BTNR_DTT75409_Open(), BTNR_DTT75409_GetDefaultSettings()

****************************************************************************/
#define BTNR_DTT75409_SETTINGS_IFFREQ			(36000000)		/* 36.00 MHz */
#define BTNR_DTT75409_SETTINGS_I2CADDR			(0x60)			/* 7bit addr. 0x60 to 0x63 depending on the way the lines are pulled high or low. */
typedef struct
{
	unsigned long ifFreq;				/* in Hertz */
	unsigned short i2cAddr; 			/* 7bit I2C address of DTT75409 */
} BTNR_DTT75409_Settings;


/***************************************************************************
Summary:
	This function opens DTT75409 Tuner module.

Description:
	This function is responsible for opening DTT75409 BTNR module. When BTNR is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BTNR_Close(), BTNR_DTT75409_GetDefaultSettings()

****************************************************************************/
BERR_Code BTNR_DTT75409_Open(
	BTNR_Handle *phDev, 				/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BTNR_DTT75409_Settings *pDefSettings /* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function returns the default settings for DTT75409 Tuner module.

Description:
	This function is responsible for returns the default setting for 
	BTNR module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BTNR_DTT75409_Open()

****************************************************************************/
BERR_Code BTNR_DTT75409_GetDefaultSettings(
	BTNR_DTT75409_Settings *pDefSettings,	/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);


#ifdef __cplusplus
}
#endif
 
#endif

