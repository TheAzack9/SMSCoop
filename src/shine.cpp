
#include <Dolphin/GX.h>
#include <macros.h>
#include <sdk.h>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>
#include <raw_fn.hxx>

#include "player.hxx"
#include "settings.hxx"

extern SMSCoop::ShineGrabDistanceSetting gShineGrabDistanceSetting;
namespace SMSCoop {
	
	// VTable for TShine
	static u32* shineVTable = (u32*)0x803c97ec;
	static int balloonTimer[2] = {0, 0};
	u32 marioThatPickedShine = 0;
	bool hasGottenShine = false;

	void resetShineLogic(TMarDirector* marDirector) {
		marioThatPickedShine = 0;
		hasGottenShine = false;
	}
	
	bool isShineGot() {
		return hasGottenShine;
	}

	// Description: Manually check which player is closest to the shine and select that as the person who "collected" it
	// We cannot just use the mario in param as that is the gp mario
	void TMario_receiveMessage_TShine_touchPlayer_override(TMario* mario, THitActor* shine, u32 param_3) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMarioById(i);
			mario->dropObject();
		}
		int marioId = getClosestMarioId(&shine->mTranslation);
		TMario* closestMario = getMarioById(marioId);
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
		setActiveMario(0);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c98b4, 0, 0, 0), (u32)(&TShine_control_override));

	void updateShineTimer(TMarDirector* marDirector) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			if(balloonTimer[i] > 0) {
				balloonTimer[i]--;
				if(balloonTimer[i] == 0) {
					startDisappearBalloon__11TGCConsole2FUlb(getConsoleForPlayer(i), 93, true);
				}
			}
		}
	}

	// Description: Check whether the shine has been touched
	// We check that both players are close enough to the shine to collect it. This is to balance the game a bit and not allow just one player to make it to the shine
	// TODO: Make optional for other mods
	void TShine_touchPlayer_override(TMapObjBase* shine, THitActor* mario) {
		OSReport("Is shine Got %d, talking player %X address of shine got %X\n", isShineGot(), getTalkingPlayer(), &hasGottenShine);
		if(isShineGot() || getTalkingPlayer() != nullptr) return;

		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* cMario = getMarioById(i);
			float dist = PSVECDistance((Vec*)&shine->mTranslation, (Vec*)&cMario->mTranslation);

			if(dist > 3000 && gShineGrabDistanceSetting.getBool()) {
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