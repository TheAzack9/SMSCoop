#include <sdk.h>
#define NTSCU
#include <raw_fn.hxx>

#include <SMS/Strategic/HitActor.hxx>
#include <JDrama/JDRGraphics.hxx>

#include "players.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"

namespace SMSCoop {
	TMario* marioEnteringGate = nullptr;

	void resetAi(TMarDirector *director) {
		marioEnteringGate = nullptr;
	}

	TMario* getMarioEnteringGate() {
		return marioEnteringGate;
	}

	void TBaseNPC_perform_override(THitActor* npc, u32 perform_flags, JDrama::TGraphics* graphics) {
			int marioId = getClosestMarioId(&npc->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		perform__8TBaseNPCFUlPQ26JDrama9TGraphics(npc, perform_flags, graphics);
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8468, 0, 0, 0), (u32)(&TBaseNPC_perform_override));
	

	// Fixes M gates entry
	void TModelGate_perform(THitActor* modelGate, u32 performFlags, JDrama::TGraphics* graphics) {
		int cm = getClosestMarioId(&modelGate->mTranslation);
		setActiveMario(cm);
		setCamera(cm);
		perform__10TModelGateFUlPQ26JDrama9TGraphics(modelGate, performFlags, graphics);
		
		setCamera(getActiveViewport());
		setActiveMario(getActiveViewport());
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
}