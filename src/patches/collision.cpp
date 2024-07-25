#include <sdk.h>

#include <raw_fn.hxx>

#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Player/Mario.hxx>

#include "SMS/MSound/MSoundSESystem.hxx"

#include "players.hxx"
#include "splitscreen.hxx"
#include "camera.hxx"
#include "pvp.hxx"

namespace SMSCoop {
	
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
				if ((mario2->mState & 0xFFFFFFF0) != 0x000024D0) {
					changePlayerStatus__6TMarioFUlUlb(mario2, 0x000208b0, 0, 0);
				}
			}
			return;
		}

		TVec3f temp;
		temp.x = 0.5f;
		temp.y = 0.5f;
		temp.z = 0.5f;

		//if(isDiving) {
		//	mario1->mSpeed.y = 50.0f;
		//}
		//else {
			mario1->mSpeed.y = 300.0f;
			mario1->setAnimation(211, 1.0f);
			changePlayerStatus__6TMarioFUlUlb(mario1, 0x02000890, 0, 0);
			mario1->setStatusToJumping(0x02000890, 0);
		//}

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

		for (int i = 0; i < getPlayerCount(); i++){
			// Check if mario should be bounced
			TMario* mario = getMario(i);
			for (int j = 0; j < mario->mNumObjs; j++){
				if (GetType(mario->mCollidingObjs[j]) == 0x803dd660){
					
					TMario* collidingMario = (TMario*)getMario(i)->mCollidingObjs[j];
					TMario* currentMario = getMario(i);
					if(isPvpLevel()) {
						touchPlayerPvp(currentMario, collidingMario);
					}

					if (mario->mSpeed.y < 0.0f) {
						if (mario->mTranslation.y - MARIO_TRAMPLEHEIGHT > ((TMario*)mario->mCollidingObjs[j])->mTranslation.y){
							BounceMario(mario, (TMario*)(mario->mCollidingObjs[j]));
						}
					}
				}
			}

			//Remove held item from player collision
			if (mario->mHeldObject != 0)
				RemoveObjectFromColArray((THitActor*)mario, mario->mHeldObject);

		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80299af8, 0, 0, 0), OnCheckActorsHit);

	

		// Description: Collision check run for TMario
	void TOBjHitCheck_suffererIsInAttackArea(void* tObjHitCheck, THitActor* hitActor, THitActor* mario){
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* zMario = getMario(i);
			if(checkDistance__FRCQ29JGeometry8TVec3_f(hitActor->mAttackRadius, hitActor->mAttackHeight, zMario->mReceiveRadius, zMario->mReceiveHeight, hitActor->mTranslation, zMario->mTranslation)) {
				suffererIsInAttackArea__12TObjHitCheckFP9THitActorP9THitActor(tObjHitCheck, hitActor, zMario);
			}
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021bc00, 0, 0, 0), TOBjHitCheck_suffererIsInAttackArea);
	
	// Description: Overrides a hard coded check for distance to global mario and checks for all players instead
	// This fixes collision with e.g shines and allows luigi to collide with them
	int CheckDistance_Override(double param_1, double param_2, double param_3, double param_4, TVec3f* positionObj, TVec3f* positionPlayer) {
		int result = 0;
		for (int i = 0; i < getPlayerCount(); i++){
			TMario* mario = getMario(i);

			result = checkDistance__FRCQ29JGeometry8TVec3_f(param_1, param_2, param_3, param_4, positionObj, &mario->mTranslation);
			if(result) {
				return result;
			}
		}

		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8021bbe0, 0, 0, 0), CheckDistance_Override);
	
	// Description: Fixes all touch interactions with closest mario
	// E.g door 
	void TMapBaseObj_touchActor_override(void *a,THitActor *hitActor) {
		int currentMario = getPlayerId(gpMarioOriginal);
		int marioId = getClosestMarioId(&hitActor->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		touchActor__11TMapObjBaseFP9THitActor(a, hitActor);
		//perform__8TBaseNPCFUlPQ26JDrama9TGraphics(npc, perform_flags, graphics);
		setActiveMario(currentMario);
		setCamera(currentMario);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801aef14, 0, 0, 0), TMapBaseObj_touchActor_override);
	SMS_WRITE_32(SMS_PORT_REGION(0x803df8a4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803df73c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d822c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8024, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d7eb0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d7d40, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d7bd0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d75fc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d7194, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d7024, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d6ec0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d6d5c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d6900, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d679c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d65f4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d6484, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d6314, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d61a4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5fc8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5dec, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5c84, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5b20, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d59b8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5830, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5568, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5404, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5290, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d5120, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d4fb0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d4e4c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d4cdc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d4ad4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d4970, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d480c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d46a8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d453c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d43d0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d426c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3ce8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3b60, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d39fc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3898, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3728, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d35c4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3460, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d32fc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3120, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d2994, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d25fc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d219c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d1f40, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d1ddc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d1c78, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d1b14, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d19b0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d184c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d16e8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d1584, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d1420, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d11f0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d1088, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0f20, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0d18, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0bb4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0a50, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0710, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d05ac, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0448, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d02e4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d017c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0010, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cfea4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cfd40, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cfbbc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cfa38, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cf8d4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cf6cc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cf568, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cf3f8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cf294, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cef84, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cee20, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cecbc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ceb54, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce9f0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce80c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce6a8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce544, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce334, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce1c0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce04c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cdedc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cdd78, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cdc08, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cdaa4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cd940, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cd7dc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cd5d4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cd470, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cd190, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cd02c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ccec8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ccd64, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ccbf4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cc7c4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cc65c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cc4f8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cc384, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cc21c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cc03c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cbed8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cbd70, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cbc0c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cba9c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cb664, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cb4fc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cb320, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cb04c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803caee8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cad84, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803cab7c, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ca974, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ca810, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ca6ac, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ca488, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ca2a4, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ca0c0, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9edc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9cf8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9b14, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9930, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9570, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9304, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9074, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c8ecc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c8c64, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c2bfc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c25bc, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b7ee8, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b7d74, 0, 0, 0), (u32)(&TMapBaseObj_touchActor_override));
}