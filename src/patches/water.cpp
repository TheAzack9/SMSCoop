#include <sdk.h>
#include <macros.h>

#include <raw_fn.hxx>

#include <SMS/System/MarDirector.hxx>

// Description: Fix water height not being calculated correctly
static void TMapObjWave_initDraw_override(void* tMapObjWave) {
	if(gpMarDirector->mAreaID == 4 || gpMarDirector->mAreaID == 6) {
		updateHeightAndAlpha__11TMapObjWaveFv(tMapObjWave);
	}
	initDraw__11TMapObjWaveFv(tMapObjWave);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801dce3c, 0, 0, 0), TMapObjWave_initDraw_override);

SMS_WRITE_32(SMS_PORT_REGION(0x801dce2c, 0, 0, 0), 0x60000000);

// Fix underwater hovering when all water particles are used
SMS_WRITE_32(SMS_PORT_REGION(0x8026cd84, 0, 0, 0), 0x60000000);
