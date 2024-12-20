/// <summary>
/// Overrides logic for loading and updating an extra camera.
/// Thanks to Mr. Brocoli for quite a bit of this code.
/// </summary>

#include <JSystem/JDrama/JDRPlacement.hxx>

#include <SMS/Player/Mario.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/raw_fn.hxx>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/Camera/CameraMarioData.hxx>
#include <memory.hxx>
#include <sdk.h>
#include <string.h>

#include "players.hxx"
#include "splitscreen.hxx"
#include "settings.hxx"
#include "pvp.hxx"

#define MARIO_COUNT 2
extern SMSCoop::CameraTypeSetting gCameraTypeSetting;

namespace SMSCoop {
	CPolarSubCamera* cameras[MARIO_COUNT];

	static TCameraMarioData* c1[4] = { 0, 0, 0, 0 }; // odd camera infor for -0x7110 0f sda
	static u32 c2[4] = { 0, 0, 0, 0 }; // odd camera infor for -0x7108 0f sda

	static CPolarSubCamera** gpCameraCoop = (CPolarSubCamera**)0x8040d0a8;
	static TCameraMarioData** gpCameraMario = (TCameraMarioData**)0x8040D0B0;
	static CPolarSubCamera** gpCameraShake = (CPolarSubCamera**)0x8040D0B8;
	
	f32 MultiplayerCameraRho = 0.0f;
	s16 MultiplayerCameraPhi = 0;
	s16 MultiplayerCameraTheta = 0;


	// Description: Sets the current global camera instance
	void setCamera(int i) {
		*gpCameraCoop = cameras[i];
		// This is mario position lmao... 0x80025690
		// This is also some pre-computed camera stuff. Used in calculation of lens glow
		*gpCameraMario = c1[i];
		*gpCameraShake = (CPolarSubCamera*)c2[i];
	}

	CPolarSubCamera* getCameraById(int i) {
		return cameras[i];
	}

	inline void SDAstoreword(int offset, u32 val) {
		__asm("stw %0, %1(13)" :: "r" (val), "X" (offset));
	}

	inline int SDAbyte(int offset) {
		int toReturn;
		__asm("lbz %0, %1(13)" : "=r" (toReturn) : "X" (offset));
		return toReturn;
	}

	// Description: Loads all cameras from stream. 
	// TODO: Cleanup
	void loadCameraInfo(u32* camera, u32* unk, u32* bleh1, u32* bleh2, u32** gadgetNode) {
		u32* susNode = gadgetNode[0];
		for (int i = 1; i < getPlayerCount(); i++) {
			u32* newNode = (u32*)__nw__FUl(12);
			newNode[2] = (u32)cameras[i];
			newNode[1] = (u32)gadgetNode;
			newNode[0] = (u32)susNode;
			susNode[1] = (u32)newNode;
			gadgetNode[0] = newNode;
			gadgetNode = (u32**)newNode;
		}

		int a = unk[1], b = unk[3], c = unk[4];
		for (int i = 0; i < getPlayerCount(); i++) {
			unk[1] = a;
			unk[3] = b;
			unk[4] = c;
			//*gpMarioOriginal = (TMario*)marios[i];
			setActiveMario(i);
			setCamera(i);
			*gpCameraMario = c1[i];
			SDAstoreword(-0x7108, c2[i]);
			load__Q26JDrama10TPlacementFR20JSUMemoryInputStream(cameras[i], unk);
		}
		*gpCameraMario = c1[0];
		SDAstoreword(-0x7108, c2[0]);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}


	// Description: Updates all cameras and assigns correct controller to each camera.
	// Note: The active perspective is set as the last one. This is to make the game use that camera for rendering the scene since that is the transform last copied to graphics
	// TODO: Cleanup
	// TODO: Fix particles being clipped when outside viewport
	void performCamerasOverhaul(CPolarSubCamera* camera, u32 param_1, JDrama::TGraphics* graphics) {
		TApplication* app = &gpApplication;
		bool focusedCamera = isFocusedCamera();

		if(focusedCamera) {
			int focusedPlayer = getFocusedPlayer();
			setActiveMario(focusedPlayer);
			CPolarSubCamera* pCamera = cameras[0];
			((u32*)pCamera)[0x120 / 4] = (u32)app->mGamePads[focusedPlayer];
			perform__15CPolarSubCameraFUlPQ26JDrama9TGraphics(pCamera, param_1, graphics);
			((u32*)pCamera)[0x120 / 4] = (u32)app->mGamePads[getActiveViewport()];
			setActiveMario(getActiveViewport());
			return;
		}

		for (int i = getPlayerCount()-1; i >= 0; i--) {
			if(i == getActiveViewport()) continue;
			CPolarSubCamera* pCamera = cameras[i];
			setActiveMario(i);
			setCamera(i);
			((u32*)pCamera)[0x120 / 4] = (u32)app->mGamePads[i];
			perform__15CPolarSubCameraFUlPQ26JDrama9TGraphics(pCamera, param_1, graphics);
		}
	
		int i = getActiveViewport();
		

		CPolarSubCamera* pCamera = cameras[i];
		setActiveMario(i);
		setCamera(i);
		((u32*)pCamera)[0x120 / 4] = (u32)app->mGamePads[i];
		perform__15CPolarSubCameraFUlPQ26JDrama9TGraphics(pCamera, param_1, graphics);

		// Custom controls for the retro camera
		if(isSingleCameraLevel() && !isFocusedCamera()) {
			for(int i = 0; i < getPlayerCount(); ++i) {

				MultiplayerCameraPhi += gpApplication.mGamePads[i]->mCStick.mStickX * 40.0;
				MultiplayerCameraRho += gpApplication.mGamePads[i]->mCStick.mStickY * 2.0;
				MultiplayerCameraTheta += gpApplication.mGamePads[i]->mCStick.mStickY * 40.0;
			}

			
			if (MultiplayerCameraTheta > 15000)
				MultiplayerCameraTheta = 15000;
			if (MultiplayerCameraTheta < 0)
				MultiplayerCameraTheta = 0;

		}
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	} 

	// Custom override of multiplayer cam to actually make it semi usable
	// Allows horizontal and vertical movement of the camera.
	// Ported straight from original SM mod
	void CalcMultiplayerCameraPos(TVec3f* center, TVec3f* out, float rho, u16 theta, u16 phi) {
		/*switch (CameraMode){
			case 2: rho *= 0.667f; break;
			case 1: rho *= 1.5f; break;
		}*/
		CLBPolarToCross__FRC3VecP3Vecfss(center, out, rho * 0.8 + 10 * (f32)sqrtf(fabsf((f32)MultiplayerCameraTheta)), MultiplayerCameraTheta, MultiplayerCameraPhi);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80030c04, 0, 0, 0), CalcMultiplayerCameraPos);

	// Description: Runs loadAfter for all cameras
	// TODO: Cleanup
	void loadAfterCameraOverhaul(CPolarSubCamera* camera) {
		for (int i = 0; i < getPlayerCount(); i++) {
			*gpCameraMario = c1[i];
			*gpCameraShake = (CPolarSubCamera*)c2[i];
			loadAfter__15CPolarSubCameraFv(cameras[i]);

		}
		*gpCameraMario = c1[0];
		*gpCameraShake = (CPolarSubCamera*)c2[0];
	
		CPolarSubCamera* originalCam = cameras[0];
		f32 originalAspect = originalCam->mProjectionAspect;
		for (int i = 0; i < getPlayerCount(); i++) {
			cameras[i]->mProjectionAspect = originalAspect;
			//cameras[i]->mProjectionFovy *= 1.8f;
			if(!isSingleCameraLevel() && gCameraTypeSetting.getInt() == CameraTypeSetting::HORIZONTAL) {
				cameras[i]->mProjectionAspect = 2.666666666f;
			}

		}
	}


	static u32* cameraVtable = (u32*)0x803acde8; // 0x10 is where the load camera stuff 802fb7a0 is called i guess

	// Description: Overrides constructor of CPolarSubCamera, overrides vtable with custom functions for loading the cameras
	// TODO: Cleanup
	void makeCameras(CPolarSubCamera* camera, char* unk) {
		cameraVtable[4] = (u32)(&loadCameraInfo);
		cameraVtable[6] = (u32)(&loadAfterCameraOverhaul);
		cameraVtable[8] = (u32)(&performCamerasOverhaul);
		
		MultiplayerCameraRho = 0.0f;
		MultiplayerCameraPhi = 0;
		MultiplayerCameraTheta = 0;

		cameras[0] = camera;
		__ct__15CPolarSubCameraFPCc(camera, unk);
		u32* bob = (u32*)(0x804141c0 - 0x7110);
		c1[0] = *gpCameraMario;
		c2[0] = (u32)bob[2];
		for (int i = 1; i < getPlayerCount(); i++) {
			// memory leak?
			cameras[i] = (CPolarSubCamera*)__nw__FUl(1020);
			__ct__15CPolarSubCameraFPCc(cameras[i], unk);
			u32* bob = (u32*)(0x804141c0 - 0x7110);
			c1[i] = *gpCameraMario;
			c2[i] = (u32)bob[2];
		}
		*gpCameraMario = c1[0];

		// returning different cameras does nothing
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029d78c, 0, 0, 0), makeCameras);

	// Description: Overrides setNoticeInfo call and runs it for each camera. 
	// TODO: Look more into what this exactly does, seems to be related too gooper blooper fight
	// TODO: Cleanup
	void setNoticeInfoCameras(CPolarSubCamera* camera) {
		for (int i = 0; i < getPlayerCount(); i++) {
			*gpCameraMario = c1[i];
			SDAstoreword(-0x7108, c2[i]);
			cameras[i]->setNoticeInfo();
		}
		*gpCameraMario = c1[0];
		SDAstoreword(-0x7108, c2[0]);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802b8f10, 0, 0, 0), setNoticeInfoCameras);

	// Description: overrides the gpCamera used for viewCalcSimple.
	// This is used for e.g trees and such when transforming from world transform to view transform
	CPolarSubCamera* getCamera() {
		return cameras[getActiveViewport()];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8023d3c4, 0, 0, 0), getCamera);

	char** cameraInside = SMS_PORT_REGION((char**)0x8040c1e8, 0, 0, 0);
	char** cameraOutside = SMS_PORT_REGION((char**)0x8040c1ec, 0, 0, 0);

	// Description: When demo starts/ends, start/end for all cameras.
	// TODO: Make it based on individual controllers input press
	void CPolarSubCamera_StartDemoCamera_Override(CPolarSubCamera* p1Camera, char* filename, TVec3f* position, s32 param_3, f32 param_4, bool param_5) {

		bool isShineDemoCamera = strcmp(filename, *cameraInside) == 0 || strcmp(filename, *cameraOutside) == 0;
		//if(isShineDemoCamera) {
		//	setShineCutscene(true);
		//}
		
		for (int i = 0; i < getPlayerCount(); i++) {
			CPolarSubCamera* camera = (CPolarSubCamera*)cameras[i];
			//if(!isShineDemoCamera || (isShineGot() && getMarioThatPickedShine() == i)) {
				camera->startDemoCamera(filename, position, param_3, param_4, param_5);
			//}
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80297f7c, 0, 0, 0), CPolarSubCamera_StartDemoCamera_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802981a8, 0, 0, 0), CPolarSubCamera_StartDemoCamera_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029839c, 0, 0, 0), CPolarSubCamera_StartDemoCamera_Override);

	void CPolarSubCamera_EndDemoCamera_Override(CPolarSubCamera* camera) {
		for (int i = 0; i < getPlayerCount(); i++) {
			CPolarSubCamera* camera = (CPolarSubCamera*)cameras[i];
			camera->endDemoCamera();
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80297f34, 0, 0, 0), CPolarSubCamera_EndDemoCamera_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80297fd4, 0, 0, 0), CPolarSubCamera_EndDemoCamera_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80298cac, 0, 0, 0), CPolarSubCamera_EndDemoCamera_Override);

	// Description: Sets up each camera as it's own sound listener. 
	void MSoundSESystem_MSoundSE_checkSoundArea(JAIBasic* jaiBasic, TVec3f* param_1, TVec3f* param_2, float* param_3, u32 sourceIdx) {
		for (int i = 0; i < getPlayerCount(); i++) {
			CPolarSubCamera* camera = (CPolarSubCamera*)cameras[i];
			setCameraInfo__8JAIBasicFP3VecP3VecPA4_fUl(jaiBasic, camera + 0x124, camera + 0x13c, camera->mTRSMatrix, i);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x800153b8, 0, 0, 0), MSoundSESystem_MSoundSE_checkSoundArea);
	SMS_WRITE_32(SMS_PORT_REGION(0x8040cd70, 0, 0, 0), 0x2); // Number of audio sources

	// Description: Goes through all audio listener and finds the closest one to the audio source
	void calculateSoundDistance(const Mtx matrix, const Vec *source, Vec *dest) {
		Vec distance = *dest;
		f32 fdist = 0.0;

		PSMTXMultVec(matrix, source, &distance);
		fdist = PSVECMag(&distance);

		for(int i = 1; i < getPlayerCount(); ++i) {
			Vec newDist;
			CPolarSubCamera* camera = (CPolarSubCamera*)cameras[i];
			PSMTXMultVec(camera->mTRSMatrix, source, &newDist);
			f32 newMag = PSVECMag(&newDist);
			if(fdist > newMag) {
				fdist = newMag;
				distance = newDist;
			}
		}
		*dest = distance;

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80305418, 0, 0, 0), calculateSoundDistance);

	//void calculateInCubeRation(void* gpCubeSoundChange, Vec* marioPos, )
	
	// Description: Goes through all audio listener and finds the closest one to the audio source
	int SMS_GetMonteVillageAreaInMario_camera() {
		int result = 3;
		return result; // This probably messes up music or something in pianta, but it seems to fix sounds
		//for(int i = getPlayerCount() - 1; i >= 0; --i) {
		//	TMario* mario = getMario(i);
		//	gpCubeFastC->mCurrentCube = gpCubeFastC->getInCubeNo((const Vec&)mario->mTranslation);
		//	setCamera(i);
		//	
		//	int newResult = SMS_GetMonteVillageAreaInMario__Fv();
		//	if(newResult != 3) {
		//		result = newResult;
		//	}
		//}
		//return result;
		//if(result == 4 || result == 1) return result;

		//u32 isInCube = getInCubeNo__16TCubeManagerBaseCFRC3Vec(gpCubeFastC, gpMarioPos);
		//if(isInCube == 1) return 0;
		//if(isInCube == 0) return 2;
		//return 3;

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80018598, 0, 0, 0), SMS_GetMonteVillageAreaInMario_camera);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8002046c, 0, 0, 0), SMS_GetMonteVillageAreaInMario_camera);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802b9fd0, 0, 0, 0), SMS_GetMonteVillageAreaInMario_camera);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802bada0, 0, 0, 0), SMS_GetMonteVillageAreaInMario_camera);
	
}