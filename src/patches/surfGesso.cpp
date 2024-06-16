#include <sdk.h>
#include <raw_fn.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/MapObj/MapObjBase.hxx>
#include <SMS/M3DUtil/MActor.hxx>
#include <JSystem/J3D/J3DModel.hxx>
#include <Dolphin/GX_types.h>

#include "surfGesso.hxx"
#include "players.hxx"

#define MARIO_COUNT 2

namespace SMSCoop {
	
	int gessoTimer[MARIO_COUNT];
	
	// TODO: Reset between scene transitions?
	MActor* gesso0[MARIO_COUNT];
	MActor* gesso1[MARIO_COUNT];
	MActor* gesso2[MARIO_COUNT];

	void updateSurfGesso(TMarDirector *director) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			if(gessoTimer[i] > 0) {
				gessoTimer[i]--;
			}
		}
	}

	void TItem_touchPlayer_TSurfGessoObj(TItem* item, TMario* mario) {
		int mId = getPlayerId(mario);
		if(gessoTimer[mId] <= 0) {
			
			void* gpMapObjManager = *(void**)0x8040df08;
			*(MActor**)((int)gpMapObjManager + 0x9c) = gesso0[mId];
			*(MActor**)((int)gpMapObjManager + 0xa0) = gesso1[mId];
			*(MActor**)((int)gpMapObjManager + 0xa4) = gesso2[mId];

			mario->getGesso(item);


			gessoTimer[mId] = 60 * 5; // about 5 sec
		}

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ce8a8, 0, 0, 0), (u32)(&TItem_touchPlayer_TSurfGessoObj));


	MActor* SMS_MakeMActorFromSDLModelData_makeGesso0(void* sdlModelData, MActorAnmData* anmData, u32 param_3) {
		for(int i = 0; i < MARIO_COUNT; ++i) {
			MActor* gesso = (MActor*)SMS_MakeMActorFromSDLModelData__FP12SDLModelDataP13MActorAnmDataUl(sdlModelData, anmData, param_3);
			gesso0[i] = gesso;
		}
		return gesso0[0];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b7544, 0, 0, 0), SMS_MakeMActorFromSDLModelData_makeGesso0);

	void TMapObjBase_initPacketMatColor_gesso0(TMapObjBase* mapObj, J3DModel* model, _GXTevRegID regId, _GXColorS10* colors10) {
		for(int i = 0; i < MARIO_COUNT; ++i) {
			initPacketMatColor__11TMapObjBaseFP8J3DModel11_GXTevRegIDPC11_GXColorS10(*((u32*)gesso0[i] + 1), model, regId, colors10);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b7594, 0, 0, 0), TMapObjBase_initPacketMatColor_gesso0);

	
	MActor* SMS_MakeMActorFromSDLModelData_makeGesso1(void* sdlModelData, MActorAnmData* anmData, u32 param_3) {
		for(int i = 0; i < MARIO_COUNT; ++i) {
			MActor* gesso = (MActor*)SMS_MakeMActorFromSDLModelData__FP12SDLModelDataP13MActorAnmDataUl(sdlModelData, anmData, param_3);
			gesso1[i] = gesso;
		}
		return gesso1[0];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b7560, 0, 0, 0), SMS_MakeMActorFromSDLModelData_makeGesso1);

	void TMapObjBase_initPacketMatColor_gesso1(TMapObjBase* mapObj, J3DModel* model, _GXTevRegID regId, _GXColorS10* colors10) {
		for(int i = 0; i < MARIO_COUNT; ++i) {
			initPacketMatColor__11TMapObjBaseFP8J3DModel11_GXTevRegIDPC11_GXColorS10(*((u32*)gesso1[i] + 1), model, regId, colors10);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b75a8, 0, 0, 0), TMapObjBase_initPacketMatColor_gesso1);

	
	
	MActor* SMS_MakeMActorFromSDLModelData_makeGesso2(void* sdlModelData, MActorAnmData* anmData, u32 param_3) {
		for(int i = 0; i < MARIO_COUNT; ++i) {
			MActor* gesso = (MActor*)SMS_MakeMActorFromSDLModelData__FP12SDLModelDataP13MActorAnmDataUl(sdlModelData, anmData, param_3);
			gesso2[i] = gesso;
		}
		return gesso2[0];
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b757c, 0, 0, 0), SMS_MakeMActorFromSDLModelData_makeGesso2);

	void TMapObjBase_initPacketMatColor_gesso2(TMapObjBase* mapObj, J3DModel* model, _GXTevRegID regId, _GXColorS10* colors10) {
		for(int i = 0; i < MARIO_COUNT; ++i) {
			initPacketMatColor__11TMapObjBaseFP8J3DModel11_GXTevRegIDPC11_GXColorS10(*((u32*)gesso2[i] + 1), model, regId, colors10);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801b75bc, 0, 0, 0), TMapObjBase_initPacketMatColor_gesso2);
}