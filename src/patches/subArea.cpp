#include <sdk.h>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/JointObj.hxx>
#include <System/GameSequence.hxx>
#define NTSCU
#include <raw_fn.hxx>
#include <MTX.h>
#include <SMS/Camera/CubeManagerBase.hxx>
 

#include "camera.hxx"
#include "splitscreen.hxx"
#include "players.hxx"

#define MARIO_COUNT 2

namespace SMSCoop {
	
	TJointObj* awakenedObjects[MARIO_COUNT];
	u32 playerPreviousWarpId[MARIO_COUNT];

	void resetUpSubArea(TMarDirector* marDirector) {
		for(int i = 0; i < MARIO_COUNT; ++i) {
			awakenedObjects[i] = nullptr;
			playerPreviousWarpId[i] = 0;
		}
	}

	void changeWarp(TJointObj* obj, u8 player) {
		if(awakenedObjects[player]) {
			awakenedObjects[player]->sleep();
		}

		awakenedObjects[player] = obj;
		obj->awake();
	}

	void changePerspective(u8 player) {
		if(!awakenedObjects[player]) return;
		//OSReport("Changing perspective %d %d %X \n", player, playerPreviousWarpId[player], awakenedObjects[player]);
		for(int i = 0; i < MARIO_COUNT; ++i) {
			if(i == player || !awakenedObjects[i]) continue;
			awakenedObjects[i]->sleep();
		}
		awakenedObjects[player]->awake();
	}

	void watchToWarp_override(TMapWarp* warp) {
		u8 currentMarioId = getActiveViewport();
		TMario* currentMario = getMario(currentMarioId);
		// Cannot warp while being held by a mario
		for(int j = 0; j < getPlayerCount(); ++j) {
			if(currentMario->mHolder == getMario(j)) {
				return; 
			}
		}
		changePerspective(currentMarioId);
		warp->mPrevID = playerPreviousWarpId[currentMarioId];

		watchToWarp__8TMapWarpFv(warp);
		playerPreviousWarpId[currentMarioId] = warp->mPrevID;
		for(int j = 0; j < getPlayerCount(); ++j) {
			if(getMario(currentMarioId)->mHeldObject == getMario(j)) {
				playerPreviousWarpId[j] = warp->mPrevID;
				awakenedObjects[j] = awakenedObjects[currentMarioId];
			}
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80189760, 0, 0, 0), watchToWarp_override);

	// Description: Try fix warps + some objects being hidden (put to sleep) on certain collision types
	// Note: I have up fixing this for sirena for now due to it being updated based on mario position and many other things. This fix is mainly for e.g rooms in delfino
	// Future fix: Keep track of all stage items for each player somehow (Potentially keeping a set of sleeping and awake items for each player)
	void TMap_perform_watchToWarp_override(TMap* tMap, u32 performFlags, JDrama::TGraphics* graphics) {
		
		if(performFlags & 0x1) {
		
			// Optimization: Only swap if perspective has changed
			//OSReport("TMap perform perspective %X %X %X %X %X\n", currentMario, playerPreviousWarpId[0], playerPreviousWarpId[1], tMap->mMapWarp->mCurrentID, tMap->mMapWarp->mPrevID);
			
			perform__4TMapFUlPQ26JDrama9TGraphics(tMap, performFlags, graphics);
			//OSReport("Previous set %X\n", tMap->mMapWarp->mPrevID);
		
		} else {
			perform__4TMapFUlPQ26JDrama9TGraphics(tMap, performFlags, graphics);
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c1678, 0, 0, 0), (u32)&TMap_perform_watchToWarp_override);
	
	void TMap_load_override(TMap* map, void* memStream) {
		load__4TMapFR20JSUMemoryInputStream(map, memStream);
		pushToPerformList(map, 0x3);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c1668, 0, 0, 0), (u32)&TMap_load_override);

	//void initUnderpass_override(JDrama::TViewObj* mapModel) {
	//	initUnderpass__9TMapModelFv(mapModel);
	//	pushToPerformList(mapModel, 0x2);
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x801945a8, 0, 0, 0), initUnderpass_override);

	void TWarpAreaActor_load_override(JDrama::TViewObj* warpAreaActor, void* memStream) {
		load__14TWarpAreaActorFR20JSUMemoryInputStream(warpAreaActor, memStream);
		pushToPerformList(warpAreaActor, 0x1);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803cd604, 0, 0, 0), (u32)&TWarpAreaActor_load_override);

	// Description: Override sleep collision to track which one was put to sleep and which one was set awake
	void TJointObj_awake_watchToWarp_override(TJointObj* jointObj) {
		int i = getActiveViewport();
		changeWarp(jointObj, i);

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8019535c, 0, 0, 0), TJointObj_awake_watchToWarp_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80195480, 0, 0, 0), TJointObj_awake_watchToWarp_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8019561c, 0, 0, 0), TJointObj_awake_watchToWarp_override);

	void TJointObj_sleep_watchToWarp_override(TJointObj* jointObj) {
		// If we have not yet had a switch we need to just execute regular behaviour since we don't know what object to sleep
		// TODO: Get this from map instead at load.
		if(!awakenedObjects[0] && !awakenedObjects[1]) {
			// Depending on what player enters a subArea first, the other player will still be on the other side (probably)
			if(!awakenedObjects[0]) {
				awakenedObjects[1] = jointObj;
			}
			if(!awakenedObjects[1]) {
				awakenedObjects[0] = jointObj;
			}
			jointObj->sleep();
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80195330, 0, 0, 0), TJointObj_sleep_watchToWarp_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80195454, 0, 0, 0), TJointObj_sleep_watchToWarp_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x801955f0, 0, 0, 0), TJointObj_sleep_watchToWarp_override);
	
	void TMapWarp_changeModel(TMapWarp* warp, u32 model) {
		changeModel__8TMapWarpFi(warp, model);
		
		u8 currentMario = getActiveViewport();
		playerPreviousWarpId[currentMario] = warp->mPrevID;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80189818, 0, 0, 0), TMapWarp_changeModel);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80189ce4, 0, 0, 0), TMapWarp_changeModel);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80189cfc, 0, 0, 0), TMapWarp_changeModel);
	
	// Description: Override TCubeManagerBase_getInCubeNo to render if mario that is currently rendered is inside fastCubes (used for fast culling checks)
	// Note: This will cause objects that are culled for only one player to have their animations run in 30 fps since it only updates on one screen (this looks wonky) Very noticable in Sirena when player is on different floor
	//u32 TCubeManagerBase_getInCubeNo_Perspective_Fix(TCubeManagerBase* cubeManagerBase, Vec& marioPos) {
	//	if(gpMarDirector->mAreaID == TGameSequence::Area::AREA_MAREBOSS) {
	//		return cubeManagerBase->getInCubeNo((Vec&)getMario(0)->mTranslation);
	//	}

	//	int i = getActiveViewport();
	//	TMario* mario = marios[i];
	//	return cubeManagerBase->getInCubeNo((Vec&)mario->mTranslation);
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8024d460, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8024d474, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8024d488, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8024d49c, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80195490, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);

	// Description: Override TCubeManagerBase_getInCubeNo to render if mario that is currently rendered is inside fastCubes (used for fast culling checks)
	// Note: This will cause objects that are culled for only one player to have their animations run in 30 fps since it only updates on one screen (this looks wonky) Very noticable in Sirena when player is on different floor
	u32 TCubeManagerBase_getInCubeNo_Perspective_Fix2(TCubeManagerBase* cubeManagerArea, Vec& objPos) {
		if(gpMarDirector->mAreaID == TGameSequence::Area::AREA_MAREBOSS) {
			return isInAreaCube__16TCubeManagerAreaCFRC3Vec(cubeManagerArea, objPos);
		}
		//return true;
		u32 result = 0;
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMario(i);
		
			u32 cubeNo = cubeManagerArea->getInCubeNo((Vec&)mario->mTranslation);
			*(u32*)((u32)cubeManagerArea + 0x1c) = cubeNo;
			u32 newResult = isInAreaCube__16TCubeManagerAreaCFRC3Vec(cubeManagerArea, objPos);
			if(newResult > result) result = newResult;
			//if(result) return result;
		}
		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021afec, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix2);
}