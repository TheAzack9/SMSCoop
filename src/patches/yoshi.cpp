#include <sdk.h>
#include <SMS/Player/Yoshi.hxx>
#include <SMS/Manager/ModelWaterManager.hxx>

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
	
	void setWaterColorForMario(TMario* mario) {
		TYoshi* yoshi = mario->mYoshi;
		if(isYoshiMounted(yoshi)) {
			gpModelWaterManager->mWaterCardType = yoshi->mType;
		} else {
			gpModelWaterManager->mWaterCardType = 0;
		}
	}

	bool TSmallEnemy_changeByJuice_override(TSpineEnemy* enemy) {
		int closestMarioId = getClosestMarioId(&enemy->mTranslation);
		TMario* closestMario = getMario(closestMarioId);
		setWaterColorForMario(closestMario);
		bool result = changeByJuice__11TSmallEnemyFv(enemy);
		setWaterColorForMario(gpMarioOriginal);
		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80052a04, 0, 0, 0), TSmallEnemy_changeByJuice_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x800b758c, 0, 0, 0), TSmallEnemy_changeByJuice_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x8012cfc0, 0, 0, 0), TSmallEnemy_changeByJuice_override);
	SMS_WRITE_32(SMS_PORT_REGION(0x803af66c, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803af824, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803af9dc, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803afee4, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b00ac, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b0604, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b07cc, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b0994, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b0b5c, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b18e0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b1b50, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b1f60, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b2258, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b24d8, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b28e0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b31c8, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b3470, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b40b8, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b4c28, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b4e28, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b5028, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b5228, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b53d4, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b672c, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b69d0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b6c78, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b6ef0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b8200, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b8768, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b8a54, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b99f0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b9c2c, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b9f3c, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ba1ac, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803ba688, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bb400, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bc3a0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bc5d8, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bccd0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcfbc, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bd4c4, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bdce4, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803be4d8, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803becc0, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bef08, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bf0dc, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803bf584, 0, 0, 0), (u32)(&TSmallEnemy_changeByJuice_override));
}