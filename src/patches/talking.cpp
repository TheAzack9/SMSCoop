#include <sdk.h>
#include <macros.h>
#include <raw_fn.hxx>

#include <SMS/Player/Mario.hxx>
#include <SMS/SPC/SpcInterp.hxx>
#include <SMS/G2D/BoundPane.hxx>
#include <SMS/GC2D/Talk2D2.hxx>
#include <JSystem/JDrama/JDRGraphics.hxx>
#include <SMS/System/Application.hxx>

#include "players.hxx"
#include "splitscreen.hxx"
#include "camera.hxx"
#include "talking.hxx"
#include "ai.hxx"

namespace SMSCoop {
	int marioIdTalking = -1;
	THitActor* nearestNpc = nullptr;

	int playerIdPerFrame = 0;

	void resetTalking(TMarDirector *director) {
		marioIdTalking = -1;
		nearestNpc = nullptr;
	}

	bool isTalking() {
		return marioIdTalking != -1;
	}

	int getTalkingPlayer() {
		return marioIdTalking;
	}

	void checkTalking(TMarDirector* marDirector) {
		bool someoneTalking = false;
		for(int i = 0; i < getPlayerCount(); ++i) {
			if(getMario(i)->mState == TMario::State::STATE_TALKING) {
				someoneTalking = true;	
			}
		}

		if(!someoneTalking) {
			marioIdTalking = -1;
			for(int i = 0; i < getPlayerCount(); ++i) {
				*((u32*)&marDirector->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
			}
		}

	}

	void handleTalking(TMarDirector* marDirector, THitActor* npc, u32 initiatingPlayer = 0) {
		u8 initiatingTalking = *(u8*)((u32)marDirector + 0x126);

		checkTalking(marDirector);

		if(initiatingTalking && !isTalking()) {
			marioIdTalking = initiatingPlayer;

		}
		
		for(int i = 0; i < getPlayerCount(); ++i) {
			if(isTalking()) {
				if(i != marioIdTalking) {
					*((u32*)&marDirector->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
					*((u32*)&marDirector->mGamePads[i]->mState) &= ~0x40000; // Player cannot talk when another is talking
				} else {
					*((u32*)&marDirector->mGamePads[i]->mState) |= 0x80000; // Player is not talking can move
				}
			}
		}
	}

	
	void updateTalking(TMarDirector *director) {
		bool someoneTalking = false;
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMario(i);
			if(mario->mState == TMario::State::STATE_TALKING) {
				someoneTalking = true;	
			}

			if(mario->mState != TMario::State::STATE_TALKING) {
				*((u32*)&director->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
			}
				
		}
			
		if(!someoneTalking) {
			marioIdTalking = -1;
			for(int i = 0; i < getPlayerCount(); ++i) {
				*((u32*)&director->mGamePads[i]->mState) &= ~0x80000; // Player is not talking
			}
		}

		// Failsafe in case missed talking flag, then start talking with someone instead of softlocking.
		if(!isTalking() && director->mTalkingNPC != nullptr) {
			handleTalking(director, director->mTalkingNPC, getClosestMarioId(&director->mTalkingNPC->mTranslation));
		}
	}
	
	void TMarDirector_movement_game_override(TMarDirector* marDirector) {

		if(getPlayerCount() > 1) {

			TMarioGamePad* p1Gamepad = marDirector->mGamePads[0];
			marDirector->mGamePads[0] = marDirector->mGamePads[getActiveViewport()];

			u32* addressOfThing = marDirector->findNearestTalkNPC();

			u32 frameMeaning = marDirector->mGamePads[0]->mFrameMeaning;
			marDirector->mGamePads[0]->mFrameMeaning = marDirector->mGamePads[0]->mMeaning;
			
			*((u32*)&marDirector->mGamePads[0]->mState) &= ~0x100000; // Allow to move during cutscenes
			
			movement_game__12TMarDirectorFv(marDirector);
			if(nearestNpc != nullptr) {
				handleTalking(marDirector, nearestNpc, getActiveViewport());
			}

			marDirector->mGamePads[0]->mFrameMeaning = frameMeaning;
			marDirector->mGamePads[0] = p1Gamepad;
		} else {
			movement_game__12TMarDirectorFv(marDirector);

		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029a4c8, 0, 0, 0), TMarDirector_movement_game_override);

	// Fix talking
	SMS_WRITE_32(SMS_PORT_REGION(0x8029a90c, 0, 0, 0), 0x60000000);

	void changePlayerStatusToTalking(TMario* mario, u32 state, u32 jumpSlipState, bool isGrounded) {
		if(isTalking() && mario == getMario(marioIdTalking)) {
			mario->changePlayerStatus(state, jumpSlipState, isGrounded);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8024deb8, 0, 0, 0), changePlayerStatusToTalking);
	
	THitActor* findNearestTalkNPC(TMarDirector* marDirector) {
		nearestNpc = (THitActor*)marDirector->findNearestTalkNPC();
		return nearestNpc;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029a8c8, 0, 0, 0), findNearestTalkNPC);

	
	// Disable hover being disabled when talking
	SMS_WRITE_32(SMS_PORT_REGION(0x80269668, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80269670, 0, 0, 0), 0x60000000);

	// Sunscript hooks
	
	void forceStartTalkExceptNpc(TSpcInterp *interp, u32 argc) {
		
		THitActor *target = reinterpret_cast<THitActor *>(interp->mSlices[0].mValue);
		int talkingPlayer = getClosestMarioId(&target->mTranslation);
		ev__ForceStartTalkExceptNpc__FP32TSpcTypedInterp_1(interp, argc);
		
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		handleTalking(director, target, talkingPlayer);
	}


	void forceStartTalk(TSpcInterp *interp, u32 argc) {
		
		THitActor *target = reinterpret_cast<THitActor *>(interp->mSlices[0].mValue);
		int talkingPlayer = getClosestMarioId(&target->mTranslation);
		ev__ForceStartTalk__FP32TSpcTypedInterp_1(interp, argc);
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		handleTalking(director, target, talkingPlayer);
	}

	void isNearActors(TSpcInterp *interp, u32 argc) {

		TMario* theMario = nullptr;
		
		for(int i = 0; i < argc; ++i) {
			TSpcSlice& slice = interp->mSlices[interp->mSlicesCount - argc];
			if(slice.mType == 0) {
				THitActor *a = reinterpret_cast<THitActor *>(slice.mValue);
				if(getMario(0) == a) {
					interp->mSlices[interp->mSlicesCount - argc].mValue = (u32)getMario(playerIdPerFrame);
				}
			} else if (slice.mType == 2) {
				u16 keyCode = JDrama::TNameRef::calcKeyCode((const char*)slice.mValue);
				if(keyCode == getMario(i)->mKeyCode) {
					slice.mValue = (u32)getMario(playerIdPerFrame)->mKeyName;
				}
			}
		}

		evIsNearActors__FP32TSpcTypedInterp_1(interp, argc);

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
	
	// Description: Ensure that the talking textbox only appears for the talking mario
	void TTalk2D2_perform(Talk2D2* talk2d, u32 renderFlags, JDrama::TGraphics* graphics) {
		int i = getActiveViewport();
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		if(isTalking()) {

			// Gamepad for progressing talking
			((TMarioGamePad **)talk2d)[0x24C / 4] = director->mGamePads[marioIdTalking];

			if(i == marioIdTalking || renderFlags & 0x1) {
				// Called twice for fps :c
				setActiveMario(marioIdTalking);
				setCamera(marioIdTalking);
				perform__8TTalk2D2FUlPQ26JDrama9TGraphics(talk2d, renderFlags, graphics);

				setActiveMario(i);
				setCamera(i);
			} else {
				perform__8TTalk2D2FUlPQ26JDrama9TGraphics(talk2d, renderFlags & ~8, graphics);
			}
		}

		if(renderFlags & 0x1) {
			playerIdPerFrame = (playerIdPerFrame+1) % getPlayerCount();
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c03e8, 0, 0, 0), (u32)(&TTalk2D2_perform));

	
	// Make the enter M cutscene play on the correct screen
	u8 updateGameMode(TMarDirector* marDirector) {
        TMario* marioEnteringGate = getMarioEnteringGate();
		if(marioEnteringGate) {
			int cm = getPlayerId(marioEnteringGate);
			setActiveMario(cm);
			setCamera(cm);
		}
		if(isTalking()) {
			setActiveMario(marioIdTalking);
			setCamera(marioIdTalking);
		}

		u8 result = updateGameMode__12TMarDirectorFv(marDirector);

		int view = getActiveViewport();
		setActiveMario(view);
		setCamera(view);
		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80299140, 0, 0, 0), updateGameMode);

	// Description: Make mario invincible when talking
	// FIXME: Better way to do this than override everywhere?
	bool isInvincible(TMario* mario) {
		return mario->isInvincible() || (isTalking() && getMario(marioIdTalking) == mario);
		
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
		for(int j = 0; j < getPlayerCount(); ++j) {
			TMario* mario = getMario(j);
			for(int i = 0; i < length; ++i) {
				TBaseNPC* npc = npcManager->npcs[i];
				if(PSVECDistance((Vec*)&npc->mTranslation, (Vec*)&mario->mTranslation) < 500) {
					npc->mStateFlags.asU32 = npc->mStateFlags.asU32 & 0xfffffffb;
					npc->mStateFlags.asFlags.mCanTalk = true;
				}

				// Fix crashes where getting shine or shine spawning when talking to npc 
				//if(isShineGot()) {
				//	npc->mStateFlags.asU32 |= 4;
				//}
			}
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d92e0, 0, 0, 0), (u32)(&TBoardNpcManager_clipActors));

	void TNPCManager_clipEnemies(NPCManager* npcManager, JDrama::TGraphics* graphics) {
	
		clipEnemies__11TNPCManagerFPQ26JDrama9TGraphics(npcManager, graphics);
		int length = npcManager->length;
		for(int j = 0; j < getPlayerCount(); ++j) {
			TMario* mario = getMario(j);
			for(int i = 0; i < length; ++i) {
				TBaseNPC* npc = npcManager->npcs[i];
				if(PSVECDistance((Vec*)&npc->mTranslation, (Vec*)&mario->mTranslation) < 500) {
					npc->mStateFlags.asU32 = npc->mStateFlags.asU32 & 0xfffffffb;
					npc->mStateFlags.asFlags.mCanTalk = true;
				}

				//// Fix crashes where getting shine or shine spawning when talking to npc 
				//if(isShineGot()) {
				//	npc->mStateFlags.asU32 |= 4;
				//}
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
}