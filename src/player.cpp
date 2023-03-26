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

#include <SMS/GC2D/Talk2D2.hxx>

#include <BetterSMS/player.hxx>
#include <BetterSMS/stage.hxx>

#include "characters.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"

namespace SMSCoop {
	static u8 loadedMarios = 0;
	static TMario* marios[2];

	TJointObj* awakenedObjects[2][2];
	u32 playerPreviousWarpId[2];

	bool hasTriggeredMechaBowserCutscene = false;
	u32 jetcoasterDemoCallbackCamera = 0;


	#define DYNAMIC_MARIO_LOADING true

	int getPlayerCount() {
		return loadedMarios;
	}


	TMario* getMarioById(int id) {
		return marios[id];
	}

	int getClosestMarioId(TVec3f* position) {
		int closestId = 0;
		float closest = 99999.0f;
		for(int i = 0; i < loadedMarios; ++i) {
			float dist = PSVECDistance((Vec*)position, (Vec*)&marios[i]->mTranslation);
			if(closest > dist ) {
				closestId = i;
				closest = dist;
			}
		}
		return closestId;
	}

	// Description: Sets the global instance of mario and all access parameters
	// Note: Cannot be set every other frame because game logic depends on one static global instance so swapping breaks the game.
	// A lot of how mario works is connected to the global mario and must be manually changed to work with multiple players
	void setActiveMario(int id) {
		TMario* mario = marios[id];
		gpMarioOriginal = mario;
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
				TMario* mario = new TMario();
				mario->load(*memoryStream);
				mario_viewObjPtrList->mViewObjList.push_back(mario);
			}


			// We cleanup the unused mario that is just used as a locator
			TMario* mario = (TMario*)response;
			delete mario;
		
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
		setActiveMario(0);
		setCamera(0);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803dd680, 0, 0, 0), (u32)(&TMario_perform_coop));

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

	// Run on update
	void updateCoop(TMarDirector* marDirector) {
		if(loadedMarios > 1) {
			// HACK: Swap the current yoshi every update in order to let one mario ride both yoshis
			// This is because collision is bound to one specific yoshi per mario
			// so if we don't do this then one mario can only ride one of the loadedyoshi
			swapYoshis();
	
			for(int i = 0; i < loadedMarios; ++i) {
				TApplication *app      = &gpApplication;
				TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
				director->mGamePads[i]->mState.mIsTaling = director->mGamePads[0]->mState.mIsTaling;
			}
			// TODO: Add to TMarDirector=
			/*char queuedCutscenes = *(char*)((u32)marDirector + 0x24c);
			char activeCutsceneCount = *(char*)((u32)marDirector + 0x24d);
			if(IsMechabowser) {
				*(char*)((u32)marDirector + 0x24d) = *(char*)((u32)marDirector + 0x24c);
			}*/
		}
	}

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
	#define playerControl__6TMarioFPQ26JDrama9TGraphics         ((int (*)(...))0x8024DE38)
	void TMario_playerControl(TMario* mario, JDrama::TGraphics* graphics) {
		u8 playerId = getPlayerId(mario);
		setActiveMario(playerId);
		setCamera(playerId);
		playerControl__6TMarioFPQ26JDrama9TGraphics(mario, graphics);
		setActiveMario(0);
		setCamera(0);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803dd72c, 0, 0, 0), (u32)(&TMario_playerControl));


	static TMarioGamePad* GamePads = (TMarioGamePad*)0x8057738c;

	// Description: Override load method for mario to set correct gamepad for p2 and load correct model
	void loadMario(TMario* mario, JSUMemoryInputStream *input) {

		load__Q26JDrama6TActorFR20JSUMemoryInputStream(mario, input);
	
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

	// Note: There should be a callback when loading starts in BetterSMS
	u32 cleanupPlayersCoop(u32 param_1) {
		for(int i = 0; i < 2; ++i) {
			marios[i] = nullptr;

			for(int j = 0; j < 2; ++j) {
				awakenedObjects[i][j] = NULL;
			}
			playerPreviousWarpId[i] = 0;
		}
		setLoading(true);
		loadedMarios = 0;
		hasTriggeredMechaBowserCutscene = false;
		return SMS_getShineIDofExStage__FUc(param_1);
	}

	SMS_PATCH_BL(SMS_PORT_REGION(0x802a681c, 0, 0, 0), cleanupPlayersCoop);


	// Description: Sets initial fields on load for player and makes player active. 
	// Note: This makes player spawn at level exits
	// TODO: Fix wrong level exit animation
	void SetMario(TMarDirector* director) {
		for (int i = loadedMarios-1; i >= 0; i--) {
			setActiveMario(i);
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
			TApplication *app      = &gpApplication;
			TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
			auto* p1Gamepad = director->mGamePads[0];
			director->mGamePads[0] = director->mGamePads[i];

			u32 frameMeaning = director->mGamePads[0]->mFrameMeaning;
			director->mGamePads[0]->mFrameMeaning = director->mGamePads[0]->mMeaning;
			//personToCheckTalk = marioId;
			movement_game__12TMarDirectorFv(marDirector);

			director->mGamePads[0]->mFrameMeaning = frameMeaning;
			setCamera(0);
			setActiveMario(0);
			director->mGamePads[0] = p1Gamepad;
		} else {
			//personToCheckTalk = 0;
			movement_game__12TMarDirectorFv(marDirector);

		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029a4c8, 0, 0, 0), TMarDirector_movement_game_override);


	// Description: Override TCubeManagerBase_getInCubeNo to render if mario that is currently rendered is inside fastCubes (used for fast culling checks)
	// Note: This will cause objects that are culled for only one player to have their animations run in 30 fps since it only updates on one screen (this looks wonky) Very noticable in Sirena when player is on different floor
	u32 TCubeManagerBase_getInCubeNo_Perspective_Fix(TCubeManagerBase* cubeManagerBase, Vec& marioPos) {
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
		for(int i = 0; i < loadedMarios; ++i) {
			TMario* mario = marios[i];
		
			u32 cubeNo = cubeManagerArea->getInCubeNo((Vec&)mario->mTranslation);
			*(u32*)((u32)cubeManagerArea + 0x1c) = cubeNo;
			u32 result = isInAreaCube__16TCubeManagerAreaCFRC3Vec(cubeManagerArea, objPos);
			if(result) return result;
		}
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
		talk2d->gamepad = director->mGamePads[i];
		perform__8TTalk2D2FUlPQ26JDrama9TGraphics(talk2d, renderFlags, graphics);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c03e8, 0, 0, 0), (u32)(&TTalk2D2_perform));

	void checkController(Talk2D2* talk2d) {
		for(int i = 0; i < loadedMarios; ++i) {
			TApplication *app      = &gpApplication;
			TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
			talk2d->gamepad = director->mGamePads[i];
			setActiveMario(i);
			setCamera(i);
			checkControler__8TTalk2D2Fv(talk2d);
		}
		setActiveMario(0);
		setCamera(0);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80151da4, 0, 0, 0), checkController);


	void checkBoardController(Talk2D2* talk2d) {
		for(int i = 0; i < loadedMarios; ++i) {
			TApplication *app      = &gpApplication;
			TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
			talk2d->gamepad = director->mGamePads[i];
			setActiveMario(i);
			setCamera(i);
			checkControler__8TTalk2D2Fv(talk2d);
		}
		setActiveMario(0);
		setCamera(0);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80151d90, 0, 0, 0), checkBoardController);


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
		for(int i = 0; i < 2; ++i) {
			load__11TGCConsole2FR20JSUMemoryInputStream(consoles[i], param_1);
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
		for (int i = loadedMarios-1; i >= 0; --i) {
			setActiveMario(i);
			setupCollisions___8TBathtubFv(tBathtub);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801fb0a4, 0, 0, 0), TBathtub_setupCollisions_Override);

	// Description: Disable the remove collision function entierly
	// NB: Might have unknown consequences, should test this further...
	// Reason: Enable collision for p2 in bowser fight
	int TMapCollisionBase_remove(void* m_this) {
		return (int) m_this;
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
}