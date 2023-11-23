#include <sdk.h>
#include <types.h>
#include <string.h>

#define NTSCU
#include <raw_fn.hxx>
#include <JDrama/JDRNameRef.hxx>
#include <JDrama/JDRViewObjPtrListT.hxx>
#include <SMS/Strategic/HitActor.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Manager/FlagManager.hxx>

#include "characters.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"

#define MARIO_COUNT 2

static TMario** gpMarioOriginalCoop = (TMario**)0x8040e0e8; // WTF?
static TMario** gpMarioForCallBackCoop = (TMario**)0x8040e0e0; // WTF?

namespace SMSCoop {
	static u8 loadedMarios = 0;
	static TMario* marios[MARIO_COUNT];
	static bool isMarioCurrentlyLoadingViewObj = false;

	void setActiveMario(int id) {
		TMario* mario = marios[id];
		gpMarioOriginal = mario;
		*gpMarioForCallBackCoop = mario;
		*gpMarioOriginalCoop = mario;
		SMS_SetMarioAccessParams__Fv();
	}
	
	u8 getPlayerId(TMario* mario) {
		for(u8 i = 0; i < loadedMarios; ++i) {
			if(mario == marios[i]) return i;
		}
		return 0;
	}
	
	int getPlayerCount() {
		return loadedMarios;
	}

	TMario* getMario(int id) {
		return marios[id];
	}
	
	int getClosestMarioId(TVec3f* position) {
		float closest0 = PSVECSquareDistance((Vec*)position, (Vec*)&marios[0]->mTranslation);
		if(loadedMarios == 1) return 0;
		float closest1 = PSVECSquareDistance((Vec*)position, (Vec*)&marios[1]->mTranslation);
		return closest0 > closest1;
	}
	
	// Description: Overrides mario loading to allow for loading a luigi game object in level
	// TODO: Make this optional based on setting
	int TMarioStrCmp_Override(const char* nameRef, void* str) {
		int isMarioCheck = strcmp(nameRef, "Mario");
		if(isMarioCheck == 0) {
			loadedMarios = 0;
			isMarioCurrentlyLoadingViewObj = true;
		}
		return isMarioCheck;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029d7d8, 0, 0, 0), TMarioStrCmp_Override);

	// Description: We hook into the load to get a pointer to TViewObjPtrListT to be able to add viewObjs manually
	JDrama::TViewObjPtrListT<THitActor,JDrama::TViewObj>* mario_viewObjPtrList = 0;
	void JDrama_TViewObjPtrListT_load(JDrama::TViewObjPtrListT<THitActor,JDrama::TViewObj>* viewObjPtrList,JSUMemoryInputStream *param_1) {
		mario_viewObjPtrList = viewObjPtrList;
		load__Q26JDrama47TViewObjPtrListT_9(viewObjPtrList, param_1);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80223548, 0, 0, 0), JDrama_TViewObjPtrListT_load);
	
	#define load__6TMarioFR20JSUMemoryInputStream         ((int (*)(...))0x80276BD0)
	// Description: The loading of the mario nameRef section.
	// Optimization: See if we can re-use the instance created in genObject instead of deleting.
	u32 JDrama_TNameRefGen_load_Mario(JDrama::TNameRef* refGen, JSUMemoryInputStream* memoryStream, JSUMemoryInputStream* memoryStream2) {
		u32 response = genObject__Q26JDrama8TNameRefFR20JSUMemoryInputStreamR20JSUMemoryInputStream(refGen, memoryStream, memoryStream2);
		if(isMarioCurrentlyLoadingViewObj) {
			/*if(!isSplitscreen()) {
				isMarioCurrentlyLoadingViewObj = false;
				return response;
			}*/
			// Read the map data for mario and store it in a temp buffer to be able to reuse memoryStream.
			// We cannot set the position since this is the direct io stream, so we need to temp store it.
			char* buffer[73];
			memoryStream->readData(buffer, 73);

			// Create marios and load them
			for(int i = 0; i < MARIO_COUNT; ++i) {
				memoryStream->setBuffer(buffer, 73);
				TMario* mario = (TMario*) response;
				marios[i] = mario;

				if(i != 0) {
					mario = new TMario();
				}
				mario->load(*memoryStream);
				
				if(i != 0) {
					mario->mKeyName = "Luigi";
					mario->mKeyCode = JDrama::TNameRef::calcKeyCode("Luigi");
				}

				mario_viewObjPtrList->mViewObjList.push_back(mario);
			}

			//loadedMarios = 1;
			//unmountActiveMarioArchive();
			// We cleanup the unused mario that is just used as a locator
			//TMario* mario = (TMario*)response;
			//delete mario;
		
			isMarioCurrentlyLoadingViewObj = false;
			return 0x0;
		}
		return response;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802a0734, 0, 0, 0), JDrama_TNameRefGen_load_Mario);

	// Description: Override load method for mario to set correct gamepad for p2 and load correct model
	void loadMario(TMario* mario, JSUMemoryInputStream *input) {

		load__Q26JDrama6TActorFR20JSUMemoryInputStream(mario, input);
	
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		*((u32*)&director->mGamePads[loadedMarios]->mState) &= ~0x80000; // Player is not talking

		marios[loadedMarios] = mario;
	
		if(loadedMarios >= 1) {
			TApplication* app = &gpApplication;
			app->mGamePads[loadedMarios]->_E0 = 2;
			mario->setGamePad(app->mGamePads[loadedMarios]);
			mario->mController = app->mGamePads[loadedMarios];
		
		}

		setActiveMarioArchive(loadedMarios);
		setActiveMario(0);
		loadedMarios++;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80276BF0, 0, 0, 0), loadMario);

	// Description: Override the controller update to ensure that correct mario is checked.
	// Note: Certain things like the y-cam is for some reason tied directly to the controller update.
	// IMPROVEMENT: I should just re-implement the entire loop at 0x80299a24 which is what calls this instead
	void TMarioGamePad_updateMeaning_override(TMarioGamePad* gamepad) {
		TApplication* app = &gpApplication;
		// A bit of a jank way to check if it is player 2 sadly
		for(int i = 0; i < MARIO_COUNT; ++i) {
			if(loadedMarios > i && gamepad == app->mGamePads[i]) {
				setActiveMario(i);
				setCamera(i);
			}
		}
		updateMeaning__13TMarioGamePadFv(gamepad);
		u8 currentId = getActiveViewport();
		setActiveMario(currentId);
		setCamera(currentId);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80299a84, 0, 0, 0), TMarioGamePad_updateMeaning_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802a6024, 0, 0, 0), TMarioGamePad_updateMeaning_override);

	
	// Description: set correct camera and player before player update to make player move after correct camera
	#define perform__6TMarioFUlPQ26JDrama9TGraphics         ((int (*)(...))0x8024D2A8)
	void TMario_perform_coop(TMario* mario, u32 param_1, JDrama::TGraphics* param_2) {
		if(param_1 & 0x1) {
			u8 playerId = getPlayerId(mario);
			setActiveMario(playerId);
			setCamera(playerId);
		}

		perform__6TMarioFUlPQ26JDrama9TGraphics(mario, param_1, param_2);

		//SimulateEscapeHeld(mario);
		if(param_1 & 0x1) {
			u8 currentId = getActiveViewport();
			setActiveMario(currentId);
			setCamera(currentId);
		}
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803dd680, 0, 0, 0), (u32)(&TMario_perform_coop));

	
	// Description: Sets initial fields on load for player and makes player active. 
	// Note: This makes player spawn at level exits
	// TODO: Fix wrong level exit animation
	void SetMario(TMarDirector* director) {
		bool bootState = TFlagManager::smInstance->getFlag(0x30006);
		for (int i = loadedMarios-1; i >= 0; i--) {
			TFlagManager::smInstance->setFlag(0x30006, bootState);
			setActiveMario(i);
			setCamera(i);
			director->setMario();
		
			f32 offset = -75.0f + 150.0f * i;
			volatile float angle = 2.0f * 3.1415 * marios[i]->mAngle.y / 65535.0f - 3.1415/2.0;
			f32 offsetX = sin(angle) * offset;
			f32 offsetZ = cos(angle) * offset;
			marios[i]->mTranslation.x += offsetX;
			marios[i]->mTranslation.z += offsetZ;

			// Start p2 about half way through mecha bowser fight
			//if(i > 0) {
			//	TMario* mario = marios[i];
			//	if(mario->mPinnaRail) {
			//		J3DFrameCtrl* ctrl = mario->mPinnaRail->getFrameCtrl(0);
			//		ctrl->mCurFrame = 1238;
			//	}

			//	if(mario->mKoopaRail) {
			//		J3DFrameCtrl* ctrl = mario->mKoopaRail->getFrameCtrl(0);
			//		ctrl->mCurFrame = 1238;
			//	}
			//}


			//
			//CPolarSubCamera* camera = getCameraById(i);
			//spawnData[i].startPosition = marios[i]->mTranslation;
			//spawnData[i].marioAngle = marios[i]->mRotation.y;
			//spawnData[i].cameraPosition = camera->mTranslation;
			//spawnData[i].cameraHorizontalAngle = camera->mHorizontalAngle;
			//spawnData[i].shouldRespawn = false;
			//spawnData[i].levelIsRestarting = false;
		}

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802983f8, 0, 0, 0), SetMario);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80298428, 0, 0, 0), SetMario);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802984d8, 0, 0, 0), SetMario);

	
	// Description: Replaces player throw function to override how TMarios are thrown
	// Throw strength is based on a combination of airborn and how far the stick is pressed. 
	// TODO: Research if this could be set as a player parameter instead of manually coded.
	void OnMarioThrow(THitActor* thrownObject, TMario* mario, u32 message) {
		float speed = mario->mControllerWork->mStickDist;
		if(speed > 0.5f || mario->mState & TMario::State::STATE_AIRBORN) {
			thrownObject->receiveMessage(mario, 7);
		} else {
			thrownObject->receiveMessage(mario, 8);
		}

		// Optimization: Could probably check if the held item is a TMario instead of interating through all players to find player
		for (int i = 0; i < loadedMarios; i++){
			TMario* thrownMario = marios[i];
			if (thrownObject == (THitActor*)thrownMario){
				TVec3f newPos = mario->mTranslation;
				const f32 PI = 3.1415;
				volatile float angle = 2.0f * PI * mario->mAngle.y / 65535.0f;
				newPos.x += sinf(angle) * 120.0f;
				newPos.z += cosf(angle) * 120.0f;

				thrownMario->mTranslation = newPos;
				thrownMario->mState = TMario::State::STATE_DIVE;
				thrownMario->mAngle.y = mario->mAngle.y;


				if(mario->mState & TMario::State::STATE_AIRBORN) {
					thrownMario->setPlayerVelocity(25.0f + speed/1.75f);
					thrownMario->mSpeed.y = 25.0f + speed/1.75f;
				} else if(speed > 0.5f) {
					thrownMario->setPlayerVelocity(10.0f + speed/3.0f);
					thrownMario->mSpeed.y = 15.0f + speed/3.0f;
				} else {
					thrownMario->setPlayerVelocity(0.0f);
					thrownMario->mSpeed.y = 0.0f;
				}

			}
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802437d0, 0, 0, 0), OnMarioThrow);

	
	// Description: Fix for luigi shadow. Basically there is a check whether shadow has been rendered This removes that check
	// Optimization: Check for shadow individually between players
	SMS_WRITE_32(SMS_PORT_REGION(0x80231834, 0, 0, 0), 0x60000000);
}