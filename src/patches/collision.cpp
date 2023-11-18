#include <sdk.h>

#define NTSCU
#include <raw_fn.hxx>

#include <SMS/Strategic/HitActor.hxx>
#include <SMS/Player/Mario.hxx>

#include "players.hxx"

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

		for (int i = 0; i < getPlayerCount(); i++){
			// Check if mario should be bounced
			TMario* mario = getMario(i);
			if (mario->mSpeed.y < 0.0f) {
				for (int j = 0; j < mario->mNumObjs; j++){
					if (GetType(mario->mCollidingObjs[j]) == 0x803dd660){
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
}