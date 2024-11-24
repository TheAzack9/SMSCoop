#include <sdk.h>
#include <macros.h>

#include <raw_fn.hxx>

#include <SMS/System/MarDirector.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>

#include "players.hxx"
#include "camera.hxx"
#include "talking.hxx"
#include "gui.hxx"
#include "splitscreen.hxx"
#include "settings.hxx"
#include "module.hxx"

extern SMSCoop::ShineGrabDistanceSetting gShineGrabDistanceSetting;
namespace SMSCoop {
	
	static int balloonTimer[2] = {0, 0};
	u32 marioThatPickedShine = 0;
	bool hasGottenShine = false;
	bool isShineCutscene = false;

	void resetShineLogic(TMarDirector* marDirector) {
		marioThatPickedShine = 0;
		hasGottenShine = false;
		isShineCutscene = false;
	}
	
	bool isShineGot() {
		return hasGottenShine;
	}

	int getMarioThatPickedShine() {
		return marioThatPickedShine;
	}

	void setShineCutscene(bool setIsShineCutscene) {
		isShineCutscene = setIsShineCutscene;
	}

	// Description: Manually check which player is closest to the shine and select that as the person who "collected" it
	// We cannot just use the mario in param as that is the gp mario
	void TMario_receiveMessage_TShine_touchPlayer_override(TMario* mario, THitActor* shine, u32 param_3) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMario(i);
			mario->dropObject();
		}
		int marioId = getClosestMarioId(&shine->mTranslation);
		TMario* closestMario = getMario(marioId);
		int response = closestMario->receiveMessage(shine, param_3);
		if(response == 1) {
			marioThatPickedShine = marioId;
			hasGottenShine = true;
		} 
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801bd360, 0, 0, 0), TMario_receiveMessage_TShine_touchPlayer_override);

	// Description: We need to set the mario that collected the shine for it to render correctly in cutscene
	void TShine_control_override(TShine* shine) {
		setActiveMario(marioThatPickedShine);
		control__6TShineFv(shine);
		setActiveMario(getActiveViewport());
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c98b4, 0, 0, 0), (u32)(&TShine_control_override));

	void updateShineTimer(TMarDirector* marDirector) {
		bool isAnyMarioInShineAnimation = false;

		bool isAnyInDemo = false;

		for(int i = 0; i < getPlayerCount(); ++i) {
			if(balloonTimer[i] > 0) {
				balloonTimer[i]--;
				if(balloonTimer[i] == 0) {
					startDisappearBalloon__11TGCConsole2FUlb(getConsoleForPlayer(i), 93, true);
				}
			}

			if(getMario(i)->mState == TMario::STATE_SHINE_C) {
				isAnyMarioInShineAnimation = true;
			}

			if(isShineCutscene) {
				CPolarSubCamera* camera = getCameraById(i);
				if(camera->getRestDemoFrames() != 0) {
					isAnyInDemo = true;
				} else {
					if(i != 0) {
						TMario* mario = getMario(i);
						if (SMS_isDivingMap__Fv() || (mario->mPrevState & 0x20D0) == 0x20D0)
							mario->mState = mario->mPrevState;
						else
							mario->mState = static_cast<u32>(TMario::STATE_IDLE);
						camera->endDemoCamera();
					}
				}
			}
			
		}
		//OSReport("HMMM %d %d %d %X\n", hasGottenShine, isAnyInDemo, isShineCutscene, marDirector->mCollectedShine);

		// Fix no-bootout shines in a hacky way...
		if(hasGottenShine && isShineCutscene && !isAnyInDemo) {
			resetShineLogic(marDirector);
		}
	}

	// Description: Check whether the shine has been touched
	// We check that both players are close enough to the shine to collect it. This is to balance the game a bit and not allow just one player to make it to the shine
	// TODO: Make optional for other mods
	void TShine_touchPlayer_override(TMapObjBase* shine, THitActor* mario) {
		//OSReport("Is shine Got %d, talking player %d address of shine got %d\n", isShineGot(), getTalkingPlayer(), hasGottenShine);
		if(isShineGot() || isTalking()) return;

		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* cMario = getMario(i);
			float dist = PSVECDistance((Vec*)&shine->mTranslation, (Vec*)&cMario->mTranslation);

			if(dist > 3000 && (gShineGrabDistanceSetting.getBool())) {
				// TODO
				if(balloonTimer[i] <= 0) {
					startAppearBalloon__11TGCConsole2FUlb(getConsoleForPlayer(i), 93, false);
					balloonTimer[i] = 60 * 5; // about 5 seconds
				}
				return;
			}
		}

		touchPlayer__6TShineFP9THitActor(shine, mario);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9934, 0, 0, 0), (u32)(&TShine_touchPlayer_override));
}