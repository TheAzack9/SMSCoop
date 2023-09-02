#include <string.h>
#include <sdk.h>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <JSystem/JDrama/JDRGraphics.hxx>
#include <JSystem/JDrama/JDRViewObjPtrListT.hxx>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/JointObj.hxx>
#include <SMS/Player/Yoshi.hxx>
#include <SMS/GC2D/GCConsole2.hxx>
#include <SMS/Manager/FlagManager.hxx>

#include <SMS/GC2D/Talk2D2.hxx>
#include <raw_fn.hxx>

#include "characters.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"
#include "shine.hxx"

#include <SMS/SPC/SpcBinary.hxx>
#include <SMS/SPC/SpcInterp.hxx>
#include "BetterSMS/sunscript.hxx"


namespace SMSCoop {
	typedef struct {
		TVec3f startPosition;
		u16 marioAngle;
		TVec3f cameraPosition;
		s16 cameraHorizontalAngle;
		bool shouldRespawn;
		bool levelIsRestarting;
	} SpawnData;


	static u8 loadedMarios = 0;
	static TMario* marios[2];

	TJointObj* awakenedObjects[2][2];
	u32 playerPreviousWarpId[2];

	bool hasTriggeredMechaBowserCutscene = false;
	u32 jetcoasterDemoCallbackCamera = 0;
	
	SpawnData spawnData[2];
	int gessoTimer[2];
	bool isBowserFight = false;

	TMario* playerTalking = nullptr;
	THitActor* nearestNpc = nullptr;

	#define DYNAMIC_MARIO_LOADING true

	int getPlayerCount() {
		return loadedMarios;
	}

	TMario* getTalkingPlayer() {
		return playerTalking;
	}

	TMario* getMarioById(int id) {
		return marios[id];
	}

	int getClosestMarioId(TVec3f* position) {
		float closest0 = PSVECSquareDistance((Vec*)position, (Vec*)&marios[0]->mTranslation);
		if(loadedMarios == 1) return 0;
		float closest1 = PSVECSquareDistance((Vec*)position, (Vec*)&marios[1]->mTranslation);
		return closest0 > closest1;
	}
	

	THitActor* findNearestTalkNPC(TMarDirector* marDirector) {
		nearestNpc = (THitActor*)marDirector->findNearestTalkNPC();
		return nearestNpc;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029a8c8, 0, 0, 0), findNearestTalkNPC);

	void handleTalking(TMarDirector* marDirector, THitActor* npc, bool isP1 = false) {
		u32 initiatingPlayer = getClosestMarioId(&npc->mTranslation);
		if(isP1) initiatingPlayer = 0;
		u8 initiatingTalking = *(u8*)((u32)marDirector + 0x126);
		
		bool someoneTalking = false;
		for(int i = 0; i < loadedMarios; ++i) {
			if(marios[i]->mState == TMario::State::STATE_TALKING) {
				someoneTalking = true;	
			}
		}

		if(!someoneTalking) {
			playerTalking = nullptr;
			for(int i = 0; i < loadedMarios; ++i) {
				*((u32*)&marDirector->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
			}
		}


		if(initiatingTalking && playerTalking == nullptr) {
			playerTalking = marios[initiatingPlayer];

		}
		
		for(int i = 0; i < loadedMarios; ++i) {
			if(playerTalking != nullptr) {
				if(marios[i] != playerTalking) {
					*((u32*)&marDirector->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
					*((u32*)&marDirector->mGamePads[i]->mState) &= ~0x40000; // Player cannot talk when another is talking
				} else {
					*((u32*)&marDirector->mGamePads[i]->mState) |= 0x80000; // Player is not talking can move
				}
			}
		}
	}


	// Description: Sets the global instance of mario and all access parameters
	// Note: Cannot be set every other frame because game logic depends on one static global instance so swapping breaks the game.
	// A lot of how mario works is connected to the global mario and must be manually changed to work with multiple players
	

int buttonsPressedWhileHeld[2];
int prevButtons[2];
float prevStickAngle[2];


	TMario* IsHeld(TMario* mario) {
		for(int i = 0; i < loadedMarios; ++i) {
			if(marios[i]->mHeldObject == mario) return marios[i];
		}
		return 0;
	}

	// Feature: Escape when held
	// Description: Keeps track of button presses while player is held, if it is above 30 then release the player
	void SimulateEscapeHeld(TMario* mario) {
		int playerIdx = mario == marios[0] ? 0 : 1;
		TMario* heldBy = IsHeld(mario);

		float lAnalogX = gpApplication.mGamePads[playerIdx]->mControlStick.mStickX;
		float lAnalogY = gpApplication.mGamePads[playerIdx]->mControlStick.mStickY;
		float angle = atan2f(lAnalogY, lAnalogX) + M_PI;

		// PI/2 - 0 = 3/2PI, real diff is PI/2
		float angleDiff = fabs(prevStickAngle[playerIdx] - angle);
		angleDiff = atan2f(sinf(angleDiff), cosf(angleDiff));

		if(
			heldBy != 0 && 
			(
				(prevButtons[playerIdx] != gpApplication.mGamePads[playerIdx]->mFrameMeaning && gpApplication.mGamePads[playerIdx]->mFrameMeaning != 0) 
				// If stick moved more than a 12th of a circle since last frame
				|| angleDiff > 0.6
			)
		) {
			buttonsPressedWhileHeld[playerIdx]++;
			TVec3f temp;
			temp.x = 0.5f;
			temp.y = 0.5f;
			temp.z = 0.5f;
		
			mario->startVoice(0x788f);
			SMS_EasyEmitParticle_2(8, &(mario->mTranslation), mario, &temp);
			SMS_EasyEmitParticle_2(9, &(mario->mTranslation), mario, &temp);
		}
		if(heldBy == 0) {
			buttonsPressedWhileHeld[playerIdx] = 0;
		}


		prevButtons[playerIdx] = gpApplication.mGamePads[playerIdx]->mFrameMeaning;
		prevStickAngle[playerIdx] = angle;

		if(buttonsPressedWhileHeld[playerIdx] > 15) {
			heldBy->dropObject();
		}
	}



static TMario** gpMarioOriginalCoop = (TMario**)0x8040e0e8; // WTF?
static TMario** gpMarioForCallBackCoop = (TMario**)0x8040e0e0; // WTF?
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
	bool isMarioCurrentlyLoadingViewObj = false;
	// Description: Overrides mario loading to allow for loading a luigi game object in level
	// TODO: Make this optional based on setting
	int TMarioStrCmp_Override(const char* nameRef, void* str) {
		int isMarioCheck = strcmp(nameRef, "Mario");
	#ifdef DYNAMIC_MARIO_LOADING
		if(isMarioCheck == 0) {
			isMarioCurrentlyLoadingViewObj = true;
		}
	#else
		if(strcmp(nameRef, "Luigi") == 0) {
			return 0;
		}
	#endif
		return isMarioCheck;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029d7d8, 0, 0, 0), TMarioStrCmp_Override);

	#ifdef DYNAMIC_MARIO_LOADING
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
			if(!isSplitscreen()) {
				isMarioCurrentlyLoadingViewObj = false;
				return response;
			}
			// Read the map data for mario and store it in a temp buffer to be able to reuse memoryStream.
			// We cannot set the position since this is the direct io stream, so we need to temp store it.
			char* buffer[73];
			memoryStream->readData(buffer, 73);

			// Create marios and load them
			for(int i = 0; i < 2; ++i) {
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
	#endif


	// Description: Override the controller update to ensure that correct mario is checked.
	// Note: Certain things like the y-cam is for some reason tied directly to the controller update.
	// IMPROVEMENT: I should just re-implement the entire loop at 0x80299a24 which is what calls this instead
	void TMarioGamePad_updateMeaning_override(TMarioGamePad* gamepad) {
		TApplication* app = &gpApplication;
		// A bit of a jank way to check if it is player 2 sadly
		if(loadedMarios > 1 && gamepad == app->mGamePads[1]) {
			setActiveMario(1);
			setCamera(1);
		}
		updateMeaning__13TMarioGamePadFv(gamepad);
		if(loadedMarios > 1 && gamepad == app->mGamePads[1]) {
			setActiveMario(0);
			setCamera(0);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80299a84, 0, 0, 0), TMarioGamePad_updateMeaning_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802a6024, 0, 0, 0), TMarioGamePad_updateMeaning_override);

	// Description: set correct camera and player before player update to make player move after correct camera
	#define perform__6TMarioFUlPQ26JDrama9TGraphics         ((int (*)(...))0x8024D2A8)
	void TMario_perform_coop(TMario* mario, u32 param_1, JDrama::TGraphics* param_2) {
		setGraphics(param_2);
		u8 playerId = getPlayerId(mario);
		setActiveMario(playerId);
		setCamera(playerId);

		perform__6TMarioFUlPQ26JDrama9TGraphics(mario, param_1, param_2);

		SimulateEscapeHeld(mario);
		setActiveMario(0);
		setCamera(0);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803dd680, 0, 0, 0), (u32)(&TMario_perform_coop));
	
	s16 TMario_getAttackAngle(TMario* mario, THitActor* hitActor) {
		return mario->mAngle.y;
		//if(!hitActor) return 0;
		//return mario->getAttackAngle(hitActor);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802412d0, 0, 0, 0), TMario_getAttackAngle);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028494c, 0, 0, 0), TMario_getAttackAngle);

	bool isYoshiMounted(TYoshi* yoshi) {
		return yoshi->mState == TYoshi::MOUNTED;
	}

	void swapYoshis() {
		TYoshi* yoshi1 = marios[0]->mYoshi;
		TYoshi* yoshi2 = marios[1]->mYoshi;
		if(isYoshiMounted(yoshi1) || isYoshiMounted(yoshi2)) return;
		marios[0]->mYoshi = yoshi2;
		marios[1]->mYoshi = yoshi1;
		yoshi1->mMario = marios[1];
		yoshi2->mMario = marios[0];
	}

	void setWaterColorForMario(int id) {
		TYoshi* yoshi = marios[id]->mYoshi;
		if(isYoshiMounted(yoshi)) {
			gpModelWaterManager->mWaterCardType = yoshi->mType;
		} else {
			gpModelWaterManager->mWaterCardType = 0;
		}
	}
	
	int framesWithoutTalking = 0;
	// Run on update
	void updateCoop(TMarDirector* marDirector) {
  //  auto *currentHeap = JKRHeap::sCurrentHeap;
		//OSReport("Free heap size %X\n", currentHeap->getTotalFreeSize());

		
		
		for(int i = 0; i < loadedMarios; ++i) {
			if(gessoTimer[i] > 0) {
				gessoTimer[i]--;
			}
		}

		if(loadedMarios > 1) {

			
			int id = getActivePerspective();
			setWaterColorForMario(id);

			TMario* luigi = marios[1];
			luigi->mJumpParams.mRotateJumpForceY.set( 70 * 1.2);
			luigi->mJumpParams.mSecJumpForce.set(52 * 1.2);
			luigi->mJumpParams.mUltraJumpForce.set(75 * 1.2);

			bool someoneTalking = false;
			for(int i = 0; i < loadedMarios; ++i) {
				
				if(marios[i]->mState == TMario::State::STATE_TALKING) {
					someoneTalking = true;	
				}

				TApplication *app      = &gpApplication;
				TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
				if(marios[i]->mState != TMario::State::STATE_TALKING) {
					*((u32*)&director->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
				}
				//*((u32*)&director->mGamePads[i]->mState) = *((u32*)&director->mGamePads[0]->mState);
				//*((u32*)&director->mGamePads[i]->mState) &= ~0x100000; // Allow to move during cutscenes

				//if(marios[i] != playerTalking) {
				//	*((u32*)&director->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
				//	if(playerTalking != nullptr) {
				//		*((u32*)&director->mGamePads[i]->mState) &= ~0x40000; // Player cannot talk when another is talking
				//	}
				//}
				/*director->mGamePads[i]->mState.mDisable = false;
				director->mGamePads[i]->mState.mDisable = false;*/
				
			}
			
			if(!someoneTalking) {
				playerTalking = nullptr;
				for(int i = 0; i < loadedMarios; ++i) {
					*((u32*)&marDirector->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
				}
			}

			if(!playerTalking && marDirector->mTalkingNPC != nullptr) {
				handleTalking(marDirector, marDirector->mTalkingNPC);
			}


			// HACK: Swap the current yoshi every update in order to let one mario ride both yoshis
			// This is because collision is bound to one specific yoshi per mario
			// so if we don't do this then one mario can only ride one of the loadedyoshi
			swapYoshis();
		}
	}



	void changePlayerStatusToTalking(TMario* mario, u32 state, u32 jumpSlipState, bool isGrounded) {
		if(mario == playerTalking) {
			mario->changePlayerStatus(state, jumpSlipState, isGrounded);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024deb8, 0, 0, 0), changePlayerStatusToTalking);

	void TCameraBck_setFrame(void* cameraBck, f32 param_1) {
		setFrame__10TCameraBckFf(cameraBck, param_1 + 1238.0 * jetcoasterDemoCallbackCamera);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80025c68, 0, 0, 0), TCameraBck_setFrame);

	u32 Camera_JetCoasterDemoCallback(CPolarSubCamera* camera, u32 param_1) {
		for(int i = 0; i < loadedMarios; ++i) {
			CPolarSubCamera* camera = getCameraById(i);
			setCamera(i);
			jetcoasterDemoCallbackCamera = i;
			// Add ctrl->mCurFrame = 1238; to anim
			JetCoasterDemoCallBack__FUlUl(camera, param_1);
		}
		jetcoasterDemoCallbackCamera = 0;
		setCamera(0);
		return 1;
	}

	void fireStartDemoCameraMechaBowser(TMarDirector* marDirector, char* param_1, TVec3f* param_2, u32 param_3, f32 param_4, bool param_5, void* param_6, u32 param_7, CPolarSubCamera* camera, void* param_9) {

		if(!hasTriggeredMechaBowserCutscene) {
			fireStartDemoCamera__12TMarDirectorFPCcPCQ29JGeometry8TVec3_f(marDirector, param_1, param_2, param_3, param_4, param_5, &Camera_JetCoasterDemoCallback, param_7, camera, param_9);
		}
		hasTriggeredMechaBowserCutscene = true;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80025bcc, 0, 0, 0), fireStartDemoCameraMechaBowser);

	void TYoshi_appearFromEgg_override(TYoshi* yoshi, TVec3f* pos, float param_2, void* egg) {
		for(int i = 0; i < loadedMarios; ++i) {
			//marios[i]->mYoshi->appearFromEgg(pos); Does not work, missing parameters?
			appearFromEgg__6TYoshiFRCQ29JGeometry8TVec3_f(marios[i]->mYoshi, pos, param_2, egg);
			pos->x += 120.0f;
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801bc730, 0, 0, 0), TYoshi_appearFromEgg_override);

	// Description: set correct camera and player before player update to make player move after correct camera
	//#define playerControl__6TMarioFPQ26JDrama9TGraphics         ((int (*)(...))0x8024DE38)
	//void TMario_playerControl(TMario* mario, JDrama::TGraphics* graphics) {
	//	u8 currentPlayer = getPlayerId(mario);
	//	u8 playerId = getPlayerId(mario);
	//	setActiveMario(playerId);
	//	setCamera(playerId);
	//	playerControl__6TMarioFPQ26JDrama9TGraphics(mario, graphics);
	//	setActiveMario(currentPlayer);
	//	setCamera(currentPlayer);
	//}
	//// Override vtable
	//SMS_WRITE_32(SMS_PORT_REGION(0x803dd72c, 0, 0, 0), (u32)(&TMario_playerControl));


	static TMarioGamePad* GamePads = (TMarioGamePad*)0x8057738c;

	// Description: Override load method for mario to set correct gamepad for p2 and load correct model
	void loadMario(TMario* mario, JSUMemoryInputStream *input) {

		load__Q26JDrama6TActorFR20JSUMemoryInputStream(mario, input);
	
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		*((u32*)&director->mGamePads[loadedMarios]->mState) &= ~0x80000; // Player is not talking

		marios[loadedMarios] = mario;
	
		if(loadedMarios == 1) {
			TApplication* app = &gpApplication;
			app->mGamePads[1]->_E0 = 2;
			mario->setGamePad(app->mGamePads[1]);
			mario->mController = app->mGamePads[1];
		
		}

		setActiveMarioArchive(loadedMarios);
		setActiveMario(0);
		loadedMarios++;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80276BF0, 0, 0, 0), loadMario);

	// Description: A setup function on stage load to reset global fields per level
	void setupPlayers(TMarDirector *director) {
		setLoading(false);

	}
	
	void isNearActors(TSpcInterp *interp, u32 argc) {
		
		for(int i = 0; i < argc; ++i) {
			TSpcSlice& slice = interp->mSlices[interp->mSlicesCount - argc];
			if(slice.mType == 0) {
				THitActor *a = reinterpret_cast<THitActor *>(slice.mValue);
				if(marios[0] == a) {
					interp->mSlices[interp->mSlicesCount - argc].mValue = (u32)marios[getActivePerspective()];
				}
			} else if (slice.mType == 2) {
				u16 keyCode = JDrama::TNameRef::calcKeyCode((const char*)slice.mValue);
				if(keyCode == marios[0]->mKeyCode) {
					slice.mValue = (u32)marios[getActivePerspective()]->mKeyName;
				}
			}
		}

		evIsNearActors__FP32TSpcTypedInterp_1(interp, argc);

	}

	void forceStartTalkExceptNpc(TSpcInterp *interp, u32 argc) {
		
		THitActor *target = reinterpret_cast<THitActor *>(interp->mSlices[0].mValue);
		ev__ForceStartTalkExceptNpc__FP32TSpcTypedInterp_1(interp, argc);
		
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		handleTalking(director, target);
	}


	void forceStartTalk(TSpcInterp *interp, u32 argc) {
		
		THitActor *target = reinterpret_cast<THitActor *>(interp->mSlices[0].mValue);
		ev__ForceStartTalk__FP32TSpcTypedInterp_1(interp, argc);
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		handleTalking(director, target, true);
	}

	#define BIND_SYMBOL(binary, symbol, func)                                                          \
    (binary)->bindSystemDataToSymbol((symbol), reinterpret_cast<u32>(&(func)))
	void bindNearActorsFunction(TSpcBinary* spcBinary) {
		BIND_SYMBOL(spcBinary, "isNearActors", isNearActors);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802892e0, 0, 0, 0), bindNearActorsFunction);

	void forceStartTalkExceptNpcBind(TSpcBinary* spcBinary) {
		BIND_SYMBOL(spcBinary, "__forceStartTalkExceptNpc", forceStartTalkExceptNpc);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8020ea4c, 0, 0, 0), forceStartTalkExceptNpcBind);
	
	void forceStartTalkBind(TSpcBinary* spcBinary) {
		BIND_SYMBOL(spcBinary, "__forceStartTalk", forceStartTalk);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8020ea38, 0, 0, 0), forceStartTalkBind);

	// Note: There should be a callback when loading starts in BetterSMS
	u32 cleanupPlayersCoop(u32 param_1) {
		for(int i = 0; i < 2; ++i) {
			marios[i] = nullptr;

			for(int j = 0; j < 2; ++j) {
				awakenedObjects[i][j] = NULL;
			}
			playerPreviousWarpId[i] = 0;

		}

		isBowserFight = false;
		setLoading(true);
		playerTalking = nullptr;
		loadedMarios = 0;
		hasTriggeredMechaBowserCutscene = false;
		return SMS_getShineIDofExStage__FUc(param_1);
	}

	SMS_PATCH_BL(SMS_PORT_REGION(0x802a681c, 0, 0, 0), cleanupPlayersCoop);

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
		
	#ifdef DYNAMIC_MARIO_LOADING
			f32 offset = -75.0f + 150.0f * i;
			volatile float angle = 2.0f * 3.1415 * marios[i]->mAngle.y / 65535.0f - 3.1415/2.0;
			f32 offsetX = sin(angle) * offset;
			f32 offsetZ = cos(angle) * offset;
			marios[i]->mTranslation.x += offsetX;
			marios[i]->mTranslation.z += offsetZ;
	#endif
			// Start p2 about half way through mecha bowser fight
			if(i > 0) {
				TMario* mario = marios[i];
				if(mario->mPinnaRail) {
					J3DFrameCtrl* ctrl = mario->mPinnaRail->getFrameCtrl(0);
					ctrl->mCurFrame = 1238;
				}

				if(mario->mKoopaRail) {
					J3DFrameCtrl* ctrl = mario->mKoopaRail->getFrameCtrl(0);
					ctrl->mCurFrame = 1238;
				}
			}


			
			CPolarSubCamera* camera = getCameraById(i);
			spawnData[i].startPosition = marios[i]->mTranslation;
			spawnData[i].marioAngle = marios[i]->mRotation.y;
			spawnData[i].cameraPosition = camera->mTranslation;
			spawnData[i].cameraHorizontalAngle = camera->mHorizontalAngle;
			spawnData[i].shouldRespawn = false;
			spawnData[i].levelIsRestarting = false;
		}

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802983f8, 0, 0, 0), SetMario);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80298428, 0, 0, 0), SetMario);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802984d8, 0, 0, 0), SetMario);

	// Description: Increases distance from gpMarioOriginal(player 1) where water can exist to a very high number. 
	// This is to fix water for second player when they are far away from the "main" player
	void TModelWaterManager_perform_move(TModelWaterManager* waterManager) {
		// OPTIMIZATION: Do this on load of water manager instead of every time it tries to move the water
		*(f32*)(((u32*)waterManager) + 0x5e08/4) = 1000000.0f;
		waterManager->move();
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8027bedc, 0, 0, 0), TModelWaterManager_perform_move);

	// Description: Fix for luigi shadow. Basically there is a check whether shadow has been rendered This removes that check
	// Optimization: Check for shadow individually between players
	SMS_WRITE_32(SMS_PORT_REGION(0x80231834, 0, 0, 0), 0x60000000);

	
	// Disable hover being disabled when talking
	SMS_WRITE_32(SMS_PORT_REGION(0x80269668, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80269670, 0, 0, 0), 0x60000000);

	// Fix spray when in cutscene
	SMS_WRITE_32(SMS_PORT_REGION(0x8026ae48, 0, 0, 0), 0x60000000);

	//SMS_WRITE_32(SMS_PORT_REGION(0x80276bb8, 0, 0, 0), 0x60000000);

	

	// Description: Removes hit actor from another hit actor's collision array
	void RemoveObjectFromColArray(THitActor* actor, THitActor* col){
		// If no collisions do nothing
		if (actor->mNumObjs == 0)
			return;

		// Find index of held mario actor
		int colActorIndex = -1;
		for(int i = 0; i < actor->mNumObjs; ++i) {
			if(actor->mCollidingObjs[i] == col) {
				colActorIndex = i;
				break;
			}
		}

		// Not found
		if (colActorIndex == -1){
			return;
		}

		// Move all other collisions one index up
		actor->mNumObjs--;
		for(int i = colActorIndex; i < actor->mNumObjs; ++i)
			actor->mCollidingObjs[i] = actor->mCollidingObjs[i+1];
	}

	// Description: Bounces TMario off another TMario
	#define changePlayerStatus__6TMarioFUlUlb                          ((int (*)(...))0x80254034)
	void BounceMario(TMario* mario1, TMario* mario2){
		int rstatus = mario1->mState;
		bool isDiving = rstatus == TMario::State::STATE_DIVE;
		bool isSpinjump = rstatus & TMario::State::STATE_JUMPSPIN;
		if (rstatus != TMario::State::STATE_JUMPSPIN && rstatus != TMario::State::STATE_JUMP && rstatus != TMario::State::STATE_JUMPSIDE && !isDiving && rstatus != TMario::State::STATE_NPC_BOUNCE)
		{
			if (rstatus == 0x00800230 || rstatus == 0x008008A0){
				//knock over other mario
				if ((mario2->mState & 0xFFFFFFF0) != 0x000024D0)
					changePlayerStatus__6TMarioFUlUlb(mario2, 0x000208b0, 0, 0);
			}
			return;
		}

		TVec3f temp;
		temp.x = 0.5f;
		temp.y = 0.5f;
		temp.z = 0.5f;

		if(isDiving) {
			mario1->mSpeed.y = 50.0f;
		}
		else {
			mario1->mSpeed.y = 300.0f;
			mario1->setAnimation(211, 1.0f);
			changePlayerStatus__6TMarioFUlUlb(mario1, 0x02000890, 0, 0);
			mario1->setStatusToJumping(0x02000890, 0);
		}

		SMS_EasyEmitParticle_2(8, &(mario1->mTranslation), (THitActor*)mario1, &temp);
		SMS_EasyEmitParticle_2(9, &(mario1->mTranslation), (THitActor*)mario1, &temp);
		startSoundActorWithInfo__Q214MSoundSESystem8MSoundSEFUlPC3VecP3VecfUlUlPP8JAISoundUlUc(6168, &(mario1->mTranslation), 0, 0.0f, 3, 0, 0, 0, 4);

		// Footstool
		rstatus = mario2->mState & 0xFFFFFFF0;
		if (rstatus == TMario::State::STATE_JUMPSPIN || rstatus == TMario::State::STATE_JUMP || rstatus == TMario::State::STATE_JUMPSIDE)
		{
			mario2->mSpeed.y = -mario2->mSpeed.y;
		}

	}

	#define GetType( object ) *(int*)object
	const float MARIO_TRAMPLEHEIGHT = 60.0f;

	// Description: Collision check run for TMario
	void OnCheckActorsHit(void* hitcheckobj){
		// Run replaced branch
		checkActorsHit__12TObjHitCheckFv(hitcheckobj);

		for (int i = 0; i < loadedMarios; i++){
			// Check if mario should be bounced
			if (marios[i]->mSpeed.y < 0.0f) {
				for (int j = 0; j < marios[i]->mNumObjs; j++){
					if (GetType(marios[i]->mCollidingObjs[j]) == 0x803dd660){
						if (marios[i]->mTranslation.y - MARIO_TRAMPLEHEIGHT > ((TMario*)marios[i]->mCollidingObjs[j])->mTranslation.y){
							BounceMario(marios[i], (TMario*)(marios[i]->mCollidingObjs[j]));
						}
					}
				}
			}

			//Remove held item from player collision
			if (marios[i]->mHeldObject != 0)
				RemoveObjectFromColArray((THitActor*)marios[i], marios[i]->mHeldObject);

		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80299af8, 0, 0, 0), OnCheckActorsHit);

	

		// Description: Collision check run for TMario
	void TOBjHitCheck_suffererIsInAttackArea(void* tObjHitCheck, THitActor* hitActor, THitActor* mario){
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* zMario = marios[i];
			if(checkDistance__FRCQ29JGeometry8TVec3_f(hitActor->mAttackRadius, hitActor->mAttackHeight, zMario->mReceiveRadius, zMario->mReceiveHeight, hitActor->mTranslation, zMario->mTranslation)) {
				suffererIsInAttackArea__12TObjHitCheckFP9THitActorP9THitActor(tObjHitCheck, hitActor, zMario);
			}
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021bc00, 0, 0, 0), TOBjHitCheck_suffererIsInAttackArea);

	// Description: Replaces player throw function to override how TMarios are thrown
	// Throw strength is based on a combination of airborn and how far the stick is pressed. 
	// TODO: Research if this could be set as a player parameter instead of manually coded.
	void OnMarioThrow(THitActor* thrownObject, TMario* mario, u32 message) {
		u8 playerId = getPlayerId(mario);
		setActiveMario(playerId);

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

		setActiveMario(0);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802437d0, 0, 0, 0), OnMarioThrow);

	// Description: Overrides a hard coded check for distance to global mario and checks for all players instead
	// This fixes collision with e.g shines and allows luigi to collide with them
	int CheckDistance_Override(double param_1, double param_2, double param_3, double param_4, TVec3f* positionObj, TVec3f* positionPlayer) {
		int result = 0;
		for (int i = 0; i < getPlayerCount(); i++){
			TMario* mario = getMarioById(i);

			result = checkDistance__FRCQ29JGeometry8TVec3_f(param_1, param_2, param_3, param_4, positionObj, &mario->mTranslation);
			if(result) {
				return result;
			}
		}

		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021bbe0, 0, 0, 0), CheckDistance_Override);


	void TMarDirector_movement_game_override(TMarDirector* marDirector) {
		int marioId = getActivePerspective();

		if(loadedMarios > 1) {
			//for(int i = 0; i < loadedMarios; ++i) {
			//	if(i == marioId) continue;
			//	setCamera(i);
			//	setActiveMario(i);
			//	TApplication *app      = &gpApplication;
			//	TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
			//	auto* p1Gamepad = director->mGamePads[0];
			//	director->mGamePads[0] = director->mGamePads[i];
			//	//personToCheckTalk = marioId;
			//	movement_game__12TMarDirectorFv(marDirector);
			//	setCamera(0);
			//	setActiveMario(0);
			//	director->mGamePads[0] = p1Gamepad;
			//}
			int i = marioId;
			setCamera(i);
			setActiveMario(i);

			TMarioGamePad* p1Gamepad = marDirector->mGamePads[0];
			marDirector->mGamePads[0] = marDirector->mGamePads[i];

			u32 frameMeaning = marDirector->mGamePads[0]->mFrameMeaning;
			marDirector->mGamePads[0]->mFrameMeaning = marDirector->mGamePads[0]->mMeaning;
			
			*((u32*)&marDirector->mGamePads[0]->mState) &= ~0x100000; // Allow to move during cutscenes
			
			movement_game__12TMarDirectorFv(marDirector);
			if(nearestNpc != nullptr) {
				handleTalking(marDirector, nearestNpc);
			}

			marDirector->mGamePads[0]->mFrameMeaning = frameMeaning;
			setCamera(0);
			setActiveMario(0);
			marDirector->mGamePads[0] = p1Gamepad;
		} else {
			//personToCheckTalk = 0;
			movement_game__12TMarDirectorFv(marDirector);

		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029a4c8, 0, 0, 0), TMarDirector_movement_game_override);


	// Description: Override TCubeManagerBase_getInCubeNo to render if mario that is currently rendered is inside fastCubes (used for fast culling checks)
	// Note: This will cause objects that are culled for only one player to have their animations run in 30 fps since it only updates on one screen (this looks wonky) Very noticable in Sirena when player is on different floor
	u32 TCubeManagerBase_getInCubeNo_Perspective_Fix(TCubeManagerBase* cubeManagerBase, Vec& marioPos) {
		if(gpMarDirector->mAreaID == TGameSequence::Area::AREA_MAREBOSS) {
			return cubeManagerBase->getInCubeNo((Vec&)marios[0]->mTranslation);
		}

		int i = getActivePerspective();
		TMario* mario = marios[i];
		return cubeManagerBase->getInCubeNo((Vec&)mario->mTranslation);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024d460, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024d474, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024d488, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024d49c, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80195490, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix);

	// Description: Override TCubeManagerBase_getInCubeNo to render if mario that is currently rendered is inside fastCubes (used for fast culling checks)
	// Note: This will cause objects that are culled for only one player to have their animations run in 30 fps since it only updates on one screen (this looks wonky) Very noticable in Sirena when player is on different floor
	u32 TCubeManagerBase_getInCubeNo_Perspective_Fix2(TCubeManagerBase* cubeManagerArea, Vec& objPos) {
		if(gpMarDirector->mAreaID == TGameSequence::Area::AREA_MAREBOSS) {
			return isInAreaCube__16TCubeManagerAreaCFRC3Vec(cubeManagerArea, objPos);
		}
		return true;
		//for(int i = 0; i < loadedMarios; ++i) {
		//	TMario* mario = marios[i];
		//
		//	u32 cubeNo = cubeManagerArea->getInCubeNo((Vec&)mario->mTranslation);
		//	*(u32*)((u32)cubeManagerArea + 0x1c) = cubeNo;
		//	u32 result = isInAreaCube__16TCubeManagerAreaCFRC3Vec(cubeManagerArea, objPos);
		//	if(result) return result;
		//}
		return 0;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021afec, 0, 0, 0), TCubeManagerBase_getInCubeNo_Perspective_Fix2);

	// Description: Try fix warps + some objects being hidden (put to sleep) on certain collision types
	// Note: I have up fixing this for sirena for now due to it being updated based on mario position and many other things. This fix is mainly for e.g rooms in delfino
	// Future fix: Keep track of all stage items for each player somehow (Potentially keeping a set of sleeping and awake items for each player)
	void TMap_perform_watchToWarp_override(TMapWarp* tMapWarp) {
		int i = getActivePerspective();
		setActiveMario(i);
		setCamera(i);

		// Fuck Sirena
		if(gpMarDirector->mAreaID == TGameSequence::Area::AREA_DELFINO) {
			watchToWarp__8TMapWarpFv(tMapWarp);
			setActiveMario(0);
			setCamera(0);
			return;

		}
		tMapWarp->mPrevID = playerPreviousWarpId[i];

		watchToWarp__8TMapWarpFv(tMapWarp);
		playerPreviousWarpId[i] = tMapWarp->mPrevID;
	
		for(int j = 0; j < getPlayerCount(); ++j) {
			if(marios[i]->mHeldObject == marios[j]) {
				playerPreviousWarpId[j] = tMapWarp->mPrevID;
			}
		}


		for(int j = 0; j < getPlayerCount(); ++j) {
			if(awakenedObjects[j][0]) {
				awakenedObjects[j][0]->sleep();
			}
			if(awakenedObjects[j][1]) {
				awakenedObjects[j][1]->sleep();
			}
		}
		if(awakenedObjects[i][0]) {
			awakenedObjects[i][0]->sleep();
		}
		if(awakenedObjects[i][1]) {
			awakenedObjects[i][1]->awake();
		}
		setActiveMario(0);
		setCamera(0);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80189760, 0, 0, 0), TMap_perform_watchToWarp_override);

	// Description: Override sleep collision to track which one was put to sleep and which one was set awake
	void TJointObj_sleep_watchToWarp_override(TJointObj* jointObj) {
		if(gpMarDirector->mAreaID == TGameSequence::Area::AREA_DELFINO) {
			jointObj->sleep();
			return;
		}
		int i = getActivePerspective();
		awakenedObjects[i][0] = jointObj;
		if(!awakenedObjects[1-i][1]) awakenedObjects[1-i][1] = jointObj;

		for(int j = 0; j < getPlayerCount(); ++j) {
			if(marios[i]->mHeldObject == marios[j]) {
				awakenedObjects[j][0] = jointObj;
			}
		}

		jointObj->sleep();
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80195330, 0, 0, 0), TJointObj_sleep_watchToWarp_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80195454, 0, 0, 0), TJointObj_sleep_watchToWarp_override);


	// Description: Override sleep collision to track which one was put to sleep and which one was set awake
	void TJointObj_awake_watchToWarp_override(TJointObj* jointObj) {
		if(gpMarDirector->mAreaID == TGameSequence::Area::AREA_DELFINO) {
			jointObj->awake();
			return;
		}
		int i = getActivePerspective();
		awakenedObjects[i][1] = jointObj;
		if(!awakenedObjects[1-i][0]) awakenedObjects[1-i][0] = jointObj;
	
		for(int j = 0; j < getPlayerCount(); ++j) {
			if(marios[i]->mHeldObject == marios[j]) {
				awakenedObjects[j][1] = jointObj;
			}
		}

		jointObj->awake();

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8019535c, 0, 0, 0), TJointObj_awake_watchToWarp_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80195480, 0, 0, 0), TJointObj_awake_watchToWarp_override);
	void TBaseNpc_perform_replace(TBaseNPC* npc, u32 param_1, void* graphics) {
	
		//int closest = getActivePerspective();
		int closest = getClosestMarioId(&npc->mTranslation);
		setActiveMario(closest);
		setCamera(closest);
		perform__8TBaseNPCFUlPQ26JDrama9TGraphics(npc, param_1, graphics);
		setActiveMario(0);
		setCamera(0);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8468, 0, 0, 0), (u32)(&TBaseNpc_perform_replace));

	struct NPCManager {
		int u0; // 0x0
		int u1; // 0x4
		int u2; // 0x8
		int u3; // 0xc
		int u4; // 0x10
		int length; // 0x14
		TBaseNPC** npcs; // 0x18
	};
	
	void TBoardNpcManager_clipActors(NPCManager* npcManager, JDrama::TGraphics* graphics) {
		
		clipActors__16TBoardNpcManagerFPQ26JDrama9TGraphics(npcManager, graphics);
		int length = npcManager->length;
		for(int j = 0; j < loadedMarios; ++j) {
			TMario* mario = marios[j];
			for(int i = 0; i < length; ++i) {
				TBaseNPC* npc = npcManager->npcs[i];
				if(PSVECDistance((Vec*)&npc->mTranslation, (Vec*)&mario->mTranslation) < 500) {
					npc->mStateFlags.asU32 = npc->mStateFlags.asU32 & 0xfffffffb;
				}

				// Fix crashes where getting shine or shine spawning when talking to npc 
				if(isShineGot()) {
					npc->mStateFlags.asU32 |= 4;
				}
			}
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d92e0, 0, 0, 0), (u32)(&TBoardNpcManager_clipActors));

	void TNPCManager_clipEnemies(NPCManager* npcManager, JDrama::TGraphics* graphics) {
	
		clipEnemies__11TNPCManagerFPQ26JDrama9TGraphics(npcManager, graphics);
		int length = npcManager->length;
		for(int j = 0; j < loadedMarios; ++j) {
			TMario* mario = marios[j];
			for(int i = 0; i < length; ++i) {
				TBaseNPC* npc = npcManager->npcs[i];
				if(PSVECDistance((Vec*)&npc->mTranslation, (Vec*)&mario->mTranslation) < 500) {
					npc->mStateFlags.asU32 = npc->mStateFlags.asU32 & 0xfffffffb;
				}

				// Fix crashes where getting shine or shine spawning when talking to npc 
				if(isShineGot()) {
					npc->mStateFlags.asU32 |= 4;
				}
			}
		}
	}

	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8734, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d87c4, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d881c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8874, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d88cc, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8924, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d897c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d89d4, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8a2c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8a84, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8adc, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8b34, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8b8c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8be4, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8c3c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8c94, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8cec, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8d44, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8d9c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8df4, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8e4c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8ea4, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8efc, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8f54, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8fac, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d9004, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d905c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d90b4, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d910c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d9164, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d91bc, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d9214, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d926c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803df90c, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));
	SMS_WRITE_32(SMS_PORT_REGION(0x803df964, 0, 0, 0), (u32)(&TNPCManager_clipEnemies));

	void TTalk2D2_perform(Talk2D2* talk2d, u32 renderFlags, JDrama::TGraphics* graphics) {
		int i = getActivePerspective();
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		if(playerTalking != nullptr) {
			int marioId = getPlayerId(playerTalking);
      ((TMarioGamePad **)talk2d)[0x24C / 4] = director->mGamePads[marioId];

			if(marios[i] == playerTalking) {
				// Called twice for fps :c
				setActiveMario(marioId);
				setCamera(marioId);
				perform__8TTalk2D2FUlPQ26JDrama9TGraphics(talk2d, renderFlags, graphics);

				setActiveMario(i);
				setCamera(i);
			} else {
				perform__8TTalk2D2FUlPQ26JDrama9TGraphics(talk2d, renderFlags & ~8, graphics);
			}
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c03e8, 0, 0, 0), (u32)(&TTalk2D2_perform));

	//void checkController(Talk2D2* talk2d) {
	//	for(int i = 0; i < loadedMarios; ++i) {
	//		TApplication *app      = &gpApplication;
	//		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
	//		talk2d->gamepad = director->mGamePads[i];
	//		setActiveMario(i);
	//		setCamera(i);
	//		checkControler__8TTalk2D2Fv(talk2d);
	//	}
	//	setActiveMario(0);
	//	setCamera(0);
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80151da4, 0, 0, 0), checkController);


	//void checkBoardController(Talk2D2* talk2d) {
	//	for(int i = 0; i < loadedMarios; ++i) {
	//		TApplication *app      = &gpApplication;
	//		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
	//		talk2d->gamepad = director->mGamePads[i];
	//		setActiveMario(i);
	//		setCamera(i);
	//		checkBoardControler__8TTalk2D2Fv(talk2d);
	//	}
	//	setActiveMario(0);
	//	setCamera(0);
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80151d90, 0, 0, 0), checkBoardController);


	// Fix underwater hovering when all water particles are used
	SMS_WRITE_32(SMS_PORT_REGION(0x8026cd84, 0, 0, 0), 0x60000000);


	// Fix collision for p2 in bowser fight
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7c8, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7d4, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7e0, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7ec, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7f8, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801440c4, 0, 0, 0), 0x60000000);


	TGCConsole2* consoles[2];
	
	TGCConsole2* getConsoleForPlayer(int id) {
		return consoles[1 - id];
	}

	// Create all instances of TGCConsole2
	void TGCConsole2_constructor(TGCConsole2* console, char* param_1) {

		consoles[0] = console;
		__ct__11TGCConsole2FPCc(console, param_1);

		for(int i = 1; i < 2; ++i) {
			TGCConsole2* otherConsole = new TGCConsole2();
			__ct__11TGCConsole2FPCc(otherConsole, param_1);
			consoles[i] = otherConsole;
		}

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029db48, 0, 0, 0), TGCConsole2_constructor);

	// Load all instances of TGCConsole2
	void TGCConsole2_load(TGCConsole2* tgcConsole2, JSUMemoryInputStream* param_1) {
		char buffer[64];

		for(int i = 0; i < 2; ++i) {
			load__11TGCConsole2FR20JSUMemoryInputStream(consoles[i], param_1);

			// TODO: Not hard coded
			if(i == 1) {
				{
					J2DPicture* marioIcon =
						reinterpret_cast<J2DPicture*>(consoles[i]->mMainScreen->search('m_ic'));
					snprintf(buffer, 64, "/game_6/timg/%s_icon.bti", "luigi");

					auto* timg = reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource(buffer));
					if (timg)
						marioIcon->changeTexture(timg, 0);
				}

				{
					J2DPicture *marioName =
						reinterpret_cast<J2DPicture *>(consoles[i]->mMainScreen->search('m_tx'));

					snprintf(buffer, 64, "/game_6/timg/%s_text.bti", "luigi");

					auto *timg = reinterpret_cast<ResTIMG *>(JKRFileLoader::getGlbResource(buffer));
					if (timg)
						marioName->changeTexture(timg, 0);
				}
			}
		}
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c0314, 0, 0, 0), (u32)(&TGCConsole2_load));

	// Load all instances of TGCConsole2
	void TGCConsole2_loadAfter(TGCConsole2* tgcConsole2) {
		for(int i = 0; i < 2; ++i) {
			loadAfter__11TGCConsole2Fv(consoles[i]);
		}
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c031c, 0, 0, 0), (u32)(&TGCConsole2_loadAfter));

	void TGCConsole2_perform_override(TGCConsole2* tgcConsole2, u32 param_1, JDrama::TGraphics* graphics) {
		int p = getActivePerspective();

		// We still need to update ui for other players for animations to play correctly
		// param_1 & 0x8 = drawing, so we remove that flag
		for(int i = 0; i < loadedMarios; ++i) {
			if(i == p) continue;
			setActiveMario(i);
			setCamera(i);
			perform__11TGCConsole2FUlPQ26JDrama9TGraphics(consoles[i], param_1 & (~8), graphics);
		}

		// Update and draw current ui
		setActiveMario(p);
		setCamera(p);
		perform__11TGCConsole2FUlPQ26JDrama9TGraphics(consoles[p], param_1, graphics);
		setActiveMario(0);
		setCamera(0);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c0324, 0, 0, 0), (u32)(&TGCConsole2_perform_override));


	// Description: Setup collision for both marios every frame
	// Reason: Allow p2 to collide with platform separately from p1
	void TBathtub_setupCollisions_Override(void* tBathtub) {
		isBowserFight = true;
		for (int i = loadedMarios-1; i >= 0; --i) {
			setActiveMario(i);
			setupCollisions___8TBathtubFv(tBathtub);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801fb0a4, 0, 0, 0), TBathtub_setupCollisions_Override);

	// Description: Disable the remove collision function entierly
	// NB: Might have unknown consequences, should test this further...
	// Reason: Enable collision for p2 in bowser fight
	// NB: Used for e.g shrinking of spring 
	int TMapCollisionBase_remove(void* m_this) {
		if(isBowserFight) {
			return (int) m_this;
		} else {
			return remove__17TMapCollisionBaseFv(m_this);
		}
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c171c, 0, 0, 0), (u32)(&TMapCollisionBase_remove));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c1764, 0, 0, 0), (u32)(&TMapCollisionBase_remove));

	// Description: Switch between which mario is active when updating TBathWaterManager
	// Reason: Allow p2 to collide with bathwater
	void TBathWaterManager_perform_override(void* tBathwaterManager, u32 param_1, void* tGraphics) {
		// is updating and not draw	
		if((param_1 & 1) != 0) {
			int ap = getActivePerspective();
			for(int i = 0; i < loadedMarios; ++i) {
				if(i == ap) continue;
				setActiveMario(i);
				perform__17TBathWaterManagerFUlPQ26JDrama9TGraphics(tBathwaterManager, param_1, tGraphics);
			}
			setActiveMario(ap);
			perform__17TBathWaterManagerFUlPQ26JDrama9TGraphics(tBathwaterManager, param_1, tGraphics);
			// Ensure that primary mario is set correct again
			setActiveMario(0);
		} else {
			perform__17TBathWaterManagerFUlPQ26JDrama9TGraphics(tBathwaterManager, param_1, tGraphics);
		}

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c27a8, 0, 0, 0), (u32)(&TBathWaterManager_perform_override));


	// Fix tree collision
	SMS_WRITE_32(SMS_PORT_REGION(0x801f6cc4, 0, 0, 0), 0x60000000);

	// Fix cloud collision
	SMS_WRITE_32(SMS_PORT_REGION(0x801dfc1c, 0, 0, 0), 0x60000000);


	
	#define MarioFlagId_Lives 0x20001

	void loserExecOverride(TMario* mario) {
		if(isShineGot()) return;
		int lives = TFlagManager::smInstance->getFlag(MarioFlagId_Lives);
		if(lives <= 0) {
			for(int i = 0; i < loadedMarios; ++i) {
				marios[i]->loserExec();
			}
			TFlagManager::smInstance->setFlag(MarioFlagId_Lives, 4);

		}

		TFlagManager::smInstance->decFlag(MarioFlagId_Lives, 1);
		int marioId = getPlayerId(mario);
		
		for(int i = 0; i < 2; ++i) {
			//OSReport("Offset %X\n", (u32)(((u32*)consoles[i]+ 0x70/4)) - (u32)consoles[i]);
			*(u16*)(((u32*)consoles[i]+ 0x70/4)) = 200;
			startAppearMario__11TGCConsole2Fb(consoles[i], true);
		}

		SpawnData* marioSpawnData = &spawnData[marioId];

		CPolarSubCamera* camera = getCameraById(marioId);
		marioSpawnData->shouldRespawn = false;
		mario->mHealth = 8;
		mario->mWaterHealth = 8.0;
		// camera->position = marioSpawnData->cameraPosition;
		// camera->position.y += 1000.0f;
		mario->mTranslation = marioSpawnData->startPosition;
		mario->mTranslation.y += 200.0;
		mario->mRotation.y = spawnData[marioId].marioAngle;
		mario->mSpeed.set(0, 0, 0);
		mario->setPlayerVelocity(0.0f);
		camera->mHorizontalAngle = spawnData[marioId].cameraHorizontalAngle;
		camera->mInterpolateDistance = 0.0;
		camera->JSGSetViewPosition((Vec&)marioSpawnData->cameraPosition);
		camera->mTranslation.y = mario->mTranslation.y + 200.0f;
		camera->JSGSetViewTargetPosition((Vec&)mario->mTranslation);
		camera->warpPosAndAt(camera->mInterpolateDistance, spawnData[marioId].cameraHorizontalAngle);
		mario->changePlayerStatus(0x0000088C, 0, false);

		// TODO: Better way to refill water?
		mario->mFludd->mCurrentWater = 0x2710;
		mario->warpOut();
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80030ff0, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802438c8, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024390c, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024b280, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024b2f8, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024f808, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802527cc, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80252874, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8025a1e0, 0, 0, 0), loserExecOverride);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028aef0, 0, 0, 0), loserExecOverride);
	// Technically a changePlayerStatus
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024fba0, 0, 0, 0), loserExecOverride);
	
	// Stop game overs
	SMS_WRITE_32(SMS_PORT_REGION(0x8024fb6c, 0, 0, 0), 0x60000000);

	// Remove loosing lives
	SMS_WRITE_32(SMS_PORT_REGION(0x80298814, 0, 0, 0), 0x60000000);
	


	int TMario_CanTake_Override(TMario* mario, THitActor* object) {
		// Check if player is already holding the object
		// This is to ensure game doesn't crash when e.g SM and mario tries to drag blooper at the same time
		for (int i = 0; i < loadedMarios; i++){
			if (marios[i] == 0)
				continue;

			if(marios[i]->mHeldObject == object) return 0;
		}


		return mario->canTake(object);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80281604, 0, 0, 0), TMario_CanTake_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80281b44, 0, 0, 0), TMario_CanTake_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80281d94, 0, 0, 0), TMario_CanTake_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80281e78, 0, 0, 0), TMario_CanTake_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80281f88, 0, 0, 0), TMario_CanTake_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802820f0, 0, 0, 0), TMario_CanTake_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802821f8, 0, 0, 0), TMario_CanTake_Override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80282238, 0, 0, 0), TMario_CanTake_Override);

	// Set diving helm when set from sunscript
	void TMario_setDivHelm(TMario* mario) {
		for (int i = 0; i < loadedMarios; i++){
			if (marios[i] == 0)
				continue;
			marios[i]->setDivHelm();
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028d21c, 0, 0, 0), TMario_setDivHelm);

	
	// Set nozzle when set from sunscript
	void TMario_changeNozzle(TWaterGun* watergun, TWaterGun::TNozzleType nozzleType, bool param_2) {
		for (int i = 0; i < loadedMarios; i++){
			if (marios[i] == 0)
				continue;
			marios[i]->mFludd->changeNozzle(nozzleType, param_2);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028d230, 0, 0, 0), TMario_changeNozzle);

	
	
	// Set sunglass when set from sunscript
	void TMario_wearGlass(TMario* mario) {
		for (int i = 0; i < loadedMarios; i++){
			if (marios[i] == 0)
				continue;
			marios[i]->wearGlass();
			
			bool shineFlag = TFlagManager::smInstance->getShineFlag('w');
			if(shineFlag) {
				*(u32*)(&marios[i]->mAttributes) |= 0x100000;
			}
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028adf0, 0, 0, 0), TMario_wearGlass);

	
	// Set sunglass when set from sunscript
	void TMario_takeOffGlass(TMario* mario) {
		for (int i = 0; i < loadedMarios; i++){
			if (marios[i] == 0)
				continue;
			marios[i]->takeOffGlass();
			bool shineFlag = TFlagManager::smInstance->getShineFlag('w');
			if(shineFlag) {
				*(u32*)(&marios[i]->mAttributes) &= 0xffefffff;
			}
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028ae2c, 0, 0, 0), TMario_takeOffGlass);
	
	int previousFluddNozzle[2];
	void TFlagManager_setFlag_fluddNozzle(TFlagManager* flagManger, u32 flagId, s32 flagValue) {
		
		for (int i = 0; i < loadedMarios; i++){
			int nozzle = marios[i]->mFludd->mSecondNozzle;
			if(nozzle == 3) nozzle = 4;
			previousFluddNozzle[i] = nozzle;
		}

		flagManger->setFlag(flagId, flagValue);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80297990, 0, 0, 0), TFlagManager_setFlag_fluddNozzle);
	
	s32 TFlagManager_getFlag_fluddNozzle(TFlagManager* flagManger, u32 flagId) {
		int id = getPlayerId(gpMarioOriginal);
		return previousFluddNozzle[id];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80298b38, 0, 0, 0), TFlagManager_getFlag_fluddNozzle);
	
	
	void TItem_touchPlayer(TItem* item, THitActor* mario) {
		if(item->mKeyCode != 53611) {
			item->taken(mario);
		} else if(!((TMario*)mario)->mAttributes.mHasFludd) {
			item->taken(mario);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801bf328, 0, 0, 0), TItem_touchPlayer);
	

	// TODO: Reset between scene transitions?
	MActor* gesso0[2];
	MActor* gesso1[2];
	MActor* gesso2[2];
	void TItem_touchPlayer_TSurfGessoObj(TItem* item, TMario* mario) {
		int mId = getPlayerId(mario);
		if(gessoTimer[mId] <= 0) {
			
			void* gpMapObjManager = *(void**)0x8040df08;
			*(MActor**)((int)gpMapObjManager + 0x9c) = gesso0[mId];
			*(MActor**)((int)gpMapObjManager + 0xa0) = gesso1[mId];
			*(MActor**)((int)gpMapObjManager + 0xa4) = gesso2[mId];

			mario->getGesso(item);


			gessoTimer[mId] = 60 * 5; // about 5 sec
		}

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce8a8, 0, 0, 0), (u32)(&TItem_touchPlayer_TSurfGessoObj));


	MActor* SMS_MakeMActorFromSDLModelData_makeGesso0(void* sdlModelData, MActorAnmData* anmData, u32 param_3) {
		for(int i = 0; i < 2; ++i) {
			MActor* gesso = (MActor*)SMS_MakeMActorFromSDLModelData__FP12SDLModelDataP13MActorAnmDataUl(sdlModelData, anmData, param_3);
			gesso0[i] = gesso;
		}
		return gesso0[0];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b7544, 0, 0, 0), SMS_MakeMActorFromSDLModelData_makeGesso0);

	void TMapObjBase_initPacketMatColor_gesso0(TMapObjBase* mapObj, J3DModel* model, _GXTevRegID regId, _GXColorS10* colors10) {
		for(int i = 0; i < 2; ++i) {
			initPacketMatColor__11TMapObjBaseFP8J3DModel11_GXTevRegIDPC11_GXColorS10(*((u32*)gesso0[i] + 1), model, regId, colors10);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b7594, 0, 0, 0), TMapObjBase_initPacketMatColor_gesso0);

	
	MActor* SMS_MakeMActorFromSDLModelData_makeGesso1(void* sdlModelData, MActorAnmData* anmData, u32 param_3) {
		for(int i = 0; i < 2; ++i) {
			MActor* gesso = (MActor*)SMS_MakeMActorFromSDLModelData__FP12SDLModelDataP13MActorAnmDataUl(sdlModelData, anmData, param_3);
			gesso1[i] = gesso;
		}
		return gesso1[0];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b7560, 0, 0, 0), SMS_MakeMActorFromSDLModelData_makeGesso1);

	void TMapObjBase_initPacketMatColor_gesso1(TMapObjBase* mapObj, J3DModel* model, _GXTevRegID regId, _GXColorS10* colors10) {
		for(int i = 0; i < 2; ++i) {
			initPacketMatColor__11TMapObjBaseFP8J3DModel11_GXTevRegIDPC11_GXColorS10(*((u32*)gesso1[i] + 1), model, regId, colors10);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b75a8, 0, 0, 0), TMapObjBase_initPacketMatColor_gesso1);

	
	
	MActor* SMS_MakeMActorFromSDLModelData_makeGesso2(void* sdlModelData, MActorAnmData* anmData, u32 param_3) {
		for(int i = 0; i < 2; ++i) {
			MActor* gesso = (MActor*)SMS_MakeMActorFromSDLModelData__FP12SDLModelDataP13MActorAnmDataUl(sdlModelData, anmData, param_3);
			gesso2[i] = gesso;
		}
		return gesso2[0];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b757c, 0, 0, 0), SMS_MakeMActorFromSDLModelData_makeGesso2);

	void TMapObjBase_initPacketMatColor_gesso2(TMapObjBase* mapObj, J3DModel* model, _GXTevRegID regId, _GXColorS10* colors10) {
		for(int i = 0; i < 2; ++i) {
			initPacketMatColor__11TMapObjBaseFP8J3DModel11_GXTevRegIDPC11_GXColorS10(*((u32*)gesso2[i] + 1), model, regId, colors10);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b75bc, 0, 0, 0), TMapObjBase_initPacketMatColor_gesso2);


	bool isOnePlayerInCube(TCubeManagerBase* cubeManager, const Vec& marioPos, s32 param_3) {
		for(int i = 0; i < loadedMarios; ++i) {
			if(cubeManager->isInCube((const Vec&)marios[i]->mTranslation, param_3)) return true;
		}
		return false;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028c834, 0, 0, 0), isOnePlayerInCube);
	
	bool isInvincible(TMario* mario) {
		return mario->isInvincible() || playerTalking == mario;
		
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x800bab44, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802428cc, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80242f80, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80243060, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283608, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802836c4, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028375c, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283c30, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283c8c, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283d00, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283d5c, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283db8, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283e24, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283ee0, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80283f80, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284010, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284074, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802840d4, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028438c, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284450, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802844ac, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284508, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802845bc, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284678, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284750, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028479c, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284850, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802848e8, 0, 0, 0), isInvincible);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80284b68, 0, 0, 0), isInvincible);
}
 