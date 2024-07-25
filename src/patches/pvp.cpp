#include <sdk.h>
#include <macros.h>

#include <SMS/MoveBG/ResetFruit.hxx>
#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <SMS/Manager/ItemManager.hxx>
#include <SMS/System/MovieDirector.hxx>
#include <SMS/System/MarNameRefGen.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/Manager/PollutionManager.hxx>

#include <raw_fn.hxx>

#include <players.hxx>
#include <BetterSMS/memory.hxx>

#include <rand.h>

#define MAX_SPAWNED_POWERUPS 2
#define POWERUP_SPAWN_TIME 60 * 60
#define SPAWN_LOCATIONS 9

u32* touchedWaterId = (u32*)0x803fc31c;
namespace SMSCoop {
	u8 waterEmitFromId[0x100];
	u8 currentPlayer = 0;
	
	int timeTilNextPowerup = 0;
	int nextPowerupId = 0;
	TItem* spawnedPowerups[MAX_SPAWNED_POWERUPS]; 
	int spawnedPowerupLocations[MAX_SPAWNED_POWERUPS] {-1, -1};
	bool firstLoad = true;
	TWaterEmitInfo* ballEmitInfo;

	TVec3f spawnLocations[SPAWN_LOCATIONS] {
		{ -6516.119141, 1425.000000, 189.293991 },
		{ -57.929207, 300.000000, 3290.058350 },
		{ 6525.496094, 1250.000000, 3736.353516 },
		{ 13683.542969, 415.538910, -10134.316406 },
		{ 2132.935791, 300.000031, -2986.915527 },
		{ -6838.304688, 300.000000, -6143.696289 },
		{ -1735.637451, 700.000000, -5965.358887 },
		{ -3511.437988, 2900.000000, -10438.366211 },
		{ -10510.169922, 300.000000, 2872.700195 }
	};

	enum POWERUP_TYPE {
		NONE,
		NOZZLE_SPRAY,
		NOZZLE_HOVER,
		NOZZLE_TURBO,
		NOZZLE_ROCKET,
		COINS,
		BALLS,
		ELECTRIC,
		COUNT
	};

	struct PowerupStates {
		POWERUP_TYPE powerup;
		int powerupTimer;
		int fluddMaxLevel;
		int maxPowerupTime;
		bool disableNozzleSwitch = false;
		bool isGoopNozzle = false;
		int superTime = 0;
	} powerupStates[2];

	void initPvp(TMarDirector* marDirector) {
		nextPowerupId = 0;
		timeTilNextPowerup = 0;
		firstLoad = true;
		spawnedPowerupLocations[0] = -1;
		spawnedPowerupLocations[1] = -1;
		for(int i = 0; i < MAX_SPAWNED_POWERUPS;++i) {
			spawnedPowerups[i] = nullptr;
		}

		ballEmitInfo = (TWaterEmitInfo*)JKRHeap::sCurrentHeap->alloc(sizeof(TWaterEmitInfo), 32);
		__ct__14TWaterEmitInfoFPCc(ballEmitInfo);

		for(int i = 0; i < 2; ++i) {
			PowerupStates& state = powerupStates[i];
			state.powerup = NONE;
			state.powerupTimer = 0;
			state.maxPowerupTime = 0;
			state.disableNozzleSwitch = false;
			state.isGoopNozzle = false;
			state.superTime = 0;
		}

	}

	bool isPvpLevel() {
		return gpApplication.mCurrentScene.mAreaID == 0xC && gpApplication.mCurrentScene.mEpisodeID == 1;
	}

	void TModelWaterManager_makeEmit_override(TModelWaterManager* waterManager, const TWaterEmitInfo& emitInfo) {
		s16* emittedWater = (s16*)(((u32)waterManager) + 0x12);

		waterEmitFromId[*emittedWater] = currentPlayer;
		waterManager->makeEmit(emitInfo);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8027fce0, 0, 0, 0), TModelWaterManager_makeEmit_override);

	u8 TModelWaterManager_emitRequest_player_override(TModelWaterManager* waterManager, const TWaterEmitInfo& emitInfo) {
		currentPlayer = getPlayerId(gpMarioOriginal) + 1;
		u8 result = waterManager->emitRequest(emitInfo);
		currentPlayer = 0;
		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8026b788, 0, 0, 0), TModelWaterManager_emitRequest_player_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8026c130, 0, 0, 0), TModelWaterManager_emitRequest_player_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8026ccfc, 0, 0, 0), TModelWaterManager_emitRequest_player_override);

	void checkWater_override(void* tObjHitCheck) {
		// Only collide with water in pvp level
		if(!isPvpLevel()) {
			checkWater__12TObjHitCheckFv(tObjHitCheck);
			return;
		}
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMario(i);
			//OSReport("Testing %X %X\n", mario->mObjectID & 0x80000000, mario->mObjectType & 0x4);
			mario->mObjectID &= ~0x80000000;
			//mario->mObjectType &= ~0x4;
		}
		checkWater__12TObjHitCheckFv(tObjHitCheck);
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMario(i);
			mario->mObjectID |= 0x80000000;
			//mario->mObjectType |= 0x4;
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021bb28, 0, 0, 0), checkWater_override);


	void TModelWaterManager_garbageCollect_fix(TModelWaterManager* waterManager) {
		if(isPvpLevel()) {
			u16 activeWaterCount = 0;
			// We do this twice, which is slow, but oh well
			for(u16 waterId = 0; waterId < waterManager->mEmitCount; ++waterId) {
				// If not a dead particle, move up to earliest slot
				if(0.0f < waterManager->mWaterAliveTime[waterId]) {
					if(activeWaterCount != waterId) {
						waterEmitFromId[activeWaterCount] = waterEmitFromId[waterId];
					}

					activeWaterCount++;
				}
			
			}
		}
		//waterManager->mEmitCount = activeWaterCount;
		garbageCollect__18TModelWaterManagerFv(waterManager);

		//OSReport("Address offset %X\n", waterManager->mMaxEmit);
		//OSReport("Is this alive time? %d %d %d\n", waterManager->mUnk[0], waterManager->mUnk[1], waterManager->mUnk[2]);

		//u32* address = (u32*)((u32)(gpModelWaterManager) + 0x2914);
		/*
		int context;
		SMS_FROM_GPR(04, context);*/

		//OSReport("Index %d\n", context);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8027fa60, 0, 0, 0), TModelWaterManager_garbageCollect_fix);
	
	TItemManager** gpItemManagerRef = (TItemManager**)0x8040df10;
	void givePowerupToPlayer(TMario* mario, POWERUP_TYPE powerup) {
		int playerId = getPlayerId(mario);
		PowerupStates& powerupState = powerupStates[playerId];
		mario->emitGetWaterEffect();

		while(mario->mHealth >= 8 && powerup == POWERUP_TYPE::COINS) {
			powerup = (POWERUP_TYPE)((rand() % ((int)POWERUP_TYPE::COUNT - 1)) + 1);
		}
		TNozzleBase *nozzle;

		switch(powerup) {
			case POWERUP_TYPE::NOZZLE_SPRAY: {
				mario->mAttributes.mHasFludd = true;
				mario->mFludd->changeNozzle(TWaterGun::Spray, true);
				powerupState.disableNozzleSwitch = true;
				powerupState.powerupTimer = 30 * 40; // 40 seconds
				nozzle = mario->mFludd->mNozzleList[TWaterGun::Spray];
				powerupState.fluddMaxLevel = nozzle->mEmitParams.mAmountMax.get();
				powerupState.isGoopNozzle = (rand() % 2) == 0;
				break;
			}
			case POWERUP_TYPE::NOZZLE_HOVER: {
				mario->mAttributes.mHasFludd = true;
				mario->mFludd->changeNozzle(TWaterGun::Hover, true);
				powerupState.disableNozzleSwitch = true;
				powerupState.powerupTimer = 30 * 40; // 40 seconds
				nozzle = mario->mFludd->mNozzleList[TWaterGun::Hover];
				powerupState.fluddMaxLevel = nozzle->mEmitParams.mAmountMax.get();
				powerupState.isGoopNozzle = (rand() % 2) == 0;
				break;
			}
			case POWERUP_TYPE::NOZZLE_TURBO: {
				mario->mAttributes.mHasFludd = true;
				mario->mFludd->changeNozzle(TWaterGun::Turbo, true);
				powerupState.disableNozzleSwitch = true;
				powerupState.powerupTimer = 30 * 40; // 40 seconds
				nozzle = mario->mFludd->mNozzleList[TWaterGun::Turbo];
				powerupState.fluddMaxLevel = nozzle->mEmitParams.mAmountMax.get();
				powerupState.isGoopNozzle = (rand() % 2) == 0;
				break;
			}
			case POWERUP_TYPE::NOZZLE_ROCKET: {
				mario->mAttributes.mHasFludd = true;
				mario->mFludd->changeNozzle(TWaterGun::Rocket, true);
				powerupState.disableNozzleSwitch = true;
				powerupState.powerupTimer = 30 * 40; // 40 seconds
				nozzle = mario->mFludd->mNozzleList[TWaterGun::Rocket];
				powerupState.fluddMaxLevel = nozzle->mEmitParams.mAmountMax.get();
				powerupState.isGoopNozzle = (rand() % 2) == 0;
				break;
			}
			case COINS: {
				TItemManager* manager = *gpItemManagerRef;
				TVec3f vec = mario->mTranslation;
				int coins = 3 + (rand() % 7);
				for(int i = 0; i < coins; ++i) {
					TItem *spawnedItem = manager->makeObjAppear(vec.x, vec.y + 800.0f, vec.z, 0x2000000e,true);
					if(spawnedItem) {
						spawnedItem->killByTimer(10 * 4 * 60);
						spawnedItem->mSpeed.x = ((f32)(s16)rand() / 32767.5) * 40.0f - 20.0f;
						spawnedItem->mSpeed.y = ((f32)(s16)rand() / 32767.5) * 40.0f;
						spawnedItem->mSpeed.z = ((f32)(s16)rand() / 32767.5) * 40.0f - 20.0f;
						spawnedItem->mStateFlags.asU32 &= 0xffffffef;
						spawnedItem->killByTimer(0x3c0);
					}
				}
				break;
			}
			case BALLS: {
				TItemManager* manager = *gpItemManagerRef;
				TVec3f vec = mario->mTranslation;
				TApplication *app      = &gpApplication;
				TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
				char objName[10];
				for(int i = 0; i < 10; ++i) {
					snprintf(objName, 10, "football%d",i );
					u16 footbalCode = JDrama::TNameRef::calcKeyCode(objName);
					TMapObjBall *football = (TMapObjBall*)director->mViewObjRoot->searchF(footbalCode, objName);
					if(football) {
						football->mTranslation = mario->mTranslation;
						football->mSpeed.x = ((f32)(s16)rand() / 32767.5) * 40.0f - 20.0f;
						football->mSpeed.y = ((f32)(s16)rand() / 32767.5) * 40.0f;
						football->mSpeed.z = ((f32)(s16)rand() / 32767.5) * 40.0f - 20.0f;
						football->mStateFlags.asU32 &= 0xffffffef;
					}
				}
				break;
			}
			case ELECTRIC: {
				powerupState.superTime = 30*60; // 60 sec
				break;
			}
			case NONE: {
				break;
			}
			case COUNT: {
				break;
			}
		}
		
		powerupState.maxPowerupTime = powerupState.powerupTimer;
	}
	
	void touchPlayerPvp(TMario* playerTouching, TMario* playerTouched) {
		int playerId = getPlayerId(playerTouching);
		int touchedPlayer = getPlayerId(playerTouched);
		auto& powerups = powerupStates[playerId];
		auto& touchedPowerup = powerupStates[touchedPlayer];
		if(powerups.superTime > 0 && touchedPowerup.superTime <= 0) {
			playerTouched->elecEffect();
			playerTouched->damageExec(playerTouching, 1, 0, 0, 0.0f, 0, 0, 300);
			return;
		}
		if(touchedPowerup.superTime > 0 && powerups.superTime <= 0) {
			playerTouching->elecEffect();
			playerTouching->damageExec(playerTouched, 1, 0, 0, 0.0f, 0, 0, 300);
			return;
		}

		if(playerTouching->mState == TMario::State::STATE_G_POUND && !(playerTouching->mHeldObject == playerTouched || playerTouched->mHeldObject == playerTouching)) {
							
			playerTouched->damageExec(playerTouching, 1, 0, 0, 0.0f, 0, 0, 300);
			return;
		}
	}
	void spawnPowerup(int id) {
		TItemManager* manager = *gpItemManagerRef;
		
		TItem *currentPowerup = spawnedPowerups[id];
		if(currentPowerup) {
			currentPowerup->kill();
			spawnedPowerups[id] = nullptr;
		}

		// Prevent same place spawn
		int location = rand() % SPAWN_LOCATIONS;
		while(location == spawnedPowerupLocations[0] || location == spawnedPowerupLocations[1]) {
			location = rand() % SPAWN_LOCATIONS;
		}
		//OSReport("Spaning item with id %d at location %d!\n", id, location);
		TVec3f vec = spawnLocations[location];
		
		TItem *spawnedItem = manager->makeObjAppear(vec.x, vec.y + 20.0f, vec.z, 0x20000002,true);
		if(spawnedItem) {
			// Expire after some time
			spawnedItem->killByTimer(100000); // because of co-op jankyness
			spawnedPowerups[id] = spawnedItem;
			spawnedPowerupLocations[id] = location;
			//OSReport("It was spawned!\n");
		} else {
			//OSReport("IT DID NOT SPAWWWWN\n\n\n\n\n\n\n");
		}
		
		timeTilNextPowerup = POWERUP_SPAWN_TIME;
		nextPowerupId = id + 1;
		if(nextPowerupId >= MAX_SPAWNED_POWERUPS) {
			nextPowerupId = 0;
		}
	}

	#define receiveMessage_TMario         ((int (*)(...))0x80282af4)
	bool TMario_receiveMessage_override(TMario* mario, THitActor *sender, u32 msg) {
		if(isPvpLevel()) {
			// Hit by water
			if(sender == &gpModelWaterManager->mStaticHitActor) {
				u8 playerShootingWater = waterEmitFromId[*touchedWaterId];
				if(playerShootingWater != 0 && playerShootingWater != (getPlayerId(mario) + 1)) {
					mario->damageExec(getMario((int)playerShootingWater-1), 1, 0, 0, 50.0f, 0, 0, 300);
				}
			}

			for(int i = 0; i < MAX_SPAWNED_POWERUPS; ++i) {
				TItem *spawnedPowerup = spawnedPowerups[i];
				if(sender == spawnedPowerup) {
					TWaterGun *fludd = mario->mFludd;
					if(fludd) {
						//OSReport("Collected item %d\n", i);
						int powerupId = (rand() % ((int)POWERUP_TYPE::COUNT - 1)) + 1;
						givePowerupToPlayer(mario, (POWERUP_TYPE)powerupId);
						//spawnPowerup(i);
						nextPowerupId = i;
						timeTilNextPowerup = 5;
					}

				}
			}
		}
		return receiveMessage_TMario(mario, sender, msg);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803dd700, 0, 0, 0), (u32)(&TMario_receiveMessage_override));

	void TWaterGun_changeBackup_override(TWaterGun *athis) {
		if(isPvpLevel()) {
			for(int i = 0; i < getPlayerCount(); ++i) {
				TWaterGun* playerGun = getMario(i)->mFludd;
				if(athis == playerGun && powerupStates[i].disableNozzleSwitch) {
					MSoundSESystem::MSoundSE::startSoundSystemSE(MS_SOUND_EFFECT::MSD_SE_SY_NOT_COLLECT, 0, 0, 0);
					return;
				}
			}
		}
		athis->changeBackup();
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024e628, 0, 0, 0), TWaterGun_changeBackup_override);

	// Since we use watertank as a timer, we must disable the succ
	void TMario_gunExec_override(TMario* athis) {
		bool inWater = athis->mAttributes.mIsWater;
		if(isPvpLevel()) {
			athis->mAttributes.mIsWater = false;
		}
		athis->gunExec();
		athis->mAttributes.mIsWater = inWater;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024e2a0, 0, 0, 0), TMario_gunExec_override);

	// Load level after epilogue
	u32* nextMovieRef = (u32*)0x803e9718;
	s32 TMovieDirector_decideNextMode_override(TMovieDirector* director, s32* param_1) {
		s32 response = director->decideNextMode(param_1);
		u32 nextMovie = *nextMovieRef;
		// Next movie is staff roll
		if(nextMovie == 0xf && response == 6) {
			*nextMovieRef = 0x0;
			gpApplication.mNextScene.set(0xC, 1, 0);
			return 5;
		}
		return response;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802b5f48, 0, 0, 0), TMovieDirector_decideNextMode_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802b606c, 0, 0, 0), TMovieDirector_decideNextMode_override);

	u32* stageBgm = (u32*)0x8040e1f0;
	void setMSoundEnterStage_override(void* a, u8 param_1, u8 param_2) {
		setMSoundEnterStage__10MSMainProcFUcUc(a, param_1, param_2);

		if(isPvpLevel()) {
			*stageBgm = 0x80010019;
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802b7a4c, 0, 0, 0), setMSoundEnterStage_override);

	void TPollutionManager_clean(TPollutionManager* pollutionManager, f32 x, f32 y, f32 z, f32 r) {
		if(isPvpLevel()) {
			u32 waterId = *touchedWaterId;
			u8 waterData = waterEmitFromId[waterId];

			if(waterData != 0 && powerupStates[(int)waterData-1].isGoopNozzle) {
				pollutionManager->stamp(1, x, y, z, r);
				return;
			}
		}
		pollutionManager->clean(x, y, z, r);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8027f7dc, 0, 0, 0), TPollutionManager_clean);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8027f94c, 0, 0, 0), TPollutionManager_clean);

	void TMario_checkGrafitoElec_override(TMario* mario) {
		if(!isPvpLevel() || powerupStates[getPlayerId(mario)].superTime <= 0) {
			mario->checkGraffitoElec();
			return;
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x802530c0, 0, 0, 0), TMario_checkGrafitoElec_override);

	SMS_WRITE_32(SMS_PORT_REGION(0x8017d0a8, 0, 0, 0), 0x4e800020);

	void onDeathPvp() {
		TApplication *app      = &gpApplication;
		TMarDirector *marDirector = reinterpret_cast<TMarDirector *>(app->mDirector);
		if((marDirector->mGameState & 0x100) == 0) {
			marDirector->mGameState |= 0x100;
			marDirector->setNextStage(0xe06, nullptr);
			*nextMovieRef = 0xf;
		}
	}
	
	u32** gpBeamManager = (u32**)(0x8040d8d0);
	void drawPvp() {
		for(int i = 0; i < MAX_SPAWNED_POWERUPS; ++i) {
			TItem *spawnedPowerup = spawnedPowerups[i];
			if(spawnedPowerup) {
				TVec3f vec = spawnedPowerup->mTranslation;
				TVec3f vec2 = spawnedPowerup->mTranslation;
				vec2.y += 8000;
				requestCone__12TBeamManagerFRCQ29JGeometry8TVec3_f(*gpBeamManager, &vec2, &vec, 200.0f, true, false, false);
				requestCone__12TBeamManagerFRCQ29JGeometry8TVec3_f(*gpBeamManager, &vec2, &vec, 100.0f, true, false, false);
			}
		}
	}

	void updatePvp(TMarDirector* marDirector) {
		//OSReport("Testing %d\n", gpApplication.mCutSceneID)
		if(!isPvpLevel() || getPlayerCount() <= 1) {
			return;
		}

		if(firstLoad) {
			for(int i = 0; i < MAX_SPAWNED_POWERUPS; ++i) {
				spawnPowerup(i);
			}
			firstLoad = false;
		}

		TMario* mario = getMario(0);
		//OSReport("Testing %f %f %f\n", mario->mSpeed.x, mario->mSpeed.y, mario->mSpeed.z);
			
		if(timeTilNextPowerup <= 0) {
			//OSReport("Item expired %d\n", nextPowerupId);
			spawnPowerup(nextPowerupId);
		}
			
		// TODO: Track which mario has powerup'
		for(int i = 0; i < getPlayerCount(); ++i) {
			auto& powerups = powerupStates[i];
			TMario* mario = getMario(i);
			if(powerups.powerupTimer > 0) {
				powerups.powerupTimer--;
				

				TWaterGun *fludd = mario->mFludd;
				if (!fludd)
					return;

				mario->mFludd->mCurrentWater = powerups.fluddMaxLevel * powerups.powerupTimer / powerups.maxPowerupTime;

				if(powerups.powerupTimer == 0) {
					powerups.disableNozzleSwitch = false;
					mario->mAttributes.mHasFludd = false;
					powerups.isGoopNozzle = false;
				}
			}

			if(powerups.superTime > 0) {
				mario->elecEffect();
				powerups.superTime -= 1;
			}
		}


		timeTilNextPowerup--;
		drawPvp();

	}
}