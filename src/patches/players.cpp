#include <sdk.h>
#include <types.h>
#include <string.h>

#include <raw_fn.hxx>
#include <JDrama/JDRNameRef.hxx>
#include <JDrama/JDRViewObjPtrListT.hxx>
#include <SMS/Strategic/HitActor.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/M3DUtil/MActorKeeper.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Enemy/EnemyMario.hxx>
#include <JSystem/J3D/J3DModelLoaderDataBase.hxx>

#include "characters.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"
#include "shine.hxx"
#include "settings.hxx"
#include "pvp.hxx"
#include "gui.hxx"

#define MARIO_COUNT 2

static TMario** gpMarioOriginalCoop = (TMario**)0x8040e0e8; // WTF?
static TMario** gpMarioForCallBackCoop = (TMario**)0x8040e0e0; // WTF?
static u8* isThpInit = (u8*)0x803ec206;
static u8* ThpState = (u8*)0x803ec204;
extern SMSCoop::CameraTypeSetting gCameraTypeSetting;
extern SMSCoop::PlayerTypeSetting gPlayer1TypeSetting;
extern SMSCoop::PlayerTypeSetting gPlayer2TypeSetting;

namespace SMSCoop {
	typedef struct {
		TVec3f startPosition;
		u16 marioAngle;
		TVec3f cameraPosition;
		s16 cameraHorizontalAngle;
	} SpawnData;
	static SpawnData spawnData[MARIO_COUNT];

	typedef struct {
		MActorAnmData* marioAnmData;
		MActor* marioMActor;
	} MarioExtModelData;
	static MarioExtModelData modelData[MARIO_COUNT];

	static u8 loadedMarios = 0;
	static TMario* marios[MARIO_COUNT];
	static bool isMarioCurrentlyLoadingViewObj = false;
	static bool marioIsSet = false;
	static bool isCameraCaptured = false;
	static u8 focusedMario = 0;

	void setActiveMario(int id) {
		if(id > loadedMarios) return;
		TMario* mario = marios[id];
		gpMarioOriginal = mario;
		*gpMarioForCallBackCoop = mario;
		*gpMarioOriginalCoop = mario;
		SMS_SetMarioAccessParams__Fv();
	}
	
	bool isMarioSet() {
		return marioIsSet;
	}

	
	bool isFocusedCamera() {
		return isCameraCaptured;
	}

	u8 getFocusedPlayer() {
		return focusedMario;
	}

	void setFocusedPlayer(u8 playerId) {
		isCameraCaptured = true;
		focusedMario = playerId;
	}
	
	int isSingleplayerLevel() {
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

		if(gCameraTypeSetting.getInt() == CameraTypeSetting::REGULAR) {
			return true;
		}
		
        if (app == nullptr || app->mContext != TApplication::CONTEXT_DIRECT_STAGE) {
            return true;
        }

        if (director == nullptr) {
            return true;
        }
		
		// Cutscene
	    if(*ThpState == 2 && *isThpInit == 0) {
		    return true;
	    }

        // intro / option level
        if (director->mAreaID == 15) {
            return true;
        }

		return false;
	}
	

	int isSingleCameraLevel() {
		if(gCameraTypeSetting.getInt() == CameraTypeSetting::SINGLE && !isPvpLevel()) return true;
		return isSingleplayerLevel();
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
		if(id > loadedMarios) return marios[0];
		return marios[id];
	}
	
	int getClosestMarioId(TVec3f* position) {
		if(loadedMarios == 1) return 0;
		float closest0 = PSVECSquareDistance((Vec*)position, (Vec*)&marios[0]->mTranslation);
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

	void setSkinForPlayer(int player, int skinType) {
		if(skinType == PlayerTypeSetting::MARIO) {
			SMSCoop::setSkinForPlayer(player, "/data/mario.arc", false, 0, 0);
		}
		else if(skinType == PlayerTypeSetting::LUIGI) {
			SMSCoop::setSkinForPlayer(player, "/data/luigi.arc", false, 1, 1);
		}
		else if(skinType == PlayerTypeSetting::SHADOW_MARIO) {
			SMSCoop::setSkinForPlayer(player, "/data/kagemario.arc", true, 2, 0);
		}
	}

	void setPlayerSkin(TMarDirector* director) {
		if(!isPvpLevel()) {
			setSkinForPlayer(0, gPlayer1TypeSetting.getInt());
			setSkinForPlayer(1, gPlayer2TypeSetting.getInt());
		} else {
			setSkinForPlayer(0, PlayerTypeSetting::MARIO);
			setSkinForPlayer(1, PlayerTypeSetting::LUIGI);
		}
		marioIsSet = false;
		isCameraCaptured = false;
		focusedMario = 0;
	}
	 
	//
	//void* setModelSizeForMario(u32 size) {
	//	OSReport("HMMM %X \n", 0x1c);
	//	return __nwa__FUl(0x1c);
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80246720, 0, 0, 0), setModelSizeForMario);
	//void try_make_sdl_model_for_mario(J3DModel* model, J3DModelData* modelData, u32 unk1, u32 unk2) {
	//	
	//	//replace__14TScreenTextureFP12J3DModelDataPCc(*(u32**)0x8040e0bc, modelData, "H_kagemario_dummy");

	//	void* sdlModelData = __nwa__FUl(0x1c);
	//	__ct__12SDLModelDataFP12J3DModelData(sdlModelData, modelData);

	//	//__ct__8SDLModelFP12SDLModelDataUlUl(model, sdlModelData, unk1, unk2);
	//	/*testAnimData = new MActorAnmData();
	//	init__13MActorAnmDataFPCcPPCc(testAnimData, "/mario/btk");
	//	testing = new MActor(testAnimData);*/
	//	////testing->setModel(mario->mModelData->mModel, 0);

	//	//void* resource = JKRFileLoader::getGlbResource("/mario/default.bmd");
	//	//if(resource != nullptr) {
	//	//	J3DModelData* data = J3DModelLoaderDataBase::load(resource, 0);
	//	//	testModel = new J3DModel(data, 0, 1);
	//		//testing->setModel(model, 0);
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8024673c, 0, 0, 0), try_make_sdl_model_for_mario);

	// We need to disable goop texture hiding on animated mario...
	// TODO: If generalized, then this should probably be improved.
	void finalDrawInitialize_override(TMario* mario) {
		if(!hasCustomAnimations(getPlayerId(mario))) {
			mario->finalDrawInitialize();
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80276bb0, 0, 0, 0), finalDrawInitialize_override);

	// Description: We hook into the load to get a pointer to TViewObjPtrListT to be able to add viewObjs manually
	JDrama::TViewObjPtrListT<THitActor,JDrama::TViewObj>* mario_viewObjPtrList = 0;
	void JDrama_TViewObjPtrListT_load(JDrama::TViewObjPtrListT<THitActor,JDrama::TViewObj>* viewObjPtrList,JSUMemoryInputStream *param_1) {
		mario_viewObjPtrList = viewObjPtrList;
		load__Q26JDrama47TViewObjPtrListT_9(viewObjPtrList, param_1);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80223548, 0, 0, 0), JDrama_TViewObjPtrListT_load);

	// Allow animated texture
	SMS_WRITE_32(SMS_PORT_REGION(0x802465e8, 0, 0, 0), 0x3c801130);
	/*
	void TMario_initModel_override(TMario* mario) {
		if(getPlayerId(mario) == 1) {
			initModel__11TEnemyMarioFv(mario);
			return;
		}
		initModel__6TMarioFv(mario);

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803dd730, 0, 0, 0), (u32)(&TMario_initModel_override));*/

	struct SDLModelData {
		u32 unk[7];
	};

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
				if(isSingleplayerLevel() && i > 0) break;
				memoryStream->setBuffer(buffer, 73);
				TMario* mario = (TMario*) response;

				if(i != 0) {
					mario = new TMario();
				}
				
				
				marios[i] = mario;
				mario->load(*memoryStream);
				
				
				if(i != 0) {
					mario->mKeyName = "Luigi";
					mario->mKeyCode = JDrama::TNameRef::calcKeyCode("Luigi");
				}

				if(hasCustomAnimations(i)) {
				
					//OSReport("Loading custom animations for player %X\n", i);
				
					
					modelData[i].marioAnmData = new MActorAnmData();
					init__13MActorAnmDataFPCcPPCc(modelData[i].marioAnmData, "mario/btk");
					MActor* marioMActor = new MActor(modelData[i].marioAnmData);
					//marioMActor->unlockDLIfNeed();
					//marioMActor->initDL();
					marioMActor->setModel(mario->mModelData->mModel, 0);

					modelData[i].marioMActor = marioMActor; 
					//testing = testingkeeper->createAndRegister(modelData, 0);
					//testing->setModel(testModel, 0x0);

					//OSReport("Exists %X\n", marioMActor->checkAnmFileExist("animation", 4));
					marioMActor->setBtk("animation");

					f32 framerate = SMSGetAnmFrameRate();
					J3DFrameCtrl* ctrl = marioMActor->getFrameCtrl(4);
					ctrl->mFrameRate = framerate;
					ctrl->mAnimState = J3DFrameCtrl::LOOP;

					//marioMActor->setLightType(3);

					//SMS_MakeDLAndLock(testing->mModel);
					//OSReport("Animation Id %X %X\n", testing->mBtkInfo, testing->mBtkInfo->mFrameCtrl);
			////testing->setModel(mario->mModelData->mModel, 0);

						//replace__14TScreenTextureFP12J3DModelDataPCc(*(u32**)0x8040e0bc, data, "H_kagemario_dummy");
						/*
					for(u32 i = 0; i < testing->mModel->mModelData->mJointNum; ++i) {
						SMS_InitPacket_Fog__FP8J3DModelUs(testing->mModel, i & 0xffff);
					}*/
					/*	replace__14TScreenTextureFP12J3DModelDataPCc(getScreenTextureForPlayer(1), mario->mBodyModelData, "H_kagemario_dummy");*/
				/*		replace__14TScreenTextureFP12J3DModelDataPCc(getScreenTextureForPlayer(1), mario->mBodyModelData, "H_kagemario_indwp4n_ia");*/
						replace__14TScreenTextureFP12J3DModelDataPCc(*(u32**)0x8040e0bc, mario->mBodyModelData, "H_kagemario_dummy");
						/*replace__14TScreenTextureFP12J3DModelDataPCc(getScreenTextureForPlayer(1), testing->mModel->mModelData, "H_kagemario_dummy");*/
					//}

						
					//testing->setBtk("animation");

					//OSReport("HMMM %X %X \n", (u32)testAnimData, (u32)testing);
					//OSReport("Custom animation actor finished\n");
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

	//SMS_WRITE_32(SMS_PORT_REGION(0x8025b8a0, 0, 0, 0), 0x60000000);

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

		
		int moveType = getMoveType(loadedMarios);

		if(moveType == 1) {
			mario->mJumpParams.mRotateJumpForceY.set( 70 * 1.2);
			mario->mJumpParams.mSecJumpForce.set(52 * 1.2);
			mario->mJumpParams.mUltraJumpForce.set(75 * 1.2);
		}
		else if(moveType == 2) {
			mario->mRunParams.mMaxSpeed.set(32.0f * 2.0);
			mario->mJumpParams.mJumpSpeedBrake.set(1.0);
			mario->mSwimParams.mMoveSp.set(0.5);
			mario->mHoverParams.mRotSp.set(200);
			mario->mHoverParams.mBrake.set(0.99);
		}

		setActiveMarioArchive(loadedMarios);
		setActiveMario(getActiveViewport());
		loadedMarios++;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80276BF0, 0, 0, 0), loadMario);

	// Description: Override the controller update to ensure that correct mario is checked.
	// Note: Certain things like the y-cam is for some reason tied directly to the controller update.
	// IMPROVEMENT: I should just re-implement the entire loop at 0x80299a24 which is what calls this instead
	void TMarioGamePad_updateMeaning_override(TMarioGamePad* gamepad) {
		TApplication* app = &gpApplication;
		
		if(!isSingleplayerLevel() && !isSingleCameraLevel()) {
			// A bit of a jank way to check if it is player 2 sadly
			for(int i = 0; i < MARIO_COUNT; ++i) {
				if(loadedMarios > i && gamepad == app->mGamePads[i]) {
					setActiveMario(i);
					setCamera(i);
				}
			}
		}
		// Enable movement
		// TODO: Figure out where this is properly set... Might cause unexpected consequences
		gamepad->mState._06 = gpApplication.mGamePads[0]->mState._06;
		updateMeaning__13TMarioGamePadFv(gamepad);
		if(loadedMarios > 0) {
			u8 currentId = getActiveViewport();
			setActiveMario(currentId);
			setCamera(currentId);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80299a84, 0, 0, 0), TMarioGamePad_updateMeaning_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802a6024, 0, 0, 0), TMarioGamePad_updateMeaning_override);
	//
	//void doRunning_override(TMario* mario) {
	//	u8 playerId = getPlayerId(mario);
	//	if(hasCustomAnimations(playerId)) {
	//		mario->mBaseAcceleration *= 2.0;
	//		mario->doRunning();
	//		mario->mBaseAcceleration /= 2.0;
	//	}
	//	else {
	//		mario->doRunning();
	//	}
	//}
	//
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8025a5a4, 0, 0, 0), doRunning_override);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8025aad8, 0, 0, 0), doRunning_override);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8025b0a4, 0, 0, 0), doRunning_override);

	int buttonsPressedWhileHeld[MARIO_COUNT];
	int prevButtons[MARIO_COUNT];
	float prevStickAngle[MARIO_COUNT];


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


	// Description: set correct camera and player before player update to make player move after correct camera
	#define perform__6TMarioFUlPQ26JDrama9TGraphics         ((int (*)(...))0x8024D2A8)
	void TMario_perform_coop(TMario* mario, u32 param_1, JDrama::TGraphics* param_2) {
		u8 playerId = getPlayerId(mario);
		if(param_1 & 0x1) {
            /*CPolarSubCamera* camera = getCameraById(playerId);
            camera->perform(0x14, param_2);*/
			setActiveMario(playerId);
			if(!isSingleCameraLevel()) {
				setCamera(playerId);
			}

			
			if(isSingleCameraLevel()) {
				bool isDpadPressed = mario->mController->mButtons.mFrameInput & JUTGamePad::EButtons::DPAD_UP;
				CPolarSubCamera* originalCamera = getCameraById(0);
				if(isDpadPressed) {
					if(!isCameraCaptured) {
						for(int i = 0; i < getPlayerCount(); ++i) {
							TMario* m = getMario(i);
							const TVec3f* marioPos = &m->mTranslation;
							originalCamera->removeMultiPlayer(marioPos);
						}
						isCameraCaptured = true;
						focusedMario = playerId;
					} else {
						for(int i = 0; i < getPlayerCount(); ++i) {
							TMario* m = getMario(i);
							const TVec3f* marioPos = &m->mTranslation;
							originalCamera->addMultiPlayer(marioPos, marioPos->y, marioPos->z);
						}
						isCameraCaptured = false;
						focusedMario = playerId;
					}
				}
				//OSReport("Adding split screen camera for player %X and cam %X\n", i, originalCamera);
			}

		}

		if(hasCustomAnimations(playerId)) {
			//OSReport("Updating mactor for %X\n", playerId);
			MActor* marioMActor = modelData[playerId].marioMActor;
			if(param_1 & 0x1) {
				marioMActor->frameUpdate();
			}

			if((param_1 & 0x200) == 0) {
				perform__6TMarioFUlPQ26JDrama9TGraphics(mario, param_1, param_2);
			} else {
				marioMActor->entryIn();
				perform__6TMarioFUlPQ26JDrama9TGraphics(mario, param_1, param_2);
				marioMActor->entryOut();
			}
		} else {
			perform__6TMarioFUlPQ26JDrama9TGraphics(mario, param_1, param_2);
		}
		

		SimulateEscapeHeld(mario);
		
		// Fix for mario silhouette
		// The way sunshine deals with silhouette is by rendering mario to a separate buffer. Each frame the two buffers are swapped
		// Problem earlier was that each mario had their own buffer, meaning that only one shadow would be shown. We are enforcing here that all other marios 
		// Use player 1s buffer, this makes both players render to the same buffer and use that one buffer when rendering silhouettes
		// FIXME: MEMORY OPTIMIZATION: We don't technically need the other buffers, so those just use unecessary memory.
		if((param_1 & 0x10000000) != 0) {
			TMario* firstMario = getMario(0);
			if(firstMario != mario) {
				mario->mDrawBufferA = firstMario->mDrawBufferA;
				mario->mDrawBufferB = firstMario->mDrawBufferB;
				mario->_39C = firstMario->_39C;
				mario->_3A0 = firstMario->_3A0;
			}
		}

		if(param_1 & 0x1) {
			u8 currentId = getActiveViewport();
            /*CPolarSubCamera* camera = getCameraById(currentId);
            camera->perform(0x14, param_2);*/
			setActiveMario(currentId);
			setCamera(currentId);
		}
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803dd680, 0, 0, 0), (u32)(&TMario_perform_coop));


	bool SMS_isMultiPlayerMap_override() {
		return gCameraTypeSetting.getInt() == CameraTypeSetting::SINGLE && !isSingleplayerLevel() && !isPvpLevel() && !isCameraCaptured;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80021138, 0, 0, 0), SMS_isMultiPlayerMap_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80025438, 0, 0, 0), SMS_isMultiPlayerMap_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80026010, 0, 0, 0), SMS_isMultiPlayerMap_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x80039540, 0, 0, 0), SMS_isMultiPlayerMap_override);
	
	// Description: Sets initial fields on load for player and makes player active. 
	// Note: This makes player spawn at level exits
	// TODO: Fix wrong level exit animation
	void SetMario(TMarDirector* director) {
		bool bootState = TFlagManager::smInstance->getFlag(0x30006);
		CPolarSubCamera* originalCamera = getCameraById(0);
		marioIsSet = true;
		for (int i = loadedMarios-1; i >= 0; i--) {
			if(gCameraTypeSetting.getInt() == CameraTypeSetting::SINGLE && !isPvpLevel()) {
				TMario* mario = getMario(i);
				const TVec3f* marioPos = &mario->mTranslation;
				originalCamera->addMultiPlayer(marioPos, marioPos->y, marioPos->z);
				//OSReport("Adding split screen camera for player %X and cam %X\n", i, originalCamera);
			}
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
			if(i > 0) {
				TMario* mario = marios[i];
				if(mario->mPinnaRail) {
					J3DFrameCtrl* ctrl = mario->mPinnaRail->getFrameCtrl(0);
					ctrl->mCurFrame = 1238;

					MActor* cameraBck = *(MActor**)((u32)getCameraById(i) + 0x2b0);
					setFrame__10TCameraBckFf(cameraBck, 1238.0);
					//OSReport("Setting rail camera \n");
				}

				if(mario->mKoopaRail) {
					J3DFrameCtrl* ctrl = mario->mKoopaRail->getFrameCtrl(0);
					ctrl->mCurFrame = 1238;

					MActor* cameraBck = *(MActor**)((u32)getCameraById(i) + 0x2b0);
					setFrame__10TCameraBckFf(cameraBck, 1238.0);
					//OSReport("Setting rail camera \n");
				}
			}


			
			CPolarSubCamera* camera = getCameraById(i);
			spawnData[i].startPosition = marios[i]->mTranslation;
			spawnData[i].marioAngle = marios[i]->mRotation.y;
			spawnData[i].cameraPosition = camera->mTranslation;
			spawnData[i].cameraHorizontalAngle = camera->mHorizontalAngle;
		}
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
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

	// Description: Always update life counter when a diff
	// Normally it only updates on increments, which is stupid
	SMS_WRITE_32(SMS_PORT_REGION(0x801426d8, 0, 0, 0), 0x418200b0);
	

	// Fix tree collision
	SMS_WRITE_32(SMS_PORT_REGION(0x801f6cc4, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801f6dd0, 0, 0, 0), 0x60000000);
	
	// Fix cloud collision
	SMS_WRITE_32(SMS_PORT_REGION(0x801dfc1c, 0, 0, 0), 0x60000000);

	
	#define MarioFlagId_Lives 0x20001

	void loserExecOverride(TMario* mario) {
		if(isShineGot()) return;
		
		if(isPvpLevel()) {
			onDeathPvp();
		}

		int lives = TFlagManager::smInstance->getFlag(MarioFlagId_Lives);
		if(lives <= 0) {
			for(int i = 0; i < loadedMarios; ++i) {
				marios[i]->loserExec();
			}
			TFlagManager::smInstance->setFlag(MarioFlagId_Lives, 4);
			return;
		}

		TFlagManager::smInstance->decFlag(MarioFlagId_Lives, 1);
		lives -= 1;
		int marioId = getPlayerId(mario);
		
		mario->dropObject();


		for(int i = 0; i < loadedMarios; ++i) {
			TGCConsole2* console = getConsoleForPlayer(i);

			//// Reset min timer time to 0 and make the startMario appear (if it isn't open)
			*(u16*)((u32)console + 0x3ae) = 0;
			startAppearMario__11TGCConsole2Fb(console, true);

		}
		//for(int i = 0; i < 2; ++i) {
		//	//OSReport("Offset %X\n", (u32)(((u32*)consoles[i]+ 0x70/4)) - (u32)consoles[i]);
		//	*(u16*)(((u32*)consoles[i]+ 0x70/4)) = 200;
		//	startAppearMario__11TGCConsole2Fb(consoles[i], true);
		//}

		SpawnData* marioSpawnData = &spawnData[marioId];

		CPolarSubCamera* camera = getCameraById(marioId);
		mario->mHealth = 8;
		mario->mWaterHealth = 8.0;
		// camera->position = marioSpawnData->cameraPosition;
		// camera->position.y += 1000.0f;
		mario->warpRequest(marioSpawnData->startPosition, spawnData[marioId].marioAngle);
		mario->mTranslation = marioSpawnData->startPosition;
		mario->mTranslation.y += 100.0;
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
		mario->mFludd->mCurrentWater = mario->mFludd->mNozzleList[mario->mFludd->mCurrentNozzle]->mEmitParams.mAmountMax.get();
		//mario->warpOut();
		mario->setAnimation(0xc3, 1.0);
		mario->changePlayerStatus(0x1337, 0x200, true);
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

	// Warp all marios in case of warpMario sunscript
	
	void SMS_MarioWarpRequest(double param_1, TVec3f position) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			setActiveMario(i);
			getMario(i)->warpRequest(position, param_1);

			f32 offset = -75.0f + 150.0f * i;
			volatile float angle = 2.0f * 3.1415 * marios[i]->mAngle.y / 65535.0f - 3.1415/2.0;
			f32 offsetX = sin(angle) * offset;
			f32 offsetZ = cos(angle) * offset;
			marios[i]->mTranslation.x += offsetX;
			marios[i]->mTranslation.z += offsetZ;

		}
		setActiveMario(getActiveViewport());
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8028a9f4, 0, 0, 0), SMS_MarioWarpRequest);
	SMS_PATCH_BL(SMS_PORT_REGION(0x801a0994, 0, 0, 0), SMS_MarioWarpRequest);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8018fc2c, 0, 0, 0), SMS_MarioWarpRequest);

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


}