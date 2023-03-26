
#include <Dolphin/GX.h>
#include <macros.h>
#include <sdk.h>
#include <SMS/Player/Mario.hxx>
#include <raw_fn.hxx>

#include "player.hxx"

namespace SMSCoop {

	// VTable for TShine
	static u32* shineVTable = (u32*)0x803c97ec;

	u32 marioThatPickedShine = 0;
	// Description: Manually check which player is closest to the shine and select that as the person who "collected" it
	// We cannot just use the mario in param as that is the gp mario
	void TMario_receiveMessage_TShine_touchPlayer_override(TMario* mario, THitActor* shine, u32 param_3) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMarioById(i);
			mario->dropObject();
		}
		int marioId = getClosestMarioId(&shine->mTranslation);
		marioThatPickedShine = marioId;
		TMario* closestMario = getMarioById(marioId);
		closestMario->receiveMessage(shine, param_3);
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

	// Description: Check whether the shine has been touched
	// We check that both players are close enough to the shine to collect it. This is to balance the game a bit and not allow just one player to make it to the shine
	// TODO: Make optional for other mods
	void TShine_touchPlayer_override(TMapObjBase* shine, THitActor* mario) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* cMario = getMarioById(i);
			float dist = PSVECDistance((Vec*)&shine->mTranslation, (Vec*)&cMario->mTranslation);

			if(dist > 3000) {
				// TODO
				//if(balloonTimer <= 0) {
				//	TApplication* application = *gpApplication;
				//	MarDirector* marDirector = application->mDirector;
				//	TGCConmsole2_startAppearBallon(marDirector->console, 93, false);
				//	balloonTimer = 60 * 5 * 20; // about 5 seconds
				//}
				return;
			}
		}


		touchPlayer__6TShineFP9THitActor(shine, mario);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c9934, 0, 0, 0), (u32)(&TShine_touchPlayer_override));
}