#include <SMS/Strategic/HitActor.hxx>
#include <sdk.h>
#include "player.hxx"
#include <raw_fn.hxx>
#include <JSystem/JDrama/JDRActor.hxx>
#include <SMS/Enemy/SpineBase.hxx>

#define GetObjectFunction( object, func ) (void*)*(void**)((int)*(int*)object + func)

// TODO: Reset on stage load
char canRocketMountMario[2] = {1, 1};

void resetNpcLogic(TMarDirector *director) {
	for(int i = 0; i < 2; ++i) {
		canRocketMountMario[i] = 1;
	}
}

void TRocket_attackToMario(JDrama::TActor* rocket) {
	int cm = getClosestMarioId(&rocket->mTranslation);
	if(canRocketMountMario[cm] == 0) return;
	setActiveMario(cm);

	TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
	char* canHaveRocket = (char*)((int)liveManager + 0x60);
	*canHaveRocket = canRocketMountMario[cm];
	char* testing = (char*)((int)liveManager + 0x68);
	OSReport("TESTING %d %d %d %d \n", *canHaveRocket, *testing, canRocketMountMario[0], canRocketMountMario[1]);

	attackToMario__7TRocketFv(rocket);

	canRocketMountMario[cm] = *canHaveRocket;

	setActiveMario(0);
}
SMS_WRITE_32(SMS_PORT_REGION(0x803bcd08, 0, 0, 0), (u32)(&TRocket_attackToMario));

void TRocket_bind(JDrama::TActor* rocket, int __fd, void* __addr, void* __len) {
	int cm = getClosestMarioId(&rocket->mTranslation);
	setActiveMario(cm);
	
	TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
	char* canHaveRocket = (char*)((int)liveManager + 0x60);
	*canHaveRocket = canRocketMountMario[cm];

	bind__7TRocketFv(rocket);
	canRocketMountMario[cm] = *canHaveRocket;
	setActiveMario(0);
}
SMS_WRITE_32(SMS_PORT_REGION(0x803bcc6c, 0, 0, 0), (u32)(&TRocket_bind));

void TRocket_calcRootMatrix(JDrama::TActor* rocket) {
	int cm = getClosestMarioId(&rocket->mTranslation);
	setActiveMario(cm);
	calcRootMatrix__7TRocketFv(rocket);
	setActiveMario(0);
}
SMS_WRITE_32(SMS_PORT_REGION(0x803bcc60, 0, 0, 0), (u32)(&TRocket_calcRootMatrix));

void TRocket_setDeadAnm(JDrama::TActor* rocket) {
	int cm = getClosestMarioId(&rocket->mTranslation);
	TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
	char* canHaveRocket = (char*)((int)liveManager + 0x60);
	*canHaveRocket = canRocketMountMario[cm];

	setDeadAnm__7TRocketFv(rocket);

	canRocketMountMario[cm] = *canHaveRocket;
}
SMS_WRITE_32(SMS_PORT_REGION(0x803bccf4, 0, 0, 0), (u32)(&TRocket_setDeadAnm));

u32 TNerveRocketFly_execute(TNerveBase<JDrama::TActor>* nerveRocketFly, TSpineBase<JDrama::TActor>* rocketSpineBase) {
	JDrama::TActor* rocket = rocketSpineBase->mTarget;
	int cm = getClosestMarioId(&rocket->mTranslation);
	TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
	char* canHaveRocket = (char*)((int)liveManager + 0x60);
	*canHaveRocket = canRocketMountMario[cm];

	u32 result = execute__15TNerveRocketFlyCFP24TSpineBase_1(nerveRocketFly, rocketSpineBase);
	
	canRocketMountMario[cm] = *canHaveRocket;
	return result;
}
SMS_WRITE_32(SMS_PORT_REGION(0x803bcb8c, 0, 0, 0), (u32)(&TNerveRocketFly_execute));

u32 TNerveRocketPossessedNozzle_execute(TNerveBase<JDrama::TActor>* nerveRocketFly, TSpineBase<JDrama::TActor>* rocketSpineBase) {
	JDrama::TActor* rocket = rocketSpineBase->mTarget;
	int cm = getClosestMarioId(&rocket->mTranslation);
	TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
	char* canHaveRocket = (char*)((int)liveManager + 0x60);
	*canHaveRocket = canRocketMountMario[cm];
	
	TApplication *app      = &gpApplication;
	TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
	auto* p1Gamepad = director->mGamePads[0];
	director->mGamePads[0] = director->mGamePads[cm];

	u32 result =  execute__27TNerveRocketPossessedNozzleCFP24TSpineBase_1(nerveRocketFly, rocketSpineBase);
	
	director->mGamePads[0] = p1Gamepad;
	canRocketMountMario[cm] = *canHaveRocket;
	return result;
}
SMS_WRITE_32(SMS_PORT_REGION(0x803bcb9c, 0, 0, 0), (u32)(&TNerveRocketPossessedNozzle_execute));

typedef void (*controlFunc)(THitActor* smallEnemy);
void TSmallEnemy_control(THitActor* smallEnemy) {
	
	int cm = getClosestMarioId(&smallEnemy->mTranslation);
	setActiveMario(cm);
	
	
 	auto control = (controlFunc) GetObjectFunction(smallEnemy, 200);

	
	control(smallEnemy);

	//executeNerve(nerve, spineBase);
	setActiveMario(0);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8006c6f8, 0, 0, 0), TSmallEnemy_control);