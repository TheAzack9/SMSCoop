#include <sdk.h>
#include <SMS/Player/Mario.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>

#include "players.hxx"

//void JAISound_setPitch(JAISound* sound, f32 pitch, u32 interpolation, u8 slot) {
//	sound->setPitch(pitch/2.0, interpolation, slot);
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x80013e18, 0, 0, 0), JAISound_setPitch);
//
u8 playerIdMakingSound = 0;
void SoundSESystem_startSoundActorInner(u32 soundId, JAISound** sound, JAIActor* origin, u32 param_4, u8 param_5) {
	//OSReport("Playing soundId %X\n", soundId);
	//if((u32)sound != 0x0 && *sound != nullptr) {
	//	JAISound* ptr = *sound;
	//	ptr->setPitch(0.7, 0, 0);
	//}

	// 78FE - yawn, but replaced with luigi climb
	// 788F - climb ledge
	// 780A and 0x78B9 is spin
	u32 overridenSoundId = soundId;
	if(playerIdMakingSound != 0) {
		if(soundId == 0x78ab || soundId == 0x7803 || soundId == 0x7807 || soundId == 0x78b1 || soundId == 0x7800 || soundId == 0x78b6 || soundId == 0x780A || soundId == 0x78B9) {
			overridenSoundId = 0x7901;
		}
		if(soundId == 0x7884) {
			overridenSoundId = 0x78fb;
		}
		if(soundId == 0x788F) {
			overridenSoundId = 0x78FE;
		}
	} else {
		if(soundId == 0x7901) {
			overridenSoundId = 0x78ab;
		}
		if(soundId == 0x78fb) {
			overridenSoundId = 0x7884;
		}
	}
	if(soundId != 0x78FE) {
		auto* newSound = MSoundSESystem::MSoundSE::startSoundActorInner(overridenSoundId, sound, origin, param_4, param_5);
		if(newSound && soundId == overridenSoundId && playerIdMakingSound != 0) {
			newSound->setPitch(0.891251, 3, 0); // Down two semitones
			newSound->setTempoProportion(1.3, 5);
			newSound->setVolume(1.0, 0, 0);
			//OSReport("changing pitch \n");
		}
	}
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80013dd4, 0, 0, 0), SoundSESystem_startSoundActorInner);

// I want to replace
// 0x7901? (for all three jumps, sounds odd to me... Mario version: 0x78ab, 0x7803, 0x7800)
// 
//
//JAISound* startMarioVoice_override(MSound* sound, u32 soundId, u16 unk, u8 unk2) {
//	
//	return sound->startMarioVoice(soundId, unk, unk2);
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x80012d38, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x801e6da0, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x8026e90c, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80285354, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x802853d8, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80285be8, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80285ca0, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286014, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286114, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x802861b4, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286200, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286348, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286398, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286774, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x802867b4, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286818, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286858, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x802868d4, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286c24, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286c64, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286d08, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286d48, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286da8, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286de8, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286e40, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286e94, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286f2c, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286f6c, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80286fc4, 0, 0, 0), startMarioVoice_override);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80287004, 0, 0, 0), startMarioVoice_override);


float pitchTest = 0.85;
bool TMario_startVoice(TMario* mario, u32 soundId) {

	bool isOnYoshi = mario->onYoshi();
	if(isOnYoshi) return false;
	playerIdMakingSound = SMSCoop::getPlayerId(mario);
	
	//OSReport("Playing soundId %X\n", soundId);
	
	return mario->startVoice(soundId);
	//u8 voiceStatus = mario->getVoiceStatus();
	//float* pitch = (float*)0x8040eecc;
	////*pitch = 0.1;
	//if(SMSCoop::getPlayerId(mario) == 0) {
	//	return gpMSound->startMarioVoice(soundId, mario->mHealth, voiceStatus);
	//}

	//if(gpMSound->gateCheck(soundId)) {
	//	auto* sound = MSoundSE::startSoundActor(soundId, mario->mTranslation, 0, nullptr, 0, 4);
	//	if(sound) {
	//		sound->setPitch(0.95, 0, 0);
	//		sound->setVolume(0.9, 0, 0);
	//		OSReport("Address %f\n", pitchTest);
	//	}
	//}
	////gpMSound->mSound->setPitch(0.1, 0, 0);

	//return 0;
}

SMS_PATCH_BL(SMS_PORT_REGION(0x8023f9a4, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8023fe70, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802401e4, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80240260, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80240f60, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80242d48, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80242d60, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80242d70, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80249af4, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024a7ac, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024aaa8, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024b53c, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024b728, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024c228, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024c6ec, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024fbb8, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80252244, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80252458, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80252804, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802534c4, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025353c, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802541a8, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025464c, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254684, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254694, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802546c8, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802546fc, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254730, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254764, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254788, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025483c, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254868, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025489c, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802548ac, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254900, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254988, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802549a8, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802549e0, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254a04, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254a20, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254a64, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254a88, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254aac, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254b54, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254d04, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80254d84, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80259024, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80259090, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025b388, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025d3ec, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025e01c, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025e900, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025e958, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025f3a8, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025f3c4, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025fe98, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80260370, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x8026039c, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802608e4, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80261024, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80265c28, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80265c38, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80282cf4, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80282d34, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x802837ac, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80284a38, 0, 0, 0), TMario_startVoice);
SMS_PATCH_BL(SMS_PORT_REGION(0x80284ae8, 0, 0, 0), TMario_startVoice);