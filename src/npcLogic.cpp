#include <SMS/Strategic/HitActor.hxx>
#include <sdk.h>
#include "player.hxx"
#include <raw_fn.hxx>
#include <JSystem/JDrama/JDRActor.hxx>
#include <SMS/Enemy/SpineBase.hxx>

#include "splitscreen.hxx"
#include "camera.hxx"

#define GetObjectFunction( object, func ) (void*)*(void**)((int)*(int*)object + func)

namespace SMSCoop {
	// TODO: Reset on stage load
	char canRocketMountMario[2] = {1, 1};

	TMario* marioEnteringGate = nullptr;

	void resetNpcLogic(TMarDirector *director) {
		for(int i = 0; i < 2; ++i) {
			canRocketMountMario[i] = 1;
		}
		marioEnteringGate = nullptr;
	}

	TMario* getMarioEnteringGate() {
		return marioEnteringGate;
	}

	void TRocket_attackToMario(JDrama::TActor* rocket) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		if(canRocketMountMario[cm] == 0) return;
		setActiveMario(cm);

		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];
		char* testing = (char*)((int)liveManager + 0x68);

		attackToMario__7TRocketFv(rocket);

		canRocketMountMario[cm] = *canHaveRocket;

		setActiveMario(0);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcd08, 0, 0, 0), (u32)(&TRocket_attackToMario));

	void TRocket_bind(JDrama::TActor* rocket, int __fd, void* __addr, void* __len) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		setActiveMario(cm);
	
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];

		bind__7TRocketFv(rocket);
		canRocketMountMario[cm] = *canHaveRocket;
		setActiveMario(0);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcc6c, 0, 0, 0), (u32)(&TRocket_bind));

	void TRocket_calcRootMatrix(JDrama::TActor* rocket) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		setActiveMario(cm);
		calcRootMatrix__7TRocketFv(rocket);
		setActiveMario(0);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcc60, 0, 0, 0), (u32)(&TRocket_calcRootMatrix));

	void TRocket_setDeadAnm(JDrama::TActor* rocket) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];

		setDeadAnm__7TRocketFv(rocket);

		canRocketMountMario[cm] = *canHaveRocket;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bccf4, 0, 0, 0), (u32)(&TRocket_setDeadAnm));

	u32 TNerveRocketFly_execute(TNerveBase<JDrama::TActor>* nerveRocketFly, TSpineBase<JDrama::TActor>* rocketSpineBase) {
		JDrama::TActor* rocket = rocketSpineBase->mTarget;
		int cm = getClosestMarioId(&rocket->mTranslation);
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];

		u32 result = execute__15TNerveRocketFlyCFP24TSpineBase_1(nerveRocketFly, rocketSpineBase);
	
		canRocketMountMario[cm] = *canHaveRocket;
		return result;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcb8c, 0, 0, 0), (u32)(&TNerveRocketFly_execute));

	u32 TNerveRocketPossessedNozzle_execute(TNerveBase<JDrama::TActor>* nerveRocketFly, TSpineBase<JDrama::TActor>* rocketSpineBase) {
		JDrama::TActor* rocket = rocketSpineBase->mTarget;
		int cm = getClosestMarioId(&rocket->mTranslation);
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];
	
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		auto* p1Gamepad = director->mGamePads[0];
		director->mGamePads[0] = director->mGamePads[cm];

		u32 result =  execute__27TNerveRocketPossessedNozzleCFP24TSpineBase_1(nerveRocketFly, rocketSpineBase);
	
		director->mGamePads[0] = p1Gamepad;
		canRocketMountMario[cm] = *canHaveRocket;
		return result;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcb9c, 0, 0, 0), (u32)(&TNerveRocketPossessedNozzle_execute));

	//typedef void (*controlFunc)(THitActor* smallEnemy);
	//void TSmallEnemy_control(THitActor* smallEnemy) {
	//
	//	int cm = getClosestMarioId(&smallEnemy->mTranslation);
	//	setActiveMario(cm);
	//
	//
 //		auto control = (controlFunc) GetObjectFunction(smallEnemy, 200);

	//
	//	control(smallEnemy);

	//	//executeNerve(nerve, spineBase);
	//	setActiveMario(0);
	//}
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8006c6f8, 0, 0, 0), TSmallEnemy_control);
	
	// Note: param_1 might not be a THitActor, i just optimistically assume so atm
	void TEnemyManager_TViewObj_testPerform(THitActor* hitActor, u32 renderFlags, JDrama::TGraphics* graphics) {
	
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		if(director->mAreaID != TGameSequence::Area::AREA_MAREBOSS) {
			int cm = getClosestMarioId(&hitActor->mTranslation);
			setActiveMario(cm);
			setCamera(cm);
		}
		testPerform__Q26JDrama8TViewObjFUlPQ26JDrama9TGraphics(hitActor, renderFlags, graphics);
		
		setCamera(0);
		setActiveMario(0);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8003e2f0, 0, 0, 0), TEnemyManager_TViewObj_testPerform);
	// TMapObj
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021c66c, 0, 0, 0), TEnemyManager_TViewObj_testPerform);
	// TEffectObjManager 
	SMS_PATCH_BL(SMS_PORT_REGION(0x80034244, 0, 0, 0), TEnemyManager_TViewObj_testPerform);
	
	// Note: param_1 might not be a THitActor, i just optimistically assume so atm
	void testPerform_each_mario(THitActor* hitActor, u32 renderFlags, JDrama::TGraphics* graphics) {
	
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		if(getPlayerCount() > 0 && director->mAreaID != TGameSequence::Area::AREA_MAREBOSS) {
			int currentMarioId = getPlayerId(gpMarioOriginal);
			int i = getActivePerspective();
			setActiveMario(i);
			setCamera(i);
			testPerform__Q26JDrama8TViewObjFUlPQ26JDrama9TGraphics(hitActor, renderFlags, graphics);
			setCamera(currentMarioId);
			setActiveMario(currentMarioId);
		} else {
				testPerform__Q26JDrama8TViewObjFUlPQ26JDrama9TGraphics(hitActor, renderFlags, graphics);
		}
		
		
	}

	SMS_PATCH_BL(SMS_PORT_REGION(0x802faad8, 0, 0, 0), testPerform_each_mario);
	 
	SMS_PATCH_BL(SMS_PORT_REGION(0x802a0450, 0, 0, 0), testPerform_each_mario);
	SMS_PATCH_BL(SMS_PORT_REGION(0x802a4ea8, 0, 0, 0), testPerform_each_mario);

	
	void TModelGate_perform(THitActor* modelGate, u32 performFlags, JDrama::TGraphics* graphics) {

		int currentMario = getPlayerId(gpMarioOriginal);

		int cm = getClosestMarioId(&modelGate->mTranslation);
		setActiveMario(cm);
		setCamera(cm);
		perform__10TModelGateFUlPQ26JDrama9TGraphics(modelGate, performFlags, graphics);
		
		setCamera(currentMario);
		setActiveMario(currentMario);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3fbc, 0, 0, 0), (u32)(&TModelGate_perform));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3fbc, 0, 0, 0), (u32)(&TModelGate_perform));

	// TODO: Fix sky in another way
	int TMario_receiveMessage_enterGate(TMario* mario, THitActor* shineGate, u32 msg) {
		if(marioEnteringGate != nullptr) return 0;
		mario->dropObject();
		int result = mario->receiveMessage(shineGate, msg);
		if(result == 1) {
			marioEnteringGate = mario;
		}
		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801eb28c, 0, 0, 0), TMario_receiveMessage_enterGate);

	// Make the enter M cutscene play on the correct screen
	u8 updateGameMode(TMarDirector* marDirector) {
		int currentMario = getPlayerId(gpMarioOriginal);
        TMario* marioEnteringGate = getMarioEnteringGate();
		if(marioEnteringGate) {
			int cm = getPlayerId(marioEnteringGate);
			setActiveMario(cm);
			setCamera(cm);
		}
		TMario* playerTalking = getTalkingPlayer();
		if(playerTalking) {
			int cm = getPlayerId(playerTalking);
			setActiveMario(cm);
			setCamera(cm);
		}


		u8 result = updateGameMode__12TMarDirectorFv(marDirector);

		setCamera(currentMario);
		setActiveMario(currentMario);
		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80299140, 0, 0, 0), updateGameMode);

	TMario* marioControllingLeaf = nullptr;
	void TLeafBoat_control(void* leafBoat) {
		int currentMario = getPlayerId(gpMarioOriginal);

		int currentPerspective = getActivePerspective();
		setActiveMario(currentPerspective);

		if(marioControllingLeaf) {
			int controllingMario = getPlayerId(marioControllingLeaf);
			setActiveMario(controllingMario);
		}
		
		control__9TLeafBoatFv(leafBoat);

		if(marioIsOn__11TMapObjBaseCFv(leafBoat) && gpMarioOriginal->mFludd->isEmitting()) {
			marioControllingLeaf = gpMarioOriginal;
		} else {
			marioControllingLeaf = nullptr;
		}

		setActiveMario(currentMario);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801c5ec8, 0, 0, 0), TLeafBoat_control);
	SMS_WRITE_32(SMS_PORT_REGION(0x803cca14, 0, 0, 0), (u32)(&TLeafBoat_control));

	TMario* marioControllingBoat = nullptr;
	
	void TMuddyBoat_control(void* muddyBoat) {
		int currentMario = getPlayerId(gpMarioOriginal);

		int currentPerspective = getActivePerspective();
		setActiveMario(currentPerspective);

		if(marioControllingBoat) {
			int controllingMario = getPlayerId(marioControllingBoat);
			setActiveMario(controllingMario);
		}

		control__10TMuddyBoatFv(muddyBoat);

		if(marioIsOn__11TMapObjBaseCFv(muddyBoat) && gpMarioOriginal->mFludd->isEmitting()) {
			marioControllingBoat = gpMarioOriginal;
		} else {
			marioControllingBoat = nullptr;
		}

		setActiveMario(currentMario);

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d166c, 0, 0, 0), (u32)(&TMuddyBoat_control));


	class TSmallEnemy : public THitActor {
	public:
		u32 _0x68[140/4]; // 0x68
		THitActor* mTarget; // 0xf4
		TVec3f mTargetPos; // 0xf8
		THitActor* mTarget2; // 0x104
		TVec3f mTargetPos2; // 0x108
	};

	// Fixes small enemy not changing target when another player is close
	// Fixes for SamboHead (Pokey head) and THanaSambo (Pokey)
	void TSmallEnemy_perform(TSmallEnemy* smallEnemy, u32 performFlags, JDrama::TGraphics* graphics) {

		TMario* closestMario = getMarioById(getClosestMarioId(&smallEnemy->mTranslation));

		smallEnemy->mTarget = closestMario;
		smallEnemy->mTarget2 = closestMario;
		smallEnemy->mTargetPos = closestMario->mTranslation;
		smallEnemy->mTargetPos2 = closestMario->mTranslation;
		
		perform__11TSmallEnemyFUlPQ26JDrama9TGraphics(smallEnemy, performFlags, graphics);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b9e2c, 0, 0, 0), (u32)(&TSmallEnemy_perform));
	SMS_PATCH_BL(SMS_PORT_REGION(0x800e3058, 0, 0, 0), TSmallEnemy_perform);

	
	// Fixes Petey (Boss Pakkun) swallow not triggering when petey is not facing both players
	void TBPHeadHit_receiveMessage(JDrama::TPlacement* bpHead, THitActor* sender, u32 message) {
		int currentMario = getPlayerId(gpMarioOriginal);
		int closestMario = getClosestMarioId(&bpHead->mTranslation);

		setActiveMario(closestMario);

		receiveMessage__10TBPHeadHitFP9THitActorUl(bpHead, sender, message);

		setActiveMario(currentMario);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b4870, 0, 0, 0), (u32)(&TBPHeadHit_receiveMessage));

		
	// Fixes Petey (Boss Pakkun) swallow not triggering when petey is not facing both players
	void TEnemyMario_checkPlayerAction(TMario* enemyMario, JDrama::TGraphics* graphics) {
		int currentMario = getPlayerId(gpMarioOriginal);
		int closestMario = getClosestMarioId(&enemyMario->mTranslation);

		setActiveMario(closestMario);

		enemyMario->checkPlayerAction(graphics);

		setActiveMario(currentMario);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x800400ac, 0, 0, 0), TEnemyMario_checkPlayerAction);

	
	// Make things perform when still talking
	SMS_WRITE_32(SMS_PORT_REGION(0x80216eac, 0, 0, 0), 0x60000000); // npc movement
	SMS_WRITE_32(SMS_PORT_REGION(0x801afb88, 0, 0, 0), 0x60000000); // object movement
	SMS_WRITE_32(SMS_PORT_REGION(0x801be828, 0, 0, 0), 0x60000000); // perform coins
	SMS_WRITE_32(SMS_PORT_REGION(0x80213bc4, 0, 0, 0), 0x60000000); // spray things when talking
	SMS_WRITE_32(SMS_PORT_REGION(0x800ed56c, 0, 0, 0), 0x60000000); // spray things when talking
	SMS_WRITE_32(SMS_PORT_REGION(0x8006b09c, 0, 0, 0), 0x60000000); // allow small enemies to move
	SMS_WRITE_32(SMS_PORT_REGION(0x80255f1c, 0, 0, 0), 0x60000000); // Not invincible while talking (This is overriden so only the talking player is invincible)
	//SMS_WRITE_32(SMS_PORT_REGION(0x80255ec0, 0, 0, 0), 0x4800000c); // Not invincible while talking (This is overriden so only the talking player is invincible)
	SMS_WRITE_32(SMS_PORT_REGION(0x802536fc, 0, 0, 0), 0x60000000); // Affected by fire goop
	//SMS_WRITE_32(SMS_PORT_REGION(0x802536a0, 0, 0, 0), 0x4800000c); // Affected by fire goop
	SMS_WRITE_32(SMS_PORT_REGION(0x802532f0, 0, 0, 0), 0x60000000); // Affected by electric goop
	//SMS_WRITE_32(SMS_PORT_REGION(0x80253294, 0, 0, 0), 0x4800000c); // Affected by electric goop

	SMS_WRITE_32(SMS_PORT_REGION(0x8024fe80, 0, 0, 0), 0x60000000); // Affected by electric goop

	// Disable eel demo camera
	SMS_WRITE_32(SMS_PORT_REGION(0x800d1540, 0, 0, 0), 0x60000000); // Affected by electric goop

	
}