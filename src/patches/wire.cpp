#include <sdk.h>
#include <JDrama/JDRViewObj.hxx>
#include <MTX.h>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/Player/Mario.hxx>

#include "splitscreen.hxx"
#include "players.hxx"
#include "raw_fn.hxx"

//extern SMSCoop::SplitScreenSetting gSplitScreenSetting;

namespace SMSCoop {

	int justTakenId = 0;

	// TODO: This seems weird and should probably be fixed...
	void TMapWireManager_loadAfter(void* wireManager) {
		int playerCount = getPlayerCount();
		for(int i = 0; i < playerCount; ++i) {
			int activeMario = 1-i;
			/*
			if(gSplitScreenSetting.getInt() == SplitScreenSetting::NONE) {
				activeMario = 0;
			}*/
			// Only happens if no mario
			if(activeMario > playerCount) activeMario = 0;
			setActiveMario(activeMario);
			loadAfter__15TMapWireManagerFv(wireManager);
		}
		setActiveMario(0);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c1cd0, 0, 0, 0), (u32)(&TMapWireManager_loadAfter));

	void THitActor_initHitActor_wire(THitActor* wire, u32 actorId, u16 param_2, int param_3, f32 param_4, f32 param_5, f32 param_6, f32 param_7) {

		wire->initHitActor(actorId, 2, param_3, param_4, param_5, param_6, param_7);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80198c38, 0, 0, 0), THitActor_initHitActor_wire);
	
	TMario* currentMario = nullptr;
	//u32 TCubeManagerBase_getInCubeNo(TCubeManagerBase* cubeManagerBase, Vec& marioPos) {

	//	for(int i = 0; i < getPlayerCount(); ++i) {
	//		TMario* mario = getMarioById(i);
	//		u32 result = cubeManagerBase->getInCubeNo((Vec&)mario->mTranslation);
	//		if(result != 0xffffffff) {
	//			justTakenId = i;
	//			/*if(i != 0) {
	//				getMarioById(i)->receiveMessage();
	//			}*/
	//			return result;
	//		}
	//	}
	//	return 0xffffffff;
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8019907c, 0, 0, 0), TCubeManagerBase_getInCubeNo);

	void TMapWireActorManager_doActorToWire(void* wireManager) {
		currentMario = *(TMario**)wireManager;
		doActorToWire__20TMapWireActorManagerFv(wireManager);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80198e44, 0, 0, 0), TMapWireActorManager_doActorToWire);
	
	Mtx* getTakenMtx(TMario* mario) {
		return currentMario->getTakenMtx();
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801991c4, 0, 0, 0), getTakenMtx);
	SMS_PATCH_BL(SMS_PORT_REGION(0x801991e0, 0, 0, 0), getTakenMtx);

	
	inline Mtx* getTakenMtx2(TMario* mario) {
		TMario* mario2;
		SMS_ASM_BLOCK("lwz %0, 0x0 (28)" : "=r"(mario2));
		return mario2->getTakenMtx();
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80198eb8, 0, 0, 0), getTakenMtx2);
	
	int TMario_receiveMessage_stickToWire(TMario* mario, THitActor* wire, u32 msg) {

		for(int i = 0; i < getPlayerCount(); ++i) {
			if(getMario(i)->mHolder == wire) return 0;
		}
		return mario->receiveMessage(wire, msg);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80199130, 0, 0, 0), TMario_receiveMessage_stickToWire);
}