#include <sdk.h>
#include <SMS/Player/Yoshi.hxx>

#define NTSCU
#include <raw_fn.hxx>

#include "players.hxx"
#include "yoshi.hxx"

namespace SMSCoop {

	bool isYoshiMounted(TYoshi* yoshi) {
		return yoshi->mState == TYoshi::MOUNTED;
	}

	void swapYoshis() {
		TYoshi* yoshi1 = getMario(0)->mYoshi;
		TYoshi* yoshi2 = getMario(1)->mYoshi;
		if(isYoshiMounted(yoshi1) || isYoshiMounted(yoshi2)) return;
		getMario(0)->mYoshi = yoshi2;
		getMario(1)->mYoshi = yoshi1;
		yoshi1->mMario = getMario(1);
		yoshi2->mMario = getMario(0);
	}

	void updateYoshi(TMarDirector *director) {
		if(!isSingleplayerLevel()) {
			swapYoshis();
		}
	}
	
	void TYoshi_appearFromEgg_override(TYoshi* yoshi, TVec3f* pos, float param_2, void* egg) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			appearFromEgg__6TYoshiFRCQ29JGeometry8TVec3_f(getMario(i)->mYoshi, pos, param_2, egg);
			pos->x += 120.0f;
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801bc730, 0, 0, 0), TYoshi_appearFromEgg_override);
}