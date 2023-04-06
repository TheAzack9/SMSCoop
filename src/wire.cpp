#include <sdk.h>
#include <JDrama/JDRViewObj.hxx>
#include <MTX.h>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/Player/Mario.hxx>

#include "splitscreen.hxx"
#include "player.hxx"

namespace SMSCoop {

	//int justTakenId = 0;


	//u32 TCubeManagerBase_getInCubeNo(TCubeManagerBase* cubeManagerBase, Vec& marioPos) {

	//	for(int i = 0; i < getPlayerCount(); ++i) {
	//		TMario* mario = getMarioById(i);
	//		u32 result = cubeManagerBase->getInCubeNo((Vec&)mario->mTranslation);
	//		if(result != 0xffffffff) {
	//			justTakenId = i;
	//			return result;
	//		}
	//	}
	//	return 0xffffffff;
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8019907c, 0, 0, 0), TCubeManagerBase_getInCubeNo);

	//
	//Mtx* getTakenMtx(TMario* mario) {

	//	mario = getMarioById(justTakenId);
	//	return mario->getTakenMtx();
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x801991c4, 0, 0, 0), getTakenMtx);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x801991e0, 0, 0, 0), getTakenMtx);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80198eb8, 0, 0, 0), getTakenMtx);
	//
}